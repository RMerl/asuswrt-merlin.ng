// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 Xilinx, Inc.
 *
 * Michal Simek <michal.simek@xilinx.com>
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/psu_init_gpl.h>

#define PSU_MASK_POLL_TIME 1100000

int __maybe_unused mask_pollonvalue(unsigned long add, u32 mask, u32 value)
{
	int i = 0;

	while ((__raw_readl(add) & mask) != value) {
		if (i == PSU_MASK_POLL_TIME)
			return 0;
		i++;
	}
	return 1;
}

__weak int mask_poll(u32 add, u32 mask)
{
	int i = 0;
	unsigned long addr = add;

	while (!(__raw_readl(addr) & mask)) {
		if (i == PSU_MASK_POLL_TIME)
			return 0;
		i++;
	}
	return 1;
}

__weak u32 mask_read(u32 add, u32 mask)
{
	unsigned long addr = add;

	return __raw_readl(addr) & mask;
}

__weak void mask_delay(u32 delay)
{
	udelay(delay);
}

__weak void psu_mask_write(unsigned long offset, unsigned long mask,
			   unsigned long val)
{
	unsigned long regval = 0;

	regval = readl(offset);
	regval &= ~(mask);
	regval |= (val & mask);
	writel(regval, offset);
}

__weak void prog_reg(unsigned long addr, unsigned long mask,
		     unsigned long shift, unsigned long value)
{
	int rdata = 0;

	rdata = readl(addr);
	rdata = rdata & (~mask);
	rdata = rdata | (value << shift);
	writel(rdata, addr);
}

__weak int psu_init(void)
{
	/*
	 * This function is overridden by the one in
	 * board/xilinx/zynqmp/(platform)/psu_init_gpl.c, if it exists.
	 */
	return -1;
}
