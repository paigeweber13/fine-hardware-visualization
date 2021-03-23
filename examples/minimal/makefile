CXX=g++

SHARED_LIB_DIR=/usr/local/lib

run: fhv_minimal
	LD_LIBRARY_PATH=$(SHARED_LIB_DIR) ./fhv_minimal

all: fhv_minimal

clean:
	rm -f fhv_minimal

fhv_minimal: fhv_minimal.cpp
	$(CXX) $^ -lfhv_perfmon -llikwid -std=c++14 -fopenmp -Wall -march=native \
		-mtune=native -O3 -o $@

