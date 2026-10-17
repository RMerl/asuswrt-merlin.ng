# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2020 Bootlin
# Author: Joao Marcos Costa <joaomarcos.costa@bootlin.com>

import os
import subprocess
import pytest

from sqfs_common import SQFS_SRC_DIR, STANDARD_TABLE
from sqfs_common import generate_sqfs_src_dir, make_all_images
from sqfs_common import clean_sqfs_src_dir, clean_all_images
from sqfs_common import check_mksquashfs_version

@pytest.mark.requiredtool('md5sum')
def original_md5sum(path):
    """ Runs md5sum command.

    Args:
        path: path to original file.
    Returns:
        The original file's checksum as a string.
    """

    out = subprocess.run(['md5sum ' + path], shell=True, check=True,
                         capture_output=True, text=True)
    checksum = out.stdout.split()[0]

    return checksum

def uboot_md5sum(u_boot_console, address, count):
    """ Runs U-Boot's md5sum command.

    Args:
        u_boot_console: provides the means to interact with U-Boot's console.
        address: address where the file was loaded (e.g.: $kernel_addr_r).
        count: file's size. It was named 'count' to match md5sum's respective
        argument name.
    Returns:
        The checksum of the file loaded with sqfsload as a string.
    """

    out = u_boot_console.run_command('md5sum {} {}'.format(address, count))
    checksum = out.split()[-1]

    return checksum

def sqfs_load_files(u_boot_console, files, sizes, address):
    """ Loads files and asserts their checksums.

    Args:
        u_boot_console: provides the means to interact with U-Boot's console.
        files: list of files to be loaded.
        sizes: the sizes of each file.
        address: the address where the files should be loaded.
    """
    build_dir = u_boot_console.config.build_dir
    for (file, size) in zip(files, sizes):
        out = u_boot_console.run_command('sqfsload host 0 {} {}'.format(address, file))

        # check if the right amount of bytes was read
        assert size in out

        # compare original file's checksum against u-boot's
        u_boot_checksum = uboot_md5sum(u_boot_console, address, hex(int(size)))
        original_file_path = os.path.join(build_dir, SQFS_SRC_DIR + '/' + file)
        original_checksum = original_md5sum(original_file_path)
        assert u_boot_checksum == original_checksum

def sqfs_load_files_at_root(u_boot_console):
    """ Calls sqfs_load_files passing the files at the SquashFS image's root.

    Args:
        u_boot_console: provides the means to interact with U-Boot's console.
    """

    files = ['f4096', 'f5096', 'f1000']
    sizes = ['4096', '5096', '1000']
    address = '$kernel_addr_r'
    sqfs_load_files(u_boot_console, files, sizes, address)

def sqfs_load_files_at_subdir(u_boot_console):
    """ Calls sqfs_load_files passing the files at the SquashFS image's subdir.

    This test checks if the path resolution works, since the file is not at the
    root directory.

    Args:
        u_boot_console: provides the means to interact with U-Boot's console.
    """
    files = ['subdir/subdir-file']
    sizes = ['100']
    address = '$kernel_addr_r'
    sqfs_load_files(u_boot_console, files, sizes, address)

def sqfs_load_non_existent_file(u_boot_console):
    """ Calls sqfs_load_files passing an non-existent file to raise an error.

    This test checks if the SquashFS support won't crash if it doesn't find the
    specified file.

    Args:
        u_boot_console: provides the means to interact with U-Boot's console.
    """
    address = '$kernel_addr_r'
    file = 'non-existent'
    out = u_boot_console.run_command('sqfsload host 0 {} {}'.format(address, file))
    assert 'Failed to load' in out

def sqfs_run_all_load_tests(u_boot_console):
    """ Runs all the previously defined test cases.

    Args:
        u_boot_console: provides the means to interact with U-Boot's console.
    """
    sqfs_load_files_at_root(u_boot_console)
    sqfs_load_files_at_subdir(u_boot_console)
    sqfs_load_non_existent_file(u_boot_console)

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_fs_generic')
@pytest.mark.buildconfigspec('cmd_squashfs')
@pytest.mark.buildconfigspec('fs_squashfs')
@pytest.mark.requiredtool('mksquashfs')
def test_sqfs_load(u_boot_console):
    """ Executes the sqfsload test suite.

    First, it generates the SquashFS images, then it runs the test cases and
    finally cleans the workspace. If an exception is raised, the workspace is
    cleaned before exiting.

    Args:
        u_boot_console: provides the means to interact with U-Boot's console.
    """
    build_dir = u_boot_console.config.build_dir

    # setup test environment
    check_mksquashfs_version()
    generate_sqfs_src_dir(build_dir)
    make_all_images(build_dir)

    # run all tests for each image
    for image in STANDARD_TABLE:
        try:
            image_path = os.path.join(build_dir, image)
            u_boot_console.run_command('host bind 0 {}'.format(image_path))
            sqfs_run_all_load_tests(u_boot_console)
        except:
            clean_all_images(build_dir)
            clean_sqfs_src_dir(build_dir)
            raise AssertionError

    # clean test environment
    clean_all_images(build_dir)
    clean_sqfs_src_dir(build_dir)
