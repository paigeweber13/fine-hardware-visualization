# Fine Hardware Visualization (FHV)
This software is designed to present the user with a visualization of their
computer architecture and indicate what parts of that architecture are most
loaded to identify bottlenecks in high-performance applications.

This information can be used both to help programmers understand how to
optimize their code and to teach students how their code maps to their
architecture.


# Warnings, Disclaimers
This software is in the early stages of development. We are reasonably
confident that this tool reports measurements within 1% of actual values (see
`tests/microbenchmarks`) but please be aware that some errors are expected in
such early stages of development. Furthermore, the usage and API are not
stable, future updates my break code which uses this tool. This software has
only been tested on the Intel Skylake processor. It should not be used in any
production-critical system. See `LICENSE` for full licesne information and
disclaimers.

We encourage you to try to build it and run the tests or examples! Feedback and
bug reports are welcome via email. My email address may be found [in my github
profile](https://github.com/paigeweber13).


- [Fine Hardware Visualization (FHV)](#fine-hardware-visualization-fhv)
- [Warnings, Disclaimers](#warnings-disclaimers)
- [Installation and Usage](#installation-and-usage)
- [Goals:](#goals)
- [Architecture of Program](#architecture-of-program)
- [Ownership and licensing](#ownership-and-licensing)
- [How to improve likwid stability:](#how-to-improve-likwid-stability)
- [How example visualizations are created](#how-example-visualizations-are-created)
- [Examples design](#examples-design)
- [Other similar tools:](#other-similar-tools)
  - [Kerncraft:](#kerncraft)
  - [TAU:](#tau)
  - [Vampir](#vampir)
  - [Others:](#others)
    - [Works by Martin Schultz:](#works-by-martin-schultz)

# Installation and Usage
Please see the [documentation](docs/installation-and-usage.md)

# Goals:
"I think we have a shot at doing something other people don't do" 
- Dr. Saule.

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
   - [ ] currently, the `benchmark.sh` script does this, but the user must
         manually copy values to `src/architecture.hpp`. In the future I would
         like this process to be automated.
 - [x] Measure what actual utilization of memory/processor is
 - [x] Compare actual utilization with peak on an piece-by-piece basis
 - [x] Visualize that
   - [ ] allow for visualizations with varying levels of detail: overview,
         detailed core metrics, detailed cache metrics, etc.

# Ownership and licensing
 - nlohmann/json is included with this repository under the MIT license. See
   `lib/nlohmann/json.hpp` for full license.
 - likwid source code is included (as a submodule in `tests/microbenchmarks/likwid`) under the GNUGPL licesne. See `tests/microbenchmarks/likwid/COPYING`.
 - this repository is licensed under GNU GPL v3 (see `LICENSE`).

# How to improve likwid stability:
 - you MUST pin threads
 - register parallel regions in parallel blocks
 - run likwid_markerThreadInit() in a parallel block
 - put `#pragma omp barrier` before calls to `likwid_markerNextGroup`. This
   seems to dramatically reduce the number of occurrences of the unreasonably
   high values bug. It may even prevent this bug from occurring, I have yet to
   see a case where this bug manifests with the barrier in place.
 - (This tip may not be necessary but does seem to help in some cases) Put
   `#pragma omp barrier` before each `likwid_markerStartRegion` call and after
   each `likwid_markerStopRegion` call. This helps prevent "WARNING: stopping
   an unknown/non-started region <region>" and "WARNING: Region <region> was
   already started"
 - try to avoid redirecting output of a program that uses likwid to a file.
   This seems to cause many "WARN: Region <region> was already started" and
   "WARN: Stopping an unknown/not-started region <regon>" errors. Workarounds
   include: 
   - instead, pipe output to `less`
   - in a script, do something like `output = $(./program_that_uses_likwid);
     output > data/output.txt`
   - outputting to `/tmp` sometimes avoids this problem

# How example visualizations are created
The `visualizations/polynomial*.svg` visualizations are created using
`tests/visualizations-poly.sh`, which should be run from the root of this
project.

The visualizations/data in `tests/data/mem_size*.png`,
`tests/data/mem_iter_comparison.csv`, and `tests/data/flops_comparison.png` are
all created using `tests/benchmark_comparison_test.py`, which should be run
from the `tests` directory. 

The `tests/data/mem_volume_through_cache*` visualizations and data are created
with `tests/mem_volume_through_cache.py`, which should be run from the `tests` directory.

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
