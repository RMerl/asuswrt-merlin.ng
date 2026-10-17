// SPDX-License-Identifier: GPL-2.0+
/*
 * MMIO interface for QFW
 *
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 * (C) Copyright 2021 Asherah Connor <ashe@kivikakk.ee>
 */

#define LOG_CATEGORY UCLASS_QFW

#include <asm/types.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/device.h>
#include <qfw.h>

struct qfw_mmio {
	/*
	 * Each access to the 64-bit data register can be 8/16/32/64 bits wide.
	 */
	union {
		u8 data8;
		u16 data16;
		u32 data32;
		u64 data64;
	};
	u16 selector;
	u8 padding[6];
	u64 dma;
};

struct qfw_mmio_plat {
	volatile struct qfw_mmio *mmio;
};

static void qfw_mmio_read_entry_io(struct udevice *dev, u16 entry, u32 size,
				   void *address)
{
	struct qfw_mmio_plat *plat = dev_get_plat(dev);

	/*
	 * writing FW_CFG_INVALID will cause read operation to resume at last
	 * offset, otherwise read will start at offset 0
	 *
	 * Note: on platform where the control register is MMIO, the register
	 * is big endian.
	 */
	if (entry != FW_CFG_INVALID)
		plat->mmio->selector = cpu_to_be16(entry);

	/* the endianness of data register is string-preserving */
	while (size >= 8) {
		*(u64 *)address = plat->mmio->data64;
		address += 8;
		size -= 8;
	}
	while (size >= 4) {
		*(u32 *)address = plat->mmio->data32;
		address += 4;
		size -= 4;
	}
	while (size >= 2) {
		*(u16 *)address = plat->mmio->data16;
		address += 2;
		size -= 2;
	}
	while (size >= 1) {
		*(u8 *)address = plat->mmio->data8;
		address += 1;
		size -= 1;
	}
}

/* Read configuration item using fw_cfg DMA interface */
static void qfw_mmio_read_entry_dma(struct udevice *dev, struct qfw_dma *dma)
{
	struct qfw_mmio_plat *plat = dev_get_plat(dev);

	/* the DMA address register is big-endian */
	plat->mmio->dma = cpu_to_be64((uintptr_t)dma);

	while (be32_to_cpu(dma->control) & ~FW_CFG_DMA_ERROR);
}

static int qfw_mmio_of_to_plat(struct udevice *dev)
{
	struct qfw_mmio_plat *plat = dev_get_plat(dev);

	plat->mmio = map_physmem(dev_read_addr(dev),
				 sizeof(struct qfw_mmio),
				 MAP_NOCACHE);

	return 0;
}

static int qfw_mmio_probe(struct udevice *dev)
{
	return qfw_register(dev);
}

static struct dm_qfw_ops qfw_mmio_ops = {
	.read_entry_io = qfw_mmio_read_entry_io,
	.read_entry_dma = qfw_mmio_read_entry_dma,
};

static const struct udevice_id qfw_mmio_ids[] = {
	{ .compatible = "qemu,fw-cfg-mmio" },
	{}
};

U_BOOT_DRIVER(qfw_mmio) = {
	.name	= "qfw_mmio",
	.id	= UCLASS_QFW,
	.of_match	= qfw_mmio_ids,
	.plat_auto	= sizeof(struct qfw_mmio_plat),
	.of_to_plat	= qfw_mmio_of_to_plat,
	.probe	= qfw_mmio_probe,
	.ops	= &qfw_mmio_ops,
};
