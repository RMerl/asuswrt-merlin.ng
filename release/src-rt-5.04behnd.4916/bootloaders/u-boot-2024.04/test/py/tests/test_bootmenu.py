# SPDX-License-Identifier: GPL-2.0+

"""Test bootmenu"""

import pytest

@pytest.mark.buildconfigspec('cmd_bootmenu')
def test_bootmenu(u_boot_console):
    """Test bootmenu

    u_boot_console -- U-Boot console
    """

    with u_boot_console.temporary_timeout(500):
        u_boot_console.run_command('setenv bootmenu_default 1')
        u_boot_console.run_command('setenv bootmenu_0 test 1=echo ok 1')
        u_boot_console.run_command('setenv bootmenu_1 test 2=echo ok 2')
        u_boot_console.run_command('setenv bootmenu_2 test 3=echo ok 3')
        u_boot_console.run_command('bootmenu 2', wait_for_prompt=False)
        for i in ('U-Boot Boot Menu', 'test 1', 'test 2', 'test 3', 'autoboot'):
            u_boot_console.p.expect([i])
        # Press enter key to execute default entry
        response = u_boot_console.run_command(cmd='\x0d', wait_for_echo=False, send_nl=False)
        assert 'ok 2' in response
        u_boot_console.run_command('bootmenu 2', wait_for_prompt=False)
        u_boot_console.p.expect(['autoboot'])
        # Press up key to select prior entry followed by the enter key
        response = u_boot_console.run_command(cmd='\x1b\x5b\x41\x0d', wait_for_echo=False,
                                              send_nl=False)
        assert 'ok 1' in response
        u_boot_console.run_command('bootmenu 2', wait_for_prompt=False)
        u_boot_console.p.expect(['autoboot'])
        # Press down key to select next entry followed by the enter key
        response = u_boot_console.run_command(cmd='\x1b\x5b\x42\x0d', wait_for_echo=False,
                                              send_nl=False)
        assert 'ok 3' in response
        u_boot_console.run_command('bootmenu 2; echo rc:$?', wait_for_prompt=False)
        u_boot_console.p.expect(['autoboot'])
        # Press the escape key
        response = u_boot_console.run_command(cmd='\x1b', wait_for_echo=False, send_nl=False)
        assert 'ok' not in response
        assert 'rc:0' in response
        u_boot_console.run_command('setenv bootmenu_default')
        u_boot_console.run_command('setenv bootmenu_0')
        u_boot_console.run_command('setenv bootmenu_1')
        u_boot_console.run_command('setenv bootmenu_2')
