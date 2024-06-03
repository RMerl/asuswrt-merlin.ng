// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2004-2006,2010 Freescale Semiconductor, Inc. All Rights Reserved.
 * Copyright (C) 2008 by Sascha Hauer <kernel@pengutronix.de>
 * Copyright (C) 2009 by Jan Weitzel Phytec Messtechnik GmbH,
 *                       <armlinux@phytec.de>
 */

#include <common.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>

#if	defined(CONFIG_MX23)
#define	DRIVE_OFFSET	0x200
#define	PULL_OFFSET	0x400
#elif	defined(CONFIG_MX28)
#define	DRIVE_OFFSET	0x300
#define	PULL_OFFSET	0x600
#else
#error "Please select CONFIG_MX23 or CONFIG_MX28"
#endif

/*
 * configures a single pad in the iomuxer
 */
int mxs_iomux_setup_pad(iomux_cfg_t pad)
{
	u32 reg, ofs, bp, bm;
	void *iomux_base = (void *)MXS_PINCTRL_BASE;
	struct mxs_register_32 *mxs_reg;

	/* muxsel */
	ofs = 0x100;
	ofs += PAD_BANK(pad) * 0x20 + PAD_PIN(pad) / 16 * 0x10;
	bp = PAD_PIN(pad) % 16 * 2;
	bm = 0x3 << bp;
	reg = readl(iomux_base + ofs);
	reg &= ~bm;
	reg |= PAD_MUXSEL(pad) << bp;
	writel(reg, iomux_base + ofs);

	/* drive */
	ofs = DRIVE_OFFSET;
	ofs += PAD_BANK(pad) * 0x40 + PAD_PIN(pad) / 8 * 0x10;
	/* mA */
	if (PAD_MA_VALID(pad)) {
		bp = PAD_PIN(pad) % 8 * 4;
		bm = 0x3 << bp;
		reg = readl(iomux_base + ofs);
		reg &= ~bm;
		reg |= PAD_MA(pad) << bp;
		writel(reg, iomux_base + ofs);
	}
	/* vol */
	if (PAD_VOL_VALID(pad)) {
		bp = PAD_PIN(pad) % 8 * 4 + 2;
		mxs_reg = (struct mxs_register_32 *)(iomux_base + ofs);
		if (PAD_VOL(pad))
			writel(1 << bp, &mxs_reg->reg_set);
		else
			writel(1 << bp, &mxs_reg->reg_clr);
	}

	/* pull */
	if (PAD_PULL_VALID(pad)) {
		ofs = PULL_OFFSET;
		ofs += PAD_BANK(pad) * 0x10;
		bp = PAD_PIN(pad);
		mxs_reg = (struct mxs_register_32 *)(iomux_base + ofs);
		if (PAD_PULL(pad))
			writel(1 << bp, &mxs_reg->reg_set);
		else
			writel(1 << bp, &mxs_reg->reg_clr);
	}

	return 0;
}

int mxs_iomux_setup_multiple_pads(const iomux_cfg_t *pad_list, unsigned count)
{
	const iomux_cfg_t *p = pad_list;
	int i;
	int ret;

	for (i = 0; i < count; i++) {
		ret = mxs_iomux_setup_pad(*p);
		if (ret)
			return ret;
		p++;
	}

	return 0;
}
