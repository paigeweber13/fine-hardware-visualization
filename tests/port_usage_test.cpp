// can be compiled with something like:
// `cd ..; make; cd tests; g++ ../obj/performance_monitor.o port_usage_test.cpp -L/usr/local/likwid-master/lib -I/usr/local/likwid-master/include -I../lib -march=native -mtune=native -fopenmp -llikwid -o port_usage_test`

// and run with something like `LD_LIBRARY_PATH=/usr/local/likwid-master/lib ./port_usage_test | less`

#include <iomanip>
#include <omp.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "performance_monitor.hpp"

void copy(double *arr, double *copy_arr, size_t n) {
  for (size_t i = 0; i < n; i++) {
    copy_arr[i] = arr[i];
  }
}

void compare_port_usage_totals(
    const per_thread_results_map_t &per_thread_results,
    const aggregate_results_map_t &aggregate_results) {
  int num_threads;
#pragma omp parallel
  { num_threads = omp_get_num_threads(); }

  std::cout << std::endl;
  for (int i = 0; i < num_threads; i++) {
    double sum_of_port_ops = 0;
    double this_port_ops;
    std::cout << "Thread " << i << std::endl;
    for (size_t j = 0; j < 8; j++) {
      this_port_ops = 0;
      try {
        this_port_ops =
            per_thread_results.at(event)
                .at(i)
                .at("double_flops")
                .at("PORT_USAGE" + std::to_string(j / 3 + 1))
                .at("UOPS_DISPATCHED_PORT_PORT_" + std::to_string(j));
      } catch (std::out_of_range e) {
        std::cout << "WARN: unable to access per_thread_results.at(event)"
                  << ".at(" << i
                  << ").at(\"double_flops\").at(\"PORT_USAGE_TEST\")"
                  << ".at(\"UOPS_DISPATCHED_PORT_PORT_" << j << "\")"
                  << std::endl;
      }
      std::cout << "UOPS_DISPATCHED_PORT_PORT_" << std::to_string(j)
                << ":        " << std::setw(40) << std::right << this_port_ops
                << std::endl;
      sum_of_port_ops += this_port_ops;
    }
    std::cout << "sum of UOPS_DISPATCHED_PORT_PORT_*: " << std::setw(40)
              << std::right << sum_of_port_ops << std::endl;

    try {
      std::cout << "UOPS_EXECUTED_CORE:                 " << std::setw(40)
                << std::right
                << per_thread_results.at(event)
                       .at(i)
                       .at("double_flops")
                       .at("PORT_USAGE_TEST")
                       .at("UOPS_EXECUTED_CORE")
                << std::endl;
    } catch (std::out_of_range e) {
      std::cout << "WARN: unable to access per_thread_results.at(event)"
                << ".at(" << i
                << ").at(\"double_flops\").at(\"PORT_USAGE_TEST\")"
                << ".at(\"UOPS_EXECUTED_CORE\")" << std::endl;
    }

    try {
      std::cout << "UOPS_EXECUTED_THREAD:               " << std::setw(40)
                << std::right
                << per_thread_results.at(event)
                       .at(i)
                       .at("double_flops")
                       .at("PORT_USAGE_TEST")
                       .at("UOPS_EXECUTED_THREAD")
                << std::endl;
    } catch (std::out_of_range e) {
      std::cout << "WARN: unable to access per_thread_results.at(event)"
                << ".at(" << i
                << ").at(\"double_flops\").at(\"PORT_USAGE_TEST\")"
                << ".at(\"UOPS_EXECUTED_THREAD\")" << std::endl;
    }

    try {
      std::cout << "UOPS_ISSUED_ANY:                    " << std::setw(40)
                << std::right
                << per_thread_results.at(event)
                       .at(i)
                       .at("double_flops")
                       .at("PORT_USAGE_TEST")
                       .at("UOPS_ISSUED_ANY")
                << std::endl;
    } catch (std::out_of_range e) {
      std::cout << "WARN: unable to access per_thread_results.at(event)"
                << ".at(" << i
                << ").at(\"double_flops\").at(\"PORT_USAGE_TEST\")"
                << ".at(\"UOPS_ISSUED_ANY\")" << std::endl;
    }

    std::cout << std::endl;
  }

  // SUMS
  double sum_of_port_ops = 0;
  for (size_t j = 0; j < 8; j++) {
    sum_of_port_ops +=
        aggregate_results.at(sum)
            .at(event)
            .at("double_flops")
            .at("PORT_USAGE" + std::to_string(j / 3 + 1))
            .at("UOPS_DISPATCHED_PORT_PORT_" + std::to_string(j));
  }
  std::cout << "Sum across threads:" << std::endl
            << "sum of UOPS_DISPATCHED_PORT_PORT_*: " << std::setw(40)
            << std::right << sum_of_port_ops << std::endl;

  try {
    std::cout << "UOPS_EXECUTED_CORE:                 " << std::setw(40)
              << std::right
              << aggregate_results.at(sum)
                     .at(event)
                     .at("double_flops")
                     .at("PORT_USAGE_TEST")
                     .at("UOPS_EXECUTED_CORE")
              << std::endl;
  } catch (std::out_of_range e) {
    std::cout << "WARN: unable to access aggregate_results"
              << "for \"UOPS_EXECUTED_CORE\"" << std::endl;
  }

  try {
    std::cout << "UOPS_EXECUTED_THREAD:               " << std::setw(40)
              << std::right
              << aggregate_results.at(sum)
                     .at(event)
                     .at("double_flops")
                     .at("PORT_USAGE_TEST")
                     .at("UOPS_EXECUTED_THREAD")
              << std::endl;
  } catch (std::out_of_range e) {
    std::cout << "WARN: unable to access aggregate_results"
              << "for \"UOPS_EXECUTED_THREAD\"" << std::endl;
  }

  try {
    std::cout << "UOPS_ISSUED_ANY:                    " << std::setw(40)
              << std::right
              << aggregate_results.at(sum)
                     .at(event)
                     .at("double_flops")
                     .at("PORT_USAGE_TEST")
                     .at("UOPS_ISSUED_ANY")
              << std::endl;
  } catch (std::out_of_range e) {
    std::cout << "WARN: unable to access aggregate_results"
              << "for \"UOPS_ISSUED_ANY\"" << std::endl;
  }

  std::cout << std::endl;

  std::cout << "port usage averages according to fhv" << std::endl;
  for (unsigned i = 0; i < 8; i++){
    std::cout << "geometric mean port " << i << " usage: "
      << std::setw(40) 
      << aggregate_results
        .at(geometric_mean)
        .at(metric)
        .at("double_flops")
        .at(fhv_port_usage_group)
        .at(fhv_port_usage_ratio_start + std::to_string(i) + fhv_port_usage_ratio_end)
      << std::endl;
  }
  std::cout << std::endl;

  std::cout << "port usages according to fhv" << std::endl;
  for (unsigned t = 0; t < num_threads; t++){
    std::cout << "Thread " << t << std::endl;
    for (unsigned i = 0; i < 8; i++){
      std::cout << "port " << i << " usage ratio: "
        << std::setw(40) 
        << per_thread_results
          .at(metric)
          .at(t)
          .at("double_flops")
          .at(fhv_port_usage_group)
          .at(fhv_port_usage_ratio_start + std::to_string(i) + fhv_port_usage_ratio_end)
        << std::endl;
    }
    std::cout << std::endl;
  }
}

int main() {
  printf("\n\nThis is a minimal example of how the fhv performance_monitor \n"
         "aggregation prints work\n");

  performance_monitor::init("PORT_USAGE1|PORT_USAGE2|PORT_USAGE3|PORT_USAGE_TEST", "double_flops,copy", "");
  // ---- begin likwid initialization

  // const char *filepath = performance_monitor::likwidOutputFilepath.c_str();

  // so 14 group/region combos
//   setenv("LIKWID_EVENTS",
//          //  "MEM|L2|L3|FLOPS_SP|FLOPS_DP|PORT_USAGE1|PORT_USAGE2|PORT_USAGE3",
//          "PORT_USAGE_TEST|PORT_USAGE1|PORT_USAGE2|PORT_USAGE3", 1);
//   // setenv("LIKWID_EVENTS", "MEM_DP|L2", 1);
//   setenv("LIKWID_MODE", "1", 1);
//   // output filepath
//   setenv("LIKWID_FILEPATH", filepath, 1);
//   setenv("LIKWID_THREADS", "0,1,2,3", 1); // list of threads
//   setenv("LIKWID_FORCE", "1", 1);

//   likwid_markerInit();

// #pragma omp parallel
//   {
//     likwid_markerThreadInit();
//     likwid_markerRegisterRegion("double_flops");
//     likwid_markerRegisterRegion("copy");
//     likwid_pinThread(omp_get_thread_num());
//   }

  // ---- end likwid initialization

  double a, b, c;
  a = 1.8;
  b = 3.2;
  c = 0.0;

  size_t n = 2048;
  double arr[n];
  double copy_arr[n];

#pragma omp parallel
  {
    for (int j = 0; j < 4; j++) {
      printf("thread %d, iteration %d\n", omp_get_thread_num(), j);
      performance_monitor::startRegion("double_flops");
      for (int i = 0; i < 10000000; i++) {
        // 2e7 scalar double floating point operations per iteration
        c = a * b + c;
      }
      performance_monitor::stopRegion("double_flops");
      performance_monitor::startRegion("copy");
      for (int i = 0; i < 10000; i++) {
        copy(arr, copy_arr, n);
      }
      performance_monitor::stopRegion("copy");
      performance_monitor::nextGroup();
    }
  }

  printf("final c: %f\n", c);
  printf("final random part of copy_arr: %f\n", copy_arr[((size_t)c) % n]);

  likwid_markerClose();

  // performance_monitor::printRegionGroupEventAndMetricData();

  performance_monitor::buildResultsMaps();
  performance_monitor::printDetailedResults();
  // performance_monitor::printOnlyAggregate();

  performance_monitor::compareActualWithBench();
  // performance_monitor::printComparison();

  performance_monitor::printHighlights();

  // performance_monitor::resultsToJson();

  std::cout << "\n";
  // performance_monitor::printCsvHeader();
  // performance_monitor::printCsvOutput();

  compare_port_usage_totals(performance_monitor::get_per_thread_results(),
                            performance_monitor::get_aggregate_results());
}
