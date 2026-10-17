# SPDX-License-Identifier: GPL-2.0+
# Copyright 2022 Google LLC
#
# Test addition of VBE

import os

import pytest
import u_boot_utils

@pytest.mark.boardspec('sandbox_vpl')
@pytest.mark.requiredtool('dtc')
def test_vbe_vpl(u_boot_console):
    cons = u_boot_console
    #cmd = [cons.config.build_dir + fname, '-v']
    ram = os.path.join(cons.config.build_dir, 'ram.bin')
    fdt = os.path.join(cons.config.build_dir, 'arch/sandbox/dts/test.dtb')
    image_fname = os.path.join(cons.config.build_dir, 'image.bin')

    # Enable firmware1 and the mmc that it uses. These are needed for the full
    # VBE flow.
    u_boot_utils.run_and_log(
        cons, f'fdtput -t s {fdt} /bootstd/firmware0 status disabled')
    u_boot_utils.run_and_log(
        cons, f'fdtput -t s {fdt} /bootstd/firmware1 status okay')
    u_boot_utils.run_and_log(
        cons, f'fdtput -t s {fdt} /mmc3 status okay')
    u_boot_utils.run_and_log(
        cons, f'fdtput -t s {fdt} /mmc3 filename {image_fname}')

    # Remove any existing RAM file, so we don't have old data present
    if os.path.exists(ram):
        os.remove(ram)
    flags = ['-p', image_fname, '-w', '-s', 'state.dtb']
    cons.restart_uboot_with_flags(flags)

    # Make sure that VBE was used in both VPL (to load SPL) and SPL (to load
    # U-Boot
    output = cons.run_command('vbe state')
    assert output == 'Phases: VPL SPL'
