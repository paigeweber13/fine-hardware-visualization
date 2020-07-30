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

#include "architecture.h"
#include "likwid_defines.hpp"
#include "performance_monitor_defines.hpp"

using json = nlohmann::json;

class performance_monitor {
  public:
    // enums for aggregation type and result type
    enum class aggregation_t { sum, arithmetic_mean, geometric_mean };
    enum class result_t { event, metric };

    // to string functions for those
    static std::string aggregationTypeToString(
      const performance_monitor::aggregation_t &aggregation_type);
    static std::string resultTypeToString(
      const performance_monitor::result_t &result_type);

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
    // dround = decimal round
    static double dround(double x, unsigned num_decimal_places);

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

    // identifies the most important metrics: the ones we will output to json
    // for later use in visualization. This is a class variable because it is
    // build in the init routine.
    static std::vector<std::string> key_metrics;

    static performance_monitor::aggregate_results_t  aggregate_results;

    static performance_monitor::per_thread_results_t per_thread_results;

};
