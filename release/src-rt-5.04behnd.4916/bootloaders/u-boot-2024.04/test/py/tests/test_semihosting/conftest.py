# SPDX-License-Identifier: GPL-2.0-or-later

"""Fixture for semihosting command test
"""

import os
import pytest

@pytest.fixture(scope='session')
def semihosting_data(u_boot_config):
    """Set up a file system to be used in semihosting tests

    Args:
        u_boot_config -- U-Boot configuration.
    """
    image_path = u_boot_config.persistent_data_dir + '/semihosting.txt'

    with open(image_path, 'w', encoding = 'utf-8') as file:
        file.write('Das U-Boot\n')

    yield image_path

    os.remove(image_path)
