# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>

import os
import pytest
import u_boot_utils as util

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_echo')
@pytest.mark.buildconfigspec('cmd_source')
@pytest.mark.buildconfigspec('fit')
def test_source(u_boot_console):
    # Compile our test script image
    cons = u_boot_console
    mkimage = os.path.join(cons.config.build_dir, 'tools/mkimage')
    its = os.path.join(cons.config.source_dir, 'test/py/tests/source.its')
    fit = os.path.join(cons.config.build_dir, 'source.itb')
    util.run_and_log(cons, (mkimage, '-f', its, fit))
    cons.run_command(f'host load hostfs - $loadaddr {fit}')

    assert '2' in cons.run_command('source')
    assert '1' in cons.run_command('source :')
    assert '1' in cons.run_command('source :script-1')
    assert '2' in cons.run_command('source :script-2')
    assert 'Fail' in cons.run_command('source :not-a-script || echo Fail')
    assert '2' in cons.run_command('source \\#')
    assert '1' in cons.run_command('source \\#conf-1')
    assert '2' in cons.run_command('source \\#conf-2')

    cons.run_command('fdt addr $loadaddr')
    cons.run_command('fdt rm /configurations default')
    assert '1' in cons.run_command('source')
    assert 'Fail' in cons.run_command('source \\# || echo Fail')

    cons.run_command('fdt rm /images default')
    assert 'Fail' in cons.run_command('source || echo Fail')
    assert 'Fail' in cons.run_command('source \\# || echo Fail')
