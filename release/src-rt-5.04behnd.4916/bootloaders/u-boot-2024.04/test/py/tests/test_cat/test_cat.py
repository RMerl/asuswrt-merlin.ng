# SPDX-License-Identifier:      GPL-2.0+

""" Unit test for cat command
"""

import pytest

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_cat')
def test_cat(u_boot_console, cat_data):
    """ Unit test for cat

    Args:
        u_boot_console -- U-Boot console
        cat_data -- Path to the disk image used for testing.
    """
    response = u_boot_console.run_command_list([
        f'host bind 0 {cat_data}',
        'cat host 0 hello'])
    assert 'hello world' in response
