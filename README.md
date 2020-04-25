# Fine Hardware Visualization
This software is a work and process. It is designed to present the user with a
visualization of their computer architecture and indicate what parts of that
architecture are most loaded to identify bottlenecks in high-performance
applications.

This software is in the early stages of development and should not yet be
assumed to be stable or correct.

- [Fine Hardware Visualization](#fine-hardware-visualization)
- [Prerequisites](#prerequisites)
- [Running](#running)
- [Usage Notes](#usage-notes)
- [Goals:](#goals)
  - [By end of semester](#by-end-of-semester)
- [Architecture of Program](#architecture-of-program)
- [Ownership and licensing](#ownership-and-licensing)
- [TODO:](#todo)
  - [Immediate:](#immediate)
    - [What other people do](#what-other-people-do)
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
- [Accomplishments:](#accomplishments)
  - [2020-04-16 through 2020-04-23](#2020-04-16-through-2020-04-23)
  - [2020-04-09 through 2020-04-16](#2020-04-09-through-2020-04-16)
    - [Learning likwid](#learning-likwid)
    - [Exploration](#exploration-1)
  - [2020-03-24 through 2020-04-09](#2020-03-24-through-2020-04-09)
    - [Playing with likwid_minimal.c](#playing-with-likwidminimalc)
    - [Improvements to performance_monitor](#improvements-to-performancemonitor)
    - [What other people are doing](#what-other-people-are-doing)
    - [Convolution as a case study](#convolution-as-a-case-study)
      - [Investigating port usage](#investigating-port-usage)
      - [Applying port usage information to convolution](#applying-port-usage-information-to-convolution)
    - [Memory](#memory)
      - [For the final 3 iterations:](#for-the-final-3-iterations)
        - [Manually calculated volumes:](#manually-calculated-volumes)
        - [Using intrinsic](#using-intrinsic)
        - [Direct assignment](#direct-assignment)
  - [2020-03-17 through 2020-03-24](#2020-03-17-through-2020-03-24)
    - [Memory](#memory-1)
    - [What other people are doing](#what-other-people-are-doing-1)
  - [2020-03-10 through 2020-03-17](#2020-03-10-through-2020-03-17)
    - [Memory: tried to align memory manual calculations with likwid report](#memory-tried-to-align-memory-manual-calculations-with-likwid-report)
    - [Convolution as a case study](#convolution-as-a-case-study-1)
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
 - **likwid >= 5.0.1:** a version above 5.0.1 is required, as this has support
   for memory counters and is confirmed to use `likwid-accessD` without root
   permissions. If this version is available with your package manager, use
   that. Otherwise, build it from source. Instructions to do this are available
   [here](https://github.com/RRZE-HPC/likwid)
 - additional perfgroups not included with likwid. These can be installed by
   running `make perfgroups` in the root directory.
 - **[nlohmann/json](https://github.com/nlohmann/json):** header-only, included
   in ./lib
 - **boost/program_options:** available on [the boost
   website](https://www.boost.org/). Also installable on ubuntu with `sudo apt
   install libboost-program-options-dev`

# Running
**warning:** in an effort to reduce workload until I get a minimal proof of
concept, I've stopped maintaining basically everything in this repository
(benchmarks, tests are notable examples). So don't expect `make tests` (test
suite) or `make bench` to work right now.

What still works:
 - minimal example of likwid: `make run-tests/likwid-minimal`
 - minimal example of perfmon: `make run-tests/fhv-minimal`
 - convolution: `cd convolution-fast && make test`

# Usage Notes
 - Region names must not have spaces
 - Chapter 19 of volume 3 of the Intel software developer's manual (page 3605
   in the combined digital version) has hardware counter names

# Goals:
"I think we have a shot at doing something other people don't do" - Dr. Saule.
So far it seems that nothing is designed to automatically identify and
visualize the architecture by socket and within the core. Additionally, most of
these tools are not easy to use.

Dr. Saule mentioned two ways this project will be useful
 - help better understand code and how the author can improve it
 - help better understand the architecture

We want people new to HPC to be able to 
 - understand how their application maps to the architecture
 - give suggestions on how to improve their application

Additionally, we hope to apply this to graph problems: kernels in graph
problems tend to change behavior throughout execution

## By end of semester
Have something that shows how bottleneck changes based on parameters. We are
less concerned about visualization, just data to confirm.

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
 - [ ] halide for convolution
   - [ ] can halide create c code?
 - [ ] get highlight performance data to show how bottleneck changes, show that
       it matches theoretical data

 - [ ] new per_thread_results seems to be small on events.
   - [ ] also, it reports stuff for threads that I don't have??? Like thread 11

Goal for end of semester: have good measurements that show that the bottleneck
changes as you adjust the parameters
 - do manual measurements with convolution to ensure code behaves that way.
 - demonstrate it with hardware counters
 - make this my focus, other stuff can be taken care of as it becomes necessary

### What other people do
 - [ ] read kerncraft paper

### Exploration
 - [ ] create "printHighlights" function that just prints stuff associated with
       saturation and port usage
 - [ ] investigate port usage in convolution: do numbers make sense?
 - [ ] mem instructions retired * 32 bytes instead of 64
   - this is because there are 2 32-byte busses?
     [Yes!](https://en.wikichip.org/w/images/thumb/7/7e/skylake_block_diagram.svg/1350px-skylake_block_diagram.svg.png)
   - this architecture only moves 32-bytes (probably because 32-byte
     vectors are the biggest they can do)
 - [ ] fix output to JSON

### Likwid stability issues
 - [ ] port counters sometimes reporting 1.8e19 for values
 - [ ] convolution sometimes not instrumenting one region?
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

### Features to add:
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

# Accomplishments:
## 2020-04-16 through 2020-04-23
Other Tools:
 - looked at TAU
   - powerful and detailed but seems to target large clusters.
   - tools are highly specified, but there may be tools for fine-grained
     modeling 
   - not intuitive
 - looked at vampir
   - closer to what we are aiming for, as it provides a mapping of function to
     performance on a fine-grained level
   - does not seem to consider architecture beyond standard core-cache-memory
     common to all modern commodity hardware.
   - not free?
 - read more of kerncraft paper

Kernel to use to demonstrate how this tool analyzes changing bottlenecks:
 - revisited convolution
   - my theoretical analysis says memory is only the bound at kernel sizes < 5
   - in practice, I don't see a roofline from memory, but perhaps I'm not using
     core to the fullest extent?
   - for now will move on to looking at it with likwid to see if I can gain
     insights

improvements to performance_monitor tool:
 - added geometric mean
 - map is built for per-thread results

Checking per-thread result map for accuracy:
 - events are off by a factor of 4 or so
 - metrics seem to match
 - events are reported for non existent threads, like
 - compared new geometric mean and double checked old arithmetic mean with
   likwid_minimal. Everything seems good, moving on for now. But we will need
   formal tests at some point.

next steps: 
 - decide on analyzing convolution or another kernel
 - use likwid to print the most pertinent information
 - create json
 - visualize (low-priority)

Questions:
 - fix up convolution or just switch to other kernel for analysis?

## 2020-04-09 through 2020-04-16
Stuff from last week:
 - Switched intrinsics to operator= (see [memory section](#memory)), compared
   compiler-produced assembly

Questions:
 - how do we know port usage numbers make sense?

### Learning likwid
 - pinning threads is NOT optional, but cli does it for you if you use it.
   - failure to pin threads is what was giving me problems when specifying
     groups with environment variables

### Exploration
 - Shifted convolution to use direct likwid calls except to aggregate and print
   results
 - For performance_monitor result printing: added ability to average results
   across core, because summing "port_usage" ratios doesn't really make sense
   - I haven't looked at convolution now that I've made these changes because I
     want to verify them again with benchmark_likwid_vs_manual

## 2020-03-24 through 2020-04-09
Main points:
 - using my "performance_monitor" adds another layer of complexity that makes
   it hard to debug problems. I barely know how to use likwid, and I'm already
   writing my own library?
   - I feel like I am trying to reinvent the wheel... maybe instead of writing
     wrappers for "init" and "close" and such we can just require them to use
     the likwid calls but have a separate close/print/etc.
 - have a way to measure usage by ports
 - instrumenting entire "convolution" program works if you don't have the call
   to next_group, with some exceptions (see next point)
 - making the number of iterations a multiple of the number of groups
   *sometimes* fixes the preceding problem.
   - this only works sometimes, and seems to work more frequently with lower
     numbers of groups (the max number of groups I've been able to successfuly
     do is 6). In general, it feels very non-deterministic and it's very
     frustrating. 
   - needs more investigation now that bug has been fixed
 - vector of doubles is only half the size of a cacheline. Considering every
   operation as one cacheline worth of transfer seems to be the reason those
   values are higher by a factor of 2. But why is vmovapd (move **aligned**
   packed double) allowing us to move *half* a cacheline worth of data? Why am
   I able to use this intrinsic without error even when I supply an address
   halfway through a cacheline?
   - "aligned" does mean aligned to cacheline boundary BUT it also means that
     if you have a double aligned to cacheline, the next double still counts as
     "aligned" for those purposes.
   - internally the memory controller manages an entire cache line but gives
     the core just a part of a cache line
 - using `operator=` instead of intrinsics gives higher transfer volumes that
   are closer to manually calculated volumes
 - improved likwid_minimal
   - behavior changes if I compile with gcc or g++: maybe copy is getting
     optimized out?
 - between `benchamrk`, `benchmark-likwid-vs-manual`, `thread_migration`, and
   `convolution` there is a LOT to change whenever I make changes to
   performance_monitor. For now I'm going to leave them broken and just update
   fhv_minimal. If we use them in the future I think I'm going to switch to
   just calling likwid until fhv is more stable
 - found it difficult to search through the vast amount of stuff other people
   are doing
 - in the case of kerncraft, they present a roofline model by level of cache
   and memory as well as a scalability model

Feeling a little overwhelmed. I've struggled this week and it's made me aware
of how little I know. I feel like I need to learn about:
 - how likwid works. Read the documentation and examples.
 - how caching and memory works
 - how floating-point arithmetic works
 - what exactly other people are doing

Not to mention actually building this library

### Playing with likwid_minimal.c
Through the likwid-users group, I discovered that you have to put barriers
betwen stop/start regions

Noticed some really weird behavior. 
 - reproduced bug in convolution where if I supply too many groups, program
   hangs on likwid_markerClose.
   - ran likwid_minimal with the command `likwid-perfctr -C S0:0-3 -g FLOPS_DP
     -g MEM -g L3 -g L2 -M 1 -m ./likwid_minimal` to reproduce
   - asked about this on the likwid_users group: see
     https://groups.google.com/forum/#!topic/likwid-users/XDLIHYdeRy4 
 - if I compile with g++ things work perfectly. If I compile with gcc, I get
   the error "WARN: Stopping an unknown/not-started region double_flops"
 - v5.0.1 is buggy, v4.3.4 does not support counters for memory (like MBOX0C1)
   - the bug is detailed here:
     https://groups.google.com/forum/#!topic/likwid-users/XDLIHYdeRy4 
   - v5.0.1 was fixed with commit [52d450](https://github.com/RRZE-HPC/likwid/commit/52d45038ba7dbc7e41c0d44818367fb891257b47)

### Improvements to performance_monitor
 - now separates sequential and parallel regions, which are registered in init
 - overloaded init to allow specifying a number of threads instead of
   enumerating them in a c string
 - created wrapper function for nextGroup 

### What other people are doing
 - feel like I could spend weeks just learning about what other tools do
 - Read a lot about kerncraft, [added a section on it](#kerncraft) in the
   `other similar tools` section. Highlights follow:
   - only instruments loops, not multi-stage programs
   - does an automatic benchmark
   - seems architecture information has to be manually written
 - one of my biggest questions: How can we better take advantage of what other
   people are using?
   - use likwid-perfctr command line? But then we'd have to scrape the output
     to use results programmatically
   - use likwid_benchmark_auto.py to benchmark code?

### Convolution as a case study
Commenting the line `likwid_markerNextGroup()` caused it to work with two
regions. Switched to raw likwid instead of using my performance_monitor wrapper

#### Investigating port usage
Tried to calculate port usage. Created 3 custom groups so we could calculate
port usage. Currently exploring if UOPS_EXECUTED_CORE, UOPS_EXECUTED_THREAD, or
UOPS_ISSUED_ANY provides counts that are the same as summing all
UOPS_DISPATCHED_PORT_PORT_*

Really struggled with getting errors in likwid. I feel like I need to spend
more time reading the documentation.

Ran a test that counts each of UOPS_EXECUTED_CORE, UOPS_EXECUTED_THREAD, and
UOPS_ISSUED_ANY. On one core, these are the results I get: 

+-----------------------+---------+----------+
|         Event         | Counter |  Core 0  |
+-----------------------+---------+----------+
|   INSTR_RETIRED_ANY   |  FIXC0  | 30003500 |
| CPU_CLK_UNHALTED_CORE |  FIXC1  | 40271970 |
|  CPU_CLK_UNHALTED_REF |  FIXC2  | 33897450 |
|   UOPS_EXECUTED_CORE  |   PMC0  | 27690410 |
|  UOPS_EXECUTED_THREAD |   PMC1  | 20082810 |
|    UOPS_ISSUED_ANY    |   PMC2  | 20121590 |
+-----------------------+---------+----------+

I summed up the individual port counts by running the same program multiple
times with different ports counted each time:

PORT 0  |  4979574
PORT 1  |  5024232
PORT 2  |  861
PORT 3  |  924
PORT 4  |  801
PORT 5  |  10317
PORT 6  |  10004330
PORT 7  |  363

SUM     |  2.00E+07

UOPS_EXECUTED_THREAD (from above): 2.01E+07
UOPS_ISSUED_ANY (from above): 2.01E+07

Here, both UOPS_EXECUTED_THREAD and UOPS_ISSUED_ANY seem to match with the sum
of all ports. However, when I run the same code in parallel, things get weird:

+----------------------------+---------+-----------+
|            Event           | Counter |    Sum    |
+----------------------------+---------+-----------+
|   INSTR_RETIRED_ANY STAT   |  FIXC0  | 120495560 |
| CPU_CLK_UNHALTED_CORE STAT |  FIXC1  | 166349120 |
|  CPU_CLK_UNHALTED_REF STAT |  FIXC2  | 143447620 |
|   UOPS_EXECUTED_CORE STAT  |   PMC0  | 161796390 |
|  UOPS_EXECUTED_THREAD STAT |   PMC1  |  80585430 |
|    UOPS_ISSUED_ANY STAT    |   PMC2  |  80664620 |
+----------------------------+---------+-----------+

(the numbers reported below are the sums of counters in each core)

PORT 0  |  19960933
PORT 1  |  20443779
PORT 2  |  60018
PORT 3  |  485226
PORT 4  |  1.84E+19
PORT 5  |  578111
PORT 6  |  42519460
PORT 7  |  568032

SUM     |  1.84E+19

UOPS_EXECUTED_THREAD (from above): 8.09E+07
UOPS_ISSUED_ANY (from above): 8.10E+07

Notice that the number of operations reported on port 4 are incredibly high.
Here's the core-by-core results:

+-----------------------------+---------+----------+----------+----------+--------------+
|            Event            | Counter |  Core 0  |  Core 1  |  Core 2  |    Core 3    |
+-----------------------------+---------+----------+----------+----------+--------------+
|      INSTR_RETIRED_ANY      |  FIXC0  | 30946300 | 30944860 | 42940730 |     30895580 |
|    CPU_CLK_UNHALTED_CORE    |  FIXC1  | 60802980 | 58707880 | 55233110 |     59092460 |
|     CPU_CLK_UNHALTED_REF    |  FIXC2  | 52916970 | 51040180 | 48027820 |     51370380 |
|      UOPS_EXECUTED_CORE     |   PMC0  | 58303410 | 41440210 | 54157630 |     41846800 |
| UOPS_DISPATCHED_PORT_PORT_3 |   PMC1  |    68465 |    85618 |  2454614 |        61633 |
| UOPS_DISPATCHED_PORT_PORT_4 |   PMC2  |      903 |    21707 |  1959782 | 1.844674e+19 |
| UOPS_DISPATCHED_PORT_PORT_5 |   PMC3  |   126036 |    64438 |  3186109 |       102977 |
+-----------------------------+---------+----------+----------+----------+--------------+

For some reason, core 3 reports huge numbers for uops dispatched on port 4.
Odd. I looked at the skylake architecture again, port 4 is responsible for one
thing only: storing data. With this in mind, one core having more UOPS would
make sense, maybe the hardware only allows one core at a time to do store
operations because issuing operations is certainly not the bottleneck in memory
write performance. However, this does not explain the sheer volume of
instructions. How does 1.8e19 instructions make sense when there were only
5.9e7 cycles?

I've run this program a few times and I only seem to get this result (1.84e19)
sometimes. And sometimes I get that result on the value for PORT_7. I believe
it's a bug or an error of some kind. Seems to behave better if the number of
iterations is increased.

#### Applying port usage information to convolution
If the number of iterations where `likwid_markerNextGroup()` is called is a
multiple of the number of groups, things USUALLY behave well. If not:

Trying to just tack on port_usage groups to the existing MEM_DP|FLOPS_SP etc.
makes likwid hang indefinitely on its analysis after convolution is run.

instrumenting "entire_program" works if I comment out likwid_markerNextGroup()
OR if the number of iterations is a multiple of the number of groups.
Sometimes. This is very non-deterministic and it's frustrating. Making the
number of iterations a multiple of the number of groups SOMETIMES works, but as
the number of groups increases it seems to work less well.

### Memory
Inspected assembly. Summary of findings:
 - compared intrinsics and assignment with operator=
 - There is one read and one write instruction per iteration in both cases
 - data volumes as calculated from number of retired instructions are off by a
   factor of 2. Is this because the instructions only move *half* a cacheline
   for some reason? Is that even possible? A vector of doubles is 256 bytes, so
   half a cacheline.
 - I realized when using intrinsics, I was trying to use intrinsic for aligned
   values at unaligned addresses. What is the behavior when this happens?
 - when using operator=, volumes reported by likwid are higher and closer to
   manually calculated vallues

```
vmovapd	(%r14,%rax,8), %ymm0	# MEM[base: array_12, index: j_31, step: 8, offset: 0B], _22
vmovapd %ymm0, 0(%r13,%rax,8) # _22, MEM[base: copy_array_11, index: j_31, step: 8, offset: 0B]
```

seems to correspond with loading `array + j` into `buffer` and then storing
`buffer` into `copy_array + j`

Can this instruction load less than a cache line? Cache line size is 64 bytes
or 512 bits. In this case, the command could move only 256 bits per call
(half a cache line). However, movapd is the instruction for "move *aligned*
packed double-precision floating-point values". And it looks like I didn't
think about alignment while writing the benchmark because I try to do a load in
the middle of a cache line. So maybe those instructions just don't happen? I
don't know how the hardware handles invalid commands.

Another theory: since vmovapd only moves a vector of doubles, maybe that just
means my manually calculated data is off because it is only 32 bytes per
instruction, not 64? "Volume by retired instructions" is off by about a factor
of two, so this sounds reasonable.

I tried switching to regular assignment without intrinsics and the assembly
instead uses **vmovsd** (vector move scalar double... seems like an oxymoron)
and now includes what seems to be six per iteration. However, there are a lot
of complicated jump statements and likwid reports the same number of retired
instructions so it's probably still just two per loop. 

Strangely, memory volume as reported by likwid was higher in L3 and Memory when
using direct assignment instead of the instrinsic. These higher values were
closer to calculated volumes. Why?

#### For the final 3 iterations:
##### Manually calculated volumes: 
Name: manually calculated volume, dtype: float64
16     5.24288
17    10.48576
18    20.97152

##### Using intrinsic
Name: L3 data volume [GBytes], dtype: float64
16     3.743730
17     7.029880
18    14.544200

Name: Memory data volume [GBytes], dtype: float64
16     1.969470
17     8.604530
18    10.764700

##### Direct assignment
Name: L3 data volume [GBytes], dtype: float64
16     5.010750
17    10.298100
18    21.149400

Name: Memory data volume [GBytes], dtype: float64
16     3.650170
17     8.718550
18    21.705100

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

