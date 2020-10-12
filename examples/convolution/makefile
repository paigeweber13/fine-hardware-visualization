include ../../config.mk

# Variables
FHV_INC_FLAG=-I$(FHV_PERFMON_PREFIX)/include
FHV_LIB_DIR=$(FHV_PERFMON_PREFIX)/lib
FHV_LIB_FLAGS=-L$(FHV_LIB_DIR) -lfhv_perfmon

LIKWID_INC_FLAG=-I$(LIKWID_PREFIX)/include
LIKWID_LIB_FLAGS=-L$(LIKWID_PREFIX)/lib -llikwid

CONVOLUTION_CXXFLAGS=-fopenmp -march=native -mtune=native -O3

BIN_DIR=bin
DATA_DIR=data

EXECS=$(BIN_DIR)/convolution $(BIN_DIR)/convolution-manual \
$(BIN_DIR)/convolution-likwid-cli $(BIN_DIR)/convolution-fhv-perfmon

RUN_CMD_PREFIX=LD_LIBRARY_PATH=$(LIKWID_PREFIX)/lib:$(FHV_LIB_DIR) \
PATH=/usr/local/likwid-master/sbin:/usr/local/likwid-master/bin:$$PATH

# rules
all: $(EXECS)

run-all: $(BIN_DIR)/convolution-run $(BIN_DIR)/convolution-manual-run \
$(BIN_DIR)/convolution-likwid-cli-run $(BIN_DIR)/convolution-fhv-perfmon-run

clean:
	rm -f $(EXECS)

define compile-command
$(CXX) $(CONVOLUTION_CXXFLAGS) $< -o $@
endef

define compile-command-manual
$(CXX) -DMANUAL_MEASUREMENT $(CONVOLUTION_CXXFLAGS) $< -o $@
endef

define compile-command-likwid-cli
$(CXX) -DLIKWID_CLI $(CONVOLUTION_CXXFLAGS) $< $(LIKWID_INC_FLAG) $(LIKWID_LIB_FLAGS) -o $@
endef

define compile-command-fhv-perfmon
$(CXX) -DFHV_PERFMON $(CONVOLUTION_CXXFLAGS) $< $(LIKWID_INC_FLAG) $(LIKWID_LIB_FLAGS) $(FHV_INC_FLAG) $(FHV_LIB_FLAGS) -o $@
endef

# rule to create bin directory:
$(BIN_DIR):
	mkdir $(BIN_DIR)

$(DATA_DIR):
	mkdir $(DATA_DIR)

# compile rules
$(BIN_DIR)/convolution: convolution.cpp | $(BIN_DIR)
	$(compile-command)

$(BIN_DIR)/convolution-manual: convolution.cpp | $(BIN_DIR)
	$(compile-command-manual)

$(BIN_DIR)/convolution-likwid-cli: convolution.cpp | $(BIN_DIR)
	$(compile-command-likwid-cli)

$(BIN_DIR)/convolution-fhv-perfmon: convolution.cpp $(FHV_OBJ) | $(BIN_DIR)
	$(compile-command-fhv-perfmon)

# run rules
# CONVOLUTION_RUN_PARAMS=4000 4000 15 10
CONVOLUTION_RUN_PARAMS=1000 1000 15 10

$(BIN_DIR)/convolution-run: $(BIN_DIR)/convolution
	$(BIN_DIR)/convolution $(CONVOLUTION_RUN_PARAMS)

$(BIN_DIR)/convolution-manual-run: $(BIN_DIR)/convolution-manual
	$(BIN_DIR)/convolution-manual $(CONVOLUTION_RUN_PARAMS)

$(BIN_DIR)/convolution-likwid-cli-run: $(BIN_DIR)/convolution-likwid-cli
	$(RUN_CMD_PREFIX) likwid-perfctr -C S0:0-3 -g L3 -g FLOPS_SP -M 1 -m $(BIN_DIR)/convolution-likwid-cli $(CONVOLUTION_RUN_PARAMS)

$(BIN_DIR)/convolution-fhv-perfmon-run: $(BIN_DIR)/convolution-fhv-perfmon | $(DATA_DIR)
	FHV_OUTPUT=$(DATA_DIR)/convolution.json $(RUN_CMD_PREFIX) $(BIN_DIR)/convolution-fhv-perfmon $(CONVOLUTION_RUN_PARAMS)
