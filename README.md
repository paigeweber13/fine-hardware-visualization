# Fine Hardware Visualization
## Goal
Present the user with a visualization of their computer architecture and
indicate what parts of that architecture are most loaded to identify
bottlenecks in high-performance applications.

## Architecture
 - Identify architecture
 - Identify peak FLOP/s, memory bandwidth, etc.
 - Identify latency
 - Measure what actual utilization of memory/processor is
 - Compare actual utilization with peak on an piece-by-piece basis
 - Visualize that

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

### Old TODO:
These are things that directly relate to the final program we want to make, but
Dr. Saule has asked me to spend time investigating likwid and how it works,
instead of working on something that would conceivably go into the final
program

 - put likwid markers into benchmark to see if I'm doing my own math right
 - compare to theoretical max to see if we can use these operations to saturate
   floating point operations
 - are there integer operation counters?
 - get a baseline benchmark
   - try to saturate machine?
     - calculate flop/s
     - or use hardware counters
   - use theoretical numbers for comparison

As a POC, for now we're just going to focus on identifying peak performance and
actual performance and then comparing the two. We will hard code an
architecture and output the results to the command line. 

 - Benchmark machine
   - handwritten benchmark
   - run likwid to measure FLOP/s and Mem bandwidth, use this as benchmark
   - compare
 - programatically run likwid given some input executable
   - get hardware counters and time execution to calculate flop/s

## Accomplishments:
 - evaluated both likwid and papi for use
 - investigated likwid-bench
 - basic research on likwid-accesD vs direct access
 - Got likwid marker to measure code
   - investigated brandon's code
   - got my code working

## Hardware Counters
Group "FLOPS_SP" and "FLOPS_DP" seem useful.

### Some notes on what does and doesn't get counted:
FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE STAT counts one vector operation as
one retired instruction. 
It counds one vector FMA operation as 2 retired instructions

AVX SP MFLOP/s counts one vector operation as 8 floating point operations: This
is what we want
