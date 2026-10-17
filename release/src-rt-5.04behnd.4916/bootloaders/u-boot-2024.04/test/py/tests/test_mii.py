# SPDX-License-Identifier: GPL-2.0
# (C) Copyright 2023, Advanced Micro Devices, Inc.

import pytest
import re

"""
Note: This test doesn't rely on boardenv_* configuration value but they can
change test behavior.

For example:

# Setup env__mii_deive_test_skip to True if tests with ethernet PHY devices
# should be skipped. For example: Missing PHY device
env__mii_device_test_skip = True

# Setup env__mii_device_test to set the MII device names. Test will be skipped
# if env_mii_device_test is not set
env__mii_device_test = {
    'device_list': ['eth0', 'eth1'],
}
"""

@pytest.mark.buildconfigspec("cmd_mii")
def test_mii_info(u_boot_console):
    if u_boot_console.config.env.get("env__mii_device_test_skip", False):
        pytest.skip("MII device test is not enabled!")
    expected_output = "PHY"
    output = u_boot_console.run_command("mii info")
    if not re.search(r"PHY (.+?):", output):
        pytest.skip("PHY device does not exist!")
    assert expected_output in output

@pytest.mark.buildconfigspec("cmd_mii")
def test_mii_list(u_boot_console):
    if u_boot_console.config.env.get("env__mii_device_test_skip", False):
        pytest.skip("MII device test is not enabled!")

    f = u_boot_console.config.env.get("env__mii_device_test", None)
    if not f:
        pytest.skip("No MII device to test!")

    dev_list = f.get("device_list")
    if not dev_list:
        pytest.fail("No MII device list provided via env__mii_device_test!")

    expected_output = "Current device"
    output = u_boot_console.run_command("mii device")
    mii_devices = (
        re.search(r"MII devices: '(.+)'", output).groups()[0].replace("'", "").split()
    )

    assert len([x for x in dev_list if x in mii_devices]) == len(dev_list)
    assert expected_output in output

@pytest.mark.buildconfigspec("cmd_mii")
def test_mii_set_device(u_boot_console):
    test_mii_list(u_boot_console)
    f = u_boot_console.config.env.get("env__mii_device_test", None)
    dev_list = f.get("device_list")
    output = u_boot_console.run_command("mii device")
    current_dev = re.search(r"Current device: '(.+?)'", output).groups()[0]

    for dev in dev_list:
        u_boot_console.run_command(f"mii device {dev}")
        output = u_boot_console.run_command("echo $?")
        assert output.endswith("0")

    u_boot_console.run_command(f"mii device {current_dev}")
    output = u_boot_console.run_command("mii device")
    dev = re.search(r"Current device: '(.+?)'", output).groups()[0]
    assert current_dev == dev

@pytest.mark.buildconfigspec("cmd_mii")
def test_mii_read(u_boot_console):
    test_mii_list(u_boot_console)
    output = u_boot_console.run_command("mii info")
    eth_addr = hex(int(re.search(r"PHY (.+?):", output).groups()[0], 16))
    u_boot_console.run_command(f"mii read {eth_addr} 0")
    output = u_boot_console.run_command("echo $?")
    assert output.endswith("0")

@pytest.mark.buildconfigspec("cmd_mii")
def test_mii_dump(u_boot_console):
    test_mii_list(u_boot_console)
    expected_response = "PHY control register"
    output = u_boot_console.run_command("mii info")
    eth_addr = hex(int(re.search(r"PHY (.+?):", output).groups()[0], 16))
    response = u_boot_console.run_command(f"mii dump {eth_addr} 0")
    assert expected_response in response
    output = u_boot_console.run_command("echo $?")
    assert output.endswith("0")
