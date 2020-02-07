CXX=g++
CXXFLAGS=-g -Wall -std=c++1y -I$(INC_DIR) -march=native -mtune=native \
  -fopenmp -O3 -DLIKWID_PERFMON
LDFLAGS=-L$(LIB_DIR) -march=native -mtune=native -fopenmp -llikwid
CXXASSEMBLYFLAGS=-S -g -fverbose-asm

# make sure likwid is installed to this prefix
PREFIX=/usr/local
INC_DIR=$(PREFIX)/include
LIB_DIR=$(PREFIX)/lib

MAIN_DIR=src
SRC_DIR=lib
OBJ_DIR=obj
ASM_DIR=asm
EXEC_DIR=bin

SOURCES=$(wildcard lib/*.cpp)
HEADERS=$(wildcard lib/*.h)
OBJS=$(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
ASM=$(SOURCES:$(SRC_DIR)/%.cpp=$(ASM_DIR)/%.s)
EXEC_NAME=fhv
EXEC=$(EXEC_DIR)/$(EXEC_NAME)
BENCH_EXEC_NAME=bench
BENCH_EXEC=$(EXEC_DIR)/$(BENCH_EXEC_NAME)

build: $(EXEC) $(BENCH_EXEC)

debug:
	@echo "sources: $(SOURCES)";
	@echo "objects: $(OBJS)";
	@echo "exec:    $(EXEC)";
debug: LDFLAGS += -Q --help=target
debug: clean build

$(BENCH_EXEC): $(OBJS) src/benchmark.cpp
	@mkdir -p $(EXEC_DIR);
	$(CXX) $(OBJS) src/benchmark.cpp $(LDFLAGS) -o $@

$(EXEC): $(OBJS) src/fhv.cpp
	@mkdir -p $(EXEC_DIR);
	$(CXX) $(OBJS) src/fhv.cpp $(LDFLAGS) -o $@

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

cph-bench: $(BENCH_EXEC)
	qsub -q copperhead -d $(shell pwd) -l nodes=1:ppn=16 -l walltime=01:00:00 $(BENCH_EXEC_NAME).sh

cph: $(EXEC)
	qsub -q copperhead -d $(shell pwd) -l nodes=1:ppn=16 -l walltime=01:00:00 $(EXEC_NAME).sh
	# qsub -q copperhead -d $(shell pwd) -l nodes=$(MAMBA_NODE):ppn=16 -l walltime=01:00:00 $(EXEC).sh
	# qsub -q copperhead -d $(shell pwd) -l nodes=$(MAMBA_NODE):ppn=16:gpus=1 -l walltime=01:00:00 $(EXEC).sh

