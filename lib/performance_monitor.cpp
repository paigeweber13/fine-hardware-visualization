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

#pragma omp parallel
  {
    // Brandon's code includdes the following comment:

    // Read on mailing list dont need to do this unless not already pinning

    // I cannot find anything in the mailing list... what does it mean?

    // Init marker api for current thread
    likwid_markerThreadInit(); 

    // optionally pin each thread to single core
    likwid_pinThread(omp_get_thread_num()); 

    this->num_threads = omp_get_num_threads();
  }

  this->runtime = 0;

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
  this->runtime = std::max(this->runtime, time);
}

void performance_monitor::close(){
  likwid_markerClose();
  printResults();
}

void performance_monitor::printResults()
{
  int gid;
  const char * flops_event_name = "FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE";
  float num_flops = 0.;
  const char * mflops_metric_name = "AVX SP [MFLOP/s]";
  const uint ops_per_vector = 8;
  float mflops = 0.;

  printf("----- begin performance_monitor report -----");
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
      for (int k = 0; k < perfmon_getEventsOfRegion(i); k++){
        const char * event_name = perfmon_getEventName(gid, k);
        const char * counter_name = perfmon_getCounterName(gid, k);
        float event_value = perfmon_getResultOfRegionThread(i, k, t);
        printf("Event %s:%s: %.3f\n", event_name, counter_name, event_value);
        if(strcmp(event_name, flops_event_name) == 0){
          num_flops += event_value;
        }
      }
      for (int k = 0; k < perfmon_getNumberOfMetrics(gid); k++){
        const char * metric_name = perfmon_getMetricName(gid, k);
        float metric_value = perfmon_getMetricOfRegionThread(i, k, t);
        printf("Metric %s: %.3f\n", metric_name, metric_value);
        if(strcmp(metric_name, mflops_metric_name) == 0){
          mflops += metric_value;
        }
      }
      printf("\n");
    }
  }


  printf("Aggregate %s: %.3e\n", flops_event_name, num_flops);
  printf("Total FP ops: %.3e\n", num_flops * ops_per_vector);
  printf("Total runtime: %f\n", this->runtime);
  printf("Aggregate %s: %f\n", mflops_metric_name, mflops);
  printf("Total TFlop/s: %f\n", mflops*MFLOPS_TO_TFLOPS);
  printf("----- end performance_monitor report -----\n");
  printf("\n");

  remove(filepath);
}
