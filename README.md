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
 - fix no code working anymore, mine or brandon's
 - can we measure integer operations?

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

## Hardware Counters
Group "FLOPS_SP" and "FLOPS_DP" seem useful.

### Some notes on what does and doesn't get counted:
FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE STAT counts one vector operation as
one retired instruction. 
It counds one vector FMA operation as 2 retired instructions

AVX SP MFLOP/s counts one vector operation as 8 floating point operations: This
is what we want

### Group "UOPS_EXEC"
+---------------------------------+---------+------------+------------+------------+------------+
|              Event              | Counter |   Core 0   |   Core 1   |   Core 2   |   Core 3   |
+---------------------------------+---------+------------+------------+------------+------------+
|        INSTR_RETIRED_ANY        |  FIXC0  | 5869595113 | 5826052646 | 5876331329 | 5815344572 |
|      CPU_CLK_UNHALTED_CORE      |  FIXC1  | 7117684759 | 7106295544 | 7082181830 | 7082108840 |
|       CPU_CLK_UNHALTED_REF      |  FIXC2  | 6177884856 | 6166269512 | 6138141256 | 6149367120 |
|    UOPS_EXECUTED_USED_CYCLES    |   PMC0  | 3849055949 | 3829156844 | 3848760721 | 3818075992 |
|    UOPS_EXECUTED_STALL_CYCLES   |   PMC1  | 3268635730 | 3277140174 | 3233430525 | 3264042206 |
| CPU_CLOCK_UNHALTED_TOTAL_CYCLES |   PMC2  | 7117691679 | 7106297018 | 7082191246 | 7082118199 |
|    UOPS_EXECUTED_STALL_CYCLES   |   PMC3  |          0 |   27532022 |   53368956 | 1824501171 |
+---------------------------------+---------+------------+------------+------------+------------+

+--------------------------------------+---------+-------------+------------+------------+--------------+
|                 Event                | Counter |     Sum     |     Min    |     Max    |      Avg     |
+--------------------------------------+---------+-------------+------------+------------+--------------+
|        INSTR_RETIRED_ANY STAT        |  FIXC0  | 23387323660 | 5815344572 | 5876331329 |   5846830915 |
|      CPU_CLK_UNHALTED_CORE STAT      |  FIXC1  | 28388270973 | 7082108840 | 7117684759 | 7.097068e+09 |
|       CPU_CLK_UNHALTED_REF STAT      |  FIXC2  | 24631662744 | 6138141256 | 6177884856 |   6157915686 |
|    UOPS_EXECUTED_USED_CYCLES STAT    |   PMC0  | 15345049506 | 3818075992 | 3849055949 | 3.836262e+09 |
|    UOPS_EXECUTED_STALL_CYCLES STAT   |   PMC1  | 13043248635 | 3233430525 | 3277140174 | 3.260812e+09 |
| CPU_CLOCK_UNHALTED_TOTAL_CYCLES STAT |   PMC2  | 28388298142 | 7082118199 | 7117691679 | 7.097075e+09 |
|    UOPS_EXECUTED_STALL_CYCLES STAT   |   PMC3  |  1905402149 |          0 | 1824501171 | 4.763505e+08 |
+--------------------------------------+---------+-------------+------------+------------+--------------+

+-----------------------------+-----------+-----------+-----------+-----------+
|            Metric           |   Core 0  |   Core 1  |   Core 2  |   Core 3  |
+-----------------------------+-----------+-----------+-----------+-----------+
|     Runtime (RDTSC) [s]     |    2.5141 |    2.5141 |    2.5141 |    2.5141 |
|     Runtime unhalted [s]    |    2.8517 |    2.8471 |    2.8374 |    2.8374 |
|         Clock [MHz]         | 2875.6745 | 2876.4813 | 2879.8574 | 2874.5705 |
|             CPI             |    1.2126 |    1.2197 |    1.2052 |    1.2178 |
|    Used cycles ratio [%]    |   54.0773 |   53.8840 |   54.3442 |   53.9115 |
|   Unused cycles ratio [%]   |   45.9227 |   46.1160 |   45.6558 |   46.0885 |
| Avg stall duration [cycles] |    inf    |  119.0301 |   60.5864 |    1.7890 |
+-----------------------------+-----------+-----------+-----------+-----------+

+----------------------------------+------------+-----------+-----------+-----------+
|              Metric              |     Sum    |    Min    |    Max    |    Avg    |
+----------------------------------+------------+-----------+-----------+-----------+
|     Runtime (RDTSC) [s] STAT     |    10.0564 |    2.5141 |    2.5141 |    2.5141 |
|     Runtime unhalted [s] STAT    |    11.3736 |    2.8374 |    2.8517 |    2.8434 |
|         Clock [MHz] STAT         | 11506.5837 | 2874.5705 | 2879.8574 | 2876.6459 |
|             CPI STAT             |     4.8553 |    1.2052 |    1.2197 |    1.2138 |
|    Used cycles ratio [%] STAT    |   216.2170 |   53.8840 |   54.3442 |   54.0542 |
|   Unused cycles ratio [%] STAT   |   183.7830 |   45.6558 |   46.1160 |   45.9458 |
| Avg stall duration [cycles] STAT |   181.4055 |         0 |  119.0301 |   45.3514 |
+----------------------------------+------------+-----------+-----------+-----------+

### Group "FLOPS_SP"
+------------------------------------------+---------+------------+------------+------------+------------+
|                   Event                  | Counter |   Core 0   |   Core 1   |   Core 2   |   Core 3   |
+------------------------------------------+---------+------------+------------+------------+------------+
|             INSTR_RETIRED_ANY            |  FIXC0  | 5943340688 | 5850170846 | 5883843955 | 5831180435 |
|           CPU_CLK_UNHALTED_CORE          |  FIXC1  | 7190048797 | 7170197914 | 7168408309 | 7129440607 |
|           CPU_CLK_UNHALTED_REF           |  FIXC2  | 6222696896 | 6206802368 | 6209095984 | 6173547120 |
| FP_ARITH_INST_RETIRED_128B_PACKED_SINGLE |   PMC0  |          0 |        372 |         55 |          0 |
|    FP_ARITH_INST_RETIRED_SCALAR_SINGLE   |   PMC1  |       7849 |       1152 |       5466 |       2476 |
| FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE |   PMC2  | 1000000006 | 1000000006 | 1000000006 | 1000000006 |
+------------------------------------------+---------+------------+------------+------------+------------+

+-----------------------------------------------+---------+-------------+------------+------------+--------------+
|                     Event                     | Counter |     Sum     |     Min    |     Max    |      Avg     |
+-----------------------------------------------+---------+-------------+------------+------------+--------------+
|             INSTR_RETIRED_ANY STAT            |  FIXC0  | 23508535924 | 5831180435 | 5943340688 |   5877133981 |
|           CPU_CLK_UNHALTED_CORE STAT          |  FIXC1  | 28658095627 | 7129440607 | 7190048797 | 7.164524e+09 |
|           CPU_CLK_UNHALTED_REF STAT           |  FIXC2  | 24812142368 | 6173547120 | 6222696896 |   6203035592 |
| FP_ARITH_INST_RETIRED_128B_PACKED_SINGLE STAT |   PMC0  |         427 |          0 |        372 |     106.7500 |
|    FP_ARITH_INST_RETIRED_SCALAR_SINGLE STAT   |   PMC1  |       16943 |       1152 |       7849 |    4235.7500 |
| FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE STAT |   PMC2  |  4000000024 | 1000000006 | 1000000006 |   1000000006 |
+-----------------------------------------------+---------+-------------+------------+------------+--------------+

+----------------------+-----------+-----------+-----------+-----------+
|        Metric        |   Core 0  |   Core 1  |   Core 2  |   Core 3  |
+----------------------+-----------+-----------+-----------+-----------+
|  Runtime (RDTSC) [s] |    2.5411 |    2.5411 |    2.5411 |    2.5411 |
| Runtime unhalted [s] |    2.8807 |    2.8727 |    2.8720 |    2.8564 |
|      Clock [MHz]     | 2883.9855 | 2883.3881 | 2881.6036 | 2882.4419 |
|          CPI         |    1.2098 |    1.2256 |    1.2183 |    1.2226 |
|      SP MFLOP/s      | 3148.2388 | 3148.2367 | 3148.2379 | 3148.2367 | <-- each core reporting 3 Tflop/s???? What???
|    AVX SP MFLOP/s    | 3148.2357 | 3148.2357 | 3148.2357 | 3148.2357 |
|    Packed MUOPS/s    |  393.5295 |  393.5296 |  393.5295 |  393.5295 |
|    Scalar MUOPS/s    |    0.0031 |    0.0005 |    0.0022 |    0.0010 |
+----------------------+-----------+-----------+-----------+-----------+

+---------------------------+------------+-----------+-----------+-----------+
|           Metric          |     Sum    |    Min    |    Max    |    Avg    |
+---------------------------+------------+-----------+-----------+-----------+
|  Runtime (RDTSC) [s] STAT |    10.1644 |    2.5411 |    2.5411 |    2.5411 |
| Runtime unhalted [s] STAT |    11.4818 |    2.8564 |    2.8807 |    2.8704 |
|      Clock [MHz] STAT     | 11531.4191 | 2881.6036 | 2883.9855 | 2882.8548 |
|          CPI STAT         |     4.8763 |    1.2098 |    1.2256 |    1.2191 |
|      SP MFLOP/s STAT      | 12592.9501 | 3148.2367 | 3148.2388 | 3148.2375 |
|    AVX SP MFLOP/s STAT    | 12592.9428 | 3148.2357 | 3148.2357 | 3148.2357 |
|    Packed MUOPS/s STAT    |  1574.1181 |  393.5295 |  393.5296 |  393.5295 |
|    Scalar MUOPS/s STAT    |     0.0068 |    0.0005 |    0.0031 |    0.0017 |
+---------------------------+------------+-----------+-----------+-----------+

