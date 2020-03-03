/*
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/regulator/consumer.h>
#include <linux/spinlock.h>
#include <video/mipi_display.h>

#include "dsi.h"
#include "dsi.xml.h"

#define MSM_DSI_VER_MAJOR_V2	0x02
#define MSM_DSI_VER_MAJOR_6G	0x03
#define MSM_DSI_6G_VER_MINOR_V1_0	0x10000000
#define MSM_DSI_6G_VER_MINOR_V1_1	0x10010000
#define MSM_DSI_6G_VER_MINOR_V1_1_1	0x10010001
#define MSM_DSI_6G_VER_MINOR_V1_2	0x10020000
#define MSM_DSI_6G_VER_MINOR_V1_3_1	0x10030001

#define DSI_6G_REG_SHIFT	4

#define DSI_REGULATOR_MAX	8
struct dsi_reg_entry {
	char name[32];
	int min_voltage;
	int max_voltage;
	int enable_load;
	int disable_load;
};

struct dsi_reg_config {
	int num;
	struct dsi_reg_entry regs[DSI_REGULATOR_MAX];
};

struct dsi_config {
	u32 major;
	u32 minor;
	u32 io_offset;
	enum msm_dsi_phy_type phy_type;
	struct dsi_reg_config reg_cfg;
};

static const struct dsi_config dsi_cfgs[] = {
	{MSM_DSI_VER_MAJOR_V2, 0, 0, MSM_DSI_PHY_UNKNOWN},
	{ /* 8974 v1 */
		.major = MSM_DSI_VER_MAJOR_6G,
		.minor = MSM_DSI_6G_VER_MINOR_V1_0,
		.io_offset = DSI_6G_REG_SHIFT,
		.phy_type = MSM_DSI_PHY_28NM,
		.reg_cfg = {
			.num = 4,
			.regs = {
				{"gdsc", -1, -1, -1, -1},
				{"vdd", 3000000, 3000000, 150000, 100},
				{"vdda", 1200000, 1200000, 100000, 100},
				{"vddio", 1800000, 1800000, 100000, 100},
			},
		},
	},
	{ /* 8974 v2 */
		.major = MSM_DSI_VER_MAJOR_6G,
		.minor = MSM_DSI_6G_VER_MINOR_V1_1,
		.io_offset = DSI_6G_REG_SHIFT,
		.phy_type = MSM_DSI_PHY_28NM,
		.reg_cfg = {
			.num = 4,
			.regs = {
				{"gdsc", -1, -1, -1, -1},
				{"vdd", 3000000, 3000000, 150000, 100},
				{"vdda", 1200000, 1200000, 100000, 100},
				{"vddio", 1800000, 1800000, 100000, 100},
			},
		},
	},
	{ /* 8974 v3 */
		.major = MSM_DSI_VER_MAJOR_6G,
		.minor = MSM_DSI_6G_VER_MINOR_V1_1_1,
		.io_offset = DSI_6G_REG_SHIFT,
		.phy_type = MSM_DSI_PHY_28NM,
		.reg_cfg = {
			.num = 4,
			.regs = {
				{"gdsc", -1, -1, -1, -1},
				{"vdd", 3000000, 3000000, 150000, 100},
				{"vdda", 1200000, 1200000, 100000, 100},
				{"vddio", 1800000, 1800000, 100000, 100},
			},
		},
	},
	{ /* 8084 */
		.major = MSM_DSI_VER_MAJOR_6G,
		.minor = MSM_DSI_6G_VER_MINOR_V1_2,
		.io_offset = DSI_6G_REG_SHIFT,
		.phy_type = MSM_DSI_PHY_28NM,
		.reg_cfg = {
			.num = 4,
			.regs = {
				{"gdsc", -1, -1, -1, -1},
				{"vdd", 3000000, 3000000, 150000, 100},
				{"vdda", 1200000, 1200000, 100000, 100},
				{"vddio", 1800000, 1800000, 100000, 100},
			},
		},
	},
	{ /* 8916 */
		.major = MSM_DSI_VER_MAJOR_6G,
		.minor = MSM_DSI_6G_VER_MINOR_V1_3_1,
		.io_offset = DSI_6G_REG_SHIFT,
		.phy_type = MSM_DSI_PHY_28NM,
		.reg_cfg = {
			.num = 4,
			.regs = {
				{"gdsc", -1, -1, -1, -1},
				{"vdd", 2850000, 2850000, 100000, 100},
				{"vdda", 1200000, 1200000, 100000, 100},
				{"vddio", 1800000, 1800000, 100000, 100},
			},
		},
	},
};

static int dsi_get_version(const void __iomem *base, u32 *major, u32 *minor)
{
	u32 ver;
	u32 ver_6g;

	if (!major || !minor)
		return -EINVAL;

	/* From DSI6G(v3), addition of a 6G_HW_VERSION register at offset 0
	 * makes all other registers 4-byte shifted down.
	 */
	ver_6g = msm_readl(base + REG_DSI_6G_HW_VERSION);
	if (ver_6g == 0) {
		ver = msm_readl(base + REG_DSI_VERSION);
		ver = FIELD(ver, DSI_VERSION_MAJOR);
		if (ver <= MSM_DSI_VER_MAJOR_V2) {
			/* old versions */
			*major = ver;
			*minor = 0;
			return 0;
		} else {
			return -EINVAL;
		}
	} else {
		ver = msm_readl(base + DSI_6G_REG_SHIFT + REG_DSI_VERSION);
		ver = FIELD(ver, DSI_VERSION_MAJOR);
		if (ver == MSM_DSI_VER_MAJOR_6G) {
			/* 6G version */
			*major = ver;
			*minor = ver_6g;
			return 0;
		} else {
			return -EINVAL;
		}
	}
}

#define DSI_ERR_STATE_ACK			0x0000
#define DSI_ERR_STATE_TIMEOUT			0x0001
#define DSI_ERR_STATE_DLN0_PHY			0x0002
#define DSI_ERR_STATE_FIFO			0x0004
#define DSI_ERR_STATE_MDP_FIFO_UNDERFLOW	0x0008
#define DSI_ERR_STATE_INTERLEAVE_OP_CONTENTION	0x0010
#define DSI_ERR_STATE_PLL_UNLOCKED		0x0020

#define DSI_CLK_CTRL_ENABLE_CLKS	\
		(DSI_CLK_CTRL_AHBS_HCLK_ON | DSI_CLK_CTRL_AHBM_SCLK_ON | \
		DSI_CLK_CTRL_PCLK_ON | DSI_CLK_CTRL_DSICLK_ON | \
		DSI_CLK_CTRL_BYTECLK_ON | DSI_CLK_CTRL_ESCCLK_ON | \
		DSI_CLK_CTRL_FORCE_ON_DYN_AHBM_HCLK)

struct msm_dsi_host {
	struct mipi_dsi_host base;

	struct platform_device *pdev;
	struct drm_device *dev;

	int id;

	void __iomem *ctrl_base;
	struct regulator_bulk_data supplies[DSI_REGULATOR_MAX];
	struct clk *mdp_core_clk;
	struct clk *ahb_clk;
	struct clk *axi_clk;
	struct clk *mmss_misc_ahb_clk;
	struct clk *byte_clk;
	struct clk *esc_clk;
	struct clk *pixel_clk;
	u32 byte_clk_rate;

	struct gpio_desc *disp_en_gpio;
	struct gpio_desc *te_gpio;

	const struct dsi_config *cfg;

	struct completion dma_comp;
	struct completion video_comp;
	struct mutex dev_mutex;
	struct mutex cmd_mutex;
	struct mutex clk_mutex;
	spinlock_t intr_lock; /* Protect interrupt ctrl register */

	u32 err_work_state;
	struct work_struct err_work;
	struct workqueue_struct *workqueue;

	struct drm_gem_object *tx_gem_obj;
	u8 *rx_buf;

	struct drm_display_mode *mode;

	/* Panel info */
	struct device_node *panel_node;
	unsigned int channel;
	unsigned int lanes;
	enum mipi_dsi_pixel_format format;
	unsigned long mode_flags;

	u32 dma_cmd_ctrl_restore;

	bool registered;
	bool power_on;
	int irq;
};

static u32 dsi_get_bpp(const enum mipi_dsi_pixel_format fmt)
{
	switch (fmt) {
	case MIPI_DSI_FMT_RGB565:		return 16;
	case MIPI_DSI_FMT_RGB666_PACKED:	return 18;
	case MIPI_DSI_FMT_RGB666:
	case MIPI_DSI_FMT_RGB888:
	default:				return 24;
	}
}

static inline u32 dsi_read(struct msm_dsi_host *msm_host, u32 reg)
{
	return msm_readl(msm_host->ctrl_base + msm_host->cfg->io_offset + reg);
}
static inline void dsi_write(struct msm_dsi_host *msm_host, u32 reg, u32 data)
{
	msm_writel(data, msm_host->ctrl_base + msm_host->cfg->io_offset + reg);
}

static int dsi_host_regulator_enable(struct msm_dsi_host *msm_host);
static void dsi_host_regulator_disable(struct msm_dsi_host *msm_host);

static const struct dsi_config *dsi_get_config(struct msm_dsi_host *msm_host)
{
	const struct dsi_config *cfg;
	struct regulator *gdsc_reg;
	int i, ret;
	u32 major = 0, minor = 0;

	gdsc_reg = regulator_get(&msm_host->pdev->dev, "gdsc");
	if (IS_ERR_OR_NULL(gdsc_reg)) {
		pr_err("%s: cannot get gdsc\n", __func__);
		goto fail;
	}
	ret = regulator_enable(gdsc_reg);
	if (ret) {
		pr_err("%s: unable to enable gdsc\n", __func__);
		regulator_put(gdsc_reg);
		goto fail;
	}
	ret = clk_prepare_enable(msm_host->ahb_clk);
	if (ret) {
		pr_err("%s: unable to enable ahb_clk\n", __func__);
		regulator_disable(gdsc_reg);
		regulator_put(gdsc_reg);
		goto fail;
	}

	ret = dsi_get_version(msm_host->ctrl_base, &major, &minor);

	clk_disable_unprepare(msm_host->ahb_clk);
	regulator_disable(gdsc_reg);
	regulator_put(gdsc_reg);
	if (ret) {
		pr_err("%s: Invalid version\n", __func__);
		goto fail;
	}

	for (i = 0; i < ARRAY_SIZE(dsi_cfgs); i++) {
		cfg = dsi_cfgs + i;
		if ((cfg->major == major) && (cfg->minor == minor))
			return cfg;
	}
	pr_err("%s: Version %x:%x not support\n", __func__, major, minor);

fail:
	return NULL;
}

static inline struct msm_dsi_host *to_msm_dsi_host(struct mipi_dsi_host *host)
{
	return container_of(host, struct msm_dsi_host, base);
}

static void dsi_host_regulator_disable(struct msm_dsi_host *msm_host)
{
	struct regulator_bulk_data *s = msm_host->supplies;
	const struct dsi_reg_entry *regs = msm_host->cfg->reg_cfg.regs;
	int num = msm_host->cfg->reg_cfg.num;
	int i;

	DBG("");
	for (i = num - 1; i >= 0; i--)
		if (regs[i].disable_load >= 0)
			regulator_set_load(s[i].consumer,
					   regs[i].disable_load);

	regulator_bulk_disable(num, s);
}

static int dsi_host_regulator_enable(struct msm_dsi_host *msm_host)
{
	struct regulator_bulk_data *s = msm_host->supplies;
	const struct dsi_reg_entry *regs = msm_host->cfg->reg_cfg.regs;
	int num = msm_host->cfg->reg_cfg.num;
	int ret, i;

	DBG("");
	for (i = 0; i < num; i++) {
		if (regs[i].enable_load >= 0) {
			ret = regulator_set_load(s[i].consumer,
						 regs[i].enable_load);
			if (ret < 0) {
				pr_err("regulator %d set op mode failed, %d\n",
					i, ret);
				goto fail;
			}
		}
	}

	ret = regulator_bulk_enable(num, s);
	if (ret < 0) {
		pr_err("regulator enable failed, %d\n", ret);
		goto fail;
	}

	return 0;

fail:
	for (i--; i >= 0; i--)
		regulator_set_load(s[i].consumer, regs[i].disable_load);
	return ret;
}

static int dsi_regulator_init(struct msm_dsi_host *msm_host)
{
	struct regulator_bulk_data *s = msm_host->supplies;
	const struct dsi_reg_entry *regs = msm_host->cfg->reg_cfg.regs;
	int num = msm_host->cfg->reg_cfg.num;
	int i, ret;

	for (i = 0; i < num; i++)
		s[i].supply = regs[i].name;

	ret = devm_regulator_bulk_get(&msm_host->pdev->dev, num, s);
	if (ret < 0) {
		pr_err("%s: failed to init regulator, ret=%d\n",
						__func__, ret);
		return ret;
	}

	for (i = 0; i < num; i++) {
		if ((regs[i].min_voltage >= 0) && (regs[i].max_voltage >= 0)) {
			ret = regulator_set_voltage(s[i].consumer,
				regs[i].min_voltage, regs[i].max_voltage);
			if (ret < 0) {
				pr_err("regulator %d set voltage failed, %d\n",
					i, ret);
				return ret;
			}
		}
	}

	return 0;
}

static int dsi_clk_init(struct msm_dsi_host *msm_host)
{
	struct device *dev = &msm_host->pdev->dev;
	int ret = 0;

	msm_host->mdp_core_clk = devm_clk_get(dev, "mdp_core_clk");
	if (IS_ERR(msm_host->mdp_core_clk)) {
		ret = PTR_ERR(msm_host->mdp_core_clk);
		pr_err("%s: Unable to get mdp core clk. ret=%d\n",
			__func__, ret);
		goto exit;
	}

	msm_host->ahb_clk = devm_clk_get(dev, "iface_clk");
	if (IS_ERR(msm_host->ahb_clk)) {
		ret = PTR_ERR(msm_host->ahb_clk);
		pr_err("%s: Unable to get mdss ahb clk. ret=%d\n",
			__func__, ret);
		goto exit;
	}

	msm_host->axi_clk = devm_clk_get(dev, "bus_clk");
	if (IS_ERR(msm_host->axi_clk)) {
		ret = PTR_ERR(msm_host->axi_clk);
		pr_err("%s: Unable to get axi bus clk. ret=%d\n",
			__func__, ret);
		goto exit;
	}

	msm_host->mmss_misc_ahb_clk = devm_clk_get(dev, "core_mmss_clk");
	if (IS_ERR(msm_host->mmss_misc_ahb_clk)) {
		ret = PTR_ERR(msm_host->mmss_misc_ahb_clk);
		pr_err("%s: Unable to get mmss misc ahb clk. ret=%d\n",
			__func__, ret);
		goto exit;
	}

	msm_host->byte_clk = devm_clk_get(dev, "byte_clk");
	if (IS_ERR(msm_host->byte_clk)) {
		ret = PTR_ERR(msm_host->byte_clk);
		pr_err("%s: can't find dsi_byte_clk. ret=%d\n",
			__func__, ret);
		msm_host->byte_clk = NULL;
		goto exit;
	}

	msm_host->pixel_clk = devm_clk_get(dev, "pixel_clk");
	if (IS_ERR(msm_host->pixel_clk)) {
		ret = PTR_ERR(msm_host->pixel_clk);
		pr_err("%s: can't find dsi_pixel_clk. ret=%d\n",
			__func__, ret);
		msm_host->pixel_clk = NULL;
		goto exit;
	}

	msm_host->esc_clk = devm_clk_get(dev, "core_clk");
	if (IS_ERR(msm_host->esc_clk)) {
		ret = PTR_ERR(msm_host->esc_clk);
		pr_err("%s: can't find dsi_esc_clk. ret=%d\n",
			__func__, ret);
		msm_host->esc_clk = NULL;
		goto exit;
	}

exit:
	return ret;
}

static int dsi_bus_clk_enable(struct msm_dsi_host *msm_host)
{
	int ret;

	DBG("id=%d", msm_host->id);

	ret = clk_prepare_enable(msm_host->mdp_core_clk);
	if (ret) {
		pr_err("%s: failed to enable mdp_core_clock, %d\n",
							 __func__, ret);
		goto core_clk_err;
	}

	ret = clk_prepare_enable(msm_host->ahb_clk);
	if (ret) {
		pr_err("%s: failed to enable ahb clock, %d\n", __func__, ret);
		goto ahb_clk_err;
	}

	ret = clk_prepare_enable(msm_host->axi_clk);
	if (ret) {
		pr_err("%s: failed to enable ahb clock, %d\n", __func__, ret);
		goto axi_clk_err;
	}

	ret = clk_prepare_enable(msm_host->mmss_misc_ahb_clk);
	if (ret) {
		pr_err("%s: failed to enable mmss misc ahb clk, %d\n",
			__func__, ret);
		goto misc_ahb_clk_err;
	}

	return 0;

misc_ahb_clk_err:
	clk_disable_unprepare(msm_host->axi_clk);
axi_clk_err:
	clk_disable_unprepare(msm_host->ahb_clk);
ahb_clk_err:
	clk_disable_unprepare(msm_host->mdp_core_clk);
core_clk_err:
	return ret;
}

static void dsi_bus_clk_disable(struct msm_dsi_host *msm_host)
{
	DBG("");
	clk_disable_unprepare(msm_host->mmss_misc_ahb_clk);
	clk_disable_unprepare(msm_host->axi_clk);
	clk_disable_unprepare(msm_host->ahb_clk);
	clk_disable_unprepare(msm_host->mdp_core_clk);
}

static int dsi_link_clk_enable(struct msm_dsi_host *msm_host)
{
	int ret;

	DBG("Set clk rates: pclk=%d, byteclk=%d",
		msm_host->mode->clock, msm_host->byte_clk_rate);

	ret = clk_set_rate(msm_host->byte_clk, msm_host->byte_clk_rate);
	if (ret) {
		pr_err("%s: Failed to set rate byte clk, %d\n", __func__, ret);
		goto error;
	}

	ret = clk_set_rate(msm_host->pixel_clk, msm_host->mode->clock * 1000);
	if (ret) {
		pr_err("%s: Failed to set rate pixel clk, %d\n", __func__, ret);
		goto error;
	}

	ret = clk_prepare_enable(msm_host->esc_clk);
	if (ret) {
		pr_err("%s: Failed to enable dsi esc clk\n", __func__);
		goto error;
	}

	ret = clk_prepare_enable(msm_host->byte_clk);
	if (ret) {
		pr_err("%s: Failed to enable dsi byte clk\n", __func__);
		goto byte_clk_err;
	}

	ret = clk_prepare_enable(msm_host->pixel_clk);
	if (ret) {
		pr_err("%s: Failed to enable dsi pixel clk\n", __func__);
		goto pixel_clk_err;
	}

	return 0;

pixel_clk_err:
	clk_disable_unprepare(msm_host->byte_clk);
byte_clk_err:
	clk_disable_unprepare(msm_host->esc_clk);
error:
	return ret;
}

static void dsi_link_clk_disable(struct msm_dsi_host *msm_host)
{
	clk_disable_unprepare(msm_host->esc_clk);
	clk_disable_unprepare(msm_host->pixel_clk);
	clk_disable_unprepare(msm_host->byte_clk);
}

static int dsi_clk_ctrl(struct msm_dsi_host *msm_host, bool enable)
{
	int ret = 0;

	mutex_lock(&msm_host->clk_mutex);
	if (enable) {
		ret = dsi_bus_clk_enable(msm_host);
		if (ret) {
			pr_err("%s: Can not enable bus clk, %d\n",
				__func__, ret);
			goto unlock_ret;
		}
		ret = dsi_link_clk_enable(msm_host);
		if (ret) {
			pr_err("%s: Can not enable link clk, %d\n",
				__func__, ret);
			dsi_bus_clk_disable(msm_host);
			goto unlock_ret;
		}
	} else {
		dsi_link_clk_disable(msm_host);
		dsi_bus_clk_disable(msm_host);
	}

unlock_ret:
	mutex_unlock(&msm_host->clk_mutex);
	return ret;
}

static int dsi_calc_clk_rate(struct msm_dsi_host *msm_host)
{
	struct drm_display_mode *mode = msm_host->mode;
	u8 lanes = msm_host->lanes;
	u32 bpp = dsi_get_bpp(msm_host->format);
	u32 pclk_rate;

	if (!mode) {
		pr_err("%s: mode not set\n", __func__);
		return -EINVAL;
	}

	pclk_rate = mode->clock * 1000;
	if (lanes > 0) {
		msm_host->byte_clk_rate = (pclk_rate * bpp) / (8 * lanes);
	} else {
		pr_err("%s: forcing mdss_dsi lanes to 1\n", __func__);
		msm_host->byte_clk_rate = (pclk_rate * bpp) / 8;
	}

	DBG("pclk=%d, bclk=%d", pclk_rate, msm_host->byte_clk_rate);

	return 0;
}

static void dsi_phy_sw_reset(struct msm_dsi_host *msm_host)
{
	DBG("");
	dsi_write(msm_host, REG_DSI_PHY_RESET, DSI_PHY_RESET_RESET);
	/* Make sure fully reset */
	wmb();
	udelay(1000);
	dsi_write(msm_host, REG_DSI_PHY_RESET, 0);
	udelay(100);
}

static void dsi_intr_ctrl(struct msm_dsi_host *msm_host, u32 mask, int enable)
{
	u32 intr;
	unsigned long flags;

	spin_lock_irqsave(&msm_host->intr_lock, flags);
	intr = dsi_read(msm_host, REG_DSI_INTR_CTRL);

	if (enable)
		intr |= mask;
	else
		intr &= ~mask;

	DBG("intr=%x enable=%d", intr, enable);

	dsi_write(msm_host, REG_DSI_INTR_CTRL, intr);
	spin_unlock_irqrestore(&msm_host->intr_lock, flags);
}

static inline enum dsi_traffic_mode dsi_get_traffic_mode(const u32 mode_flags)
{
	if (mode_flags & MIPI_DSI_MODE_VIDEO_BURST)
		return BURST_MODE;
	else if (mode_flags & MIPI_DSI_MODE_VIDEO_SYNC_PULSE)
		return NON_BURST_SYNCH_PULSE;

	return NON_BURST_SYNCH_EVENT;
}

static inline enum dsi_vid_dst_format dsi_get_vid_fmt(
				const enum mipi_dsi_pixel_format mipi_fmt)
{
	switch (mipi_fmt) {
	case MIPI_DSI_FMT_RGB888:	return VID_DST_FORMAT_RGB888;
	case MIPI_DSI_FMT_RGB666:	return VID_DST_FORMAT_RGB666_LOOSE;
	case MIPI_DSI_FMT_RGB666_PACKED:	return VID_DST_FORMAT_RGB666;
	case MIPI_DSI_FMT_RGB565:	return VID_DST_FORMAT_RGB565;
	default:			return VID_DST_FORMAT_RGB888;
	}
}

static inline enum dsi_cmd_dst_format dsi_get_cmd_fmt(
				const enum mipi_dsi_pixel_format mipi_fmt)
{
	switch (mipi_fmt) {
	case MIPI_DSI_FMT_RGB888:	return CMD_DST_FORMAT_RGB888;
	case MIPI_DSI_FMT_RGB666_PACKED:
	case MIPI_DSI_FMT_RGB666:	return VID_DST_FORMAT_RGB666;
	case MIPI_DSI_FMT_RGB565:	return CMD_DST_FORMAT_RGB565;
	default:			return CMD_DST_FORMAT_RGB888;
	}
}

static void dsi_ctrl_config(struct msm_dsi_host *msm_host, bool enable,
				u32 clk_pre, u32 clk_post)
{
	u32 flags = msm_host->mode_flags;
	enum mipi_dsi_pixel_format mipi_fmt = msm_host->format;
	u32 data = 0;

	if (!enable) {
		dsi_write(msm_host, REG_DSI_CTRL, 0);
		return;
	}

	if (flags & MIPI_DSI_MODE_VIDEO) {
		if (flags & MIPI_DSI_MODE_VIDEO_HSE)
			data |= DSI_VID_CFG0_PULSE_MODE_HSA_HE;
		if (flags & MIPI_DSI_MODE_VIDEO_HFP)
			data |= DSI_VID_CFG0_HFP_POWER_STOP;
		if (flags & MIPI_DSI_MODE_VIDEO_HBP)
			data |= DSI_VID_CFG0_HBP_POWER_STOP;
		if (flags & MIPI_DSI_MODE_VIDEO_HSA)
			data |= DSI_VID_CFG0_HSA_POWER_STOP;
		/* Always set low power stop mode for BLLP
		 * to let command engine send packets
		 */
		data |= DSI_VID_CFG0_EOF_BLLP_POWER_STOP |
			DSI_VID_CFG0_BLLP_POWER_STOP;
		data |= DSI_VID_CFG0_TRAFFIC_MODE(dsi_get_traffic_mode(flags));
		data |= DSI_VID_CFG0_DST_FORMAT(dsi_get_vid_fmt(mipi_fmt));
		data |= DSI_VID_CFG0_VIRT_CHANNEL(msm_host->channel);
		dsi_write(msm_host, REG_DSI_VID_CFG0, data);

		/* Do not swap RGB colors */
		data = DSI_VID_CFG1_RGB_SWAP(SWAP_RGB);
		dsi_write(msm_host, REG_DSI_VID_CFG1, 0);
	} else {
		/* Do not swap RGB colors */
		data = DSI_CMD_CFG0_RGB_SWAP(SWAP_RGB);
		data |= DSI_CMD_CFG0_DST_FORMAT(dsi_get_cmd_fmt(mipi_fmt));
		dsi_write(msm_host, REG_DSI_CMD_CFG0, data);

		data = DSI_CMD_CFG1_WR_MEM_START(MIPI_DCS_WRITE_MEMORY_START) |
			DSI_CMD_CFG1_WR_MEM_CONTINUE(
					MIPI_DCS_WRITE_MEMORY_CONTINUE);
		/* Always insert DCS command */
		data |= DSI_CMD_CFG1_INSERT_DCS_COMMAND;
		dsi_write(msm_host, REG_DSI_CMD_CFG1, data);
	}

	dsi_write(msm_host, REG_DSI_CMD_DMA_CTRL,
			DSI_CMD_DMA_CTRL_FROM_FRAME_BUFFER |
			DSI_CMD_DMA_CTRL_LOW_POWER);

	data = 0;
	/* Always assume dedicated TE pin */
	data |= DSI_TRIG_CTRL_TE;
	data |= DSI_TRIG_CTRL_MDP_TRIGGER(TRIGGER_NONE);
	data |= DSI_TRIG_CTRL_DMA_TRIGGER(TRIGGER_SW);
	data |= DSI_TRIG_CTRL_STREAM(msm_host->channel);
	if ((msm_host->cfg->major == MSM_DSI_VER_MAJOR_6G) &&
		(msm_host->cfg->minor >= MSM_DSI_6G_VER_MINOR_V1_2))
		data |= DSI_TRIG_CTRL_BLOCK_DMA_WITHIN_FRAME;
	dsi_write(msm_host, REG_DSI_TRIG_CTRL, data);

	data = DSI_CLKOUT_TIMING_CTRL_T_CLK_POST(clk_post) |
		DSI_CLKOUT_TIMING_CTRL_T_CLK_PRE(clk_pre);
	dsi_write(msm_host, REG_DSI_CLKOUT_TIMING_CTRL, data);

	data = 0;
	if (!(flags & MIPI_DSI_MODE_EOT_PACKET))
		data |= DSI_EOT_PACKET_CTRL_TX_EOT_APPEND;
	dsi_write(msm_host, REG_DSI_EOT_PACKET_CTRL, data);

	/* allow only ack-err-status to generate interrupt */
	dsi_write(msm_host, REG_DSI_ERR_INT_MASK0, 0x13ff3fe0);

	dsi_intr_ctrl(msm_host, DSI_IRQ_MASK_ERROR, 1);

	dsi_write(msm_host, REG_DSI_CLK_CTRL, DSI_CLK_CTRL_ENABLE_CLKS);

	data = DSI_CTRL_CLK_EN;

	DBG("lane number=%d", msm_host->lanes);
	if (msm_host->lanes == 2) {
		data |= DSI_CTRL_LANE1 | DSI_CTRL_LANE2;
		/* swap lanes for 2-lane panel for better performance */
		dsi_write(msm_host, REG_DSI_LANE_SWAP_CTRL,
			DSI_LANE_SWAP_CTRL_DLN_SWAP_SEL(LANE_SWAP_1230));
	} else {
		/* Take 4 lanes as default */
		data |= DSI_CTRL_LANE0 | DSI_CTRL_LANE1 | DSI_CTRL_LANE2 |
			DSI_CTRL_LANE3;
		/* Do not swap lanes for 4-lane panel */
		dsi_write(msm_host, REG_DSI_LANE_SWAP_CTRL,
			DSI_LANE_SWAP_CTRL_DLN_SWAP_SEL(LANE_SWAP_0123));
	}
	data |= DSI_CTRL_ENABLE;

	dsi_write(msm_host, REG_DSI_CTRL, data);
}

static void dsi_timing_setup(struct msm_dsi_host *msm_host)
{
	struct drm_display_mode *mode = msm_host->mode;
	u32 hs_start = 0, vs_start = 0; /* take sync start as 0 */
	u32 h_total = mode->htotal;
	u32 v_total = mode->vtotal;
	u32 hs_end = mode->hsync_end - mode->hsync_start;
	u32 vs_end = mode->vsync_end - mode->vsync_start;
	u32 ha_start = h_total - mode->hsync_start;
	u32 ha_end = ha_start + mode->hdisplay;
	u32 va_start = v_total - mode->vsync_start;
	u32 va_end = va_start + mode->vdisplay;
	u32 wc;

	DBG("");

	if (msm_host->mode_flags & MIPI_DSI_MODE_VIDEO) {
		dsi_write(msm_host, REG_DSI_ACTIVE_H,
			DSI_ACTIVE_H_START(ha_start) |
			DSI_ACTIVE_H_END(ha_end));
		dsi_write(msm_host, REG_DSI_ACTIVE_V,
			DSI_ACTIVE_V_START(va_start) |
			DSI_ACTIVE_V_END(va_end));
		dsi_write(msm_host, REG_DSI_TOTAL,
			DSI_TOTAL_H_TOTAL(h_total - 1) |
			DSI_TOTAL_V_TOTAL(v_total - 1));

		dsi_write(msm_host, REG_DSI_ACTIVE_HSYNC,
			DSI_ACTIVE_HSYNC_START(hs_start) |
			DSI_ACTIVE_HSYNC_END(hs_end));
		dsi_write(msm_host, REG_DSI_ACTIVE_VSYNC_HPOS, 0);
		dsi_write(msm_host, REG_DSI_ACTIVE_VSYNC_VPOS,
			DSI_ACTIVE_VSYNC_VPOS_START(vs_start) |
			DSI_ACTIVE_VSYNC_VPOS_END(vs_end));
	} else {		/* command mode */
		/* image data and 1 byte write_memory_start cmd */
		wc = mode->hdisplay * dsi_get_bpp(msm_host->format) / 8 + 1;

		dsi_write(msm_host, REG_DSI_CMD_MDP_STREAM_CTRL,
			DSI_CMD_MDP_STREAM_CTRL_WORD_COUNT(wc) |
			DSI_CMD_MDP_STREAM_CTRL_VIRTUAL_CHANNEL(
					msm_host->channel) |
			DSI_CMD_MDP_STREAM_CTRL_DATA_TYPE(
					MIPI_DSI_DCS_LONG_WRITE));

		dsi_write(msm_host, REG_DSI_CMD_MDP_STREAM_TOTAL,
			DSI_CMD_MDP_STREAM_TOTAL_H_TOTAL(mode->hdisplay) |
			DSI_CMD_MDP_STREAM_TOTAL_V_TOTAL(mode->vdisplay));
	}
}

static void dsi_sw_reset(struct msm_dsi_host *msm_host)
{
	dsi_write(msm_host, REG_DSI_CLK_CTRL, DSI_CLK_CTRL_ENABLE_CLKS);
	wmb(); /* clocks need to be enabled before reset */

	dsi_write(msm_host, REG_DSI_RESET, 1);
	wmb(); /* make sure reset happen */
	dsi_write(msm_host, REG_DSI_RESET, 0);
}

static void dsi_op_mode_config(struct msm_dsi_host *msm_host,
					bool video_mode, bool enable)
{
	u32 dsi_ctrl;

	dsi_ctrl = dsi_read(msm_host, REG_DSI_CTRL);

	if (!enable) {
		dsi_ctrl &= ~(DSI_CTRL_ENABLE | DSI_CTRL_VID_MODE_EN |
				DSI_CTRL_CMD_MODE_EN);
		dsi_intr_ctrl(msm_host, DSI_IRQ_MASK_CMD_MDP_DONE |
					DSI_IRQ_MASK_VIDEO_DONE, 0);
	} else {
		if (video_mode) {
			dsi_ctrl |= DSI_CTRL_VID_MODE_EN;
		} else {		/* command mode */
			dsi_ctrl |= DSI_CTRL_CMD_MODE_EN;
			dsi_intr_ctrl(msm_host, DSI_IRQ_MASK_CMD_MDP_DONE, 1);
		}
		dsi_ctrl |= DSI_CTRL_ENABLE;
	}

	dsi_write(msm_host, REG_DSI_CTRL, dsi_ctrl);
}

static void dsi_set_tx_power_mode(int mode, struct msm_dsi_host *msm_host)
{
	u32 data;

	data = dsi_read(msm_host, REG_DSI_CMD_DMA_CTRL);

	if (mode == 0)
		data &= ~DSI_CMD_DMA_CTRL_LOW_POWER;
	else
		data |= DSI_CMD_DMA_CTRL_LOW_POWER;

	dsi_write(msm_host, REG_DSI_CMD_DMA_CTRL, data);
}

static void dsi_wait4video_done(struct msm_dsi_host *msm_host)
{
	dsi_intr_ctrl(msm_host, DSI_IRQ_MASK_VIDEO_DONE, 1);

	reinit_completion(&msm_host->video_comp);

	wait_for_completion_timeout(&msm_host->video_comp,
			msecs_to_jiffies(70));

	dsi_intr_ctrl(msm_host, DSI_IRQ_MASK_VIDEO_DONE, 0);
}

static void dsi_wait4video_eng_busy(struct msm_dsi_host *msm_host)
{
	if (!(msm_host->mode_flags & MIPI_DSI_MODE_VIDEO))
		return;

	if (msm_host->power_on) {
		dsi_wait4video_done(msm_host);
		/* delay 4 ms to skip BLLP */
		usleep_range(2000, 4000);
	}
}

/* dsi_cmd */
static int dsi_tx_buf_alloc(struct msm_dsi_host *msm_host, int size)
{
	struct drm_device *dev = msm_host->dev;
	int ret;
	u32 iova;

	mutex_lock(&dev->struct_mutex);
	msm_host->tx_gem_obj = msm_gem_new(dev, size, MSM_BO_UNCACHED);
	if (IS_ERR(msm_host->tx_gem_obj)) {
		ret = PTR_ERR(msm_host->tx_gem_obj);
		pr_err("%s: failed to allocate gem, %d\n", __func__, ret);
		msm_host->tx_gem_obj = NULL;
		mutex_unlock(&dev->struct_mutex);
		return ret;
	}

	ret = msm_gem_get_iova_locked(msm_host->tx_gem_obj, 0, &iova);
	if (ret) {
		pr_err("%s: failed to get iova, %d\n", __func__, ret);
		return ret;
	}
	mutex_unlock(&dev->struct_mutex);

	if (iova & 0x07) {
		pr_err("%s: buf NOT 8 bytes aligned\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static void dsi_tx_buf_free(struct msm_dsi_host *msm_host)
{
	struct drm_device *dev = msm_host->dev;

	if (msm_host->tx_gem_obj) {
		msm_gem_put_iova(msm_host->tx_gem_obj, 0);
		mutex_lock(&dev->struct_mutex);
		msm_gem_free_object(msm_host->tx_gem_obj);
		msm_host->tx_gem_obj = NULL;
		mutex_unlock(&dev->struct_mutex);
	}
}

/*
 * prepare cmd buffer to be txed
 */
static int dsi_cmd_dma_add(struct drm_gem_object *tx_gem,
			const struct mipi_dsi_msg *msg)
{
	struct mipi_dsi_packet packet;
	int len;
	int ret;
	u8 *data;

	ret = mipi_dsi_create_packet(&packet, msg);
	if (ret) {
		pr_err("%s: create packet failed, %d\n", __func__, ret);
		return ret;
	}
	len = (packet.size + 3) & (~0x3);

	if (len > tx_gem->size) {
		pr_err("%s: packet size is too big\n", __func__);
		return -EINVAL;
	}

	data = msm_gem_vaddr(tx_gem);

	if (IS_ERR(data)) {
		ret = PTR_ERR(data);
		pr_err("%s: get vaddr failed, %d\n", __func__, ret);
		return ret;
	}

	/* MSM specific command format in memory */
	data[0] = packet.header[1];
	data[1] = packet.header[2];
	data[2] = packet.header[0];
	data[3] = BIT(7); /* Last packet */
	if (mipi_dsi_packet_format_is_long(msg->type))
		data[3] |= BIT(6);
	if (msg->rx_buf && msg->rx_len)
		data[3] |= BIT(5);

	/* Long packet */
	if (packet.payload && packet.payload_length)
		memcpy(data + 4, packet.payload, packet.payload_length);

	/* Append 0xff to the end */
	if (packet.size < len)
		memset(data + packet.size, 0xff, len - packet.size);

	return len;
}

/*
 * dsi_short_read1_resp: 1 parameter
 */
static int dsi_short_read1_resp(u8 *buf, const struct mipi_dsi_msg *msg)
{
	u8 *data = msg->rx_buf;
	if (data && (msg->rx_len >= 1)) {
		*data = buf[1]; /* strip out dcs type */
		return 1;
	} else {
		pr_err("%s: read data does not match with rx_buf len %zu\n",
			__func__, msg->rx_len);
		return -EINVAL;
	}
}

/*
 * dsi_short_read2_resp: 2 parameter
 */
static int dsi_short_read2_resp(u8 *buf, const struct mipi_dsi_msg *msg)
{
	u8 *data = msg->rx_buf;
	if (data && (msg->rx_len >= 2)) {
		data[0] = buf[1]; /* strip out dcs type */
		data[1] = buf[2];
		return 2;
	} else {
		pr_err("%s: read data does not match with rx_buf len %zu\n",
			__func__, msg->rx_len);
		return -EINVAL;
	}
}

static int dsi_long_read_resp(u8 *buf, const struct mipi_dsi_msg *msg)
{
	/* strip out 4 byte dcs header */
	if (msg->rx_buf && msg->rx_len)
		memcpy(msg->rx_buf, buf + 4, msg->rx_len);

	return msg->rx_len;
}


static int dsi_cmd_dma_tx(struct msm_dsi_host *msm_host, int len)
{
	int ret;
	u32 iova;
	bool triggered;

	ret = msm_gem_get_iova(msm_host->tx_gem_obj, 0, &iova);
	if (ret) {
		pr_err("%s: failed to get iova: %d\n", __func__, ret);
		return ret;
	}

	reinit_completion(&msm_host->dma_comp);

	dsi_wait4video_eng_busy(msm_host);

	triggered = msm_dsi_manager_cmd_xfer_trigger(
						msm_host->id, iova, len);
	if (triggered) {
		ret = wait_for_completion_timeout(&msm_host->dma_comp,
					msecs_to_jiffies(200));
		DBG("ret=%d", ret);
		if (ret == 0)
			ret = -ETIMEDOUT;
		else
			ret = len;
	} else
		ret = len;

	return ret;
}

static int dsi_cmd_dma_rx(struct msm_dsi_host *msm_host,
			u8 *buf, int rx_byte, int pkt_size)
{
	u32 *lp, *temp, data;
	int i, j = 0, cnt;
	u32 read_cnt;
	u8 reg[16];
	int repeated_bytes = 0;
	int buf_offset = buf - msm_host->rx_buf;

	lp = (u32 *)buf;
	temp = (u32 *)reg;
	cnt = (rx_byte + 3) >> 2;
	if (cnt > 4)
		cnt = 4; /* 4 x 32 bits registers only */

	if (rx_byte == 4)
		read_cnt = 4;
	else
		read_cnt = pkt_size + 6;

	/*
	 * In case of multiple reads from the panel, after the first read, there
	 * is possibility that there are some bytes in the payload repeating in
	 * the RDBK_DATA registers. Since we read all the parameters from the
	 * panel right from the first byte for every pass. We need to skip the
	 * repeating bytes and then append the new parameters to the rx buffer.
	 */
	if (read_cnt > 16) {
		int bytes_shifted;
		/* Any data more than 16 bytes will be shifted out.
		 * The temp read buffer should already contain these bytes.
		 * The remaining bytes in read buffer are the repeated bytes.
		 */
		bytes_shifted = read_cnt - 16;
		repeated_bytes = buf_offset - bytes_shifted;
	}

	for (i = cnt - 1; i >= 0; i--) {
		data = dsi_read(msm_host, REG_DSI_RDBK_DATA(i));
		*temp++ = ntohl(data); /* to host byte order */
		DBG("data = 0x%x and ntohl(data) = 0x%x", data, ntohl(data));
	}

	for (i = repeated_bytes; i < 16; i++)
		buf[j++] = reg[i];

	return j;
}

static int dsi_cmds2buf_tx(struct msm_dsi_host *msm_host,
				const struct mipi_dsi_msg *msg)
{
	int len, ret;
	int bllp_len = msm_host->mode->hdisplay *
			dsi_get_bpp(msm_host->format) / 8;

	len = dsi_cmd_dma_add(msm_host->tx_gem_obj, msg);
	if (!len) {
		pr_err("%s: failed to add cmd type = 0x%x\n",
			__func__,  msg->type);
		return -EINVAL;
	}

	/* for video mode, do not send cmds more than
	* one pixel line, since it only transmit it
	* during BLLP.
	*/
	/* TODO: if the command is sent in LP mode, the bit rate is only
	 * half of esc clk rate. In this case, if the video is already
	 * actively streaming, we need to check more carefully if the
	 * command can be fit into one BLLP.
	 */
	if ((msm_host->mode_flags & MIPI_DSI_MODE_VIDEO) && (len > bllp_len)) {
		pr_err("%s: cmd cannot fit into BLLP period, len=%d\n",
			__func__, len);
		return -EINVAL;
	}

	ret = dsi_cmd_dma_tx(msm_host, len);
	if (ret < len) {
		pr_err("%s: cmd dma tx failed, type=0x%x, data0=0x%x, len=%d\n",
			__func__, msg->type, (*(u8 *)(msg->tx_buf)), len);
		return -ECOMM;
	}

	return len;
}

static void dsi_sw_reset_restore(struct msm_dsi_host *msm_host)
{
	u32 data0, data1;

	data0 = dsi_read(msm_host, REG_DSI_CTRL);
	data1 = data0;
	data1 &= ~DSI_CTRL_ENABLE;
	dsi_write(msm_host, REG_DSI_CTRL, data1);
	/*
	 * dsi controller need to be disabled before
	 * clocks turned on
	 */
	wmb();

	dsi_write(msm_host, REG_DSI_CLK_CTRL, DSI_CLK_CTRL_ENABLE_CLKS);
	wmb();	/* make sure clocks enabled */

	/* dsi controller can only be reset while clocks are running */
	dsi_write(msm_host, REG_DSI_RESET, 1);
	wmb();	/* make sure reset happen */
	dsi_write(msm_host, REG_DSI_RESET, 0);
	wmb();	/* controller out of reset */
	dsi_write(msm_host, REG_DSI_CTRL, data0);
	wmb();	/* make sure dsi controller enabled again */
}

static void dsi_err_worker(struct work_struct *work)
{
	struct msm_dsi_host *msm_host =
		container_of(work, struct msm_dsi_host, err_work);
	u32 status = msm_host->err_work_state;

	pr_err_ratelimited("%s: status=%x\n", __func__, status);
	if (status & DSI_ERR_STATE_MDP_FIFO_UNDERFLOW)
		dsi_sw_reset_restore(msm_host);

	/* It is safe to clear here because error irq is disabled. */
	msm_host->err_work_state = 0;

	/* enable dsi error interrupt */
	dsi_intr_ctrl(msm_host, DSI_IRQ_MASK_ERROR, 1);
}

static void dsi_ack_err_status(struct msm_dsi_host *msm_host)
{
	u32 status;

	status = dsi_read(msm_host, REG_DSI_ACK_ERR_STATUS);

	if (status) {
		dsi_write(msm_host, REG_DSI_ACK_ERR_STATUS, status);
		/* Writing of an extra 0 needed to clear error bits */
		dsi_write(msm_host, REG_DSI_ACK_ERR_STATUS, 0);
		msm_host->err_work_state |= DSI_ERR_STATE_ACK;
	}
}

static void dsi_timeout_status(struct msm_dsi_host *msm_host)
{
	u32 status;

	status = dsi_read(msm_host, REG_DSI_TIMEOUT_STATUS);

	if (status) {
		dsi_write(msm_host, REG_DSI_TIMEOUT_STATUS, status);
		msm_host->err_work_state |= DSI_ERR_STATE_TIMEOUT;
	}
}

static void dsi_dln0_phy_err(struct msm_dsi_host *msm_host)
{
	u32 status;

	status = dsi_read(msm_host, REG_DSI_DLN0_PHY_ERR);

	if (status) {
		dsi_write(msm_host, REG_DSI_DLN0_PHY_ERR, status);
		msm_host->err_work_state |= DSI_ERR_STATE_DLN0_PHY;
	}
}

static void dsi_fifo_status(struct msm_dsi_host *msm_host)
{
	u32 status;

	status = dsi_read(msm_host, REG_DSI_FIFO_STATUS);

	/* fifo underflow, overflow */
	if (status) {
		dsi_write(msm_host, REG_DSI_FIFO_STATUS, status);
		msm_host->err_work_state |= DSI_ERR_STATE_FIFO;
		if (status & DSI_FIFO_STATUS_CMD_MDP_FIFO_UNDERFLOW)
			msm_host->err_work_state |=
					DSI_ERR_STATE_MDP_FIFO_UNDERFLOW;
	}
}

static void dsi_status(struct msm_dsi_host *msm_host)
{
	u32 status;

	status = dsi_read(msm_host, REG_DSI_STATUS0);

	if (status & DSI_STATUS0_INTERLEAVE_OP_CONTENTION) {
		dsi_write(msm_host, REG_DSI_STATUS0, status);
		msm_host->err_work_state |=
			DSI_ERR_STATE_INTERLEAVE_OP_CONTENTION;
	}
}

static void dsi_clk_status(struct msm_dsi_host *msm_host)
{
	u32 status;

	status = dsi_read(msm_host, REG_DSI_CLK_STATUS);

	if (status & DSI_CLK_STATUS_PLL_UNLOCKED) {
		dsi_write(msm_host, REG_DSI_CLK_STATUS, status);
		msm_host->err_work_state |= DSI_ERR_STATE_PLL_UNLOCKED;
	}
}

static void dsi_error(struct msm_dsi_host *msm_host)
{
	/* disable dsi error interrupt */
	dsi_intr_ctrl(msm_host, DSI_IRQ_MASK_ERROR, 0);

	dsi_clk_status(msm_host);
	dsi_fifo_status(msm_host);
	dsi_ack_err_status(msm_host);
	dsi_timeout_status(msm_host);
	dsi_status(msm_host);
	dsi_dln0_phy_err(msm_host);

	queue_work(msm_host->workqueue, &msm_host->err_work);
}

static irqreturn_t dsi_host_irq(int irq, void *ptr)
{
	struct msm_dsi_host *msm_host = ptr;
	u32 isr;
	unsigned long flags;

	if (!msm_host->ctrl_base)
		return IRQ_HANDLED;

	spin_lock_irqsave(&msm_host->intr_lock, flags);
	isr = dsi_read(msm_host, REG_DSI_INTR_CTRL);
	dsi_write(msm_host, REG_DSI_INTR_CTRL, isr);
	spin_unlock_irqrestore(&msm_host->intr_lock, flags);

	DBG("isr=0x%x, id=%d", isr, msm_host->id);

	if (isr & DSI_IRQ_ERROR)
		dsi_error(msm_host);

	if (isr & DSI_IRQ_VIDEO_DONE)
		complete(&msm_host->video_comp);

	if (isr & DSI_IRQ_CMD_DMA_DONE)
		complete(&msm_host->dma_comp);

	return IRQ_HANDLED;
}

static int dsi_host_init_panel_gpios(struct msm_dsi_host *msm_host,
			struct device *panel_device)
{
	int ret;

	msm_host->disp_en_gpio = devm_gpiod_get(panel_device,
						"disp-enable");
	if (IS_ERR(msm_host->disp_en_gpio)) {
		DBG("cannot get disp-enable-gpios %ld",
				PTR_ERR(msm_host->disp_en_gpio));
		msm_host->disp_en_gpio = NULL;
	}
	if (msm_host->disp_en_gpio) {
		ret = gpiod_direction_output(msm_host->disp_en_gpio, 0);
		if (ret) {
			pr_err("cannot set dir to disp-en-gpios %d\n", ret);
			return ret;
		}
	}

	msm_host->te_gpio = devm_gpiod_get(panel_device, "disp-te");
	if (IS_ERR(msm_host->te_gpio)) {
		DBG("cannot get disp-te-gpios %ld", PTR_ERR(msm_host->te_gpio));
		msm_host->te_gpio = NULL;
	}

	if (msm_host->te_gpio) {
		ret = gpiod_direction_input(msm_host->te_gpio);
		if (ret) {
			pr_err("%s: cannot set dir to disp-te-gpios, %d\n",
				__func__, ret);
			return ret;
		}
	}

	return 0;
}

static int dsi_host_attach(struct mipi_dsi_host *host,
					struct mipi_dsi_device *dsi)
{
	struct msm_dsi_host *msm_host = to_msm_dsi_host(host);
	int ret;

	msm_host->channel = dsi->channel;
	msm_host->lanes = dsi->lanes;
	msm_host->format = dsi->format;
	msm_host->mode_flags = dsi->mode_flags;

	msm_host->panel_node = dsi->dev.of_node;

	/* Some gpios defined in panel DT need to be controlled by host */
	ret = dsi_host_init_panel_gpios(msm_host, &dsi->dev);
	if (ret)
		return ret;

	DBG("id=%d", msm_host->id);
	if (msm_host->dev)
		drm_helper_hpd_irq_event(msm_host->dev);

	return 0;
}

static int dsi_host_detach(struct mipi_dsi_host *host,
					struct mipi_dsi_device *dsi)
{
	struct msm_dsi_host *msm_host = to_msm_dsi_host(host);

	msm_host->panel_node = NULL;

	DBG("id=%d", msm_host->id);
	if (msm_host->dev)
		drm_helper_hpd_irq_event(msm_host->dev);

	return 0;
}

static ssize_t dsi_host_transfer(struct mipi_dsi_host *host,
					const struct mipi_dsi_msg *msg)
{
	struct msm_dsi_host *msm_host = to_msm_dsi_host(host);
	int ret;

	if (!msg || !msm_host->power_on)
		return -EINVAL;

	mutex_lock(&msm_host->cmd_mutex);
	ret = msm_dsi_manager_cmd_xfer(msm_host->id, msg);
	mutex_unlock(&msm_host->cmd_mutex);

	return ret;
}

static struct mipi_dsi_host_ops dsi_host_ops = {
	.attach = dsi_host_attach,
	.detach = dsi_host_detach,
	.transfer = dsi_host_transfer,
};

int msm_dsi_host_init(struct msm_dsi *msm_dsi)
{
	struct msm_dsi_host *msm_host = NULL;
	struct platform_device *pdev = msm_dsi->pdev;
	int ret;

	msm_host = devm_kzalloc(&pdev->dev, sizeof(*msm_host), GFP_KERNEL);
	if (!msm_host) {
		pr_err("%s: FAILED: cannot alloc dsi host\n",
		       __func__);
		ret = -ENOMEM;
		goto fail;
	}

	ret = of_property_read_u32(pdev->dev.of_node,
				"qcom,dsi-host-index", &msm_host->id);
	if (ret) {
		dev_err(&pdev->dev,
			"%s: host index not specified, ret=%d\n",
			__func__, ret);
		goto fail;
	}
	msm_host->pdev = pdev;

	ret = dsi_clk_init(msm_host);
	if (ret) {
		pr_err("%s: unable to initialize dsi clks\n", __func__);
		goto fail;
	}

	msm_host->ctrl_base = msm_ioremap(pdev, "dsi_ctrl", "DSI CTRL");
	if (IS_ERR(msm_host->ctrl_base)) {
		pr_err("%s: unable to map Dsi ctrl base\n", __func__);
		ret = PTR_ERR(msm_host->ctrl_base);
		goto fail;
	}

	msm_host->cfg = dsi_get_config(msm_host);
	if (!msm_host->cfg) {
		ret = -EINVAL;
		pr_err("%s: get config failed\n", __func__);
		goto fail;
	}

	ret = dsi_regulator_init(msm_host);
	if (ret) {
		pr_err("%s: regulator init failed\n", __func__);
		goto fail;
	}

	msm_host->rx_buf = devm_kzalloc(&pdev->dev, SZ_4K, GFP_KERNEL);
	if (!msm_host->rx_buf) {
		pr_err("%s: alloc rx temp buf failed\n", __func__);
		goto fail;
	}

	init_completion(&msm_host->dma_comp);
	init_completion(&msm_host->video_comp);
	mutex_init(&msm_host->dev_mutex);
	mutex_init(&msm_host->cmd_mutex);
	mutex_init(&msm_host->clk_mutex);
	spin_lock_init(&msm_host->intr_lock);

	/* setup workqueue */
	msm_host->workqueue = alloc_ordered_workqueue("dsi_drm_work", 0);
	INIT_WORK(&msm_host->err_work, dsi_err_worker);

	msm_dsi->phy = msm_dsi_phy_init(pdev, msm_host->cfg->phy_type,
					msm_host->id);
	if (!msm_dsi->phy) {
		ret = -EINVAL;
		pr_err("%s: phy init failed\n", __func__);
		goto fail;
	}
	msm_dsi->host = &msm_host->base;
	msm_dsi->id = msm_host->id;

	DBG("Dsi Host %d initialized", msm_host->id);
	return 0;

fail:
	return ret;
}

void msm_dsi_host_destroy(struct mipi_dsi_host *host)
{
	struct msm_dsi_host *msm_host = to_msm_dsi_host(host);

	DBG("");
	dsi_tx_buf_free(msm_host);
	if (msm_host->workqueue) {
		flush_workqueue(msm_host->workqueue);
		destroy_workqueue(msm_host->workqueue);
		msm_host->workqueue = NULL;
	}

	mutex_destroy(&msm_host->clk_mutex);
	mutex_destroy(&msm_host->cmd_mutex);
	mutex_destroy(&msm_host->dev_mutex);
}

int msm_dsi_host_modeset_init(struct mipi_dsi_host *host,
					struct drm_device *dev)
{
	struct msm_dsi_host *msm_host = to_msm_dsi_host(host);
	struct platform_device *pdev = msm_host->pdev;
	int ret;

	msm_host->irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
	if (msm_host->irq < 0) {
		ret = msm_host->irq;
		dev_err(dev->dev, "failed to get irq: %d\n", ret);
		return ret;
	}

	ret = devm_request_irq(&pdev->dev, msm_host->irq,
			dsi_host_irq, IRQF_TRIGGER_HIGH | IRQF_ONESHOT,
			"dsi_isr", msm_host);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to request IRQ%u: %d\n",
				msm_host->irq, ret);
		return ret;
	}

	msm_host->dev = dev;
	ret = dsi_tx_buf_alloc(msm_host, SZ_4K);
	if (ret) {
		pr_err("%s: alloc tx gem obj failed, %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

int msm_dsi_host_register(struct mipi_dsi_host *host, bool check_defer)
{
	struct msm_dsi_host *msm_host = to_msm_dsi_host(host);
	struct device_node *node;
	int ret;

	/* Register mipi dsi host */
	if (!msm_host->registered) {
		host->dev = &msm_host->pdev->dev;
		host->ops = &dsi_host_ops;
		ret = mipi_dsi_host_register(host);
		if (ret)
			return ret;

		msm_host->registered = true;

		/* If the panel driver has not been probed after host register,
		 * we should defer the host's probe.
		 * It makes sure panel is connected when fbcon detects
		 * connector status and gets the proper display mode to
		 * create framebuffer.
		 */
		if (check_defer) {
			node = of_get_child_by_name(msm_host->pdev->dev.of_node,
							"panel");
			if (node) {
				if (!of_drm_find_panel(node))
					return -EPROBE_DEFER;
			}
		}
	}

	return 0;
}

void msm_dsi_host_unregister(struct mipi_dsi_host *host)
{
	struct msm_dsi_host *msm_host = to_msm_dsi_host(host);

	if (msm_host->registered) {
		mipi_dsi_host_unregister(host);
		host->dev = NULL;
		host->ops = NULL;
		msm_host->registered = false;
	}
}

int msm_dsi_host_xfer_prepare(struct mipi_dsi_host *host,
				const struct mipi_dsi_msg *msg)
{
	struct msm_dsi_host *msm_host = to_msm_dsi_host(host);

	/* TODO: make sure dsi_cmd_mdp is idle.
	 * Since DSI6G v1.2.0, we can set DSI_TRIG_CTRL.BLOCK_DMA_WITHIN_FRAME
	 * to ask H/W to wait until cmd mdp is idle. S/W wait is not needed.
	 * How to handle the old versions? Wait for mdp cmd done?
	 */

	/*
	 * mdss interrupt is generated in mdp core clock domain
	 * mdp clock need to be enabled to receive dsi interrupt
	 */
	dsi_clk_ctrl(msm_host, 1);

	/* TODO: vote for bus bandwidth */

	if (!(msg->flags & MIPI_DSI_MSG_USE_LPM))
		dsi_set_tx_power_mode(0, msm_host);

	msm_host->dma_cmd_ctrl_restore = dsi_read(msm_host, REG_DSI_CTRL);
	dsi_write(msm_host, REG_DSI_CTRL,
		msm_host->dma_cmd_ctrl_restore |
		DSI_CTRL_CMD_MODE_EN |
		DSI_CTRL_ENABLE);
	dsi_intr_ctrl(msm_host, DSI_IRQ_MASK_CMD_DMA_DONE, 1);

	return 0;
}

void msm_dsi_host_xfer_restore(struct mipi_dsi_host *host,
				const struct mipi_dsi_msg *msg)
{
	struct msm_dsi_host *msm_host = to_msm_dsi_host(host);

	dsi_intr_ctrl(msm_host, DSI_IRQ_MASK_CMD_DMA_DONE, 0);
	dsi_write(msm_host, REG_DSI_CTRL, msm_host->dma_cmd_ctrl_restore);

	if (!(msg->flags & MIPI_DSI_MSG_USE_LPM))
		dsi_set_tx_power_mode(1, msm_host);

	/* TODO: unvote for bus bandwidth */

	dsi_clk_ctrl(msm_host, 0);
}

int msm_dsi_host_cmd_tx(struct mipi_dsi_host *host,
				const struct mipi_dsi_msg *msg)
{
	struct msm_dsi_host *msm_host = to_msm_dsi_host(host);

	return dsi_cmds2buf_tx(msm_host, msg);
}

int msm_dsi_host_cmd_rx(struct mipi_dsi_host *host,
				const struct mipi_dsi_msg *msg)
{
	struct msm_dsi_host *msm_host = to_msm_dsi_host(host);
	int data_byte, rx_byte, dlen, end;
	int short_response, diff, pkt_size, ret = 0;
	char cmd;
	int rlen = msg->rx_len;
	u8 *buf;

	if (rlen <= 2) {
		short_response = 1;
		pkt_size = rlen;
		rx_byte = 4;
	} else {
		short_response = 0;
		data_byte = 10;	/* first read */
		if (rlen < data_byte)
			pkt_size = rlen;
		else
			pkt_size = data_byte;
		rx_byte = data_byte + 6; /* 4 header + 2 crc */
	}

	buf = msm_host->rx_buf;
	end = 0;
	while (!end) {
		u8 tx[2] = {pkt_size & 0xff, pkt_size >> 8};
		struct mipi_dsi_msg max_pkt_size_msg = {
			.channel = msg->channel,
			.type = MIPI_DSI_SET_MAXIMUM_RETURN_PACKET_SIZE,
			.tx_len = 2,
			.tx_buf = tx,
		};

		DBG("rlen=%d pkt_size=%d rx_byte=%d",
			rlen, pkt_size, rx_byte);

		ret = dsi_cmds2buf_tx(msm_host, &max_pkt_size_msg);
		if (ret < 2) {
			pr_err("%s: Set max pkt size failed, %d\n",
				__func__, ret);
			return -EINVAL;
		}

		if ((msm_host->cfg->major == MSM_DSI_VER_MAJOR_6G) &&
			(msm_host->cfg->minor >= MSM_DSI_6G_VER_MINOR_V1_1)) {
			/* Clear the RDBK_DATA registers */
			dsi_write(msm_host, REG_DSI_RDBK_DATA_CTRL,
					DSI_RDBK_DATA_CTRL_CLR);
			wmb(); /* make sure the RDBK registers are cleared */
			dsi_write(msm_host, REG_DSI_RDBK_DATA_CTRL, 0);
			wmb(); /* release cleared status before transfer */
		}

		ret = dsi_cmds2buf_tx(msm_host, msg);
		if (ret < msg->tx_len) {
			pr_err("%s: Read cmd Tx failed, %d\n", __func__, ret);
			return ret;
		}

		/*
		 * once cmd_dma_done interrupt received,
		 * return data from client is ready and stored
		 * at RDBK_DATA register already
		 * since rx fifo is 16 bytes, dcs header is kept at first loop,
		 * after that dcs header lost during shift into registers
		 */
		dlen = dsi_cmd_dma_rx(msm_host, buf, rx_byte, pkt_size);

		if (dlen <= 0)
			return 0;

		if (short_response)
			break;

		if (rlen <= data_byte) {
			diff = data_byte - rlen;
			end = 1;
		} else {
			diff = 0;
			rlen -= data_byte;
		}

		if (!end) {
			dlen -= 2; /* 2 crc */
			dlen -= diff;
			buf += dlen;	/* next start position */
			data_byte = 14;	/* NOT first read */
			if (rlen < data_byte)
				pkt_size += rlen;
			else
				pkt_size += data_byte;
			DBG("buf=%p dlen=%d diff=%d", buf, dlen, diff);
		}
	}

	/*
	 * For single Long read, if the requested rlen < 10,
	 * we need to shift the start position of rx
	 * data buffer to skip the bytes which are not
	 * updated.
	 */
	if (pkt_size < 10 && !short_response)
		buf = msm_host->rx_buf + (10 - rlen);
	else
		buf = msm_host->rx_buf;

	cmd = buf[0];
	switch (cmd) {
	case MIPI_DSI_RX_ACKNOWLEDGE_AND_ERROR_REPORT:
		pr_err("%s: rx ACK_ERR_PACLAGE\n", __func__);
		ret = 0;
		break;
	case MIPI_DSI_RX_GENERIC_SHORT_READ_RESPONSE_1BYTE:
	case MIPI_DSI_RX_DCS_SHORT_READ_RESPONSE_1BYTE:
		ret = dsi_short_read1_resp(buf, msg);
		break;
	case MIPI_DSI_RX_GENERIC_SHORT_READ_RESPONSE_2BYTE:
	case MIPI_DSI_RX_DCS_SHORT_READ_RESPONSE_2BYTE:
		ret = dsi_short_read2_resp(buf, msg);
		break;
	case MIPI_DSI_RX_GENERIC_LONG_READ_RESPONSE:
	case MIPI_DSI_RX_DCS_LONG_READ_RESPONSE:
		ret = dsi_long_read_resp(buf, msg);
		break;
	default:
		pr_warn("%s:Invalid response cmd\n", __func__);
		ret = 0;
	}

	return ret;
}

void msm_dsi_host_cmd_xfer_commit(struct mipi_dsi_host *host, u32 iova, u32 len)
{
	struct msm_dsi_host *msm_host = to_msm_dsi_host(host);

	dsi_write(msm_host, REG_DSI_DMA_BASE, iova);
	dsi_write(msm_host, REG_DSI_DMA_LEN, len);
	dsi_write(msm_host, REG_DSI_TRIG_DMA, 1);

	/* Make sure trigger happens */
	wmb();
}

int msm_dsi_host_enable(struct mipi_dsi_host *host)
{
	struct msm_dsi_host *msm_host = to_msm_dsi_host(host);

	dsi_op_mode_config(msm_host,
		!!(msm_host->mode_flags & MIPI_DSI_MODE_VIDEO), true);

	/* TODO: clock should be turned off for command mode,
	 * and only turned on before MDP START.
	 * This part of code should be enabled once mdp driver support it.
	 */
	/* if (msm_panel->mode == MSM_DSI_CMD_MODE)
		dsi_clk_ctrl(msm_host, 0); */

	return 0;
}

int msm_dsi_host_disable(struct mipi_dsi_host *host)
{
	struct msm_dsi_host *msm_host = to_msm_dsi_host(host);

	dsi_op_mode_config(msm_host,
		!!(msm_host->mode_flags & MIPI_DSI_MODE_VIDEO), false);

	/* Since we have disabled INTF, the video engine won't stop so that
	 * the cmd engine will be blocked.
	 * Reset to disable video engine so that we can send off cmd.
	 */
	dsi_sw_reset(msm_host);

	return 0;
}

int msm_dsi_host_power_on(struct mipi_dsi_host *host)
{
	struct msm_dsi_host *msm_host = to_msm_dsi_host(host);
	u32 clk_pre = 0, clk_post = 0;
	int ret = 0;

	mutex_lock(&msm_host->dev_mutex);
	if (msm_host->power_on) {
		DBG("dsi host already on");
		goto unlock_ret;
	}

	ret = dsi_calc_clk_rate(msm_host);
	if (ret) {
		pr_err("%s: unable to calc clk rate, %d\n", __func__, ret);
		goto unlock_ret;
	}

	ret = dsi_host_regulator_enable(msm_host);
	if (ret) {
		pr_err("%s:Failed to enable vregs.ret=%d\n",
			__func__, ret);
		goto unlock_ret;
	}

	ret = dsi_bus_clk_enable(msm_host);
	if (ret) {
		pr_err("%s: failed to enable bus clocks, %d\n", __func__, ret);
		goto fail_disable_reg;
	}

	dsi_phy_sw_reset(msm_host);
	ret = msm_dsi_manager_phy_enable(msm_host->id,
					msm_host->byte_clk_rate * 8,
					clk_get_rate(msm_host->esc_clk),
					&clk_pre, &clk_post);
	dsi_bus_clk_disable(msm_host);
	if (ret) {
		pr_err("%s: failed to enable phy, %d\n", __func__, ret);
		goto fail_disable_reg;
	}

	ret = dsi_clk_ctrl(msm_host, 1);
	if (ret) {
		pr_err("%s: failed to enable clocks. ret=%d\n", __func__, ret);
		goto fail_disable_reg;
	}

	dsi_timing_setup(msm_host);
	dsi_sw_reset(msm_host);
	dsi_ctrl_config(msm_host, true, clk_pre, clk_post);

	if (msm_host->disp_en_gpio)
		gpiod_set_value(msm_host->disp_en_gpio, 1);

	msm_host->power_on = true;
	mutex_unlock(&msm_host->dev_mutex);

	return 0;

fail_disable_reg:
	dsi_host_regulator_disable(msm_host);
unlock_ret:
	mutex_unlock(&msm_host->dev_mutex);
	return ret;
}

int msm_dsi_host_power_off(struct mipi_dsi_host *host)
{
	struct msm_dsi_host *msm_host = to_msm_dsi_host(host);

	mutex_lock(&msm_host->dev_mutex);
	if (!msm_host->power_on) {
		DBG("dsi host already off");
		goto unlock_ret;
	}

	dsi_ctrl_config(msm_host, false, 0, 0);

	if (msm_host->disp_en_gpio)
		gpiod_set_value(msm_host->disp_en_gpio, 0);

	msm_dsi_manager_phy_disable(msm_host->id);

	dsi_clk_ctrl(msm_host, 0);

	dsi_host_regulator_disable(msm_host);

	DBG("-");

	msm_host->power_on = false;

unlock_ret:
	mutex_unlock(&msm_host->dev_mutex);
	return 0;
}

int msm_dsi_host_set_display_mode(struct mipi_dsi_host *host,
					struct drm_display_mode *mode)
{
	struct msm_dsi_host *msm_host = to_msm_dsi_host(host);

	if (msm_host->mode) {
		drm_mode_destroy(msm_host->dev, msm_host->mode);
		msm_host->mode = NULL;
	}

	msm_host->mode = drm_mode_duplicate(msm_host->dev, mode);
	if (IS_ERR(msm_host->mode)) {
		pr_err("%s: cannot duplicate mode\n", __func__);
		return PTR_ERR(msm_host->mode);
	}

	return 0;
}

struct drm_panel *msm_dsi_host_get_panel(struct mipi_dsi_host *host,
				unsigned long *panel_flags)
{
	struct msm_dsi_host *msm_host = to_msm_dsi_host(host);
	struct drm_panel *panel;

	panel = of_drm_find_panel(msm_host->panel_node);
	if (panel_flags)
			*panel_flags = msm_host->mode_flags;

	return panel;
}

