#pragma once

#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace fhv {
  namespace config {
    const std::string configFileName_template = "config-template.json";
    const std::string configFileName = "config.json";
    const std::string configFileLocation_system = "/etc/fhv";
    const std::string configFileLocation_userPostfix = ".config/fhv";

    json loadConfig();
  }
}
