# Fine Hardware Visualization
## Goal
Present the user with a visualization of their computer architecture and
indicate what parts of that architecture are most loaded to identify
bottlenecks in high-performance applications.

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
   - possible to count vector stuff:
     - SIMD_INT_64.PACKED_MPY counts packed multiply operations
     - FP_COMP_OPS_EXE.SSE2_INTEGER
     - SIMD_INT_128.PACKED_MPY
   - we can create custom event sets
   - event groups are stored in $(PREFIX)/share/$(ARCH)/
   - ex /usr/local/share/skylake/FLOPS_SP.txt
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
 - check out brandon's bw and lat code, report on that
 - How does likwid behave when you..
 	- pin 1 omp thread to each physical thread? 
 	- pin 1 omp thread to each physical core?
 	- pin multiple omp threads to one physical thread?
    - according to
      https://github.com/RRZE-HPC/likwid/wiki/TutorialMarkerC#problems , this
      is not supported. Access to hash table entries is not thread safe
  - start with pinning 1 omp thread to each physical thread and then partway
    through execution shift all omp threads to one physical thread (use
    pthread_setaffinity_np to migrate threads)
 - Spend a little bit of time finding out how to avoid using sudo. This is a
   LOWER priority
	 - IF I BUILD LIKWID FROM SOURCE, I CAN USE IT WITHOUT SUDO????? WTF???
 -  can likwid-accessD can only monitor at the process level?

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

AVX SP MFLOP/s counts one vector operation as 8 floating point operations: This
is what we want
