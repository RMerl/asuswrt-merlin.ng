/* SPDX-License-Identifier: GPL-2.0+
 *
 * (C) Copyright 2016 Nexell
 * Hyunseok, Jung <hsjung@nexell.co.kr>
 */

#ifndef __ASM_ARM_ARCH_CLK_H_
#define __ASM_ARM_ARCH_CLK_H_

struct clk {
	unsigned long rate;
};

void clk_init(void);

struct clk *clk_get(const char *id);
void clk_put(struct clk *clk);
unsigned long clk_get_rate(struct clk *clk);
long clk_round_rate(struct clk *clk, unsigned long rate);
int clk_set_rate(struct clk *clk, unsigned long rate);
int clk_enable(struct clk *clk);
void clk_disable(struct clk *clk);

#endif
