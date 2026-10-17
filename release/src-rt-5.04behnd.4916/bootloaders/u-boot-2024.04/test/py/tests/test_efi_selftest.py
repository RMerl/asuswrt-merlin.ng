# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2017, Heinrich Schuchardt <xypron.glpk@gmx.de>

""" Test UEFI API implementation
"""

import pytest

@pytest.mark.buildconfigspec('cmd_bootefi_selftest')
def test_efi_selftest_base(u_boot_console):
    """Run UEFI unit tests

    u_boot_console -- U-Boot console

    This function executes all selftests that are not marked as on request.
    """
    u_boot_console.run_command(cmd='setenv efi_selftest')
    u_boot_console.run_command(cmd='bootefi selftest', wait_for_prompt=False)
    if u_boot_console.p.expect(['Summary: 0 failures', 'Press any key']):
        raise Exception('Failures occurred during the EFI selftest')
    u_boot_console.restart_uboot()

@pytest.mark.buildconfigspec('cmd_bootefi_selftest')
@pytest.mark.buildconfigspec('hush_parser')
@pytest.mark.buildconfigspec('of_control')
@pytest.mark.notbuildconfigspec('generate_acpi_table')
def test_efi_selftest_device_tree(u_boot_console):
    """Test the device tree support in the UEFI sub-system

    u_boot_console -- U-Boot console

    This test executes the UEFI unit test by calling 'bootefi selftest'.
    """
    u_boot_console.run_command(cmd='setenv efi_selftest list')
    output = u_boot_console.run_command('bootefi selftest')
    assert '\'device tree\'' in output
    u_boot_console.run_command(cmd='setenv efi_selftest device tree')
    # Set serial# if it is not already set.
    u_boot_console.run_command(cmd='setenv efi_test "${serial#}x"')
    u_boot_console.run_command(cmd='test "${efi_test}" = x && setenv serial# 0')
    u_boot_console.run_command(cmd='bootefi selftest ${fdtcontroladdr}', wait_for_prompt=False)
    if u_boot_console.p.expect(['serial-number:', 'U-Boot']):
        raise Exception('serial-number missing in device tree')
    u_boot_console.restart_uboot()

@pytest.mark.buildconfigspec('cmd_bootefi_selftest')
def test_efi_selftest_watchdog_reboot(u_boot_console):
    """Test the watchdog timer

    u_boot_console -- U-Boot console

    This function executes the 'watchdog reboot' unit test.
    """
    u_boot_console.run_command(cmd='setenv efi_selftest list')
    output = u_boot_console.run_command('bootefi selftest')
    assert '\'watchdog reboot\'' in output
    u_boot_console.run_command(cmd='setenv efi_selftest watchdog reboot')
    u_boot_console.run_command(cmd='bootefi selftest', wait_for_prompt=False)
    if u_boot_console.p.expect(['resetting', 'U-Boot']):
        raise Exception('Reset failed in \'watchdog reboot\' test')
    u_boot_console.restart_uboot()

@pytest.mark.buildconfigspec('cmd_bootefi_selftest')
def test_efi_selftest_text_input(u_boot_console):
    """Test the EFI_SIMPLE_TEXT_INPUT_PROTOCOL

    u_boot_console -- U-Boot console

    This function calls the text input EFI selftest.
    """
    u_boot_console.run_command(cmd='setenv efi_selftest text input')
    u_boot_console.run_command(cmd='bootefi selftest', wait_for_prompt=False)
    if u_boot_console.p.expect([r'To terminate type \'x\'']):
        raise Exception('No prompt for \'text input\' test')
    u_boot_console.drain_console()
    # EOT
    u_boot_console.run_command(cmd=chr(4), wait_for_echo=False,
                               send_nl=False, wait_for_prompt=False)
    if u_boot_console.p.expect([r'Unicode char 4 \(unknown\), scan code 0 \(Null\)']):
        raise Exception('EOT failed in \'text input\' test')
    u_boot_console.drain_console()
    # BS
    u_boot_console.run_command(cmd=chr(8), wait_for_echo=False,
                               send_nl=False, wait_for_prompt=False)
    if u_boot_console.p.expect([r'Unicode char 8 \(BS\), scan code 0 \(Null\)']):
        raise Exception('BS failed in \'text input\' test')
    u_boot_console.drain_console()
    # TAB
    u_boot_console.run_command(cmd=chr(9), wait_for_echo=False,
                               send_nl=False, wait_for_prompt=False)
    if u_boot_console.p.expect([r'Unicode char 9 \(TAB\), scan code 0 \(Null\)']):
        raise Exception('BS failed in \'text input\' test')
    u_boot_console.drain_console()
    # a
    u_boot_console.run_command(cmd='a', wait_for_echo=False, send_nl=False,
                               wait_for_prompt=False)
    if u_boot_console.p.expect([r'Unicode char 97 \(\'a\'\), scan code 0 \(Null\)']):
        raise Exception('\'a\' failed in \'text input\' test')
    u_boot_console.drain_console()
    # UP escape sequence
    u_boot_console.run_command(cmd=chr(27) + '[A', wait_for_echo=False,
                               send_nl=False, wait_for_prompt=False)
    if u_boot_console.p.expect([r'Unicode char 0 \(Null\), scan code 1 \(Up\)']):
        raise Exception('UP failed in \'text input\' test')
    u_boot_console.drain_console()
    # Euro sign
    u_boot_console.run_command(cmd=b'\xe2\x82\xac'.decode(), wait_for_echo=False,
                               send_nl=False, wait_for_prompt=False)
    if u_boot_console.p.expect([r'Unicode char 8364 \(\'']):
        raise Exception('Euro sign failed in \'text input\' test')
    u_boot_console.drain_console()
    u_boot_console.run_command(cmd='x', wait_for_echo=False, send_nl=False,
                               wait_for_prompt=False)
    if u_boot_console.p.expect(['Summary: 0 failures', 'Press any key']):
        raise Exception('Failures occurred during the EFI selftest')
    u_boot_console.restart_uboot()

@pytest.mark.buildconfigspec('cmd_bootefi_selftest')
def test_efi_selftest_text_input_ex(u_boot_console):
    """Test the EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL

    u_boot_console -- U-Boot console

    This function calls the extended text input EFI selftest.
    """
    u_boot_console.run_command(cmd='setenv efi_selftest extended text input')
    u_boot_console.run_command(cmd='bootefi selftest', wait_for_prompt=False)
    if u_boot_console.p.expect([r'To terminate type \'CTRL\+x\'']):
        raise Exception('No prompt for \'text input\' test')
    u_boot_console.drain_console()
    # EOT
    u_boot_console.run_command(cmd=chr(4), wait_for_echo=False,
                               send_nl=False, wait_for_prompt=False)
    if u_boot_console.p.expect([r'Unicode char 100 \(\'d\'\), scan code 0 \(CTRL\+Null\)']):
        raise Exception('EOT failed in \'text input\' test')
    u_boot_console.drain_console()
    # BS
    u_boot_console.run_command(cmd=chr(8), wait_for_echo=False,
                               send_nl=False, wait_for_prompt=False)
    if u_boot_console.p.expect([r'Unicode char 8 \(BS\), scan code 0 \(\+Null\)']):
        raise Exception('BS failed in \'text input\' test')
    u_boot_console.drain_console()
    # TAB
    u_boot_console.run_command(cmd=chr(9), wait_for_echo=False,
                               send_nl=False, wait_for_prompt=False)
    if u_boot_console.p.expect([r'Unicode char 9 \(TAB\), scan code 0 \(\+Null\)']):
        raise Exception('TAB failed in \'text input\' test')
    u_boot_console.drain_console()
    # a
    u_boot_console.run_command(cmd='a', wait_for_echo=False, send_nl=False,
                               wait_for_prompt=False)
    if u_boot_console.p.expect([r'Unicode char 97 \(\'a\'\), scan code 0 \(Null\)']):
        raise Exception('\'a\' failed in \'text input\' test')
    u_boot_console.drain_console()
    # UP escape sequence
    u_boot_console.run_command(cmd=chr(27) + '[A', wait_for_echo=False,
                               send_nl=False, wait_for_prompt=False)
    if u_boot_console.p.expect([r'Unicode char 0 \(Null\), scan code 1 \(\+Up\)']):
        raise Exception('UP failed in \'text input\' test')
    u_boot_console.drain_console()
    # Euro sign
    u_boot_console.run_command(cmd=b'\xe2\x82\xac'.decode(), wait_for_echo=False,
                               send_nl=False, wait_for_prompt=False)
    if u_boot_console.p.expect([r'Unicode char 8364 \(\'']):
        raise Exception('Euro sign failed in \'text input\' test')
    u_boot_console.drain_console()
    # SHIFT+ALT+FN 5
    u_boot_console.run_command(cmd=b'\x1b\x5b\x31\x35\x3b\x34\x7e'.decode(),
                               wait_for_echo=False, send_nl=False,
                               wait_for_prompt=False)
    if u_boot_console.p.expect([r'Unicode char 0 \(Null\), scan code 15 \(SHIFT\+ALT\+FN 5\)']):
        raise Exception('SHIFT+ALT+FN 5 failed in \'text input\' test')
    u_boot_console.drain_console()
    u_boot_console.run_command(cmd=chr(24), wait_for_echo=False, send_nl=False,
                               wait_for_prompt=False)
    if u_boot_console.p.expect(['Summary: 0 failures', 'Press any key']):
        raise Exception('Failures occurred during the EFI selftest')
    u_boot_console.restart_uboot()

@pytest.mark.buildconfigspec('cmd_bootefi_selftest')
@pytest.mark.buildconfigspec('efi_tcg2_protocol')
def test_efi_selftest_tcg2(u_boot_console):
    """Test the EFI_TCG2 PROTOCOL

    u_boot_console -- U-Boot console

    This function executes the 'tcg2' unit test.
    """
    u_boot_console.restart_uboot()
    u_boot_console.run_command(cmd='setenv efi_selftest list')
    output = u_boot_console.run_command('bootefi selftest')
    assert '\'tcg2\'' in output
    u_boot_console.run_command(cmd='setenv efi_selftest tcg2')
    u_boot_console.run_command(cmd='bootefi selftest', wait_for_prompt=False)
    if u_boot_console.p.expect(['Summary: 0 failures', 'Press any key']):
        raise Exception('Failures occurred during the EFI selftest')
    u_boot_console.restart_uboot()
