# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2020
# Niel Fourie, DENX Software Engineering, lusus@denx.de

import pytest

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_fs_generic')
def test_fstypes(u_boot_console):
    """Test that `fstypes` prints a result which includes `sandbox`."""
    output = u_boot_console.run_command('fstypes')
    assert "Supported filesystems:" in output
    assert "sandbox" in output
