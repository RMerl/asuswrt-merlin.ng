# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2017 Alison Chaiken
# Copyright (c) 2017, NVIDIA CORPORATION. All rights reserved.

# Test GPT manipulation commands.

import os
import pytest
import u_boot_utils

"""
These tests rely on a 4 MB disk image, which is automatically created by
the test.
"""

# Mark all tests here as slow
pytestmark = pytest.mark.slow

def parse_gpt_parts(disk_str):
    """Parser a partition string into a list of partitions.

    Args:
        disk_str: The disk description string, as returned by `gpt read`

    Returns:
        A list of parsed partitions. Each partition is a dictionary with the
        string value from each specified key in the partition description, or a
        key with with the value True for a boolean flag
    """
    parts = []
    for part_str in disk_str.split(';'):
        part = {}
        for option in part_str.split(","):
            if not option:
                continue

            if "=" in option:
                key, value = option.split("=")
                part[key] = value
            else:
                part[option] = True

        if part:
            parts.append(part)

    return parts

class GptTestDiskImage(object):
    """Disk Image used by the GPT tests."""

    def __init__(self, u_boot_console):
        """Initialize a new GptTestDiskImage object.

        Args:
            u_boot_console: A U-Boot console.

        Returns:
            Nothing.
        """

        filename = 'test_gpt_disk_image.bin'

        persistent = u_boot_console.config.persistent_data_dir + '/' + filename
        self.path = u_boot_console.config.result_dir  + '/' + filename

        with u_boot_utils.persistent_file_helper(u_boot_console.log, persistent):
            if os.path.exists(persistent):
                u_boot_console.log.action('Disk image file ' + persistent +
                    ' already exists')
            else:
                u_boot_console.log.action('Generating ' + persistent)
                fd = os.open(persistent, os.O_RDWR | os.O_CREAT)
                os.ftruncate(fd, 4194304)
                os.close(fd)
                cmd = ('sgdisk',
                    '--disk-guid=375a56f7-d6c9-4e81-b5f0-09d41ca89efe',
                    persistent)
                u_boot_utils.run_and_log(u_boot_console, cmd)
                # part1 offset 1MB size 1MB
                cmd = ('sgdisk', '--new=1:2048:4095', '--change-name=1:part1',
                    '--partition-guid=1:33194895-67f6-4561-8457-6fdeed4f50a3',
                    '-A 1:set:2',
                    persistent)
                # part2 offset 2MB size 1.5MB
                u_boot_utils.run_and_log(u_boot_console, cmd)
                cmd = ('sgdisk', '--new=2:4096:7167', '--change-name=2:part2',
                    '--partition-guid=2:cc9c6e4a-6551-4cb5-87be-3210f96c86fb',
                    persistent)
                u_boot_utils.run_and_log(u_boot_console, cmd)
                cmd = ('sgdisk', '--load-backup=' + persistent)
                u_boot_utils.run_and_log(u_boot_console, cmd)

        cmd = ('cp', persistent, self.path)
        u_boot_utils.run_and_log(u_boot_console, cmd)

@pytest.fixture(scope='function')
def state_disk_image(u_boot_console):
    """pytest fixture to provide a GptTestDiskImage object to tests.
    This is function-scoped because it uses u_boot_console, which is also
    function-scoped. A new disk is returned each time to prevent tests from
    interfering with each other."""

    return GptTestDiskImage(u_boot_console)

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.buildconfigspec('cmd_part')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_read(state_disk_image, u_boot_console):
    """Test the gpt read command."""

    u_boot_console.run_command('host bind 0 ' + state_disk_image.path)
    output = u_boot_console.run_command('gpt read host 0')
    assert 'Start 1MiB, size 1MiB' in output
    assert 'Block size 512, name part1' in output
    assert 'Start 2MiB, size 1MiB' in output
    assert 'Block size 512, name part2' in output
    output = u_boot_console.run_command('part list host 0')
    assert '0x00000800	0x00000fff	"part1"' in output
    assert '0x00001000	0x00001bff	"part2"' in output

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.buildconfigspec('partition_type_guid')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_read_var(state_disk_image, u_boot_console):
    """Test the gpt read command."""

    u_boot_console.run_command('host bind 0 ' + state_disk_image.path)
    output = u_boot_console.run_command('gpt read host 0 gpt_parts')
    assert 'success!' in output

    output = u_boot_console.run_command('echo ${gpt_parts}')
    parts = parse_gpt_parts(output.rstrip())

    assert parts == [
        {
            "uuid_disk": "375a56f7-d6c9-4e81-b5f0-09d41ca89efe",
        },
        {
            "name": "part1",
            "start": "0x100000",
            "size": "0x100000",
            "type": "0fc63daf-8483-4772-8e79-3d69d8477de4",
            "uuid": "33194895-67f6-4561-8457-6fdeed4f50a3",
            "bootable": True,
        },
        {
            "name": "part2",
            "start": "0x200000",
            "size": "0x180000",
            "type": "0fc63daf-8483-4772-8e79-3d69d8477de4",
            "uuid": "cc9c6e4a-6551-4cb5-87be-3210f96c86fb",
        },
    ]

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_verify(state_disk_image, u_boot_console):
    """Test the gpt verify command."""

    u_boot_console.run_command('host bind 0 ' + state_disk_image.path)
    output = u_boot_console.run_command('gpt verify host 0')
    assert 'Verify GPT: success!' in output

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_repair(state_disk_image, u_boot_console):
    """Test the gpt repair command."""

    u_boot_console.run_command('host bind 0 ' + state_disk_image.path)
    output = u_boot_console.run_command('gpt repair host 0')
    assert 'Repairing GPT: success!' in output

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_guid(state_disk_image, u_boot_console):
    """Test the gpt guid command."""

    u_boot_console.run_command('host bind 0 ' + state_disk_image.path)
    output = u_boot_console.run_command('gpt guid host 0')
    assert '375a56f7-d6c9-4e81-b5f0-09d41ca89efe' in output

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_setenv(state_disk_image, u_boot_console):
    """Test the gpt setenv command."""
    u_boot_console.run_command('host bind 0 ' + state_disk_image.path)
    output = u_boot_console.run_command('gpt setenv host 0 part1')
    assert 'success!' in output
    output = u_boot_console.run_command('echo ${gpt_partition_addr}')
    assert output.rstrip() == '800'
    output = u_boot_console.run_command('echo ${gpt_partition_size}')
    assert output.rstrip() == '800'
    output = u_boot_console.run_command('echo ${gpt_partition_name}')
    assert output.rstrip() == 'part1'
    output = u_boot_console.run_command('echo ${gpt_partition_entry}')
    assert output.rstrip() == '1'
    output = u_boot_console.run_command('echo ${gpt_partition_bootable}')
    assert output.rstrip() == '1'

    output = u_boot_console.run_command('gpt setenv host 0 part2')
    assert 'success!' in output
    output = u_boot_console.run_command('echo ${gpt_partition_addr}')
    assert output.rstrip() == '1000'
    output = u_boot_console.run_command('echo ${gpt_partition_size}')
    assert output.rstrip() == 'c00'
    output = u_boot_console.run_command('echo ${gpt_partition_name}')
    assert output.rstrip() == 'part2'
    output = u_boot_console.run_command('echo ${gpt_partition_entry}')
    assert output.rstrip() == '2'
    output = u_boot_console.run_command('echo ${gpt_partition_bootable}')
    assert output.rstrip() == '0'

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_save_guid(state_disk_image, u_boot_console):
    """Test the gpt guid command to save GUID into a string."""

    if u_boot_console.config.buildconfig.get('config_cmd_gpt', 'n') != 'y':
        pytest.skip('gpt command not supported')
    u_boot_console.run_command('host bind 0 ' + state_disk_image.path)
    output = u_boot_console.run_command('gpt guid host 0 newguid')
    output = u_boot_console.run_command('printenv newguid')
    assert '375a56f7-d6c9-4e81-b5f0-09d41ca89efe' in output

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_part_type_uuid(state_disk_image, u_boot_console):
    """Test the gpt partittion type UUID command."""

    u_boot_console.run_command('host bind 0 ' + state_disk_image.path)
    output = u_boot_console.run_command('part type host 0:1')
    assert '0fc63daf-8483-4772-8e79-3d69d8477de4' in output

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_part_type_save_uuid(state_disk_image, u_boot_console):
    """Test the gpt partittion type to save UUID into a string."""

    if u_boot_console.config.buildconfig.get('config_cmd_gpt', 'n') != 'y':
        pytest.skip('gpt command not supported')
    u_boot_console.run_command('host bind 0 ' + state_disk_image.path)
    output = u_boot_console.run_command('part type host 0:1 newguid')
    output = u_boot_console.run_command('printenv newguid')
    assert '0fc63daf-8483-4772-8e79-3d69d8477de4' in output

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.buildconfigspec('cmd_gpt_rename')
@pytest.mark.buildconfigspec('cmd_part')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_rename_partition(state_disk_image, u_boot_console):
    """Test the gpt rename command to write partition names."""

    u_boot_console.run_command('host bind 0 ' + state_disk_image.path)
    u_boot_console.run_command('gpt rename host 0 1 first')
    output = u_boot_console.run_command('gpt read host 0')
    assert 'name first' in output
    u_boot_console.run_command('gpt rename host 0 2 second')
    output = u_boot_console.run_command('gpt read host 0')
    assert 'name second' in output
    output = u_boot_console.run_command('part list host 0')
    assert '0x00000800	0x00000fff	"first"' in output
    assert '0x00001000	0x00001bff	"second"' in output

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.buildconfigspec('cmd_gpt_rename')
@pytest.mark.buildconfigspec('cmd_part')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_swap_partitions(state_disk_image, u_boot_console):
    """Test the gpt swap command to exchange two partition names."""

    u_boot_console.run_command('host bind 0 ' + state_disk_image.path)
    output = u_boot_console.run_command('part list host 0')
    assert '0x00000800	0x00000fff	"part1"' in output
    assert '0x00001000	0x00001bff	"part2"' in output
    u_boot_console.run_command('gpt swap host 0 part1 part2')
    output = u_boot_console.run_command('part list host 0')
    assert '0x00000800	0x00000fff	"part2"' in output
    assert '0x00001000	0x00001bff	"part1"' in output

@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.buildconfigspec('cmd_gpt_rename')
@pytest.mark.buildconfigspec('cmd_part')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_set_bootable(state_disk_image, u_boot_console):
    """Test the gpt set-bootable command."""

    u_boot_console.run_command('host bind 0 ' + state_disk_image.path)
    parts = ('part2', 'part1')
    for bootable in parts:
        output = u_boot_console.run_command(f'gpt set-bootable host 0 {bootable}')
        assert 'success!' in output

        for p in parts:
            output = u_boot_console.run_command(f'gpt setenv host 0 {p}')
            assert 'success!' in output
            output = u_boot_console.run_command('echo ${gpt_partition_bootable}')
            if p == bootable:
                assert output.rstrip() == '1'
            else:
                assert output.rstrip() == '0'

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.buildconfigspec('cmd_part')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_write(state_disk_image, u_boot_console):
    """Test the gpt write command."""

    u_boot_console.run_command('host bind 0 ' + state_disk_image.path)
    output = u_boot_console.run_command('gpt write host 0 "name=all,size=0"')
    assert 'Writing GPT: success!' in output
    output = u_boot_console.run_command('part list host 0')
    assert '0x00000022	0x00001fde	"all"' in output
    output = u_boot_console.run_command('gpt write host 0 "uuid_disk=375a56f7-d6c9-4e81-b5f0-09d41ca89efe;name=first,start=1M,size=1M;name=second,start=0x200000,size=0x180000;"')
    assert 'Writing GPT: success!' in output
    output = u_boot_console.run_command('part list host 0')
    assert '0x00000800	0x00000fff	"first"' in output
    assert '0x00001000	0x00001bff	"second"' in output
    output = u_boot_console.run_command('gpt guid host 0')
    assert '375a56f7-d6c9-4e81-b5f0-09d41ca89efe' in output

@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.buildconfigspec('cmd_gpt_rename')
@pytest.mark.buildconfigspec('cmd_part')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_transpose(state_disk_image, u_boot_console):
    """Test the gpt transpose command."""

    u_boot_console.run_command('host bind 0 ' + state_disk_image.path)
    output = u_boot_console.run_command('part list host 0')
    assert '1\t0x00000800\t0x00000fff\t"part1"' in output
    assert '2\t0x00001000\t0x00001bff\t"part2"' in output

    output = u_boot_console.run_command('gpt transpose host 0 1 2')
    assert 'success!' in output

    output = u_boot_console.run_command('part list host 0')
    assert '2\t0x00000800\t0x00000fff\t"part1"' in output
    assert '1\t0x00001000\t0x00001bff\t"part2"' in output
