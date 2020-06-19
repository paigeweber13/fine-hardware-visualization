// used to demonstrate unreasonably high values sometimes produced by likwid.
// This is non-deterministic behavior and is documented at the following
// locations:

// https://groups.google.com/forum/?utm_medium=email&utm_source=footer#!msg/likwid-users/m1ElsBTerfk/rHczVoFkBQAJ

// https://github.com/RRZE-HPC/likwid/issues/292 

// can be compiled with
// `gcc likwid_minimal.c -L/path/to/likwid/lib -march=native -mtune=native -fopenmp -llikwid -o likwid_minimal`

// and ran with
// `LD_LIBRARY_PATH=/path/to/likwid/lib PATH=/path/to/likwid/sbin:$PATH ./likwid_minimal`

#include <omp.h>
#include <likwid.h>
#include <stdio.h>
#include <stdlib.h>

// #define NUM_FLOPS 10000000
#define NUM_FLOPS 100000000
// #define NUM_COPIES 1000
#define NUM_COPIES 100000

typedef unsigned long long ull;

void do_copy(double *arr, double *copy_arr, size_t n, ull num_copies) {
  for (ull iter = 0; iter < num_copies; iter++) {
    for (size_t i = 0; i < n; i++) {
      copy_arr[i] = arr[i];
    }
  }
}

double do_flops(double a, double b, double c, ull num_flops) {
  for (ull i = 0; i < num_flops; i++) {
    c = a * b + c;
  }
  return c;
}


int main()
{
  int num_threads;
  const char *filepath = "/tmp/likwid.out";

  setenv("LIKWID_EVENTS",
        //  "MEM|L2|L3|FLOPS_SP|FLOPS_DP|PORT_USAGE1|PORT_USAGE2|PORT_USAGE3",
        //  "MEM|L2|L3|FLOPS_SP|FLOPS_DP",
        //  "L2",
         "L2|L3|FLOPS_SP|FLOPS_DP",
         1);
  setenv("LIKWID_MODE", "1", 1); // 1 is code for accesDaemon
  setenv("LIKWID_FILEPATH", filepath, 1); // output filepath
  setenv("LIKWID_FORCE", "1", 1);
  setenv("LIKWID_DEBUG", "3", 1); // verbosity of debug output

  // optional. Used to disable hyperthreading on my machine
  // num_threads = 2;
  // setenv("LIKWID_THREADS", "0,1", 1); 

  num_threads = 4;
  setenv("LIKWID_THREADS", "0,1,2,3", 1); // list of threads

  remove(filepath); // just in case file is left over from a previous run 
  omp_set_num_threads(num_threads);

  likwid_markerInit();

#pragma omp parallel
  {
    // printf("num threads: %d\n", omp_get_num_threads());
    // num_threads = omp_get_num_threads();
    likwid_markerThreadInit();
    likwid_markerRegisterRegion("double_flops");
    likwid_markerRegisterRegion("copy");
    likwid_pinThread(omp_get_thread_num());
  }

  double a, b, c;
  a = 1.8;
  b = 3.2;
  c = 1.0;

  size_t n = 256;
  double arr[n];
  double copy_arr[n];

#pragma omp parallel
  {
    for (int j = 0; j < 8; j++)
    {
#pragma omp barrier
      likwid_markerStartRegion("double_flops");
      do_flops(a, b, c, NUM_FLOPS);
      likwid_markerStopRegion("double_flops");
#pragma omp barrier
      likwid_markerStartRegion("copy");
      do_copy(arr, copy_arr, n, NUM_COPIES);
      likwid_markerStopRegion("copy");
#pragma omp barrier
      likwid_markerNextGroup();
    }
  }

  likwid_markerClose();

  // these values are meaningless; the print is used to ensure computation and
  // copies aren't optimized away 
  printf("c = %f, copy_arr[c %% n] = %f\n", c, copy_arr[((size_t)c) % n]);

  perfmon_readMarkerFile(filepath);

  const char *regionName, *groupName, *event_name, *metric_name;
  int gid;
  double event_value, metric_value;
  for (int t = 0; t < num_threads; t++)
  {
    for (int i = 0; i < perfmon_getNumberOfRegions(); i++)
    {
      regionName = perfmon_getTagOfRegion(i);
      gid = perfmon_getGroupOfRegion(i);
      groupName = perfmon_getGroupName(gid);

      for (int k = 0; k < perfmon_getEventsOfRegion(i); k++){
        event_name = perfmon_getEventName(gid, k);
        event_value = perfmon_getResultOfRegionThread(i, k, t);
        if (event_value > 1e15)
        {
          printf("WARNING: unreasonably high event value detected\n");
        }
        printf("thread %d : region %s : group %s : event %s : %f\n", 
          t, regionName, groupName, event_name, event_value);
      }

      for (int k = 0; k < perfmon_getNumberOfMetrics(gid); k++){
        metric_name = perfmon_getMetricName(gid, k);
        metric_value = perfmon_getMetricOfRegionThread(i, k, t);
        if (metric_value > 1e6)
        {
          printf("WARNING: unreasonably high metric value detected\n");
        }
        printf("thread %d : region %s : group %s : metric %s : %f\n", 
          t, regionName, groupName, metric_name, metric_value);
      }
    }
  }

  // not removing filepath afterwards so I can optionally inspect it manually
  remove(filepath);

}
