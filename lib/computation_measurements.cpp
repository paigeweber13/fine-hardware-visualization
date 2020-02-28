#include "computation_measurements.h"

__m256 flops(std::uint64_t num_iterations)
{
  __m256 a = _mm256_setr_ps(1., 2., 3., 4., 5., 6., 7., 8.);
  __m256 b = _mm256_setr_ps(10., 20., 30., 40., 50., 60., 70., 80.);
  __m256 c = _mm256_setr_ps(11., 21., 31., 41., 51., 61., 71., 81.);

  __m256 d = _mm256_setr_ps(77., 27., 37., 47., 57., 67., 77., 87.);
  __m256 e = _mm256_setr_ps(88., 28., 38., 48., 58., 68., 78., 88.);
  __m256 f = _mm256_setr_ps(99., 29., 39., 49., 59., 69., 79., 89.);

  __m256 g = _mm256_setr_ps(22., 22., 32., 42., 52., 62., 72., 82.);
  __m256 h = _mm256_setr_ps(33., 23., 33., 43., 53., 63., 73., 83.);
  __m256 j = _mm256_setr_ps(44., 24., 34., 44., 54., 64., 74., 84.);

  __m256 k = _mm256_setr_ps(55., 25., 35., 45., 55., 65., 75., 85.);
  __m256 l = _mm256_setr_ps(66., 26., 36., 46., 56., 66., 76., 86.);
  __m256 m = _mm256_setr_ps(1111., 211., 311., 411., 511., 611., 711., 811.);

  for (std::uint64_t i = 0; i < num_iterations; i++)
  {
    // operations per loop iteration: 64
    // do four things per loop to allow for the processor to pipeline
    // instructions
    c = _mm256_fmadd_ps(a, b, c); // 2 ops on 8 floats
    f = _mm256_fmadd_ps(d, e, f); // 2 more ops on 8 floats
    j = _mm256_fmadd_ps(g, h, j); // 2 more ops on 8 floats
    m = _mm256_fmadd_ps(k, l, m); // 2 more ops on 8 floats
  }

  // return whatever is computed
  c = _mm256_fmadd_ps(f, j, c);
  a = _mm256_fmadd_ps(c, m, a);

  return a;
}

void bandwidth_rw(std::uint64_t num_iterations, std::uint64_t size_kib)
{
  // reduction(max:ticks) previously at the end of this pragma

  // unsigned thr_num;
  std::uint64_t i, j;
  __m256d buffer;
  performance_monitor perfmon;

  const char * tag = "L2";
  // align to cache line, which is 512 bits or 64 bytes
  double * array = static_cast<double *>(
    aligned_alloc(64, size_kib * KILO_BYTE_SIZE));
  double * copy_array = static_cast<double *>(
    aligned_alloc(64, size_kib * KILO_BYTE_SIZE));

  std::uint64_t num_doubles = size_kib * KILO_BYTE_SIZE/BYTES_PER_DOUBLE;

#pragma omp parallel default(shared) private(buffer, i, j)
  {

    // thr_num = omp_get_thread_num();
    for (i = 0; i < num_iterations; i++)
    {
      //Get time snapshot just for one iteration
      if (i == num_iterations / 2)
      {
        //	start = system_clock::now();
        //	Maybe start likwid region here
        //    printf("likwid start region %s on thread %d\n", bw->mark_tag, omp_get_thread_num());
        perfmon.startRegion(tag);
      }
      for (int k = 0; k < 100; k++)
        for (j = 0; j < num_doubles; j += DOUBLES_PER_VECTOR)
        {
          // Loading 256-bits into memory address of array
          buffer = _mm256_load_pd(array + j);
          // Storing 256-bits from buffer into address of cpy_arr
          _mm256_store_pd(copy_array + j, buffer);
        }
      //Get time snapshot just for one iteration
      if (i == num_iterations / 2)
      {
        //	end = system_clock::now();
        //	Maybe stop likwid regin here
        //    printf("likwid stop region %s on thread %d\n", bw->mark_tag, omp_get_thread_num());
        perfmon.stopRegion(tag);
      }
      asm(""); //Say no to loop optimization
    }
  }

  free(array);
  free(copy_array);
}
