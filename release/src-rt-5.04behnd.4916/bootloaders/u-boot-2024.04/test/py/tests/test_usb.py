# SPDX-License-Identifier: GPL-2.0
# (C) Copyright 2023, Advanced Micro Devices, Inc.

import pytest
import random
import re
import u_boot_utils

"""
Note: This test doesn't rely on boardenv_* configuration values but it can
change the test behavior. To test USB file system cases (fat32, ext2, ext4),
USB device should be formatted and valid partitions should be created for
different file system, otherwise it may leads to failure. This test will be
skipped if the USB device is not detected.

For example:

# Setup env__usb_device_test_skip to not skipping the test. By default, its
# value is set to True. Set it to False to run all tests for USB device.
env__usb_device_test_skip = False
"""

def setup_usb(u_boot_console):
    if u_boot_console.config.env.get('env__usb_device_test_skip', True):
        pytest.skip('USB device test is not enabled')

@pytest.mark.buildconfigspec('cmd_usb')
def test_usb_start(u_boot_console):
    setup_usb(u_boot_console)
    output = u_boot_console.run_command('usb start')

    # if output is empty, usb start may already run as part of preboot command
    # re-start the usb, in that case
    if not output:
        u_boot_console.run_command('usb stop')
        output = u_boot_console.run_command('usb start')

    if 'No USB device found' in output:
        pytest.skip('No USB controller available')

    if 'Card did not respond to voltage select' in output:
        pytest.skip('No USB device present')

    controllers = 0
    storage_device = 0
    obj = re.search(r'\d USB Device\(s\) found', output)
    controllers = int(obj.group()[0])

    if not controllers:
        pytest.skip('No USB device present')

    obj = re.search(r'\d Storage Device\(s\) found', output)
    storage_device = int(obj.group()[0])

    if not storage_device:
        pytest.skip('No USB storage device present')

    assert 'USB init failed' not in output
    assert 'starting USB...' in output

    if 'Starting the controller' in output:
        assert 'USB XHCI' in output

    output = u_boot_console.run_command('echo $?')
    assert output.endswith('0')
    return controllers, storage_device

@pytest.mark.buildconfigspec('cmd_usb')
def test_usb_stop(u_boot_console):
    setup_usb(u_boot_console)
    output = u_boot_console.run_command('usb stop')
    assert 'stopping USB..' in output

    output = u_boot_console.run_command('echo $?')
    assert output.endswith('0')

    output = u_boot_console.run_command('usb dev')
    assert "USB is stopped. Please issue 'usb start' first." in output

@pytest.mark.buildconfigspec('cmd_usb')
def test_usb_reset(u_boot_console):
    setup_usb(u_boot_console)
    output = u_boot_console.run_command('usb reset')

    if 'No USB device found' in output:
        pytest.skip('No USB controller available')

    if 'Card did not respond to voltage select' in output:
        pytest.skip('No USB device present')

    obj = re.search(r'\d USB Device\(s\) found', output)
    usb_dev_num = int(obj.group()[0])

    if not usb_dev_num:
        pytest.skip('No USB device present')

    obj = re.search(r'\d Storage Device\(s\) found', output)
    usb_stor_num = int(obj.group()[0])

    if not usb_stor_num:
        pytest.skip('No USB storage device present')

    assert 'BUG' not in output
    assert 'USB init failed' not in output
    assert 'resetting USB...' in output

    if 'Starting the controller' in output:
        assert 'USB XHCI' in output

    output = u_boot_console.run_command('echo $?')
    assert output.endswith('0')

@pytest.mark.buildconfigspec('cmd_usb')
def test_usb_info(u_boot_console):
    controllers, storage_device = test_usb_start(u_boot_console)
    output = u_boot_console.run_command('usb info')

    num_controller = len(re.findall(': Hub,', output))
    num_mass_storage = len(re.findall(': Mass Storage,', output))

    assert num_controller == controllers - 1
    assert num_mass_storage == storage_device

    output = u_boot_console.run_command('echo $?')
    assert output.endswith('0')

    for i in range(0, storage_device + controllers - 1):
        output = u_boot_console.run_command('usb info %d' % i)
        num_controller = len(re.findall(': Hub,', output))
        num_mass_storage = len(re.findall(': Mass Storage,', output))
        assert num_controller + num_mass_storage == 1
        assert 'No device available' not in output
        output = u_boot_console.run_command('echo $?')
        assert output.endswith('0')

@pytest.mark.buildconfigspec('cmd_usb')
def test_usb_tree(u_boot_console):
    controllers, storage_device = test_usb_start(u_boot_console)
    output = u_boot_console.run_command('usb tree')

    num_controller = len(re.findall('Hub', output))
    num_mass_storage = len(re.findall('Mass Storage', output))

    assert num_controller == controllers - 1
    assert num_mass_storage == storage_device

    output = u_boot_console.run_command('echo $?')
    assert output.endswith('0')

@pytest.mark.buildconfigspec('cmd_usb')
@pytest.mark.buildconfigspec('usb_storage')
def test_usb_storage(u_boot_console):
    controllers, storage_device = test_usb_start(u_boot_console)
    output = u_boot_console.run_command('usb storage')

    obj = re.findall(r'Capacity: (\d+|\d+[\.]?\d)', output)
    devices = {}

    for key in range(int(storage_device)):
        devices[key] = {}

    for x in range(int(storage_device)):
        try:
            capacity = float(obj[x].split()[0])
            devices[x]['capacity'] = capacity
            print('USB storage device %d capacity is: %g MB' % (x, capacity))
        except ValueError:
            pytest.fail('USB storage device capacity not recognized')

    output = u_boot_console.run_command('echo $?')
    assert output.endswith('0')

@pytest.mark.buildconfigspec('cmd_usb')
def test_usb_dev(u_boot_console):
    controllers, storage_device = test_usb_start(u_boot_console)
    output = u_boot_console.run_command('usb dev')

    assert 'no usb devices available' not in output

    output = u_boot_console.run_command('echo $?')
    assert output.endswith('0')

    devices = {}

    for key in range(int(storage_device)):
        devices[key] = {}

    fail = 0
    for x in range(0, storage_device):
        devices[x]['detected'] = 'yes'
        output = u_boot_console.run_command('usb dev %d' % x)

        if 'Card did not respond to voltage select' in output:
            fail = 1
            devices[x]['detected'] = 'no'

        if 'No USB device found' in output:
            devices[x]['detected'] = 'no'

        if 'unknown device' in output:
            devices[x]['detected'] = 'no'

        assert 'is now current device' in output
        output = u_boot_console.run_command('echo $?')
        assert output.endswith('0')

    if fail:
        pytest.fail('USB device not present')

    return devices, controllers, storage_device

@pytest.mark.buildconfigspec('cmd_usb')
def test_usb_part(u_boot_console):
    devices, controllers, storage_device = test_usb_dev(u_boot_console)
    if not devices:
        pytest.skip('No devices detected')

    u_boot_console.run_command('usb part')

    output = u_boot_console.run_command('echo $?')
    assert output.endswith('0')

    for i in range(0, storage_device):
        if devices[i]['detected'] == 'yes':
            u_boot_console.run_command('usb dev %d' % i)
            output = u_boot_console.run_command('usb part')

            lines = output.split('\n')
            part_fat = []
            part_ext = []
            for line in lines:
                obj = re.search(r'(\d)\s+\d+\s+\d+\s+\w+\d+\w+-\d+\s+(\d+\w+)', line)
                if obj:
                    part_id = int(obj.groups()[0])
                    part_type = obj.groups()[1]
                    print('part_id:%d, part_type:%s' % (part_id, part_type))

                    if part_type == '0c' or part_type == '0b' or part_type == '0e':
                        print('Fat detected')
                        part_fat.append(part_id)
                    elif part_type == '83':
                        print('ext detected')
                        part_ext.append(part_id)
                    else:
                        pytest.fail('Unsupported Filesystem on device %d' % i)
            devices[i]['ext4'] = part_ext
            devices[i]['ext2'] = part_ext
            devices[i]['fat'] = part_fat

            if not part_ext and not part_fat:
                pytest.fail('No partition detected on device %d' % i)

    return devices, controllers, storage_device

@pytest.mark.buildconfigspec('cmd_usb')
@pytest.mark.buildconfigspec('cmd_fat')
def test_usb_fatls_fatinfo(u_boot_console):
    devices, controllers, storage_device = test_usb_part(u_boot_console)
    if not devices:
        pytest.skip('No devices detected')

    part_detect = 0
    fs = 'fat'
    for x in range(0, int(storage_device)):
        if devices[x]['detected'] == 'yes':
            u_boot_console.run_command('usb dev %d' % x)
            try:
                partitions = devices[x][fs]
            except:
                print('No %s table on this device' % fs.upper())
                continue

            for part in partitions:
                output = u_boot_console.run_command('fatls usb %d:%s' % (x, part))
                if 'Unrecognized filesystem type' in output:
                    partitions.remove(part)
                    pytest.fail('Unrecognized filesystem')

                if not re.search(r'\d file\(s\), \d dir\(s\)', output):
                    pytest.fail('%s read failed on device %d' % (fs.upper, x))

                output = u_boot_console.run_command('fatinfo usb %d:%s' % (x, part))
                string = 'Filesystem: %s' % fs.upper
                if re.search(string, output):
                    pytest.fail('%s FS failed on device %d' % (fs.upper(), x))
                part_detect = 1

    if not part_detect:
        pytest.skip('No %s partition detected' % fs.upper())

@pytest.mark.buildconfigspec('cmd_usb')
@pytest.mark.buildconfigspec('cmd_fat')
@pytest.mark.buildconfigspec('cmd_memory')
def test_usb_fatload_fatwrite(u_boot_console):
    devices, controllers, storage_device = test_usb_part(u_boot_console)
    if not devices:
        pytest.skip('No devices detected')

    part_detect = 0
    fs = 'fat'
    for x in range(0, int(storage_device)):
        if devices[x]['detected'] == 'yes':
            u_boot_console.run_command('usb dev %d' % x)
            try:
                partitions = devices[x][fs]
            except:
                print('No %s table on this device' % fs.upper())
                continue

            for part in partitions:
                part_detect = 1
                addr = u_boot_utils.find_ram_base(u_boot_console)
                size = random.randint(4, 1 * 1024 * 1024)
                output = u_boot_console.run_command('crc32 %x %x' % (addr, size))
                m = re.search('==> (.+?)', output)
                if not m:
                    pytest.fail('CRC32 failed')
                expected_crc32 = m.group(1)

                file = '%s_%d' % ('uboot_test', size)
                output = u_boot_console.run_command(
                    '%swrite usb %d:%s %x %s %x' % (fs, x, part, addr, file, size)
                )
                assert 'Unable to write' not in output
                assert 'Error' not in output
                assert 'overflow' not in output
                expected_text = '%d bytes written' % size
                assert expected_text in output

                alignment = int(
                    u_boot_console.config.buildconfig.get(
                        'config_sys_cacheline_size', 128
                    )
                )
                offset = random.randrange(alignment, 1024, alignment)
                output = u_boot_console.run_command(
                    '%sload usb %d:%s %x %s' % (fs, x, part, addr + offset, file)
                )
                assert 'Invalid FAT entry' not in output
                assert 'Unable to read file' not in output
                assert 'Misaligned buffer address' not in output
                expected_text = '%d bytes read' % size
                assert expected_text in output

                output = u_boot_console.run_command(
                    'crc32 %x $filesize' % (addr + offset)
                )
                assert expected_crc32 in output

    if not part_detect:
        pytest.skip('No %s partition detected' % fs.upper())

    return file, size

@pytest.mark.buildconfigspec('cmd_usb')
@pytest.mark.buildconfigspec('cmd_ext4')
def test_usb_ext4ls(u_boot_console):
    devices, controllers, storage_device = test_usb_part(u_boot_console)
    if not devices:
        pytest.skip('No devices detected')

    part_detect = 0
    fs = 'ext4'
    for x in range(0, int(storage_device)):
        if devices[x]['detected'] == 'yes':
            try:
                partitions = devices[x][fs]
            except:
                print('No %s table on this device' % fs.upper())
                continue

            u_boot_console.run_command('usb dev %d' % x)
            for part in partitions:
                output = u_boot_console.run_command('%sls usb %d:%s' % (fs, x, part))
                if 'Unrecognized filesystem type' in output:
                    partitions.remove(part)
                    pytest.fail('Unrecognized filesystem')
                part_detect = 1

    if not part_detect:
        pytest.skip('No %s partition detected' % fs.upper())

@pytest.mark.buildconfigspec('cmd_usb')
@pytest.mark.buildconfigspec('cmd_ext4')
@pytest.mark.buildconfigspec('ext4_write')
@pytest.mark.buildconfigspec('cmd_memory')
def test_usb_ext4load_ext4write(u_boot_console):
    devices, controllers, storage_device = test_usb_part(u_boot_console)
    if not devices:
        pytest.skip('No devices detected')

    part_detect = 0
    fs = 'ext4'
    for x in range(0, int(storage_device)):
        if devices[x]['detected'] == 'yes':
            u_boot_console.run_command('usb dev %d' % x)
            try:
                partitions = devices[x][fs]
            except:
                print('No %s table on this device' % fs.upper())
                continue

            for part in partitions:
                part_detect = 1
                addr = u_boot_utils.find_ram_base(u_boot_console)
                size = random.randint(4, 1 * 1024 * 1024)
                output = u_boot_console.run_command('crc32 %x %x' % (addr, size))
                m = re.search('==> (.+?)', output)
                if not m:
                    pytest.fail('CRC32 failed')
                expected_crc32 = m.group(1)
                file = '%s_%d' % ('uboot_test', size)

                output = u_boot_console.run_command(
                    '%swrite usb %d:%s %x /%s %x' % (fs, x, part, addr, file, size)
                )
                assert 'Unable to write' not in output
                assert 'Error' not in output
                assert 'overflow' not in output
                expected_text = '%d bytes written' % size
                assert expected_text in output

                offset = random.randrange(128, 1024, 128)
                output = u_boot_console.run_command(
                    '%sload usb %d:%s %x /%s' % (fs, x, part, addr + offset, file)
                )
                expected_text = '%d bytes read' % size
                assert expected_text in output

                output = u_boot_console.run_command(
                    'crc32 %x $filesize' % (addr + offset)
                )
                assert expected_crc32 in output

    if not part_detect:
        pytest.skip('No %s partition detected' % fs.upper())

    return file, size

@pytest.mark.buildconfigspec('cmd_usb')
@pytest.mark.buildconfigspec('cmd_ext2')
def test_usb_ext2ls(u_boot_console):
    devices, controllers, storage_device = test_usb_part(u_boot_console)
    if not devices:
        pytest.skip('No devices detected')

    part_detect = 0
    fs = 'ext2'
    for x in range(0, int(storage_device)):
        if devices[x]['detected'] == 'yes':
            u_boot_console.run_command('usb dev %d' % x)
            try:
                partitions = devices[x][fs]
            except:
                print('No %s table on this device' % fs.upper())
                continue

            for part in partitions:
                part_detect = 1
                output = u_boot_console.run_command('%sls usb %d:%s' % (fs, x, part))
                if 'Unrecognized filesystem type' in output:
                    partitions.remove(part)
                    pytest.fail('Unrecognized filesystem')
                part_detect = 1

    if not part_detect:
        pytest.skip('No %s partition detected' % fs.upper())

@pytest.mark.buildconfigspec('cmd_usb')
@pytest.mark.buildconfigspec('cmd_ext2')
@pytest.mark.buildconfigspec('cmd_ext4')
@pytest.mark.buildconfigspec('ext4_write')
@pytest.mark.buildconfigspec('cmd_memory')
def test_usb_ext2load(u_boot_console):
    devices, controllers, storage_device = test_usb_part(u_boot_console)
    file, size = test_usb_ext4load_ext4write(u_boot_console)

    if not devices:
        pytest.skip('No devices detected')

    part_detect = 0
    fs = 'ext2'
    for x in range(0, int(storage_device)):
        if devices[x]['detected'] == 'yes':
            u_boot_console.run_command('usb dev %d' % x)
            try:
                partitions = devices[x][fs]
            except:
                print('No %s table on this device' % fs.upper())
                continue

            for part in partitions:
                part_detect = 1
                addr = u_boot_utils.find_ram_base(u_boot_console)
                output = u_boot_console.run_command('crc32 %x %x' % (addr, size))
                m = re.search('==> (.+?)', output)
                if not m:
                    pytest.fail('CRC32 failed')
                expected_crc32 = m.group(1)

                offset = random.randrange(128, 1024, 128)
                output = u_boot_console.run_command(
                    '%sload usb %d:%s %x /%s' % (fs, x, part, addr + offset, file)
                )
                expected_text = '%d bytes read' % size
                assert expected_text in output

                output = u_boot_console.run_command(
                    'crc32 %x $filesize' % (addr + offset)
                )
                assert expected_crc32 in output

    if not part_detect:
        pytest.skip('No %s partition detected' % fs.upper())

@pytest.mark.buildconfigspec('cmd_usb')
@pytest.mark.buildconfigspec('cmd_fs_generic')
def test_usb_ls(u_boot_console):
    devices, controllers, storage_device = test_usb_part(u_boot_console)
    if not devices:
        pytest.skip('No devices detected')

    part_detect = 0
    for x in range(0, int(storage_device)):
        if devices[x]['detected'] == 'yes':
            u_boot_console.run_command('usb dev %d' % x)
            for fs in ['fat', 'ext4']:
                try:
                    partitions = devices[x][fs]
                except:
                    print('No %s table on this device' % fs.upper())
                    continue

                for part in partitions:
                    part_detect = 1
                    output = u_boot_console.run_command('ls usb %d:%s' % (x, part))
                    if re.search(r'No \w+ table on this device', output):
                        pytest.fail(
                            '%s: Partition table not found %d' % (fs.upper(), x)
                        )

    if not part_detect:
        pytest.skip('No partition detected')

@pytest.mark.buildconfigspec('cmd_usb')
@pytest.mark.buildconfigspec('cmd_fs_generic')
def test_usb_load(u_boot_console):
    devices, controllers, storage_device = test_usb_part(u_boot_console)
    if not devices:
        pytest.skip('No devices detected')

    part_detect = 0
    for x in range(0, int(storage_device)):
        if devices[x]['detected'] == 'yes':
            u_boot_console.run_command('usb dev %d' % x)
            for fs in ['fat', 'ext4']:
                try:
                    partitions = devices[x][fs]
                except:
                    print('No %s table on this device' % fs.upper())
                    continue

                for part in partitions:
                    part_detect = 1
                    addr = u_boot_utils.find_ram_base(u_boot_console)

                    if fs == 'fat':
                        file, size = test_usb_fatload_fatwrite(u_boot_console)
                    elif fs == 'ext4':
                        file, size = test_usb_ext4load_ext4write(u_boot_console)

                    output = u_boot_console.run_command('crc32 %x %x' % (addr, size))
                    m = re.search('==> (.+?)', output)
                    if not m:
                        pytest.fail('CRC32 failed')
                    expected_crc32 = m.group(1)

                    offset = random.randrange(128, 1024, 128)
                    output = u_boot_console.run_command(
                        'load usb %d:%s %x /%s' % (x, part, addr + offset, file)
                    )
                    expected_text = '%d bytes read' % size
                    assert expected_text in output

                    output = u_boot_console.run_command(
                        'crc32 %x $filesize' % (addr + offset)
                    )
                    assert expected_crc32 in output

    if not part_detect:
        pytest.skip('No partition detected')

@pytest.mark.buildconfigspec('cmd_usb')
@pytest.mark.buildconfigspec('cmd_fs_generic')
def test_usb_save(u_boot_console):
    devices, controllers, storage_device = test_usb_part(u_boot_console)
    if not devices:
        pytest.skip('No devices detected')

    part_detect = 0
    for x in range(0, int(storage_device)):
        if devices[x]['detected'] == 'yes':
            u_boot_console.run_command('usb dev %d' % x)
            for fs in ['fat', 'ext4']:
                try:
                    partitions = devices[x][fs]
                except:
                    print('No %s table on this device' % fs.upper())
                    continue

                for part in partitions:
                    part_detect = 1
                    addr = u_boot_utils.find_ram_base(u_boot_console)
                    size = random.randint(4, 1 * 1024 * 1024)
                    file = '%s_%d' % ('uboot_test', size)

                    offset = random.randrange(128, 1024, 128)
                    output = u_boot_console.run_command(
                        'save usb %d:%s %x /%s %x'
                        % (x, part, addr + offset, file, size)
                    )
                    expected_text = '%d bytes written' % size
                    assert expected_text in output

    if not part_detect:
        pytest.skip('No partition detected')
