# SPDX-License-Identifier:      GPL-2.0+
#
# Copyright (c) 2018, Linaro Limited
# Author: Takahiro Akashi <takahiro.akashi@linaro.org>

"""Helper functions for dealing with filesystems"""

import re
import os
from subprocess import call, check_call, check_output, CalledProcessError

def mk_fs(config, fs_type, size, prefix, size_gran = 0x100000):
    """Create a file system volume

    Args:
        config (u_boot_config): U-Boot configuration
        fs_type (str): File system type, e.g. 'ext4'
        size (int): Size of file system in bytes
        prefix (str): Prefix string of volume's file name
        size_gran (int): Size granularity of file system image in bytes

    Raises:
        CalledProcessError: if any error occurs when creating the filesystem
    """
    fs_img = f'{prefix}.{fs_type}.img'
    fs_img = os.path.join(config.persistent_data_dir, fs_img)

    if fs_type == 'fat12':
        mkfs_opt = '-F 12'
    elif fs_type == 'fat16':
        mkfs_opt = '-F 16'
    elif fs_type == 'fat32':
        mkfs_opt = '-F 32'
    else:
        mkfs_opt = ''

    if re.match('fat', fs_type):
        fs_lnxtype = 'vfat'
    else:
        fs_lnxtype = fs_type

    count = (size + size_gran - 1) // size_gran

    # Some distributions do not add /sbin to the default PATH, where mkfs lives
    if '/sbin' not in os.environ["PATH"].split(os.pathsep):
        os.environ["PATH"] += os.pathsep + '/sbin'

    try:
        check_call(f'rm -f {fs_img}', shell=True)
        check_call(f'dd if=/dev/zero of={fs_img} bs={size_gran} count={count}',
                   shell=True)
        check_call(f'mkfs.{fs_lnxtype} {mkfs_opt} {fs_img}', shell=True)
        if fs_type == 'ext4':
            sb_content = check_output(f'tune2fs -l {fs_img}',
                                      shell=True).decode()
            if 'metadata_csum' in sb_content:
                check_call(f'tune2fs -O ^metadata_csum {fs_img}', shell=True)
        return fs_img
    except CalledProcessError:
        call(f'rm -f {fs_img}', shell=True)
        raise

# Just for trying out
if __name__ == "__main__":
    import collections

    CNF= collections.namedtuple('config', 'persistent_data_dir')

    mk_fs(CNF('.'), 'ext4', 0x1000000, 'pref')
