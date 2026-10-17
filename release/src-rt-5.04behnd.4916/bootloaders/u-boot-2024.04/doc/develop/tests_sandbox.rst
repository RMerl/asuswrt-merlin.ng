.. SPDX-License-Identifier: GPL-2.0+

Sandbox tests
=============

Test Design
-----------

Most uclasses and many functions of U-Boot have sandbox tests. This allows much
of the code to be checked in an developer-friendly environment.

Sandbox provides a way to write and run unit tests. The traditional approach to
unit tests is to build lots of little executables, one for each test or
category of tests. With sandbox, so far as possible, all the tests share a
small number of executables (e.g. 'u-boot' for sandbox, 'u-boot-spl' and
'u-boot' for sandbox_spl) and can be run very quickly. The vast majority of
tests can run on the 'sandbox' build,

Available tests
---------------

Some of the available tests are:

  - command_ut: Unit tests for command parsing and handling
  - compression: Unit tests for U-Boot's compression algorithms, useful for
      security checking. It supports gzip, bzip2, lzma and lzo.
  - image: Unit tests for images:

     - test/image/test-imagetools.sh - multi-file images
     - test/py/tests/test-fit.py     - FIT images
  - tracing: test/trace/test-trace.sh tests the tracing system (see
      README.trace)
  - verified boot: test/py/tests/test_vboot.py

If you change or enhance any U-Boot subsystem, you should write or expand a
test and include it with your patch series submission. Test coverage in some
older areas of U-Boot is still somewhat limited and we need to work to improve
it.

Note that many of these tests are implemented as commands which you can
run natively on your board if desired (and enabled).

To run all tests, use 'make check'.


Running sandbox tests directly
------------------------------

Typically tests are run using the pytest suite. Running pytests on sandbox is
easy and always gets things right. For example some tests require files to be
set up before they can work.

But it is also possible to run some sandbox tests directly. For example, this
runs the dm_test_gpio() test which you can find in test/dm/gpio.c::

   $ ./u-boot -T -c "ut dm gpio"


   U-Boot 2021.01

   Model: sandbox
   DRAM:  128 MiB
   WDT:   Started with servicing (60s timeout)
   MMC:   mmc2: 2 (SD), mmc1: 1 (SD), mmc0: 0 (SD)
   In:    serial
   Out:   vidconsole
   Err:   vidconsole
   Model: sandbox
   SCSI:
   Net:   eth0: eth@10002000, eth5: eth@10003000, eth3: sbe5, eth6: eth@10004000
   Test: dm_test_gpio: gpio.c
   Test: dm_test_gpio: gpio.c (flat tree)
   Failures: 0

The -T option tells the U-Boot sandbox to run with the 'test' devicetree
(test.dts) instead of -D which selects the normal sandbox.dts - this is
necessary because many tests rely on nodes or properties in the test devicetree.
If you try running tests without -T then you may see failures, like::

   $ ./u-boot -c "ut dm gpio"


   U-Boot 2021.01

   DRAM:  128 MiB
   WDT:   Not found!
   MMC:
   In:    serial
   Out:   serial
   Err:   serial
   SCSI:
   Net:   No ethernet found.
   Please run with test device tree:
       ./u-boot -d arch/sandbox/dts/test.dtb
   Test: dm_test_gpio: gpio.c
   test/dm/gpio.c:37, dm_test_gpio(): 0 == gpio_lookup_name("b4", &dev, &offset, &gpio): Expected 0x0 (0), got 0xffffffea (-22)
   Test: dm_test_gpio: gpio.c (flat tree)
   test/dm/gpio.c:37, dm_test_gpio(): 0 == gpio_lookup_name("b4", &dev, &offset, &gpio): Expected 0x0 (0), got 0xffffffea (-22)
   Failures: 2

The message above should provide a hint if you forget to use the -T flag. Even
running with -D will produce different results.

You can easily use gdb on these tests, without needing --gdbserver::

   $ gdb --args u-boot -T -c "ut dm gpio"
   ...
   (gdb) break dm_test_gpio
   Breakpoint 1 at 0x1415bd: file test/dm/gpio.c, line 37.
   (gdb) run -T -c "ut dm gpio"
   Starting program: u-boot -T -c "ut dm gpio"
   Test: dm_test_gpio: gpio.c

   Breakpoint 1, dm_test_gpio (uts=0x5555558029a0 <global_dm_test_state>)
       at files/test/dm/gpio.c:37
   37		ut_assertok(gpio_lookup_name("b4", &dev, &offset, &gpio));
   (gdb)

You can then single-step and look at variables as needed.


Running tests multiple times
----------------------------

Some tests can have race conditions which are hard to detect on a single
one. It is possible to run each individual test multiple times, before moving
to the next test, with the '-r' flag.

This is most useful when running a single test, since running all tests
multiple times can take a while.

For example::

   => ut dm -r1000 dm_test_rtc_set_get
   ...
   Test: dm_test_rtc_set_get: rtc.c (flat tree)
   Test: dm_test_rtc_set_get: rtc.c
   test/dm/rtc.c:257, dm_test_rtc_reset(): old_base_time == base_time: Expected 0x62e7453c (1659323708), got 0x62e7453d (1659323709)
   Test: dm_test_rtc_set_get: rtc.c (flat tree)
   Test: dm_test_rtc_set_get: rtc.c
   Test: dm_test_rtc_set_get: rtc.c (flat tree)
   ...
   Test dm_test_rtc_reset failed 3 times


Isolating a test that breaks another
------------------------------------

When running unit tests, some may have side effects which cause a subsequent
test to break. This can sometimes be seen when using 'ut dm' or similar.

You can use the `-I` argument to the `ut` command to isolate this problem.
First use `ut info` to see how many tests there are, then use a binary search to
home in on the problem. Note that you might need to restart U-Boot after each
iteration, so the `-c` argument to U-Boot is useful.

For example, let's stay that dm_test_host() is failing::

   => ut dm
   ...
   Test: dm_test_get_stats: core.c
   Test: dm_test_get_stats: core.c (flat tree)
   Test: dm_test_host: host.c
   test/dm/host.c:71, dm_test_host(): 0 == ut_check_delta(mem_start): Expected 0x0 (0), got 0xffffcbb0 (-13392)
   Test: dm_test_host: host.c (flat tree)
   Test <NULL> failed 1 times
   Test: dm_test_host_dup: host.c
   Test: dm_test_host_dup: host.c (flat tree)
   ...

You can then tell U-Boot to run the failing test at different points in the
sequence:

   => ut info
   Test suites: 21
   Total tests: 645

::

   $ ./u-boot -T -c "ut dm -I300:dm_test_host"
   ...
   Test: dm_test_pinctrl_single: pinmux.c (flat tree)
   Test: dm_test_host: host.c
   test/dm/host.c:71, dm_test_host(): 0 == ut_check_delta(mem_start): Expected 0x0 (0), got 0xfffffdb0 (-592)
   Test: dm_test_host: host.c (flat tree)
   Test dm_test_host failed 1 times (position 300)
   Failures: 4

So it happened before position 300. Trying 150 shows it failing, so we try 75::

   $ ./u-boot  -T  -c "ut dm -I75:dm_test_host"
   ...
   Test: dm_test_autoprobe: core.c
   Test: dm_test_autoprobe: core.c (flat tree)
   Test: dm_test_host: host.c
   Test: dm_test_host: host.c (flat tree)
   Failures: 0

That succeeds, so we try 120, etc. until eventually we can figure out that the
problem first happens at position 82.

   $ ./u-boot  -T  -c "ut dm -I82:dm_test_host"
   ...
   Test: dm_test_blk_flags: blk.c
   Test: dm_test_blk_flags: blk.c (flat tree)
   Test: dm_test_host: host.c
   test/dm/host.c:71, dm_test_host(): 0 == ut_check_delta(mem_start): Expected 0x0 (0), got 0xffffc960 (-13984)
   Test: dm_test_host: host.c (flat tree)
   Test dm_test_host failed 1 times (position 82)
   Failures: 1

From this we can deduce that `dm_test_blk_flags()` causes the problem with
`dm_test_host()`.

Running sandbox_spl tests directly
----------------------------------

SPL is the phase before U-Boot proper. It is present in the sandbox_spl build,
so you can run SPL like this::

   ./spl/u-boot-spl

SPL tests are special in that they run (only in the SPL phase, of course) if the
-u flag is given::

   ./spl/u-boot-spl -u

   U-Boot SPL 2021.01-00723-g43c77b51be5-dirty (Jan 24 2021 - 16:38:24 -0700)
   Running 5 driver model tests
   Test: dm_test_of_plat_base: of_platdata.c (flat tree)
   Test: dm_test_of_plat_dev: of_platdata.c (flat tree)
   Test: dm_test_of_plat_parent: of_platdata.c (flat tree)
   Test: dm_test_of_plat_phandle: of_platdata.c (flat tree)
   Test: dm_test_of_plat_props: of_platdata.c (flat tree)
   Failures: 0


   U-Boot 2021.01-00723-g43c77b51be5-dirty (Jan 24 2021 - 16:38:24 -0700)

   DRAM:  128 MiB
   ...

It is not possible to run SPL tests in U-Boot proper, firstly because they are
not built into U-Boot proper and secondly because the environment is very
different, e.g. some SPL tests rely on of-platdata which is only available in
SPL.

Note that after running, SPL continues to boot into U-Boot proper. You can add
'-c exit' to make U-Boot quit without doing anything further. It is not
currently possible to run SPL tests and then stop, since the pytests require
that U-Boot produces the expected banner.

You can use the -k flag to select which tests run::

   ./spl/u-boot-spl -u -k dm_test_of_plat_parent

Of course you can use gdb with sandbox_spl, just as with sandbox.


Running all tests directly
--------------------------

A fast way to run all sandbox tests is::

   ./u-boot -T -c "ut all"

It typically runs single-thread in 6 seconds on 2021 hardware, with 2s of that
to the delays in the time test.

This should not be considered a substitute for 'make check', but can be helpful
for git bisect, etc.


What tests are built in?
------------------------

Whatever sandbox build is used, which tests are present is determined by which
source files are built. For sandbox_spl, the of_platdata tests are built
because of the build rule in test/dm/Makefile::

   ifeq ($(CONFIG_SPL_BUILD),y)
   obj-$(CONFIG_SPL_OF_PLATDATA) += of_platdata.o
   else
   ...other tests for non-spl
   endif

You can get a list of tests in a U-Boot ELF file by looking for the
linker_list::

   $ nm /tmp/b/sandbox_spl/spl/u-boot-spl |grep 2_dm_test
   000000000001f200 D _u_boot_list_2_dm_test_2_dm_test_of_plat_base
   000000000001f220 D _u_boot_list_2_dm_test_2_dm_test_of_plat_dev
   000000000001f240 D _u_boot_list_2_dm_test_2_dm_test_of_plat_parent
   000000000001f260 D _u_boot_list_2_dm_test_2_dm_test_of_plat_phandle
   000000000001f280 D _u_boot_list_2_dm_test_2_dm_test_of_plat_props


Writing tests
-------------

See :doc:`tests_writing` for how to write new tests.

