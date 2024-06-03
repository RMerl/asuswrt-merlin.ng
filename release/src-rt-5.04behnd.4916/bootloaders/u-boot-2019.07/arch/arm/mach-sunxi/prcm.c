// SPDX-License-Identifier: GPL-2.0+
/*
 * Sunxi A31 Power Management Unit
 *
 * (C) Copyright 2013 Oliver Schinagl <oliver@schinagl.nl>
 * http://linux-sunxi.org
 *
 * Based on sun6i sources and earlier U-Boot Allwinner A10 SPL work
 *
 * (C) Copyright 2006-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Berg Xing <bergxing@allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/prcm.h>
#include <asm/arch/sys_proto.h>

/* APB0 clock gate and reset bit offsets are the same. */
void prcm_apb0_enable(u32 flags)
{
	struct sunxi_prcm_reg *prcm =
		(struct sunxi_prcm_reg *)SUNXI_PRCM_BASE;

	/* open the clock for module */
	setbits_le32(&prcm->apb0_gate, flags);

	/* deassert reset for module */
	setbits_le32(&prcm->apb0_reset, flags);
}

void prcm_apb0_disable(u32 flags)
{
	struct sunxi_prcm_reg *prcm =
		(struct sunxi_prcm_reg *)SUNXI_PRCM_BASE;

	/* assert reset for module */
	clrbits_le32(&prcm->apb0_reset, flags);

	/* close the clock for module */
	clrbits_le32(&prcm->apb0_gate, flags);
}
