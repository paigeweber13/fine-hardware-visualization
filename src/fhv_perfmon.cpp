#include "fhv_perfmon.hpp"

// declarations
fhv::types::aggregate_results_t
  fhv_perfmon::aggregate_results;

fhv::types::per_thread_results_t
  fhv_perfmon::per_thread_results;

int fhv_perfmon::num_threads = -1;


// ------ perfmon stuff ------ //

void
fhv_perfmon::registerRegions(
    const std::string regions,
    bool parallel)
{
  if (regions == "")
  {
    std::cerr << "WARNING: No regions supplied! Doing nothing." << std::endl;
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
    if(parallel) {
      #pragma omp parallel
      {
        likwid_markerRegisterRegion(token.c_str());
      }
    }
    else {
      likwid_markerRegisterRegion(token.c_str());
    }
    start_pos = end_pos + delimiter.length();
  } while (end_pos != std::string::npos);
}

void fhv_perfmon::init(std::string parallel_regions,
                       std::string sequential_regions, std::string event_groups)
{
  // initialize num_threads
  #pragma omp parallel
  {
      fhv_perfmon::num_threads = omp_get_num_threads();
  }

  std::string likwid_threads_string;
  for(int i = 0; i < fhv_perfmon::num_threads; i++){
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
    registerRegions(sequential_regions, false);
  
  if(parallel_regions != "")
  {
    #pragma omp parallel
    {
      registerRegions(parallel_regions, true);
    }
  }
}

void fhv_perfmon::startRegion(const char * tag)
{
  likwid_markerStartRegion(tag);
}

void fhv_perfmon::stopRegion(const char * tag)
{
  likwid_markerStopRegion(tag);
}

void fhv_perfmon::nextGroup(){
#pragma omp barrier
#pragma omp single
  {
    likwid_markerNextGroup();
  }
}

void fhv_perfmon::close(){
  likwid_markerClose();

  load_likwid_data();
  calculate_port_usage_ratios();
  std::sort(per_thread_results.begin(), per_thread_results.end());

  perform_result_aggregation();
  calculate_saturation(); 
  std::sort(aggregate_results.begin(), aggregate_results.end());
}

void fhv_perfmon::validate_and_store_likwid_result(
        int thread_num,
        fhv::types::result_t result_type,
        const char * region_name,
        const char * group_name,
        const char * result_name,
        double result_value)
{
  bool keep_result = true;

  if(isnan(result_value)){
    std::cerr << "ERROR: likwid returned a NAN result value, which MAY "
      << "indicate that something went wrong." << std::endl;

    keep_result = false;
  }
  else if(result_value < 0){
    std::cerr << "ERROR: likwid returned a negative result value, indicating "
      << "that something went wrong." << std::endl;

    keep_result = false;
  }
  else if(!std::getenv(perfmon_keep_large_values_envvar.c_str())){

    if(result_type == fhv::types::result_t::event &&
      result_value >= EVENT_VALUE_ERROR_THRESHOLD)
    {
      std::cerr << "WARNING: unreasonably high event value detected."
        << std::endl;

      std::cerr
        << "This event will be discarded. We will try to detect all "
        << "metrics associated with this event, but do not guarantee to "
        << "catch all of them."
        << std::endl
        << "To disable detection and removal of 'unreasonably' high "
        << "values, set the environment variable '" 
        << perfmon_keep_large_values_envvar << "'."
        << std::endl;

      keep_result = false;
    }

    if(result_type == fhv::types::result_t::metric &&
      result_value >= METRIC_VALUE_ERROR_THRESHOLD)
    {
      std::cerr << "WARNING: unreasonably high metric value detected!"
        << std::endl;
      std::cerr
        << "This metric will be discarded. To disable detection and "
        << "removal of 'unreasonably' high values, set the environment "
        << "variable '" << perfmon_keep_large_values_envvar << "'." 
        << std::endl
      keep_result = false;
    }
  }

  // build result object in memory
  fhv::types::PerThreadResult perThreadResult = {
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
    std::cerr << "The erroneous result is printed below:" << std::endl
      << perThreadResult.toString();
    std::cerr << std::endl;
  }
}

void fhv_perfmon::load_likwid_data(){
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
                                         fhv::types::result_t::event, regionName, groupName,
                                         event_name, event_value);
      }

      for (int k = 0; k < perfmon_getNumberOfMetrics(gid); k++){
        const char * metric_name = perfmon_getMetricName(gid, k);
        double metric_value = perfmon_getMetricOfRegionThread(i, k, t);

        validate_and_store_likwid_result(t,
                                         fhv::types::result_t::metric, regionName, groupName,
                                         metric_name, metric_value);
      }
    }
  }
}

void fhv_perfmon::perform_result_aggregation()
{
  std::vector<fhv::types::PerThreadResult> ptr_copy(
          fhv_perfmon::get_per_thread_results());
  
  // a stack is used because we add things in ascending position. Therefore, if
  // we remove in descending position we will not offset anything that comes
  // later 
  while(ptr_copy.size() > 0)
  {
    std::stack<fhv::types::per_thread_results_t::iterator> indices_aggregated;

    // initialize our 3 aggregations to the first thing in the vector
    fhv::types::AggregateResult current_sum = {
      .region_name = ptr_copy.begin()->region_name,
      .group_name = ptr_copy.begin()->group_name,
      .result_type = ptr_copy.begin()->result_type,
      .result_name = ptr_copy.begin()->result_name,
      .aggregation_type = fhv::types::aggregation_t::sum,
      .result_value = ptr_copy.begin()->result_value,
    };

    // copy the initialized "sum" object into the other two objects we need
    fhv::types::AggregateResult current_arithmetic_mean = current_sum;
    current_arithmetic_mean.aggregation_type = 
      fhv::types::aggregation_t::arithmetic_mean;

    fhv::types::AggregateResult current_geometric_mean = current_sum;
    current_geometric_mean.aggregation_type = 
      fhv::types::aggregation_t::geometric_mean;

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

    current_arithmetic_mean.result_value /= fhv_perfmon::num_threads;
    current_geometric_mean.result_value = 
      pow(
        current_geometric_mean.result_value, 
        1.0 / static_cast<double>(fhv_perfmon::num_threads)
      );

    // add the things we found to the aggregate results
    fhv_perfmon::aggregate_results.push_back(current_sum);
    fhv_perfmon::aggregate_results.push_back(current_arithmetic_mean);
    fhv_perfmon::aggregate_results.push_back(current_geometric_mean);

    // remove used things from ptr_copy
    while (!indices_aggregated.empty())
    {
      ptr_copy.erase(indices_aggregated.top());
      indices_aggregated.pop();
    }
  }
}

void fhv_perfmon::calculate_port_usage_ratios()
{
  checkInit();

  // create list of regions
  std::set<std::string> regions;
  for (const auto &ptr : per_thread_results)
    regions.insert(ptr.region_name);

  // instead of using this vector, we could iterate through per_thread_results
  // again. The vector makes things easy, though
  std::vector<double> uops_executed_port(NUM_PORTS_IN_CORE);
  double total_num_port_ops;

  // everything is done on a per-thread, per-region basis
  for (int t = 0; t < fhv_perfmon::num_threads; t++)
  {
    for (const auto &region_name : regions)
    {
      // reset counters for this (thread, region) pair
      for (size_t i = 0; i < uops_executed_port.size(); i++) {
        uops_executed_port[i] = 0;
      }
      total_num_port_ops = 0;

      // first, sum all UOPS_DISPATCHED_PORT_PORT*
      for (size_t port_num = 0; port_num < NUM_PORTS_IN_CORE; port_num++)
      {
        for (const auto &ptr : fhv_perfmon::per_thread_results)
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
        fhv::types::PerThreadResult port_usage_metric = {
          .region_name = region_name,
          .thread_num = t,
          .group_name = fhv_performance_monitor_group,
          .result_type = fhv::types::result_t::metric,
          .result_name = fhv_port_usage_metrics[port_num],
          .result_value = uops_executed_port[port_num] / total_num_port_ops
        };

        fhv_perfmon::per_thread_results.push_back(port_usage_metric);
      }
    }
  }
}

void fhv_perfmon::calculate_saturation(){
  // TODO: eventually I want to build this on a per-thread basis and then have
  // "perform_result_aggregation" automatically aggregate these. however, for
  // now we're just going to build overall results manually and add them to
  // "aggregate results" under the special key "saturation"

  bool is_saturation_result;
  std::string saturation_result_name;
  double saturation_result_value;

  for (const auto & ar : fhv_perfmon::aggregate_results)
  {
    if(ar.aggregation_type == fhv::types::aggregation_t::sum)
    {
      is_saturation_result = false;

      for (size_t i = 0; i < fhv_saturation_metric_names.size(); i++)
      {
        if (ar.result_name == fhv_saturation_source_metrics[i])
        {
          is_saturation_result = true;
          saturation_result_name = fhv_saturation_metric_names[i];
          saturation_result_value = 
            ar.result_value/fhv_saturation_reference_rates[i];
        }
      }

      if (is_saturation_result)
      {
        fhv::types::AggregateResult new_ar = {
          .region_name = ar.region_name,
          .group_name = fhv_performance_monitor_group,
          .result_type = fhv::types::result_t::metric,
          .result_name = saturation_result_name,
          .aggregation_type = fhv::types::aggregation_t::saturation,
          .result_value = saturation_result_value
        };
        
        fhv_perfmon::aggregate_results.push_back(new_ar);
      }
    }
  }
}

void fhv_perfmon::checkInit()
{
  std::string error_str = "";
  bool something_went_wrong = false;

  if (fhv_perfmon::num_threads == -1)
  {
    error_str += "WARNING: fhv_perfmon::num_threads was not set! This "
      "usually happens\n"
      "because the user failed to call fhv_perfmon::init(). FHV will "
      "not be\n"
      "able to build results. Major functionality will be disabled.\n"
      "If init() was called, then something is wrong with init() "
      "internally.\n";
    
    something_went_wrong = true;
  }

  if(something_went_wrong) std::cerr << error_str;
}

void fhv_perfmon::checkResults(){
  std::string error_str = "WARNING: there doesn't seem to be any results for "
    "likwid. This commonly \n"
    "happens because fhv_perfmon::close() was not called.\n";
  
  std::string error_more_info = "";
  bool something_went_wrong = false;

  if (fhv_perfmon::per_thread_results.size() == 0)
  {
    error_more_info += "If close() was called, then something is wrong "
      "internally. Did close() call \n"
      "load_likwid_data()? Is something wrong with load_likwid_data()?\n";
    something_went_wrong = true;
  }

  if (fhv_perfmon::aggregate_results.size() == 0)
  {
    error_more_info += "If close() was called, then something is wrong "
      "internally. Did close() call \n"
      "perform_result_aggregation()? Is something wrong with "
      "perform_result_aggregation()?\n";
    something_went_wrong = true;
  }

  if (something_went_wrong) 
  {
    std::cerr << error_str;
    std::cerr << error_more_info;
  }
}

void fhv_perfmon::printDetailedResults(){
  std::cout << std::endl
    << "----- FHV Performance Monitor: detailed results ----- "
    << std::endl;

  checkResults();

  for (const fhv::types::PerThreadResult &ptr : fhv_perfmon::per_thread_results)
  {
    std::cout << ptr.toString();
  }
}

void fhv_perfmon::printAggregateResults(){
  std::cout << std::endl
    << "----- FHV Performance Monitor: aggregate results ----- "
    << std::endl;

  checkResults();

  for (const fhv::types::AggregateResult &ar : fhv_perfmon::aggregate_results)
  {
    std::cout << ar.toString();
  }
}

void fhv_perfmon::printHighlights(){
  checkResults();
  
  std::cout << std::endl 
    << "----- fhv_perfmon highlights report -----"
    << std::endl;

  std::cout << "----- key metrics, per-thread -----" << std::endl;
  for(const auto & ptr : fhv_perfmon::per_thread_results)
  {
    for(const auto & key_metric : fhv_key_metrics)
    {
      if (ptr.result_name == key_metric) std::cout << ptr.toString();
    }
  }

  std::cout << "----- key metrics, aggregated across threads -----"
    << std::endl;
  for(const auto & ar : fhv_perfmon::aggregate_results)
  {
    for(const auto & key_metric : fhv_key_metrics)
    {
      if (ar.result_name == key_metric) std::cout << ar.toString();
    }
  }
}

void fhv_perfmon::setJsonCpuInfo(json &j){
  topology_init();
  CpuInfo_t cpu_info = get_cpuInfo();
  CpuTopology_t cpu_topology = get_cpuTopology();

  numa_init();
  int num_numa_nodes = likwid_getNumberOfNodes();
  numa_finalize();

  int num_threads;

#pragma omp parallel
    num_threads = omp_get_num_threads();

  std::vector<unsigned> affinity(num_threads);

#pragma omp parallel
    affinity[omp_get_thread_num()] = sched_getcpu();

  std::string affinity_str = "";
  for(size_t i = 0; i < affinity.size(); i++) {
    affinity_str += std::to_string(affinity[i]);

    if (i != affinity.size()-1)
      affinity_str += ",";
  }

  j[json_info_section][json_processor_section][json_processor_name_key] =
      std::string(cpu_info->osname) + " (" + cpu_info->short_name + ")";
  j[json_info_section][json_processor_section][json_processor_num_sockets_key] =
      cpu_topology->numSockets;
  j[json_info_section][json_processor_section][json_processor_num_numa_nodes_key] =
      num_numa_nodes;
  j[json_info_section][json_processor_section][json_processor_num_hw_threads_key] =
      cpu_topology->numHWThreads;
  j[json_info_section][json_processor_section][json_processor_num_threads_in_use_key] =
      num_threads;
  j[json_info_section][json_processor_section][json_processor_affinity_key] =
      affinity_str;

  // this call *must* come at the end of the function, or the pointers in the
  // structs "cpu_info" and "cpu_topology" will reference null.
  topology_finalize();
}

void fhv_perfmon::resultsToJson(std::string param_info_string)
{
  checkInit();
  checkResults();
  
  json results;
  
  // set parameter info string
  results[json_info_section][json_parameter_key] = param_info_string;

  // set system info
  setJsonCpuInfo(results);

  // populate json with per_thread_results
  for (const auto & ptr : fhv_perfmon::per_thread_results)
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
  for (const auto & ar : fhv_perfmon::aggregate_results)
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
  if(const char* env_p = std::getenv(perfmon_output_envvar.c_str()))
    output_filename = env_p;

  fhv::utils::create_directories_for_file(output_filename);

  std::ofstream o(output_filename);
  o << std::setw(4) << results << std::endl;
}

const fhv::types::aggregate_results_t&
fhv_perfmon::get_aggregate_results()
{
	return aggregate_results;
}

const fhv::types::per_thread_results_t&
fhv_perfmon::get_per_thread_results()
{
  return per_thread_results;
}
