.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: scmi (command)

scmi command
============

Synopsis
--------

::

    scmi info
    scmi perm_dev <agent id> <device id> <flags>
    scmi perm_proto <agent id> <device id> <protocol id> <flags>
    scmi reset <agent id> <flags>

Description
-----------

Arm System Control and Management Interface (SCMI hereafter) is a set of
standardised interfaces to manage system resources, like clocks, power
domains, pin controls, reset and so on, in a system-wide manner.

An entity which provides those services is called a SCMI firmware (or
SCMI server if you like) may be placed/implemented by EL3 software or
by a dedicated system control processor (SCP) or else.

A user of SCMI interfaces, including U-Boot, is called a SCMI agent and
may issues commands, which are defined in each protocol for specific system
resources, to SCMI server via a communication channel, called a transport.
Those interfaces are independent from the server's implementation thanks to
a transport layer.

For more details, see the `SCMI specification`_.

While most of system resources managed under SCMI protocols are implemented
and handled as standard U-Boot devices, for example clk_scmi, scmi command
provides additional management functionality against SCMI server.

scmi info
~~~~~~~~~
    Show base information about SCMI server and supported protocols

scmi perm_dev
~~~~~~~~~~~~~
    Allow or deny access permission to the device

scmi perm_proto
~~~~~~~~~~~~~~~
    Allow or deny access to the protocol on the device

scmi reset
~~~~~~~~~~
    Reset the already-configured permissions against the device

Parameters are used as follows:

<agent id>
    SCMI Agent ID, hex value

<device id>
    SCMI Device ID, hex value

    Please note that what a device means is not defined
    in the specification.

<protocol id>
    SCMI Protocol ID, hex value

    It must not be 0x10 (base protocol)

<flags>
    Flags to control the action, hex value

    0 to deny, 1 to allow. The other values are reserved and allowed
    values may depend on the implemented version of SCMI server in
    the future. See SCMI specification for more details.

Example
-------

Obtain basic information about SCMI server:

::

    => scmi info
    SCMI device: scmi
      protocol version: 0x20000
      # of agents: 3
          0: platform
        > 1: OSPM
          2: PSCI
      # of protocols: 4
          Power domain management
          Performance domain management
          Clock management
          Sensor management
      vendor: Linaro
      sub vendor: PMWG
      impl version: 0x20b0000

Ask for access permission to device#0:

::

    => scmi perm_dev 1 0 1

Reset configurations with all access permission settings retained:

::

    => scmi reset 1 0

Configuration
-------------

The scmi command is only available if CONFIG_CMD_SCMI=y.
Default n because this command is mainly for debug purpose.

Return value
------------

The return value ($?) is set to 0 if the operation succeeded,
1 if the operation failed or -1 if the operation failed due to
a syntax error.

.. _`SCMI specification`: https://developer.arm.com/documentation/den0056/e/?lang=en
