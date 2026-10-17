.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: panic (command)

panic command
=============

Synopsis
--------

::

    panic [message]

Description
-----------

Display a message and reset the board.

message
    text to be displayed

Examples
--------

::

    => panic 'Unrecoverable error'
    Unrecoverable error
    resetting ...

Configuration
-------------

If CONFIG_PANIC_HANG=y, the user has to reset the board manually.
