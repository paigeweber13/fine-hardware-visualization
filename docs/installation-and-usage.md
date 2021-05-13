# Installation
In summary, to install all dependencies on ubuntu (should work with all 
debian-based systems), follow the workflow below:
1. compile likwid from source: see [here](https://github.com/RRZE-HPC/likwid)
2. edit `LIKWID_PREFIX` in config.mk in the fhv root directory to match the
   location where likwid was installed in step 1
3. Install build dependencies for fhv with the command `sudo apt-get install 
   libboost-program-options-dev libcairo2-dev libpango1.0-dev libfmt-dev`
4. In the directory of fhv, run the following commands: 
   - TODO: figure out what git submodule commands are needed
   - `make`
   - `make perfgroups`
   - `sudo make install`

If you'd like more details of what is used and why, read the "Dependencies"
section.

# Dependencies
## Compiled from source
 - **likwid >= 5.0.1:** a version above 5.0.1 is required, as this has support
   for memory counters and is confirmed to use `likwid-accessD` without root
   permissions. If this version is available with your package manager, use
   that. Otherwise, build it from source. Instructions to do this are available
   [here](https://github.com/RRZE-HPC/likwid). Be sure to change
   `LIKWID_PREFIX` in this repository's makefile to match wherever likwid is 
   installed
 - additional perfgroups not included with likwid. These can be installed by
   running `make perfgroups` in the root directory.

## Installed via package manager
 - **boost/program_options:** available on [the boost
   website](https://www.boost.org/). Also installable on ubuntu with `sudo apt
   install libboost-program-options-dev` on debian-based distributions. The
   makefile assumes boost program options is already in a directory included by
   gcc.
 - **cairo:** available [here](https://www.cairographics.org/), also
   installable with `sudo apt install libcairo2-dev` The makefile uses
   `pkg-config` to ensure cairo is automatically found and included.
 - **pango:** pango is used for text rendering in conjunction with cairo.
   The pangocairo interface comes preinstalled with cairo, but this does not
   include development tools. Pango is available 
   [here](https://pango.gnome.org/Download), or installable on ubuntu with 
   `sudo apt install libpango1.0-dev`
 - **{fmt}:** available [here](https://fmt.dev/latest/index.html), also
   installable with `sudo apt install libfmt-dev`

## Automatically included
 - **[nlohmann/json](https://github.com/nlohmann/json):** header-only, included
   in ./lib

# Running
Before running anything, make sure you have access to the model specific
registers (MSRs). These provide hardware counters, which are necessary for FHV
to work. If you have root permissions, you can do this by running `sudo
modprobe msr`. On a cluster, you might have to do something like `module load
msr`. If you need more help, contact the IT support for your cluster and ask
them if it's possible to get access to model specific registers (aka hardware
counters).

## Configuring Environment Variables
Note: if you installed FHV with `make && sudo make install` WITHOUT modifying `config.mk` or `makefile`, you may skip this section.

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
TODO: add tests info

## Examples
TODO: add examples info

# Measuring Your Code (API Usage)
In `examples/minimal` you will find a minimal example that does a good job
indicating how to use the API. The next section outlines the basic usage, and
that is followed by full instructions, detaiils, and "gotchas".

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

#pragma omp parallel
  {
    for (int j = 0; j < NUM_FHV_GROUPS; j++)
    {
      fhv_perfmon::startRegion(MY_REGION);
      myCodeToMeasure(someParam1, someParam2);
      fhv_perfmon::stopRegion(MY_REGION);
      fhv_perfmon::nextGroup();
    }
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
measure code (for more on regions, see [the measuring section
below](##measuring)). Specifying region names ahead of time is recommended,
because it eliminates potential overhead that would occur at the first call to
`fhv::startRegion()` for each group. If you do not specify region names ahead
of time, regions will be initialized on the first call to `fhv::startRegion()`. 

If you would like to manually specify groups, notice that `fhv::init()` has two
different parameters for setting groups. The first is `parallel_regions` and
the second is `sequential_regions`. As you might imagine, use the first to
specify regions that will run in parallel (they will be initialized for each
thread) and use the second to specify regions that will run in a sequential
block. Each of these parameters takes a string with a comma-separated list of regions.

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
in `fhv_perfmon::init`, as indicated above.

In most cases you will measure more than one group, and to do that you will
need to run your code in a loop and switch groups each iteration. To do this,
call `fhv_perfmon::nextGroup();` *after* stopping your region.


# Usage Notes
 - Customizing which HW threads are used:
   - To set the number of threads , OMP_NUM_THREADS may be used
   - On GNU OpenMP, one may explicitly specify which cores should be used with
     GOMP_CPU_AFFINITY, e.g. `export GOMP_CPU_AFFINITY=0,2,8,1`
   - For other implementations of OpenMP, see [the OpenMP docs](
     https://pages.tacc.utexas.edu/~eijkhout/pcse/html/omp-affinity.html)
   - Another option is [sched_setaffinity](
     https://linux.die.net/man/2/sched_getaffinity)
 - Region names must not have spaces
 - Chapter 19 of volume 3 of the Intel software developer's manual (page 3605
   in the combined digital version) has hardware counter names. This is useful
   if you want to create custom performance groups
