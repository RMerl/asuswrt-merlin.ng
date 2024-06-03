/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018 - Beniamino Galvani <b.galvani@gmail.com>
 * (C) Copyright 2018 - BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#ifndef CLK_MESON_H
#define CLK_MESON_H

/* Gate Structure */

struct meson_gate {
	unsigned int reg;
	unsigned int bit;
};

#define MESON_GATE(id, _reg, _bit)		\
	[id] = {				\
		.reg = (_reg),			\
		.bit = (_bit),			\
	}

/* PLL Parameters */

struct parm {
	u16 reg_off;
	u8 shift;
	u8 width;
};

#define PMASK(width)                    GENMASK(width - 1, 0)
#define SETPMASK(width, shift)          GENMASK(shift + width - 1, shift)
#define CLRPMASK(width, shift)          (~SETPMASK(width, shift))

#define PARM_GET(width, shift, reg)                                     \
	(((reg) & SETPMASK(width, shift)) >> (shift))
#define PARM_SET(width, shift, reg, val)                                \
	(((reg) & CLRPMASK(width, shift)) | ((val) << (shift)))

/* MPLL Parameters */

#define SDM_DEN 16384
#define N2_MIN  4
#define N2_MAX  511

#endif
