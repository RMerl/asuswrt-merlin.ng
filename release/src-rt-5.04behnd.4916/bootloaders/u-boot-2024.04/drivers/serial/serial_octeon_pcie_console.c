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
#include <asm/io.h>
#include <mach/cvmx-regs.h>
#include <mach/cvmx-bootmem.h>

#define DRIVER_NAME				"pci-console"
#define OCTEONTX_PCIE_CONSOLE_NAME_LEN		16

/* Current versions */
#define OCTEON_PCIE_CONSOLE_MAJOR_VERSION	1
#define OCTEON_PCIE_CONSOLE_MINOR_VERSION	0

#define OCTEON_PCIE_CONSOLE_BLOCK_NAME		"__pci_console"

/*
 * Structure that defines a single console.
 * Note: when read_index == write_index, the buffer is empty.
 * The actual usable size of each console is console_buf_size -1;
 */
struct octeon_pcie_console {
	u64 input_base_addr;
	u32 input_read_index;
	u32 input_write_index;
	u64 output_base_addr;
	u32 output_read_index;
	u32 output_write_index;
	u32 lock;
	u32 buf_size;
};

/*
 * This is the main container structure that contains all the information
 * about all PCI consoles. The address of this structure is passed to various
 * routines that operation on PCI consoles.
 */
struct octeon_pcie_console_desc {
	u32 major_version;
	u32 minor_version;
	u32 lock;
	u32 flags;
	u32 num_consoles;
	u32 pad;
	/* must be 64 bit aligned here... */
	/* Array of addresses of octeon_pcie_console_t structures */
	u64 console_addr_array[0];
	/* Implicit storage for console_addr_array */
};

struct octeon_pcie_console_priv {
	struct octeon_pcie_console *console;
	int console_num;
	bool console_active;
};

/* Flag definitions for read/write functions */
enum {
	/*
	 * If set, read/write functions won't block waiting for space or data.
	 * For reads, 0 bytes may be read, and for writes not all of the
	 * supplied data may be written.
	 */
	OCT_PCI_CON_FLAG_NONBLOCK = 1 << 0,
};

static int buffer_free_bytes(u32 buffer_size, u32 wr_idx, u32 rd_idx)
{
	if (rd_idx >= buffer_size || wr_idx >= buffer_size)
		return -1;

	return ((buffer_size - 1) - (wr_idx - rd_idx)) % buffer_size;
}

static int buffer_avail_bytes(u32 buffer_size, u32 wr_idx, u32 rd_idx)
{
	if (rd_idx >= buffer_size || wr_idx >= buffer_size)
		return -1;

	return buffer_size - 1 - buffer_free_bytes(buffer_size, wr_idx, rd_idx);
}

static int buffer_read_avail(struct udevice *dev, unsigned int console_num)
{
	struct octeon_pcie_console_priv *priv = dev_get_priv(dev);
	struct octeon_pcie_console *cons_ptr = priv->console;
	int avail;

	avail = buffer_avail_bytes(cons_ptr->buf_size,
				   cons_ptr->input_write_index,
				   cons_ptr->input_read_index);
	if (avail >= 0)
		return avail;

	return 0;
}

static int octeon_pcie_console_read(struct udevice *dev,
				    unsigned int console_num, char *buffer,
				    int buffer_size, u32 flags)
{
	struct octeon_pcie_console_priv *priv = dev_get_priv(dev);
	struct octeon_pcie_console *cons_ptr = priv->console;
	int avail;
	char *buf_ptr;
	int bytes_read;
	int read_size;

	buf_ptr = (char *)cvmx_phys_to_ptr(cons_ptr->input_base_addr);

	avail =	buffer_avail_bytes(cons_ptr->buf_size,
				   cons_ptr->input_write_index,
				   cons_ptr->input_read_index);
	if (avail < 0)
		return avail;

	if (!(flags & OCT_PCI_CON_FLAG_NONBLOCK)) {
		/* Wait for some data to be available */
		while (0 == (avail = buffer_avail_bytes(cons_ptr->buf_size,
							cons_ptr->input_write_index,
							cons_ptr->input_read_index))) {
			mdelay(10);
			schedule();
		}
	}

	bytes_read = 0;

	/* Don't overflow the buffer passed to us */
	read_size = min_t(int, avail, buffer_size);

	/* Limit ourselves to what we can input in a contiguous block */
	if (cons_ptr->input_read_index + read_size >= cons_ptr->buf_size)
		read_size = cons_ptr->buf_size - cons_ptr->input_read_index;

	memcpy(buffer, buf_ptr + cons_ptr->input_read_index, read_size);
	cons_ptr->input_read_index =
		(cons_ptr->input_read_index + read_size) % cons_ptr->buf_size;
	bytes_read += read_size;

	/* Mark the PCIe console to be active from now on */
	if (bytes_read)
		priv->console_active = true;

	return bytes_read;
}

static int octeon_pcie_console_write(struct udevice *dev,
				     unsigned int console_num,
				     const char *buffer,
				     int bytes_to_write, u32 flags)
{
	struct octeon_pcie_console_priv *priv = dev_get_priv(dev);
	struct octeon_pcie_console *cons_ptr = priv->console;
	int avail;
	char *buf_ptr;
	int bytes_written;

	buf_ptr = (char *)cvmx_phys_to_ptr(cons_ptr->output_base_addr);
	bytes_written = 0;
	while (bytes_to_write > 0) {
		avail = buffer_free_bytes(cons_ptr->buf_size,
					  cons_ptr->output_write_index,
					  cons_ptr->output_read_index);

		if (avail > 0) {
			int write_size = min_t(int, avail, bytes_to_write);

			/*
			 * Limit ourselves to what we can output in a contiguous
			 * block
			 */
			if (cons_ptr->output_write_index + write_size >=
			    cons_ptr->buf_size) {
				write_size = cons_ptr->buf_size -
					     cons_ptr->output_write_index;
			}

			memcpy(buf_ptr + cons_ptr->output_write_index,
			       buffer + bytes_written, write_size);
			/*
			 * Make sure data is visible before changing write
			 * index
			 */
			CVMX_SYNCW;
			cons_ptr->output_write_index =
				(cons_ptr->output_write_index + write_size) %
				cons_ptr->buf_size;
			bytes_to_write -= write_size;
			bytes_written += write_size;
		} else if (avail == 0) {
			/*
			 * Check to see if we should wait for room, or return
			 * after a partial write
			 */
			if (flags & OCT_PCI_CON_FLAG_NONBLOCK)
				goto done;

			schedule();
			mdelay(10);	/* Delay if we are spinning */
		} else {
			bytes_written = -1;
			goto done;
		}
	}

done:
	return bytes_written;
}

static struct octeon_pcie_console_desc *octeon_pcie_console_init(int num_consoles,
								 int buffer_size)
{
	struct octeon_pcie_console_desc *cons_desc_ptr;
	struct octeon_pcie_console *cons_ptr;
	s64 addr;
	u64 avail_addr;
	int alloc_size;
	int i;

	/* Compute size required for pci console structure */
	alloc_size = num_consoles *
		(buffer_size * 2 + sizeof(struct octeon_pcie_console) +
		 sizeof(u64)) + sizeof(struct octeon_pcie_console_desc);

	/*
	 * Allocate memory for the consoles.  This must be in the range
	 * addresssible by the bootloader.
	 * Try to do so in a manner which minimizes fragmentation.  We try to
	 * put it at the top of DDR0 or bottom of DDR2 first, and only do
	 * generic allocation if those fail
	 */
	addr = cvmx_bootmem_phy_named_block_alloc(alloc_size,
						  OCTEON_DDR0_SIZE - alloc_size - 128,
						  OCTEON_DDR0_SIZE, 128,
						  OCTEON_PCIE_CONSOLE_BLOCK_NAME,
						  CVMX_BOOTMEM_FLAG_END_ALLOC);
	if (addr < 0) {
		addr = cvmx_bootmem_phy_named_block_alloc(alloc_size, 0,
							  0x1fffffff, 128,
							  OCTEON_PCIE_CONSOLE_BLOCK_NAME,
							  CVMX_BOOTMEM_FLAG_END_ALLOC);
	}
	if (addr < 0)
		return 0;

	cons_desc_ptr = cvmx_phys_to_ptr(addr);

	/* Clear entire alloc'ed memory */
	memset(cons_desc_ptr, 0, alloc_size);

	/* Initialize as locked until we are done */
	cons_desc_ptr->lock = 1;
	CVMX_SYNCW;
	cons_desc_ptr->num_consoles = num_consoles;
	cons_desc_ptr->flags = 0;
	cons_desc_ptr->major_version = OCTEON_PCIE_CONSOLE_MAJOR_VERSION;
	cons_desc_ptr->minor_version = OCTEON_PCIE_CONSOLE_MINOR_VERSION;

	avail_addr = addr + sizeof(struct octeon_pcie_console_desc) +
		num_consoles * sizeof(u64);

	for (i = 0; i < num_consoles; i++) {
		cons_desc_ptr->console_addr_array[i] = avail_addr;
		cons_ptr = (void *)cons_desc_ptr->console_addr_array[i];
		avail_addr += sizeof(struct octeon_pcie_console);
		cons_ptr->input_base_addr = avail_addr;
		avail_addr += buffer_size;
		cons_ptr->output_base_addr = avail_addr;
		avail_addr += buffer_size;
		cons_ptr->buf_size = buffer_size;
	}
	CVMX_SYNCW;
	cons_desc_ptr->lock = 0;

	return cvmx_phys_to_ptr(addr);
}

static int octeon_pcie_console_getc(struct udevice *dev)
{
	char c;

	octeon_pcie_console_read(dev, 0, &c, 1, 0);
	return c;
}

static int octeon_pcie_console_putc(struct udevice *dev, const char c)
{
	struct octeon_pcie_console_priv *priv = dev_get_priv(dev);

	if (priv->console_active)
		octeon_pcie_console_write(dev, 0, (char *)&c, 1, 0);

	return 0;
}

static int octeon_pcie_console_pending(struct udevice *dev, bool input)
{
	if (input) {
		udelay(100);
		return buffer_read_avail(dev, 0) > 0;
	}

	return 0;
}

static const struct dm_serial_ops octeon_pcie_console_ops = {
	.getc = octeon_pcie_console_getc,
	.putc = octeon_pcie_console_putc,
	.pending = octeon_pcie_console_pending,
};

static int octeon_pcie_console_probe(struct udevice *dev)
{
	struct octeon_pcie_console_priv *priv = dev_get_priv(dev);
	struct octeon_pcie_console_desc *cons_desc;
	int console_count;
	int console_size;
	int console_num;

	/*
	 * Currently only 1 console is supported. Perhaps we need to add
	 * a console nexus if more than one needs to be supported.
	 */
	console_count = 1;
	console_size = 1024;
	console_num = 0;

	cons_desc = octeon_pcie_console_init(console_count, console_size);
	priv->console =
		cvmx_phys_to_ptr(cons_desc->console_addr_array[console_num]);

	debug("PCI console init succeeded, %d consoles, %d bytes each\n",
	      console_count, console_size);

	return 0;
}

static const struct udevice_id octeon_pcie_console_serial_id[] = {
	{ .compatible = "marvell,pci-console", },
	{ },
};

U_BOOT_DRIVER(octeon_pcie_console) = {
	.name = DRIVER_NAME,
	.id = UCLASS_SERIAL,
	.ops = &octeon_pcie_console_ops,
	.of_match = of_match_ptr(octeon_pcie_console_serial_id),
	.probe = octeon_pcie_console_probe,
	.priv_auto = sizeof(struct octeon_pcie_console_priv),
};
