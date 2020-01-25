CXX=g++
CXXFLAGS=-g -Wall -std=c++1y -march=native -mtune=native -fopenmp -O3
LDFLAGS=-fopenmp
CXXASSEMBLYFLAGS=-S -g -fverbose-asm

SRC_DIR=src
OBJ_DIR=obj
EXEC_DIR=bin

SOURCES=$(wildcard src/*.cpp)
HEADERS=$(wildcard src/*.h)
# OBJS=$(SOURCES:.cpp=.o)
OBJS=$(SOURCES:src/%.cpp=obj/%.o)
EXEC=$(EXEC_DIR)/fhv

build: $(EXEC)

debug:
	@echo "sources: $(SOURCES)";
	@echo "objects: $(OBJS)";
	@echo "exec:    $(EXEC)";

$(EXEC): $(OBJS)
	@mkdir -p $(EXEC_DIR);
	$(CXX) $(LDFLAGS) $(OBJS) -o $@

# $(OBJS): $(SOURCES)
# $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR);
	$(CXX) $(CXXFLAGS) -c $< -o $@

assembly: $(SOURCES)
	$(CXX) $(CXXFLAGS) $(CXXASSEMBLYFLAGS) $(SOURCES)

clean:
	rm -f $(OBJS) $(EXEC)

test: build
	./$(EXEC)


mamba: $(EXEC)
	qsub -q mamba -d $(shell pwd) -l nodes=1:ppn=16 -l walltime=01:00:00 $(EXEC).sh
	# qsub -q mamba -d $(shell pwd) -l nodes=$(MAMBA_NODE):ppn=16 -l walltime=01:00:00 $(EXEC).sh
	# qsub -q mamba -d $(shell pwd) -l nodes=$(MAMBA_NODE):ppn=16:gpus=1 -l walltime=01:00:00 $(EXEC).sh

