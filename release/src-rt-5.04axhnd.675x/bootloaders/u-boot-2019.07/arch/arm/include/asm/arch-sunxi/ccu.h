// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Amarula Solutions.
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 */

#ifndef _ASM_ARCH_CCU_H
#define _ASM_ARCH_CCU_H

/**
 * enum ccu_flags - ccu clock/reset flags
 *
 * @CCU_CLK_F_IS_VALID:		is given clock gate is valid?
 * @CCU_RST_F_IS_VALID:		is given reset control is valid?
 */
enum ccu_flags {
	CCU_CLK_F_IS_VALID		= BIT(0),
	CCU_RST_F_IS_VALID		= BIT(1),
};

/**
 * struct ccu_clk_gate - ccu clock gate
 * @off:	gate offset
 * @bit:	gate bit
 * @flags:	ccu clock gate flags
 */
struct ccu_clk_gate {
	u16 off;
	u32 bit;
	enum ccu_flags flags;
};

#define GATE(_off, _bit) {			\
	.off = _off,				\
	.bit = _bit,				\
	.flags = CCU_CLK_F_IS_VALID,		\
}

/**
 * struct ccu_reset - ccu reset
 * @off:	reset offset
 * @bit:	reset bit
 * @flags:	ccu reset control flags
 */
struct ccu_reset {
	u16 off;
	u32 bit;
	enum ccu_flags flags;
};

#define RESET(_off, _bit) {			\
	.off = _off,				\
	.bit = _bit,				\
	.flags = CCU_RST_F_IS_VALID,		\
}

/**
 * struct ccu_desc - clock control unit descriptor
 *
 * @gates:	clock gates
 * @resets:	reset unit
 */
struct ccu_desc {
	const struct ccu_clk_gate *gates;
	const struct ccu_reset *resets;
};

/**
 * struct ccu_priv - sunxi clock control unit
 *
 * @base:	base address
 * @desc:	ccu descriptor
 */
struct ccu_priv {
	void *base;
	const struct ccu_desc *desc;
};

/**
 * sunxi_clk_probe - common sunxi clock probe
 * @dev:	clock device
 */
int sunxi_clk_probe(struct udevice *dev);

extern struct clk_ops sunxi_clk_ops;

/**
 * sunxi_reset_bind() - reset binding
 *
 * @dev:       reset device
 * @count:     reset count
 * @return 0 success, or error value
 */
int sunxi_reset_bind(struct udevice *dev, ulong count);

#endif /* _ASM_ARCH_CCU_H */
