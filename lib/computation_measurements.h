#pragma once

#include <cstdint>
#include <cstdlib>
#pragma once

#include <immintrin.h>
#include <omp.h>

#include "performance_monitor.h"

#define KILO_BYTE_SIZE 1024
#define BYTES_PER_DOUBLE 8
#define DOUBLES_PER_VECTOR 4

__m256 flops(std::uint64_t);
__m256i iops(std::uint64_t);
void bandwidth_rw(std::uint64_t num_iterations, std::uint64_t size_kib);
