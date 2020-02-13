#include "performance_monitor.h"

void performance_monitor::init(const char * event_group)
{
  setenv("LIKWID_EVENTS", event_group, 1);
  setenv("LIKWID_MODE", this->accessmode, 1);
  setenv("LIKWID_FILEPATH", this->filepath, 1); // output filepath
  // unfortunately, this likwid_threads envvar is absolutely necessary
  setenv("LIKWID_THREADS", "0,1,2,3", 1); // list of threads
  // forces likwid to take control of registers even if they are in use
  setenv("LIKWID_FORCE", "1", 1);

  // likwid marker init reads the environment variables above
  likwid_markerInit();

  // optionally pin each thread to single core
#pragma omp parallel
  {
    // Brandon's code includdes the following comment:

    // Read on mailing list dont need to do this unless not already pinning

    // I cannot find anything in the mailing list... what does it mean?

    // Init marker api for current thread
    likwid_markerThreadInit(); 

    // pin each thread to single core
    likwid_pinThread(omp_get_thread_num()); 

    this->num_threads = omp_get_num_threads();
  }

  printf("Thread count initialized to %d\n", this->num_threads);
  printf("Number of groups setup: %d\n", perfmon_getNumberOfGroups());
}

void performance_monitor::startRegion(const char * tag)
{
  // optional according to
  // https://github.com/RRZE-HPC/likwid/wiki/TutorialMarkerC

  // BUT highly recommended when using accessD according to
  // https://github.com/RRZE-HPC/likwid/wiki/likwid-perfctr#using-the-marker-api

  likwid_markerRegisterRegion(tag);

  perfmon_startCounters();
  likwid_markerStartRegion(tag);
}

void performance_monitor::stopRegion(const char * tag)
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

void performance_monitor::close(){
  likwid_markerClose();
  printResults();
}

void performance_monitor::printResults()
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
