.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: ums (command)

ums command
===========

Synopsis
--------

::

    ums <dev> [<interface>] <devnum[:partnum]>

Description
-----------

Use the USB Mass Storage class (also known as UMS) to make accessible an U-Boot
block device (fully or with :ref:`U-Boot's partition syntax <partitions>`)
to a USB host and to enable file transfers. U-Boot, the USB device, acts as a
simple external hard drive plugged on the host USB port.

This command "ums" stays in the USB's treatment loop until user enters Ctrl-C.

dev
    USB gadget device number

interface
    interface for accessing the block device (mmc, sata, scsi, usb, ....)
    defaults is "mmc"

devnum
    device number for selected interface

partnum
    partition number or 0 to expose all partitions, defaults to 0

Example
-------

::

    => ums 0 mmc 0
    => ums 0 usb 1:2

Configuration
-------------

The ums command is only available if CONFIG_CMD_USB_MASS_STORAGE=y
and depends on CONFIG_USB_USB_GADGET and CONFIG_BLK.

Return value
------------

The return value $? is set to 0 (true) when the USB stack was successfully
started and interrupted, with Ctrl-C or after USB cable issue (detection
timeout or cable removal).

If an error occurs, the return value $? is set to 1 (false).
