#include <chrono>
#include <immintrin.h>
#include <iostream>
#include <omp.h>

#include "../lib/computation_measurements.h"
#include "../lib/performance_monitor.h"

const double microseconds_to_seconds = 1e-6;

struct bw_results {
  double duration_seconds;
  double mb_transferred;
  double bandwidth;
  // double thing_computed;
};

bw_results bandwidth_rw_bench_compare(
  std::uint64_t num_iterations, std::uint64_t size_kib)
{
  // Is this code getting optimized out??? is that why I'm getting huge
  // bandwidth numbers?

  // I'm no longer getting huge bandwidth numbers but my manual calculations
  // differ from likwid by a factor of 2

  const unsigned num_inner_iterations = 100;
  const double kb_to_mb = 1e-3;
  std::uint64_t num_doubles = size_kib * KILO_BYTE_SIZE/BYTES_PER_DOUBLE;

  unsigned num_threads;
  double duration;
  std::chrono::high_resolution_clock::time_point start_time, end_time;
  std::uint64_t i, j, k;
  __m256d buffer;

  const char * tag = "RAM";
  // align to cache line, which is 512 bits or 64 bytes
  double * array = static_cast<double *>(
    aligned_alloc(64, size_kib * KILO_BYTE_SIZE));
  double * copy_array = static_cast<double *>(
    aligned_alloc(64, size_kib * KILO_BYTE_SIZE));

#pragma omp parallel default(shared) private(buffer, i, j, k)
  {
    num_threads = omp_get_num_threads();

    // thr_num = omp_get_thread_num();
    for (i = 0; i < num_iterations; i++)
    {
      //Get time snapshot just for one iteration
      if (i == num_iterations / 2)
      {
        //	start = system_clock::now();
        //	Maybe start likwid region here
        //    printf("likwid start region %s on thread %d\n", bw->mark_tag, omp_get_thread_num());
        performance_monitor::startRegion(tag);
        start_time = std::chrono::high_resolution_clock::now();
      }
      for (k = 0; k < num_inner_iterations; k++)
        for (j = 0; j < num_doubles; j += DOUBLES_PER_VECTOR)
        {
          // Loading 256-bits into memory address of array
          buffer = _mm256_load_pd(array + j);
          // Storing 256-bits from buffer into address of cpy_arr
          _mm256_store_pd(copy_array + j, buffer);
        }
      //Get time snapshot just for one iteration
      if (i == num_iterations / 2)
      {
        //	end = system_clock::now();
        //	Maybe stop likwid regin here
        //    printf("likwid stop region %s on thread %d\n", bw->mark_tag, omp_get_thread_num());
        performance_monitor::stopRegion(tag);
        end_time = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
      }
      asm(""); //Say no to loop optimization
    }
  }

  bw_results results;
  results.mb_transferred = num_inner_iterations*size_kib*kb_to_mb*num_threads;
  results.duration_seconds = duration * microseconds_to_seconds;
  results.bandwidth = results.mb_transferred/results.duration_seconds;
  // results.thing_computed = copy_array[static_cast<size_t>(copy_array[0])];

  free(array);
  free(copy_array);
  return results;
}


int main(int argc, char* argv[])
{
  std::cout << "---- running benchmark and doing manual timing to compare";
  std::cout << " with likwid results ----" << std::endl;
  //                                         100 000 000 one hundred million
  const std::uint64_t FLOAT_NUM_ITERATIONS = 100000000;

  // we do 5 fma, so 10 total operations on 8 floats each
  const std::uint64_t FLOP_PER_ITERATION = 64; 

  const std::uint64_t INT_NUM_ITERATIONS = 1000000000;
  const std::uint64_t IOP_PER_ITERATION = 80; //32 adds + 32 adds + 16 muls

  std::uint64_t NUM_CORES;

  __m256 d;
  __m256i e;

  performance_monitor perfmon;

  // FLOPS ----------------------------
  performance_monitor::init("FLOPS_SP|MEM_DP");
  auto start_time = std::chrono::high_resolution_clock::now();
  #pragma omp parallel
  {
    NUM_CORES = omp_get_num_threads();
    // std::cout << "I am processor #" << omp_get_thread_num() << std::endl;

    performance_monitor::startRegion("flops");
    // #pragma omp barrier
    d = flops(FLOAT_NUM_ITERATIONS);
    // #pragma omp barrier
    performance_monitor::stopRegion("flops");
  }
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

  likwid_markerNextGroup();
  bw_results bandwidth_results = bandwidth_rw_bench_compare(10, 100000);

  performance_monitor::close();
  performance_monitor::printResults();
  
  double total_float_ops = FLOAT_NUM_ITERATIONS * FLOP_PER_ITERATION * NUM_CORES;
  const double flops_to_tflops = 1e-12;

  std::cout << "manually calculated results:\n";
  std::cout << "time taken for flops: " << duration*1.0e-6 << " seconds." << std::endl;
  std::cout << "total floating point operations: " << total_float_ops << std::endl;
  std::cout << (total_float_ops*flops_to_tflops) / (duration*microseconds_to_seconds) << " TFlop/s" << std::endl;
  std::cout << "time taken for memory transfer: " << bandwidth_results.duration_seconds << " seconds." << std::endl;
  std::cout << "size of data transferred (MB): " << bandwidth_results.mb_transferred << std::endl;
  std::cout << "bandwidth (MB/s): " << bandwidth_results.bandwidth << std::endl;
}