# Makefile
TODO. This section was intended to explain how the makefile works.

# Source Files

This section gives an overview of what each source file does.

- **computation_measurements**: This file is *deprecated*. Before we began to
  use the likwid benchmark (see `./benchmark.sh` in this repository) this was
  used to try to get a maximum value for flop rates and bandwidths. It is
  included here only for informational purposes and should **not** be used.
- **config**: Reads and parses the `machine-stats.json` file.
- **fhv_main**: The entry point for the `fhv` command line tool. This file is
  only responsible for parsing command line parameters and calling the
  functions in `saturation_diagram` that are needed to create a visualization.
- **fhv_perfmon**: The primary working code of FHV. This is the file that
  becomes the `libfhv` library. This file provides the functions like `init`,
  `startRegion`, `calculate_saturation`, and `resultsToJson` that do the actual
  program measuring and result output.
- **likwid_defines**: Strings and codes used by likwid to identify
  architectures, events, and metrics. This allows fhv to identify key parts of
  the likwid output without relying on the programmer remembering exactly how
  to spell each thing likwid uses.
- **performance_monitor_defines**: Like above, but for fhv. These are all codes
  defined by fhv, whereas `likwid_defines` is for codes defined by likwid.
- **saturation_diagram**: The code which creates a visualization from a json.
  The most important function here is `draw_diagram_overview`. Also includes
  code that was used to test swatches, scales, and color interpolation.
- **types**: Defines types used by FHV to gather and aggregate likwid data.
  Types defined include `PerThreadResult`, `AggregateResult`, and
  `aggregation_t`.
- **utils**: This was its own file because it was intended to be a place to
  gather utility functions that were used across many other files. Currently it
  only includes one function, `create_directories_for_file`.

# Some Easy Todo Items for a New Developer

- replace instances of `std::cout` and `std::cerr` with `fmt::print`
- unify namespacing. `types.c/hpp` and `utils.c/hpp` are very cleanly namespaced:
  everything is within `fhv::types` and `fhv::utils`, respectively. By
  contrast, `fhv_perfmon.c/hpp` has a class with the title `fhv_perfmon`, so
  all functions are in the namespace `fhv_perfmon`. Ideally, this should be a
  class `perfmon` within the namespace `fhv`.
  - also, `performance_monitor_defines` should be inside its own namespace
    which is inside the fhv namespace. I would suggest using `fhv::defines`.
  - also, `likwid_defines` should have some namespacing like above. Consider
    `likwid::defines`.
- load/store arrows are un-colored for the broadwellEP architecture. This is a
  relatively easy fix.
  - For the Skylake architecture, memory data is reported as "Memory *load*
    bandwidth" and "Memory *evict* bandwidth". For the BroadwellEP arcitecture,
    these same metrics are reported as "Memory *read* bandwidth" and "Memory
    *write* bandwidth"
  - options to fix include: 1. open a pull request with likwid to change the
    name of the metrics in the perfgroups and thus unify this discrepency or 2.
    edit the FHV code to check for both possibilities.
- function naming is inconsistent. sometimes `snake_case` is used, and other
  times `camelCase` is used. Convert all functions with `snake_case` names to `camelCase`.

# Suggested Workflow

The makefile provides two convenience rules to adjust the `PATH` and
`LD_LIBRARY_PATH` variables. These rules are `make exports` and `make
devexports`.

`make exports` is only useful if likwid or fhv are built to non-standard
prefixes (the prefix used is set in `config.mk`). If you use the default prefix
of `/usr/local`, linux will automatically find the libraries and headers for
both likwid and fhv. However, if you're developing fhv, it can be annoying to
call `sudo make install` after every change. This is why the `devexports` rule
is provided.

By running `export $(make devexports)` in a terminal, you set the environment
variables to first look in the `./build/*` directories for libraries and
executables before looking in `/usr/local/*`. This way you can just run `make`
and then immediately run `fhv` and see the changes updated. Linux will
sometimes still pick up on the installed library so it's best to remove it
first with `sudo make uninstall`

Note that to test with an example you will still need to run `sudo make install`.

# Custom Counters and Performance Groups

## Identifying events

To create custom perfgroups like those in the `perfgroups` directory, you
should first identify what performance counters you want to use and experiment
with them on your own. A great resource to discover what is available on your
system is `likwid-perfctr`. Assuming you have likwid installed, run
`likwid-perfctr -e` to get a list of all the events supported by likwid and all
counters available on your architecture (i.e. the registers which can be
configured to count an event). The [Intel Software Developer's
Manual](https://software.intel.com/content/www/us/en/develop/articles/intel-sdm.html)
can help you understand what these counters are. Note that when intel uses `.`
in an event name, likwid replaces the `.` with `_`. Once you've identified a
few key events to watch, first explore them with the `likwid-perfctr` tool.
This section will outline the basic usage of `likwid-perfctr`, but go to [the
likwid docs](https://github.com/RRZE-HPC/likwid/wiki/likwid-perfctr) for full
usage information. 

A good place to start when exploring events and counters are the likwid
examples, available in the `examples` directory of the likwid repository.
Suppose you've identified two events you'd like to explore, `L2_LINES_IN_ALL`
and `L2_TRANS_L2_WB`. 

To measure those two events, go to the `examples` directory of the likwid
repository, call `make`, and then issue the following command:

`likwid-perfctr -C 0-$(nproc) -m -M 1 -g L2_LINES_IN_ALL:PMC0,L2_TRANS_L2_WB:PMC1 ./C-likwidAPI`

This command uses `likwid-perfctr` to instruct the processor to count
occurances of the event `L2_LINES_IN_ALL` in the counter `PMC0` and occurances
of the event `L2_TRANS_L2_WB` in the counter `PMC1`. The `PMC` counters are
general-purpose (and therefore configurable). Most of the other counters, like
`FIXC`, `PWR`, or `VTG`, are not configurable. The only other counters I know
of which are configurable are `MBOX0`, which will accept a limited number of
memory-specific counters.

At the time of writing, the `C-likwidAPI` program will produce basic output
about those counters, such as `event set L2_LINES_IN_ALL:PMC0 at CPU 0:
6563341.000000`. You can modify that program to perform the kind of computation
you're interested in.

## Creating a Custom Perfgroup

Once you've identified the events you'd like to track, you can create a custom
perfgroup by mimicing the format of those in the `groups` directory of the
likwid repository. After creating your perfgroup, all you need to do is place
it into `<likwid-prefix>/share/likwid/perfgroups/<your-architecture>/`.
Remember the default likwid prefix is `/usr/local`. This repository is set up
so that all perfgroups in the `perfgroups` directory of this repository are
copied to the likwid perfgroups directory when calling `make perfgroups` or
`make install`.

The syntax of perfgroup files does not seem to be documented in the likwid
documentation. However, the syntax is fairly simple and shouldn't give you too
much trouble. There are three primary sections: `SHORT`, `EVENTSET`, and
`METRICS`. `SHORT` is a short description that is displayed when calling
`likwid-perfctr -a`. `EVENTSET` specifies which events should be assigned to
which counters, you will probably only need to change the `PMC` counters.
Finally, the `METRICS` section tells likwid what metrics to calculate and how
they are calculated. the syntax seems to be `<name of metric> <formula>`. See
the likwid perfgroups for examples.

## More Information about Architecture Events

Note that the list of events produced by `likwid-perfctr -e` is probably not an
exhaustive list of the events your architecture can track. For a full list of
events supported on intel architectures, consult the [Intel Software
Developer's
Manual](https://software.intel.com/content/www/us/en/develop/articles/intel-sdm.html).
Custom events can be specified using tools like [likwid
perfmon](https://rrze-hpc.github.io/likwid/Doxygen/group__PerfMon.html) (a
low-level API underlying the likwidAPI and markerAPI) or
[PAPI](https://icl.utk.edu/papi/) (a powerful, low-level performance monitoring
tool designed to use hardware counters). Specifying events requires use of the
`id` and `umask` of the event. 

This level of granularity is rarely required and is outside the scope of this
repository, so no further notes on usage will be supplied.

# Adding Another Architecture

Adding support for another architecture in FHV is as simple as discovering what
counters that architecture supports and creating any perfgroups necessary to
add functionality not included with likwid.

For full FHV functionality, an architecture must be able to do the following:
- count time
- count floating point operations (`FP_ARITH_INST_RETIRED*` or similar)
- count the volume of data read and written through each level of cache
  (`L1D_REPLACEMENT`, `L1D_M_EVICT`, and similar)
- count the volume of data read and written through memory (`DRAM_READS` and
  `DRAM_WRITES` or similar)
- count the number of instructions executed in each port
  (`UOPS_DISPATCHED_PORT_PORT_*` or similar)

As far as I know, every architecture supported by likwid has perfgroups that
will give you counter values for floating point operations and movement of data
through cache. The two that are less-widely supported are memory volume and
port usage counters. Likwid *does* include a `PORT_USAGE` group for skylake,
but it requires that hyperthreading be disabled (so that eight counters may be
used instead of four). Adding support for your architecture will likely boil
down to adding memory counters and port-usage counters.

## Memory Counters

When calculating saturation for the diagram, FHV looks for metrics with the
names "Memory bandwidth [MBytes/s]", "Memory evict bandwidth [MBytes/s]", and
"Memory load bandwidth [MBytes/s]". If your architecture provides events that
record the number of reads and writes to DRAM, you can calculate these metrics
yourself. For read bandwidth multiply the number of reads by the cache line
size. Do this with the number of writes for write bandwidth and add those two
values together for total bandwidth. For more specific help on how to do this,
look at one of the `MEM` perfgroups provided by likwid, like the file
`./groups/skylake/MEM.txt` in the likwid repository.

After you add the requried `MEM` perfgroups, find your architecture's code in
the likwid repository in the file `./src/includes/topology.h` and add it to the
file `./src/likwid_defines.hpp` in this repository. You will also need to add
that code to the list `ARCH_WITH_MEM_COUNTER` in the `likwid_defines.hpp` file.

Unfortunately, some architectures simply do not provide events for tracking
reads and writes to RAM. In this case it's impossible to get memory saturation.

## Port Usage Ratios

Port usage ratios are all calculated by FHV, in the function
`fhv_perfmon.cpp::calculate_port_usage_ratios()`. This is because you have to
use multiple groups to track all port_usage counters, so it's difficult to use
the likwid metric feature to calculate port usage ratios. Instead, FHV looks
for the events `UOPS_DISPATCHED_PORT_PORT_*` or `UOPS_EXECUTED_PORT_PORT_*`,
depending on the architecture, and sums up the events for each port to get a
total. 

If your architecture has one of those counters, adding support for port usage
ratios is as simple as adding a perfgroup that tracks those counters.
`calculate_port_usage_ratios   will automatically check for both variants. If
your architecture provides a different event that records uops executed on a
per-core basis, you will have to extend `calculate_port_usage_ratios` to
support that event.

# How `likwid-bench` Works

`likwid-bench` is a very clever piece of software that is used by the
microbenchmarks (see `./tests/microbenchmarks` in this repository). This
software is entirely contained in the `./bench` directory of the likwid
repository. In short, there are hand-written `.ptt` files for each architecture
that use a likwid-specific syntax which is fairly similar to assembly. When
`make` is called in the `./bench` directory, these file are processed into
`.pas` files by the script `./bench/perl/generatePas.pl`. Those are then
converted into `.s` (assembly) files by `./bench/perl/AsmGen.pl`, which are
finally compiled into `.o` (object) files by an assembler (the GNU assembler
`as`, if you're compiling with GCC).

The microbenchmarks rely on likwid to do all this wizardry, and just call
`make` in the `bench` directory of the likwid repository. Then, they link the
object files into C++ code by declaring the assembly functions as `extern "C"`
functions. See the file `./tests/microbenchmarks/src/peakflops_sp_avx_fma.hpp`
for the code I used.

If I had more time I would have loved to add bandwidth microbenchmarks.
