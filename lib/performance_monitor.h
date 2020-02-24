#pragma once

#include <math.h>
#include <iostream>
#include <likwid.h>
#include <omp.h>
#include <stdlib.h>

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
    void printResults();
    void printDetailedResults();
    void printOnlyAggregate();

    // ------ getters and setters ----- //
    float getMFlops();

  private:
    // ------ attributes ------ //
    int num_threads;
    double runtime;

    // aggregate results
    const char * flops_event_name = "FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE";
    float num_flops;
    const char * mflops_metric_name = "AVX SP [MFLOP/s]";
    float mflops;
    const char * l2_bandwidth_metric_name = "L2 bandwidth [MBytes/s]";
    float l2_bw;
    const char * l3_bandwidth_metric_name = "L3 bandwidth [MBytes/s]";
    float l3_bw;

};
