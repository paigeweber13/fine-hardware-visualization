#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#include "performance_monitor.hpp"

#define NUM_FLOPS 10000000
#define NUM_COPIES 10000

typedef long long int lli;

void do_copy(double *arr, double *copy_arr, lli n, lli num_copies) {
  for (lli iter = 0; iter < num_copies; iter++) {
    for (lli i = 0; i < n; i++) {
      copy_arr[i] = arr[i];
    }
  }
}

double do_flops(double a, double b, double c, lli num_flops) {
  for (lli i = 0; i < num_flops; i++) {
    c = a * b + c;
  }
  return c;
}

int main()
{
  printf("\nThis is a minimal example of how the fhv performance_monitor works"
         "\n");

  // performance_monitor::init("double_flops,copy");
  performance_monitor::init();

  double a, b, c;
  a = 1.8;
  b = 3.2;
  c = 0.0;

  const lli n = 2048;
  double arr[n];
  double copy_arr[n];

#pragma omp parallel
  {
    for (int j = 0; j < 8; j++)
    {
      printf("thread %d, iteration %d\n", omp_get_thread_num(), j);

      performance_monitor::startRegion("double_flops");
      c = do_flops(a, b, c, NUM_FLOPS);
      performance_monitor::stopRegion("double_flops");

      performance_monitor::startRegion("copy");
      do_copy(arr, copy_arr, n, NUM_COPIES);
      performance_monitor::stopRegion("copy");

      performance_monitor::nextGroup();
    }
  }

  printf("final c: %f\n", c);
  printf("final random part of copy_arr: %f\n", 
         copy_arr[( (lli) c ) % n]);

  performance_monitor::close();

  // performance_monitor::printDetailedResults();
  // performance_monitor::printAggregateResults();
  performance_monitor::printHighlights();
  performance_monitor::resultsToJson();
}
