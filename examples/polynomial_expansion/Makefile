FHV_PERFMON_PREFIX=/usr/local

FHV_INC_FLAG=-I$(FHV_PERFMON_PREFIX)/include
FHV_LIB_DIR=$(FHV_PERFMON_PREFIX)/lib
FHV_LIB_FLAGS=-L$(FHV_LIB_DIR) -lfhv_perfmon

LIKWID_INC_FLAG=-I$(LIKWID_PREFIX)/include
LIKWID_LIB_FLAGS=-L$(LIKWID_PREFIX)/lib -llikwid

RUN_CMD_PREFIX=LD_LIBRARY_PATH=$(LIKWID_PREFIX)/lib:$(FHV_LIB_DIR) \
PATH=$(LIKWID_PREFIX)/sbin:$(LIKWID_PREFIX)/bin:$$PATH

CXX=g++
CXXFLAGS=-std=c++14 -O3 -fopenmp -march=native -mtune=native
MANUAL_CXXFLAGS=$(CXXFLAGS) -DMANUAL_MEASUREMENT
LIKWID_CXXFLAGS=$(CXXFLAGS) $(LIKWID_INC_FLAG) 
FHV_PERFMON_CXXFLAGS=$(CXXFLAGS) $(LIKWID_INC_FLAG) $(FHV_INC_FLAG)

LDFLAGS=-fopenmp
MANUAL_LDFLAGS=$(LDFLAGS) -DMANUAL_MEASUREMENT
LIKWID_LDFLAGS=$(LDFLAGS) $(LIKWID_LIB_FLAGS) -DLIKWID_CLI
FHV_PERFMON_LDFLAGS=$(LDFLAGS) $(LIKWID_LIB_FLAGS) $(FHV_LIB_FLAGS) \
-DFHV_PERFMON

# prefixed used for rules that run something
RUN_CMD_PREFIX=LD_LIBRARY_PATH=$(LIKWID_PREFIX)/lib:$(FHV_LIB_DIR) \
PATH="$(LIKWID_PREFIX)/sbin:$(LIKWID_PREFIX)/bin:$$PATH"

# This seems to be the most optimal through a few quick experiments
OMP_S=OMP_SCHEDULE="dynamic,8"

EXECS=polynomial polynomial_manual polynomial_likwid polynomial_fhv_perfmon \
polynomial_block polynomial_block_manual polynomial_block_likwid \
polynomial_block_fhv_perfmon

all: $(EXECS)

run-all: polynomial-run polynomial_block-run polynomial_manual-run \
polynomial_block_manual-run polynomial_likwid-run polynomial_block_likwid-run \
polynomial_fhv_perfmon-run polynomial_block_fhv_perfmon-run

clean:
	-rm -f $(EXECS)

POLY_RUN_PARAMS=1024 1000 100

#### GENERAL RULES
define compile-comamand
	$(CXX) $< $(CXXFLAGS) $(LDFLAGS) -o $@
endef

define compile-command-manual
	$(CXX) $< $(MANUAL_CXXFLAGS) $(MANUAL_LDFLAGS) -o $@
endef

define compile-command-likwid
	$(CXX) $< $(LIKWID_CXXFLAGS) $(LIKWID_LDFLAGS) -o $@
endef

define compile-command-fhv
	$(CXX) $< $(FHV_PERFMON_CXXFLAGS) $(FHV_PERFMON_LDFLAGS) -o $@
endef

### polynomial: basic code
# Complie rules:
polynomial: polynomial.cpp 
	$(compile-command)

polynomial_manual: polynomial.cpp 
	$(compile-command-manual)

polynomial_likwid: polynomial.cpp 
	$(compile-command-likwid)

polynomial_fhv_perfmon: polynomial.cpp 
	$(compile-command-fhv)

# run rules:
polynomial-run: polynomial
	$(OMP_S) ./polynomial $(POLY_RUN_PARAMS)

polynomial_manual-run: polynomial_manual
	$(OMP_S) ./polynomial_manual $(POLY_RUN_PARAMS)

polynomial_likwid-run: polynomial_likwid
	$(OMP_S) $(RUN_CMD_PREFIX) likwid-perfctr -C S0:0-3 -g MEM_DP -g FLOPS_SP -g L3 -g L2 -g PORT_USAGE1 -g PORT_USAGE2 -g PORT_USAGE3 -M 1 -m ./polynomial_likwid $(POLY_RUN_PARAMS)

polynomial_fhv_perfmon-run: polynomial_fhv_perfmon
	$(OMP_S) $(RUN_CMD_PREFIX) FHV_OUTPUT="polynomial_basic_output.json" ./polynomial_fhv_perfmon $(POLY_RUN_PARAMS)

### polynomial_block: optimized code
# Complie rules:
polynomial_block: polynomial_block.cpp 
	$(compile-command)

polynomial_block_manual: polynomial_block.cpp 
	$(compile-command-manual)

polynomial_block_likwid: polynomial_block.cpp 
	$(compile-command-likwid)

polynomial_block_fhv_perfmon: polynomial_block.cpp 
	$(compile-command-fhv)

# run rules:
polynomial_block-run: polynomial_block
	$(OMP_S) ./polynomial_block $(POLY_RUN_PARAMS)

polynomial_block_manual-run: polynomial_block_manual
	$(OMP_S) ./polynomial_block_manual $(POLY_RUN_PARAMS)

polynomial_block_likwid-run: polynomial_block_likwid
	$(OMP_S) $(RUN_CMD_PREFIX) likwid-perfctr -C S0:0-3 -g MEM_DP -g FLOPS_SP -g L3 -g L2 -g PORT_USAGE1 -g PORT_USAGE2 -g PORT_USAGE3 -M 1 -m ./polynomial_block_likwid $(POLY_RUN_PARAMS)

polynomial_block_fhv_perfmon-run: polynomial_block_fhv_perfmon
	$(OMP_S) $(RUN_CMD_PREFIX) FHV_OUTPUT="polynomial_block_output.json" ./polynomial_block_fhv_perfmon $(POLY_RUN_PARAMS)


