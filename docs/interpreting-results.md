- [Key Terms](#key-terms)
- [Understanding Numerical Results](#understanding-numerical-results)
- [How are FHV Visualizations Created?](#how-are-fhv-visualizations-created)
- [Creating and Interpreting Visualizations](#creating-and-interpreting-visualizations)

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

# Understanding Numerical Results
TODO

# How are FHV Visualizations Created?
TODO

# Creating and Interpreting Visualizations
TODO
