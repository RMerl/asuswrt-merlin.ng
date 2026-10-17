/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * SoC-specific lowlevel code for DA850
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */
#ifndef __DA850_LOWLEVEL_H
#define __DA850_LOWLEVEL_H

#include <asm/arch/pinmux_defs.h>

/* pinmux_resource[] vector is defined in the board specific file */
extern const struct pinmux_resource pinmuxes[];
extern const int pinmuxes_size;

extern const struct lpsc_resource lpsc[];
extern const int lpsc_size;

/* NOR Boot Configuration Word Field Descriptions */
#define DA850_NORBOOT_COPY_XK(X)	((X - 1) << 8)
#define DA850_NORBOOT_METHOD_DIRECT	(1 << 4)
#define DA850_NORBOOT_16BIT		(1 << 0)

#define dv_maskbits(addr, val) \
	writel((readl(addr) & val), addr)

void da850_lpc_transition(unsigned char pscnum, unsigned char module,
		unsigned char domain, unsigned char state);
void da850_psc_init(void);
void da850_pinmux_ctl(unsigned long offset, unsigned long mask,
	unsigned long value);

#endif /* #ifndef __DA850_LOWLEVEL_H */
