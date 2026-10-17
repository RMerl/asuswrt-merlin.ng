.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: coninfo (command)

coninfo command
===============

Synopsis
--------

::

    coninfo

Description
-----------

The coninfo command provides a list of available console input and output
devices and their assignment as stdin, stdout, stderr console devices.

If CONFIG_SYS_CONSOLE_IS_IN_ENV=y, the assignment is controlled by the
environment variables stdin, stdout, stderr which contain a comma separated
list of device names.

Example
-------

.. code-block:: console

    => coninfo
    List of available devices
    |-- pl011@9000000 (IO)
    |   |-- stdin
    |   |-- stdout
    |   |-- stderr
    |-- serial (IO)
    |-- usbkbd (I)
    => setenv stdin pl011@9000000,usbkbd
    => coninfo
    List of available devices
    |-- pl011@9000000 (IO)
    |   |-- stdin
    |   |-- stdout
    |   |-- stderr
    |-- serial (IO)
    |-- usbkbd (I)
    |   |-- stdin

Configuration
-------------

The coninfo command is only available if CONFIG_CMD_CONSOLE=y.

Return value
------------

The return value $? is always 0 (true).
