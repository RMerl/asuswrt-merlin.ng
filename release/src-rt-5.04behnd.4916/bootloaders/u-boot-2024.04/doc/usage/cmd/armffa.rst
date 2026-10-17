.. SPDX-License-Identifier: GPL-2.0+
.. Copyright 2022-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>

.. index::
   single: armffa (command)

armffa command
==============

Synopsis
--------

::

   armffa [sub-command] [arguments]

   sub-commands:

        getpart [partition UUID]

            lists the partition(s) info

        ping [partition ID]

            sends a data pattern to the specified partition

        devlist

            displays information about the FF-A device/driver

Description
-----------

armffa is a command showcasing how to use the FF-A bus and how to invoke its operations.

This provides a guidance to the client developers on how to call the FF-A bus interfaces.

The command also allows to gather secure partitions information and ping these  partitions.

The command is also helpful in testing the communication with secure partitions.

Example
-------

The following examples are run on Corstone-1000 platform.

* ping

::

   corstone1000# armffa ping 0x8003
   SP response:
   [LSB]
   fffffffe
   0
   0
   0
   0

* ping (failure case)

::

   corstone1000# armffa ping 0
   Sending direct request error (-22)

* getpart

::

   corstone1000# armffa getpart 33d532ed-e699-0942-c09c-a798d9cd722d
   Partition: id = 8003 , exec_ctxt 1 , properties 3

* getpart (failure case)

::

   corstone1000# armffa getpart 33d532ed-e699-0942-c09c-a798d9cd7221
   INVALID_PARAMETERS: Unrecognized UUID
   Failure in querying partitions count (error code: -22)

* devlist

::

   corstone1000# armffa devlist
   device name arm_ffa, dev 00000000fdf41c30, driver name arm_ffa, ops 00000000fffc0e98

Configuration
-------------

The command is available if CONFIG_CMD_ARMFFA=y and CONFIG_ARM_FFA_TRANSPORT=y.

Return value
------------

The return value $? is 0 (true) on success, 1 (false) on failure.
