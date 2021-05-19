# Writing tests for Tor: an incomplete guide

Tor uses a variety of testing frameworks and methodologies to try to
keep from introducing bugs.  The major ones are:

   1. Unit tests written in C and shipped with the Tor distribution.

   2. Integration tests written in Python 2 (>= 2.7) or Python 3
      (>= 3.1) and shipped with the Tor distribution.

   3. Integration tests written in Python and shipped with the Stem
      library.  Some of these use the Tor controller protocol.

   4. System tests written in Python and SH, and shipped with the
      Chutney package.  These work by running many instances of Tor
      locally, and sending traffic through them.

   5. The Shadow network simulator.

## How to run these tests

### The easy version

To run all the tests that come bundled with Tor, run `make check`.

To run the Stem tests as well, fetch stem from the git repository,
set `STEM_SOURCE_DIR` to the checkout, and run `make test-stem`.

To run the Chutney tests as well, fetch chutney from the git repository,
set `CHUTNEY_PATH` to the checkout, and run `make test-network`.

To run all of the above, run `make test-full`.

To run all of the above, plus tests that require a working connection to the
internet, run `make test-full-online`.

### Running particular subtests

The Tor unit tests are divided into separate programs and a couple of
bundled unit test programs.

Separate programs are easy.  For example, to run the memwipe tests in
isolation, you just run `./src/test/test-memwipe`.

To run tests within the unit test programs, you can specify the name
of the test.  The string ".." can be used as a wildcard at the end of the
test name.  For example, to run all the cell format tests, enter
`./src/test/test cellfmt/..`.

Many tests that need to mess with global state run in forked subprocesses in
order to keep from contaminating one another.  But when debugging a failing test,
you might want to run it without forking a subprocess.  To do so, use the
`--no-fork` option with a single test.  (If you specify it along with
multiple tests, they might interfere.)

You can turn on logging in the unit tests by passing one of `--debug`,
`--info`, `--notice`, or `--warn`.  By default only errors are displayed.

Unit tests are divided into `./src/test/test` and `./src/test/test-slow`.
The former are those that should finish in a few seconds; the latter tend to
take more time, and may include CPU-intensive operations, deliberate delays,
and stuff like that.

## Finding test coverage

Test coverage is a measurement of which lines your tests actually visit.

When you configure Tor with the `--enable-coverage` option, it should
build with support for coverage in the unit tests, and in a special
`tor-cov` binary.

Then, run the tests you'd like to see coverage from.  If you have old
coverage output, you may need to run `reset-gcov` first.

Now you've got a bunch of files scattered around your build directories
called `*.gcda`.  In order to extract the coverage output from them, make a
temporary directory for them and run `./scripts/test/coverage ${TMPDIR}`,
where `${TMPDIR}` is the temporary directory you made.  This will create a
`.gcov` file for each source file under tests, containing that file's source
annotated with the number of times the tests hit each line.  (You'll need to
have gcov installed.)

You can get a summary of the test coverage for each file by running
`./scripts/test/cov-display ${TMPDIR}/*` .  Each line lists the file's name,
the number of uncovered lines, the number of uncovered lines, and the
coverage percentage.

For a summary of the test coverage for each _function_, run
`./scripts/test/cov-display -f ${TMPDIR}/*`.

For more details on using gcov, including the helper scripts in
scripts/test, see HelpfulTools.md.

### Comparing test coverage

Sometimes it's useful to compare test coverage for a branch you're writing to
coverage from another branch (such as git master, for example).  But you
can't run `diff` on the two coverage outputs directly, since the actual
number of times each line is executed aren't so important, and aren't wholly
deterministic.

Instead, follow the instructions above for each branch, creating a separate
temporary directory for each.  Then, run `./scripts/test/cov-diff ${D1}
${D2}`, where D1 and D2 are the directories you want to compare.  This will
produce a diff of the two directories, with all lines normalized to be either
covered or uncovered.

To count new or modified uncovered lines in D2, you can run:

```console
$ ./scripts/test/cov-diff ${D1} ${D2}" | grep '^+ *\#' | wc -l
```

## Marking lines as unreachable by tests

You can mark a specific line as unreachable by using the special
string LCOV_EXCL_LINE.  You can mark a range of lines as unreachable
with LCOV_EXCL_START... LCOV_EXCL_STOP.  Note that older versions of
lcov don't understand these lines.

You can post-process .gcov files to make these lines 'unreached' by
running ./scripts/test/cov-exclude on them.  It marks excluded
unreached lines with 'x', and excluded reached lines with '!!!'.

Note: you should never do this unless the line is meant to 100%
unreachable by actual code.

## What kinds of test should I write?

Integration testing and unit testing are complementary: it's probably a
good idea to make sure that your code is hit by both if you can.

If your code is very-low level, and its behavior is easily described in
terms of a relation between inputs and outputs, or a set of state
transitions, then it's a natural fit for unit tests.  (If not, please
consider refactoring it until most of it _is_ a good fit for unit
tests!)

If your code adds new externally visible functionality to Tor, it would
be great to have a test for that functionality.  That's where
integration tests more usually come in.

## Unit and regression tests: Does this function do what it's supposed to?

Most of Tor's unit tests are made using the "tinytest" testing framework.
You can see a guide to using it in the tinytest manual at

    https://github.com/nmathewson/tinytest/blob/master/tinytest-manual.md

To add a new test of this kind, either edit an existing C file in `src/test/`,
or create a new C file there.  Each test is a single function that must
be indexed in the table at the end of the file.  We use the label "done:" as
a cleanup point for all test functions.

If you have created a new test file, you will need to:
1. Add the new test file to include.am
2. In `test.h`, include the new test cases (testcase_t)
3. In `test.c`, add the new test cases to testgroup_t testgroups

(Make sure you read `tinytest-manual.md` before proceeding.)

I use the term "unit test" and "regression tests" very sloppily here.

## A simple example

Here's an example of a test function for a simple function in util.c:

```c
static void
test_util_writepid(void *arg)
{
    (void) arg;

    char *contents = NULL;
    const char *fname = get_fname("tmp_pid");
    unsigned long pid;
    char c;

    write_pidfile(fname);

    contents = read_file_to_str(fname, 0, NULL);
    tt_assert(contents);

    int n = sscanf(contents, "%lu\n%c", &pid, &c);
    tt_int_op(n, OP_EQ, 1);
    tt_int_op(pid, OP_EQ, getpid());

done:
    tor_free(contents);
}
```

This should look pretty familiar to you if you've read the tinytest
manual.  One thing to note here is that we use the testing-specific
function `get_fname` to generate a file with respect to a temporary
directory that the tests use.  You don't need to delete the file;
it will get removed when the tests are done.

Also note our use of `OP_EQ` instead of `==` in the `tt_int_op()` calls.
We define `OP_*` macros to use instead of the binary comparison
operators so that analysis tools can more easily parse our code.
(Coccinelle really hates to see `==` used as a macro argument.)

Finally, remember that by convention, all `*_free()` functions that
Tor defines are defined to accept NULL harmlessly.  Thus, you don't
need to say `if (contents)` in the cleanup block.

## Exposing static functions for testing

Sometimes you need to test a function, but you don't want to expose
it outside its usual module.

To support this, Tor's build system compiles a testing version of
each module, with extra identifiers exposed.  If you want to
declare a function as static but available for testing, use the
macro `STATIC` instead of `static`.  Then, make sure there's a
macro-protected declaration of the function in the module's header.

For example, `crypto_curve25519.h` contains:

```c
#ifdef CRYPTO_CURVE25519_PRIVATE
STATIC int curve25519_impl(uint8_t *output, const uint8_t *secret,
        const uint8_t *basepoint);
#endif
```

The `crypto_curve25519.c` file and the `test_crypto.c` file both define
`CRYPTO_CURVE25519_PRIVATE`, so they can see this declaration.

## STOP!  Does this test really test?

When writing tests, it's not enough to just generate coverage on all the
lines of the code that you're testing:  It's important to make sure that
the test _really tests_ the code.

For example, here is a _bad_ test for the unlink() function (which is
supposed to remove a file).

```c
static void
test_unlink_badly(void *arg)
{
    (void) arg;
    int r;

    const char *fname = get_fname("tmpfile");

    /* If the file isn't there, unlink returns -1 and sets ENOENT */
    r = unlink(fname);
    tt_int_op(n, OP_EQ, -1);
    tt_int_op(errno, OP_EQ, ENOENT);

    /* If the file DOES exist, unlink returns 0. */
    write_str_to_file(fname, "hello world", 0);
    r = unlink(fnme);
    tt_int_op(r, OP_EQ, 0);

done:
    tor_free(contents);
}
```

This test might get very high coverage on unlink().  So why is it a
bad test? Because it doesn't check that unlink() *actually removes the
named file*!

Remember, the purpose of a test is to succeed if the code does what
it's supposed to do, and fail otherwise.  Try to design your tests so
that they check for the code's intended and documented functionality
as much as possible.

## Mock functions for testing in isolation

Often we want to test that a function works right, but the function to
be tested depends on other functions whose behavior is hard to observe,
or which require a working Tor network, or something like that.

To write tests for this case, you can replace the underlying functions
with testing stubs while your unit test is running.  You need to declare
the underlying function as 'mockable', as follows:

```c
MOCK_DECL(returntype, functionname, (argument list));
```

and then later implement it as:

```c
MOCK_IMPL(returntype, functionname, (argument list))
{
    /* implementation here */
}
```

For example, if you had a 'connect to remote server' function, you could
declare it as:

```c
MOCK_DECL(int, connect_to_remote, (const char *name, status_t *status));
```

When you declare a function this way, it will be declared as normal in
regular builds, but when the module is built for testing, it is declared
as a function pointer initialized to the actual implementation.

In your tests, if you want to override the function with a temporary
replacement, you say:

```c
MOCK(functionname, replacement_function_name);
```

And later, you can restore the original function with:

```c
UNMOCK(functionname);
```

For more information, see the definitions of this mocking logic in
`testsupport.h`.

## Okay but what should my tests actually do?

We talk above about "test coverage" -- making sure that your tests visit
every line of code, or every branch of code.  But visiting the code isn't
enough: we want to verify that it's correct.

So when writing tests, try to make tests that should pass with any correct
implementation of the code, and that should fail if the code doesn't do what
it's supposed to do.

You can write "black-box" tests or "glass-box" tests.  A black-box test is
one that you write without looking at the structure of the function.  A
glass-box one is one you implement while looking at how the function is
implemented.

In either case, make sure to consider common cases *and* edge cases; success
cases and failure csaes.

For example, consider testing this function:

```c
/** Remove all elements E from sl such that E==element.  Preserve
 * the order of any elements before E, but elements after E can be
 * rearranged.
 */
void smartlist_remove(smartlist_t *sl, const void *element);
```

In order to test it well, you should write tests for at least all of the
following cases.  (These would be black-box tests, since we're only looking
at the declared behavior for the function:

   * Remove an element that is in the smartlist.
   * Remove an element that is not in the smartlist.
   * Remove an element that appears in the smartlist more than once.

And your tests should verify that it behaves correct.  At minimum, you should
test:

   * That other elements before E are in the same order after you call the
     functions.
   * That the target element is really removed.
   * That _only_ the target element is removed.

When you consider edge cases, you might try:

   * Remove an element from an empty list.
   * Remove an element from a singleton list containing that element.
   * Remove an element for a list containing several instances of that
     element, and nothing else.

Now let's look at the implementation:

```c
void
smartlist_remove(smartlist_t *sl, const void *element)
{
    int i;
    if (element == NULL)
        return;
    for (i=0; i < sl->num_used; i++)
        if (sl->list[i] == element) {
            sl->list[i] = sl->list[--sl->num_used]; /* swap with the end */
            i--; /* so we process the new i'th element */
            sl->list[sl->num_used] = NULL;
        }
}
```

Based on the implementation, we now see three more edge cases to test:

   * Removing NULL from the list.
   * Removing an element from the end of the list
   * Removing an element from a position other than the end of the list.

## What should my tests NOT do?

Tests shouldn't require a network connection.

Whenever possible, tests shouldn't take more than a second.  Put the test
into test/slow if it genuinely needs to be run.

Tests should not alter global state unless they run with `TT_FORK`: Tests
should not require other tests to be run before or after them.

Tests should not leak memory or other resources.  To find out if your tests
are leaking memory, run them under valgrind (see HelpfulTools.txt for more
information on how to do that).

When possible, tests should not be over-fit to the implementation.  That is,
the test should verify that the documented behavior is implemented, but
should not break if other permissible behavior is later implemented.

## Advanced techniques: Namespaces

Sometimes, when you're doing a lot of mocking at once, it's convenient to
isolate your identifiers within a single namespace.  If this were C++, we'd
already have namespaces, but for C, we do the best we can with macros and
token-pasting.

We have some macros defined for this purpose in `src/test/test.h`.  To use
them, you define `NS_MODULE` to a prefix to be used for your identifiers, and
then use other macros in place of identifier names.  See `src/test/test.h` for
more documentation.

## Integration tests: Calling Tor from the outside

Some tests need to invoke Tor from the outside, and shouldn't run from the
same process as the Tor test program.  Reasons for doing this might include:

   * Testing the actual behavior of Tor when run from the command line
   * Testing that a crash-handler correctly logs a stack trace
   * Verifying that violating a sandbox or capability requirement will
     actually crash the program.
   * Needing to run as root in order to test capability inheritance or
     user switching.

To add one of these, you generally want a new C program in `src/test`.  Add it
to `TESTS` and `noinst_PROGRAMS` if it can run on its own and return success or
failure.  If it needs to be invoked multiple times, or it needs to be
wrapped, add a new shell script to `TESTS`, and the new program to
`noinst_PROGRAMS`.  If you need access to any environment variable from the
makefile (eg `${PYTHON}` for a python interpreter), then make sure that the
makefile exports them.

## Writing integration tests with Stem

The 'stem' library includes extensive tests for the Tor controller protocol.
You can run stem tests from tor with `make test-stem`, or see
`https://stem.torproject.org/faq.html#how-do-i-run-the-tests`.

To see what tests are available, have a look around the `test/*` directory in
stem. The first thing you'll notice is that there are both `unit` and `integ`
tests. The former are for tests of the facilities provided by stem itself that
can be tested on their own, without the need to hook up a tor process. These
are less relevant, unless you want to develop a new stem feature. The latter,
however, are a very useful tool to write tests for controller features. They
provide a default environment with a connected tor instance that can be
modified and queried. Adding more integration tests is a great way to increase
the test coverage inside Tor, especially for controller features.

Let's assume you actually want to write a test for a previously untested
controller feature. I'm picking the `exit-policy/*` GETINFO queries. Since
these are a controller feature that we want to write an integration test for,
the right file to modify is
`https://gitweb.torproject.org/stem.git/tree/test/integ/control/controller.py`.

First off we notice that there is an integration test called
`test_get_exit_policy()` that's already written. This exercises the interaction
of stem's `Controller.get_exit_policy()` method, and is not relevant for our
test since there are no stem methods to make use of all `exit-policy/*`
queries (if there were, likely they'd be tested already. Maybe you want to
write a stem feature, but I chose to just add tests).

Our test requires a tor controller connection, so we'll use the
`@require_controller` annotation for our `test_exit_policy()` method. We need a
controller instance, which we get from
`test.runner.get_runner().get_tor_controller()`. The attached Tor instance is
configured as a client, but the exit-policy GETINFO queries need a relay to
work, so we have to change the config (using `controller.set_options()`). This
is OK for us to do, we just have to remember to set DisableNetwork so we don't
actually start an exit relay and also to undo the changes we made (by calling
`controller.reset_conf()` at the end of our test). Additionally, we have to
configure a static Address for Tor to use, because it refuses to build a
descriptor when it can't guess a suitable IP address. Unfortunately, these
kinds of tripwires are everywhere. Don't forget to file appropriate tickets if
you notice any strange behaviour that seems totally unreasonable.

Check out the `test_exit_policy()` function in abovementioned file to see the
final implementation for this test.

## System testing with Chutney

The 'chutney' program configures and launches a set of Tor relays,
authorities, and clients on your local host.  It has a `test network`
functionality to send traffic through them and verify that the traffic
arrives correctly.

You can write new test networks by adding them to `networks`. To add
them to Tor's tests, add them to the `test-network` or `test-network-all`
targets in `Makefile.am`.

(Adding new kinds of program to chutney will still require hacking the
code.)

## Other integration tests

It's fine to write tests that use a POSIX shell to invoke Tor or test other
aspects of the system.  When you do this, have a look at our existing tests
of this kind in `src/test/` to make sure that you haven't forgotten anything
important.  For example: it can be tricky to make sure you're invoking Tor at
the right path in various build scenarios.

We use a POSIX shell whenever possible here, and we use the shellcheck tool
to make sure that our scripts portable.  We should only require bash for
scripts that are developer-only.
