# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2018, Linaro Limited
# Author: Takahiro Akashi <takahiro.akashi@linaro.org>
#
# U-Boot File System:Basic Test

"""
This test verifies basic read/write operation on file system.
"""

import pytest
import re
from fstest_defs import *
from fstest_helpers import assert_fs_integrity

@pytest.mark.boardspec('sandbox')
@pytest.mark.slow
class TestFsBasic(object):
    def test_fs1(self, u_boot_console, fs_obj_basic):
        """
        Test Case 1 - ls command, listing a root directory and invalid directory
        """
        fs_type,fs_img,md5val = fs_obj_basic
        with u_boot_console.log.section('Test Case 1a - ls'):
            # Test Case 1 - ls
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sls host 0:0' % fs_type])
            assert(re.search('2621440000 *%s' % BIG_FILE, ''.join(output)))
            assert(re.search('1048576 *%s' % SMALL_FILE, ''.join(output)))

        with u_boot_console.log.section('Test Case 1b - ls (invalid dir)'):
            # In addition, test with a nonexistent directory to see if we crash.
            output = u_boot_console.run_command(
                '%sls host 0:0 invalid_d' % fs_type)
            if fs_type == 'ext4':
                assert('Can not find directory' in output)
            else:
                assert('' == output)

    def test_fs2(self, u_boot_console, fs_obj_basic):
        """
        Test Case 2 - size command for a small file
        """
        fs_type,fs_img,md5val = fs_obj_basic
        with u_boot_console.log.section('Test Case 2a - size (small)'):
            # 1MB is 0x0010 0000
            # Test Case 2a - size of small file
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%ssize host 0:0 /%s' % (fs_type, SMALL_FILE),
                'printenv filesize',
                'setenv filesize'])
            assert('filesize=100000' in ''.join(output))

        with u_boot_console.log.section('Test Case 2b - size (/../<file>)'):
            # Test Case 2b - size of small file via a path using '..'
            output = u_boot_console.run_command_list([
                '%ssize host 0:0 /SUBDIR/../%s' % (fs_type, SMALL_FILE),
                'printenv filesize',
                'setenv filesize'])
            assert('filesize=100000' in ''.join(output))

    def test_fs3(self, u_boot_console, fs_obj_basic):
        """
        Test Case 3 - size command for a large file
        """
        fs_type,fs_img,md5val = fs_obj_basic
        with u_boot_console.log.section('Test Case 3 - size (large)'):
            # 2.5GB (1024*1024*2500) is 0x9C40 0000
            # Test Case 3 - size of big file
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%ssize host 0:0 /%s' % (fs_type, BIG_FILE),
                'printenv filesize',
                'setenv filesize'])
            assert('filesize=9c400000' in ''.join(output))

    def test_fs4(self, u_boot_console, fs_obj_basic):
        """
        Test Case 4 - load a small file, 1MB
        """
        fs_type,fs_img,md5val = fs_obj_basic
        with u_boot_console.log.section('Test Case 4 - load (small)'):
            # Test Case 4a - Read full 1MB of small file
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s' % (fs_type, ADDR, SMALL_FILE),
                'printenv filesize'])
            assert('filesize=100000' in ''.join(output))

            # Test Case 4b - Read full 1MB of small file
            output = u_boot_console.run_command_list([
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val[0] in ''.join(output))

    def test_fs5(self, u_boot_console, fs_obj_basic):
        """
        Test Case 5 - load, reading first 1MB of 3GB file
        """
        fs_type,fs_img,md5val = fs_obj_basic
        with u_boot_console.log.section('Test Case 5 - load (first 1MB)'):
            # Test Case 5a - First 1MB of big file
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s %x 0x0' % (fs_type, ADDR, BIG_FILE, LENGTH),
                'printenv filesize'])
            assert('filesize=100000' in ''.join(output))

            # Test Case 5b - First 1MB of big file
            output = u_boot_console.run_command_list([
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val[1] in ''.join(output))

    def test_fs6(self, u_boot_console, fs_obj_basic):
        """
        Test Case 6 - load, reading last 1MB of 3GB file
        """
        fs_type,fs_img,md5val = fs_obj_basic
        with u_boot_console.log.section('Test Case 6 - load (last 1MB)'):
            # fails for ext as no offset support
            # Test Case 6a - Last 1MB of big file
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s %x 0x9c300000'
                    % (fs_type, ADDR, BIG_FILE, LENGTH),
                'printenv filesize'])
            assert('filesize=100000' in ''.join(output))

            # Test Case 6b - Last 1MB of big file
            output = u_boot_console.run_command_list([
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val[2] in ''.join(output))

    def test_fs7(self, u_boot_console, fs_obj_basic):
        """
        Test Case 7 - load, 1MB from the last 1MB in 2GB
        """
        fs_type,fs_img,md5val = fs_obj_basic
        with u_boot_console.log.section('Test Case 7 - load (last 1MB in 2GB)'):
            # fails for ext as no offset support
            # Test Case 7a - One from the last 1MB chunk of 2GB
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s %x 0x7ff00000'
                    % (fs_type, ADDR, BIG_FILE, LENGTH),
                'printenv filesize'])
            assert('filesize=100000' in ''.join(output))

            # Test Case 7b - One from the last 1MB chunk of 2GB
            output = u_boot_console.run_command_list([
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val[3] in ''.join(output))

    def test_fs8(self, u_boot_console, fs_obj_basic):
        """
        Test Case 8 - load, reading first 1MB in 2GB
        """
        fs_type,fs_img,md5val = fs_obj_basic
        with u_boot_console.log.section('Test Case 8 - load (first 1MB in 2GB)'):
            # fails for ext as no offset support
            # Test Case 8a - One from the start 1MB chunk from 2GB
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s %x 0x80000000'
                    % (fs_type, ADDR, BIG_FILE, LENGTH),
                'printenv filesize'])
            assert('filesize=100000' in ''.join(output))

            # Test Case 8b - One from the start 1MB chunk from 2GB
            output = u_boot_console.run_command_list([
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val[4] in ''.join(output))

    def test_fs9(self, u_boot_console, fs_obj_basic):
        """
        Test Case 9 - load, 1MB crossing 2GB boundary
        """
        fs_type,fs_img,md5val = fs_obj_basic
        with u_boot_console.log.section('Test Case 9 - load (crossing 2GB boundary)'):
            # fails for ext as no offset support
            # Test Case 9a - One 1MB chunk crossing the 2GB boundary
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s %x 0x7ff80000'
                    % (fs_type, ADDR, BIG_FILE, LENGTH),
                'printenv filesize'])
            assert('filesize=100000' in ''.join(output))

            # Test Case 9b - One 1MB chunk crossing the 2GB boundary
            output = u_boot_console.run_command_list([
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val[5] in ''.join(output))

    def test_fs10(self, u_boot_console, fs_obj_basic):
        """
        Test Case 10 - load, reading beyond file end'):
        """
        fs_type,fs_img,md5val = fs_obj_basic
        with u_boot_console.log.section('Test Case 10 - load (beyond file end)'):
            # Generic failure case
            # Test Case 10 - 2MB chunk from the last 1MB of big file
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s 0x00200000 0x9c300000'
                    % (fs_type, ADDR, BIG_FILE),
                'printenv filesize',
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
        assert('filesize=100000' in ''.join(output))

    def test_fs11(self, u_boot_console, fs_obj_basic):
        """
        Test Case 11 - write'
        """
        fs_type,fs_img,md5val = fs_obj_basic
        with u_boot_console.log.section('Test Case 11 - write'):
            # Read 1MB from small file
            # Write it back to test the writes
            # Test Case 11a - Check that the write succeeded
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s' % (fs_type, ADDR, SMALL_FILE),
                '%swrite host 0:0 %x /%s.w $filesize'
                    % (fs_type, ADDR, SMALL_FILE)])
            assert('1048576 bytes written' in ''.join(output))

            # Test Case 11b - Check md5 of written to is same
            # as the one read from
            output = u_boot_console.run_command_list([
                '%sload host 0:0 %x /%s.w' % (fs_type, ADDR, SMALL_FILE),
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val[0] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_fs12(self, u_boot_console, fs_obj_basic):
        """
        Test Case 12 - write to "." directory
        """
        fs_type,fs_img,md5val = fs_obj_basic
        with u_boot_console.log.section('Test Case 12 - write (".")'):
            # Next test case checks writing a file whose dirent
            # is the first in the block, which is always true for "."
            # The write should fail, but the lookup should work
            # Test Case 12 - Check directory traversal
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%swrite host 0:0 %x /. 0x10' % (fs_type, ADDR)])
            assert('Unable to write' in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_fs13(self, u_boot_console, fs_obj_basic):
        """
        Test Case 13 - write to a file with "/./<filename>"
        """
        fs_type,fs_img,md5val = fs_obj_basic
        with u_boot_console.log.section('Test Case 13 - write  ("./<file>")'):
            # Read 1MB from small file
            # Write it via "same directory", i.e. "." dirent
            # Test Case 13a - Check directory traversal
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s' % (fs_type, ADDR, SMALL_FILE),
                '%swrite host 0:0 %x /./%s2 $filesize'
                    % (fs_type, ADDR, SMALL_FILE)])
            assert('1048576 bytes written' in ''.join(output))

            # Test Case 13b - Check md5 of written to is same
            # as the one read from
            output = u_boot_console.run_command_list([
                'mw.b %x 00 100' % ADDR,
                '%sload host 0:0 %x /./%s2' % (fs_type, ADDR, SMALL_FILE),
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val[0] in ''.join(output))

            # Test Case 13c - Check md5 of written to is same
            # as the one read from
            output = u_boot_console.run_command_list([
                'mw.b %x 00 100' % ADDR,
                '%sload host 0:0 %x /%s2' % (fs_type, ADDR, SMALL_FILE),
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val[0] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)
