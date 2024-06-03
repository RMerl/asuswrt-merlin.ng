# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2016, Xilinx Inc. Michal Simek
# Copyright (c) 2017, Xiphos Systems Corp. All rights reserved.

import re
import pytest
import random
import u_boot_utils

"""
Note: This test relies on boardenv_* containing configuration values to define
which SPI Flash areas are available for testing.  Without this, this test will
be automatically skipped.
For example:

# A list of sections of Flash memory to be tested.
env__sf_configs = (
    {
        # Where in SPI Flash should the test operate.
        'offset': 0x00000000,
        # This value is optional.
        #   If present, specifies the [[bus:]cs] argument used in `sf probe`
        #   If missing, defaults to 0.
        'id': '0:1',
        # This value is optional.
        #   If set as a number, specifies the speed of the SPI Flash.
        #   If set as an array of 2, specifies a range for a random speed.
        #   If missing, defaults to 0.
        'speed': 1000000,
        # This value is optional.
        #   If present, specifies the size to use for read/write operations.
        #   If missing, the SPI Flash page size is used as a default (based on
        #   the `sf probe` output).
        'len': 0x10000,
        # This value is optional.
        #   If present, specifies if the test can write to Flash offset
        #   If missing, defaults to False.
        'writeable': False,
        # This value is optional.
        #   If present, specifies the expected CRC32 value of the flash area.
        #   If missing, extra check is ignored.
        'crc32': 0xCAFECAFE,
    },
)
"""

def sf_prepare(u_boot_console, env__sf_config):
    """Check global state of the SPI Flash before running any test.

   Args:
        u_boot_console: A U-Boot console connection.
        env__sf_config: The single SPI Flash device configuration on which to
            run the tests.

    Returns:
        sf_params: a dictionary of SPI Flash parameters.
    """

    sf_params = {}
    sf_params['ram_base'] = u_boot_utils.find_ram_base(u_boot_console)

    probe_id = env__sf_config.get('id', 0)
    speed = env__sf_config.get('speed', 0)
    if isinstance(speed, int):
        sf_params['speed'] = speed
    else:
        assert len(speed) == 2, "If speed is a list, it must have 2 entries"
        sf_params['speed'] = random.randint(speed[0], speed[1])

    cmd = 'sf probe %d %d' % (probe_id, sf_params['speed'])

    output = u_boot_console.run_command(cmd)
    assert 'SF: Detected' in output, 'No Flash device available'

    m = re.search('page size (.+?) Bytes', output)
    assert m, 'SPI Flash page size not recognized'
    sf_params['page_size'] = int(m.group(1))

    m = re.search('erase size (.+?) KiB', output)
    assert m, 'SPI Flash erase size not recognized'
    sf_params['erase_size'] = int(m.group(1))
    sf_params['erase_size'] *= 1024

    m = re.search('total (.+?) MiB', output)
    assert m, 'SPI Flash total size not recognized'
    sf_params['total_size'] = int(m.group(1))
    sf_params['total_size'] *= 1024 * 1024

    assert 'offset' in env__sf_config, \
        '\'offset\' is required for this test.'
    sf_params['len'] = env__sf_config.get('len', sf_params['erase_size'])

    assert not env__sf_config['offset'] % sf_params['erase_size'], \
        'offset not multiple of erase size.'
    assert not sf_params['len'] % sf_params['erase_size'], \
        'erase length not multiple of erase size.'

    assert not (env__sf_config.get('writeable', False) and
                'crc32' in env__sf_config), \
        'Cannot check crc32 on writeable sections'

    return sf_params

def sf_read(u_boot_console, env__sf_config, sf_params):
    """Helper function used to read and compute the CRC32 value of a section of
    SPI Flash memory.

    Args:
        u_boot_console: A U-Boot console connection.
        env__sf_config: The single SPI Flash device configuration on which to
            run the tests.
        sf_params: SPI Flash parameters.

    Returns:
        CRC32 value of SPI Flash section
    """

    addr = sf_params['ram_base']
    offset = env__sf_config['offset']
    count = sf_params['len']
    pattern = random.randint(0, 0xFF)
    crc_expected = env__sf_config.get('crc32', None)

    cmd = 'mw.b %08x %02x %x' % (addr, pattern, count)
    u_boot_console.run_command(cmd)
    crc_pattern = u_boot_utils.crc32(u_boot_console, addr, count)
    if crc_expected:
        assert crc_pattern != crc_expected

    cmd = 'sf read %08x %08x %x' % (addr, offset, count)
    response = u_boot_console.run_command(cmd)
    assert 'Read: OK' in response, 'Read operation failed'
    crc_readback = u_boot_utils.crc32(u_boot_console, addr, count)
    assert crc_pattern != crc_readback, 'sf read did not update RAM content.'
    if crc_expected:
        assert crc_readback == crc_expected

    return crc_readback

def sf_update(u_boot_console, env__sf_config, sf_params):
    """Helper function used to update a section of SPI Flash memory.

   Args:
        u_boot_console: A U-Boot console connection.
        env__sf_config: The single SPI Flash device configuration on which to
           run the tests.

    Returns:
        CRC32 value of SPI Flash section
    """

    addr = sf_params['ram_base']
    offset = env__sf_config['offset']
    count = sf_params['len']
    pattern = int(random.random() * 0xFF)

    cmd = 'mw.b %08x %02x %x' % (addr, pattern, count)
    u_boot_console.run_command(cmd)
    crc_pattern = u_boot_utils.crc32(u_boot_console, addr, count)

    cmd = 'sf update %08x %08x %x' % (addr, offset, count)
    u_boot_console.run_command(cmd)
    crc_readback = sf_read(u_boot_console, env__sf_config, sf_params)

    assert crc_readback == crc_pattern

@pytest.mark.buildconfigspec('cmd_sf')
@pytest.mark.buildconfigspec('cmd_crc32')
@pytest.mark.buildconfigspec('cmd_memory')
def test_sf_read(u_boot_console, env__sf_config):
    sf_params = sf_prepare(u_boot_console, env__sf_config)
    sf_read(u_boot_console, env__sf_config, sf_params)

@pytest.mark.buildconfigspec('cmd_sf')
@pytest.mark.buildconfigspec('cmd_crc32')
@pytest.mark.buildconfigspec('cmd_memory')
def test_sf_read_twice(u_boot_console, env__sf_config):
    sf_params = sf_prepare(u_boot_console, env__sf_config)

    crc1 = sf_read(u_boot_console, env__sf_config, sf_params)
    sf_params['ram_base'] += 0x100
    crc2 = sf_read(u_boot_console, env__sf_config, sf_params)

    assert crc1 == crc2, 'CRC32 of two successive read operation do not match'

@pytest.mark.buildconfigspec('cmd_sf')
@pytest.mark.buildconfigspec('cmd_crc32')
@pytest.mark.buildconfigspec('cmd_memory')
def test_sf_erase(u_boot_console, env__sf_config):
    if not env__sf_config.get('writeable', False):
        pytest.skip('Flash config is tagged as not writeable')

    sf_params = sf_prepare(u_boot_console, env__sf_config)
    addr = sf_params['ram_base']
    offset = env__sf_config['offset']
    count = sf_params['len']

    cmd = 'sf erase %08x %x' % (offset, count)
    output = u_boot_console.run_command(cmd)
    assert 'Erased: OK' in output, 'Erase operation failed'

    cmd = 'mw.b %08x ff %x' % (addr, count)
    u_boot_console.run_command(cmd)
    crc_ffs = u_boot_utils.crc32(u_boot_console, addr, count)

    crc_read = sf_read(u_boot_console, env__sf_config, sf_params)
    assert crc_ffs == crc_read, 'Unexpected CRC32 after erase operation.'

@pytest.mark.buildconfigspec('cmd_sf')
@pytest.mark.buildconfigspec('cmd_crc32')
@pytest.mark.buildconfigspec('cmd_memory')
def test_sf_update(u_boot_console, env__sf_config):
    if not env__sf_config.get('writeable', False):
        pytest.skip('Flash config is tagged as not writeable')

    sf_params = sf_prepare(u_boot_console, env__sf_config)
    sf_update(u_boot_console, env__sf_config, sf_params)
