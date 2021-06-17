# Makefile

# Source Files

# Suggested Workflow

The makefile provides two convenience rules to adjust the `PATH` and `LD_LIBRARY_PATH` variables. These rules are `make exports` and `make devexports`.

`make exports` is only useful if likwid or fhv are built to non-standard prefixes (the prefix used is set in `config.mk`). If you use the default prefix of `/usr/local`, linux will automatically find the libraries and headers for both likwid and fhv. However, if you're developing fhv, it can be annoying to call `sudo make install` after every change. This is why the `devexports` rule is provided.

By running `export $(make devexports)` in a terminal, you set the environment variables to first look in the `./build/*` directories for libraries and executables before looking in `/usr/local/*`. This way you can just run `make` and then immediately run `fhv` and see the changes updated. Linux will sometimes still pick up on the installed library so it's best to remove it first with `sudo make uninstall`

Note that to test with an example you will still need to run `sudo make install`.
