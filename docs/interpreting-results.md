- [Key Terms](#key-terms)
- [Understanding Numerical Results](#understanding-numerical-results)
- [Understanding Visualizations](#understanding-visualizations)
- [Case Study: Reproducing Results with Examples](#case-study-reproducing-results-with-examples)

# Key Terms

These words are used throughout the code and documentation and they have very
particular meanings.

- **saturation**: The ratio of how much a compute resource is being used to how
  much performance is possible from that resource. For example, if a machine is
  benchmarked and shown to have a maximum possible real-world bandwidth of `80
  GiB/s` writing to L2 cache, and during code execution only `20 GiB/s` of
  write bandwidth is used, FHV will report an L2 write saturation of `0.25`.
  Intuitively, this is how *saturated* a particular resource is.
- **micro-op**: aka "uop", where 'u' is used in place of the greek symbol 'μ'.
  Because modern x86 processors have a CISC architecture, a single instruction
  may require multiple lower-level instructions in the processor. "Micro ops"
  are these lower-level instructions. 
- **port**: aka "execution port": this is a generic term for ALUs/load
  units/store units/branching units that execute micro-ops on the backend. This
  term is borrowed directly from intel's documentation.
  [Wikichip](https://en.wikichip.org/wiki/intel/microarchitectures/skylake_(client)#Individual_Core)
  has many good visualizations of ports.
- **port usage**: The ratio of how many micro-ops are executed by a given port
  x to the total number of micro-ops executed across all ports. This is useful
  because it gives insight into what kind of operations are taking up time in
  the processor backend. For instance, if you have a skylake processor and
  notice that a large portion of operations are on port 4, this would tell you
  that most operations are store operations. If you could somehow reduce the
  number of stores you would be able to make fewer stores to RAM and free up
  the instruction decoder and scheduler to spend time on operations that do
  computation. Both these things would improve speed.
- **counter**: The raw value of a hardware counter. Can be things like
  "FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE" (number of retired single-point
  avx2 operations) or "L1D_M_EVICT" (number of cachelines evicted in L1 data
  cache, i.e. the number of cache lines that had to be written back to L2.) 
- **metric**: A value calculated from a hardware counter. These are often more
  readily interpretable than counters. These are things like Flops/s and
  bandwidths. 

# Understanding Numerical Results

The information in this section applies to the output produced by the fhv
functions `printDetailedResults()`, `printAggregateResults();`,
`printHighlights();`, and `resultsToJson(param_string);`. 

Results are always reported by region. Within each region, results are
separated into two primary groups: *per-thread* results and *aggregate*
results. Per-thread results report counters and metrics for only a single core.
So per-thread Flops/s would tell you the flop rate for only a single core. 

Be aware that per-thread results can sometimes be misleading. For instance,
the intel skylake architecture shares the memory controller across all cores,
so per-core DRAM counters for all cores except core 0 will report zeroes.

There are several different types of results aggregation to be found in the
"aggregate" section of results. These are self-explanatory. The "sum" section
under "aggregate results" is the sum of the same counter or metric across all
cores, and so for the arithmetic and geometric means.

The three functions whose identifiers start with "print" have the same format.
On each line of code the region is first printed, followed by the thread or
aggregation type associated with the value, followed by the name of the value
and finally the actual value itself.

The JSON output follows much the same format, but wraps all regions in an
object called `region_results`. Within each region is a section for each
aggregation type and each thread. Only metrics are included, as counters aren't
really useful on their own. Additionally, only the metrics used to make the
visualization are included. There is also a section in the JSON object `info`
which includes information about the processor which the code ran on and a
user-defineable parameter string. The parameter string will be included in the
visualization and is intended to help identify what parameters will passed to
the user's kernel. This is important; many kernels will have different
performance needs depending on what parameters are passed to it.

# Understanding Visualizations
TODO

# Case Study: Reproducing Results with Examples

To help the reader understand how FHV can be used to gain insights into
software and to demonstrate the accuracy of FHV, this section will describe
what results to expect from the examples included in this repository. 
