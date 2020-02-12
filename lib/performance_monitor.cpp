#include "performance_monitor.h"

void performance_monitor::likwid_perfmonInit(const char * event_group)
{
  setenv("LIKWID_EVENTS", event_group, 1);
  setenv("LIKWID_MODE", this->accessmode, 1);
  setenv("LIKWID_FILEPATH", this->filepath, 1); // output filepath
  // unfortunately, this likwid_threads envvar is absolutely necessary
  setenv("LIKWID_THREADS", "0,1,2,3", 1); // list of threads
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

void performance_monitor::likwid_perfmonStartRegion(const char * tag)
{
  // optional according to https://github.com/RRZE-HPC/likwid/wiki/TutorialMarkerC
  // likwid_markerRegisterRegion(tag);

  perfmon_startCounters();
  likwid_markerStartRegion(tag);
}

void performance_monitor::likwid_perfmonStopRegion(const char * tag)
{
  likwid_markerStopRegion(tag);
  perfmon_stopCounters();

  int nevents = 20;
  double events[nevents];
  double time;
  int count;

  LIKWID_MARKER_GET(tag, &nevents, events, &time, &count);
  printf("Tag %s: Thread %d got %d events, runtime %f s, call count %d\n",
         tag, omp_get_thread_num(), nevents, time, count);

}

void performance_monitor::likwid_perfmonClose(){
  likwid_markerClose();
  likwid_perfmonPrintResults();
  perfmon_finalize();
}

void performance_monitor::likwid_perfmonPrintResults()
{
  int gid;

  perfmon_readMarkerFile(this->filepath);
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
