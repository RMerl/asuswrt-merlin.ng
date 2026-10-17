.. SPDX-License-Identifier: GPL-2.0+
.. Copyright 2021 Google LLC
.. sectionauthor:: Simon Glass <sjg@chromium.org>

Writing Tests
=============

This describes how to write tests in U-Boot and describes the possible options.

Test types
----------

There are two basic types of test in U-Boot:

  - Python tests, in test/py/tests
  - C tests, in test/ and its subdirectories

(there are also UEFI tests in lib/efi_selftest/ not considered here.)

Python tests talk to U-Boot via the command line. They support both sandbox and
real hardware. They typically do not require building test code into U-Boot
itself. They are fairly slow to run, due to the command-line interface and there
being two separate processes. Python tests are fairly easy to write. They can
be a little tricky to debug sometimes due to the voluminous output of pytest.

C tests are written directly in U-Boot. While they can be used on boards, they
are more commonly used with sandbox, as they obviously add to U-Boot code size.
C tests are easy to write so long as the required facilities exist. Where they
do not it can involve refactoring or adding new features to sandbox. They are
fast to run and easy to debug.

Regardless of which test type is used, all tests are collected and run by the
pytest framework, so there is typically no need to run them separately. This
means that C tests can be used when it makes sense, and Python tests when it
doesn't.


This table shows how to decide whether to write a C or Python test:

=====================  ===========================  =============================
Attribute              C test                       Python test
=====================  ===========================  =============================
Fast to run?           Yes                          No (two separate processes)
Easy to write?         Yes, if required test        Yes
                       features exist in sandbox
                       or the target system
Needs code in U-Boot?  Yes                          No, provided the test can be
                                                    executed and the result
                                                    determined using the command
                                                    line
Easy to debug?         Yes                          No, since access to the U-Boot
                                                    state is not available and the
                                                    amount of output can
                                                    sometimes require a bit of
                                                    digging
Can use gdb?           Yes, directly                Yes, with --gdbserver
Can run on boards?     Some can, but only if        Some
                       compiled in and not
                       dependent on sandboxau
=====================  ===========================  =============================


Python or C
-----------

Typically in U-Boot we encourage C test using sandbox for all features. This
allows fast testing, easy development and allows contributors to make changes
without needing dozens of boards to test with.

When a test requires setup or interaction with the running host (such as to
generate images and then running U-Boot to check that they can be loaded), or
cannot be run on sandbox, Python tests should be used. These should typically
NOT rely on running with sandbox, but instead should function correctly on any
board supported by U-Boot.


Mixing Python and C
-------------------

The best of both worlds is sometimes to have a Python test set things up and
perform some operations, with a 'checker' C unit test doing the checks
afterwards. This can be achieved with these steps:

- Add the `UT_TESTF_MANUAL` flag to the checker test so that the `ut` command
  does not run it by default
- Add a `_norun` suffix to the name so that pytest knows to skip it too

In your Python test use the `-f` flag to the `ut` command to force the checker
test to run it, e.g.::

   # Do the Python part
   host load ...
   bootm ...

   # Run the checker to make sure that everything worked
   ut -f bootstd vbe_test_fixup_norun

Note that apart from the `UT_TESTF_MANUAL` flag, the code in a 'manual' C test
is just like any other C test. It still uses ut_assert...() and other such
constructs, in this case to check that the expected things happened in the
Python test.


How slow are Python tests?
--------------------------

Under the hood, when running on sandbox, Python tests work by starting a sandbox
test and connecting to it via a pipe. Each interaction with the U-Boot process
requires at least a context switch to handle the pipe interaction. The test
sends a command to U-Boot, which then reacts and shows some output, then the
test sees that and continues. Of course on real hardware, communications delays
(e.g. with a serial console) make this slower.

For comparison, consider a test that checks the 'md' (memory dump). All times
below are approximate, as measured on an AMD 2950X system. Here is is the test
in Python::

   @pytest.mark.buildconfigspec('cmd_memory')
   def test_md(u_boot_console):
       """Test that md reads memory as expected, and that memory can be modified
       using the mw command."""

       ram_base = u_boot_utils.find_ram_base(u_boot_console)
       addr = '%08x' % ram_base
       val = 'a5f09876'
       expected_response = addr + ': ' + val
       u_boot_console.run_command('mw ' + addr + ' 0 10')
       response = u_boot_console.run_command('md ' + addr + ' 10')
       assert(not (expected_response in response))
       u_boot_console.run_command('mw ' + addr + ' ' + val)
       response = u_boot_console.run_command('md ' + addr + ' 10')
       assert(expected_response in response)

This runs a few commands and checks the output. Note that it runs a command,
waits for the response and then checks it agains what is expected. If run by
itself it takes around 800ms, including test collection. For 1000 runs it takes
19 seconds, or 19ms per run. Of course 1000 runs it not that useful since we
only want to run it once.

There is no exactly equivalent C test, but here is a similar one that tests 'ms'
(memory search)::

   /* Test 'ms' command with bytes */
   static int mem_test_ms_b(struct unit_test_state *uts)
   {
      u8 *buf;

      buf = map_sysmem(0, BUF_SIZE + 1);
      memset(buf, '\0', BUF_SIZE);
      buf[0x0] = 0x12;
      buf[0x31] = 0x12;
      buf[0xff] = 0x12;
      buf[0x100] = 0x12;
      ut_assertok(console_record_reset_enable());
      run_command("ms.b 1 ff 12", 0);
      ut_assert_nextline("00000030: 00 12 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................");
      ut_assert_nextline("--");
      ut_assert_nextline("000000f0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 12    ................");
      ut_assert_nextline("2 matches");
      ut_assert_console_end();

      ut_asserteq(2, env_get_hex("memmatches", 0));
      ut_asserteq(0xff, env_get_hex("memaddr", 0));
      ut_asserteq(0xfe, env_get_hex("mempos", 0));

      unmap_sysmem(buf);

      return 0;
   }
   MEM_TEST(mem_test_ms_b, UT_TESTF_CONSOLE_REC);

This runs the command directly in U-Boot, then checks the console output, also
directly in U-Boot. If run by itself this takes 100ms. For 1000 runs it takes
660ms, or 0.66ms per run.

So overall running a C test is perhaps 8 times faster individually and the
interactions are perhaps 25 times faster.

It should also be noted that the C test is fairly easy to debug. You can set a
breakpoint on do_mem_search(), which is what implements the 'ms' command,
single step to see what might be wrong, etc. That is also possible with the
pytest, but requires two terminals and --gdbserver.


Why does speed matter?
----------------------

Many development activities rely on running tests:

  - 'git bisect run make qcheck' can be used to find a failing commit
  - test-driven development relies on quick iteration of build/test
  - U-Boot's continuous integration (CI) systems make use of tests. Running
      all sandbox tests typically takes 90 seconds and running each qemu test
      takes about 30 seconds. This is currently dwarfed by the time taken to
      build all boards

As U-Boot continues to grow its feature set, fast and reliable tests are a
critical factor factor in developer productivity and happiness.


Writing C tests
---------------

C tests are arranged into suites which are typically executed by the 'ut'
command. Each suite is in its own file. This section describes how to accomplish
some common test tasks.

(there are also UEFI C tests in lib/efi_selftest/ not considered here.)

Add a new driver model test
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Use this when adding a test for a new or existing uclass, adding new operations
or features to a uclass, adding new ofnode or dev_read_() functions, or anything
else related to driver model.

Find a suitable place for your test, perhaps near other test functions in
existing code, or in a new file. Each uclass should have its own test file.

Declare the test with::

   /* Test that ... */
   static int dm_test_uclassname_what(struct unit_test_state *uts)
   {
      /* test code here */

      return 0;
   }
   DM_TEST(dm_test_uclassname_what, UT_TESTF_SCAN_FDT);

Replace 'uclassname' with the name of your uclass, if applicable. Replace 'what'
with what you are testing.

The flags for DM_TEST() are defined in test/test.h and you typically want
UT_TESTF_SCAN_FDT so that the devicetree is scanned and all devices are bound
and ready for use. The DM_TEST macro adds UT_TESTF_DM automatically so that
the test runner knows it is a driver model test.

Driver model tests are special in that the entire driver model state is
recreated anew for each test. This ensures that if a previous test deletes a
device, for example, it does not affect subsequent tests. Driver model tests
also run both with livetree and flattree, to ensure that both devicetree
implementations work as expected.

Example commit: c48cb7ebfb4 ("sandbox: add ADC unit tests") [1]

[1] https://gitlab.denx.de/u-boot/u-boot/-/commit/c48cb7ebfb4


Add a C test to an existing suite
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Use this when you are adding to or modifying an existing feature outside driver
model. An example is bloblist.

Add a new function in the same file as the rest of the suite and register it
with the suite. For example, to add a new mem_search test::

   /* Test 'ms' command with 32-bit values */
   static int mem_test_ms_new_thing(struct unit_test_state *uts)
   {
         /* test code here*/

         return 0;
   }
   MEM_TEST(mem_test_ms_new_thing, UT_TESTF_CONSOLE_REC);

Note that the MEM_TEST() macros is defined at the top of the file.

Example commit: 9fe064646d2 ("bloblist: Support relocating to a larger space") [1]

[1] https://gitlab.denx.de/u-boot/u-boot/-/commit/9fe064646d2


Add a new test suite
~~~~~~~~~~~~~~~~~~~~

Each suite should focus on one feature or subsystem, so if you are writing a
new one of those, you should add a new suite.

Create a new file in test/ or a subdirectory and define a macro to register the
suite. For example::

   #include <common.h>
   #include <console.h>
   #include <mapmem.h>
   #include <dm/test.h>
   #include <test/ut.h>

   /* Declare a new wibble test */
   #define WIBBLE_TEST(_name, _flags)   UNIT_TEST(_name, _flags, wibble_test)

   /* Tetss go here */

   /* At the bottom of the file: */

   int do_ut_wibble(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
   {
     struct unit_test *tests = UNIT_TEST_SUITE_START(wibble_test);
     const int n_ents = UNIT_TEST_SUITE_COUNT(wibble_test);

     return cmd_ut_category("cmd_wibble", "wibble_test_", tests, n_ents, argc, argv);
   }

Then add new tests to it as above.

Register this new suite in test/cmd_ut.c by adding to cmd_ut_sub[]::

  /* Within cmd_ut_sub[]... */

  U_BOOT_CMD_MKENT(wibble, CONFIG_SYS_MAXARGS, 1, do_ut_wibble, "", ""),

and adding new help to ut_help_text[]::

  "ut wibble - Test the wibble feature\n"

If your feature is conditional on a particular Kconfig, then you can use #ifdef
to control that.

Finally, add the test to the build by adding to the Makefile in the same
directory::

  obj-$(CONFIG_$(SPL_)CMDLINE) += wibble.o

Note that CMDLINE is never enabled in SPL, so this test will only be present in
U-Boot proper. See below for how to do SPL tests.

As before, you can add an extra Kconfig check if needed::

  ifneq ($(CONFIG_$(SPL_)WIBBLE),)
  obj-$(CONFIG_$(SPL_)CMDLINE) += wibble.o
  endif


Example commit: 919e7a8fb64 ("test: Add a simple test for bloblist") [1]

[1] https://gitlab.denx.de/u-boot/u-boot/-/commit/919e7a8fb64


Making the test run from pytest
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

All C tests must run from pytest. Typically this is automatic, since pytest
scans the U-Boot executable for available tests to run. So long as you have a
'ut' subcommand for your test suite, it will run. The same applies for driver
model tests since they use the 'ut dm' subcommand.

See test/py/tests/test_ut.py for how unit tests are run.


Add a C test for SPL
~~~~~~~~~~~~~~~~~~~~

Note: C tests are only available for sandbox_spl at present. There is currently
no mechanism in other boards to existing SPL tests even if they are built into
the image.

SPL tests cannot be run from the 'ut' command since there are no commands
available in SPL. Instead, sandbox (only) calls ut_run_list() on start-up, when
the -u flag is given. This runs the available unit tests, no matter what suite
they are in.

To create a new SPL test, follow the same rules as above, either adding to an
existing suite or creating a new one.

An example SPL test is spl_test_load().


Writing Python tests
--------------------

See :doc:`py_testing` for brief notes how to write Python tests. You
should be able to use the existing tests in test/py/tests as examples.
