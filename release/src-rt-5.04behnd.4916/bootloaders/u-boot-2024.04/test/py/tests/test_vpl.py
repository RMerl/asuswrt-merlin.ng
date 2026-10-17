# SPDX-License-Identifier: GPL-2.0
# Copyright 2022 Google LLC
# Written by Simon Glass <sjg@chromium.org>

import os.path
import pytest

def test_vpl(u_boot_console, ut_vpl_subtest):
    """Execute a "ut" subtest.

    The subtests are collected in function generate_ut_subtest() from linker
    generated lists by applying a regular expression to the lines of file
    vpl/u-boot-vpl.sym. The list entries are created using the C macro
    UNIT_TEST().

    Strict naming conventions have to be followed to match the regular
    expression. Use UNIT_TEST(foo_test_bar, _flags, foo_test) for a test bar in
    test suite foo that can be executed via command 'ut foo bar' and is
    implemented in C function foo_test_bar().

    Args:
        u_boot_console (ConsoleBase): U-Boot console
        ut_subtest (str): VPL test to be executed (e.g. 'dm platdata_phandle')
    """
    try:
        cons = u_boot_console
        cons.restart_uboot_with_flags(['-u', '-k', ut_vpl_subtest.split()[1]])
        output = cons.get_spawn_output().replace('\r', '')
        assert 'Failures: 0' in output
    finally:
        # Restart afterward in case a non-VPL test is run next. This should not
        # happen since VPL tests are run in their own invocation of test.py, but
        # the cost of doing this is not too great at present.
        u_boot_console.restart_uboot()
