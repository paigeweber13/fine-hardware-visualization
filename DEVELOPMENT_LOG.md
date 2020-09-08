# Development Log
This file tracks my past accomplishments and work as I have developed Fine
Hardware Visualization

- [Development Log](#development-log)
- [2020-08-18 through 2020-08-](#2020-08-18-through-2020-08-)
  - [Questions](#questions)
  - [Accomplishments](#accomplishments)
- [2020-08-11 through 2020-08-18](#2020-08-11-through-2020-08-18)
  - [Questions](#questions-1)
  - [Accomplishments](#accomplishments-1)
    - [Intel TMA method](#intel-tma-method)
    - [Intel vTune](#intel-vtune)
    - [General](#general)
  - [Next steps](#next-steps)
- [Biggest questions, main points](#biggest-questions-main-points)
- [2020-07-28 through 2020-08-11](#2020-07-28-through-2020-08-11)
  - [Questions](#questions-2)
  - [Accomplishments](#accomplishments-2)
- [2020-07-21 through 2020-07-28](#2020-07-21-through-2020-07-28)
  - [Questions](#questions-3)
  - [Accomplishments](#accomplishments-3)
- [2020-07-14 through 2020-07-21](#2020-07-14-through-2020-07-21)
  - [Accomplishments](#accomplishments-4)
  - [Next steps](#next-steps-1)
  - [Backlog](#backlog)
- [2020-07-07 through 2020-07-14](#2020-07-07-through-2020-07-14)
  - [Questions](#questions-4)
  - [Accomplishments](#accomplishments-5)
- [2020-06-30 through 2020-07-07](#2020-06-30-through-2020-07-07)
  - [Questions](#questions-5)
  - [Accomplishments](#accomplishments-6)
- [2020-06-16 through 2020-06-30](#2020-06-16-through-2020-06-30)
  - [Questions](#questions-6)
  - [Accomplishments](#accomplishments-7)
- [2020-06-09 through 2020-06-16](#2020-06-09-through-2020-06-16)
  - [Questions](#questions-7)
  - [Accomplishments](#accomplishments-8)
- [2020-06-02 through 2020-06-09](#2020-06-02-through-2020-06-09)
  - [This Week's Questions](#this-weeks-questions)
    - [Top priority](#top-priority)
    - [Secondary](#secondary)
    - [What new counters should we use?](#what-new-counters-should-we-use)
  - [Experiential results from comparing counters across polynomial and polynomial_block](#experiential-results-from-comparing-counters-across-polynomial-and-polynomial_block)
    - [Counters we're already using](#counters-were-already-using)
  - [Likwid stability issues](#likwid-stability-issues)
- [2020-05-17 through 2020-06-02](#2020-05-17-through-2020-06-02)
- [2020-04-30 through 2020-05-07](#2020-04-30-through-2020-05-07)
  - [Thoughts on coloring of diagram:](#thoughts-on-coloring-of-diagram)
- [2020-04-23 through 2020-04-30](#2020-04-23-through-2020-04-30)
- [2020-04-16 through 2020-04-23](#2020-04-16-through-2020-04-23)
- [2020-04-09 through 2020-04-16](#2020-04-09-through-2020-04-16)
  - [Learning likwid](#learning-likwid)
  - [Exploration](#exploration)
- [2020-03-24 through 2020-04-09](#2020-03-24-through-2020-04-09)
  - [Playing with likwid_minimal.c](#playing-with-likwid_minimalc)
  - [Improvements to performance_monitor](#improvements-to-performance_monitor)
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
    counted:](#some-notes-on-what-does-and-doesnt-get-counted)

# 2020-08-18 through 2020-08-
## Questions
- is it valuable to have a miss rate? (would be easy to implement for L2, L3.
  unclear how difficult it would be for L1)
  - speaking of L1, the more I learn about CPU (and GPU) architecture, the more
    confused I get on what exactly it is.... Seems more closely tied to core
    than other caches, especially since I and D are separate... also, likwid
    doesn't seem able to directly measure L1 bandwidth. 
- C port of NAS benchmarks adequate?
- seems like all performance monitoring tools are written in C. Will we need to
  port this tool to C eventually?

## Accomplishments
- as I was adding separate load/store to diagram, noticed benchmarks were poor:
  some of the test cases we were using performed better than the benchmark
- tinkered with benchmark to improve performance
- ended up using likwid-bench
- likwid-bench broke for bandwidth tests too large for any cache (meant to test
  RAM bw)
- looked into NAS parallel benchmarks
  - 90% are in fortran
  - [found a C port](https://github.com/benchmark-subsetting/NPB3.0-omp-C) that
    seems to be pretty good
- began changing makefile to produce shared library for easy inclusion at
  compile time (for measuring nas parallel benchmarks)

How do we measure NAS parallel benchmarks using our program?
- source code will have `#ifdef FHV_PERFMON ... #endif` sections
- how to compile?
  - in make.def, include fhv stuff and add -DFHV_PERFMON flag?
  - add `PERFMON=` variable, to be specified when make is run, that will
    automatically include things and add flag?

# 2020-08-11 through 2020-08-18
## Questions
- what is our goal to finish before end of summer?
- is Instruction Cache important to visualize? Only metrics likwid provides
  are:
  - L1I request rate = ICACHE_ACCESSES / INSTR_RETIRED_ANY
  - L1I miss rate = ICACHE_MISSES / INSTR_RETIRED_ANY
  - L1I stall rate = ICACHE_IFETCH_STALL / INSTR_RETIRED_ANY
  - L1I miss ratio = ICACHE_MISSES / ICACHE_ACCESSES
  - would be easy to create ratios for stalls as well
- Is visualizing slots in a vector an upcoming goal?
- For now at least, is average execution conditions across entire run of a
  program adequate? Eventually, we'll want to have short samples so we can see
  how execution changes, correct?
- what do you think about using machine learning to gain insights into the
  immense number of performance counters available to us?
  - I don't think our final project has a need for machine learning, but maybe
    during the research process it could give us some insights into what
    counters and metrics are important. Thoughts?
  - It can find patterns while sorting through huge amounts of data, which is
    something I've been struggling to do as I examine even just the perfgropus
    supplied by likwid (which is a small subset of the total number of
    performance counters supplied by intel)

Not related to this project
- Do you know any professors that teach web development who would be willing to
  answer some questions for me? I'm working on a side project that will involve
  building a webapp
- What OS do supercomputer clusters use? CentOS? RHEL? OpenSUSE?

## Accomplishments
- started restructuring diagram to fit overview with more detailed in-core
  performance 
- began adding new counters that will be used in the detailed overview diagram
- Thinking about config files. This is for the future, not to be implemented
  now. However, this may affect how some things surrounding config are built
  - architecture: will be hand written or generated by fhv. Not intended to be
    edited by end-user
    - hard part is it's nice to have a list of the key words for port usage
      events and the port usage metrics we use. This makes it easy to identify
      those results in per_thread_results as well as find the aggregate ratios
      fhv creates in aggregate_results. This can be created dynamically using
      port_nums, but this requires some code to run, and when to run that code
      is a complicated question. Because then we can't use
      performance_monitor_defines unless we add some kind of init function or
      something.
    - let's have architecture config also include key_metrics,
      port_usage_metrics, and the like. For now, will be hard-coded. May be
      automatically generated later.
  - diagram: configures size of things in diagram. May be edited by end-user,
    but there may not even be a need for that
- Consider
  [this](https://software.intel.com/content/www/us/en/develop/documentation/vtune-cookbook/top/tuning-recipes/poor-port-utilization.html):
  what does it mean to have "Vector capacity Usage" at 100%? if we have one
  full sized AVX vector operation, does the counter immediately jump to 100%?

### Intel TMA method
- Seems to be showing latency, not bandwidth
- multiple instructions could be waiting on the same memory operation
- perhaps more abstract than what we are going for? Perhaps we want something more low-level, more raw?
	- In TMA, if an add is stalled by memory, the memory will be highlighted. 
	- In our application, if an add is stalled, the add ports will be highlighted
- Retired instructions still contribute to usage, latency, etc. However, they're kind of ignored...

### Intel vTune
- Do MUL and FMA count the same?
- as soon as you put in one vector operation, does Vector Capacity Usage jump to 100%?

### General
- "a lot of the tools I've seen are hard to use, interface with, and understand"
- as long as what we do is easy to use, it's okay if we don't let people see everything
	- we want to show people as their code is executing like maybe at first there's a lot of I/O so that flashes, but then there's computation and so that flashes.
	- Quick overview for experienced programmers
	- intro to architecture (maybe even in CS 1) for new students
- "the closwer we are to the actualy architecture, the harder it is to see where the load is"
- someday it may be useful to bake in expectations. Have the user say "this is what I expect my application to do. If it doesn't do this, tell me"
	- For instance, "I expect my application to be flop bound and do 20 million flops" so if it does 20 billion flops, we know something is wrong

## Next steps
- Finish adjusting diagram to be a detailed overview
- Add load/store benchmarks
- Add load/store saturation
- Gather a set of simple test applications
  - NAS parallel benchmarks
- Using these tests, compare our application to intel vTune

# Biggest questions, main points
- How will we do per-core saturation measurements? How will we average them for
  the overviews?
  - How should we get a benchmark for single-thread performance in key areas?
    Should we run one thread alone? Run all threads and take the average?
    - what about Memory, where only one thread reports metrics?
  - Will we average "saturation" for multiple performance measurements (e.g.
    portion back-end bound, decoding rate, etc) for the overall saturation?
  - ANSWERS: 
    - shared resources, like caches and memory, necessitate averages. It might
      not even make sense to do per-core measurements for these resources
      - what if thread 1 only hits because thread 0 missed 100 cycles ago?
- What new counters will we use?
  - Does the Top-down Microarchitecture Analysis method make sense? Does this
    fit the 3 core areas you mentioned weeks ago?
  - What about L2CACHE, L3CACHE, CYCLE_ACTIVITY, ICACHE, etc?
  - What will we benchmark from these groups?
- [Here](https://app.lucidchart.com/documents/view/5ac956c3-ac5c-4717-ade2-6b13cec09f26/0_0)
  is a diagram presenting drafts of the visualizations we'll create. How does
  this look? What changes would you make?
- Info on TMA
  - most came from [an intel article](https://software.intel.com/content/www/us/en/develop/documentation/vtune-cookbook/top/methodologies/top-down-microarchitecture-analysis-method.html)
  - There was also a handy
    [graphic](https://i.postimg.cc/V5QKV26D/20200810-181103.png). Original
    source is
    [here](http://www.cs.technion.ac.il/~erangi/TMA_using_Linux_perf__Ahmad_Yasin.pdf)

# 2020-07-28 through 2020-08-11
## Questions
See last week. Additional questions listed here

- one of the key performance groups you mentioned is "micro instruction
  retiring", and one of the ways you explained that was "can you fetch
  instructions quickly enough". What exactly does that mean? Does that mean you
  can fetch operands quickly enough from memory? Does that mean getting new
  instructions from memory? Is this factor unique to the front-end or back-end
  of the processor?
- how does TMA group sound for identifying performance in our 3 key areas?
  - certainly seems to highlight uop decoding and retiring
- as the number of things we want to measure grows, would it make sense to use
  the likwid stethoscope mode?
  - but I know the execution loads can change throughout the execution of a
    program.... Perhaps it makes sense to do many short stethoscope
    measurements across execution?
  - this may be a good way to get a sense for how execution changes as a
    program runs
  - right now we're only showing the average state across entire execution
    anyways 
  - this could make a pretty animation
- stethoscope is not really meant for fine-tuned execution. We could hack it to
  do this though? I don't know how we'd trigger it from code
  - what other technologies might allow us to do this?
  - require people to create fine-tuned groups? this is unwieldy

## Accomplishments
- Finished restructuring performance_monitor
- Added numbers to legend
- revisited examples and made them all work again
- some minor work on software engineering in makefiles
- added port usage to saturation diagram
- Identified some counters that we can use to measure 3 key performance areas:
  - TMA!!!
  - CYCLE_ACTIVITY/CYCLE_STALLS: stalls due to L1D misses, L2 misses, and L3
    misses (memory loads) - can help diagnose back-end bound applications
  - L2CACHE, L3CACHE for diagnosing memory issues
  - ICACHE for instruction fetching?
  - UOPS_EXEC/ISSUE/RETIRE might be good if we can understand them...
  - BRANCH for front-end bound?
- feel like a good procedure is: 
  1. use TMA to determine front-end/back-end bound
  1. if back-end bound, look at memory (for memory-bound cases) and port-usage
     (for core-bound cases)
     - use counters L2CACHE, L3CACHE, CYCLE_ACTIVITY, etc
  1. if front-end bound, look at code size and branching
  1. if bad-speculation, not really sure what to do. Intel docs recommend
     "compiler techniques such as profile-guided optimization" 
  1. if retiring, code is probably good. However, consider if the code is
     retiring non-useful operations. At this point, looking at flops and
     bandwidth may be appropriate

# 2020-07-21 through 2020-07-28
## Questions
Follow up on polynomial expansion
- have you made any progress on investigating polynomial_expansion? Why is
  port4 the most saturated? Why when switching to clang++, port 0 and 1 are the
  most saturated? 

These questions all focus on how we should report per-thread saturation
- we're going to do per-thread saturation. I also felt it makes sense to have
  an "overview" that shows an average. Is this useful?
- Several questions on making things per-thread
  - for computation (flop rates, instruction decoding, etc) it's pretty easy to
    create an estimate for the maximum possible rate. However, it gets more
    difficult with memory
  - only one core can control the memory controller... I guess there's no way
    to split up expected bandwidth between four cores, then. Is that right? 
    - should we use a counter like "mem_instr_retired" to estimate pressure on
      the memory? That could be done per-core. But then how will we know if
      something will miss the TLB?
  - For L1 and L2 cache, bandwidth varies widely between two threads on the
    same core. If we use both threads, one will have 1.5-2.0x the bandwidth of
    the other.
  - L3 cahce, which is shared by all cores, has similar patterns of variation
    but even greater magnitudes. One thread will report 16 GB/s of bandwidth
    while the other only reports 2 GB/s
- does it make sense to just run a single-threaded benchmark and use that as
  the reference, even though we expect that performance to be much higher than
  all threads are capable of simultaneously?

Miscellaneous questions:
- is creating separate svgs for overview/detailed view a good course of action
  for now? 
  - from what I understand, the final goal/vision is to have an interactive
    diagram the user can pan and zoom, right?
  - alternative is to create very small drawings and expect people to zoom into
    the svgs....
- for likwid documentation: does it make sense for me to write documentation
  even though I'm frequently wrong about expected usage? (Thomas and I talked
  for months before I finally learned that likwid_markerNextGroup() must be
  called in a serial region)

Low-priority
- should `#define` or `const` be used? My instinct is `const` because then you
  get type-checking. 
- all the performance monitoring tools I've seen are written in C, presumably
  because they are used by C programmers as well as C++ programmers. I seem to
  remember that we decided to use C++ at the beginning. why not C? Do you think
  we'll have to port this to C at some point in the future?

*END OF QUESTIONS THAT ARE STILL UNANSWERED*

-------------------------------------------------------

## Accomplishments
- completely revamped how we process and store results. Things are now much
  easier to work with and much less prone to bugs
- did some research and formed some questions on how we will present per-core
  saturation 

# 2020-07-14 through 2020-07-21
## Accomplishments
- [pull request](https://github.com/RRZE-HPC/likwid/pull/303) for likwid is
  merged!
- dramatically improved draw_diagram function
  - moved to own file so it's not bloating the driver file
  - created reusable functions to keep code DRY
  - switched to pango, a much more powerful text engine that has first-class
    support in cairo
  - made it easy to configure diagram (and easy to move configuration to a
    separate .ini file or similar)
  - began work on a "detail view" that will show us in-core performance with
    finer granularity: things like port saturation

## Next steps
In general, I want to pivot away from maintenance and start adding features. We
can return to code maintenance later.

- implement port usage in "detail view"
- look at counters that will help us identify instruction decoding and
  micro-instruction retiring (key areas identified in the past)
- have the program automatically adapt to new configurations (for now, this
  means different numbers of cores, but same architecture)

## Backlog
- clean up and simplify makefile (goal is to make it easy to maintain,
  uncomplicated)
- make program adapt to other architectures
- improve likwid documentation
  - tom hasn't made it clear how I can help or if my help is even wanted
  - he may create a "todo" list for documentation that I can use
- give examples their own makefiles
- verify tests/examples. Many of them don't work anymore. Decide what should be
  updated and what should be removed

# 2020-07-07 through 2020-07-14
## Questions
Q: at what point should I make a test suite?

A: There are two purposes:
   - bug track
   - demonstrate how software is intended to be used

   There's a tradeoff. you can find bugs faster with a test suite, but then when
   code isn't stable you end up breaking your tests all the time

   Now is probably not a good time

Q: how should I organize the file tree for this repo? I had main files in a
   separate src directory so that more than one main file won't get grabbed by
   a wildcard in the makefile

A: separating library and driver file directories as mentioned above makes
   sense 

Q: do wildcards in the makefile even make sense? Each file has such different
   dependencies. Does compilation go faster if I only add the -I, -L, and -l
   flags for the files that need those headers and libraries?
   
A: you can optimize a little bit... but it's not a big amount, and the
   carefully-crafted makefile will require a lot of time to maintain

Q: for example code, I thought it'd be good to use the existing makefile in the
   root directory, since the compile commands for each example are the same.
   does this make sense?

A: yes, but it can get messy if each example doesn't have the same dependencies
   (which is nearly always). Instead, consider including the install prefix and
   other common choices from another makefile but having separate makefiles for
   each example.
  
Q: how do I keep the makefile DRY and not rewrite the command for each case 
   in a single example? 

A: one option is to test the name of the target, e.g. pick a different command
    based on the suffix of the target 
    
    ifeq, ifneq, subst are useful makefile commands to look into

## Accomplishments
A big part of this week was doing work to refactor fhv to make it more readable
and maintainable. This work is found in the branch "make-diagram-own-file"
- switched from cairo's "Toy" text api to pango
- moved all drawing functions to a different file and class,
  "saturation_diagram". 
- started moving things to their own functions

I also spent some time working on my pull request to improve likwid's examples.
This was in response to some changes Thomas wanted me to make.
- removed one region registration to demonstrate that it's optional
- removed some unnecessary barriers and improved comments
- made a separate parallel block to demonstrate GET/RESET, which further
  cleaned up the code
- in the example designed to be independent of the likwid-perfctr CLI tool, the
  program now errors and exits if used with CLI tool.

# 2020-06-30 through 2020-07-07
## Questions
- lower priority: questions on cache from last week

## Accomplishments
- double checked new method for calculating port saturation
- put in two PRs for likwid repo!
  - corrected likwid documentation on marker api
  - updated likwid-markerAPI example. Mainly just fixed position of
    LIKWID_MARKER_SWITCH and improved comments
  - rewrote likwid-internalMarkerAPI from scratch
    - more descriptive comments
    - advanced usage 
    - nested groups
    - GET, RESET
    - completely independent of likwid-perfctr (sets environment variables and
    analyzes results)
- began adding to likwid doxygen documentation
- improve software engineering in fhv.cpp
  - moved cairo stuff to own file

# 2020-06-16 through 2020-06-30
## Questions
- cache line size is 64 bytes. But on a skylake system we can only use 32
  bytes. Also, vectors are 32 bytes. What if we used the smaller vectors, the
  16 byte vectors? The core would still load 32 bytes because it can't load
  part of a cache line, so it would just discard the other 16 bytes?
  - wikichip says the bandwidth (for L1/L2 loads) is 64B/cycle. Wikichip
    also says L1 and L2 caches are shared between threads on a single core. I
    assume this implies that the 64B/cycle bandwidth is also shared and that
    this accounts for the 32B cache line we really get per thread in L1. Is
    that right?
  - could you make it so that each thread gets 16 bytes so that you use
    everything loaded?
- for UOPS, what's the difference between "executed", "issued", and
  "dispatched"? The intel documentation uses all 3 and they seem to have
  nuances that separate them, but they are not clearly defined. Intel uses the
  word in its own definition. 
  - for instance, the description of "UOPS_EXECUTED_PORT.PORT_0" reads "Counts
    the number of cycles in which a uop is dispatched to port 0."
- why do I get such different saturations/port usages between compilers with
  the same flags?
- at what point should I make a test suite?

## Accomplishments
- explored likwid-api. If we do decide to move away from the marker API, I
  think this is what we should choose. 
  - see `tests/likwid_api_minimal.c`
  - single-threaded applications with multiple groups do not have the
    unreasonably-high values problem
  - this is not a definitive result: multi-threaded code must still be tested
  - allows us to take advantage of perfgroups (at least the events, I have not
    got metrics to work yet. I believe the problem comes from not having a
    correct runtime)
- workaround discovered for unreasonably large values bug: add `#pragma omp
  barrier` before call to `likwid_markerNextGroup`
- fhv performance_monitor now ignores large values and warns the user about
  them 
- added barriers to fhv performance_monitor calls (seems to improve stability
  of using fhv wrapper)
  - using fhv start/stopRegion and nextGroup: went 410 iterations without
    having any problems. Final iteration showed unreasonably high values.
  - also using fhv init: went 798 iterations before having a problem.
- "stopping non-started region" seems to happen when one thread stops before
  another has started. I suppose regions are not completely thread-independent,
  but the likwid documentation doesn't mention anything about this...
- explored high saturation on port 4 even for CPU-heavy parameters of
  polynomial_block:
  - kept n at 1024 and increased degree from 1e3 to 1e8. In all cases, port4
    was the most saturated. That seems odd; now I wonder if I'm measuring port
    usage correctly...
  - started inspecting assembly. Also used llvm-mca to get some insights.
    Noticed that I was getting different results from llvm-mca if I used clang,
    so I tried to compile polynomial-block with clang. Results follow
    - command used (in both cases, $(CXX) is replaced with g++ or clang++):
      `$(CXX) polynomial_block.cpp -std=c++14 -O3 -fopenmp -march=native
      -mtune=native -I /usr/local/likwid-master/include -I
      /home/riley/code/fine-hardware-visualization/lib -fopenmp
      /home/riley/code/fine-hardware-visualization/obj/performance_monitor.o -L
      /usr/local/likwid-master/lib -llikwid -DFHV_PERFMON -o
      polynomial_block_fhv_perfmon`
    - key parts are the flags `-std=c++14 -O3 -fopenmp -march=native
      -mtune=native -llikwid -DFHV_PERFMON`
    - g++ version: g++ (Ubuntu 7.4.0-1ubuntu1~18.04.1) 7.4.0
    - clang++ version: clang version
      10.0.1-++20200519095410+f79cd71e145-1~exp1~20200519200813.165 Target:
      x86_64-pc-linux-gnu Thread model: posix InstalledDir: /usr/bin
    - Results: (updated with correct port saturation)
      | Compiler | SP Flops | Top Port Saturation | Second Port Saturation |
      | -------- | -------- | ------------------- | ---------------------- |
      | g++      | 5.654e+04| Port 4 @ 0.236      | Port 3 @ 0.180         |
      | clang++  | 7.642e+04| Port 0 @ 0.182      | Port 1 @ 0.180         |
    - in the case of clang++, ports 2/3 were also fairly saturated at 0.169
    - on skylake, we have to read for writeback, so it makes sense that port 2
      and 3 (the ports for loads) are also fairly saturated. In the case of the
      g++ code, both port 2 and 3 were about 0.180
    - this is much more what we expect in the cpu-heavy case. Interesting how
      much the compiler makes a difference even with the same flags
- looked at how we calculate port usage. Summing all port usages should be 1,
  because it's supposed to be usage of 1/total usage. But that isn't the case.
  The sum of them all was around 0.43 or so for both the g++ and clang++
  versions. Why is that?
  - I think it's because I was using UOPS_EXECUTED_CORE, which counts from both
    threads. Evaluating replacements, but UOPS_EXECUTED_THREAD looks promising
  - counts were pretty consistent across runs, here is one example:
  - Thread 2
    sum of UOPS_DISPATCHED_PORT_PORT_*:     2.70143e+08
    UOPS_EXECUTED_CORE:                     3.99846e+08
    UOPS_EXECUTED_THREAD:                   2.01832e+08
    UOPS_ISSUED_ANY:                        1.53631e+08
  - none are spot on, UOPS_EXECUTED_THREAD is certainly better
- discovered that in many places the documentation is simply wrong. [The
  documentation for the marker
  API](https://rrze-hpc.github.io/likwid/Doxygen/group__MarkerAPI.html#gaeaba86bf606c2f0044ece3600c5657a9)
  says that `likwid_markerNextGroup` should be called in a parallel region.
  Thomas, the lead dev [mentioned in an
  issue](https://github.com/RRZE-HPC/likwid/issues/292#issuecomment-646493838)
  that `likwid_markerGroup` should be called in a sequential region.
- fixed how we measure port usage: made fhv sum all the uops executed by port
  and then divide each by that sum
- looked at polynomial_block with Dr. Saule and discovered a few things.
  - even with clang, code is producing a lot of move instructions in assembly
  - unroll 64 was too high; need 8 YMM registers for out, 8 YMM registers for
    xtothepowerof, 8 YMM registers for x. In theory x could spill over to stack
    since we're just reading from it after the beginning, but this may not be
    ideal.
    - in any case, since there weren't enough registers it was spilling to the
      stack. Since the stack is on L1 it was still quite fast
  - changing to unroll 32 still had lots of moves. Changing to unroll 16 had
    fewer moves but no longer used fma. This is odd. 

# 2020-06-09 through 2020-06-16
We've decided to continue using likwid at this point, but PAPI may prove useful
in the future.

## Questions
 - likwid marker API (already using) vs PAPI vs likwid perfmon API
   (lower-level)
   - IMO likwid marker API enables us to focus more on this research instead of
     redoing the works of others, IF we can get the stability issues ironed out
   - The nice thing about PAPI or the likwid perfmon api is that it gives us
     more control. If something goes wrong, we are more likely to be able to
     fix it. This comes at the cost of more work though.
 - keep test results in github or somewhere else?
 - planned next steps: 
   - decide remain with likwid marker API or switch
   - explore port usage and other counters for polynomial_expansion and
     convolution 

## Accomplishments
 - worked on fixing likwid; spent some time in the codebase with the goal of 
   deciding how long it would take to fix everything
   - Likwid's own examples don't work
   - documentation is somewhat limited. There are doxygen-generated docs but
     they aren't always helpful
   - I don't think likwid has some inherent advantage over other tools like 
     PAPI or perf_events. I may be able to fix this, but is it a better use of
     time to just use a different tool?
     - LIKWID does do a lot of the work that I'd have to do otherwise with
       PAPI, like timing code and calculating rates like FLOP/s
   - I'm no longer digging through codebase to resolve this issue, but I will
     provide Thomas with any information I can.
   - while looking for alternatives to likwid, discovered a tool called 
     [extrae](https://tools.bsc.es/extrae), which uses PAPI. I think PAPI
     requires some serious investigation
   - tried to run likwid_minimal test on chameleon cloud. The unreasonably high
     values problem became much, much more common. I believe this is because
     since there are many threads, it is more likely that any given thread will
     experience this problem
 - explored PAPI
   - I think this is something we should seriously consider
   - looked at duplicating existing saturation values and port_usage values
   - High level API was giving me trouble... Specifying multiple events (even
     events I was sure my arch supports) did not work at all. It would just do
     the first event on the list.
     - turns out HL API fails silently if a counter is not available...
   - Low level api makes errors more clear
   - still not able to meter both computation and cache/memory things at the
     same time. Will investigate
     - seems that PAPI binds to a "component" on PAPI_add_event or
       PAPI_assign_eventset_component. See
       https://bitbucket.org/icl/papi/wiki/PAPI-Multiplexing.md 
     - solution is to just create two event sets. With multiple eventSets, is
       it even necessary to use multiplexing?
   - I complained a bit about LIKWID's lack of documentation... honestly PAPI
     suffers from the same problem. Had to figure out about eventSets being
     bound to components on my own.
   - next questions:
     - how do I use PAPI on multithreaded sections?
     - how do I use native event sets?
     - how difficult is it to switch event sets in like a for loop or
       something?
       - furthermore, how would this be used if we were to replace likwid
         in fhv?
   - majority of events would be native events, which are difficult to find the
     codes for. There is probably a map function that we can repeatedly use but
     I haven't figured it out yet
   - another benefit of PAPI is that it supports multiplexing, which would
     allow for multiple performance groups to be measured during one run of the
     key code
 - explored Chameleon Cloud
   - the [resource browser](https://www.chameleoncloud.org/hardware/) allows
     you to see the information on the hardware you can allocate. 
   - Skylake nodes:
     - use Intel(R) Xeon(R) Gold 6126 CPU @ 2.60GHz cpus
     - almost always have 48 threads. One node in UC only has 24.
     - lists ram size as 205084688384. Assuming that is bytes, we're looking at
       205 GB of ram in a node
   - having trouble with likwid...
     - when running likwid_minimal, get the following error:
       `Warning: Counter PMC3 cannot be used if Restricted Transactional Memory feature is enabled and
         bit 0 of register TSX_FORCE_ABORT is 0. As workaround write 0x1 to TSX_FORCE_ABORT:
         sudo wrmsr 0x10f 0x1`
     - running the command given (`sudo wrmsr 0x10f 0x1`) does not help
     - FIXED! Had to add the `-a` flag to the command. This flag runs the
       operation on all processors
     - however, still get the same number of DP scalar ops as on my computer,
       so results seem to be coming through just fine?
  
# 2020-06-02 through 2020-06-09
 - wrote tests to experiment on polynomial_expansion with multiple perfgroups
 - inspected results (see subheading below)
 - worked on fixing likwid stability issues
   - also discovered workaround: don't use hyperthreading. This doesn't
     completely eliminate non-deterministic behavior but it does greatly
     alleviate it
 - added optional, customizable parameter string to JSON output

## This Week's Questions
### Top priority
 - should we demonstrate change in behavior across many parameters? If so, how?
   by an animation? Slider?
   - should we incorporate this work with Yonghong's work in visualization?
   -  this can happen later
 - what new counters should we incorporate?
   - counters that help us determine:
     - [ ] port usage: are we overloading one port and preventing others from 
           being used?
     - [ ] instruction decoding: can you decode instructions quickly enough?
     - [ ] micro-instruction retiring: can you fetch instructions quickly
           enough? 

### Secondary
 - should I visualize saturation on a per-core basis?
   - Dr. Saule: one of the use-cases might be to see load imbalance, so let's
     visualize per-core
 - should I visualize double precision, single precision, or both?
   - currently picking the larger value (more saturated) and using that one
   - this visualization will eventually be somewhat hierarchical. You won't see
     all the detail at the high-level, but you can select different sections to
     "zoom in" and get a higher level of detail
   - complex applications may do single-precision for half of the execution and
     double-precision for the other half. That is full saturation overall.
   - core may be saturated for many reasons, we want to incorporate all them
     - what if you can't decode instructions fast enough?
   - "single precision flops" may be a bad metric. What if you can't support
     fma? Then you immediately lose a huge portion of performance.
 - is there some way we can include execution parameters in the visualization?
   It's hard to keep track of how we generated the visualization
   - perhaps include a line that says "Command used to generate this
     visualization: <command>"

### What new counters should we use?
Look for counters that demonstrate the three main things: port usage, 
instruction decoding, and micro-instruction retiring. The stuff below was 
considered and only remains for the purpose of logging past work

 - BRANCH: might identify when code is not doing useful computation? Is it 
   fair to call branching "not useful"?
 - CYCLE_STALLS: memory load reduced on optimized code for CPU-heavy params.
   Load shifted from memory to cache on mem-heavy params. 
   - Interpretation of this data kinda needs knowledge of whether you're
     loading CPU or memory
   - For instance: 45% memory stalls is pretty good for memory-heavy
     parameters, but bad if we're expecting CPU to be loaded
 - FALSE_SHARE: may represent locality of data access?
 - "Vectorization Ratio": a metric in the FLOPS_SP and FLOPS_DP groups that
   we are currently not using, but I think is valuable information.

## Experiential results from comparing counters across polynomial and polynomial_block
 - BRANCH perfgroup:
   - Branch rate: portion of instructions which branch.
     - this is lower on polynomial_block as compared to polynomial. Is this
       because of vectorization? 
   - Branch misprediction rate: portion of instructions which branched to
     correct misprediction
   - Branch misprediction ratio: portion of branch instructions which branched
     to correct misprediction
   - Instructions per branch: total instructions / branch instructions
 - BRANCH, cpu-heavy params: 
   - polynomial_block had about 1/10th the total number of instructions
   - ratio of branch instructions to total instructions was also about
     1/10th that of un-optimized code
 - BRANCH, mem-heavy params: 
   - polynomial_block again had about 1/10th the total number of instructions
   - branch rate of optimized code was 1/3rd that of basic code
   - both basic and optimized code had VERY low misprediction ratios, but basic
     code was about 1/3rd that of optimized code
 - CLOCK wasn't too useful. Mostly power information.
 - CYCLE_ACTIVITY: CYCLE_ACTIVITY_CYCLES_*_PENDING / CPU_CLK_UNHALTED_CORE
 - CYCLE_ACTIVITY, cpu-heavy:
   - cycles without execution was 9.8% for basic code, 6.8% for block-optimized
     code
   - Cycles without execution due to L1D was HIGHER for optimized code. Basic
     code was 1.22%, optimized was 2.71%
   - Cycles without execution due to L2 was HIGHER for optimized code. Basic
     code was 1.05%, optimized was 3.41%
   - Cycles without execution due to memory loads was very high in both cases.
     Basic code was 94.69%, optimized code was 96.51%
 - CYCLE_ACTIVITY, mem-heavy:
   - cycles without execution was 15.5% for basic code, 26.7% for block-optimized
     code
   - Cycles without execution due to L1D was HIGHER for optimized code. Basic
     code was 8.8%, optimized was 20.6%
   - Cycles without execution due to L2 was HIGHER for optimized code. Basic
     code was 8.3%, optimized was 30.2%
   - Cycles without execution due to memory loads was very high in both cases.
     Basic code was 99.74%, optimized code was 86.52%
 - CYCLE_ACTIVITY interpretation:
   - Memory load was obviously shifted to caches in the optimized code... 
     Also, it's natural that there wasn't nearly as much execution in the
     memory-heavy case
 - CYCLE_STALLS seems very similar to CYCLE_ACTIVITY, still trying to figure
   out the differences...
   - CYCLE_ACTIVITY_STALLS_*_PENDING / CPU_CLK_UNHALTED_CORE
   - haven't yet found a part of the intel documentation that makes clear the 
     differences between CYCLE_ACTIVITY_STALLS_*_PENDING and 
     CYCLE_ACTIVITY_CYCLES_*_PENDING 
   - descriptions in perfgroup files basically say the same thing too
   - "stalls caused by..." are the number of stalls from a given source 
     divided by the total number of stalls
   - "rates" in this group are the number of stalls from a given source 
     divided by the unhalted clock
 - CYCLE_STALLS, cpu-heavy:
   - stall rate (total stalls / unhalted clock) is lower for optimized: 
     basic code had 9.8% and optimized code had 3.5%
   - portion of stalls caused by L1D and L2 was pretty similar
   - portion of stalls caused by memory had a big difference. 37.04% for
     basic, 12.61% for optimized
 - CYCLE_STALLS, mem-heavy:
   - stall rate is lower for BASIC (!): 16.42% for  basic, 40.46% for opt
   - portion of stalls caused by L1D and L2 was pretty similar, though L2 was
     slightly higher for opt code. Basic was 16.64% and opt was 21.15%
   - portion of stalls caused by memory had a significant difference. 75.51%
     for basic, 45.7% for optimized
 - CYCLE_STALLS interpretation:
   - clearer difference between basic/opt versions of cpu code
   - again makes clear the shift from mem to cache with optimized code
 - DATA:
   - really only measures load-to-store ratio. 
 - DATA, cpu-heavy:
   - MUCH lower on optimized code. Basic code had a ratio of 19.09 and
     optimized code had a ratio of 1.07. Probably just a manifestation of how
     there are lower memory operations overall.
   - inspecting MEM_INST_RETIRED_ALL_* showed that this was not exactly the
     case... optimized code had about 1/4 the loads and 4x the stores as basic
     code
 - DATA, mem-heavy:
   - basic code had a ratio of 3.14, Optimized code had a ratio of 1.29
   - basic code had about 1.0e12 loads, 3.3e11 stores
   - opt code had about 8.3e10 loads, 6.4e10 stores
 - DIVIDE
   - DIVIDE, cpu-heavy:
     - opt code had about 1/10th the number of divide ops
     - ratio of ARITH_DIVIDER_ACTIVE to ARITH_DIVIDER_COUNT was about the same
   - DIVIDE, mem-heavy:
     - basic code had 2e7 divides, opt had 5e5. That's about 1/40th the number
       of divides
     - ratio was again about the same
 - ENERGY
   - ENERGY, cpu-heavy
     - power somewhat higher in all areas
   - ENERGY, mem-heavy
     - power somewhat higher in all areas, significantly higher (7x higher) 
       for RAM
 - TODO: expand sections after this one
 - FALSE_SHARE 
   - conceptually, false sharing is when multiple threads share a cacheline
     containing a value that will not be changed, but something else in the
     cacheline gets changed, forcing the value to be stored to memory and then
     re-loaded by all threads, even if they only use the read-only part of the
     cacheline. 
   - uses two counters: MEM_INST_RETIRED_ALL (all memory instructions) and 
     MEM_LOAD_L3_HIT_RETIRED.XSNP_HITM. Description in intel docs says: 
     "Retired load instructions which data sources were HitM responses from 
     shared L3."
   - FALSE_SHARE, cpu-heavy
     - false sharing is much higher for basic code at 166 GB vs 2 GB in the
       optimized case
     - false sharing rate (falsely shared ops / total mem ops) is also higher
       for basic code at 0.0157 (1.5%), compared with 0.0005 (0.05%) for opt 
       code
   - FALSE_SHARE, mem-heavy
     - again, false sharing volumes are higher for basic code (4.08 TB??) vs 
       34 GB for opt code
     - notice that in general false sharing volumes are higher
     - false sharing rates are higher for basic code as well: 0.195 vs 0.0147
       for opt
 - FLOPS_AVX
   - not super useful. Just gives a subset of information available in 
     FLOPS_SP and FLOPS_DP
 - FLOPS_SP/FLOPS_DP vectorization ratio
   - percentage of flop operations that were vector operations
   - went from numbers in the order of 1e-10 (in basic) to 100 (in opt)
 - ICACHE
   - stats like L1I request rate, L1I miss rate, and L1I stall rate
   - avg values used
   - ICACHE, cpu-heavy
     - request rate: 0.4209 in basic, 0.1442 in opt
     - miss rate:    0.0001 in basic, 0.0001 in opt
     - stall rate:   0.0005 in basic, 0.0004 in opt
   - ICACHE, mem-heavy
     - request rate: 0.1825 in basic, 0.1396 in opt
     - miss rate:    0.0004 in basic, 0.0003 in opt
     - stall rate:   0.0021 in basic, 0.0017 in opt
 - L2CACHE
   - whereas the L2 performance group provides data like bandwidth and data 
     volume, L2CACHE measures request rates and miss rates. Here, we will 
     focus on request rates (L2 requests / total num instructions) and miss
     ratios (L2 misses / L2 requests). We will consider average rates across
     threads.
   - L2CACHE, cpu-heavy
     - |               | basic  | opt    |  
       |---------------|--------|--------|  
       | request rate: | 0.0221 | 0.0158 |
       | miss ratio:   | 0.3516 | 0.2799 |
   - L2CACHE, mem-heavy
     - |               | basic  | opt    |  
       |---------------|--------|--------|  
       | request rate: | 0.2058 | 0.2890 |
       | miss ratio:   | 0.3538 | 0.2844 |
 - L3CACHE
   - L3CACHE, cpu-heavy
     - |               | basic  | opt    |  
       |---------------|--------|--------|  
       | request rate: | 0.0000 | 0.0001 |
       | miss ratio:   | 0.4099 | 0.8734 |
   - L3CACHE, mem-heavy
     - |               | basic  | opt    |  
       |---------------|--------|--------|  
       | request rate: | 0.0000 | 0.0006 |
       | miss ratio:   | 0.4930 | 0.8640 |
 - PORT_USAGE*
   - we record these already, but we don't use them in the visualization
 - RECOVERY
   - RECOVERY, cpu-heavy
     - avg recovery duration: 6.0592 in basic, 5.9054 in opt
   - RECOVERY, mem-heavy
     - avg recovery duration: 6.3531 in basic, 6.0475 in opt
 - TLB_DATA
   - TLB_DATA, cpu-heavy
     - |                          | basic  | opt    |  
       |--------------------------|--------|--------|  
       | load miss rate:          | 0.0000 | 0.0001 |
       | load miss avg duration:  | 37.912 | 31.365 |
       | store miss rate:         | 0.0000 | 0.0000 |
       | store miss avg duration: | 15.253 | 23.535 |
   - TLB_DATA, mem-heavy
     - |                          | basic  | opt    |  
       |--------------------------|--------|--------|  
       | load miss rate:          | 0.0001 | 0.0011 |
       | load miss avg duration:  | 31.853 | 29.965 |
       | store miss rate:         | 0.0000 | 0.0000 |
       | store miss avg duration: | 7.0166 | 33.291 |
 - TLB_INSTR
   - TLB_INSTR, cpu-heavy
     - |                     | basic  | opt    |  
       |---------------------|--------|--------|  
       | miss rate:          | 0.0000 | 0.0000 |
       | miss avg duration:  | 55.168 | 73.526 |
   - TLB_INSTR, mem-heavy
     - |                     | basic  | opt    |  
       |---------------------|--------|--------|  
       | miss rate:          | 0.0000 | 0.0000 |
       | miss avg duration:  | 51.307 | 67.659 |
 - TMA
   - intel article about this methodology: https://software.intel.com/content/www/us/en/develop/documentation/vtune-cookbook/top/methodologies/top-down-microarchitecture-analysis-method.html
   - one counter is "UOPS_RETIRED_RETIRE_SLOTS". Intel documentation describes
     this counter by saying "Counts the retirement slots used". The above link
     describes a slot as a theoretical place for a uop. Each cycle has four
     slots
   - IDQ_UOPS_NOT_DELIVERED_CORE: Intel docs describe this one as "Counts the 
     number of uops not delivered to by the Instruction Decode Queue (IDQ) to 
     the back-end of the pipeline when there were no back-end stalls. This 
     event counts for one SMT thread in a given cycle."
   - INT_MISC_RECOVERY_CYCLES: Intel docs describe this one as "Core cycles 
     the allocator was stalled due to recovery from earlier machine clear event
     for this thread (for example, misprediction or memory order conflict).
   - these are used to create metrics for retired, front-end bound, back-end 
     bound, and badly-speculated instructions. We will evaluate those here.
     - front-end bound is calculated by IDQ_UOPS_NOT_DELIVERED_CORE/total_slots
       * 100. In other words, fetch bubbles/total slots * 100
     - badly-speculated percentage is calculated by finding uops not retired
       and dividing my number of slots. Finding uops not retired is somewhat
       complicated. Likwid does this by finding 
       (UOPS_ISSUED_ANY-UOPS_RETIRED_RETIRE_SLOTS+(4*INT_MISC_RECOVERY_CYCLES))
       so they include INT_MISC_RECOVERY_CYCLES as part of the ops completed
     - percent retired is simply UOPS_RETIRED_RETIRE_SLOTS/total slots * 100
     - back-end bound is complicated and I'm not sure I understand it. So I
       will just paste the formula here
       Back End [%] = (
         1 - (
           ( 
             IDQ_UOPS_NOT_DELIVERED_CORE + UOPS_ISSUED_ANY
             + (4*INT_MISC_RECOVERY_CYCLES)
           )
           / (4 * CPU_CLK_UNHALTED_CORE)
         )
       ) * 100
   - All measurements are given in percentages and are averaged across threads
   - TMA, cpu-heavy
     - |                 | basic  | opt    |  
       |-----------------|--------|--------|  
       | front-end bound | 10.591 | 11.323 |
       | bad speculation | 01.029 | 00.181 |
       | retired         | 22.159 | 31.584 |
       | back-end bound  | 66.220 | 56.912 |
   - TMA, mem-heavy
     - |                 | basic  | opt    |  
       |-----------------|--------|--------|  
       | front-end bound | 08.084 | 04.665 |
       | bad speculation | 04.753 | 00.455 |
       | retired         | 05.484 | 20.653 |
       | back-end bound  | 81.678 | 74.228 |
   - Front-end stalls are due to lack of frontend bandwidth or high front-end
     latency.
   - bad speculation is caused by branch mispredicts or machine clears
   - back-end stalls are the hardest to diagnose. They can be memory-bound or
     core-bound.
     - core-bound: Are one or two ports saturated? (esp. ports that don't do 
       vector operations). Are we doing scalar ops where vector ops are 
       possible? Are several consecutive divide operations competing for use
       of the divide units?
     - memory bound: some level of the memory subsystem is being over-stressed.
       Is the application trying to make many stores to memory? Does the data
       structure have poor locality, preventing good caching? Do loads depend
       on prior stores?
 - UOPS
   - just measures raw numbers of UOPS. less useful, probably.
   - Noticed that for the cpu case, optimized code has between 1/10th and 1/5th
     the uops issued, executed, and retired as the basic code.
   - for the mem-heavy case, the optimized code had about 1/20th the uops 
     issued, 1/10 the uops executed, and 1/10th the uops retired
   - uops retired was consistently higher than uops executed.... I'm unclear 
     why, because the intel documentation made it sound like retired meant
     executed, especially because uops from miss-predictions are NOT counted
     as "retired"
     - source: link at beginning of TMA section says the following: "The 
       completion of a uOp’s execution is called retirement"
 - UOPS_EXEC
   - measures portion of cycles were used by uops and portion that were 
     stalled. Also measures average stall duration.
   - measures "the execution stage in the pipeline" according to likwid
   - UOPS_EXEC, cpu-bound:
     - |                             | basic  | opt    |  
       |-----------------------------|--------|--------|  
       | used cycles ratio [%]       | 59.513 | 86.070 |
       | stalled cycles ratio [%]    | 40.487 | 13.930 |
       | avg stall duration [cycles] | 1.18e11| 3.56e9 |
   - UOPS_EXEC, mem-bound:
     - |                             | basic  | opt    |  
       |-----------------------------|--------|--------|  
       | used cycles ratio [%]       | 14.577 | 46.261 |
       | stalled cycles ratio [%]    | 85.424 | 53.739 |
       | avg stall duration [cycles] | 1.07e12| 2.09e10|
   - my processor runs at 2.40GHz, 2.40e9.... meaning on average a stall is
     more than 1 second??? That seems unlikely...
   - likwid calculates average stall duration with the following formula:
     Avg stall duration [cycles] = UOPS_EXECUTED_STALL_CYCLES 
     / UOPS_EXECUTED_STALL_CYCLES:EDGEDETECT
 - UOPS_ISSUE
   - measures "the issue stage in the pipeline" according to likwid
   - UOPS_ISSUE, cpu-bound:
     - |                             | basic  | opt    |  
       |-----------------------------|--------|--------|  
       | used cycles ratio [%]       | 24.226 | 31.994 |
       | stalled cycles ratio [%]    | 75.774 | 68.002 |
       | avg stall duration [cycles] | 2.15e11| 1.74e10|
   - UOPS_ISSUE, mem-bound:
     - |                             | basic  | opt    |  
       |-----------------------------|--------|--------|  
       | used cycles ratio [%]       | 06.524 | 22.595 |
       | stalled cycles ratio [%]    | 93.477 | 77.405 |
       | avg stall duration [cycles] | 1.18e12| 2.94e10|
 - UOPS_RETIRE
   - measures "the retirement stage in the pipeline (re-order buffer)" 
     according to likwid
   - UOPS_RETIRE, cpu-bound:
     - |                             | basic  | opt    |  
       |-----------------------------|--------|--------|  
       | used cycles ratio [%]       | 33.636 | 50.088 |
       | stalled cycles ratio [%]    | 66.364 | 49.912 |
       | avg stall duration [cycles] | 1.96e11| 1.27e10|
   - UOPS_RETIRE, mem-bound:
     - |                             | basic  | opt    |  
       |-----------------------------|--------|--------|  
       | used cycles ratio [%]       | 07.639 | 27.381 |
       | stalled cycles ratio [%]    | 92.361 | 72.620 |
       | avg stall duration [cycles] | 1.18e12| 2.81e10|

### Counters we're already using
 - FLOPS_DP - already used
 - FLOPS_SP - already used
 - L2 - already used
 - L3 - already used
 - MEM - already used
 - MEM_DP - just mem + FLOPS_DP combined
 - MEM_SP - just mem + FLOPS_SP combined

## Likwid stability issues
 - some key places to check:
   - file "src/perfmon.c": perfmon_startCounters(), perfmon_setupCounters(),
     perfmon_stopCounters() are all used by likwid_markerNextGroup()
   - file "src/libperfctr.c": likwid_markerStopRegion() produces the error
     "WARN: Stopping an unknown/not-started region ..." 
 - "WARN: Skipping region (null) for evaluation" messages:
   - Tried printing results of computation, still had this error quite
     frequently
   - Figured this would prevent optimizing out computation, maybe it isn't?
   - other options include: volatile, #pragma GCC optimize("O0")
   - piping output of likwid_minimal to a file in the same directory
     consistently produces a LOT of these errors
   - doesn't happen when I output to /tmp/tmp.txt, which is on the same disk,
     but a different partition
 - [ ] counters sometimes reporting unreasonably high values
   - many examples [available here](https://pastebin.com/u/rileyw13)
   - port counters sometimes reporting 1.8e19 for values
   - this also happens with many other counters
   - port_usage sometimes reporting 461375897600.000
   - also noticed L3 bandwidth was in the order of 1e11 or so for an
     execution of fhv_minimal in the double_flops region. This also doesn't
     make sense
     - this is the value reported by likwid, not a problem with my
       post-processing. Ran a test to demonstrate this, where I printed the
       result of `perfmon_get_MetricOfRegionThread` and then compared with
       the value I stored in my map:
     - L2 bandwidth: 8.33646e+16
       per_thread_results for thread 38.33646e+16
     - Wrote script to repeatedly run likwid_minimal. (see
       `tests/likwid_minimal_repeated.sh`). With groups 
       MEM|L2|L3|FLOPS_SP|FLOPS_DP|PORT_USAGE1|PORT_USAGE2|PORT_USAGE3 all
       being measured, only 1/100 iterations produced output above 1e6. In
       this case it was L3 bandwidth and volume
     - increased number of tests to 200 and ran again. 14/200 failed, some
       of this was unreasonably high output, and some of it was "skipping
       region ___ " errors
     - switched to the groups MEM|L2|L3|FLOPS_SP|FLOPS_DP and ran another
       100 iterations. 4/100 had some kind of problem
   - ran some tests with v4.3.4 and v5.0.1, on multiple groups and just one
     group, and with hyperthreading enabled and disabled
     - hyperthreading was disabled by selecting cores 0,1 in LIKWID_THREADS and
       then setting the number of openMP threads to be 2
     - First, compiling and running with the following commands:
       g++ likwid_minimal.c -L/usr/local/likwid-v4.3.4/lib -I/usr/local/likwid-v4.3.4/include -llikwid -mtune=native -fopenmp -o likwid_minimal;
       LD_LIBRARY_PATH=/usr/local/likwid-v4.3.4/lib PATH=/usr/local/likwid-v4.3.4/sbin:$PATH ./likwid_minimal_repeated.sh
     - Results with likwid v4.3.4: 
        - No hyperthreading, specifying one group (L2): 0 failures out of 100
          tests
        - With hyperthreading, specifying one group (L2): 0 failures out of 200
          tests
           - there was one case where I received the error "WARN: Skipping
             region (null) for evaluation"
        - No hyperthreading, specifying multiple groups
          (L2|L3|FLOPS_SP|FLOPS_DP): 3 failures out of 200 tests
        - With hyperthreading, specifying multiple groups
          (L2|L3|FLOPS_SP|FLOPS_DP): 12 failures out of 100 tests
           - full output available here: https://pastebin.com/qcM34Rv6
     - next, compiled with:
       g++ likwid_minimal.c -L/usr/local/likwid-master/lib -I/usr/local/likwid-master/include -llikwid -mtune=native -fopenmp -o likwid_minimal
       LD_LIBRARY_PATH=/usr/local/likwid-master/lib PATH=/usr/local/likwid-master/sbin:$PATH ./likwid_minimal_repeated.sh
     - Results with likwid compiled from master branch (commit
       99b0d23927f5e65cfa4eb5aeac1c57504395694b )
       - No hyperthreading, specifying one group (L2): 0 failures out of 100
         tests
       - With hyperthreading, specifying one group (L2): 0 failures out of 100
         tests
       - No hyperthreading, specifying multiple groups
         (L2|L3|FLOPS_SP|FLOPS_DP): 1 failure out of 100 tests
       - With hyperthreading, specifying multiple groups
         (L2|L3|FLOPS_SP|FLOPS_DP):5 failures out of 100 tests
          - full output: https://pastebin.com/X5MwUVUq
   - It seems to me that the problem is brought out by specifying multiple
     groups, but that it is exacerbated by hyperthreading. That being said, I
     do wonder if the hyperthreading problems have to do with intel (and the
     associated spectre/meltdown problems), I will try to test on my personal
     machine which uses an AMD processor.
   - seems to be related to error below about "stopping non-started region"
 - sometimes get "stopping non-started region _____"
 - sometimes get errors like the following:
   WARN: Skipping region double_flops-0 for evaluation.
   WARN: Skipping region copy-0 for evaluation.
   WARN: Regions are skipped because:
         - The region was only registered
         - The region was started but never stopped
         - The region was never started but stopped
 - [ ] convolution sometimes not instrumenting one region?
   - noticed this also in fhv_minimal. Happens once every 10 executions or
     so 
   - compiler optimization?

# 2020-05-17 through 2020-06-02
 - worked on visualization
 - created function for interpolating colors
 - created function to test interpolation
 - added exponential scale as discussed with Dr. Saule
   - scale(0.01) = 0.0509
   - scale(0.2)  = 0.6683
   - scale(0.5)  = 0.8571
 - visualized saturation on a scale inspired by
   [colorbrewer](https://colorbrewer2.org/#type=sequential&scheme=PuBu&n=9)
 - found some good colors. 
   - My favorite was (200, 200, 200) to (43, 140, 190) which is a light gray
     to a nice pale blue. 
   - Choosing a darker blue, like (2, 56, 88) might make the differences more
     clear
   - an orange-to-blue scale like (227, 74, 51) to (43, 140, 190) was an 
     interesting idea, but what the diagram was representing was less clear to
     me. I had to spend more time "learning" the scale.
 - demonstrated how visualizations highlights a change in load
   - to generate memory-stressed versions, used parameters n=67108864 d=1
     nbiter=800
   - to generate CPU-stressed versions, used parameters n=67108864 d=1000
     nbiter=80
   - in both cases, used OMP_SCHEDULE="dynamic,8"
   - compare `visualizations/polynomial_basic_cpu_poly.svg` with
     `visualizations/polynomial_block_cpu_poly_block.svg` to see how CPU is
     being better utilized with the optimized code
   - compare `visualizations/polynomial_basic_mem_poly.svg` with
     `visualizations/polynomial_block_mem_poly_block.svg` to see how in general
     saturation is better (components are more evenly saturated) with the 
     block code. Additionally, RAM is much more saturated.
 - explored some other counters we can use
   - first identified some of code to use while experimenting with counters:
     - CONVOLUTION: found some parameters that I initially thought would stress
       the CPU, but don't indicate a bottleneck when visualized with fhv. These
       parameters are: n,m = 4000; k = 15
     - POLYNOMIAL: for the parameters n=67108864 d=1 nbiter=800, seemed like L3
       was the bottleneck? But it would be good to add some clarity
   - PORT_USAGE groups might provide some insight: for convolution, Ports 3 and
     6 reported higher saturation than the others. Port 6 was 0.12, Port 3 was
     0.10. Port 2 was a close second at 0.09 saturation. The rest were at 0.05
     or lower
 - gave myself a lot of problems by forgetting to replace an optimization flag:
   - when I ran previous tests, `polynomial_block_likwid 67108864 1 10` gave a
     saturation of 0.21 for L3 cache and 0.41 for RAM. See line 14 of
     `examples/polynomial_expansion/data/block_optimized_likwid_2020-05-01_1448.csv`.
     However, now I can't replicate those results??? I'm getting saturation of
     about 0.039 for L3 and 0.082 for RAM
     - trying higher values for parameters (e.g. `polynomial_block_likwid
       100000000 1 10`) yielded slightly better saturation, but nothing close
       to the `.4` I got before. Results were an L3 saturation of  0.043 and
       RAM saturation of 0.130.
     - tried closing browser, discord, gitkraken, and other resource-heavy
       applications. Results were similar. 
       - for n=67108864, deg=1, got L3-sat: 0.0363, RAM-sat: 0.0748
       - for n=100000000, deg=1, got L3-sat: 0.0363, RAM-sat: 0.0797
         this is significantly lower?? weird
     The solution was (as is often the case) laughably obvious. I had removed
     the -O3 flag because I was debugging and didn't want symbols to be
     optimized out. I had never replaced it.
 - worked a LOT on likwid stability issues
   - wrote on this in README.md, under the heading "likwid stability
     issues". if I remove that section, it will be copied to this document.
   - SECTION FROM README HAS BEEN COPIED INTO A HEADING ABOVE
 - wrote simpler convolution for testing other counters we can use
 - made polynomial expansion also have 4 different executable targets based on
   compile-time environment variables. 

# 2020-04-30 through 2020-05-07
 - created CSV output from fhv to allow for plotting in place of architecture
   visualization that will come later
 - investigated convolution polynomial_expansion as an example kernel
 - demonstrated how RAM utilization changed
 - for some reason, polynomial_expansion (both basic and block) are using
   double-precision components even though "float" is used, which should (I
   think) use the single-precision data structure.
   - header did not match order of the data in lines below. It was reporting SP
     as DP, DP as L2, L2 as L3, L3 as MEM, and MEM as DP
   - fixed header to match and created a plot which showed how computation
     usage changed
 - created plots to demonstrate how fhv can identify bottlenecks and
   improvements 
 - spent some time considering color choices to represent saturation of
   hardware
 - began working on svg diagram of hardware
   - generates a couple rectangles
   - started calculating colors, need to pick a data structure to store those
     in 

## Thoughts on coloring of diagram:
Notes from meeting with Dr. Saule
 - anything above 20% is probably pretty well-saturated
 - color by saturation (more saturation - more intense color)

my thoughts
 - ideal is to have everything saturated
 - how should we color this visualization? More saturated things are more
   green? Do like a red-to-green scale?
 - but if only one thing is saturated, that's BAD right? because you want to
   take the load off that part and put it on others.
 - but you want EVERYTHING to be loaded.... does that mean best way to color
   is by difference in lowest to highest saturation level?
 - what if more saturation -> more color intensity, but the choice of color
   is based on difference between highest and lowest saturation?

Ideal: everything is 1.0

Good:
 - everything is 0.5

Bad:
 - something is 1.0 but other things are 0.0
 - everything is 0.0 

What if red denotes overuse, gray denotes under-use, and green denotes ideal
use

This sounds like the last point, where more saturation means more intensity of
color, but difference in saturation denotes green or red

Given saturation has a small range but grows from low to high:
 - constant hue of 125
 - start at 0.5 value, 0 saturation. Grow to 1.0 value, 1.0 saturation 

Given saturation is high but changes from having a large range to having a low
range:
 - constant value of 1.0 and saturation of 1.0
 - start at hue of 0, grow to hue of 125

Green: Hue around 125
Red: Hue around 0
Green -> Red would be 125 -> 0

# 2020-04-23 through 2020-04-30
 - added printHighlights function
 - fixed geometric mean on metrics
 - confirmed changes to performance_monitor still seems to match manual
   benchmark and make sense
   - this was not very rigorous, we still need unit tests at some point.
   - checked that benchmark without comparison still saturates, it does
   - manually counted the number of flops in fhv_minimal, performance_monitor
     output matches. 
   - in any case, I'm just collecting and aggregating data, so the chance for
     error is lower than if I was building the data myself.
 - looked at convolution for kernel to analyze
 - looked at halide

# 2020-04-16 through 2020-04-23
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

# 2020-04-09 through 2020-04-16
Stuff from last week:
 - Switched intrinsics to operator= (see [memory section](#memory)), compared
   compiler-produced assembly

Questions:
 - how do we know port usage numbers make sense?

## Learning likwid
 - pinning threads is NOT optional, but cli does it for you if you use it.
   - failure to pin threads is what was giving me problems when specifying
     groups with environment variables

## Exploration
 - Shifted convolution to use direct likwid calls except to aggregate and print
   results
 - For performance_monitor result printing: added ability to average results
   across core, because summing "port_usage" ratios doesn't really make sense
   - I haven't looked at convolution now that I've made these changes because I
     want to verify them again with benchmark_likwid_vs_manual

# 2020-03-24 through 2020-04-09
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
     numbers of groups (the max number of groups I've been able to successfully
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
 - between `benchmark`, `benchmark-likwid-vs-manual`, `thread_migration`, and
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

## Playing with likwid_minimal.c
Through the likwid-users group, I discovered that you have to put barriers
between stop/start regions

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

## Improvements to performance_monitor
 - now separates sequential and parallel regions, which are registered in init
 - overloaded init to allow specifying a number of threads instead of
   enumerating them in a c string
 - created wrapper function for nextGroup 

## What other people are doing
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

## Convolution as a case study
Commenting the line `likwid_markerNextGroup()` caused it to work with two
regions. Switched to raw likwid instead of using my performance_monitor wrapper

## Investigating port usage
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

## Applying port usage information to convolution
If the number of iterations where `likwid_markerNextGroup()` is called is a
multiple of the number of groups, things USUALLY behave well. If not:

Trying to just tack on port_usage groups to the existing MEM_DP|FLOPS_SP etc.
makes likwid hang indefinitely on its analysis after convolution is run.

instrumenting "entire_program" works if I comment out likwid_markerNextGroup()
OR if the number of iterations is a multiple of the number of groups.
Sometimes. This is very non-deterministic and it's frustrating. Making the
number of iterations a multiple of the number of groups SOMETIMES works, but as
the number of groups increases it seems to work less well.

## Memory
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
   manually calculated values

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
using direct assignment instead of the intrinsic. These higher values were
closer to calculated volumes. Why?

## For the final 3 iterations:
## Manually calculated volumes: 
Name: manually calculated volume, dtype: float64
16     5.24288
17    10.48576
18    20.97152

## Using intrinsic
Name: L3 data volume [GBytes], dtype: float64
16     3.743730
17     7.029880
18    14.544200

Name: Memory data volume [GBytes], dtype: float64
16     1.969470
17     8.604530
18    10.764700

## Direct assignment
Name: L3 data volume [GBytes], dtype: float64
16     5.010750
17    10.298100
18    21.149400

Name: Memory data volume [GBytes], dtype: float64
16     3.650170
17     8.718550
18    21.705100

# 2020-03-17 through 2020-03-24
## Memory
 - MEM_INST_RETIRED_ALL_LOAD/STORE count all retired load/store instructions,
   respectively. See Table 19-3 of "Performance monitoring events" in intel
   developer's guide
 - using these to get load/store ratios gave us ratios of 4-6x reads to writes
   (see `tests/mem_volume_through_cache_load_to_store.png`)
 - Tried to get absolute measurement of volume of data transferred. 
   - Couldn't find counter of L1 memory instructions besides evict/read (which
     we already use in L2 measurements) so I tried using
     MEM_INSTR_RETIRED_LOADS_ALL. See
     `tests/mem_volume_through_cache_total_volume.png`
     
## What other people are doing
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

# 2020-03-10 through 2020-03-17
## Memory: tried to align memory manual calculations with likwid report
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

## Convolution as a case study
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

## When both groups were started/stopped:
----- begin saturation level performance_monitor report -----
Percentage of available DP [MFLOP/s] used: 1.05821e-05
Percentage of available L2 bandwidth [MBytes/s] used: 0.0242764
Percentage of available L3 bandwidth [MBytes/s] used: 0.0197555
Percentage of available Memory bandwidth [MBytes/s] used: 0.108281
Percentage of available SP [MFLOP/s] used: 0.0298234
----- end saturation level performance_monitor report -----

## When only actual convolution was inside group:
----- begin saturation level performance_monitor report -----
Percentage of available DP [MFLOP/s] used: 8.11656e-06
Percentage of available L2 bandwidth [MBytes/s] used: 0.0215583
Percentage of available L3 bandwidth [MBytes/s] used: 0.0364879
Percentage of available Memory bandwidth [MBytes/s] used: 0.0938187
Percentage of available SP [MFLOP/s] used: 0.0249263
----- end saturation level performance_monitor report -----

## Analysis
Results are similar.... saturation levels differ on speedtest but the kernel is
also bigger

## QOL and software engineering
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

# 2020-03-03 through 2020-03-10
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

# 2020-02-25 through 2020-03-03
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

# 2020-02-18 through 25
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

# 2020-02-11 through 18
## Misc. discoveries:
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

## Integer operations:
 - Measuring integer operations is tough... 
   - some counters count both FP and INT operations, could in theory subtract
     FP ops. (See ARITH.MUL, MUL, DIV)
   - some counters only count integer vector things (see
     SIMD_INT_128.PACKED_MP&). Unfortunately, I didn't see anything for 256B or
     512B registers

## Sampling:
 - no extrapolation is done on counters.
 - when specifying multiple groups, likwid switches which group is tracked when
   likwid_markerNextGroup is called if the marker api is used. Else, the group
   being tracked switches after the time specified with the -T flag passes
   (default 2s)
   - for example: `likwid-perfctr -C S0:0 -g FLOPS_DP -g L3 -g L3 -T
     250ms -M 1 ./a.out`

## Number of Registers for hardware counters
 - on my CPU (intel i5-6300U, skylake) there are 4 customizable hardware
   counter registers per hardware thread and 8 are available per hardware core
   if hyperthreading is disabled. These are numbered PMC0-7
 - the registers used for hardware counters are part of the context switch!
 - FIXC0-2 are not customizable

## Threads and migration
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

# before 2020-02-11
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

## Some notes on what does and doesn't get counted:
FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE STAT counts one vector operation as
one retired instruction. 
It counts one vector FMA operation as 2 retired instructions

AVX SP MFLOP/s counts vector operation as 8 floating point operations: This
is what we want

so aggregate AVX SP MFLOP/s should correspond with what we expect on bench
