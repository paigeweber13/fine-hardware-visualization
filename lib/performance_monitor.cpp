#include "performance_monitor.h"

void performance_monitor::likwid_perfmonInit()
{
  setenv("LIKWID_EVENTS", this->event_group, 1);
  setenv("LIKWID_MODE", this->accessmode, 1);
  setenv("LIKWID_FILEPATH", this->filepath, 1); // output filepath
  // setenv("LIKWID_THREADS", "0,1", 1); // list of threads
  // setenv("LIKWID_FORCE", "1", 1);

  // likwid marker init reads the environment variables above
  likwid_markerInit();

  // optionally pin each thread to single core
#pragma omp parallel
  {
    // Init marker api for current thread
    // Read on mailing list dont need to do this unless not already pinning
    // likwid_markerThreadInit(); // init thread hash table

    // pin each thread to single core
    likwid_pinThread(omp_get_thread_num()); 

    this->num_threads = omp_get_num_threads();
  }

  printf("Thread count initialized to %d\n", this->num_threads);
  printf("Number of groups setup: %d\n", perfmon_getNumberOfGroups());
}

void performance_monitor::likwid_perfmonStart()
{
  perfmon_startCounters();
  likwid_markerRegisterRegion(this->tag);

  likwid_markerStartRegion(this->tag);
}

void performance_monitor::likwid_perfmonStop()
{
  int nevents = 20;
  double events[nevents];
  double time;
  int count;

  likwid_markerStopRegion(this->tag);
  LIKWID_MARKER_GET(this->tag, &nevents, events, &time, &count);
  printf("Tag %s: Thread %d got %d events, runtime %f s, call count %d\n",
         this->tag, omp_get_thread_num(), nevents, time, count);

  perfmon_stopCounters();
  likwid_markerClose();
  likwid_perfmonPrintResults();
  perfmon_finalize();
}

void performance_monitor::likwid_perfmonPrintResults()
{
  int gid;

  perfmon_readMarkerFile(filepath);
  printf("\nMarker API measured %d regions\n", perfmon_getNumberOfRegions());
  for (int i = 0; i < perfmon_getNumberOfRegions(); i++)
  {
    gid = perfmon_getGroupOfRegion(i);
    printf("Region %s with %d events and %d metrics\n", perfmon_getTagOfRegion(i),
           perfmon_getEventsOfRegion(i),
           perfmon_getMetricsOfRegion(i));
  }

  for (int t = 0; t < this->num_threads; t++)
  {
    printf("\nMetrics output for thread %d\n", t);

    for (int i = 0; i < perfmon_getNumberOfRegions(); i++)
    {
      gid = perfmon_getGroupOfRegion(i);
      printf("Region %s with gid %d\n", perfmon_getTagOfRegion(i), gid);
      for (int k = 0; k < perfmon_getEventsOfRegion(i); k++)
        printf("Event %s:%s: %f\n", perfmon_getEventName(gid, k),
               perfmon_getCounterName(gid, k),
               perfmon_getResultOfRegionThread(i, k, t));
      for (int k = 0; k < perfmon_getNumberOfMetrics(gid); k++)
        printf("Metric %s: %f\n", perfmon_getMetricName(gid, k),
               perfmon_getMetricOfRegionThread(i, k, t));
      printf("\n");
    }
  }
  remove(filepath);
}
