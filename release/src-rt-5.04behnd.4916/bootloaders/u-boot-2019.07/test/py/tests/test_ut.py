# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.

import os.path
import pytest

@pytest.mark.buildconfigspec('ut_dm')
def test_ut_dm_init(u_boot_console):
    """Initialize data for ut dm tests."""

    fn = u_boot_console.config.source_dir + '/testflash.bin'
    if not os.path.exists(fn):
        data = 'this is a test'
        data += '\x00' * ((4 * 1024 * 1024) - len(data))
        with open(fn, 'wb') as fh:
            fh.write(data)

    fn = u_boot_console.config.source_dir + '/spi.bin'
    if not os.path.exists(fn):
        data = '\x00' * (2 * 1024 * 1024)
        with open(fn, 'wb') as fh:
            fh.write(data)

def test_ut(u_boot_console, ut_subtest):
    """Execute a "ut" subtest."""

    output = u_boot_console.run_command('ut ' + ut_subtest)
    assert output.endswith('Failures: 0')
