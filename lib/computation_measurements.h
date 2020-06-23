#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <immintrin.h>
#include <likwid.h>
#include <omp.h>

#include "performance_monitor.h"

#define KILO_BYTE_SIZE 1024
#define BYTES_PER_DOUBLE 8
#define DOUBLES_PER_VECTOR 4

__m256 flops_sp(std::uint64_t);
__m256d flops_dp(std::uint64_t);
__m256i iops(std::uint64_t);
/*
tag: name of region to be used by likwid
num_groups: the number of groups to measure. likwid_markerNextGroup will be
  called between each
num_iterations_to_measure: halfway through the total number of iterations
  bandwidth_rw will measure a copy operation. This copy operation will be
  performed num_iterations_to_measure times
num_iterations_per_group: the number of iterations to run for each group. This
  will be split up into an inner loop that will execute 
  num_iterations_to_measure times and an outer loop that will execute i times, 
  where num_iterations_to_measure*i ~= num_iterations_per_group.
  num_iterations_per_group should be at least 3x num_iterations_to_measure. If 
  it is not, the outer loop will default to 3 iterations.
*/
double bandwidth_rw(const char *tag, std::uint64_t num_groups,
                    std::uint64_t num_iterations_per_group,
                    std::uint64_t num_iterations_to_measure,
                    std::uint64_t size_kib);
