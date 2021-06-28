#pragma once

#include <algorithm>
#include <fmt/core.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <likwid.h>
#include <math.h>
#include <nlohmann/json.hpp>
#include <omp.h>
#include <sched.h>
#include <set>
#include <sstream>
#include <stack>
#include <string>

#include "config.hpp"
#include "likwid_defines.hpp"
#include "performance_monitor_defines.hpp"
#include "types.hpp"
#include "utils.hpp"

using json = nlohmann::json;

class fhv_perfmon {
  public:
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
    static void init(std::string parallel_regions,
      std::string sequential_regions,
      std::string event_groups);
    static void init(std::string parallel_regions = "",
        std::string sequential_regions = "");

    // if parallel is true, will register regions in a parallel block
    static void registerRegions(const std::string regions, bool parallel);

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

    static void setJsonCpuInfo(json &j);

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

    const static fhv::types::aggregate_results_t& get_aggregate_results();
    const static fhv::types::per_thread_results_t& get_per_thread_results();

  private:
    // ------ functions ------ //
    // helper function to validate data from likwid
    static void validate_and_store_likwid_result(
            int thread_num,
            fhv::types::result_t result_type,
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

    static fhv::types::aggregate_results_t  aggregate_results;

    static fhv::types::per_thread_results_t per_thread_results;

};
