# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc

import pytest

# Magic number to check that SPL handoff is working
TEST_HANDOFF_MAGIC = 0x14f93c7b

@pytest.mark.boardspec('sandbox_spl')
@pytest.mark.buildconfigspec('spl')
def test_handoff(u_boot_console):
    """Test that of-platdata can be generated and used in sandbox"""
    cons = u_boot_console
    response = cons.run_command('sb handoff')
    assert ('SPL handoff magic %x' % TEST_HANDOFF_MAGIC) in response
