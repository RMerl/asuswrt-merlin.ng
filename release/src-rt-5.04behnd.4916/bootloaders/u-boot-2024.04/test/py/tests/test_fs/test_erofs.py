# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2022 Huang Jianan <jnhuang95@gmail.com>
# Author: Huang Jianan <jnhuang95@gmail.com>

import os
import pytest
import shutil
import subprocess

EROFS_SRC_DIR = 'erofs_src_dir'
EROFS_IMAGE_NAME = 'erofs.img'

def generate_file(name, size):
    """
    Generates a file filled with 'x'.
    """
    content = 'x' * size
    file = open(name, 'w')
    file.write(content)
    file.close()

def make_erofs_image(build_dir):
    """
    Makes the EROFS images used for the test.

    The image is generated at build_dir with the following structure:
    erofs_src_dir/
    ├── f4096
    ├── f7812
    ├── subdir/
    │   └── subdir-file
    ├── symdir -> subdir
    └── symfile -> f5096
    """
    root = os.path.join(build_dir, EROFS_SRC_DIR)
    os.makedirs(root)

    # 4096: uncompressed file
    generate_file(os.path.join(root, 'f4096'), 4096)

    # 7812: Compressed file
    generate_file(os.path.join(root, 'f7812'), 7812)

    # sub-directory with a single file inside
    subdir_path = os.path.join(root, 'subdir')
    os.makedirs(subdir_path)
    generate_file(os.path.join(subdir_path, 'subdir-file'), 100)

    # symlink
    os.symlink('subdir', os.path.join(root, 'symdir'))
    os.symlink('f7812', os.path.join(root, 'symfile'))

    input_path = os.path.join(build_dir, EROFS_SRC_DIR)
    output_path = os.path.join(build_dir, EROFS_IMAGE_NAME)
    args = ' '.join([output_path, input_path])
    subprocess.run(['mkfs.erofs -zlz4 ' + args], shell=True, check=True,
                   stdout=subprocess.DEVNULL)

def clean_erofs_image(build_dir):
    """
    Deletes the image and src_dir at build_dir.
    """
    path = os.path.join(build_dir, EROFS_SRC_DIR)
    shutil.rmtree(path)
    image_path = os.path.join(build_dir, EROFS_IMAGE_NAME)
    os.remove(image_path)

def erofs_ls_at_root(u_boot_console):
    """
    Test if all the present files and directories were listed.
    """
    no_slash = u_boot_console.run_command('erofsls host 0')
    slash = u_boot_console.run_command('erofsls host 0 /')
    assert no_slash == slash

    expected_lines = ['./', '../', '4096   f4096', '7812   f7812', 'subdir/',
                      '<SYM>   symdir', '<SYM>   symfile', '4 file(s), 3 dir(s)']

    output = u_boot_console.run_command('erofsls host 0')
    for line in expected_lines:
        assert line in output

def erofs_ls_at_subdir(u_boot_console):
    """
    Test if the path resolution works.
    """
    expected_lines = ['./', '../', '100   subdir-file', '1 file(s), 2 dir(s)']
    output = u_boot_console.run_command('erofsls host 0 subdir')
    for line in expected_lines:
        assert line in output

def erofs_ls_at_symlink(u_boot_console):
    """
    Test if the symbolic link's target resolution works.
    """
    output = u_boot_console.run_command('erofsls host 0 symdir')
    output_subdir = u_boot_console.run_command('erofsls host 0 subdir')
    assert output == output_subdir

    expected_lines = ['./', '../', '100   subdir-file', '1 file(s), 2 dir(s)']
    for line in expected_lines:
        assert line in output

def erofs_ls_at_non_existent_dir(u_boot_console):
    """
    Test if the EROFS support will crash when get a nonexistent directory.
    """
    out_non_existent = u_boot_console.run_command('erofsls host 0 fff')
    out_not_dir = u_boot_console.run_command('erofsls host 0 f1000')
    assert out_non_existent == out_not_dir
    assert '' in out_non_existent

def erofs_load_files(u_boot_console, files, sizes, address):
    """
    Loads files and asserts their checksums.
    """
    build_dir = u_boot_console.config.build_dir
    for (file, size) in zip(files, sizes):
        out = u_boot_console.run_command('erofsload host 0 {} {}'.format(address, file))

        # check if the right amount of bytes was read
        assert size in out

        # calculate u-boot file's checksum
        out = u_boot_console.run_command('md5sum {} {}'.format(address, hex(int(size))))
        u_boot_checksum = out.split()[-1]

        # calculate original file's checksum
        original_file_path = os.path.join(build_dir, EROFS_SRC_DIR + '/' + file)
        out = subprocess.run(['md5sum ' + original_file_path], shell=True, check=True,
                             capture_output=True, text=True)
        original_checksum = out.stdout.split()[0]

        # compare checksum
        assert u_boot_checksum == original_checksum

def erofs_load_files_at_root(u_boot_console):
    """
    Test load file from the root directory.
    """
    files = ['f4096', 'f7812']
    sizes = ['4096', '7812']
    address = '$kernel_addr_r'
    erofs_load_files(u_boot_console, files, sizes, address)

def erofs_load_files_at_subdir(u_boot_console):
    """
    Test load file from the subdirectory.
    """
    files = ['subdir/subdir-file']
    sizes = ['100']
    address = '$kernel_addr_r'
    erofs_load_files(u_boot_console, files, sizes, address)

def erofs_load_files_at_symlink(u_boot_console):
    """
    Test load file from the symlink.
    """
    files = ['symfile']
    sizes = ['7812']
    address = '$kernel_addr_r'
    erofs_load_files(u_boot_console, files, sizes, address)

def erofs_load_non_existent_file(u_boot_console):
    """
    Test if the EROFS support will crash when load a nonexistent file.
    """
    address = '$kernel_addr_r'
    file = 'non-existent'
    out = u_boot_console.run_command('erofsload host 0 {} {}'.format(address, file))
    assert 'Failed to load' in out

def erofs_run_all_tests(u_boot_console):
    """
    Runs all test cases.
    """
    erofs_ls_at_root(u_boot_console)
    erofs_ls_at_subdir(u_boot_console)
    erofs_ls_at_symlink(u_boot_console)
    erofs_ls_at_non_existent_dir(u_boot_console)
    erofs_load_files_at_root(u_boot_console)
    erofs_load_files_at_subdir(u_boot_console)
    erofs_load_files_at_symlink(u_boot_console)
    erofs_load_non_existent_file(u_boot_console)

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_fs_generic')
@pytest.mark.buildconfigspec('cmd_erofs')
@pytest.mark.buildconfigspec('fs_erofs')
@pytest.mark.requiredtool('mkfs.erofs')
@pytest.mark.requiredtool('md5sum')

def test_erofs(u_boot_console):
    """
    Executes the erofs test suite.
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

    try:
        # setup test environment
        make_erofs_image(build_dir)
        image_path = os.path.join(build_dir, EROFS_IMAGE_NAME)
        u_boot_console.run_command('host bind 0 {}'.format(image_path))
        # run all tests
        erofs_run_all_tests(u_boot_console)
    except:
        clean_erofs_image(build_dir)
        raise AssertionError

    # clean test environment
    clean_erofs_image(build_dir)
