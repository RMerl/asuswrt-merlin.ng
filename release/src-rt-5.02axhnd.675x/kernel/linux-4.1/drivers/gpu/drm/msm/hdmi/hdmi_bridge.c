/*
 * Copyright (C) 2013 Red Hat
 * Author: Rob Clark <robdclark@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "hdmi.h"

struct hdmi_bridge {
	struct drm_bridge base;
	struct hdmi *hdmi;
};
#define to_hdmi_bridge(x) container_of(x, struct hdmi_bridge, base)

void hdmi_bridge_destroy(struct drm_bridge *bridge)
{
}

static void power_on(struct drm_bridge *bridge)
{
	struct drm_device *dev = bridge->dev;
	struct hdmi_bridge *hdmi_bridge = to_hdmi_bridge(bridge);
	struct hdmi *hdmi = hdmi_bridge->hdmi;
	const struct hdmi_platform_config *config = hdmi->config;
	int i, ret;

	for (i = 0; i < config->pwr_reg_cnt; i++) {
		ret = regulator_enable(hdmi->pwr_regs[i]);
		if (ret) {
			dev_err(dev->dev, "failed to enable pwr regulator: %s (%d)\n",
					config->pwr_reg_names[i], ret);
		}
	}

	if (config->pwr_clk_cnt > 0) {
		DBG("pixclock: %lu", hdmi->pixclock);
		ret = clk_set_rate(hdmi->pwr_clks[0], hdmi->pixclock);
		if (ret) {
			dev_err(dev->dev, "failed to set pixel clk: %s (%d)\n",
					config->pwr_clk_names[0], ret);
		}
	}

	for (i = 0; i < config->pwr_clk_cnt; i++) {
		ret = clk_prepare_enable(hdmi->pwr_clks[i]);
		if (ret) {
			dev_err(dev->dev, "failed to enable pwr clk: %s (%d)\n",
					config->pwr_clk_names[i], ret);
		}
	}
}

static void power_off(struct drm_bridge *bridge)
{
	struct drm_device *dev = bridge->dev;
	struct hdmi_bridge *hdmi_bridge = to_hdmi_bridge(bridge);
	struct hdmi *hdmi = hdmi_bridge->hdmi;
	const struct hdmi_platform_config *config = hdmi->config;
	int i, ret;

	/* TODO do we need to wait for final vblank somewhere before
	 * cutting the clocks?
	 */
	mdelay(16 + 4);

	for (i = 0; i < config->pwr_clk_cnt; i++)
		clk_disable_unprepare(hdmi->pwr_clks[i]);

	for (i = 0; i < config->pwr_reg_cnt; i++) {
		ret = regulator_disable(hdmi->pwr_regs[i]);
		if (ret) {
			dev_err(dev->dev, "failed to disable pwr regulator: %s (%d)\n",
					config->pwr_reg_names[i], ret);
		}
	}
}

static void hdmi_bridge_pre_enable(struct drm_bridge *bridge)
{
	struct hdmi_bridge *hdmi_bridge = to_hdmi_bridge(bridge);
	struct hdmi *hdmi = hdmi_bridge->hdmi;
	struct hdmi_phy *phy = hdmi->phy;

	DBG("power up");

	if (!hdmi->power_on) {
		power_on(bridge);
		hdmi->power_on = true;
		hdmi_audio_update(hdmi);
	}

	phy->funcs->powerup(phy, hdmi->pixclock);
	hdmi_set_mode(hdmi, true);
}

static void hdmi_bridge_enable(struct drm_bridge *bridge)
{
}

static void hdmi_bridge_disable(struct drm_bridge *bridge)
{
}

static void hdmi_bridge_post_disable(struct drm_bridge *bridge)
{
	struct hdmi_bridge *hdmi_bridge = to_hdmi_bridge(bridge);
	struct hdmi *hdmi = hdmi_bridge->hdmi;
	struct hdmi_phy *phy = hdmi->phy;

	DBG("power down");
	hdmi_set_mode(hdmi, false);
	phy->funcs->powerdown(phy);

	if (hdmi->power_on) {
		power_off(bridge);
		hdmi->power_on = false;
		hdmi_audio_update(hdmi);
	}
}

static void hdmi_bridge_mode_set(struct drm_bridge *bridge,
		 struct drm_display_mode *mode,
		 struct drm_display_mode *adjusted_mode)
{
	struct hdmi_bridge *hdmi_bridge = to_hdmi_bridge(bridge);
	struct hdmi *hdmi = hdmi_bridge->hdmi;
	int hstart, hend, vstart, vend;
	uint32_t frame_ctrl;

	mode = adjusted_mode;

	hdmi->pixclock = mode->clock * 1000;

	hdmi->hdmi_mode = drm_match_cea_mode(mode) > 1;

	hstart = mode->htotal - mode->hsync_start;
	hend   = mode->htotal - mode->hsync_start + mode->hdisplay;

	vstart = mode->vtotal - mode->vsync_start - 1;
	vend   = mode->vtotal - mode->vsync_start + mode->vdisplay - 1;

	DBG("htotal=%d, vtotal=%d, hstart=%d, hend=%d, vstart=%d, vend=%d",
			mode->htotal, mode->vtotal, hstart, hend, vstart, vend);

	hdmi_write(hdmi, REG_HDMI_TOTAL,
			HDMI_TOTAL_H_TOTAL(mode->htotal - 1) |
			HDMI_TOTAL_V_TOTAL(mode->vtotal - 1));

	hdmi_write(hdmi, REG_HDMI_ACTIVE_HSYNC,
			HDMI_ACTIVE_HSYNC_START(hstart) |
			HDMI_ACTIVE_HSYNC_END(hend));
	hdmi_write(hdmi, REG_HDMI_ACTIVE_VSYNC,
			HDMI_ACTIVE_VSYNC_START(vstart) |
			HDMI_ACTIVE_VSYNC_END(vend));

	if (mode->flags & DRM_MODE_FLAG_INTERLACE) {
		hdmi_write(hdmi, REG_HDMI_VSYNC_TOTAL_F2,
				HDMI_VSYNC_TOTAL_F2_V_TOTAL(mode->vtotal));
		hdmi_write(hdmi, REG_HDMI_VSYNC_ACTIVE_F2,
				HDMI_VSYNC_ACTIVE_F2_START(vstart + 1) |
				HDMI_VSYNC_ACTIVE_F2_END(vend + 1));
	} else {
		hdmi_write(hdmi, REG_HDMI_VSYNC_TOTAL_F2,
				HDMI_VSYNC_TOTAL_F2_V_TOTAL(0));
		hdmi_write(hdmi, REG_HDMI_VSYNC_ACTIVE_F2,
				HDMI_VSYNC_ACTIVE_F2_START(0) |
				HDMI_VSYNC_ACTIVE_F2_END(0));
	}

	frame_ctrl = 0;
	if (mode->flags & DRM_MODE_FLAG_NHSYNC)
		frame_ctrl |= HDMI_FRAME_CTRL_HSYNC_LOW;
	if (mode->flags & DRM_MODE_FLAG_NVSYNC)
		frame_ctrl |= HDMI_FRAME_CTRL_VSYNC_LOW;
	if (mode->flags & DRM_MODE_FLAG_INTERLACE)
		frame_ctrl |= HDMI_FRAME_CTRL_INTERLACED_EN;
	DBG("frame_ctrl=%08x", frame_ctrl);
	hdmi_write(hdmi, REG_HDMI_FRAME_CTRL, frame_ctrl);

	hdmi_audio_update(hdmi);
}

static const struct drm_bridge_funcs hdmi_bridge_funcs = {
		.pre_enable = hdmi_bridge_pre_enable,
		.enable = hdmi_bridge_enable,
		.disable = hdmi_bridge_disable,
		.post_disable = hdmi_bridge_post_disable,
		.mode_set = hdmi_bridge_mode_set,
};


/* initialize bridge */
struct drm_bridge *hdmi_bridge_init(struct hdmi *hdmi)
{
	struct drm_bridge *bridge = NULL;
	struct hdmi_bridge *hdmi_bridge;
	int ret;

	hdmi_bridge = devm_kzalloc(hdmi->dev->dev,
			sizeof(*hdmi_bridge), GFP_KERNEL);
	if (!hdmi_bridge) {
		ret = -ENOMEM;
		goto fail;
	}

	hdmi_bridge->hdmi = hdmi;

	bridge = &hdmi_bridge->base;
	bridge->funcs = &hdmi_bridge_funcs;

	ret = drm_bridge_attach(hdmi->dev, bridge);
	if (ret)
		goto fail;

	return bridge;

fail:
	if (bridge)
		hdmi_bridge_destroy(bridge);

	return ERR_PTR(ret);
}
