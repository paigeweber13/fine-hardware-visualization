#include <cmath>
#include <iostream>
#include <omp.h>

#ifdef MANUAL_MEASUREMENT
#include <chrono>
#endif

#ifdef LIKWID_CLI
#include <likwid.h>
#endif

#ifdef FHV_PERFMON
#include <performance_monitor.h>
#include <likwid.h>
#endif


// 32 bytes to align with cache line
#define ALIGNMENT 32

typedef float** Image;
typedef float** Kernel;

Image make_image(std::uint64_t m, std::uint64_t n);
void destroy_image(Image image, std::uint64_t n);
Kernel make_kernel(unsigned k);
void destroy_kernel(Kernel kernel, unsigned k);
void convolve( Image input_image, Image output_image, std::uint64_t m,
  std::uint64_t n, Kernel kernel, unsigned k);

int main(int argc, char ** argv){
  if(argc < 5){
    printf("usage: %s m n k nbiter\n", argv[0]);
    return 1;
  }

  std::uint64_t m = std::stoi(argv[1]);
  std::uint64_t n = std::stoi(argv[2]);
  std::uint64_t k = std::stoi(argv[3]);
  std::uint64_t nbiter = std::stoi(argv[4]);

  auto input = make_image(m, n);
  auto output = make_image(m, n);
  auto kernel = make_kernel(k);

#ifdef MANUAL_MEASUREMENT
  auto start_time = std::chrono::high_resolution_clock::now();
#endif

#ifdef LIKWID_CLI
  likwid_markerInit();
  #pragma omp parallel
  {
    likwid_markerThreadInit();
    likwid_markerRegisterRegion("convolution");
    likwid_pinThread(omp_get_thread_num());
  }
#endif

#ifdef FHV_PERFMON
  // eventually replace with fhv_perfmon calls

  const char *filepath = performance_monitor::likwidOutputFilepath.c_str();

  // so 14 group/region combos
  setenv("LIKWID_EVENTS",
         "MEM|L2|L3|FLOPS_SP|FLOPS_DP|PORT_USAGE1|PORT_USAGE2|PORT_USAGE3",
         1);
  // setenv("LIKWID_EVENTS", "MEM_DP|L2", 1);
  setenv("LIKWID_MODE", "1", 1);
  // output filepath
  setenv("LIKWID_FILEPATH", filepath, 1); 
  setenv("LIKWID_THREADS", "0,1,2,3", 1); // list of threads
  setenv("LIKWID_FORCE", "1", 1);

  likwid_markerInit();
  #pragma omp parallel
  {
    likwid_markerThreadInit();
    likwid_markerRegisterRegion("convolution");
    likwid_pinThread(omp_get_thread_num());
  }
#endif

  for (size_t i = 0; i < static_cast<size_t>(nbiter); i++)
  {
    convolve(input, output, m, n, kernel, k);

#ifdef LIKWID_CLI
    likwid_markerNextGroup();
#endif
#ifdef FHV_PERFMON
    likwid_markerNextGroup();
#endif
  }

#ifdef MANUAL_MEASUREMENT
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
    end_time - start_time).count();

  const double nanoseconds_to_seconds = 1e-9;
  const double pixels_to_megapixels = 1e-6;

  double avg_duration_seconds = duration * nanoseconds_to_seconds/
    static_cast<double>(nbiter);
  double avg_megapixels_per_second = static_cast<double>(m*n) 
    * pixels_to_megapixels / avg_duration_seconds;

  printf("%10lu, %10lu, %2lu, %10.3f, %10.3f\n", m, n, k, avg_duration_seconds,
    avg_megapixels_per_second);
#endif

#ifdef LIKWID_CLI
  likwid_markerClose();
#endif

#ifdef FHV_PERFMON
  likwid_markerClose();

  performance_monitor::buildResultsMaps();
  // performance_monitor::printDetailedResults();
  // performance_monitor::printOnlyAggregate();

  performance_monitor::compareActualWithBench();
  // performance_monitor::printComparison();

  performance_monitor::printHighlights();

  performance_monitor::resultsToJson();
#endif

  destroy_image(input, n);
  destroy_image(output, n);
  destroy_kernel(kernel, k);
}

// ---- utility functions ---- //

Image make_image(std::uint64_t m, std::uint64_t n){
  Image image = (float**)aligned_alloc(ALIGNMENT, m * sizeof(float*));
  
  for (std::uint64_t i = 0; i < n; i++){
    image[i] = (float*)aligned_alloc(ALIGNMENT, n * sizeof(float));
  }

  return image;
}

void destroy_image(Image image, std::uint64_t n){
  for (std::uint64_t i = 0; i < n; i++){
    free(image[i]);
  }

  free(image);
}

Kernel make_kernel(unsigned k){
  Kernel kernel = (float**)aligned_alloc(ALIGNMENT, k * sizeof(float*));
  float kernel_value = 1.0/(k*k);
  
  for (unsigned i = 0; i < k; i++){
    kernel[i] = (float*)aligned_alloc(ALIGNMENT, k * sizeof(float));

    for (unsigned j = 0; j < k; j++){
      kernel[i][j] = kernel_value;
    }
  }

  return kernel;
}

void destroy_kernel(Kernel kernel, unsigned k){
  for (std::uint64_t i = 0; i < k; i++){
    free(kernel[i]);
  }

  free(kernel);
}

// actual convolution work
void 
convolve(
  Image input_image, 
  Image output_image, 
  std::uint64_t m,
  std::uint64_t n,
  Kernel kernel,
  unsigned k)
{
  unsigned kernel_midpoint = static_cast<int>(ceil(k/2));

    /* speedup:
        * "#pragma omp for collapse(2)"
          this linearizes the two for loops - combines these two for lopos
        * tiling will improve locality
        * unrolling loop will yield better performance
          for example:
          sum += image[i-1][j-1]
          sum += image[i-1][j]
          ...
          sum += image[i+1][i+1]
    */

  #pragma omp parallel
  {
    #ifdef LIKWID_CLI
    likwid_markerStartRegion("convolution");
    #endif
    #ifdef FHV_PERFMON
    // eventually, replace with perfmon call:

    // performance_monitor::startRegion("convolution");
    likwid_markerStartRegion("convolution");
    #endif

    float sum;
    // #pragma omp for collapse(2)
    #pragma omp for
    // height is m
    for(size_t y = kernel_midpoint; y < m-kernel_midpoint; y++){
      for(size_t x = kernel_midpoint; x < n-kernel_midpoint; x++){
        sum = 0;
        for(size_t o = 0; o < k; o++){
          for(size_t p = 0; p < k; p++){
            sum += input_image[y-kernel_midpoint+o][x-kernel_midpoint+p] 
              * kernel[o][p];
          }
        }
        output_image[y][x] = sum;
      }
    }

    #ifdef LIKWID_CLI
    likwid_markerStopRegion("convolution");
    #endif
    #ifdef FHV_PERFMON
    // eventually, replace with perfmon call:

    // performance_monitor::stopRegion("convolution");
    likwid_markerStopRegion("convolution");
    #endif
  }
}
