# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2011 The Chromium OS Authors.
#

import os
import cros_subprocess

"""Shell command ease-ups for Python."""

class CommandResult:
    """A class which captures the result of executing a command.

    Members:
        stdout: stdout obtained from command, as a string
        stderr: stderr obtained from command, as a string
        return_code: Return code from command
        exception: Exception received, or None if all ok
    """
    def __init__(self):
        self.stdout = None
        self.stderr = None
        self.combined = None
        self.return_code = None
        self.exception = None

    def __init__(self, stdout='', stderr='', combined='', return_code=0,
                 exception=None):
        self.stdout = stdout
        self.stderr = stderr
        self.combined = combined
        self.return_code = return_code
        self.exception = exception


# This permits interception of RunPipe for test purposes. If it is set to
# a function, then that function is called with the pipe list being
# executed. Otherwise, it is assumed to be a CommandResult object, and is
# returned as the result for every RunPipe() call.
# When this value is None, commands are executed as normal.
test_result = None

def RunPipe(pipe_list, infile=None, outfile=None,
            capture=False, capture_stderr=False, oneline=False,
            raise_on_error=True, cwd=None, **kwargs):
    """
    Perform a command pipeline, with optional input/output filenames.

    Args:
        pipe_list: List of command lines to execute. Each command line is
            piped into the next, and is itself a list of strings. For
            example [ ['ls', '.git'] ['wc'] ] will pipe the output of
            'ls .git' into 'wc'.
        infile: File to provide stdin to the pipeline
        outfile: File to store stdout
        capture: True to capture output
        capture_stderr: True to capture stderr
        oneline: True to strip newline chars from output
        kwargs: Additional keyword arguments to cros_subprocess.Popen()
    Returns:
        CommandResult object
    """
    if test_result:
        if hasattr(test_result, '__call__'):
            result = test_result(pipe_list=pipe_list)
            if result:
                return result
        else:
            return test_result
        # No result: fall through to normal processing
    result = CommandResult()
    last_pipe = None
    pipeline = list(pipe_list)
    user_pipestr =  '|'.join([' '.join(pipe) for pipe in pipe_list])
    kwargs['stdout'] = None
    kwargs['stderr'] = None
    while pipeline:
        cmd = pipeline.pop(0)
        if last_pipe is not None:
            kwargs['stdin'] = last_pipe.stdout
        elif infile:
            kwargs['stdin'] = open(infile, 'rb')
        if pipeline or capture:
            kwargs['stdout'] = cros_subprocess.PIPE
        elif outfile:
            kwargs['stdout'] = open(outfile, 'wb')
        if capture_stderr:
            kwargs['stderr'] = cros_subprocess.PIPE

        try:
            last_pipe = cros_subprocess.Popen(cmd, cwd=cwd, **kwargs)
        except Exception as err:
            result.exception = err
            if raise_on_error:
                raise Exception("Error running '%s': %s" % (user_pipestr, str))
            result.return_code = 255
            return result

    if capture:
        result.stdout, result.stderr, result.combined = (
                last_pipe.CommunicateFilter(None))
        if result.stdout and oneline:
            result.output = result.stdout.rstrip('\r\n')
        result.return_code = last_pipe.wait()
    else:
        result.return_code = os.waitpid(last_pipe.pid, 0)[1]
    if raise_on_error and result.return_code:
        raise Exception("Error running '%s'" % user_pipestr)
    return result

def Output(*cmd, **kwargs):
    raise_on_error = kwargs.get('raise_on_error', True)
    return RunPipe([cmd], capture=True, raise_on_error=raise_on_error).stdout

def OutputOneLine(*cmd, **kwargs):
    raise_on_error = kwargs.pop('raise_on_error', True)
    return (RunPipe([cmd], capture=True, oneline=True,
            raise_on_error=raise_on_error,
            **kwargs).stdout.strip())

def Run(*cmd, **kwargs):
    return RunPipe([cmd], **kwargs).stdout

def RunList(cmd):
    return RunPipe([cmd], capture=True).stdout

def StopAll():
    cros_subprocess.stay_alive = False
