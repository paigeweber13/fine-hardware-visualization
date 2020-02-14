#include <iostream>
#include <likwid.h>
#include <omp.h>
#include <stdlib.h>

#define ACCESSMODE_DAEMON "1"
#define ACCESSMODE_DIRECT "0"
#define MFLOPS_TO_TFLOPS 1e-6

class performance_monitor {
  public:
    // ------ attributes ------ //
    const char *filepath = "/tmp/test_marker.out";
    const char *accessmode = ACCESSMODE_DAEMON;
    int num_threads;

    // ------ functions ------ //
    void init(const char * event_group);
    void startRegion(const char * tag);
    void stopRegion(const char * tag);
    void close();
    void printResults();

  private:
};
