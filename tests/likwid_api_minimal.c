// based on "C-likwidAPI.c", available at:
// https://github.com/RRZE-HPC/likwid/blob/v5.0.1/examples/C-likwidAPI.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <likwid.h>

#define ERR_OK 0
#define NUM_GROUPS 2

// #define NUM_FLOPS 10000000
#define NUM_FLOPS 100000000
// #define NUM_COPIES 1000
#define NUM_COPIES 100000

typedef unsigned long long ull;

void do_copy(double *arr, double *copy_arr, size_t n, ull num_copies) {
  for (ull iter = 0; iter < num_copies; iter++) {
    for (size_t i = 0; i < n; i++) {
      copy_arr[i] = arr[i];
    }
  }
}

double do_flops(double a, double b, double c, ull num_flops) {
  for (ull i = 0; i < num_flops; i++) {
    c = a * b + c;
  }
  return c;
}

void handle_error(int error_code, char *message) {
  printf("ERROR: %s\n", message);
  printf("ERROR: got error code %d\n", error_code);
  perfmon_finalize();
  topology_finalize();
  affinity_finalize();
  exit(error_code);
}

int main(int argc, char *argv[]) {
  int i, j, k;
  int err;
  int *cpus;
  int gids[NUM_GROUPS];
  double result = 0.0;
  char * estrs[NUM_GROUPS];
  char l2_estr[] = "L2_LINES_IN_ALL:PMC0,L2_TRANS_L2_WB:PMC1";
  char dp_estr[] = "FP_ARITH_INST_RETIRED_SCALAR_DOUBLE:PMC0,"
    "FP_ARITH_INST_RETIRED_128B_PACKED_DOUBLE:PMC1,"
    "FP_ARITH_INST_RETIRED_256B_PACKED_DOUBLE:PMC2";
  estrs[0] = l2_estr;
  estrs[1] = dp_estr;
  char error_string[256];
  // perfmon_setVerbosity(3);

  // I think this is the only way to force likwid to override registers in use
  setenv("LIKWID_FORCE", "1", 1);

  // variables for computation/copy
  double a = 1.3, b = 1.2, c = 1.1;
  size_t n = 2048;
  double arr[n];
  double copy_arr[n];

  // Load the topology module and print some values.
  err = topology_init();
  if (err < ERR_OK)
    handle_error(err, "Failed to initialize LIKWID's topology module\n");

  // CpuInfo_t contains global information like name, CPU family, ...
  CpuInfo_t info = get_cpuInfo();
  // CpuTopology_t contains information about the topology of the CPUs.
  CpuTopology_t topo = get_cpuTopology();
  // Create affinity domains. Commonly only needed when reading Uncore
  // counters
  affinity_init();

  printf("Likwid example on a %s with %d CPUs\n", info->name,
         topo->numHWThreads);
  
  cpus = (int *)malloc(topo->numHWThreads * sizeof(int));
  if (!cpus) {
    sprintf(error_string, "malloc is unable to allocate %lu bytes of space",
            topo->numHWThreads * sizeof(int));
    handle_error(1, error_string);
  }

  for (i = 0; i < topo->numHWThreads; i++) {
    cpus[i] = topo->threadPool[i].apicId;
  }

  // Must be called before perfmon_init() but only if you want to use another
  // access mode as the pre-configured one. For direct access (0) you have to
  // be root.
  // accessClient_setaccessmode(0);

  // Initialize the perfmon module.
  err = perfmon_init(topo->numHWThreads, cpus);
  if (err < ERR_OK)
    handle_error(
        err, "Failed to initialize LIKWID's performance monitoring module\n");

  for (i = 0; i < NUM_GROUPS; i++){
    // Add eventset string for L2 counters to the perfmon module.
    gids[i] = perfmon_addEventSet(estrs[i]);
    if (gids[i] < 0) {
      sprintf(error_string,
              "Failed to add event string %s to LIKWID's performance monitoring "
              "module",
              estrs[i]);
      handle_error(gids[i], error_string);
    }

  // Setup the eventset identified by group ID (gid).
    err = perfmon_setupCounters(gids[i]);
    if (err < ERR_OK) {
      sprintf(
        error_string,
        "Failed to setup group %d in LIKWID's performance monitoring module\n",
        gids[i]);
      handle_error(err, error_string);
    }

    // Start all counters in the current event set
    err = perfmon_startCounters();
    if (err < ERR_OK) {
      sprintf(error_string, 
        "Failed to start counters for group %d for thread %d\n", gids[i],
        (-1 * err) - 1);
      handle_error(err, error_string);
    }

    // ---- Perform something ---- //
    printf("measuring group %d, %s\n", gids[i], perfmon_getGroupName(gids[i]));
    c = do_flops(a, b, c, NUM_FLOPS);
    do_copy(arr, copy_arr, n, NUM_COPIES);
    printf("c: %f\n", c);
    printf("copy_arr[c %% n]: %f\n", copy_arr[((size_t)c) % n]);

    // Stop all counters in the previously started event set.
    err = perfmon_stopCounters();
    if (err < ERR_OK) {
      sprintf(error_string, 
        "Failed to stop counters for group %d for thread %d\n", gids[i],
        (-1 * err) - 1);
      handle_error(err, error_string);
    }
  }

  for (k = 0; k < NUM_GROUPS; k++){
    // Print the result of every thread/CPU for all events in estr.
    char *ptr = strtok(estrs[k], ",");
    j = 0;
    while (ptr != NULL) {
      for (i = 0; i < topo->numHWThreads; i++) {
        result = perfmon_getResult(gids[0], j, i);
        printf("Measurement result for event set %s at CPU %d: %f\n", ptr,
               cpus[i], result);
      }
      ptr = strtok(NULL, ",");
      j++;
    }
  }

  free(cpus);
  // Uninitialize the perfmon module.
  perfmon_finalize();
  affinity_finalize();
  // Uninitialize the topology module.
  topology_finalize();
  return 0;
}
