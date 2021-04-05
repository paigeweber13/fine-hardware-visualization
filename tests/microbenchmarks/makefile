CXX=g++


# for use by this example
SRCS=main.cpp
BUILD_DIR=bin

FHV_PREFIX=/usr/local

CXX_FLAGS=-I$(FHV_PREFIX)/include -std=c++14 -fopenmp -Wall -g 
CXX_COMMAND=$(CXX) $(CXX_FLAGS) -c $< -o $@
LD_FLAGS=-L$(FHV_PREFIX)/lib -lfhv_perfmon

OBJS=$(SRCS:%.cpp=$(BUILD_DIR)/%.o)
EXEC=$(BUILD_DIR)/microbenchmarks


# for compiling likwid's microbenchmarks
LIKWID_SRC_ROOT=./likwid/bench
BENCHMARKS=peakflops_avx_fma peakflops_sp_avx_fma copy_avx load_avx store_avx
BENCHMARK_OBJS=$(addsuffix .o,$(addprefix GCC/, $(BENCHMARKS)))


# building this example
all: benchmarks $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(OBJS) $(LD_FLAGS) -o $@

$(BUILD_DIR)/%.o: ./%.cpp ./%.hpp | $(BUILD_DIR)
	$(CXX_COMMAND)

$(BUILD_DIR)/%.o: ./%.cpp | $(BUILD_DIR)
	$(CXX_COMMAND)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)


# Building likwid things

# this rule will also create GCC/testcases.h
benchmarks:
	cd $(LIKWID_SRC_ROOT) && make $(BENCHMARK_OBJS) -j$(shell nproc)

clean:
	@cd $(LIKWID_SRC_ROOT) && make distclean
	@rm -f $(OBJS) $(EXEC)

