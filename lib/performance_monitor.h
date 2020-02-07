#include <iostream>
#include <likwid.h>
#include <omp.h>
#include <stdlib.h>

#define ACCESSMODE_DAEMON "1"
#define ACCESSMODE_DIRECT "0"

class performance_monitor {
  public:
    // ------ attributes ------ //
    const char *tag = "region";
    const char *event_group = "FLOPS_SP";
    const char *filepath = "/tmp/test_marker.out";
    const char *accessmode = ACCESSMODE_DAEMON;
    int num_threads;

    // ------ functions ------ //
    void likwid_perfmonInit();
    void likwid_perfmonStart();
    void likwid_perfmonStop();
    void likwid_perfmonPrintResults();

  private:
};
