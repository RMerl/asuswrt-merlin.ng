# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2015 Stephen Warren
# Copyright (c) 2015-2016, NVIDIA CORPORATION. All rights reserved.

# Implementation of pytest run-time hook functions. These are invoked by
# pytest at certain points during operation, e.g. startup, for each executed
# test, at shutdown etc. These hooks perform functions such as:
# - Parsing custom command-line options.
# - Pullilng in user-specified board configuration.
# - Creating the U-Boot console test fixture.
# - Creating the HTML log file.
# - Monitoring each test's results.
# - Implementing custom pytest markers.

import atexit
import configparser
import errno
import filelock
import io
import os
import os.path
from pathlib import Path
import pytest
import re
from _pytest.runner import runtestprotocol
import sys

# Globals: The HTML log file, and the connection to the U-Boot console.
log = None
console = None

TEST_PY_DIR = os.path.dirname(os.path.abspath(__file__))

def mkdir_p(path):
    """Create a directory path.

    This includes creating any intermediate/parent directories. Any errors
    caused due to already extant directories are ignored.

    Args:
        path: The directory path to create.

    Returns:
        Nothing.
    """

    try:
        os.makedirs(path)
    except OSError as exc:
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise

def pytest_addoption(parser):
    """pytest hook: Add custom command-line options to the cmdline parser.

    Args:
        parser: The pytest command-line parser.

    Returns:
        Nothing.
    """

    parser.addoption('--build-dir', default=None,
        help='U-Boot build directory (O=)')
    parser.addoption('--result-dir', default=None,
        help='U-Boot test result/tmp directory')
    parser.addoption('--persistent-data-dir', default=None,
        help='U-Boot test persistent generated data directory')
    parser.addoption('--board-type', '--bd', '-B', default='sandbox',
        help='U-Boot board type')
    parser.addoption('--board-identity', '--id', default='na',
        help='U-Boot board identity/instance')
    parser.addoption('--build', default=False, action='store_true',
        help='Compile U-Boot before running tests')
    parser.addoption('--buildman', default=False, action='store_true',
        help='Use buildman to build U-Boot (assuming --build is given)')
    parser.addoption('--gdbserver', default=None,
        help='Run sandbox under gdbserver. The argument is the channel '+
        'over which gdbserver should communicate, e.g. localhost:1234')

def run_build(config, source_dir, build_dir, board_type, log):
    """run_build: Build U-Boot

    Args:
        config: The pytest configuration.
        soruce_dir (str): Directory containing source code
        build_dir (str): Directory to build in
        board_type (str): board_type parameter (e.g. 'sandbox')
        log (Logfile): Log file to use
    """
    if config.getoption('buildman'):
        if build_dir != source_dir:
            dest_args = ['-o', build_dir, '-w']
        else:
            dest_args = ['-i']
        cmds = (['buildman', '--board', board_type] + dest_args,)
        name = 'buildman'
    else:
        if build_dir != source_dir:
            o_opt = 'O=%s' % build_dir
        else:
            o_opt = ''
        cmds = (
            ['make', o_opt, '-s', board_type + '_defconfig'],
            ['make', o_opt, '-s', '-j{}'.format(os.cpu_count())],
        )
        name = 'make'

    with log.section(name):
        runner = log.get_runner(name, sys.stdout)
        for cmd in cmds:
            runner.run(cmd, cwd=source_dir)
        runner.close()
        log.status_pass('OK')

def pytest_xdist_setupnodes(config, specs):
    """Clear out any 'done' file from a previous build"""
    global build_done_file
    build_dir = config.getoption('build_dir')
    board_type = config.getoption('board_type')
    source_dir = os.path.dirname(os.path.dirname(TEST_PY_DIR))
    if not build_dir:
        build_dir = source_dir + '/build-' + board_type
    build_done_file = Path(build_dir) / 'build.done'
    if build_done_file.exists():
        os.remove(build_done_file)

def pytest_configure(config):
    """pytest hook: Perform custom initialization at startup time.

    Args:
        config: The pytest configuration.

    Returns:
        Nothing.
    """
    def parse_config(conf_file):
        """Parse a config file, loading it into the ubconfig container

        Args:
            conf_file: Filename to load (within build_dir)

        Raises
            Exception if the file does not exist
        """
        dot_config = build_dir + '/' + conf_file
        if not os.path.exists(dot_config):
            raise Exception(conf_file + ' does not exist; ' +
                            'try passing --build option?')

        with open(dot_config, 'rt') as f:
            ini_str = '[root]\n' + f.read()
            ini_sio = io.StringIO(ini_str)
            parser = configparser.RawConfigParser()
            parser.read_file(ini_sio)
            ubconfig.buildconfig.update(parser.items('root'))

    global log
    global console
    global ubconfig

    source_dir = os.path.dirname(os.path.dirname(TEST_PY_DIR))

    board_type = config.getoption('board_type')
    board_type_filename = board_type.replace('-', '_')

    board_identity = config.getoption('board_identity')
    board_identity_filename = board_identity.replace('-', '_')

    build_dir = config.getoption('build_dir')
    if not build_dir:
        build_dir = source_dir + '/build-' + board_type
    mkdir_p(build_dir)

    result_dir = config.getoption('result_dir')
    if not result_dir:
        result_dir = build_dir
    mkdir_p(result_dir)

    persistent_data_dir = config.getoption('persistent_data_dir')
    if not persistent_data_dir:
        persistent_data_dir = build_dir + '/persistent-data'
    mkdir_p(persistent_data_dir)

    gdbserver = config.getoption('gdbserver')
    if gdbserver and not board_type.startswith('sandbox'):
        raise Exception('--gdbserver only supported with sandbox targets')

    import multiplexed_log
    log = multiplexed_log.Logfile(result_dir + '/test-log.html')

    if config.getoption('build'):
        worker_id = os.environ.get("PYTEST_XDIST_WORKER")
        with filelock.FileLock(os.path.join(build_dir, 'build.lock')):
            build_done_file = Path(build_dir) / 'build.done'
            if (not worker_id or worker_id == 'master' or
                not build_done_file.exists()):
                run_build(config, source_dir, build_dir, board_type, log)
                build_done_file.touch()

    class ArbitraryAttributeContainer(object):
        pass

    ubconfig = ArbitraryAttributeContainer()
    ubconfig.brd = dict()
    ubconfig.env = dict()

    modules = [
        (ubconfig.brd, 'u_boot_board_' + board_type_filename),
        (ubconfig.env, 'u_boot_boardenv_' + board_type_filename),
        (ubconfig.env, 'u_boot_boardenv_' + board_type_filename + '_' +
            board_identity_filename),
    ]
    for (dict_to_fill, module_name) in modules:
        try:
            module = __import__(module_name)
        except ImportError:
            continue
        dict_to_fill.update(module.__dict__)

    ubconfig.buildconfig = dict()

    # buildman -k puts autoconf.mk in the rootdir, so handle this as well
    # as the standard U-Boot build which leaves it in include/autoconf.mk
    parse_config('.config')
    if os.path.exists(build_dir + '/' + 'autoconf.mk'):
        parse_config('autoconf.mk')
    else:
        parse_config('include/autoconf.mk')

    ubconfig.test_py_dir = TEST_PY_DIR
    ubconfig.source_dir = source_dir
    ubconfig.build_dir = build_dir
    ubconfig.result_dir = result_dir
    ubconfig.persistent_data_dir = persistent_data_dir
    ubconfig.board_type = board_type
    ubconfig.board_identity = board_identity
    ubconfig.gdbserver = gdbserver
    ubconfig.dtb = build_dir + '/arch/sandbox/dts/test.dtb'

    env_vars = (
        'board_type',
        'board_identity',
        'source_dir',
        'test_py_dir',
        'build_dir',
        'result_dir',
        'persistent_data_dir',
    )
    for v in env_vars:
        os.environ['U_BOOT_' + v.upper()] = getattr(ubconfig, v)

    if board_type.startswith('sandbox'):
        import u_boot_console_sandbox
        console = u_boot_console_sandbox.ConsoleSandbox(log, ubconfig)
    else:
        import u_boot_console_exec_attach
        console = u_boot_console_exec_attach.ConsoleExecAttach(log, ubconfig)

re_ut_test_list = re.compile(r'[^a-zA-Z0-9_]_u_boot_list_2_ut_(.*)_test_2_(.*)\s*$')
def generate_ut_subtest(metafunc, fixture_name, sym_path):
    """Provide parametrization for a ut_subtest fixture.

    Determines the set of unit tests built into a U-Boot binary by parsing the
    list of symbols generated by the build process. Provides this information
    to test functions by parameterizing their ut_subtest fixture parameter.

    Args:
        metafunc: The pytest test function.
        fixture_name: The fixture name to test.
        sym_path: Relative path to the symbol file with preceding '/'
            (e.g. '/u-boot.sym')

    Returns:
        Nothing.
    """
    fn = console.config.build_dir + sym_path
    try:
        with open(fn, 'rt') as f:
            lines = f.readlines()
    except:
        lines = []
    lines.sort()

    vals = []
    for l in lines:
        m = re_ut_test_list.search(l)
        if not m:
            continue
        suite, name = m.groups()

        # Tests marked with _norun should only be run manually using 'ut -f'
        if name.endswith('_norun'):
            continue

        vals.append(f'{suite} {name}')

    ids = ['ut_' + s.replace(' ', '_') for s in vals]
    metafunc.parametrize(fixture_name, vals, ids=ids)

def generate_config(metafunc, fixture_name):
    """Provide parametrization for {env,brd}__ fixtures.

    If a test function takes parameter(s) (fixture names) of the form brd__xxx
    or env__xxx, the brd and env configuration dictionaries are consulted to
    find the list of values to use for those parameters, and the test is
    parametrized so that it runs once for each combination of values.

    Args:
        metafunc: The pytest test function.
        fixture_name: The fixture name to test.

    Returns:
        Nothing.
    """

    subconfigs = {
        'brd': console.config.brd,
        'env': console.config.env,
    }
    parts = fixture_name.split('__')
    if len(parts) < 2:
        return
    if parts[0] not in subconfigs:
        return
    subconfig = subconfigs[parts[0]]
    vals = []
    val = subconfig.get(fixture_name, [])
    # If that exact name is a key in the data source:
    if val:
        # ... use the dict value as a single parameter value.
        vals = (val, )
    else:
        # ... otherwise, see if there's a key that contains a list of
        # values to use instead.
        vals = subconfig.get(fixture_name+ 's', [])
    def fixture_id(index, val):
        try:
            return val['fixture_id']
        except:
            return fixture_name + str(index)
    ids = [fixture_id(index, val) for (index, val) in enumerate(vals)]
    metafunc.parametrize(fixture_name, vals, ids=ids)

def pytest_generate_tests(metafunc):
    """pytest hook: parameterize test functions based on custom rules.

    Check each test function parameter (fixture name) to see if it is one of
    our custom names, and if so, provide the correct parametrization for that
    parameter.

    Args:
        metafunc: The pytest test function.

    Returns:
        Nothing.
    """
    for fn in metafunc.fixturenames:
        if fn == 'ut_subtest':
            generate_ut_subtest(metafunc, fn, '/u-boot.sym')
            continue
        m_subtest = re.match('ut_(.)pl_subtest', fn)
        if m_subtest:
            spl_name = m_subtest.group(1)
            generate_ut_subtest(
                metafunc, fn, f'/{spl_name}pl/u-boot-{spl_name}pl.sym')
            continue
        generate_config(metafunc, fn)

@pytest.fixture(scope='session')
def u_boot_log(request):
     """Generate the value of a test's log fixture.

     Args:
         request: The pytest request.

     Returns:
         The fixture value.
     """

     return console.log

@pytest.fixture(scope='session')
def u_boot_config(request):
     """Generate the value of a test's u_boot_config fixture.

     Args:
         request: The pytest request.

     Returns:
         The fixture value.
     """

     return console.config

@pytest.fixture(scope='function')
def u_boot_console(request):
    """Generate the value of a test's u_boot_console fixture.

    Args:
        request: The pytest request.

    Returns:
        The fixture value.
    """

    console.ensure_spawned()
    return console

anchors = {}
tests_not_run = []
tests_failed = []
tests_xpassed = []
tests_xfailed = []
tests_skipped = []
tests_warning = []
tests_passed = []

def pytest_itemcollected(item):
    """pytest hook: Called once for each test found during collection.

    This enables our custom result analysis code to see the list of all tests
    that should eventually be run.

    Args:
        item: The item that was collected.

    Returns:
        Nothing.
    """

    tests_not_run.append(item.name)

def cleanup():
    """Clean up all global state.

    Executed (via atexit) once the entire test process is complete. This
    includes logging the status of all tests, and the identity of any failed
    or skipped tests.

    Args:
        None.

    Returns:
        Nothing.
    """

    if console:
        console.close()
    if log:
        with log.section('Status Report', 'status_report'):
            log.status_pass('%d passed' % len(tests_passed))
            if tests_warning:
                log.status_warning('%d passed with warning' % len(tests_warning))
                for test in tests_warning:
                    anchor = anchors.get(test, None)
                    log.status_warning('... ' + test, anchor)
            if tests_skipped:
                log.status_skipped('%d skipped' % len(tests_skipped))
                for test in tests_skipped:
                    anchor = anchors.get(test, None)
                    log.status_skipped('... ' + test, anchor)
            if tests_xpassed:
                log.status_xpass('%d xpass' % len(tests_xpassed))
                for test in tests_xpassed:
                    anchor = anchors.get(test, None)
                    log.status_xpass('... ' + test, anchor)
            if tests_xfailed:
                log.status_xfail('%d xfail' % len(tests_xfailed))
                for test in tests_xfailed:
                    anchor = anchors.get(test, None)
                    log.status_xfail('... ' + test, anchor)
            if tests_failed:
                log.status_fail('%d failed' % len(tests_failed))
                for test in tests_failed:
                    anchor = anchors.get(test, None)
                    log.status_fail('... ' + test, anchor)
            if tests_not_run:
                log.status_fail('%d not run' % len(tests_not_run))
                for test in tests_not_run:
                    anchor = anchors.get(test, None)
                    log.status_fail('... ' + test, anchor)
        log.close()
atexit.register(cleanup)

def setup_boardspec(item):
    """Process any 'boardspec' marker for a test.

    Such a marker lists the set of board types that a test does/doesn't
    support. If tests are being executed on an unsupported board, the test is
    marked to be skipped.

    Args:
        item: The pytest test item.

    Returns:
        Nothing.
    """

    required_boards = []
    for boards in item.iter_markers('boardspec'):
        board = boards.args[0]
        if board.startswith('!'):
            if ubconfig.board_type == board[1:]:
                pytest.skip('board "%s" not supported' % ubconfig.board_type)
                return
        else:
            required_boards.append(board)
    if required_boards and ubconfig.board_type not in required_boards:
        pytest.skip('board "%s" not supported' % ubconfig.board_type)

def setup_buildconfigspec(item):
    """Process any 'buildconfigspec' marker for a test.

    Such a marker lists some U-Boot configuration feature that the test
    requires. If tests are being executed on an U-Boot build that doesn't
    have the required feature, the test is marked to be skipped.

    Args:
        item: The pytest test item.

    Returns:
        Nothing.
    """

    for options in item.iter_markers('buildconfigspec'):
        option = options.args[0]
        if not ubconfig.buildconfig.get('config_' + option.lower(), None):
            pytest.skip('.config feature "%s" not enabled' % option.lower())
    for options in item.iter_markers('notbuildconfigspec'):
        option = options.args[0]
        if ubconfig.buildconfig.get('config_' + option.lower(), None):
            pytest.skip('.config feature "%s" enabled' % option.lower())

def tool_is_in_path(tool):
    for path in os.environ["PATH"].split(os.pathsep):
        fn = os.path.join(path, tool)
        if os.path.isfile(fn) and os.access(fn, os.X_OK):
            return True
    return False

def setup_requiredtool(item):
    """Process any 'requiredtool' marker for a test.

    Such a marker lists some external tool (binary, executable, application)
    that the test requires. If tests are being executed on a system that
    doesn't have the required tool, the test is marked to be skipped.

    Args:
        item: The pytest test item.

    Returns:
        Nothing.
    """

    for tools in item.iter_markers('requiredtool'):
        tool = tools.args[0]
        if not tool_is_in_path(tool):
            pytest.skip('tool "%s" not in $PATH' % tool)

def setup_singlethread(item):
    """Process any 'singlethread' marker for a test.

    Skip this test if running in parallel.

    Args:
        item: The pytest test item.

    Returns:
        Nothing.
    """
    for single in item.iter_markers('singlethread'):
        worker_id = os.environ.get("PYTEST_XDIST_WORKER")
        if worker_id and worker_id != 'master':
            pytest.skip('must run single-threaded')

def start_test_section(item):
    anchors[item.name] = log.start_section(item.name)

def pytest_runtest_setup(item):
    """pytest hook: Configure (set up) a test item.

    Called once for each test to perform any custom configuration. This hook
    is used to skip the test if certain conditions apply.

    Args:
        item: The pytest test item.

    Returns:
        Nothing.
    """

    start_test_section(item)
    setup_boardspec(item)
    setup_buildconfigspec(item)
    setup_requiredtool(item)
    setup_singlethread(item)

def pytest_runtest_protocol(item, nextitem):
    """pytest hook: Called to execute a test.

    This hook wraps the standard pytest runtestprotocol() function in order
    to acquire visibility into, and record, each test function's result.

    Args:
        item: The pytest test item to execute.
        nextitem: The pytest test item that will be executed after this one.

    Returns:
        A list of pytest reports (test result data).
    """

    log.get_and_reset_warning()
    ihook = item.ihook
    ihook.pytest_runtest_logstart(nodeid=item.nodeid, location=item.location)
    reports = runtestprotocol(item, nextitem=nextitem)
    ihook.pytest_runtest_logfinish(nodeid=item.nodeid, location=item.location)
    was_warning = log.get_and_reset_warning()

    # In pytest 3, runtestprotocol() may not call pytest_runtest_setup() if
    # the test is skipped. That call is required to create the test's section
    # in the log file. The call to log.end_section() requires that the log
    # contain a section for this test. Create a section for the test if it
    # doesn't already exist.
    if not item.name in anchors:
        start_test_section(item)

    failure_cleanup = False
    if not was_warning:
        test_list = tests_passed
        msg = 'OK'
        msg_log = log.status_pass
    else:
        test_list = tests_warning
        msg = 'OK (with warning)'
        msg_log = log.status_warning
    for report in reports:
        if report.outcome == 'failed':
            if hasattr(report, 'wasxfail'):
                test_list = tests_xpassed
                msg = 'XPASSED'
                msg_log = log.status_xpass
            else:
                failure_cleanup = True
                test_list = tests_failed
                msg = 'FAILED:\n' + str(report.longrepr)
                msg_log = log.status_fail
            break
        if report.outcome == 'skipped':
            if hasattr(report, 'wasxfail'):
                failure_cleanup = True
                test_list = tests_xfailed
                msg = 'XFAILED:\n' + str(report.longrepr)
                msg_log = log.status_xfail
                break
            test_list = tests_skipped
            msg = 'SKIPPED:\n' + str(report.longrepr)
            msg_log = log.status_skipped

    if failure_cleanup:
        console.drain_console()

    test_list.append(item.name)
    tests_not_run.remove(item.name)

    try:
        msg_log(msg)
    except:
        # If something went wrong with logging, it's better to let the test
        # process continue, which may report other exceptions that triggered
        # the logging issue (e.g. console.log wasn't created). Hence, just
        # squash the exception. If the test setup failed due to e.g. syntax
        # error somewhere else, this won't be seen. However, once that issue
        # is fixed, if this exception still exists, it will then be logged as
        # part of the test's stdout.
        import traceback
        print('Exception occurred while logging runtest status:')
        traceback.print_exc()
        # FIXME: Can we force a test failure here?

    log.end_section(item.name)

    if failure_cleanup:
        console.cleanup_spawn()

    return True
