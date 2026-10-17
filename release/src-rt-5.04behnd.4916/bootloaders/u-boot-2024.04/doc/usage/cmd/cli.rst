.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: cli (command)

cli command
===========

Synopsis
--------

::

    cli get
    cli set cli_flavor

Description
-----------

The cli command permits getting and changing the current parser at runtime.

cli get
~~~~~~~

It shows the current value of the parser used by the CLI.

cli set
~~~~~~~

It permits setting the value of the parser used by the CLI.

Possible values are old and modern.
Note that, to use a specific parser its code should have been compiled, that
is to say you need to enable the corresponding CONFIG_HUSH*.
Otherwise, an error message is printed.

Examples
--------

Get the current parser::

    => cli get
    old

Change the current parser::

    => cli get
    old
    => cli set modern
    => cli get
    modern
    => cli set old
    => cli get
    old

Trying to set the current parser to an unknown value::

    => cli set foo
    Bad value for parser name: foo
    cli - cli

    Usage:
    cli get - print current cli
    set - set the current cli, possible values are: old, modern

Trying to set the current parser to a correct value but its code was not
compiled::

    => cli get
    modern
    => cli set old
    Want to set current parser to old, but its code was not compiled!

Return value
------------

The return value $? indicates whether the command succeeded.
