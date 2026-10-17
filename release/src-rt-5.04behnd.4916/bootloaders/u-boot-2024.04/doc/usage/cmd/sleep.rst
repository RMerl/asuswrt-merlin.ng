.. SPDX-License-Identifier: GPL-2.0+
.. Copyright 2023, Heinrich Schuchardt <heinrich.schuchardt@canonical.com>

.. index::
   single: sleep (command)

sleep command
=============

Synopsis
--------

::

    sleep <delay>

Description
-----------

The *sleep* command waits for *delay* seconds. It can be interrupted by
CTRL+C.

delay
   delay in seconds. The value is decimal and can be fractional.

Example
-------

The current data and time is display before and after sleeping for 3.2
seconds:

::

    => date; sleep 3.2; date
    Date: 2023-01-21 (Saturday)    Time: 16:02:41
    Date: 2023-01-21 (Saturday)    Time: 16:02:44
    =>

Configuration
-------------

The command is only available if CONFIG_CMD_SLEEP=y.

Return value
------------

The return value $? is 0 (true) if the command completes.
The return value is 1 (false) if the command is interrupted by CTRL+C.
