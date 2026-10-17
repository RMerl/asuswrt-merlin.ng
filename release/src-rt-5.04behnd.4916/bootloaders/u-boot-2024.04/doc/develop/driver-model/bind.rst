.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Patrice Chotard <patrice.chotard@foss.st.com>

Binding/unbinding a driver
==========================

This document aims to describe the bind and unbind commands.

For debugging purpose, it should be useful to bind or unbind a driver from
the U-Boot command line.

The unbind command calls the remove device driver callback and unbind the
device from its driver.

The bind command binds a device to its driver.

In some cases it can be useful to be able to bind a device to a driver from
the command line.
The obvious example is for versatile devices such as USB gadget.
Another use case is when the devices are not yet ready at startup and
require some setup before the drivers are bound (ex: FPGA which bitsream is
fetched from a mass storage or ethernet)

usage:

bind <node path> <driver>
bind <class> <index> <driver>

unbind <node path>
unbind <class> <index>
unbind <class> <index> <driver>

Where:
 - <node path> is the node's device tree path
 - <class> is one of the class available in the list given by the "dm uclass"
   command or first column of "dm tree" command.
 - <index> is the index of the parent's node (second column of "dm tree" output).
 - <driver> is the driver name to bind given by the "dm drivers" command or the by
   the fourth column of "dm tree" output.

example:

bind usb_dev_generic 0 usb_ether
unbind usb_dev_generic 0 usb_ether
or
unbind eth 1

bind /ocp/omap_dwc3@48380000/usb@48390000 usb_ether
unbind /ocp/omap_dwc3@48380000/usb@48390000
