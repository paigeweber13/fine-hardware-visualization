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
  printf("\n\nThis is a minimal example of how the fhv performance_monitor \n"
         "aggregation prints work\n");

  // ---- begin likwid initialization

  const char *filepath = performance_monitor::likwidOutputFilepath.c_str();

  // so 14 group/region combos
  setenv("LIKWID_EVENTS",
         "MEM|L2|L3|FLOPS_SP|FLOPS_DP|PORT_USAGE1|PORT_USAGE2|PORT_USAGE3",
         1);
  // setenv("LIKWID_EVENTS", "MEM_DP|L2", 1);
  setenv("LIKWID_MODE", "1", 1);
  // output filepath
  setenv("LIKWID_FILEPATH", filepath, 1); 
  setenv("LIKWID_THREADS", "0,1,2,3", 1); // list of threads
  setenv("LIKWID_FORCE", "1", 1);

  likwid_markerInit();

#pragma omp parallel
  {
    likwid_markerThreadInit();
    likwid_markerRegisterRegion("double_flops");
    likwid_markerRegisterRegion("copy");
    likwid_pinThread(omp_get_thread_num());
  }

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
      likwid_markerStartRegion("double_flops");
#pragma omp barrier
      for (int i = 0; i < 10000000; i++)
      {
        // 1e7 scalar double floating point operations per iteration
        c = a * b + c;
      }
#pragma omp barrier
      likwid_markerStopRegion("double_flops");
      likwid_markerStartRegion("copy");
      for (int i = 0; i < 1000; i++){
        copy(arr, copy_arr, n);
      }
#pragma omp barrier
      likwid_markerStopRegion("copy");
      likwid_markerNextGroup();
    }
  }

  printf("final c: %f\n", c);
  printf("final random part of copy_arr: %f\n", 
         copy_arr[( (size_t) c ) % n]);

  likwid_markerClose();

  // performance_monitor::printRegionGroupEventAndMetricData();

  performance_monitor::buildResultsMaps();
  // performance_monitor::printDetailedResults();
  // performance_monitor::printOnlyAggregate();

  performance_monitor::compareActualWithBench();
  // performance_monitor::printComparison();

  performance_monitor::printHighlights();

  performance_monitor::resultsToJson();

  std::cout << "\n";
  performance_monitor::printCsvHeader();
  performance_monitor::printCsvOutput();
}
