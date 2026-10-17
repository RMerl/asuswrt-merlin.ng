.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: size (command)

size command
============

Synopsis
--------

::

    size <interface> <dev[:part]> <filename>

Description
-----------

The size command determines the size of a file and sets the environment variable
filesize to this value. If filename points to a directory, the value is set to
zero.

If the command fails, the filesize environment variable is not changed.

dev
    device number

part
    partition number, defaults to 1

filename
    path to file

Configuration
-------------

The size command is only available if CONFIG_CMD_FS_GENERIC=y.

Return value
------------

The return value $? is set to 0 (true) if the command succeded and to 1 (false)
otherwise.
