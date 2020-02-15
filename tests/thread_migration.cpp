#include <likwid.h>
#include <sched.h>

#include "../lib/computation_measurements.h"
#include "../lib/performance_monitor.h"

// starting/stopping regions seems to cause everything to get reported on
// threads 0 and 1

// leaving whole parallel loop as one region seems to report by hardware thread...
void migrate_cores_12_to_cores_34(){
  performance_monitor perfmon;
  __m256 d;
  __m256 e;

  perfmon.init("FLOPS_SP");
  omp_set_num_threads(4);
  printf("here, only omp threads 0 and 1 are allowed to do work\n");

#pragma omp parallel
  {
    perfmon.startRegion("flops");

    int original_core = sched_getcpu();
    int omp_thread = omp_get_thread_num();
    // printf("I am thread %d, on core %d\n", omp_thread, original_core);

    // compute round 1
    if(omp_thread == 0 || omp_thread == 1){
      printf("Thread %d doing work on core %d\n", omp_thread, original_core);
      // perfmon.startRegion("flops");
      d = flops(10000000);
      // perfmon.stopRegion("flops");
    }

    // migrate threads
    if(original_core == 0 || original_core == 1){
      likwid_pinThread(original_core + 2);
    } else {
      likwid_pinThread(original_core - 2);
    }

    int new_core = sched_getcpu();
    // printf("I am thread %d. I was originally on core %d, I am now on core %d\n",
    //        omp_thread, original_core, new_core);

    // compute round 2
    if(omp_thread == 0 || omp_thread == 1){
      printf("Thread %d doing work on core %d\n", omp_thread, new_core);
      // perfmon.startRegion("flops");
      e = flops(10000000);
      // perfmon.stopRegion("flops");
    }

    perfmon.stopRegion("flops");
  }

  perfmon.close();
}

void migrate_all_to_one(){
  performance_monitor perfmon;
  __m256 d;
  __m256 e;

  perfmon.init("FLOPS_SP");
  omp_set_num_threads(4);

#pragma omp parallel
  {
    int omp_thread = omp_get_thread_num();
    likwid_pinThread(omp_thread);
    int original_core = sched_getcpu();
    printf("I am thread %d, on core %d\n", omp_thread, original_core);

    // compute round 1
    printf("Thread %d doing work on core %d\n", omp_thread, original_core);
    perfmon.startRegion("flops");
    d = flops(10000000);
    perfmon.stopRegion("flops");

    // migrate threads
    likwid_pinThread(0);

    int new_core = sched_getcpu();
    printf("I am thread %d. I was originally on core %d, I am now on core %d\n",
           omp_thread, original_core, new_core);

    // compute round 2
    printf("Thread %d doing work on core %d\n", omp_thread, new_core);
    perfmon.startRegion("flops");
    d += flops(10000000);
    perfmon.stopRegion("flops");
  }

  perfmon.close();
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    printf("usage: %s 0|1\n", argv[0]);
    printf("0 will run test that swaps even and odd cores\n");
    printf("1 will run test that starts with 1 thread per core and then \n"
           "moves all threads to one core\n");
  }

  if (std::stoi(argv[1]) == 0)
  {
    printf("----- testing thread migration by swapping even and odd cores\n");
    migrate_cores_12_to_cores_34();
    printf("\n\n");
  }
  else
  {
    printf("----- testing thread migration by moving all threads to one core\n");
    migrate_all_to_one();
    printf("\n\n");
  }
}
