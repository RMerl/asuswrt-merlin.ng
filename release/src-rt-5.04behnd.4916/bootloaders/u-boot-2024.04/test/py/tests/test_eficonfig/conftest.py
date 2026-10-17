# SPDX-License-Identifier:      GPL-2.0+

"""Fixture for UEFI eficonfig test
"""

import os
import shutil
from subprocess import check_call
import pytest

@pytest.fixture(scope='session')
def efi_eficonfig_data(u_boot_config):
    """Set up a file system to be used in UEFI "eficonfig" command
       tests

    Args:
        u_boot_config -- U-Boot configuration.

    Return:
        A path to disk image to be used for testing
    """
    mnt_point = u_boot_config.persistent_data_dir + '/test_efi_eficonfig'
    image_path = u_boot_config.persistent_data_dir + '/efi_eficonfig.img'

    shutil.rmtree(mnt_point, ignore_errors=True)
    os.mkdir(mnt_point, mode = 0o755)

    with open(mnt_point + '/initrd-1.img', 'w', encoding = 'ascii') as file:
        file.write("initrd 1")

    with open(mnt_point + '/initrd-2.img', 'w', encoding = 'ascii') as file:
        file.write("initrd 2")

    shutil.copyfile(u_boot_config.build_dir + '/lib/efi_loader/initrddump.efi',
                    mnt_point + '/initrddump.efi')

    check_call(f'virt-make-fs --partition=gpt --size=+1M --type=vfat {mnt_point} {image_path}',
               shell=True)

    return image_path
