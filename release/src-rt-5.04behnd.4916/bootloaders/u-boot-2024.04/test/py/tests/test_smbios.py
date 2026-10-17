# SPDX-License-Identifier: GPL-2.0-or-later

"""Test smbios command"""

import pytest

@pytest.mark.buildconfigspec('cmd_smbios')
@pytest.mark.notbuildconfigspec('qfw_smbios')
@pytest.mark.notbuildconfigspec('sandbox')
def test_cmd_smbios(u_boot_console):
    """Run the smbios command"""
    output = u_boot_console.run_command('smbios')
    assert 'DMI type 127,' in output

@pytest.mark.buildconfigspec('cmd_smbios')
@pytest.mark.buildconfigspec('qfw_smbios')
@pytest.mark.notbuildconfigspec('sandbox')
# TODO:
# QEMU v8.2.0 lacks SMBIOS support for RISC-V
# Once support is available in our Docker image we can remove the constraint.
@pytest.mark.notbuildconfigspec('riscv')
def test_cmd_smbios_qemu(u_boot_console):
    """Run the smbios command on QEMU"""
    output = u_boot_console.run_command('smbios')
    assert 'DMI type 1,' in output
    assert 'Manufacturer: QEMU' in output
    assert 'DMI type 127,' in output

@pytest.mark.buildconfigspec('cmd_smbios')
@pytest.mark.buildconfigspec('sandbox')
def test_cmd_smbios_sandbox(u_boot_console):
    """Run the smbios command on the sandbox"""
    output = u_boot_console.run_command('smbios')
    assert 'DMI type 0,' in output
    assert 'String 1: U-Boot' in output
    assert 'DMI type 1,' in output
    assert 'Manufacturer: sandbox' in output
    assert 'DMI type 2,' in output
    assert 'DMI type 3,' in output
    assert 'DMI type 4,' in output
    assert 'DMI type 127,' in output
