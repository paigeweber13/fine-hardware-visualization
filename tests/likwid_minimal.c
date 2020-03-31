// can be compiled with
// gcc likwid_minimal.c -L/usr/local/lib -march=native -mtune=native -fopenmp -llikwid
// or the like

// this can be run alone `./a.out`
// or with likwid-perfctr by commenting out the `setenv` lines and then running
// likwid-perfctr -C S0:0 -g FLOPS_DP -M 1 -m ./a.out
// or the like

// some other command examples, since I've been using this to test lots of
// stuff:
//  - likwid-perfctr -C S0:0 -g L3 -g FLOPS_DP -M 1 -m ./a.out
//  - likwid-perfctr -C S0:0 -g FP_ARITH_INST_RETIRED_SCALAR_DOUBLE:PMC0,L2_LINES_IN_ALL:PMC1 -M 1 -m ./a.out   

#include <omp.h>
#include <likwid.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
  printf("\n\nThis is a minimal example of how the likwid marker api works\n");

  // const char *filepath = "/tmp/likwid.out";

  // setenv("LIKWID_EVENTS", "FLOPS_DP", 1);
  // setenv("LIKWID_MODE", "1", 1);
  // setenv("LIKWID_FILEPATH", filepath, 1); // output filepath
  // setenv("LIKWID_THREADS", "0", 1); // list of threads
  // setenv("LIKWID_FORCE", "1", 1);

  likwid_markerInit();

  #pragma omp parallel
  {
    likwid_markerThreadInit();
  }

  double a, b, c;
  a = 1.8;
  b = 3.2;

  perfmon_startCounters();
  #pragma omp parallel
  {
  for (int j = 0; j < 4; j++)
  {
    printf("thread %d, iteration %d\n", omp_get_thread_num(), j);
    likwid_markerRegisterRegion("double_flops");
    likwid_markerStartRegion("double_flops");
    #pragma omp barrier
    for (int i = 0; i < 10000000; i++)
    {
      c = a * b + c;
    }
    #pragma omp barrier
    likwid_markerStopRegion("double_flops");
    likwid_markerNextGroup();
  }
  }
  perfmon_stopCounters();

  printf("final c: %f\n", c);

  // print results here if you want

  // calling both 'likwid_markerClose()' and 'perfmon_finalize()' causes
  // segfault, but I can call either twice in a row and be fine. This only
  // happens when running the program on its own. If wrapped in likwid-perfctr
  // (see comment block before inlcudes), this doesn't happen

  // likwid_markerClose();
  likwid_markerClose();
  // perfmon_finalize();
  // perfmon_finalize();
}
