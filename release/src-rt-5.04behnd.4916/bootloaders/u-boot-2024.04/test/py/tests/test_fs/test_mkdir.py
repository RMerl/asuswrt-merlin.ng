# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2018, Linaro Limited
# Author: Takahiro Akashi <takahiro.akashi@linaro.org>
#
# U-Boot File System:mkdir Test

"""
This test verifies mkdir operation on file system.
"""

import pytest
from fstest_helpers import assert_fs_integrity

@pytest.mark.boardspec('sandbox')
@pytest.mark.slow
class TestMkdir(object):
    def test_mkdir1(self, u_boot_console, fs_obj_mkdir):
        """
        Test Case 1 - create a directory under a root
        """
        fs_type,fs_img = fs_obj_mkdir
        with u_boot_console.log.section('Test Case 1 - mkdir'):
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%smkdir host 0:0 dir1' % fs_type,
                '%sls host 0:0 /' % fs_type])
            assert('dir1/' in ''.join(output))

            output = u_boot_console.run_command(
                '%sls host 0:0 dir1' % fs_type)
            assert('./'   in output)
            assert('../'  in output)
            assert_fs_integrity(fs_type, fs_img)


    def test_mkdir2(self, u_boot_console, fs_obj_mkdir):
        """
        Test Case 2 - create a directory under a sub-directory
        """
        fs_type,fs_img = fs_obj_mkdir
        with u_boot_console.log.section('Test Case 2 - mkdir (sub-sub directory)'):
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%smkdir host 0:0 dir1/dir2' % fs_type,
                '%sls host 0:0 dir1' % fs_type])
            assert('dir2/' in ''.join(output))

            output = u_boot_console.run_command(
                '%sls host 0:0 dir1/dir2' % fs_type)
            assert('./'   in output)
            assert('../'  in output)
            assert_fs_integrity(fs_type, fs_img)

    def test_mkdir3(self, u_boot_console, fs_obj_mkdir):
        """
        Test Case 3 - trying to create a directory with a non-existing
        path should fail
        """
        fs_type,fs_img = fs_obj_mkdir
        with u_boot_console.log.section('Test Case 3 - mkdir (non-existing path)'):
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%smkdir host 0:0 none/dir3' % fs_type])
            assert('Unable to create a directory' in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_mkdir4(self, u_boot_console, fs_obj_mkdir):
        """
        Test Case 4 - trying to create "." should fail
        """
        fs_type,fs_img = fs_obj_mkdir
        with u_boot_console.log.section('Test Case 4 - mkdir (".")'):
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%smkdir host 0:0 .' % fs_type])
            assert('Unable to create a directory' in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_mkdir5(self, u_boot_console, fs_obj_mkdir):
        """
        Test Case 5 - trying to create ".." should fail
        """
        fs_type,fs_img = fs_obj_mkdir
        with u_boot_console.log.section('Test Case 5 - mkdir ("..")'):
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%smkdir host 0:0 ..' % fs_type])
            assert('Unable to create a directory' in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_mkdir6(self, u_boot_console, fs_obj_mkdir):
        """
        'Test Case 6 - create as many directories as amount of directory
        entries goes beyond a cluster size)'
        """
        fs_type,fs_img = fs_obj_mkdir
        with u_boot_console.log.section('Test Case 6 - mkdir (create many)'):
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                '%smkdir host 0:0 dir6' % fs_type,
                '%sls host 0:0 /' % fs_type])
            assert('dir6/' in ''.join(output))

            for i in range(0, 20):
                output = u_boot_console.run_command(
                    '%smkdir host 0:0 dir6/0123456789abcdef%02x'
                    % (fs_type, i))
            output = u_boot_console.run_command('%sls host 0:0 dir6' % fs_type)
            assert('0123456789abcdef00/'  in output)
            assert('0123456789abcdef13/'  in output)

            output = u_boot_console.run_command(
                '%sls host 0:0 dir6/0123456789abcdef13/.' % fs_type)
            assert('./'   in output)
            assert('../'  in output)

            output = u_boot_console.run_command(
                '%sls host 0:0 dir6/0123456789abcdef13/..' % fs_type)
            assert('0123456789abcdef00/'  in output)
            assert('0123456789abcdef13/'  in output)
            assert_fs_integrity(fs_type, fs_img)
