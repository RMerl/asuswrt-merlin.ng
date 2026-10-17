// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2012 Freescale Semiconductor, Inc.
 */

#ifndef __PINCTRL_MXS_H
#define __PINCTRL_MXS_H

#include <dm/pinctrl.h>

#define SET	0x4
#define CLR	0x8
#define TOG	0xc

#define MXS_PINCTRL_PIN(pin)	PINCTRL_PIN(pin, #pin)
#define PINID(bank, pin)	((bank) * 32 + (pin))

/*
 * pinmux-id bit field definitions
 *
 * bank:	15..12	(4)
 * pin:		11..4	(8)
 * muxsel:	3..0	(4)
 */
#define MUXID_TO_PINID(m)	PINID((m) >> 12 & 0xf, (m) >> 4 & 0xff)
#define MUXID_TO_MUXSEL(m)	((m) & 0xf)

#define PINID_TO_BANK(p)	((p) >> 5)
#define PINID_TO_PIN(p)		((p) % 32)

/*
 * pin config bit field definitions
 *
 * pull-up:	6..5	(2)
 * voltage:	4..3	(2)
 * mA:		2..0	(3)
 *
 * MSB of each field is presence bit for the config.
 */
#define PULL_PRESENT		(1 << 6)
#define PULL_SHIFT		5
#define VOL_PRESENT		(1 << 4)
#define VOL_SHIFT		3
#define MA_PRESENT		(1 << 2)
#define MA_SHIFT		0
#define CFG_TO_PULL(c)	((c) >> PULL_SHIFT & 0x1)
#define CFG_TO_VOL(c)	((c) >> VOL_SHIFT & 0x1)
#define CFG_TO_MA(c)		((c) >> MA_SHIFT & 0x3)

struct mxs_regs {
	u16 muxsel;
	u16 drive;
	u16 pull;
};

static inline void mxs_pinctrl_rmwl(u32 value, u32 mask, u8 shift,
				    void __iomem *reg)
{
	clrsetbits_le32(reg, mask << shift, value << shift);
}
#endif /* __PINCTRL_MXS_H */
