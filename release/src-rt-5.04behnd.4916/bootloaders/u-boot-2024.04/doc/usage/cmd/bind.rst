.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: bind (command)

bind command
============

Synopsis
--------

::

    bind <node path> <driver>
    bind <class> <index> <driver>

Description
-----------

The bind command is used to bind a device to a driver. This makes the
device available in U-Boot.

While binding to a *node path* typically provides a working device
binding by parent node and driver may lead to a device that is only
partially initialized.

node path
    path of the device's device-tree node

class
    device class name

index
    index of the parent device in the device class

driver
    device driver name

Example
-------

Given a system with a real time clock device with device path */pl031@9010000*
and using driver rtc-pl031 unbinding and binding of the device is demonstrated
using the two alternative bind syntaxes.

.. code-block::

    => dm tree
     Class     Index  Probed  Driver                Name
    -----------------------------------------------------------
     root          0  [ + ]   root_driver           root_driver
    ...
     rtc           0  [   ]   rtc-pl031             |-- pl031@9010000
    ...
    => fdt addr $fdtcontroladdr
    Working FDT set to 7ed7fdb0
    => fdt print
    / {
            interrupt-parent = <0x00008003>;
            model = "linux,dummy-virt";
            #size-cells = <0x00000002>;
            #address-cells = <0x00000002>;
            compatible = "linux,dummy-virt";
    ...
            pl031@9010000 {
                    clock-names = "apb_pclk";
                    clocks = <0x00008000>;
                    interrupts = <0x00000000 0x00000002 0x00000004>;
                    reg = <0x00000000 0x09010000 0x00000000 0x00001000>;
                    compatible = "arm,pl031", "arm,primecell";
            };
    ...
    }
    => unbind /pl031@9010000
    => date
    Cannot find RTC: err=-19
    => dm tree
     Class     Index  Probed  Driver                Name
    -----------------------------------------------------------
     root          0  [ + ]   root_driver           root_driver
    ...
    => bind /pl031@9010000 rtc-pl031
    => dm tree
     Class     Index  Probed  Driver                Name
    -----------------------------------------------------------
     root          0  [ + ]   root_driver           root_driver
    ...
     rtc           0  [   ]   rtc-pl031             |-- pl031@9010000
    => date
    Date: 2023-06-22 (Thursday)    Time: 15:14:51
    => unbind rtc 0 rtc-pl031
    => bind root 0 rtc-pl031
    => date
    Date: 1980-08-19 (Tuesday)    Time: 14:45:30

Obviously the device is not initialized correctly by the last bind command.

Configuration
-------------

The bind command is only available if CONFIG_CMD_BIND=y.

Return code
-----------

The return code $? is 0 (true) on success and 1 (false) on failure.
