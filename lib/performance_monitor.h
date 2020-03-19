#pragma once

#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <likwid.h>
#include <map>
#include <math.h>
#include <nlohmann/json.hpp>
#include <omp.h>
#include <stdlib.h>
#include <string>

#include "architecture.h"

// magic numbers
#define ACCESSMODE_DAEMON "1"
#define ACCESSMODE_DIRECT "0"
#define MFLOPS_TO_TFLOPS 1e-6
#define OPS_PER_SP_256_VECTOR 8
#define OPS_PER_SP_128_VECTOR 4

// ---- Names of things ---- //
// number of flops
#define total_sp_flops_event_name "total sp flops"
#define sp_scalar_flops_event_name "FP_ARITH_INST_RETIRED_SCALAR_SINGLE"
#define sp_avx_128_flops_event_name "FP_ARITH_INST_RETIRED_128B_PACKED_SINGLE"
#define sp_avx_256_flops_event_name "FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE"

// flop rates
#define mflops_metric_name "SP [MFLOP/s]"
#define mflops_dp_metric_name "DP [MFLOP/s]"

// cache volume and bandwidth
#define l2_bandwidth_metric_name "L2 bandwidth [MBytes/s]"
#define l2_data_volume_name "L2 data volume [GBytes]"
#define l2_evict_bandwidth_name "L2D evict bandwidth [MBytes/s]"
#define l2_evict_data_volume_name "L2D evict data volume [GBytes]"
#define l2_load_bandwidth_name "L2D load bandwidth [MBytes/s]"
#define l2_load_data_volume_name "L2D load data volume [GBytes]"

#define l3_bandwidth_metric_name "L3 bandwidth [MBytes/s]"
#define l3_data_volume_name "L3 data volume [GBytes]"
#define l3_evict_bandwidth_name "L3 evict bandwidth [MBytes/s]"
#define l3_evict_data_volume_name "L3 evict data volume [GBytes]"
#define l3_load_bandwidth_name "L3 load bandwidth [MBytes/s]"
#define l3_load_data_volume_name "L3 load data volume [GBytes]"

// memory volume and bandwidth
#define ram_bandwidth_metric_name "Memory bandwidth [MBytes/s]"
#define ram_data_volume_metric_name "Memory data volume [GBytes]"
#define ram_evict_bandwidth_name "Memory evict bandwidth [MBytes/s]"
#define ram_evict_data_volume_name "Memory evict data volume [GBytes]"
#define ram_load_bandwidth_name "Memory load bandwidth [MBytes/s]"
#define ram_load_data_volume_name "Memory load data volume [GBytes]"

// other memroy/cache stuff
#define load_to_store_ratio_metric_name "Load to store ratio"

// ---- End names of things ---- //

using json = nlohmann::json;

class performance_monitor {
  public:
    // ------ attributes ------ //
    const static std::string likwidOutputFilepath;
    const static std::string jsonResultOutputFilepath;
    const static std::string accessmode;

    // ------ functions ------ //
    // actual functionality
    static void init();
    static void init(const char * event_group);
    static void startRegion(const char * tag);
    static void stopRegion(const char * tag);
    static void close();

    // building data to print
    static void getAggregateResults();
    static void compareActualWithbench();

    // print results
    static void printResults();
    static void printDetailedResults();
    static void printOnlyAggregate();
    static void printComparison();

    // output to json
    static void resultsToJson();

    // ------ getters ----- //
    const static std::map<std::string,double> get_runtimes_by_tag();
    const static std::map<std::string,double> get_aggregate_events();
    const static std::map<std::string,double> get_aggregate_metrics();
    const static std::map<std::string,double> get_saturation();

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

};
