# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2018, Linaro Limited
# Author: Takahiro Akashi <takahiro.akashi@linaro.org>
#
# U-Boot File System:Exntented Test

"""
This test verifies extended write operation on file system.
"""

import pytest
import re
from fstest_defs import *
from fstest_helpers import assert_fs_integrity

@pytest.mark.boardspec('sandbox')
@pytest.mark.slow
class TestFsExt(object):
    def test_fs_ext1(self, u_boot_console, fs_obj_ext):
        """
        Test Case 1 - write a file with absolute path
        """
        fs_type,fs_img,md5val = fs_obj_ext
        with u_boot_console.log.section('Test Case 1 - write with abs path'):
            # Test Case 1a - Check if command successfully returned
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s' % (fs_type, ADDR, MIN_FILE),
                '%swrite host 0:0 %x /dir1/%s.w1 $filesize'
                    % (fs_type, ADDR, MIN_FILE)])
            assert('20480 bytes written' in ''.join(output))

            # Test Case 1b - Check md5 of file content
            output = u_boot_console.run_command_list([
                'mw.b %x 00 100' % ADDR,
                '%sload host 0:0 %x /dir1/%s.w1' % (fs_type, ADDR, MIN_FILE),
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val[0] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_fs_ext2(self, u_boot_console, fs_obj_ext):
        """
        Test Case 2 - write to a file with relative path
        """
        fs_type,fs_img,md5val = fs_obj_ext
        with u_boot_console.log.section('Test Case 2 - write with rel path'):
            # Test Case 2a - Check if command successfully returned
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s' % (fs_type, ADDR, MIN_FILE),
                '%swrite host 0:0 %x dir1/%s.w2 $filesize'
                    % (fs_type, ADDR, MIN_FILE)])
            assert('20480 bytes written' in ''.join(output))

            # Test Case 2b - Check md5 of file content
            output = u_boot_console.run_command_list([
                'mw.b %x 00 100' % ADDR,
                '%sload host 0:0 %x dir1/%s.w2' % (fs_type, ADDR, MIN_FILE),
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val[0] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_fs_ext3(self, u_boot_console, fs_obj_ext):
        """
        Test Case 3 - write to a file with invalid path
        """
        fs_type,fs_img,md5val = fs_obj_ext
        with u_boot_console.log.section('Test Case 3 - write with invalid path'):
            # Test Case 3 - Check if command expectedly failed
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s' % (fs_type, ADDR, MIN_FILE),
                '%swrite host 0:0 %x /dir1/none/%s.w3 $filesize'
                    % (fs_type, ADDR, MIN_FILE)])
            assert('Unable to write "/dir1/none/' in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_fs_ext4(self, u_boot_console, fs_obj_ext):
        """
        Test Case 4 - write at non-zero offset, enlarging file size
        """
        fs_type,fs_img,md5val = fs_obj_ext
        with u_boot_console.log.section('Test Case 4 - write at non-zero offset, enlarging file size'):
            # Test Case 4a - Check if command successfully returned
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s' % (fs_type, ADDR, MIN_FILE),
                '%swrite host 0:0 %x /dir1/%s.w4 $filesize'
                    % (fs_type, ADDR, MIN_FILE)])
            output = u_boot_console.run_command(
                '%swrite host 0:0 %x /dir1/%s.w4 $filesize 0x1400'
                    % (fs_type, ADDR, MIN_FILE))
            assert('20480 bytes written' in output)

            # Test Case 4b - Check size of written file
            output = u_boot_console.run_command_list([
                '%ssize host 0:0 /dir1/%s.w4' % (fs_type, MIN_FILE),
                'printenv filesize',
                'setenv filesize'])
            assert('filesize=6400' in ''.join(output))

            # Test Case 4c - Check md5 of file content
            output = u_boot_console.run_command_list([
                'mw.b %x 00 100' % ADDR,
                '%sload host 0:0 %x /dir1/%s.w4' % (fs_type, ADDR, MIN_FILE),
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val[1] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_fs_ext5(self, u_boot_console, fs_obj_ext):
        """
        Test Case 5 - write at non-zero offset, shrinking file size
        """
        fs_type,fs_img,md5val = fs_obj_ext
        with u_boot_console.log.section('Test Case 5 - write at non-zero offset, shrinking file size'):
            # Test Case 5a - Check if command successfully returned
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s' % (fs_type, ADDR, MIN_FILE),
                '%swrite host 0:0 %x /dir1/%s.w5 $filesize'
                    % (fs_type, ADDR, MIN_FILE)])
            output = u_boot_console.run_command(
                '%swrite host 0:0 %x /dir1/%s.w5 0x1400 0x1400'
                    % (fs_type, ADDR, MIN_FILE))
            assert('5120 bytes written' in output)

            # Test Case 5b - Check size of written file
            output = u_boot_console.run_command_list([
                '%ssize host 0:0 /dir1/%s.w5' % (fs_type, MIN_FILE),
                'printenv filesize',
                'setenv filesize'])
            assert('filesize=2800' in ''.join(output))

            # Test Case 5c - Check md5 of file content
            output = u_boot_console.run_command_list([
                'mw.b %x 00 100' % ADDR,
                '%sload host 0:0 %x /dir1/%s.w5' % (fs_type, ADDR, MIN_FILE),
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val[2] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_fs_ext6(self, u_boot_console, fs_obj_ext):
        """
        Test Case 6 - write nothing at the start, truncating to zero
        """
        fs_type,fs_img,md5val = fs_obj_ext
        with u_boot_console.log.section('Test Case 6 - write nothing at the start, truncating to zero'):
            # Test Case 6a - Check if command successfully returned
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s' % (fs_type, ADDR, MIN_FILE),
                '%swrite host 0:0 %x /dir1/%s.w6 $filesize'
                    % (fs_type, ADDR, MIN_FILE)])
            output = u_boot_console.run_command(
                '%swrite host 0:0 %x /dir1/%s.w6 0 0'
                    % (fs_type, ADDR, MIN_FILE))
            assert('0 bytes written' in output)

            # Test Case 6b - Check size of written file
            output = u_boot_console.run_command_list([
                '%ssize host 0:0 /dir1/%s.w6' % (fs_type, MIN_FILE),
                'printenv filesize',
                'setenv filesize'])
            assert('filesize=0' in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_fs_ext7(self, u_boot_console, fs_obj_ext):
        """
        Test Case 7 - write at the end (append)
        """
        fs_type,fs_img,md5val = fs_obj_ext
        with u_boot_console.log.section('Test Case 7 - write at the end (append)'):
            # Test Case 7a - Check if command successfully returned
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s' % (fs_type, ADDR, MIN_FILE),
                '%swrite host 0:0 %x /dir1/%s.w7 $filesize'
                    % (fs_type, ADDR, MIN_FILE)])
            output = u_boot_console.run_command(
                '%swrite host 0:0 %x /dir1/%s.w7 $filesize $filesize'
                    % (fs_type, ADDR, MIN_FILE))
            assert('20480 bytes written' in output)

            # Test Case 7b - Check size of written file
            output = u_boot_console.run_command_list([
                '%ssize host 0:0 /dir1/%s.w7' % (fs_type, MIN_FILE),
                'printenv filesize',
                'setenv filesize'])
            assert('filesize=a000' in ''.join(output))

            # Test Case 7c - Check md5 of file content
            output = u_boot_console.run_command_list([
                'mw.b %x 00 100' % ADDR,
                '%sload host 0:0 %x /dir1/%s.w7' % (fs_type, ADDR, MIN_FILE),
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val[3] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_fs_ext8(self, u_boot_console, fs_obj_ext):
        """
        Test Case 8 - write at offset beyond the end of file
        """
        fs_type,fs_img,md5val = fs_obj_ext
        with u_boot_console.log.section('Test Case 8 - write beyond the end'):
            # Test Case 8a - Check if command expectedly failed
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s' % (fs_type, ADDR, MIN_FILE),
                '%swrite host 0:0 %x /dir1/%s.w8 $filesize'
                    % (fs_type, ADDR, MIN_FILE)])
            output = u_boot_console.run_command(
                '%swrite host 0:0 %x /dir1/%s.w8 0x1400 %x'
                    % (fs_type, ADDR, MIN_FILE, 0x100000 + 0x1400))
            assert('Unable to write "/dir1' in output)
            assert_fs_integrity(fs_type, fs_img)

    def test_fs_ext9(self, u_boot_console, fs_obj_ext):
        """
        Test Case 9 - write to a non-existing file at non-zero offset
        """
        fs_type,fs_img,md5val = fs_obj_ext
        with u_boot_console.log.section('Test Case 9 - write to non-existing file with non-zero offset'):
            # Test Case 9a - Check if command expectedly failed
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s' % (fs_type, ADDR, MIN_FILE),
                '%swrite host 0:0 %x /dir1/%s.w9 0x1400 0x1400'
                    % (fs_type, ADDR, MIN_FILE)])
            assert('Unable to write "/dir1' in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)
