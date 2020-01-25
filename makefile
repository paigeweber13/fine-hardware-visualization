CXX=g++
CXXFLAGS=-g -Wall -std=c++1y -march=native -mtune=native -fopenmp -O3
LDFLAGS=
CXXASSEMBLYFLAGS=-S -g -fverbose-asm

SOURCES=$(wildcard src/*.cpp)
OBJS=$(SOURCES:.cpp=.o)
EXEC=fhv

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

$(OBJS): $(SOURCES)

assembly: $(FILES)
	$(CXX) $(CXXFLAGS) $(CXXASSEMBLYFLAGS) $(CPPFLAGS) $(SOURCES)

clean:
	rm -f $(EXEC) $(OBJS)

test: $(EXEC)
	./$(EXEC)

# SOURCES=$(wildcard src/*.cpp)
# OBJS_DIR=obj
# OBJS=$(SOURCES:%.cpp=$(OBJS_DIR)/%.o)
# EXEC_DIR=bin
# EXEC=$(EXEC_DIR)/fhv
# EXEC_FILE=$(EXEC:%=$(EXEC_DIR)/%)
# ASM_DIR=asm
# ASM=$(SOURCES:%.cpp=$(ASM_DIR)/%.s)

# OBJS=$(patsubst %.cpp,$(OBJS_DIR)/%.o,$(SOURCES)) 

# all: $(EXEC)

# $(EXEC): $(OBJS)
# 	$(CXX) $(LDFLAGS) $(OBJS) -o $@ $^

# $(OBJS): $(OBJS_DIR)/%.o: %.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@ 

# assembly: $(ASM)

# $(ASM): $(FILES)
# 	$(CXX) $(CXXFLAGS) $(CXXASSEMBLYFLAGS) $(CPPFLAGS) $(SOURCES) -0 $@ $<

# clean:
# 	rm -f $(EXEC) $(OBJS)

# test: $(EXEC)
# 	./$(EXEC)

mamba: $(EXEC)
	qsub -q mamba -d $(shell pwd) -l nodes=1:ppn=16 -l walltime=01:00:00 $(EXEC).sh
	# qsub -q mamba -d $(shell pwd) -l nodes=$(MAMBA_NODE):ppn=16 -l walltime=01:00:00 $(EXEC).sh
	# qsub -q mamba -d $(shell pwd) -l nodes=$(MAMBA_NODE):ppn=16:gpus=1 -l walltime=01:00:00 $(EXEC).sh

