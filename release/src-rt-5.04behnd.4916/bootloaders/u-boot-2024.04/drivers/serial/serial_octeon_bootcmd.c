// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Marvell International Ltd.
 * Copyright (C) 2021 Stefan Roese <sr@denx.de>
 */

#include <dm.h>
#include <dm/uclass.h>
#include <errno.h>
#include <input.h>
#include <iomux.h>
#include <log.h>
#include <serial.h>
#include <stdio_dev.h>
#include <string.h>
#include <watchdog.h>
#include <linux/delay.h>
#include <asm/addrspace.h>
#include <asm/io.h>
#include <mach/cvmx-regs.h>
#include <mach/cvmx-bootmem.h>

#define DRIVER_NAME				"pci-bootcmd"

/*
 * Important:
 * This address cannot be changed as the PCI console tool relies on exactly
 * this value!
 */
#define BOOTLOADER_PCI_READ_BUFFER_BASE		0x6c000
#define BOOTLOADER_PCI_READ_BUFFER_SIZE		256
#define BOOTLOADER_PCI_WRITE_BUFFER_SIZE	256

#define BOOTLOADER_PCI_READ_BUFFER_STR_LEN	\
	(BOOTLOADER_PCI_READ_BUFFER_SIZE - 8)
#define BOOTLOADER_PCI_WRITE_BUFFER_STR_LEN	\
	(BOOTLOADER_PCI_WRITE_BUFFER_SIZE - 8)

#define BOOTLOADER_PCI_READ_BUFFER_OWNER_ADDR	\
	(BOOTLOADER_PCI_READ_BUFFER_BASE + 0)
#define BOOTLOADER_PCI_READ_BUFFER_LEN_ADDR	\
	(BOOTLOADER_PCI_READ_BUFFER_BASE + 4)
#define BOOTLOADER_PCI_READ_BUFFER_DATA_ADDR	\
	(BOOTLOADER_PCI_READ_BUFFER_BASE + 8)

enum octeon_pci_io_buf_owner {
	/* Must be zero, set when memory cleared */
	OCTEON_PCI_IO_BUF_OWNER_INVALID = 0,
	OCTEON_PCI_IO_BUF_OWNER_OCTEON = 1,
	OCTEON_PCI_IO_BUF_OWNER_HOST = 2,
};

/* Structure for bootloader PCI IO buffers */
struct octeon_pci_io_buf {
	u32 owner;
	u32 len;
	char data[0];
};

struct octeon_bootcmd_priv {
	bool started;
	int copy_offset;
	bool eol;
	bool unlocked;
	struct octeon_pci_io_buf *buf;
};

static int octeon_bootcmd_pending(struct udevice *dev, bool input)
{
	struct octeon_bootcmd_priv *priv = dev_get_priv(dev);

	if (!input)
		return 0;

	if (priv->eol)
		return 1;

	CVMX_SYNC;
	if (priv->buf->owner != OCTEON_PCI_IO_BUF_OWNER_OCTEON)
		return 0;

	if (priv->buf->len > priv->copy_offset &&
	    (priv->buf->data[priv->copy_offset] != '\0'))
		return 1;

	return 0;
}

static int octeon_bootcmd_getc(struct udevice *dev)
{
	struct octeon_bootcmd_priv *priv = dev_get_priv(dev);
	char c;

	/* There's no EOL for boot commands so we fake it. */
	if (priv->eol) {
		priv->eol = false;
		return '\n';
	}

	while (!octeon_bootcmd_pending(dev, true)) {
		schedule();
		/*
		 * ToDo:
		 * The original code calls octeon_board_poll() here. We may
		 * need to implement something similar here.
		 */
		udelay(100);
	}

	c = priv->buf->data[priv->copy_offset];
	priv->buf->data[priv->copy_offset++] = '\0';

	if (priv->copy_offset >= min_t(int, CONFIG_SYS_CBSIZE - 1,
				       BOOTLOADER_PCI_READ_BUFFER_STR_LEN - 1) ||
	    (priv->buf->data[priv->copy_offset] == '\0')) {
		priv->copy_offset = 0;
		priv->buf->len = 0;
		priv->buf->owner = OCTEON_PCI_IO_BUF_OWNER_HOST;
		priv->eol = true;
		CVMX_SYNC;
	}

	return c;
}

static const struct dm_serial_ops octeon_bootcmd_ops = {
	.getc = octeon_bootcmd_getc,
	.pending = octeon_bootcmd_pending,
};

static int octeon_bootcmd_probe(struct udevice *dev)
{
	struct octeon_bootcmd_priv *priv = dev_get_priv(dev);

	priv->buf = (void *)CKSEG0ADDR(BOOTLOADER_PCI_READ_BUFFER_BASE);
	memset(priv->buf, 0, BOOTLOADER_PCI_READ_BUFFER_SIZE);
	priv->eol = false;

	/*
	 * When the bootcmd console is first started it is started as locked to
	 * block any calls sending a command until U-Boot is ready to accept
	 * commands.  Just before the main loop starts to accept commands the
	 * bootcmd console is unlocked.
	 */
	if (priv->unlocked)
		priv->buf->owner = OCTEON_PCI_IO_BUF_OWNER_HOST;
	else
		priv->buf->owner = OCTEON_PCI_IO_BUF_OWNER_OCTEON;

	debug("%s called, buffer ptr: 0x%p, owner: %s\n", __func__,
	      priv->buf,
	      priv->buf->owner == OCTEON_PCI_IO_BUF_OWNER_HOST ?
	      "host" : "octeon");
	debug("&priv->copy_offset: 0x%p\n", &priv->copy_offset);
	CVMX_SYNC;

	/*
	 * Perhaps reinvestige this: In the original code, "unlocked" etc
	 * is set in the octeon_pci_bootcmd_unlock() function called very
	 * late.
	 */
	priv->buf->owner = OCTEON_PCI_IO_BUF_OWNER_HOST;
	priv->unlocked = true;
	priv->started = true;
	CVMX_SYNC;

	return 0;
}

static const struct udevice_id octeon_bootcmd_serial_id[] = {
	{ .compatible = "marvell,pci-bootcmd", },
	{ },
};

U_BOOT_DRIVER(octeon_bootcmd) = {
	.name = DRIVER_NAME,
	.id = UCLASS_SERIAL,
	.ops = &octeon_bootcmd_ops,
	.of_match = of_match_ptr(octeon_bootcmd_serial_id),
	.probe = octeon_bootcmd_probe,
	.priv_auto = sizeof(struct octeon_bootcmd_priv),
};
