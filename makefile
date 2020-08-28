include ./config.mk

### meta-rules for easier calling
build: $(EXEC)

tests: run-tests/thread_migration bin/tests/likwid_minimal-run run-tests/benchmark-likwid-vs-manual bin/tests/fhv_minimal-run

build-examples: 
	@cd examples/polynomial_expansion; make;
	@cd examples/convolution; make;

assembly: $(ASM) 

fhv: $(EXEC)
	$(EXEC)

perfgroups: $(PERFGROUPS_DIRS)

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
	rm -f $(OBJS) $(EXEC)

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
