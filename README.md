# Fine Hardware Visualization
This software is a work and process. It is designed to present the user with a
visualization of their computer architecture and indicate what parts of that
architecture are most loaded to identify bottlenecks in high-performance
applications.

- [Fine Hardware Visualization](#fine-hardware-visualization)
- [Prerequisites](#prerequisites)
- [Running](#running)
- [Usage Notes](#usage-notes)
- [Goals:](#goals)
- [Architecture of Program](#architecture-of-program)
- [TODO:](#todo)
  - [Immediate:](#immediate)
  - [Long-term:](#long-term)
    - [Problems to fix:](#problems-to-fix)
    - [Features to add:](#features-to-add)
- [Other similar tools:](#other-similar-tools)
  - [Kerncraft:](#kerncraft)
  - [Others:](#others)
- [Accomplishments:](#accomplishments)
  - [2020-03-24 through 2020-03-30](#2020-03-24-through-2020-03-30)
    - [What other people are doing](#what-other-people-are-doing)
  - [2020-03-17 through 2020-03-24](#2020-03-17-through-2020-03-24)
    - [Memory](#memory)
    - [What other people are doing](#what-other-people-are-doing-1)
  - [2020-03-10 through 2020-03-17](#2020-03-10-through-2020-03-17)
    - [Memory: tried to align memory manual calculations with likwid report](#memory-tried-to-align-memory-manual-calculations-with-likwid-report)
    - [Convolution as a case study](#convolution-as-a-case-study)
      - [When both groups were started/stopped:](#when-both-groups-were-startedstopped)
      - [When only actual convolution was inside group:](#when-only-actual-convolution-was-inside-group)
      - [Analysis](#analysis)
    - [QOL and software engineering](#qol-and-software-engineering)
  - [2020-03-03 through 2020-03-10](#2020-03-03-through-2020-03-10)
  - [2020-02-25 through 2020-03-03](#2020-02-25-through-2020-03-03)
  - [2020-02-18 through 25](#2020-02-18-through-25)
  - [2020-02-11 through 18](#2020-02-11-through-18)
    - [Misc. discoveries:](#misc-discoveries)
    - [Integer operations:](#integer-operations)
    - [Sampling:](#sampling)
    - [Number of Registers for hardware counters](#number-of-registers-for-hardware-counters)
    - [Threads and migration](#threads-and-migration)
  - [before 2020-02-11](#before-2020-02-11)
    - [Some notes on what does and doesn't get counted:](#some-notes-on-what-does-and-doesnt-get-counted)

# Prerequisites
 - **likwid:** it is preferred you build likwid from source and install to
   `/usr/local`, as this is the only confirmed way to use `likwid-accessD`
   without root permissions. Instructions to do this are available
   [here](https://github.com/RRZE-HPC/likwid) Alternatively, you can also
   install it with your package manger (ex. `sudo apt install likwid` on
   ubuntu)
 - **[nlohmann/json](https://github.com/nlohmann/json):** header-only, included
   in ./lib
 - **boost/program_options:** available on [the boost
   website](https://www.boost.org/). Also installable on ubuntu with `sudo apt
   install libboost-program-options-dev`

# Running
To build and run the (currently limited) test suite, run `make tests`. It is
also possible to benchmark your machine by running `make bench`. 

# Usage Notes
 - Region names must not have spaces
 - Chapter 19 of volume 3 of the Intel software developer's manual (page 3605
   in the combined digital version) has hardware counter names

# Goals:
"I think we have a shot at doing something other people don't do" - Dr. Saule

Dr. Saule mentioned two ways this project will be useful
 - help better understand code and how the author can improve it
 - help better understand the architecture

We want people new to HPC to be able to 
 - understand how their application maps to the architecture
 - give suggestions on how to improve their application

Additionally, we hope to apply this to graph problems: kernels in graph
problems tend to change behavior throughout execution

# Architecture of Program
 - Identify hardware architecture
 - Identify peak FLOP/s, memory bandwidth, etc.
 - Identify latency
 - Measure what actual utilization of memory/processor is
 - Compare actual utilization with peak on an piece-by-piece basis
 - Visualize that

# TODO:
## Immediate:
 - look into approaches of others
   - what are people using these counters for?
   - Is anyone doing things like this?

 - make convolution into a case study
   - nothing is reporting as being saturated... but maybe we are saturating one
     scalar single precision float unit? 
   - must be something with CPU, because memory is not the bottleneck. 
   - "entire_program" region doesn't work: remove everything but region and
     then rebuild from there
   - look at ports! - is there a counter for total number of uops dispatched
     - uops_issued_any
   - PORT_USAGE: split into 2 groups
   - identify types of instructions, identify parts of ports that are being
     used? 
   - identify how many instructions are getting piped from the front end to the
     back end?
   - look into metric for number of instructions decoded?
   - Visualize usage
   - aggregate results by region?? Are nested regions allowed?

 - memory
   - "we need to understand how accurate these counters are and how they map to
     what we expect them to do"
   - read what every programmer should know about memory
   - inspect assembly: are we using instructions that load less than a
     cacheline? 
      - should have equal load and store instructions

 - CLI which benchmarks and process JSON into svg
   - this will probably just come as I work on other stuff, because it'll be
     easier to visualize than read text
   - main part of program dumps info, second part reads and evaluates and
     creates svg
   - generate svg
   - libcairo is an option for graphics

## Long-term:
### Problems to fix:
 - manual benchmark only prints runtime for flops region
   - in other words, runtime_by_tag doesn't seem to work for more than one 
     region
 - in convolution, the "entire_program" tag, which is designed to measure
   across all stages of code, doesn't work. However, the tag "convolution"
   inside the actual convolution does work.
 - software engineering
   - move benchmark stuff in fhv.cpp to separate file
     - combine with computation_measurements and just make it "benchmark"?
   - combine benchmark in fhv with benchmark-likwid-vs-manual
     - rewrite computation_measurements to optionally include manual results
     - update CLI to optionally include manual results
   - rename "computation_measurements" to "measurements"?
   - replace printf statements with cout
   - combine all memory bandwidth functions
   - make it consistent when using variable name and raw string to refer to
     counters
 - things CLI will need to do:
   - benchmark machine
   - create visualization from output data

### Features to add:
 - expand suite of test software that has balanced/imbalanced usage
   - consider standard benchmarks
     - like NAS parallel benchmarks
 - improve benchmark
   - have it check bandwidth for all types of memory/cache
   - have it check architecture to know what size of caches
   - have it populate architecture.h
   - improve software engineering: make it consistent what calls likwid, etc.
 - have LIKWID_THREADS environment variable get set dynamically instead of hard
   coded

# Other similar tools:
## Kerncraft:
There's a
[paper](https://link.springer.com/chapter/10.1007%2F978-3-319-56702-0_1) about
it. The benchmark tool should be evaluated, we can draw from it.

 - uses IACA to generate in-core predictions
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
     created
 - kerncraft does automatically benchmark a lot of great things (bandwidths
   are a notable example) but also provides much more information than we
   are just planning on benchmarking
 - I feel like I could spend weeks just learning everything about kerncraft

## Others:
 - Intel PCM
 - HPC toolkit "Tau" out of Oak Ridge
 - other laboratory toolkits
 - Vampir
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

# Accomplishments:
## 2020-03-24 through 2020-03-30
### What other people are doing
 - Read a lot about kerncraft, [added a section on it](#kerncraft) in the
   `other similar tools` section.

## 2020-03-17 through 2020-03-24
### Memory
 - MEM_INST_RETIRED_ALL_LOAD/STORE count all retired load/store instructions,
   respecitvely. See Table 19-3 of "Performance monitoring events" in intel
   developer's guide
 - using these to get load/store ratios gave us ratios of 4-6x reads to writes
   (see `tests/mem_volume_through_cache_load_to_store.png`)
 - Tried to get absolute measurement of volume of data transferred. 
   - Couldn't find counter of L1 memory instructions besides evict/read (which
     we already use in L2 measurements) so I tried using
     MEM_INSTR_RETIRED_LOADS_ALL. See
     `tests/mem_volume_through_cache_total_volume.png`
     
### What other people are doing
 - browsed some projects on github
   - searching "hardware counters" turned up countless projects that just
     expose hardware counters in language X
 - a few small projects: [cpm](https://github.com/wichtounet/cpm) had last
   commit in 2018, ~40 stars
 - stumbled into a cool project called
   ["Kerncraft"](https://github.com/RRZE-HPC/kerncraft)
   - There's an [academic
     paper](https://link.springer.com/chapter/10.1007%2F978-3-319-56702-0_1)
     about it
   - by same group that owns likwid (RRZE-HPC)
   - active: last commit today
   - somewhat popular, ~50 stars.
   - uses likwid and IACA (Intel Architecture Code Analyzer, a static analysis
     tool)
   - this project seems to focus on memory and caching
   - not yet super sure how it works and what it produces, I'll download it and
     play with it
 - [Intel PCM](https://software.intel.com/en-us/articles/intel-performance-counter-monitor)
   - does not visualize hardware but visualizes memory usage compared to
     computation intensive things

## 2020-03-10 through 2020-03-17
### Memory: tried to align memory manual calculations with likwid report
 - likwid is reporting less data transferred, even in best case manually
   calculated transfer amounts are 1.25x the likwid reported ones. See
   `./tests/mem_size_comparison_size_ratio.png` 
 - compared ratio of reads to writes in each level of cache/memory
   - expected ratio to be 2:1 in cache and 1:1 in memory.. not the case
   - at small volumes, ratio of reads to writes was very high: maybe writeback
     just hasn't happened while we're instrumenting things?
   - at high volumes, L2 was 2:1, ram was about 1.5:1, and L3 was about 1:1
     (see the chart `./tests/mem_volume_through_cache_ratios.png`)
 - compared volume of data through every level of cache/memory (see the charts
   mem_volume_through_cache_X.png where X is l2, l3, and memory)
   - expected volume reported in RAM and caches to match at higher volumes.
     This was the case except for with L2, which was significantly lower. See
     mem_volume_through_cache_total_volume.png
 - the counter "COREWB" (Counts the number of modified cachelines written
   back.) may be useful here
    - doesn't work on my arch (skylake). It works on Haswell according to the
      intel developer's guide

### Convolution as a case study
 - worked on debugging why entire_program region doesn't work
   - Initial theories proved not to be correct. Some things I considered:
     - likwid doesn't allow nested groups. But, even after removing inner group
       was not fixed
     - trying to initialize in a parallel block caused it to fail because the
       threads were getting destroyed or something, but even when starting and
       stopping the group in a sequential block, no results.
     - maybe it was getting counted but just not in all threads or something?
       But aggregate results were the same whether the region `entire` was
       specified or not
   - googled the error I got, only result was to the source code where the
     error is printed
   - I'm not even sure how to ask a question about this. Maybe need to make a
     minimal example to post to discussion board? Just ask "what's wrong?"?
 - discovered PORT_USAGE performance group that we can look into to analyze
   stuff inside the core. Will need to split it into two groups though

#### When both groups were started/stopped:
----- begin saturation level performance_monitor report -----
Percentage of available DP [MFLOP/s] used: 1.05821e-05
Percentage of available L2 bandwidth [MBytes/s] used: 0.0242764
Percentage of available L3 bandwidth [MBytes/s] used: 0.0197555
Percentage of available Memory bandwidth [MBytes/s] used: 0.108281
Percentage of available SP [MFLOP/s] used: 0.0298234
----- end saturation level performance_monitor report -----

#### When only actual convolution was inside group:
----- begin saturation level performance_monitor report -----
Percentage of available DP [MFLOP/s] used: 8.11656e-06
Percentage of available L2 bandwidth [MBytes/s] used: 0.0215583
Percentage of available L3 bandwidth [MBytes/s] used: 0.0364879
Percentage of available Memory bandwidth [MBytes/s] used: 0.0938187
Percentage of available SP [MFLOP/s] used: 0.0249263
----- end saturation level performance_monitor report -----

#### Analysis
Results are similar.... saturation levels differ on speedtest but the kernel is
also bigger

### QOL and software engineering
 - Performance monitor now automatically aggregates all metrics and events
 - flops plots now have annotated times of execution
 - Figured out how to supply multiple performance groups in code
 - **Software Engineering:** trying to build a deliverable as quickly as
   possible has caused some problems with unmaintainable code, so I spent some
   time this week on software engineering stuff
   - improved performance monitor to automatically aggregate every event and
     metric 
     - this dramatic refactoring has made me aware of the need for **unit
       tests**.... should we do that soon?
   - would be nice to combine benchmark and benchmark_compare_likwid_manual

## 2020-03-03 through 2020-03-10
 - tried to align manual memory benchmark and likwid benchmark, learned a few
   things: 
   - manual benchmark now counts read and write in each iteration (2 ops per
     iteration) 
   - found some related intrinsics: 
     - _mm256_stream_load_si256 reads non-temporally
     - _mm256_stream_pd stores non-temporally
     - streaming loads only seem to be supported with integer data for some
       reason 
     - (non-temporal means it goes into queue as LEAST-recently read thing
       instead of most-recently read thing)
   - aligning memory is proving to be harder than aligning flops. Amount of
     memory reported as transferred by likwid changes each time. However, it
     also never exceeds the manually calculated amount
   - likwid measures:
     - Memory load bandwidth [MBytes/s]  1.0E-06*DRAM_READS*64.0/time
     - Memory evict bandwidth [MBytes/s]  1.0E-06*DRAM_WRITES*64.0/time
   - so it seems likwid measures read and write. Does not seem to count the
     read for ownership part of the write as a read operation, as there were
     about 20% more reads than writes in benchmark, which is expected to be at
     least double the number of writes if read for ownership is counted.
   - maybe there's no read for ownership since L3 cache is shared across all
     cores and there's only one processor
 - aligned manual FLOP benchmark with likwid benchmark
   - made graphs to show reports from each, they get closer as time spent
     computing increases
 - CLI 
   - ended up writing cli for manual vs likwid because that's what I was
     working with the most. Still useful to figure out program options
   - couldn't get program_options to work, either from apt or when building from
     source. Would get linker errors even with supplied examples
     - PROBLEM WAS THE ORDER OF FLAGS GIVEN TO G++ WOW I SPENT LIKE 2 HOURS ON
       THIS. You have to put -lboost_program_options AFTER the file you're
       linking 

## 2020-02-25 through 2020-03-03
 - double check bandwidth by doing manual calculations
   - only one core is reporting work even though I'm using multiple threads...
   - is bandwidth too low to be taken advantage of by all cores? Or does the
     memory controller only allow one core at a time to use it? Or maybe it's
     just how likwid reports things?
   - when I make code sequential, all threads but thread 0 report NAN values
     for ram-related stuff. Also bandwidth is halved. I think this is just how
     likwid reports things?
   - results are off by a factor of 2...
 - added HPC convolution to test
   - instrument loading/saving data too - different phases to application
   - this has been added, but only able to instrument actual convolution for
     some reason?
   - some minor fixes: aggregates all fp operations, not just avx ones
 - changed performance_monitor functions to static
 - outputs data to JSON now
 - started executable which will bench system and visualize from json
 - multiple groups now specified

## 2020-02-18 through 25
 - planning on using svgpp for svg generation https://github.com/svgpp/svgpp
 - not sure how to supply multiple groups from within code...
   - if I can find a way to close and re-init without segfaulting, I could just
     do that
   - feels a little hacky though.
   - I asked on the likwid-users google group if there's a way to specify
     multiple groups using the environment variable LIKWID_EVENTS
 - would this be easier to write as a likwid extension?
 - hardcode architecture and result of benchmark

The way likwid measures cache bandwidth is interesting. Following is an example
with L2 cache: 
 - measures L1D_REPLACEMENT, L1D_M_EVICT, and ICACHE_64B_IFTAG_MISS
 - calculates the following: 
   - L2D load bandwidth [MBytes/s] = 1.0E-06*L1D_REPLACEMENT*64.0/time
   - L2D load data volume [GBytes] = 1.0E-09*L1D_REPLACEMENT*64.0
   - L2D evict bandwidth [MBytes/s] = 1.0E-06*L1D_M_EVICT*64.0/time
   - L2D evict data volume [GBytes] = 1.0E-09*L1D_M_EVICT*64.0
   - L2 bandwidth [MBytes/s] =
     1.0E-06*(L1D_REPLACEMENT+L1D_M_EVICT+ICACHE_64B_IFTAG_MISS)*64.0/time
   - L2 data volume [GBytes] =
     1.0E-09*(L1D_REPLACEMENT+L1D_M_EVICT+ICACHE_64B_IFTAG_MISS)*64.0

Do we want to separate out load/evict?

Learned some things about memory:
 - counters associated with DRAM obtained when running brandon's bw program
   with size 100000 and number of iterations 10:
   - DRAM_READS:MBOX0C1: 18446740000000000000.000000
   - DRAM_WRITES:MBOX0C2: 321320900.000000
   - Metric Memory bandwidth [MBytes/s]: 464390389739345.750000 (this seems
     unreasonably high...)
 - there are benchmarks for memory. For instance `likwid-bench -t copy -w
   S0:100MB`

these came from the group "MEM_DP", which also happened to include a lot of
information about DP flops

## 2020-02-11 through 18
### Misc. discoveries:
 - discovered "vectorization ratio" metric in likwid
 - there are counters for NVIDIA gpus - use the -W flag in likwid-perfctr
 - event groups are stored in $(PREFIX)/share/likwid/perfgroups/$(ARCH)/
   - ex /usr/local/share/likwid/perfgroups/skylakeFLOPS_SP.txt
 - custom groups can be specified as described here:
   https://github.com/RRZE-HPC/likwid/wiki/likwid-perfctr#using-custom-event-sets
 - likwid measures by hardware thread; hardware counter registers are part of
   context switch when hyperthreading
 - doesn't seem possible to re-initialize likwid inside code, so you can't
   change the group name once it's set without restarting the executable.
 - likwid_startCounters() should be run once in sequential part of code,
   otherwise counters were getting restarted and results were not correct. I
   moved it to "init" in performance_monitor.
 - can use likwid-accessD to avoid need for sudo. This also monitors at hardware
   level, results are consistent with using direct access.

### Integer operations:
 - Measuring integer operations is tough... 
   - some counters count both FP and INT operations, could in theory subtract
     FP ops. (See ARITH.MUL, MUL, DIV)
   - some counters only count integer vector things (see
     SIMD_INT_128.PACKED_MP&). Unfortunately, I didn't see anything for 256B or
     512B registers

### Sampling:
 - no extrapolation is done on counters.
 - when specifying multiple groups, likwid switches which group is tracked when
   likwid_markerNextGroup is called if the marker api is used. Else, the group
   being tracked switches after the time specified with the -T flag passes
   (default 2s)
   - for example: `likwid-perfctr -C S0:0 -g FLOPS_DP -g L3 -g L3 -T
     250ms -M 1 ./a.out`

### Number of Registers for hardware counters
 - on my CPU (intel i5-6300U, skylake) there are 4 customizable hardware
   counter registers per hardware thread and 8 are available per hardware core
   if hyperthreading is disabled. These are numbered PMC0-7
 - the registers used for hardware counters are part of the context switch!
 - FIXC0-2 are not customizable

### Threads and migration
 - NOTE: expected performance is 371 GFlop/s
 - pin 1 omp thread to each physical thread 
   - 193.8 GFlop/s
 - pin 1 omp thread to each physical core
   - pinned one omp thread to core 0 and one thread to core 2
   - got 100.1 GFlop/s
 - pin multiple omp threads to one physical thread
   - according to
     https://github.com/RRZE-HPC/likwid/wiki/TutorialMarkerC#problems , this
     is not supported. Access to hash table entries is not thread safe
   - I tried it anyways. 
     - most threads reported "-22" for all counters... some kind of code?
     - still did all operations
     - reported 56.8 GFlop/s, which is about 1/4 our 4-core measurement and
       1/2 our 2-core measurement
     - results were incredibly inconsistent and changed based on if I used
       accessD or direct access.
 - general notes on thread migration
   - see tests/thread_migration.cpp
   - used likwid_pinThread to migrate threads
   - note: IDs reported by omp_get_thread_num do not change when cores are
     migrated! I assume this is because the IDs are unique to OpenMP threads,
     not physical cores
   - SCHED_GETCPU(3) will tell you what core is being used
 - swap odd and even cores
   - only even-numbered omp threads were allowed to do work, but odd cores
     also reported data. This indicates that likwid checks on a hardware
     thread level and not by omp thread.
 - instead of swapping even and odd, started on cores 1 and 2 and moved to
   cores 3 and 4
   - when I did start region -> run1 -> migrate -> run2 -> stop region, I
     got "WARN: Stopping an unknown/not-started region flops". So sometimes
     one thread would start a region and another would stop it... this is
     interesting, and further reinforces the theory that regions and stuff
     is done on a hardware-thread level
   - adding "#pragma omp barrier" after starting regions and before stopping
     them" made everything report work every time
   - for all cores to report work, region had to be started before doing
     work/migration/work and then stopped after all that was done. Doing
     start/work/stop/migration/start/work/stop caused only some cores to report
     work.
 - move single thread from core 0 to core 2
   - seems that if you start region on one core and stop on another, nothing
     is reported
   - starting region on one core, switching to another, doing work,
     switching back to first, and stopping region causes only first core to
     report work
   - results inconsistent.... not sure what this means yet

## before 2020-02-11
 - evaluated both likwid and papi for use
 - investigated likwid-bench
 - basic research on likwid-accessD vs direct access
 - Got likwid marker to measure code
   - investigated brandon's code
   - got my code working
 - Investigated how likwid aggregates
   - aggregates by region but on a per-thread basis
   - if two regions have the same name and they are on the same thread, they
     will be aggregated.
 - Group "FLOPS_SP" and "FLOPS_DP" seem useful.

### Some notes on what does and doesn't get counted:
FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE STAT counts one vector operation as
one retired instruction. 
It counts one vector FMA operation as 2 retired instructions

AVX SP MFLOP/s counts vector operation as 8 floating point operations: This
is what we want

so aggregate AVX SP MFLOP/s should correspond with what we expect on bench

