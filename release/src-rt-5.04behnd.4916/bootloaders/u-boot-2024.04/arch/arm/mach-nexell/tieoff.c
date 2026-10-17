// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Nexell
 * Youngbok, Park <park@nexell.co.kr>
 */

#include <common.h>
#include <asm/arch/nexell.h>
#include <asm/arch/clk.h>
#include <asm/arch/reset.h>
#include <asm/arch/nx_gpio.h>
#include <asm/arch/tieoff.h>
#include <asm/arch/sec_reg.h>

#define	NX_PIN_FN_SIZE	4
#define TIEOFF_REG_NUM 33

struct	nx_tieoff_registerset {
	u32	tieoffreg[TIEOFF_REG_NUM];
};

static struct nx_tieoff_registerset *nx_tieoff = (void *)PHY_BASEADDR_TIEOFF;

static int tieoff_readl(void __iomem *reg)
{
	if (IS_ENABLED(CONFIG_ARCH_S5P4418))
		return read_sec_reg_by_id(reg, NEXELL_TOFF_SEC_ID);
	else
		return readl(reg);
}

static int  tieoff_writetl(void __iomem *reg, int val)
{
	if (IS_ENABLED(CONFIG_ARCH_S5P4418))
		return write_sec_reg_by_id(reg, val, NEXELL_TOFF_SEC_ID);
	else
		return writel(val, reg);
}

void nx_tieoff_set(u32 tieoff_index, u32 tieoff_value)
{
	u32 regindex, mask;
	u32 lsb, msb;
	u32 regval;

	u32 position;
	u32 bitwidth;

	position = tieoff_index & 0xffff;
	bitwidth = (tieoff_index >> 16) & 0xffff;

	regindex	= position >> 5;

	lsb = position & 0x1F;
	msb = lsb + bitwidth;

	if (msb > 32) {
		msb &= 0x1F;
		mask   = ~(0xffffffff << lsb);
		regval = tieoff_readl(&nx_tieoff->tieoffreg[regindex]) & mask;
		regval |= ((tieoff_value & ((1UL << bitwidth) - 1)) << lsb);
		tieoff_writetl(&nx_tieoff->tieoffreg[regindex], regval);

		mask   = (0xffffffff << msb);
		regval = tieoff_readl(&nx_tieoff->tieoffreg[regindex]) & mask;
		regval |= ((tieoff_value & ((1UL << bitwidth) - 1)) >> msb);
		tieoff_writetl(&nx_tieoff->tieoffreg[regindex + 1], regval);
	} else	{
		mask	= (0xffffffff << msb) | (~(0xffffffff << lsb));
		regval = tieoff_readl(&nx_tieoff->tieoffreg[regindex]) & mask;
		regval	|= ((tieoff_value & ((1UL << bitwidth) - 1)) << lsb);
		tieoff_writetl(&nx_tieoff->tieoffreg[regindex], regval);
	}
}

u32 nx_tieoff_get(u32 tieoff_index)
{
	u32 regindex, mask;
	u32 lsb, msb;
	u32 regval;

	u32 position;
	u32 bitwidth;

	position = tieoff_index & 0xffff;
	bitwidth = (tieoff_index >> 16) & 0xffff;

	regindex = position / 32;
	lsb = position % 32;
	msb = lsb + bitwidth;

	if (msb > 32) {
		msb &= 0x1F;
		mask   = 0xffffffff << lsb;
		regval = tieoff_readl(&nx_tieoff->tieoffreg[regindex]) & mask;
		regval >>= lsb;

		mask   = ~(0xffffffff << msb);
		regval |= ((tieoff_readl(&nx_tieoff->tieoffreg[regindex + 1])
					& mask) << (32 - lsb));
	} else	{
		mask   = ~(0xffffffff << msb) & (0xffffffff << lsb);
		regval = tieoff_readl(&nx_tieoff->tieoffreg[regindex]) & mask;
		regval >>= lsb;
	}
	return regval;
}
