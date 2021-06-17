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

    json loadMachineStats();
  }
}
