#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <omp.h>

struct Pixel { uint8_t r, g, b; };
using Clock = std::chrono::high_resolution_clock;

static Pixel color_map(int iter, int max_iter) {
    if (iter >= max_iter) return {0,0,0};
    double t = static_cast<double>(iter) / max_iter;
    return {static_cast<uint8_t>(9 * (1-t) * t*t*t * 255),
            static_cast<uint8_t>(15 * (1-t)*(1-t) * t*t * 255),
            static_cast<uint8_t>(8.5 * (1-t)*(1-t)*(1-t) * t * 255)};
}

static void save_ppm(const std::string& path, const std::vector<Pixel>& img, int w, int h) {
    std::ofstream out(path, std::ios::binary);
    out << "P6\n" << w << " " << h << "\n255\n";
    out.write(reinterpret_cast<const char*>(img.data()), img.size()*sizeof(Pixel));
}

static void set_schedule(const std::string& scheduler, int chunk) {
    omp_sched_t kind = omp_sched_static;
    if (scheduler == "dynamic") kind = omp_sched_dynamic;
    else if (scheduler == "guided") kind = omp_sched_guided;
    else kind = omp_sched_static;
    omp_set_schedule(kind, chunk > 0 ? chunk : 1);
}

static void mandelbrot_omp(std::vector<Pixel>& img, int w, int h, int max_iter) {
    const double xmin=-2.0, xmax=1.0, ymin=-1.2, ymax=1.2;
    #pragma omp parallel for schedule(runtime)
    for (int y=0; y<h; ++y) {
        double cy = ymin + (ymax-ymin) * y / (h-1);
        for (int x=0; x<w; ++x) {
            double cx = xmin + (xmax-xmin) * x / (w-1);
            double zx=0.0, zy=0.0;
            int iter=0;
            while (zx*zx + zy*zy <= 4.0 && iter < max_iter) {
                double tmp = zx*zx - zy*zy + cx;
                zy = 2.0*zx*zy + cy;
                zx = tmp;
                ++iter;
            }
            img[y*w+x] = color_map(iter, max_iter);
        }
    }
}

static void gaussian_blur_spmd(const std::vector<Pixel>& in, std::vector<Pixel>& out, int w, int h, int radius) {
    const double sigma = radius / 2.0;
    std::vector<double> kernel(2*radius+1);
    double sum=0.0;
    for (int i=-radius; i<=radius; ++i) { double v=std::exp(-(i*i)/(2*sigma*sigma)); kernel[i+radius]=v; sum+=v; }
    for (double& v: kernel) v/=sum;
    std::vector<Pixel> tmp(w*h);

    #pragma omp parallel for schedule(static)
    for (int y=0; y<h; ++y) {
        for (int x=0; x<w; ++x) {
            double r=0,g=0,b=0;
            #pragma omp simd reduction(+:r,g,b)
            for (int k=-radius; k<=radius; ++k) {
                int xx = x+k; if (xx<0) xx=0; if (xx>=w) xx=w-1;
                Pixel p = in[y*w+xx]; double kv = kernel[k+radius];
                r += kv*p.r; g += kv*p.g; b += kv*p.b;
            }
            tmp[y*w+x] = {(uint8_t)r,(uint8_t)g,(uint8_t)b};
        }
    }

    #pragma omp parallel for schedule(static)
    for (int y=0; y<h; ++y) {
        for (int x=0; x<w; ++x) {
            double r=0,g=0,b=0;
            #pragma omp simd reduction(+:r,g,b)
            for (int k=-radius; k<=radius; ++k) {
                int yy = y+k; if (yy<0) yy=0; if (yy>=h) yy=h-1;
                Pixel p = tmp[yy*w+x]; double kv = kernel[k+radius];
                r += kv*p.r; g += kv*p.g; b += kv*p.b;
            }
            out[y*w+x] = {(uint8_t)r,(uint8_t)g,(uint8_t)b};
        }
    }
}

static std::array<uint64_t, 256> histogram_atomic(const std::vector<Pixel>& img) {
    std::array<uint64_t,256> hist{};
    #pragma omp parallel for schedule(static)
    for (size_t i=0;i<img.size();++i) {
        uint8_t gray = static_cast<uint8_t>((img[i].r + img[i].g + img[i].b)/3);
        #pragma omp atomic
        hist[gray]++;
    }
    return hist;
}

static std::array<uint64_t, 256> histogram_local(const std::vector<Pixel>& img) {
    int T = omp_get_max_threads();
    struct alignas(64) PaddedHist { uint64_t v[256] = {0}; };
    std::vector<PaddedHist> locals(T);
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        #pragma omp for schedule(static)
        for (size_t i=0;i<img.size();++i) {
            uint8_t gray = static_cast<uint8_t>((img[i].r + img[i].g + img[i].b)/3);
            locals[tid].v[gray]++;
        }
    }
    std::array<uint64_t,256> hist{};
    for (int t=0;t<T;++t) for(int i=0;i<256;++i) hist[i]+=locals[t].v[i];
    return hist;
}

static void save_hist(const std::string& path, const std::array<uint64_t,256>& h) {
    std::ofstream out(path); out << "intensity,count\n";
    for (int i=0;i<256;++i) out << i << "," << h[i] << "\n";
}

int main(int argc, char** argv) {
    int w = argc > 1 ? std::stoi(argv[1]) : 7680;
    int h = argc > 2 ? std::stoi(argv[2]) : 4320;
    int it = argc > 3 ? std::stoi(argv[3]) : 1000;
    int radius = argc > 4 ? std::stoi(argv[4]) : 15;
    std::string scheduler = argc > 5 ? argv[5] : "static";
    int chunk = argc > 6 ? std::stoi(argv[6]) : 32;
    set_schedule(scheduler, chunk);

    std::vector<Pixel> img(w*h), filtered(w*h);
    auto t0=Clock::now(); mandelbrot_omp(img,w,h,it); auto t1=Clock::now();
    gaussian_blur_spmd(img,filtered,w,h,radius); auto t2=Clock::now();
    auto h0=Clock::now(); auto ha=histogram_atomic(filtered); auto h1=Clock::now();
    auto hl=histogram_local(filtered); auto h2=Clock::now();

    save_ppm("images/optimized_final.ppm", filtered, w, h);
    save_hist("results/histogram_local.csv", hl);
    save_hist("results/histogram_atomic.csv", ha);
    double tA=std::chrono::duration<double>(t1-t0).count();
    double tB=std::chrono::duration<double>(t2-t1).count();
    double tHa=std::chrono::duration<double>(h1-h0).count();
    double tHl=std::chrono::duration<double>(h2-h1).count();
    std::cout << "version,optimized,threads," << omp_get_max_threads()
              << ",scheduler," << scheduler << ",chunk," << chunk
              << ",width," << w << ",height," << h
              << ",taskA," << tA << ",taskB," << tB
              << ",hist_atomic," << tHa << ",hist_local," << tHl
              << ",total," << (tA+tB+tHl) << "\n";
}
