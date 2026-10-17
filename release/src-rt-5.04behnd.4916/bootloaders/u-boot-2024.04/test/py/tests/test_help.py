# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2015 Stephen Warren
# Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.

import pytest

def test_help(u_boot_console):
    """Test that the "help" command can be executed."""

    u_boot_console.run_command('help')

@pytest.mark.boardspec('sandbox')
def test_help_no_devicetree(u_boot_console):
    try:
        cons = u_boot_console
        cons.restart_uboot_with_flags([], use_dtb=False)
        cons.run_command('help')
        output = cons.get_spawn_output().replace('\r', '')
        assert 'print command description/usage' in output
    finally:
        # Restart afterward to get the normal device tree back
        u_boot_console.restart_uboot()

@pytest.mark.boardspec('sandbox_vpl')
def test_vpl_help(u_boot_console):
    try:
        cons = u_boot_console
        cons.restart_uboot()
        cons.run_command('help')
        output = cons.get_spawn_output().replace('\r', '')
        assert 'print command description/usage' in output
    finally:
        # Restart afterward to get the normal device tree back
        u_boot_console.restart_uboot()
