// compile with something like:
// g++ -I/usr/local/papi-master/include -L/usr/local/papi-master/lib papi_minimal_ll.c -o papi_minimal_ll -fopenmp -pthread -lpapi

// run with something like:
// LD_LIBRARY_PATH=/usr/local/papi-master/lib ./papi_minimal_ll

// adding `GOMP_CPU_AFFINITY="0-3"` before call to compiled program may improve
// stability by pinning threads... but this hasn't been tested yet.

// based closely on example code found at
// https://bitbucket.org/icl/papi/wiki/PAPI-LL.md 

#include <omp.h>
#include <papi.h>
#include <pthread.h>
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
  int retval;

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

#pragma omp parallel
{
  unsigned long tid;
  int thread_retval;
  int ComputationEventSet = PAPI_NULL;
  int MemEventSet = PAPI_NULL;
  int event_code = 0x0;
  size_t num_values = 4;
  long_long values[num_values];

  for(size_t i = 0; i < num_values; i++){
    values[i] = 0;
  }

  /* initialize multithreading for PAPI */
  thread_retval = PAPI_thread_init(pthread_self);
  if (thread_retval != PAPI_OK) handle_error(thread_retval, "While initializing threads");
  if ((tid = PAPI_thread_id()) == (unsigned long int)-1) 
    handle_error(1, "Got a bad thread id");

  /* Create the Event Set */
  thread_retval = PAPI_create_eventset(&ComputationEventSet);
  if (thread_retval != PAPI_OK) handle_error(thread_retval, "While creating event set ComputationEventSet");

  /* Add Total Instructions Executed to our Event Set */
  thread_retval = PAPI_add_event(ComputationEventSet, PAPI_TOT_INS);
  if (thread_retval != PAPI_OK) handle_error(thread_retval, "While adding PAPI_TOT_INS");

  thread_retval = PAPI_add_event(ComputationEventSet, PAPI_DP_OPS);
  if (thread_retval != PAPI_OK) handle_error(thread_retval, "While adding PAPI_DP_OPS");
  
  // FP_ARITH_INST_RETIRED.SCALAR_DOUBLE
  // event num: C7H
  // umask value: 01H

  // event_code = 0xC7;
  // thread_retval = PAPI_enum_event(&event_code, 0x01);
  // if (thread_retval != PAPI_OK) handle_error(thread_retval, "While running PAPI_enum_event for 0xC7 and 0x01");

  // thread_retval = PAPI_add_event(ComputationEventSet, event_code);
  // if (thread_retval != PAPI_OK) handle_error(thread_retval, "While running PAPI_enum_event for 0xC7 and 0x01");

  thread_retval = PAPI_create_eventset(&MemEventSet);
  if (thread_retval != PAPI_OK) handle_error(thread_retval, "While creating event set MemEventSet");

  thread_retval = PAPI_add_event(MemEventSet, PAPI_L2_DCA);
  if (thread_retval != PAPI_OK) handle_error(thread_retval, "While adding PAPI_L2_DCA");

  thread_retval = PAPI_add_event(MemEventSet, PAPI_L3_DCA);
  if (thread_retval != PAPI_OK) handle_error(thread_retval, "While adding PAPI_L3_DCA");

  thread_retval = PAPI_add_event(MemEventSet, PAPI_MEM_WCY);
  if (thread_retval != PAPI_OK) handle_error(thread_retval, "While adding PAPI_MEM_WCY");

  // ========= FLOPS ========= //
  printf("\nFLOPS SECTION\n");
  /* Start counting events in the Event Set */
  thread_retval = PAPI_start(ComputationEventSet);
  if (thread_retval != PAPI_OK) handle_error(thread_retval, "While starting ComputationEventSet");

  /* Defined in tests/do_loops.c in the PAPI source distribution */
  do_flops(a, b, c, NUM_FLOPS);

  /* Read the counting events in the Event Set */
  // does not reset counters!
  thread_retval = PAPI_read(ComputationEventSet, values);
  if (thread_retval != PAPI_OK) handle_error(thread_retval, "While reading ComputationEventSet");

  printf("Thread %lu, After reading the counters:\n", tid);
  print_values(values, num_values);

  /* Reset the counting events in the Event Set */
  thread_retval = PAPI_reset(ComputationEventSet);
  if (thread_retval != PAPI_OK) handle_error(thread_retval, "While reseting ComputationEventSet");

  do_flops(a, b, c, NUM_FLOPS);

  thread_retval = PAPI_read(ComputationEventSet, values);
  if (thread_retval != PAPI_OK) handle_error(thread_retval, "While reading ComputationEventSet");

  printf("Thread %lu, After reading the counters, a second time:\n", tid);
  print_values(values, num_values);
  printf("Not doing a reset!\n");

  /* Add the counters in the Event Set */
  // this will report everything since start
  // also, resets the counters in the case that you do another read or stop

  thread_retval = PAPI_accum(ComputationEventSet, values);
  if (thread_retval != PAPI_OK) handle_error(thread_retval, "While running PAPI_accum on ComputationEventSet");

  printf("Thread %lu, After adding the counters with PAPI_accum:\n", tid);
  print_values(values, num_values);

  do_flops(a, b, c, NUM_FLOPS);

  /* Stop the counting of events in the Event Set */
  thread_retval = PAPI_stop(ComputationEventSet, values);
  if (thread_retval != PAPI_OK) handle_error(thread_retval, "While stopping ComputationEventSet");

  printf("Thread %lu, after stopping the counters:\n", tid);
  print_values(values, num_values);

  // ========= MEM ========= //
  printf("\nMEM SECTION\n");
  /* Start counting events in the Event Set */
  thread_retval = PAPI_start(MemEventSet);
  if (thread_retval != PAPI_OK) handle_error(thread_retval, "While starting MemEventSet");

  /* Defined in tests/do_loops.c in the PAPI source distribution */
  do_copy(arr, copy_arr, n, NUM_COPIES);

  /* Read the counting events in the Event Set */
  thread_retval = PAPI_read(MemEventSet, values);
  if (thread_retval != PAPI_OK) handle_error(thread_retval, "While reading MemEventSet");

  printf("Thread %lu, after reading the counters:\n", tid);
  print_values(values, num_values);

  /* Reset the counting events in the Event Set */
  thread_retval = PAPI_reset(MemEventSet);
  if (thread_retval != PAPI_OK) handle_error(thread_retval, "While resetting MemEventSet");

  do_copy(arr, copy_arr, n, NUM_COPIES);

  /* Add the counters in the Event Set */
  thread_retval = PAPI_accum(MemEventSet, values);
  if (thread_retval != PAPI_OK) handle_error(thread_retval, "While runing PAPI_accum on MemEventSet");

  printf("Thread %lu, after adding the counters:\n", tid);
  print_values(values, num_values);

  do_copy(arr, copy_arr, n, NUM_COPIES);

  /* Stop the counting of events in the Event Set */
  thread_retval = PAPI_stop(MemEventSet, values);
  if (thread_retval != PAPI_OK) handle_error(thread_retval, "While stopping MemEventSet");

  printf("Thread %lu, after stopping the counters:\n", tid);
  print_values(values, num_values);
}

  return 0;
}
