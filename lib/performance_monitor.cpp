#include "performance_monitor.h"

int performance_monitor::num_threads;

std::map<std::string, double> performance_monitor::runtimes_by_tag;
std::map<std::string, std::map<std::string, double>> performance_monitor::aggregate_events;
std::map<std::string, std::map<std::string, double>> performance_monitor::aggregate_metrics;
std::map<std::string, double> performance_monitor::saturation;

// filenames
const std::string performance_monitor::likwidOutputFilepath = "/tmp/test_marker.out";
const std::string performance_monitor::jsonResultOutputFilepath = "./perfmon_output.json";

// misc
const std::string performance_monitor::accessmode = ACCESSMODE_DAEMON;

void performance_monitor::init(const char * parallel_regions,
                               const char * sequential_regions)
{
  init("MEM_DP|FLOPS_SP|L3|L2|PORT_USAGE1|PORT_USAGE2|PORT_USAGE3",
       parallel_regions, sequential_regions);
}

void
performance_monitor::init(const char * event_group, 
                          const char * parallel_regions,
                          const char * sequential_regions)
{
  int num_threads;
#pragma omp parallel
  {
    num_threads = omp_get_num_threads();
  }

  init(event_group, parallel_regions, sequential_regions, num_threads);
}

void
performance_monitor::init(const char * event_group, 
                          const char * parallel_regions,
                          const char * sequential_regions,
                          int num_threads)
{
  std::string likwid_threads_string;
  for(int i = 0; i < num_threads; i++){
    likwid_threads_string += std::to_string(i);
    if(i != num_threads - 1){
      likwid_threads_string += ',';
    }
  }
  const char * list_of_threads = likwid_threads_string.c_str();

  // std::cout << "list of threads: " << list_of_threads << "\n";

  init(event_group, parallel_regions, sequential_regions, list_of_threads);
}

void
registerRegions(
    const char *regions)
{
  if (strcmp(regions, "") == 0)
    return;

  std::string regions_string(regions);
  std::string delimiter = ",";

  size_t start_pos = 0;
  size_t end_pos = 0;
  std::string token;
  do
  {
    end_pos = regions_string.find(delimiter, start_pos);
    if (end_pos != std::string::npos)
      token = regions_string.substr(start_pos, end_pos - start_pos);
    else
      token = regions_string.substr(start_pos, end_pos);

    std::cout << "registering region " + token + " on thread " +
                     std::to_string(omp_get_thread_num()) + "\n";
    likwid_markerRegisterRegion(token.c_str());
    start_pos = end_pos + delimiter.length();
  } while (end_pos != std::string::npos);
}

void
performance_monitor::init(const char * event_group, 
                          const char * parallel_regions,
                          const char * sequential_regions,
                          const char * list_of_threads)
{
  remove(likwidOutputFilepath.c_str());

  setenv("LIKWID_EVENTS", event_group, 1);
  setenv("LIKWID_MODE", accessmode.c_str(), 1);

  // output filepath
  setenv("LIKWID_FILEPATH", likwidOutputFilepath.c_str(), 1); 
  
  // list of threads to use
  setenv("LIKWID_THREADS", list_of_threads, 1);

  // forces likwid to take control of registers even if they are in use
  setenv("LIKWID_FORCE", "1", 1);

  // likwid marker init reads the environment variables above
  likwid_markerInit();


#pragma omp parallel
  {
    // Init marker api for current thread
    likwid_markerThreadInit(); 

    // about 'likwid_markerRegisterRegion:`

    // optional according to
    // https://github.com/RRZE-HPC/likwid/wiki/TutorialMarkerC

    // BUT highly recommended when using accessD according to
    // https://github.com/RRZE-HPC/likwid/wiki/likwid-perfctr#using-the-marker-api

    // initialize every parallel region supplied
    registerRegions(parallel_regions);

    // optionally pin each thread to single core
    // likwid_pinThread(omp_get_thread_num()); 
  }

  // initialize every sequential region supplied
  registerRegions(sequential_regions);

  // printf("Thread count initialized to %d\n", num_threads);
  // printf("Number of groups setup: %d\n", perfmon_getNumberOfGroups());
}

void performance_monitor::startRegion(const char * tag)
{
  likwid_markerStartRegion(tag);
}

void performance_monitor::stopRegion(const char * tag)
{
  likwid_markerStopRegion(tag);

  int nevents = 20;
  double events[nevents];
  double time;
  int count;

  LIKWID_MARKER_GET(tag, &nevents, events, &time, &count);

  // if there are multiple runs of the same tag this will report the longest of
  // all runs. Maybe it would be better to have an average? This isn't widely
  // used now so it isn't a big concern
  if(runtimes_by_tag.count(tag)){
    runtimes_by_tag[tag] = fmax(runtimes_by_tag[tag], time);
  } else {
    runtimes_by_tag[tag] = time;
  }
}

void performance_monitor::nextGroup(){
  likwid_markerNextGroup();
}

void performance_monitor::close(){
  perfmon_stopCounters();
  likwid_markerClose();

  getAggregateResults();
  compareActualWithbench();
  resultsToJson();
}

void performance_monitor::getAggregateResults(){
  int gid;
  float event_value, metric_value;
  const char * event_name, * metric_name;

  perfmon_readMarkerFile(likwidOutputFilepath.c_str());

  char * groupName;

  // initialize everything to 0
  for (int i = 0; i < perfmon_getNumberOfRegions(); i++)
  {
    gid = perfmon_getGroupOfRegion(i);
    groupName = perfmon_getGroupName(gid);
    aggregate_events[groupName][total_sp_flops_event_name] = 0.;

    for (int k = 0; k < perfmon_getEventsOfRegion(i); k++){
      event_name = perfmon_getEventName(gid, k);
      aggregate_events[groupName][event_name] = 0.;
    }
    for (int k = 0; k < perfmon_getNumberOfMetrics(gid); k++){
      metric_name = perfmon_getMetricName(gid, k);
      aggregate_metrics[groupName][metric_name] = 0.;
    }
  }

  for (int t = 0; t < num_threads; t++)
  {
    for (int i = 0; i < perfmon_getNumberOfRegions(); i++)
    {
      gid = perfmon_getGroupOfRegion(i);
      groupName = perfmon_getGroupName(gid);

      for (int k = 0; k < perfmon_getEventsOfRegion(i); k++){
        event_name = perfmon_getEventName(gid, k);
        event_value = perfmon_getResultOfRegionThread(i, k, t);
        if(event_value > 0){
          aggregate_events[groupName][event_name] += event_value;

          if(strcmp(sp_scalar_flops_event_name, event_name) == 0){
            aggregate_events[groupName][total_sp_flops_event_name] += event_value;
          }
          else if(strcmp(sp_avx_128_flops_event_name, event_name) == 0){
            aggregate_events[groupName][total_sp_flops_event_name] += 
              event_value * OPS_PER_SP_128_VECTOR;
          }
          else if(strcmp(sp_avx_256_flops_event_name, event_name) == 0){
            aggregate_events[groupName][total_sp_flops_event_name] += 
              event_value * OPS_PER_SP_256_VECTOR;
          }
        }
      }
      for (int k = 0; k < perfmon_getNumberOfMetrics(gid); k++){
        metric_name = perfmon_getMetricName(gid, k);
        metric_value = perfmon_getMetricOfRegionThread(i, k, t);
        if(!isnan(metric_value)){
          aggregate_metrics[groupName][metric_name] += metric_value;
        }
      }
    }
  }
}

void performance_monitor::compareActualWithbench()
{
  int gid;
  char * groupName;
  for (int i = 0; i < perfmon_getNumberOfRegions(); i++)
  {
    gid = perfmon_getGroupOfRegion(i);
    groupName = perfmon_getGroupName(gid);
    saturation[mflops_metric_name] =
        fmax(saturation[mflops_metric_name], 
             aggregate_metrics[groupName][mflops_metric_name] 
             / EXPERIENTIAL_SP_RATE_MFLOPS);
    saturation[mflops_dp_metric_name] =
        fmax(saturation[mflops_dp_metric_name],
             aggregate_metrics[groupName][mflops_dp_metric_name] 
             / EXPERIENTIAL_DP_RATE_MFLOPS);
    saturation[l2_bandwidth_metric_name] =
        fmax(saturation[l2_bandwidth_metric_name],
             aggregate_metrics[groupName][l2_bandwidth_metric_name] 
             / EXPERIENTIAL_RW_BW_L2);
    saturation[l3_bandwidth_metric_name] =
        fmax(saturation[l3_bandwidth_metric_name],
             aggregate_metrics[groupName][l3_bandwidth_metric_name] 
             / EXPERIENTIAL_RW_BW_L3);
    saturation[ram_bandwidth_metric_name] =
        fmax(saturation[ram_bandwidth_metric_name],
             aggregate_metrics[groupName][ram_bandwidth_metric_name] 
             / EXPERIENTIAL_RW_BW_RAM);
  }
}

void performance_monitor::printResults()
{
  printDetailedResults();
  printOnlyAggregate();
  printComparison();
}

void performance_monitor::printDetailedResults()
{
  int gid;
  float event_value, metric_value;
  const char * event_name, * counter_name, * metric_name;

  printf("----- begin performance_monitor report -----\n");
  perfmon_readMarkerFile(likwidOutputFilepath.c_str());
  printf("\nMarker API measured %d regions\n", perfmon_getNumberOfRegions());
  for (int i = 0; i < perfmon_getNumberOfRegions(); i++)
  {
    gid = perfmon_getGroupOfRegion(i);
    printf("Region %s with %d events and %d metrics\n", perfmon_getTagOfRegion(i),
           perfmon_getEventsOfRegion(i),
           perfmon_getMetricsOfRegion(i));
  }

  for (int t = 0; t < num_threads; t++)
  {
    printf("\nMetrics output for hardware thread %d\n", t);

    for (int i = 0; i < perfmon_getNumberOfRegions(); i++)
    {
      gid = perfmon_getGroupOfRegion(i);
      printf("Region %s with gid %d\n", perfmon_getTagOfRegion(i), gid);
      for (int k = 0; k < perfmon_getEventsOfRegion(i); k++){
        event_name = perfmon_getEventName(gid, k);
        counter_name = perfmon_getCounterName(gid, k);
        event_value = perfmon_getResultOfRegionThread(i, k, t);
        printf("Event %s:%s: %.3f\n", event_name, counter_name, event_value);
      }
      for (int k = 0; k < perfmon_getNumberOfMetrics(gid); k++){
        metric_name = perfmon_getMetricName(gid, k);
        metric_value = perfmon_getMetricOfRegionThread(i, k, t);
        printf("Metric %s: %.3f\n", metric_name, metric_value);
      }
      printf("\n");
    }
  }
}

void performance_monitor::printOnlyAggregate()
{
  // unnecessary, compareActualWithBench() calls this and it is called in close
  // getAggregateResults();

  std::cout << "----- begin performance_monitor report -----\n";
  std::cout << "--- runtimes by tag: \n";
  for (std::map<std::string, double>::iterator it=runtimes_by_tag.begin(); 
       it!=runtimes_by_tag.end(); ++it){
    std::cout << "Runtime for " + it->first + ": "
               + std::to_string(it->second) + "\n";
  }
  std::cout << std::endl;

  std::cout << "----- begin aggregate performance_monitor report -----\n";
  std::cout << "--- aggregate events by group: \n";
  for (auto it=aggregate_events.begin(); it!=aggregate_events.end(); ++it)
  {
    std::cout << "- Aggregate events for group " << it->first << '\n';
    for (auto it2=it->second.begin(); it2!=it->second.end(); ++it2){
      std::cout << "Aggregate " << it2->first << ": " << it2->second << '\n';
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;

  std::cout << "--- aggregate metrics by group: \n";
  for (auto it=aggregate_metrics.begin(); it!=aggregate_metrics.end(); ++it)
  {
    std::cout << "- Aggregate metrics for group " << it->first << '\n';
    for (auto it2=it->second.begin(); it2!=it->second.end(); ++it2){
      std::cout << "Aggregate " << it2->first << ": " << it2->second << '\n';
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;

  std::cout << "----- end aggregate performance_monitor report -----\n";
  std::cout << std::endl;
}

void performance_monitor::printComparison(){
  // unnecessary, it is called in close
  // compareActualWithbench();

  std::cout << 
    "----- begin saturation level performance_monitor report -----\n";
  for (auto it=saturation.begin(); it!=saturation.end(); ++it)
  {
    std::cout << "Percentage of available " << it->first << " used: " 
      << it->second << '\n';
  }
  std::cout << std::endl;
  std::cout << "----- end saturation level performance_monitor report -----\n";
  std::cout << std::endl;
}

void performance_monitor::resultsToJson(){
  // json results;
  // for (auto it=saturation.begin(); it!=saturation.end(); ++it)
  //   results["saturation"][it->first] = it->second;

  // std::ofstream o(jsonResultOutputFilepath);
  // o << std::setw(4) << results << std::endl;
}

const std::map<std::string,double> performance_monitor::get_runtimes_by_tag() {
	return runtimes_by_tag;
}

const std::map<std::string,std::map<std::string, double>> performance_monitor::get_aggregate_events() {
	return aggregate_events;
}

const std::map<std::string,std::map<std::string, double>> performance_monitor::get_aggregate_metrics() {
	return aggregate_metrics;
}

const std::map<std::string, double> performance_monitor::get_saturation() {
	return saturation;
}
