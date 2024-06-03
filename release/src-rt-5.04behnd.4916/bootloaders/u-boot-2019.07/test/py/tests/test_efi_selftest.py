# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2017, Heinrich Schuchardt <xypron.glpk@gmx.de>

# Test efi API implementation

import pytest
import u_boot_utils

@pytest.mark.buildconfigspec('cmd_bootefi_selftest')
def test_efi_selftest(u_boot_console):
	"""Test the UEFI implementation

	:param u_boot_console: U-Boot console

	This function executes all selftests that are not marked as on request.
	"""
	u_boot_console.run_command(cmd='setenv efi_selftest')
	u_boot_console.run_command(cmd='bootefi selftest', wait_for_prompt=False)
	m = u_boot_console.p.expect(['Summary: 0 failures', 'Press any key'])
	if m != 0:
		raise Exception('Failures occurred during the EFI selftest')
	u_boot_console.run_command(cmd='', wait_for_echo=False, wait_for_prompt=False);
	m = u_boot_console.p.expect(['resetting', 'U-Boot'])
	if m != 0:
		raise Exception('Reset failed during the EFI selftest')
	u_boot_console.restart_uboot();

@pytest.mark.buildconfigspec('cmd_bootefi_selftest')
@pytest.mark.buildconfigspec('of_control')
@pytest.mark.notbuildconfigspec('generate_acpi_table')
def test_efi_selftest_device_tree(u_boot_console):
	u_boot_console.run_command(cmd='setenv efi_selftest list')
	output = u_boot_console.run_command('bootefi selftest')
	assert '\'device tree\'' in output
	u_boot_console.run_command(cmd='setenv efi_selftest device tree')
	u_boot_console.run_command(cmd='setenv -f serial# Testing DT')
	u_boot_console.run_command(cmd='bootefi selftest ${fdtcontroladdr}', wait_for_prompt=False)
	m = u_boot_console.p.expect(['serial-number: Testing DT', 'U-Boot'])
	if m != 0:
		raise Exception('Reset failed in \'device tree\' test')
	u_boot_console.restart_uboot();

@pytest.mark.buildconfigspec('cmd_bootefi_selftest')
def test_efi_selftest_watchdog_reboot(u_boot_console):
	u_boot_console.run_command(cmd='setenv efi_selftest list')
	output = u_boot_console.run_command('bootefi selftest')
	assert '\'watchdog reboot\'' in output
	u_boot_console.run_command(cmd='setenv efi_selftest watchdog reboot')
	u_boot_console.run_command(cmd='bootefi selftest', wait_for_prompt=False)
	m = u_boot_console.p.expect(['resetting', 'U-Boot'])
	if m != 0:
		raise Exception('Reset failed in \'watchdog reboot\' test')
	u_boot_console.restart_uboot();

@pytest.mark.buildconfigspec('cmd_bootefi_selftest')
def test_efi_selftest_text_input(u_boot_console):
	"""Test the EFI_SIMPLE_TEXT_INPUT_PROTOCOL

	:param u_boot_console: U-Boot console

	This function calls the text input EFI selftest.
	"""
	u_boot_console.run_command(cmd='setenv efi_selftest text input')
	output = u_boot_console.run_command(cmd='bootefi selftest',
					    wait_for_prompt=False)
	m = u_boot_console.p.expect(['To terminate type \'x\''])
	if m != 0:
		raise Exception('No prompt for \'text input\' test')
	u_boot_console.drain_console()
	u_boot_console.p.timeout = 500
	# EOT
	u_boot_console.run_command(cmd=chr(4), wait_for_echo=False,
				   send_nl=False, wait_for_prompt=False)
	m = u_boot_console.p.expect(
		['Unicode char 4 \(unknown\), scan code 0 \(Null\)'])
	if m != 0:
		raise Exception('EOT failed in \'text input\' test')
	u_boot_console.drain_console()
	# BS
	u_boot_console.run_command(cmd=chr(8), wait_for_echo=False,
				   send_nl=False, wait_for_prompt=False)
	m = u_boot_console.p.expect(
		['Unicode char 8 \(BS\), scan code 0 \(Null\)'])
	if m != 0:
		raise Exception('BS failed in \'text input\' test')
	u_boot_console.drain_console()
	# TAB
	u_boot_console.run_command(cmd=chr(9), wait_for_echo=False,
				   send_nl=False, wait_for_prompt=False)
	m = u_boot_console.p.expect(
		['Unicode char 9 \(TAB\), scan code 0 \(Null\)'])
	if m != 0:
		raise Exception('BS failed in \'text input\' test')
	u_boot_console.drain_console()
	# a
	u_boot_console.run_command(cmd='a', wait_for_echo=False, send_nl=False,
				   wait_for_prompt=False)
	m = u_boot_console.p.expect(
		['Unicode char 97 \(\'a\'\), scan code 0 \(Null\)'])
	if m != 0:
		raise Exception('\'a\' failed in \'text input\' test')
	u_boot_console.drain_console()
	# UP escape sequence
	u_boot_console.run_command(cmd=chr(27) + '[A', wait_for_echo=False,
				   send_nl=False, wait_for_prompt=False)
	m = u_boot_console.p.expect(
		['Unicode char 0 \(Null\), scan code 1 \(Up\)'])
	if m != 0:
		raise Exception('UP failed in \'text input\' test')
	u_boot_console.drain_console()
	# Euro sign
	u_boot_console.run_command(cmd='\xe2\x82\xac', wait_for_echo=False,
				   send_nl=False, wait_for_prompt=False)
	m = u_boot_console.p.expect(['Unicode char 8364 \(\''])
	if m != 0:
		raise Exception('Euro sign failed in \'text input\' test')
	u_boot_console.drain_console()
	u_boot_console.run_command(cmd='x', wait_for_echo=False, send_nl=False,
				   wait_for_prompt=False)
	m = u_boot_console.p.expect(['Summary: 0 failures', 'Press any key'])
	if m != 0:
		raise Exception('Failures occurred during the EFI selftest')
	u_boot_console.restart_uboot();

@pytest.mark.buildconfigspec('cmd_bootefi_selftest')
def test_efi_selftest_text_input_ex(u_boot_console):
	"""Test the EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL

	:param u_boot_console: U-Boot console

	This function calls the extended text input EFI selftest.
	"""
	u_boot_console.run_command(cmd='setenv efi_selftest extended text input')
	output = u_boot_console.run_command(cmd='bootefi selftest',
					    wait_for_prompt=False)
	m = u_boot_console.p.expect(['To terminate type \'CTRL\+x\''])
	if m != 0:
		raise Exception('No prompt for \'text input\' test')
	u_boot_console.drain_console()
	u_boot_console.p.timeout = 500
	# EOT
	u_boot_console.run_command(cmd=chr(4), wait_for_echo=False,
				   send_nl=False, wait_for_prompt=False)
	m = u_boot_console.p.expect(
		['Unicode char 100 \\(\'d\'\\), scan code 0 \\(CTRL\\+Null\\)'])
	if m != 0:
		raise Exception('EOT failed in \'text input\' test')
	u_boot_console.drain_console()
	# BS
	u_boot_console.run_command(cmd=chr(8), wait_for_echo=False,
				   send_nl=False, wait_for_prompt=False)
	m = u_boot_console.p.expect(
		['Unicode char 8 \(BS\), scan code 0 \(\+Null\)'])
	if m != 0:
		raise Exception('BS failed in \'text input\' test')
	u_boot_console.drain_console()
	# TAB
	u_boot_console.run_command(cmd=chr(9), wait_for_echo=False,
				   send_nl=False, wait_for_prompt=False)
	m = u_boot_console.p.expect(
		['Unicode char 9 \(TAB\), scan code 0 \(\+Null\)'])
	if m != 0:
		raise Exception('TAB failed in \'text input\' test')
	u_boot_console.drain_console()
	# a
	u_boot_console.run_command(cmd='a', wait_for_echo=False, send_nl=False,
				   wait_for_prompt=False)
	m = u_boot_console.p.expect(
		['Unicode char 97 \(\'a\'\), scan code 0 \(Null\)'])
	if m != 0:
		raise Exception('\'a\' failed in \'text input\' test')
	u_boot_console.drain_console()
	# UP escape sequence
	u_boot_console.run_command(cmd=chr(27) + '[A', wait_for_echo=False,
				   send_nl=False, wait_for_prompt=False)
	m = u_boot_console.p.expect(
		['Unicode char 0 \(Null\), scan code 1 \(\+Up\)'])
	if m != 0:
		raise Exception('UP failed in \'text input\' test')
	u_boot_console.drain_console()
	# Euro sign
	u_boot_console.run_command(cmd='\xe2\x82\xac', wait_for_echo=False,
				   send_nl=False, wait_for_prompt=False)
	m = u_boot_console.p.expect(['Unicode char 8364 \(\''])
	if m != 0:
		raise Exception('Euro sign failed in \'text input\' test')
	u_boot_console.drain_console()
	# SHIFT+ALT+FN 5
	u_boot_console.run_command(cmd='\x1b\x5b\x31\x35\x3b\x34\x7e',
				   wait_for_echo=False, send_nl=False,
				   wait_for_prompt=False)
	m = u_boot_console.p.expect(
		['Unicode char 0 \(Null\), scan code 15 \(SHIFT\+ALT\+FN 5\)'])
	if m != 0:
		raise Exception('SHIFT+ALT+FN 5 failed in \'text input\' test')
	u_boot_console.drain_console()
	u_boot_console.run_command(cmd=chr(24), wait_for_echo=False, send_nl=False,
				   wait_for_prompt=False)
	m = u_boot_console.p.expect(['Summary: 0 failures', 'Press any key'])
	if m != 0:
		raise Exception('Failures occurred during the EFI selftest')
	u_boot_console.restart_uboot();
