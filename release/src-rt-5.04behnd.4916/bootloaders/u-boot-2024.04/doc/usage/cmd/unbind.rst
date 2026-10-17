.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: unbind (command)

unbind command
==============

Synopsis
--------

::

    unbind <node path>
    unbind <class> <index>
    unbind <class> <index> <driver>

Description
-----------

The unbind command is used to unbind a device from a driver. This makes the
device unavailable in U-Boot.

node path
    path of the device's device-tree node

class
    device class name

index
    index of the device in the device class

driver
    device driver name

Example
-------

Given a system with a real time clock device with device path */pl031@9010000*
and using driver rtc-pl031 unbinding and binding of the device is demonstrated
using the three alternative unbind syntaxes.

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
    => dm tree
     Class     Index  Probed  Driver                Name
    -----------------------------------------------------------
     root          0  [ + ]   root_driver           root_driver
    ...
    => unbind /pl031@9010000
    Cannot find a device with path /pl031@9010000
    => bind /pl031@9010000 rtc-pl031
    => dm tree
     Class     Index  Probed  Driver                Name
    -----------------------------------------------------------
     root          0  [ + ]   root_driver           root_driver
    ...
     rtc           0  [   ]   rtc-pl031             |-- pl031@9010000
    => unbind rtc 0
    => bind /pl031@9010000 rtc-pl031
    => unbind rtc 0 rtc-pl031

Configuration
-------------

The unbind command is only available if CONFIG_CMD_BIND=y.

Return code
-----------

The return code $? is 0 (true) on success and 1 (false) on failure.
