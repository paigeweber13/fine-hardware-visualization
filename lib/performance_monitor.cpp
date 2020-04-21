#include "performance_monitor.h"

std::map<std::string, double> performance_monitor::runtimes_by_tag;

std::map<
  aggregation_type, std::map<
    result_type, std::map<
      std::string, std::map<
        std::string, std::map<
          std::string, double
        >
      >
    >
  >
> performance_monitor::aggregate_results;

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
    aggregate_results[sum][event][regionName][groupName]
      [total_sp_flops_event_name] = 0.;

    for (int k = 0; k < perfmon_getEventsOfRegion(i); k++){
      event_name = perfmon_getEventName(gid, k);
      aggregate_results[sum][event][regionName][groupName][event_name] = 0.;
      aggregate_results[arithmetic_mean][event][regionName][groupName][event_name] = 0.;
      aggregate_results[geometric_mean][event][regionName][groupName][event_name] = 1.;
    }
    for (int k = 0; k < perfmon_getMetricsOfRegion(i); k++){
      metric_name = perfmon_getMetricName(gid, k);
      aggregate_results[sum][metric][regionName][groupName][metric_name] = 0.;
      aggregate_results[arithmetic_mean][metric][regionName][groupName][metric_name] = 0.;
      aggregate_results[geometric_mean][metric][regionName][groupName][metric_name] = 1.;
    }
  }

  // populate aggregate maps

  // first, sums
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
          aggregate_results[sum][event][regionName][groupName][event_name] 
            += event_value;
          aggregate_results[geometric_mean][event][regionName][groupName]
            [event_name] *= event_value;

          // aggregate_results[regionName][all_groups_keyword][event_name] +=
          //   event_value;
          // aggregate_results[all_regions_keyword][all_groups_keyword]
          //   [event_name] += event_value;

          if(strcmp(sp_scalar_flops_event_name, event_name) == 0){
            aggregate_results[sum][event][regionName][groupName]
              [total_sp_flops_event_name] += event_value;
            // aggregate_results[regionName][all_groups_keyword]
            //   [total_sp_flops_event_name] += event_value;
            // aggregate_results[all_regions_keyword][all_groups_keyword]
            //   [total_sp_flops_event_name] += event_value;
          }
          else if(strcmp(sp_avx_128_flops_event_name, event_name) == 0){
            aggregate_results[sum][event][regionName][groupName]
              [total_sp_flops_event_name]
              += event_value * OPS_PER_SP_128_VECTOR;
            // aggregate_results[regionName][all_groups_keyword]
            //   [total_sp_flops_event_name]
            //   += event_value * OPS_PER_SP_128_VECTOR;
            // aggregate_results[all_regions_keyword][all_groups_keyword]
            //   [total_sp_flops_event_name]
            //   += event_value * OPS_PER_SP_128_VECTOR;
          }
          else if(strcmp(sp_avx_256_flops_event_name, event_name) == 0){
            aggregate_results[sum][event][regionName][groupName]
              [total_sp_flops_event_name]
              += event_value * OPS_PER_SP_256_VECTOR;
            // aggregate_results[regionName][all_groups_keyword]
            //   [total_sp_flops_event_name]
            //   += event_value * OPS_PER_SP_256_VECTOR;
            // aggregate_results[all_regions_keyword][all_groups_keyword]
            //   [total_sp_flops_event_name]
            //   += event_value * OPS_PER_SP_256_VECTOR;
          }
        }
      }

      for (int k = 0; k < perfmon_getNumberOfMetrics(gid); k++){
        metric_name = perfmon_getMetricName(gid, k);
        metric_value = perfmon_getMetricOfRegionThread(i, k, t);
        if(!isnan(metric_value)){
          aggregate_results[sum][metric][regionName][groupName][metric_name]
            += metric_value;
          aggregate_results[geometric_mean][metric][regionName][groupName]
            [metric_name] *= metric_value;
          // aggregate_results[sum][metric][regionName][all_groups_keyword][metric_name]
          //   += metric_value;
          // aggregate_results[sum][metric][all_regions_keyword][all_groups_keyword]
          //   [metric_name] += metric_value;
        }
      }
    }
  }

  // then, averages
  for (int i = 0; i < perfmon_getNumberOfRegions(); i++)
  {
    regionName = perfmon_getTagOfRegion(i);
    gid = perfmon_getGroupOfRegion(i);
    groupName = perfmon_getGroupName(gid);

    for (int k = 0; k < perfmon_getEventsOfRegion(i); k++)
    {
      event_name = perfmon_getEventName(gid, k);
      aggregate_results[arithmetic_mean][event][regionName][groupName][event_name] =
          aggregate_results[sum][event][regionName][groupName][event_name] / num_threads;

      aggregate_results[geometric_mean][event][regionName][groupName]
        [event_name] = 
        pow(
          aggregate_results[geometric_mean][event][regionName][groupName]
            [event_name],
          1/(double)num_threads);
    }

    for (int k = 0; k < perfmon_getNumberOfMetrics(gid); k++)
    {
      metric_name = perfmon_getMetricName(gid, k);
      aggregate_results[arithmetic_mean][metric][regionName][groupName][metric_name] =
          aggregate_results[sum][metric][regionName][groupName][metric_name] / num_threads;
    }
  }
}

void performance_monitor::compareActualWithBench()
{
  if(aggregate_results.size() == 0){
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

      if(!aggregate_results[sum][metric][regionName].count(metricGroupName))
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
          aggregate_results[sum][metric][regionName][metricGroupName][metricName] 
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
  if(aggregate_results.size() == 0){
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

  std::string aggregation_type_string;
  std::string result_type_string;

  std::cout << "aggregation type" << " : " 
            << "result type" <<      " : "
            << "region name" << " : " 
            << "group name" << " : " 
            << "metric name" << " : "
            << "metric value" << '\n';
  for (auto it1 = aggregate_results.begin(); it1 != aggregate_results.end(); ++it1)
  {
    if (it1->first == aggregation_type::sum)
      aggregation_type_string = "sum";
    else if (it1->first == aggregation_type::arithmetic_mean)
      aggregation_type_string = "arithmetic mean";
    else if (it1->first == aggregation_type::geometric_mean)
      aggregation_type_string = "geometric mean";

    for (auto it2 = it1->second.begin(); it2 != it1->second.end(); ++it2)
    {
      if (it2->first == result_type::event)
        result_type_string = "event";
      else if (it2->first == result_type::metric)
        result_type_string = "metric";

      for (auto it3 = it2->second.begin(); it3 != it2->second.end(); ++it3)
      {
        for (auto it4 = it3->second.begin(); it4 != it3->second.end(); ++it4)
        {
          for (auto it5 = it4->second.begin(); it5 != it4->second.end(); ++it5)
          {
            std::cout << aggregation_type_string << " : " 
                      << result_type_string <<      " : "
                      << "region " << it3->first << " : " 
                      << "group " << it4->first <<  " : " 
                      << it5->first <<              " : "
                      << it5->second << '\n';
          }
        }
      }
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;

  std::cout << " ----- end aggregate performance_monitor report -----\n";
  std::cout << std::endl;
}

void performance_monitor::printComparison(){
  if(aggregate_results.size() == 0){
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

void performance_monitor::printPortUsageInfo(){
  if(aggregate_results.size() == 0){
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
  if(aggregate_results.size() == 0){
    std::cout << "ERROR: you must run "
              << "performance_monitor::compareActualWithBench before\n"
              << "printing comparison.\n";
    return;
  }
  if(aggregate_results.size() == 0){
    std::cout << "ERROR: you must run "
              << "performance_monitor::compareActualWithBench before\n"
              << "printing comparison.\n";
    return;
  }

  // json results;
  // for (auto it=saturation.begin(); it!=saturation.end(); ++it)
  //   results["saturation"][it->first] = it->second;

  // std::ofstream o(jsonResultOutputFilepath);
  // o << std::setw(4) << results << std::endl;
}

const std::map<std::string,double> performance_monitor::get_runtimes_by_tag() {
	return runtimes_by_tag;
}


const std::map<
  aggregation_type, std::map<
    result_type, std::map<
      std::string, std::map<
        std::string, std::map<
          std::string, double
        >
      >
    >
  >
>
performance_monitor::get_aggregate_results()
{
	return aggregate_results;
}

const std::map<std::string, std::map<std::string, double>>
performance_monitor::get_saturation()
{
	return saturation;
}
