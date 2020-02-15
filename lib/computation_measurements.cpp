#include "computation_measurements.h"

__m256 flops(std::uint64_t num_iterations){
  __m256 a = _mm256_setr_ps (1., 2., 3., 4., 5., 6., 7., 8.);
  __m256 b = _mm256_setr_ps (10., 20., 30., 40., 50., 60., 70., 80.);
  __m256 c = _mm256_setr_ps (11., 21., 31., 41., 51., 61., 71., 81.);

  __m256 d = _mm256_setr_ps (77., 27., 37., 47., 57., 67., 77., 87.);
  __m256 e = _mm256_setr_ps (88., 28., 38., 48., 58., 68., 78., 88.);
  __m256 f = _mm256_setr_ps (99., 29., 39., 49., 59., 69., 79., 89.);

  __m256 g = _mm256_setr_ps (22., 22., 32., 42., 52., 62., 72., 82.);
  __m256 h = _mm256_setr_ps (33., 23., 33., 43., 53., 63., 73., 83.);
  __m256 j = _mm256_setr_ps (44., 24., 34., 44., 54., 64., 74., 84.);

  __m256 k = _mm256_setr_ps (55., 25., 35., 45., 55., 65., 75., 85.);
  __m256 l = _mm256_setr_ps (66., 26., 36., 46., 56., 66., 76., 86.);
  __m256 m = _mm256_setr_ps (1111., 211., 311., 411., 511., 611., 711., 811.);

  for (std::uint64_t i = 0; i < num_iterations; i++){
    // operations per loop iteration: 64
    // do four things per loop to allow for the processor to pipeline 
    // instructions
    c = _mm256_fmadd_ps (a, b, c); // 2 ops on 8 floats
    f = _mm256_fmadd_ps (d, e, f); // 2 more ops on 8 floats
    j = _mm256_fmadd_ps (g, h, j); // 2 more ops on 8 floats
    m = _mm256_fmadd_ps (k, l, m); // 2 more ops on 8 floats
  }

  // return whatever is computed
  c = _mm256_fmadd_ps(f, j, c);
  a = _mm256_fmadd_ps(c, m, a);
  return a;
}

__m256i iops(std::uint64_t num_iterations){
  __m256i a = _mm256_setr_epi8 (1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
                                1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
                                1, 2, 3, 4, 1, 2, 3, 4);
  __m256i b = _mm256_setr_epi8 (1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
                                1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
                                1, 2, 3, 4, 1, 2, 3, 4);
  __m256i c = _mm256_setr_epi8 (1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
                                1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
                                1, 2, 3, 4, 1, 2, 3, 4);
  __m256i d = _mm256_setr_epi8 (1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
                                1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
                                1, 2, 3, 4, 1, 2, 3, 4);
  __m256i e = _mm256_setr_epi8 (1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
                                1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
                                1, 2, 3, 4, 1, 2, 3, 4);
  __m256i f = _mm256_setr_epi8 (1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
                                1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
                                1, 2, 3, 4, 1, 2, 3, 4);

  for (std::uint64_t i = 0; i < num_iterations; i++){
    // operations per iteration: 80

    a = _mm256_add_epi8 (a, b); // 32 adds
    c = _mm256_add_epi8 (c, d); // 32 adds
    e = _mm256_add_epi8 (e, f); // 32 adds

  }

  return a;
}