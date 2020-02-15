#include <likwid.h>
#include <sched.h>

#include "../lib/computation_measurements.h"
#include "../lib/performance_monitor.h"

void swap_even_and_odd_cores(){
  performance_monitor perfmon;
  __m256 d;
  __m256 e;

  perfmon.init("FLOPS_SP");
  omp_set_num_threads(4);

#pragma omp parallel
  {
    perfmon.startRegion("flops");

    int original_core = sched_getcpu();
    int omp_thread = omp_get_thread_num();
    printf("I am thread %d, on core %d\n", omp_thread, original_core);

    // compute round 1
    if(omp_thread % 2 == 0){
      printf("Thread %d doing work on core %d\n", omp_thread, original_core);
      d = flops(10000000);
    }

    // migrate threads
    if(original_core % 2 == 0){
      likwid_pinThread(original_core + 1);
    } else {
      likwid_pinThread(original_core - 1);
    }

    int new_core = sched_getcpu();
    printf("I am thread %d. I was originally on core %d, I am now on core %d\n",
           omp_thread, original_core, new_core);

    // compute round 2
    if(omp_thread % 2 == 0){
      printf("Thread %d doing work on core %d\n", omp_thread, new_core);
      d += flops(10000000);
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
    perfmon.startRegion("flops");

    int omp_thread = omp_get_thread_num();
    likwid_pinThread(omp_thread);
    int original_core = sched_getcpu();
    printf("I am thread %d, on core %d\n", omp_thread, original_core);

    // compute round 1
    printf("Thread %d doing work on core %d\n", omp_thread, original_core);
    d = flops(10000000);

    // migrate threads
    likwid_pinThread(0);

    int new_core = sched_getcpu();
    printf("I am thread %d. I was originally on core %d, I am now on core %d\n",
           omp_thread, original_core, new_core);

    // compute round 2
    printf("Thread %d doing work on core %d\n", omp_thread, new_core);
    d += flops(10000000);

    perfmon.stopRegion("flops");
  }

  perfmon.close();
}

int main(int argc, char *argv[])
{
  printf("----- testing thread migration by swapping even and odd cores\n");
  swap_even_and_odd_cores();
  printf("\n\n");

  // running both in a row causes segfault, no matter the order
  printf("----- testing thread migration by moving all threads to one core\n");
  migrate_all_to_one();
  printf("\n\n");
}
