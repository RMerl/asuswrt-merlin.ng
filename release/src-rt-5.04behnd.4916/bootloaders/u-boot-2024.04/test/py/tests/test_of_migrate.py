# SPDX-License-Identifier: GPL-2.0
# Copyright 2023 Google LLC
# Written by Simon Glass <sjg@chromium.org>

"""Test handling of unmigrated u-boot,dm- tags"""

import os
import pytest

import u_boot_utils as util

# This is needed for Azure, since the default '..' directory is not writeable
TMPDIR1 = '/tmp/test_no_migrate'
TMPDIR2 = '/tmp/test_no_migrate_spl'
TMPDIR3 = '/tmp/test_migrate'

def build_for_migrate(cons, replace_pair, board, tmpdir, disable_migrate=True):
    """Build an updated U-Boot with a slightly modified device tree

    Args:
        cons (ConsoleBase): U-Boot console
        replace_pair (tuple):
            String to find
            String to replace it with
        board (str): Board to build
        tmpdir (str): Temporary directory to use
        disable_migrate (bool): True to disable CONFIG_OF_TAG_MIGRATE in build
    """
    srcdir = cons.config.source_dir
    build_dir = cons.config.build_dir

    # Get the source for the existing dts
    dt_dir = os.path.join(build_dir, 'arch', 'sandbox', 'dts')
    orig_fname = os.path.join(dt_dir, 'sandbox.dtb')
    out_dts = os.path.join(dt_dir, 'sandbox_out.dts')
    util.run_and_log(cons, ['dtc', orig_fname, '-I', 'dtb', '-O', 'dts',
                            '-o', out_dts])

    # Update it to use an old tag
    with open(out_dts) as inf:
        data = inf.read()
    data = data.replace(*replace_pair)

    dts_fname = os.path.join(dt_dir, 'sandbox_oldtag.dts')
    with open(dts_fname, 'w') as outf:
        print(data, file=outf)
    dtb_fname = os.path.join(dt_dir, 'sandbox_oldtag.dtb')
    util.run_and_log(cons, ['dtc', dts_fname, '-o', dtb_fname])

    migrate = ['-a', '~CONFIG_OF_TAG_MIGRATE'] if disable_migrate else []

    # Build sandbox with this new dtb, turning off OF_TAG_MIGRATE
    env = dict(os.environ)
    env['EXT_DTB'] = dtb_fname
    env['DEVICE_TREE'] = 'sandbox_new'
    env['NO_LTO'] = '1'  # Speed up build
    out = util.run_and_log(
        cons, ['./tools/buildman/buildman', '-m', '--board', board,
               *migrate, '-w', '-o', tmpdir], ignore_errors=True, env=env)
    return out

@pytest.mark.slow
@pytest.mark.boardspec('sandbox')
def test_of_no_migrate(u_boot_console):
    """Test sandbox with old boot phase tags like u-boot,dm-pre-proper"""
    cons = u_boot_console

    build_for_migrate(cons, ['bootph-some-ram', 'u-boot,dm-pre-proper'],
                      'sandbox', TMPDIR1)

    # It should fail to run, since the lcd device will not be bound before
    # relocation. so won't get its frame-buffer memory
    out = util.run_and_log(
        cons, [os.path.join(TMPDIR1, 'u-boot'), '-D', '-c', 'help'],
        ignore_errors=True)
    assert "Video device 'lcd' cannot allocate frame buffer memory" in out


@pytest.mark.slow
@pytest.mark.boardspec('sandbox_spl')
@pytest.mark.boardspec('spl_of_platdata_inst')
@pytest.mark.boardspec('!sandbox_tpl')
def test_of_no_migrate_spl(u_boot_console):
    """Test sandbox with old boot phase tags like u-boot,dm-spl"""
    cons = u_boot_console

    out = build_for_migrate(cons, ['bootph-pre-ram', 'u-boot,dm-spl'],
                            'sandbox_spl', TMPDIR2)

    # It should fail to build, since the SPL DT will not include 'spl-test'
    # node, among others
    assert "undefined type ‘struct dtd_sandbox_spl_test’" in out


@pytest.mark.slow
@pytest.mark.boardspec('sandbox')
def test_of_migrate(u_boot_console):
    """Test sandbox shows a message when tags were migrated"""
    cons = u_boot_console

    build_for_migrate(cons, ['bootph-some-ram', 'u-boot,dm-pre-proper'],
                      'sandbox', TMPDIR3, disable_migrate=False)

    # It should show a migration message
    out = util.run_and_log(
        cons, [os.path.join(TMPDIR3, 'u-boot'), '-D', '-c', 'help'],
        ignore_errors=True)
    assert "Warning: Device tree includes old 'u-boot,dm-' tags" in out
