// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <pwm.h>
#include <div64.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/time.h>

/* PWM registers and bits definitions */
#define PWMCON			0x00
#define PWMHDUR			0x04
#define PWMLDUR			0x08
#define PWMGDUR			0x0c
#define PWMWAVENUM		0x28
#define PWMDWIDTH		0x2c
#define PWM45DWIDTH_FIXUP	0x30
#define PWMTHRES		0x30
#define PWM45THRES_FIXUP	0x34

#define PWM_CLK_DIV_MAX		7
#define MAX_PWM_NUM		8

enum mtk_pwm_reg_ver {
	PWM_REG_V1,
	PWM_REG_V2,
};

static const unsigned int mtk_pwm_reg_offset_v1[] = {
	0x0010, 0x0050, 0x0090, 0x00d0, 0x0110, 0x0150, 0x0190, 0x0220
};

static const unsigned int mtk_pwm_reg_offset_v2[] = {
	0x0080, 0x00c0, 0x0100, 0x0140, 0x0180, 0x01c0, 0x0200, 0x0240
};

struct mtk_pwm_soc {
	unsigned int num_pwms;
	bool pwm45_fixup;
	enum mtk_pwm_reg_ver reg_ver;
};

struct mtk_pwm_priv {
	void __iomem *base;
	struct clk top_clk;
	struct clk main_clk;
	struct clk pwm_clks[MAX_PWM_NUM];
	const struct mtk_pwm_soc *soc;
};

static void mtk_pwm_w32(struct udevice *dev, uint channel, uint reg, uint val)
{
	struct mtk_pwm_priv *priv = dev_get_priv(dev);
	u32 offset;

	switch (priv->soc->reg_ver) {
	case PWM_REG_V2:
		offset = mtk_pwm_reg_offset_v2[channel];
		break;

	default:
		offset = mtk_pwm_reg_offset_v1[channel];
	}

	writel(val, priv->base + offset + reg);
}

static int mtk_pwm_set_config(struct udevice *dev, uint channel,
			      uint period_ns, uint duty_ns)
{
	struct mtk_pwm_priv *priv = dev_get_priv(dev);
	u32 clkdiv = 0, clksel = 0, cnt_period, cnt_duty,
	    reg_width = PWMDWIDTH, reg_thres = PWMTHRES;
	u64 resolution;
	int ret = 0;

	clk_enable(&priv->top_clk);
	clk_enable(&priv->main_clk);
	/* Using resolution in picosecond gets accuracy higher */
	resolution = (u64)NSEC_PER_SEC * 1000;
	do_div(resolution, clk_get_rate(&priv->pwm_clks[channel]));
	cnt_period = DIV_ROUND_CLOSEST_ULL((u64)period_ns * 1000, resolution);
	while (cnt_period > 8191) {
		resolution *= 2;
		clkdiv++;
		cnt_period = DIV_ROUND_CLOSEST_ULL((u64)period_ns * 1000,
						   resolution);
		if (clkdiv > PWM_CLK_DIV_MAX && clksel == 0) {
			clksel = 1;
			clkdiv = 0;
			resolution = (u64)NSEC_PER_SEC * 1000 * 1625;
			do_div(resolution,
			       clk_get_rate(&priv->pwm_clks[channel]));
			cnt_period = DIV_ROUND_CLOSEST_ULL(
					(u64)period_ns * 1000, resolution);
			clk_enable(&priv->pwm_clks[channel]);
		}
	}
	if (clkdiv > PWM_CLK_DIV_MAX && clksel == 1) {
		printf("pwm period %u not supported\n", period_ns);
		return -EINVAL;
	}
	if (priv->soc->pwm45_fixup && channel > 2) {
		/*
		 * PWM[4,5] has distinct offset for PWMDWIDTH and PWMTHRES
		 * from the other PWMs on MT7623.
		 */
		reg_width = PWM45DWIDTH_FIXUP;
		reg_thres = PWM45THRES_FIXUP;
	}
	cnt_duty = DIV_ROUND_CLOSEST_ULL((u64)duty_ns * 1000, resolution);
	if (clksel == 1)
		mtk_pwm_w32(dev, channel, PWMCON, BIT(15) | BIT(3) | clkdiv);
	else
		mtk_pwm_w32(dev, channel, PWMCON, BIT(15) | clkdiv);
	mtk_pwm_w32(dev, channel, reg_width, cnt_period);
	mtk_pwm_w32(dev, channel, reg_thres, cnt_duty);

	return ret;
};

static int mtk_pwm_set_enable(struct udevice *dev, uint channel, bool enable)
{
	struct mtk_pwm_priv *priv = dev_get_priv(dev);
	u32 val = 0;

	val = readl(priv->base);
	if (enable)
		val |= BIT(channel);
	else
		val &= ~BIT(channel);
	writel(val, priv->base);

	return 0;
};

static int mtk_pwm_probe(struct udevice *dev)
{
	struct mtk_pwm_priv *priv = dev_get_priv(dev);
	int ret = 0;
	int i;

	priv->soc = (struct mtk_pwm_soc *)dev_get_driver_data(dev);
	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;
	ret = clk_get_by_name(dev, "top", &priv->top_clk);
	if (ret < 0)
		return ret;
	ret = clk_get_by_name(dev, "main", &priv->main_clk);
	if (ret < 0)
		return ret;
	for (i = 0; i < priv->soc->num_pwms; i++) {
		char name[8];

		snprintf(name, sizeof(name), "pwm%d", i + 1);
		ret = clk_get_by_name(dev, name, &priv->pwm_clks[i]);
		if (ret < 0)
			return ret;
	}

	return ret;
}

static const struct pwm_ops mtk_pwm_ops = {
	.set_config	= mtk_pwm_set_config,
	.set_enable	= mtk_pwm_set_enable,
};

static const struct mtk_pwm_soc mt7622_data = {
	.num_pwms = 6,
	.pwm45_fixup = false,
	.reg_ver = PWM_REG_V1,
};

static const struct mtk_pwm_soc mt7623_data = {
	.num_pwms = 5,
	.pwm45_fixup = true,
	.reg_ver = PWM_REG_V1,
};

static const struct mtk_pwm_soc mt7629_data = {
	.num_pwms = 1,
	.pwm45_fixup = false,
	.reg_ver = PWM_REG_V1,
};

static const struct mtk_pwm_soc mt7981_data = {
	.num_pwms = 2,
	.pwm45_fixup = false,
	.reg_ver = PWM_REG_V2,
};

static const struct mtk_pwm_soc mt7986_data = {
	.num_pwms = 2,
	.pwm45_fixup = false,
	.reg_ver = PWM_REG_V1,
};

static const struct mtk_pwm_soc mt7988_data = {
	.num_pwms = 8,
	.pwm45_fixup = false,
	.reg_ver = PWM_REG_V2,
};

static const struct udevice_id mtk_pwm_ids[] = {
	{ .compatible = "mediatek,mt7622-pwm", .data = (ulong)&mt7622_data },
	{ .compatible = "mediatek,mt7623-pwm", .data = (ulong)&mt7623_data },
	{ .compatible = "mediatek,mt7629-pwm", .data = (ulong)&mt7629_data },
	{ .compatible = "mediatek,mt7981-pwm", .data = (ulong)&mt7981_data },
	{ .compatible = "mediatek,mt7986-pwm", .data = (ulong)&mt7986_data },
	{ .compatible = "mediatek,mt7988-pwm", .data = (ulong)&mt7988_data },
	{ }
};

U_BOOT_DRIVER(mtk_pwm) = {
	.name = "mtk_pwm",
	.id = UCLASS_PWM,
	.of_match = mtk_pwm_ids,
	.ops = &mtk_pwm_ops,
	.probe = mtk_pwm_probe,
	.priv_auto	= sizeof(struct mtk_pwm_priv),
};
