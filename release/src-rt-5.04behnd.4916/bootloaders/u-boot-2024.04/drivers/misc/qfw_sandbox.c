// SPDX-License-Identifier: GPL-2.0+
/*
 * Sandbox interface for QFW
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

struct qfw_sandbox_plat {
	u8 file_dir_offset;
};

static void qfw_sandbox_read_entry_io(struct udevice *dev, u16 entry, u32 size,
				      void *address)
{
	debug("%s: entry 0x%x size %u address %p\n", __func__, entry, size,
	      address);

	switch (entry) {
	case FW_CFG_SIGNATURE:
		if (size == 4)
			*((u32 *)address) = cpu_to_be32(QEMU_FW_CFG_SIGNATURE);
		break;
	case FW_CFG_ID:
		/* Advertise DMA support */
		if (size == 1)
			*((u8 *)address) = FW_CFG_DMA_ENABLED;
		break;
	default:
		debug("%s got unsupported entry 0x%x\n", __func__, entry);
	/*
	 * Sandbox driver doesn't support other entries here, assume we use DMA
	 * to read them -- the uclass driver will exclusively use it when
	 * advertised.
	 */
	}
}

static void qfw_sandbox_read_entry_dma(struct udevice *dev, struct qfw_dma *dma)
{
	u16 entry;
	u32 control = be32_to_cpu(dma->control);
	void *address = (void *)(uintptr_t)be64_to_cpu(dma->address);
	u32 length = be32_to_cpu(dma->length);
	struct qfw_sandbox_plat *plat = dev_get_plat(dev);
	struct fw_cfg_file *file;

	debug("%s\n", __func__);

	if (!(control & FW_CFG_DMA_READ))
		return;

	if (control & FW_CFG_DMA_SELECT) {
		/* Start new read. */
		entry = control >> 16;

		/* Arbitrary values to be used by tests. */
		switch (entry) {
		case FW_CFG_NB_CPUS:
			if (length == 2)
				*((u16 *)address) = cpu_to_le16(5);
			break;
		case FW_CFG_FILE_DIR:
			if (length == 4) {
				*((u32 *)address) = cpu_to_be32(2);
				plat->file_dir_offset = 1;
			}
			break;
		default:
			debug("%s got unsupported entry 0x%x\n", __func__,
			      entry);
		}
	} else if (plat->file_dir_offset && length == 64) {
		file = address;
		switch (plat->file_dir_offset) {
		case 1:
			file->size = cpu_to_be32(8);
			file->select = cpu_to_be16(FW_CFG_FILE_FIRST);
			strcpy(file->name, "test-one");
			plat->file_dir_offset++;
			break;
		case 2:
			file->size = cpu_to_be32(8);
			file->select = cpu_to_be16(FW_CFG_FILE_FIRST + 1);
			strcpy(file->name, "test-two");
			plat->file_dir_offset++;
			break;
		}
	}

	/*
	 * Signal that we are finished. No-one checks this in sandbox --
	 * normally the platform-specific driver looks for it -- but let's
	 * replicate the behaviour in case someone relies on it later.
	 */
	dma->control = 0;
}

static int qfw_sandbox_probe(struct udevice *dev)
{
	return qfw_register(dev);
}

static struct dm_qfw_ops qfw_sandbox_ops = {
	.read_entry_io = qfw_sandbox_read_entry_io,
	.read_entry_dma = qfw_sandbox_read_entry_dma,
};

U_BOOT_DRIVER(qfw_sandbox) = {
	.name	= "qfw_sandbox",
	.id	= UCLASS_QFW,
	.plat_auto	= sizeof(struct qfw_sandbox_plat),
	.probe	= qfw_sandbox_probe,
	.ops	= &qfw_sandbox_ops,
};

U_BOOT_DRVINFO(qfw_sandbox) = {
	.name = "qfw_sandbox",
};
