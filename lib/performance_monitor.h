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
    void printOnlyAggregate();

    // ------ getters and setters ----- //

  private:
    // ------ attributes ------ //
    int num_threads;
    double runtime;

    // aggregate results
    const char * flops_event_name = "FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE";
    float num_flops = 0.;
    const char * mflops_metric_name = "AVX SP [MFLOP/s]";
    float mflops = 0.;
};
