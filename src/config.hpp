#pragma once

#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace fhv {
  namespace config {
    const std::string machineStatsFileName_template = 
      "machine-stats-template.json";
    const std::string machineStatsFileName = "machine-stats.json";
    const std::string machineStatsFileLocation_system = "/etc/fhv";
    const std::string machineStatsFileLocation_userPostfix = ".config/fhv";

    struct Architecture {
      unsigned num_ports_in_core;
    };

    struct BenchmarkResults {
      double mflops_sp;
      double mflops_dp;
      double bw_r_l1;
      double bw_r_l2;
      double bw_r_l3;
      double bw_r_ram;
      double bw_w_l1;
      double bw_w_l2;
      double bw_w_l3;
      double bw_w_ram;
      double bw_rw_l1;
      double bw_rw_l2;
      double bw_rw_l3;
      double bw_rw_ram;
    };

    struct MachineStats {
      Architecture architecture;
      BenchmarkResults benchmarkResults;
    };

    MachineStats loadMachineStats();
  }
}
