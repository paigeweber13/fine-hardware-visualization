#include "config.hpp"

fhv::config::MachineStats fhv::config::loadMachineStats() {
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
      return MachineStats{};
    }
  }

  json j;
  i >> j;

  MachineStats machineStats{
    architecture: Architecture{
      num_ports_in_core: j["architecture"]["num_ports_in_core"],
    },
    benchmarkResults: BenchmarkResults{
      mflops_sp: j["benchmark_results"]["mflops_sp"],
      mflops_dp: j["benchmark_results"]["mflops_dp"],
      bw_r_l1: j["benchmark_results"]["bw_r_l1"],
      bw_r_l2: j["benchmark_results"]["bw_r_l2"],
      bw_r_l3: j["benchmark_results"]["bw_r_l3"],
      bw_r_ram: j["benchmark_results"]["bw_r_ram"],
      bw_w_l1: j["benchmark_results"]["bw_w_l1"],
      bw_w_l2: j["benchmark_results"]["bw_w_l2"],
      bw_w_l3: j["benchmark_results"]["bw_w_l3"],
      bw_w_ram: j["benchmark_results"]["bw_w_ram"],
      bw_rw_l1: j["benchmark_results"]["bw_rw_l1"],
      bw_rw_l2: j["benchmark_results"]["bw_rw_l2"],
      bw_rw_l3: j["benchmark_results"]["bw_rw_l3"],
      bw_rw_ram: j["benchmark_results"]["bw_rw_ram"],
    },
  };

  return machineStats;
}
