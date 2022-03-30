// SPDX-License-Identifier: GPL-2.0+
/*
 * Adaptive Body Bias programming sequence for OMAP family
 *
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Andrii Tseglytskyi <andrii.tseglytskyi@ti.com>
 */

#include <common.h>
#include <asm/omap_common.h>
#include <asm/arch/clock.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>

__weak s8 abb_setup_ldovbb(u32 fuse, u32 ldovbb)
{
	return -1;
}

static void abb_setup_timings(u32 setup)
{
	u32 sys_rate, sr2_cnt, clk_cycles;

	/*
	 * SR2_WTCNT_VALUE is the settling time for the ABB ldo after a
	 * transition and must be programmed with the correct time at boot.
	 * The value programmed into the register is the number of SYS_CLK
	 * clock cycles that match a given wall time profiled for the ldo.
	 * This value depends on:
	 * settling time of ldo in micro-seconds (varies per OMAP family),
	 * of clock cycles per SYS_CLK period (varies per OMAP family),
	 * the SYS_CLK frequency in MHz (varies per board)
	 * The formula is:
	 *
	 *		       ldo settling time (in micro-seconds)
	 * SR2_WTCNT_VALUE = ------------------------------------------
	 *		    (# system clock cycles) * (sys_clk period)
	 *
	 * Put another way:
	 *
	 * SR2_WTCNT_VALUE = settling time / (# SYS_CLK cycles / SYS_CLK rate))
	 *
	 * To avoid dividing by zero multiply both "# clock cycles" and
	 * "settling time" by 10 such that the final result is the one we want.
	 */

	/* calculate SR2_WTCNT_VALUE */
	sys_rate = DIV_ROUND_CLOSEST(V_OSCK, 1000000);
	clk_cycles = DIV_ROUND_CLOSEST(OMAP_ABB_CLOCK_CYCLES * 10, sys_rate);
	sr2_cnt = DIV_ROUND_CLOSEST(OMAP_ABB_SETTLING_TIME * 10, clk_cycles);

	setbits_le32(setup,
		     sr2_cnt << (ffs(OMAP_ABB_SETUP_SR2_WTCNT_VALUE_MASK) - 1));
}

void abb_setup(u32 fuse, u32 ldovbb, u32 setup, u32 control,
	       u32 txdone, u32 txdone_mask, u32 opp)
{
	u32 abb_type_mask, opp_sel_mask;

	/* sanity check */
	if (!setup || !control || !txdone)
		return;

	/* setup ABB only in case of Fast or Slow OPP */
	switch (opp) {
	case OMAP_ABB_FAST_OPP:
		abb_type_mask = OMAP_ABB_SETUP_ACTIVE_FBB_SEL_MASK;
		opp_sel_mask = OMAP_ABB_CONTROL_FAST_OPP_SEL_MASK;
		break;
	case OMAP_ABB_SLOW_OPP:
		abb_type_mask = OMAP_ABB_SETUP_ACTIVE_RBB_SEL_MASK;
		opp_sel_mask = OMAP_ABB_CONTROL_SLOW_OPP_SEL_MASK;
		break;
	default:
	       return;
	}

	/*
	 * For some OMAP silicons additional setup for LDOVBB register is
	 * required. This is determined by data retrieved from corresponding
	 * OPP EFUSE register. Data, which is retrieved from EFUSE - is
	 * ABB enable/disable flag and VSET value, which must be copied
	 * to LDOVBB register. If function call fails - return quietly,
	 * it means no ABB is required for such silicon.
	 *
	 * For silicons, which don't require LDOVBB setup "fuse" and
	 * "ldovbb" offsets are not defined. ABB will be initialized in
	 * the common way for them.
	 */
	if (fuse && ldovbb) {
		if (abb_setup_ldovbb(fuse, ldovbb))
			return;
	}

	/* clear ABB registers */
	writel(0, setup);
	writel(0, control);

	/* configure timings, based on oscillator value */
	abb_setup_timings(setup);

	/* clear pending interrupts before setup */
	setbits_le32(txdone, txdone_mask);

	/* select ABB type */
	setbits_le32(setup, abb_type_mask | OMAP_ABB_SETUP_SR2EN_MASK);

	/* initiate ABB ldo change */
	setbits_le32(control, opp_sel_mask | OMAP_ABB_CONTROL_OPP_CHANGE_MASK);

	/* wait until transition complete */
	if (!wait_on_value(txdone_mask, txdone_mask, (void *)txdone, LDELAY))
		puts("Error: ABB txdone is not set\n");

	/* clear ABB tranxdone */
	setbits_le32(txdone, txdone_mask);
}
