// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Broadcom
 */

#include <common.h>
#include <dm.h>
#include <generic-phy.h>
#include <asm/io.h>
#include <linux/bitops.h>

/* we have up to 8 PAXB based RC. The 9th one is always PAXC */
#define SR_NR_PCIE_PHYS               8

#define PCIE_PIPEMUX_CFG_OFFSET       0x10c
#define PCIE_PIPEMUX_SELECT_STRAP     GENMASK(3, 0)

#define CDRU_STRAP_DATA_LSW_OFFSET    0x5c
#define PCIE_PIPEMUX_SHIFT            19
#define PCIE_PIPEMUX_MASK             GENMASK(3, 0)

/**
 * struct sr_pcie_phy_core - Stingray PCIe PHY core control
 *
 * @dev: pointer to device
 * @base: base register of PCIe SS
 * @cdru: CDRU base address
 * @pipemux: pipemuex strap
 */
struct sr_pcie_phy_core {
	struct udevice *dev;
	void __iomem *base;
	void __iomem *cdru;
	u32 pipemux;
};

/*
 * PCIe PIPEMUX lookup table
 *
 * Each array index represents a PIPEMUX strap setting
 * The array element represents a bitmap where a set bit means the PCIe
 * core and associated serdes has been enabled as RC and is available for use
 */
static const u8 pipemux_table[] = {
	/* PIPEMUX = 0, EP 1x16 */
	0x00,
	/* PIPEMUX = 1, EP 1x8 + RC 1x8, core 7 */
	0x80,
	/* PIPEMUX = 2, EP 4x4 */
	0x00,
	/* PIPEMUX = 3, RC 2x8, cores 0, 7 */
	0x81,
	/* PIPEMUX = 4, RC 4x4, cores 0, 1, 6, 7 */
	0xc3,
	/* PIPEMUX = 5, RC 8x2, all 8 cores */
	0xff,
	/* PIPEMUX = 6, RC 3x4 + 2x2, cores 0, 2, 3, 6, 7 */
	0xcd,
	/* PIPEMUX = 7, RC 1x4 + 6x2, cores 0, 2, 3, 4, 5, 6, 7 */
	0xfd,
	/* PIPEMUX = 8, EP 1x8 + RC 4x2, cores 4, 5, 6, 7 */
	0xf0,
	/* PIPEMUX = 9, EP 1x8 + RC 2x4, cores 6, 7 */
	0xc0,
	/* PIPEMUX = 10, EP 2x4 + RC 2x4, cores 1, 6 */
	0x42,
	/* PIPEMUX = 11, EP 2x4 + RC 4x2, cores 2, 3, 4, 5 */
	0x3c,
	/* PIPEMUX = 12, EP 1x4 + RC 6x2, cores 2, 3, 4, 5, 6, 7 */
	0xfc,
	/* PIPEMUX = 13, RC 2x4 + RC 1x4 + 2x2, cores 2, 3, 6 */
	0x4c,
};

/*
 * Return true if the strap setting is valid
 */
static bool pipemux_strap_is_valid(u32 pipemux)
{
	return !!(pipemux < ARRAY_SIZE(pipemux_table));
}

/*
 * Read the PCIe PIPEMUX from strap
 */
static u32 pipemux_strap_read(struct sr_pcie_phy_core *core)
{
	u32 pipemux;

	/*
	 * Read PIPEMUX configuration register to determine the pipemux setting
	 *
	 * In the case when the value indicates using HW strap, fall back to
	 * use HW strap
	 */
	pipemux = readl(core->base + PCIE_PIPEMUX_CFG_OFFSET);
	pipemux &= PCIE_PIPEMUX_MASK;
	if (pipemux == PCIE_PIPEMUX_SELECT_STRAP) {
		pipemux = readl(core->cdru + CDRU_STRAP_DATA_LSW_OFFSET);
		pipemux >>= PCIE_PIPEMUX_SHIFT;
		pipemux &= PCIE_PIPEMUX_MASK;
	}

	return pipemux;
}

static int sr_pcie_phy_init(struct phy *phy)
{
	struct sr_pcie_phy_core *core = dev_get_priv(phy->dev);
	unsigned int core_idx = phy->id;

	debug("%s %lx\n", __func__, phy->id);
	/*
	 * Check whether this PHY is for root complex or not. If yes, return
	 * zero so the host driver can proceed to enumeration. If not, return
	 * an error and that will force the host driver to bail out
	 */
	if (!!((pipemux_table[core->pipemux] >> core_idx) & 0x1))
		return 0;

	return -ENODEV;
}

static int sr_pcie_phy_xlate(struct phy *phy, struct ofnode_phandle_args *args)
{
	debug("%s %d\n", __func__, args->args[0]);
	if (args->args_count && args->args[0] < SR_NR_PCIE_PHYS)
		phy->id = args->args[0];
	else
		return -ENODEV;

	return 0;
}

static const struct phy_ops sr_pcie_phy_ops = {
	.of_xlate = sr_pcie_phy_xlate,
	.init = sr_pcie_phy_init,
};

static int sr_pcie_phy_probe(struct udevice *dev)
{
	struct sr_pcie_phy_core *core = dev_get_priv(dev);

	core->dev = dev;

	core->base = (void __iomem *)dev_read_addr_name_ptr(dev, "reg_base");
	core->cdru = (void __iomem *)dev_read_addr_name_ptr(dev, "cdru_base");
	debug("ip base %p\n", core->base);
	debug("cdru base %p\n", core->cdru);

	/* read the PCIe PIPEMUX strap setting */
	core->pipemux = pipemux_strap_read(core);
	if (!pipemux_strap_is_valid(core->pipemux)) {
		pr_err("invalid PCIe PIPEMUX strap %u\n", core->pipemux);
		return -EIO;
	}
	debug("%s %#x\n", __func__, core->pipemux);

	pr_info("Stingray PCIe PHY driver initialized\n");

	return 0;
}

static const struct udevice_id sr_pcie_phy_match_table[] = {
	{ .compatible = "brcm,sr-pcie-phy" },
	{ }
};

U_BOOT_DRIVER(sr_pcie_phy) = {
	.name = "sr-pcie-phy",
	.id = UCLASS_PHY,
	.probe = sr_pcie_phy_probe,
	.of_match = sr_pcie_phy_match_table,
	.ops = &sr_pcie_phy_ops,
	.plat_auto	= sizeof(struct sr_pcie_phy_core),
	.priv_auto	= sizeof(struct sr_pcie_phy_core),
};
