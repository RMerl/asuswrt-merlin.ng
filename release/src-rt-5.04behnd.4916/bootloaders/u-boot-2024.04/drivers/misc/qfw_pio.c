// SPDX-License-Identifier: GPL-2.0+
/*
 * PIO interface for QFW
 *
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 * (C) Copyright 2021 Asherah Connor <ashe@kivikakk.ee>
 */

#define LOG_CATEGORY UCLASS_QFW

#include <asm/io.h>
#include <dm/device.h>
#include <qfw.h>

/*
 * PIO ports are correct for x86, which appears to be the only arch that uses
 * PIO.
 */
#define FW_CONTROL_PORT      0x510
#define FW_DATA_PORT         0x511
#define FW_DMA_PORT_LOW      0x514
#define FW_DMA_PORT_HIGH     0x518

static void qfw_pio_read_entry_io(struct udevice *dev, u16 entry, u32 size,
				  void *address)
{
	/*
	 * writing FW_CFG_INVALID will cause read operation to resume at last
	 * offset, otherwise read will start at offset 0
	 *
	 * Note: on platform where the control register is IO port, the
	 * endianness is little endian.
	 */
	if (entry != FW_CFG_INVALID)
		outw(cpu_to_le16(entry), FW_CONTROL_PORT);

	/* the endianness of data register is string-preserving */
	u32 i = 0;
	u8 *data = address;

	while (size--)
		data[i++] = inb(FW_DATA_PORT);
}

/* Read configuration item using fw_cfg DMA interface */
static void qfw_pio_read_entry_dma(struct udevice *dev, struct qfw_dma *dma)
{
	/* the DMA address register is big-endian */
	outl(cpu_to_be32((uintptr_t)dma), FW_DMA_PORT_HIGH);

	while (be32_to_cpu(dma->control) & ~FW_CFG_DMA_ERROR);
}

static int qfw_pio_probe(struct udevice *dev)
{
	return qfw_register(dev);
}

static struct dm_qfw_ops qfw_pio_ops = {
	.read_entry_io = qfw_pio_read_entry_io,
	.read_entry_dma = qfw_pio_read_entry_dma,
};

U_BOOT_DRIVER(qfw_pio) = {
	.name	= "qfw_pio",
	.id	= UCLASS_QFW,
	.probe	= qfw_pio_probe,
	.ops	= &qfw_pio_ops,
};
