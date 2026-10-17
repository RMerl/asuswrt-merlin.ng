// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SMBus block read/write support added by Stefan Roese:
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <log.h>
#include <pci.h>
#include <asm/io.h>

/* PCI Configuration Space (D31:F3): SMBus */
#define SMB_BASE		0x20
#define HOSTC			0x40
#define  HST_EN			(1 << 0)
#define SMB_RCV_SLVA		0x09

/* SMBus I/O bits. */
#define SMBHSTSTAT		0x0
#define SMBHSTCTL		0x2
#define SMBHSTCMD		0x3
#define SMBXMITADD		0x4
#define SMBHSTDAT0		0x5
#define SMBHSTDAT1		0x6
#define SMBBLKDAT		0x7
#define SMBTRNSADD		0x9
#define SMBSLVDATA		0xa
#define SMBAUXCTL		0xd
#define SMLINK_PIN_CTL		0xe
#define SMBUS_PIN_CTL		0xf

/* I801 Hosts Status register bits */
#define SMBHSTSTS_BYTE_DONE	0x80
#define SMBHSTSTS_INUSE_STS	0x40
#define SMBHSTSTS_SMBALERT_STS	0x20
#define SMBHSTSTS_FAILED	0x10
#define SMBHSTSTS_BUS_ERR	0x08
#define SMBHSTSTS_DEV_ERR	0x04
#define SMBHSTSTS_INTR		0x02
#define SMBHSTSTS_HOST_BUSY	0x01

/* I801 Host Control register bits */
#define SMBHSTCNT_INTREN	0x01
#define SMBHSTCNT_KILL		0x02
#define SMBHSTCNT_LAST_BYTE	0x20
#define SMBHSTCNT_START		0x40
#define SMBHSTCNT_PEC_EN	0x80	/* ICH3 and later */

/* Auxiliary control register bits, ICH4+ only */
#define SMBAUXCTL_CRC		1
#define SMBAUXCTL_E32B		2

#define SMBUS_TIMEOUT	100	/* 100 ms */

struct intel_i2c {
	u32 base;
	int running;
};

static int smbus_wait_until_ready(u32 base)
{
	unsigned long ts;
	u8 byte;

	ts = get_timer(0);
	do {
		byte = inb(base + SMBHSTSTAT);
		if (!(byte & 1))
			return 0;
	} while (get_timer(ts) < SMBUS_TIMEOUT);

	return -ETIMEDOUT;
}

static int smbus_wait_until_done(u32 base)
{
	unsigned long ts;
	u8 byte;

	ts = get_timer(0);
	do {
		byte = inb(base + SMBHSTSTAT);
		if (!((byte & 1) || (byte & ~((1 << 6) | (1 << 0))) == 0))
			return 0;
	} while (get_timer(ts) < SMBUS_TIMEOUT);

	return -ETIMEDOUT;
}

static int smbus_block_read(u32 base, u8 dev, u8 *buffer,
			    int offset, int len)
{
	u8 buf_temp[32];
	int count;
	int i;

	debug("%s (%d): dev=0x%x offs=0x%x len=0x%x\n",
	      __func__, __LINE__, dev, offset, len);
	if (smbus_wait_until_ready(base) < 0)
		return -ETIMEDOUT;

	/* Setup transaction */

	/* Reset the data buffer index */
	inb(base + SMBHSTCTL);

	/* Set the device I'm talking too */
	outb(((dev & 0x7f) << 1) | 1, base + SMBXMITADD);
	/* Set the command/address... */
	outb(offset & 0xff, base + SMBHSTCMD);
	/* Set up for a block read */
	outb((inb(base + SMBHSTCTL) & (~(0x7) << 2)) | (0x5 << 2),
	     (base + SMBHSTCTL));
	/* Clear any lingering errors, so the transaction will run */
	outb(inb(base + SMBHSTSTAT), base + SMBHSTSTAT);

	/* Start the command */
	outb((inb(base + SMBHSTCTL) | SMBHSTCNT_START), base + SMBHSTCTL);

	/* Poll for transaction completion */
	if (smbus_wait_until_done(base) < 0) {
		printf("SMBUS read transaction timeout (dev=0x%x)\n", dev);
		return -ETIMEDOUT;
	}

	count = inb(base + SMBHSTDAT0);
	debug("%s (%d): count=%d (len=%d)\n", __func__, __LINE__, count, len);
	if (count == 0) {
		debug("ERROR: len=0 on read\n");
		return -EIO;
	}

	if (count < len) {
		debug("ERROR: too few bytes read\n");
		return -EIO;
	}

	if (count > 32) {
		debug("ERROR: count=%d too high\n", count);
		return -EIO;
	}

	/* Read all available bytes from buffer */
	for (i = 0; i < count; i++)
		buf_temp[i] = inb(base + SMBBLKDAT);

	memcpy(buffer, buf_temp, len);

	/* Return results of transaction */
	if (!(inb(base + SMBHSTSTAT) & SMBHSTSTS_INTR))
		return -EIO;

	return 0;
}

static int smbus_block_write(u32 base, u8 dev, u8 *buffer,
			     int offset, int len)
{
	int i;

	debug("%s (%d): dev=0x%x offs=0x%x len=0x%x\n",
	      __func__, __LINE__, dev, offset, len);
	if (smbus_wait_until_ready(base) < 0)
		return -ETIMEDOUT;

	/* Setup transaction */
	/* Set the device I'm talking too */
	outb(((dev & 0x7f) << 1) & ~0x01, base + SMBXMITADD);
	/* Set the command/address... */
	outb(offset, base + SMBHSTCMD);
	/* Set up for a block write */
	outb((inb(base + SMBHSTCTL) & (~(0x7) << 2)) | (0x5 << 2),
	     (base + SMBHSTCTL));
	/* Clear any lingering errors, so the transaction will run */
	outb(inb(base + SMBHSTSTAT), base + SMBHSTSTAT);

	/* Write count in DAT0 register */
	outb(len, base + SMBHSTDAT0);

	/* Write data bytes... */
	for (i = 0; i < len; i++)
		outb(*buffer++, base + SMBBLKDAT);

	/* Start the command */
	outb((inb(base + SMBHSTCTL) | SMBHSTCNT_START), base + SMBHSTCTL);

	/* Poll for transaction completion */
	if (smbus_wait_until_done(base) < 0) {
		printf("SMBUS write transaction timeout (dev=0x%x)\n", dev);
		return -ETIMEDOUT;
	}

	/* Return results of transaction */
	if (!(inb(base + SMBHSTSTAT) & SMBHSTSTS_INTR))
		return -EIO;

	return 0;
}

static int intel_i2c_xfer(struct udevice *bus, struct i2c_msg *msg, int nmsgs)
{
	struct intel_i2c *i2c = dev_get_priv(bus);
	struct i2c_msg *dmsg, *omsg, dummy;

	debug("i2c_xfer: %d messages\n", nmsgs);

	memset(&dummy, 0, sizeof(struct i2c_msg));

	/*
	 * We expect either two messages (one with an offset and one with the
	 * actual data) or one message (just data)
	 */
	if (nmsgs > 2 || nmsgs == 0) {
		debug("%s: Only one or two messages are supported", __func__);
		return -EIO;
	}

	omsg = nmsgs == 1 ? &dummy : msg;
	dmsg = nmsgs == 1 ? msg : msg + 1;

	if (dmsg->flags & I2C_M_RD)
		return smbus_block_read(i2c->base, dmsg->addr, &dmsg->buf[0],
					omsg->buf[0], dmsg->len);
	else
		return smbus_block_write(i2c->base, dmsg->addr, &dmsg->buf[1],
					 dmsg->buf[0], dmsg->len - 1);
}

static int intel_i2c_probe_chip(struct udevice *bus, uint chip_addr,
				uint chip_flags)
{
	struct intel_i2c *i2c = dev_get_priv(bus);
	u8 buf[4];

	return smbus_block_read(i2c->base, chip_addr, buf, 0, 1);
}

static int intel_i2c_set_bus_speed(struct udevice *bus, unsigned int speed)
{
	return 0;
}

static int intel_i2c_probe(struct udevice *dev)
{
	struct intel_i2c *priv = dev_get_priv(dev);
	ulong base;

	/* Save base address from PCI BAR */
	priv->base = (ulong)dm_pci_map_bar(dev, PCI_BASE_ADDRESS_4, 0, 0, PCI_REGION_TYPE,
					   PCI_REGION_IO);
	base = priv->base;

	/* Set SMBus enable. */
	dm_pci_write_config8(dev, HOSTC, HST_EN);

	/* Disable interrupts */
	outb(inb(base + SMBHSTCTL) & ~SMBHSTCNT_INTREN, base + SMBHSTCTL);

	/* Set 32-byte data buffer mode */
	outb(inb(base + SMBAUXCTL) | SMBAUXCTL_E32B, base + SMBAUXCTL);

	return 0;
}

static int intel_i2c_bind(struct udevice *dev)
{
	char name[20];

	/* Create a unique device name for PCI type devices */
	if (device_is_on_pci_bus(dev)) {
		sprintf(name, "intel_i2c#%u", dev_seq(dev));
		device_set_name(dev, name);
	}

	return 0;
}

static const struct dm_i2c_ops intel_i2c_ops = {
	.xfer		= intel_i2c_xfer,
	.probe_chip	= intel_i2c_probe_chip,
	.set_bus_speed	= intel_i2c_set_bus_speed,
};

static const struct udevice_id intel_i2c_ids[] = {
	{ .compatible = "intel,ich-i2c" },
	{ }
};

U_BOOT_DRIVER(intel_i2c) = {
	.name	= "i2c_intel",
	.id	= UCLASS_I2C,
	.of_match = intel_i2c_ids,
	.ops	= &intel_i2c_ops,
	.priv_auto	= sizeof(struct intel_i2c),
	.bind	= intel_i2c_bind,
	.probe	= intel_i2c_probe,
};

static struct pci_device_id intel_smbus_pci_supported[] = {
	/* Intel BayTrail SMBus on the PCI bus */
	{ PCI_VDEVICE(INTEL, 0x0f12) },
	/* Intel IvyBridge (Panther Point PCH) SMBus on the PCI bus */
	{ PCI_VDEVICE(INTEL, 0x1e22) },
	{},
};

U_BOOT_PCI_DEVICE(intel_i2c, intel_smbus_pci_supported);
