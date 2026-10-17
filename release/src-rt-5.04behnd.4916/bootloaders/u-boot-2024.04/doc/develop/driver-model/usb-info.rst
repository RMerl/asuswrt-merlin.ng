.. SPDX-License-Identifier: GPL-2.0+

How USB works with driver model
===============================

Introduction
------------

Driver model USB support makes use of existing features but changes how
drivers are found. This document provides some information intended to help
understand how things work with USB in U-Boot when driver model is enabled.


Enabling driver model for USB
-----------------------------

A new CONFIG_DM_USB option is provided to enable driver model for USB. This
causes the USB uclass to be included, and drops the equivalent code in
usb.c. In particular the usb_init() function is then implemented by the
uclass.


Support for EHCI and XHCI
-------------------------

So far OHCI is not supported. Both EHCI and XHCI drivers should be declared
as drivers in the USB uclass. For example:

.. code-block:: c

	static const struct udevice_id ehci_usb_ids[] = {
		{ .compatible = "nvidia,tegra20-ehci", .data = USB_CTLR_T20 },
		{ .compatible = "nvidia,tegra30-ehci", .data = USB_CTLR_T30 },
		{ .compatible = "nvidia,tegra114-ehci", .data = USB_CTLR_T114 },
		{ }
	};

	U_BOOT_DRIVER(usb_ehci) = {
		.name	= "ehci_tegra",
		.id	= UCLASS_USB,
		.of_match = ehci_usb_ids,
		.of_to_plat = ehci_usb_of_to_plat,
		.probe = tegra_ehci_usb_probe,
		.remove = tegra_ehci_usb_remove,
		.ops	= &ehci_usb_ops,
		.plat_auto = sizeof(struct usb_plat),
		.priv_auto = sizeof(struct fdt_usb),
		.flags	= DM_FLAG_ALLOC_PRIV_DMA,
	};

Here ehci_usb_ids is used to list the controllers that the driver supports.
Each has its own data value. Controllers must be in the UCLASS_USB uclass.

The of_to_plat() method allows the controller driver to grab any
necessary settings from the device tree.

The ops here are ehci_usb_ops. All EHCI drivers will use these same ops in
most cases, since they are all EHCI-compatible. For EHCI there are also some
special operations that can be overridden when calling ehci_register().

The driver can use priv_auto to set the size of its private data.
This can hold run-time information needed by the driver for operation. It
exists when the device is probed (not when it is bound) and is removed when
the driver is removed.

Note that usb_plat is currently only used to deal with setting up a bus
in USB device mode (OTG operation). It can be omitted if that is not
supported.

The driver's probe() method should do the basic controller init and then
call ehci_register() to register itself as an EHCI device. It should call
ehci_deregister() in the remove() method. Registering a new EHCI device
does not by itself cause the bus to be scanned.

The old ehci_hcd_init() function is no-longer used. Nor is it necessary to
set up the USB controllers from board init code. When 'usb start' is used,
each controller will be probed and its bus scanned.

XHCI works in a similar way.


Data structures
---------------

The following primary data structures are in use:

- struct usb_device:
	This holds information about a device on the bus. All devices have
	this structure, even the root hub. The controller itself does not
	have this structure. You can access it for a device 'dev' with
	dev_get_parent_priv(dev). It matches the old structure except that the
	parent and child information is not present (since driver model
	handles that). Once the device is set up, you can find the device
	descriptor and current configuration descriptor in this structure.

- struct usb_plat:
	This holds platform data for a controller. So far this is only used
	as a work-around for controllers which can act as USB devices in OTG
	mode, since the gadget framework does not use driver model.

- struct usb_dev_plat:
	This holds platform data for a device. You can access it for a
	device 'dev' with dev_get_parent_plat(dev). It holds the device
	address and speed - anything that can be determined before the device
	driver is actually set up. When probing the bus this structure is
	used to provide essential information to the device driver.

- struct usb_bus_priv:
	This is private information for each controller, maintained by the
	controller uclass. It is mostly used to keep track of the next
	device address to use.

Of these, only struct usb_device was used prior to driver model.


USB buses
---------

Given a controller, you know the bus - it is the one attached to the
controller. Each controller handles exactly one bus. Every controller has a
root hub attached to it. This hub, which is itself a USB device, can provide
one or more 'ports' to which additional devices can be attached. It is
possible to power up a hub and find out which of its ports have devices
attached.

Devices are given addresses starting at 1. The root hub is always address 1,
and from there the devices are numbered in sequence. The USB uclass takes
care of this numbering automatically during enumeration.

USB devices are enumerated by finding a device on a particular hub, and
setting its address to the next available address. The USB bus stretches out
in a tree structure, potentially with multiple hubs each with several ports
and perhaps other hubs. Some hubs will have their own power since otherwise
the 5V 500mA power supplied by the controller will not be sufficient to run
very many devices.

Enumeration in U-Boot takes a long time since devices are probed one at a
time, and each is given sufficient time to wake up and announce itself. The
timeouts are set for the slowest device.

Up to 127 devices can be on each bus. USB has four bus speeds: low
(1.5Mbps), full (12Mbps), high (480Mbps) which is only available with USB2
and newer (EHCI), and super (5Gbps) which is only available with USB3 and
newer (XHCI). If you connect a super-speed device to a high-speed hub, you
will only get high-speed.


USB operations
--------------

As before driver model, messages can be sent using submit_bulk_msg() and the
like. These are now implemented by the USB uclass and route through the
controller drivers. Note that messages are not sent to the driver of the
device itself - i.e. they don't pass down the stack to the controller.
U-Boot simply finds the controller to which the device is attached, and sends
the message there with an appropriate 'pipe' value so it can be addressed
properly. Having said that, the USB device which should receive the message
is passed in to the driver methods, for use by sandbox. This design decision
is open for review and the code impact of changing it is small since the
methods are typically implemented by the EHCI and XHCI stacks.

Controller drivers (in UCLASS_USB) themselves provide methods for sending
each message type. For XHCI an additional alloc_device() method is provided
since XHCI needs to allocate a device context before it can even read the
device's descriptor.

These methods use a 'pipe' which is a collection of bit fields used to
describe the type of message, direction of transfer and the intended
recipient (device number).


USB Devices
-----------

USB devices are found using a simple algorithm which works through the
available hubs in a depth-first search. Devices can be in any uclass, but
are attached to a parent hub (or controller in the case of the root hub) and
so have parent data attached to them (this is struct usb_device).

By the time the device's probe() method is called, it is enumerated and is
ready to talk to the host.

The enumeration process needs to work out which driver to attach to each USB
device. It does this by examining the device class, interface class, vendor
ID, product ID, etc. See struct usb_driver_entry for how drivers are matched
with USB devices - you can use the USB_DEVICE() macro to declare a USB
driver. For example, usb_storage.c defines a USB_DEVICE() to handle storage
devices, and it will be used for all USB devices which match.



Technical details on enumeration flow
-------------------------------------

It is useful to understand precisely how a USB bus is enumerating to avoid
confusion when dealing with USB devices.

Device initialisation happens roughly like this:

- At some point the 'usb start' command is run
- This calls usb_init() which works through each controller in turn
- The controller is probed(). This does no enumeration.
- Then usb_scan_bus() is called. This calls usb_scan_device() to scan the
  (only) device that is attached to the controller - a root hub
- usb_scan_device() sets up a fake struct usb_device and calls
  usb_setup_device(), passing the port number to be scanned, in this case
  port 0
- usb_setup_device() first calls usb_prepare_device() to set the device
  address, then usb_select_config() to select the first configuration
- at this point the device is enumerated but we do not have a real struct
  udevice for it. But we do have the descriptor in struct usb_device so we can
  use this to figure out what driver to use
- back in usb_scan_device(), we call usb_find_child() to try to find an
  existing device which matches the one we just found on the bus. This can
  happen if the device is mentioned in the device tree, or if we previously
  scanned the bus and so the device was created before
- if usb_find_child() does not find an existing device, we call
  usb_find_and_bind_driver() which tries to bind one
- usb_find_and_bind_driver() searches all available USB drivers (declared
  with USB_DEVICE()). If it finds a match it binds that driver to create a
  new device.
- If it does not, it binds a generic driver. A generic driver is good enough
  to allow access to the device (sending it packets, etc.) but all
  functionality will need to be implemented outside the driver model.
- in any case, when usb_find_child() and/or usb_find_and_bind_driver() are
  done, we have a device with the correct uclass. At this point we want to
  probe the device
- first we store basic information about the new device (address, port,
  speed) in its parent platform data. We cannot store it its private data
  since that will not exist until the device is probed.
- then we call device_probe() which probes the device
- the first probe step is actually the USB controller's (or USB hubs's)
  child_pre_probe() method. This gets called before anything else and is
  intended to set up a child device ready to be used with its parent bus. For
  USB this calls usb_child_pre_probe() which grabs the information that was
  stored in the parent platform data and stores it in the parent private data
  (which is struct usb_device, a real one this time). It then calls
  usb_select_config() again to make sure that everything about the device is
  set up
- note that we have called usb_select_config() twice. This is inefficient
  but the alternative is to store additional information in the platform data.
  The time taken is minimal and this way is simpler
- at this point the device is set up and ready for use so far as the USB
  subsystem is concerned
- the device's probe() method is then called. It can send messages and do
  whatever else it wants to make the device work.

Note that the first device is always a root hub, and this must be scanned to
find any devices. The above steps will have created a hub (UCLASS_USB_HUB),
given it address 1 and set the configuration.

For hubs, the hub uclass has a post_probe() method. This means that after
any hub is probed, the uclass gets to do some processing. In this case
usb_hub_post_probe() is called, and the following steps take place:

- usb_hub_post_probe() calls usb_hub_scan() to scan the hub, which in turn
  calls usb_hub_configure()
- hub power is enabled
- we loop through each port on the hub, performing the same steps for each
- first, check if there is a device present. This happens in
  usb_hub_port_connect_change(). If so, then usb_scan_device() is called to
  scan the device, passing the appropriate port number.
- you will recognise usb_scan_device() from the steps above. It sets up the
  device ready for use. If it is a hub, it will scan that hub before it
  continues here (recursively, depth-first)
- once all hub ports are scanned in this way, the hub is ready for use and
  all of its downstream devices also
- additional controllers are scanned in the same way

The above method has some nice properties:

- the bus enumeration happens by virtue of driver model's natural device flow
- most logic is in the USB controller and hub uclasses; the actual device
  drivers do not need to know they are on a USB bus, at least so far as
  enumeration goes
- hub scanning happens automatically after a hub is probed


Hubs
----

USB hubs are scanned as in the section above. While hubs have their own
uclass, they share some common elements with controllers:

- they both attach private data to their children (struct usb_device,
  accessible for a child with dev_get_parent_priv(child))
- they both use usb_child_pre_probe() to set up their children as proper USB
  devices


Example - Mass Storage
----------------------

As an example of a USB device driver, see usb_storage.c. It uses its own
uclass and declares itself as follows:

.. code-block:: c

	U_BOOT_DRIVER(usb_mass_storage) = {
		.name	= "usb_mass_storage",
		.id	= UCLASS_MASS_STORAGE,
		.of_match = usb_mass_storage_ids,
		.probe = usb_mass_storage_probe,
	};

	static const struct usb_device_id mass_storage_id_table[] = {
		{ .match_flags = USB_DEVICE_ID_MATCH_INT_CLASS,
		  .bInterfaceClass = USB_CLASS_MASS_STORAGE},
		{ }	/* Terminating entry */
	};

	USB_DEVICE(usb_mass_storage, mass_storage_id_table);

The USB_DEVICE() macro attaches the given table of matching information to
the given driver. Note that the driver is declared in U_BOOT_DRIVER() as
'usb_mass_storage' and this must match the first parameter of USB_DEVICE.

When usb_find_and_bind_driver() is called on a USB device with the
bInterfaceClass value of USB_CLASS_MASS_STORAGE, it will automatically find
this driver and use it.


Counter-example: USB Ethernet
-----------------------------

As an example of the old way of doing things, see usb_ether.c. When the bus
is scanned, all Ethernet devices will be created as generic USB devices (in
uclass UCLASS_USB_DEV_GENERIC). Then, when the scan is completed,
usb_host_eth_scan() will be called. This looks through all the devices on
each bus and manually figures out which are Ethernet devices in the ways of
yore.

In fact, usb_ether should be moved to driver model. Each USB Ethernet driver
(e.g drivers/usb/eth/asix.c) should include a USB_DEVICE() declaration, so
that it will be found as part of normal USB enumeration. Then, instead of a
generic USB driver, a real (driver-model-aware) driver will be used. Since
Ethernet now supports driver model, this should be fairly easy to achieve,
and then usb_ether.c and the usb_host_eth_scan() will melt away.


Sandbox
-------

All driver model uclasses must have tests and USB is no exception. To
achieve this, a sandbox USB controller is provided. This can make use of
emulation drivers which pretend to be USB devices. Emulations are provided
for a hub and a flash stick. These are enough to create a pretend USB bus
(defined by the sandbox device tree sandbox.dts) which can be scanned and
used.

Tests in test/dm/usb.c make use of this feature. It allows much of the USB
stack to be tested without real hardware being needed.

Here is an example device tree fragment:

.. code-block:: none

	usb@1 {
		compatible = "sandbox,usb";
		hub {
			compatible = "usb-hub";
			usb,device-class = <USB_CLASS_HUB>;
			hub-emul {
				compatible = "sandbox,usb-hub";
				#address-cells = <1>;
				#size-cells = <0>;
				flash-stick {
					reg = <0>;
					compatible = "sandbox,usb-flash";
					sandbox,filepath = "flash.bin";
				};
			};
		};
	};

This defines a single controller, containing a root hub (which is required).
The hub is emulated by a hub emulator, and the emulated hub has a single
flash stick to emulate on one of its ports.

When 'usb start' is used, the following 'dm tree' output will be available::

   usb         [ + ]    `-- usb@1
   usb_hub     [ + ]        `-- hub
   usb_emul    [ + ]            |-- hub-emul
   usb_emul    [ + ]            |   `-- flash-stick
   usb_mass_st [ + ]            `-- usb_mass_storage


This may look confusing. Most of it mirrors the device tree, but the
'usb_mass_storage' device is not in the device tree. This is created by
usb_find_and_bind_driver() based on the USB_DRIVER in usb_storage.c. While
'flash-stick' is the emulation device, 'usb_mass_storage' is the real U-Boot
USB device driver that talks to it.


Future work
-----------

It is pretty uncommon to have a large USB bus with lots of hubs on an
embedded system. In fact anything other than a root hub is uncommon. Still
it would be possible to speed up enumeration in two ways:

- breadth-first search would allow devices to be reset and probed in
  parallel to some extent
- enumeration could be lazy, in the sense that we could enumerate just the
  root hub at first, then only progress to the next 'level' when a device is
  used that we cannot find. This could be made easier if the devices were
  statically declared in the device tree (which is acceptable for production
  boards where the same, known, things are on each bus).

But in common cases the current algorithm is sufficient.

Other things that need doing:
- Convert usb_ether to use driver model as described above
- Test that keyboards work (and convert to driver model)
- Move the USB gadget framework to driver model
- Implement OHCI in driver model
- Implement USB PHYs in driver model
- Work out a clever way to provide lazy init for USB devices


.. Simon Glass <sjg@chromium.org>
.. 23-Mar-15
