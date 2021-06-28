# Supported Architectures

Anything not listed here should be assumed to be non-working.

# Tested
## With All Features
- Skylake

# Untested, but Should Theoretically Work
## With All Features
- Broadwell
- BroadwellD
- BroadwellEP (testing in progress)
- Haswell
- HaswellEP
- Ivybridge
- IvybridgeEP
- Sandybridge
- SandybridgeEP

## Without Port Usage Metrics
- Atom
- Core2
- KNL
- Nehalem
- Phi
- Westmere
- Zen
- Zen2 (testing in progress)
- Zen3

## Without RAM Saturation or Port Usage Metrics
Most any other Intel or AMD architecture *should* work. Every architecture I've
looked at has at least the counters for FLOPS_DP/SP and cache bandwidth.
However, I certainly haven't looked at them all.

# A Note on ARM and Power Architectures:
None of the ARM or Power architectures have port usage counters in likwid, but
they all have RAM counters in likwid. They would likely work with minor
modifications but I don't have any idea what considerations we'd have to make
for ARM processors, and I also don't have any test equipment. 

If anyone has test equipment and would be willing to try to add support for
those architectures, I'm pretty sure the only things that would need to be done
are the following:
- copy the architecture codes from `<likwid-repo>/src/includes/topology.h` to
  `<fhv-repo>/src/likwid_defines.hpp` 
- adding those new codes to the array `ARCH_WITH_MEM_COUNTER` in
  `<fhv-repo>/src/likwid_defines.hpp`
