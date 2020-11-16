/*
 * Example of how to compile: 
 * cc -I/usr/local/likwid-master/include likwid-topology-test.c -L/usr/local/likwid-master/lib -llikwid -fopenmp -o likwid-topology-test 
 *
 * Example of how to run: 
 * LD_LIBRARY_PATH=/usr/local/likwid-master/lib OMP_NUM_THREADS=2 GOMP_CPU_AFFINITY="3 0" ./likwid-topology-test
 * 
 */

#define _GNU_SOURCE

#include <omp.h>
#include <likwid.h>
#include <sched.h>
#include <stdio.h>

int main() {
  topology_init();
  CpuInfo_t cpu_info = get_cpuInfo();
  CpuTopology_t cpu_topology = get_cpuTopology();

  numa_init();
  int num_nodes = likwid_getNumberOfNodes();
  numa_finalize();

  printf("--- CPU INFO ---\n");
  printf("CPU name: %s\n", cpu_info->name);
  printf("CPU osname: %s\n", cpu_info->osname);
  printf("CPU short_name: %s\n", cpu_info->short_name);
  printf("CPU clock: %lu\n", cpu_info->clock);
  printf("\n");

  printf("--- CPU TOPOLOGY ---\n");
  printf("activeHWThreads: %u\n", cpu_topology->activeHWThreads);
  printf("numHWThreads: %u\n", cpu_topology->numHWThreads);
  printf("numSockets: %u\n", cpu_topology->numSockets);
  printf("numThreadsPerCore: %u\n", cpu_topology->numThreadsPerCore);
  printf("\n");

  printf("--- NUMA INFO ---\n");
  printf("num NUMA nodes: %d\n", num_nodes);
  printf("\n");

  printf("--- Number of currently used threads ---\n");
#pragma omp parallel
  {
    #pragma omp single
    {
      printf("omp_get_num_threads(): %u\n", omp_get_num_threads());
      printf("thread sound-off:\n");
    }
    printf("I am OpenMP thread %u, running on HW thread %u.\n", 
        omp_get_thread_num(), sched_getcpu());
  }
  printf("\n");

  // this call *must* come at the end of the file, or the pointers in the 
  // structs above will reference null.
  topology_finalize();
}

