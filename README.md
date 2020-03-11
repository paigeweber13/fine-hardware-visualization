# Fine Hardware Visualization
This software is a work and process. It is designed to present the user with a
visualization of their computer architecture and indicate what parts of that
architecture are most loaded to identify bottlenecks in high-performance
applications.

- [Fine Hardware Visualization](#fine-hardware-visualization)
- [Prerequisites](#prerequisites)
- [Running](#running)
- [Usage Notes](#usage-notes)
- [Architecture of Program](#architecture-of-program)
- [Goals:](#goals)
- [TODO:](#todo)
  - [Immediate:](#immediate)
  - [Long-term:](#long-term)
    - [Problems to fix:](#problems-to-fix)
    - [Features to add:](#features-to-add)
- [Accomplishments:](#accomplishments)
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
   without root permissions. Alternatively, you can also install it with your
   package manger (ex. `sudo apt install likwid` on ubuntu)
 - **nlohmann/json:** available [on github](https://github.com/nlohmann/json).
   Also installable on ubuntu with `sudo apt install nlohmann-json-dev` 
 - **boost/program_options:** available on [the boost
   website](https://www.boost.org/). Also installable on ubuntu with `sudo apt
   install libboost-program-options1.65-dev`

# Running
To build and run the (currently limited) test suite, run `make tests`

# Usage Notes
 - Region names must not have spaces

# Architecture of Program
 - Identify hardware architecture
 - Identify peak FLOP/s, memory bandwidth, etc.
 - Identify latency
 - Measure what actual utilization of memory/processor is
 - Compare actual utilization with peak on an piece-by-piece basis
 - Visualize that

# Goals:
 - main goal is to give people new to HPC something they can use to:
   - understand how their application maps to the architecture
   - give suggestions on how to improveme their application
 - apply this to graph problems: kernels in graph problems tend to change
   behavior throughout execution

# TODO:
## Immediate:
 - try to align manual memory benchmark and likwid benchmark:
   - seems that only reads are getting counted by likwid?
   - Don't forget you have to read for ownership before you write to a cache
     line 
   - manual benchmark doesn't count how there's a read and then a write in each
     iteration
   - some related intrinsics:
     - streamingread is a non-temporal read (so it goes into queue as
       LEAST-recently read thing instead of most-recently read thing)
   - aligning memory is proving to be harder than aligning flops. Amount of
     memory reported as transferred by likwid changes each time. However, it
     also never exceeds the manually calculated amount
     - possible causes include:
     - some iterations of loop getting optimized out?
     - getting optimized to memcpy?
 - try to align manual FLOP benchmark with likwid benchmark
   - does difference decrease as computation size increases?
     - YES
   - make graphs of different metrics for different numbers of iterations
     - DONE
 - make convolution into a case study
   - google error I get when trying to instrument entire pipeline
   - nothing is reporting as being saturated... but maybe we are saturating one
     scalar single precision float unit? 
   - must be something with CPU, because memory is not the bandwidth. 
   - identify how many instructions are getting piped from the front end to the
     back end?
   - metric for number of instructions decoded?
   - identify types of instructions, identify parts of ports that are being
     used? 
 - CLI which benchmarks and process JSON into svg
 - generate svg
   - main part of program dumps info, second part reads and evaluates and creates
     svg
      - use json?
   - libcairo is an option for graphics

## Long-term:
### Problems to fix:
 - manual benchmark off by a factor of 2 - investigate
 - manual benchmark only prints runtime for flops region
   - in other words, runtime_by_tag doesn't seem to work for more than one 
     region
 - in convolution, the "entire_program" tag, which is designed to measure
   across all stages of code, doesn't work. However, the tag "convolution"
   inside the actual convolution does work.

### Features to add:
 - expand suite of test software that has balanced/inbalanced usage
   - consider standard benchmarks
     - like NAS parallel benchmarks
 - improve benchmark
   - have it check bandwidth for all types of memory/cache
   - have it check architecture to know what size of caches
   - have it populate architecture.h
   - improve software engineering: make it consistent what calls likwid, etc.
 - have LIKWID_THREADS environment variable get set dynamically instead of hard
   coded
 - software engineering
   - rename "computation_measurements" to "measurements"?
   - replace printf statements with cout
   - combine all memory bandwidth functions
 - things CLI will need to do:
   - benchmark machine
   - create visualization from output data

# Accomplishments:
## 2020-03-03 through 2020-03-10
 - try to align manual memory benchmark and likwid benchmark:
   - likwid measures:
   - Memory load bandwidth [MBytes/s]  1.0E-06*DRAM_READS*64.0/time
   - Memory evict bandwidth [MBytes/s]  1.0E-06*DRAM_WRITES*64.0/time
   - so it seems likwid measures read and write. Does not seem to count the
     read for ownership part of the write as a read operation, as there were
     about 20% more reads than writes in benchmark, which is expected to be at
     least double the number of writes if read for ownership is counted.
 - try to align manual FLOP benchmark with likwid benchmark
 - make convolution into a case study
 - CLI which benchmarks and process JSON into svg
   - couldn't get program_options to work, either from apt or when building from
     source. Would get linker errors even with supplied examples
     - PROBLEM WAS THE ORDER OF FLAGS GIVEN TO G++ WOW I SPENT LIKE 2 HOURS ON
       THIS. You have to put -lboost_program_options AFTER the file you're
       linking 
 - generate svg

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
 - can use likwid-accesD to avoid need for sudo. This also monitors at hardware
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
   - starting region on one core, siwtching to another, doing work,
     siwtching back to first, and stopping region causes only first core to
     report work
   - results inconsistent.... not sure what this means yet

## before 2020-02-11
 - evaluated both likwid and papi for use
 - investigated likwid-bench
 - basic research on likwid-accesD vs direct access
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
It counds one vector FMA operation as 2 retired instructions

AVX SP MFLOP/s counts vector operation as 8 floating point operations: This
is what we want

so aggregate AVX SP MFLOP/s should correspond with what we expect on bench

