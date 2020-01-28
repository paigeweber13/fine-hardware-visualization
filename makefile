CXX=g++
CXXFLAGS=-g -Wall -std=c++1y -march=native -mtune=native -fopenmp -O3 
# for my home computer, "-march=znver2" is ideal, but compiler support is iffy
LDFLAGS=-march=native -mtune=native -fopenmp
CXXASSEMBLYFLAGS=-S -g -fverbose-asm

MAIN_DIR=src
SRC_DIR=lib
OBJ_DIR=obj
ASM_DIR=asm
EXEC_DIR=bin

SOURCES=$(wildcard lib/*.cpp)
HEADERS=$(wildcard lib/*.h)
OBJS=$(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
ASM=$(SOURCES:$(SRC_DIR)/%.cpp=$(ASM_DIR)/%.s)
EXEC=$(EXEC_DIR)/fhv
BENCH_EXEC=$(EXEC_DIR)/bench

build: $(EXEC) $(BENCH_EXEC)

debug:
	@echo "sources: $(SOURCES)";
	@echo "objects: $(OBJS)";
	@echo "exec:    $(EXEC)";
debug: LDFLAGS += -Q --help=target
debug: clean build

$(BENCH_EXEC): $(OBJS) src/benchmark.cpp
	@mkdir -p $(EXEC_DIR);
	$(CXX) $(LDFLAGS) $(OBJS) src/benchmark.cpp -o $@

$(EXEC): $(OBJS) src/fhv.cpp
	@mkdir -p $(EXEC_DIR);
	$(CXX) $(LDFLAGS) $(OBJS) src/fhv.cpp -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR);
	$(CXX) $(CXXFLAGS) -c $< -o $@

assembly: $(ASM)

$(ASM_DIR)/%.s: $(SRC_DIR)/%.cpp
	@mkdir -p $(ASM_DIR);
	$(CXX) $(CXXFLAGS) $(CXXASSEMBLYFLAGS) $< -o $@

clean:
	rm -f $(OBJS) $(EXEC) $(BENCH_EXEC)

test: build
	./$(EXEC)

mamba: $(EXEC)
	qsub -q mamba -d $(shell pwd) -l nodes=1:ppn=16 -l walltime=01:00:00 $(EXEC).sh
	# qsub -q mamba -d $(shell pwd) -l nodes=$(MAMBA_NODE):ppn=16 -l walltime=01:00:00 $(EXEC).sh
	# qsub -q mamba -d $(shell pwd) -l nodes=$(MAMBA_NODE):ppn=16:gpus=1 -l walltime=01:00:00 $(EXEC).sh

