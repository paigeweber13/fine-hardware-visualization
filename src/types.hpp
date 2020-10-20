//
// Created by riley on 10/20/20.
//

#pragma once

#include <iostream>
#include <iomanip>
#include <string>

namespace fhv {
  namespace types {
    // enums for aggregation type and result type
    enum class aggregation_t { sum, arithmetic_mean, geometric_mean,
      saturation };
    enum class result_t { event, metric };

    // to string functions for those
    static std::string aggregationTypeToString(
        const fhv::types::aggregation_t &aggregation_type);
    static std::string resultTypeToString(
        const fhv::types::result_t &result_type);

    // ---- TYPES

    // represents a result unique to a thread. "thread" refers to the hardware
    // thread
    struct PerThreadResult {
      std::string region_name;
      int thread_num;
      std::string group_name;
      fhv::types::result_t result_type;
      std::string result_name;
      double result_value;

      bool operator<(const PerThreadResult& other) const;
      bool matchesForAggregation(const PerThreadResult& other) const;
      std::string toString(std::string delim = " | ") const;
    };

    // represents a result aggregated across threads in a per-thread manner
    struct AggregateResult {
      std::string region_name;
      std::string group_name;
      fhv::types::result_t result_type;
      std::string result_name;
      fhv::types::aggregation_t aggregation_type;
      double result_value;

      bool operator<(const AggregateResult& other) const;
      std::string toString(std::string delim = " | ") const;
    };

    typedef
    std::vector< PerThreadResult >
        per_thread_results_t;

    typedef
    std::vector< AggregateResult >
        aggregate_results_t;

    // maps region to saturation name to saturation value
    typedef std::map<std::string, std::map<std::string, double>>
        saturation_map_t;

  };
};
