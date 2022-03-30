// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>,
 */

#include <common.h>
#include <asm/io.h>
#include <mvmfp.h>
#include <asm/arch/mfp.h>

/*
 * mfp_config
 *
 * On most of Marvell SoCs (ex. ARMADA100) there is Multi-Funtion-Pin
 * configuration registers to configure each GPIO/Function pin on the
 * SoC.
 *
 * This function reads the array of values for
 * MFPR_X registers and programms them into respective
 * Multi-Function Pin registers.
 * It supports - Alternate Function Selection programming.
 *
 * Whereas,
 * The Configureation value is constructed using MFP()
 * array consists of 32bit values as defined in MFP(xx,xx..) macro
 */
void mfp_config(u32 *mfp_cfgs)
{
	u32 *p_mfpr = NULL;
	u32 cfg_val, val;

	do {
		cfg_val = *mfp_cfgs++;
		/* exit if End of configuration table detected */
		if (cfg_val == MFP_EOC)
			break;

		p_mfpr = (u32 *)(MV_MFPR_BASE
				+ MFP_REG_GET_OFFSET(cfg_val));

		/* Write a mfg register as per configuration */
		val = 0;
		if (cfg_val & MFP_VALUE_MASK)
			val |= cfg_val & MFP_VALUE_MASK;

		writel(val, p_mfpr);
	} while (1);
	/*
	 * perform a read-back of any MFPR register to make sure the
	 * previous writings are finished
	 */
	readl(p_mfpr);
}
