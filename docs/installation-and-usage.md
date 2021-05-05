# Installation
In summary, to install all dependencies on ubuntu (should work with all 
debian-based systems), follow the workflow below:
1. compile likwid from source: see [here](https://github.com/RRZE-HPC/likwid)
2. edit `LIKWID_PREFIX` in config.mk in the fhv root directory to match the
   location where likwid was installed in step 1
3. Install build dependencies for fhv with the command `sudo apt-get install 
   libboost-program-options-dev libcairo2-dev libpango1.0-dev libfmt-dev`
4. In the directory of fhv, run the following commands: 
   - `make`
   - `make perfgroups`
   - (optional) `make install`

If you'd like more details of what is used and why, read the prerequisites 
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
Before running anything, make sure you have access to the MSRs
(model-specific-registers). These are used for the hardware counters. You can do
this by running `sudo modprobe msr`. Also, if you have installed `fhv` (this
software) or `likwid` to a non-standard directory, you should run run the
following commands in your terminal before using this software:

```bash
# note: change path to match your installation directory
$ export PATH=$PATH:/path/to/fhv/bin:/path/to/likwid/bin
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/fhv/lib:/path/to/likwid/lib
```


In an effort to reduce workload until I get a minimal poc, I've stopped
maintaining basically everything in this repository (benchmarks, tests are
notable examples). So many of the tests (`make tests`) don't work right now.

What still works:
 - minimal example of likwid: `make build/bin/tests/likwid-minimal-run`
 - minimal example of the fhv performance monitor: `make 
   build/bin/tests/fhv-minimal-run`
 - convolution: 
   - `cd examples/convolution`
   - `make bin/convolution-likwid-cli-run`
   - `make bin/convolution-fhv-perfmon-run`
 - fhv benchmarks *seem* to be working but haven't been rigorously tested:
   run `make` and then
   `LD_LIBRARY_PATH=/usr/local/likwid-master/lib:./build/lib build/bin/fhv -s 10000`
   or similar. Note that sometimes has problems with "stopping non-started 
   region", seems to be when you run benchmark-all. This is noted in the 
   section "problems to fix"

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
