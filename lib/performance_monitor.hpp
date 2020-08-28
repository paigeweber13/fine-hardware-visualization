#pragma once

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <likwid.h>
#include <map>
#include <math.h>
#include <nlohmann/json.hpp>
#include <omp.h>
#include <set>
#include <sstream>
#include <stack>
#include <string>

#include "architecture.hpp"
#include "likwid_defines.hpp"
#include "performance_monitor_defines.hpp"

using json = nlohmann::json;

class performance_monitor {
  public:
    // enums for aggregation type and result type
    enum class aggregation_t { sum, arithmetic_mean, geometric_mean, 
      saturation };
    enum class result_t { event, metric };

    // to string functions for those
    static std::string aggregationTypeToString(
      const performance_monitor::aggregation_t &aggregation_type);
    static std::string resultTypeToString(
      const performance_monitor::result_t &result_type);

    // ---- TYPES

    // represents a result unique to a thread. "thread" refers to the hardware
    // thread 
    struct PerThreadResult {
      std::string region_name;
      int thread_num;
      std::string group_name;
      performance_monitor::result_t result_type;
      std::string result_name;
      double result_value;

      bool operator<(const PerThreadResult& other) const;
      bool matchesForAggregation(const PerThreadResult& other) const;
      std::string toString(std::string delim = " | ") const;
    };

    // represents a result aggregated across threads in a per-thread manner
    struct AggregateResult {
      std::string region_name;
      std::string group_name;
      performance_monitor::result_t result_type;
      std::string result_name;
      performance_monitor::aggregation_t aggregation_type;
      double result_value;

      bool operator<(const AggregateResult& other) const;
      std::string toString(std::string delim = " | ") const;
    };

    typedef
    std::vector< PerThreadResult >
    per_thread_results_t;

    typedef 
    std::vector< AggregateResult >
    aggregate_results_t;

    // maps region to saturation name to saturation value
    typedef std::map<std::string, std::map<std::string, double>> 
      saturation_map_t;

    // ------ functions ------ //
    // actual functionality

    // init()
    //
    // if parallel_regions or sequential_regions are not supplied, no regions
    // will be registered. Registering regions is optional with likwid, but it
    // reduces overhead, which can prevent wrong counts for short regions
    // 
    // the user may specify everything. The defaults choose an event_group that
    // will give fhv everything it needs to create a visualization
    //   - event_group should be of the format "FLOPS_SP|L2|..."
    //   - parallel_regions and sequential_regions should be of the format
    //      "region1,region2,..."
    //   - parallel_regions are regions that will be executed in a parallel block
    //   - sequential_regions are regions that will be executed in sequential
    //      code 
    //
    // OMP_NUM_THREADS is respected. Currently, threads will be assigned
    // sequentially from the first.
    // 
    static void init(std::string parallel_regions = "",
      std::string sequential_regions = "",
      std::string event_groups = 
        "MEM_DP|FLOPS_SP|L3|L2|PORT_USAGE1|PORT_USAGE2|PORT_USAGE3");

    static void registerRegions(const std::string regions);

    static void startRegion(const char * tag);
    static void stopRegion(const char * tag);
    static void nextGroup();
    static void close();

    // print everything per core
    static void printDetailedResults();

    // print results aggregated across cores
    static void printAggregateResults();

    // consider removing
    static void printHighlights();

    // consider removing
    // static void printCsvHeader();
    // static void printCsvOutput();

    // resultsToJson
    //  - gets file name from environment variable FHV_OUTPUT. If unset, will
    //    use a default name
    //  - param_info_string is entirely arbitrary and left to the user. The
    //    intention is to use it to track parameters used to generate a
    //    visualization. For example, when metering convolution, the user may
    //    set the string to be "n=4000, m=6000, k=7". The visualization will
    //    preface this with the string "Parameters used to generate:"

    static void resultsToJson(std::string param_info_string = "");

    // ------ getters ----- //

    const static aggregate_results_t& get_aggregate_results();
    const static per_thread_results_t& get_per_thread_results();

  private:
    // ------ functions ------ //
    // helper function to validate data from likwid
    static void validate_and_store_likwid_result(
      int thread_num,
      performance_monitor::result_t result_type,
      const char * region_name, 
      const char * group_name,
      const char * result_name, 
      double result_value);
    
    // used to make sure things got initialized correctly
    static void checkInit();
    static void checkResults();

    // used to load likwid data
    static void load_likwid_data();

    // used to aggregate results. Depends on "load_likwid_data" being called
    // before this is called
    static void perform_result_aggregation();

    // must be called after load_likwid_data(), as it depends on the loaded
    // data. Should be called before perform_result_aggregation so that port
    // usage ratios also get aggregated
    static void calculate_port_usage_ratios();

    // must be called after load_likwid_data() and
    // perform_result_aggregation(). This manually creates saturation values in
    // "geometric mean" even though they are not actually means. They do,
    // however, fill the same function: providing an overview

    // TODO: make this per-thread and let perform_result_aggregation()
    // aggregate it
    static void calculate_saturation();

    // ------ attributes ------ //

    // --- important numbers
    static int num_threads;

    static performance_monitor::aggregate_results_t  aggregate_results;

    static performance_monitor::per_thread_results_t per_thread_results;

};
