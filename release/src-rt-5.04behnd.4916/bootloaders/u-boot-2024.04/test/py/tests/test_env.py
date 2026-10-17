# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2015 Stephen Warren
# Copyright (c) 2015-2016, NVIDIA CORPORATION. All rights reserved.

"""
Test operation of shell commands relating to environment variables.
"""

import os
import os.path
import re
from subprocess import call, CalledProcessError
import tempfile

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
            (var, value) = l.split('=', 1)
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

    response = state_test_env.u_boot_console.run_command('echo ${%s}' % var)
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

@pytest.mark.boardspec('sandbox')
def test_env_initial_env_file(u_boot_console):
    """Test that the u-boot-initial-env make target works"""
    cons = u_boot_console
    builddir = 'O=' + cons.config.build_dir
    envfile = cons.config.build_dir + '/u-boot-initial-env'

    # remove if already exists from an older run
    try:
        os.remove(envfile)
    except:
        pass

    u_boot_utils.run_and_log(cons, ['make', builddir, 'u-boot-initial-env'])

    assert os.path.exists(envfile)

    # assume that every environment has a board variable, e.g. board=sandbox
    with open(envfile, 'r') as file:
        env = file.read()
    regex = re.compile('board=.+\\n')
    assert re.search(regex, env)

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
    assert response == '## Error: "%s" not defined' % var

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
    assert response == '## Error: external checksum format must pass size'

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
    assert response == '## Error: external checksum format must pass size'

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

@pytest.mark.buildconfigspec('cmd_nvedit_info')
def test_env_info(state_test_env):

    """Test 'env info' command with all possible options.
    """
    c = state_test_env.u_boot_console

    response = c.run_command('env info')
    nb_line = 0
    for l in response.split('\n'):
        if 'env_valid = ' in l:
            assert '= invalid' in l or '= valid' in l or '= redundant' in l
            nb_line += 1
        elif 'env_ready =' in l or 'env_use_default =' in l:
            assert '= true' in l or '= false' in l
            nb_line += 1
        else:
            assert True
    assert nb_line == 3

    response = c.run_command('env info -p -d')
    assert 'Default environment is used' in response or \
           "Environment was loaded from persistent storage" in response
    assert 'Environment can be persisted' in response or \
           "Environment cannot be persisted" in response

    response = c.run_command('env info -p -d -q')
    assert response == ""

    response = c.run_command('env info -p -q')
    assert response == ""

    response = c.run_command('env info -d -q')
    assert response == ""

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_nvedit_info')
@pytest.mark.buildconfigspec('cmd_echo')
def test_env_info_sandbox(state_test_env):
    """Test 'env info' command result with several options on sandbox
       with a known ENV configuration: ready & default & persistent
    """
    c = state_test_env.u_boot_console

    response = c.run_command('env info')
    assert 'env_ready = true' in response
    assert 'env_use_default = true' in response

    response = c.run_command('env info -p -d')
    assert 'Default environment is used' in response
    assert 'Environment cannot be persisted' in response

    response = c.run_command('env info -d -q')
    response = c.run_command('echo $?')
    assert response == "0"

    response = c.run_command('env info -p -q')
    response = c.run_command('echo $?')
    assert response == "1"

    response = c.run_command('env info -d -p -q')
    response = c.run_command('echo $?')
    assert response == "1"

def mk_env_ext4(state_test_env):

    """Create a empty ext4 file system volume."""
    c = state_test_env.u_boot_console
    filename = 'env.ext4.img'
    persistent = c.config.persistent_data_dir + '/' + filename
    fs_img = c.config.result_dir  + '/' + filename

    if os.path.exists(persistent):
        c.log.action('Disk image file ' + persistent + ' already exists')
    else:
        # Some distributions do not add /sbin to the default PATH, where mkfs.ext4 lives
        os.environ["PATH"] += os.pathsep + '/sbin'
        try:
            u_boot_utils.run_and_log(c, 'dd if=/dev/zero of=%s bs=1M count=16' % persistent)
            u_boot_utils.run_and_log(c, 'mkfs.ext4 %s' % persistent)
            sb_content = u_boot_utils.run_and_log(c, 'tune2fs -l %s' % persistent)
            if 'metadata_csum' in sb_content:
                u_boot_utils.run_and_log(c, 'tune2fs -O ^metadata_csum %s' % persistent)
        except CalledProcessError:
            call('rm -f %s' % persistent, shell=True)
            raise

    u_boot_utils.run_and_log(c, ['cp',  '-f', persistent, fs_img])
    return fs_img

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_echo')
@pytest.mark.buildconfigspec('cmd_nvedit_info')
@pytest.mark.buildconfigspec('cmd_nvedit_load')
@pytest.mark.buildconfigspec('cmd_nvedit_select')
@pytest.mark.buildconfigspec('env_is_in_ext4')
def test_env_ext4(state_test_env):

    """Test ENV in EXT4 on sandbox."""
    c = state_test_env.u_boot_console
    fs_img = ''
    try:
        fs_img = mk_env_ext4(state_test_env)

        c.run_command('host bind 0  %s' % fs_img)

        response = c.run_command('ext4ls host 0:0')
        assert 'uboot.env' not in response

        # force env location: EXT4 (prio 1 in sandbox)
        response = c.run_command('env select EXT4')
        assert 'Select Environment on EXT4: OK' in response

        response = c.run_command('env save')
        assert 'Saving Environment to EXT4' in response

        response = c.run_command('env load')
        assert 'Loading Environment from EXT4... OK' in response

        response = c.run_command('ext4ls host 0:0')
        assert '8192 uboot.env' in response

        response = c.run_command('env info')
        assert 'env_valid = valid' in response
        assert 'env_ready = true' in response
        assert 'env_use_default = false' in response

        response = c.run_command('env info -p -d')
        assert 'Environment was loaded from persistent storage' in response
        assert 'Environment can be persisted' in response

        response = c.run_command('env info -d -q')
        assert response == ""
        response = c.run_command('echo $?')
        assert response == "1"

        response = c.run_command('env info -p -q')
        assert response == ""
        response = c.run_command('echo $?')
        assert response == "0"

        response = c.run_command('env erase')
        assert 'OK' in response

        response = c.run_command('env load')
        assert 'Loading Environment from EXT4... ' in response
        assert 'bad CRC, using default environment' in response

        response = c.run_command('env info')
        assert 'env_valid = invalid' in response
        assert 'env_ready = true' in response
        assert 'env_use_default = true' in response

        response = c.run_command('env info -p -d')
        assert 'Default environment is used' in response
        assert 'Environment can be persisted' in response

        # restore env location: NOWHERE (prio 0 in sandbox)
        response = c.run_command('env select nowhere')
        assert 'Select Environment on nowhere: OK' in response

        response = c.run_command('env load')
        assert 'Loading Environment from nowhere... OK' in response

        response = c.run_command('env info')
        assert 'env_valid = invalid' in response
        assert 'env_ready = true' in response
        assert 'env_use_default = true' in response

        response = c.run_command('env info -p -d')
        assert 'Default environment is used' in response
        assert 'Environment cannot be persisted' in response

    finally:
        if fs_img:
            call('rm -f %s' % fs_img, shell=True)

def test_env_text(u_boot_console):
    """Test the script that converts the environment to a text file"""

    def check_script(intext, expect_val):
        """Check a test case

        Args:
            intext: Text to pass to the script
            expect_val: Expected value of the CONFIG_EXTRA_ENV_TEXT string, or
                None if we expect it not to be defined
        """
        with tempfile.TemporaryDirectory() as path:
            fname = os.path.join(path, 'infile')
            with open(fname, 'w') as inf:
                print(intext, file=inf)
            result = u_boot_utils.run_and_log(cons, ['awk', '-f', script, fname])
            if expect_val is not None:
                expect = '#define CONFIG_EXTRA_ENV_TEXT "%s"\n' % expect_val
                assert result == expect
            else:
                assert result == ''

    cons = u_boot_console
    script = os.path.join(cons.config.source_dir, 'scripts', 'env2string.awk')

    # simple script with a single var
    check_script('fred=123', 'fred=123\\0')

    # no vars
    check_script('', None)

    # two vars
    check_script('''fred=123
mary=456''', 'fred=123\\0mary=456\\0')

    # blank lines
    check_script('''fred=123


mary=456

''', 'fred=123\\0mary=456\\0')

    # append
    check_script('''fred=123
mary=456
fred+= 456''', 'fred=123 456\\0mary=456\\0')

    # append from empty
    check_script('''fred=
mary=456
fred+= 456''', 'fred= 456\\0mary=456\\0')

    # variable with + in it
    check_script('fred+mary=123', 'fred+mary=123\\0')

    # ignores variables that are empty
    check_script('''fred=
fred+=
mary=456''', 'mary=456\\0')

    # single-character env name
    check_script('''m=123
e=456
m+= 456''', 'e=456\\0m=123 456\\0')

    # contains quotes
    check_script('''fred="my var"
mary=another"''', 'fred=\\"my var\\"\\0mary=another\\"\\0')

    # variable name ending in +
    check_script('''fred\\+=my var
fred++= again''', 'fred+=my var again\\0')

    # variable name containing +
    check_script('''fred+jane=both
fred+jane+=again
mary=456''', 'fred+jane=bothagain\\0mary=456\\0')

    # multi-line vars - new vars always start at column 1
    check_script('''fred=first
 second
\tthird with tab

   after blank
 confusing=oops
mary=another"''', 'fred=first second third with tab after blank confusing=oops\\0mary=another\\"\\0')

    # real-world example
    check_script('''ubifs_boot=
	env exists bootubipart ||
		env set bootubipart UBI;
	env exists bootubivol ||
		env set bootubivol boot;
	if ubi part ${bootubipart} &&
		ubifsmount ubi${devnum}:${bootubivol};
	then
		devtype=ubi;
		run scan_dev_for_boot;
	fi
''',
        'ubifs_boot=env exists bootubipart || env set bootubipart UBI; '
        'env exists bootubivol || env set bootubivol boot; '
        'if ubi part ${bootubipart} && ubifsmount ubi${devnum}:${bootubivol}; '
        'then devtype=ubi; run scan_dev_for_boot; fi\\0')
