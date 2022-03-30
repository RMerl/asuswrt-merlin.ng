/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2016 Google, Inc
 */
#ifndef _ASM_ARCH_TIMER_H
#define _ASM_ARCH_TIMER_H

/* Each timer has 4 control bits in ctrl1 register.
 * Timer1 uses bits 0:3, Timer2 uses bits 4:7 and so on,
 * such that timer X uses bits (4 * X - 4):(4 * X - 1)
 * If the timer does not support PWM, bit 4 is reserved.
 */
#define AST_TMC_EN			(1 << 0)
#define AST_TMC_1MHZ			(1 << 1)
#define AST_TMC_OVFINTR			(1 << 2)
#define AST_TMC_PWM			(1 << 3)

/* Timers are counted from 1 in the datasheet. */
#define AST_TMC_CTRL1_SHIFT(n)			(4 * ((n) - 1))

#define AST_TMC_RATE  (1000*1000)

#ifndef __ASSEMBLY__

/*
 * All timers share control registers, which makes it harder to make them
 * separate devices. Since only one timer is needed at the moment, making
 * it this just one device.
 */

struct ast_timer_counter {
	u32 status;
	u32 reload_val;
	u32 match1;
	u32 match2;
};

struct ast_timer {
	struct ast_timer_counter timers1[3];
	u32 ctrl1;
	u32 ctrl2;
#ifdef CONFIG_ASPEED_AST2500
	u32 ctrl3;
	u32 ctrl1_clr;
#else
	u32 reserved[2];
#endif
	struct ast_timer_counter timers2[5];
};

#endif  /* __ASSEMBLY__ */

#endif  /* _ASM_ARCH_TIMER_H */
