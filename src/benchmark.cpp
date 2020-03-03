#include <chrono>
#include <immintrin.h>
#include <iostream>
#include <omp.h>

#include "../lib/computation_measurements.h"
#include "../lib/performance_monitor.h"

const std::uint64_t FLOAT_NUM_ITERATIONS_SHORT = 1000000000;
const std::uint64_t FLOAT_NUM_ITERATIONS =       10000000000;

enum precision { SINGLE_P, DOUBLE_P };

void benchmark_flops(precision p)
{
  __m256 d_s;
  __m256d d_d;
  __m256i e;

  if (p == precision::SINGLE_P){
    performance_monitor::init("FLOPS_SP");
    std::cout << "starting single precision flop benchmark" << std::endl;
  } else if (p == precision::DOUBLE_P){
    performance_monitor::init("FLOPS_DP");
    std::cout << "starting double precision flop benchmark" << std::endl;
  }
#pragma omp parallel
  {
    performance_monitor::startRegion("flops");
    if(p == precision::SINGLE_P){
      d_s = flops_sp(FLOAT_NUM_ITERATIONS);
    } else if (p == precision::DOUBLE_P){
      d_d = flops_dp(FLOAT_NUM_ITERATIONS);
    }
    performance_monitor::stopRegion("flops");
  }
  performance_monitor::close();
  // performance_monitor::printDetailedResults();
  performance_monitor::printOnlyAggregate();
  performance_monitor::printComparison();
}

void benchmark_l2_bw(){
  performance_monitor::init("L2");
  std::cout << "starting L2 rw bandwidth benchmark" << std::endl;
  // 10000 iterations for a good average
  // 100 kilobytes to fit well inside L2 cache
  bandwidth_rw(10000, 100);

  performance_monitor::close();
  // performance_monitor::printDetailedResults();
  performance_monitor::printOnlyAggregate();
  performance_monitor::printComparison();
}

void benchmark_l3_bw(){
  performance_monitor::init("L3");
  std::cout << "starting L3 rw bandwidth benchmark" << std::endl;
  // 1000 iterations for a good average
  // 1000 kilobytes to fit well inside L3 cache
  bandwidth_rw(1000, 1000);

  performance_monitor::close();
  // performance_monitor::printDetailedResults();
  performance_monitor::printOnlyAggregate();
  performance_monitor::printComparison();
}

void benchmark_ram_bw(){
  performance_monitor::init("MEM_DP");
  std::cout << "starting RAM rw bandwidth benchmark" << std::endl;
  // 10 iterations for a good average
  // 100000 kilobytes (100MB) so it can't all be cached
  bandwidth_rw(10, 100000);

  performance_monitor::close();
  // performance_monitor::printDetailedResults();
  performance_monitor::printOnlyAggregate();
  performance_monitor::printComparison();
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " 0|1" << std::endl;
    std::cout << "       0 for FLOPS_SP benchmark" << std::endl;
    std::cout << "       1 for L2 r/w bandwidth benchmark" << std::endl;
    return 1;
  }

  int choice = std::stoi(argv[1]);

  switch (choice)
  {
  case 0:
    benchmark_flops(precision::SINGLE_P);
    break;
  case 1:
    benchmark_flops(precision::DOUBLE_P);
    break;
  case 2:
    benchmark_l2_bw();
    break;
  case 3:
    benchmark_l3_bw();
    break;
  case 4:
    benchmark_ram_bw();
    break;
  }
}