#pragma once

// ----- Intel i5-6300U ----- //
#define NUM_SOCKETS 1
#define CORES_PER_SOCKET 2
#define THREADS_PER_CORE 2

const unsigned SOCKET_THREADS [1][4] = {{0, 2, 1, 3}};

#define L1_CACHE_SIZE_KB 32
#define L2_CACHE_SIZE_KB 256
#define L3_CACHE_SIZE_MB 3

const unsigned L1_CACHE_GROUPS [2][2] = {{0,2}, {1, 3}};
const unsigned L2_CACHE_GROUPS [2][2] = {{0,2}, {1, 3}};
const unsigned L3_CACHE_GROUPS [1][4] = {{0, 2, 1, 3}};

#define NUM_NUMA_DOMAINS 1

const unsigned NUMA_DOMAINS [1][4] = {{0, 1, 2, 3}};

// ---- Benchmark ---- //
// -- computation --
#define EXPERIENTIAL_SP_RATE_MFLOPS 177336.453125

// -- memory -- 
// all measurements are in MiB/s
#define EXPERIENTIAL_RW_BW_L1  0.
#define EXPERIENTIAL_RW_BW_L2  150194.921875
#define EXPERIENTIAL_RW_BW_L3  102951.289062
#define EXPERIENTIAL_RW_BW_RAM 0.
