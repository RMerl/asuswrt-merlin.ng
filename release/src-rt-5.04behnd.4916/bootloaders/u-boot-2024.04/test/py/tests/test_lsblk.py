# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2020
# Niel Fourie, DENX Software Engineering, lusus@denx.de

import pytest

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('blk')
@pytest.mark.buildconfigspec('cmd_lsblk')
def test_lsblk(u_boot_console):
    """Test that `lsblk` prints a result which includes `host`."""
    output = u_boot_console.run_command('lsblk')
    assert "Block Driver" in output
    assert "sandbox_host_blk" in output
