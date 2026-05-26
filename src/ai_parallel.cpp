#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <omp.h>

struct Pixel { uint8_t r, g, b; };
using Clock = std::chrono::high_resolution_clock;

static Pixel color_map(int iter, int max_iter) {
    if (iter >= max_iter) return {0, 0, 0};
    double t = static_cast<double>(iter) / max_iter;
    return {static_cast<uint8_t>(9 * (1-t) * t*t*t * 255),
            static_cast<uint8_t>(15 * (1-t)*(1-t) * t*t * 255),
            static_cast<uint8_t>(8.5 * (1-t)*(1-t)*(1-t) * t * 255)};
}

static void save_ppm(const std::string& path, const std::vector<Pixel>& img, int w, int h) {
    std::ofstream out(path, std::ios::binary);
    out << "P6\n" << w << " " << h << "\n255\n";
    out.write(reinterpret_cast<const char*>(img.data()), img.size() * sizeof(Pixel));
}

static void mandelbrot(std::vector<Pixel>& img, int w, int h, int max_iter) {
    const double xmin=-2.0, xmax=1.0, ymin=-1.2, ymax=1.2;
    #pragma omp parallel for schedule(static)
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

static void gaussian_blur_heavy(const std::vector<Pixel>& in, std::vector<Pixel>& out, int w, int h, int radius) {
    const double sigma = radius / 2.0;
    std::vector<double> kernel(2*radius+1);
    double sum=0.0;
    for (int i=-radius; i<=radius; ++i) {
        double v = std::exp(-(i*i)/(2*sigma*sigma));
        kernel[i+radius] = v; sum += v;
    }
    for (double& v: kernel) v /= sum;

    std::vector<Pixel> tmp(w*h);
    #pragma omp parallel for schedule(static)
    for (int y=0; y<h; ++y) {
        for (int x=0; x<w; ++x) {
            double r=0,g=0,b=0;
            for (int k=-radius; k<=radius; ++k) {
                int xx = std::clamp(x+k, 0, w-1);
                Pixel p = in[y*w+xx];
                double kv = kernel[k+radius];
                r += kv*p.r; g += kv*p.g; b += kv*p.b;
            }
            tmp[y*w+x] = {(uint8_t)r,(uint8_t)g,(uint8_t)b};
        }
    }
    #pragma omp parallel for schedule(static)
    for (int y=0; y<h; ++y) {
        for (int x=0; x<w; ++x) {
            double r=0,g=0,b=0;
            for (int k=-radius; k<=radius; ++k) {
                int yy = std::clamp(y+k, 0, h-1);
                Pixel p = tmp[yy*w+x];
                double kv = kernel[k+radius];
                r += kv*p.r; g += kv*p.g; b += kv*p.b;
            }
            out[y*w+x] = {(uint8_t)r,(uint8_t)g,(uint8_t)b};
        }
    }
}

int main(int argc, char** argv) {
    int w = argc > 1 ? std::stoi(argv[1]) : 7680;
    int h = argc > 2 ? std::stoi(argv[2]) : 4320;
    int it = argc > 3 ? std::stoi(argv[3]) : 1000;
    int radius = argc > 4 ? std::stoi(argv[4]) : 15;
    std::vector<Pixel> img(w*h), filtered(w*h);

    auto t0 = Clock::now();
    mandelbrot(img, w, h, it);
    auto t1 = Clock::now();
    gaussian_blur_heavy(img, filtered, w, h, radius);
    auto t2 = Clock::now();
    save_ppm("images/sequential_final.ppm", filtered, w, h);

    double tA = std::chrono::duration<double>(t1-t0).count();
    double tB = std::chrono::duration<double>(t2-t1).count();
    std::cout << "version,ai_parallel,threads," << omp_get_max_threads() << ",scheduler,static,chunk,default,width," << w << ",height," << h
              << ",taskA," << tA << ",taskB," << tB << ",total," << (tA+tB) << "\n";
}
