// does not yet work

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

  // ---- performance_monitor initialization (currently broken)
  // performance_monitor::init(
  //   "MEM|FLOPS_DP|L3|L2|PORT_USAGE1|PORT_USAGE2|PORT_USAGE3",
  //   // "MEM|FLOPS_DP|L3|L2",
  //   "double_flops,copy", "");

  // ---- begin likwid initialization

  performance_monitor::setEnvironmentVariables(
    "MEM|FLOPS_DP|L3|L2|PORT_USAGE1|PORT_USAGE2|PORT_USAGE3");
  likwid_markerInit();

#pragma omp parallel
  {
    likwid_markerThreadInit();
    likwid_markerRegisterRegion("double_flops");
    likwid_markerRegisterRegion("copy");
  }

  perfmon_startCounters();
  // ---- end likwid initialization

  double a, b, c;
  a = 1.8;
  b = 3.2;
  c = 0.0;

  size_t n = 256;
  double arr[n];
  double copy_arr[n];

#pragma omp parallel
  {
    for (int j = 0; j < 8; j++)
    {
      printf("thread %d, iteration %d\n", omp_get_thread_num(), j);
      // likwid_markerStartRegion("double_flops");
      performance_monitor::startRegion("double_flops");
#pragma omp barrier
      for (int i = 0; i < 10000000; i++)
      {
        c = a * b + c;
      }
#pragma omp barrier
      performance_monitor::stopRegion("double_flops");
      performance_monitor::startRegion("copy");
      // likwid_markerStopRegion("double_flops");
      // likwid_markerStartRegion("copy");
      for (int i = 0; i < 1000; i++){
        copy(arr, copy_arr, n);
      }
#pragma omp barrier
      performance_monitor::stopRegion("copy");
      performance_monitor::nextGroup();
      // likwid_markerStopRegion("copy");
      // likwid_markerNextGroup();
    }
  }

  printf("final c: %f\n", c);
  printf("final random part of copy_arr: %f\n", 
         copy_arr[( (size_t) c ) % n]);

  performance_monitor::close();
  // currently broken
  performance_monitor::printOnlyAggregate();
  performance_monitor::printComparison();
}
