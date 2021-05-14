- [Installation](#installation)
- [Dependencies](#dependencies)
  - [Compiled from source](#compiled-from-source)
  - [Installed via package manager](#installed-via-package-manager)
  - [Automatically included](#automatically-included)

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

If you were able to compile and install without errors, you're ready to use
FHV. Skip to the `docs/usage.md` document to learn how to use FHV.

If you'd like more details of what is used and why, read the "Dependencies"
section.

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
