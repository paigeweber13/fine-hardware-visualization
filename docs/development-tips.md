# Makefile

# Source Files

# Suggested Workflow

The makefile provides two convenience rules to adjust the `PATH` and `LD_LIBRARY_PATH` variables. These rules are `make exports` and `make devexports`.

`make exports` is only useful if likwid or fhv are built to non-standard prefixes (the prefix used is set in `config.mk`). If you use the default prefix of `/usr/local`, linux will automatically find the libraries and headers for both likwid and fhv. However, if you're developing fhv, it can be annoying to call `sudo make install` after every change. This is why the `devexports` rule is provided.

By running `export $(make devexports)` in a terminal, you set the environment variables to first look in the `./build/*` directories for libraries and executables before looking in `/usr/local/*`. This way you can just run `make` and then immediately run `fhv` and see the changes updated. Linux will sometimes still pick up on the installed library so it's best to remove it first with `sudo make uninstall`

Note that to test with an example you will still need to run `sudo make install`.

# Custom Counters and Performance Groups

## Identifying events

To create custom perfgroups like those in the `perfgroups` directory, you
should first identify what performance counters you want to use and experiment
with them on your own. A great resource to discover what is available on your
system is `likwid-perfctr`. Assuming you have likwid installed, run
`likwid-perfctr -e` to get a list of all the events supported by likwid and all
counters available on your architecture (i.e. the registers which can be
configured to count an event). The [Intel Software Developer's
Manual](https://software.intel.com/content/www/us/en/develop/articles/intel-sdm.html)
can help you understand what these counters are. Note that when intel uses `.`
in an event name, likwid replaces the `.` with `_`. Once you've identified a
few key events to watch, first explore them with the `likwid-perfctr` tool.
This section will outline the basic usage of `likwid-perfctr`, but go to [the
likwid docs](https://github.com/RRZE-HPC/likwid/wiki/likwid-perfctr) for full
usage information. 

A good place to start when exploring events and counters are the likwid
examples, available in the `examples` directory of the likwid repository.
Suppose you've identified two events you'd like to explore, `L2_LINES_IN_ALL`
and `L2_TRANS_L2_WB`. 

To measure those two events, go to the `examples` directory of the likwid
repository, call `make`, and then issue the following command:

`likwid-perfctr -C 0-$(nproc) -m -M 1 -g L2_LINES_IN_ALL:PMC0,L2_TRANS_L2_WB:PMC1 ./C-likwidAPI`

This command uses `likwid-perfctr` to instruct the processor to count
occurances of the event `L2_LINES_IN_ALL` in the counter `PMC0` and occurances
of the event `L2_TRANS_L2_WB` in the counter `PMC1`. The `PMC` counters are
general-purpose (and therefore configurable). Most of the other counters, like
`FIXC`, `PWR`, or `VTG`, are not configurable. The only other counters I know
of which are configurable are `MBOX0`, which will accept a limited number of
memory-specific counters.

At the time of writing, the `C-likwidAPI` program will produce basic output
about those counters, such as `event set L2_LINES_IN_ALL:PMC0 at CPU 0:
6563341.000000`. You can modify that program to perform the kind of computation
you're interested in.

## Creating a Custom Perfgroup

Once you've identified the events you'd like to track, you can create a custom
perfgroup by mimicing the format of those in the `groups` directory of the
likwid repository. After creating your perfgroup, all you need to do is place
it into `<likwid-prefix>/share/likwid/perfgroups/<your-architecture>/`.
Remember the default likwid prefix is `/usr/local`. This repository is set up
so that all perfgroups in the `perfgroups` directory of this repository are
copied to the likwid perfgroups directory when calling `make perfgroups` or
`make install`.

The syntax of perfgroup files does not seem to be documented in the likwid
documentation. However, the syntax is fairly simple and shouldn't give you too
much trouble. There are three primary sections: `SHORT`, `EVENTSET`, and
`METRICS`. `SHORT` is a short description that is displayed when calling
`likwid-perfctr -a`. `EVENTSET` specifies which events should be assigned to
which counters, you will probably only need to change the `PMC` counters.
Finally, the `METRICS` section tells likwid what metrics to calculate and how
they are calculated. the syntax seems to be `<name of metric> <formula>`. See
the likwid perfgroups for examples.

## More Information about Architecture Events

Note that the list of events produced by `likwid-perfctr -e` is probably not an
exhaustive list of the events your architecture can track. For a full list of
events supported on intel architectures, consult the [Intel Software
Developer's
Manual](https://software.intel.com/content/www/us/en/develop/articles/intel-sdm.html).
Custom events can be specified using tools like [likwid
perfmon](https://rrze-hpc.github.io/likwid/Doxygen/group__PerfMon.html) (a
low-level API underlying the likwidAPI and markerAPI) or
[PAPI](https://icl.utk.edu/papi/) (a powerful, low-level performance monitoring
tool designed to use hardware counters). Specifying events requires use of the
`id` and `umask` of the event. 

This level of granularity is rarely required and is outside the scope of this
repository, so no further notes on usage will be supplied.

# Adding Another Architecture
TODO

# How `likwid-bench` Works
TODO
