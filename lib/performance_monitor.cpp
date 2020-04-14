#include "performance_monitor.h"

int performance_monitor::num_threads;

std::map<std::string, double> performance_monitor::runtimes_by_tag;
std::map<std::string, std::map<std::string, std::map<std::string, double>>> 
  performance_monitor::aggregate_events;
std::map<std::string, std::map<std::string, std::map<std::string, double>>>
  performance_monitor::aggregate_metrics;
std::map<std::string, std::map<std::string, double>>
  performance_monitor::saturation;

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

// void
// performance_monitor::setEnvironmentVariables(
//   const char * event_group)
// {
//   int num_threads;
// #pragma omp parallel
//   {
//     num_threads = omp_get_num_threads();
//   }

//   setEnvironmentVariables(event_group, num_threads);
// }

// void
// performance_monitor::setEnvironmentVariables(
//   const char * event_group,
//   int num_threads)
// {
//   std::string likwid_threads_string;
//   for(int i = 0; i < num_threads; i++){
//     likwid_threads_string += std::to_string(i);
//     if(i != num_threads - 1){
//       likwid_threads_string += ',';
//     }
//   }
//   const char * list_of_threads = likwid_threads_string.c_str();

//   setEnvironmentVariables(event_group, list_of_threads);
// }

void
performance_monitor::setEnvironmentVariables(
  const char * event_group,
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
}

void
performance_monitor::registerRegions(
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
  setEnvironmentVariables(event_group, list_of_threads);

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

    // pin each thread to single core. If you don't do this, many "stopping
    // non-started region x" errors will happen 
    likwid_pinThread(omp_get_thread_num()); 
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
  likwid_markerClose();

  getAggregateResults();
  compareActualWithBench();
  resultsToJson();
}

void performance_monitor::printRegionGroupEventAndMetricData(){
  int gid;

  printf("\n\n ----- Region, Group, Event, and Metric Data ----- \n\n");
  perfmon_readMarkerFile(likwidOutputFilepath.c_str());
  printf("number of regions: %d\n", perfmon_getNumberOfRegions());

  for (int i = 0; i < perfmon_getNumberOfRegions(); i++)
  {
    gid = perfmon_getGroupOfRegion(i);
    printf("Region %s with %d events and %d metrics\n", 
           perfmon_getTagOfRegion(i),
           perfmon_getEventsOfRegion(i),
           perfmon_getMetricsOfRegion(i));
    printf("    This region has associated group with id %d and name %s\n",
           gid, perfmon_getGroupName(gid));
  }

  printf("\nnumber of groups: %d\n", perfmon_getNumberOfGroups());
  for (int i = 0; i < perfmon_getNumberOfGroups(); i++) {
    printf("group %s\n", perfmon_getGroupName(i));
  }
}

// note on getAggregateResults: I've considered also aggregating across groups
// and regions (see commented lines in this function)

// This doesn't always make sense and isn't terribly useful right now, so it's
// commented out.

// the "all_groups" keyword takes the place of a group name. it is calculated
// for each region and includes all metrics
// from all groups. This only makes sense if the set of stuff checked by each
// group is mutually exclusive with the other groups, but this is almost
// always the case. The only time I can think of when this is not the case is
// if someone specifies both the "MEM_DP" group and the "FLOPS_DP" group.
// Likewise, specifying both "MEM_SP" and "FLOPS_SP" would mess up the
// results.

// the "all_regions" keyword takes the place of a region tag and only
// supports one group: the "all_groups" keyword. This only makes sense
// sometimes. For example: what if you have two regions "work_part_1" and
// "work_part_2" that both do lots of DP FP operations. It doesn't make
// sense to ADD the FLOP/S rates from both regions together...

void performance_monitor::getAggregateResults()
{
  int gid, num_threads;
  float event_value, metric_value;
  const char *event_name, *metric_name;

#pragma omp parallel
  {
    // needed because we use it to print results later
    num_threads = omp_get_num_threads();
  }

  perfmon_readMarkerFile(likwidOutputFilepath.c_str());

  char * groupName;
  char * regionName;

  // initialize everything to 0

  // there is a region for every region/group combo, so we only need one outer
  // for loop
  for (int i = 0; i < perfmon_getNumberOfRegions(); i++)
  {
    regionName = perfmon_getTagOfRegion(i);
    gid = perfmon_getGroupOfRegion(i);
    groupName = perfmon_getGroupName(gid);
    aggregate_events[regionName][groupName][total_sp_flops_event_name] = 0.;
    // aggregate_events[regionName][all_groups_keyword][total_sp_flops_event_name] = 0.;
    // aggregate_events[all_regions_keyword][all_groups_keyword][total_sp_flops_event_name] = 0.;

    for (int k = 0; k < perfmon_getEventsOfRegion(i); k++){
      event_name = perfmon_getEventName(gid, k);
      aggregate_events[regionName][groupName][event_name] = 0.;
      // aggregate_events[regionName][all_groups_keyword][event_name] = 0.;
      // aggregate_events[all_regions_keyword][all_groups_keyword][event_name] = 0.;
    }
    for (int k = 0; k < perfmon_getMetricsOfRegion(i); k++){
      metric_name = perfmon_getMetricName(gid, k);
      aggregate_metrics[regionName][groupName][metric_name] = 0.;
      // aggregate_metrics[regionName][all_groups_keyword][metric_name] = 0.;
      // aggregate_metrics[all_regions_keyword][all_groups_keyword][metric_name] = 0.;
    }
  }

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
        if(event_value > 0){
          aggregate_events[regionName][groupName][event_name] += event_value;
          // aggregate_events[regionName][all_groups_keyword][event_name] +=
          //   event_value;
          // aggregate_events[all_regions_keyword][all_groups_keyword]
          //   [event_name] += event_value;

          if(strcmp(sp_scalar_flops_event_name, event_name) == 0){
            aggregate_events[regionName][groupName][total_sp_flops_event_name] 
              += event_value;
            // aggregate_events[regionName][all_groups_keyword]
            //   [total_sp_flops_event_name] += event_value;
            // aggregate_events[all_regions_keyword][all_groups_keyword]
            //   [total_sp_flops_event_name] += event_value;
          }
          else if(strcmp(sp_avx_128_flops_event_name, event_name) == 0){
            aggregate_events[regionName][groupName][total_sp_flops_event_name]
              += event_value * OPS_PER_SP_128_VECTOR;
            // aggregate_events[regionName][all_groups_keyword]
            //   [total_sp_flops_event_name]
            //   += event_value * OPS_PER_SP_128_VECTOR;
            // aggregate_events[all_regions_keyword][all_groups_keyword]
            //   [total_sp_flops_event_name]
            //   += event_value * OPS_PER_SP_128_VECTOR;
          }
          else if(strcmp(sp_avx_256_flops_event_name, event_name) == 0){
            aggregate_events[regionName][groupName][total_sp_flops_event_name]
              += event_value * OPS_PER_SP_256_VECTOR;
            // aggregate_events[regionName][all_groups_keyword]
            //   [total_sp_flops_event_name]
            //   += event_value * OPS_PER_SP_256_VECTOR;
            // aggregate_events[all_regions_keyword][all_groups_keyword]
            //   [total_sp_flops_event_name]
            //   += event_value * OPS_PER_SP_256_VECTOR;
          }
        }
      }

      for (int k = 0; k < perfmon_getNumberOfMetrics(gid); k++){
        metric_name = perfmon_getMetricName(gid, k);
        metric_value = perfmon_getMetricOfRegionThread(i, k, t);
        if(!isnan(metric_value)){
          aggregate_metrics[regionName][groupName][metric_name]
            += metric_value;
          // aggregate_metrics[regionName][all_groups_keyword][metric_name]
          //   += metric_value;
          // aggregate_metrics[all_regions_keyword][all_groups_keyword]
          //   [metric_name] += metric_value;
        }
      }
    }
  }
}

void performance_monitor::compareActualWithBench()
{
  if(aggregate_events.size() == 0){
    std::cout << "ERROR: you must run performance_monitor::getAggregateResults"
              << " before printing\n"
              << "aggregate results. If you are getting this error and you did"
              << " run getAggregateResults,\n"
              << "No metrics have been aggregated. Pleases submit a bug report"
              << "\n";
    return;
  }

  const char * regionName;
  const char * metricGroupName;
  const char * metricName;
  float referenceBenchmarkValue;

  // getNumberOfRegions actually returns num_regions * num_groups
  int num_regions = perfmon_getNumberOfRegions()/perfmon_getNumberOfGroups();

  std::vector<const char *> saturationMetricGroups = {
    likwid_group_flops_sp,
    likwid_group_flops_dp,
    likwid_group_l2,
    likwid_group_l3,
    likwid_group_mem,
  };

  std::vector<const char *> saturation_metrics = {
    mflops_metric_name,
    mflops_dp_metric_name,
    l2_bandwidth_metric_name,
    l3_bandwidth_metric_name,
    ram_bandwidth_metric_name,
  };

  std::vector<float> saturationBenchmarkReferences = {
    EXPERIENTIAL_SP_RATE_MFLOPS,
    EXPERIENTIAL_DP_RATE_MFLOPS,
    EXPERIENTIAL_RW_BW_L2,
    EXPERIENTIAL_RW_BW_L3,
    EXPERIENTIAL_RW_BW_RAM,
  };

  // initialize all_regions part to 0
  for (size_t j = 0; j < saturation_metrics.size(); j++){
    metricName = saturation_metrics[j];
    saturation[all_regions_keyword][metricName] = 0.;
  }

  for (int i = 0; i < num_regions; i++)
  {
    regionName = perfmon_getTagOfRegion(i);

    for (size_t j = 0; j < saturation_metrics.size(); j++){
      metricGroupName = saturationMetricGroups[j];
      metricName = saturation_metrics[j];
      referenceBenchmarkValue = saturationBenchmarkReferences[j];

      if(!aggregate_metrics[regionName].count(metricGroupName))
      {
        std::cout << "WARN: group " << metricGroupName << " was not measured."
                  << " Therefore, saturation for " << metricName << " will \n"
                  << "not be calculated and will be reported as NaN.\n";

        saturation[regionName][metricName] = NAN;
        saturation[all_regions_keyword][metricName] = NAN;
      }
      else
      {
        saturation[regionName][metricName] =
          aggregate_metrics[regionName][metricGroupName][metricName] 
          / referenceBenchmarkValue;
        saturation[all_regions_keyword][metricName] +=
          saturation[regionName][metricName];
      }
    }
  }

  for (size_t i = 0; i < saturation_metrics.size(); i++)
  {
    metricName = saturation_metrics[i];

    // it doesn't make sense to sum saturation values, so we find the average of
    // saturations to get saturation across regions
    saturation[all_regions_keyword][metricName] /= num_regions;
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
  int gid, num_threads;
  float event_value, metric_value;
  const char *event_name, *counter_name, *metric_name;

#pragma omp parallel
  {
    // needed because we use it to print results later
    num_threads = omp_get_num_threads();
  }

  printf("\n\n ----- Detailed performance_monitor report ----- \n\n");
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
  printf("\n\n ----- end detailed performance_monitor report ----- \n\n");
}

void performance_monitor::printOnlyAggregate()
{
  // unnecessary, compareActualWithBench() calls this and it is called in close
  // getAggregateResults();

  std::cout << "\n\n----- aggregate performance_monitor report -----\n\n";
  if(aggregate_events.size() == 0){
    std::cout << "ERROR: you must run performance_monitor::getAggregateResults"
              << " before printing\n"
              << "aggregate results\n";
    return;
  }

  // TODO: fix runtimes (enforce calling performance_monitor::stopRegion
  // instead of likwid_markerStopRegion?)

  // std::cout << " --- runtimes by tag: \n";
  // for (std::map<std::string, double>::iterator it=runtimes_by_tag.begin();
  //      it!=runtimes_by_tag.end(); ++it){
  //   std::cout << "Runtime for " + it->first + ": "
  //              + std::to_string(it->second) + "\n";
  // }
  // std::cout << std::endl;

  std::cout << " --- aggregate events: \n";
  for (auto it = aggregate_events.begin(); it != aggregate_events.end(); ++it)
  {
    std::cout << "\n\n -- Aggregate events for region " << it->first << "\n";
    for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
    {
      std::cout << "\n - Aggregate events for group " << it2->first << '\n';
      for (auto it3 = it2->second.begin(); it3 != it2->second.end(); ++it3)
      {
        std::cout << "Aggregate " << it3->first << ": " << it3->second
                  << '\n';
      }
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;

  std::cout << " --- aggregate metrics: \n";
  for (auto it = aggregate_metrics.begin(); it != aggregate_metrics.end(); ++it)
  {
    std::cout << "\n\n -- Aggregate metrics for region " << it->first << "\n";
    for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
    {
      std::cout << "\n - Aggregate metrics for group " << it2->first << '\n';
      for (auto it3 = it2->second.begin(); it3 != it2->second.end(); ++it3)
      {
        std::cout << "Aggregate " << it3->first << ": " << it3->second
                  << '\n';
      }
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;

  std::cout << " ----- end aggregate performance_monitor report -----\n";
  std::cout << std::endl;
}

void performance_monitor::printComparison(){
  if(aggregate_events.size() == 0){
    std::cout << "ERROR: you must run "
              << "performance_monitor::compareActualWithBench before\n"
              << "printing comparison.\n";
    return;
  }

  std::cout << 
    "\n\n ----- saturation level performance_monitor report -----\n\n";
  for (auto it=saturation.begin(); it!=saturation.end(); ++it)
  {
    std::cout << "\nRegion " << it->first << ":\n";
    for (auto it2=it->second.begin(); it2!=it->second.end(); ++it2)
    {
      std::cout << "Percentage of available " << it2->first << " used: "
        << it2->second << '\n';
    }
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

const std::map<std::string,std::map<std::string, std::map<std::string, double>>> 
performance_monitor::get_aggregate_events() 
{
	return aggregate_events;
}

const std::map<std::string,std::map<std::string, std::map<std::string, double>>> 
performance_monitor::get_aggregate_metrics() 
{
	return aggregate_metrics;
}

const std::map<std::string, std::map<std::string, double>>
performance_monitor::get_saturation()
{
	return saturation;
}
