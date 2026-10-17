# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2021 Broadcom

import pytest
import signal

@pytest.mark.buildconfigspec('cmd_stackprotector_test')
@pytest.mark.notbuildconfigspec('asan')
def test_stackprotector(u_boot_console):
    """Test that the stackprotector function works."""

    u_boot_console.run_command('stackprot_test',wait_for_prompt=False)
    expected_response = 'Stack smashing detected'
    u_boot_console.wait_for(expected_response)
    u_boot_console.restart_uboot()
