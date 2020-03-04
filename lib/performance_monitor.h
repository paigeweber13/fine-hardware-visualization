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
#define OPS_PER_VECTOR 8

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
    static std::map<std::string,double> get_runtimes_by_tag();
    static const std::string get_flops_event_name();
    static float get_num_flops();

    static const std::string get_mflops_metric_name();
    static float getMFlops();
    static float get_mflops_saturation();
    static const std::string get_mflops_dp_metric_name();
    static float get_mflops_dp();
    static float get_mflops_dp_saturation();

    static const std::string get_l2_bandwidth_metric_name();
    static float get_l2_bw();
    static float get_l2_bw_saturation();
    static const std::string get_l3_bandwidth_metric_name();
    static float get_l3_bw();
    static float get_l3_bw_saturation();
    static const std::string get_ram_bandwidth_metric_name();
    static float get_ram_bw();
    static float get_ram_bw_saturation();

  private:
    // ------ attributes ------ //
    static int num_threads;
    static std::map<std::string, double> runtimes_by_tag;

    // aggregate results
    static const std::string flops_event_name;
    static float num_flops;

    static const std::string mflops_metric_name;
    static float mflops;
    static float mflops_saturation;
    static const std::string mflops_dp_metric_name;
    static float mflops_dp;
    static float mflops_dp_saturation;

    static const std::string l2_bandwidth_metric_name;
    static float l2_bw;
    static float l2_bw_saturation;
    static const std::string l3_bandwidth_metric_name;
    static float l3_bw;
    static float l3_bw_saturation;
    static const std::string ram_bandwidth_metric_name;
    static float ram_bw;
    static float ram_bw_saturation;

};
