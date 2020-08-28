include ../../config.mk

FHV_INCLUDE_DIR=-I ../../$(SRC_DIR)

FHV_OBJ=../../$(OBJ_DIR)/performance_monitor.o

CXX=g++
CXXFLAGS=-std=c++14 -O3 -fopenmp -march=native -mtune=native
MANUAL_CXXFLAGS=$(CXXFLAGS) -DMANUAL_MEASUREMENT
LIKWID_CXXFLAGS=$(CXXFLAGS) $(LIKWID_INC_DIR) 
FHV_PERFMON_CXXFLAGS=$(CXXFLAGS) $(LIKWID_INC_DIR) $(FHV_INCLUDE_DIR)

LDFLAGS=-fopenmp
MANUAL_LDFLAGS=$(LDFLAGS) -DMANUAL_MEASUREMENT
LIKWID_LDFLAGS=$(LDFLAGS) $(LIKWID_LIB_DIR) $(LIKWID_LIB_FLAG) -DLIKWID_CLI
FHV_PERFMON_LDFLAGS=$(LDFLAGS) $(FHV_OBJ) $(LIKWID_LIB_DIR) $(LIKWID_LIB_FLAG) -DFHV_PERFMON

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

### polynomial: basic code
# Complie rules:
polynomial: polynomial.cpp $(FHV_OBJ)
	$(CXX) $< $(CXXFLAGS) $(LDFLAGS) -o $@

polynomial_manual: polynomial.cpp $(FHV_OBJ)
	$(CXX) $< $(MANUAL_CXXFLAGS) $(MANUAL_LDFLAGS) -o $@

polynomial_likwid: polynomial.cpp $(FHV_OBJ)
	$(CXX) $< $(LIKWID_CXXFLAGS) $(LIKWID_LDFLAGS) -o $@

polynomial_fhv_perfmon: polynomial.cpp $(FHV_OBJ)
	$(CXX) $< $(FHV_PERFMON_CXXFLAGS) $(FHV_PERFMON_LDFLAGS) -o $@

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
polynomial_block: polynomial_block.cpp $(FHV_OBJ)
	$(CXX) $< $(CXXFLAGS) $(LDFLAGS) -o $@

polynomial_block_manual: polynomial_block.cpp $(FHV_OBJ)
	$(CXX) $< $(MANUAL_CXXFLAGS) $(MANUAL_LDFLAGS) -o $@

polynomial_block_likwid: polynomial_block.cpp $(FHV_OBJ)
	$(CXX) $< $(LIKWID_CXXFLAGS) $(LIKWID_LDFLAGS) -o $@

polynomial_block_fhv_perfmon: polynomial_block.cpp $(FHV_OBJ)
	$(CXX) $< $(FHV_PERFMON_CXXFLAGS) $(FHV_PERFMON_LDFLAGS) -o $@

# run rules:
polynomial_block-run: polynomial_block
	$(OMP_S) ./polynomial_block $(POLY_RUN_PARAMS)

polynomial_block_manual-run: polynomial_block_manual
	$(OMP_S) ./polynomial_block_manual $(POLY_RUN_PARAMS)

polynomial_block_likwid-run: polynomial_block_likwid
	$(OMP_S) $(RUN_CMD_PREFIX) likwid-perfctr -C S0:0-3 -g MEM_DP -g FLOPS_SP -g L3 -g L2 -g PORT_USAGE1 -g PORT_USAGE2 -g PORT_USAGE3 -M 1 -m ./polynomial_block_likwid $(POLY_RUN_PARAMS)

polynomial_block_fhv_perfmon-run: polynomial_block_fhv_perfmon
	$(OMP_S) $(RUN_CMD_PREFIX) FHV_OUTPUT="polynomial_block_output.json" ./polynomial_block_fhv_perfmon $(POLY_RUN_PARAMS)


