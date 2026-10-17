# SPDX-License-Identifier: GPL-2.0
# (C) Copyright 2023, Advanced Micro Devices, Inc.

import pytest
import random
import re

"""
Note: This test relies on boardenv_* containing configuration values to define
the i2c device info including the bus list and eeprom address/value. This test
will be automatically skipped without this.

For example:

# Setup env__i2c_device_test to set the i2c bus list and probe_all boolean
# parameter. For i2c_probe_all_buses case, if probe_all parameter is set to
# False then it probes all the buses listed in bus_list instead of probing all
# the buses available.
env__i2c_device_test = {
    'bus_list': [0, 2, 5, 12, 16, 18],
    'probe_all': False,
}

# Setup env__i2c_eeprom_device_test to set the i2c bus number, eeprom address
# and configured value for i2c_eeprom test case. Test will be skipped if
# env__i2c_eeprom_device_test is not set
env__i2c_eeprom_device_test = {
    'bus': 3,
    'eeprom_addr': 0x54,
    'eeprom_val': '30 31',
}
"""

def get_i2c_test_env(u_boot_console):
    f = u_boot_console.config.env.get("env__i2c_device_test", None)
    if not f:
        pytest.skip("No I2C device to test!")
    else:
        bus_list = f.get("bus_list", None)
        if not bus_list:
            pytest.skip("I2C bus list is not provided!")
        probe_all = f.get("probe_all", False)
        return bus_list, probe_all

@pytest.mark.buildconfigspec("cmd_i2c")
def test_i2c_bus(u_boot_console):
    bus_list, probe = get_i2c_test_env(u_boot_console)
    bus = random.choice(bus_list)
    expected_response = f"Bus {bus}:"
    response = u_boot_console.run_command("i2c bus")
    assert expected_response in response

@pytest.mark.buildconfigspec("cmd_i2c")
def test_i2c_dev(u_boot_console):
    bus_list, probe = get_i2c_test_env(u_boot_console)
    expected_response = "Current bus is"
    response = u_boot_console.run_command("i2c dev")
    assert expected_response in response

@pytest.mark.buildconfigspec("cmd_i2c")
def test_i2c_probe(u_boot_console):
    bus_list, probe = get_i2c_test_env(u_boot_console)
    bus = random.choice(bus_list)
    expected_response = f"Setting bus to {bus}"
    response = u_boot_console.run_command(f"i2c dev {bus}")
    assert expected_response in response
    expected_response = "Valid chip addresses:"
    response = u_boot_console.run_command("i2c probe")
    assert expected_response in response

@pytest.mark.buildconfigspec("cmd_i2c")
def test_i2c_eeprom(u_boot_console):
    f = u_boot_console.config.env.get("env__i2c_eeprom_device_test", None)
    if not f:
        pytest.skip("No I2C eeprom to test!")

    bus = f.get("bus", 0)
    if bus < 0:
        pytest.fail("No bus specified via env__i2c_eeprom_device_test!")

    addr = f.get("eeprom_addr", -1)
    if addr < 0:
        pytest.fail("No eeprom address specified via env__i2c_eeprom_device_test!")

    value = f.get("eeprom_val")
    if not value:
        pytest.fail(
            "No eeprom configured value provided via env__i2c_eeprom_device_test!"
        )

    # Enable i2c mux bridge
    u_boot_console.run_command("i2c dev %x" % bus)
    u_boot_console.run_command("i2c probe")
    output = u_boot_console.run_command("i2c md %x 0 5" % addr)
    assert value in output

@pytest.mark.buildconfigspec("cmd_i2c")
def test_i2c_probe_all_buses(u_boot_console):
    bus_list, probe = get_i2c_test_env(u_boot_console)
    bus = random.choice(bus_list)
    expected_response = f"Bus {bus}:"
    response = u_boot_console.run_command("i2c bus")
    assert expected_response in response

    # Get all the bus list
    if probe:
        buses = re.findall("Bus (.+?):", response)
        bus_list = [int(x) for x in buses]

    for dev in bus_list:
        expected_response = f"Setting bus to {dev}"
        response = u_boot_console.run_command(f"i2c dev {dev}")
        assert expected_response in response
        expected_response = "Valid chip addresses:"
        response = u_boot_console.run_command("i2c probe")
        assert expected_response in response
