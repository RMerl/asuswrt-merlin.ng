/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * TI clock utilities header
 *
 * Copyright (C) 2020 Dario Binacchi <dariobin@libero.it>
 */

#ifndef _CLK_TI_H
#define _CLK_TI_H

/**
 * struct clk_ti_reg - TI register declaration
 * @offset: offset from the master IP module base address
 * @index: index of the master IP module
 */
struct clk_ti_reg {
	u16 offset;
	u8 index;
};

void clk_ti_latch(struct clk_ti_reg *reg, s8 shift);
void clk_ti_writel(u32 val, struct clk_ti_reg *reg);
u32 clk_ti_readl(struct clk_ti_reg *reg);
int clk_ti_get_reg_addr(struct udevice *dev, int index, struct clk_ti_reg *reg);

#endif /* #ifndef _CLK_TI_H */
