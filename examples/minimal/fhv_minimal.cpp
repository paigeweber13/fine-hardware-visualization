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

  double a, b, c;
  a = 1.8;
  b = 3.2;
  c = 0.0;

  const lli n = 2048;
  const int NUM_FHV_GROUPS = 7;
  const std::string REGION_FLOPS = "double_flops";
  const std::string REGION_COPY = "copy";

  double arr[n];
  double copy_arr[n];

  // optional: specify regions ahead of time. This is recommended, as it
  // removes the risk of overhead on the first call to `startRegion`
  // fhv_perfmon::init((REGION_FLOPS + "," + REGION_COPY).c_str());
  fhv_perfmon::init();

#pragma omp parallel
  {
    for (int j = 0; j < NUM_FHV_GROUPS; j++)
    {
      printf("thread %d, iteration %d\n", omp_get_thread_num(), j);

      fhv_perfmon::startRegion(REGION_FLOPS.c_str());
      c = do_flops(a, b, c, NUM_FLOPS);
      fhv_perfmon::stopRegion(REGION_FLOPS.c_str());

      fhv_perfmon::startRegion(REGION_COPY.c_str());
      do_copy(arr, copy_arr, n, NUM_COPIES);
      fhv_perfmon::stopRegion(REGION_COPY.c_str());

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
