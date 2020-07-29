#include "performance_monitor.h"

// declarations
performance_monitor::aggregate_results_t 
  performance_monitor::aggregate_results;

performance_monitor::per_thread_results_t 
  performance_monitor::per_thread_results;

// std::map<std::string, std::map<std::string, double>>
//   performance_monitor::saturation;

int performance_monitor::num_threads = -1;

// filenames
const std::string performance_monitor::likwidOutputFilepath = "/tmp/likwid_marker.out";
const std::string performance_monitor::jsonResultOutputFilepath = "./perfmon_output.json";

std::vector<std::string> performance_monitor::key_metrics;

// consider removing
// saturation metrics
// const std::vector<const char *> performance_monitor::saturationMetricGroups = {
//   likwid_group_flops_sp,
//   likwid_group_flops_dp,
//   likwid_group_l2,
//   likwid_group_l3,
//   likwid_group_mem,
// };

// const std::vector<const char *> performance_monitor::saturation_metrics = {
//   mflops_metric_name,
//   mflops_dp_metric_name,
//   l2_bandwidth_metric_name,
//   l3_bandwidth_metric_name,
//   ram_bandwidth_metric_name,
// };

// const std::vector<float> performance_monitor::saturationBenchmarkReferences = {
//   EXPERIENTIAL_SP_RATE_MFLOPS,
//   EXPERIENTIAL_DP_RATE_MFLOPS,
//   EXPERIENTIAL_RW_BW_L2,
//   EXPERIENTIAL_RW_BW_L3,
//   EXPERIENTIAL_RW_BW_RAM,
// };

// // port usage metrics
// const std::vector<std::string> performance_monitor::port_usage_metrics = {
//   fhv_port_usage_ratio_start + "0" + fhv_port_usage_ratio_end,
//   fhv_port_usage_ratio_start + "1" + fhv_port_usage_ratio_end,
//   fhv_port_usage_ratio_start + "2" + fhv_port_usage_ratio_end,
//   fhv_port_usage_ratio_start + "3" + fhv_port_usage_ratio_end,
//   fhv_port_usage_ratio_start + "4" + fhv_port_usage_ratio_end,
//   fhv_port_usage_ratio_start + "5" + fhv_port_usage_ratio_end,
//   fhv_port_usage_ratio_start + "6" + fhv_port_usage_ratio_end,
//   fhv_port_usage_ratio_start + "7" + fhv_port_usage_ratio_end,
// };


// misc
const std::string performance_monitor::accessmode = ACCESSMODE_DAEMON;

// PerThreadResult function definitions
bool
performance_monitor::PerThreadResult::operator<(
  const PerThreadResult& other) const
{
  // start by comparing region names
  if (this->region_name != other.region_name)
    return this->region_name < other.region_name;
  else 
  {
    // if region names match, order by thread num
    if (this->thread_num != other.thread_num)
      return this->thread_num < other.thread_num;
    else
    {
      // if thread nums match, order by group name
      if(this->group_name != other.group_name)
        return this->group_name < other.group_name;
      else
      {
        // if group names match, order by result type
        if(this->result_type != other.result_type)
          return this->result_type < other.result_type;
        else {
          // if result types match, order by result name
          return this->result_name < other.result_name;
        }
      }
    }
  }
}

bool
performance_monitor::PerThreadResult::matchesForAggregation(
  const PerThreadResult& other) const
{
  if(this->region_name == other.region_name
    && this->group_name == other.group_name
    && this->result_type == other.result_type
    && this->result_name == other.result_name)
    return true;
  else 
    return false;
}

// AggregateResult function definitions
bool
performance_monitor::AggregateResult::operator<(
  const AggregateResult& other) const
{
  if (this->region_name != other.region_name)
    return this->region_name < other.region_name;
  else 
  {
    if (this->aggregation_type != other.aggregation_type)
      return this->aggregation_type < other.aggregation_type;
    else
    {
      if(this->group_name != other.group_name)
        return this->group_name < other.group_name;
      else {
        if(this->result_type != other.result_type)
          return this->result_type < other.result_type;
        else 
          return this->result_name < other.result_name;
      }
    }
  }
}

// ------ utility functions ------ //

double performance_monitor::dround(double x, unsigned num_decimal_places){
  double rounding_factor = pow(10., static_cast<double>(num_decimal_places));
  double rounded_x = round(x * rounding_factor)/rounding_factor;
  return rounded_x;
}

// ------ perfmon stuff ------ //

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
#pragma omp parallel
  {
    performance_monitor::num_threads = omp_get_num_threads();
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
  // ----- Initialize likwid stuff ----- // 
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

    // currently, this only supports using the first n threads, where n is the
    // number of threads specified with OMP_NUM_THREADS (or, the maximum number
    // of threads if OMP_NUM_THREADS is not specified)

    // TODO: support setting affinity with GOMP_CPU_AFFINITY or a custom
    // environment variable that is used here to pin threads manually
    likwid_pinThread(omp_get_thread_num()); 
  }

  // initialize every sequential region supplied
  registerRegions(sequential_regions);

  // printf("Thread count initialized to %d\n", num_threads);
  // printf("Number of groups setup: %d\n", perfmon_getNumberOfGroups());

  
  // ----- initialize variables ----- // 
  // start by adding bandwidth, flops, etc
  performance_monitor::key_metrics.clear();
  performance_monitor::key_metrics.insert(
    performance_monitor::key_metrics.begin(), 
    {
      mflops_metric_name,
      mflops_dp_metric_name,
      l2_bandwidth_metric_name,
      l2_data_volume_name,
      l2_evict_bandwidth_name,
      l2_evict_data_volume_name,
      l2_load_bandwidth_name,
      l2_load_data_volume_name,
      l3_bandwidth_metric_name,
      l3_data_volume_name,
      l3_evict_bandwidth_name,
      l3_evict_data_volume_name,
      l3_load_bandwidth_name,
      l3_load_data_volume_name,
      ram_bandwidth_metric_name,
      ram_data_volume_metric_name,
      ram_evict_bandwidth_name,
      ram_evict_data_volume_name,
      ram_load_bandwidth_name,
      ram_load_data_volume_name,
    });

  // add port usage stuff to key metrics
  for (size_t i = 0; i < NUM_PORTS_IN_CORE; i++)
  {
    performance_monitor::key_metrics.push_back(
      fhv_port_usage_ratio_start + std::to_string(i) + 
      fhv_port_usage_ratio_end);
  }

  // add saturation metrics
  performance_monitor::key_metrics.insert(
    performance_monitor::key_metrics.end(), 
    fhv_saturation_metrics.begin(),
    fhv_saturation_metrics.end());

  // initialize num_threads
  #pragma omp parallel
  {
    performance_monitor::num_threads = omp_get_num_threads();
  }
}

void performance_monitor::startRegion(const char * tag)
{
  likwid_markerStartRegion(tag);
}

void performance_monitor::stopRegion(const char * tag)
{
  likwid_markerStopRegion(tag);

  // consider removing everything below here

  // int nevents = 20;
  // double events[nevents];
  // double time;
  // int count;

  // LIKWID_MARKER_GET(tag, &nevents, events, &time, &count);

  // // if there are multiple runs of the same tag this will report the longest of
  // // all runs. Maybe it would be better to have an average? This isn't widely
  // // used now so it isn't a big concern
  // if(runtimes_by_tag.count(tag)){
  //   runtimes_by_tag[tag] = fmax(runtimes_by_tag[tag], time);
  // } else {
  //   runtimes_by_tag[tag] = time;
  // }
}

void performance_monitor::nextGroup(){
#pragma omp barrier
#pragma omp single
  {
    likwid_markerNextGroup();
  }
}

void performance_monitor::close(){
  likwid_markerClose();

  load_likwid_data();
  calculate_port_usage_ratios();
  std::sort(per_thread_results.begin(), per_thread_results.end());

  perform_result_aggregation();
  // TODO: move calculate_saturation() to BEFORE perform_result_aggregation
  // once we get per-core saturation workding
  calculate_saturation(); 
  std::sort(aggregate_results.begin(), aggregate_results.end());
}

std::string performance_monitor::aggregationTypeToString(
  const performance_monitor::aggregation_t &aggregation_type)
{
  if (aggregation_type == performance_monitor::aggregation_t::sum)
    return "sum";
  else if (aggregation_type == 
    performance_monitor::aggregation_t::arithmetic_mean)
    return "arithmetic_mean";
  else if (aggregation_type == 
    performance_monitor::aggregation_t::geometric_mean)
    return "geometric_mean";
  else
    return "unknown_aggregation_type";
}

std::string performance_monitor::resultTypeToString(
  const performance_monitor::result_t &result_type)
{
  if (result_type == performance_monitor::result_t::event)
    return "event";
  else if (result_type == performance_monitor::result_t::metric)
    return "metric";
  else
    return "unknown_result_type";
}

std::string performance_monitor::PerThreadResult::toString(
  std::string delim) const 
{
  std::string result_t_string = resultTypeToString(this->result_type);

  std::stringstream ss;
  ss << std::left << "region " << this->region_name << delim
    << "thread " << std::setw(3) << this->thread_num << delim
    // << "group " << std::setw(15) << this->group_name << delim
    // << std::setw(6) << result_t_string << delim
    << std::setw(40) << this->result_name << delim
    << std::setprecision(4) << std::fixed << std::right << std::setw(20)
    << this->result_value
    << std::endl;
  return ss.str();
}

std::string performance_monitor::AggregateResult::toString(
  std::string delim) const
{
  std::string result_t_string = resultTypeToString(this->result_type);
  std::string aggregation_t_string = aggregationTypeToString(
    this->aggregation_type);

  std::stringstream ss;
  ss << std::left << "region " << this->region_name << delim
    // << "group " << std::setw(15) << this->group_name << delim
    // << std::setw(6) << result_t_string << delim
    << "aggregation_type " << std::setw(15) << aggregation_t_string << delim
    << std::setw(40) << this->result_name << delim
    << std::setprecision(4) << std::fixed << std::right << std::setw(20)
    << this->result_value
    << std::endl;
  return ss.str();
}

void performance_monitor::validate_and_store_likwid_result(
  int thread_num,
  performance_monitor::result_t result_type,
  const char * region_name, 
  const char * group_name,
  const char * result_name, 
  double result_value)
{
  bool keep_result = true;

  if(isnan(result_value)){
    std::cout << "ERROR: likwid returned a NAN result value, which MAY "
      << "indicate that something went wrong." << std::endl;

    keep_result = false;
  }
  else if(result_value < 0){
    std::cout << "ERROR: likwid returned a negative result value, indicating "
      << "that something went wrong." << std::endl;

    keep_result = false;
  }
  else if(!std::getenv(perfmon_keep_large_values_envvar)){

    if(result_type == performance_monitor::result_t::event &&
      result_value >= EVENT_VALUE_ERROR_THRESHOLD)
    {
      std::cout << "WARNING: unreasonably high event value detected:"
        << std::endl;

      std::cout << std::endl
        << "This event will be discarded. We will try to detect all "
        << "metrics associated with this event, but do not guarantee to "
        << "catch all of them." << std::endl
        << std::endl
        << "To disable detection and removal of 'unreasonably' high "
        << "values, set the environment variable '" 
        << perfmon_keep_large_values_envvar << "'." << std::endl
        << std::endl;

      keep_result = false;
    }

    if(result_type == performance_monitor::result_t::metric &&
      result_value >= METRIC_VALUE_ERROR_THRESHOLD)
    {
      std::cout << "WARNING: unreasonably high metric value detected!"
        << std::endl;

      std::cout << std::endl
        << "This metric will be discarded. To disable detection and "
        << "removal of 'unreasonably' high values, set the environment "
        << "variable '" << perfmon_keep_large_values_envvar << "'." 
        << std::endl << std::endl;
      keep_result = false;
    }
  }

  // build result object in memory
  PerThreadResult perThreadResult = {
    .region_name = region_name,
    .thread_num = thread_num,
    .group_name = group_name,
    .result_type = result_type,
    .result_name = result_name,
    .result_value = result_value
  };

  if(keep_result){
    per_thread_results.push_back(perThreadResult);
  }
  else
  {
    std::cout << "The erroneous result is printed below:" << std::endl
      << perThreadResult.toString();
    std::cout << std::endl;
  }
}

void performance_monitor::load_likwid_data(){
  checkInit();

  perfmon_readMarkerFile(likwidOutputFilepath.c_str());

  // populate maps
  for (int t = 0; t < num_threads; t++)
  {
    // this loop is not actually regions, it's regions * groups. This is
    // because perfmon_getNumberOfRegions considers each region + group
    // combination as a "region"
    for (int i = 0; i < perfmon_getNumberOfRegions(); i++)
    {
      const char * regionName = perfmon_getTagOfRegion(i);
      int gid = perfmon_getGroupOfRegion(i);
      const char * groupName = perfmon_getGroupName(gid);

      for (int k = 0; k < perfmon_getEventsOfRegion(i); k++){
        const char * event_name = perfmon_getEventName(gid, k);
        double event_value = perfmon_getResultOfRegionThread(i, k, t);

        validate_and_store_likwid_result(t, 
          performance_monitor::result_t::event, regionName, groupName, 
          event_name, event_value);
      }

      for (int k = 0; k < perfmon_getNumberOfMetrics(gid); k++){
        const char * metric_name = perfmon_getMetricName(gid, k);
        double metric_value = perfmon_getMetricOfRegionThread(i, k, t);

        validate_and_store_likwid_result(t, 
          performance_monitor::result_t::metric, regionName, groupName, 
          metric_name, metric_value);
      }
    }
  }
}

void performance_monitor::perform_result_aggregation()
{
  std::vector<PerThreadResult> ptr_copy(
    performance_monitor::get_per_thread_results());
  
  // a stack is used because we add things in ascending position. Therefore, if
  // we remove in descending position we will not offset anything that comes
  // later 
  while(ptr_copy.size() > 0)
  {
    std::stack<per_thread_results_t::iterator> indices_aggregated;

    // initialize our 3 aggregations to the first thing in the vector
    AggregateResult current_sum = {
      .region_name = ptr_copy.begin()->region_name,
      .group_name = ptr_copy.begin()->group_name,
      .result_type = ptr_copy.begin()->result_type,
      .result_name = ptr_copy.begin()->result_name,
      .aggregation_type = performance_monitor::aggregation_t::sum,
      .result_value = ptr_copy.begin()->result_value,
    };

    // copy the initialized "sum" object into the other two objects we need
    AggregateResult current_arithmetic_mean = current_sum;
    current_arithmetic_mean.aggregation_type = 
      performance_monitor::aggregation_t::arithmetic_mean;

    AggregateResult current_geometric_mean = current_sum;
    current_geometric_mean.aggregation_type = 
      performance_monitor::aggregation_t::geometric_mean;

    indices_aggregated.push(ptr_copy.begin());

    // for everything else, aggregate it if it is a match
    for(auto it = ++(ptr_copy.begin()); it != ptr_copy.end(); ++it)
    {
      if(ptr_copy.begin()->matchesForAggregation(*it))
      {
        current_sum.result_value += it->result_value;
        current_arithmetic_mean.result_value += it->result_value;
        current_geometric_mean.result_value *= it->result_value;
        indices_aggregated.push(it);
      }
    }

    // finally, perform division for arithmetic means and take the power of
    // geometric means

    current_arithmetic_mean.result_value /= performance_monitor::num_threads;
    current_geometric_mean.result_value = 
      pow(
        current_geometric_mean.result_value, 
        1.0 / static_cast<double>(performance_monitor::num_threads)
      );

    // add the things we found to the aggregate results
    performance_monitor::aggregate_results.push_back(current_sum);
    performance_monitor::aggregate_results.push_back(current_arithmetic_mean);
    performance_monitor::aggregate_results.push_back(current_geometric_mean);

    // remove used things from ptr_copy
    while (!indices_aggregated.empty())
    {
      ptr_copy.erase(indices_aggregated.top());
      indices_aggregated.pop();
    }
  }
}

void performance_monitor::calculate_port_usage_ratios()
{
  // TODO: this should check if performance_monitor::num_threads has been 
  // initialized

  // create list of regions
  std::set<std::string> regions;
  for (const auto &ptr : per_thread_results)
    regions.insert(ptr.region_name);

  // build list of port usage event/metric names
  std::vector<std::string> port_usage_event_names;
  std::vector<std::string> port_usage_ratio_metric_names;
  for (unsigned i = 0; i < NUM_PORTS_IN_CORE; i++)
  {
    port_usage_event_names.push_back(uops_port_base_name + std::to_string(i));
    port_usage_ratio_metric_names.push_back(fhv_port_usage_ratio_start + 
      std::to_string(i) + fhv_port_usage_ratio_end);
  }

  // instead of using this vector, we could iterate through per_thread_results
  // again. The vector makes things easy, though
  std::vector<double> uops_executed_port(NUM_PORTS_IN_CORE);
  double total_num_port_ops = 0;

  // first, sum all UOPS_DISPATCHED_PORT_PORT* on a per-thread, per-region
  // basis
  for (int t = 0; t < performance_monitor::num_threads; t++)
  {
    for (const auto &region_name : regions)
    {
      for (size_t port_num = 0; port_num < NUM_PORTS_IN_CORE; port_num++)
      {
        for (const PerThreadResult &ptr : performance_monitor::per_thread_results)
        {
          if(ptr.thread_num == t 
            && ptr.region_name == region_name
            && ptr.result_name == port_usage_event_names[port_num])
          {
            // sum 
            total_num_port_ops += ptr.result_value;
            uops_executed_port[port_num] = ptr.result_value;
          }
        }
      }

      // next, find ratios and create metrics for them
      for (size_t port_num = 0; port_num < NUM_PORTS_IN_CORE; port_num++)
      {
        PerThreadResult port_usage_metric = {
          .region_name = region_name,
          .thread_num = t,
          .group_name = fhv_performance_monitor_group,
          .result_type = performance_monitor::result_t::metric,
          .result_name = port_usage_ratio_metric_names[port_num],
          .result_value = uops_executed_port[port_num] / total_num_port_ops
        };

        performance_monitor::per_thread_results.push_back(port_usage_metric);
      }
    }
  }
}

void performance_monitor::calculate_saturation(){
  // TODO: eventually I want to build this on a per-thread basis and then have
  // "perform_result_aggregation" automatically aggregate these. however, for
  // now we're just going to build overall results manually and add them to
  // "aggregate results" under "geometric mean"

  bool is_saturation_result;
  std::string saturation_result_name;
  double saturation_result_value;

  for (const auto & ar : performance_monitor::aggregate_results)
  {
    if(ar.aggregation_type == performance_monitor::aggregation_t::sum)
    {
      is_saturation_result = false;

      if (ar.result_name == mflops_metric_name)
      {
        is_saturation_result = true;
        saturation_result_name = flops_sp_saturation_metric_name;
        saturation_result_value = ar.result_value/EXPERIENTIAL_SP_RATE_MFLOPS;
      }
      else if (ar.result_name == mflops_dp_metric_name)
      {
        is_saturation_result = true;
        saturation_result_name = flops_dp_saturation_metric_name;
        saturation_result_value = ar.result_value/EXPERIENTIAL_DP_RATE_MFLOPS;
      }
      else if (ar.result_name == l2_bandwidth_metric_name)
      {
        is_saturation_result = true;
        saturation_result_name = l2_saturation_metric_name;
        saturation_result_value = ar.result_value/EXPERIENTIAL_RW_BW_L2;
      }
      else if (ar.result_name == l3_bandwidth_metric_name)
      {
        is_saturation_result = true;
        saturation_result_name = l3_saturation_metric_name;
        saturation_result_value = ar.result_value/EXPERIENTIAL_RW_BW_L3;
      }
      else if (ar.result_name == ram_bandwidth_metric_name)
      {
        is_saturation_result = true;
        saturation_result_name = mem_saturation_metric_name;
        saturation_result_value = ar.result_value/EXPERIENTIAL_RW_BW_RAM;
      }

      if (is_saturation_result)
      {
        AggregateResult new_ar = {
          .region_name = ar.region_name,
          .group_name = fhv_performance_monitor_group,
          .result_type = performance_monitor::result_t::metric,
          .result_name = saturation_result_name,
          .aggregation_type = performance_monitor::aggregation_t::geometric_mean,
          .result_value = saturation_result_value
        };
        
        performance_monitor::aggregate_results.push_back(new_ar);
      }
    }
  }
}

void performance_monitor::checkInit()
{
  std::string error_str = "";
  bool something_went_wrong = false;

  if (performance_monitor::key_metrics.size() == 0)
  {
    error_str += "WARNING: performance_monitor::key_metrics is empty!"
      "printHighlights and\n"
      "resultsToJson will not work. This usually happens because the user "
      "failed to\n"
      "call performance_monitor::init(). If init() was called, then something "
      "is wrong\n"
      "with init() internally.\n";
    
    something_went_wrong = true;
  }
  if (performance_monitor::num_threads == -1)
  {
    error_str += "WARNING: performance_monitor::num_threads was not set! This "
      "usually happens\n"
      "because the user failed to call performance_monitor::init(). FHV will "
      "not be\n"
      "able to build results. Major functionality will be disabled.\n"
      "If init() was called, then something is wrong with init() "
      "internally.\n";
    
    something_went_wrong = true;
  }

  if(something_went_wrong) std::cout << error_str;
}

void performance_monitor::checkResults(){
  std::string error_str = "WARNING: there doesn't seem to be any results for "
    "likwid. This commonly \n"
    "happens because performance_monitor::close() was not called.\n";
  
  std::string error_more_info = "";
  bool something_went_wrong = false;

  if (performance_monitor::per_thread_results.size() == 0)
  {
    error_more_info += "If close() was called, then something is wrong "
      "internally. Did close() call \n"
      "load_likwid_data()? Is something wrong with load_likwid_data()?\n";
    something_went_wrong = true;
  }

  if (performance_monitor::aggregate_results.size() == 0)
  {
    error_more_info += "If close() was called, then something is wrong "
      "internally. Did close() call \n"
      "perform_result_aggregation()? Is something wrong with "
      "perform_result_aggregation()?\n";
    something_went_wrong = true;
  }

  if (something_went_wrong) 
  {
    std::cout << error_str;
    std::cout << error_more_info;
  }
}

void performance_monitor::printDetailedResults(){
  std::cout << std::endl
    << "----- FHV Performance Monitor: detailed results ----- "
    << std::endl;

  checkResults();

  for (const PerThreadResult &ptr : performance_monitor::per_thread_results)
  {
    std::cout << ptr.toString();
  }
}

void performance_monitor::printAggregateResults(){
  std::cout << std::endl
    << "----- FHV Performance Monitor: aggregate results ----- "
    << std::endl;

  checkResults();

  for (const AggregateResult &ar : performance_monitor::aggregate_results)
  {
    std::cout << ar.toString();
  }
}

// void performance_monitor::buildResultsMaps()
// {
//   // int gid, num_threads;
//   int gid;
//   float event_value = -1, metric_value = -1;
//   const char *event_name, *metric_name, * groupName, * regionName;
//   bool keep_event, keep_metric;

//   std::string this_port_event_name;
//   std::string this_port_usage_metric_name;

// #pragma omp parallel
//   {
//     // needed because we use it to print results later
//     performance_monitor::num_threads = omp_get_num_threads();
//   }

//   perfmon_readMarkerFile(likwidOutputFilepath.c_str());

//   // initialize everything to 0

//   // there is a region for every region/group combo, so we only need one outer
//   // for loop
//   for (int i = 0; i < perfmon_getNumberOfRegions(); i++)
//   {
//     regionName = perfmon_getTagOfRegion(i);
//     gid = perfmon_getGroupOfRegion(i);
//     groupName = perfmon_getGroupName(gid);
//     aggregate_results[sum][event][regionName][groupName]
//       [total_sp_flops_event_name] = 0.;

//     for (int k = 0; k < perfmon_getEventsOfRegion(i); k++){
//       event_name = perfmon_getEventName(gid, k);
//       aggregate_results[sum][event][regionName][groupName][event_name] = 0.;
//       aggregate_results[arithmetic_mean][event][regionName][groupName][event_name] = 0.;
//       aggregate_results[geometric_mean][event][regionName][groupName][event_name] = 1.;
//     }
//     for (int k = 0; k < perfmon_getMetricsOfRegion(i); k++){
//       metric_name = perfmon_getMetricName(gid, k);
//       aggregate_results[sum][metric][regionName][groupName][metric_name] = 0.;
//       aggregate_results[arithmetic_mean][metric][regionName][groupName][metric_name] = 0.;
//       aggregate_results[geometric_mean][metric][regionName][groupName][metric_name] = 1.;
//       aggregate_results[geometric_mean][metric][all_regions_keyword][groupName][metric_name] = 1.;
//     }
//     for (unsigned port_num = 0; port_num < NUM_PORTS_IN_CORE; port_num++){
//       groupName = fhv_port_usage_group;
//       this_port_usage_metric_name = fhv_port_usage_ratio_start 
//         + std::to_string(port_num) + fhv_port_usage_ratio_end;
//       aggregate_results[sum][metric][regionName][groupName][this_port_usage_metric_name] = 0.;
//       aggregate_results[arithmetic_mean][metric][regionName][groupName][this_port_usage_metric_name] = 0.;
//       aggregate_results[geometric_mean][metric][regionName][groupName][this_port_usage_metric_name] = 1.;
//     }
//   }

//   // populate maps
//   for (int t = 0; t < num_threads; t++)
//   {
//     // this loop is not actually regions, it's regions * groups. This is
//     // because perfmon_getNumberOfRegions considers each region + group
//     // combination as a "region"
//     for (int i = 0; i < perfmon_getNumberOfRegions(); i++)
//     {
//       regionName = perfmon_getTagOfRegion(i);
//       gid = perfmon_getGroupOfRegion(i);
//       groupName = perfmon_getGroupName(gid);

//       for (int k = 0; k < perfmon_getEventsOfRegion(i); k++){
//         // "getCounterName" gives name like "PMC0"

//         // event_name = perfmon_getCounterName(gid, k);
//         event_name = perfmon_getEventName(gid, k);
//         event_value = perfmon_getResultOfRegionThread(i, k, t);
//         keep_event = true;


//         if(event_value >= EVENT_VALUE_ERROR_THRESHOLD){
//           if(!std::getenv(perfmon_keep_large_values_envvar)){
//             std::cout << "WARNING: unreasonably high event value detected:"
//               << std::endl
//               << "Thread: '" << t << "'" << std::endl
//               << "Region: '" << regionName << "'" << std::endl
//               << "Group:  '" << groupName << "'" << std::endl
//               << "Event:  '" << event_name << "'" << std::endl
//               << "Value:  '" << event_value << "'" << std::endl
//               << std::endl
//               << "This event will be discarded. We will try to detect all "
//               << "metrics associated with this event, but do not guarantee to "
//               << "catch all of them." << std::endl
//               << std::endl
//               << "To disable detection and removal of 'unreasonably' high "
//               << "values, set the environment variable '" 
//               << perfmon_keep_large_values_envvar << "'." << std::endl
//               << std::endl;
//             keep_event = false;
//           }
//         }
//         if(event_value > 0 && keep_event){
//           aggregate_results[sum][event][regionName][groupName][event_name] 
//             += event_value;
//           aggregate_results[geometric_mean][event][regionName][groupName]
//             [event_name] *= event_value;
//           per_thread_results[event][t][regionName][groupName][event_name]
//             = event_value;

//           if(strcmp(sp_scalar_flops_event_name, event_name) == 0){
//             aggregate_results[sum][event][regionName][groupName]
//               [total_sp_flops_event_name] += event_value;
//           }
//           else if(strcmp(sp_avx_128_flops_event_name, event_name) == 0){
//             aggregate_results[sum][event][regionName][groupName]
//               [total_sp_flops_event_name]
//               += event_value * OPS_PER_SP_128_VECTOR;
//           }
//           else if(strcmp(sp_avx_256_flops_event_name, event_name) == 0){
//             aggregate_results[sum][event][regionName][groupName]
//               [total_sp_flops_event_name]
//               += event_value * OPS_PER_SP_256_VECTOR;
//           }
//         }
//       }

//       for (int k = 0; k < perfmon_getNumberOfMetrics(gid); k++){
//         metric_name = perfmon_getMetricName(gid, k);
//         metric_value = perfmon_getMetricOfRegionThread(i, k, t);
//         keep_metric = true;

//         if(metric_value >= METRIC_VALUE_ERROR_THRESHOLD){
//           if(!std::getenv(perfmon_keep_large_values_envvar)){
//             std::cout << "WARNING: unreasonably high metric value detected:"
//               << std::endl
//               << "Thread: '" << t << "'" << std::endl
//               << "Region: '" << regionName << "'" << std::endl
//               << "Group:  '" << groupName << "'" << std::endl
//               << "Metric: '" << metric_name << "'" << std::endl
//               << "Value:  '" << metric_value << "'" << std::endl
//               << std::endl
//               << "This metric will be discarded. To disable detection and "
//               << "removal of 'unreasonably' high values, set the environment "
//               << "variable '" << perfmon_keep_large_values_envvar << "'." 
//               << std::endl << std::endl;
//             keep_metric = false;
//           }
//         }
//         if(!isnan(metric_value) && keep_metric){
//           aggregate_results[sum][metric][regionName][groupName][metric_name]
//             += metric_value;
//           aggregate_results[geometric_mean][metric][regionName][groupName]
//             [metric_name] *= metric_value;

//           aggregate_results[geometric_mean][metric][all_regions_keyword]
//             [groupName][metric_name] *= metric_value;

//           per_thread_results[metric][t][regionName][groupName][metric_name]
//             = metric_value;
//         }
//       }
//     }
//   }

//   // sum port usage on a per-thread basis and find port usage ratios

//   double total_port_usage, port_usage_value;

//   std::vector<double> port_usages(NUM_PORTS_IN_CORE);

//   for (int t = 0; t < num_threads; t++){

//     for (auto region = per_thread_results[event][t].begin();
//     region != per_thread_results[event][t].end();
//     ++region)
//     {
//       total_port_usage = 0;
//       port_usages.clear();

//       for (auto group = region->second.begin(); 
//         group != region->second.end(); 
//         ++group)
//       {
//         for (auto event = group->second.begin();
//           event != group->second.end();
//           ++event)
//         {
//           for(unsigned port_num = 0; port_num < NUM_PORTS_IN_CORE; port_num++){
//             this_port_event_name = 
//               uops_port_base_name + std::to_string(port_num);
//             if (event->first.compare(this_port_event_name) == 0){
//               total_port_usage += event->second;
//               port_usages[port_num] = event->second;
//             }
//           }
//         }
//       }

//       for(unsigned port_num = 0; port_num < NUM_PORTS_IN_CORE; port_num++){
//         groupName = fhv_port_usage_group;
//         this_port_usage_metric_name = fhv_port_usage_ratio_start 
//           + std::to_string(port_num) + fhv_port_usage_ratio_end;
//         port_usage_value = port_usages[port_num]/total_port_usage;

//         per_thread_results[metric][t][region->first][groupName][this_port_usage_metric_name] = port_usage_value;
//         aggregate_results[sum][metric][region->first][groupName][this_port_usage_metric_name] += port_usage_value;
//         if(port_usage_value != 0.0){
//           aggregate_results[geometric_mean][metric][region->first][groupName][this_port_usage_metric_name] *= port_usage_value;
//         }
//       }
//     }
//   }

//   // convert sums/products to averages where appropriate

//   for (auto region = aggregate_results[geometric_mean][performance_monitor::result_t::event].begin();
//     region != aggregate_results[geometric_mean][performance_monitor::result_t::event].end();
//     ++region)
//   {
//     for (auto group = region->second.begin(); 
//       group != region->second.end(); 
//       ++group)
//     {
//       for (auto event = group->second.begin();
//         event != group->second.end();
//         ++event)
//       {
//         aggregate_results[arithmetic_mean][performance_monitor::result_t::event][region->first][group->first][event->first] =
//             aggregate_results[sum][performance_monitor::result_t::event][region->first][group->first][event->first] / num_threads;

//         aggregate_results[geometric_mean][performance_monitor::result_t::event][region->first][group->first]
//           [event->first] = 
//           pow(
//             aggregate_results[geometric_mean][performance_monitor::result_t::event][region->first][group->first]
//               [event->first],
//             1.0/static_cast<double>(num_threads));
//       }
//     }
//   }

//   for (auto region = aggregate_results[geometric_mean][performance_monitor::result_t::metric].begin();
//     region != aggregate_results[geometric_mean][performance_monitor::result_t::metric].end();
//     ++region)
//   {
//     for (auto group = region->second.begin(); 
//       group != region->second.end(); 
//       ++group)
//     {
//       for (auto metric = group->second.begin();
//         metric != group->second.end();
//         ++metric)
//       {
//         aggregate_results[arithmetic_mean][performance_monitor::result_t::metric][region->first][group->first][metric->first] =
//             aggregate_results[sum][performance_monitor::result_t::metric][region->first][group->first][metric->first] / num_threads;

//         aggregate_results[geometric_mean][performance_monitor::result_t::metric][region->first][group->first]
//           [metric->first] = 
//           pow(
//             aggregate_results[geometric_mean][performance_monitor::result_t::metric][region->first][group->first]
//               [metric->first],
//             1.0/static_cast<double>(num_threads));
//       }
//     }
//   }

//   int num_regions = perfmon_getNumberOfRegions()/perfmon_getNumberOfGroups();

//   for (int i = 0; i < perfmon_getNumberOfGroups(); i++)
//   {
//     gid = i;
//     groupName = perfmon_getGroupName(gid);

//     for (int k = 0; k < perfmon_getNumberOfMetrics(gid); k++)
//     {
//       metric_name = perfmon_getMetricName(gid, k);
//       aggregate_results[geometric_mean][metric][all_regions_keyword][groupName]
//         [metric_name] = 
//         pow(
//           aggregate_results[geometric_mean][metric][all_regions_keyword][groupName]
//             [metric_name],
//           1.0/static_cast<double>(num_regions * num_threads));
//     }
//   }
// }

// void performance_monitor::compareActualWithBench()
// {
//   if(aggregate_results.size() == 0){
//     std::cout << "ERROR: you must run performance_monitor::buildResultsMaps"
//               << " before comparing\n"
//               << "actual with bench. If you are getting this error and you did"
//               << " run buildResultsMaps,\n"
//               << "No metrics have been aggregated. Pleases submit a bug report"
//               << "\n";
//     return;
//   }

//   const char * regionName;
//   const char * metricGroupName;
//   const char * metricName;
//   float referenceBenchmarkValue;

//   // getNumberOfRegions actually returns num_regions * num_groups
//   int num_regions = perfmon_getNumberOfRegions()/perfmon_getNumberOfGroups();

//   // initialize all_regions part to 0
//   for (size_t j = 0; j < saturation_metrics.size(); j++){
//     metricName = saturation_metrics[j];
//     saturation[all_regions_keyword][metricName] = 1.;
//   }

//   for (int i = 0; i < num_regions; i++)
//   {
//     regionName = perfmon_getTagOfRegion(i);

//     for (size_t j = 0; j < saturation_metrics.size(); j++){
//       metricGroupName = saturationMetricGroups[j];
//       metricName = saturation_metrics[j];
//       referenceBenchmarkValue = saturationBenchmarkReferences[j];

//       if(!aggregate_results[sum][metric][regionName].count(metricGroupName))
//       {
//         std::cout << "WARN: group " << metricGroupName << " was not measured."
//                   << " Therefore, saturation for " << metricName << " will \n"
//                   << "not be calculated and will be reported as NaN.\n";

//         saturation[regionName][metricName] = NAN;
//         saturation[all_regions_keyword][metricName] = NAN;
//       }
//       else
//       {
//         saturation[regionName][metricName] =
//           aggregate_results[sum][metric][regionName][metricGroupName][metricName] 
//           / referenceBenchmarkValue;
//         saturation[all_regions_keyword][metricName] *=
//           saturation[regionName][metricName];
//       }
//     }
//   }

//   for (size_t i = 0; i < saturation_metrics.size(); i++)
//   {
//     metricName = saturation_metrics[i];

//     // it doesn't make sense to sum saturation values, so we find the average 
//     // of saturations to get saturation across regions. This average is a
//     // geometric mean, which is the standard for ratios
//     saturation[all_regions_keyword][metricName] = 
//       pow(saturation[all_regions_keyword][metricName],
//         1/static_cast<double>(num_regions));
//   }
// }

// void performance_monitor::printResults()
// {
//   printDetailedResults();
//   printOnlyAggregate();
//   printComparison();
// }

// void performance_monitor::printDetailedResults()
// {
//   if(per_thread_results.size() == 0){
//     std::cout << "ERROR: you must run performance_monitor::buildResultsMaps"
//               << " before printing\n"
//               << "detailed, per-thread results\n";
//     return;
//   }

//   std::string performance_monitor::result_t_string;
//   std::string delim = " : ";

//   printf("\n\n ----- Detailed performance_monitor report ----- \n\n");

//   for (auto it1 = per_thread_results.begin(); it1 != per_thread_results.end(); ++it1)
//   {
//     if (it1->first == performance_monitor::result_t::event)
//       performance_monitor::result_t_string = "event";
//     else if (it1->first == performance_monitor::result_t::metric)
//       performance_monitor::result_t_string = "metric";
//     for (auto it2 = it1->second.begin(); it2 != it1->second.end(); ++it2)
//     {
//       for (auto it3 = it2->second.begin(); it3 != it2->second.end(); ++it3)
//       {
//         for (auto it4 = it3->second.begin(); it4 != it3->second.end(); ++it4)
//         {
//           for (auto it5 = it4->second.begin(); it5 != it4->second.end(); ++it5)
//           {
//             std::cout << performance_monitor::result_t_string << delim
//                       << "thread " << it2->first << delim
//                       << "region " << it3->first << delim
//                       << "group " << it4->first << delim
//                       << it5->first << delim
//                       << it5->second 
//                       << '\n';
//           }
//         }
//       }
//     }
//     std::cout << std::endl;
//   }
//   std::cout << std::endl;
  
//   printf("\n\n ----- end detailed performance_monitor report ----- \n\n");
// }

// void performance_monitor::printOnlyAggregate()
// {
//   // unnecessary, compareActualWithBench() calls this and it is called in close
//   // buildResultsMaps();

//   std::cout << "\n\n----- aggregate performance_monitor report -----\n\n";
//   if(aggregate_results.size() == 0){
//     std::cout << "ERROR: you must run performance_monitor::buildResultsMaps"
//               << " before printing\n"
//               << "aggregate results\n";
//     return;
//   }

//   // TODO: fix runtimes (enforce calling performance_monitor::stopRegion
//   // instead of likwid_markerStopRegion?)

//   // std::cout << " --- runtimes by tag: \n";
//   // for (std::map<std::string, double>::iterator it=runtimes_by_tag.begin();
//   //      it!=runtimes_by_tag.end(); ++it){
//   //   std::cout << "Runtime for " + it->first + ": "
//   //              + std::to_string(it->second) + "\n";
//   // }
//   // std::cout << std::endl;

//   std::string performance_monitor::aggregation_t_string;
//   std::string performance_monitor::result_t_string;

//   std::cout << "aggregation type" << " : " 
//             << "result type" <<      " : "
//             << "region name" << " : " 
//             << "group name" << " : " 
//             << "metric name" << " : "
//             << "metric value" << '\n';
//   for (auto it1 = aggregate_results.begin(); it1 != aggregate_results.end(); ++it1)
//   {
//     if (it1->first == performance_monitor::aggregation_t::sum)
//       performance_monitor::aggregation_t_string = "sum";
//     else if (it1->first == performance_monitor::aggregation_t::arithmetic_mean)
//       performance_monitor::aggregation_t_string = "arithmetic mean";
//     else if (it1->first == performance_monitor::aggregation_t::geometric_mean)
//       performance_monitor::aggregation_t_string = "geometric mean";

//     for (auto it2 = it1->second.begin(); it2 != it1->second.end(); ++it2)
//     {
//       if (it2->first == performance_monitor::result_t::event)
//         performance_monitor::result_t_string = "event";
//       else if (it2->first == performance_monitor::result_t::metric)
//         performance_monitor::result_t_string = "metric";

//       for (auto it3 = it2->second.begin(); it3 != it2->second.end(); ++it3)
//       {
//         for (auto it4 = it3->second.begin(); it4 != it3->second.end(); ++it4)
//         {
//           for (auto it5 = it4->second.begin(); it5 != it4->second.end(); ++it5)
//           {
//             std::cout << performance_monitor::aggregation_t_string << " : " 
//                       << performance_monitor::result_t_string <<      " : "
//                       << "region " << it3->first << " : " 
//                       << "group " << it4->first <<  " : " 
//                       << it5->first <<              " : "
//                       << it5->second << '\n';
//           }
//         }
//       }
//     }
//     std::cout << std::endl;
//   }
//   std::cout << std::endl;

//   std::cout << " ----- end aggregate performance_monitor report -----\n";
//   std::cout << std::endl;
// }

// void performance_monitor::printComparison(){
//   if(saturation.size() == 0){
//     std::cout << "ERROR: you must run "
//               << "performance_monitor::compareActualWithBench before\n"
//               << "printing comparison.\n";
//     return;
//   }

//   double metric_value;

//   std::cout << 
//     "\n\n ----- saturation level performance_monitor report -----\n\n";
//   for (auto it=saturation.begin(); it!=saturation.end(); ++it)
//   {
//     std::cout << "\nRegion " << it->first << ":\n";
//     for (auto it2=it->second.begin(); it2!=it->second.end(); ++it2)
//     {
//       metric_value = it2->second;
//       std::cout << std::setw(60) 
//         << "Percentage of available " + it2->first + " used: "
//         << std::setw(7)
//         << std::right
//         << std::setprecision(3)
//         << std::fixed
//         << metric_value << '\n';
//     }
//   }
//   std::cout << std::endl;
//   std::cout << "----- end saturation level performance_monitor report -----\n";
//   std::cout << std::endl;
// }

void performance_monitor::printHighlights(){
  checkInit();
  checkResults();
  
  std::cout << std::endl 
    << "----- performance_monitor highlights report -----" 
    << std::endl;

  std::cout << "----- key metrics, per-thread -----" << std::endl;
  for(const auto & ptr : performance_monitor::per_thread_results)
  {
    for(const auto & key_metric : performance_monitor::key_metrics)
    {
      if (ptr.result_name == key_metric) std::cout << ptr.toString();
    }
  }

  std::cout << "----- key metrics, aggregated across threads -----"
    << std::endl;
  for(const auto & ar : performance_monitor::aggregate_results)
  {
    for(const auto & key_metric : performance_monitor::key_metrics)
    {
      if (ar.result_name == key_metric) std::cout << ar.toString();
    }
  }
}

//   // for each region
//   for (auto region_it = aggregate_results[sum][metric].begin();
//        region_it != aggregate_results[sum][metric].end();
//        ++region_it)
//   {
//     std::cout << "\nREGION " << region_it->first << "\n";

//     // for each key metric
//     for (
//         auto key_metric_it = sum_metrics.begin();
//         key_metric_it != sum_metrics.end();
//         ++key_metric_it)
//     {

//       // for each group
//       for (auto group_it = region_it->second.begin();
//            group_it != region_it->second.end();
//            ++group_it)
//       {
//         // if this group contains the current key metric:
//         if (group_it->second.count(*key_metric_it))
//         {
//           metric_value = group_it->second[*key_metric_it];
//           // rounded_metric_value = dround(metric_value, 3);

//           // TODO:
//           // these should be rounded and presented in scientific notation
//           std::cout << std::right << std::setw(40) << *key_metric_it;
//           std::cout << " " 
//                     << std::setw(20)
//                     << std::right
//                     << std::scientific
//                     << std::setprecision(3)
//                     << metric_value;

//           std::cout << "\n";
//         }
//       }
//     }
//   }
//   std::cout << "\n";

//   std::cout << " ---- key metrics, per-thread ----\n";

//   // int num_threads;
//   // #pragma omp parallel
//   // {
//   //   num_threads = omp_get_num_threads();
//   // }

//   std::stringstream delayed_errors;
  
//   // "thread #" is of length 8, so 8 is the smallest width possible while
//   // maintaining pretty formatting
//   unsigned value_print_width = 8;

//   // for each region
//   for (auto const& region: per_thread_results[metric][0])
//   {
//     std::cout << "\nREGION " << region.first << "\n";

//     std::cout << std::right << std::setw(40) << "Metric" << "    ";
//     for (int t = 0; t < num_threads; t++)
//     {
//       std::cout << " " 
//                 << std::setw(value_print_width)
//                 << std::left 
//                 << "Thread " + std::to_string(t);
//     }
//     std::cout << "\n";

//     // for each key metric
//     for (auto const& key_metric: per_core_metrics)
//     {

//       // for each group
//       for (auto const& group: region.second)
//       {

//         // if this group contains the current key metric:
//         if (group.second.count(key_metric))
//         {
//           std::cout << std::setw(40) << std::right << key_metric << "    ";

//           for (int t = 0; t < num_threads; t++)
//           {
//             try {
//               metric_value = per_thread_results.at(metric)
//                 .at(t)
//                 .at(region.first)
//                 .at(group.first)
//                 .at(key_metric);
//             }
//             catch (std::out_of_range& e){
//               delayed_errors << "WARN: unable to access "
//                 << "per_thread_results[metric]" 
//                 << "[" << t << "]"
//                 << "[" << region.first << "]"
//                 << "[" << group.first << "]"
//                 << "[" << key_metric << "]" << std::endl;
//               metric_value = NAN;
//             }

//             std::cout << " "
//                       << std::setprecision(3) 
//                       << std::fixed
//                       << std::setw(value_print_width)
//                       << std::left
//                       << metric_value;
//           }
//           std::cout << "\n";
//         }
//       }
//     }
//   }
//   std::cout << delayed_errors.str();

//   std::cout << "\n";

//   std::cout << " ---- key metrics, averaged across threads ----\n";
//   std::cout << " ---- using a geometric mean ----\n";

//   auto geometric_mean_metrics = per_core_metrics;

//   // for each region
//   for (auto const& region: aggregate_results[geometric_mean][metric] )
//   {
//     std::cout << "\nREGION " << region.first << "\n";

//     // for each key metric
//     for (auto const& key_metric: geometric_mean_metrics )
//     {

//       // for each group
//       for (auto const& group : region.second)
//       {
//         // if this group contains the current key metric:
//         if (group.second.count(key_metric))
//         {
//           metric_value = group.second.at(key_metric); 

//           std::cout << std::setw(40) << std::right << key_metric;
//           std::cout << " " 
//                     << std::setw(7)
//                     << std::right
//                     << std::fixed
//                     << std::setprecision(3)
//                     << metric_value;
//           std::cout << "\n";
//         }
//       }
//     }
//   }

//   std::cout << "\n";

//   printComparison();

//   std::cout << "\n ----- end performance_monitor highlights report -----\n\n";
// }

// void performance_monitor::printCsvHeader(){
//   std::stringstream header_ss;
//   std::string header;
//   std::string delim = ",";
//   std::string line_end = "\n";

//   #pragma omp parallel
//   {
//     num_threads = omp_get_num_threads();
//   }

//   // for each region
//   header_ss << "region" << delim;

//   // for each thing in saturation
//   for (auto const& saturation_metric: saturation_metrics){
//     header_ss << "saturation " << saturation_metric << delim;
//   }

//   // for each thread
//   for (int t = 0; t < num_threads; t++){

//     // for each key metric
//     for (auto const& port_usage_metric: port_usage_metrics)
//     {
//       header_ss << "thread " << t << " " << port_usage_metric << delim;
//     }

//   }

//   header = header_ss.str();
//   header.pop_back();
//   header += line_end;
  
//   std::cout << header;
// }

// void performance_monitor::printCsvOutput(){
//   if(aggregate_results.size() == 0){
//     std::cout << "ERROR: you must run performance_monitor::buildResultsMaps"
//               << " before outputting\n"
//               << "to csv\n";
//     return;
//   }

//   if(saturation.size() == 0){
//     std::cout << "ERROR: you must run "
//               << "performance_monitor::compareActualWithBench before\n"
//               << "outputting to csv.\n";
//     return;
//   }

//   double metric_value;

//   std::string result_csv_line = "";
//   std::string delim = ",";
//   std::string line_end = "\n";

//   // for each region
//   for (auto const& region: aggregate_results[geometric_mean][metric] )
//   {
//     std::stringstream result_csv_line_ss;
//     result_csv_line_ss << region.first << delim;

//     // for each saturation metric (using names to maintain order with header)
//     for (auto const& saturation_metric: saturation_metrics){
//       result_csv_line_ss << saturation.at(region.first).at(saturation_metric) 
//         << delim;
//     }

//     // for each thread
//     for (int t = 0; t < num_threads; t++)
//     {

//       // for each key metric
//       for (auto const &port_usage_metric : port_usage_metrics)
//       {

//         // for each group
//         for (auto const &group : region.second)
//         {
//           // if this group contains the current key metric:
//           if (group.second.count(port_usage_metric))
//           {
//             metric_value = group.second.at(port_usage_metric);

//             result_csv_line_ss << metric_value << delim;
//           }

//           // results["port_usage"][region.first][port_usage_metric]
//           //   = metric_value;
//         }
//       }
//     }

//     result_csv_line += result_csv_line_ss.str();
//     result_csv_line.pop_back();
//     result_csv_line += line_end;
  
//   }

//   std::cout << result_csv_line;
// }


void performance_monitor::resultsToJson(std::string param_info_string)
{
  checkInit();
  checkResults();
  
  json results;
  
  // set parameter info string
  results[json_info_section][json_parameter_key] = param_info_string;

  // populate json with per_thread_results
  for (const auto & ptr : performance_monitor::per_thread_results)
  {
    for (const auto &key_metric : performance_monitor::key_metrics)
    {
      if(ptr.result_name == key_metric)
      {
        results[json_results_section][ptr.region_name]
          [json_thread_section_base + std::to_string(ptr.thread_num)]
          [ptr.result_name] = ptr.result_value;
      }
    }
  }

  // populate json with aggregate results
  for (const auto & ar : performance_monitor::aggregate_results)
  {
    for (const auto &key_metric : performance_monitor::key_metrics)
    {
      if(ar.result_name == key_metric)
      {
        results[json_results_section][ar.region_name]
          [aggregationTypeToString(ar.aggregation_type)]
          [ar.result_name] = ar.result_value;
      }
    }
  }

  // write json to disk
  std::string output_filename = jsonResultOutputFilepath;
  if(const char* env_p = std::getenv(perfmon_output_envvar))
    output_filename = env_p;

  std::ofstream o(output_filename);
  o << std::setw(4) << results << std::endl;
}

// void performance_monitor::resultsToJson(std::string param_info_string)
// {
//   if(aggregate_results.size() == 0){
//     std::cout << "ERROR: you must run performance_monitor::buildResultsMaps"
//               << " before outputting\n"
//               << "to json\n";
//     return;
//   }

//   if(saturation.size() == 0){
//     std::cout << "ERROR: you must run "
//               << "performance_monitor::compareActualWithBench before\n"
//               << "outputting to json.\n";
//     return;
//   }

//   json results;
//   results["info"]["parameters"] = param_info_string;

//   // don't add "all_regions" region

//   // TODO: honestly if we don't need the "all_regions" region then we shouldn't
//   // even build it in the first place. Evaluate if we need it.
//   for (auto const& region: saturation)
//   {
//     if(region.first.compare(all_regions_keyword) != 0)
//     {
//       // if we ARE NOT the all_regions region
//       results["saturation"][region.first] = region.second;
//     }
//   }

//   double metric_value;

//   // for each thread
//   for (int t = 0; t < performance_monitor::num_threads; t++)
//   {

//     // for each region
//     for (auto const& region: per_thread_results[metric][t] )
//     {

//       // for each key metric
//       for (auto const& port_usage_metric: port_usage_metrics)
//       {

//         // for each group
//         for (auto const& group : region.second)
//         {
//           // if this group contains the current key metric:
//           if (group.second.count(port_usage_metric))
//           {
//             metric_value = group.second.at(port_usage_metric);

//             results["port_usage"][region.first]["thread_" + std::to_string(t)]
//               [port_usage_metric] = metric_value;
//           }
//         }
//       }
//     }
//   }

//   // for each region
//   for (auto const& region: aggregate_results[geometric_mean][metric] )
//   {

//     // for each key metric
//     for (auto const& port_usage_metric: port_usage_metrics)
//     {

//       // for each group
//       for (auto const& group : region.second)
//       {
//         // if this group contains the current key metric:
//         if (group.second.count(port_usage_metric))
//         {
//           metric_value = group.second.at(port_usage_metric);

//           results["port_usage"][region.first]["geometric_mean"]
//             [port_usage_metric] = metric_value;
//         }
//       }
//     }
//   }

//   std::string output_filename = jsonResultOutputFilepath;
//   if(const char* env_p = std::getenv(perfmon_output_envvar))
//     output_filename = env_p;

//   std::ofstream o(output_filename);
//   o << std::setw(4) << results << std::endl;
// }

// const std::map<std::string,double> performance_monitor::get_runtimes_by_tag() {
// 	return runtimes_by_tag;
// }


const performance_monitor::aggregate_results_t&
performance_monitor::get_aggregate_results()
{
	return aggregate_results;
}

const performance_monitor::per_thread_results_t&
performance_monitor::get_per_thread_results()
{
  return per_thread_results;
}

// const performance_monitor::saturation_map_t&
// performance_monitor::get_saturation()
// {
// 	return saturation;
// }
