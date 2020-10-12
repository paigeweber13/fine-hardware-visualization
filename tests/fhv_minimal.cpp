#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#include "fhv_perfmon.hpp"

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
  printf("\nThis is a minimal example of how the fhv fhv_perfmon works"
         "\n");

  // fhv_perfmon::init("double_flops,copy");
  fhv_perfmon::init();

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

      fhv_perfmon::startRegion("double_flops");
      c = do_flops(a, b, c, NUM_FLOPS);
      fhv_perfmon::stopRegion("double_flops");

      fhv_perfmon::startRegion("copy");
      do_copy(arr, copy_arr, n, NUM_COPIES);
      fhv_perfmon::stopRegion("copy");

      fhv_perfmon::nextGroup();
    }
  }

  printf("final c: %f\n", c);
  printf("final random part of copy_arr: %f\n", 
         copy_arr[( (lli) c ) % n]);

  fhv_perfmon::close();

  // fhv_perfmon::printDetailedResults();
  // fhv_perfmon::printAggregateResults();
  fhv_perfmon::printHighlights();
  fhv_perfmon::resultsToJson();
}
