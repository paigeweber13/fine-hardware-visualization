// can be compiled with
// gcc likwid_minimal.c -L/usr/local/lib -march=native -mtune=native -fopenmp -llikwid
// or the like

// this can be run alone `./a.out`
// or with likwid-perfctr by commenting out the `setenv` lines and then running
// likwid-perfctr -C S0:0 -g FLOPS_DP -M 1 -m ./a.out
// or the like

// some other command examples, since I've been using this to test lots of
// stuff:
//  - likwid-perfctr -C S0:0 -g L3 -g FLOPS_DP -M 1 -m ./a.out
//  - likwid-perfctr -C S0:0 -g FP_ARITH_INST_RETIRED_SCALAR_DOUBLE:PMC0,L2_LINES_IN_ALL:PMC1 -M 1 -m ./a.out

#include <omp.h>
#include <likwid.h>
#include <stdio.h>
#include <stdlib.h>

void copy(double * arr, double * copy_arr, size_t n){
  for (size_t i = 0; i < n; i++){
    copy_arr[i] = arr[i];
  }
}

int main()
{
  // printf("\n\nThis is a minimal example of how the likwid marker api works\n");

  int num_threads;
  const char *filepath = "/tmp/likwid.out";

  setenv("LIKWID_EVENTS",
        //  "MEM|L2|L3|FLOPS_SP|FLOPS_DP|PORT_USAGE1|PORT_USAGE2|PORT_USAGE3",
        //  "MEM|L2|L3|FLOPS_SP|FLOPS_DP",
        //  "L2",
         "L2|L3|FLOPS_SP|FLOPS_DP",
         1);
  setenv("LIKWID_MODE", "1", 1);
  setenv("LIKWID_FILEPATH", filepath, 1); // output filepath

  num_threads = 2;
  setenv("LIKWID_THREADS", "0,1", 1); // list of threads
  setenv("LIKWID_FORCE", "1", 1);

  // num_threads = 4;
  // setenv("LIKWID_THREADS", "0,1,2,3", 1); // list of threads
  // setenv("LIKWID_FORCE", "1", 1);

  remove(filepath);

  likwid_markerInit();

  // disable dynamic teams... may help with "stopping non-started region"
  // errors
  // omp_set_dynamic(0); 
  omp_set_num_threads(num_threads);

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
  c = 0.0;

  size_t n = 256;
  double arr[n];
  double copy_arr[n];

// perfmon_startCounters();
#pragma omp parallel
  {
    for (int j = 0; j < 8; j++)
    {
      // printf("thread %d, iteration %d\n", omp_get_thread_num(), j);
      likwid_markerStartRegion("double_flops");
#pragma omp barrier
      for (int i = 0; i < 10000000; i++)
      {
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
  // perfmon_stopCounters();

  // these may not be necessary? I'm trying to prevent actual work from being
  // optimized out by the compiler...
  // printf("final c: %f\n", c);
  // printf("final random part of copy_arr: %f\n", 
  //        copy_arr[( (size_t) c ) % n]);

  // print results here if you want

  // calling both 'likwid_markerClose()' and 'perfmon_finalize()' causes
  // segfault, but I can call either twice in a row and be fine. This only
  // happens when running the program on its own. If wrapped in likwid-perfctr
  // (see comment block before includes), this doesn't happen

  // About the above comment: I discovered that perfmon is a lower-level
  // library used by likwid. So calling likwid_* will automatically take care
  // of the perfmon_* stuff

  // likwid_markerClose();
  likwid_markerClose();
  // perfmon_finalize();
  // perfmon_finalize();

  perfmon_readMarkerFile(filepath);
  // printf("\nMarker API measured %d regions\n", perfmon_getNumberOfRegions());
  // for (int i = 0; i < perfmon_getNumberOfRegions(); i++)
  // {
  //   int gid = perfmon_getGroupOfRegion(i);
  //   printf("Region %s with %d events and %d metrics\n", perfmon_getTagOfRegion(i),
  //          perfmon_getEventsOfRegion(i),
  //          perfmon_getMetricsOfRegion(i));
  // }

  // for (int t = 0; t < num_threads; t++)
  // {
  //   printf("\nMetrics output for hardware thread %d\n", t);

  //   for (int i = 0; i < perfmon_getNumberOfRegions(); i++)
  //   {
  //     int gid = perfmon_getGroupOfRegion(i);
  //     printf("Region %s\n", perfmon_getTagOfRegion(i));
  //     for (int k = 0; k < perfmon_getEventsOfRegion(i); k++)
  //       printf("Event %s:%s: %f\n", perfmon_getEventName(gid, k),
  //              perfmon_getCounterName(gid, k),
  //              perfmon_getResultOfRegionThread(i, k, 0));
  //     for (int k = 0; k < perfmon_getNumberOfMetrics(gid); k++)
  //       printf("Metric %s: %f\n", perfmon_getMetricName(gid, k),
  //              perfmon_getMetricOfRegionThread(i, k, 0));
  //     printf("\n");
  //   }
  // }
  // remove(filepath);

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
        // "getCounterName" gives name like "PMC0"

        // event_name = perfmon_getCounterName(gid, k);
        event_name = perfmon_getEventName(gid, k);
        event_value = perfmon_getResultOfRegionThread(i, k, t);
        if (event_value > 1e15)
        {
          printf("thread %d : region %s : group %s : event %s : %f\n", 
            t, regionName, groupName, event_name, event_value);
        }
      }

      for (int k = 0; k < perfmon_getNumberOfMetrics(gid); k++){
        metric_name = perfmon_getMetricName(gid, k);
        metric_value = perfmon_getMetricOfRegionThread(i, k, t);
        if (metric_value > 1e6)
        {
          printf("thread %d : region %s : group %s : metric %s : %f\n", 
            t, regionName, groupName, metric_name, metric_value);
        }
      }
    }
  }

  // remove(filepath);

}
