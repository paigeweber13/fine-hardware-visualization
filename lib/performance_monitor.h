#pragma once

#include <math.h>
#include <iostream>
#include <likwid.h>
#include <map>
#include <omp.h>
#include <stdlib.h>
#include <string>

#include "architecture.h"

#define ACCESSMODE_DAEMON "1"
#define ACCESSMODE_DIRECT "0"
#define MFLOPS_TO_TFLOPS 1e-6
#define OPS_PER_VECTOR 8

class performance_monitor {
  public:
    ~performance_monitor();

    // ------ attributes ------ //
    const char *filepath = "/tmp/test_marker.out";
    const char *accessmode = ACCESSMODE_DAEMON;

    // ------ functions ------ //
    void init(const char * event_group);
    void startRegion(const char * tag);
    void stopRegion(const char * tag);
    void close();

    void getAggregateResults();
    void compareActualWithbench();
    void printResults();
    void printDetailedResults();
    void printOnlyAggregate();
    void printComparison();

    // ------ getters and setters ----- //
    float getMFlops();

  private:
    // ------ attributes ------ //
    int num_threads;
    std::map<std::string, double> runtimes_by_tag;

    // aggregate results
    const char * flops_event_name = "FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE";
    float num_flops;

    const char * mflops_metric_name = "AVX SP [MFLOP/s]";
    float mflops;
    float mflops_saturation;
    const char * mflops_dp_metric_name = "AVX DP [MFLOP/s]";
    float mflops_dp;
    float mflops_dp_saturation;

    const char * l2_bandwidth_metric_name = "L2 bandwidth [MBytes/s]";
    float l2_bw;
    float l2_bw_saturation;
    const char * l3_bandwidth_metric_name = "L3 bandwidth [MBytes/s]";
    float l3_bw;
    float l3_bw_saturation;
    const char * ram_bandwidth_metric_name = "Memory bandwidth [MBytes/s]";
    float ram_bw;
    float ram_bw_saturation;

};
