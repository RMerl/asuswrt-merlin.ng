// SPDX-License-Identifier: GPL-2.0+
/*
 * Uclass for Primary-to-sideband bus, used to access various peripherals
 *
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_P2SB

#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <p2sb.h>
#include <spl.h>
#include <asm/io.h>
#include <dm/uclass-internal.h>

#define PCR_COMMON_IOSF_1_0	1

int p2sb_set_hide(struct udevice *dev, bool hide)
{
	struct p2sb_ops *ops = p2sb_get_ops(dev);

	if (!ops->set_hide)
		return -ENOSYS;

	return ops->set_hide(dev, hide);
}

void *pcr_reg_address(struct udevice *dev, uint offset)
{
	struct p2sb_child_plat *pplat = dev_get_parent_plat(dev);
	struct udevice *p2sb = dev_get_parent(dev);
	struct p2sb_uc_priv *upriv = dev_get_uclass_priv(p2sb);
	uintptr_t reg_addr;

	/* Create an address based off of port id and offset */
	reg_addr = upriv->mmio_base;
	reg_addr += pplat->pid << PCR_PORTID_SHIFT;
	reg_addr += offset;

	return map_sysmem(reg_addr, 4);
}

/*
 * The mapping of addresses via the SBREG_BAR assumes the IOSF-SB
 * agents are using 32-bit aligned accesses for their configuration
 * registers. For IOSF versions greater than 1_0, IOSF-SB
 * agents can use any access (8/16/32 bit aligned) for their
 * configuration registers
 */
static inline void check_pcr_offset_align(uint offset, uint size)
{
	const size_t align = PCR_COMMON_IOSF_1_0 ? sizeof(uint32_t) : size;

	assert(IS_ALIGNED(offset, align));
}

uint pcr_read32(struct udevice *dev, uint offset)
{
	void *ptr;
	uint val;

	/* Ensure the PCR offset is correctly aligned */
	assert(IS_ALIGNED(offset, sizeof(uint32_t)));

	ptr = pcr_reg_address(dev, offset);
	val = readl(ptr);
	unmap_sysmem(ptr);

	return val;
}

uint pcr_read16(struct udevice *dev, uint offset)
{
	/* Ensure the PCR offset is correctly aligned */
	check_pcr_offset_align(offset, sizeof(uint16_t));

	return readw(pcr_reg_address(dev, offset));
}

uint pcr_read8(struct udevice *dev, uint offset)
{
	/* Ensure the PCR offset is correctly aligned */
	check_pcr_offset_align(offset, sizeof(uint8_t));

	return readb(pcr_reg_address(dev, offset));
}

/*
 * After every write one needs to perform a read an innocuous register to
 * ensure the writes are completed for certain ports. This is done for
 * all ports so that the callers don't need the per-port knowledge for
 * each transaction.
 */
static void write_completion(struct udevice *dev, uint offset)
{
	readl(pcr_reg_address(dev, ALIGN_DOWN(offset, sizeof(uint32_t))));
}

void pcr_write32(struct udevice *dev, uint offset, uint indata)
{
	/* Ensure the PCR offset is correctly aligned */
	assert(IS_ALIGNED(offset, sizeof(indata)));

	writel(indata, pcr_reg_address(dev, offset));
	/* Ensure the writes complete */
	write_completion(dev, offset);
}

void pcr_write16(struct udevice *dev, uint offset, uint indata)
{
	/* Ensure the PCR offset is correctly aligned */
	check_pcr_offset_align(offset, sizeof(uint16_t));

	writew(indata, pcr_reg_address(dev, offset));
	/* Ensure the writes complete */
	write_completion(dev, offset);
}

void pcr_write8(struct udevice *dev, uint offset, uint indata)
{
	/* Ensure the PCR offset is correctly aligned */
	check_pcr_offset_align(offset, sizeof(uint8_t));

	writeb(indata, pcr_reg_address(dev, offset));
	/* Ensure the writes complete */
	write_completion(dev, offset);
}

void pcr_clrsetbits32(struct udevice *dev, uint offset, uint clr, uint set)
{
	uint data32;

	data32 = pcr_read32(dev, offset);
	data32 &= ~clr;
	data32 |= set;
	pcr_write32(dev, offset, data32);
}

void pcr_clrsetbits16(struct udevice *dev, uint offset, uint clr, uint set)
{
	uint data16;

	data16 = pcr_read16(dev, offset);
	data16 &= ~clr;
	data16 |= set;
	pcr_write16(dev, offset, data16);
}

void pcr_clrsetbits8(struct udevice *dev, uint offset, uint clr, uint set)
{
	uint data8;

	data8 = pcr_read8(dev, offset);
	data8 &= ~clr;
	data8 |= set;
	pcr_write8(dev, offset, data8);
}

int p2sb_get_port_id(struct udevice *dev)
{
	struct p2sb_child_plat *pplat = dev_get_parent_plat(dev);

	return pplat->pid;
}

int p2sb_set_port_id(struct udevice *dev, int portid)
{
	struct p2sb_child_plat *pplat;

	if (!CONFIG_IS_ENABLED(OF_PLATDATA))
		return -ENOSYS;

	pplat = dev_get_parent_plat(dev);
	pplat->pid = portid;

	return 0;
}

static int p2sb_child_post_bind(struct udevice *dev)
{
	if (CONFIG_IS_ENABLED(OF_REAL)) {
		struct p2sb_child_plat *pplat = dev_get_parent_plat(dev);
		int ret;
		u32 pid;

		ret = dev_read_u32(dev, "intel,p2sb-port-id", &pid);
		if (ret)
			return ret;
		pplat->pid = pid;
	}

	return 0;
}

static int p2sb_post_bind(struct udevice *dev)
{
	if (spl_phase() > PHASE_TPL && !CONFIG_IS_ENABLED(OF_PLATDATA))
		return dm_scan_fdt_dev(dev);

	return 0;
}

UCLASS_DRIVER(p2sb) = {
	.id		= UCLASS_P2SB,
	.name		= "p2sb",
	.per_device_auto	= sizeof(struct p2sb_uc_priv),
	.post_bind	= p2sb_post_bind,
	.child_post_bind = p2sb_child_post_bind,
	.per_child_plat_auto = sizeof(struct p2sb_child_plat),
};
