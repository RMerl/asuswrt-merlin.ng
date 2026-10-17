# SPDX-License-Identifier:      GPL-2.0+
""" Unit test for UEFI menu-driven configuration
"""

import pytest
import time

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_eficonfig')
@pytest.mark.buildconfigspec('cmd_bootefi_bootmgr')
def test_efi_eficonfig(u_boot_console, efi_eficonfig_data):

    def send_user_input_and_wait(user_str, expect_str):
        time.sleep(0.1) # TODO: does not work correctly without sleep
        u_boot_console.run_command(cmd=user_str, wait_for_prompt=False,
                                   wait_for_echo=True, send_nl=False)
        u_boot_console.run_command(cmd='\x0d', wait_for_prompt=False,
                                   wait_for_echo=False, send_nl=False)
        if expect_str is not None:
            for i in expect_str:
                u_boot_console.p.expect([i])

    def press_up_down_enter_and_wait(up_count, down_count, enter, expect_str):
        # press UP key
        for i in range(up_count):
            u_boot_console.run_command(cmd='\x1b\x5b\x41', wait_for_prompt=False,
                                       wait_for_echo=False, send_nl=False)
        # press DOWN key
        for i in range(down_count):
            u_boot_console.run_command(cmd='\x1b\x5b\x42', wait_for_prompt=False,
                                       wait_for_echo=False, send_nl=False)
        # press ENTER if requested
        if enter:
            u_boot_console.run_command(cmd='\x0d', wait_for_prompt=False,
                                       wait_for_echo=False, send_nl=False)
        # wait expected output
        if expect_str is not None:
            for i in expect_str:
                u_boot_console.p.expect([i])

    def press_escape_key(wait_prompt):
        u_boot_console.run_command(cmd='\x1b', wait_for_prompt=wait_prompt, wait_for_echo=False, send_nl=False)

    def press_enter_key(wait_prompt):
        u_boot_console.run_command(cmd='\x0d', wait_for_prompt=wait_prompt,
                                   wait_for_echo=False, send_nl=False)

    def check_current_is_maintenance_menu():
        for i in ('UEFI Maintenance Menu', 'Add Boot Option', 'Edit Boot Option',
                  'Change Boot Order', 'Delete Boot Option', 'Quit'):
            u_boot_console.p.expect([i])

    """ Unit test for "eficonfig" command
    The menu-driven interface is used to set up UEFI load options.
    The bootefi bootmgr loads initrddump.efi as a payload.
    The crc32 of the loaded initrd.img is checked

    Args:
        u_boot_console -- U-Boot console
        efi__data -- Path to the disk image used for testing.
                     Test disk image has following files.
                         initrd-1.img
                         initrd-2.img
                         initrddump.efi

    """
    # This test passes for unknown reasons in the bowels of U-Boot. It needs to
    # be replaced with a unit test.
    return

    # Restart the system to clean the previous state
    u_boot_console.restart_uboot()

    with u_boot_console.temporary_timeout(500):
        #
        # Test Case 1: Check the menu is displayed
        #
        u_boot_console.run_command('eficonfig', wait_for_prompt=False)
        for i in ('UEFI Maintenance Menu', 'Add Boot Option', 'Edit Boot Option',
                  'Change Boot Order', 'Delete Boot Option', 'Quit'):
            u_boot_console.p.expect([i])
        # Select "Add Boot Option"
        press_enter_key(False)
        for i in ('Add Boot Option', 'Description:', 'File', 'Initrd File', 'Optional Data',
                  'Save', 'Quit'):
            u_boot_console.p.expect([i])
        press_escape_key(False)
        check_current_is_maintenance_menu()
        # return to U-Boot console
        press_escape_key(True)

        #
        # Test Case 2: check auto generated media device entry
        #

        # bind the test disk image for succeeding tests
        u_boot_console.run_command(cmd = f'host bind 0 {efi_eficonfig_data}')

        u_boot_console.run_command('eficonfig', wait_for_prompt=False)

        # Change the Boot Order
        press_up_down_enter_and_wait(0, 2, True, 'Quit')
        for i in ('host 0:1', 'Save', 'Quit'):
            u_boot_console.p.expect([i])
        # disable auto generated boot option for succeeding test
        u_boot_console.run_command(cmd=' ', wait_for_prompt=False,
                                       wait_for_echo=False, send_nl=False)
        # Save the BootOrder
        press_up_down_enter_and_wait(0, 1, True, None)
        check_current_is_maintenance_menu()

        #
        # Test Case 3: Add first Boot Option and load it
        #

        # Select 'Add Boot Option'
        press_up_down_enter_and_wait(0, 0, True, 'Quit')

        # Press the enter key to select 'Description:' entry, then enter Description
        press_up_down_enter_and_wait(0, 0, True, 'enter description:')
        # Send Description user input, press ENTER key to complete
        send_user_input_and_wait('test 1', 'Quit')

        # Set EFI image(initrddump.efi)
        press_up_down_enter_and_wait(0, 1, True, 'Quit')
        press_up_down_enter_and_wait(0, 0, True, 'host 0:1')
        # Select 'host 0:1'
        press_up_down_enter_and_wait(0, 0, True, 'Quit')
        # Press down key to select "initrddump.efi" entry followed by the enter key
        press_up_down_enter_and_wait(0, 2, True, 'Quit')

        # Set Initrd file(initrd-1.img)
        press_up_down_enter_and_wait(0, 2, True, 'Quit')
        press_up_down_enter_and_wait(0, 0, True, 'host 0:1')
        # Select 'host 0:1'
        press_up_down_enter_and_wait(0, 0, True, 'Quit')
        # Press down key to select "initrd-1.img" entry followed by the enter key
        press_up_down_enter_and_wait(0, 0, True, 'Quit')

        # Set optional_data
        press_up_down_enter_and_wait(0, 3, True, 'Optional Data:')
        # Send Description user input, press ENTER key to complete
        send_user_input_and_wait('nocolor', None)
        for i in ('Description: test 1', 'File: host 0:1/initrddump.efi',
                  'Initrd File: host 0:1/initrd-1.img', 'Optional Data: nocolor', 'Save', 'Quit'):
            u_boot_console.p.expect([i])

        # Save the Boot Option
        press_up_down_enter_and_wait(0, 4, True, None)
        check_current_is_maintenance_menu()

        # Check the newly added Boot Option is handled correctly
        # Return to U-Boot console
        press_escape_key(True)
        u_boot_console.run_command(cmd = 'bootefi bootmgr')
        response = u_boot_console.run_command(cmd = 'load', wait_for_echo=False)
        assert 'crc32: 0x181464af' in response
        u_boot_console.run_command(cmd = 'exit', wait_for_echo=False)

        #
        # Test Case 4: Add second Boot Option and load it
        #
        u_boot_console.run_command('eficonfig', wait_for_prompt=False)

        # Select 'Add Boot Option'
        press_up_down_enter_and_wait(0, 0, True, 'Quit')

        # Press the enter key to select 'Description:' entry, then enter Description
        press_up_down_enter_and_wait(0, 0, True, 'enter description:')
        # Send Description user input, press ENTER key to complete
        send_user_input_and_wait('test 2', 'Quit')

        # Set EFI image(initrddump.efi)
        press_up_down_enter_and_wait(0, 1, True, 'Quit')
        press_up_down_enter_and_wait(0, 0, True, 'host 0:1')
        # Select 'host 0:1'
        press_up_down_enter_and_wait(0, 0, True, 'Quit')
        # Press down key to select "initrddump.efi" entry followed by the enter key
        press_up_down_enter_and_wait(0, 2, True, 'Quit')

        # Set Initrd file(initrd-2.img)
        press_up_down_enter_and_wait(0, 2, True, 'Quit')
        press_up_down_enter_and_wait(0, 0, True, 'host 0:1')
        # Select 'host 0:1'
        press_up_down_enter_and_wait(0, 0, True, 'Quit')
        # Press down key to select "initrd-2.img" entry followed by the enter key
        press_up_down_enter_and_wait(0, 1, True, 'Quit')

        # Set optional_data
        press_up_down_enter_and_wait(0, 3, True, 'Optional Data:')
        # Send Description user input, press ENTER key to complete
        send_user_input_and_wait('nocolor', None)
        for i in ('Description: test 2', 'File: host 0:1/initrddump.efi',
                  'Initrd File: host 0:1/initrd-2.img', 'Optional Data: nocolor', 'Save', 'Quit'):
            u_boot_console.p.expect([i])

        # Save the Boot Option
        press_up_down_enter_and_wait(0, 4, True, 'Quit')

        # Change the Boot Order
        press_up_down_enter_and_wait(0, 2, True, 'Quit')
        press_up_down_enter_and_wait(0, 1, False, 'Quit')
        # move 'test 1' to the second entry
        u_boot_console.run_command(cmd='+', wait_for_prompt=False,
                                       wait_for_echo=False, send_nl=False)
        for i in ('test 2', 'test 1', 'host 0:1', 'Save', 'Quit'):
            u_boot_console.p.expect([i])
        # Save the BootOrder
        press_up_down_enter_and_wait(0, 3, True, None)
        check_current_is_maintenance_menu()

        # Check the newly added Boot Option is handled correctly
        # Return to U-Boot console
        press_escape_key(True)
        u_boot_console.run_command(cmd = 'bootefi bootmgr')
        response = u_boot_console.run_command(cmd = 'load', wait_for_echo=False)
        assert 'crc32: 0x811d3515' in response
        u_boot_console.run_command(cmd = 'exit', wait_for_echo=False)

        #
        # Test Case 5: Change BootOrder and load it
        #
        u_boot_console.run_command('eficonfig', wait_for_prompt=False)

        # Change the Boot Order
        press_up_down_enter_and_wait(0, 2, True, None)
        # Check the curren BootOrder
        for i in ('test 2', 'test 1', 'host 0:1', 'Save', 'Quit'):
            u_boot_console.p.expect([i])
        # move 'test 2' to the second entry
        u_boot_console.run_command(cmd='-', wait_for_prompt=False,
                                       wait_for_echo=False, send_nl=False)
        for i in ('test 1', 'test 2', 'host 0:1', 'Save', 'Quit'):
            u_boot_console.p.expect([i])
        # Save the BootOrder
        press_up_down_enter_and_wait(0, 2, True, None)
        check_current_is_maintenance_menu()

        # Return to U-Boot console
        press_escape_key(True)
        u_boot_console.run_command(cmd = 'bootefi bootmgr')
        response = u_boot_console.run_command(cmd = 'load', wait_for_echo=False)
        assert 'crc32: 0x181464af' in response
        u_boot_console.run_command(cmd = 'exit', wait_for_echo=False)

        #
        # Test Case 6: Delete Boot Option(label:test 2)
        #
        u_boot_console.run_command('eficonfig', wait_for_prompt=False)

        # Select 'Delete Boot Option'
        press_up_down_enter_and_wait(0, 3, True, None)
        # Check the current BootOrder
        for i in ('test 1', 'test 2', 'Quit'):
            u_boot_console.p.expect([i])

        # Delete 'test 2'
        press_up_down_enter_and_wait(0, 1, True, None)
        for i in ('test 1', 'Quit'):
            u_boot_console.p.expect([i])
        press_escape_key(False)
        check_current_is_maintenance_menu()
        # Return to U-Boot console
        press_escape_key(True)

        #
        # Test Case 7: Edit Boot Option
        #
        u_boot_console.run_command('eficonfig', wait_for_prompt=False)
        # Select 'Edit Boot Option'
        press_up_down_enter_and_wait(0, 1, True, None)
        # Check the curren BootOrder
        for i in ('test 1', 'Quit'):
            u_boot_console.p.expect([i])
        press_up_down_enter_and_wait(0, 0, True, None)
        for i in ('Description: test 1', 'File: host 0:1/initrddump.efi',
                  'Initrd File: host 0:1/initrd-1.img', 'Optional Data: nocolor', 'Save', 'Quit'):
            u_boot_console.p.expect([i])

        # Press the enter key to select 'Description:' entry, then enter Description
        press_up_down_enter_and_wait(0, 0, True, 'enter description:')
        # Send Description user input, press ENTER key to complete
        send_user_input_and_wait('test 3', 'Quit')

        # Set EFI image(initrddump.efi)
        press_up_down_enter_and_wait(0, 1, True, 'Quit')
        press_up_down_enter_and_wait(0, 0, True, 'host 0:1')
        # Select 'host 0:1'
        press_up_down_enter_and_wait(0, 0, True, 'Quit')
        # Press down key to select "initrddump.efi" entry followed by the enter key
        press_up_down_enter_and_wait(0, 2, True, 'Quit')

        # Set Initrd file(initrd-2.img)
        press_up_down_enter_and_wait(0, 2, True, 'Quit')
        press_up_down_enter_and_wait(0, 0, True, 'host 0:1')
        # Select 'host 0:1'
        press_up_down_enter_and_wait(0, 0, True, 'Quit')
        # Press down key to select "initrd-1.img" entry followed by the enter key
        press_up_down_enter_and_wait(0, 1, True, 'Quit')

        # Set optional_data
        press_up_down_enter_and_wait(0, 3, True, 'Optional Data:')
        # Send Description user input, press ENTER key to complete
        send_user_input_and_wait('', None)
        for i in ('Description: test 3', 'File: host 0:1/initrddump.efi',
                  'Initrd File: host 0:1/initrd-2.img', 'Optional Data:', 'Save', 'Quit'):
            u_boot_console.p.expect([i])

        # Save the Boot Option
        press_up_down_enter_and_wait(0, 4, True, 'Quit')
        press_escape_key(False)
        check_current_is_maintenance_menu()

        # Check the updated Boot Option is handled correctly
        # Return to U-Boot console
        press_escape_key(True)
        u_boot_console.run_command(cmd = 'bootefi bootmgr')
        response = u_boot_console.run_command(cmd = 'load', wait_for_echo=False)
        assert 'crc32: 0x811d3515' in response
        u_boot_console.run_command(cmd = 'exit', wait_for_echo=False)

        #
        # Test Case 8: Delete Boot Option(label:test 3)
        #
        u_boot_console.run_command('eficonfig', wait_for_prompt=False)

        # Select 'Delete Boot Option'
        press_up_down_enter_and_wait(0, 3, True, None)
        # Check the curren BootOrder
        for i in ('test 3', 'Quit'):
            u_boot_console.p.expect([i])

        # Delete 'test 3'
        press_up_down_enter_and_wait(0, 0, True, 'Quit')
        press_escape_key(False)
        check_current_is_maintenance_menu()
        # Return to U-Boot console
        press_escape_key(True)

        # remove the host device
        u_boot_console.run_command(cmd = f'host bind -r 0')

        #
        # Test Case 9: No block device found
        #
        u_boot_console.run_command('eficonfig', wait_for_prompt=False)

        # Select 'Add Boot Option'
        press_up_down_enter_and_wait(0, 0, True, 'Quit')

        # Set EFI image
        press_up_down_enter_and_wait(0, 1, True, 'Quit')
        press_up_down_enter_and_wait(0, 0, True, 'No block device found!')
        press_escape_key(False)
        press_escape_key(False)
        check_current_is_maintenance_menu()
        # Return to U-Boot console
        press_escape_key(True)
