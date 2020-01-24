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
As a POC, for now we're just going to focus on identifying peak performance and
actual performance and then comparing the two. We will hard code an
architecture and output the results to the command line. 

 - Benchmark machine
 - Identify actual resource utilization when running some software
 - Compare benchmark with actual
 - Test with software that uses a lot of memory and no CPU or vice-versa

