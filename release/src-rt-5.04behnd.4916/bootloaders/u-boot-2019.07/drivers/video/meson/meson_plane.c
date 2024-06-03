// SPDX-License-Identifier: GPL-2.0
/*
 * Amlogic Meson Video Processing Unit driver
 *
 * Copyright (c) 2018 BayLibre, SAS.
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#include "meson_vpu.h"

/* OSDx_BLKx_CFG */
#define OSD_CANVAS_SEL		16

#define OSD_ENDIANNESS_LE	BIT(15)
#define OSD_ENDIANNESS_BE	(0)

#define OSD_BLK_MODE_422	(0x03 << 8)
#define OSD_BLK_MODE_16		(0x04 << 8)
#define OSD_BLK_MODE_32		(0x05 << 8)
#define OSD_BLK_MODE_24		(0x07 << 8)

#define OSD_OUTPUT_COLOR_RGB	BIT(7)
#define OSD_OUTPUT_COLOR_YUV	(0)

#define OSD_COLOR_MATRIX_32_RGBA	(0x00 << 2)
#define OSD_COLOR_MATRIX_32_ARGB	(0x01 << 2)
#define OSD_COLOR_MATRIX_32_ABGR	(0x02 << 2)
#define OSD_COLOR_MATRIX_32_BGRA	(0x03 << 2)

#define OSD_COLOR_MATRIX_24_RGB		(0x00 << 2)

#define OSD_COLOR_MATRIX_16_RGB655	(0x00 << 2)
#define OSD_COLOR_MATRIX_16_RGB565	(0x04 << 2)

#define OSD_INTERLACE_ENABLED	BIT(1)
#define OSD_INTERLACE_ODD	BIT(0)
#define OSD_INTERLACE_EVEN	(0)

/* OSDx_CTRL_STAT */
#define OSD_ENABLE		BIT(21)
#define OSD_BLK0_ENABLE		BIT(0)

#define OSD_GLOBAL_ALPHA_SHIFT	12

/* OSDx_CTRL_STAT2 */
#define OSD_REPLACE_EN		BIT(14)
#define OSD_REPLACE_SHIFT	6

/*
 * When the output is interlaced, the OSD must switch between
 * each field using the INTERLACE_SEL_ODD (0) of VIU_OSD1_BLK0_CFG_W0
 * at each vsync.
 * But the vertical scaler can provide such funtionnality if
 * is configured for 2:1 scaling with interlace options enabled.
 */
static void meson_vpp_setup_interlace_vscaler_osd1(struct meson_vpu_priv *priv,
						   struct video_priv *uc_priv)
{
	writel(BIT(3) /* Enable scaler */ |
	       BIT(2), /* Select OSD1 */
	       priv->io_base + _REG(VPP_OSD_SC_CTRL0));

	writel(((uc_priv->xsize - 1) << 16) | (uc_priv->ysize - 1),
	       priv->io_base + _REG(VPP_OSD_SCI_WH_M1));
	/* 2:1 scaling */
	writel((0 << 16) | uc_priv->xsize,
	       priv->io_base + _REG(VPP_OSD_SCO_H_START_END));
	writel(((0 >> 1) << 16) | (uc_priv->ysize >> 1),
	       priv->io_base + _REG(VPP_OSD_SCO_V_START_END));

	/* 2:1 scaling values */
	writel(BIT(16), priv->io_base + _REG(VPP_OSD_VSC_INI_PHASE));
	writel(BIT(25), priv->io_base + _REG(VPP_OSD_VSC_PHASE_STEP));

	writel(0, priv->io_base + _REG(VPP_OSD_HSC_CTRL0));

	writel((4 << 0)  /* osd_vsc_bank_length */ |
	       (4 << 3)  /* osd_vsc_top_ini_rcv_num0 */ |
	       (1 << 8)  /* osd_vsc_top_rpt_p0_num0 */ |
	       (6 << 11) /* osd_vsc_bot_ini_rcv_num0 */ |
	       (2 << 16) /* osd_vsc_bot_rpt_p0_num0 */ |
	       BIT(23)	 /* osd_prog_interlace */ |
	       BIT(24),  /* Enable vertical scaler */
	       priv->io_base + _REG(VPP_OSD_VSC_CTRL0));
}

static void
meson_vpp_disable_interlace_vscaler_osd1(struct meson_vpu_priv *priv)
{
	writel(0, priv->io_base + _REG(VPP_OSD_SC_CTRL0));
	writel(0, priv->io_base + _REG(VPP_OSD_VSC_CTRL0));
	writel(0, priv->io_base + _REG(VPP_OSD_HSC_CTRL0));
}

void meson_vpu_setup_plane(struct udevice *dev, bool is_interlaced)
{
	struct video_uc_platdata *uc_plat = dev_get_uclass_platdata(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct meson_vpu_priv *priv = dev_get_priv(dev);
	u32 osd1_ctrl_stat;
	u32 osd1_blk0_cfg[5];
	bool osd1_interlace;
	unsigned int src_x1, src_x2, src_y1, src_y2;
	unsigned int dest_x1, dest_x2, dest_y1, dest_y2;

	dest_x1 = src_x1 = 0;
	dest_x2 = src_x2 = uc_priv->xsize;
	dest_y1 = src_y1 = 0;
	dest_y2 = src_y2 = uc_priv->ysize;

	/* Enable VPP Postblend */
	writel(uc_priv->xsize,
	       priv->io_base + _REG(VPP_POSTBLEND_H_SIZE));

	writel_bits(VPP_POSTBLEND_ENABLE, VPP_POSTBLEND_ENABLE,
		    priv->io_base + _REG(VPP_MISC));

	/* uc_plat->base is the framebuffer */

	/* Enable OSD and BLK0, set max global alpha */
	osd1_ctrl_stat = OSD_ENABLE | (0xFF << OSD_GLOBAL_ALPHA_SHIFT) |
			 OSD_BLK0_ENABLE;

	/* Set up BLK0 to point to the right canvas */
	osd1_blk0_cfg[0] = ((MESON_CANVAS_ID_OSD1 << OSD_CANVAS_SEL) |
			   OSD_ENDIANNESS_LE);

	/* On GXBB, Use the old non-HDR RGB2YUV converter */
	if (meson_vpu_is_compatible(priv, VPU_COMPATIBLE_GXBB))
		osd1_blk0_cfg[0] |= OSD_OUTPUT_COLOR_RGB;

	/* For XRGB, replace the pixel's alpha by 0xFF */
	writel_bits(OSD_REPLACE_EN, OSD_REPLACE_EN,
		    priv->io_base + _REG(VIU_OSD1_CTRL_STAT2));
	osd1_blk0_cfg[0] |= OSD_BLK_MODE_32 |
		OSD_COLOR_MATRIX_32_ARGB;

	if (is_interlaced) {
		osd1_interlace = true;
		dest_y1 /= 2;
		dest_y2 /= 2;
	} else {
		osd1_interlace = false;
	}

	/*
	 * The format of these registers is (x2 << 16 | x1),
	 * where x2 is exclusive.
	 * e.g. +30x1920 would be (1919 << 16) | 30
	 */
	osd1_blk0_cfg[1] = ((src_x2 - 1) << 16) | src_x1;
	osd1_blk0_cfg[2] = ((src_y2 - 1) << 16) | src_y1;
	osd1_blk0_cfg[3] = ((dest_x2 - 1) << 16) | dest_x1;
	osd1_blk0_cfg[4] = ((dest_y2 - 1) << 16) | dest_y1;

	writel(osd1_ctrl_stat, priv->io_base + _REG(VIU_OSD1_CTRL_STAT));
	writel(osd1_blk0_cfg[0], priv->io_base + _REG(VIU_OSD1_BLK0_CFG_W0));
	writel(osd1_blk0_cfg[1], priv->io_base + _REG(VIU_OSD1_BLK0_CFG_W1));
	writel(osd1_blk0_cfg[2], priv->io_base + _REG(VIU_OSD1_BLK0_CFG_W2));
	writel(osd1_blk0_cfg[3], priv->io_base + _REG(VIU_OSD1_BLK0_CFG_W3));
	writel(osd1_blk0_cfg[4], priv->io_base + _REG(VIU_OSD1_BLK0_CFG_W4));

	/* If output is interlace, make use of the Scaler */
	if (osd1_interlace)
		meson_vpp_setup_interlace_vscaler_osd1(priv, uc_priv);
	else
		meson_vpp_disable_interlace_vscaler_osd1(priv);

	meson_canvas_setup(priv, MESON_CANVAS_ID_OSD1,
			   uc_plat->base, uc_priv->xsize * 4,
			   uc_priv->ysize, MESON_CANVAS_WRAP_NONE,
			   MESON_CANVAS_BLKMODE_LINEAR);

	/* Enable OSD1 */
	writel_bits(VPP_OSD1_POSTBLEND, VPP_OSD1_POSTBLEND,
		    priv->io_base + _REG(VPP_MISC));
}
