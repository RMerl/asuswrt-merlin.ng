.. SPDX-License-Identifier: GPL-2.0-or-later

.. index::
   single: temperature (command)

temperature command
===================

Synopsis
--------

::

    temperature list
    temperature get [thermal device name]

Description
-----------

The *temperature* command is used to list thermal sensors and get their
readings.

The 'temperature list' command diplays the available thermal devices.

The 'temperature get' command is used to get the reading in degrees C from
the desired device which is selected by passing its device name.

    thermal device name
        device name of thermal sensor to select

Example
-------

::


    => temperature list
    | Device                        | Driver                        | Parent
    | tmon@610508110                | sparx5-temp                   | axi@600000000
    =>
    => temperature get tmon@610508110
    tmon@610508110: 42 C

Configuration
-------------

The *temperature* command is only available if CONFIG_CMD_TEMPERATURE=y.

Return value
------------

The return value $? is set to 0 (true) if the command succeeded and to 1 (false)
otherwise.
