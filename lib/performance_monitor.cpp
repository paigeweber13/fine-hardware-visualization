#include "performance_monitor.h"

// declarations
performance_monitor::aggregate_results_t 
  performance_monitor::aggregate_results;

performance_monitor::per_thread_results_t 
  performance_monitor::per_thread_results;

int performance_monitor::num_threads = -1;

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

// ------ perfmon stuff ------ //

void
performance_monitor::registerRegions(
    const std::string regions)
{
  if (regions == "")
  {
    std::cout << "WARNING: No regions supplied! Doing nothing." << std::endl;
    return;
  }

  std::string delimiter = ",";

  size_t start_pos = 0;
  size_t end_pos = 0;
  std::string token;
  do
  {
    end_pos = regions.find(delimiter, start_pos);
    if (end_pos != std::string::npos)
      token = regions.substr(start_pos, end_pos - start_pos);
    else
      token = regions.substr(start_pos, end_pos);

    // std::cout << "DEBUG: registering region " + token + " on thread " +
    //                  std::to_string(omp_get_thread_num()) + "\n";
    likwid_markerRegisterRegion(token.c_str());
    start_pos = end_pos + delimiter.length();
  } while (end_pos != std::string::npos);
}

void performance_monitor::init(std::string parallel_regions,
  std::string sequential_regions, std::string event_groups)
{
  // initialize num_threads
  #pragma omp parallel
  {
    performance_monitor::num_threads = omp_get_num_threads();
  }

  std::string likwid_threads_string;
  for(int i = 0; i < performance_monitor::num_threads; i++){
    likwid_threads_string += std::to_string(i);
    if(i != num_threads - 1){
      likwid_threads_string += ',';
    }
  }

  setenv("LIKWID_EVENTS", event_groups.c_str(), 1);
  setenv("LIKWID_THREADS", likwid_threads_string.c_str(), 1);
  setenv("LIKWID_FILEPATH", likwidOutputFilepath.c_str(), 
    1);
  setenv("LIKWID_MODE", accessmode.c_str(), 1);
  setenv("LIKWID_FORCE", "1", 1);
  // setenv("LIKWID_DEBUG", "3", 1);

  likwid_markerInit();

  #pragma omp parallel
  {
    likwid_pinThread(omp_get_thread_num());

    /* LIKWID_MARKER_THREADINIT was required with past versions of likwid but
     * now is now commonly not needed and is, in fact, deprecated with likwid
     * v5.0.1
     *
     * It is only required if the pinning library fails and there is a risk of
     * threads getting migrated. I am currently unaware of any runtime system
     * that doesn't work. 
     */ 
    // LIKWID_MARKER_THREADINIT;
  }

  /* Registering regions is optional but strongly recommended, as it reduces
   * overhead of LIKWID_MARKER_START and prevents wrong counts in short
   * regions.
   *
   * There must be a barrier between registering a region and starting that
   * region. Typically these are done in separate parallel blocks, relying on
   * the implicit barrier at the end of the parallel block. Usually there is
   * a parallel block for initialization and a parallel block for execution.
   */
  if(sequential_regions != "")
    registerRegions(sequential_regions);
  
  if(parallel_regions != "")
  {
    #pragma omp parallel
    {
      registerRegions(parallel_regions);
    }
  }
}

void performance_monitor::startRegion(const char * tag)
{
  likwid_markerStartRegion(tag);
}

void performance_monitor::stopRegion(const char * tag)
{
  likwid_markerStopRegion(tag);
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
  else if (aggregation_type == 
    performance_monitor::aggregation_t::saturation)
    return "saturation";
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
  checkInit();

  // create list of regions
  std::set<std::string> regions;
  for (const auto &ptr : per_thread_results)
    regions.insert(ptr.region_name);

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
            && ptr.result_name == likwid_port_usage_event_names[port_num])
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
          .result_name = fhv_port_usage_metrics[port_num],
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
  // "aggregate results" under the special key "saturation"

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
          .aggregation_type = performance_monitor::aggregation_t::saturation,
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

void performance_monitor::printHighlights(){
  checkResults();
  
  std::cout << std::endl 
    << "----- performance_monitor highlights report -----" 
    << std::endl;

  std::cout << "----- key metrics, per-thread -----" << std::endl;
  for(const auto & ptr : performance_monitor::per_thread_results)
  {
    for(const auto & key_metric : fhv_key_metrics)
    {
      if (ptr.result_name == key_metric) std::cout << ptr.toString();
    }
  }

  std::cout << "----- key metrics, aggregated across threads -----"
    << std::endl;
  for(const auto & ar : performance_monitor::aggregate_results)
  {
    for(const auto & key_metric : fhv_key_metrics)
    {
      if (ar.result_name == key_metric) std::cout << ar.toString();
    }
  }
}

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
    for (const auto &key_metric : fhv_key_metrics)
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
    for (const auto &key_metric : fhv_key_metrics)
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
  std::string output_filename = jsonResultOutputDefaultFilepath;
  if(const char* env_p = std::getenv(perfmon_output_envvar))
    output_filename = env_p;

  std::ofstream o(output_filename);
  o << std::setw(4) << results << std::endl;
}

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
