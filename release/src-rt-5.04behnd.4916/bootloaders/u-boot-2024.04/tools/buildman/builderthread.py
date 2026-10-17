# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2014 Google, Inc
#

"""Implementation the bulider threads

This module provides the BuilderThread class, which handles calling the builder
based on the jobs provided.
"""

import errno
import glob
import io
import os
import shutil
import sys
import threading

from buildman import cfgutil
from patman import gitutil
from u_boot_pylib import command

RETURN_CODE_RETRY = -1
BASE_ELF_FILENAMES = ['u-boot', 'spl/u-boot-spl', 'tpl/u-boot-tpl']

# Common extensions for images
COMMON_EXTS = ['.bin', '.rom', '.itb', '.img']

def mkdir(dirname, parents=False):
    """Make a directory if it doesn't already exist.

    Args:
        dirname (str): Directory to create
        parents (bool): True to also make parent directories

    Raises:
        OSError: File already exists
    """
    try:
        if parents:
            os.makedirs(dirname)
        else:
            os.mkdir(dirname)
    except OSError as err:
        if err.errno == errno.EEXIST:
            if os.path.realpath('.') == os.path.realpath(dirname):
                print(f"Cannot create the current working directory '{dirname}'!")
                sys.exit(1)
        else:
            raise


def _remove_old_outputs(out_dir):
    """Remove any old output-target files

    Args:
        out_dir (str): Output directory for the build

    Since we use a build directory that was previously used by another
    board, it may have produced an SPL image. If we don't remove it (i.e.
    see do_config and self.mrproper below) then it will appear to be the
    output of this build, even if it does not produce SPL images.
    """
    for elf in BASE_ELF_FILENAMES:
        fname = os.path.join(out_dir, elf)
        if os.path.exists(fname):
            os.remove(fname)


def copy_files(out_dir, build_dir, dirname, patterns):
    """Copy files from the build directory to the output.

    Args:
        out_dir (str): Path to output directory containing the files
        build_dir (str): Place to copy the files
        dirname (str): Source directory, '' for normal U-Boot, 'spl' for SPL
        patterns (list of str): A list of filenames to copy, each relative
           to the build directory
    """
    for pattern in patterns:
        file_list = glob.glob(os.path.join(out_dir, dirname, pattern))
        for fname in file_list:
            target = os.path.basename(fname)
            if dirname:
                base, ext = os.path.splitext(target)
                if ext:
                    target = f'{base}-{dirname}{ext}'
            shutil.copy(fname, os.path.join(build_dir, target))


# pylint: disable=R0903
class BuilderJob:
    """Holds information about a job to be performed by a thread

    Members:
        brd: Board object to build
        commits: List of Commit objects to build
        keep_outputs: True to save build output files
        step: 1 to process every commit, n to process every nth commit
        work_in_output: Use the output directory as the work directory and
            don't write to a separate output directory.
    """
    def __init__(self):
        self.brd = None
        self.commits = []
        self.keep_outputs = False
        self.step = 1
        self.work_in_output = False


class ResultThread(threading.Thread):
    """This thread processes results from builder threads.

    It simply passes the results on to the builder. There is only one
    result thread, and this helps to serialise the build output.
    """
    def __init__(self, builder):
        """Set up a new result thread

        Args:
            builder: Builder which will be sent each result
        """
        threading.Thread.__init__(self)
        self.builder = builder

    def run(self):
        """Called to start up the result thread.

        We collect the next result job and pass it on to the build.
        """
        while True:
            result = self.builder.out_queue.get()
            self.builder.process_result(result)
            self.builder.out_queue.task_done()


class BuilderThread(threading.Thread):
    """This thread builds U-Boot for a particular board.

    An input queue provides each new job. We run 'make' to build U-Boot
    and then pass the results on to the output queue.

    Members:
        builder: The builder which contains information we might need
        thread_num: Our thread number (0-n-1), used to decide on a
            temporary directory. If this is -1 then there are no threads
            and we are the (only) main process
        mrproper: Use 'make mrproper' before each reconfigure
        per_board_out_dir: True to build in a separate persistent directory per
            board rather than a thread-specific directory
        test_exception: Used for testing; True to raise an exception instead of
            reporting the build result
    """
    def __init__(self, builder, thread_num, mrproper, per_board_out_dir,
                 test_exception=False):
        """Set up a new builder thread"""
        threading.Thread.__init__(self)
        self.builder = builder
        self.thread_num = thread_num
        self.mrproper = mrproper
        self.per_board_out_dir = per_board_out_dir
        self.test_exception = test_exception
        self.toolchain = None

    def make(self, commit, brd, stage, cwd, *args, **kwargs):
        """Run 'make' on a particular commit and board.

        The source code will already be checked out, so the 'commit'
        argument is only for information.

        Args:
            commit (Commit): Commit that is being built
            brd (Board): Board that is being built
            stage (str): Stage of the build. Valid stages are:
                        mrproper - can be called to clean source
                        config - called to configure for a board
                        build - the main make invocation - it does the build
            cwd (str): Working directory to set, or None to leave it alone
            *args (list of str): Arguments to pass to 'make'
            **kwargs (dict): A list of keyword arguments to pass to
                command.run_pipe()

        Returns:
            CommandResult object
        """
        return self.builder.do_make(commit, brd, stage, cwd, *args,
                **kwargs)

    def _build_args(self, brd, out_dir, out_rel_dir, work_dir, commit_upto):
        """Set up arguments to the args list based on the settings

        Args:
            brd (Board): Board to create arguments for
            out_dir (str): Path to output directory containing the files
            out_rel_dir (str): Output directory relative to the current dir
            work_dir (str): Directory to which the source will be checked out
            commit_upto (int): Commit number to build (0...n-1)

        Returns:
            tuple:
                list of str: Arguments to pass to make
                str: Current working directory, or None if no commit
                str: Source directory (typically the work directory)
        """
        args = []
        cwd = work_dir
        src_dir = os.path.realpath(work_dir)
        if not self.builder.in_tree:
            if commit_upto is None:
                # In this case we are building in the original source directory
                # (i.e. the current directory where buildman is invoked. The
                # output directory is set to this thread's selected work
                # directory.
                #
                # Symlinks can confuse U-Boot's Makefile since we may use '..'
                # in our path, so remove them.
                real_dir = os.path.realpath(out_dir)
                args.append(f'O={real_dir}')
                cwd = None
                src_dir = os.getcwd()
            else:
                args.append(f'O={out_rel_dir}')
        if self.builder.verbose_build:
            args.append('V=1')
        else:
            args.append('-s')
        if self.builder.num_jobs is not None:
            args.extend(['-j', str(self.builder.num_jobs)])
        if self.builder.warnings_as_errors:
            args.append('KCFLAGS=-Werror')
            args.append('HOSTCFLAGS=-Werror')
        if self.builder.allow_missing:
            args.append('BINMAN_ALLOW_MISSING=1')
        if self.builder.no_lto:
            args.append('NO_LTO=1')
        if self.builder.reproducible_builds:
            args.append('SOURCE_DATE_EPOCH=0')
        args.extend(self.builder.toolchains.GetMakeArguments(brd))
        args.extend(self.toolchain.MakeArgs())
        return args, cwd, src_dir

    def _reconfigure(self, commit, brd, cwd, args, env, config_args, config_out,
                     cmd_list):
        """Reconfigure the build

        Args:
            commit (Commit): Commit only being built
            brd (Board): Board being built
            cwd (str): Current working directory
            args (list of str): Arguments to pass to make
            env (dict): Environment strings
            config_args (list of str): defconfig arg for this board
            cmd_list (list of str): List to add the commands to, for logging

        Returns:
            CommandResult object
        """
        if self.mrproper:
            result = self.make(commit, brd, 'mrproper', cwd, 'mrproper', *args,
                               env=env)
            config_out.write(result.combined)
            cmd_list.append([self.builder.gnu_make, 'mrproper', *args])
        result = self.make(commit, brd, 'config', cwd, *(args + config_args),
                           env=env)
        cmd_list.append([self.builder.gnu_make] + args + config_args)
        config_out.write(result.combined)
        return result

    def _build(self, commit, brd, cwd, args, env, cmd_list, config_only):
        """Perform the build

        Args:
            commit (Commit): Commit only being built
            brd (Board): Board being built
            cwd (str): Current working directory
            args (list of str): Arguments to pass to make
            env (dict): Environment strings
            cmd_list (list of str): List to add the commands to, for logging
            config_only (bool): True if this is a config-only build (using the
                'make cfg' target)

        Returns:
            CommandResult object
        """
        if config_only:
            args.append('cfg')
        result = self.make(commit, brd, 'build', cwd, *args, env=env)
        cmd_list.append([self.builder.gnu_make] + args)
        if (result.return_code == 2 and
            ('Some images are invalid' in result.stderr)):
            # This is handled later by the check for output in stderr
            result.return_code = 0
        return result

    def _read_done_file(self, commit_upto, brd, force_build,
                        force_build_failures):
        """Check the 'done' file and see if this commit should be built

        Args:
            commit (Commit): Commit only being built
            brd (Board): Board being built
            force_build (bool): Force a build even if one was previously done
            force_build_failures (bool): Force a bulid if the previous result
                showed failure

        Returns:
            tuple:
                bool: True if build should be built
                CommandResult: if there was a previous run:
                    - already_done set to True
                    - return_code set to return code
                    - result.stderr set to 'bad' if stderr output was recorded
        """
        result = command.CommandResult()
        done_file = self.builder.get_done_file(commit_upto, brd.target)
        result.already_done = os.path.exists(done_file)
        will_build = (force_build or force_build_failures or
            not result.already_done)
        if result.already_done:
            with open(done_file, 'r', encoding='utf-8') as outf:
                try:
                    result.return_code = int(outf.readline())
                except ValueError:
                    # The file may be empty due to running out of disk space.
                    # Try a rebuild
                    result.return_code = RETURN_CODE_RETRY

            # Check the signal that the build needs to be retried
            if result.return_code == RETURN_CODE_RETRY:
                will_build = True
            elif will_build:
                err_file = self.builder.get_err_file(commit_upto, brd.target)
                if os.path.exists(err_file) and os.stat(err_file).st_size:
                    result.stderr = 'bad'
                elif not force_build:
                    # The build passed, so no need to build it again
                    will_build = False
        return will_build, result

    def _decide_dirs(self, brd, work_dir, work_in_output):
        """Decide the output directory to use

        Args:
            work_dir (str): Directory to which the source will be checked out
            work_in_output (bool): Use the output directory as the work
                directory and don't write to a separate output directory.

        Returns:
            tuple:
                out_dir (str): Output directory for the build
                out_rel_dir (str): Output directory relatie to the current dir
        """
        if work_in_output or self.builder.in_tree:
            out_rel_dir = None
            out_dir = work_dir
        else:
            if self.per_board_out_dir:
                out_rel_dir = os.path.join('..', brd.target)
            else:
                out_rel_dir = 'build'
            out_dir = os.path.join(work_dir, out_rel_dir)
        return out_dir, out_rel_dir

    def _checkout(self, commit_upto, work_dir):
        """Checkout the right commit

        Args:
            commit_upto (int): Commit number to build (0...n-1)
            work_dir (str): Directory to which the source will be checked out

        Returns:
            Commit: Commit being built, or 'current' for current source
        """
        if self.builder.commits:
            commit = self.builder.commits[commit_upto]
            if self.builder.checkout:
                git_dir = os.path.join(work_dir, '.git')
                gitutil.checkout(commit.hash, git_dir, work_dir, force=True)
        else:
            commit = 'current'
        return commit

    def _config_and_build(self, commit_upto, brd, work_dir, do_config,
                          config_only, adjust_cfg, commit, out_dir, out_rel_dir,
                          result):
        """Do the build, configuring first if necessary

        Args:
            commit_upto (int): Commit number to build (0...n-1)
            brd (Board): Board to create arguments for
            work_dir (str): Directory to which the source will be checked out
            do_config (bool): True to run a make <board>_defconfig on the source
            config_only (bool): Only configure the source, do not build it
            adjust_cfg (list of str): See the cfgutil module and run_commit()
            commit (Commit): Commit only being built
            out_dir (str): Output directory for the build
            out_rel_dir (str): Output directory relatie to the current dir
            result (CommandResult): Previous result

        Returns:
            tuple:
                result (CommandResult): Result of the build
                do_config (bool): indicates whether 'make config' is needed on
                    the next incremental build
        """
        # Set up the environment and command line
        env = self.toolchain.MakeEnvironment(self.builder.full_path)
        mkdir(out_dir)

        args, cwd, src_dir = self._build_args(brd, out_dir, out_rel_dir,
                                              work_dir, commit_upto)
        config_args = [f'{brd.target}_defconfig']
        config_out = io.StringIO()

        _remove_old_outputs(out_dir)

        # If we need to reconfigure, do that now
        cfg_file = os.path.join(out_dir, '.config')
        cmd_list = []
        if do_config or adjust_cfg:
            result = self._reconfigure(
                commit, brd, cwd, args, env, config_args, config_out, cmd_list)
            do_config = False   # No need to configure next time
            if adjust_cfg:
                cfgutil.adjust_cfg_file(cfg_file, adjust_cfg)

        # Now do the build, if everything looks OK
        if result.return_code == 0:
            if adjust_cfg:
                oldc_args = list(args) + ['oldconfig']
                oldc_result = self.make(commit, brd, 'oldconfig', cwd,
                                        *oldc_args, env=env)
                if oldc_result.return_code:
                    return oldc_result
            result = self._build(commit, brd, cwd, args, env, cmd_list,
                                 config_only)
            if adjust_cfg:
                errs = cfgutil.check_cfg_file(cfg_file, adjust_cfg)
                if errs:
                    result.stderr += errs
                    result.return_code = 1
        result.stderr = result.stderr.replace(src_dir + '/', '')
        if self.builder.verbose_build:
            result.stdout = config_out.getvalue() + result.stdout
        result.cmd_list = cmd_list
        return result, do_config

    def run_commit(self, commit_upto, brd, work_dir, do_config, config_only,
                  force_build, force_build_failures, work_in_output,
                  adjust_cfg):
        """Build a particular commit.

        If the build is already done, and we are not forcing a build, we skip
        the build and just return the previously-saved results.

        Args:
            commit_upto (int): Commit number to build (0...n-1)
            brd (Board): Board to build
            work_dir (str): Directory to which the source will be checked out
            do_config (bool): True to run a make <board>_defconfig on the source
            config_only (bool): Only configure the source, do not build it
            force_build (bool): Force a build even if one was previously done
            force_build_failures (bool): Force a bulid if the previous result
                showed failure
            work_in_output (bool) : Use the output directory as the work
                directory and don't write to a separate output directory.
            adjust_cfg (list of str): List of changes to make to .config file
                before building. Each is one of (where C is either CONFIG_xxx
                or just xxx):
                     C to enable C
                     ~C to disable C
                     C=val to set the value of C (val must have quotes if C is
                         a string Kconfig

        Returns:
            tuple containing:
                - CommandResult object containing the results of the build
                - boolean indicating whether 'make config' is still needed
        """
        # Create a default result - it will be overwritte by the call to
        # self.make() below, in the event that we do a build.
        out_dir, out_rel_dir = self._decide_dirs(brd, work_dir, work_in_output)

        # Check if the job was already completed last time
        will_build, result = self._read_done_file(commit_upto, brd, force_build,
                                                  force_build_failures)

        if will_build:
            # We are going to have to build it. First, get a toolchain
            if not self.toolchain:
                try:
                    self.toolchain = self.builder.toolchains.Select(brd.arch)
                except ValueError as err:
                    result.return_code = 10
                    result.stdout = ''
                    result.stderr = f'Tool chain error for {brd.arch}: {str(err)}'

            if self.toolchain:
                commit = self._checkout(commit_upto, work_dir)
                result, do_config = self._config_and_build(
                    commit_upto, brd, work_dir, do_config, config_only,
                    adjust_cfg, commit, out_dir, out_rel_dir, result)
            result.already_done = False

        result.toolchain = self.toolchain
        result.brd = brd
        result.commit_upto = commit_upto
        result.out_dir = out_dir
        return result, do_config

    def _write_result(self, result, keep_outputs, work_in_output):
        """Write a built result to the output directory.

        Args:
            result (CommandResult): result to write
            keep_outputs (bool): True to store the output binaries, False
                to delete them
            work_in_output (bool): Use the output directory as the work
                directory and don't write to a separate output directory.
        """
        # If we think this might have been aborted with Ctrl-C, record the
        # failure but not that we are 'done' with this board. A retry may fix
        # it.
        maybe_aborted = result.stderr and 'No child processes' in result.stderr

        if result.return_code >= 0 and result.already_done:
            return

        # Write the output and stderr
        output_dir = self.builder.get_output_dir(result.commit_upto)
        mkdir(output_dir)
        build_dir = self.builder.get_build_dir(result.commit_upto,
                result.brd.target)
        mkdir(build_dir)

        outfile = os.path.join(build_dir, 'log')
        with open(outfile, 'w', encoding='utf-8') as outf:
            if result.stdout:
                outf.write(result.stdout)

        errfile = self.builder.get_err_file(result.commit_upto,
                result.brd.target)
        if result.stderr:
            with open(errfile, 'w', encoding='utf-8') as outf:
                outf.write(result.stderr)
        elif os.path.exists(errfile):
            os.remove(errfile)

        # Fatal error
        if result.return_code < 0:
            return

        if result.toolchain:
            # Write the build result and toolchain information.
            done_file = self.builder.get_done_file(result.commit_upto,
                    result.brd.target)
            with open(done_file, 'w', encoding='utf-8') as outf:
                if maybe_aborted:
                    # Special code to indicate we need to retry
                    outf.write(f'{RETURN_CODE_RETRY}')
                else:
                    outf.write(f'{result.return_code}')
            with open(os.path.join(build_dir, 'toolchain'), 'w',
                      encoding='utf-8') as outf:
                print('gcc', result.toolchain.gcc, file=outf)
                print('path', result.toolchain.path, file=outf)
                print('cross', result.toolchain.cross, file=outf)
                print('arch', result.toolchain.arch, file=outf)
                outf.write(f'{result.return_code}')

            # Write out the image and function size information and an objdump
            env = result.toolchain.MakeEnvironment(self.builder.full_path)
            with open(os.path.join(build_dir, 'out-env'), 'wb') as outf:
                for var in sorted(env.keys()):
                    outf.write(b'%s="%s"' % (var, env[var]))

            with open(os.path.join(build_dir, 'out-cmd'), 'w',
                      encoding='utf-8') as outf:
                for cmd in result.cmd_list:
                    print(' '.join(cmd), file=outf)

            lines = []
            for fname in BASE_ELF_FILENAMES:
                cmd = [f'{self.toolchain.cross}nm', '--size-sort', fname]
                nm_result = command.run_pipe([cmd], capture=True,
                        capture_stderr=True, cwd=result.out_dir,
                        raise_on_error=False, env=env)
                if nm_result.stdout:
                    nm_fname = self.builder.get_func_sizes_file(
                        result.commit_upto, result.brd.target, fname)
                    with open(nm_fname, 'w', encoding='utf-8') as outf:
                        print(nm_result.stdout, end=' ', file=outf)

                cmd = [f'{self.toolchain.cross}objdump', '-h', fname]
                dump_result = command.run_pipe([cmd], capture=True,
                        capture_stderr=True, cwd=result.out_dir,
                        raise_on_error=False, env=env)
                rodata_size = ''
                if dump_result.stdout:
                    objdump = self.builder.get_objdump_file(result.commit_upto,
                                    result.brd.target, fname)
                    with open(objdump, 'w', encoding='utf-8') as outf:
                        print(dump_result.stdout, end=' ', file=outf)
                    for line in dump_result.stdout.splitlines():
                        fields = line.split()
                        if len(fields) > 5 and fields[1] == '.rodata':
                            rodata_size = fields[2]

                cmd = [f'{self.toolchain.cross}size', fname]
                size_result = command.run_pipe([cmd], capture=True,
                        capture_stderr=True, cwd=result.out_dir,
                        raise_on_error=False, env=env)
                if size_result.stdout:
                    lines.append(size_result.stdout.splitlines()[1] + ' ' +
                                 rodata_size)

            # Extract the environment from U-Boot and dump it out
            cmd = [f'{self.toolchain.cross}objcopy', '-O', 'binary',
                   '-j', '.rodata.default_environment',
                   'env/built-in.o', 'uboot.env']
            command.run_pipe([cmd], capture=True,
                            capture_stderr=True, cwd=result.out_dir,
                            raise_on_error=False, env=env)
            if not work_in_output:
                copy_files(result.out_dir, build_dir, '', ['uboot.env'])

            # Write out the image sizes file. This is similar to the output
            # of binutil's 'size' utility, but it omits the header line and
            # adds an additional hex value at the end of each line for the
            # rodata size
            if lines:
                sizes = self.builder.get_sizes_file(result.commit_upto,
                                result.brd.target)
                with open(sizes, 'w', encoding='utf-8') as outf:
                    print('\n'.join(lines), file=outf)

        if not work_in_output:
            # Write out the configuration files, with a special case for SPL
            for dirname in ['', 'spl', 'tpl']:
                copy_files(
                    result.out_dir, build_dir, dirname,
                    ['u-boot.cfg', 'spl/u-boot-spl.cfg', 'tpl/u-boot-tpl.cfg',
                     '.config', 'include/autoconf.mk',
                     'include/generated/autoconf.h'])

            # Now write the actual build output
            if keep_outputs:
                to_copy = ['u-boot*', '*.map', 'MLO', 'SPL',
                           'include/autoconf.mk', 'spl/u-boot-spl*',
                           'tpl/u-boot-tpl*', 'vpl/u-boot-vpl*']
                to_copy += [f'*{ext}' for ext in COMMON_EXTS]
                copy_files(result.out_dir, build_dir, '', to_copy)

    def _send_result(self, result):
        """Send a result to the builder for processing

        Args:
            result (CommandResult): results of the build

        Raises:
            ValueError: self.test_exception is true (for testing)
        """
        if self.test_exception:
            raise ValueError('test exception')
        if self.thread_num != -1:
            self.builder.out_queue.put(result)
        else:
            self.builder.process_result(result)

    def run_job(self, job):
        """Run a single job

        A job consists of a building a list of commits for a particular board.

        Args:
            job (Job): Job to build

        Raises:
            ValueError: Thread was interrupted
        """
        brd = job.brd
        work_dir = self.builder.get_thread_dir(self.thread_num)
        self.toolchain = None
        if job.commits:
            # Run 'make board_defconfig' on the first commit
            do_config = True
            commit_upto  = 0
            force_build = False
            for commit_upto in range(0, len(job.commits), job.step):
                result, request_config = self.run_commit(commit_upto, brd,
                        work_dir, do_config, self.builder.config_only,
                        force_build or self.builder.force_build,
                        self.builder.force_build_failures,
                        job.work_in_output, job.adjust_cfg)
                failed = result.return_code or result.stderr
                did_config = do_config
                if failed and not do_config:
                    # If our incremental build failed, try building again
                    # with a reconfig.
                    if self.builder.force_config_on_failure:
                        result, request_config = self.run_commit(commit_upto,
                            brd, work_dir, True, False, True, False,
                            job.work_in_output, job.adjust_cfg)
                        did_config = True
                if not self.builder.force_reconfig:
                    do_config = request_config

                # If we built that commit, then config is done. But if we got
                # an warning, reconfig next time to force it to build the same
                # files that created warnings this time. Otherwise an
                # incremental build may not build the same file, and we will
                # think that the warning has gone away.
                # We could avoid this by using -Werror everywhere...
                # For errors, the problem doesn't happen, since presumably
                # the build stopped and didn't generate output, so will retry
                # that file next time. So we could detect warnings and deal
                # with them specially here. For now, we just reconfigure if
                # anything goes work.
                # Of course this is substantially slower if there are build
                # errors/warnings (e.g. 2-3x slower even if only 10% of builds
                # have problems).
                if (failed and not result.already_done and not did_config and
                        self.builder.force_config_on_failure):
                    # If this build failed, try the next one with a
                    # reconfigure.
                    # Sometimes if the board_config.h file changes it can mess
                    # with dependencies, and we get:
                    # make: *** No rule to make target `include/autoconf.mk',
                    #     needed by `depend'.
                    do_config = True
                    force_build = True
                else:
                    force_build = False
                    if self.builder.force_config_on_failure:
                        if failed:
                            do_config = True
                    result.commit_upto = commit_upto
                    if result.return_code < 0:
                        raise ValueError('Interrupt')

                # We have the build results, so output the result
                self._write_result(result, job.keep_outputs, job.work_in_output)
                self._send_result(result)
        else:
            # Just build the currently checked-out build
            result, request_config = self.run_commit(None, brd, work_dir, True,
                        self.builder.config_only, True,
                        self.builder.force_build_failures, job.work_in_output,
                        job.adjust_cfg)
            result.commit_upto = 0
            self._write_result(result, job.keep_outputs, job.work_in_output)
            self._send_result(result)

    def run(self):
        """Our thread's run function

        This thread picks a job from the queue, runs it, and then goes to the
        next job.
        """
        while True:
            job = self.builder.queue.get()
            try:
                self.run_job(job)
            except Exception as exc:
                print('Thread exception (use -T0 to run without threads):',
                      exc)
                self.builder.thread_exceptions.append(exc)
            self.builder.queue.task_done()
