CXX=g++
# CXX=gcc

# for use by this example
SRC_DIR=src
SRC_NAMES=main.cpp
SRCS=$(addprefix $(SRC_DIR)/, $(SRC_NAMES))
BIN_DIR=build/bin
OBJ_DIR=build/obj

FHV_PREFIX=/usr/local


### for building this example
CXX_FLAGS=-I$(FHV_PREFIX)/include -I$(LIKWID_SRC_ROOT)/GCC \
	-I$(LIKWID_SRC_ROOT)/includes -std=c++14 -fopenmp -Wall -g 
CXX_COMMAND=$(CXX) $(CXX_FLAGS) -c $< -o $@
LD_FLAGS=-L$(FHV_PREFIX)/lib -lfhv_perfmon -fopenmp

OBJS=$(SRC_NAMES:%.cpp=$(OBJ_DIR)/%.o)
EXEC=$(BIN_DIR)/microbenchmarks

# rules for building this example
all: benchmarks $(EXEC)

$(EXEC): $(OBJS) $(BENCHMARK_OBJS) | $(BIN_DIR)
	$(CXX) $(OBJS) $(BENCHMARK_OBJS) $(LD_FLAGS) -o $@

$(OBJ_DIR)/%.o: ./$(SRC_DIR)/%.cpp ./$(SRC_DIR)/%.hpp | $(OBJ_DIR)
	$(CXX_COMMAND)

$(OBJ_DIR)/%.o: ./$(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX_COMMAND)

MKDIR_COMMAND=mkdir -p $@

$(OBJ_DIR):
	$(MKDIR_COMMAND)

$(BIN_DIR):
	$(MKDIR_COMMAND)


### for compiling likwid's microbenchmarks
LIKWID_SRC_ROOT=./likwid/bench
LIKWID_BENCHMARK_ROOT=GCC
BENCHMARK_ROOT=$(LIKWID_SRC_ROOT)/$(LIKWID_BENCHMARK_ROOT)
BENCHMARKS=peakflops_avx_fma peakflops_sp_avx_fma copy_avx load_avx store_avx
LIKWID_BENCHMARK_OBJS=$(addsuffix .o,$(addprefix $(LIKWID_BENCHMARK_ROOT)/, $(BENCHMARKS)))
BENCHMARK_OBJS=$(addsuffix .o,$(addprefix $(BENCHMARK_ROOT)/, $(BENCHMARKS)))

# this rule will also create GCC/testcases.h
benchmarks: $(BENCHMARK_OBJS)

$(BENCHMARK_OBJS):
	cd $(LIKWID_SRC_ROOT) && make $(LIKWID_BENCHMARK_OBJS)


clean:
	@cd $(LIKWID_SRC_ROOT) && make distclean
	@rm -f $(OBJS) $(EXEC)

