// compile with something like: 
// g++ -I/usr/local/papi-master/include -L/usr/local/papi-master/lib papi_minimal_hl.c -o papi_minimal_hl -lpapi

// run with something like:
// LD_LIBRARY_PATH=/usr/local/papi-master/lib PAPI_OUTPUT_DIRECTORY="data" PAPI_EVENTS="PAPI_DP_OPS,PAPI_L1_TCA" ./papi_minimal_hl

#include <papi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

// #define NUM_FLOPS 10000000
#define NUM_FLOPS 100000000
// #define NUM_COPIES 1000
#define NUM_COPIES 100000

void copy(double * arr, double * copy_arr, size_t n){
  for (size_t i = 0; i < n; i++){
    copy_arr[i] = arr[i];
  }
}

void handle_error(int retval){
   printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
   exit(1);
}

int main()
{
    int retval;


  double a, b, c;
  a = 1.8;
  b = 3.2;
  c = 0.0;

  size_t n = 2048;
  double arr[n];
  double copy_arr[n];

#pragma omp parallel
  {
    for (int j = 0; j < 8; j++)
    {
      if(omp_get_thread_num() == 0) printf("iteration %d\n", j);

    retval = PAPI_hl_region_begin("double_flops");
    if ( retval != PAPI_OK ) handle_error(1);

      if(omp_get_thread_num() == 0) printf("doing flops...\n");
      for (unsigned long long i = 0; i < NUM_FLOPS; i++)
      {
        c = a * b + c;
      }

    retval = PAPI_hl_region_end("double_flops");
    if ( retval != PAPI_OK ) handle_error(1);

    retval = PAPI_hl_region_begin("copy");
    if ( retval != PAPI_OK ) handle_error(1);

      if(omp_get_thread_num() == 0) printf("doing copy operations...\n");
      for (int i = 0; i < NUM_COPIES; i++){
        copy(arr, copy_arr, n);
      }

    retval = PAPI_hl_region_end("copy");
    if ( retval != PAPI_OK ) handle_error(1);
    }
  }

}

