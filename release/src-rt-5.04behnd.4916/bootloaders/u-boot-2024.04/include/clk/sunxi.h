// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Amarula Solutions.
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 */

#ifndef _CLK_SUNXI_H
#define _CLK_SUNXI_H

#include <linux/bitops.h>

/**
 * enum ccu_flags - ccu clock/reset flags
 *
 * @CCU_CLK_F_IS_VALID:		is given clock gate is valid?
 * @CCU_RST_F_IS_VALID:		is given reset control is valid?
 */
enum ccu_flags {
	CCU_CLK_F_IS_VALID		= BIT(0),
	CCU_RST_F_IS_VALID		= BIT(1),
	CCU_CLK_F_DUMMY_GATE		= BIT(2),
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

#define GATE_DUMMY {				\
	.flags = CCU_CLK_F_DUMMY_GATE,		\
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
	u8 num_gates;
	u8 num_resets;
};

/**
 * struct ccu_plat - sunxi clock control unit platform data
 *
 * @base:	base address
 * @desc:	ccu descriptor
 */
struct ccu_plat {
	void *base;
	const struct ccu_desc *desc;
};

extern struct clk_ops sunxi_clk_ops;

#endif /* _CLK_SUNXI_H */
