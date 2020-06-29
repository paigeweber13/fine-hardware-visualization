#pragma once
// JUSTIFICATION: using defines for these things turns incorrect spellings into
// compile-time errors instead of runtime errors. This is good!

// group names
#define likwid_group_flops_sp "FLOPS_SP"
#define likwid_group_flops_dp "FLOPS_DP"
#define likwid_group_l2 "L2"
#define likwid_group_l3 "L3"
#define likwid_group_mem "MEM"
#define likwid_group_port1 "PORT_USAGE1"
#define likwid_group_port2 "PORT_USAGE2"
#define likwid_group_port3 "PORT_USAGE3"

// number of flops
#define sp_scalar_flops_event_name "FP_ARITH_INST_RETIRED_SCALAR_SINGLE"
#define sp_avx_128_flops_event_name "FP_ARITH_INST_RETIRED_128B_PACKED_SINGLE"
#define sp_avx_256_flops_event_name "FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE"

// flop rates
#define mflops_metric_name "SP [MFLOP/s]"
#define mflops_dp_metric_name "DP [MFLOP/s]"

// port usage
#define uops_port_base_name "UOPS_DISPATCHED_PORT_PORT_"

// cache volume and bandwidth
#define l2_bandwidth_metric_name "L2 bandwidth [MBytes/s]"
#define l2_data_volume_name "L2 data volume [GBytes]"
#define l2_evict_bandwidth_name "L2D evict bandwidth [MBytes/s]"
#define l2_evict_data_volume_name "L2D evict data volume [GBytes]"
#define l2_load_bandwidth_name "L2D load bandwidth [MBytes/s]"
#define l2_load_data_volume_name "L2D load data volume [GBytes]"

#define l3_bandwidth_metric_name "L3 bandwidth [MBytes/s]"
#define l3_data_volume_name "L3 data volume [GBytes]"
#define l3_evict_bandwidth_name "L3 evict bandwidth [MBytes/s]"
#define l3_evict_data_volume_name "L3 evict data volume [GBytes]"
#define l3_load_bandwidth_name "L3 load bandwidth [MBytes/s]"
#define l3_load_data_volume_name "L3 load data volume [GBytes]"

// memory volume and bandwidth
#define ram_bandwidth_metric_name "Memory bandwidth [MBytes/s]"
#define ram_data_volume_metric_name "Memory data volume [GBytes]"
#define ram_evict_bandwidth_name "Memory evict bandwidth [MBytes/s]"
#define ram_evict_data_volume_name "Memory evict data volume [GBytes]"
#define ram_load_bandwidth_name "Memory load bandwidth [MBytes/s]"
#define ram_load_data_volume_name "Memory load data volume [GBytes]"

// other memory/cache stuff
#define load_to_store_ratio_metric_name "Load to store ratio"
