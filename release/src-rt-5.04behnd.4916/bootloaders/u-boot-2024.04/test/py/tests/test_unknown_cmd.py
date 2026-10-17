# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2015 Stephen Warren
# Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.

def test_unknown_command(u_boot_console):
    """Test that executing an unknown command causes U-Boot to print an
    error."""

    # The "unknown command" error is actively expected here,
    # so error detection for it is disabled.
    with u_boot_console.disable_check('unknown_command'):
        response = u_boot_console.run_command('non_existent_cmd')
    assert('Unknown command \'non_existent_cmd\' - try \'help\'' in response)
