# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2015 Stephen Warren
# Copyright (c) 2015-2016, NVIDIA CORPORATION. All rights reserved.

import pytest
import signal

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('sysreset_cmd_poweroff')
def test_poweroff(u_boot_console):
    """Test that the "poweroff" command exits sandbox process."""

    u_boot_console.run_command('poweroff', wait_for_prompt=False)
    assert(u_boot_console.validate_exited())

@pytest.mark.boardspec('sandbox')
def test_ctrl_c(u_boot_console):
    """Test that sending SIGINT to sandbox causes it to exit."""

    u_boot_console.kill(signal.SIGINT)
    assert(u_boot_console.validate_exited())

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_exception')
@pytest.mark.buildconfigspec('sandbox_crash_reset')
def test_exception_reset(u_boot_console):
    """Test that SIGILL causes a reset."""

    u_boot_console.run_command('exception undefined', wait_for_prompt=False)
    m = u_boot_console.p.expect(['resetting ...', 'U-Boot'])
    if m != 0:
        raise Exception('SIGILL did not lead to reset')
    m = u_boot_console.p.expect(['U-Boot', '=>'])
    if m != 0:
        raise Exception('SIGILL did not lead to reset')
    u_boot_console.restart_uboot()

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_exception')
@pytest.mark.notbuildconfigspec('sandbox_crash_reset')
def test_exception_exit(u_boot_console):
    """Test that SIGILL causes a reset."""

    u_boot_console.run_command('exception undefined', wait_for_prompt=False)
    assert(u_boot_console.validate_exited())
