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
   did one per physical thread

## TODO:
 - can we measure integer operations?
 - Do some basic sampling tests
	 - how many counters before sampling starts?
	 - if we add a TON of counters, are results similar?
 - check out brandon's bw and lat code, report on that
 - How does likwid behave when you..
 	- pin 1 omp thread to each physical thread? 
 	- pin 1 omp thread to each physical core?
 	- pin multiple omp threads to one physical thread?
  - start with pinning 1 omp thread to each physical thread and then partway
    through execution shift all omp threads to one physical thread (use
    pthread_setaffinity_np to migrate threads)
 - Spend a little bit of time finding out how to avoid using sudo. This is a
   LOWER priority
	 - IF I BUILD LIKWID FROM SOURCE, I CAN USE IT WITHOUT SUDO????? WTF???
 -  can likwid-accessD can only monitor at the process level?

## Accomplishments:
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

## Hardware Counters
Group "FLOPS_SP" and "FLOPS_DP" seem useful.

### Some notes on what does and doesn't get counted:
FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE STAT counts one vector operation as
one retired instruction. 
It counds one vector FMA operation as 2 retired instructions

AVX SP MFLOP/s counts one vector operation as 8 floating point operations: This
is what we want
