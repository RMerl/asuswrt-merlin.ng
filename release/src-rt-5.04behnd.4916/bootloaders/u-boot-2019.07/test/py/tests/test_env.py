# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2015 Stephen Warren
# Copyright (c) 2015-2016, NVIDIA CORPORATION. All rights reserved.

# Test operation of shell commands relating to environment variables.

import pytest
import u_boot_utils

# FIXME: This might be useful for other tests;
# perhaps refactor it into ConsoleBase or some other state object?
class StateTestEnv(object):
    """Container that represents the state of all U-Boot environment variables.
    This enables quick determination of existant/non-existant variable
    names.
    """

    def __init__(self, u_boot_console):
        """Initialize a new StateTestEnv object.

        Args:
            u_boot_console: A U-Boot console.

        Returns:
            Nothing.
        """

        self.u_boot_console = u_boot_console
        self.get_env()
        self.set_var = self.get_non_existent_var()

    def get_env(self):
        """Read all current environment variables from U-Boot.

        Args:
            None.

        Returns:
            Nothing.
        """

        if self.u_boot_console.config.buildconfig.get(
                'config_version_variable', 'n') == 'y':
            with self.u_boot_console.disable_check('main_signon'):
                response = self.u_boot_console.run_command('printenv')
        else:
            response = self.u_boot_console.run_command('printenv')
        self.env = {}
        for l in response.splitlines():
            if not '=' in l:
                continue
            (var, value) = l.strip().split('=', 1)
            self.env[var] = value

    def get_existent_var(self):
        """Return the name of an environment variable that exists.

        Args:
            None.

        Returns:
            The name of an environment variable.
        """

        for var in self.env:
            return var

    def get_non_existent_var(self):
        """Return the name of an environment variable that does not exist.

        Args:
            None.

        Returns:
            The name of an environment variable.
        """

        n = 0
        while True:
            var = 'test_env_' + str(n)
            if var not in self.env:
                return var
            n += 1

ste = None
@pytest.fixture(scope='function')
def state_test_env(u_boot_console):
    """pytest fixture to provide a StateTestEnv object to tests."""

    global ste
    if not ste:
        ste = StateTestEnv(u_boot_console)
    return ste

def unset_var(state_test_env, var):
    """Unset an environment variable.

    This both executes a U-Boot shell command and updates a StateTestEnv
    object.

    Args:
        state_test_env: The StateTestEnv object to update.
        var: The variable name to unset.

    Returns:
        Nothing.
    """

    state_test_env.u_boot_console.run_command('setenv %s' % var)
    if var in state_test_env.env:
        del state_test_env.env[var]

def set_var(state_test_env, var, value):
    """Set an environment variable.

    This both executes a U-Boot shell command and updates a StateTestEnv
    object.

    Args:
        state_test_env: The StateTestEnv object to update.
        var: The variable name to set.
        value: The value to set the variable to.

    Returns:
        Nothing.
    """

    bc = state_test_env.u_boot_console.config.buildconfig
    if bc.get('config_hush_parser', None):
        quote = '"'
    else:
        quote = ''
        if ' ' in value:
            pytest.skip('Space in variable value on non-Hush shell')

    state_test_env.u_boot_console.run_command(
        'setenv %s %s%s%s' % (var, quote, value, quote))
    state_test_env.env[var] = value

def validate_empty(state_test_env, var):
    """Validate that a variable is not set, using U-Boot shell commands.

    Args:
        var: The variable name to test.

    Returns:
        Nothing.
    """

    response = state_test_env.u_boot_console.run_command('echo $%s' % var)
    assert response == ''

def validate_set(state_test_env, var, value):
    """Validate that a variable is set, using U-Boot shell commands.

    Args:
        var: The variable name to test.
        value: The value the variable is expected to have.

    Returns:
        Nothing.
    """

    # echo does not preserve leading, internal, or trailing whitespace in the
    # value. printenv does, and hence allows more complete testing.
    response = state_test_env.u_boot_console.run_command('printenv %s' % var)
    assert response == ('%s=%s' % (var, value))

def test_env_echo_exists(state_test_env):
    """Test echoing a variable that exists."""

    var = state_test_env.get_existent_var()
    value = state_test_env.env[var]
    validate_set(state_test_env, var, value)

@pytest.mark.buildconfigspec('cmd_echo')
def test_env_echo_non_existent(state_test_env):
    """Test echoing a variable that doesn't exist."""

    var = state_test_env.set_var
    validate_empty(state_test_env, var)

def test_env_printenv_non_existent(state_test_env):
    """Test printenv error message for non-existant variables."""

    var = state_test_env.set_var
    c = state_test_env.u_boot_console
    with c.disable_check('error_notification'):
        response = c.run_command('printenv %s' % var)
    assert(response == '## Error: "%s" not defined' % var)

@pytest.mark.buildconfigspec('cmd_echo')
def test_env_unset_non_existent(state_test_env):
    """Test unsetting a nonexistent variable."""

    var = state_test_env.get_non_existent_var()
    unset_var(state_test_env, var)
    validate_empty(state_test_env, var)

def test_env_set_non_existent(state_test_env):
    """Test set a non-existant variable."""

    var = state_test_env.set_var
    value = 'foo'
    set_var(state_test_env, var, value)
    validate_set(state_test_env, var, value)

def test_env_set_existing(state_test_env):
    """Test setting an existant variable."""

    var = state_test_env.set_var
    value = 'bar'
    set_var(state_test_env, var, value)
    validate_set(state_test_env, var, value)

@pytest.mark.buildconfigspec('cmd_echo')
def test_env_unset_existing(state_test_env):
    """Test unsetting a variable."""

    var = state_test_env.set_var
    unset_var(state_test_env, var)
    validate_empty(state_test_env, var)

def test_env_expansion_spaces(state_test_env):
    """Test expanding a variable that contains a space in its value."""

    var_space = None
    var_test = None
    try:
        var_space = state_test_env.get_non_existent_var()
        set_var(state_test_env, var_space, ' ')

        var_test = state_test_env.get_non_existent_var()
        value = ' 1${%(var_space)s}${%(var_space)s} 2 ' % locals()
        set_var(state_test_env, var_test, value)
        value = ' 1   2 '
        validate_set(state_test_env, var_test, value)
    finally:
        if var_space:
            unset_var(state_test_env, var_space)
        if var_test:
            unset_var(state_test_env, var_test)

@pytest.mark.buildconfigspec('cmd_importenv')
def test_env_import_checksum_no_size(state_test_env):
    """Test that omitted ('-') size parameter with checksum validation fails the
       env import function.
    """
    c = state_test_env.u_boot_console
    ram_base = u_boot_utils.find_ram_base(state_test_env.u_boot_console)
    addr = '%08x' % ram_base

    with c.disable_check('error_notification'):
        response = c.run_command('env import -c %s -' % addr)
    assert(response == '## Error: external checksum format must pass size')

@pytest.mark.buildconfigspec('cmd_importenv')
def test_env_import_whitelist_checksum_no_size(state_test_env):
    """Test that omitted ('-') size parameter with checksum validation fails the
       env import function when variables are passed as parameters.
    """
    c = state_test_env.u_boot_console
    ram_base = u_boot_utils.find_ram_base(state_test_env.u_boot_console)
    addr = '%08x' % ram_base

    with c.disable_check('error_notification'):
        response = c.run_command('env import -c %s - foo1 foo2 foo4' % addr)
    assert(response == '## Error: external checksum format must pass size')

@pytest.mark.buildconfigspec('cmd_exportenv')
@pytest.mark.buildconfigspec('cmd_importenv')
def test_env_import_whitelist(state_test_env):
    """Test importing only a handful of env variables from an environment."""
    c = state_test_env.u_boot_console
    ram_base = u_boot_utils.find_ram_base(state_test_env.u_boot_console)
    addr = '%08x' % ram_base

    set_var(state_test_env, 'foo1', 'bar1')
    set_var(state_test_env, 'foo2', 'bar2')
    set_var(state_test_env, 'foo3', 'bar3')

    c.run_command('env export %s' % addr)

    unset_var(state_test_env, 'foo1')
    set_var(state_test_env, 'foo2', 'test2')
    set_var(state_test_env, 'foo4', 'bar4')

    # no foo1 in current env, foo2 overridden, foo3 should be of the value
    # before exporting and foo4 should be of the value before importing.
    c.run_command('env import %s - foo1 foo2 foo4' % addr)

    validate_set(state_test_env, 'foo1', 'bar1')
    validate_set(state_test_env, 'foo2', 'bar2')
    validate_set(state_test_env, 'foo3', 'bar3')
    validate_set(state_test_env, 'foo4', 'bar4')

    # Cleanup test environment
    unset_var(state_test_env, 'foo1')
    unset_var(state_test_env, 'foo2')
    unset_var(state_test_env, 'foo3')
    unset_var(state_test_env, 'foo4')

@pytest.mark.buildconfigspec('cmd_exportenv')
@pytest.mark.buildconfigspec('cmd_importenv')
def test_env_import_whitelist_delete(state_test_env):

    """Test importing only a handful of env variables from an environment, with.
       deletion if a var A that is passed to env import is not in the
       environment to be imported.
    """
    c = state_test_env.u_boot_console
    ram_base = u_boot_utils.find_ram_base(state_test_env.u_boot_console)
    addr = '%08x' % ram_base

    set_var(state_test_env, 'foo1', 'bar1')
    set_var(state_test_env, 'foo2', 'bar2')
    set_var(state_test_env, 'foo3', 'bar3')

    c.run_command('env export %s' % addr)

    unset_var(state_test_env, 'foo1')
    set_var(state_test_env, 'foo2', 'test2')
    set_var(state_test_env, 'foo4', 'bar4')

    # no foo1 in current env, foo2 overridden, foo3 should be of the value
    # before exporting and foo4 should be empty.
    c.run_command('env import -d %s - foo1 foo2 foo4' % addr)

    validate_set(state_test_env, 'foo1', 'bar1')
    validate_set(state_test_env, 'foo2', 'bar2')
    validate_set(state_test_env, 'foo3', 'bar3')
    validate_empty(state_test_env, 'foo4')

    # Cleanup test environment
    unset_var(state_test_env, 'foo1')
    unset_var(state_test_env, 'foo2')
    unset_var(state_test_env, 'foo3')
    unset_var(state_test_env, 'foo4')
