# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2020 Bootlin
# Author: Joao Marcos Costa <joaomarcos.costa@bootlin.com>

import os
import pytest

from sqfs_common import STANDARD_TABLE
from sqfs_common import generate_sqfs_src_dir, make_all_images
from sqfs_common import clean_sqfs_src_dir, clean_all_images
from sqfs_common import check_mksquashfs_version

def sqfs_ls_at_root(u_boot_console):
    """ Runs sqfsls at image's root.

    This test checks if all the present files and directories were listed. Also,
    it checks if passing the slash or not changes the output, which it shouldn't.

    Args:
        u_boot_console: provides the means to interact with U-Boot's console.
    """

    no_slash = u_boot_console.run_command('sqfsls host 0')
    slash = u_boot_console.run_command('sqfsls host 0 /')
    assert no_slash == slash

    expected_lines = ['empty-dir/', '1000   f1000', '4096   f4096', '5096   f5096',
                      'subdir/', '<SYM>   sym', '4 file(s), 2 dir(s)']

    output = u_boot_console.run_command('sqfsls host 0')
    for line in expected_lines:
        assert line in output

def sqfs_ls_at_empty_dir(u_boot_console):
    """ Runs sqfsls at an empty directory.

    This tests checks if sqfsls will print anything other than the 'Empty directory'
    message.

    Args:
        u_boot_console: provides the means to interact with U-Boot's console.
    """
    assert u_boot_console.run_command('sqfsls host 0 empty-dir') == 'Empty directory.'

def sqfs_ls_at_subdir(u_boot_console):
    """ Runs sqfsls at the SquashFS image's subdir.

    This test checks if the path resolution works, since the directory is not the
    root.

    Args:
        u_boot_console: provides the means to interact with U-Boot's console.
    """
    expected_lines = ['100   subdir-file', '1 file(s), 0 dir(s)']
    output = u_boot_console.run_command('sqfsls host 0 subdir')
    for line in expected_lines:
        assert line in output

def sqfs_ls_at_symlink(u_boot_console):
    """ Runs sqfsls at a SquashFS image's symbolic link.

    This test checks if the symbolic link's target resolution works.

    Args:
        u_boot_console: provides the means to interact with U-Boot's console.
    """
    # since sym -> subdir, the following outputs must be equal
    output = u_boot_console.run_command('sqfsls host 0 sym')
    output_subdir = u_boot_console.run_command('sqfsls host 0 subdir')
    assert output == output_subdir

    expected_lines = ['100   subdir-file', '1 file(s), 0 dir(s)']
    for line in expected_lines:
        assert line in output

def sqfs_ls_at_non_existent_dir(u_boot_console):
    """ Runs sqfsls at a file and at a non-existent directory.

    This test checks if the SquashFS support won't crash if it doesn't find the
    specified directory or if it takes a file as an input instead of an actual
    directory. In both cases, the output should be the same.

    Args:
        u_boot_console: provides the means to interact with U-Boot's console.
    """
    out_non_existent = u_boot_console.run_command('sqfsls host 0 fff')
    out_not_dir = u_boot_console.run_command('sqfsls host 0 f1000')
    assert out_non_existent == out_not_dir
    assert '** Cannot find directory. **' in out_non_existent

def sqfs_run_all_ls_tests(u_boot_console):
    """ Runs all the previously defined test cases.

    Args:
        u_boot_console: provides the means to interact with U-Boot's console.
    """
    sqfs_ls_at_root(u_boot_console)
    sqfs_ls_at_empty_dir(u_boot_console)
    sqfs_ls_at_subdir(u_boot_console)
    sqfs_ls_at_symlink(u_boot_console)
    sqfs_ls_at_non_existent_dir(u_boot_console)

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_fs_generic')
@pytest.mark.buildconfigspec('cmd_squashfs')
@pytest.mark.buildconfigspec('fs_squashfs')
@pytest.mark.requiredtool('mksquashfs')
@pytest.mark.singlethread
def test_sqfs_ls(u_boot_console):
    """ Executes the sqfsls test suite.

    First, it generates the SquashFS images, then it runs the test cases and
    finally cleans the workspace. If an exception is raised, the workspace is
    cleaned before exiting.

    Args:
        u_boot_console: provides the means to interact with U-Boot's console.
    """
    build_dir = u_boot_console.config.build_dir

    # If the EFI subsystem is enabled and initialized, EFI subsystem tries to
    # add EFI boot option when the new disk is detected. If there is no EFI
    # System Partition exists, EFI subsystem outputs error messages and
    # it ends up with test failure.
    # Restart U-Boot to clear the previous state.
    # TODO: Ideally EFI test cases need to be fixed, but it will
    # increase the number of system reset.
    u_boot_console.restart_uboot()

    # setup test environment
    check_mksquashfs_version()
    generate_sqfs_src_dir(build_dir)
    make_all_images(build_dir)

    # run all tests for each image
    for image in STANDARD_TABLE:
        try:
            image_path = os.path.join(build_dir, image)
            u_boot_console.run_command('host bind 0 {}'.format(image_path))
            sqfs_run_all_ls_tests(u_boot_console)
        except:
            clean_all_images(build_dir)
            clean_sqfs_src_dir(build_dir)
            raise AssertionError

    # clean test environment
    clean_all_images(build_dir)
    clean_sqfs_src_dir(build_dir)
