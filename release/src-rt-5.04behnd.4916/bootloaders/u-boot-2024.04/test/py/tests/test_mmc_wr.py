# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2019, Texas Instrument
# Author: Jean-Jacques Hiblot <jjhiblot@ti.com>

# Test U-Boot's "mmc write" command. The test generates random data, writes it
# to the eMMC or SD card, then reads it back and performs a comparison.

import pytest
import u_boot_utils

"""
This test relies on boardenv_* to containing configuration values to define
which MMC devices should be tested. For example:

env__mmc_wr_configs = (
    {
        "fixture_id": "emmc-boot0",
        "is_emmc": True,
        "devid": 1,
        "partid": 1,
        "sector": 0x10,
        "count": 100,
        "test_iterations": 50,
    },
    {
        "fixture_id": "emmc-boot1",
        "is_emmc": True,
        "devid": 1,
        "partid": 2,
        "sector": 0x10,
        "count": 100,
        "test_iterations": 50,
    },
)

"""

@pytest.mark.buildconfigspec('cmd_mmc')
@pytest.mark.buildconfigspec('cmd_memory')
@pytest.mark.buildconfigspec('cmd_random')
def test_mmc_wr(u_boot_console, env__mmc_wr_config):
    """Test the "mmc write" command.

    Args:
        u_boot_console: A U-Boot console connection.
        env__mmc_wr_config: The single MMC configuration on which
            to run the test. See the file-level comment above for details
            of the format.

    Returns:
        Nothing.
    """

    is_emmc = env__mmc_wr_config['is_emmc']
    devid = env__mmc_wr_config['devid']
    partid = env__mmc_wr_config.get('partid', 0)
    sector = env__mmc_wr_config.get('sector', 0)
    count_sectors = env__mmc_wr_config.get('count', 1)
    test_iterations = env__mmc_wr_config.get('test_iterations', 1)


    count_bytes = count_sectors * 512
    bcfg = u_boot_console.config.buildconfig
    ram_base = u_boot_utils.find_ram_base(u_boot_console)
    src_addr = '0x%08x' % ram_base
    dst_addr = '0x%08x' % (ram_base + count_bytes)


    for i in range(test_iterations):
        # Generate random data
        cmd = 'random %s %x' % (src_addr, count_bytes)
        response = u_boot_console.run_command(cmd)
        good_response = '%d bytes filled with random data' % (count_bytes)
        assert good_response in response

        # Select MMC device
        cmd = 'mmc dev %d' % devid
        if is_emmc:
            cmd += ' %d' % partid
        response = u_boot_console.run_command(cmd)
        assert 'no card present' not in response
        if is_emmc:
            partid_response = "(part %d)" % partid
        else:
            partid_response = ""
        good_response = 'mmc%d%s is current device' % (devid, partid_response)
        assert good_response in response

        # Write data
        cmd = 'mmc write %s %x %x' % (src_addr, sector, count_sectors)
        response = u_boot_console.run_command(cmd)
        good_response = 'MMC write: dev # %d, block # %d, count %d ... %d blocks written: OK' % (devid, sector, count_sectors, count_sectors)
        assert good_response in response

        # Read data
        cmd = 'mmc read %s %x %x' % (dst_addr, sector, count_sectors)
        response = u_boot_console.run_command(cmd)
        good_response = 'MMC read: dev # %d, block # %d, count %d ... %d blocks read: OK' % (devid, sector, count_sectors, count_sectors)
        assert good_response in response

        # Compare src and dst data
        cmd = 'cmp.b %s %s %x' % (src_addr, dst_addr, count_bytes)
        response = u_boot_console.run_command(cmd)
        good_response = 'Total of %d byte(s) were the same' % (count_bytes)
        assert good_response in response
