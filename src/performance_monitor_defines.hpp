#pragma once

#include <likwid.h>
#include <string>
#include <vector>

#include <likwid_defines.hpp>

// magic numbers
// this is the value that seems to always come up in the likwid unreasonably
// high values bug (see https://github.com/RRZE-HPC/likwid/issues/292 )
const double EVENT_VALUE_ERROR_THRESHOLD = 1.84467e19;
const double METRIC_VALUE_ERROR_THRESHOLD = 1e10;
const double MFLOPS_TO_TFLOPS = 1e-6;
const double OPS_PER_SP_256_VECTOR = 8;
const double OPS_PER_SP_128_VECTOR = 4;

// ----- LIKWID-related configuration ----- //

// These may be LIKWID-related, but because these affect our configuration and
// may be changed by the user of this program, they go here in and not in
// likwid_defines.hpp

const std::string likwidOutputFilepath = "/tmp/likwid_marker.out";
const std::string accessmode = std::to_string(ACCESSMODE_DAEMON);

// ------ FHV Keywords ----- //
/* These are not used in likwid or any other tools. They were created for FHV
 */

const std::string jsonResultOutputDefaultFilepath = "./perfmon_output.json";
const std::string perfmon_output_envvar = "FHV_OUTPUT";
const std::string perfmon_keep_large_values_envvar = "FHV_KEEP_LARGE_VALUES";

// const std::string fhv_port_usage_group = "FHV Port usage ratios";
const std::string fhv_port_usage_ratio_start = "Port";
const std::string fhv_port_usage_ratio_end = " usage ratio";

const std::string fhv_port0_usage_ratio = fhv_port_usage_ratio_start 
  + std::to_string(0) + fhv_port_usage_ratio_end;
const std::string fhv_port1_usage_ratio = fhv_port_usage_ratio_start 
  + std::to_string(1) + fhv_port_usage_ratio_end;
const std::string fhv_port2_usage_ratio = fhv_port_usage_ratio_start 
  + std::to_string(2) + fhv_port_usage_ratio_end;
const std::string fhv_port3_usage_ratio = fhv_port_usage_ratio_start 
  + std::to_string(3) + fhv_port_usage_ratio_end;
const std::string fhv_port4_usage_ratio = fhv_port_usage_ratio_start 
  + std::to_string(4) + fhv_port_usage_ratio_end;
const std::string fhv_port5_usage_ratio = fhv_port_usage_ratio_start 
  + std::to_string(5) + fhv_port_usage_ratio_end;
const std::string fhv_port6_usage_ratio = fhv_port_usage_ratio_start 
  + std::to_string(6) + fhv_port_usage_ratio_end;
const std::string fhv_port7_usage_ratio = fhv_port_usage_ratio_start 
  + std::to_string(7) + fhv_port_usage_ratio_end;

// JSON keywords
const std::string json_info_section = "info";
const std::string json_parameter_key = "parameters";
const std::string json_processor_section = "processor";
const std::string json_processor_name_key = "name";
const std::string json_processor_num_sockets_key = "num_sockets";
const std::string json_processor_num_numa_nodes_key = "num_numa_nodes";
const std::string json_processor_num_hw_threads_key = "num_hw_threads";
const std::string json_processor_num_threads_in_use_key = "num_threads_in_use";
const std::string json_processor_affinity_key = "affinity";

const std::string json_results_section = "region_results";
const std::string json_thread_section_base = "thread_";

// port usage ratio names
const std::string fhv_performance_monitor_group = "FHV_PERFORMANCE_MONITOR";

// architecture dependent, can change between runs. Therefore, does not go in
// likwid_defines.hpp
const std::vector<std::string> likwid_port_usage_event_names = {
  uops_port_base_name + std::to_string(0),
  uops_port_base_name + std::to_string(1),
  uops_port_base_name + std::to_string(2),
  uops_port_base_name + std::to_string(3),
  uops_port_base_name + std::to_string(4),
  uops_port_base_name + std::to_string(5),
  uops_port_base_name + std::to_string(6),
  uops_port_base_name + std::to_string(7),
};

const std::vector<std::string> fhv_port_usage_metrics = {
  fhv_port0_usage_ratio,
  fhv_port1_usage_ratio,
  fhv_port2_usage_ratio,
  fhv_port3_usage_ratio,
  fhv_port4_usage_ratio,
  fhv_port5_usage_ratio,
  fhv_port6_usage_ratio,
  fhv_port7_usage_ratio,
};

const std::vector<std::string> fhv_saturation_source_metrics = {
  mflops_metric_name,
  mflops_dp_metric_name,
  l2_bandwidth_metric_name,
  l2_evict_bandwidth_name,
  l2_load_bandwidth_name,
  l3_bandwidth_metric_name,
  l3_evict_bandwidth_name,
  l3_load_bandwidth_name,
  ram_bandwidth_metric_name,
  ram_evict_bandwidth_name,
  ram_load_bandwidth_name,
};

// saturation keywords
const std::string fhv_flops_sp_saturation_metric_name = "Saturation FLOPS SP";
const std::string fhv_flops_dp_saturation_metric_name = "Saturation FLOPS DP";
const std::string fhv_l2_rw_saturation_metric_name = "Saturation L2 read/write bandwidth";
const std::string fhv_l2_w_saturation_metric_name = "Saturation L2 write bandwidth";
const std::string fhv_l2_r_saturation_metric_name = "Saturation L2 read bandwidth";
const std::string fhv_l3_rw_saturation_metric_name = "Saturation L3 read/write bandwidth";
const std::string fhv_l3_w_saturation_metric_name = "Saturation L3 write bandwidth";
const std::string fhv_l3_r_saturation_metric_name = "Saturation L3 read bandwidth";
const std::string fhv_mem_rw_saturation_metric_name = "Saturation Memory read/write bandwidth";
const std::string fhv_mem_w_saturation_metric_name = "Saturation Memory write bandwidth";
const std::string fhv_mem_r_saturation_metric_name = "Saturation Memory read bandwidth";

const std::vector<std::string> fhv_saturation_metric_names = {
  fhv_flops_sp_saturation_metric_name,
  fhv_flops_dp_saturation_metric_name,
  fhv_l2_rw_saturation_metric_name,
  fhv_l2_w_saturation_metric_name,
  fhv_l2_r_saturation_metric_name,
  fhv_l3_rw_saturation_metric_name,
  fhv_l3_w_saturation_metric_name,
  fhv_l3_r_saturation_metric_name,
  fhv_mem_rw_saturation_metric_name,
  fhv_mem_w_saturation_metric_name,
  fhv_mem_r_saturation_metric_name,
};

const std::vector<std::string> fhv_other_diagram_metrics = {
  // these are just port_usage_names from above
  fhv_port0_usage_ratio,
  fhv_port1_usage_ratio,
  fhv_port2_usage_ratio,
  fhv_port3_usage_ratio,
  fhv_port4_usage_ratio,
  fhv_port5_usage_ratio,
  fhv_port6_usage_ratio,
  fhv_port7_usage_ratio,
};

// Intended use:
// - these all get printed with "printHighlights"
// - get output to the json for later use
const std::vector<std::string> fhv_key_metrics = {
  mflops_metric_name,
  mflops_dp_metric_name,
  l2_bandwidth_metric_name,
  l2_data_volume_name,
  l2_evict_bandwidth_name,
  l2_evict_data_volume_name,
  l2_load_bandwidth_name,
  l2_load_data_volume_name,
  l3_bandwidth_metric_name,
  l3_data_volume_name,
  l3_evict_bandwidth_name,
  l3_evict_data_volume_name,
  l3_load_bandwidth_name,
  l3_load_data_volume_name,
  ram_bandwidth_metric_name,
  ram_data_volume_metric_name,
  ram_evict_bandwidth_name,
  ram_evict_data_volume_name,
  ram_load_bandwidth_name,
  ram_load_data_volume_name,

  // notice that everything below here is also in saturation metrics
  fhv_flops_sp_saturation_metric_name,
  fhv_flops_dp_saturation_metric_name,
  fhv_flops_sp_saturation_metric_name,
  fhv_flops_dp_saturation_metric_name,
  fhv_l2_rw_saturation_metric_name,
  fhv_l2_w_saturation_metric_name,
  fhv_l2_r_saturation_metric_name,
  fhv_l3_rw_saturation_metric_name,
  fhv_l3_w_saturation_metric_name,
  fhv_l3_r_saturation_metric_name,
  fhv_mem_rw_saturation_metric_name,
  fhv_mem_w_saturation_metric_name,
  fhv_mem_r_saturation_metric_name,
  fhv_port0_usage_ratio,
  fhv_port1_usage_ratio,
  fhv_port2_usage_ratio,
  fhv_port3_usage_ratio,
  fhv_port4_usage_ratio,
  fhv_port5_usage_ratio,
  fhv_port6_usage_ratio,
  fhv_port7_usage_ratio,
};

enum class precision { SINGLE_P, DOUBLE_P };
