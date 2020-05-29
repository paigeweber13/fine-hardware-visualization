CXX=g++
CXXFLAGS_DEBUG=-g -Wall -std=c++14 $(INC_DIRS) -march=native -mtune=native \
  -fopenmp -DLIKWID_PERFMON
CXXFLAGS=$(CXXFLAGS_DEBUG) -O3
LDFLAGS=$(LIB_DIRS) $(LIBS) -march=native -mtune=native -fopenmp
CXXASSEMBLYFLAGS=-S -g -fverbose-asm

# make sure likwid is installed to this prefix
# manual install to this directory is preferred because then we can run without
# sudo permission
LIKWID_PREFIX=/usr/local/likwid-master
# LIKWID_PREFIX=/usr/local/likwid-v4.3.4
# LIKWID_PREFIX=/usr/local
LIKWID_INC_DIR=-I$(LIKWID_PREFIX)/include
FHV_INC_DIRS=-I./lib
CAIRO_INC_DIRS=$(shell pkg-config --cflags cairo)
INC_DIRS=$(LIKWID_INC_DIR) $(FHV_INC_DIRS) $(CAIRO_INC_DIRS)

LIKWID_LIB_DIR=-L$(LIKWID_PREFIX)/lib
LIB_DIRS=$(LIKWID_LIB_DIR)

LIKWID_LIB_FLAG=-llikwid
BOOST_PO_LIB_FLAG=-lboost_program_options
CAIRO_LIB_FLAG=$(shell pkg-config --libs cairo)
LIBS=$(LIKWID_LIB_FLAG) $(BOOST_PO_LIB_FLAG) $(CAIRO_LIB_FLAG)

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

### examples
EXAMPLE_DIR=examples
EXAMPLE_EXECS=$(EXAMPLE_DIR)/convolution/bin/convolution
EXAMPLE_EXECS+=$(EXAMPLE_DIR)/convolution/bin/convolution-manual
EXAMPLE_EXECS+=$(EXAMPLE_DIR)/convolution/bin/convolution-likwid-cli
EXAMPLE_EXECS+=$(EXAMPLE_DIR)/convolution/bin/convolution-fhv-perfmon

### perfgroup things
SYSTEM_PERFGROUPS_DIR=$(LIKWID_PREFIX)/share/likwid/
PERFGROUPS_ROOT_DIR_NAME=perfgroups
PERFGROUPS_DIRS=$(shell find $(wildcard $(PERFGROUPS_ROOT_DIR_NAME)/*) -type d)

### exec
EXEC_NAME=fhv
EXEC=$(EXEC_DIR)/$(EXEC_NAME)

### prefix used to ensure likwid libraries and access daemon are detected and 
  # used at runtime
RUN_CMD_PREFIX=LD_LIBRARY_PATH=$(LIKWID_PREFIX)/lib PATH=$(LIKWID_PREFIX)/sbin:$$PATH

### meta-rules for easier calling
build: $(EXEC)

tests: run-tests/thread_migration run-tests/likwid_minimal run-tests/benchmark-likwid-vs-manual

build-examples: $(EXAMPLE_EXECS)

assembly: $(ASM) 

fhv: $(EXEC)
	$(EXEC)

perfgroups: $(PERFGROUPS_DIRS)

cph-bench: $(EXEC)
	qsub -q copperhead -d $(shell pwd) -l nodes=1:ppn=16 -l walltime=01:00:00 bench.sh

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
	rm -f $(OBJS) $(EXEC) $(EXAMPLE_EXECS)

### rules to create directories
$(EXEC_DIR):
	mkdir $(EXEC_DIR)

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

$(TEST_EXEC_DIR):
	mkdir $(TEST_EXEC_DIR)

$(ASM_DIR):
	mkdir $(ASM_DIR)

### rules to copy perfgroups
.PHONY: $(PERFGROUPS_DIRS)

define copy-command
sudo cp $(wildcard $@/*) $(SYSTEM_PERFGROUPS_DIR)$@/
endef

$(PERFGROUPS_DIRS):
	@echo sudo permission needed to copy to system directory. This will override
	@echo previously-copied groups but not groups shipped with likwid.
	@echo
	@echo "$(copy-command)"
	@$(copy-command)

### rules to link executables
define ld-command
$(CXX) $(LIB_OBJS) $< $(LDFLAGS) -o $@
endef

$(EXEC): $(OBJ_DIR)/fhv.o $(LIB_OBJS) | $(EXEC_DIR)
	$(ld-command)

### rules to compile sources
$(OBJS): $(HEADERS) | $(OBJ_DIR)

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
	bin/tests/likwid_minimal

run-tests/likwid_minimal-cli: bin/tests/likwid_minimal
	# if this rule is to be used, the setenv stuff in likwid_minimal.c should be
	# commented out 
	$(LIKWID_PREFIX)/bin/likwid-perfctr -C S0:0-3 -g MEM -g L2 -g L3 -g FLOPS_SP -g FLOPS_DP -g PORT_USAGE1 -g PORT_USAGE2 -g PORT_USAGE3 -M 1 -m bin/tests/likwid_minimal

run-tests/port-counter-cli: bin/tests/likwid_minimal
	# if this rule is to be used, the setenv stuff in likwid_minimal.c should be
	# commented out 
	$(LIKWID_PREFIX)/bin/likwid-perfctr -C S0:0-3 -g PORT_USAGE1 -g PORT_USAGE2 -g PORT_USAGE3 -g PORT_USAGE_TEST -M 1 -m bin/tests/likwid_minimal

bin/tests/fhv_minimal: $(OBJ_DIR)/fhv_minimal.o $(LIB_OBJS) | $(TEST_EXEC_DIR)
	$(ld-command)

run-tests/fhv_minimal: bin/tests/fhv_minimal
	bin/tests/fhv_minimal

### variables for Likwid/fhv perfmon examples. These are designed to have fewer
  # dependencies than those required to compile fhv, to demonstrate what 
  # dependencies a programmer using this tool would be concerned with

EXAMPLE_CXXFLAGS=-fopenmp -march=native -mtune=native -O3

example-debug:
	@echo "example execs:"
	@echo $(EXAMPLE_EXECS)

define compile-command-example
$(CXX) $(EXAMPLE_CXXFLAGS) $< -o $@
endef

define compile-command-example-manual
$(CXX) -DMANUAL_MEASUREMENT $(EXAMPLE_CXXFLAGS) $< -o $@
endef

define compile-command-example-likwid-cli
$(CXX) -DLIKWID_CLI $(EXAMPLE_CXXFLAGS) $< $(LIKWID_INC_DIR) $(LIKWID_LIB_DIR) $(LIKWID_LIB_FLAG) -o $@
endef

define compile-command-example-fhv-perfmon
$(CXX) -DFHV_PERFMON $(EXAMPLE_CXXFLAGS) $< $(OBJ_DIR)/performance_monitor.o $(FHV_INC_DIRS) $(LIKWID_INC_DIR) $(LIKWID_LIB_DIR) $(LIKWID_LIB_FLAG) -o $@
endef

# rule to create bin directory:
$(EXAMPLE_DIR)/convolution/bin:
	mkdir $(EXAMPLE_DIR)/convolution/bin

$(EXAMPLE_DIR)/convolution/data:
	mkdir $(EXAMPLE_DIR)/convolution/data

$(EXAMPLE_DIR)/convolution/bin/convolution: $(EXAMPLE_DIR)/convolution/convolution.cpp | $(EXAMPLE_DIR)/convolution/bin
	$(compile-command-example)

$(EXAMPLE_DIR)/convolution/bin/convolution-manual: $(EXAMPLE_DIR)/convolution/convolution.cpp | $(EXAMPLE_DIR)/convolution/bin
	$(compile-command-example-manual)

$(EXAMPLE_DIR)/convolution/bin/convolution-likwid-cli: $(EXAMPLE_DIR)/convolution/convolution.cpp | $(EXAMPLE_DIR)/convolution/bin
	$(compile-command-example-likwid-cli)

$(EXAMPLE_DIR)/convolution/bin/convolution-fhv-perfmon: $(EXAMPLE_DIR)/convolution/convolution.cpp $(OBJ_DIR)/performance_monitor.o | $(EXAMPLE_DIR)/convolution/bin
	$(compile-command-example-fhv-perfmon)

run-example-convolution: $(EXAMPLE_DIR)/convolution/bin/convolution
	$(EXAMPLE_DIR)/convolution/bin/convolution 4000 4000 15 10

run-example-convolution-manual: $(EXAMPLE_DIR)/convolution/bin/convolution-manual
	$(EXAMPLE_DIR)/convolution/bin/convolution-manual 4000 4000 15 10

run-example-convolution-likwid-cli: $(EXAMPLE_DIR)/convolution/bin/convolution-likwid-cli
	$(RUN_CMD_PREFIX) $(LIKWID_PREFIX)/bin/likwid-perfctr -C S0:0-3 -g L3 -g FLOPS_SP -M 1 -m $(EXAMPLE_DIR)/convolution/bin/convolution-likwid-cli 4000 4000 15 10

run-example-convolution-fhv-perfmon: $(EXAMPLE_DIR)/convolution/bin/convolution-fhv-perfmon | $(EXAMPLE_DIR)/convolution/data
	FHV_OUTPUT=$(EXAMPLE_DIR)/convolution/data/convolution.json $(RUN_CMD_PREFIX) $(EXAMPLE_DIR)/convolution/bin/convolution-fhv-perfmon 4000 4000 15 10
