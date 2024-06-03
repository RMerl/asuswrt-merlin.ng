// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Theobroma Systems Design und Consulting GmbH
 * Copyright (c) 2015 Google, Inc
 * Copyright 2014 Rockchip Inc.
 */

#include <common.h>
#include <clk.h>
#include <display.h>
#include <dm.h>
#include <dw_hdmi.h>
#include <edid.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/hardware.h>
#include "rk_hdmi.h"
#include "rk_vop.h" /* for rk_vop_probe_regulators */

static const struct hdmi_phy_config rockchip_phy_config[] = {
	{
		.mpixelclock = 74250000,
		.sym_ctr = 0x8009, .term = 0x0004, .vlev_ctr = 0x0272,
	}, {
		.mpixelclock = 148500000,
		.sym_ctr = 0x802b, .term = 0x0004, .vlev_ctr = 0x028d,
	}, {
		.mpixelclock = 297000000,
		.sym_ctr = 0x8039, .term = 0x0005, .vlev_ctr = 0x028d,
	}, {
		.mpixelclock = 584000000,
		.sym_ctr = 0x8039, .term = 0x0000, .vlev_ctr = 0x019d,
	}, {
		.mpixelclock = ~0ul,
		.sym_ctr = 0x0000, .term = 0x0000, .vlev_ctr = 0x0000,
	}
};

static const struct hdmi_mpll_config rockchip_mpll_cfg[] = {
	{
		.mpixelclock = 40000000,
		.cpce = 0x00b3, .gmp = 0x0000, .curr = 0x0018,
	}, {
		.mpixelclock = 65000000,
		.cpce = 0x0072, .gmp = 0x0001, .curr = 0x0028,
	}, {
		.mpixelclock = 66000000,
		.cpce = 0x013e, .gmp = 0x0003, .curr = 0x0038,
	}, {
		.mpixelclock = 83500000,
		.cpce = 0x0072, .gmp = 0x0001, .curr = 0x0028,
	}, {
		.mpixelclock = 146250000,
		.cpce = 0x0051, .gmp = 0x0002, .curr = 0x0038,
	}, {
		.mpixelclock = 148500000,
		.cpce = 0x0051, .gmp = 0x0003, .curr = 0x0000,
	}, {
		.mpixelclock = 272000000,
		.cpce = 0x0040, .gmp = 0x0003, .curr = 0x0000,
	}, {
		.mpixelclock = 340000000,
		.cpce = 0x0040, .gmp = 0x0003, .curr = 0x0000,
	}, {
		.mpixelclock = ~0ul,
		.cpce = 0x0051, .gmp = 0x0003, .curr = 0x0000,
	}
};

int rk_hdmi_read_edid(struct udevice *dev, u8 *buf, int buf_size)
{
	struct rk_hdmi_priv *priv = dev_get_priv(dev);

	return dw_hdmi_read_edid(&priv->hdmi, buf, buf_size);
}

int rk_hdmi_ofdata_to_platdata(struct udevice *dev)
{
	struct rk_hdmi_priv *priv = dev_get_priv(dev);
	struct dw_hdmi *hdmi = &priv->hdmi;

	hdmi->ioaddr = (ulong)dev_read_addr(dev);
	hdmi->mpll_cfg = rockchip_mpll_cfg;
	hdmi->phy_cfg = rockchip_phy_config;

	/* hdmi->i2c_clk_{high,low} are set up by the SoC driver */

	hdmi->reg_io_width = 4;
	hdmi->phy_set = dw_hdmi_phy_cfg;

	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);

	return 0;
}

void rk_hdmi_probe_regulators(struct udevice *dev,
			      const char * const *names, int cnt)
{
	rk_vop_probe_regulators(dev, names, cnt);
}

int rk_hdmi_probe(struct udevice *dev)
{
	struct rk_hdmi_priv *priv = dev_get_priv(dev);
	struct dw_hdmi *hdmi = &priv->hdmi;
	int ret;

	ret = dw_hdmi_phy_wait_for_hpd(hdmi);
	if (ret < 0) {
		debug("hdmi can not get hpd signal\n");
		return -1;
	}

	dw_hdmi_init(hdmi);
	dw_hdmi_phy_init(hdmi);

	return 0;
}
