// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <bitfield.h>
#include <clk.h>
#include <cpu.h>
#include <dm.h>
#include <log.h>
#include <vsprintf.h>
#include <linux/bitops.h>

#include "mpc83xx_cpu.h"

/**
 * struct mpc83xx_cpu_priv - Private data for MPC83xx CPUs
 * @e300_type:      The e300 core type of the MPC83xx CPU
 * @family:         The MPC83xx family the CPU belongs to
 * @type:           The MPC83xx type of the CPU
 * @is_e_processor: Flag indicating whether the CPU is a E processor or not
 * @is_a_variant:   Flag indicating whtther the CPU is a A variant or not
 * @revid:          The revision ID of the CPU
 * @revid.major:    The major part of the CPU's revision ID
 * @revid.minor:    The minor part of the CPU's revision ID
 */
struct mpc83xx_cpu_priv {
	enum e300_type e300_type;
	enum mpc83xx_cpu_family family;
	enum mpc83xx_cpu_type type;
	bool is_e_processor;
	bool is_a_variant;
	struct {
		uint major;
		uint minor;
	} revid;
};

int checkcpu(void)
{
	/* Activate all CPUs  from board_f.c */
	return cpu_probe_all();
}

/**
 * get_spridr() - Read SPRIDR (System Part and Revision ID Register) of CPU
 *
 * Return: The SPRIDR value
 */
static inline u32 get_spridr(void)
{
	immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;

	return in_be32(&immr->sysconf.spridr);
}

/**
 * determine_type() - Determine CPU family of MPC83xx device
 * @dev: CPU device from which to read CPU family from
 */
static inline void determine_family(const struct udevice *dev)
{
	struct mpc83xx_cpu_priv *priv = dev_get_priv(dev);
	/* Upper 12 bits of PARTID field (bits 0-23 in SPRIDR) */
	const u32 PARTID_FAMILY_MASK = 0xFFF00000;

	switch (bitfield_extract_by_mask(get_spridr(), PARTID_FAMILY_MASK)) {
	case 0x810:
	case 0x811:
		priv->family = FAMILY_830X;
		break;
	case 0x80B:
		priv->family = FAMILY_831X;
		break;
	case 0x806:
		priv->family = FAMILY_832X;
		break;
	case 0x803:
		priv->family = FAMILY_834X;
		break;
	case 0x804:
		priv->family = FAMILY_836X;
		break;
	case 0x80C:
		priv->family = FAMILY_837X;
		break;
	default:
		priv->family = FAMILY_UNKNOWN;
	}
}

/**
 * determine_type() - Determine CPU type of MPC83xx device
 * @dev: CPU device from which to read CPU type from
 */
static inline void determine_type(const struct udevice *dev)
{
	struct mpc83xx_cpu_priv *priv = dev_get_priv(dev);
	/* Upper 16 bits of PVR (Processor Version Register) */
	const u32 PCR_UPPER_MASK = 0xFFFF0000;
	u32 val;

	val = bitfield_extract_by_mask(get_spridr(), PCR_UPPER_MASK);

	/* Mask out E-variant bit */
	switch (val & 0xFFFE) {
	case 0x8100:
		priv->type = TYPE_8308;
		break;
	case 0x8110:
		priv->type = TYPE_8309;
		break;
	case 0x80B2:
		priv->type = TYPE_8311;
		break;
	case 0x80B0:
		priv->type = TYPE_8313;
		break;
	case 0x80B6:
		priv->type = TYPE_8314;
		break;
	case 0x80B4:
		priv->type = TYPE_8315;
		break;
	case 0x8066:
		priv->type = TYPE_8321;
		break;
	case 0x8062:
		priv->type = TYPE_8323;
		break;
	case 0x8036:
		priv->type = TYPE_8343;
		break;
	case 0x8032:
		priv->type = TYPE_8347_TBGA;
		break;
	case 0x8034:
		priv->type = TYPE_8347_PBGA;
		break;
	case 0x8030:
		priv->type = TYPE_8349;
		break;
	case 0x804A:
		priv->type = TYPE_8358_TBGA;
		break;
	case 0x804E:
		priv->type = TYPE_8358_PBGA;
		break;
	case 0x8048:
		priv->type = TYPE_8360;
		break;
	case 0x80C6:
		priv->type = TYPE_8377;
		break;
	case 0x80C4:
		priv->type = TYPE_8378;
		break;
	case 0x80C2:
		priv->type = TYPE_8379;
		break;
	default:
		priv->type = TYPE_UNKNOWN;
	}
}

/**
 * determine_e300_type() - Determine e300 core type of MPC83xx device
 * @dev: CPU device from which to read e300 core type from
 */
static inline void determine_e300_type(const struct udevice *dev)
{
	struct mpc83xx_cpu_priv *priv = dev_get_priv(dev);
	/* Upper 16 bits of PVR (Processor Version Register) */
	const u32 PCR_UPPER_MASK = 0xFFFF0000;
	u32 pvr = get_pvr();

	switch ((pvr & PCR_UPPER_MASK) >> 16) {
	case 0x8083:
		priv->e300_type = E300C1;
		break;
	case 0x8084:
		priv->e300_type = E300C2;
		break;
	case 0x8085:
		priv->e300_type = E300C3;
		break;
	case 0x8086:
		priv->e300_type = E300C4;
		break;
	default:
		priv->e300_type = E300_UNKNOWN;
	}
}

/**
 * determine_revid() - Determine revision ID of CPU device
 * @dev: CPU device from which to read revision ID
 */
static inline void determine_revid(const struct udevice *dev)
{
	struct mpc83xx_cpu_priv *priv = dev_get_priv(dev);
	u32 REVID_MAJOR_MASK;
	u32 REVID_MINOR_MASK;
	u32 spridr = get_spridr();

	if (priv->family == FAMILY_834X) {
		REVID_MAJOR_MASK = 0x0000FF00;
		REVID_MINOR_MASK = 0x000000FF;
	} else {
		REVID_MAJOR_MASK = 0x000000F0;
		REVID_MINOR_MASK = 0x0000000F;
	}

	priv->revid.major = bitfield_extract_by_mask(spridr, REVID_MAJOR_MASK);
	priv->revid.minor = bitfield_extract_by_mask(spridr, REVID_MINOR_MASK);
}

/**
 * determine_cpu_data() - Determine CPU information from hardware
 * @dev: CPU device from which to read information
 */
static void determine_cpu_data(const struct udevice *dev)
{
	struct mpc83xx_cpu_priv *priv = dev_get_priv(dev);
	const u32 E_FLAG_MASK = 0x00010000;
	u32 spridr = get_spridr();

	determine_family(dev);
	determine_type(dev);
	determine_e300_type(dev);
	determine_revid(dev);

	if ((priv->family == FAMILY_834X ||
	     priv->family == FAMILY_836X) && priv->revid.major >= 2)
		priv->is_a_variant = true;

	priv->is_e_processor = !bitfield_extract_by_mask(spridr, E_FLAG_MASK);
}

static int mpc83xx_cpu_get_desc(const struct udevice *dev, char *buf, int size)
{
	struct mpc83xx_cpu_priv *priv = dev_get_priv(dev);
	struct clk core_clk;
	struct clk csb_clk;
	char core_freq[32];
	char csb_freq[32];
	int ret;

	ret = clk_get_by_index((struct udevice *)dev, 0, &core_clk);
	if (ret) {
		debug("%s: Failed to get core clock (err = %d)\n",
		      dev->name, ret);
		return ret;
	}

	ret = clk_get_by_index((struct udevice *)dev, 1, &csb_clk);
	if (ret) {
		debug("%s: Failed to get CSB clock (err = %d)\n",
		      dev->name, ret);
		return ret;
	}

	determine_cpu_data(dev);

	snprintf(buf, size,
		 "%s, MPC%s%s%s, Rev: %d.%d at %s MHz, CSB: %s MHz",
		 e300_names[priv->e300_type],
		 cpu_type_names[priv->type],
		 priv->is_e_processor ? "E" : "",
		 priv->is_a_variant ? "A" : "",
		 priv->revid.major,
		 priv->revid.minor,
		 strmhz(core_freq, clk_get_rate(&core_clk)),
		 strmhz(csb_freq, clk_get_rate(&csb_clk)));

	return 0;
}

static int mpc83xx_cpu_get_info(const struct udevice *dev,
				struct cpu_info *info)
{
	struct clk clock;
	int ret;
	ulong freq;

	ret = clk_get_by_index((struct udevice *)dev, 0, &clock);
	if (ret) {
		debug("%s: Failed to get core clock (err = %d)\n",
		      dev->name, ret);
		return ret;
	}

	freq = clk_get_rate(&clock);
	if (!freq) {
		debug("%s: Core clock speed is zero\n", dev->name);
		return -EINVAL;
	}

	info->cpu_freq = freq;
	info->features = BIT(CPU_FEAT_L1_CACHE) | BIT(CPU_FEAT_MMU);

	return 0;
}

static int mpc83xx_cpu_get_count(const struct udevice *dev)
{
	/* We have one e300cX core */
	return 1;
}

static int mpc83xx_cpu_get_vendor(const struct udevice *dev, char *buf,
				  int size)
{
	snprintf(buf, size, "NXP");

	return 0;
}

static const struct cpu_ops mpc83xx_cpu_ops = {
	.get_desc = mpc83xx_cpu_get_desc,
	.get_info = mpc83xx_cpu_get_info,
	.get_count = mpc83xx_cpu_get_count,
	.get_vendor = mpc83xx_cpu_get_vendor,
};

static int mpc83xx_cpu_probe(struct udevice *dev)
{
	return 0;
}

static const struct udevice_id mpc83xx_cpu_ids[] = {
	{ .compatible = "fsl,mpc83xx", },
	{ .compatible = "fsl,mpc8308", },
	{ .compatible = "fsl,mpc8309", },
	{ .compatible = "fsl,mpc8313", },
	{ .compatible = "fsl,mpc8315", },
	{ .compatible = "fsl,mpc832x", },
	{ .compatible = "fsl,mpc8349", },
	{ .compatible = "fsl,mpc8360", },
	{ .compatible = "fsl,mpc8379", },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(mpc83xx_cpu) = {
	.name = "mpc83xx_cpu",
	.id = UCLASS_CPU,
	.of_match = mpc83xx_cpu_ids,
	.probe = mpc83xx_cpu_probe,
	.priv_auto	= sizeof(struct mpc83xx_cpu_priv),
	.ops = &mpc83xx_cpu_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
