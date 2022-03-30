# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2014 Google, Inc
#

import os
import shutil
import sys
import tempfile
import unittest

import board
import bsettings
import cmdline
import command
import control
import gitutil
import terminal
import toolchain

settings_data = '''
# Buildman settings file

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

boards = [
    ['Active', 'arm', 'armv7', '', 'Tester', 'ARM Board 1', 'board0',  ''],
    ['Active', 'arm', 'armv7', '', 'Tester', 'ARM Board 2', 'board1', ''],
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
        self._git_dir = os.path.join(self._base_dir, 'src')
        self._buildman_pathname = sys.argv[0]
        self._buildman_dir = os.path.dirname(os.path.realpath(sys.argv[0]))
        command.test_result = self._HandleCommand
        self.setupToolchains()
        self._toolchains.Add('arm-gcc', test=False)
        self._toolchains.Add('powerpc-gcc', test=False)
        bsettings.Setup(None)
        bsettings.AddFile(settings_data)
        self._boards = board.Boards()
        for brd in boards:
            self._boards.AddBoard(board.Board(*brd))

        # Directories where the source been cloned
        self._clone_dirs = []
        self._commits = len(commit_shortlog.splitlines()) + 1
        self._total_builds = self._commits * len(boards)

        # Number of calls to make
        self._make_calls = 0

        # Map of [board, commit] to error messages
        self._error = {}

        self._test_branch = TEST_BRANCH

        # Avoid sending any output and clear all terminal output
        terminal.SetPrintTestMode()
        terminal.GetPrintTestLines()

    def tearDown(self):
        shutil.rmtree(self._base_dir)

    def setupToolchains(self):
        self._toolchains = toolchain.Toolchains()
        self._toolchains.Add('gcc', test=False)

    def _RunBuildman(self, *args):
        return command.RunPipe([[self._buildman_pathname] + list(args)],
                capture=True, capture_stderr=True)

    def _RunControl(self, *args, **kwargs):
        sys.argv = [sys.argv[0]] + list(args)
        options, args = cmdline.ParseArgs()
        result = control.DoBuildman(options, args, toolchains=self._toolchains,
                make_func=self._HandleMake, boards=self._boards,
                clean_dir=kwargs.get('clean_dir', True))
        self._builder = control.builder
        return result

    def testFullHelp(self):
        command.test_result = None
        result = self._RunBuildman('-H')
        help_file = os.path.join(self._buildman_dir, 'README')
        # Remove possible extraneous strings
        extra = '::::::::::::::\n' + help_file + '\n::::::::::::::\n'
        gothelp = result.stdout.replace(extra, '')
        self.assertEqual(len(gothelp), os.path.getsize(help_file))
        self.assertEqual(0, len(result.stderr))
        self.assertEqual(0, result.return_code)

    def testHelp(self):
        command.test_result = None
        result = self._RunBuildman('-h')
        help_file = os.path.join(self._buildman_dir, 'README')
        self.assertTrue(len(result.stdout) > 1000)
        self.assertEqual(0, len(result.stderr))
        self.assertEqual(0, result.return_code)

    def testGitSetup(self):
        """Test gitutils.Setup(), from outside the module itself"""
        command.test_result = command.CommandResult(return_code=1)
        gitutil.Setup()
        self.assertEqual(gitutil.use_no_decorate, False)

        command.test_result = command.CommandResult(return_code=0)
        gitutil.Setup()
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
        print 'git log', args
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
        print 'git config', args
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

        # Not handled, so abort
        print 'git', git_args, sub_cmd, args
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
                print 'invalid pipe', kwargs
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
            print 'unknown command', kwargs
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
            kwargs: Arguments to pass to command.RunPipe()
        """
        self._make_calls += 1
        if stage == 'mrproper':
            return command.CommandResult(return_code=0)
        elif stage == 'config':
            return command.CommandResult(return_code=0,
                    combined='Test configuration complete')
        elif stage == 'build':
            stderr = ''
            if type(commit) is not str:
                stderr = self._error.get((brd.target, commit.sequence))
            if stderr:
                return command.CommandResult(return_code=1, stderr=stderr)
            return command.CommandResult(return_code=0)

        # Not handled, so abort
        print 'make', stage
        sys.exit(1)

    # Example function to print output lines
    def print_lines(self, lines):
        print len(lines)
        for line in lines:
            print line
        #self.print_lines(terminal.GetPrintTestLines())

    def testNoBoards(self):
        """Test that buildman aborts when there are no boards"""
        self._boards = board.Boards()
        with self.assertRaises(SystemExit):
            self._RunControl()

    def testCurrentSource(self):
        """Very simple test to invoke buildman on the current source"""
        self.setupToolchains();
        self._RunControl()
        lines = terminal.GetPrintTestLines()
        self.assertIn('Building current source for %d boards' % len(boards),
                      lines[0].text)

    def testBadBranch(self):
        """Test that we can detect an invalid branch"""
        with self.assertRaises(ValueError):
            self._RunControl('-b', 'badbranch')

    def testBadToolchain(self):
        """Test that missing toolchains are detected"""
        self.setupToolchains();
        ret_code = self._RunControl('-b', TEST_BRANCH)
        lines = terminal.GetPrintTestLines()

        # Buildman always builds the upstream commit as well
        self.assertIn('Building %d commits for %d boards' %
                (self._commits, len(boards)), lines[0].text)
        self.assertEqual(self._builder.count, self._total_builds)

        # Only sandbox should succeed, the others don't have toolchains
        self.assertEqual(self._builder.fail,
                         self._total_builds - self._commits)
        self.assertEqual(ret_code, 128)

        for commit in range(self._commits):
            for board in self._boards.GetList():
                if board.arch != 'sandbox':
                  errfile = self._builder.GetErrFile(commit, board.target)
                  fd = open(errfile)
                  self.assertEqual(fd.readlines(),
                          ['No tool chain for %s\n' % board.arch])
                  fd.close()

    def testBranch(self):
        """Test building a branch with all toolchains present"""
        self._RunControl('-b', TEST_BRANCH)
        self.assertEqual(self._builder.count, self._total_builds)
        self.assertEqual(self._builder.fail, 0)

    def testCount(self):
        """Test building a specific number of commitst"""
        self._RunControl('-b', TEST_BRANCH, '-c2')
        self.assertEqual(self._builder.count, 2 * len(boards))
        self.assertEqual(self._builder.fail, 0)
        # Each board has a mrproper, config, and then one make per commit
        self.assertEqual(self._make_calls, len(boards) * (2 + 2))

    def testIncremental(self):
        """Test building a branch twice - the second time should do nothing"""
        self._RunControl('-b', TEST_BRANCH)

        # Each board has a mrproper, config, and then one make per commit
        self.assertEqual(self._make_calls, len(boards) * (self._commits + 2))
        self._make_calls = 0
        self._RunControl('-b', TEST_BRANCH, clean_dir=False)
        self.assertEqual(self._make_calls, 0)
        self.assertEqual(self._builder.count, self._total_builds)
        self.assertEqual(self._builder.fail, 0)

    def testForceBuild(self):
        """The -f flag should force a rebuild"""
        self._RunControl('-b', TEST_BRANCH)
        self._make_calls = 0
        self._RunControl('-b', TEST_BRANCH, '-f', clean_dir=False)
        # Each board has a mrproper, config, and then one make per commit
        self.assertEqual(self._make_calls, len(boards) * (self._commits + 2))

    def testForceReconfigure(self):
        """The -f flag should force a rebuild"""
        self._RunControl('-b', TEST_BRANCH, '-C')
        # Each commit has a mrproper, config and make
        self.assertEqual(self._make_calls, len(boards) * self._commits * 3)

    def testErrors(self):
        """Test handling of build errors"""
        self._error['board2', 1] = 'fred\n'
        self._RunControl('-b', TEST_BRANCH)
        self.assertEqual(self._builder.count, self._total_builds)
        self.assertEqual(self._builder.fail, 1)

        # Remove the error. This should have no effect since the commit will
        # not be rebuilt
        del self._error['board2', 1]
        self._make_calls = 0
        self._RunControl('-b', TEST_BRANCH, clean_dir=False)
        self.assertEqual(self._builder.count, self._total_builds)
        self.assertEqual(self._make_calls, 0)
        self.assertEqual(self._builder.fail, 1)

        # Now use the -F flag to force rebuild of the bad commit
        self._RunControl('-b', TEST_BRANCH, '-F', clean_dir=False)
        self.assertEqual(self._builder.count, self._total_builds)
        self.assertEqual(self._builder.fail, 0)
        self.assertEqual(self._make_calls, 3)

    def testBranchWithSlash(self):
        """Test building a branch with a '/' in the name"""
        self._test_branch = '/__dev/__testbranch'
        self._RunControl('-b', self._test_branch, clean_dir=False)
        self.assertEqual(self._builder.count, self._total_builds)
        self.assertEqual(self._builder.fail, 0)

    def testBadOutputDir(self):
        """Test building with an output dir the same as out current dir"""
        self._test_branch = '/__dev/__testbranch'
        with self.assertRaises(SystemExit):
            self._RunControl('-b', self._test_branch, '-o', os.getcwd())
        with self.assertRaises(SystemExit):
            self._RunControl('-b', self._test_branch, '-o',
                             os.path.join(os.getcwd(), 'test'))
