#pragma once

#include <string>
#include <vector>

// magic numbers
// this is the value that seems to always come up in the likwid unreasonably
// high values bug (see https://github.com/RRZE-HPC/likwid/issues/292 )
#define EVENT_VALUE_ERROR_THRESHOLD 1.84467e19 
#define METRIC_VALUE_ERROR_THRESHOLD 1e10 
#define ACCESSMODE_DAEMON "1"
#define ACCESSMODE_DIRECT "0"
#define MFLOPS_TO_TFLOPS 1e-6
#define OPS_PER_SP_256_VECTOR 8
#define OPS_PER_SP_128_VECTOR 4

// keywords
#define perfmon_output_envvar "FHV_OUTPUT"
#define perfmon_keep_large_values_envvar "FHV_KEEP_LARGE_VALUES"

// saturation keywords
// TODO: add "fhv_" prefix
#define flops_sp_saturation_metric_name "Saturation FLOPS SP"
#define flops_dp_saturation_metric_name "Saturation FLOPS DP"
#define l2_saturation_metric_name "Saturation L2 bandwidth"
#define l3_saturation_metric_name "Saturation L3 bandwidth"
#define mem_saturation_metric_name "Saturation Memory bandwidth"

// remove
// #define total_sp_flops_event_name "total sp flops"

// remove
// #define group_sum_by_region_keyword "group_sum_by_region"

// remove
// #define all_groups_keyword "all_groups"

// #define all_regions_keyword "all_regions"

// remove
// #define fhv_port0_usage_ratio "Port0 usage ratio"
// #define fhv_port1_usage_ratio "Port1 usage ratio"
// #define fhv_port2_usage_ratio "Port2 usage ratio"
// #define fhv_port3_usage_ratio "Port3 usage ratio"
// #define fhv_port4_usage_ratio "Port4 usage ratio"
// #define fhv_port5_usage_ratio "Port5 usage ratio"
// #define fhv_port6_usage_ratio "Port6 usage ratio"
// #define fhv_port7_usage_ratio "Port7 usage ratio"

// JSON keywords
const std::string json_info_section = "info";
const std::string json_parameter_key = "parameters";
const std::string json_results_section = "region_results";
const std::string json_thread_section_base = "thread_";

// ------ FHV Keywords ----- //
/* These are not used in likwid or any other tools. They were created for FHV
 */
// port usage ratio names
const std::string fhv_performance_monitor_group = "FHV_PERFORMANCE_MONITOR";

// remove
// const std::string fhv_port_usage_group = "FHV Port usage ratios";
const std::string fhv_port_usage_ratio_start = "Port";
const std::string fhv_port_usage_ratio_end = " usage ratio";

const std::vector<std::string> fhv_saturation_metrics = {
  flops_sp_saturation_metric_name,
  flops_dp_saturation_metric_name,
  l2_saturation_metric_name,
  l3_saturation_metric_name,
  mem_saturation_metric_name,
};

enum class precision { SINGLE_P, DOUBLE_P };
