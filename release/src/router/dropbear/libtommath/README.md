[![Build Status - master](https://travis-ci.org/libtom/libtommath.png?branch=master)](https://travis-ci.org/libtom/libtommath)

[![Build Status - develop](https://travis-ci.org/libtom/libtommath.png?branch=develop)](https://travis-ci.org/libtom/libtommath)

This is the git repository for [LibTomMath](http://www.libtom.org/), a free open source portable number theoretic multiple-precision integer (MPI) library written entirely in C.

The `develop` branch contains the in-development version. Stable releases are tagged.

Documentation is built from the LaTeX file `bn.tex`. There is also limited documentation in `tommath.h`. There is also a document, `tommath.pdf`, which describes the goals of the project and many of the algorithms used.

The project can be build by using `make`. Along with the usual `make`, `make clean` and `make install`, there are several other build targets, see the makefile for details. There are also makefiles for certain specific platforms.

Tests are located in `demo/` and can be built in two flavors.
* `make test` creates a test binary that is intended to be run against `mtest`. `mtest` can be built with `make mtest` and test execution is done like `./mtest/mtest | ./test`. `mtest` is creating test vectors using an alternative MPI library and `test` is consuming these vectors to verify correct behavior of ltm
* `make test_standalone` creates a stand-alone test binary that executes several test routines.
