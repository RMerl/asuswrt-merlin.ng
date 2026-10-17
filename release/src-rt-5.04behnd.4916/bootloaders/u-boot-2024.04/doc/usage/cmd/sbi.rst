.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: sbi (command)

sbi command
===========

Synopsis
--------

::

    sbi

Description
-----------

The sbi command is used to display information about the SBI (Supervisor Binary
Interface) implementation on RISC-V systems.

The output may look like:

::

    => sbi
    SBI 1.0
    OpenSBI 1.1
    Machine:
      Vendor ID 0
      Architecture ID 0
      Implementation ID 0
    Extensions:
      Set Timer
      Console Putchar
      Console Getchar
      Clear IPI
      Send IPI
      Remote FENCE.I
      Remote SFENCE.VMA
      Remote SFENCE.VMA with ASID
      System Shutdown
      SBI Base Functionality
      Timer Extension
      IPI Extension
      RFENCE Extension
      Hart State Management Extension
      System Reset Extension
      Performance Monitoring Unit Extension

The first line indicates the version of the RISC-V SBI specification.
The second line indicates the implementation.
The Machine section shows the values of the machine information registers.
The Extensions section enumerates the implemented SBI extensions.

Configuration
-------------

To use the sbi command you must specify CONFIG_CMD_SBI=y.
