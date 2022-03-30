// SPDX-License-Identifier: GPL-2.0+
/*
 * Adaptive Body Bias programming sequence for OMAP5 family
 *
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Andrii Tseglytskyi <andrii.tseglytskyi@ti.com>
 */

#include <common.h>
#include <asm/omap_common.h>
#include <asm/io.h>

/*
 * Setup LDOVBB for OMAP5.
 * On OMAP5+ some ABB settings are fused. They are handled
 * in the following way:
 *
 * 1. corresponding EFUSE register contains ABB enable bit
 *    and VSET value
 * 2. If ABB enable bit is set to 1, than ABB should be
 *    enabled, otherwise ABB should be disabled
 * 3. If ABB is enabled, than VSET value should be copied
 *    to corresponding MUX control register
 */
s8 abb_setup_ldovbb(u32 fuse, u32 ldovbb)
{
	u32 vset;
	u32 fuse_enable_mask = OMAP5_PROD_ABB_FUSE_ENABLE_MASK;
	u32 fuse_vset_mask = OMAP5_PROD_ABB_FUSE_VSET_MASK;

	if (!is_omap54xx()) {
		/* DRA7 */
		fuse_enable_mask = DRA7_ABB_FUSE_ENABLE_MASK;
		fuse_vset_mask = DRA7_ABB_FUSE_VSET_MASK;
	}
	/*
	 * ABB parameters must be properly fused
	 * otherwise ABB should be disabled
	 */
	vset = readl(fuse);
	if (!(vset & fuse_enable_mask))
		return -1;

	/* prepare VSET value for LDOVBB mux register */
	vset &= fuse_vset_mask;
	vset >>= ffs(fuse_vset_mask) - 1;
	vset <<= ffs(OMAP5_ABB_LDOVBBMPU_VSET_OUT_MASK) - 1;
	vset |= OMAP5_ABB_LDOVBBMPU_MUX_CTRL_MASK;

	/* setup LDOVBB using fused value */
	clrsetbits_le32(ldovbb,  OMAP5_ABB_LDOVBBMPU_VSET_OUT_MASK, vset);

	return 0;
}
