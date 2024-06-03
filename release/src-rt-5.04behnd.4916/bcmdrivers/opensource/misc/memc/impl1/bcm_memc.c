/*
<:copyright-BRCM:2021:GPL/GPL:standard

	 Copyright (c) 2021 Broadcom
	 All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/export.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include "bcmbca_memc_dt_bindings.h"
#include "bcm_memc.h"

extern int bcm_memc_irq_init(struct platform_device *pdev);

int bcm_memc_get_of_memcfg(unsigned int *memcfg)
{
	struct device_node *np;
	const __be32 *val;

	if (!memcfg)
		return -EINVAL;
	np = of_find_node_opts_by_path("/memory_controller", NULL);
	if (!np)
		return -ENOENT;
	val = of_get_property(np, "memcfg", NULL);
	if (!val) {
		of_node_put(np);
		return -ESRCH;
	}

	*memcfg = be32_to_cpup(val);
	of_node_put(np);
	pr_info("%s: of memcfg=0x%x\n", __func__, *memcfg);
	return 0;
}
EXPORT_SYMBOL(bcm_memc_get_of_memcfg);

static int bcm_memc_get_spd_mhz_v0(unsigned int memcfg, unsigned int *spd_mhz)
{
	switch (memcfg & BP_DDR_SPEED_MASK) {
	case BP_DDR_SPEED_800_10_10_10:
	case BP_DDR_SPEED_800_11_11_11:
		*spd_mhz = 800;
		break;
	case BP_DDR_SPEED_933_10_10_10:
	case BP_DDR_SPEED_933_11_11_11:
	case BP_DDR_SPEED_933_12_12_12:
	case BP_DDR_SPEED_933_13_13_13:
		*spd_mhz = 933;
		break;
	case BP_DDR_SPEED_1067_11_11_11:
	case BP_DDR_SPEED_1067_12_12_12:
	case BP_DDR_SPEED_1067_13_13_13:
	case BP_DDR_SPEED_1067_14_14_14:
	case BP_DDR_SPEED_1067_15_15_15:
	case BP_DDR_SPEED_1067_16_16_16:
		*spd_mhz = 1067;
		break;
	case BP_DDR_SPEED_1200_17_17_17:
		*spd_mhz = 1200;
		break;
	case BP_DDR_SPEED_1333_18_18_18:
	case BP_DDR_SPEED_1333_19_19_19:
		*spd_mhz = 1333;
		break;
	case BP_DDR_SPEED_1467_21_21_21:
		*spd_mhz = 1467;
		break;
	case BP_DDR_SPEED_1600_22_22_22:
		*spd_mhz = 1600;
		break;
	case BP_DDR_SPEED_CUSTOM_1: /* CUSTOM speed 1026MHz */
		*spd_mhz = 1026;
		break;
	case BP_DDR_SPEED_CUSTOM_2: /* have not been used in dts yet */
	case BP_DDR_SPEED_CUSTOM_3: /* have not been used in dts yet */
	case BP_DDR_SPEED_CUSTOM_4: /* have not been used in dts yet */
	default:
		return -1;
	}

	return 0;
}

static int bcm_memc_get_spd_mhz_v1(unsigned int memcfg, unsigned int *spd_mhz)
{
	switch (memcfg & BP1_DDR_SPEED_MASK) {
	case BP1_DDR_SPEED_1333_24_24_24:  /* 1   LPDDR4-1333 */
		*spd_mhz = 1333;
		break;
	case BP1_DDR_SPEED_1600_28_29_29:  /* 2   LPDDR4-1600 */
	case BP1_DDR_SPEED_1600_29_29_29:  /* 5   LPDDR5-1600 */
		*spd_mhz = 1600;
		break;
	case BP1_DDR_SPEED_1866_32_34_34:  /* 3   LPDDR4-1866 */
		*spd_mhz = 1866;
		break;
	case BP1_DDR_SPEED_2133_36_39_39:  /* 4   LPDDR4-2133 */
	case BP1_DDR_SPEED_2133_39_39_39:  /* 7   LPDDR5-2133 */
		*spd_mhz = 2133;
		break;
	case BP1_DDR_SPEED_1867_34_34_34:  /* 6   LPDDR5-1867 */
		*spd_mhz = 1867;
		break;
	case BP1_DDR_SPEED_2400_44_44_44:  /* 8   LPDDR5-2400 */
		*spd_mhz = 2400;
		break;
	case BP1_DDR_SPEED_2750_50_50_50:  /* 9   LPDDR5-2750 */
		*spd_mhz = 2750;
		break;
	case BP1_DDR_SPEED_3000_54_54_54:  /* 10  LPDDR5-3000 */
	case BP1_DDR_SPEED_3200_58_58_58:  /* 11  LPDDR5-3200 */
		*spd_mhz = 3200;
		break;
	default:
		return -1;
	}

	return 0;
}

int bcm_memc_get_spd_mhz(unsigned int *spd_mhz)
{
	unsigned int memcfg;
	int ret; 

	if (!spd_mhz) {
		pr_err("%s: Error - invalid input\n", __func__);
		return -EINVAL;
	}

	ret = bcm_memc_get_of_memcfg(&memcfg);
	if (ret) {
		pr_err("%s: Error - failed to get memcfg from device tree, "
			"ret=%d\n", __func__, ret);
		return ret;
	}

	switch (memcfg & BP_DDR_MCBSEL_FORMAT_MASK) {
	case BP_DDR_MCBSEL_FORMAT_VER0:
		ret = bcm_memc_get_spd_mhz_v0(memcfg, spd_mhz);
		break;
	case BP_DDR_MCBSEL_FORMAT_VER1:
		ret = bcm_memc_get_spd_mhz_v1(memcfg, spd_mhz);
		break;
	default:
		ret = -1;
		break;
	}

	if (ret) {
		pr_err("%s: Error - failed to get ddr speed, memcfg=0x%x\n",
			__func__, memcfg);
		return -EDOM;
	}

	pr_info("%s: memcfg=0x%x spd_mhz=%u\n", __func__, memcfg, *spd_mhz);
	return 0;
}
EXPORT_SYMBOL(bcm_memc_get_spd_mhz);

static struct of_device_id const bcm_memc_of_match[] = {
	{ .compatible = "brcm,bcm-memc" },
	{ /* end of list */ }
};

static int __init bcm_memc_probe(struct platform_device *pdev)
{
	const struct of_device_id *match;
	struct device *dev = &pdev->dev;

	match = of_match_device(bcm_memc_of_match, dev);
	if (!match) {
		dev_err(dev, "failed to match the memory controller\n");
		return -ENODEV;
	}
	dev_info(dev, "matched the memory controller to %s\n",
			match->compatible);

	bcm_memc_init_self_refresh(pdev);

#ifdef CONFIG_OPTEE
	if (bcm_memc_irq_init(pdev))
		dev_err(&pdev->dev, "Failed to configure MEMC interrupt\n");
#endif

	return 0;
}

static struct platform_driver bcm_memc_platform_driver = {
	.driver = {
		.name = "bcm_memc",
		.of_match_table = bcm_memc_of_match,
	},
};

builtin_platform_driver_probe(bcm_memc_platform_driver, bcm_memc_probe);

MODULE_DESCRIPTION("Broadcom memory controller driver");
MODULE_LICENSE("GPL v2");
