# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2020, Collabora
# Author: Frédéric Danis <frederic.danis@collabora.com>

import pytest
import u_boot_utils
import os
import tempfile
import shutil

PSTORE_ADDR=0x3000000
PSTORE_LENGTH=0x100000
PSTORE_PANIC1='test/py/tests/test_pstore_data_panic1.hex'
PSTORE_PANIC2='test/py/tests/test_pstore_data_panic2.hex'
PSTORE_CONSOLE='test/py/tests/test_pstore_data_console.hex'
ADDR=0x01000008

def load_pstore(u_boot_console):
    """Load PStore records from sample files"""

    output = u_boot_console.run_command_list([
        'host load hostfs - 0x%x %s' % (PSTORE_ADDR,
            os.path.join(u_boot_console.config.source_dir, PSTORE_PANIC1)),
        'host load hostfs - 0x%x %s' % (PSTORE_ADDR + 4096,
            os.path.join(u_boot_console.config.source_dir, PSTORE_PANIC2)),
        'host load hostfs - 0x%x %s' % (PSTORE_ADDR + 253 * 4096,
            os.path.join(u_boot_console.config.source_dir, PSTORE_CONSOLE)),
        'pstore set 0x%x 0x%x' % (PSTORE_ADDR, PSTORE_LENGTH)])

def checkfile(u_boot_console, path, filesize, checksum):
    """Check file against MD5 checksum"""

    output = u_boot_console.run_command_list([
        'load hostfs - %x %s' % (ADDR, path),
        'printenv filesize'])
    assert('filesize=%x' % (filesize) in ''.join(output))

    output = u_boot_console.run_command_list([
        'md5sum %x $filesize' % ADDR,
        'setenv filesize'])
    assert(checksum in ''.join(output))

@pytest.mark.buildconfigspec('cmd_pstore')
def test_pstore_display_all_records(u_boot_console):
    """Test that pstore displays all records."""

    u_boot_console.run_command('')
    load_pstore(u_boot_console)
    response = u_boot_console.run_command('pstore display')
    assert('**** Dump' in response)
    assert('**** Console' in response)

@pytest.mark.buildconfigspec('cmd_pstore')
def test_pstore_display_one_record(u_boot_console):
    """Test that pstore displays only one record."""

    u_boot_console.run_command('')
    load_pstore(u_boot_console)
    response = u_boot_console.run_command('pstore display dump 1')
    assert('Panic#2 Part1' in response)
    assert('**** Console' not in response)

@pytest.mark.buildconfigspec('cmd_pstore')
def test_pstore_save_records(u_boot_console):
    """Test that pstore saves all records."""

    outdir = tempfile.mkdtemp()

    u_boot_console.run_command('')
    load_pstore(u_boot_console)
    u_boot_console.run_command('pstore save hostfs - %s' % (outdir))

    checkfile(u_boot_console, '%s/dmesg-ramoops-0' % (outdir), 3798, '8059335ab4cfa62c77324c491659c503')
    checkfile(u_boot_console, '%s/dmesg-ramoops-1' % (outdir), 4035, '3ff30df3429d81939c75d0070b5187b9')
    checkfile(u_boot_console, '%s/console-ramoops-0' % (outdir), 4084, 'bb44de4a9b8ebd9b17ae98003287325b')

    shutil.rmtree(outdir)
