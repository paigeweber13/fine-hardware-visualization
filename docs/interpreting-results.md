- [Key Terms](#key-terms)
- [Result Interpretation: The Big Picture](#result-interpretation-the-big-picture)
- [Understanding Numerical Results](#understanding-numerical-results)
- [Understanding Visualizations](#understanding-visualizations)
  - [How are these sections colored?](#how-are-these-sections-colored)
- [How do I interpret these results?](#how-do-i-interpret-these-results)
  - [Low saturation all around](#low-saturation-all-around)
  - [High cache/memory saturation, low FLOP saturation:](#high-cachememory-saturation-low-flop-saturation)
  - [High memory saturation, low cache saturation:](#high-memory-saturation-low-cache-saturation)
  - [High FLOP saturation, low memory saturation:](#high-flop-saturation-low-memory-saturation)
  - [Per-Core Values](#per-core-values)
- [Case Study: Reproducing Results with Examples](#case-study-reproducing-results-with-examples)
  - [Polynomial Expansion](#polynomial-expansion)
  - [Convolution](#convolution)

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
- **port usage ratio**: The ratio of how many micro-ops are executed by a given
  port x to the total number of micro-ops executed across all ports. This is
  useful because it gives insight into what kind of operations are taking up
  time in the processor backend. For instance, if you have a skylake processor
  and notice that a large portion of operations are on port 4, this would tell
  you that most operations are store operations. If you could somehow reduce
  the number of stores you would be able to make fewer stores to RAM and free
  up the instruction decoder and scheduler to spend time on operations that do
  computation. Both these things would improve speed.
- **counter**: The raw value of a hardware counter. Can be things like
  "FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE" (number of retired single-point
  avx2 operations) or "L1D_M_EVICT" (number of cachelines evicted in L1 data
  cache, i.e. the number of cache lines that had to be written back to L2.) 
- **metric**: A value calculated from a hardware counter. These are often more
  readily interpretable than counters. These are things like Flops/s and
  bandwidths. 

Also, note that "memory" and "RAM" are used interchangeably in this document.

# Result Interpretation: The Big Picture

Before looking at any data, it's good to develop an idea of what kind of things
you expect. If you have no clue, you might still gain insights from looking at
data, but it's much better to reason about your code at first and make some
hypotheses about what will happen. This way you've already put some thought
into *why* your code might perform a certain way, which will make it much
easier to interpret results down the road.

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

The visualization is intended to be a symbolic representation of a typical
modern processor and RAM architecture, where both single- and double-precision
flops are performed within the core and operations are split among several
execution ports. Data is retreived from the lowest level of cache possible, or
from RAM if the data is not cached.

The visualization is designed to give an at-a-glance overview of performance,
so metrics are all aggregations.

## How are these sections colored?

Ports are colored according to their *port usage ratios*, and all other
sections are colored according to their *saturations*.

The blocks representing RAM and the various cache levels are colored according
to read/write bandwidth saturation. Each level that we measure also has two
arrows, one going away from that section and one going towards that section.
These arrows are colored according to read saturation and write saturation,
respectively. These are all colored according to the *sum* of the same
bandwidth metric across all cores.

The "in-core" section has blocks representing each port and blocks representing
SP and DP flops/s performance. The FLOP/s blocks are colored according to the
saturation of single- and double-precision flops/s, respectively. Again, these
are colored according to the *sum* of the single- and double-precison flop
rates across all cores.

Ports are colored according to the geometric mean of the port usage ratios
across all cores.

Notice that the color scale is logarithmic: 0.1 is dramatically more colored
than 0.01, but 0.2 is only slightly more colored than 0.1. This is because
fully saturating any part of your architecture is very difficult outside of toy
microbenchmarks designed specifically to saturate a component. In most
real-world situations, saturating 50% of a component is about the maximum you
can hope to achieve, and saturating 30% of a component is still very good.
Therefore, we made the decision to exaggerate the differences between
0.0 saturation and 0.2 saturation.

# How do I interpret these results?

Most of the time you will focus on bandwidth and flop rates, though port usage
metrics can also give you some insight onto what is happening within the core.
Below are a few common cases and what they mean. These are by no means
exhaustive, but they are indended to help get you thinking about your code.

## Low saturation all around

Most likely, your code is either sequential or not vectorized. See ["Per-Core
Values"](#per-core-values) for hints on determining if your code is
parallelized as you expect. For hints about vectorization, read the rest of
this section.

Modern compilers are *very* good at generating AVX and SSE instructions from
naive code, and therefore using [intel
intrinsics](https://software.intel.com/sites/landingpage/IntrinsicsGuide/#!) is
often not required. In fact, hand-written code using intrinsics often performs
worse than naive code compiled with the `-O3` flag. The people who write these
compilers are almost always smarter than those of us who just use the compilers
:). 

To determine if your low saturation is coming from a lack of vectorization, the
first step is to inspect the assembly generated by the compiler. With GCC/G++
you can use the `-S` compiler flag. Look for things like "fmadd", vadd, "vmov",
and really any instruction beginning with "v".

Some kernels are difficult to vectorize and will benefit from use of
intrinsics. One example is convolution. If you have a 3x3 kernel, you're only
accessing 3 items in contiguous memory, and the compiler typically won't use
SSE/AVX ops unless there are 4 or 8 items (respectively) in contiguous memory.

If your code isn't vectorized, think about if you can re-organize the flow of
execution so that contiguous chunks are accessed. You may be able to change the
nesting of loops, for instance. After each change, inspect the assembly to see
if the compiler is generating the code you want. Additionally, it can be very
valuable to *try* to use intrinsics, even if you eliminate them later. Using
intrinsics will get you reasoning about how you can operate on an entire vector
of operands at once.

## High cache/memory saturation, low FLOP saturation:

If the flops units are undersaturated but the cache and RAM report high
saturation, There are a few things you can try. The first thing to look at is
how your data is accessed; do you have regular striding, or do you access items
contiguously in memory? If neither of these cases apply, the processor will
have a hard time predicting what data you will access next and will have a hard
time pipelining data into the cache. Regular, predictable memory accessing is
key. 

Perhaps you could look at reducing the number of copies of data or
the number of intermediate data structures. Is there a way you can perform your
calculation in-place rather than copying the result to another data structure? 

## High memory saturation, low cache saturation:

Your data is too large to fit in cache. Sometimes this is unavoidable, you will
have to decide if this is the case for your code. Keep in mind that even if the
entire problem is too large to fit in cache (it almost certainly is), there may
be some data that you can fit in cache. For instance, with convolution, your
image may be too large to fit in L3 cache, but your kernel is almost certainly
small enough to fit in L1 cache. Think about what data is accessed most
frequently and see if you can fit it into cache.

Convolution is a good example of a case where the naive implementation will
have low cache saturation, but by cleverly organizing the flow of execution
cache usage can be dramatically improved. This is done by processing the image in
"chunks" instead of line-by-line, i.e. by splitting the image into 100x100
pixel squares and finishing convolution in an entire square before moving onto
the next one. In the naive, line-by-line implementation, the processor has to
load part of the next line to perform convolution, but by the time you're at
the end of the line, that part of the next line will have been evicted from
cache and it will have to be re-loaded when the program moves on to the next
line. By processing in chunks, parts of the lines before and after the current
line are kept in-cache and performance improves.

## High FLOP saturation, low memory saturation:

Oftentimes, this is the ideal situation! Modern computers are almost always
memory-bound, and it can be very difficult to have good FLOPS saturation with
real-world problems. The only time I can think of where this would be bad is if
your code is performing redundant computation.

## Per-Core Values

This section applies only to numerical results, as the visualization currently
only reports aggregate results.

Per-core values can also be useful, though this is less often the case.
Per-core values can help you identify if your code is actually parallelizing
the way you expect. Is one core doing all the work? (Did you add an openmp
block but forget to add the `-fopenmp` compile-time flag?)

Also, if you have code that has both sequential and parallel computation, you
might add regions for both the sequential and parallel parts and the compare
how much work is done in each region.  Does the sequential code makes up a
large part of your code's execution?

# Case Study: Reproducing Results with Examples

To help the reader understand how FHV can be used to gain insights into
software and to demonstrate the accuracy of FHV, this section will describe
what results to expect from the examples included in this repository. 

## Polynomial Expansion
TODO

## Convolution
TODO
