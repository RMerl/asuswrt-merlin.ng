.. index::
   single: conitrace (command)

conitrace command
=================

Synopsis
--------

::

    conitrace

Description
-----------

The conitrace command is used to test the correct function of the console input
driver. It is especially valuable for checking the support for special keys like
<F1> or <POS1>.

To display escape sequences on a single line the output only advances to the
next line after detecting a pause of a few milliseconds.

The output is hexadecimal.

Examples
--------

Entering keys <B><SHIFT-B><CTRL-B><X>

::

    => conitrace
    Waiting for your input
    To terminate type 'x'
    62
    42
    02
    =>

Entering keys <F1><POS1><DEL><BACKSPACE><X>

::

    => conitrace
    Waiting for your input
    To terminate type 'x'
    1b 4f 50
    1b 5b 48
    1b 5b 33 7e
    7f
    =>

Configuration
-------------

The conitrace command is only available if CONFIG_CMD_CONITRACE=y.
