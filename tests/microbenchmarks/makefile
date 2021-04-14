CXX=g++
# CXX=gcc

# for use by this example
SRCS=main.cpp
BUILD_DIR=bin

FHV_PREFIX=/usr/local


### for building this example
CXX_FLAGS=-I$(FHV_PREFIX)/include -I$(LIKWID_SRC_ROOT)/GCC \
	-I$(LIKWID_SRC_ROOT)/includes -std=c++14 -fopenmp -Wall -g 
CXX_COMMAND=$(CXX) $(CXX_FLAGS) -c $< -o $@
LD_FLAGS=-L$(FHV_PREFIX)/lib -lfhv_perfmon -fopenmp

OBJS=$(SRCS:%.cpp=$(BUILD_DIR)/%.o)
EXEC=$(BUILD_DIR)/microbenchmarks

# rules for building this example
all: benchmarks $(EXEC)

$(EXEC): $(OBJS) $(BENCHMARK_OBJS)
	$(CXX) $(OBJS) $(BENCHMARK_OBJS) $(LD_FLAGS) -o $@

$(BUILD_DIR)/%.o: ./%.cpp ./%.hpp | $(BUILD_DIR)
	$(CXX_COMMAND)

$(BUILD_DIR)/%.o: ./%.cpp | $(BUILD_DIR)
	$(CXX_COMMAND)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)


### for compiling likwid's microbenchmarks
LIKWID_SRC_ROOT=./likwid/bench
LIKWID_BENCHMARK_ROOT=$(LIKWID_SRC_ROOT)/GCC
BENCHMARKS=peakflops_avx_fma peakflops_sp_avx_fma copy_avx load_avx store_avx
BENCHMARK_OBJS=$(addsuffix .o,$(addprefix $(LIKWID_BENCHMARK_ROOT)/, $(BENCHMARKS)))
BENCHMARK_COMPILE_COMMAND=cd $(LIKWID_SRC_ROOT) && make $@

# this rule will also create GCC/testcases.h
benchmarks: $(BENCHMARK_OBJS)

$(LIKWID_BENCHMARK_ROOT)/peakflops_avx_fma.o:
	$(BENCHMARK_COMPILE_COMMAND)


clean:
	@cd $(LIKWID_SRC_ROOT) && make distclean
	@rm -f $(OBJS) $(EXEC)

