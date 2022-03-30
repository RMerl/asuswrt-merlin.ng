// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Google, Inc
 */
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <pch.h>
#include <asm/cpu.h>
#include <asm/intel_regs.h>
#include <asm/io.h>
#include <asm/lapic.h>
#include <asm/lpc_common.h>
#include <asm/pci.h>
#include <asm/arch/model_206ax.h>
#include <asm/arch/pch.h>
#include <asm/arch/sandybridge.h>

DECLARE_GLOBAL_DATA_PTR;

#define GPIO_BASE		0x48
#define BIOS_CTRL		0xdc

#define RCBA_AUDIO_CONFIG	0x2030
#define RCBA_AUDIO_CONFIG_HDA	BIT(31)
#define RCBA_AUDIO_CONFIG_MASK	0xfe

#ifndef CONFIG_HAVE_FSP
static int pch_revision_id = -1;
static int pch_type = -1;

/**
 * pch_silicon_revision() - Read silicon revision ID from the PCH
 *
 * @dev:	PCH device
 * @return silicon revision ID
 */
static int pch_silicon_revision(struct udevice *dev)
{
	u8 val;

	if (pch_revision_id < 0) {
		dm_pci_read_config8(dev, PCI_REVISION_ID, &val);
		pch_revision_id = val;
	}

	return pch_revision_id;
}

int pch_silicon_type(struct udevice *dev)
{
	u8 val;

	if (pch_type < 0) {
		dm_pci_read_config8(dev, PCI_DEVICE_ID + 1, &val);
		pch_type = val;
	}

	return pch_type;
}

/**
 * pch_silicon_supported() - Check if a certain revision is supported
 *
 * @dev:	PCH device
 * @type:	PCH type
 * @rev:	Minimum required resion
 * @return 0 if not supported, 1 if supported
 */
static int pch_silicon_supported(struct udevice *dev, int type, int rev)
{
	int cur_type = pch_silicon_type(dev);
	int cur_rev = pch_silicon_revision(dev);

	switch (type) {
	case PCH_TYPE_CPT:
		/* CougarPoint minimum revision */
		if (cur_type == PCH_TYPE_CPT && cur_rev >= rev)
			return 1;
		/* PantherPoint any revision */
		if (cur_type == PCH_TYPE_PPT)
			return 1;
		break;

	case PCH_TYPE_PPT:
		/* PantherPoint minimum revision */
		if (cur_type == PCH_TYPE_PPT && cur_rev >= rev)
			return 1;
		break;
	}

	return 0;
}

#define IOBP_RETRY 1000
static inline int iobp_poll(void)
{
	unsigned try = IOBP_RETRY;
	u32 data;

	while (try--) {
		data = readl(RCB_REG(IOBPS));
		if ((data & 1) == 0)
			return 1;
		udelay(10);
	}

	printf("IOBP timeout\n");
	return 0;
}

void pch_iobp_update(struct udevice *dev, u32 address, u32 andvalue,
		     u32 orvalue)
{
	u32 data;

	/* Set the address */
	writel(address, RCB_REG(IOBPIRI));

	/* READ OPCODE */
	if (pch_silicon_supported(dev, PCH_TYPE_CPT, PCH_STEP_B0))
		writel(IOBPS_RW_BX, RCB_REG(IOBPS));
	else
		writel(IOBPS_READ_AX, RCB_REG(IOBPS));
	if (!iobp_poll())
		return;

	/* Read IOBP data */
	data = readl(RCB_REG(IOBPD));
	if (!iobp_poll())
		return;

	/* Check for successful transaction */
	if ((readl(RCB_REG(IOBPS)) & 0x6) != 0) {
		printf("IOBP read 0x%08x failed\n", address);
		return;
	}

	/* Update the data */
	data &= andvalue;
	data |= orvalue;

	/* WRITE OPCODE */
	if (pch_silicon_supported(dev, PCH_TYPE_CPT, PCH_STEP_B0))
		writel(IOBPS_RW_BX, RCB_REG(IOBPS));
	else
		writel(IOBPS_WRITE_AX, RCB_REG(IOBPS));
	if (!iobp_poll())
		return;

	/* Write IOBP data */
	writel(data, RCB_REG(IOBPD));
	if (!iobp_poll())
		return;
}

static int bd82x6x_probe(struct udevice *dev)
{
	if (!(gd->flags & GD_FLG_RELOC))
		return 0;

	/* Cause the SATA device to do its init */
	uclass_first_device(UCLASS_AHCI, &dev);

	return 0;
}
#endif /* CONFIG_HAVE_FSP */

static int bd82x6x_pch_get_spi_base(struct udevice *dev, ulong *sbasep)
{
	u32 rcba;

	dm_pci_read_config32(dev, PCH_RCBA, &rcba);
	/* Bits 31-14 are the base address, 13-1 are reserved, 0 is enable */
	rcba = rcba & 0xffffc000;
	*sbasep = rcba + 0x3800;

	return 0;
}

static int bd82x6x_set_spi_protect(struct udevice *dev, bool protect)
{
	return lpc_set_spi_protect(dev, BIOS_CTRL, protect);
}

static int bd82x6x_get_gpio_base(struct udevice *dev, u32 *gbasep)
{
	u32 base;

	/*
	 * GPIO_BASE moved to its current offset with ICH6, but prior to
	 * that it was unused (or undocumented). Check that it looks
	 * okay: not all ones or zeros.
	 *
	 * Note we don't need check bit0 here, because the Tunnel Creek
	 * GPIO base address register bit0 is reserved (read returns 0),
	 * while on the Ivybridge the bit0 is used to indicate it is an
	 * I/O space.
	 */
	dm_pci_read_config32(dev, GPIO_BASE, &base);
	if (base == 0x00000000 || base == 0xffffffff) {
		debug("%s: unexpected BASE value\n", __func__);
		return -ENODEV;
	}

	/*
	 * Okay, I guess we're looking at the right device. The actual
	 * GPIO registers are in the PCI device's I/O space, starting
	 * at the offset that we just read. Bit 0 indicates that it's
	 * an I/O address, not a memory address, so mask that off.
	 */
	*gbasep = base & 1 ? base & ~3 : base & ~15;

	return 0;
}

static int bd82x6x_ioctl(struct udevice *dev, enum pch_req_t req, void *data,
			 int size)
{
	u32 rcba, val;

	switch (req) {
	case PCH_REQ_HDA_CONFIG:
		dm_pci_read_config32(dev, PCH_RCBA, &rcba);
		val = readl(rcba + RCBA_AUDIO_CONFIG);
		if (!(val & RCBA_AUDIO_CONFIG_HDA))
			return -ENOENT;

		return val & RCBA_AUDIO_CONFIG_MASK;
	case PCH_REQ_PMBASE_INFO: {
		struct pch_pmbase_info *pm = data;
		int ret;

		/* Find the base address of the powermanagement registers */
		ret = dm_pci_read_config16(dev, 0x40, &pm->base);
		if (ret)
			return ret;
		pm->base &= 0xfffe;
		pm->gpio0_en_ofs = GPE0_EN;
		pm->pm1_sts_ofs = PM1_STS;
		pm->pm1_cnt_ofs = PM1_CNT;

		return 0;
	}
	default:
		return -ENOSYS;
	}
}

static const struct pch_ops bd82x6x_pch_ops = {
	.get_spi_base	= bd82x6x_pch_get_spi_base,
	.set_spi_protect = bd82x6x_set_spi_protect,
	.get_gpio_base	= bd82x6x_get_gpio_base,
	.ioctl		= bd82x6x_ioctl,
};

static const struct udevice_id bd82x6x_ids[] = {
	{ .compatible = "intel,bd82x6x" },
	{ }
};

U_BOOT_DRIVER(bd82x6x_drv) = {
	.name		= "bd82x6x",
	.id		= UCLASS_PCH,
	.of_match	= bd82x6x_ids,
#ifndef CONFIG_HAVE_FSP
	.probe		= bd82x6x_probe,
#endif
	.ops		= &bd82x6x_pch_ops,
};
