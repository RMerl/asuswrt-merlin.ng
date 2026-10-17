.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: setexpr (command)

setexpr command
===============

Synopsis
--------

::

    setexpr[.b, .w, .l .s] <name> [*]<value> <op> [*]<value2>
    setexpr[.b, .w, .l] <name> [*]<value>
    setexpr <name> fmt <format> [value]...
    setexpr <name> gsub r s [t]
    setexpr <name> sub r s [t]

Description
-----------

The setexpr command is used to set an environment variable to the result
of an evaluation.

setexpr[.b, .w, .l .s] <name> [*]<value> <op> [*]<value2>
     Set environment variable <name> to the result of the evaluated
     expression specified by <op>.

setexpr[.b, .w, .l] name [*]value
     Load <value> into environment variable <name>

setexpr name fmt <format> value
     Set environment variable <name> to the result of the C like
     format string <format> evaluation of <value>.

setexpr name gsub <r> <s> [<t>]
     For each substring matching the regular expression <r> in the
     string <t>, substitute the string <s>.
     The result is assigned to <name>.
     If <t> is not supplied, use the old value of <name>.

setexpr name sub <r> <s> [<t>]
     Just like gsub(), but replace only the first matching substring

The setexpr command takes the following arguments:

format
    This parameter contains a C or Bash like format string.
    The number of arguments is limited to 4.
    The following format types are supported:

    c
        single character
    d, i
        decimal value
    o
        octal value
    s
        string
    u
        unsigned decimal value
    x, X
        hexadecimal value
    '%'
        no conversion, instead a % character will be written

    Backslash escapes:

    \" = double quote
    \\ = backslash
    \a = alert (bell)
    \b = backspace
    \c = produce no further output
    \f = form feed
    \n = new line
    \r = carriage return
    \t = horizontal tab
    \v = vertical tab
    \NNN = octal number (NNN is 0 to 3 digits)

name
    The name of the environment variable to be set

op
    '|'
        name = value | value2
    '&'
        name = value & value2
    '+'
        name = value + value2
        (This is the only operator supported for strings.
	It acts as concatenation operator on strings)
    '^'
        name = value ^ value2
    '-'
        name = value - value2
    '*'
        name = value * value2
    '/'
        name = value / value2
    '%'
        name = value % value2

r
    Regular expression

s
    Substitution string

t
    string

value
    Can either be an integer value, a string.
    If the pointer prefix '*' is given value is treated as memory address.

value2
    See value

Example
-------

::

    => setexpr foo fmt %d 0x100
    => echo $foo
    256
    =>

    => setexpr foo fmt 0x%08x 63
    => echo $foo
    0x00000063
    =>

    => setexpr foo fmt %%%o 8
    => echo $foo
    %10
    =>

Configuration
-------------

* The *setexpr* command is only available if CMD_SETEXPR=y.
* The *setexpr fmt* sub-command is only available if CMD_SETEXPR_FMT=y.
* The *setexpr gsub* and *setexpr sub* sub-commands are only available if
  CONFIG_REGEX=y.

Return value
------------

The return value $? is set to 0 (true) if the operation was successful.

If an error occurs, the return value $? is set to 1 (false).
