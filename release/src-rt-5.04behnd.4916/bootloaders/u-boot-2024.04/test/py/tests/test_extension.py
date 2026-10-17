# SPDX-License-Identifier:  GPL-2.0+
# Copyright (c) 2020
# Author: Kory Maincent <kory.maincent@bootlin.com>

# Test U-Boot's "extension" commands.

import os
import pytest
import u_boot_utils

overlay_addr = 0x1000

SANDBOX_DTB='arch/sandbox/dts/sandbox.dtb'
OVERLAY_DIR='arch/sandbox/dts/'

def load_dtb(u_boot_console):
    u_boot_console.log.action('Loading devicetree to RAM...')
    u_boot_console.run_command('host load hostfs - $fdt_addr_r %s' % (os.path.join(u_boot_console.config.build_dir, SANDBOX_DTB)))
    u_boot_console.run_command('fdt addr $fdt_addr_r')

@pytest.mark.buildconfigspec('cmd_fdt')
@pytest.mark.boardspec('sandbox')
def test_extension(u_boot_console):
    """Test the 'extension' command."""

    load_dtb(u_boot_console)

    output = u_boot_console.run_command('extension list')
    assert('No extension' in output)

    output = u_boot_console.run_command('extension scan')
    assert output == 'Found 2 extension board(s).'

    output = u_boot_console.run_command('extension list')
    assert('overlay0.dtbo' in output)
    assert('overlay1.dtbo' in output)

    u_boot_console.run_command_list([
        'setenv extension_overlay_addr %s' % (overlay_addr),
        'setenv extension_overlay_cmd \'host load hostfs - ${extension_overlay_addr} %s${extension_overlay_name}\'' % (os.path.join(u_boot_console.config.build_dir, OVERLAY_DIR))])

    output = u_boot_console.run_command('extension apply 0')
    assert('bytes read' in output)

    output = u_boot_console.run_command('fdt print')
    assert('button3' in output)

    output = u_boot_console.run_command('extension apply all')
    assert('bytes read' in output)

    output = u_boot_console.run_command('fdt print')
    assert('button4' in output)

