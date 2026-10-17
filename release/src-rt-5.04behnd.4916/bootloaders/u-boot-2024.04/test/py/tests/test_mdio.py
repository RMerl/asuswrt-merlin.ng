# SPDX-License-Identifier: GPL-2.0
# (C) Copyright 2023, Advanced Micro Devices, Inc.

import pytest
import re

"""
Note: This test relies on boardenv_* containing configuration values to define
the PHY device info including the device name, address, register address/value
and write data value. This test will be automatically skipped without this.

For example:

# Setup env__mdio_util_test to set the PHY address, device names, register
# address, register address value, and write data value to test mdio commands.
# Test will be skipped if env_mdio_util_test is not set
env__mdio_util_test = {
    "eth0": {"phy_addr": 0xc, "device_name": "TI DP83867", "reg": 0,
                 "reg_val": 0x1000, "write_val": 0x100},
    "eth1": {"phy_addr": 0xa0, "device_name": "TI DP83867", "reg": 1,
                 "reg_val": 0x2000, "write_val": 0x100},
}
"""

def get_mdio_test_env(u_boot_console):
    f = u_boot_console.config.env.get("env__mdio_util_test", None)
    if not f or len(f) == 0:
        pytest.skip("No PHY device to test!")
    else:
        return f

@pytest.mark.buildconfigspec("cmd_mii")
@pytest.mark.buildconfigspec("phylib")
def test_mdio_list(u_boot_console):
    f = get_mdio_test_env(u_boot_console)
    output = u_boot_console.run_command("mdio list")
    for dev, val in f.items():
        phy_addr = val.get("phy_addr")
        dev_name = val.get("device_name")

        assert f"{phy_addr:x} -" in output
        assert dev_name in output

@pytest.mark.buildconfigspec("cmd_mii")
@pytest.mark.buildconfigspec("phylib")
def test_mdio_read(u_boot_console):
    f = get_mdio_test_env(u_boot_console)
    output = u_boot_console.run_command("mdio list")
    for dev, val in f.items():
        phy_addr = hex(val.get("phy_addr"))
        dev_name = val.get("device_name")
        reg = hex(val.get("reg"))
        reg_val = hex(val.get("reg_val"))

        output = u_boot_console.run_command(f"mdio read {phy_addr} {reg}")
        assert f"PHY at address {int(phy_addr, 16):x}:" in output
        assert f"{int(reg, 16):x} - {reg_val}" in output

@pytest.mark.buildconfigspec("cmd_mii")
@pytest.mark.buildconfigspec("phylib")
def test_mdio_write(u_boot_console):
    f = get_mdio_test_env(u_boot_console)
    output = u_boot_console.run_command("mdio list")
    for dev, val in f.items():
        phy_addr = hex(val.get("phy_addr"))
        dev_name = val.get("device_name")
        reg = hex(val.get("reg"))
        reg_val = hex(val.get("reg_val"))
        wr_val = hex(val.get("write_val"))

        u_boot_console.run_command(f"mdio write {phy_addr} {reg} {wr_val}")
        output = u_boot_console.run_command(f"mdio read {phy_addr} {reg}")
        assert f"PHY at address {int(phy_addr, 16):x}:" in output
        assert f"{int(reg, 16):x} - {wr_val}" in output

        u_boot_console.run_command(f"mdio write {phy_addr} {reg} {reg_val}")
        output = u_boot_console.run_command(f"mdio read {phy_addr} {reg}")
        assert f"PHY at address {int(phy_addr, 16):x}:" in output
        assert f"{int(reg, 16):x} - {reg_val}" in output
