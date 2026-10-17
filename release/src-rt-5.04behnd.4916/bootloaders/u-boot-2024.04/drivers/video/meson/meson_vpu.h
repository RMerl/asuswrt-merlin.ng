/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Amlogic Meson Video Processing Unit driver
 *
 * Copyright (c) 2018 BayLibre, SAS.
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#ifndef __MESON_VPU_H__
#define __MESON_VPU_H__

#include <video.h>
#include "meson_registers.h"

struct display_timing;
struct udevice;

enum {
	/* Maximum size we support */
	VPU_MAX_WIDTH		= 3840,
	VPU_MAX_HEIGHT		= 2160,
	VPU_MAX_LOG2_BPP	= VIDEO_BPP32,
};

enum vpu_compatible {
	VPU_COMPATIBLE_GXBB = 0,
	VPU_COMPATIBLE_GXL = 1,
	VPU_COMPATIBLE_GXM = 2,
	VPU_COMPATIBLE_G12A = 3,
};

struct meson_vpu_priv {
	struct udevice *dev;
	void __iomem *io_base;
	void __iomem *hhi_base;
	void __iomem *dmc_base;
};

bool meson_vpu_is_compatible(struct meson_vpu_priv *priv,
			     enum vpu_compatible family);

#define hhi_update_bits(offset, mask, value) \
	writel_bits(mask, value, priv->hhi_base + offset)

#define hhi_write(offset, value) \
	writel(value, priv->hhi_base + offset)

#define hhi_read(offset) \
	readl(priv->hhi_base + offset)

#define dmc_update_bits(offset, mask, value) \
	writel_bits(mask, value, priv->dmc_base + offset)

#define dmc_write(offset, value) \
	writel(value, priv->dmc_base + offset)

#define dmc_read(offset) \
	readl(priv->dmc_base + offset)

#define MESON_CANVAS_ID_OSD1	0x4e

/* Canvas configuration. */
#define MESON_CANVAS_WRAP_NONE	0x00
#define	MESON_CANVAS_WRAP_X	0x01
#define	MESON_CANVAS_WRAP_Y	0x02

#define	MESON_CANVAS_BLKMODE_LINEAR	0x00
#define	MESON_CANVAS_BLKMODE_32x32	0x01
#define	MESON_CANVAS_BLKMODE_64x64	0x02

void meson_canvas_setup(struct meson_vpu_priv *priv,
			u32 canvas_index, u32 addr,
			u32 stride, u32 height,
			unsigned int wrap,
			unsigned int blkmode);

/* Mux VIU/VPP to ENCI */
#define MESON_VIU_VPP_MUX_ENCI	0x5
/* Mux VIU/VPP to ENCP */
#define MESON_VIU_VPP_MUX_ENCP	0xA

void meson_vpp_setup_mux(struct meson_vpu_priv *priv, unsigned int mux);
void meson_vpu_init(struct udevice *dev);
void meson_vpu_setup_plane(struct udevice *dev, bool is_interlaced);
bool meson_venc_hdmi_supported_mode(const struct display_timing *mode);
void meson_vpu_setup_venc(struct udevice *dev,
			  const struct display_timing *mode, bool is_cvbs);
bool meson_vclk_dmt_supported_freq(struct meson_vpu_priv *priv,
				   unsigned int freq);
void meson_vpu_setup_vclk(struct udevice *dev,
			  const struct display_timing *mode, bool is_cvbs);
#endif
