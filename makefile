include ./config.mk

### examples
EXAMPLE_DIR=examples
EXAMPLE_EXECS=$(EXAMPLE_DIR)/convolution/bin/convolution
EXAMPLE_EXECS+=$(EXAMPLE_DIR)/convolution/bin/convolution-manual
EXAMPLE_EXECS+=$(EXAMPLE_DIR)/convolution/bin/convolution-likwid-cli
EXAMPLE_EXECS+=$(EXAMPLE_DIR)/convolution/bin/convolution-fhv-perfmon

### meta-rules for easier calling
build: $(EXEC)

tests: run-tests/thread_migration bin/tests/likwid_minimal-run run-tests/benchmark-likwid-vs-manual bin/tests/fhv_minimal-run

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
	LD_LIBRARY_PATH=$(LIKWID_PREFIX)/lib bin/tests/benchmark-likwid-vs-manual

bin/tests/thread_migration: $(OBJ_DIR)/thread_migration.o $(LIB_OBJS)  | $(TEST_EXEC_DIR)
	$(ld-command)

run-tests/thread_migration: bin/tests/thread_migration
	LD_LIBRARY_PATH=$(LIKWID_PREFIX)/lib bin/tests/thread_migration 0; \
	# bin/tests/thread_migration 1; \
	bin/tests/thread_migration 2;

bin/tests/likwid_minimal: $(TEST_DIR)/likwid_minimal.c | $(TEST_EXEC_DIR)
	gcc tests/likwid_minimal.c -I$(LIKWID_PREFIX)/include -L$(LIKWID_PREFIX)/lib -march=native -mtune=native -fopenmp -llikwid -o $@

bin/tests/likwid_minimal-run: bin/tests/likwid_minimal
	LD_LIBRARY_PATH=$(LIKWID_PREFIX)/lib bin/tests/likwid_minimal

bin/tests/likwid_minimal-run-with-cli: bin/tests/likwid_minimal
	# if this rule is to be used, the setenv stuff in likwid_minimal.c should be
	# commented out 
	LD_LIBRARY_PATH=$(LIKWID_PREFIX)/lib $(LIKWID_PREFIX)/bin/likwid-perfctr -C S0:0-3 -g MEM -g L2 -g L3 -g FLOPS_SP -g FLOPS_DP -g PORT_USAGE1 -g PORT_USAGE2 -g PORT_USAGE3 -M 1 -m bin/tests/likwid_minimal

run-tests/port-counter-cli: bin/tests/likwid_minimal
	# if this rule is to be used, the setenv stuff in likwid_minimal.c should be
	# commented out 
	LD_LIBRARY_PATH=$(LIKWID_PREFIX)/lib $(LIKWID_PREFIX)/bin/likwid-perfctr -C S0:0-3 -g PORT_USAGE1 -g PORT_USAGE2 -g PORT_USAGE3 -g PORT_USAGE_TEST -M 1 -m bin/tests/likwid_minimal

bin/tests/fhv_minimal: $(OBJ_DIR)/fhv_minimal.o $(LIB_OBJS) | $(TEST_EXEC_DIR)
	$(ld-command)

bin/tests/fhv_minimal-run: bin/tests/fhv_minimal
	LD_LIBRARY_PATH=$(LIKWID_PREFIX)/lib bin/tests/fhv_minimal

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

# compile rules
$(EXAMPLE_DIR)/convolution/bin/convolution: $(EXAMPLE_DIR)/convolution/convolution.cpp | $(EXAMPLE_DIR)/convolution/bin
	$(compile-command-example)

$(EXAMPLE_DIR)/convolution/bin/convolution-manual: $(EXAMPLE_DIR)/convolution/convolution.cpp | $(EXAMPLE_DIR)/convolution/bin
	$(compile-command-example-manual)

$(EXAMPLE_DIR)/convolution/bin/convolution-likwid-cli: $(EXAMPLE_DIR)/convolution/convolution.cpp | $(EXAMPLE_DIR)/convolution/bin
	$(compile-command-example-likwid-cli)

$(EXAMPLE_DIR)/convolution/bin/convolution-fhv-perfmon: $(EXAMPLE_DIR)/convolution/convolution.cpp $(OBJ_DIR)/performance_monitor.o | $(EXAMPLE_DIR)/convolution/bin
	$(compile-command-example-fhv-perfmon)

# run rules
# CONVOLUTION_RUN_PARAMS=4000 4000 15 10
CONVOLUTION_RUN_PARAMS=1000 1000 15 10

$(EXAMPLE_DIR)/convolution/bin/convolution-run: $(EXAMPLE_DIR)/convolution/bin/convolution
	$(EXAMPLE_DIR)/convolution/bin/convolution $(CONVOLUTION_RUN_PARAMS)

$(EXAMPLE_DIR)/convolution/bin/convolution-manual-run: $(EXAMPLE_DIR)/convolution/bin/convolution-manual
	$(EXAMPLE_DIR)/convolution/bin/convolution-manual $(CONVOLUTION_RUN_PARAMS)

$(EXAMPLE_DIR)/convolution/bin/convolution-likwid-cli-run: $(EXAMPLE_DIR)/convolution/bin/convolution-likwid-cli
	$(RUN_CMD_PREFIX) likwid-perfctr -C S0:0-3 -g L3 -g FLOPS_SP -M 1 -m $(EXAMPLE_DIR)/convolution/bin/convolution-likwid-cli $(CONVOLUTION_RUN_PARAMS)

$(EXAMPLE_DIR)/convolution/bin/convolution-fhv-perfmon-run: $(EXAMPLE_DIR)/convolution/bin/convolution-fhv-perfmon | $(EXAMPLE_DIR)/convolution/data
	FHV_OUTPUT=$(EXAMPLE_DIR)/convolution/data/convolution.json $(RUN_CMD_PREFIX) $(EXAMPLE_DIR)/convolution/bin/convolution-fhv-perfmon $(CONVOLUTION_RUN_PARAMS)
