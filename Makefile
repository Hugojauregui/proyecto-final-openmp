CXX=g++
CXXFLAGS=-O3 -fopenmp -std=c++17 -march=native -Wall -Wextra
VECFLAGS=-fopt-info-vec-optimized=results/vectorization_report.txt -fopt-info-vec-missed=results/vectorization_missed.txt

all: bin/sequential bin/ai_parallel bin/optimized

bin:
	mkdir -p bin results images

bin/sequential: src/sequential.cpp | bin
	$(CXX) $(CXXFLAGS) $< -o $@

bin/ai_parallel: src/ai_parallel.cpp | bin
	$(CXX) $(CXXFLAGS) $< -o $@

bin/optimized: src/optimized_openmp.cpp | bin
	$(CXX) $(CXXFLAGS) $(VECFLAGS) $< -o $@

clean:
	rm -rf bin results/*.csv results/*.txt results/*.png images/*.ppm
