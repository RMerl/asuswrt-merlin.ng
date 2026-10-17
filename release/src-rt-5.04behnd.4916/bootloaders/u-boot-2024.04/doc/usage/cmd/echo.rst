.. index::
   single: echo (command)

echo command
============

Synopsis
--------

::

    echo [-n] [args ...]

Description
-----------

The echo command prints its arguments to the console separated by spaces.

-n
    Do not print a line feed after the last argument.

args
    Arguments to be printed. The arguments are evaluated before being passed to
    the command.

Examples
--------

Strings are parsed before the arguments are passed to the echo command:

::

    => echo "a" 'b' c
    a b c
    =>

Observe how variables included in strings are handled:

::

    => setenv var X; echo "a)" ${var} 'b)' '${var}' c) ${var}
    a) X b) ${var} c) X
    =>


-n suppresses the line feed:

::

    => echo -n 1 2 3; echo a b c
    1 2 3a b c
    => echo -n 1 2 3
    1 2 3=>

A more complex example:

::

    => for i in a b c; do for j in 1 2 3; do echo -n "${i}${j}, "; done; echo; done;
    a1, a2, a3,
    b1, b2, b3,
    c1, c2, c3,
    =>

Return value
------------

The return value $? is always set to 0 (true).
