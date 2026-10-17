# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.

import pytest
import time

"""
Note: This test doesn't rely on boardenv_* configuration values but they can
change test behavior.

# Setup env__sleep_accurate to False if time is not accurate on your platform
env__sleep_accurate = False

# Setup env__sleep_time time in seconds board is set to sleep
env__sleep_time = 3

# Setup env__sleep_margin set a margin for any system overhead
env__sleep_margin = 0.25

"""

def test_sleep(u_boot_console):
    """Test the sleep command, and validate that it sleeps for approximately
    the correct amount of time."""

    sleep_skip = u_boot_console.config.env.get('env__sleep_accurate', True)
    if not sleep_skip:
        pytest.skip('sleep is not accurate')

    if u_boot_console.config.buildconfig.get('config_cmd_misc', 'n') != 'y':
        pytest.skip('sleep command not supported')

    # 3s isn't too long, but is enough to cross a few second boundaries.
    sleep_time = u_boot_console.config.env.get('env__sleep_time', 3)
    sleep_margin = u_boot_console.config.env.get('env__sleep_margin', 0.25)
    tstart = time.time()
    u_boot_console.run_command('sleep %d' % sleep_time)
    tend = time.time()
    elapsed = tend - tstart
    assert elapsed >= (sleep_time - 0.01)
    if not u_boot_console.config.gdbserver:
        # margin is hopefully enough to account for any system overhead.
        assert elapsed < (sleep_time + sleep_margin)

@pytest.mark.buildconfigspec("cmd_misc")
def test_time(u_boot_console):
    """Test the time command, and validate that it gives approximately the
    correct amount of command execution time."""

    sleep_skip = u_boot_console.config.env.get("env__sleep_accurate", True)
    if not sleep_skip:
        pytest.skip("sleep is not accurate")

    sleep_time = u_boot_console.config.env.get("env__sleep_time", 10)
    sleep_margin = u_boot_console.config.env.get("env__sleep_margin", 0.25)
    output = u_boot_console.run_command("time sleep %d" % sleep_time)
    execute_time = float(output.split()[1])
    assert sleep_time >= (execute_time - 0.01)
    if not u_boot_console.config.gdbserver:
        # margin is hopefully enough to account for any system overhead.
        assert sleep_time < (execute_time + sleep_margin)
