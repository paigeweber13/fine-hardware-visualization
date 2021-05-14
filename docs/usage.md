- [The Basics](#the-basics)
  - [Model-Specific Registers (MSRs)](#model-specific-registers-msrs)
  - [Configuring Environment Variables](#configuring-environment-variables)
  - [Tests](#tests)
  - [Examples](#examples)
- [Key Terms](#key-terms)
- [Measuring Your Code (API Usage)](#measuring-your-code-api-usage)
  - [Basic Usage](#basic-usage)
  - [Init](#init)
  - [Measuring](#measuring)
  - [Group Switching](#group-switching)
  - [Finalizing](#finalizing)
  - [Output Results](#output-results)
    - [`printDetailedResults()`](#printdetailedresults)
    - [`printAggregateResults();`](#printaggregateresults)
    - [`printHighlights();`](#printhighlights)
    - [`resultsToJson(param_string);`](#resultstojsonparam_string)
- [Create a Visualization](#create-a-visualization)
- [Advanced Usage and notes](#advanced-usage-and-notes)
  - [Thread Affinity](#thread-affinity)

# The Basics

Unless otherwise specified, don't skip this section. It contains fundamental
information about using FHV.

## Model-Specific Registers (MSRs)

Before running anything, make sure you have access to the model specific
registers (MSRs). These provide hardware counters, which are necessary for FHV
to work. If you have root permissions, you can do this by running `sudo
modprobe msr`. On a cluster, you might have to do something like `module load
msr`. If you need more help, contact the IT support for your cluster and ask
them if it's possible to get access to model specific registers (aka hardware
counters).

If you don't do this, you will get many errors, most notably "Perfmon module
not properly initialized"

## Configuring Environment Variables

Note: if you installed FHV with `make && sudo make install` WITHOUT modifying
`config.mk` or `makefile`, you may skip this section.

If you have installed `fhv` (this software) or `likwid` to a non-standard
directory, you should run run the following commands in your terminal before
using this software:

```bash
# note: change path to match your installation directory
$ export PATH=$PATH:/path/to/fhv/bin:/path/to/likwid/bin
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/fhv/lib:/path/to/likwid/lib
```

Furthermore, note that the makefiles for the tests and examples expect you to
have installed fhv to the default directory. If you have installed fhv to a
different directory, be sure to edit the respective makefiles to match.

## Tests

Don't forget to run `sudo modprobe msr` before running any code that uses
likwid or fhv!

TODO: add tests info

## Examples

don't forget to run `sudo modprobe msr` before running any code that uses
likwid or fhv!

TODO: add examples info

# Key Terms

- **FHV**: this software, fine-hardware-visualization
- **Region**: a user-defined section of code that will be measured by fhv.
- **Group**: aka likwid performance group: a list of hardware counters and
  metrics that will be measured by likwid.

# Measuring Your Code (API Usage)

In `examples/minimal` you will find a minimal example that does a good job
indicating how to use the API. The first section outlines the basic usage and
after that there is full instructions, details, and "gotchas".

As a reminder, don't forget to run `sudo modprobe msr` before running any code
that uses likwid or fhv!

## Basic Usage

There are 9 things you will need to do to measure and visualize your code's
performance.

1. Before any computation is done, call `fhv_perfmon::init()` exactly once, on
   only one thread.
2. Surround the code you want to measure with a loop that runs at least seven
   times, and in each iteration call `fhv_perfmon::startRegion(<region
   name>);`, then the code you want to measure, then
   `fhv_perfmon::stopRegion(<region name>);`. This can be done in either
   parallel or sequential blocks. `<region name>` is a user-specified string
   that identifies a region of code.
3. After calling `stopRegion` but before the end of the loop, call
   `fhv_perfmon::nextGroup();` to tell fhv to start measuring the next group of
   performance counters.
4. After all computation is done, call `fhv_perfmon::close()` exactly once, on
   only one thread.
5. Optionally print the results to `stdout` by calling
   `fhv_perfmon::printHighlights();`.
6. Finally, call `fhv_perfmon::resultsToJson();` to write the results to disk.
   By default this will create a file named `perfmon_output.json` in the same
   directory where you ran your code.
7. Compile your code with the flag `-lfhv_perfmon` included after all input
   files.
8. Run your code.
9. To visualize your code, run `fhv -v perfmon_output.json`. This will produce
   a file named `perfmon_output_<region name>.svg` in the same directory.

These steps are summarized below in pseudo-code:

```c++
int main() {
  const int NUM_FHV_GROUPS = 7;
  const char* MY_REGION = "my_region";
  fhv_perfmon::init(MY_REGION);

  for (int j = 0; j < NUM_FHV_GROUPS; j++)
  {
    #pragma omp parallel
    {
      fhv_perfmon::startRegion(MY_REGION);
      myCodeToMeasure(someParam1, someParam2);
      fhv_perfmon::stopRegion(MY_REGION);
    }
    fhv_perfmon::nextGroup();
  }

  fhv_perfmon::close();

  fhv_perfmon::printHighlights();
  fhv_perfmon::resultsToJson();
}
```

## Init

`fhv_perfmon::init()` must be called once, on one thread. This does all the set
up work required to measure your code. It automatically selects seven likwid
groups necessary to produce a visualization of your code performance.

For more control over initialization, you may provide some optional parameters.
First, you may optionally specify the region names that you will later use to
measure code (for how regions are used to measure code, see [the measuring
section below](#measuring)). Specifying region names ahead of time is
recommended, because it eliminates potential overhead that would occur at the
first call to `fhv::startRegion()` for each group. If you do not specify region
names ahead of time, regions will be initialized on the first call to
`fhv::startRegion()`.

If you would like to manually specify groups, notice that `fhv::init()` has two
different parameters for setting groups. The first is `parallel_regions` and
the second is `sequential_regions`. As you might imagine, use the first to
specify regions that will run in parallel (they will be initialized for each
thread) and use the second to specify regions that will run in a sequential
block. Each of these parameters takes a string with a comma-separated list of
regions.

If you'd like to manually select the groups that likwid will measure, use the
third optional parameter to `fhv::init()`. This parameter accepts a string with
a list of likwid groups, deliniated with the pipe symbol (`|`). For example,
`"FLOPS_SP|CYCLE_ACTIVITY|UOPS_EXEC"` is a valid list of groups. To find what
groups are supported on your system, run `likwid-perfctr -a`.

## Measuring

To identify which parts of your code should be measured, surround the code with
calls to `fhv_perfmon::startRegion(<region name>);` and
`fhv_perfmon::stopRegion(<region name>);`. This can be done in either parallel
or sequential blocks. `<region name>` is a user-specified string that
identifies a region of code. These may be optionally be specified ahead of time
in `fhv_perfmon::init`, as indicated in the [init section above](#init).

In most cases you will measure more than one group, and to do that you will
need to run your code in a loop and switch groups each iteration. To do this,
call `fhv_perfmon::nextGroup();` _after_ stopping your region.

## Group Switching

Because there are only a limited number of counters available in the hardware,
performance counters are separated into "groups". In order to measure different
parts of the architecture, like memory and in-core operations, different groups
must be measured. Most of the time you'll want to measure more than one group.
Measuring all groups specified by the default `init` routine is required for
diagram generation.

In practice, this is often done by wrapping the code you want to measure in a
loop that runs `n` times (where `n` is the number of groups being measured) and
switching groups at the end of every iteration.

Lidwid group switching _must_ be done by exactly one thread, and all regions
must be stopped before switching. `fhv_perfmon::nextGroup()` will enforce this
with `#pragma omp barrier` and `#pragma omp single`, so keep that in mind. In
practice, an openMP parallel block often only contains the calls to start the
region, run the important code, and stop the region. While not strictly
necessary, this usage is recommended to make clear that synchronization will
necessarily occur every iteration. This is demonstrated in the [basic usage
section](#basic-usage).

## Finalizing

After all calls to init, startRegion, stopRegion, and nextGroup, you _must_
call `fhv_perfmon::close()` exactly once, on exactly one thread. Only after
closing fhv_perfmon may you output results.

`fhv_perfmon::close()` does five things: it finalizes likwid, which makes the
likwid results available. Next, it loads the data from likwid, calculates port
usage ratios, aggregates results, and calculates saturation. After this you are
ready to output data.

## Output Results

There are four functions available for producing output.

### `printDetailedResults()`

This will print every event and metric gathered by fhv on a per-core basis.
This provides the highest level of detail available and is mostly useful if you
are using a custom group or debugging fhv.

### `printAggregateResults();`

This will print aggregate data about every event and metric. FHV will produce a
mean and sum across cores for every event and metric. These provide a good
overview of general performance.

### `printHighlights();`

This function highlights a couple dozen frequently used metrics and only prints
those. This function avoids highly granulated data like
`UOPS_DISPATCHED_PORT_PORT_5` (which isn't very useful on its own) and includes
things like bandwidth and MFlop/s. This is the function I use the most.

### `resultsToJson(param_string);`

This function saves gathered data to disk in json format, enabling the creation
of a visualization. It takes one parameter, an optional user-defined parameter
string designed to store data about what kind of computation was done.

For instance, in a convolution kernel you might want your parameter string to
be `"n=100000, k=9, schedule=dynamic"` so that anyone viewing the diagram later
will have some information on what kind of convolution was done. This
information will be useful while trying to determine if the reported
performance matches what you expect.

To control where this function stores the json, use the environment variable
`FHV_OUTPUT`. For instance, if you're running the program `convolution`, you
could issue the command `FHV_OUTPUT=convolution.json ./convolution`.

# Create a Visualization

To create a visualization, you must first measure some code and generate a json
by calling [resultsToJson](#resultstojsonparam_string). After that, simply run
`fhv -v ./path/to/perfmon_output.json`. An `.svg` diagram will be created in
the same directory as the json.

# Advanced Usage and notes

Region names must not have spaces.

Chapter 19 of volume 3 of the Intel software developer's manual (page 3605 in
the combined digital version) has hardware counter names. This is useful if you
want to create custom performance groups.

## Thread Affinity

To set the number of threads, OMP_NUM_THREADS may be used. For example, if you
are running `examples/minimal` and you only wanted to use 2 threads, you could
issue the command `OMP_NUM_THREADS=2 ./fhv_minimal`.

On GNU OpenMP, one may explicitly specify which cores should be used with
GOMP_CPU_AFFINITY, e.g. `GOMP_CPU_AFFINITY=0,2,8,1 ./fhv_minimal`

Note that because these are both environment variables, you may preserve this
configuration across runs by `export`ing them: e.g. `export
GOMP_CPU_AFFINITY=0,2,8,1 && ./fhv_minimal`

Another option for setting explicity thread affinity that will work with any
posix-compliant C/C++ compiler is
[sched_setaffinity](https://linux.die.net/man/2/sched_getaffinity)

For additional ways to control thread affinity (many of which work regardless
of implementation), see [the OpenMP
docs](https://pages.tacc.utexas.edu/~eijkhout/pcse/html/omp-affinity.html)
