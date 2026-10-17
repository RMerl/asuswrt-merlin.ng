.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: sm (command)

sm command
==========

Synopsis
--------

::

    sm serial <address>
    sm reboot_reason [name]
    sm efuseread <offset> <size> <address>
    sm efusewrite <offset> <size> <address>
    sm efusedump <offset> <size>

Description
-----------

The sm command is used to request services from the secure monitor. User
can call secure monitor to request special TEE function, for example chip
serial number info, reboot reason, etc.

sm serial
  Retrieve chip unique serial number from sm and write it to memory on
  appropriate address.

sm reboot_reason
  Print reboot reason to the console, if parameter [name] isn't specified.
  If parameter specified, set reboot reason string to environment variable
  with this name.

sm efuseread
  Read <size> bytes starting from <offset> from efuse memory bank and write
  result to the address <address>.

sm efusewrite
  Write into efuse memory bank, starting from <offset>, the <size> bytes
  of data, located at address <address>.

sm efusedump
  Read <size> bytes starting from <offset> from efuse memory bank and print
  them to the console.

Configuration
-------------

To use the sm command you must specify CONFIG_CMD_MESON=y
