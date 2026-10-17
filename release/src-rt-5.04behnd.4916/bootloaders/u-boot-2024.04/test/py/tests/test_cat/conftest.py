# SPDX-License-Identifier:      GPL-2.0+

"""Fixture for cat command test
"""

import os
import shutil
from subprocess import check_call, CalledProcessError
import pytest

@pytest.fixture(scope='session')
def cat_data(u_boot_config):
    """Set up a file system to be used in cat tests

    Args:
        u_boot_config -- U-Boot configuration.
    """
    mnt_point = u_boot_config.persistent_data_dir + '/test_cat'
    image_path = u_boot_config.persistent_data_dir + '/cat.img'

    try:
        os.mkdir(mnt_point, mode = 0o755)

        with open(mnt_point + '/hello', 'w', encoding = 'ascii') as file:
            file.write('hello world\n')

        check_call(f'virt-make-fs --partition=gpt --size=+1M --type=vfat {mnt_point} {image_path}',
                   shell=True)

        yield image_path
    except CalledProcessError:
        pytest.skip('Setup failed')
    finally:
        shutil.rmtree(mnt_point)
        if os.path.exists(image_path):
            os.remove(image_path)
