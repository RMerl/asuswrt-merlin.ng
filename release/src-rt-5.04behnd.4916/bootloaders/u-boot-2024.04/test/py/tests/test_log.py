# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016, Google Inc.
#
# U-Boot Verified Boot Test

"""
This tests U-Boot logging. It uses the 'log test' command with various options
and checks that the output is correct.
"""

import pytest

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

@pytest.mark.buildconfigspec('debug_uart')
@pytest.mark.boardspec('sandbox')
def test_log_dropped(u_boot_console):
    """Test dropped 'log' message when debug_uart is activated"""

    cons = u_boot_console
    cons.restart_uboot()
    output = cons.get_spawn_output().replace('\r', '')
    assert (not 'debug: main' in output)
