#pragma once

#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <likwid.h>
#include <map>
#include <math.h>
#include <nlohmann/json.hpp>
#include <omp.h>
#include <sstream>
#include <stdlib.h>
#include <string>

#include "architecture.h"
#include "likwid_defines.hpp"

// magic numbers
#define ACCESSMODE_DAEMON "1"
#define ACCESSMODE_DIRECT "0"
#define MFLOPS_TO_TFLOPS 1e-6
#define OPS_PER_SP_256_VECTOR 8
#define OPS_PER_SP_128_VECTOR 4

// keywords
#define total_sp_flops_event_name "total sp flops"
// #define group_sum_by_region_keyword "group_sum_by_region"
#define all_groups_keyword "all_groups"
#define all_regions_keyword "all_regions"


// enums for aggregation type and result type
enum aggregation_type { sum, arithmetic_mean, geometric_mean };
enum result_type { event, metric };

using json = nlohmann::json;

class performance_monitor {
  public:
    // ------ attributes ------ //
    const static std::string likwidOutputFilepath;
    const static std::string jsonResultOutputFilepath;
    const static std::string accessmode;

    // ------ functions ------ //
    // actual functionality

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

    static void buildResultsMaps();
    static void compareActualWithBench();

    // print results

    // debug info about what groups and regions were found
    static void printRegionGroupEventAndMetricData();

    // print everything, utility function that does everything below
    static void printResults();

    // print everything per core
    static void printDetailedResults();

    // print results aggregated across cores
    static void printOnlyAggregate();

    // print comparison between actual rates and theoretical maximums. Also
    // called "saturation"

    // prints comparison by region, but includes a special region "all_regions"
    // that supplies the average saturation
    static void printComparison();

    static void printHighlights();
    static void printCsvHeader();
    static void printCsvOutput();

    // output to json
    static void resultsToJson();

    // ------ getters ----- //
    const static std::map<std::string, double> get_runtimes_by_tag();
    const static std::map<
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
      get_aggregate_results();

    const static std::map<
      result_type, std::map<
        int, std::map<
          std::string, std::map<
            std::string, std::map<
              std::string, double
            >
          >
        >
      >
    >
      get_per_thread_results();
    const static std::map<std::string, std::map<std::string, double>>
      get_saturation();
    const static std::map<std::string, std::map<std::string, double>>
      get_average_port_usage_info();

  private:
    // ------ functions ------ //
    // dround = decimal round
    static double dround(double x, unsigned num_decimal_places);

    // ------ attributes ------ //

    // --- constants and magic numbers
    static int num_threads;

    // names of saturation metrics
    static const std::vector<const char *> saturationMetricGroups;
    static const std::vector<const char *> saturation_metrics;
    static const std::vector<float> saturationBenchmarkReferences;

    // names of port usage metrics
    static const std::vector<std::string> port_usage_metrics;

    // --- Data

    // "runtimes by tag" should really be called "max runtime by tag" because
    // that's how it's calculated, but likwid seems to calculate flops on a
    // per-thread basis so this won't let us double-check the likwid

    static std::map<std::string, double> runtimes_by_tag;

    // aggregate results

    // each includes group for "all groups" and region for "all regions"

    // aggregation type -> result type (event or metric) -> region name ->
    // group name -> thing name -> thing value
    static std::map<
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
      aggregate_results;

    // in the case of "per_thread_results", "thread" refers to the hardware
    // thread 

    // result type (event or metric) -> thread number -> region name -> group
    // name -> thing name -> thing value
    static std::map<
      result_type, std::map<
        int, std::map<
          std::string, std::map<
            std::string, std::map<
              std::string, double
            >
          >
        >
      >
    >
      per_thread_results;

    // map region to saturation name to saturation value

    // saturation name matches metric name
    static std::map<std::string, std::map<std::string, double>> saturation;
};
