# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.

# Test various network-related functionality, such as the dhcp, ping, and
# tftpboot commands.

import pytest
import u_boot_utils

"""
Note: This test relies on boardenv_* containing configuration values to define
which the network environment available for testing. Without this, this test
will be automatically skipped.

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
env__net_tftp_readable_file = {
    'fn': 'ubtest-readable.bin',
    'addr': 0x10000000,
    'size': 5058624,
    'crc32': 'c2244b26',
}

# Details regarding a file that may be read from a NFS server. This variable
# may be omitted or set to None if NFS testing is not possible or desired.
env__net_nfs_readable_file = {
    'fn': 'ubtest-readable.bin',
    'addr': 0x10000000,
    'size': 5058624,
    'crc32': 'c2244b26',
}
"""

net_set_up = False

def test_net_pre_commands(u_boot_console):
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
def test_net_dhcp(u_boot_console):
    """Test the dhcp command.

    The boardenv_* file may be used to enable/disable this test; see the
    comment at the beginning of this file.
    """

    test_dhcp = u_boot_console.config.env.get('env__net_dhcp_server', False)
    if not test_dhcp:
        pytest.skip('No DHCP server available')

    u_boot_console.run_command('setenv autoload no')
    output = u_boot_console.run_command('dhcp')
    assert 'DHCP client bound to address ' in output

    global net_set_up
    net_set_up = True

@pytest.mark.buildconfigspec('net')
def test_net_setup_static(u_boot_console):
    """Set up a static IP configuration.

    The configuration is provided by the boardenv_* file; see the comment at
    the beginning of this file.
    """

    env_vars = u_boot_console.config.env.get('env__net_static_env_vars', None)
    if not env_vars:
        pytest.skip('No static network configuration is defined')

    for (var, val) in env_vars:
        u_boot_console.run_command('setenv %s %s' % (var, val))

    global net_set_up
    net_set_up = True

@pytest.mark.buildconfigspec('cmd_ping')
def test_net_ping(u_boot_console):
    """Test the ping command.

    The $serverip (as set up by either test_net_dhcp or test_net_setup_static)
    is pinged. The test validates that the host is alive, as reported by the
    ping command's output.
    """

    if not net_set_up:
        pytest.skip('Network not initialized')

    output = u_boot_console.run_command('ping $serverip')
    assert 'is alive' in output

@pytest.mark.buildconfigspec('cmd_net')
def test_net_tftpboot(u_boot_console):
    """Test the tftpboot command.

    A file is downloaded from the TFTP server, its size and optionally its
    CRC32 are validated.

    The details of the file to download are provided by the boardenv_* file;
    see the comment at the beginning of this file.
    """

    if not net_set_up:
        pytest.skip('Network not initialized')

    f = u_boot_console.config.env.get('env__net_tftp_readable_file', None)
    if not f:
        pytest.skip('No TFTP readable file to read')

    addr = f.get('addr', None)

    fn = f['fn']
    if not addr:
        output = u_boot_console.run_command('tftpboot %s' % (fn))
    else:
        output = u_boot_console.run_command('tftpboot %x %s' % (addr, fn))
    expected_text = 'Bytes transferred = '
    sz = f.get('size', None)
    if sz:
        expected_text += '%d' % sz
    assert expected_text in output

    expected_crc = f.get('crc32', None)
    if not expected_crc:
        return

    if u_boot_console.config.buildconfig.get('config_cmd_crc32', 'n') != 'y':
        return

    output = u_boot_console.run_command('crc32 $fileaddr $filesize')
    assert expected_crc in output

@pytest.mark.buildconfigspec('cmd_nfs')
def test_net_nfs(u_boot_console):
    """Test the nfs command.

    A file is downloaded from the NFS server, its size and optionally its
    CRC32 are validated.

    The details of the file to download are provided by the boardenv_* file;
    see the comment at the beginning of this file.
    """

    if not net_set_up:
        pytest.skip('Network not initialized')

    f = u_boot_console.config.env.get('env__net_nfs_readable_file', None)
    if not f:
        pytest.skip('No NFS readable file to read')

    addr = f.get('addr', None)
    if not addr:
        addr = u_boot_utils.find_ram_base(u_boot_console)

    fn = f['fn']
    output = u_boot_console.run_command('nfs %x %s' % (addr, fn))
    expected_text = 'Bytes transferred = '
    sz = f.get('size', None)
    if sz:
        expected_text += '%d' % sz
    assert expected_text in output

    expected_crc = f.get('crc32', None)
    if not expected_crc:
        return

    if u_boot_console.config.buildconfig.get('config_cmd_crc32', 'n') != 'y':
        return

    output = u_boot_console.run_command('crc32 %x $filesize' % addr)
    assert expected_crc in output
