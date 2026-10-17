# SPDX-License-Identifier: GPL-2.0
# (C) Copyright 2023, Advanced Micro Devices, Inc.

import pytest

"""
Note: This test relies on boardenv_* containing configuration values to define
the SCSI device number, type and capacity. This test will be automatically
skipped without this.

For example:

# Setup env__scsi_device_test to set the SCSI device number/slot, the type of
device, and the device capacity in MB.
env__scsi_device_test = {
    'dev_num': 0,
    'device_type': 'Hard Disk',
    'device_capacity': '476940.0 MB',
}
"""

def scsi_setup(u_boot_console):
    f = u_boot_console.config.env.get('env__scsi_device_test', None)
    if not f:
        pytest.skip('No SCSI device to test')

    dev_num = f.get('dev_num', None)
    if not isinstance(dev_num, int):
        pytest.skip('No device number specified in env file to read')

    dev_type = f.get('device_type')
    if not dev_type:
        pytest.skip('No device type specified in env file to read')

    dev_size = f.get('device_capacity')
    if not dev_size:
        pytest.skip('No device capacity specified in env file to read')

    return dev_num, dev_type, dev_size

@pytest.mark.buildconfigspec('cmd_scsi')
def test_scsi_reset(u_boot_console):
    dev_num, dev_type, dev_size = scsi_setup(u_boot_console)
    output = u_boot_console.run_command('scsi reset')
    assert f'Device {dev_num}:' in output
    assert f'Type: {dev_type}' in output
    assert f'Capacity: {dev_size}' in output
    output = u_boot_console.run_command('echo $?')
    assert output.endswith('0')

@pytest.mark.buildconfigspec('cmd_scsi')
def test_scsi_info(u_boot_console):
    dev_num, dev_type, dev_size = scsi_setup(u_boot_console)
    output = u_boot_console.run_command('scsi info')
    assert f'Device {dev_num}:' in output
    assert f'Type: {dev_type}' in output
    assert f'Capacity: {dev_size}' in output
    output = u_boot_console.run_command('echo $?')
    assert output.endswith('0')

@pytest.mark.buildconfigspec('cmd_scsi')
def test_scsi_scan(u_boot_console):
    dev_num, dev_type, dev_size = scsi_setup(u_boot_console)
    output = u_boot_console.run_command('scsi scan')
    assert f'Device {dev_num}:' in output
    assert f'Type: {dev_type}' in output
    assert f'Capacity: {dev_size}' in output
    output = u_boot_console.run_command('echo $?')
    assert output.endswith('0')

@pytest.mark.buildconfigspec('cmd_scsi')
def test_scsi_dev(u_boot_console):
    dev_num, dev_type, dev_size = scsi_setup(u_boot_console)
    output = u_boot_console.run_command('scsi device')
    assert 'no scsi devices available' not in output
    assert f'device {dev_num}:' in output
    assert f'Type: {dev_type}' in output
    assert f'Capacity: {dev_size}' in output
    output = u_boot_console.run_command('echo $?')
    assert output.endswith('0')
    output = u_boot_console.run_command('scsi device %d' % dev_num)
    assert 'is now current device' in output
    output = u_boot_console.run_command('echo $?')
    assert output.endswith('0')

@pytest.mark.buildconfigspec('cmd_scsi')
def test_scsi_part(u_boot_console):
    test_scsi_dev(u_boot_console)
    output = u_boot_console.run_command('scsi part')
    assert 'Partition Map for SCSI device' in output
    output = u_boot_console.run_command('echo $?')
    assert output.endswith('0')
