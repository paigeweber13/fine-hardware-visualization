#include "config.hpp"

json fhv::config::loadMachineStats() {
  std::ifstream i;

  std::string homedir(getenv("HOME"));
  i.open(homedir + "/" + machineStatsFileLocation_userPostfix + "/" 
    + machineStatsFileName);

  if(!i){
    i.open(machineStatsFileLocation_system + "/" + machineStatsFileName);

    if (!i) {
      std::cerr << "ERROR: no machine stats file exists! Please copy "
        << "\"" << machineStatsFileLocation_system << "/" 
        << machineStatsFileName_template << "\" "
        << "to \"" << machineStatsFileLocation_system << "/"
        << machineStatsFileName << "\". "
        << "Then, fill out the file with your machine's performance. "
        << "If you do not have permissions necessary to write "
        << "to \"" << machineStatsFileLocation_system << "/"
        << machineStatsFileName << "\", "
        << "you may instead create a machineStats file at "
        << "to \"" << homedir << "/" << machineStatsFileLocation_userPostfix 
        << "/" << machineStatsFileName << "\". "
        << "For more information, see \"docs/installation.md\"."
        << std::endl;
      return json();
    }
  }

  json j;
  i >> j;

  return j;
}
