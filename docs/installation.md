- [Installation](#installation)
- [Preparation](#preparation)
- [Dependencies](#dependencies)
  - [Compiled from source](#compiled-from-source)
  - [Installed via package manager](#installed-via-package-manager)
  - [Automatically included](#automatically-included)


# Installation

In summary, to install all dependencies on ubuntu (should work with all
debian-based systems), follow the workflow below:

1. Compile likwid v5.1.1 from source. An overview of the process is given here,
   but see [the likwid repository](https://github.com/RRZE-HPC/likwid) for full
   instructions.
   1. Clone the repo: `git clone https://github.com/RRZE-HPC/likwid.git`
   2. Change to the likwid directory: `cd likwid`
   3. Checkout the tag for version 5.1.1: `git checkout v5.1.1`
   4. Build likwid: `make`
   5. Install likwid: `sudo make install`
   6. OPTIONAL: if you don't `make install` likwid, you will need to prepend
      any command which uses fhv with `LD_LIBRARY_PATH=/path/to/likwid/lib;
      PATH=/path/to/likwid/bin; <your command here>`
2. edit `LIKWID_PREFIX` in config.mk in the fhv root directory to match the
   location where likwid was installed in step 1. Use `which likwid-perfctr`
   for hints if you're not sure where it was installed.
3. Install build dependencies for fhv with the command `sudo apt-get install
   libboost-program-options-dev libcairo2-dev libpango1.0-dev libfmt-dev`
4. In the directory of fhv, run the following commands:
   - `git submodule init && git submodule update --recursive` - this will get
     the likwid code, which is a submodule to this repository.
   - `make`
   - `make perfgroups`
   - `sudo make install`

If you were able to compile and install without errors, you're ready to use
FHV. Skip to the `docs/usage.md` document to learn how to use FHV.

If you'd like more details of what is used and why, read the "Dependencies"
section.

# Preparation

*Note: I would like this process to be automatic, but for now this solution
will do.*

Before using FHV, you *must* gather some information about your machine. You
need to know how many execution ports per core your architecture has and you
will need to benchmark your processor and gather some key metrics.

These values will later be used as a reference by fhv to give you an idea of
how well other programs perform. For instance, if you indicate your computer
can read data from RAM with a maximum throughput of 20 GB/s, but your program
only shows 10 GB/s of performance, FHV would indicate that you are currently
using 50% of available bandwidth.

The "machine stats" file that contains these values must be named
`machine-stats.json` and may be stored in either `/etc/fhv` or `~/.config/fhv`.
In the case that there is a machine stats file in both directories, the one in
`~/.config/fhv` will be used; this allows users to have full control over
baseline data even when they do not have permissions to write to `/etc`. The
format of the machine stats file is described in a template copied to
`/etc/fhv/machine-stats-template.json` upon calling `make install`. This
template can also be found in the source code at
`./machine-stats/machine-stats-template.json`. Begin by copying this file to
one of the supported locations and renaming it to `machine-stats.json`.

To benchmark performance, run the script `benchmark.sh` in the root of this
repository. Then, open `machine-stats.json` and update the values in the
section "benchmark-results" to match the benchmark output.

For instance, to populate the variable `EXPERIENTIAL_RW_BW_L2`, read the
benchmark output to find the section `L2`. Then look for the test `copy_avx`
(because `copy_avx` test R/W bandwidth). Finally, in that section find where
the benchmark prints `MByte/s`. This is the bandwidth value that you should put
for `EXPERIENTIAL_RW_BW_L2`.

For the flop rates, look for `MFlops/s`.


# Dependencies

## Compiled from source

- **likwid >= 5.0.1:** a version above 5.0.1 is required, as this has support
  for memory counters and is confirmed to use `likwid-accessD` without root
  permissions. If this version is available with your package manager, use
  that. Otherwise, build it from source. Instructions to do this are available
  [here](https://github.com/RRZE-HPC/likwid). Be sure to change `LIKWID_PREFIX`
  in this repository's makefile to match wherever likwid is installed
- additional perfgroups not included with likwid. These can be installed by
  running `make perfgroups` in the root directory.

## Installed via package manager

- **boost/program_options:** available on [the boost
  website](https://www.boost.org/). Also installable on ubuntu with `sudo apt
  install libboost-program-options-dev` on debian-based distributions. The
  makefile assumes boost program options is already in a directory included by
  gcc.
- **cairo:** available [here](https://www.cairographics.org/), also installable
  with `sudo apt install libcairo2-dev` The makefile uses `pkg-config` to
  ensure cairo is automatically found and included.
- **pango:** pango is used for text rendering in conjunction with cairo. The
  pangocairo interface comes preinstalled with cairo, but this does not include
  development tools. Pango is available
  [here](https://pango.gnome.org/Download), or installable on ubuntu with `sudo
  apt install libpango1.0-dev`
- **{fmt}:** available [here](https://fmt.dev/latest/index.html), also
  installable with `sudo apt install libfmt-dev`

## Automatically included

- **[nlohmann/json](https://github.com/nlohmann/json):** header-only, included
  in ./lib
