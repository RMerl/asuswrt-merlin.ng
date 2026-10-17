# SPDX-License-Identifier:  GPL-2.0+
#
# Copyright (c) 2021 Adarsh Babu Kalepalli <opensource.kab@gmail.com>
# Copyright (c) 2020 Alex Kiernan <alex.kiernan@gmail.com>

import pytest
import time
import u_boot_utils

"""
	test_gpio_input is intended to test the fix 4dbc107f4683.
	4dbc107f4683:"cmd: gpio: Correct do_gpio() return value"
"""

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpio')
def test_gpio_input(u_boot_console):
    """Test that gpio input correctly returns the value of a gpio pin."""

    response = u_boot_console.run_command('gpio input 0; echo rc:$?')
    expected_response = 'rc:0'
    assert(expected_response in response)
    response = u_boot_console.run_command('gpio toggle 0; gpio input 0; echo rc:$?')
    expected_response = 'rc:1'
    assert(expected_response in response)

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpio')
def test_gpio_exit_statuses(u_boot_console):
    """Test that non-input gpio commands correctly return the command
    success/failure status."""

    expected_response = 'rc:0'
    response = u_boot_console.run_command('gpio clear 0; echo rc:$?')
    assert(expected_response in response)
    response = u_boot_console.run_command('gpio set 0; echo rc:$?')
    assert(expected_response in response)
    response = u_boot_console.run_command('gpio toggle 0; echo rc:$?')
    assert(expected_response in response)
    response = u_boot_console.run_command('gpio status -a; echo rc:$?')
    assert(expected_response in response)

    expected_response = 'rc:1'
    response = u_boot_console.run_command('gpio nonexistent-command; echo rc:$?')
    assert(expected_response in response)
    response = u_boot_console.run_command('gpio input 200; echo rc:$?')
    assert(expected_response in response)

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpio')
def test_gpio_read(u_boot_console):
    """Test that gpio read correctly sets the variable to the value of a gpio pin."""

    u_boot_console.run_command('gpio clear 0')
    response = u_boot_console.run_command('gpio read var 0; echo val:$var,rc:$?')
    expected_response = 'val:0,rc:0'
    assert(expected_response in response)
    response = u_boot_console.run_command('gpio toggle 0; gpio read var 0; echo val:$var,rc:$?')
    expected_response = 'val:1,rc:0'
    assert(expected_response in response)
    response = u_boot_console.run_command('setenv var; gpio read var nonexistent-gpio; echo val:$var,rc:$?')
    expected_response = 'val:,rc:1'
    assert(expected_response in response)

"""
Generic Tests for 'gpio' command on sandbox and real hardware.
The below sequence of tests rely on env__gpio_dev_config for configuration values of gpio pins.

 Configuration data for gpio command.
 The  set,clear,toggle ,input and status options of 'gpio' command are verified.
 For sake of verification,A  LED/buzzer could be connected to GPIO pins configured as O/P.
 Logic level '1'/'0' can be applied onto GPIO pins configured as I/P


env__gpio_dev_config = {
        #the number of 'gpio_str_x' strings should equal to
        #'gpio_str_count' value
        'gpio_str_count':4 ,
        'gpio_str_1': '0',
        'gpio_str_2': '31',
        'gpio_str_3': '63',
        'gpio_str_4': '127',
        'gpio_op_pin': '64',
        'gpio_ip_pin_set':'65',
        'gpio_ip_pin_clear':'66',
        'gpio_clear_value': 'value is 0',
        'gpio_set_value': 'value is 1',
        # GPIO pin list to test gpio functionality for each pins, pin should be
        # pin names (str)
        'gpio_pin_list': ['gpio@1000031', 'gpio@1000032', 'gpio@20000033'],
        # GPIO input output list for shorted gpio pins to test gpio
        # functionality for each of pairs, where the first element is
        # configured as input and second as output
        'gpio_ip_op_list': [['gpio0', 'gpio1'], ['gpio2', 'gpio3']],
}
"""


@pytest.mark.buildconfigspec('cmd_gpio')
def test_gpio_status_all_generic(u_boot_console):
    """Test the 'gpio status' command.

	Displays all gpio pins available on the Board.
	To verify if the status of pins is displayed or not,
        the user can configure (gpio_str_count) and verify existence of certain
	pins.The details of these can be configured in 'gpio_str_n'.
        of boardenv_* (example above).User can configure any
        number of such pins and mention that count in 'gpio_str_count'.
    """

    f = u_boot_console.config.env.get('env__gpio_dev_config',False)
    if not f:
        pytest.skip("gpio not configured")

    gpio_str_count = f['gpio_str_count']

    #Display all the GPIO ports
    cmd = 'gpio status -a'
    response = u_boot_console.run_command(cmd)

    for str_value in range(1,gpio_str_count + 1):
        assert f["gpio_str_%d" %(str_value)] in response


@pytest.mark.buildconfigspec('cmd_gpio')
def test_gpio_set_generic(u_boot_console):
    """Test the 'gpio set' command.

	A specific gpio pin configured by user as output
        (mentioned in gpio_op_pin) is verified for
	'set' option

    """

    f = u_boot_console.config.env.get('env__gpio_dev_config',False)
    if not f:
        pytest.skip("gpio not configured")

    gpio_pin_adr = f['gpio_op_pin'];
    gpio_set_value = f['gpio_set_value'];


    cmd = 'gpio set ' + gpio_pin_adr
    response = u_boot_console.run_command(cmd)
    good_response = gpio_set_value
    assert good_response in response



@pytest.mark.buildconfigspec('cmd_gpio')
def test_gpio_clear_generic(u_boot_console):
    """Test the 'gpio clear' command.

	A specific gpio pin configured by user as output
        (mentioned in gpio_op_pin) is verified for
	'clear' option
    """

    f = u_boot_console.config.env.get('env__gpio_dev_config',False)
    if not f:
        pytest.skip("gpio not configured")

    gpio_pin_adr = f['gpio_op_pin'];
    gpio_clear_value = f['gpio_clear_value'];


    cmd = 'gpio clear ' + gpio_pin_adr
    response = u_boot_console.run_command(cmd)
    good_response = gpio_clear_value
    assert good_response in response


@pytest.mark.buildconfigspec('cmd_gpio')
def test_gpio_toggle_generic(u_boot_console):
    """Test the 'gpio toggle' command.

	A specific gpio pin configured by user as output
        (mentioned in gpio_op_pin) is verified for
	'toggle' option
    """


    f = u_boot_console.config.env.get('env__gpio_dev_config',False)
    if not f:
        pytest.skip("gpio not configured")

    gpio_pin_adr = f['gpio_op_pin'];
    gpio_set_value = f['gpio_set_value'];
    gpio_clear_value = f['gpio_clear_value'];

    cmd = 'gpio set ' + gpio_pin_adr
    response = u_boot_console.run_command(cmd)
    good_response = gpio_set_value
    assert good_response in response

    cmd = 'gpio toggle ' + gpio_pin_adr
    response = u_boot_console.run_command(cmd)
    good_response = gpio_clear_value
    assert good_response in response


@pytest.mark.buildconfigspec('cmd_gpio')
def test_gpio_input_generic(u_boot_console):
    """Test the 'gpio input' command.

	Specific gpio pins configured by user as input
        (mentioned in gpio_ip_pin_set and gpio_ip_pin_clear)
	is verified for logic '1' and logic '0' states
    """

    f = u_boot_console.config.env.get('env__gpio_dev_config',False)
    if not f:
        pytest.skip("gpio not configured")

    gpio_pin_adr = f['gpio_ip_pin_clear'];
    gpio_clear_value = f['gpio_clear_value'];


    cmd = 'gpio input ' + gpio_pin_adr
    response = u_boot_console.run_command(cmd)
    good_response = gpio_clear_value
    assert good_response in response


    gpio_pin_adr = f['gpio_ip_pin_set'];
    gpio_set_value = f['gpio_set_value'];


    cmd = 'gpio input ' + gpio_pin_adr
    response = u_boot_console.run_command(cmd)
    good_response = gpio_set_value
    assert good_response in response

@pytest.mark.buildconfigspec('cmd_gpio')
def test_gpio_pins_generic(u_boot_console):
    """Test various gpio related functionality, such as the input, set, clear,
       and toggle for the set of gpio pin list.

       Specific set of gpio pins (by mentioning gpio pin name) configured as
       input (mentioned as 'gpio_pin_list') to be tested for multiple gpio
       commands.
    """

    f = u_boot_console.config.env.get('env__gpio_dev_config', False)
    if not f:
        pytest.skip('gpio not configured')

    gpio_pins = f.get('gpio_pin_list', None)
    if not gpio_pins:
        pytest.skip('gpio pin list are not configured')

    for gpin in gpio_pins:
        # gpio input
        u_boot_console.run_command(f'gpio input {gpin}')
        expected_response = f'{gpin}: input:'
        response = u_boot_console.run_command(f'gpio status -a {gpin}')
        assert expected_response in response

        # gpio set
        u_boot_console.run_command(f'gpio set {gpin}')
        expected_response = f'{gpin}: output: 1'
        response = u_boot_console.run_command(f'gpio status -a {gpin}')
        assert expected_response in response

        # gpio clear
        u_boot_console.run_command(f'gpio clear {gpin}')
        expected_response = f'{gpin}: output: 0'
        response = u_boot_console.run_command(f'gpio status -a {gpin}')
        assert expected_response in response

        # gpio toggle
        u_boot_console.run_command(f'gpio toggle {gpin}')
        expected_response = f'{gpin}: output: 1'
        response = u_boot_console.run_command(f'gpio status -a {gpin}')
        assert expected_response in response

@pytest.mark.buildconfigspec('cmd_gpio')
def test_gpio_pins_input_output_generic(u_boot_console):
    """Test gpio related functionality such as input and output for the list of
       shorted gpio pins provided as a pair of input and output pins. This test
       will fail, if the gpio pins are not shorted properly.

       Specific set of shorted gpio pins (by mentioning gpio pin name)
       configured as input and output (mentioned as 'gpio_ip_op_list') as a
       pair to be tested for gpio input output case.
    """

    f = u_boot_console.config.env.get('env__gpio_dev_config', False)
    if not f:
        pytest.skip('gpio not configured')

    gpio_pins = f.get('gpio_ip_op_list', None)
    if not gpio_pins:
        pytest.skip('gpio pin list for input and output are not configured')

    for gpins in gpio_pins:
        u_boot_console.run_command(f'gpio input {gpins[0]}')
        expected_response = f'{gpins[0]}: input:'
        response = u_boot_console.run_command(f'gpio status -a {gpins[0]}')
        assert expected_response in response

        u_boot_console.run_command(f'gpio set {gpins[1]}')
        expected_response = f'{gpins[1]}: output:'
        response = u_boot_console.run_command(f'gpio status -a {gpins[1]}')
        assert expected_response in response

        u_boot_console.run_command(f'gpio clear {gpins[1]}')
        expected_response = f'{gpins[0]}: input: 0'
        response = u_boot_console.run_command(f'gpio status -a {gpins[0]}')
        assert expected_response in response

        u_boot_console.run_command(f'gpio set {gpins[1]}')
        expected_response = f'{gpins[0]}: input: 1'
        response = u_boot_console.run_command(f'gpio status -a {gpins[0]}')
        assert expected_response in response
