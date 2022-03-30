// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Theobroma Systems Design und Consulting GmbH
 * Copyright (c) 2015 Google, Inc
 * Copyright 2014 Rockchip Inc.
 */

#include <common.h>
#include <display.h>
#include <dm.h>
#include <regmap.h>
#include <syscon.h>
#include <video.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/grf_rk3288.h>
#include <asm/arch-rockchip/hardware.h>
#include "rk_vop.h"

DECLARE_GLOBAL_DATA_PTR;

static void rk3288_set_pin_polarity(struct udevice *dev,
				    enum vop_modes mode, u32 polarity)
{
	struct rk_vop_priv *priv = dev_get_priv(dev);
	struct rk3288_vop *regs = priv->regs;

	/* The RK3328 VOP (v3.1) has its polarity configuration in ctrl0 */
	clrsetbits_le32(&regs->dsp_ctrl0,
			M_DSP_DCLK_POL | M_DSP_DEN_POL |
			M_DSP_VSYNC_POL | M_DSP_HSYNC_POL,
			V_DSP_PIN_POL(polarity));
}

static void rk3288_set_io_vsel(struct udevice *dev)
{
	struct rk3288_grf *grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);

	/* lcdc(vop) iodomain select 1.8V */
	rk_setreg(&grf->io_vsel, 1 << 0);
}

/*
 * Try some common regulators. We should really get these from the
 * device tree somehow.
 */
static const char * const rk3288_regulator_names[] = {
	"vcc18_lcd",
	"VCC18_LCD",
	"vdd10_lcd_pwren_h",
	"vdd10_lcd",
	"VDD10_LCD",
	"vcc33_lcd"
};

static int rk3288_vop_probe(struct udevice *dev)
{
	/* Before relocation we don't need to do anything */
	if (!(gd->flags & GD_FLG_RELOC))
		return 0;

	/* Set the LCDC(vop) iodomain to 1.8V */
	rk3288_set_io_vsel(dev);

	/* Probe regulators required for the RK3288 VOP */
	rk_vop_probe_regulators(dev, rk3288_regulator_names,
				ARRAY_SIZE(rk3288_regulator_names));

	return rk_vop_probe(dev);
}

static int rk_vop_remove(struct udevice *dev)
{
	struct rk_vop_priv *priv = dev_get_priv(dev);
        struct rk3288_vop *regs = priv->regs;

	setbits_le32(&regs->sys_ctrl, V_STANDBY_EN(1));

	/* wait frame complete (60Hz) to enter standby */
	mdelay(17);

	return 0;
}

struct rkvop_driverdata rk3288_driverdata = {
	.features = VOP_FEATURE_OUTPUT_10BIT,
	.set_pin_polarity = rk3288_set_pin_polarity,
};

static const struct udevice_id rk3288_vop_ids[] = {
	{ .compatible = "rockchip,rk3288-vop",
	  .data = (ulong)&rk3288_driverdata },
	{ }
};

static const struct video_ops rk3288_vop_ops = {
};

U_BOOT_DRIVER(rk_vop) = {
	.name	= "rk3288_vop",
	.id	= UCLASS_VIDEO,
	.of_match = rk3288_vop_ids,
	.ops	= &rk3288_vop_ops,
	.bind	= rk_vop_bind,
	.probe	= rk3288_vop_probe,
        .remove = rk_vop_remove,
	.priv_auto_alloc_size	= sizeof(struct rk_vop_priv),
};
