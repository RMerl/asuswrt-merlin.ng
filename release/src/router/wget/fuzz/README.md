# Fuzzers

These are fuzzers designed for use with `libFuzzer` or `afl`. They can
be used to run on Google's OSS-Fuzz (https://github.com/google/oss-fuzz/).

The convention used here is that the initial values for each parser fuzzer
are taken from the $NAME.in directory.

Crash reproducers from OSS-Fuzz are put into $NAME.repro directory for
regression testing with top dir 'make check' or 'make check-valgrind'.


# Running a fuzzer using clang

Use the following commands on top dir:
```
export CC=clang
# address sanitizer:
#export CFLAGS="-O1 -g -fno-omit-frame-pointer -gline-tables-only -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION -fsanitize=undefined,integer,nullability -fsanitize=address -fsanitize-address-use-after-scope -fsanitize-coverage=trace-pc-guard,trace-cmp"
export CFLAGS="-O1 -fno-omit-frame-pointer -gline-tables-only -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION -fsanitize=undefined -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=fuzzer-no-link"
# undefined sanitizer;
export CFLAGS="-O1 -fno-omit-frame-pointer -gline-tables-only -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION -fsanitize=bool,array-bounds,float-divide-by-zero,function,integer-divide-by-zero,return,shift,signed-integer-overflow,vla-bound,vptr -fno-sanitize-recover=bool,array-bounds,float-divide-by-zero,function,integer-divide-by-zero,return,shift,signed-integer-overflow,vla-bound,vptr -fsanitize=fuzzer-no-link"
export LIB_FUZZING_ENGINE="-lFuzzer  -lstdc++"
./configure --enable-fuzzing --without-metalink --without-zlib --disable-pcre --without-libuuid --enable-assert
make clean
make -j$(nproc)
cd fuzz

# run wget_options_fuzzer
UBSAN_OPTIONS=print_stacktrace=1 ASAN_SYMBOLIZER_PATH=/usr/bin/llvm-symbolizer \
  ./run-clang.sh wget_options_fuzzer
```

If you see a crash, then a crash corpora is written that can be used for further
investigation. E.g.
```
==2410==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000004e90 at pc 0x00000049cf9c bp 0x7fffb5543f70 sp 0x7fffb5543720
...
Test unit written to ./crash-adc83b19e793491b1c6ea0fd8b46cd9f32e592fc
```

To reproduce the crash:
```
./wget_options_fuzzer < ./crash-adc83b19e793491b1c6ea0fd8b46cd9f32e592fc
```

You can also copy/move that file into wget_options_fuzzer.repro/
and re-build the project without fuzzing for a valgrind run, if you like that better.
Just a `./configure` and a `make check-valgrind` should reproduce it.


# Running a fuzzer using AFL

Use the following commands on top dir:

```
$ export LIB_FUZZING_ENGINE=""
$ CC=afl-clang-fast ./configure --enable-fuzzing
$ make -j$(nproc) clean all
$ cd fuzz
$ ./run-afl.sh wget_options_fuzzer
```

# Fuzz code coverage using the corpus directories *.in/

Code coverage reports currently work best with gcc+lcov+genhtml.

In the top directory:
```
CC=gcc CFLAGS="-O0 -g" ./configure
make fuzz-coverage
xdg-open lcov/index.html
```

To work on corpora for better coverage, `cd fuzz` and use e.g.
`./view-coverage.sh wget_options_fuzzer`.


# Creating wget_options_fuzzer.dict

```
for i in `../src/wget --help|tr ' ' '\n'|grep ^--|cut -c 3-|sort`;do echo \"$i\"; done >wget_options_fuzzer.dict
```
