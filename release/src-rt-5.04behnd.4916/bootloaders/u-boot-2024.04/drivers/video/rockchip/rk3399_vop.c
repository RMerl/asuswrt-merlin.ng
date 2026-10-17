// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2017 Theobroma Systems Design und Consulting GmbH
 * Copyright (c) 2015 Google, Inc
 * Copyright 2014 Rockchip Inc.
 */

#include <common.h>
#include <display.h>
#include <dm.h>
#include <log.h>
#include <regmap.h>
#include <video.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include "rk_vop.h"

DECLARE_GLOBAL_DATA_PTR;

static void rk3399_set_pin_polarity(struct udevice *dev,
				    enum vop_modes mode, u32 polarity)
{
	struct rk_vop_priv *priv = dev_get_priv(dev);
	struct rk3288_vop *regs = priv->regs;

	/*
	 * The RK3399 VOPs (v3.5 and v3.6) require a per-mode setting of
	 * the polarity configuration (in ctrl1).
	 */
	switch (mode) {
	case VOP_MODE_HDMI:
		clrsetbits_le32(&regs->dsp_ctrl1,
				M_RK3399_DSP_HDMI_POL,
				V_RK3399_DSP_HDMI_POL(polarity));
		break;

	case VOP_MODE_EDP:
		clrsetbits_le32(&regs->dsp_ctrl1,
				M_RK3399_DSP_EDP_POL,
				V_RK3399_DSP_EDP_POL(polarity));
		break;

	case VOP_MODE_MIPI:
		clrsetbits_le32(&regs->dsp_ctrl1,
				M_RK3399_DSP_MIPI_POL,
				V_RK3399_DSP_MIPI_POL(polarity));
		break;

	default:
		debug("%s: unsupported output mode %x\n", __func__, mode);
	}
}

/*
 * Try some common regulators. We should really get these from the
 * device tree somehow.
 */
static const char * const rk3399_regulator_names[] = {
	"vcc33_lcd"
};

static int rk3399_vop_probe(struct udevice *dev)
{
	/* Before relocation we don't need to do anything */
	if (!(gd->flags & GD_FLG_RELOC))
		return 0;

	/* Probe regulators required for the RK3399 VOP */
	rk_vop_probe_regulators(dev, rk3399_regulator_names,
				ARRAY_SIZE(rk3399_regulator_names));

	return rk_vop_probe(dev);
}

struct rkvop_driverdata rk3399_lit_driverdata = {
	.set_pin_polarity = rk3399_set_pin_polarity,
};

struct rkvop_driverdata rk3399_big_driverdata = {
	.features = VOP_FEATURE_OUTPUT_10BIT,
	.set_pin_polarity = rk3399_set_pin_polarity,
};

static const struct udevice_id rk3399_vop_ids[] = {
	{ .compatible = "rockchip,rk3399-vop-big",
	  .data = (ulong)&rk3399_big_driverdata },
	{ .compatible = "rockchip,rk3399-vop-lit",
	  .data = (ulong)&rk3399_lit_driverdata },
	{ }
};

static const struct video_ops rk3399_vop_ops = {
};

U_BOOT_DRIVER(rk3399_vop) = {
	.name	= "rk3399_vop",
	.id	= UCLASS_VIDEO,
	.of_match = rk3399_vop_ids,
	.ops	= &rk3399_vop_ops,
	.bind	= rk_vop_bind,
	.probe	= rk3399_vop_probe,
	.priv_auto	= sizeof(struct rk_vop_priv),
};
