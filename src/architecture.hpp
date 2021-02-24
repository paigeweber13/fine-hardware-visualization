#pragma once

// ----- Intel i5-6300U ----- //
const unsigned NUM_SOCKETS = 1;
const unsigned CORES_PER_SOCKET = 2;
const unsigned THREADS_PER_CORE = 2;
const double CLOCK_RATE_GHZ = 2.9;

const unsigned SOCKET_THREADS [1][4] = {{0, 2, 1, 3}};

const unsigned NUM_PORTS_IN_CORE = 8;

const unsigned L1_CACHE_SIZE_KB = 32;
const unsigned L2_CACHE_SIZE_KB = 256;
const unsigned L3_CACHE_SIZE_MB = 3;

const unsigned L1_CACHE_GROUPS [2][2] = {{0,2}, {1, 3}};
const unsigned L2_CACHE_GROUPS [2][2] = {{0,2}, {1, 3}};
const unsigned L3_CACHE_GROUPS [1][4] = {{0, 2, 1, 3}};

// https://en.wikichip.org/wiki/intel/microarchitectures/skylake_(client)#Memory_Hierarchy
// this site shows how the line size and load/cycle bandwidth is 64 bytes.
// However, this is shared between the two threads on one core
const unsigned CACHE_LINE_SIZE_BYTES = 32;

const unsigned NUM_NUMA_DOMAINS = 1;

const unsigned NUMA_DOMAINS [1][4] = {{0, 1, 2, 3}};

// ---- Benchmark ---- //
// -- computation --

// Theoretical maxima
/* 
 * SP Flops - 76 GFlop/s per core, 304 GFlop/s total
 *
 * DP Flops - 38 GFlop/s per core, 152 GFlop/s total
 *
 * Is the discrepancy from how intel tends to scale frequency based on 
 * instruction 
 */
const float EXPERIENTIAL_SP_RATE_MFLOPS = 183598.031250; // ~184 GFlop/s
const float EXPERIENTIAL_DP_RATE_MFLOPS = 91583.672;     // ~92  GFlop/s

// -- memory -- 
// all measurements are in MiB/s
const float EXPERIENTIAL_R_BW_L1  = 0.;              //
const float EXPERIENTIAL_R_BW_L2  = 172162.1187;     // ~172 GB/s
const float EXPERIENTIAL_R_BW_L3  =  94259.5077;     // ~ 94 GB/s
const float EXPERIENTIAL_R_BW_RAM =  23520.7488;     // ~ 24 GB/s

const float EXPERIENTIAL_W_BW_L1  = 0.;              //
const float EXPERIENTIAL_W_BW_L2  = 109407.8277;     // ~109 GB/s
const float EXPERIENTIAL_W_BW_L3  =  63675.2813;     // ~ 63 GB/s
const float EXPERIENTIAL_W_BW_RAM =  12160.8669;     // ~ 12 GB/s

const float EXPERIENTIAL_RW_BW_L1  = 0.;             //
const float EXPERIENTIAL_RW_BW_L2  = 214060.9152;    // ~214 GB/s
const float EXPERIENTIAL_RW_BW_L3  = 127028.1843;    // ~127 GB/s
const float EXPERIENTIAL_RW_BW_RAM =  24208.177734;  // ~ 24 GB/s
