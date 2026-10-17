# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.
# Copyright (c) 2016, Alexander Graf <agraf@suse.de>
#
# based on test_net.py.

# Test efi loader implementation

"""
Note: This test relies on boardenv_* containing configuration values to define
which network environment is available for testing. Without this, the parts
that rely on network will be automatically skipped.

For example:

# Boolean indicating whether the Ethernet device is attached to USB, and hence
# USB enumeration needs to be performed prior to network tests.
# This variable may be omitted if its value is False.
env__net_uses_usb = False

# Boolean indicating whether the Ethernet device is attached to PCI, and hence
# PCI enumeration needs to be performed prior to network tests.
# This variable may be omitted if its value is False.
env__net_uses_pci = True

# True if a DHCP server is attached to the network, and should be tested.
# If DHCP testing is not possible or desired, this variable may be omitted or
# set to False.
env__net_dhcp_server = True

# A list of environment variables that should be set in order to configure a
# static IP. If solely relying on DHCP, this variable may be omitted or set to
# an empty list.
env__net_static_env_vars = [
    ('ipaddr', '10.0.0.100'),
    ('netmask', '255.255.255.0'),
    ('serverip', '10.0.0.1'),
]

# Details regarding a file that may be read from a TFTP server. This variable
# may be omitted or set to None if TFTP testing is not possible or desired.
env__efi_loader_helloworld_file = {
    'fn': 'lib/efi_loader/helloworld.efi', # file name
    'size': 5058624,                       # file length in bytes
    'crc32': 'c2244b26',                   # CRC32 check sum
    'addr': 0x40400000,                    # load address
}
"""

import pytest
import u_boot_utils

net_set_up = False

def test_efi_pre_commands(u_boot_console):
    """Execute any commands required to enable network hardware.

    These commands are provided by the boardenv_* file; see the comment at the
    beginning of this file.
    """

    init_usb = u_boot_console.config.env.get('env__net_uses_usb', False)
    if init_usb:
        u_boot_console.run_command('usb start')

    init_pci = u_boot_console.config.env.get('env__net_uses_pci', False)
    if init_pci:
        u_boot_console.run_command('pci enum')

@pytest.mark.buildconfigspec('cmd_dhcp')
def test_efi_setup_dhcp(u_boot_console):
    """Set up the network using DHCP.

    The boardenv_* file may be used to enable/disable this test; see the
    comment at the beginning of this file.
    """

    test_dhcp = u_boot_console.config.env.get('env__net_dhcp_server', False)
    if not test_dhcp:
        env_vars = u_boot_console.config.env.get('env__net_static_env_vars', None)
        if not env_vars:
            pytest.skip('No DHCP server available')
        return

    u_boot_console.run_command('setenv autoload no')
    output = u_boot_console.run_command('dhcp')
    assert 'DHCP client bound to address ' in output

    global net_set_up
    net_set_up = True

@pytest.mark.buildconfigspec('net')
def test_efi_setup_static(u_boot_console):
    """Set up the network using a static IP configuration.

    The configuration is provided by the boardenv_* file; see the comment at
    the beginning of this file.
    """

    env_vars = u_boot_console.config.env.get('env__net_static_env_vars', None)
    if not env_vars:
        test_dhcp = u_boot_console.config.env.get('env__net_dhcp_server', False)
        if not test_dhcp:
            pytest.skip('No static network configuration is defined')
        return None

    for (var, val) in env_vars:
        u_boot_console.run_command('setenv %s %s' % (var, val))

    global net_set_up
    net_set_up = True

def fetch_tftp_file(u_boot_console, env_conf):
    """Grab an env described file via TFTP and return its address

    A file as described by an env config <env_conf> is downloaded from the TFTP
    server. The address to that file is returned.
    """
    if not net_set_up:
        pytest.skip('Network not initialized')

    f = u_boot_console.config.env.get(env_conf, None)
    if not f:
        pytest.skip('No %s binary specified in environment' % env_conf)

    addr = f.get('addr', None)
    if not addr:
        addr = u_boot_utils.find_ram_base(u_boot_console)

    fn = f['fn']
    output = u_boot_console.run_command('tftpboot %x %s' % (addr, fn))
    expected_text = 'Bytes transferred = '
    sz = f.get('size', None)
    if sz:
        expected_text += '%d' % sz
    assert expected_text in output

    expected_crc = f.get('crc32', None)
    if not expected_crc:
        return addr

    if u_boot_console.config.buildconfig.get('config_cmd_crc32', 'n') != 'y':
        return addr

    output = u_boot_console.run_command('crc32 %x $filesize' % addr)
    assert expected_crc in output

    return addr

@pytest.mark.buildconfigspec('of_control')
@pytest.mark.buildconfigspec('cmd_bootefi_hello_compile')
def test_efi_helloworld_net(u_boot_console):
    """Run the helloworld.efi binary via TFTP.

    The helloworld.efi file is downloaded from the TFTP server and is executed
    using the fallback device tree at $fdtcontroladdr.
    """

    addr = fetch_tftp_file(u_boot_console, 'env__efi_loader_helloworld_file')

    output = u_boot_console.run_command('bootefi %x' % addr)
    expected_text = 'Hello, world'
    assert expected_text in output
    expected_text = '## Application failed'
    assert expected_text not in output

@pytest.mark.buildconfigspec('cmd_bootefi_hello')
def test_efi_helloworld_builtin(u_boot_console):
    """Run the builtin helloworld.efi binary.

    The helloworld.efi file is included in U-Boot, execute it using the
    special "bootefi hello" command.
    """

    output = u_boot_console.run_command('bootefi hello')
    expected_text = 'Hello, world'
    assert expected_text in output

@pytest.mark.buildconfigspec('of_control')
@pytest.mark.buildconfigspec('cmd_bootefi')
def test_efi_grub_net(u_boot_console):
    """Run the grub.efi binary via TFTP.

    The grub.efi file is downloaded from the TFTP server and gets
    executed.
    """

    addr = fetch_tftp_file(u_boot_console, 'env__efi_loader_grub_file')

    u_boot_console.run_command('bootefi %x' % addr, wait_for_prompt=False)

    # Verify that we have an SMBIOS table
    check_smbios = u_boot_console.config.env.get('env__efi_loader_check_smbios', False)
    if check_smbios:
        u_boot_console.wait_for('grub>')
        u_boot_console.run_command('lsefisystab', wait_for_prompt=False, wait_for_echo=False)
        u_boot_console.wait_for('SMBIOS')

    # Then exit cleanly
    u_boot_console.wait_for('grub>')
    u_boot_console.run_command('exit', wait_for_prompt=False, wait_for_echo=False)
    u_boot_console.wait_for(u_boot_console.prompt)
    # And give us our U-Boot prompt back
    u_boot_console.run_command('')
