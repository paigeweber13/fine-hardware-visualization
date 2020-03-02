#include <likwid.h>
#include <sched.h>

#include "../lib/computation_measurements.h"
#include "../lib/performance_monitor.h"

// starting/stopping regions seems to cause everything to get reported on
// threads 0 and 1

// leaving whole parallel loop as one region seems to report by hardware thread...
void migrate_cores_12_to_cores_34(){
  __m256 d;
  __m256 e;

  performance_monitor::init("FLOPS_SP");
  omp_set_num_threads(4);
  printf("here, only omp threads 0 and 1 are allowed to do work\n");

#pragma omp parallel
  {
    performance_monitor::startRegion("flops");
    #pragma omp barrier

    int original_core = sched_getcpu();
    int omp_thread = omp_get_thread_num();
    // printf("I am thread %d, on core %d\n", omp_thread, original_core);

    // compute round 1
    if(omp_thread == 0 || omp_thread == 1){
      printf("Thread %d doing work on core %d\n", omp_thread, original_core);
      // performance_monitor::startRegion("flops");
      d = flops(10000000);
      // performance_monitor::stopRegion("flops");
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
      // performance_monitor::startRegion("flops");
      e = flops(10000000);
      // performance_monitor::stopRegion("flops");
    }

    #pragma omp barrier
    performance_monitor::stopRegion("flops");
  }

  performance_monitor::close();
  performance_monitor::printResults();
}

void migrate_thread_core_0_to_2()
{
  __m256 d;
  __m256 e;

  performance_monitor::init("FLOPS_SP");
  printf("here, one thread is migrated from core 0 to core 2\n");

#pragma omp parallel
{
  performance_monitor::startRegion("flops");
}

  likwid_pinThread(0);
  d = flops(10000000);

  likwid_pinThread(2);
  e = flops(10000000);

#pragma omp parallel
{
  performance_monitor::stopRegion("flops");
}

  performance_monitor::close();
  performance_monitor::printResults();
}

void migrate_all_to_one(){
  __m256 d;
  __m256 e;

  performance_monitor::init("FLOPS_SP");
  omp_set_num_threads(4);

#pragma omp parallel
  {
    performance_monitor::startRegion("flops");
    // #pragma omp barrier

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

    #pragma omp barrier
    performance_monitor::stopRegion("flops");
  }

  performance_monitor::close();
  performance_monitor::printResults();
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    printf("usage: %s 0|1|2\n", argv[0]);
    printf("0 will run test that moves from cores 0,1 to cores 2,3\n");
    printf("1 will run test that moves thread from core 0 to core 2\n");
    printf("2 will run test that starts with 1 thread per core and then \n"
           "moves all threads to one core\n");
  }

  if (std::stoi(argv[1]) == 0)
  {
    printf("----- testing thread migration by moving from cores 0,1 to cores 2,3\n");
    migrate_cores_12_to_cores_34();
    printf("\n\n");
  }
  else if (std::stoi(argv[1]) == 1)
  {
    printf("----- testing thread migration by moving from core 0 to 2\n");
    migrate_thread_core_0_to_2();
    printf("\n\n");
  }
  else
  {
    printf("----- testing thread migration by moving all threads to one core\n");
    migrate_all_to_one();
    printf("\n\n");
  }
}
