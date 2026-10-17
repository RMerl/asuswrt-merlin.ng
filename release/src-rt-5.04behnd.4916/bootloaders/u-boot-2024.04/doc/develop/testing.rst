.. SPDX-License-Identifier: GPL-2.0+

Introduction to testing
=======================

U-Boot has a large amount of code. This file describes how this code is
tested and what tests you should write when adding a new feature.


Running tests
-------------

To run most tests on sandbox, type this::

    make check

in the U-Boot directory. Note that only the pytest suite is run using this
command.

Some tests take ages to run and are marked with @pytest.mark.slow. To run just
the quick ones, type this::

    make qcheck

It is also possible to run just the tests for tools (patman, binman, etc.).
Such tests are included with those tools, i.e. no actual U-Boot unit tests are
run. Type this::

    make tcheck

You can also run a selection tests in parallel with::

    make pcheck

All of the above use the test/run script with a paremeter to select which tests
are run. See :doc:`py_testing` for more information.


Sandbox
-------
U-Boot can be built as a user-space application (e.g. for Linux). This
allows test to be executed without needing target hardware. The 'sandbox'
target provides this feature and it is widely used in tests.

See :doc:`tests_sandbox` for more information.

Pytest Suite
------------

Many tests are available using the pytest suite, in test/py. This can run
either on sandbox or on real hardware. It relies on the U-Boot console to
inject test commands and check the result. It is slower to run than C code,
but provides the ability to unify lots of tests and summarise their results.

You can run the tests on sandbox with::

   ./test/py/test.py --bd sandbox --build

This will produce HTML output in build-sandbox/test-log.html

Some tests run with other versions of sandbox. For example sandbox_flattree
runs the tests with livetree (the hierachical devicetree) disabled. You can
also select particular tests with -k::

   ./test/py/test.py --bd sandbox_flattree --build -k hello

There are some special tests that run in SPL. For this you need the sandbox_spl
build::

   ./test/py/test.py --bd sandbox_spl --build -k test_spl

See test/py/README.md for more information about the pytest suite.

See :doc:`tests_sandbox` for how to run tests directly (not through pytest).


tbot
----

Tbot provides a way to execute tests on target hardware. It is intended for
trying out both U-Boot and Linux (and potentially other software) on a
number of boards automatically. It can be used to create a continuous test
environment. See http://www.tbot.tools for more information.


Ad-hoc tests
------------

There are several ad-hoc tests which run outside the pytest environment:

test/fs
   File system test (shell script)
test/image
   FIT and legacy image tests (shell script and Python)
test/stdint
   A test that stdint.h can be used in U-Boot (shell script)
trace
   Test for the tracing feature (shell script)

TODO: Move these into pytest.


When to write tests
-------------------

If you add code to U-Boot without a test you are taking a risk. Even if you
perform thorough manual testing at the time of submission, it may break when
future changes are made to U-Boot. It may even break when applied to mainline,
if other changes interact with it. A good mindset is that untested code
probably doesn't work and should be deleted.

You can assume that the Pytest suite will be run before patches are accepted
to mainline, so this provides protection against future breakage.

On the other hand there is quite a bit of code that is not covered with tests,
or is covered sparingly. So here are some suggestions:

- If you are adding a new uclass, add a sandbox driver and a test that uses it
- If you are modifying code covered by an existing test, add a new test case
  to cover your changes
- If the code you are modifying has not tests, consider writing one. Even a
  very basic test is useful, and may be picked up and enhanced by others. It
  is much easier to add onto a test - writing a new large test can seem
  daunting to most contributors.

See doc:`tests_writing` for how to write tests.


Future work
-----------

Converting existing shell scripts into pytest tests.
