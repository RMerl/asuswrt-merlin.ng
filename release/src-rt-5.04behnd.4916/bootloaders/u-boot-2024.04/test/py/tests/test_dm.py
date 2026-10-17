# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2020 Sean Anderson

import pytest

@pytest.mark.buildconfigspec('cmd_dm')
def test_dm_compat(u_boot_console):
    """Test that each driver in `dm tree` is also listed in `dm compat`."""
    response = u_boot_console.run_command('dm tree')
    driver_index = response.find('Driver')
    assert driver_index != -1
    drivers = (line[driver_index:].split()[0]
               for line in response[:-1].split('\n')[2:])

    response = u_boot_console.run_command('dm compat')
    for driver in drivers:
        assert driver in response

    # check sorting - output looks something like this:
    #  testacpi      0  [   ]   testacpi_drv          |-- acpi-test
    #  testacpi      1  [   ]   testacpi_drv          |   `-- child
    #  pci_emul_p    1  [   ]   pci_emul_parent_drv   |-- pci-emul2
    #  pci_emul      5  [   ]   sandbox_swap_case_em  |   `-- emul2@1f,0

    # The number of '|   ' and '--' matches indicate the indent level. We start
    # checking sorting only after UCLASS_AXI_EMUL after which the names should
    # be sorted.

    response = u_boot_console.run_command('dm tree -s')
    lines = response.split('\n')[2:]
    stack = []   # holds where we were up to at the previous indent level
    prev = ''    # uclass name of previous line
    start = False
    for line in lines:
        indent = line.count('|   ') + ('--' in line)
        cur = line.split()[0]
        if not start:
            if cur != 'axi_emul':
                continue
            start = True

        # Handle going up or down an indent level
        if indent > len(stack):
            stack.append(prev)
            prev = ''
        elif indent < len(stack):
            prev = stack.pop()

        # Check that the current uclass name is not alphabetically before the
        # previous one
        if 'emul' not in cur and cur < prev:
            print('indent', cur >= prev, indent, prev, cur, stack)
            assert cur >= prev
            prev = cur


@pytest.mark.buildconfigspec('cmd_dm')
def test_dm_drivers(u_boot_console):
    """Test that each driver in `dm compat` is also listed in `dm drivers`."""
    response = u_boot_console.run_command('dm compat')
    drivers = (line[:20].rstrip() for line in response[:-1].split('\n')[2:])
    response = u_boot_console.run_command('dm drivers')
    for driver in drivers:
        assert driver in response

@pytest.mark.buildconfigspec('cmd_dm')
def test_dm_static(u_boot_console):
    """Test that each driver in `dm static` is also listed in `dm drivers`."""
    response = u_boot_console.run_command('dm static')
    drivers = (line[:25].rstrip() for line in response[:-1].split('\n')[2:])
    response = u_boot_console.run_command('dm drivers')
    for driver in drivers:
        assert driver in response

@pytest.mark.buildconfigspec("cmd_dm")
def test_dm_uclass(u_boot_console):
    response = u_boot_console.run_command("dm uclass")

@pytest.mark.buildconfigspec("cmd_dm")
def test_dm_devres(u_boot_console):
    response = u_boot_console.run_command("dm devres")
