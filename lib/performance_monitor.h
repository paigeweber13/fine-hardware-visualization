#include <iostream>
#include <likwid.h>
#include <omp.h>
#include <stdlib.h>

#define ACCESSMODE_DAEMON "1"
#define ACCESSMODE_DIRECT "0"

class performance_monitor {
  public:
    // ------ attributes ------ //
    const char *filepath = "/tmp/test_marker.out";
    const char *accessmode = ACCESSMODE_DAEMON;
    int num_threads;

    // ------ functions ------ //
    void likwid_perfmonInit(const char * event_group);
    void likwid_perfmonStart(const char * tag); // could also be called "begin region"
    void likwid_perfmonStop(const char * tag); // could also be called "end region"
    void likwid_perfmonClose();
    void likwid_perfmonPrintResults();

  private:
};
