# SPDX-License-Identifier:      GPL-2.0+

""" Unit test for xxd command
"""

import pytest

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_xxd')
def test_xxd(u_boot_console, xxd_data):
    """ Unit test for xxd

    Args:
        u_boot_console -- U-Boot console
        xxd_data -- Path to the disk image used for testing.
    """
    response = u_boot_console.run_command_list([
        f'host bind 0 {xxd_data}',
        'xxd host 0 hello'])

    assert '00000000: 68 65 6c 6c 6f 20 77 6f 72 6c 64 0a 00 01 02 03  hello world.....\r\r\n' + \
           '00000010: 04 05                                            ..' \
           in response
