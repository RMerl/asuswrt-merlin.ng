# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
#
# Terminal output logging.
#

import sys

import terminal

# Output verbosity levels that we support
ERROR = 0
WARNING = 1
NOTICE = 2
INFO = 3
DEBUG = 4

in_progress = False

"""
This class handles output of progress and other useful information
to the user. It provides for simple verbosity level control and can
output nothing but errors at verbosity zero.

The idea is that modules set up an Output object early in their years and pass
it around to other modules that need it. This keeps the output under control
of a single class.

Public properties:
    verbose: Verbosity level: 0=silent, 1=progress, 3=full, 4=debug
"""
def __enter__():
    return

def __exit__(unused1, unused2, unused3):
    """Clean up and remove any progress message."""
    ClearProgress()
    return False

def UserIsPresent():
    """This returns True if it is likely that a user is present.

    Sometimes we want to prompt the user, but if no one is there then this
    is a waste of time, and may lock a script which should otherwise fail.

    Returns:
        True if it thinks the user is there, and False otherwise
    """
    return stdout_is_tty and verbose > 0

def ClearProgress():
    """Clear any active progress message on the terminal."""
    global in_progress
    if verbose > 0 and stdout_is_tty and in_progress:
        _stdout.write('\r%s\r' % (" " * len (_progress)))
        _stdout.flush()
        in_progress = False

def Progress(msg, warning=False, trailer='...'):
    """Display progress information.

    Args:
        msg: Message to display.
        warning: True if this is a warning."""
    global in_progress
    ClearProgress()
    if verbose > 0:
        _progress = msg + trailer
        if stdout_is_tty:
            col = _color.YELLOW if warning else _color.GREEN
            _stdout.write('\r' + _color.Color(col, _progress))
            _stdout.flush()
            in_progress = True
        else:
            _stdout.write(_progress + '\n')

def _Output(level, msg, color=None):
    """Output a message to the terminal.

    Args:
        level: Verbosity level for this message. It will only be displayed if
                this as high as the currently selected level.
        msg; Message to display.
        error: True if this is an error message, else False.
    """
    if verbose >= level:
        ClearProgress()
        if color:
            msg = _color.Color(color, msg)
        _stdout.write(msg + '\n')

def DoOutput(level, msg):
    """Output a message to the terminal.

    Args:
        level: Verbosity level for this message. It will only be displayed if
                this as high as the currently selected level.
        msg; Message to display.
    """
    _Output(level, msg)

def Error(msg):
    """Display an error message

    Args:
        msg; Message to display.
    """
    _Output(0, msg, _color.RED)

def Warning(msg):
    """Display a warning message

    Args:
        msg; Message to display.
    """
    _Output(1, msg, _color.YELLOW)

def Notice(msg):
    """Display an important infomation message

    Args:
        msg; Message to display.
    """
    _Output(2, msg)

def Info(msg):
    """Display an infomation message

    Args:
        msg; Message to display.
    """
    _Output(3, msg)

def Debug(msg):
    """Display a debug message

    Args:
        msg; Message to display.
    """
    _Output(4, msg)

def UserOutput(msg):
    """Display a message regardless of the current output level.

    This is used when the output was specifically requested by the user.
    Args:
        msg; Message to display.
    """
    _Output(0, msg)

def Init(_verbose=WARNING, stdout=sys.stdout):
    """Initialize a new output object.

    Args:
        verbose: Verbosity level (0-4).
        stdout: File to use for stdout.
    """
    global verbose, _progress, _color, _stdout, stdout_is_tty

    verbose = _verbose
    _progress = ''                    # Our last progress message
    _color = terminal.Color()
    _stdout = stdout

    # TODO(sjg): Move this into Chromite libraries when we have them
    stdout_is_tty = hasattr(sys.stdout, 'isatty') and sys.stdout.isatty()

def Uninit():
    ClearProgress()

Init()
