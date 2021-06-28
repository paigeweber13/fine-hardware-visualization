/*
 * NOTE: THIS FILE HAS BEEN DEPRECATED
 * 
 * It was leftover from when fhv was also intended to run the benchmark to get
 * baseline performance. Since then that functionality has moved to the
 * `benchmark.sh` script, which is much more stable and consistent, as it
 * relies on the very well-written likwid-bench tool
 * 
 * This code is included for reference only, as it can give maintainers an idea
 * of how one might try to go about saturating FLOP/S
 * 
 * Note that the bandwidth benchmark was never very good and often gave results
 * that were off by as much as a factor of two.
 * 
 */

#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <immintrin.h>
#include <likwid.h>
#include <omp.h>

#include "fhv_perfmon.hpp"

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

void benchmark_flops(precision p, uint64_t num_iter);
void benchmark_memory_bw(std::string memory_type, uint64_t num_iterations,
                         uint64_t mem_size_kb);
void benchmark_all();
void print_csv_header();
