# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2021, Asherah Connor <ashe@kivikakk.ee>

# Test qfw command implementation

import pytest

@pytest.mark.buildconfigspec('cmd_qfw')
def test_qfw_cpus(u_boot_console):
    "Test QEMU firmware config reports the CPU count."

    output = u_boot_console.run_command('qfw cpus')
    # The actual number varies depending on the board under test, so only
    # assert a non-zero output.
    assert 'cpu(s) online' in output
    assert '0 cpu(s) online' not in output

@pytest.mark.buildconfigspec('cmd_qfw')
def test_qfw_list(u_boot_console):
    "Test QEMU firmware config lists devices."

    output = u_boot_console.run_command('qfw list')
    # Assert either:
    # 1) 'test-one', from the sandbox driver, or
    # 2) 'bootorder', found in every real QEMU implementation.
    assert ("bootorder" in output) or ("test-one" in output)
