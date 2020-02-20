CXX=g++
CXXFLAGS=-g -Wall -std=c++1y -I$(INC_DIR) -march=native -mtune=native \
  -fopenmp -O3 -DLIKWID_PERFMON
LDFLAGS=-L$(LIB_DIR) -march=native -mtune=native -fopenmp -llikwid
CXXASSEMBLYFLAGS=-S -g -fverbose-asm

# make sure likwid is installed to this prefix
# manual install to this directory is preferred because then we can run without
# sudo permission
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

# build: $(EXEC) $(BENCH_EXEC)
build: $(BENCH_EXEC)

init:
	@mkdir -p $(EXEC_DIR);
	@mkdir -p $(OBJ_DIR);
	@mkdir -p $(EXEC_DIR)/tests;

debug:
	@echo "sources: $(SOURCES)";
	@echo "objects: $(OBJS)";
	@echo "exec:    $(EXEC)";
debug: LDFLAGS += -Q --help=target
debug: clean build

tests: run-tests/thread_migration run-tests/likwid_minimal

$(BENCH_EXEC): $(OBJS) src/benchmark.cpp
	$(CXX) $(OBJS) src/benchmark.cpp $(LDFLAGS) -o $@

$(EXEC): $(OBJS) src/fhv.cpp
	$(CXX) $(OBJS) src/fhv.cpp $(LDFLAGS) -o $@

bin/tests/benchmark-likwid-vs-manual: $(OBJS) tests/benchmark-likwid-vs-manual.cpp
	$(CXX) $(OBJS) tests/benchmark-likwid-vs-manual.cpp $(LDFLAGS) -o $@

run-bin/tests/benchmark-likwid-vs-manual: bin/tests/benchmark-likwid-vs-manual
	bin/tests/benchmark-likwid-vs-manual

bin/tests/thread_migration: $(OBJS) tests/thread_migration.cpp
	$(CXX) $(OBJS) tests/thread_migration.cpp $(LDFLAGS) -o $@

run-tests/thread_migration: bin/tests/thread_migration
	bin/tests/thread_migration 0; \
	# bin/tests/thread_migration 1; \
	bin/tests/thread_migration 2;

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp init
	$(CXX) $(CXXFLAGS) -c $< -o $@

bin/tests/likwid_minimal: $(OBJS) tests/likwid_minimal.c
	gcc tests/likwid_minimal.c -L/usr/local/lib -march=native -mtune=native -fopenmp -llikwid -o bin/tests/likwid_minimal

run-tests/likwid_minimal: bin/tests/likwid_minimal
	likwid-perfctr -C S0:0 -g L3 -g FLOPS_DP -M 1 -m tests/likwid_minimal

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

