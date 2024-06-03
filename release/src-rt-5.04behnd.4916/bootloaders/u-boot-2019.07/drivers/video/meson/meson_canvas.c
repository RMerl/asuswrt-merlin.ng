// SPDX-License-Identifier: GPL-2.0
/*
 * Amlogic Meson Video Processing Unit driver
 *
 * Copyright (c) 2018 BayLibre, SAS.
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#include "meson_vpu.h"

/* DMC Registers */
#define DMC_CAV_LUT_DATAL	0x48 /* 0x12 offset in data sheet */
#define CANVAS_WIDTH_LBIT	29
#define CANVAS_WIDTH_LWID	3
#define DMC_CAV_LUT_DATAH	0x4c /* 0x13 offset in data sheet */
#define CANVAS_WIDTH_HBIT	0
#define CANVAS_HEIGHT_BIT	9
#define CANVAS_BLKMODE_BIT	24
#define DMC_CAV_LUT_ADDR	0x50 /* 0x14 offset in data sheet */
#define CANVAS_LUT_WR_EN	(0x2 << 8)
#define CANVAS_LUT_RD_EN	(0x1 << 8)

void meson_canvas_setup(struct meson_vpu_priv *priv,
			u32 canvas_index, u32 addr,
			u32 stride, u32 height,
			unsigned int wrap,
			unsigned int blkmode)
{
	dmc_write(DMC_CAV_LUT_DATAL,
		  (((addr + 7) >> 3)) |
		  (((stride + 7) >> 3) << CANVAS_WIDTH_LBIT));

	dmc_write(DMC_CAV_LUT_DATAH,
		  ((((stride + 7) >> 3) >> CANVAS_WIDTH_LWID) <<
						CANVAS_WIDTH_HBIT) |
		  (height << CANVAS_HEIGHT_BIT) |
		  (wrap << 22) |
		  (blkmode << CANVAS_BLKMODE_BIT));

	dmc_write(DMC_CAV_LUT_ADDR,
		  CANVAS_LUT_WR_EN | canvas_index);

	/* Force a read-back to make sure everything is flushed. */
	dmc_read(DMC_CAV_LUT_DATAH);
}
