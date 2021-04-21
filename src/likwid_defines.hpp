#pragma once

#include <string>

// Intended to be used to track the keys that likwid uses. Intended to be
// static.

// JUSTIFICATION: using defines for these things turns incorrect spellings into
// compile-time errors instead of runtime errors. This is good!

// group names
const std::string likwid_group_flops_sp = "FLOPS_SP";
const std::string likwid_group_flops_dp = "FLOPS_DP";
const std::string likwid_group_l2 = "L2";
const std::string likwid_group_l3 = "L3";
const std::string likwid_group_mem = "MEM";
const std::string likwid_group_port1 = "PORT_USAGE1";
const std::string likwid_group_port2 = "PORT_USAGE2";
const std::string likwid_group_port3 = "PORT_USAGE3";

// number of flops
const std::string sp_scalar_flops_event_name = "FP_ARITH_INST_RETIRED_SCALAR_SINGLE";
const std::string sp_avx_128_flops_event_name = "FP_ARITH_INST_RETIRED_128B_PACKED_SINGLE";
const std::string sp_avx_256_flops_event_name = "FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE";
const std::string dp_scalar_flops_event_name = "FP_ARITH_INST_RETIRED_SCALAR_DOUBLE";
const std::string dp_avx_128_flops_event_name = "FP_ARITH_INST_RETIRED_128B_PACKED_DOUBLE";
const std::string dp_avx_256_flops_event_name = "FP_ARITH_INST_RETIRED_256B_PACKED_DOUBLE";

// flop rates
const std::string mflops_metric_name = "SP [MFLOP/s]";
const std::string mflops_dp_metric_name = "DP [MFLOP/s]";

// port usage
const std::string uops_port_base_name = "UOPS_DISPATCHED_PORT_PORT_";

// cache volume and bandwidth
const std::string l2_bandwidth_metric_name = "L2 bandwidth [MBytes/s]";
const std::string l2_data_volume_name = "L2 data volume [GBytes]";
const std::string l2_evict_bandwidth_name = "L2D evict bandwidth [MBytes/s]";
const std::string l2_evict_data_volume_name = "L2D evict data volume [GBytes]";
const std::string l2_load_bandwidth_name = "L2D load bandwidth [MBytes/s]";
const std::string l2_load_data_volume_name = "L2D load data volume [GBytes]";

const std::string l3_bandwidth_metric_name = "L3 bandwidth [MBytes/s]";
const std::string l3_data_volume_name = "L3 data volume [GBytes]";
const std::string l3_evict_bandwidth_name = "L3 evict bandwidth [MBytes/s]";
const std::string l3_evict_data_volume_name = "L3 evict data volume [GBytes]";
const std::string l3_load_bandwidth_name = "L3 load bandwidth [MBytes/s]";
const std::string l3_load_data_volume_name = "L3 load data volume [GBytes]";

// memory volume and bandwidth
const std::string ram_bandwidth_metric_name = "Memory bandwidth [MBytes/s]";
const std::string ram_data_volume_metric_name = "Memory data volume [GBytes]";
const std::string ram_evict_bandwidth_name = "Memory evict bandwidth [MBytes/s]";
const std::string ram_evict_data_volume_name = "Memory evict data volume [GBytes]";
const std::string ram_load_bandwidth_name = "Memory load bandwidth [MBytes/s]";
const std::string ram_load_data_volume_name = "Memory load data volume [GBytes]";

// other memory/cache stuff
const std::string load_to_store_ratio_metric_name = "Load to store ratio";
