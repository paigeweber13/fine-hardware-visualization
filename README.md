# Fine Hardware Visualization
This software is a work and process. It is designed to present the user with a
visualization of their computer architecture and indicate what parts of that
architecture are most loaded to identify bottlenecks in high-performance
applications.

This software is in the early stages of development and should not yet be
assumed to be stable or correct.

- [Fine Hardware Visualization](#fine-hardware-visualization)
- [Prerequisites](#prerequisites)
  - [Compiled from source](#compiled-from-source)
  - [Installed via package manager](#installed-via-package-manager)
  - [Automatically included](#automatically-included)
- [Running](#running)
- [Usage Notes](#usage-notes)
- [Goals:](#goals)
- [Architecture of Program](#architecture-of-program)
- [Ownership and licensing](#ownership-and-licensing)
- [TODO:](#todo)
  - [Immediate:](#immediate)
    - [Exploration](#exploration)
    - [Likwid stability issues](#likwid-stability-issues)
  - [Long-term:](#long-term)
    - [Problems to fix:](#problems-to-fix)
    - [Features to add:](#features-to-add)
- [Other similar tools:](#other-similar-tools)
  - [Kerncraft:](#kerncraft)
  - [TAU:](#tau)
  - [Vampir](#vampir)
  - [Others:](#others)
    - [Works by Martin Schultz:](#works-by-martin-schultz)

# Prerequisites
## Compiled from source
 - **likwid >= 5.0.1:** a version above 5.0.1 is required, as this has support
   for memory counters and is confirmed to use `likwid-accessD` without root
   permissions. If this version is available with your package manager, use
   that. Otherwise, build it from source. Instructions to do this are available
   [here](https://github.com/RRZE-HPC/likwid). Be sure to change
   `LIKWID_PREFIX` in the makefile to match wherever likwid is installed
 - additional perfgroups not included with likwid. These can be installed by
   running `make perfgroups` in the root directory.

## Installed via package manager

 - **boost/program_options:** available on [the boost
   website](https://www.boost.org/). Also installable on ubuntu with `sudo apt
   install libboost-program-options-dev` on debian-based distributions. The
   makefile assumes boost program options is already in a directory included by
   gcc.
 - **cairo:** available [here](https://www.cairographics.org/), also
   installable with `sudo apt install libcairo2-dev` The makefile uses
   `pkg-config` to ensure cairo is automatically found and included.

## Automatically included
 - **[nlohmann/json](https://github.com/nlohmann/json):** header-only, included
   in ./lib

# Running
**warning:** in an effort to reduce workload until I get a minimal proof of
concept, I've stopped maintaining basically everything in this repository
(benchmarks, tests are notable examples). So don't expect `make tests` (test
suite) or `make bench` to work right now.

What still works:
 - minimal example of likwid: `make run-tests/likwid-minimal`
 - minimal example of the fhv performance monitor: `make run-tests/fhv-minimal`
 - convolution: `cd convolution-fast && make test`
 - fhv benchmarks: `make bin/fhv` and then `bin/fhv -s 100000` or the like.
   Note that sometimes has problems with "stopping non-started region", seems
   to be when you run benchmark-all. This is noted in the section "problems to
   fix"

# Usage Notes
 - Region names must not have spaces
 - Chapter 19 of volume 3 of the Intel software developer's manual (page 3605
   in the combined digital version) has hardware counter names

# Goals:
"I think we have a shot at doing something other people don't do" - Dr. Saule.
So far it seems that nothing is designed to automatically identify and
visualize the architecture by socket and within the core. Additionally, most of
these tools are not easy to use.

We want people new to HPC to be able to 
 - understand how their code maps to their architecture
 - give suggestions on how to improve their application

Additionally, we hope to apply this to graph problems: kernels in graph
problems tend to change behavior throughout execution

# Architecture of Program
 - Identify hardware architecture
 - Identify peak FLOP/s, memory bandwidth, latency etc.
   - there are lots of benchmarks that could help us with this. See:
     - `likwid_bench_auto` script included with kerncraft
     - `likwid-bench` utility included with likwid
     - [NAS parallel benchmarks](https://www.nas.nasa.gov/publications/npb.html)
 - Measure what actual utilization of memory/processor is
 - Compare actual utilization with peak on an piece-by-piece basis
 - Visualize that

# Ownership and licensing
 - nlohmann/json is included with this repository under the MIT license. See
   `lib/nlohmann/json.hpp` for full license
 - this repository is licensed under GNU GPL v3 (see `LICENSE`)

# TODO:
## Immediate:
 - [x] color visualization according to saturation
   - [x] once we get below 1% I don't think we really care about saturation
   - [x] largest visual difference in color from 0.01 to 0.2. Some color
         difference between 0.2 and 0.5. Above that is really hard to obtain
   - [x] (log_2 (saturation) + 10)/10 then clamp that to (0.0, 1.0)
 - [ ] add region name as a suffix to image filename
 - [ ] automatically create visualizations for each 
 - [ ] more counters to visualize?
   - [ ] basic polynomial expansion code has to be saturated somewhere... can
         we find it?

### Exploration
 - [ ] mem instructions retired * 32 bytes instead of 64
   - this is because there are 2 32-byte busses?
     [Yes!](https://en.wikichip.org/w/images/thumb/7/7e/skylake_block_diagram.svg/1350px-skylake_block_diagram.svg.png)
   - this architecture only moves 32-bytes (probably because 32-byte
     vectors are the biggest they can do)

### Likwid stability issues
 - [ ] port counters sometimes reporting 1.8e19 for values
   - [ ] also noticed L3 bandwidth was in the order of 1e11 or so for an
         execution of fhv_minimal in the double_flops region. This also doesn't
         make sense
 - [ ] convolution sometimes not instrumenting one region?
   - [ ] noticed this also in fhv_minimal. Happens once every 10 executions or
         so 
   - [ ] compiler optimization?

To accomplish this, I plan a typical daily usage of time as follows: 1 hour
toying with likwid and my examples, then 1 hour researching other people's
work, repeat until the day is done.

## Long-term:
### Problems to fix:
 - fix benchmark, benchmark-likwid-vs-manual, and thread_migration 
 - manual benchmark only prints runtime for flops region
   - in other words, runtime_by_tag doesn't seem to work for more than one 
     region
 - sometimes make rule for `run-tests/fhv_minimal` fails with a segmentation
   fault, seems to be right after compilation but before running. Immediately
   running the rule again succeeds.
 - sometimes get "stopping non-started region" error in fhv. I think it only
   happens when you run "benchmark-all" (the `-b` flag)
 - performance_monitor assumes maximum number of threads are used every time it
   sets performance_monitor::num_threads. Perhaps replace by initializing once
   in init once init routines are working?

### Features to add:
 - talk to a visualization expert about how we can improve our visualization
 - when nothing is saturated, what do you do? Look in to helping this problem
 - combine benchmark in fhv with benchmark-likwid-vs-manual
   - rewrite computation_measurements to optionally include manual results
   - update CLI to optionally include manual results
 - expand suite of test software that has balanced/imbalanced usage
   - consider standard benchmarks
 - improve benchmark
   - consider other benchmark tools (see ["architecture of program"
     section](#architecture-of-program))
   - have it check bandwidth for all types of memory/cache
   - have it check architecture to know what size of caches
   - have it populate architecture.h
   - improve software engineering: make it consistent what calls likwid, etc.

# Other similar tools:
## Kerncraft:
There's a
[paper](https://link.springer.com/chapter/10.1007%2F978-3-319-56702-0_1) about
it. The benchmark tool should be evaluated, we can draw from it.

 - advertises that it can instrument single core and full-socket performance:
   This is a similar granularity to that which we want to do
 - uses IACA to generate in-core predictions
 - Big focus on cache and memory and predicting what level requested data comes
   from (see section 2.4 "Cache Miss Prediction")
 - chooses to use theoretical performance numbers for some things and
   benchmarks for others. The choice is complicated but detailed in Fig. 1
   of their paper
 - unable to instrument multi-stage programs. You can only instrument a
   loop 
 - was interested in a tool mentioned on page 9 of the report which
   benchmarks the system and may also generate a topology `.yml`. 
   - In the paper it was called `likwid_bench_auto.py` but seems to have been
     renamed to `kerncraft/models/benchmark.py`
   - to run it, I installed kerncraft with `pip install kerncraft` and then
     ran `likwid_bench_auto` in bash. So name of executable is still the
     same 
   - seems to do **a lot of the same stuff we want to do with a benchmark**...
 - for benchmark to have useful data, you have to fill out a .yml file
   about your machine. With our program, we would like architecture to be
   automatically detected
   - honestly we might just have to write .yml files for intel
     architectures that have enough information for visualizations to be
     created. A lot of the information can be scraped from the output of things
     like `likwid-topology` and `cat /proc/cpuinfo` but not everything.
 - kerncraft does automatically benchmark a lot of great things (bandwidths
   are a notable example) but also provides much more information than we
   are planning on getting from our benchmarks 
 - As part of their roofline model, they predict the cache miss rate, use that
   to predict the amount of data needed from each level of cache, and divide
   that by a measured achievable bandwidth to get a throughput time. They even
   try to find a benchmark that is similar to the code being tested by
   analyzing the amount of reads from and writes to memory.
 - I feel like I could spend weeks just learning everything about kerncraft
 - skipped ahead to usage and samples of results
   - not really sure how to interpret them
   - presented some cache/memory data
   - presented some information on how things scale
 - hardware counter usage seems to be limited to validation From paper: "The
   output of likwid-perfctr is used to derive familiar metrics (Gflop/s,
   MLUP/s, etc.), which in turn are used for validations.

## TAU:
 - url: https://www.cs.uoregon.edu/research/tau/home.php 
 - [this tutoral](http://tau.uoregon.edu/tau.ppt) was helpful
 - mentions a ["topology
   view"](https://www.cs.uoregon.edu/research/tau/docs/newguide/bk01pt02ch10s04.html)
   provided by ParaProf that visualizes how performance maps to architecture.
   This sounds like exactly what we want to do 
   - doesn't seem to visualize architecture? Still trying to figure out how to
     interpret this plot
   - perhaps it's nodes in a cluster for distributed computing... It does not
     seem to provide the fine-grained visualization we are trying to create
 - difficult to use

## Vampir
 - helps identify communication bottlenecks
 - overhead associated with procedure calling
 - cache miss rates (associates them with the call to a specific function or
   functions, which allows the programmer to focus their attention.)
 - requires a paid license, but there is a free demo version
 - per-thread granularity

## Others:
 - check what Dr. Martin Schultz is up to:
   https://www.professoren.tum.de/en/schulz-martin/ 
 - Intel PCM
 - other laboratory toolkits
 - Vtune
 - IACA (Intel Architecture Core Analyzer)
   - has reached end of life according to intel website. LLVM-MCA should be
     considered as a replacement
 - LLVM-MCA
 - [NVidia GPU
   tools](https://developer.nvidia.com/performance-analysis-tools)
 - The following are mentioned in the kerncraft paper:
   - PBound - extracts information like arithmetic operations and
     loads/stores from source code. Does not consider cache effects or
     parallel execution. Model is "rather idealized" [[kerncraft
     paper]](https://link.springer.com/chapter/10.1007%2F978-3-319-56702-0_1)
   - ExaSAT - source code analysis with emphasis on data travel through cache
     levels. Does not include compiler optimizations in measurements. Can
     measure message-passing overhead
   - Roofline Model Toolkit
     - seems to also be called the Empirical Roofline Toolkit (ERT)
     - attempts to **generate hardware models**, like we do.
   - MAQAO
     - does runtime analysis

### Works by Martin Schultz:
 - see [their CV](https://www.in.tum.de/caps/mitarbeiter/martin-schulz),
   specifically the section "journal publications". A few seem pertinient, such
   as:
   - "Visualization and analytics for characterizing complex memory performance
     behaviors"
   - "Connecting Performance Analysis and Visualization to Advance Extreme
     Scale" 
