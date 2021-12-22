Relationscutter
===============
Calculates the leakage from the relations output of a modified [CBMC](https://github.com/parttimenerd/cbmc)
(with the `RELATIONS` environment variable set).
It creates a relations graph (which should be equivalent to a bit dependency graph) to compute the leakage of
the program related to the output.

Build it via `./build.sh` or `./build_debug.sh`.

```sh
> ./build/relationscutter --help
A tool to calculate the leakage for the __rel__ output of a modified CBMC
Usage:
  RelationsCutter [OPTION...] cnf file, uses stdin if omitted

  -i, --input arg   input variable prefixes (default: symex::nondet)
  -o, --output arg  output variable prefixes (default: "")
  -m, --method arg  output method, that the output variables have to be 
                    related to (default: main)
  -v, --verbose     enable some debug output
  -d, --debug       enable all debug output
  -h, --help        Print usage

```

[dsharpy](https://github.com/parttimenerd/dsharpy) is a tool that combines the relationscutter with
the modified CBMC into a usable analysis tool (with some preprocessing).

License
-------
MIT
