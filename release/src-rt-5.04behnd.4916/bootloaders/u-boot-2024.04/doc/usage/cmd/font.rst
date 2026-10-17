.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: font (command)

font command
============

Synopsis
--------

::

    font list
    font select <name> [<size>]
    font size <size>

Description
-----------

The *font* command allows selection of the font to use on the video console.
This is available when the TrueType console is in use.

font list
~~~~~~~~~

This lists the available fonts, using the name of the font file in the build.

font select
~~~~~~~~~~~

This selects a new font and optionally changes the size.

font size
~~~~~~~~~

This changes the font size only.

Examples
--------

::

    => font list
    nimbus_sans_l_regular
    cantoraone_regular
    => font size 40
    => font select cantoraone_regular 20
    =>

Configuration
-------------

The command is only available if CONFIG_CONSOLE_TRUETYPE=y.

Return value
------------

The return value $? is 0 (true) if the command completes.
The return value is 1 (false) if the command fails.
