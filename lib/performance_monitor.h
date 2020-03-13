#pragma once

#include <fstream>
#include <iostream>
#include <likwid.h>
#include <map>
#include <math.h>
#include <nlohmann/json.hpp>
#include <omp.h>
#include <stdlib.h>
#include <string>

#include "architecture.h"

#define ACCESSMODE_DAEMON "1"
#define ACCESSMODE_DIRECT "0"
#define MFLOPS_TO_TFLOPS 1e-6
#define OPS_PER_SP_256_VECTOR 8
#define OPS_PER_SP_128_VECTOR 4

using json = nlohmann::json;

class performance_monitor {
  public:
    // ------ attributes ------ //
    static const std::string likwidOutputFilepath;
    static const std::string jsonResultOutputFilepath;
    static const std::string accessmode;

    // ------ functions ------ //
    static void init();
    static void init(const char * event_group);
    static void startRegion(const char * tag);
    static void stopRegion(const char * tag);
    static void close();

    static void getAggregateResults();
    static void compareActualWithbench();
    static void printResults();
    static void printDetailedResults();
    static void printOnlyAggregate();
    static void printComparison();
    static void resultsToJson();

    // ------ getters and setters ----- //
    const static std::map<std::string,double> get_runtimes_by_tag();
    const static std::map<std::string,double> get_aggregate_events();
    const static std::map<std::string,double> get_aggregate_metrics();
    const static std::map<std::string,double> get_saturation();

    const static std::string get_total_sp_flops_event_name();
    const static std::string get_sp_scalar_flops_event_name();
    const static std::string get_sp_avx_256_flops_event_name();
    const static std::string get_sp_avx_128_flops_event_name();

    const static std::string get_mflops_metric_name();
    const static std::string get_mflops_dp_metric_name();
    const static std::string get_l2_bandwidth_metric_name();
    const static std::string get_l3_bandwidth_metric_name();
    const static std::string get_ram_bandwidth_metric_name();
    const static std::string get_ram_data_volume_metric_name();

  private:
    // ------ attributes ------ //
    static int num_threads;

    // "runtimes by tag" should really be called "max runtime by tag" because
    // that's how it's calculated, but likwid seems to calculate flops on a
    // per-thread basis so this won't let us double-check the likwid

    // calculations
    static std::map<std::string, double> runtimes_by_tag;

    // aggregate results
    static std::map<std::string, double> aggregate_events;
    static std::map<std::string, double> aggregate_metrics;
    static std::map<std::string, double> saturation;

    // names for things
    static const std::string total_sp_flops_event_name;
    static const std::string sp_scalar_flops_event_name;
    static const std::string sp_avx_256_flops_event_name;
    static const std::string sp_avx_128_flops_event_name;

    static const std::string mflops_metric_name;
    static const std::string mflops_dp_metric_name;

    static const std::string ram_data_volume_metric_name; 

    static const std::string l2_bandwidth_metric_name;
    static const std::string l3_bandwidth_metric_name;
    static const std::string ram_bandwidth_metric_name;
};
