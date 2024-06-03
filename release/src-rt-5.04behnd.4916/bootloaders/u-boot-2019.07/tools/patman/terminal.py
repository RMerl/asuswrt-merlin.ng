# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2011 The Chromium OS Authors.
#

"""Terminal utilities

This module handles terminal interaction including ANSI color codes.
"""

from __future__ import print_function

import os
import sys

# Selection of when we want our output to be colored
COLOR_IF_TERMINAL, COLOR_ALWAYS, COLOR_NEVER = range(3)

# Initially, we are set up to print to the terminal
print_test_mode = False
print_test_list = []

class PrintLine:
    """A line of text output

    Members:
        text: Text line that was printed
        newline: True to output a newline after the text
        colour: Text colour to use
    """
    def __init__(self, text, newline, colour):
        self.text = text
        self.newline = newline
        self.colour = colour

    def __str__(self):
        return 'newline=%s, colour=%s, text=%s' % (self.newline, self.colour,
                self.text)

def Print(text='', newline=True, colour=None):
    """Handle a line of output to the terminal.

    In test mode this is recorded in a list. Otherwise it is output to the
    terminal.

    Args:
        text: Text to print
        newline: True to add a new line at the end of the text
        colour: Colour to use for the text
    """
    if print_test_mode:
        print_test_list.append(PrintLine(text, newline, colour))
    else:
        if colour:
            col = Color()
            text = col.Color(colour, text)
        print(text, end='')
        if newline:
            print()
        else:
            sys.stdout.flush()

def SetPrintTestMode():
    """Go into test mode, where all printing is recorded"""
    global print_test_mode

    print_test_mode = True

def GetPrintTestLines():
    """Get a list of all lines output through Print()

    Returns:
        A list of PrintLine objects
    """
    global print_test_list

    ret = print_test_list
    print_test_list = []
    return ret

def EchoPrintTestLines():
    """Print out the text lines collected"""
    for line in print_test_list:
        if line.colour:
            col = Color()
            print(col.Color(line.colour, line.text), end='')
        else:
            print(line.text, end='')
        if line.newline:
            print()


class Color(object):
    """Conditionally wraps text in ANSI color escape sequences."""
    BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE = range(8)
    BOLD = -1
    BRIGHT_START = '\033[1;%dm'
    NORMAL_START = '\033[22;%dm'
    BOLD_START = '\033[1m'
    RESET = '\033[0m'

    def __init__(self, colored=COLOR_IF_TERMINAL):
        """Create a new Color object, optionally disabling color output.

        Args:
          enabled: True if color output should be enabled. If False then this
            class will not add color codes at all.
        """
        try:
            self._enabled = (colored == COLOR_ALWAYS or
                    (colored == COLOR_IF_TERMINAL and
                     os.isatty(sys.stdout.fileno())))
        except:
            self._enabled = False

    def Start(self, color, bright=True):
        """Returns a start color code.

        Args:
          color: Color to use, .e.g BLACK, RED, etc.

        Returns:
          If color is enabled, returns an ANSI sequence to start the given
          color, otherwise returns empty string
        """
        if self._enabled:
            base = self.BRIGHT_START if bright else self.NORMAL_START
            return base % (color + 30)
        return ''

    def Stop(self):
        """Retruns a stop color code.

        Returns:
          If color is enabled, returns an ANSI color reset sequence,
          otherwise returns empty string
        """
        if self._enabled:
            return self.RESET
        return ''

    def Color(self, color, text, bright=True):
        """Returns text with conditionally added color escape sequences.

        Keyword arguments:
          color: Text color -- one of the color constants defined in this
                  class.
          text: The text to color.

        Returns:
          If self._enabled is False, returns the original text. If it's True,
          returns text with color escape sequences based on the value of
          color.
        """
        if not self._enabled:
            return text
        if color == self.BOLD:
            start = self.BOLD_START
        else:
            base = self.BRIGHT_START if bright else self.NORMAL_START
            start = base % (color + 30)
        return start + text + self.RESET
