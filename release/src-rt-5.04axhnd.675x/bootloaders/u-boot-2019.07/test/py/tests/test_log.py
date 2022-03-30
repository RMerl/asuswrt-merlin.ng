# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016, Google Inc.
#
# U-Boot Verified Boot Test

"""
This tests U-Boot logging. It uses the 'log test' command with various options
and checks that the output is correct.
"""

import pytest

LOGL_FIRST, LOGL_WARNING, LOGL_INFO = (0, 4, 6)

@pytest.mark.buildconfigspec('cmd_log')
def test_log(u_boot_console):
    """Test that U-Boot logging works correctly."""
    def check_log_entries(lines, mask, max_level=LOGL_INFO):
        """Check that the expected log records appear in the output

        Args:
            lines: iterator containing lines to check
            mask: bit mask to select which lines to check for:
                bit 0: standard log line
                bit 1: _log line
            max_level: maximum log level to expect in the output
        """
        for i in range(max_level):
            if mask & 1:
                assert 'log_run() log %d' % i == lines.next()
            if mask & 3:
                assert 'func() _log %d' % i == lines.next()

    def run_test(testnum):
        """Run a particular test number (the 'log test' command)

        Args:
            testnum: Test number to run
        Returns:
            iterator containing the lines output from the command
        """
        with cons.log.section('basic'):
           output = u_boot_console.run_command('log test %d' % testnum)
        split = output.replace('\r', '').splitlines()
        lines = iter(split)
        assert 'test %d' % testnum == lines.next()
        return lines

    def test0():
        lines = run_test(0)
        check_log_entries(lines, 3)

    def test1():
        lines = run_test(1)
        check_log_entries(lines, 3)

    def test2():
        lines = run_test(2)

    def test3():
        lines = run_test(3)
        check_log_entries(lines, 2)

    def test4():
        lines = run_test(4)
        assert next(lines, None) == None

    def test5():
        lines = run_test(5)
        check_log_entries(lines, 2)

    def test6():
        lines = run_test(6)
        check_log_entries(lines, 3)

    def test7():
        lines = run_test(7)
        check_log_entries(lines, 3, LOGL_WARNING)

    def test8():
        lines = run_test(8)
        check_log_entries(lines, 3)

    def test9():
        lines = run_test(9)
        check_log_entries(lines, 3)

    def test10():
        lines = run_test(10)
        for i in range(7):
            assert 'log_test() level %d' % i == lines.next()

    # TODO(sjg@chromium.org): Consider structuring this as separate tests
    cons = u_boot_console
    test0()
    test1()
    test2()
    test3()
    test4()
    test5()
    test6()
    test7()
    test8()
    test9()
    test10()

@pytest.mark.buildconfigspec('cmd_log')
def test_log_format(u_boot_console):
    """Test the 'log format' and 'log rec' commands"""
    def run_with_format(fmt, expected_output):
        """Set up the log format and then write a log record

        Args:
            fmt: Format to use for 'log format'
            expected_output: Expected output from the 'log rec' command
        """
        output = cons.run_command('log format %s' % fmt)
        assert output == ''
        output = cons.run_command('log rec arch notice file.c 123 func msg')
        assert output == expected_output

    cons = u_boot_console
    with cons.log.section('format'):
        run_with_format('all', 'NOTICE.arch,file.c:123-func() msg')
        output = cons.run_command('log format')
        assert output == 'Log format: clFLfm'

        run_with_format('fm', 'func() msg')
        run_with_format('clfm', 'NOTICE.arch,func() msg')
        run_with_format('FLfm', 'file.c:123-func() msg')
        run_with_format('lm', 'NOTICE. msg')
        run_with_format('m', 'msg')
