#include "computation_measurements.h"

__m256 flops(std::uint64_t num_iterations){
  // float* a = static_cast<float*>(aligned_alloc(32, 32));
  // float* b = static_cast<float*>(aligned_alloc(32, 32));
  // float* c = static_cast<float*>(aligned_alloc(32, 32));

  // __m256 reg_a  = _mm256_load_ps(a);
  // __m256 reg_b  = _mm256_load_ps(b);
  // __m256 reg_c  = _mm256_load_ps(c);

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

  __m256 n = _mm256_setr_ps (1212., 212., 312., 412., 512., 612., 712., 812.);
  __m256 o = _mm256_setr_ps (1313., 213., 313., 413., 513., 613., 713., 813.);
  __m256 p = _mm256_setr_ps (1414., 214., 314., 414., 514., 614., 714., 814.);

  // __m256 q = _mm256_setr_ps (1515., 215., 315., 415., 515., 615., 715., 815.);
  // __m256 r = _mm256_setr_ps (1616., 216., 316., 416., 516., 616., 716., 816.);
  // __m256 s = _mm256_setr_ps (1717., 217., 317., 417., 517., 617., 717., 817.);

  // __m256 t = _mm256_setr_ps (1818., 218., 318., 418., 518., 618., 718., 818.);
  // __m256 u = _mm256_setr_ps (1919., 219., 319., 419., 519., 619., 719., 819.);
  // __m256 v = _mm256_setr_ps (2020., 220., 320., 420., 520., 620., 720., 820.);

  for (std::uint64_t i = 0; i < num_iterations; i++){
    // operations per loop iteration
    // d = _mm256_fmadd_ps (reg_a, reg_b, reg_c);
    c = _mm256_fmadd_ps (a, b, c); // 2 ops on 8 floats
    f = _mm256_fmadd_ps (d, e, f); // 2 more ops on 8 floats
    j = _mm256_fmadd_ps (g, h, j); // 2 more ops on 8 floats
    m = _mm256_fmadd_ps (k, l, m); // 2 more ops on 8 floats
    p = _mm256_fmadd_ps (n, o, p); // 2 more ops on 8 floats
  }

  // delete a;
  // delete b;
  // delete c;
  
  // return whatever is computed
  c = _mm256_fmadd_ps(f, j, c);
  b = _mm256_fmadd_ps(p, m, b);
  a = _mm256_fmadd_ps(c, b, a);
  return a;
}

__m256i iops(std::uint64_t num_iterations){
  __m256i a = _mm256_setr_epi8 (1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
                                1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
                                1, 2, 3, 4, 1, 2, 3, 4);
  __m256i b = _mm256_setr_epi8 (1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
                                1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
                                1, 2, 3, 4, 1, 2, 3, 4);
  // __m256i c = _mm256_setr_epi8 (1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
  //                               1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
  //                               1, 2, 3, 4, 1, 2, 3, 4);
  // __m256i d = _mm256_setr_epi8 (1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
  //                               1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
  //                               1, 2, 3, 4, 1, 2, 3, 4);
  // __m256i e = _mm256_setr_epi8 (1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
  //                               1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
  //                               1, 2, 3, 4, 1, 2, 3, 4);
  // __m256i f = _mm256_setr_epi8 (1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
  //                               1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
  //                               1, 2, 3, 4, 1, 2, 3, 4);
  // __m256i g = _mm256_setr_epi8 (1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
  //                               1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
  //                               1, 2, 3, 4, 1, 2, 3, 4);

  for (std::uint64_t i = 0; i < num_iterations; i++){
    // operations per iteration: 80
    a = _mm256_add_epi8 (a, b); // 32 adds
    a = _mm256_add_epi8 (a, b); // 32 adds
    a = _mm256_mullo_epi16 (a, b); // 16 muls
  }

  return a;
}