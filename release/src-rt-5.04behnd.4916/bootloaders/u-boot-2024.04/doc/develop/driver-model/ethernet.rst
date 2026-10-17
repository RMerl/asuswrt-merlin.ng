Ethernet Driver Guide
=====================

The networking stack in Das U-Boot is designed for multiple network devices
to be easily added and controlled at runtime.  This guide is meant for people
who wish to review the net driver stack with an eye towards implementing your
own ethernet device driver.  Here we will describe a new pseudo 'APE' driver.

Most existing drivers do already - and new network driver MUST - use the
U-Boot core driver model. Generic information about this can be found in
doc/driver-model/design.rst, this document will thus focus on the network
specific code parts.
Some drivers are still using the old Ethernet interface, differences between
the two and hints about porting will be handled at the end.

Driver framework
----------------

A network driver following the driver model must declare itself using
the UCLASS_ETH .id field in the U-Boot driver struct:

.. code-block:: c

	U_BOOT_DRIVER(eth_ape) = {
		.name			= "eth_ape",
		.id			= UCLASS_ETH,
		.of_match		= eth_ape_ids,
		.of_to_plat	= eth_ape_of_to_plat,
		.probe			= eth_ape_probe,
		.ops			= &eth_ape_ops,
		.priv_auto	= sizeof(struct eth_ape_priv),
		.plat_auto = sizeof(struct eth_ape_pdata),
		.flags			= DM_FLAG_ALLOC_PRIV_DMA,
	};

struct eth_ape_priv contains runtime per-instance data, like buffers, pointers
to current descriptors, current speed settings, pointers to PHY related data
(like struct mii_dev) and so on. Declaring its size in .priv_auto
will let the driver framework allocate it at the right time.
It can be retrieved using a dev_get_priv(dev) call.

struct eth_ape_pdata contains static platform data, like the MMIO base address,
a hardware variant, the MAC address. ``struct eth_pdata eth_pdata``
as the first member of this struct helps to avoid duplicated code.
If you don't need any more platform data beside the standard member,
just use sizeof(struct eth_pdata) for the plat_auto.

PCI devices add a line pointing to supported vendor/device ID pairs:

.. code-block:: c

	static struct pci_device_id supported[] = {
		{ PCI_DEVICE(PCI_VENDOR_ID_APE, 0x4223) },
		{}
	};

	U_BOOT_PCI_DEVICE(eth_ape, supported);

It is also possible to declare support for a whole class of PCI devices::

	{ PCI_DEVICE_CLASS(PCI_CLASS_SYSTEM_SDHCI << 8, 0xffff00) },

Device probing and instantiation will be handled by the driver model framework,
so follow the guidelines there. The probe() function would initialise the
platform specific parts of the hardware, like clocks, resets, GPIOs, the MDIO
bus. Also it would take care of any special PHY setup (power rails, enable
bits for internal PHYs, etc.).

Driver methods
--------------

The real work will be done in the driver method functions the driver provides
by defining the members of struct eth_ops:

.. code-block:: c

	struct eth_ops {
		int (*start)(struct udevice *dev);
		int (*send)(struct udevice *dev, void *packet, int length);
		int (*recv)(struct udevice *dev, int flags, uchar **packetp);
		int (*free_pkt)(struct udevice *dev, uchar *packet, int length);
		void (*stop)(struct udevice *dev);
		int (*mcast)(struct udevice *dev, const u8 *enetaddr, int join);
		int (*write_hwaddr)(struct udevice *dev);
		int (*read_rom_hwaddr)(struct udevice *dev);
	};

An up-to-date version of this struct together with more information can be
found in include/net.h.

Only start, stop, send and recv are required, the rest are optional and are
handled by generic code or ignored if not provided.

The **start** function initialises the hardware and gets it ready for send/recv
operations.  You often do things here such as resetting the MAC
and/or PHY, and waiting for the link to autonegotiate.  You should also take
the opportunity to program the device's MAC address with the enetaddr member
of the generic struct eth_pdata (which would be the first member of your
own plat struct). This allows the rest of U-Boot to dynamically change
the MAC address and have the new settings be respected.

The **send** function does what you think -- transmit the specified packet
whose size is specified by length (in bytes). The packet buffer can (and
will!) be reused for subsequent calls to send(), so it must be no longer
used when the send() function returns. The easiest way to achieve this is
to wait until the transmission is complete. Alternatively, if supported by
the hardware, just waiting for the buffer to be consumed (by some DMA engine)
might be an option as well.
Another way of consuming the buffer could be to copy the data to be send,
then just queue the copied packet (for instance handing it over to a DMA
engine), and return immediately afterwards.
In any case you should leave the state such that the send function can be
called multiple times in a row.

The **recv** function polls for availability of a new packet. If none is
available, it must return with -EAGAIN.
If a packet has been received, make sure it is accessible to the CPU
(invalidate caches if needed), then write its address to the packetp pointer,
and return the length. If there is an error (receive error, too short or too
long packet), return 0 if you require the packet to be cleaned up normally,
or a negative error code otherwise (cleanup not necessary or already done).
The U-Boot network stack will then process the packet.

If **free_pkt** is defined, U-Boot will call it after a received packet has
been processed, so the packet buffer can be freed or recycled. Typically you
would hand it back to the hardware to acquire another packet. free_pkt() will
be called after recv(), for the same packet, so you don't necessarily need
to infer the buffer to free from the ``packet`` pointer, but can rely on that
being the last packet that recv() handled.
The common code sets up packet buffers for you already in the .bss
(net_rx_packets), so there should be no need to allocate your own. This doesn't
mean you must use the net_rx_packets array however; you're free to use any
buffer you wish.

The **stop** function should turn off / disable the hardware and place it back
in its reset state.  It can be called at any time (before any call to the
related start() function), so make sure it can handle this sort of thing.

The (optional) **write_hwaddr** function should program the MAC address stored
in pdata->enetaddr into the Ethernet controller.

So the call graph at this stage would look something like:

.. code-block:: c

	(some net operation (ping / tftp / whatever...))
	eth_init()
		ops->start()
	eth_send()
		ops->send()
	eth_rx()
		ops->recv()
		(process packet)
		if (ops->free_pkt)
			ops->free_pkt()
	eth_halt()
		ops->stop()


CONFIG_PHYLIB / CONFIG_CMD_MII
------------------------------

If your device supports banging arbitrary values on the MII bus (pretty much
every device does), you should add support for the mii command.  Doing so is
fairly trivial and makes debugging mii issues a lot easier at runtime.

In your driver's ``probe()`` function, add a call to mdio_alloc() and
mdio_register() like so:

.. code-block:: c

	bus = mdio_alloc();
	if (!bus) {
		...
		return -ENOMEM;
	}

	bus->read = ape_mii_read;
	bus->write = ape_mii_write;
	mdio_register(bus);

And then define the mii_read and mii_write functions if you haven't already.
Their syntax is straightforward::

	int mii_read(struct mii_dev *bus, int addr, int devad, int reg);
	int mii_write(struct mii_dev *bus, int addr, int devad, int reg,
		      u16 val);

The read function should read the register 'reg' from the phy at address 'addr'
and return the result to its caller.  The implementation for the write function
should logically follow.

................................................................

Legacy network drivers
----------------------

!!! WARNING !!!

This section below describes the old way of doing things. No new Ethernet
drivers should be implemented this way. All new drivers should be written
against the U-Boot core driver model, as described above.

The actual callback functions are fairly similar, the differences are:

- ``start()`` is called ``init()``
- ``stop()`` is called ``halt()``
- The ``recv()`` function must loop until all packets have been received, for
  each packet it must call the net_process_received_packet() function,
  handing it over the pointer and the length. Afterwards it should free
  the packet, before checking for new data.

For porting an old driver to the new driver model, split the existing recv()
function into the actual new recv() function, just fetching **one** packet,
remove the call to net_process_received_packet(), then move the packet
cleanup into the ``free_pkt()`` function.

Registering the driver and probing a device is handled very differently,
follow the recommendations in the driver model design documentation for
instructions on how to port this over. For the records, the old way of
initialising a network driver is as follows:

Old network driver registration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When U-Boot initializes, it will call the common function eth_initialize().
This will in turn call the board-specific board_eth_init() (or if that fails,
the cpu-specific cpu_eth_init()).  These board-specific functions can do random
system handling, but ultimately they will call the driver-specific register
function which in turn takes care of initializing that particular instance.

Keep in mind that you should code the driver to avoid storing state in global
data as someone might want to hook up two of the same devices to one board.
Any such information that is specific to an interface should be stored in a
private, driver-defined data structure and pointed to by eth->priv (see below).

So the call graph at this stage would look something like:

.. code-block:: c

	board_init()
		eth_initialize()
			board_eth_init() / cpu_eth_init()
				driver_register()
					initialize eth_device
					eth_register()

At this point in time, the only thing you need to worry about is the driver's
register function.  The pseudo code would look something like:

.. code-block:: c

	int ape_register(struct bd_info *bis, int iobase)
	{
		struct ape_priv *priv;
		struct eth_device *dev;
		struct mii_dev *bus;

		priv = malloc(sizeof(*priv));
		if (priv == NULL)
			return -ENOMEM;

		dev = malloc(sizeof(*dev));
		if (dev == NULL) {
			free(priv);
			return -ENOMEM;
		}

		/* setup whatever private state you need */

		memset(dev, 0, sizeof(*dev));
		sprintf(dev->name, "APE");

		/*
		 * if your device has dedicated hardware storage for the
		 * MAC, read it and initialize dev->enetaddr with it
		 */
		ape_mac_read(dev->enetaddr);

		dev->iobase = iobase;
		dev->priv = priv;
		dev->init = ape_init;
		dev->halt = ape_halt;
		dev->send = ape_send;
		dev->recv = ape_recv;
		dev->write_hwaddr = ape_write_hwaddr;

		eth_register(dev);

	#ifdef CONFIG_PHYLIB
		bus = mdio_alloc();
		if (!bus) {
			free(priv);
			free(dev);
			return -ENOMEM;
		}

		bus->read = ape_mii_read;
		bus->write = ape_mii_write;
		mdio_register(bus);
	#endif

		return 1;
	}

The exact arguments needed to initialize your device are up to you.  If you
need to pass more/less arguments, that's fine.  You should also add the
prototype for your new register function to include/netdev.h.

The return value for this function should be as follows:
< 0 - failure (hardware failure, not probe failure)
>=0 - number of interfaces detected

You might notice that many drivers seem to use xxx_initialize() rather than
xxx_register().  This is the old naming convention and should be avoided as it
causes confusion with the driver-specific init function.

Other than locating the MAC address in dedicated hardware storage, you should
not touch the hardware in anyway.  That step is handled in the driver-specific
init function.  Remember that we are only registering the device here, we are
not checking its state or doing random probing.
