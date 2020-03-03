/*
 * Copyright 2008-2010 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 *
 * This file contains the CPU initialization code.
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>

#include "hardware.h"
#include "common.h"

static int mx5_cpu_rev = -1;

#define IIM_SREV 0x24

static u32 imx5_read_srev_reg(const char *compat)
{
	void __iomem *iim_base;
	struct device_node *np;
	u32 srev;

	np = of_find_compatible_node(NULL, NULL, compat);
	iim_base = of_iomap(np, 0);
	WARN_ON(!iim_base);

	srev = readl(iim_base + IIM_SREV) & 0xff;

	iounmap(iim_base);

	return srev;
}

static int get_mx51_srev(void)
{
	u32 rev = imx5_read_srev_reg("fsl,imx51-iim");

	switch (rev) {
	case 0x0:
		return IMX_CHIP_REVISION_2_0;
	case 0x10:
		return IMX_CHIP_REVISION_3_0;
	default:
		return IMX_CHIP_REVISION_UNKNOWN;
	}
}

/*
 * Returns:
 *	the silicon revision of the cpu
 *	-EINVAL - not a mx51
 */
int mx51_revision(void)
{
	if (!cpu_is_mx51())
		return -EINVAL;

	if (mx5_cpu_rev == -1)
		mx5_cpu_rev = get_mx51_srev();

	return mx5_cpu_rev;
}
EXPORT_SYMBOL(mx51_revision);

#ifdef CONFIG_NEON

/*
 * All versions of the silicon before Rev. 3 have broken NEON implementations.
 * Dependent on link order - so the assumption is that vfp_init is called
 * before us.
 */
int __init mx51_neon_fixup(void)
{
	if (mx51_revision() < IMX_CHIP_REVISION_3_0 &&
			(elf_hwcap & HWCAP_NEON)) {
		elf_hwcap &= ~HWCAP_NEON;
		pr_info("Turning off NEON support, detected broken NEON implementation\n");
	}
	return 0;
}

#endif

static int get_mx53_srev(void)
{
	u32 rev = imx5_read_srev_reg("fsl,imx53-iim");

	switch (rev) {
	case 0x0:
		return IMX_CHIP_REVISION_1_0;
	case 0x2:
		return IMX_CHIP_REVISION_2_0;
	case 0x3:
		return IMX_CHIP_REVISION_2_1;
	default:
		return IMX_CHIP_REVISION_UNKNOWN;
	}
}

/*
 * Returns:
 *	the silicon revision of the cpu
 *	-EINVAL - not a mx53
 */
int mx53_revision(void)
{
	if (!cpu_is_mx53())
		return -EINVAL;

	if (mx5_cpu_rev == -1)
		mx5_cpu_rev = get_mx53_srev();

	return mx5_cpu_rev;
}
EXPORT_SYMBOL(mx53_revision);
