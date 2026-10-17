# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2015 Stephen Warren
# Copyright (c) 2015-2016, NVIDIA CORPORATION. All rights reserved.

# Common logic to interact with U-Boot via the console. This class provides
# the interface that tests use to execute U-Boot shell commands and wait for
# their results. Sub-classes exist to perform board-type-specific setup
# operations, such as spawning a sub-process for Sandbox, or attaching to the
# serial console of real hardware.

import multiplexed_log
import os
import pytest
import re
import sys
import u_boot_spawn

# Regexes for text we expect U-Boot to send to the console.
pattern_u_boot_spl_signon = re.compile('(U-Boot SPL \\d{4}\\.\\d{2}[^\r\n]*\\))')
pattern_u_boot_spl2_signon = re.compile('(U-Boot SPL \\d{4}\\.\\d{2}[^\r\n]*\\))')
pattern_u_boot_main_signon = re.compile('(U-Boot \\d{4}\\.\\d{2}[^\r\n]*\\))')
pattern_stop_autoboot_prompt = re.compile('Hit any key to stop autoboot: ')
pattern_unknown_command = re.compile('Unknown command \'.*\' - try \'help\'')
pattern_error_notification = re.compile('## Error: ')
pattern_error_please_reset = re.compile('### ERROR ### Please RESET the board ###')

PAT_ID = 0
PAT_RE = 1

bad_pattern_defs = (
    ('spl_signon', pattern_u_boot_spl_signon),
    ('spl2_signon', pattern_u_boot_spl2_signon),
    ('main_signon', pattern_u_boot_main_signon),
    ('stop_autoboot_prompt', pattern_stop_autoboot_prompt),
    ('unknown_command', pattern_unknown_command),
    ('error_notification', pattern_error_notification),
    ('error_please_reset', pattern_error_please_reset),
)

class ConsoleDisableCheck(object):
    """Context manager (for Python's with statement) that temporarily disables
    the specified console output error check. This is useful when deliberately
    executing a command that is known to trigger one of the error checks, in
    order to test that the error condition is actually raised. This class is
    used internally by ConsoleBase::disable_check(); it is not intended for
    direct usage."""

    def __init__(self, console, check_type):
        self.console = console
        self.check_type = check_type

    def __enter__(self):
        self.console.disable_check_count[self.check_type] += 1
        self.console.eval_bad_patterns()

    def __exit__(self, extype, value, traceback):
        self.console.disable_check_count[self.check_type] -= 1
        self.console.eval_bad_patterns()

class ConsoleSetupTimeout(object):
    """Context manager (for Python's with statement) that temporarily sets up
    timeout for specific command. This is useful when execution time is greater
    then default 30s."""

    def __init__(self, console, timeout):
        self.p = console.p
        self.orig_timeout = self.p.timeout
        self.p.timeout = timeout

    def __enter__(self):
        return self

    def __exit__(self, extype, value, traceback):
        self.p.timeout = self.orig_timeout

class ConsoleBase(object):
    """The interface through which test functions interact with the U-Boot
    console. This primarily involves executing shell commands, capturing their
    results, and checking for common error conditions. Some common utilities
    are also provided too."""

    def __init__(self, log, config, max_fifo_fill):
        """Initialize a U-Boot console connection.

        Can only usefully be called by sub-classes.

        Args:
            log: A mulptiplex_log.Logfile object, to which the U-Boot output
                will be logged.
            config: A configuration data structure, as built by conftest.py.
            max_fifo_fill: The maximum number of characters to send to U-Boot
                command-line before waiting for U-Boot to echo the characters
                back. For UART-based HW without HW flow control, this value
                should be set less than the UART RX FIFO size to avoid
                overflow, assuming that U-Boot can't keep up with full-rate
                traffic at the baud rate.

        Returns:
            Nothing.
        """

        self.log = log
        self.config = config
        self.max_fifo_fill = max_fifo_fill

        self.logstream = self.log.get_stream('console', sys.stdout)

        # Array slice removes leading/trailing quotes
        self.prompt = self.config.buildconfig['config_sys_prompt'][1:-1]
        self.prompt_compiled = re.compile('^' + re.escape(self.prompt), re.MULTILINE)
        self.p = None
        self.disable_check_count = {pat[PAT_ID]: 0 for pat in bad_pattern_defs}
        self.eval_bad_patterns()

        self.at_prompt = False
        self.at_prompt_logevt = None

    def get_spawn(self):
        # This is not called, ssubclass must define this.
        # Return a value to avoid:
        #   u_boot_console_base.py:348:12: E1128: Assigning result of a function
        #   call, where the function returns None (assignment-from-none)
        return u_boot_spawn.Spawn([])


    def eval_bad_patterns(self):
        self.bad_patterns = [pat[PAT_RE] for pat in bad_pattern_defs \
            if self.disable_check_count[pat[PAT_ID]] == 0]
        self.bad_pattern_ids = [pat[PAT_ID] for pat in bad_pattern_defs \
            if self.disable_check_count[pat[PAT_ID]] == 0]

    def close(self):
        """Terminate the connection to the U-Boot console.

        This function is only useful once all interaction with U-Boot is
        complete. Once this function is called, data cannot be sent to or
        received from U-Boot.

        Args:
            None.

        Returns:
            Nothing.
        """

        if self.p:
            self.p.close()
        self.logstream.close()

    def wait_for_boot_prompt(self, loop_num = 1):
        """Wait for the boot up until command prompt. This is for internal use only.
        """
        try:
            bcfg = self.config.buildconfig
            config_spl = bcfg.get('config_spl', 'n') == 'y'
            config_spl_serial = bcfg.get('config_spl_serial', 'n') == 'y'
            env_spl_skipped = self.config.env.get('env__spl_skipped', False)
            env_spl2_skipped = self.config.env.get('env__spl2_skipped', True)

            while loop_num > 0:
                loop_num -= 1
                if config_spl and config_spl_serial and not env_spl_skipped:
                    m = self.p.expect([pattern_u_boot_spl_signon] +
                                      self.bad_patterns)
                    if m != 0:
                        raise Exception('Bad pattern found on SPL console: ' +
                                        self.bad_pattern_ids[m - 1])
                if not env_spl2_skipped:
                    m = self.p.expect([pattern_u_boot_spl2_signon] +
                                      self.bad_patterns)
                    if m != 0:
                        raise Exception('Bad pattern found on SPL2 console: ' +
                                        self.bad_pattern_ids[m - 1])
                m = self.p.expect([pattern_u_boot_main_signon] + self.bad_patterns)
                if m != 0:
                    raise Exception('Bad pattern found on console: ' +
                                    self.bad_pattern_ids[m - 1])
            self.u_boot_version_string = self.p.after
            while True:
                m = self.p.expect([self.prompt_compiled,
                    pattern_stop_autoboot_prompt] + self.bad_patterns)
                if m == 0:
                    break
                if m == 1:
                    self.p.send(' ')
                    continue
                raise Exception('Bad pattern found on console: ' +
                                self.bad_pattern_ids[m - 2])

        except Exception as ex:
            self.log.error(str(ex))
            self.cleanup_spawn()
            raise
        finally:
            self.log.timestamp()

    def run_command(self, cmd, wait_for_echo=True, send_nl=True,
            wait_for_prompt=True, wait_for_reboot=False):
        """Execute a command via the U-Boot console.

        The command is always sent to U-Boot.

        U-Boot echoes any command back to its output, and this function
        typically waits for that to occur. The wait can be disabled by setting
        wait_for_echo=False, which is useful e.g. when sending CTRL-C to
        interrupt a long-running command such as "ums".

        Command execution is typically triggered by sending a newline
        character. This can be disabled by setting send_nl=False, which is
        also useful when sending CTRL-C.

        This function typically waits for the command to finish executing, and
        returns the console output that it generated. This can be disabled by
        setting wait_for_prompt=False, which is useful when invoking a long-
        running command such as "ums".

        Args:
            cmd: The command to send.
            wait_for_echo: Boolean indicating whether to wait for U-Boot to
                echo the command text back to its output.
            send_nl: Boolean indicating whether to send a newline character
                after the command string.
            wait_for_prompt: Boolean indicating whether to wait for the
                command prompt to be sent by U-Boot. This typically occurs
                immediately after the command has been executed.
            wait_for_reboot: Boolean indication whether to wait for the
                reboot U-Boot. If this sets True, wait_for_prompt must also
                be True.

        Returns:
            If wait_for_prompt == False:
                Nothing.
            Else:
                The output from U-Boot during command execution. In other
                words, the text U-Boot emitted between the point it echod the
                command string and emitted the subsequent command prompts.
        """

        if self.at_prompt and \
                self.at_prompt_logevt != self.logstream.logfile.cur_evt:
            self.logstream.write(self.prompt, implicit=True)

        try:
            self.at_prompt = False
            if send_nl:
                cmd += '\n'
            while cmd:
                # Limit max outstanding data, so UART FIFOs don't overflow
                chunk = cmd[:self.max_fifo_fill]
                cmd = cmd[self.max_fifo_fill:]
                self.p.send(chunk)
                if not wait_for_echo:
                    continue
                chunk = re.escape(chunk)
                chunk = chunk.replace('\\\n', '[\r\n]')
                m = self.p.expect([chunk] + self.bad_patterns)
                if m != 0:
                    self.at_prompt = False
                    raise Exception('Bad pattern found on console: ' +
                                    self.bad_pattern_ids[m - 1])
            if not wait_for_prompt:
                return
            if wait_for_reboot:
                self.wait_for_boot_prompt()
            else:
                m = self.p.expect([self.prompt_compiled] + self.bad_patterns)
                if m != 0:
                    self.at_prompt = False
                    raise Exception('Bad pattern found on console: ' +
                                    self.bad_pattern_ids[m - 1])
            self.at_prompt = True
            self.at_prompt_logevt = self.logstream.logfile.cur_evt
            # Only strip \r\n; space/TAB might be significant if testing
            # indentation.
            return self.p.before.strip('\r\n')
        except Exception as ex:
            self.log.error(str(ex))
            self.cleanup_spawn()
            raise
        finally:
            self.log.timestamp()

    def run_command_list(self, cmds):
        """Run a list of commands.

        This is a helper function to call run_command() with default arguments
        for each command in a list.

        Args:
            cmd: List of commands (each a string).
        Returns:
            A list of output strings from each command, one element for each
            command.
        """
        output = []
        for cmd in cmds:
            output.append(self.run_command(cmd))
        return output

    def ctrlc(self):
        """Send a CTRL-C character to U-Boot.

        This is useful in order to stop execution of long-running synchronous
        commands such as "ums".

        Args:
            None.

        Returns:
            Nothing.
        """

        self.log.action('Sending Ctrl-C')
        self.run_command(chr(3), wait_for_echo=False, send_nl=False)

    def wait_for(self, text):
        """Wait for a pattern to be emitted by U-Boot.

        This is useful when a long-running command such as "dfu" is executing,
        and it periodically emits some text that should show up at a specific
        location in the log file.

        Args:
            text: The text to wait for; either a string (containing raw text,
                not a regular expression) or an re object.

        Returns:
            Nothing.
        """

        if type(text) == type(''):
            text = re.escape(text)
        m = self.p.expect([text] + self.bad_patterns)
        if m != 0:
            raise Exception('Bad pattern found on console: ' +
                            self.bad_pattern_ids[m - 1])

    def drain_console(self):
        """Read from and log the U-Boot console for a short time.

        U-Boot's console output is only logged when the test code actively
        waits for U-Boot to emit specific data. There are cases where tests
        can fail without doing this. For example, if a test asks U-Boot to
        enable USB device mode, then polls until a host-side device node
        exists. In such a case, it is useful to log U-Boot's console output
        in case U-Boot printed clues as to why the host-side even did not
        occur. This function will do that.

        Args:
            None.

        Returns:
            Nothing.
        """

        # If we are already not connected to U-Boot, there's nothing to drain.
        # This should only happen when a previous call to run_command() or
        # wait_for() failed (and hence the output has already been logged), or
        # the system is shutting down.
        if not self.p:
            return

        orig_timeout = self.p.timeout
        try:
            # Drain the log for a relatively short time.
            self.p.timeout = 1000
            # Wait for something U-Boot will likely never send. This will
            # cause the console output to be read and logged.
            self.p.expect(['This should never match U-Boot output'])
        except:
            # We expect a timeout, since U-Boot won't print what we waited
            # for. Squash it when it happens.
            #
            # Squash any other exception too. This function is only used to
            # drain (and log) the U-Boot console output after a failed test.
            # The U-Boot process will be restarted, or target board reset, once
            # this function returns. So, we don't care about detecting any
            # additional errors, so they're squashed so that the rest of the
            # post-test-failure cleanup code can continue operation, and
            # correctly terminate any log sections, etc.
            pass
        finally:
            self.p.timeout = orig_timeout

    def ensure_spawned(self, expect_reset=False):
        """Ensure a connection to a correctly running U-Boot instance.

        This may require spawning a new Sandbox process or resetting target
        hardware, as defined by the implementation sub-class.

        This is an internal function and should not be called directly.

        Args:
            expect_reset: Boolean indication whether this boot is expected
                to be reset while the 1st boot process after main boot before
                prompt. False by default.

        Returns:
            Nothing.
        """

        if self.p:
            # Reset the console timeout value as some tests may change
            # its default value during the execution
            if not self.config.gdbserver:
                self.p.timeout = 30000
            return
        try:
            self.log.start_section('Starting U-Boot')
            self.at_prompt = False
            self.p = self.get_spawn()
            # Real targets can take a long time to scroll large amounts of
            # text if LCD is enabled. This value may need tweaking in the
            # future, possibly per-test to be optimal. This works for 'help'
            # on board 'seaboard'.
            if not self.config.gdbserver:
                self.p.timeout = 30000
            self.p.logfile_read = self.logstream
            if expect_reset:
                loop_num = 2
            else:
                loop_num = 1
            self.wait_for_boot_prompt(loop_num = loop_num)
            self.at_prompt = True
            self.at_prompt_logevt = self.logstream.logfile.cur_evt
        except Exception as ex:
            self.log.error(str(ex))
            self.cleanup_spawn()
            raise
        finally:
            self.log.timestamp()
            self.log.end_section('Starting U-Boot')

    def cleanup_spawn(self):
        """Shut down all interaction with the U-Boot instance.

        This is used when an error is detected prior to re-establishing a
        connection with a fresh U-Boot instance.

        This is an internal function and should not be called directly.

        Args:
            None.

        Returns:
            Nothing.
        """

        try:
            if self.p:
                self.p.close()
        except:
            pass
        self.p = None

    def restart_uboot(self, expect_reset=False):
        """Shut down and restart U-Boot."""
        self.cleanup_spawn()
        self.ensure_spawned(expect_reset)

    def get_spawn_output(self):
        """Return the start-up output from U-Boot

        Returns:
            The output produced by ensure_spawed(), as a string.
        """
        if self.p:
            return self.p.get_expect_output()
        return None

    def validate_version_string_in_text(self, text):
        """Assert that a command's output includes the U-Boot signon message.

        This is primarily useful for validating the "version" command without
        duplicating the signon text regex in a test function.

        Args:
            text: The command output text to check.

        Returns:
            Nothing. An exception is raised if the validation fails.
        """

        assert(self.u_boot_version_string in text)

    def disable_check(self, check_type):
        """Temporarily disable an error check of U-Boot's output.

        Create a new context manager (for use with the "with" statement) which
        temporarily disables a particular console output error check.

        Args:
            check_type: The type of error-check to disable. Valid values may
            be found in self.disable_check_count above.

        Returns:
            A context manager object.
        """

        return ConsoleDisableCheck(self, check_type)

    def temporary_timeout(self, timeout):
        """Temporarily set up different timeout for commands.

        Create a new context manager (for use with the "with" statement) which
        temporarily change timeout.

        Args:
            timeout: Time in milliseconds.

        Returns:
            A context manager object.
        """

        return ConsoleSetupTimeout(self, timeout)
