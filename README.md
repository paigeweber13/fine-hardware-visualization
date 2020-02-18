# Fine Hardware Visualization
## Goal
Present the user with a visualization of their computer architecture and
indicate what parts of that architecture are most loaded to identify
bottlenecks in high-performance applications.

## Running
After cloning the repository, there are a couple things you can do:
 - for a quick demo of how the program works, run `make` and then run
   `bin/bench`
 - to build and run the (currently limited) test suite, run `make tests`

## Architecture of Program
 - Identify hardware architecture
 - Identify peak FLOP/s, memory bandwidth, etc.
 - Identify latency
 - Measure what actual utilization of memory/processor is
 - Compare actual utilization with peak on an piece-by-piece basis
 - Visualize that

## Some things to keep in mind:
 - take notes on what I'm doing inside this readme! This will be used later to
   justify the work I'm doing this semester
 - end goal is to apply this to graph problems: kernels in graph problems tend
   to change behavior throughout execution
 - Dr. Saule said last week "now we know likwid counts by region"... what
   exactly does that mean? I guess that he's referring to how it aggregates by
   region... but it also aggregates per physical thread: running flops code
   twice did not change the number of regions reported by likwid: still only
   did one per physical thread. But within each thread, both runs were included
   even though we closed and opened that region. So you can like... re-enter
   regions

## TODO:
 - can we measure integer operations?
   - some counters count both FP and INT operations: 
     - ARITH.MUL includes int and fp multiplication
     - FP_COMP_OPS_EXE
     - MUL
     - DIV
   - it is possible to count by port, which could help us classify operations
     - UOPS_EXECUTED.PORT0/1
     - UOPS_EXECUTED.PORT2_CORE
     - UOPS_EXECUTED.PORT5
     - UOPS_EXECUTED.PORT015
     - UOPS_EXECUTED.PORT234
     - unforunately, most float and int operations share ports 0 and 1
   - possible to count vector stuff:
     - SIMD_INT_64.PACKED_MPY counts packed multiply operations
     - FP_COMP_OPS_EXE.SSE2_INTEGER
     - SIMD_INT_128.PACKED_MPY
   - we can create custom event sets
   - event groups are stored in $(PREFIX)/share/likwid/perfgroups/$(ARCH)/
   - ex /usr/local/share/likwid/perfgroups/skylakeFLOPS_SP.txt
 - Do some basic sampling tests
   - if you specify more than one group, switch groups in a round-robin
     fashion. 
     - for instance: 
       `likwid-perfctr -C S0:0 -g FLOPS_DP -g FLOPS_SP -g L3 -T 250ms -M 1 -m
       ./a.out`
       will switch groups every 250ms (if -T is not specified, default is 2s)
     - this does NOT do any sort of extrapolation, just reports that time
       period
     - I've struggled to get this working for me... switching just doesn't
       happen?
     - fixed it: all I had to do was add a call to "likwid_markerNextGroup"
     - this overrides time set by -T. It does not switch after a given time,
       but just when that function is called
     - so we'd have to run whatever it is multiple times
     - would be possible to switch by time if we use the command line interface
     - had very similar rates (MFLOP/s) but half the number of flops when
       running two groups
	 - how many counters before sampling starts?
     - to test this, I specified custom groups. See:
       https://github.com/RRZE-HPC/likwid/wiki/likwid-perfctr#using-custom-event-sets
     - number of available registers is architecture dependent and changes
       based on if you have hyperthreading enabled/disabled. On my laptop,
       PMC0-3 are available without turning off hyperthreading
        - likwid reports "WARN: Counter PMC4 is only available with deactivated
          HyperThreading. Counter results defaults to 0."
        - I assume if you want to use PMC4-7 those would be used in context
          switches but if you disable hyperthreading then you can use them 
        - when trying to use counter PMC8 and above, likwid reports "WARN:
          Counter PMC8 not defined for current architecture"
        - it also reports "WARN: Counter FIXC0 already used in event set,
          skipping". I wonder if it's trying to wrap around with counters?
        - the FIXC0-2 registers will only allow certain counters
        - even trying to move a counter that works on FIXC0 to FIXC2 will not
          work
	 - if we add a TON of counters, are results similar?
        - abstracted ones are, but no extrapolation is done for actual counts
   - note: doesn't seem to be able to re-initialize, so you can't change the
     group name. Have to specify all at the beginning and then call
     likwid_markerSwitch() 
 - How does likwid behave when you..
  - NOTE: expected performance is 371 GFlop/s
 	- pin 1 omp thread to each physical thread? 
    - 193.8 GFlop/s
 	- pin 1 omp thread to each physical core?
    - likwid calls both threads and cores "threads"
    - pinned one thread to core 0 and one thread to core 2
    - got 100.1 GFlop/s
 	- pin multiple omp threads to one physical thread?
    - according to
      https://github.com/RRZE-HPC/likwid/wiki/TutorialMarkerC#problems , this
      is not supported. Access to hash table entries is not thread safe
    - I tried it anyways. 
      - most threads reported "-22" for all counters... some kind of code?
      - still did all operations
      - reported 56.8 GFlop/s, which is about 1/4 our 4-core measurement and
        1/2 our 2-core measurement
  - play with thread migration
    - use pthread_setaffinity_np to migrate threads
      - pthread_setaffinity_np is unnecessary because likwid_pinThread does the
        same much more easily
    - note: IDs reported by omp_get_thread_num do not change when cores are
      migrated! I assume this is because the IDs are unique to OpenMP threads,
      not physical cores
    - SCHED_GETCPU(3) will tell you what core is being used
    - swap odd and even cores
      - only even-numbered omp threads were allowed to do work, but odd cores
        also reported data. This indicates that likwid checks on a hardware
        core level and not by omp thread.
      - core 2 didn't have work? I suppose that's because of hyperthreading:
        maybe hardware thread 0 will run work for hardware thread 1 because
        they're on the same physical core?
      - yeah, swapping even and odd cores is somewhat non-deterministic and I'm
        pretty sure it's because of hyperthreading
    - instead of swapping even and odd, started on cores 1 and 2 and moved to
      cores 3 and 4
      - when I did start region -> run1 -> migrate -> run2 -> stop region, I
        got "WARN: Stopping an unknown/not-started region flops". So sometimes
        one thread would start a region and another would stop it... this is
        interesting, and further reinforces the theory that regions and stuff
        is done on a hardware-thread level
      - this was changed to start and stop regions immediately surrounding runs
      - not all cores always had work reported... but thread 2 or 3 almost
        always had work, which would not be possible if likwid wasn't tracking
        hardware threads
      - adding "#pragma omp barrier" after starting regions and before stopping
        them" made everything report work every time
      - if regions were started and stopped immediately surrounding work, only
        threads 0 and 1 reported work.... which corresponds with openmp
        threads, not physical threads... Does this mean when a region is
        started it sets it up with omp threads but then measures the associated
        physical threads?
    - move single thread from core 0 to core 2
      - seems that if you start region on one core and stop on another, nothing
        is reported
      - starting region on one core, siwtching to another, doing work,
        siwtching back to first, and stopping region causes only first core to
        report work
    - start with pinning 1 omp thread to each physical thread and then partway
      through execution shift all omp threads to one physical thread 
      - only reported work for final core that had all threads.... weird.
 - Spend a little bit of time finding out how to avoid using sudo. This is a
   LOWER priority
	 - IF I BUILD LIKWID FROM SOURCE, I CAN USE IT WITHOUT SUDO????? WTF???
   - likwid-perfctr in ubuntu repos is version 4.3.0
   - likwid-perfctr built from source is version 5.0.1
 - can likwid-accessD can only monitor at the process level?
   - one test: ran thread migration 0,1,2,3 -> 0
     - in both cases, only thread 0 reported work
     - with direct access 4.0e8 packed sp float ops
     - with accessD 16.0e8 packed sp float ops - four times the amount???
   - both access modes reported same number of fp ops when migrating 0,1 -> 2,3
     - the fact that results were the same is telling me it's reporting on
       hardware level, because only openmp threads 0,1 are allowed to do work
       but all four hardware threads report work
     - this tells me accessD is NOT monitoring on the process level
 - explore accessD code to better understand how it works?
   - would have to compare version 4.3.0 and 5.0.1 to really understand lack of
     need for sudo in new version
   - could use strace to evaluate need for sudo
 - something I discovered:
   - counts were inconsistent, but adding "#pragma omp barrier" after
     perfmon.startRegion and before perfmon.stopRegion fixed this
   - I later discovered that an alternate solution was to run
     perfmon_startCounters in perfmon.init and perfmon_stopCounters in
     perfmon.close and this also fixed the problem, even after removing the
     barriers
   - code where threads migrated still needed the barrier before stopping
     regions, likely because some regions were getting stopped by a thread not
     doing work, but before work was done.
 - check out brandon's bw and lat code, report on that

## Accomplishments:
### before 2020-02-11
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

### 2020-02-11 through 18
 - discovered "vectorization ratio" metric in likwid
 - there are counters for NVIDIA gpus - use the -W flag in likwid-perfctr

## Hardware Counters
Group "FLOPS_SP" and "FLOPS_DP" seem useful.

### Some notes on what does and doesn't get counted:
FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE STAT counts one vector operation as
one retired instruction. 
It counds one vector FMA operation as 2 retired instructions

AVX SP MFLOP/s counts vector operation as 8 floating point operations: This
is what we want

so aggregate AVX SP MFLOP/s should correspond with what we expect on bench
