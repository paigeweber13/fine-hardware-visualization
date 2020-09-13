#### These variables may be edited to suit your environment

# choice of compiler
CXX=g++

#### likwid prefix

# make sure likwid is installed to this prefix. Manual install is preferred
# because manual installs allow for newer versions of likwid, which allow
# likwid to be used without sudo permission

LIKWID_PREFIX=/usr/local/likwid-master
# LIKWID_PREFIX=/usr/local/likwid-v4.3.4
# LIKWID_PREFIX=/usr/local

# this is where "make install" will install the performance_monitor library and
# the "fhv" executable. Examples will use this directory to load the
# performance monitor
FHV_PERFMON_PREFIX=/usr/local/fhv

#### Directories
BUILD_DIR=./build
BUILT_LIB_DIR=$(BUILD_DIR)/lib
EXEC_DIR=$(BUILD_DIR)/bin
TEST_EXEC_DIR=$(EXEC_DIR)/tests
OBJ_DIR=$(BUILD_DIR)/obj
TEST_OBJ_DIR=$(OBJ_DIR)/tests
ASM_DIR=$(BUILD_DIR)/asm

# name of executable
EXEC_NAME=fhv
PERFMON_LIB_NAME_SHORT=fhv_perfmon
PERFMON_LIB_NAME=lib$(PERFMON_LIB_NAME_SHORT).so

# some flags are absolutely required (like "-std=c++14" and "-fopenmp"). Those
# are added in the root makefile. Here you may add other flags you would like.
# This is pre-populated with recommended flags.
ADDITIONAL_COMPILER_FLAGS=-Wall -march=native -mtune=native -O3
ADDITIONAL_LINKER_FLAGS=
