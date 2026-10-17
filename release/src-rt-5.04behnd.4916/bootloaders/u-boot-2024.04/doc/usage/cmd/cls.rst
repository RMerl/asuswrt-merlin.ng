.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: cls (command)

cls command
===========

Synopsis
--------

::

    cls

Description
-----------

The cls command clears the screen.

Configuration
-------------

The cls command is only available if CONFIG_CMD_CLS=y.

Return value
------------

The return value $? is 0 (true) on success and 1 (false) on failure.
