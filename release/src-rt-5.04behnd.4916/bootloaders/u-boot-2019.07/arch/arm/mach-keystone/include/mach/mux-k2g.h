/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * K2G: Pinmux configuration
 *
 * (C) Copyright 2015
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef __ASM_ARCH_MUX_K2G_H
#define __ASM_ARCH_MUX_K2G_H

#include <common.h>
#include <asm/io.h>

#define K2G_PADCFG_REG	(KS2_DEVICE_STATE_CTRL_BASE + 0x1000)

/*
 * 20:19 - buffer class RW fixed
 * 18    - rxactive (Input enabled for the pad ) 0 - Di; 1 - En;
 * 17    - pulltypesel (0 - PULLDOWN; 1 - PULLUP);
 * 16    - pulluden (0 - PULLUP/DOWN EN; 1 - DI);
 * 3:0   - muxmode (available modes 0:5)
 */

#define PIN_IEN	(1 << 18) /* pin input enabled */
#define PIN_PDIS	(1 << 16) /* pull up/down disabled */
#define PIN_PTU	(1 << 17) /* pull up */
#define PIN_PTD	(0 << 17) /* pull down */

#define BUFFER_CLASS_B	(0 << 19)
#define BUFFER_CLASS_C	(1 << 19)
#define BUFFER_CLASS_D	(2 << 19)
#define BUFFER_CLASS_E	(3 << 19)

#define MODE(m)	((m) & 0x7)
#define MAX_PIN_N	260

#define MUX_CFG(value, index)  \
	__raw_writel(\
		     (value) | \
		     (__raw_readl(K2G_PADCFG_REG + (index << 2)) & \
		      (0x3 << 19)),\
		     (K2G_PADCFG_REG + (index << 2))\
		    );

struct pin_cfg {
	int	reg_inx;
	u32	val;
};

static inline void configure_pin_mux(struct pin_cfg *pin_mux)
{
	if (!pin_mux)
		return;

	while ((pin_mux->reg_inx >= 0) && (pin_mux->reg_inx < MAX_PIN_N)) {
		MUX_CFG(pin_mux->val, pin_mux->reg_inx);
		pin_mux++;
	}
}

#endif /* __ASM_ARCH_MUX_K2G_H */
