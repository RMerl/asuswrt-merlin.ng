# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2014 Google, Inc
#

import os
from pathlib import Path
import shutil
import sys
import tempfile
import time
import unittest

from buildman import board
from buildman import boards
from buildman import bsettings
from buildman import cmdline
from buildman import control
from buildman import toolchain
from patman import gitutil
from u_boot_pylib import command
from u_boot_pylib import terminal
from u_boot_pylib import test_util
from u_boot_pylib import tools

settings_data = '''
# Buildman settings file
[global]

[toolchain]

[toolchain-alias]

[make-flags]
src=/home/sjg/c/src
chroot=/home/sjg/c/chroot
vboot=VBOOT_DEBUG=1 MAKEFLAGS_VBOOT=DEBUG=1 CFLAGS_EXTRA_VBOOT=-DUNROLL_LOOPS VBOOT_SOURCE=${src}/platform/vboot_reference
chromeos_coreboot=VBOOT=${chroot}/build/link/usr ${vboot}
chromeos_daisy=VBOOT=${chroot}/build/daisy/usr ${vboot}
chromeos_peach=VBOOT=${chroot}/build/peach_pit/usr ${vboot}
'''

BOARDS = [
    ['Active', 'arm', 'armv7', '', 'Tester', 'ARM Board 0', 'board0',  ''],
    ['Active', 'arm', 'armv7', '', 'Tester', 'ARM Board 1', 'board1', ''],
    ['Active', 'powerpc', 'powerpc', '', 'Tester', 'PowerPC board 1', 'board2', ''],
    ['Active', 'sandbox', 'sandbox', '', 'Tester', 'Sandbox board', 'board4', ''],
]

commit_shortlog = """4aca821 patman: Avoid changing the order of tags
39403bb patman: Use --no-pager' to stop git from forking a pager
db6e6f2 patman: Remove the -a option
f2ccf03 patman: Correct unit tests to run correctly
1d097f9 patman: Fix indentation in terminal.py
d073747 patman: Support the 'reverse' option for 'git log
"""

commit_log = ["""commit 7f6b8315d18f683c5181d0c3694818c1b2a20dcd
Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
Date:   Fri Aug 22 19:12:41 2014 +0900

    buildman: refactor help message

    "buildman [options]" is displayed by default.

    Append the rest of help messages to parser.usage
    instead of replacing it.

    Besides, "-b <branch>" is not mandatory since commit fea5858e.
    Drop it from the usage.

    Signed-off-by: Masahiro Yamada <yamada.m@jp.panasonic.com>
""",
"""commit d0737479be6baf4db5e2cdbee123e96bc5ed0ba8
Author: Simon Glass <sjg@chromium.org>
Date:   Thu Aug 14 16:48:25 2014 -0600

    patman: Support the 'reverse' option for 'git log'

    This option is currently not supported, but needs to be, for buildman to
    operate as expected.

    Series-changes: 7
    - Add new patch to fix the 'reverse' bug

    Series-version: 8

    Change-Id: I79078f792e8b390b8a1272a8023537821d45feda
    Reported-by: York Sun <yorksun@freescale.com>
    Signed-off-by: Simon Glass <sjg@chromium.org>

""",
"""commit 1d097f9ab487c5019152fd47bda126839f3bf9fc
Author: Simon Glass <sjg@chromium.org>
Date:   Sat Aug 9 11:44:32 2014 -0600

    patman: Fix indentation in terminal.py

    This code came from a different project with 2-character indentation. Fix
    it for U-Boot.

    Series-changes: 6
    - Add new patch to fix indentation in teminal.py

    Change-Id: I5a74d2ebbb3cc12a665f5c725064009ac96e8a34
    Signed-off-by: Simon Glass <sjg@chromium.org>

""",
"""commit f2ccf03869d1e152c836515a3ceb83cdfe04a105
Author: Simon Glass <sjg@chromium.org>
Date:   Sat Aug 9 11:08:24 2014 -0600

    patman: Correct unit tests to run correctly

    It seems that doctest behaves differently now, and some of the unit tests
    do not run. Adjust the tests to work correctly.

     ./tools/patman/patman --test
    <unittest.result.TestResult run=10 errors=0 failures=0>

    Series-changes: 6
    - Add new patch to fix patman unit tests

    Change-Id: I3d2ca588f4933e1f9d6b1665a00e4ae58269ff3b

""",
"""commit db6e6f2f9331c5a37647d6668768d4a40b8b0d1c
Author: Simon Glass <sjg@chromium.org>
Date:   Sat Aug 9 12:06:02 2014 -0600

    patman: Remove the -a option

    It seems that this is no longer needed, since checkpatch.pl will catch
    whitespace problems in patches. Also the option is not widely used, so
    it seems safe to just remove it.

    Series-changes: 6
    - Add new patch to remove patman's -a option

    Suggested-by: Masahiro Yamada <yamada.m@jp.panasonic.com>
    Change-Id: I5821a1c75154e532c46513486ca40b808de7e2cc

""",
"""commit 39403bb4f838153028a6f21ca30bf100f3791133
Author: Simon Glass <sjg@chromium.org>
Date:   Thu Aug 14 21:50:52 2014 -0600

    patman: Use --no-pager' to stop git from forking a pager

""",
"""commit 4aca821e27e97925c039e69fd37375b09c6f129c
Author: Simon Glass <sjg@chromium.org>
Date:   Fri Aug 22 15:57:39 2014 -0600

    patman: Avoid changing the order of tags

    patman collects tags that it sees in the commit and places them nicely
    sorted at the end of the patch. However, this is not really necessary and
    in fact is apparently not desirable.

    Series-changes: 9
    - Add new patch to avoid changing the order of tags

    Series-version: 9

    Suggested-by: Masahiro Yamada <yamada.m@jp.panasonic.com>
    Change-Id: Ib1518588c1a189ad5c3198aae76f8654aed8d0db
"""]

TEST_BRANCH = '__testbranch'

class TestFunctional(unittest.TestCase):
    """Functional test for buildman.

    This aims to test from just below the invocation of buildman (parsing
    of arguments) to 'make' and 'git' invocation. It is not a true
    emd-to-end test, as it mocks git, make and the tool chain. But this
    makes it easier to detect when the builder is doing the wrong thing,
    since in many cases this test code will fail. For example, only a
    very limited subset of 'git' arguments is supported - anything
    unexpected will fail.
    """
    def setUp(self):
        self._base_dir = tempfile.mkdtemp()
        self._output_dir = tempfile.mkdtemp()
        self._git_dir = os.path.join(self._base_dir, 'src')
        self._buildman_pathname = sys.argv[0]
        self._buildman_dir = os.path.dirname(os.path.realpath(sys.argv[0]))
        command.test_result = self._HandleCommand
        bsettings.setup(None)
        bsettings.add_file(settings_data)
        self.setupToolchains()
        self._toolchains.Add('arm-gcc', test=False)
        self._toolchains.Add('powerpc-gcc', test=False)
        self._boards = boards.Boards()
        for brd in BOARDS:
            self._boards.add_board(board.Board(*brd))

        # Directories where the source been cloned
        self._clone_dirs = []
        self._commits = len(commit_shortlog.splitlines()) + 1
        self._total_builds = self._commits * len(BOARDS)

        # Number of calls to make
        self._make_calls = 0

        # Map of [board, commit] to error messages
        self._error = {}

        self._test_branch = TEST_BRANCH

        # Set to True to report missing blobs
        self._missing = False

        self._buildman_dir = os.path.dirname(os.path.realpath(sys.argv[0]))
        self._test_dir = os.path.join(self._buildman_dir, 'test')

        # Set up some fake source files
        shutil.copytree(self._test_dir, self._git_dir)

        # Avoid sending any output and clear all terminal output
        terminal.set_print_test_mode()
        terminal.get_print_test_lines()

    def tearDown(self):
        shutil.rmtree(self._base_dir)
        shutil.rmtree(self._output_dir)

    def setupToolchains(self):
        self._toolchains = toolchain.Toolchains()
        self._toolchains.Add('gcc', test=False)

    def _RunBuildman(self, *args):
        return command.run_pipe([[self._buildman_pathname] + list(args)],
                capture=True, capture_stderr=True)

    def _RunControl(self, *args, brds=False, clean_dir=False,
                    test_thread_exceptions=False, get_builder=True):
        """Run buildman

        Args:
            args: List of arguments to pass
            brds: Boards object, or False to pass self._boards, or None to pass
                None
            clean_dir: Used for tests only, indicates that the existing output_dir
                should be removed before starting the build
            test_thread_exceptions: Uses for tests only, True to make the threads
                raise an exception instead of reporting their result. This simulates
                a failure in the code somewhere
            get_builder (bool): Set self._builder to the resulting builder

        Returns:
            result code from buildman
        """
        sys.argv = [sys.argv[0]] + list(args)
        args = cmdline.parse_args()
        if brds == False:
            brds = self._boards
        result = control.do_buildman(
            args, toolchains=self._toolchains, make_func=self._HandleMake,
            brds=brds, clean_dir=clean_dir,
            test_thread_exceptions=test_thread_exceptions)
        if get_builder:
            self._builder = control.TEST_BUILDER
        return result

    def testFullHelp(self):
        command.test_result = None
        result = self._RunBuildman('-H')
        help_file = os.path.join(self._buildman_dir, 'README.rst')
        # Remove possible extraneous strings
        extra = '::::::::::::::\n' + help_file + '\n::::::::::::::\n'
        gothelp = result.stdout.replace(extra, '')
        self.assertEqual(len(gothelp), os.path.getsize(help_file))
        self.assertEqual(0, len(result.stderr))
        self.assertEqual(0, result.return_code)

    def testHelp(self):
        command.test_result = None
        result = self._RunBuildman('-h')
        help_file = os.path.join(self._buildman_dir, 'README.rst')
        self.assertTrue(len(result.stdout) > 1000)
        self.assertEqual(0, len(result.stderr))
        self.assertEqual(0, result.return_code)

    def testGitSetup(self):
        """Test gitutils.Setup(), from outside the module itself"""
        command.test_result = command.CommandResult(return_code=1)
        gitutil.setup()
        self.assertEqual(gitutil.use_no_decorate, False)

        command.test_result = command.CommandResult(return_code=0)
        gitutil.setup()
        self.assertEqual(gitutil.use_no_decorate, True)

    def _HandleCommandGitLog(self, args):
        if args[-1] == '--':
            args = args[:-1]
        if '-n0' in args:
            return command.CommandResult(return_code=0)
        elif args[-1] == 'upstream/master..%s' % self._test_branch:
            return command.CommandResult(return_code=0, stdout=commit_shortlog)
        elif args[:3] == ['--no-color', '--no-decorate', '--reverse']:
            if args[-1] == self._test_branch:
                count = int(args[3][2:])
                return command.CommandResult(return_code=0,
                                            stdout=''.join(commit_log[:count]))

        # Not handled, so abort
        print('git log', args)
        sys.exit(1)

    def _HandleCommandGitConfig(self, args):
        config = args[0]
        if config == 'sendemail.aliasesfile':
            return command.CommandResult(return_code=0)
        elif config.startswith('branch.badbranch'):
            return command.CommandResult(return_code=1)
        elif config == 'branch.%s.remote' % self._test_branch:
            return command.CommandResult(return_code=0, stdout='upstream\n')
        elif config == 'branch.%s.merge' % self._test_branch:
            return command.CommandResult(return_code=0,
                                         stdout='refs/heads/master\n')

        # Not handled, so abort
        print('git config', args)
        sys.exit(1)

    def _HandleCommandGit(self, in_args):
        """Handle execution of a git command

        This uses a hacked-up parser.

        Args:
            in_args: Arguments after 'git' from the command line
        """
        git_args = []           # Top-level arguments to git itself
        sub_cmd = None          # Git sub-command selected
        args = []               # Arguments to the git sub-command
        for arg in in_args:
            if sub_cmd:
                args.append(arg)
            elif arg[0] == '-':
                git_args.append(arg)
            else:
                if git_args and git_args[-1] in ['--git-dir', '--work-tree']:
                    git_args.append(arg)
                else:
                    sub_cmd = arg
        if sub_cmd == 'config':
            return self._HandleCommandGitConfig(args)
        elif sub_cmd == 'log':
            return self._HandleCommandGitLog(args)
        elif sub_cmd == 'clone':
            return command.CommandResult(return_code=0)
        elif sub_cmd == 'checkout':
            return command.CommandResult(return_code=0)
        elif sub_cmd == 'worktree':
            return command.CommandResult(return_code=0)

        # Not handled, so abort
        print('git', git_args, sub_cmd, args)
        sys.exit(1)

    def _HandleCommandNm(self, args):
        return command.CommandResult(return_code=0)

    def _HandleCommandObjdump(self, args):
        return command.CommandResult(return_code=0)

    def _HandleCommandObjcopy(self, args):
        return command.CommandResult(return_code=0)

    def _HandleCommandSize(self, args):
        return command.CommandResult(return_code=0)

    def _HandleCommand(self, **kwargs):
        """Handle a command execution.

        The command is in kwargs['pipe-list'], as a list of pipes, each a
        list of commands. The command should be emulated as required for
        testing purposes.

        Returns:
            A CommandResult object
        """
        pipe_list = kwargs['pipe_list']
        wc = False
        if len(pipe_list) != 1:
            if pipe_list[1] == ['wc', '-l']:
                wc = True
            else:
                print('invalid pipe', kwargs)
                sys.exit(1)
        cmd = pipe_list[0][0]
        args = pipe_list[0][1:]
        result = None
        if cmd == 'git':
            result = self._HandleCommandGit(args)
        elif cmd == './scripts/show-gnu-make':
            return command.CommandResult(return_code=0, stdout='make')
        elif cmd.endswith('nm'):
            return self._HandleCommandNm(args)
        elif cmd.endswith('objdump'):
            return self._HandleCommandObjdump(args)
        elif cmd.endswith('objcopy'):
            return self._HandleCommandObjcopy(args)
        elif cmd.endswith( 'size'):
            return self._HandleCommandSize(args)

        if not result:
            # Not handled, so abort
            print('unknown command', kwargs)
            sys.exit(1)

        if wc:
            result.stdout = len(result.stdout.splitlines())
        return result

    def _HandleMake(self, commit, brd, stage, cwd, *args, **kwargs):
        """Handle execution of 'make'

        Args:
            commit: Commit object that is being built
            brd: Board object that is being built
            stage: Stage that we are at (mrproper, config, build)
            cwd: Directory where make should be run
            args: Arguments to pass to make
            kwargs: Arguments to pass to command.run_pipe()
        """
        self._make_calls += 1
        out_dir = ''
        for arg in args:
            if arg.startswith('O='):
                out_dir = arg[2:]
        if stage == 'mrproper':
            return command.CommandResult(return_code=0)
        elif stage == 'config':
            fname = os.path.join(cwd or '', out_dir, '.config')
            tools.write_file(fname, b'CONFIG_SOMETHING=1')
            return command.CommandResult(return_code=0,
                    combined='Test configuration complete')
        elif stage == 'oldconfig':
            return command.CommandResult(return_code=0)
        elif stage == 'build':
            stderr = ''
            fname = os.path.join(cwd or '', out_dir, 'u-boot')
            tools.write_file(fname, b'U-Boot')

            # Handle missing blobs
            if self._missing:
                if 'BINMAN_ALLOW_MISSING=1' in args:
                    stderr = '''+Image 'main-section' is missing external blobs and is non-functional: intel-descriptor intel-ifwi intel-fsp-m intel-fsp-s intel-vbt
Image 'main-section' has faked external blobs and is non-functional: descriptor.bin fsp_m.bin fsp_s.bin vbt.bin

Some images are invalid'''
                else:
                    stderr = "binman: Filename 'fsp.bin' not found in input path"
            elif type(commit) is not str:
                stderr = self._error.get((brd.target, commit.sequence))

            if stderr:
                return command.CommandResult(return_code=2, stderr=stderr)
            return command.CommandResult(return_code=0)

        # Not handled, so abort
        print('_HandleMake failure: make', stage)
        sys.exit(1)

    # Example function to print output lines
    def print_lines(self, lines):
        print(len(lines))
        for line in lines:
            print(line)
        #self.print_lines(terminal.get_print_test_lines())

    def testNoBoards(self):
        """Test that buildman aborts when there are no boards"""
        self._boards = boards.Boards()
        with self.assertRaises(SystemExit):
            self._RunControl()

    def testCurrentSource(self):
        """Very simple test to invoke buildman on the current source"""
        self.setupToolchains();
        self._RunControl('-o', self._output_dir)
        lines = terminal.get_print_test_lines()
        self.assertIn('Building current source for %d boards' % len(BOARDS),
                      lines[0].text)

    def testBadBranch(self):
        """Test that we can detect an invalid branch"""
        with self.assertRaises(ValueError):
            self._RunControl('-b', 'badbranch')

    def testBadToolchain(self):
        """Test that missing toolchains are detected"""
        self.setupToolchains();
        ret_code = self._RunControl('-b', TEST_BRANCH, '-o', self._output_dir)
        lines = terminal.get_print_test_lines()

        # Buildman always builds the upstream commit as well
        self.assertIn('Building %d commits for %d boards' %
                (self._commits, len(BOARDS)), lines[0].text)
        self.assertEqual(self._builder.count, self._total_builds)

        # Only sandbox should succeed, the others don't have toolchains
        self.assertEqual(self._builder.fail,
                         self._total_builds - self._commits)
        self.assertEqual(ret_code, 100)

        for commit in range(self._commits):
            for brd in self._boards.get_list():
                if brd.arch != 'sandbox':
                  errfile = self._builder.get_err_file(commit, brd.target)
                  fd = open(errfile)
                  self.assertEqual(
                      fd.readlines(),
                      [f'Tool chain error for {brd.arch}: '
                       f"No tool chain found for arch '{brd.arch}'"])
                  fd.close()

    def testBranch(self):
        """Test building a branch with all toolchains present"""
        self._RunControl('-b', TEST_BRANCH, '-o', self._output_dir)
        self.assertEqual(self._builder.count, self._total_builds)
        self.assertEqual(self._builder.fail, 0)

    def testCount(self):
        """Test building a specific number of commitst"""
        self._RunControl('-b', TEST_BRANCH, '-c2', '-o', self._output_dir)
        self.assertEqual(self._builder.count, 2 * len(BOARDS))
        self.assertEqual(self._builder.fail, 0)
        # Each board has a config, and then one make per commit
        self.assertEqual(self._make_calls, len(BOARDS) * (1 + 2))

    def testIncremental(self):
        """Test building a branch twice - the second time should do nothing"""
        self._RunControl('-b', TEST_BRANCH, '-o', self._output_dir)

        # Each board has a mrproper, config, and then one make per commit
        self.assertEqual(self._make_calls, len(BOARDS) * (self._commits + 1))
        self._make_calls = 0
        self._RunControl('-b', TEST_BRANCH, '-o', self._output_dir, clean_dir=False)
        self.assertEqual(self._make_calls, 0)
        self.assertEqual(self._builder.count, self._total_builds)
        self.assertEqual(self._builder.fail, 0)

    def testForceBuild(self):
        """The -f flag should force a rebuild"""
        self._RunControl('-b', TEST_BRANCH, '-o', self._output_dir)
        self._make_calls = 0
        self._RunControl('-b', TEST_BRANCH, '-f', '-o', self._output_dir, clean_dir=False)
        # Each board has a config and one make per commit
        self.assertEqual(self._make_calls, len(BOARDS) * (self._commits + 1))

    def testForceReconfigure(self):
        """The -f flag should force a rebuild"""
        self._RunControl('-b', TEST_BRANCH, '-C', '-o', self._output_dir)
        # Each commit has a config and make
        self.assertEqual(self._make_calls, len(BOARDS) * self._commits * 2)

    def testMrproper(self):
        """The -f flag should force a rebuild"""
        self._RunControl('-b', TEST_BRANCH, '-m', '-o', self._output_dir)
        # Each board has a mkproper, config and then one make per commit
        self.assertEqual(self._make_calls, len(BOARDS) * (self._commits + 2))

    def testErrors(self):
        """Test handling of build errors"""
        self._error['board2', 1] = 'fred\n'
        self._RunControl('-b', TEST_BRANCH, '-o', self._output_dir)
        self.assertEqual(self._builder.count, self._total_builds)
        self.assertEqual(self._builder.fail, 1)

        # Remove the error. This should have no effect since the commit will
        # not be rebuilt
        del self._error['board2', 1]
        self._make_calls = 0
        self._RunControl('-b', TEST_BRANCH, '-o', self._output_dir, clean_dir=False)
        self.assertEqual(self._builder.count, self._total_builds)
        self.assertEqual(self._make_calls, 0)
        self.assertEqual(self._builder.fail, 1)

        # Now use the -F flag to force rebuild of the bad commit
        self._RunControl('-b', TEST_BRANCH, '-o', self._output_dir, '-F', clean_dir=False)
        self.assertEqual(self._builder.count, self._total_builds)
        self.assertEqual(self._builder.fail, 0)
        self.assertEqual(self._make_calls, 2)

    def testBranchWithSlash(self):
        """Test building a branch with a '/' in the name"""
        self._test_branch = '/__dev/__testbranch'
        self._RunControl('-b', self._test_branch, '-o', self._output_dir,
                         clean_dir=False)
        self.assertEqual(self._builder.count, self._total_builds)
        self.assertEqual(self._builder.fail, 0)

    def testEnvironment(self):
        """Test that the done and environment files are written to out-env"""
        self._RunControl('-o', self._output_dir)
        board0_dir = os.path.join(self._output_dir, 'current', 'board0')
        self.assertTrue(os.path.exists(os.path.join(board0_dir, 'done')))
        self.assertTrue(os.path.exists(os.path.join(board0_dir, 'out-env')))

    def testEnvironmentUnicode(self):
        """Test there are no unicode errors when the env has non-ASCII chars"""
        try:
            varname = b'buildman_test_var'
            os.environb[varname] = b'strange\x80chars'
            self.assertEqual(0, self._RunControl('-o', self._output_dir))
            board0_dir = os.path.join(self._output_dir, 'current', 'board0')
            self.assertTrue(os.path.exists(os.path.join(board0_dir, 'done')))
            self.assertTrue(os.path.exists(os.path.join(board0_dir, 'out-env')))
        finally:
            del os.environb[varname]

    def testWorkInOutput(self):
        """Test the -w option which should write directly to the output dir"""
        board_list = boards.Boards()
        board_list.add_board(board.Board(*BOARDS[0]))
        self._RunControl('-o', self._output_dir, '-w', clean_dir=False,
                         brds=board_list)
        self.assertTrue(
            os.path.exists(os.path.join(self._output_dir, 'u-boot')))
        self.assertTrue(
            os.path.exists(os.path.join(self._output_dir, 'done')))
        self.assertTrue(
            os.path.exists(os.path.join(self._output_dir, 'out-env')))

    def testWorkInOutputFail(self):
        """Test the -w option failures"""
        with self.assertRaises(SystemExit) as e:
            self._RunControl('-o', self._output_dir, '-w', clean_dir=False)
        self.assertIn("single board", str(e.exception))
        self.assertFalse(
            os.path.exists(os.path.join(self._output_dir, 'u-boot')))

        board_list = boards.Boards()
        board_list.add_board(board.Board(*BOARDS[0]))
        with self.assertRaises(SystemExit) as e:
            self._RunControl('-b', self._test_branch, '-o', self._output_dir,
                             '-w', clean_dir=False, brds=board_list)
        self.assertIn("single commit", str(e.exception))

        board_list = boards.Boards()
        board_list.add_board(board.Board(*BOARDS[0]))
        with self.assertRaises(SystemExit) as e:
            self._RunControl('-w', clean_dir=False)
        self.assertIn("specify -o", str(e.exception))

    def testThreadExceptions(self):
        """Test that exceptions in threads are reported"""
        with test_util.capture_sys_output() as (stdout, stderr):
            self.assertEqual(102, self._RunControl('-o', self._output_dir,
                                                   test_thread_exceptions=True))
        self.assertIn(
            'Thread exception (use -T0 to run without threads): test exception',
            stdout.getvalue())

    def testBlobs(self):
        """Test handling of missing blobs"""
        self._missing = True

        board0_dir = os.path.join(self._output_dir, 'current', 'board0')
        errfile = os.path.join(board0_dir, 'err')
        logfile = os.path.join(board0_dir, 'log')

        # We expect failure when there are missing blobs
        result = self._RunControl('board0', '-o', self._output_dir)
        self.assertEqual(100, result)
        self.assertTrue(os.path.exists(os.path.join(board0_dir, 'done')))
        self.assertTrue(os.path.exists(errfile))
        self.assertIn(b"Filename 'fsp.bin' not found in input path",
                      tools.read_file(errfile))

    def testBlobsAllowMissing(self):
        """Allow missing blobs - still failure but a different exit code"""
        self._missing = True
        result = self._RunControl('board0', '-o', self._output_dir, '-M',
                                  clean_dir=True)
        self.assertEqual(101, result)
        board0_dir = os.path.join(self._output_dir, 'current', 'board0')
        errfile = os.path.join(board0_dir, 'err')
        self.assertTrue(os.path.exists(errfile))
        self.assertIn(b'Some images are invalid', tools.read_file(errfile))

    def testBlobsWarning(self):
        """Allow missing blobs and ignore warnings"""
        self._missing = True
        result = self._RunControl('board0', '-o', self._output_dir, '-MW')
        self.assertEqual(0, result)
        board0_dir = os.path.join(self._output_dir, 'current', 'board0')
        errfile = os.path.join(board0_dir, 'err')
        self.assertIn(b'Some images are invalid', tools.read_file(errfile))

    def testBlobSettings(self):
        """Test with no settings"""
        self.assertEqual(False,
                         control.get_allow_missing(False, False, 1, False))
        self.assertEqual(True,
                         control.get_allow_missing(True, False, 1, False))
        self.assertEqual(False,
                         control.get_allow_missing(True, True, 1, False))

    def testBlobSettingsAlways(self):
        """Test the 'always' policy"""
        bsettings.set_item('global', 'allow-missing', 'always')
        self.assertEqual(True,
                         control.get_allow_missing(False, False, 1, False))
        self.assertEqual(False,
                         control.get_allow_missing(False, True, 1, False))

    def testBlobSettingsBranch(self):
        """Test the 'branch' policy"""
        bsettings.set_item('global', 'allow-missing', 'branch')
        self.assertEqual(False,
                         control.get_allow_missing(False, False, 1, False))
        self.assertEqual(True,
                         control.get_allow_missing(False, False, 1, True))
        self.assertEqual(False,
                         control.get_allow_missing(False, True, 1, True))

    def testBlobSettingsMultiple(self):
        """Test the 'multiple' policy"""
        bsettings.set_item('global', 'allow-missing', 'multiple')
        self.assertEqual(False,
                         control.get_allow_missing(False, False, 1, False))
        self.assertEqual(True,
                         control.get_allow_missing(False, False, 2, False))
        self.assertEqual(False,
                         control.get_allow_missing(False, True, 2, False))

    def testBlobSettingsBranchMultiple(self):
        """Test the 'branch multiple' policy"""
        bsettings.set_item('global', 'allow-missing', 'branch multiple')
        self.assertEqual(False,
                         control.get_allow_missing(False, False, 1, False))
        self.assertEqual(True,
                         control.get_allow_missing(False, False, 1, True))
        self.assertEqual(True,
                         control.get_allow_missing(False, False, 2, False))
        self.assertEqual(True,
                         control.get_allow_missing(False, False, 2, True))
        self.assertEqual(False,
                         control.get_allow_missing(False, True, 2, True))

    def check_command(self, *extra_args):
        """Run a command with the extra arguments and return the commands used

        Args:
            extra_args (list of str): List of extra arguments

        Returns:
            list of str: Lines returned in the out-cmd file
        """
        self._RunControl('-o', self._output_dir, *extra_args)
        board0_dir = os.path.join(self._output_dir, 'current', 'board0')
        self.assertTrue(os.path.exists(os.path.join(board0_dir, 'done')))
        cmd_fname = os.path.join(board0_dir, 'out-cmd')
        self.assertTrue(os.path.exists(cmd_fname))
        data = tools.read_file(cmd_fname)

        config_fname = os.path.join(board0_dir, '.config')
        self.assertTrue(os.path.exists(config_fname))
        cfg_data = tools.read_file(config_fname)

        return data.splitlines(), cfg_data

    def testCmdFile(self):
        """Test that the -cmd-out file is produced"""
        lines = self.check_command()[0]
        self.assertEqual(2, len(lines))
        self.assertRegex(lines[0], b'make O=/.*board0_defconfig')
        self.assertRegex(lines[0], b'make O=/.*-s.*')

    def testNoLto(self):
        """Test that the --no-lto flag works"""
        lines = self.check_command('-L')[0]
        self.assertIn(b'NO_LTO=1', lines[0])

    def testReproducible(self):
        """Test that the -r flag works"""
        lines, cfg_data = self.check_command('-r')
        self.assertIn(b'SOURCE_DATE_EPOCH=0', lines[0])

        # We should see CONFIG_LOCALVERSION_AUTO unset
        self.assertEqual(b'''CONFIG_SOMETHING=1
# CONFIG_LOCALVERSION_AUTO is not set
''', cfg_data)

        with test_util.capture_sys_output() as (stdout, stderr):
            lines, cfg_data = self.check_command('-r', '-a', 'LOCALVERSION')
        self.assertIn(b'SOURCE_DATE_EPOCH=0', lines[0])

        # We should see CONFIG_LOCALVERSION_AUTO unset
        self.assertEqual(b'''CONFIG_SOMETHING=1
CONFIG_LOCALVERSION=y
''', cfg_data)
        self.assertIn('Not dropping LOCALVERSION_AUTO', stdout.getvalue())

    def test_scan_defconfigs(self):
        """Test scanning the defconfigs to obtain all the boards"""
        src = self._git_dir

        # Scan the test directory which contains a Kconfig and some *_defconfig
        # files
        params, warnings = self._boards.scan_defconfigs(src, src)

        # We should get two boards
        self.assertEquals(2, len(params))
        self.assertFalse(warnings)
        first = 0 if params[0]['target'] == 'board0' else 1
        board0 = params[first]
        board2 = params[1 - first]

        self.assertEquals('arm', board0['arch'])
        self.assertEquals('armv7', board0['cpu'])
        self.assertEquals('-', board0['soc'])
        self.assertEquals('Tester', board0['vendor'])
        self.assertEquals('ARM Board 0', board0['board'])
        self.assertEquals('config0', board0['config'])
        self.assertEquals('board0', board0['target'])

        self.assertEquals('powerpc', board2['arch'])
        self.assertEquals('ppc', board2['cpu'])
        self.assertEquals('mpc85xx', board2['soc'])
        self.assertEquals('Tester', board2['vendor'])
        self.assertEquals('PowerPC board 1', board2['board'])
        self.assertEquals('config2', board2['config'])
        self.assertEquals('board2', board2['target'])

    def test_output_is_new(self):
        """Test detecting new changes to Kconfig"""
        base = self._base_dir
        src = self._git_dir
        config_dir = os.path.join(src, 'configs')
        delay = 0.02

        # Create a boards.cfg file
        boards_cfg = os.path.join(base, 'boards.cfg')
        content = b'''#
# List of boards
#   Automatically generated by buildman/boards.py: don't edit
#
# Status, Arch, CPU, SoC, Vendor, Board, Target, Config, Maintainers

Active  aarch64     armv8 - armltd corstone1000 board0
Active  aarch64     armv8 - armltd total_compute board2
'''
        # Check missing file
        self.assertFalse(boards.output_is_new(boards_cfg, config_dir, src))

        # Check that the board.cfg file is newer
        time.sleep(delay)
        tools.write_file(boards_cfg, content)
        self.assertTrue(boards.output_is_new(boards_cfg, config_dir, src))

        # Touch the Kconfig files after a show delay to avoid a race
        time.sleep(delay)
        Path(os.path.join(src, 'Kconfig')).touch()
        self.assertFalse(boards.output_is_new(boards_cfg, config_dir, src))
        Path(boards_cfg).touch()
        self.assertTrue(boards.output_is_new(boards_cfg, config_dir, src))

        # Touch a different Kconfig file
        time.sleep(delay)
        Path(os.path.join(src, 'Kconfig.something')).touch()
        self.assertFalse(boards.output_is_new(boards_cfg, config_dir, src))
        Path(boards_cfg).touch()
        self.assertTrue(boards.output_is_new(boards_cfg, config_dir, src))

        # Touch a MAINTAINERS file
        time.sleep(delay)
        Path(os.path.join(src, 'MAINTAINERS')).touch()
        self.assertFalse(boards.output_is_new(boards_cfg, config_dir, src))

        Path(boards_cfg).touch()
        self.assertTrue(boards.output_is_new(boards_cfg, config_dir, src))

        # Touch a defconfig file
        time.sleep(delay)
        Path(os.path.join(config_dir, 'board0_defconfig')).touch()
        self.assertFalse(boards.output_is_new(boards_cfg, config_dir, src))
        Path(boards_cfg).touch()
        self.assertTrue(boards.output_is_new(boards_cfg, config_dir, src))

        # Remove a board and check that the board.cfg file is now older
        Path(os.path.join(config_dir, 'board0_defconfig')).unlink()
        self.assertFalse(boards.output_is_new(boards_cfg, config_dir, src))

    def test_maintainers(self):
        """Test detecting boards without a MAINTAINERS entry"""
        src = self._git_dir
        main = os.path.join(src, 'boards', 'board0', 'MAINTAINERS')
        other = os.path.join(src, 'boards', 'board2', 'MAINTAINERS')
        kc_file = os.path.join(src, 'Kconfig')
        config_dir = os.path.join(src, 'configs')
        params_list, warnings = self._boards.build_board_list(config_dir, src)

        # There should be two boards no warnings
        self.assertEquals(2, len(params_list))
        self.assertFalse(warnings)

        # Set an invalid status line in the file
        orig_data = tools.read_file(main, binary=False)
        lines = ['S:      Other\n' if line.startswith('S:') else line
                  for line in orig_data.splitlines(keepends=True)]
        tools.write_file(main, ''.join(lines), binary=False)
        params_list, warnings = self._boards.build_board_list(config_dir, src)
        self.assertEquals(2, len(params_list))
        params = params_list[0]
        if params['target'] == 'board2':
            params = params_list[1]
        self.assertEquals('-', params['status'])
        self.assertEquals(["WARNING: Other: unknown status for 'board0'"],
                          warnings)

        # Remove the status line (S:) from a file
        lines = [line for line in orig_data.splitlines(keepends=True)
                 if not line.startswith('S:')]
        tools.write_file(main, ''.join(lines), binary=False)
        params_list, warnings = self._boards.build_board_list(config_dir, src)
        self.assertEquals(2, len(params_list))
        self.assertEquals(["WARNING: -: unknown status for 'board0'"], warnings)

        # Remove the configs/ line (F:) from a file - this is the last line
        data = ''.join(orig_data.splitlines(keepends=True)[:-1])
        tools.write_file(main, data, binary=False)
        params_list, warnings = self._boards.build_board_list(config_dir, src)
        self.assertEquals(2, len(params_list))
        self.assertEquals(["WARNING: no maintainers for 'board0'"], warnings)

        # Mark a board as orphaned - this should give a warning
        lines = ['S: Orphaned' if line.startswith('S') else line
                 for line in orig_data.splitlines(keepends=True)]
        tools.write_file(main, ''.join(lines), binary=False)
        params_list, warnings = self._boards.build_board_list(config_dir, src)
        self.assertEquals(2, len(params_list))
        self.assertEquals(["WARNING: no maintainers for 'board0'"], warnings)

        # Change the maintainer to '-' - this should give a warning
        lines = ['M: -' if line.startswith('M') else line
                 for line in orig_data.splitlines(keepends=True)]
        tools.write_file(main, ''.join(lines), binary=False)
        params_list, warnings = self._boards.build_board_list(config_dir, src)
        self.assertEquals(2, len(params_list))
        self.assertEquals(["WARNING: -: unknown status for 'board0'"], warnings)

        # Remove the maintainer line (M:) from a file
        lines = [line for line in orig_data.splitlines(keepends=True)
                 if not line.startswith('M:')]
        tools.write_file(main, ''.join(lines), binary=False)
        params_list, warnings = self._boards.build_board_list(config_dir, src)
        self.assertEquals(2, len(params_list))
        self.assertEquals(["WARNING: no maintainers for 'board0'"], warnings)

        # Move the contents of the second file into this one, removing the
        # second file, to check multiple records in a single file.
        both_data = orig_data + tools.read_file(other, binary=False)
        tools.write_file(main, both_data, binary=False)
        os.remove(other)
        params_list, warnings = self._boards.build_board_list(config_dir, src)
        self.assertEquals(2, len(params_list))
        self.assertFalse(warnings)

        # Add another record, this should be ignored with a warning
        extra = '\n\nAnother\nM: Fred\nF: configs/board9_defconfig\nS: other\n'
        tools.write_file(main, both_data + extra, binary=False)
        params_list, warnings = self._boards.build_board_list(config_dir, src)
        self.assertEquals(2, len(params_list))
        self.assertFalse(warnings)

        # Add another TARGET to the Kconfig
        tools.write_file(main, both_data, binary=False)
        orig_kc_data = tools.read_file(kc_file)
        extra = (b'''
if TARGET_BOARD2
config TARGET_OTHER
\tbool "other"
\tdefault y
endif
''')
        tools.write_file(kc_file, orig_kc_data + extra)
        params_list, warnings = self._boards.build_board_list(config_dir, src,
                                                              warn_targets=True)
        self.assertEquals(2, len(params_list))
        self.assertEquals(
            ['WARNING: board2_defconfig: Duplicate TARGET_xxx: board2 and other'],
             warnings)

        # Remove the TARGET_BOARD0 Kconfig option
        lines = [b'' if line == b'config TARGET_BOARD2\n' else line
                  for line in orig_kc_data.splitlines(keepends=True)]
        tools.write_file(kc_file, b''.join(lines))
        params_list, warnings = self._boards.build_board_list(config_dir, src,
                                                              warn_targets=True)
        self.assertEquals(2, len(params_list))
        self.assertEquals(
            ['WARNING: board2_defconfig: No TARGET_BOARD2 enabled'],
             warnings)
        tools.write_file(kc_file, orig_kc_data)

        # Replace the last F: line of board 2 with an N: line
        data = ''.join(both_data.splitlines(keepends=True)[:-1])
        tools.write_file(main, data + 'N: oa.*2\n', binary=False)
        params_list, warnings = self._boards.build_board_list(config_dir, src)
        self.assertEquals(2, len(params_list))
        self.assertFalse(warnings)

    def testRegenBoards(self):
        """Test that we can regenerate the boards.cfg file"""
        outfile = os.path.join(self._output_dir, 'test-boards.cfg')
        if os.path.exists(outfile):
            os.remove(outfile)
        with test_util.capture_sys_output() as (stdout, stderr):
            result = self._RunControl('-R', outfile, brds=None,
                                      get_builder=False)
        self.assertTrue(os.path.exists(outfile))

    def test_print_prefix(self):
        """Test that we can print the toolchain prefix"""
        with test_util.capture_sys_output() as (stdout, stderr):
            result = self._RunControl('-A', 'board0')
        self.assertEqual('arm-\n', stdout.getvalue())
        self.assertEqual('', stderr.getvalue())

    def test_exclude_one(self):
        """Test excluding a single board from an arch"""
        self._RunControl('arm', '-x', 'board1', '-o', self._output_dir)
        self.assertEqual(['board0'],
                         [b.target for b in self._boards.get_selected()])

    def test_exclude_arch(self):
        """Test excluding an arch"""
        self._RunControl('-x', 'arm', '-o', self._output_dir)
        self.assertEqual(['board2', 'board4'],
                         [b.target for b in self._boards.get_selected()])

    def test_exclude_comma(self):
        """Test excluding a comma-separated list of things"""
        self._RunControl('-x', 'arm,powerpc', '-o', self._output_dir)
        self.assertEqual(['board4'],
                         [b.target for b in self._boards.get_selected()])

    def test_exclude_list(self):
        """Test excluding a list of things"""
        self._RunControl('-x', 'board2', '-x' 'board4', '-o', self._output_dir)
        self.assertEqual(['board0', 'board1'],
                         [b.target for b in self._boards.get_selected()])

    def test_single_boards(self):
        """Test building single boards"""
        self._RunControl('--boards', 'board1', '-o', self._output_dir)
        self.assertEqual(1, self._builder.count)

        self._RunControl('--boards', 'board1', '--boards', 'board2',
                         '-o', self._output_dir)
        self.assertEqual(2, self._builder.count)

        self._RunControl('--boards', 'board1,board2', '--boards', 'board4',
                         '-o', self._output_dir)
        self.assertEqual(3, self._builder.count)

    def test_print_arch(self):
        """Test that we can print the board architecture"""
        with test_util.capture_sys_output() as (stdout, stderr):
            result = self._RunControl('--print-arch', 'board0')
        self.assertEqual('arm\n', stdout.getvalue())
        self.assertEqual('', stderr.getvalue())
