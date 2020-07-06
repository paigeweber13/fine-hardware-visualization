# Fine Hardware Visualization
This software is designed to present the user with a visualization of their 
computer architecture and indicate what parts of that
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
- [How to improve likwid stability:](#how-to-improve-likwid-stability)
- [TODO:](#todo)
  - [Immediate:](#immediate)
  - [Long-term:](#long-term)
    - [Problems to fix:](#problems-to-fix)
    - [Features to add:](#features-to-add)
    - [Improve software engineering](#improve-software-engineering)
- [Parameters used to create visualizations](#parameters-used-to-create-visualizations)
- [Examples design](#examples-design)
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
 - polynomial_expansion: First run `git submodule init && git submodule update`. Then, see `tests/visualization-color-tests.sh` demonstrates 
   usage

# Usage Notes
 - Region names must not have spaces
 - Chapter 19 of volume 3 of the Intel software developer's manual (page 3605
   in the combined digital version) has hardware counter names. This is useful
   if you want to create custom performance groups

# Goals:
"I think we have a shot at doing something other people don't do" - Dr. Saule.

So far it seems that nothing is designed to automatically identify and
visualize the architecture by socket and within the core. Additionally, most 
performance monitoring tools are not easy to use.

We want people new to HPC to be able to 
 - understand how their code maps to their architecture
 - give suggestions on how to improve their application

We want all programmers to
 - visualize hardware usage automatically, without requiring the software to
   have prior knowledge of hardware
 - be able to understand complex, multi-stage applications in relation to
   hardware. For example:
   - kernels written for graph problems often change behavior part way through
     execution. The computational requirements and bottlenecks shift.
   - in image processing, there is frequently a mix of storage-intensive,
     memory-intensive, and cpu-intensive computation.

# Architecture of Program
 - [ ] Identify hardware architecture
 - [ ] Identify peak FLOP/s, memory bandwidth, latency etc.
   - [ ] there are lots of benchmarks that could help us with this. See:
     - [ ] `likwid_bench_auto` script included with kerncraft
     - [ ] `likwid-bench` utility included with likwid
     - [ ] [NAS parallel benchmarks](https://www.nas.nasa.gov/publications/npb.html)
 - [x] Measure what actual utilization of memory/processor is
 - [x] Compare actual utilization with peak on an piece-by-piece basis
 - [x] Visualize that
   - [ ] allow for visualizations with varying levels of detail: overview,
         detailed core metrics, detailed cache metrics, etc.

# Ownership and licensing
 - nlohmann/json is included with this repository under the MIT license. See
   `lib/nlohmann/json.hpp` for full license
 - this repository is licensed under GNU GPL v3 (see `LICENSE`)

# How to improve likwid stability:
 - you MUST pin threads
 - register parallel regions in parallel blocks
 - run likwid_markerThreadInit() in a parallel block
 - put `#pragma omp barrier` before calls to `likwid_markerNextGroup`. This
   seems to dramatically reduce the number of occurrences of the unreasonably
   high values bug. It may even prevent this bug from occurring, I have yet to
   see a case where this bug manifests with the barrier in place.
 - put `#pragma omp barrier` before each `likwid_markerStartRegion` call and
   after each `likwid_markerStopRegion` call. This helps prevent "WARNING:
   stopping an unknown/non-started region <region>" and "WARNING: Region
   <region> was already started"
 - try to avoid redirecting output of a program that uses likwid to a file.
   This seems to cause many "WARN: Region <region> was already started" and
   "WARN: Stopping an unknown/not-started region <regon>" errors. Workarounds
   include: 
   - instead, pipe output to `less`
   - in a script, do something like `output = $(./program_that_uses_likwid);
     output > data/output.txt`
   - outputting to `/tmp` sometimes avoids this problem

# TODO:
## Immediate:
 - [x] port usage isn't what we expected it... why on CPU-heavy polynomial
       block, port4 (store data) is still the most saturated
   - [x] what happens if we increase degree past 100? Same behavior...
   - [x] inspect assembly to find stores
   - [x] look at how we measure port usage
     - corrected how we measure port usage. Values are still odd, though
   - Dr. Saule will look at this
 - [x] work on likwid stability issues
   - [x] BLOCKED: waiting to hear back from Thomas
     - thomas said likwid_markerNextGroup should be called in a sequential
       region. Seems to be solved?
   - [x] teleconference with Thomas to fix
 - [ ] improve likwid documentation
   - [x] BLOCKED: waiting to hear back from Thomas
   - [x] update doxygen for likwid
   - [x] update likwid examples
   - [ ] ask Tom how he'd like me to contribute. Some ideas follow:
     - [x] update `./examples/C-internalMarkerAPI.c`
     - [ ] write some wiki pages about general use (e.g. "There are three ways
           to use likwid...")
     - [ ] write test cases
     - [ ] consider improving doxygen comments and writing man pages for usage
 - [ ] more counters to visualize.
   - [ ] quickly finish looking at all perfgroups
   - [ ] Dr. Saule identified the following areas as key. What counters and
         perfgroups can help us identify performance in these areas?
     - [ ] port usage
     - [ ] instruction decoding: can you decode instructions quickly enough?
     - [ ] micro-instruction retiring: can you fetch instructions quickly
           enough? 
   - [ ] change computation saturation to be per-core
 - [ ] improve software engineering
   - [x] typedef long names in performance_monitor
   - [x] remove old, unused things
   - [ ] improve makefile 
     - [ ] simplify: there's some redundant stuff in there
     - [ ] can I make it so there's a general rule for all examples?
     - [ ] tests don't work any more
   - [ ] fhv.cpp: split up cairo stuff hierarchically
     - [ ] move cairo stuff to library in lib
     - [ ] separate functions for different parts of diagram
     - [ ] cairo_save/cairo_restore operate as a stack machine: you can
           save/restore in a nested manner
   - [ ] give convolution its own makefile
   - [ ] there are a lot of text files floating around (like in `tests/`). Can
         those be removed?
 - [ ] explore how well fhv works with other kernels and codebases
   - [ ] consider standard benchmarks
   - [ ] Dr. Saule may be able to throw together some software that
         demonstrates stress on more granular things like TLB or instruction
         decoder
 - [ ] start exploring different machines
   - [ ] another skylake architecture with a different number of cores
   - [ ] broadwell/haswell
   - [ ] eventual goal is to have architecture detection totally automatic but
         for now it's adequate to have a few sets of parameters hardcoded that
         are selected automatically
 - [ ] add "command-used-to-generate" to json and svg

## Long-term:
### Problems to fix:
 - if we are continuing to warn users about unreasonably high values, give them
   the option to disable the warning and count the values anyways
 - fix benchmark-likwid-vs-manual and thread_migration 
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
   - data generated by `examples/polynomial_expansion/script2*.sh` are not
   completely accurate. My own tests have show that saturation as high as 0.7
   for memory is possible, but these scripts only demonstrate a maximum 
   saturation of 0.4. See the .csv files in 
   `examples/polynomial_expansion/data` for examples.
   - parameters which demonstrated higher saturation are: n=67108864 d=1
     nbiter=800
     
### Features to add:
 - make core saturation (and therefore, color of the core) an average of many
   key metrics.
   - For example, might average flop saturation, instruction decoding, port
     usage, and instruction retiring.
   - This visualization will be general at the initial zoom but when you zoom
     in will separate into the different factors we consider
   - this should include single and double precision flops, because complex
     calculations may use both. However, only consider the highest for the
     average?
   - for more info, see notes taken on what Dr. Saule had to say about the
     subject in DEVELOPMENT_LOG.md -> 2020-06-02 through 2020-06-09 ->
     secondary
 - talk to a visualization expert about how we can improve our visualization
 - combine benchmark in fhv with benchmark-likwid-vs-manual
   - rewrite computation_measurements to optionally include manual results
   - update CLI to optionally include manual results
 - improve benchmark
   - consider other benchmark tools (see ["architecture of program"
     section](#architecture-of-program))
   - have it check bandwidth for all types of memory/cache
   - have it check architecture to know what size of caches
   - have it populate architecture.h
 - In some cases, color buses instead of components themselves
   - RAM: read/write separately are useful. Also, this is easy to incorporate
   - NUMA: This is the case where it's most important. There's potential for
     the bus(es) between CPUs to be saturated, when it wouldn't be saturated if
     it was memory directly to CPU

### Improve software engineering
 - make it consistent what calls likwid
 - makefile has some unnecessary repetition of variables
   - compare ./examples/polynomial_expansion/makefile with ./makefile

# Parameters used to create visualizations
 - memory-stressed versions used parameters n=67108864 d=1 nbiter=800.
   Examples of memory-stressed versions include:
   - ./visualizations/polynomial_basic_mem*
   - ./visualizations/polynomial_block_mem*
 - CPU-stressed versions used parameters n=67108864 d=1000 nbiter=80. Examples
   of memory-stressed versions include:
   - ./visualizations/polynomial_basic_cpu*
   - ./visualizations/polynomial_block_cpu*

# Examples design
Examples should be designed with simplicity as a priority. There should be no
fancy cli argument handling and recompilation should be limited.

Three features should be supported in each example:
 - Manual measurements to demonstrate bottlenecks 
 - fhv measurements to demonstrate bottlenecks 
 - Likwid CLI support for testing

./makefile 
Handles compilation of examples

./examples/test_code/
Should contain 1 source file. This file should have no dependencies outside of
the standard library when manual measurements are made. 

A single file should contain manual timing, likwid cli, and FHV metering. You
pick the one you want with compile time flags. These flags should be:
 - -DLIKWID_CLI for using the likwid-perfctr cli
 - -DFHV for using fhv

A single file should contain both basic and optimized code. The code used will
be chosen by compile-time flags:
 - -DBASIC_CODE for basic code
 - -DOPT_CODE for optimized code

Has 3+ scripts for testing 
 - manual timing that stresses both cpu and mem
 - likwid cli examples
 - fhv usage that stresses both cpu and mem

./examples/test_code/data/
Contains test data output from above scripts 

./examples/test_code/visualizations/
Contains visualizations of data

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
 - [this tutorial](http://tau.uoregon.edu/tau.ppt) was helpful
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
   specifically the section "journal publications". A few seem pertinent, such
   as:
   - "Visualization and analytics for characterizing complex memory performance
     behaviors"
   - "Connecting Performance Analysis and Visualization to Advance Extreme
     Scale" 
