#### These variables may be edited to suit your environment

# choice of compiler
CXX=g++

# make sure likwid is installed to this prefix
# manual install to this directory is preferred because then we can run without
# sudo permission
LIKWID_PREFIX=/usr/local/likwid-master
# LIKWID_PREFIX=/usr/local/likwid-v4.3.4
# LIKWID_PREFIX=/usr/local

# directories where we will build things
EXEC_DIR=bin
TEST_EXEC_DIR=$(EXEC_DIR)/tests
OBJ_DIR=obj
ASM_DIR=asm

# name of executable
EXEC_NAME=fhv

#### DO NOT EDIT PAST THIS LINE. 

# Fhv depends on these variables being set to specific values. Any edits you do
# may break the build process. However, some of these variables are used by the
# examples, so they are inlcuded here for easy access.

CXXFLAGS_BASE= -Wall -std=c++14 $(INC_DIRS) -march=native -mtune=native \
  -fopenmp -DLIKWID_PERFMON
CXXFLAGS_DEBUG=$(CXXFLAGS_BASE) -g 
CXXFLAGS=$(CXXFLAGS_BASE) -O3
CXXASSEMBLYFLAGS=-S -g -fverbose-asm

LDFLAGS=$(LIB_DIRS) $(LIBS) -march=native -mtune=native -fopenmp

LIKWID_INC_DIR=-I$(LIKWID_PREFIX)/include
FHV_INC_DIRS=-I./lib
PANGOCAIRO_INC_DIRS=$(shell pkg-config --cflags pangocairo)
INC_DIRS=$(LIKWID_INC_DIR) $(FHV_INC_DIRS) $(PANGOCAIRO_INC_DIRS)

LIKWID_LIB_DIR=-L$(LIKWID_PREFIX)/lib
LIB_DIRS=$(LIKWID_LIB_DIR)

LIKWID_LIB_FLAG=-llikwid
BOOST_PO_LIB_FLAG=-lboost_program_options
PANGOCAIRO_LIB_FLAG=$(shell pkg-config --libs pangocairo)
LIBS=$(LIKWID_LIB_FLAG) $(BOOST_PO_LIB_FLAG) $(PANGOCAIRO_LIB_FLAG)

MAIN_DIR=src
SRC_DIR=lib
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

### perfgroup things
SYSTEM_PERFGROUPS_DIR=$(LIKWID_PREFIX)/share/likwid/
PERFGROUPS_ROOT_DIR_NAME=perfgroups
PERFGROUPS_DIRS=$(shell find $(wildcard $(PERFGROUPS_ROOT_DIR_NAME)/*) -type d)

### exec
EXEC=$(EXEC_DIR)/$(EXEC_NAME)

### prefix used to ensure likwid libraries and access daemon are detected and 
  # used at runtime
RUN_CMD_PREFIX=LD_LIBRARY_PATH=$(LIKWID_PREFIX)/lib PATH="$(LIKWID_PREFIX)/sbin:$(LIKWID_PREFIX)/bin:$$PATH"