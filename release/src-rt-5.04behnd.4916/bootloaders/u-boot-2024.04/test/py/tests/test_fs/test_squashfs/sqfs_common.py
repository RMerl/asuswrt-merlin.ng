# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2020 Bootlin
# Author: Joao Marcos Costa <joaomarcos.costa@bootlin.com>

import os
import shutil
import subprocess

""" standard test images table: Each table item is a key:value pair
representing the output image name and its respective mksquashfs options.
This table should be modified only when adding support for new compression
algorithms. The 'default' case takes no options but the input and output
names, so it must be assigned with an empty string.
"""
STANDARD_TABLE = {
        'default' : '',
        'lzo_comp_frag' : '',
        'lzo_frag' : '',
        'lzo_no_frag' : '',
        'zstd_comp_frag' : '',
        'zstd_frag' : '',
        'zstd_no_frag' : '',
        'gzip_comp_frag' : '',
        'gzip_frag' : '',
        'gzip_no_frag' : ''
}

""" EXTRA_TABLE: Set this table's keys and values if you want to make squashfs
images with your own customized options.
"""
EXTRA_TABLE = {}

# path to source directory used to make squashfs test images
SQFS_SRC_DIR = 'sqfs_src_dir'

def get_opts_list():
    """ Combines fragmentation and compression options into a list of strings.

    opts_list's firts item is an empty string as STANDARD_TABLE's first item is
    the 'default' case.

    Returns:
        A list of strings whose items are formed by a compression and a
        fragmentation option joined by a whitespace.
    """
    # supported compression options only
    comp_opts = ['-comp lzo', '-comp zstd', '-comp gzip']
    # file fragmentation options
    frag_opts = ['-always-use-fragments', '-always-use-fragments -noF', '-no-fragments']

    opts_list = [' ']
    for comp_opt in comp_opts:
        for frag_opt in frag_opts:
            opts_list.append(' '.join([comp_opt, frag_opt]))

    return opts_list

def init_standard_table():
    """ Initializes STANDARD_TABLE values.

    STANDARD_TABLE's keys are pre-defined, and init_standard_table() assigns
    the right value for each one of them.
    """
    opts_list = get_opts_list()

    for key, value in zip(STANDARD_TABLE.keys(), opts_list):
        STANDARD_TABLE[key] = value

def generate_file(file_name, file_size):
    """ Generates a file filled with 'x'.

    Args:
        file_name: the file's name.
        file_size: the content's length and therefore the file size.
    """
    content = 'x' * file_size

    file = open(file_name, 'w')
    file.write(content)
    file.close()

def generate_sqfs_src_dir(build_dir):
    """ Generates the source directory used to make the SquashFS images.

    The source directory is generated at build_dir, and it has the following
    structure:
    sqfs_src_dir/
    ├── empty-dir/
    ├── f1000
    ├── f4096
    ├── f5096
    ├── subdir/
    │   └── subdir-file
    └── sym -> subdir

    3 directories, 4 files

    The files in the root dir. are prefixed with an 'f' followed by its size.

    Args:
        build_dir: u-boot's build-sandbox directory.
    """

    root = os.path.join(build_dir, SQFS_SRC_DIR)
    # make root directory
    os.makedirs(root)

    # 4096: minimum block size
    file_name = 'f4096'
    generate_file(os.path.join(root, file_name), 4096)

    # 5096: minimum block size + 1000 chars (fragment)
    file_name = 'f5096'
    generate_file(os.path.join(root, file_name), 5096)

    # 1000: less than minimum block size (fragment only)
    file_name = 'f1000'
    generate_file(os.path.join(root, file_name), 1000)

    # sub-directory with a single file inside
    subdir_path = os.path.join(root, 'subdir')
    os.makedirs(subdir_path)
    generate_file(os.path.join(subdir_path, 'subdir-file'), 100)

    # symlink (target: sub-directory)
    os.symlink('subdir', os.path.join(root, 'sym'))

    # empty directory
    os.makedirs(os.path.join(root, 'empty-dir'))

def mksquashfs(args):
    """ Runs mksquashfs command.

    Args:
        args: mksquashfs options (e.g.: compression and fragmentation).
    """
    subprocess.run(['mksquashfs ' + args], shell=True, check=True,
                   stdout=subprocess.DEVNULL)

def get_mksquashfs_version():
    """ Parses the output of mksquashfs -version.

    Returns:
        mksquashfs's version as a float.
    """
    out = subprocess.run(['mksquashfs -version'], shell=True, check=True,
                         capture_output=True, text=True)
    # 'out' is: mksquashfs version X (yyyy/mm/dd) ...
    return out.stdout.split()[2].split('.')[0:2]

def check_mksquashfs_version():
    """ Checks if mksquashfs meets the required version. """

    version = get_mksquashfs_version();
    if int(version[0]) < 4 or int(version[0]) == 4 and int(version[1]) < 4 :
        print('Error: mksquashfs is too old, required version: 4.4')
        raise AssertionError

def make_all_images(build_dir):
    """ Makes the SquashFS images used in the test suite.

    The image names and respective mksquashfs options are defined in STANDARD_TABLE
    and EXTRA_TABLE. The destination is defined by 'build_dir'.

    Args:
        build_dir: u-boot's build-sandbox directory.
    """

    init_standard_table()
    input_path = os.path.join(build_dir, SQFS_SRC_DIR)

    # make squashfs images according to STANDARD_TABLE
    for out, opts in zip(STANDARD_TABLE.keys(), STANDARD_TABLE.values()):
        output_path = os.path.join(build_dir, out)
        mksquashfs(' '.join([input_path, output_path, opts]))

    # make squashfs images according to EXTRA_TABLE
    for out, opts in zip(EXTRA_TABLE.keys(), EXTRA_TABLE.values()):
        output_path = os.path.join(build_dir, out)
        mksquashfs(' '.join([input_path, output_path, opts]))

def clean_all_images(build_dir):
    """ Deletes the SquashFS images at build_dir.

    Args:
        build_dir: u-boot's build-sandbox directory.
    """

    for image_name in STANDARD_TABLE:
        image_path = os.path.join(build_dir, image_name)
        os.remove(image_path)

    for image_name in EXTRA_TABLE:
        image_path = os.path.join(build_dir, image_name)
        os.remove(image_path)

def clean_sqfs_src_dir(build_dir):
    """ Deletes the source directory at build_dir.

    Args:
        build_dir: u-boot's build-sandbox directory.
    """
    path = os.path.join(build_dir, SQFS_SRC_DIR)
    shutil.rmtree(path)
