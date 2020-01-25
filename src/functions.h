#pragma once

#include <cstdint>
#include <cstdlib>
#include <immintrin.h>

__m256 flops(std::uint64_t);
__m256i iops(std::uint64_t);