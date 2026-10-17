.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: wdt (command)

wdt command
===========

Synopsis
--------

::

    wdt list
    wdt dev [<name>]
    wdt start <timeout_ms> [flags]
    wdt stop
    wdt reset
    wdt expirer [flags]

Description
-----------

The wdt command is used to control watchdog timers.

The 'wdt list' command shows a list of all watchdog devices.

The 'wdt dev' command called without argument shows the current watchdog device.
The current device is set when passing the name of the device as argument.

The 'wdt start' command starts the current watchdog timer.

The 'wdt stop' command stops the current watchdog timer.

The 'wdt reset' command resets the current watchdog timer without stopping it.

The 'wdt expire' command let's the current watchdog timer expire immediately.
This will lead to a reset.

name
    name of the watchdog device

timeout_ms
    timeout interval in milliseconds

flags
    unsigned long value passed to the driver. The usage is driver specific.
    The value is ignored by most drivers.

Example
-------

::

    => wdt dev
    No watchdog timer device set!
    => wdt list
    watchdog@1c20ca0 (sunxi_wdt)
    => wdt dev watchdog@1c20ca0
    => wdt dev
    dev: watchdog@1c20ca0
    => wdt start 3000
    => wdt reset
    => wdt stop
    => wdt expire

    U-Boot SPL 2022.04-rc3 (Mar 25 2022 - 13:48:33 +0000)

 In the example above '(sunxi_wdt)' refers to the driver for the watchdog
 device.

Configuration
-------------

The command is only available if CONFIG_CMD_WDT=y.

Return value
------------

The return value $? is 0 if the command succeeds, 1 upon failure.
