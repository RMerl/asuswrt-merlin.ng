# SPDX-License-Identifier: GPL-2.0
# (C) Copyright 2023, Advanced Micro Devices, Inc.

import pytest
import re
import u_boot_utils
import test_net

"""
This test verifies different type of secure boot images loaded at the DDR for
AMD's ZynqMP SoC.

Note: This test relies on boardenv_* containing configuration values to define
the files to be used for testing. Without this, this test will be automatically
skipped. It also relies on dhcp or setup_static net test to support tftp to
load files from a TFTP server.

For example:

# Details regarding the files that may be read from a TFTP server. This
# variable may be omitted or set to None if zynqmp secure testing is not
# possible or desired.
env__zynqmp_secure_readable_file = {
    'fn': 'auth_bhdr_ppk1.bin',
    'enckupfn': 'auth_bhdr_enc_kup_load.bin',
    'addr': 0x1000000,
    'keyaddr': 0x100000,
    'keyfn': 'aes.txt',
}
"""

@pytest.mark.buildconfigspec('cmd_zynqmp')
def test_zynqmp_secure_boot_image(u_boot_console):
    """This test verifies secure boot image at the DDR address for
    authentication only case.
    """

    f = u_boot_console.config.env.get('env__zynqmp_secure_readable_file', None)
    if not f:
        pytest.skip('No TFTP readable file for zynqmp secure cases to read')

    test_net.test_net_dhcp(u_boot_console)
    if not test_net.net_set_up:
        test_net.test_net_setup_static(u_boot_console)

    addr = f.get('addr', None)
    if not addr:
        addr = u_boot_utils.find_ram_base(u_boot_console)

    expected_tftp = 'Bytes transferred = '
    fn = f['fn']
    output = u_boot_console.run_command('tftpboot %x %s' % (addr, fn))
    assert expected_tftp in output

    output = u_boot_console.run_command('zynqmp secure %x $filesize' % (addr))
    assert 'Verified image at' in output
    ver_addr = re.search(r'Verified image at 0x(.+)', output).group(1)
    output = u_boot_console.run_command('echo $?')
    assert output.endswith('0')
    output = u_boot_console.run_command('print zynqmp_verified_img_addr')
    assert f'zynqmp_verified_img_addr={ver_addr}' in output
    assert 'Error' not in output


@pytest.mark.buildconfigspec('cmd_zynqmp')
def test_zynqmp_secure_boot_img_kup(u_boot_console):
    """This test verifies secure boot image at the DDR address for encryption
    with kup key case.
    """

    f = u_boot_console.config.env.get('env__zynqmp_secure_readable_file', None)
    if not f:
        pytest.skip('No TFTP readable file for zynqmp secure cases to read')

    test_net.test_net_dhcp(u_boot_console)
    if not test_net.net_set_up:
        test_net.test_net_setup_static(u_boot_console)

    keyaddr = f.get('keyaddr', None)
    if not keyaddr:
        addr = u_boot_utils.find_ram_base(u_boot_console)
    expected_tftp = 'Bytes transferred = '
    keyfn = f['keyfn']
    output = u_boot_console.run_command('tftpboot %x %s' % (keyaddr, keyfn))
    assert expected_tftp in output

    addr = f.get('addr', None)
    if not addr:
        addr = u_boot_utils.find_ram_base(u_boot_console)
    expected_tftp = 'Bytes transferred = '
    fn = f['enckupfn']
    output = u_boot_console.run_command('tftpboot %x %s' % (addr, fn))
    assert expected_tftp in output

    output = u_boot_console.run_command(
        'zynqmp secure %x $filesize %x' % (addr, keyaddr)
    )
    assert 'Verified image at' in output
    ver_addr = re.search(r'Verified image at 0x(.+)', output).group(1)
    output = u_boot_console.run_command('echo $?')
    assert output.endswith('0')
    output = u_boot_console.run_command('print zynqmp_verified_img_addr')
    assert f'zynqmp_verified_img_addr={ver_addr}' in output
    assert 'Error' not in output
