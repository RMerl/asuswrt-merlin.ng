# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.

# Utility code shared across multiple tests.

import hashlib
import inspect
import os
import os.path
import pytest
import sys
import time
import re

def md5sum_data(data):
    """Calculate the MD5 hash of some data.

    Args:
        data: The data to hash.

    Returns:
        The hash of the data, as a binary string.
    """

    h = hashlib.md5()
    h.update(data)
    return h.digest()

def md5sum_file(fn, max_length=None):
    """Calculate the MD5 hash of the contents of a file.

    Args:
        fn: The filename of the file to hash.
        max_length: The number of bytes to hash. If the file has more
            bytes than this, they will be ignored. If None or omitted, the
            entire file will be hashed.

    Returns:
        The hash of the file content, as a binary string.
    """

    with open(fn, 'rb') as fh:
        if max_length:
            params = [max_length]
        else:
            params = []
        data = fh.read(*params)
    return md5sum_data(data)

class PersistentRandomFile(object):
    """Generate and store information about a persistent file containing
    random data."""

    def __init__(self, u_boot_console, fn, size):
        """Create or process the persistent file.

        If the file does not exist, it is generated.

        If the file does exist, its content is hashed for later comparison.

        These files are always located in the "persistent data directory" of
        the current test run.

        Args:
            u_boot_console: A console connection to U-Boot.
            fn: The filename (without path) to create.
            size: The desired size of the file in bytes.

        Returns:
            Nothing.
        """

        self.fn = fn

        self.abs_fn = u_boot_console.config.persistent_data_dir + '/' + fn

        if os.path.exists(self.abs_fn):
            u_boot_console.log.action('Persistent data file ' + self.abs_fn +
                ' already exists')
            self.content_hash = md5sum_file(self.abs_fn)
        else:
            u_boot_console.log.action('Generating ' + self.abs_fn +
                ' (random, persistent, %d bytes)' % size)
            data = os.urandom(size)
            with open(self.abs_fn, 'wb') as fh:
                fh.write(data)
            self.content_hash = md5sum_data(data)

def attempt_to_open_file(fn):
    """Attempt to open a file, without throwing exceptions.

    Any errors (exceptions) that occur during the attempt to open the file
    are ignored. This is useful in order to test whether a file (in
    particular, a device node) exists and can be successfully opened, in order
    to poll for e.g. USB enumeration completion.

    Args:
        fn: The filename to attempt to open.

    Returns:
        An open file handle to the file, or None if the file could not be
            opened.
    """

    try:
        return open(fn, 'rb')
    except:
        return None

def wait_until_open_succeeds(fn):
    """Poll until a file can be opened, or a timeout occurs.

    Continually attempt to open a file, and return when this succeeds, or
    raise an exception after a timeout.

    Args:
        fn: The filename to attempt to open.

    Returns:
        An open file handle to the file.
    """

    for i in range(100):
        fh = attempt_to_open_file(fn)
        if fh:
            return fh
        time.sleep(0.1)
    raise Exception('File could not be opened')

def wait_until_file_open_fails(fn, ignore_errors):
    """Poll until a file cannot be opened, or a timeout occurs.

    Continually attempt to open a file, and return when this fails, or
    raise an exception after a timeout.

    Args:
        fn: The filename to attempt to open.
        ignore_errors: Indicate whether to ignore timeout errors. If True, the
            function will simply return if a timeout occurs, otherwise an
            exception will be raised.

    Returns:
        Nothing.
    """

    for i in range(100):
        fh = attempt_to_open_file(fn)
        if not fh:
            return
        fh.close()
        time.sleep(0.1)
    if ignore_errors:
        return
    raise Exception('File can still be opened')

def run_and_log(u_boot_console, cmd, ignore_errors=False):
    """Run a command and log its output.

    Args:
        u_boot_console: A console connection to U-Boot.
        cmd: The command to run, as an array of argv[], or a string.
            If a string, note that it is split up so that quoted spaces
            will not be preserved. E.g. "fred and" becomes ['"fred', 'and"']
        ignore_errors: Indicate whether to ignore errors. If True, the function
            will simply return if the command cannot be executed or exits with
            an error code, otherwise an exception will be raised if such
            problems occur.

    Returns:
        The output as a string.
    """
    if isinstance(cmd, str):
        cmd = cmd.split()
    runner = u_boot_console.log.get_runner(cmd[0], sys.stdout)
    output = runner.run(cmd, ignore_errors=ignore_errors)
    runner.close()
    return output

def run_and_log_expect_exception(u_boot_console, cmd, retcode, msg):
    """Run a command that is expected to fail.

    This runs a command and checks that it fails with the expected return code
    and exception method. If not, an exception is raised.

    Args:
        u_boot_console: A console connection to U-Boot.
        cmd: The command to run, as an array of argv[].
        retcode: Expected non-zero return code from the command.
        msg: String that should be contained within the command's output.
    """
    try:
        runner = u_boot_console.log.get_runner(cmd[0], sys.stdout)
        runner.run(cmd)
    except Exception as e:
        assert(retcode == runner.exit_status)
        assert(msg in runner.output)
    else:
        raise Exception("Expected an exception with retcode %d message '%s',"
                        "but it was not raised" % (retcode, msg))
    finally:
        runner.close()

ram_base = None
def find_ram_base(u_boot_console):
    """Find the running U-Boot's RAM location.

    Probe the running U-Boot to determine the address of the first bank
    of RAM. This is useful for tests that test reading/writing RAM, or
    load/save files that aren't associated with some standard address
    typically represented in an environment variable such as
    ${kernel_addr_r}. The value is cached so that it only needs to be
    actively read once.

    Args:
        u_boot_console: A console connection to U-Boot.

    Returns:
        The address of U-Boot's first RAM bank, as an integer.
    """

    global ram_base
    if u_boot_console.config.buildconfig.get('config_cmd_bdi', 'n') != 'y':
        pytest.skip('bdinfo command not supported')
    if ram_base == -1:
        pytest.skip('Previously failed to find RAM bank start')
    if ram_base is not None:
        return ram_base

    with u_boot_console.log.section('find_ram_base'):
        response = u_boot_console.run_command('bdinfo')
        for l in response.split('\n'):
            if '-> start' in l or 'memstart    =' in l:
                ram_base = int(l.split('=')[1].strip(), 16)
                break
        if ram_base is None:
            ram_base = -1
            raise Exception('Failed to find RAM bank start in `bdinfo`')

    # We don't want ram_base to be zero as some functions test if the given
    # address is NULL (0). Let's add 2MiB then (size of an ARM LPAE/v8 section).

    if ram_base == 0:
        ram_base += 1024 * 1024 * 2

    return ram_base

class PersistentFileHelperCtxMgr(object):
    """A context manager for Python's "with" statement, which ensures that any
    generated file is deleted (and hence regenerated) if its mtime is older
    than the mtime of the Python module which generated it, and gets an mtime
    newer than the mtime of the Python module which generated after it is
    generated. Objects of this type should be created by factory function
    persistent_file_helper rather than directly."""

    def __init__(self, log, filename):
        """Initialize a new object.

        Args:
            log: The Logfile object to log to.
            filename: The filename of the generated file.

        Returns:
            Nothing.
        """

        self.log = log
        self.filename = filename

    def __enter__(self):
        frame = inspect.stack()[1]
        module = inspect.getmodule(frame[0])
        self.module_filename = module.__file__
        self.module_timestamp = os.path.getmtime(self.module_filename)

        if os.path.exists(self.filename):
            filename_timestamp = os.path.getmtime(self.filename)
            if filename_timestamp < self.module_timestamp:
                self.log.action('Removing stale generated file ' +
                    self.filename)
                os.unlink(self.filename)

    def __exit__(self, extype, value, traceback):
        if extype:
            try:
                os.path.unlink(self.filename)
            except:
                pass
            return
        logged = False
        for i in range(20):
            filename_timestamp = os.path.getmtime(self.filename)
            if filename_timestamp > self.module_timestamp:
                break
            if not logged:
                self.log.action(
                    'Waiting for generated file timestamp to increase')
                logged = True
            os.utime(self.filename)
            time.sleep(0.1)

def persistent_file_helper(u_boot_log, filename):
    """Manage the timestamps and regeneration of a persistent generated
    file. This function creates a context manager for Python's "with"
    statement

    Usage:
        with persistent_file_helper(u_boot_console.log, filename):
            code to generate the file, if it's missing.

    Args:
        u_boot_log: u_boot_console.log.
        filename: The filename of the generated file.

    Returns:
        A context manager object.
    """

    return PersistentFileHelperCtxMgr(u_boot_log, filename)

def crc32(u_boot_console, address, count):
    """Helper function used to compute the CRC32 value of a section of RAM.

    Args:
        u_boot_console: A U-Boot console connection.
        address: Address where data starts.
        count: Amount of data to use for calculation.

    Returns:
        CRC32 value
    """

    bcfg = u_boot_console.config.buildconfig
    has_cmd_crc32 = bcfg.get('config_cmd_crc32', 'n') == 'y'
    assert has_cmd_crc32, 'Cannot compute crc32 without CONFIG_CMD_CRC32.'
    output = u_boot_console.run_command('crc32 %08x %x' % (address, count))

    m = re.search('==> ([0-9a-fA-F]{8})$', output)
    assert m, 'CRC32 operation failed.'

    return m.group(1)
