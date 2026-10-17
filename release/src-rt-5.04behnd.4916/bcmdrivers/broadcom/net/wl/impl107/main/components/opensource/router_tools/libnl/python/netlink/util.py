#
# Utilities
#
# Copyright (c) 2011 Thomas Graf <tgraf@suug.ch>
#

"""utility module for netlink

"""

from __future__ import absolute_import

from . import core as netlink
from . import capi as capi
from string import Formatter
import types

__version__ = '1.0'

#rename into colored_output
def _color(t, c):
    return '{esc}[{color}m{text}{esc}[0m'.format(esc=b'\x1b'.decode(), color=c, text=t)

def black(t):
    return _color(t, 30)

def red(t):
    return _color(t, 31)

def green(t):
    return _color(t, 32)

def yellow(t):
    return _color(t, 33)

def blue(t):
    return _color(t, 34)

def magenta(t):
    return _color(t, 35)

def cyan(t):
    return _color(t, 36)

def white(t):
    return _color(t, 37)

def bold(t):
    return _color(t, 1)

def kw(t):
    return yellow(t)

def num(t):
    return str(t)

def string(t):
    return t

def addr(t):
    return str(t)

def bad(t):
    return red(t)

def good(t):
    return green(t)

def title(t):
    return t

def boolean(t):
    return str(t)

def handle(t):
    return str(t)

class MyFormatter(Formatter):
    def __init__(self, obj, indent=''):
        self._obj = obj
        self._indent = indent

    def _nlattr(self, key):
        value = getattr(self._obj.__class__, key)
        if not isinstance(value, property):
            raise ValueError('Invalid formatting string {0}'.format(key))

        d = getattr(value.fget, 'formatinfo', dict())

        # value = value.fget() is exactly the same
        value = getattr(self._obj, key)

        if 'fmt' in d:
            value = d['fmt'](value)

        title_ = d.get('title', None)

        return title_, str(value)

    def get_value(self, key, args, kwds):
        # Let default get_value() handle ints
        if not isinstance(key, str):
            return Formatter.get_value(self, key, args, kwds)

        # HACK, we allow defining strings via fields to allow
        # conversions
        if key[:2] == 's|':
            return key[2:]

        if key[:2] == 't|':
            # title mode ("TITLE ATTR")
            include_title = True
        elif key[:2] == 'a|':
            # plain attribute mode ("ATTR")
            include_title = False
        else:
            # No special field, have default get_value() get it
            return Formatter.get_value(self, key, args, kwds)

        key = key[2:]
        (title_, value) = self._nlattr(key)

        if include_title:
            if not title_:
                title_ = key    # fall back to key as title
            value = '{0} {1}'.format(kw(title_), value)

        return value

    def convert_field(self, value, conversion):
        if conversion == 'r':
            return repr(value)
        elif conversion == 's':
            return str(value)
        elif conversion == 'k':
            return kw(value)
        elif conversion == 'b':
            return bold(value)
        elif conversion is None:
            return value

        raise ValueError('Unknown converion specifier {0!s}'.format(conversion))

    def nl(self, format_string=''):
        return '\n' + self._indent + self.format(format_string)

NL_BYTE_RATE = 0
NL_BIT_RATE = 1

class Rate(object):
    def __init__(self, rate, mode=NL_BYTE_RATE):
        self._rate = rate
        self._mode = mode

    def __str__(self):
        return capi.nl_rate2str(self._rate, self._mode, 32)[1]

    def __int__(self):
        return self._rate

    def __cmp__(self, other):
        return int(self) - int(other)

class Size(object):
    def __init__(self, size):
        self._size = size

    def __str__(self):
        return capi.nl_size2str(self._size, 32)[0]

    def __int__(self):
        return self._size

    def __cmp__(self, other):
        return int(self) - int(other)
