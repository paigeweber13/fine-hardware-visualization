CXX=g++
CXXFLAGS=-g -Wall -std=c++1y -I$(INC_DIR) -march=native -mtune=native \
  -fopenmp -O3 -DLIKWID_PERFMON
LDFLAGS=-L$(LIB_DIR) $(LIBS) -march=native -mtune=native -fopenmp
CXXASSEMBLYFLAGS=-S -g -fverbose-asm

# make sure likwid is installed to this prefix
# manual install to this directory is preferred because then we can run without
# sudo permission
PREFIX=/usr/local
INC_DIR=$(PREFIX)/include
LIB_DIR=$(PREFIX)/lib
LIBS=-llikwid -lboost_program_options

MAIN_DIR=src
SRC_DIR=lib
OBJ_DIR=obj
ASM_DIR=asm
EXEC_DIR=bin
TEST_EXEC_DIR=$(EXEC_DIR)/tests
TEST_DIR=tests

SOURCES=$(wildcard lib/*.cpp)
HEADERS=$(wildcard lib/*.h)
MAINS=$(wildcard src/*.cpp)
TEST_MAINS=$(wildcard tests/*.cpp)
TEST_MAINS_C+=$(wildcard tests/*.c)

LIB_OBJS=$(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
OBJS=$(LIB_OBJS)
OBJS+=$(TEST_MAINS:$(TEST_DIR)/%.cpp=$(OBJ_DIR)/%.o)
OBJS+=$(TEST_MAINS_C:$(TEST_DIR)/%.c=$(OBJ_DIR)/%.o)
OBJS+=$(MAINS:$(MAIN_DIR)/%.cpp=$(OBJ_DIR)/%.o)

ASM=$(SOURCES:$(SRC_DIR)/%.cpp=$(ASM_DIR)/%.s)
ASM+=$(TEST_MAINS:$(TEST_DIR)/%.cpp=$(ASM_DIR)/%.s)
ASM+=$(TEST_MAINS_C:$(TEST_DIR)/%.c=$(ASM_DIR)/%.s)
ASM+=$(MAINS:$(MAIN_DIR)/%.cpp=$(ASM_DIR)/%.s)

EXEC_NAME=fhv
EXEC=$(EXEC_DIR)/$(EXEC_NAME)
BENCH_EXEC_NAME=bench
BENCH_EXEC=$(EXEC_DIR)/$(BENCH_EXEC_NAME)

### meta-rules for easier calling
build: $(EXEC) $(BENCH_EXEC)

tests: run-tests/thread_migration run-tests/likwid_minimal run-tests/benchmark-likwid-vs-manual

assembly: $(ASM) 

fhv: $(EXEC)
	$(EXEC)

bench: $(BENCH_EXEC)
	$(BENCH_EXEC) 0; \
	$(BENCH_EXEC) 1; \
	$(BENCH_EXEC) 2; \
	$(BENCH_EXEC) 3; \
	$(BENCH_EXEC) 4; 

cph-bench: $(BENCH_EXEC)
	qsub -q copperhead -d $(shell pwd) -l nodes=1:ppn=16 -l walltime=01:00:00 $(BENCH_EXEC_NAME).sh

cph: $(EXEC)
	qsub -q copperhead -d $(shell pwd) -l nodes=1:ppn=16 -l walltime=01:00:00 $(EXEC_NAME).sh
	# qsub -q copperhead -d $(shell pwd) -l nodes=$(MAMBA_NODE):ppn=16 -l walltime=01:00:00 $(EXEC).sh
	# qsub -q copperhead -d $(shell pwd) -l nodes=$(MAMBA_NODE):ppn=16:gpus=1 -l walltime=01:00:00 $(EXEC).sh

### utility rules
debug:
	@echo "sources:       $(SOURCES)";
	@echo "mainfiles:     $(MAIN_DIR)";
	@echo "lib objects:   $(LIB_OBJS)";
	@echo "objects:       $(OBJS)";
	@echo "exec:          $(EXEC)";
	@echo "asm:           $(ASM)"; 
debug: LDFLAGS += -Q --help=target
# debug: clean build

clean:
	rm -f $(OBJS) $(EXEC) $(BENCH_EXEC)

### rules to create directories
$(EXEC_DIR):
	mkdir $(EXEC_DIR)

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

$(TEST_EXEC_DIR):
	mkdir $(TEST_EXEC_DIR)

$(ASM_DIR):
	mkdir $(ASM_DIR)

### rules to link executables
define ld-command
$(CXX) $(LIB_OBJS) $< $(LDFLAGS) -o $@
endef

$(BENCH_EXEC): $(OBJ_DIR)/benchmark.o $(LIB_OBJS) | $(EXEC_DIR)
	$(ld-command)

$(EXEC): $(OBJ_DIR)/fhv.o $(LIB_OBJS) | $(EXEC_DIR)
	$(ld-command)

### rules to compile sources
$(OBJS): | $(OBJ_DIR)

define compile-command
$(CXX) $(CXXFLAGS) -c $< -o $@
endef

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(compile-command)

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(compile-command)

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	$(compile-command)

$(OBJ_DIR)/%.o: $(MAIN_DIR)/%.cpp
	$(compile-command)

### rules to create assembly files
$(ASM): | $(ASM_DIR)

define asm-command
$(CXX) $(CXXFLAGS) $(CXXASSEMBLYFLAGS) $< -o $@
endef

$(ASM_DIR)/%.s: $(SRC_DIR)/%.cpp 
	$(asm-command)

$(ASM_DIR)/%.s: $(TEST_DIR)/%.cpp
	$(asm-command)

$(ASM_DIR)/%.s: $(TEST_DIR)/%.c
	$(asm-command)

$(ASM_DIR)/%.s: $(MAIN_DIR)/%.cpp
	$(asm-command)

### manual commands for each test
#TODO: make this more DRY...
bin/tests/benchmark-likwid-vs-manual: $(OBJ_DIR)/benchmark-likwid-vs-manual.o $(LIB_OBJS) | $(TEST_EXEC_DIR)
	$(ld-command)

run-tests/benchmark-likwid-vs-manual: bin/tests/benchmark-likwid-vs-manual 
	bin/tests/benchmark-likwid-vs-manual

bin/tests/thread_migration: $(OBJ_DIR)/thread_migration.o $(LIB_OBJS)  | $(TEST_EXEC_DIR)
	$(ld-command)

run-tests/thread_migration: bin/tests/thread_migration
	bin/tests/thread_migration 0; \
	# bin/tests/thread_migration 1; \
	bin/tests/thread_migration 2;

bin/tests/likwid_minimal: $(OBJ_DIR)/likwid_minimal.o $(LIB_OBJS) | $(TEST_EXEC_DIR)
	$(ld-command)

run-tests/likwid_minimal: bin/tests/likwid_minimal
	likwid-perfctr -C S0:0 -g L3 -g FLOPS_DP -M 1 -m bin/tests/likwid_minimal
