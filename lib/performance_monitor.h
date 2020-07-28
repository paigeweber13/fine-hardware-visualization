#pragma once

// add back only what is necessary
#include <algorithm>
// #include <cmath>
// #include <cstring>
// #include <fstream>
#include <iomanip>
#include <iostream>
#include <likwid.h>
// #include <map>
#include <math.h>
#include <nlohmann/json.hpp>
#include <omp.h>
#include <set>
#include <sstream>
#include <stack>
// #include <stdexcept>
// #include <stdlib.h>
#include <string>

#include "architecture.h"
#include "likwid_defines.hpp"

// magic numbers
// this is the value that seems to always come up in the likwid unreasonably
// high values bug (see https://github.com/RRZE-HPC/likwid/issues/292 )
#define EVENT_VALUE_ERROR_THRESHOLD 1.84467e19 
#define METRIC_VALUE_ERROR_THRESHOLD 1e10 
#define ACCESSMODE_DAEMON "1"
#define ACCESSMODE_DIRECT "0"
#define MFLOPS_TO_TFLOPS 1e-6
#define OPS_PER_SP_256_VECTOR 8
#define OPS_PER_SP_128_VECTOR 4

// keywords
#define perfmon_output_envvar "FHV_OUTPUT"
#define perfmon_keep_large_values_envvar "FHV_KEEP_LARGE_VALUES"

// saturation keywords
#define flops_sp_saturation_metric_name "FLOPS SP saturation"
#define flops_dp_saturation_metric_name "FLOPS DP saturation"
#define l2_saturation_metric_name "L2 bandwidth saturation"
#define l3_saturation_metric_name "L3 bandwidth saturation"
#define mem_saturation_metric_name "Memory bandwidth saturation"

// remove
// #define total_sp_flops_event_name "total sp flops"

// remove
// #define group_sum_by_region_keyword "group_sum_by_region"

// remove
// #define all_groups_keyword "all_groups"

// #define all_regions_keyword "all_regions"

// port usage ratio names
const std::string fhv_performance_monitor_group = "FHV_PERFORMANCE_MONITOR";

// remove
// const std::string fhv_port_usage_group = "FHV Port usage ratios";
const std::string fhv_port_usage_ratio_start = "Port";
const std::string fhv_port_usage_ratio_end = " usage ratio";

// remove
// #define fhv_port0_usage_ratio "Port0 usage ratio"
// #define fhv_port1_usage_ratio "Port1 usage ratio"
// #define fhv_port2_usage_ratio "Port2 usage ratio"
// #define fhv_port3_usage_ratio "Port3 usage ratio"
// #define fhv_port4_usage_ratio "Port4 usage ratio"
// #define fhv_port5_usage_ratio "Port5 usage ratio"
// #define fhv_port6_usage_ratio "Port6 usage ratio"
// #define fhv_port7_usage_ratio "Port7 usage ratio"

using json = nlohmann::json;

class performance_monitor {
  public:
    // enums for aggregation type and result type
    enum class aggregation_t { sum, arithmetic_mean, geometric_mean };
    enum class result_t { event, metric };

    // ---- TYPES

    // TODO: consider adding multimaps from value to index of thing (or value
    // to pointer to thing) for easy access and easy collection of all entries
    // within a range using std::lower_bound and std::upper_bound

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

    // ------ attributes ------ //
    const static std::string likwidOutputFilepath;
    const static std::string jsonResultOutputFilepath;
    const static std::string accessmode;

    // ------ functions ------ //
    // actual functionality

    // TODO: merge init functions and use optional parameters instead of
    // overloading.  

    // does a default set of groups that allows fhv to work normally. Also
    // automatically uses the default number of threads created by openMP as
    // below 
    static void init(const char * parallel_regions,
                     const char * sequential_regions);

    // automatically uses the default number of threads created by openMP,
    // which is probably the max number of threads supported by the hardware
    static void init(const char * event_group, 
                     const char * parallel_regions,
                     const char * sequential_regions);

    // lets you choose a number of threads and automatically chooses the first
    // num_threads threads to create a list_of_threads
    static void init(const char * event_group,
                     const char * parallel_regions,
                     const char * sequential_regions,
                     int num_threads);

    // lets you specify everything manually
    //   event_group should be of the format "FLOPS_SP|L2|..."
    //   parallel_regions and sequential_regions should be of the format
    //    "region1,region2,..."
    //   parallel_regions are regions that will be executed in a parallel block
    //   sequential_regions are regions that will be executed in sequential
    //    code 
    //   list_of_threads should be of the format "0,2,..."
    static void init(const char * event_group,
                     const char * parallel_regions,
                     const char * sequential_regions,
                     const char * list_of_threads);

    // static void setEnvironmentVariables(const char * regions);
    // static void setEnvironmentVariables(const char * regions,
    //                                     int num_threads);
    static void setEnvironmentVariables(const char * regions,
                                        const char * list_of_threads);
    static void registerRegions(const char * regions);

    static void startRegion(const char * tag);
    static void stopRegion(const char * tag);
    static void nextGroup();
    static void close();

    // building data to print

    // sum events and metrics across threads. Organize by region and group

    // TODO: evaluate these functions. buildingResultsMaps may need to be
    // rewritten from the ground up... in general this file has a lot of
    // hard-to-read (and, therefore, hard to maintain) code and I feel like
    // this falls prey to that. Can these functions be simplified?
    // consider removing and just build everything in close()
    // static void buildResultsMaps();

    // KEEP. Commented out for now
    // static void compareActualWithBench();

    // print results

    // debug info about what groups and regions were found

    // consider removing
    // static void printRegionGroupEventAndMetricData();

    // print everything, utility function that does everything below
    
    // consider removing
    // static void printResults();

    // print everything per core
    static void printDetailedResults();

    // print results aggregated across cores
    static void printAggregateResults();

    // print comparison between actual rates and theoretical maximums. Also
    // called "saturation"

    // prints comparison by region, but includes a special region "all_regions"
    // that supplies the average saturation

    // keep, but commented out for now
    // static void printComparison();

    // consider removing
    static void printHighlights();

    // consider removing
    // static void printCsvHeader();
    // static void printCsvOutput();

    // output to json
    //  - gets file name from environment variable FHV_OUTPUT. If unset, will
    //    use a default name
    //  - param_info_string is entirely arbitrary and left to the user. The
    //    intention is to use it to track parameters used to generate a
    //    visualization. For example, when metering convolution, the user may
    //    set the string to be "n=4000, m=6000, k=7". The visualization will
    //    preface this with the string "Parameters used to generate:"

    // keep, but commented out for now
    static void resultsToJson(std::string param_info_string = "");

    // ------ getters ----- //

    // consider removing
    // const static std::map<std::string, double> get_runtimes_by_tag();

    const static aggregate_results_t& get_aggregate_results();
    const static per_thread_results_t& get_per_thread_results();

    // consider removing: can we just put this into per_thread_results and aggregate_results?
    // const static saturation_map_t& get_saturation();

  private:
    // ------ functions ------ //
    // dround = decimal round
    static double dround(double x, unsigned num_decimal_places);

    static void validate_and_store_likwid_result(
      int thread_num,
      performance_monitor::result_t result_type,
      const char * region_name, 
      const char * group_name,
      const char * result_name, 
      double result_value);
    
    static void checkInit();
    static void checkResults();

    // used to load likwid data
    static void load_likwid_data();

    // used to aggregate results. Depends on "load_likwid_data" being called
    // before this is called
    static void perform_result_aggregation();

    static void calculate_port_usage_ratios();

    static void calculate_saturation();

    // ------ attributes ------ //

    // --- important numbers
    static int num_threads;

    // identifies the most important metrics: the ones we will output to json
    // for later use in visualization
    static std::vector<std::string> key_metrics;

    static std::vector<std::string> saturation_metrics;

    // names of saturation metrics
    
    // remove
    // static const std::vector<const char *> saturationMetricGroups;

    // consider removing
    // static const std::vector<const char *> saturation_metrics;

    // try to avoid, consider removing
    // static const std::vector<float> saturationBenchmarkReferences;

    // names of port usage metrics

    // consider removing
    // static const std::vector<std::string> port_usage_metrics;

    // --- Data

    // "runtimes by tag" should really be called "max runtime by tag" because
    // that's how it's calculated, but likwid seems to calculate flops on a
    // per-thread basis so this won't let us double-check the likwid

    // consider removing
    // static std::map<std::string, double> runtimes_by_tag;

    static performance_monitor::aggregate_results_t  aggregate_results;

    static performance_monitor::per_thread_results_t per_thread_results;

    // map region to saturation name to saturation value

    // saturation name matches metric name
    static performance_monitor::saturation_map_t saturation;
};
