#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#include "performance_monitor.h"

void copy(double * arr, double * copy_arr, size_t n){
  for (size_t i = 0; i < n; i++){
    copy_arr[i] = arr[i];
  }
}

int main()
{
  printf("\n\nThis is a minimal example of how the fhv performance_monitor "
         "api works\n");

  performance_monitor::init(
    // "MEM_DP|FLOPS_SP|L3|L2|PORT_USAGE1|PORT_USAGE2|PORT_USAGE3",
    "MEM_DP|FLOPS_SP|L3|L2",
    "double_flops,copy", "");

  double a, b, c;
  a = 1.8;
  b = 3.2;
  c = 0.0;

  size_t n = 256;
  double arr[n];
  double copy_arr[n];

// perfmon_startCounters();
#pragma omp parallel
  {
    for (int j = 0; j < 8; j++)
    {
      printf("thread %d, iteration %d\n", omp_get_thread_num(), j);
      performance_monitor::startRegion("double_flops");
#pragma omp barrier
      for (int i = 0; i < 10000000; i++)
      {
        c = a * b + c;
      }
#pragma omp barrier
      performance_monitor::stopRegion("double_flops");
#pragma omp barrier
      performance_monitor::startRegion("copy");
#pragma omp barrier
      for (int i = 0; i < 1000; i++){
        copy(arr, copy_arr, n);
      }
#pragma omp barrier
      performance_monitor::stopRegion("copy");
#pragma omp barrier
      performance_monitor::nextGroup();
    }
  }

  printf("final c: %f\n", c);

  performance_monitor::close();
  performance_monitor::printOnlyAggregate();
  performance_monitor::printComparison();
}
