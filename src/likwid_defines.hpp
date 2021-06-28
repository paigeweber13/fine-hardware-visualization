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

// number of flops
const std::string sp_scalar_flops_event_name = "FP_ARITH_INST_RETIRED_SCALAR_SINGLE";
const std::string sp_avx_128_flops_event_name = "FP_ARITH_INST_RETIRED_128B_PACKED_SINGLE";
const std::string sp_avx_256_flops_event_name = "FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE";
const std::string dp_scalar_flops_event_name = "FP_ARITH_INST_RETIRED_SCALAR_DOUBLE";
const std::string dp_avx_128_flops_event_name = "FP_ARITH_INST_RETIRED_128B_PACKED_DOUBLE";
const std::string dp_avx_256_flops_event_name = "FP_ARITH_INST_RETIRED_256B_PACKED_DOUBLE";

// flop rates
const std::string mflops_metric_name = "SP [MFLOP/s]";
const std::string mflops_sp_metric_name = mflops_metric_name;
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

// topology information
const uint32_t CPUFAMILY_ATOM                 = 0x1CU;
const uint32_t CPUFAMILY_ATOM_45              = 0x26U;
const uint32_t CPUFAMILY_ATOM_32              = 0x36U;
const uint32_t CPUFAMILY_ATOM_22              = 0x27U;
const uint32_t CPUFAMILY_ATOM_SILVERMONT_E    = 0x37U;
const uint32_t CPUFAMILY_ATOM_SILVERMONT_C    = 0x4DU;
const uint32_t CPUFAMILY_ATOM_SILVERMONT_Z1   = 0x4AU;
const uint32_t CPUFAMILY_ATOM_SILVERMONT_Z2   = 0x5AU;
const uint32_t CPUFAMILY_ATOM_SILVERMONT_F    = 0x5DU;
const uint32_t CPUFAMILY_ATOM_SILVERMONT_AIR  = 0x4CU;
const uint32_t CPUFAMILY_ATOM_SILVERMONT_GOLD = 0x5CU;
const uint32_t CPUFAMILY_ATOM_DENVERTON       = 0x5FU;
const uint32_t CPUFAMILY_ATOM_GOLDMONT_PLUS   = 0x7AU;
const uint32_t CPUFAMILY_ATOM_TREMONT         = 0x86U;
const uint32_t CPUFAMILY_BROADWELL_E          = 0x4FU;
const uint32_t CPUFAMILY_BROADWELL_D          = 0x56U;
const uint32_t CPUFAMILY_BROADWELL_E3         = 0x47U;
const uint32_t CPUFAMILY_CORE2_65             = 0x0FU;
const uint32_t CPUFAMILY_CORE2_45             = 0x17U;
const uint32_t CPUFAMILY_HASWELL              = 0x3CU;
const uint32_t CPUFAMILY_HASWELL_EP           = 0x3FU;
const uint32_t CPUFAMILY_IVYBRIDGE_EP         = 0x3EU;
const uint32_t CPUFAMILY_SANDYBRIDGE_EP       = 0x2DU;
const uint32_t CPUFAMILY_NEHALEM              = 0x1AU;
const uint32_t CPUFAMILY_NEHALEM_BLOOMFIELD   = 0x1AU;
const uint32_t CPUFAMILY_NEHALEM_EX           = 0x2EU;
const uint32_t CPUFAMILY_NEHALEM_LYNNFIELD    = 0x1EU;
const uint32_t CPUFAMILY_NEHALEM_LYNNFIELD_M  = 0x1FU;
const uint32_t CPUFAMILY_NEHALEM_WESTMERE     = 0x2CU;
const uint32_t CPUFAMILY_NEHALEM_WESTMERE_M   = 0x25U;
const uint32_t CPUFAMILY_SKYLAKE1             = 0x4EU;
const uint32_t CPUFAMILY_SKYLAKE2             = 0x5EU;
const uint32_t CPUFAMILY_SKYLAKEX             = 0x55U;
const uint32_t CPUFAMILY_WESTMERE_EX          = 0x2FU;
const uint32_t CPUFAMILY_XEON_PHI             = 0x01U;
const uint32_t CPUFAMILY_XEON_PHI_KNL         = 0x57U;

// these are all the architectures with a mem counter
#define NUM_ARCH_MEM_COUNTER 36
const uint32_t ARCH_WITH_MEM_COUNTER[NUM_ARCH_MEM_COUNTER] = {
  CPUFAMILY_ATOM, CPUFAMILY_ATOM_45, CPUFAMILY_ATOM_32, CPUFAMILY_ATOM_22,
  CPUFAMILY_ATOM_SILVERMONT_E, CPUFAMILY_ATOM_SILVERMONT_C,
  CPUFAMILY_ATOM_SILVERMONT_Z1, CPUFAMILY_ATOM_SILVERMONT_Z2,
  CPUFAMILY_ATOM_SILVERMONT_F, CPUFAMILY_ATOM_SILVERMONT_AIR,
  CPUFAMILY_ATOM_SILVERMONT_GOLD, CPUFAMILY_ATOM_DENVERTON,
  CPUFAMILY_ATOM_GOLDMONT_PLUS, CPUFAMILY_ATOM_TREMONT,
  CPUFAMILY_BROADWELL_E, CPUFAMILY_BROADWELL_D, CPUFAMILY_BROADWELL_E3,
  CPUFAMILY_CORE2_65, CPUFAMILY_CORE2_45, CPUFAMILY_HASWELL,
  CPUFAMILY_HASWELL_EP, CPUFAMILY_IVYBRIDGE_EP, CPUFAMILY_SANDYBRIDGE_EP,
  CPUFAMILY_NEHALEM, CPUFAMILY_NEHALEM_BLOOMFIELD, CPUFAMILY_NEHALEM_EX,
  CPUFAMILY_NEHALEM_LYNNFIELD, CPUFAMILY_NEHALEM_LYNNFIELD_M,
  CPUFAMILY_NEHALEM_WESTMERE, CPUFAMILY_NEHALEM_WESTMERE_M,
  CPUFAMILY_SKYLAKE1, CPUFAMILY_SKYLAKE2, CPUFAMILY_SKYLAKEX,
  CPUFAMILY_WESTMERE_EX, CPUFAMILY_XEON_PHI, CPUFAMILY_XEON_PHI_KNL
};

