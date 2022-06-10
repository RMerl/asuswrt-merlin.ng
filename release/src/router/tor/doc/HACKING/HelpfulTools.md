# Useful tools

These aren't strictly necessary for hacking on Tor, but they can help track
down bugs.

## Travis/Appveyor CI

It's CI.

Looks like this:
* https://travis-ci.org/torproject/tor
* https://ci.appveyor.com/project/torproject/tor

Travis builds and runs tests on Linux, and eventually macOS (#24629).
Appveyor builds and runs tests on Windows (using Windows Services for Linux).

Runs automatically on Pull Requests sent to torproject/tor. You can set it up
for your fork to build commits outside of PRs too:

1. sign up for GitHub: https://github.com/join
2. fork https://github.com/torproject/tor:
   https://help.github.com/articles/fork-a-repo/
3. follow https://docs.travis-ci.com/user/getting-started/#To-get-started-with-Travis-CI.
   skip steps involving `.travis.yml` (we already have one).
4. go to https://ci.appveyor.com/login , log in with github, and select
   "NEW PROJECT"

Builds should show up on the web at travis-ci.com and on IRC at #tor-ci on
OFTC. If they don't, ask #tor-dev (also on OFTC).

## Jenkins

It's CI/builders. Looks like this: https://jenkins.torproject.org

Runs automatically on commits merged to git.torproject.org. We CI the
main branch and all supported tor versions. We also build nightly debian
packages from main.

Builds Linux and Windows cross-compilation. Runs Linux tests.

Builds should show up on the web at jenkins.torproject.org and on IRC at
#tor-bots on OFTC. If they don't, ask #tor-dev (also on OFTC).

## Valgrind

```console
$ valgrind --leak-check=yes --error-limit=no --show-reachable=yes src/app/tor
```

(Note that if you get a zillion openssl warnings, you will also need to
pass `--undef-value-errors=no` to valgrind, or rebuild your openssl
with `-DPURIFY`.)

## Coverity

Nick regularly runs the coverity static analyzer on the Tor codebase.

The preprocessor define `__COVERITY__` is used to work around instances
where coverity picks up behavior that we wish to permit.

## clang Static Analyzer

The clang static analyzer can be run on the Tor codebase using Xcode (WIP)
or a command-line build.

The preprocessor define `__clang_analyzer__` is used to work around instances
where clang picks up behavior that we wish to permit.

## clang Runtime Sanitizers

To build the Tor codebase with the clang Address and Undefined Behavior
sanitizers, see the file `contrib/clang/sanitize_blacklist.txt`.

Preprocessor workarounds for instances where clang picks up behavior that
we wish to permit are also documented in the blacklist file.

## Running lcov for unit test coverage

Lcov is a utility that generates pretty HTML reports of test code coverage.
To generate such a report:

```console
$ ./configure --enable-coverage
$ make
$ make coverage-html
$ $BROWSER ./coverage_html/index.html
```

This will run the tor unit test suite `./src/test/test` and generate the HTML
coverage code report under the directory `./coverage_html/`. To change the
output directory, use `make coverage-html HTML_COVER_DIR=./funky_new_cov_dir`.

Coverage diffs using lcov are not currently implemented, but are being
investigated (as of July 2014).

## Running the unit tests

To quickly run all the tests distributed with Tor:

```console
$ make check
```

To run the fast unit tests only:

```console
$ make test
```

To selectively run just some tests (the following can be combined
arbitrarily):

```console
$ ./src/test/test <name_of_test> [<name of test 2>] ...
$ ./src/test/test <prefix_of_name_of_test>.. [<prefix_of_name_of_test2>..] ...
$ ./src/test/test :<name_of_excluded_test> [:<name_of_excluded_test2]...
```

To run all tests, including those based on Stem or Chutney:

```console
$ make test-full
```

To run all tests, including those based on Stem or Chutney that require a
working connection to the internet:

```console
$ make test-full-online
```

## Running gcov for unit test coverage

```console
$ ./configure --enable-coverage
$ make
$ make check
$ # or--- make test-full ? make test-full-online?
$ mkdir coverage-output
$ ./scripts/test/coverage coverage-output
```

(On OSX, you'll need to start with `--enable-coverage CC=clang`.)

If that doesn't work:

   * Try configuring Tor with `--disable-gcc-hardening`
   * You might need to run `make clean` after you run `./configure`.

Then, look at the .gcov files in `coverage-output`.  '-' before a line means
that the compiler generated no code for that line.  '######' means that the
line was never reached.  Lines with numbers were called that number of times.

For more details about how to read gcov output, see the [Invoking
gcov](https://gcc.gnu.org/onlinedocs/gcc/Invoking-Gcov.html) chapter
of the GCC manual.

If you make changes to Tor and want to get another set of coverage results,
you can run `make reset-gcov` to clear the intermediary gcov output.

If you have two different `coverage-output` directories, and you want to see
a meaningful diff between them, you can run:

```console
$ ./scripts/test/cov-diff coverage-output1 coverage-output2 | less
```

In this diff, any lines that were visited at least once will have coverage "1",
and line numbers are deleted.  This lets you inspect what you (probably) really
want to know: which untested lines were changed?  Are there any new untested
lines?

If you run ./scripts/test/cov-exclude, it marks excluded unreached
lines with 'x', and excluded reached lines with '!!!'.

## Running integration tests

We have the beginnings of a set of scripts to run integration tests using
Chutney. To try them, set CHUTNEY_PATH to your chutney source directory, and
run `make test-network`.

We also have scripts to run integration tests using Stem.  To try them, set
`STEM_SOURCE_DIR` to your Stem source directory, and run `test-stem`.

## Profiling Tor

Ongoing notes about Tor profiling can be found at
https://pad.riseup.net/p/profiling-tor

## Profiling Tor with oprofile

The oprofile tool runs (on Linux only!) to tell you what functions Tor is
spending its CPU time in, so we can identify performance bottlenecks.

Here are some basic instructions

 - Build tor with debugging symbols (you probably already have, unless
   you messed with CFLAGS during the build process).
 - Build all the libraries you care about with debugging symbols
   (probably you only care about libssl, maybe zlib and Libevent).
 - Copy this tor to a new directory
 - Copy all the libraries it uses to that dir too (`ldd ./tor` will
   tell you)
 - Set LD_LIBRARY_PATH to include that dir.  `ldd ./tor` should now
   show you it's using the libs in that dir
 - Run that tor
 - Reset oprofiles counters/start it
   * `opcontrol --reset; opcontrol --start`, if Nick remembers right.
 - After a while, have it dump the stats on tor and all the libs
   in that dir you created.
   * `opcontrol --dump;`
   * `opreport -l that_dir/*`
 - Profit

## Profiling Tor with perf

This works with a running Tor, and requires root.

1. Decide how long you want to profile for. Start with (say) 30 seconds. If that
   works, try again with longer times.

2. Find the PID of your running tor process.

3. Run `perf record --call-graph dwarf -p <PID> sleep <SECONDS>`

   (You may need to do this as root.)

   You might need to add `-e cpu-clock` as an option to the perf record line
   above, if you are on an older CPU without access to hardware profiling
   events, or in a VM, or something.

4. Now you have a perf.data file. Have a look at it with `perf report
   --no-children --sort symbol,dso` or `perf report --no-children --sort
   symbol,dso --stdio --header`. How does it look?

5a. Once you have a nice big perf.data file, you can compress it, encrypt it,
    and send it to your favorite Tor developers.

5b. Or maybe you'd rather not send a nice big perf.data file. Who knows what's
    in that!? It's kinda scary. To generate a less scary file, you can use `perf
    report -g > <FILENAME>.out`. Then you can compress that and put it somewhere
    public.

## Profiling Tor with gperftools aka Google-performance-tools

This should work on nearly any unixy system. It doesn't seem to be compatible
with RunAsDaemon though.

Beforehand, install google-perftools.

1. You need to rebuild Tor, hack the linking steps to add `-lprofiler` to the
   libs. You can do this by adding `LIBS=-lprofiler` when you call `./configure`.

Now you can run Tor with profiling enabled, and use the pprof utility to look at
performance! See the gperftools manual for more info, but basically:

2. Run `env CPUPROFILE=/tmp/profile src/app/tor -f <path/torrc>`. The profile file
   is not written to until Tor finishes execution.

3. Run `pprof src/app/tor /tmp/profile` to start the REPL.

## Generating and analyzing a callgraph

0. Build Tor on linux or mac, ideally with -O0 or -fno-inline.

1. Clone 'https://git.torproject.org/user/nickm/calltool.git/' .
   Follow the README in that repository.

Note that currently the callgraph generator can't detect calls that pass
through function pointers.

## Getting emacs to edit Tor source properly

Nick likes to put the following snippet in his .emacs file:


    (add-hook 'c-mode-hook
          (lambda ()
            (font-lock-mode 1)
            (set-variable 'show-trailing-whitespace t)

            (let ((fname (expand-file-name (buffer-file-name))))
              (cond
               ((string-match "^/home/nickm/src/libevent" fname)
                (set-variable 'indent-tabs-mode t)
                (set-variable 'c-basic-offset 4)
                (set-variable 'tab-width 4))
               ((string-match "^/home/nickm/src/tor" fname)
                (set-variable 'indent-tabs-mode nil)
                (set-variable 'c-basic-offset 2))
               ((string-match "^/home/nickm/src/openssl" fname)
                (set-variable 'indent-tabs-mode t)
                (set-variable 'c-basic-offset 8)
                (set-variable 'tab-width 8))
            ))))


You'll note that it defaults to showing all trailing whitespace.  The `cond`
test detects whether the file is one of a few C free software projects that I
often edit, and sets up the indentation level and tab preferences to match
what they want.

If you want to try this out, you'll need to change the filename regex
patterns to match where you keep your Tor files.

If you use emacs for editing Tor and nothing else, you could always just say:


    (add-hook 'c-mode-hook
        (lambda ()
            (font-lock-mode 1)
            (set-variable 'show-trailing-whitespace t)
            (set-variable 'indent-tabs-mode nil)
            (set-variable 'c-basic-offset 2)))


There is probably a better way to do this.  No, we are probably not going
to clutter the files with emacs stuff.

## Building a tag file (code index)

Many functions in tor use `MOCK_IMPL` wrappers for unit tests. Your
tag-building program must be told how to handle this syntax.

If you're using emacs, you can generate an emacs-compatible tag file using
`make tags`. This will run your system's `etags`. Tor's build system assumes
that you're using the emacs-specific version of `etags` (bundled under the
`xemacs21-bin` package on Debian). This is incompatible with other versions of
`etags` such as the version provided by Exuberant Ctags.

If you're using vim or emacs, you can also use Universal Ctags to build a tag
file using the syntax:

```console
$ ctags -R -D 'MOCK_IMPL(r,h,a)=r h a' .
```

If you're using an older version of Universal Ctags, you can use the following
instead:

```console
ctags -R --mline-regex-c='/MOCK_IMPL\([^,]+,\W*([a-zA-Z0-9_]+)\W*,/\1/f/{mgroup=1}' .
```

A vim-compatible tag file will be generated by default. If you use emacs, add
the `-e` flag to generate an emacs-compatible tag file.

## Doxygen

We use the 'doxygen' utility to generate documentation from our
source code. Here's how to use it:

  1. Begin every file that should be documented with

```
 /**
  * \file filename.c
  * \brief Short description of the file.
  */
```

  (Doxygen will recognize any comment beginning with /** as special.)

  2. Before any function, structure, #define, or variable you want to
     document, add a comment of the form:

```
/** Describe the function's actions in imperative sentences.
 *
 * Use blank lines for paragraph breaks
 *   - and
 *   - hyphens
 *   - for
 *   - lists.
 *
 * Write <b>argument_names</b> in boldface.
 *
 * \code
 *     place_example_code();
 *     between_code_and_endcode_commands();
 * \endcode
 */
```

  3. Make sure to escape the characters `<`, `>`, `\`, `%` and `#` as `\<`,
     `\>`, `\\`, `\%` and `\#`.

  4. To document structure members, you can use two forms:

```c
struct foo {
  /** You can put the comment before an element; */
  int a;
  int b; /**< Or use the less-than symbol to put the comment
         * after the element. */
};
```

  5. To generate documentation from the Tor source code, type:

```console
$ doxygen -g
```

  to generate a file called `Doxyfile`.  Edit that file and run
  `doxygen` to generate the API documentation.

  6. See the Doxygen manual for more information; this summary just
     scratches the surface.

## Style and best-practices checking

We use scripts to check for various problems in the formatting and style
of our source code.  The "check-spaces" test detects a bunch of violations
of our coding style on the local level.  The "check-best-practices" test
looks for violations of some of our complexity guidelines.

You can tell the tool about exceptions to the complexity guidelines via its
exceptions file (scripts/maint/practracker/exceptions.txt).  But before you
do this, consider whether you shouldn't fix the underlying problem.  Maybe
that file really _is_ too big.  Maybe that function really _is_ doing too
much.  (On the other hand, for stable release series, it is sometimes better
to leave things unrefactored.)
