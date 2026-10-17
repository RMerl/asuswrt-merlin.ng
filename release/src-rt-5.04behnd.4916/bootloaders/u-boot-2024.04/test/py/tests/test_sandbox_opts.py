# SPDX-License-Identifier: GPL-2.0
# Copyright 2022 Google LLC
# Written by Simon Glass <sjg@chromium.org>

import pytest

import u_boot_utils as util

# This is needed for Azure, since the default '..' directory is not writeable
TMPDIR = '/tmp/test_cmdline'

@pytest.mark.slow
@pytest.mark.boardspec('sandbox')
def test_sandbox_cmdline(u_boot_console):
    """Test building sandbox without CONFIG_CMDLINE"""
    cons = u_boot_console

    out = util.run_and_log(
        cons, ['./tools/buildman/buildman', '-m', '--board', 'sandbox',
               '-a', '~CMDLINE', '-o', TMPDIR])

@pytest.mark.slow
@pytest.mark.boardspec('sandbox')
def test_sandbox_lto(u_boot_console):
    """Test building sandbox without CONFIG_LTO"""
    cons = u_boot_console

    out = util.run_and_log(
        cons, ['./tools/buildman/buildman', '-m', '--board', 'sandbox',
               '-a', '~LTO', '-o', TMPDIR])
