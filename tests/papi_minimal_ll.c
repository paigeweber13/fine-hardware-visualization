// compile with something like:
// g++ -I/usr/local/papi-master/include -L/usr/local/papi-master/lib papi_minimal_hl.c -o papi_minimal_hl -lpapi

// run with something like:
// LD_LIBRARY_PATH=/usr/local/papi-master/lib PAPI_OUTPUT_DIRECTORY="data" PAPI_EVENTS="PAPI_DP_OPS,PAPI_L1_TCA" ./papi_minimal_hl

#include <omp.h>
#include <papi.h>
#include <stdio.h>
#include <stdlib.h>

// #define NUM_FLOPS 10000000
#define NUM_FLOPS 100000000
// #define NUM_COPIES 1000
#define NUM_COPIES 100000

typedef unsigned long long ull;

void do_copy(double *arr, double *copy_arr, size_t n, ull num_copies) {
  printf("doing copy!\n");
  for (ull iter = 0; iter < num_copies; iter++) {
    for (size_t i = 0; i < n; i++) {
      copy_arr[i] = arr[i];
    }
  }
}

double do_flops(double a, double b, double c, ull num_flops) {
  printf("doing flops!\n");
  for (ull i = 0; i < num_flops; i++) {
    c = a * b + c;
  }
  return c;
}

void handle_error(int retval, char * custom_text) {
  fprintf(stderr, "PAPI error %d: %s. %s\n", 
    retval, PAPI_strerror(retval), custom_text);
  exit(retval);
}

void print_values(long_long * values, size_t n){
  for(size_t i = 0; i < n; i++){
    printf("values[%lu]: %lld\n", i, values[i]);
  }
}

int main() {
  int retval, ComputationEventSet = PAPI_NULL, MemEventSet = PAPI_NULL;
  size_t num_values = 4;
  long_long values[num_values];

  for(size_t i = 0; i < num_values; i++){
    values[i] = 0;
  }

  double a, b, c;
  a = 1.8;
  b = 3.2;
  c = 0.0;

  size_t n = 2048;
  double arr[n];
  double copy_arr[n];

  /* Initialize the PAPI library */
  retval = PAPI_library_init(PAPI_VER_CURRENT);
  if (retval != PAPI_VER_CURRENT && retval > 0) {
    handle_error(retval, "PAPI library version mismatch!\n");
  }
  if (retval < 0) {
    handle_error(retval, "PAPI library initialization error!\n");
  }

  /* Create the Event Set */
  retval = PAPI_create_eventset(&ComputationEventSet);
  if (retval != PAPI_OK) handle_error(retval, "While creating event set ComputationEventSet");

  /* Add Total Instructions Executed to our Event Set */
  retval = PAPI_add_event(ComputationEventSet, PAPI_TOT_INS);
  if (retval != PAPI_OK) handle_error(retval, "While adding PAPI_TOT_INS");

  retval = PAPI_add_event(ComputationEventSet, PAPI_DP_OPS);
  if (retval != PAPI_OK) handle_error(retval, "While adding PAPI_DP_OPS");

  retval = PAPI_create_eventset(&MemEventSet);
  if (retval != PAPI_OK) handle_error(retval, "While creating event set MemEventSet");

  retval = PAPI_add_event(MemEventSet, PAPI_L2_DCA);
  if (retval != PAPI_OK) handle_error(retval, "While adding PAPI_L2_DCA");

  retval = PAPI_add_event(MemEventSet, PAPI_L3_DCA);
  if (retval != PAPI_OK) handle_error(retval, "While adding PAPI_L3_DCA");

  retval = PAPI_add_event(MemEventSet, PAPI_MEM_WCY);
  if (retval != PAPI_OK) handle_error(retval, "While adding PAPI_MEM_WCY");

  // ========= FLOPS ========= //
  printf("\nFLOPS SECTION\n");
  /* Start counting events in the Event Set */
  retval = PAPI_start(ComputationEventSet);
  if (retval != PAPI_OK) handle_error(retval, "While starting ComputationEventSet");

  /* Defined in tests/do_loops.c in the PAPI source distribution */
  do_flops(a, b, c, NUM_FLOPS);

  /* Read the counting events in the Event Set */
  // does not reset counters!
  retval = PAPI_read(ComputationEventSet, values);
  if (retval != PAPI_OK) handle_error(retval, "While reading ComputationEventSet");

  printf("After reading the counters:\n");
  print_values(values, num_values);

  /* Reset the counting events in the Event Set */
  retval = PAPI_reset(ComputationEventSet);
  if (retval != PAPI_OK) handle_error(retval, "While reseting ComputationEventSet");

  do_flops(a, b, c, NUM_FLOPS);

  retval = PAPI_read(ComputationEventSet, values);
  if (retval != PAPI_OK) handle_error(retval, "While reading ComputationEventSet");

  printf("After reading the counters, a second time:\n");
  print_values(values, num_values);
  printf("Not doing a reset!\n");

  /* Add the counters in the Event Set */
  // this will report everything since start
  // also, resets the counters in the case that you do another read or stop

  retval = PAPI_accum(ComputationEventSet, values);
  if (retval != PAPI_OK) handle_error(retval, "While running PAPI_accum on ComputationEventSet");

  printf("After adding the counters with PAPI_accum:\n");
  print_values(values, num_values);

  do_flops(a, b, c, NUM_FLOPS);

  /* Stop the counting of events in the Event Set */
  retval = PAPI_stop(ComputationEventSet, values);
  if (retval != PAPI_OK) handle_error(retval, "While stopping ComputationEventSet");

  printf("After stopping the counters:\n");
  print_values(values, num_values);

  // ========= MEM ========= //
  printf("\nMEM SECTION\n");
  /* Start counting events in the Event Set */
  retval = PAPI_start(MemEventSet);
  if (retval != PAPI_OK) handle_error(retval, "While starting MemEventSet");

  /* Defined in tests/do_loops.c in the PAPI source distribution */
  do_copy(arr, copy_arr, n, NUM_COPIES);

  /* Read the counting events in the Event Set */
  retval = PAPI_read(MemEventSet, values);
  if (retval != PAPI_OK) handle_error(retval, "While reading MemEventSet");

  printf("After reading the counters:\n");
  print_values(values, num_values);

  /* Reset the counting events in the Event Set */
  retval = PAPI_reset(MemEventSet);
  if (retval != PAPI_OK) handle_error(retval, "While resetting MemEventSet");

  do_copy(arr, copy_arr, n, NUM_COPIES);

  /* Add the counters in the Event Set */
  retval = PAPI_accum(MemEventSet, values);
  if (retval != PAPI_OK) handle_error(retval, "While runing PAPI_accum on MemEventSet");

  printf("After adding the counters:\n");
  print_values(values, num_values);

  do_copy(arr, copy_arr, n, NUM_COPIES);

  /* Stop the counting of events in the Event Set */
  retval = PAPI_stop(MemEventSet, values);
  if (retval != PAPI_OK) handle_error(retval, "While stopping MemEventSet");

  printf("After stopping the counters:\n");
  print_values(values, num_values);

  return 0;
}
