# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.

import pytest
import time

"""
Note: This test doesn't rely on boardenv_* configuration values but they can
change test behavior.

# Setup env__sleep_accurate to False if time is not accurate on your platform
env__sleep_accurate = False

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
    sleep_time = 3
    tstart = time.time()
    u_boot_console.run_command('sleep %d' % sleep_time)
    tend = time.time()
    elapsed = tend - tstart
    assert elapsed >= (sleep_time - 0.01)
    if not u_boot_console.config.gdbserver:
        # 0.25s margin is hopefully enough to account for any system overhead.
        assert elapsed < (sleep_time + 0.25)
