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
                cmd = ('sgdisk', '-U', '375a56f7-d6c9-4e81-b5f0-09d41ca89efe',
                    persistent)
                u_boot_utils.run_and_log(u_boot_console, cmd)
                # part1 offset 1MB size 1MB
                cmd = ('sgdisk', '--new=1:2048:4095', '-c 1:part1', persistent)
                # part2 offset 2MB size 1.5MB
                u_boot_utils.run_and_log(u_boot_console, cmd)
                cmd = ('sgdisk', '--new=2:4096:7167', '-c 2:part2', persistent)
                u_boot_utils.run_and_log(u_boot_console, cmd)
                cmd = ('sgdisk', '-l', persistent)
                u_boot_utils.run_and_log(u_boot_console, cmd)

        cmd = ('cp', persistent, self.path)
        u_boot_utils.run_and_log(u_boot_console, cmd)

gtdi = None
@pytest.fixture(scope='function')
def state_disk_image(u_boot_console):
    """pytest fixture to provide a GptTestDiskImage object to tests.
    This is function-scoped because it uses u_boot_console, which is also
    function-scoped. However, we don't need to actually do any function-scope
    work, so this simply returns the same object over and over each time."""

    global gtdi
    if not gtdi:
        gtdi = GptTestDiskImage(u_boot_console)
    return gtdi

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
@pytest.mark.requiredtool('sgdisk')
def test_gpt_verify(state_disk_image, u_boot_console):
    """Test the gpt verify command."""

    u_boot_console.run_command('host bind 0 ' + state_disk_image.path)
    output = u_boot_console.run_command('gpt verify host 0')
    assert 'Verify GPT: success!' in output

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
    assert '0x00000800	0x00000fff	"first"' in output
    assert '0x00001000	0x00001bff	"second"' in output
    u_boot_console.run_command('gpt swap host 0 first second')
    output = u_boot_console.run_command('part list host 0')
    assert '0x00000800	0x00000fff	"second"' in output
    assert '0x00001000	0x00001bff	"first"' in output

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
