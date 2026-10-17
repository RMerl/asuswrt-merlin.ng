# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2015-2016, NVIDIA CORPORATION. All rights reserved.

# Test basic shell functionality, such as commands separate by semi-colons.

import pytest

pytestmark = pytest.mark.buildconfigspec('cmd_echo')

def test_shell_execute(u_boot_console):
    """Test any shell command."""

    response = u_boot_console.run_command('echo hello')
    assert response.strip() == 'hello'

def test_shell_semicolon_two(u_boot_console):
    """Test two shell commands separate by a semi-colon."""

    cmd = 'echo hello; echo world'
    response = u_boot_console.run_command(cmd)
    # This validation method ignores the exact whitespace between the strings
    assert response.index('hello') < response.index('world')

def test_shell_semicolon_three(u_boot_console):
    """Test three shell commands separate by a semi-colon, with variable
    expansion dependencies between them."""

    cmd = 'setenv list 1; setenv list ${list}2; setenv list ${list}3; ' + \
        'echo ${list}'
    response = u_boot_console.run_command(cmd)
    assert response.strip() == '123'
    u_boot_console.run_command('setenv list')

def test_shell_run(u_boot_console):
    """Test the "run" shell command."""

    u_boot_console.run_command('setenv foo \'setenv monty 1; setenv python 2\'')
    u_boot_console.run_command('run foo')
    response = u_boot_console.run_command('echo ${monty}')
    assert response.strip() == '1'
    response = u_boot_console.run_command('echo ${python}')
    assert response.strip() == '2'
    u_boot_console.run_command('setenv foo')
    u_boot_console.run_command('setenv monty')
    u_boot_console.run_command('setenv python')
