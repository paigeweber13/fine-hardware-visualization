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


float polynomial (float x, float* poly, int degree) {
  float out = 0.;
  float xtothepowerof = 1.;
  for (int i=0; i<=degree; ++i) {
    out += xtothepowerof*poly[i];
    xtothepowerof *= x;
  }
  return out;
}

template <int BS>
void polynomial_block (float* xin, float* poly, int degree, float* writeout) {

  float x[BS];
  for (int j=0; j<BS; ++j) {
    x[j] = xin[j];
  }
  
       float out[BS] = {0.};
       float xtothepowerof[BS] = {1.};
  for (int i=0; i<=degree; ++i) {
#pragma unroll BS
    for (int j=0; j<BS; ++j) {
      out[j] += xtothepowerof[j]*poly[i];
      xtothepowerof[j] *= x[j];
    }
  }

  for (int j=0; j<BS; ++j) {
    writeout[j] = out[j];
  }
}


void polynomial_expansion (float* poly, int degree,
			   int n, float* array) {

#define UNROLL 64

  //assumes n is divisible by UNROLL

#pragma omp parallel
  {
  #ifdef LIKWID_CLI
    likwid_markerStartRegion("poly_block");
  #endif
  #ifdef FHV_PERFMON
    // eventually, replace with perfmon call:

    // performance_monitor::startRegion("poly_block");
    likwid_markerStartRegion("poly_block");
  #endif

#pragma omp for schedule(runtime)
    for (int base = 0; base < n; base += UNROLL)
    {
      polynomial_block<UNROLL>(&(array[base]), poly, degree, &(array[base]));
    }

  #ifdef LIKWID_CLI
    likwid_markerStopRegion("poly_block");
  #endif
  #ifdef FHV_PERFMON
    // eventually, replace with perfmon call:

    // performance_monitor::stopRegion("poly_block");
    likwid_markerStopRegion("poly_block");
  #endif
  }
}

void likwid_cli_init(){
#ifdef LIKWID_CLI
  likwid_markerInit();
  #pragma omp parallel
  {
    likwid_markerThreadInit();
    likwid_markerRegisterRegion("poly_block");
    likwid_pinThread(omp_get_thread_num());
  }
#endif
}

void fhv_perfmon_init(){
#ifdef FHV_PERFMON
  performance_monitor::init("poly_block", "");
#endif
}

int main (int argc, char* argv[]) {
  if (argc != 4) {
    std::cout << "Too few arguments!\n";
    std::cerr<<"Usage: " << argv[0] << " n degree nbiter"<<std::endl;
    return -1;
  }
  
  int n = atoi(argv[1]); //TODO: atoi is an unsafe function
  int degree = atoi(argv[2]);
  int nbiter = atoi(argv[3]);

  float* array = new float[n];
  float* poly = new float[degree+1];
  for (int i=0; i<n; ++i)
    array[i] = 1.;

  for (int i=0; i<degree+1; ++i)
    poly[i] = 1.;

#ifdef MANUAL_MEASUREMENT
  std::chrono::time_point<std::chrono::system_clock> begin, end;
  begin = std::chrono::system_clock::now();
#endif

#ifdef LIKWID_CLI
  // techincally don't need the #ifdef block here because entire contents of
  // likwid_cli_init() are surrounded by their own #ifdef. But this helps keep
  // the intention of the code clear
  likwid_cli_init();
#endif

#ifdef FHV_PERFMON
  // techincally don't need the #ifdef block here because entire contents of
  // likwid_cli_init() are surrounded by their own #ifdef. But this helps keep
  // the intention of the code clear
  fhv_perfmon_init();
#endif
  
  for (int iter = 0; iter<nbiter; ++iter){
    polynomial_expansion (poly, degree, n, array);

#ifdef LIKWID_CLI
    likwid_markerNextGroup();
#endif
#ifdef FHV_PERFMON
    likwid_markerNextGroup();
#endif

  }

#ifdef MANUAL_MEASUREMENT
  end = std::chrono::system_clock::now();
  std::chrono::duration<double> totaltime = (end-begin)/nbiter;
  std::cerr << array[0] << std::endl;
  std::cout << n << " " << degree << " " << totaltime.count() << std::endl;
#endif

#ifdef LIKWID_CLI
  likwid_markerClose();
#endif

#ifdef FHV_PERFMON
  performance_monitor::close();

  performance_monitor::printHighlights();
  std::string param_info = "polynomial block params: n: " +
    std::to_string(n) + ", " +
    "degree: " + std::to_string(degree) + ", " +
    "number of iterations: " + std::to_string(nbiter);
  performance_monitor::resultsToJson(param_info);
#endif

  delete[] array;
  delete[] poly;

  return 0;
}
