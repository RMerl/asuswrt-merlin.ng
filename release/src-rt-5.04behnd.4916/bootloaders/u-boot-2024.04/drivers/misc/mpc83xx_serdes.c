// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 *
 * base on the MPC83xx serdes initialization, which is
 *
 * Copyright 2007,2011 Freescale Semiconductor, Inc.
 * Copyright (C) 2008 MontaVista Software, Inc.
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <mapmem.h>
#include <misc.h>
#include <linux/delay.h>

#include "mpc83xx_serdes.h"

/**
 * struct mpc83xx_serdes_priv - Private structure for MPC83xx serdes
 * @regs:  The device's register map
 * @rfcks: Variable to keep the serdes reference clock selection set during
 *	   initialization in (is or'd to every value written to SRDSCR4)
 */
struct mpc83xx_serdes_priv {
	struct mpc83xx_serdes_regs *regs;
	u32 rfcks;
};

/**
 * setup_sata() - Configure the SerDes device to SATA mode
 * @dev: The device to configure
 */
static void setup_sata(struct udevice *dev)
{
	struct mpc83xx_serdes_priv *priv = dev_get_priv(dev);

	/* Set and clear reset bits */
	setbits_be32(&priv->regs->srdsrstctl, SRDSRSTCTL_SATA_RESET);
	udelay(1000);
	clrbits_be32(&priv->regs->srdsrstctl, SRDSRSTCTL_SATA_RESET);

	/* Configure SRDSCR0 */
	clrsetbits_be32(&priv->regs->srdscr0,
			SRDSCR0_TXEQA_MASK | SRDSCR0_TXEQE_MASK,
			SRDSCR0_TXEQA_SATA | SRDSCR0_TXEQE_SATA);

	/* Configure SRDSCR1 */
	clrbits_be32(&priv->regs->srdscr1, SRDSCR1_PLLBW);

	/* Configure SRDSCR2 */
	clrsetbits_be32(&priv->regs->srdscr2,
			SRDSCR2_SEIC_MASK,
			SRDSCR2_SEIC_SATA);

	/* Configure SRDSCR3 */
	out_be32(&priv->regs->srdscr3,
		 SRDSCR3_KFR_SATA | SRDSCR3_KPH_SATA |
		 SRDSCR3_SDFM_SATA_PEX | SRDSCR3_SDTXL_SATA);

	/* Configure SRDSCR4 */
	out_be32(&priv->regs->srdscr4, priv->rfcks | SRDSCR4_PROT_SATA);
}

/**
 * setup_pex() - Configure the SerDes device to PCI Express mode
 * @dev:  The device to configure
 * @type: The PCI Express type to configure for (x1 or x2)
 */
static void setup_pex(struct udevice *dev, enum pex_type type)
{
	struct mpc83xx_serdes_priv *priv = dev_get_priv(dev);

	/* Configure SRDSCR1 */
	setbits_be32(&priv->regs->srdscr1, SRDSCR1_PLLBW);

	/* Configure SRDSCR2 */
	clrsetbits_be32(&priv->regs->srdscr2,
			SRDSCR2_SEIC_MASK,
			SRDSCR2_SEIC_PEX);

	/* Configure SRDSCR3 */
	out_be32(&priv->regs->srdscr3, SRDSCR3_SDFM_SATA_PEX);

	/* Configure SRDSCR4 */
	if (type == PEX_X2)
		out_be32(&priv->regs->srdscr4,
			 priv->rfcks | SRDSCR4_PROT_PEX | SRDSCR4_PLANE_X2);
	else
		out_be32(&priv->regs->srdscr4,
			 priv->rfcks | SRDSCR4_PROT_PEX);
}

/**
 * setup_sgmii() - Configure the SerDes device to SGMII mode
 * @dev: The device to configure
 */
static void setup_sgmii(struct udevice *dev)
{
	struct mpc83xx_serdes_priv *priv = dev_get_priv(dev);

	/* Configure SRDSCR1 */
	clrbits_be32(&priv->regs->srdscr1, SRDSCR1_PLLBW);

	/* Configure SRDSCR2 */
	clrsetbits_be32(&priv->regs->srdscr2,
			SRDSCR2_SEIC_MASK,
			SRDSCR2_SEIC_SGMII);

	/* Configure SRDSCR3 */
	out_be32(&priv->regs->srdscr3, 0);

	/* Configure SRDSCR4 */
	out_be32(&priv->regs->srdscr4, priv->rfcks | SRDSCR4_PROT_SGMII);
}

static int mpc83xx_serdes_probe(struct udevice *dev)
{
	struct mpc83xx_serdes_priv *priv = dev_get_priv(dev);
	bool vdd;
	const char *proto;

	priv->regs = map_sysmem(dev_read_addr(dev),
				sizeof(struct mpc83xx_serdes_regs));

	switch (dev_read_u32_default(dev, "serdes-clk", -1)) {
	case 100:
		priv->rfcks = SRDSCR4_RFCKS_100;
		break;
	case 125:
		priv->rfcks = SRDSCR4_RFCKS_125;
		break;
	case 150:
		priv->rfcks = SRDSCR4_RFCKS_150;
		break;
	default:
		debug("%s: Could not read serdes clock value\n", dev->name);
		return -EINVAL;
	}

	vdd = dev_read_bool(dev, "vdd");

	/* 1.0V corevdd */
	if (vdd) {
		/* DPPE/DPPA = 0 */
		clrbits_be32(&priv->regs->srdscr0, SRDSCR0_DPP_1V2);

		/* VDD = 0 */
		clrbits_be32(&priv->regs->srdscr0, SRDSCR2_VDD_1V2);
	}

	proto = dev_read_string(dev, "proto");

	/* protocol specific configuration */
	if (!strcmp(proto, "sata")) {
		setup_sata(dev);
	} else if (!strcmp(proto, "pex")) {
		setup_pex(dev, PEX_X1);
	} else if (!strcmp(proto, "pex-x2")) {
		setup_pex(dev, PEX_X2);
	} else if (!strcmp(proto, "sgmii")) {
		setup_sgmii(dev);
	} else {
		debug("%s: Invalid protocol value %s\n", dev->name, proto);
		return -EINVAL;
	}

	/* Do a software reset */
	setbits_be32(&priv->regs->srdsrstctl, SRDSRSTCTL_RST);

	return 0;
}

static const struct udevice_id mpc83xx_serdes_ids[] = {
	{ .compatible = "fsl,mpc83xx-serdes" },
	{ }
};

U_BOOT_DRIVER(mpc83xx_serdes) = {
	.name           = "mpc83xx_serdes",
	.id             = UCLASS_MISC,
	.of_match       = mpc83xx_serdes_ids,
	.probe          = mpc83xx_serdes_probe,
	.priv_auto	= sizeof(struct mpc83xx_serdes_priv),
};
