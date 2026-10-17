# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2019, Texas Instrument
# Author: Jean-Jacques Hiblot <jjhiblot@ti.com>
#
# U-Boot File System:symlink Test

"""
This test verifies unlink operation (deleting a file or a directory)
on file system.
"""

import pytest
import re
from fstest_defs import *
from fstest_helpers import assert_fs_integrity


@pytest.mark.boardspec('sandbox')
@pytest.mark.slow
class TestSymlink(object):
    def test_symlink1(self, u_boot_console, fs_obj_symlink):
        """
        Test Case 1 - create a link. and follow it when reading
        """
        fs_type, fs_img, md5val = fs_obj_symlink
        with u_boot_console.log.section('Test Case 1 - create link and read'):
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                'setenv filesize',
                'ln host 0:0 %s /%s.link ' % (SMALL_FILE, SMALL_FILE),
            ])
            assert('' in ''.join(output))

            output = u_boot_console.run_command_list([
                '%sload host 0:0 %x /%s.link' % (fs_type, ADDR, SMALL_FILE),
                'printenv filesize'])
            assert('filesize=100000' in ''.join(output))

            # Test Case 4b - Read full 1MB of small file
            output = u_boot_console.run_command_list([
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val[0] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_symlink2(self, u_boot_console, fs_obj_symlink):
        """
        Test Case 2 - create chained links
        """
        fs_type, fs_img, md5val = fs_obj_symlink
        with u_boot_console.log.section('Test Case 2 - create chained links'):
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                'setenv filesize',
                'ln host 0:0 %s /%s.link1 ' % (SMALL_FILE, SMALL_FILE),
                'ln host 0:0 /%s.link1 /SUBDIR/%s.link2' % (
                    SMALL_FILE, SMALL_FILE),
                'ln host 0:0 SUBDIR/%s.link2 /%s.link3' % (
                    SMALL_FILE, SMALL_FILE),
            ])
            assert('' in ''.join(output))

            output = u_boot_console.run_command_list([
                '%sload host 0:0 %x /%s.link3' % (fs_type, ADDR, SMALL_FILE),
                'printenv filesize'])
            assert('filesize=100000' in ''.join(output))

            # Test Case 4b - Read full 1MB of small file
            output = u_boot_console.run_command_list([
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val[0] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_symlink3(self, u_boot_console, fs_obj_symlink):
        """
        Test Case 3 - replace file/link with link
        """
        fs_type, fs_img, md5val = fs_obj_symlink
        with u_boot_console.log.section('Test Case 1 - create link and read'):
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                'setenv filesize',
                'ln host 0:0 %s /%s ' % (MEDIUM_FILE, SMALL_FILE),
                'ln host 0:0 %s /%s.link ' % (MEDIUM_FILE, MEDIUM_FILE),
            ])
            assert('' in ''.join(output))

            output = u_boot_console.run_command_list([
                '%sload host 0:0 %x /%s' % (fs_type, ADDR, SMALL_FILE),
                'printenv filesize'])
            assert('filesize=a00000' in ''.join(output))

            output = u_boot_console.run_command_list([
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val[1] in ''.join(output))

            output = u_boot_console.run_command_list([
                'ln host 0:0 %s.link /%s ' % (MEDIUM_FILE, SMALL_FILE),
                '%sload host 0:0 %x /%s' % (fs_type, ADDR, SMALL_FILE),
                'printenv filesize'])
            assert('filesize=a00000' in ''.join(output))

            output = u_boot_console.run_command_list([
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val[1] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_symlink4(self, u_boot_console, fs_obj_symlink):
        """
        Test Case 4 - create a broken link
        """
        fs_type, fs_img, md5val = fs_obj_symlink
        with u_boot_console.log.section('Test Case 1 - create link and read'):

            output = u_boot_console.run_command_list([
                'setenv filesize',
                'ln host 0:0 nowhere /link ',
            ])
            assert('' in ''.join(output))

            output = u_boot_console.run_command(
                '%sload host 0:0 %x /link' %
                (fs_type, ADDR))
            with u_boot_console.disable_check('error_notification'):
                output = u_boot_console.run_command('printenv filesize')
            assert('"filesize" not defined' in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)
