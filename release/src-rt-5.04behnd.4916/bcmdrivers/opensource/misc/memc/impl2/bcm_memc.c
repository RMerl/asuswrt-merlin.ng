/*
<:copyright-BRCM:2023:GPL/GPL:standard

     Copyright (c) 2023 Broadcom
     All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
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

int bcm_memc_get_of_memcfg(unsigned int *memcfg)
{
	struct device_node *np;
	const __be32 *val;

	if (!memcfg)
		return -EINVAL;
	np = of_find_compatible_node(NULL, NULL, "brcm,bcm-memc");
	if (!np) {
		np = of_find_node_opts_by_path("/memory_controller", NULL);
		if (!np)
			return -ENOENT;
		pr_warn("%s: FIX the absolute memory_controller path in the board dts!!!\n",
				__func__);
	}
	val = of_get_property(np, "memcfg", NULL);
	if (!val) {
		of_node_put(np);
		return -ESRCH;
	}

	*memcfg = be32_to_cpup(val);
	of_node_put(np);
	return 0;
}
EXPORT_SYMBOL(bcm_memc_get_of_memcfg);

int bcm_memc_get_ddr_size(unsigned int *ddr_size)
{
	int ret=0; 
	unsigned int memcfg;

	if (!ddr_size)
		return -1;

	// get mcb selector
	ret = bcm_memc_get_of_memcfg(&memcfg);
	if (ret) {
		pr_err("%s: Error - failed to get memcfg from device tree, "
		       "ret=%d\n", __func__, ret);
		return -1;
	}

	// get the ddr size
	if ((memcfg&BP_DDR_MCBSEL_FORMAT_MASK) == BP_DDR_MCBSEL_FORMAT_VER1)
	{
	// lpddr4/5
		switch (memcfg & BP1_DDR_TOTAL_SIZE_MASK)
		{
		case BP1_DDR_TOTAL_SIZE_2Gb:
				*ddr_size = 2048;
				break;
			case BP1_DDR_TOTAL_SIZE_4Gb:
				*ddr_size = 4096;
				break;
			case BP1_DDR_TOTAL_SIZE_8Gb:
				*ddr_size = 8192;
				break;
			case BP1_DDR_TOTAL_SIZE_12Gb:
				*ddr_size = 12288;
				break;
			case BP1_DDR_TOTAL_SIZE_16Gb:
				*ddr_size = 16384;
				break;
			case BP1_DDR_TOTAL_SIZE_24Gb:
				*ddr_size = 24576;
				break;
			case BP1_DDR_TOTAL_SIZE_32Gb:
				*ddr_size = 32768;
				break;
			case BP1_DDR_TOTAL_SIZE_48Gb:
				*ddr_size = 49152;
				break;
			case BP1_DDR_TOTAL_SIZE_64Gb:
				*ddr_size = 65536;
				break;
			case BP1_DDR_TOTAL_SIZE_96Gb:
				*ddr_size = 98304;
				break;
			default:
				*ddr_size = 0;
				break;
		}
	}
	else if ((memcfg&BP_DDR_MCBSEL_FORMAT_MASK) == BP_DDR_MCBSEL_FORMAT_VER0)
	{
		memcfg &= BP_DDR_TOTAL_SIZE_MASK;
		memcfg >>= BP_DDR_TOTAL_SIZE_SHIFT;
		*ddr_size=(1<< (memcfg+5));
	}

	return ret;
}
EXPORT_SYMBOL(bcm_memc_get_ddr_size);

int bcm_memc_get_spd_mhz(unsigned int *spd_mhz)
{
	int ret = 0;
	unsigned int memcfg;

	if (!spd_mhz)
		return -1;

	// get mcb selector
	ret = bcm_memc_get_of_memcfg(&memcfg);
	if (ret) {
		pr_err("%s: Error - failed to get memcfg from device tree, "
		       "ret=%d\n", __func__, ret);
		return -1;
	}

	// get the ddr clock
	if ((memcfg&BP_DDR_MCBSEL_FORMAT_MASK) == BP_DDR_MCBSEL_FORMAT_VER1)
	{
		// lpddr4/5
		unsigned int ddr_sg = memcfg & BP1_DDR_SPEED_MASK;
		switch (ddr_sg)
		{
			case BP1_DDR_SPEED_3200_58_58_58:
				*spd_mhz = 3200;
				break;
			case BP1_DDR_SPEED_3000_54_54_54:
				*spd_mhz = 3000;
				break;
			case BP1_DDR_SPEED_2750_50_50_50:
				*spd_mhz = 2750;
				break;
			case BP1_DDR_SPEED_2400_44_44_44:
				*spd_mhz = 2400;
				break;
			case BP1_DDR_SPEED_2133_39_39_39:
			case BP1_DDR_SPEED_2133_36_39_39:
				*spd_mhz = 2133;
				break;
			case BP1_DDR_SPEED_1867_34_34_34:
			case BP1_DDR_SPEED_1866_32_34_34:
				*spd_mhz = 1867;
				break;
			case BP1_DDR_SPEED_1600_29_29_29:
			case BP1_DDR_SPEED_1600_28_29_29:
				*spd_mhz = 1600;
				break;
			case BP1_DDR_SPEED_1333_24_24_24:
				*spd_mhz = 1333;
				break;
			default:
				return -1;
				break;
		}
	}
	else if ((memcfg&BP_DDR_MCBSEL_FORMAT_MASK) == BP_DDR_MCBSEL_FORMAT_VER0)
	{
		// ddr3/4
		unsigned int ddr_sg = memcfg & BP_DDR_SPEED_MASK;
		switch (ddr_sg)
		{
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
			default:
				return -1;
				break;
		}
	}
	else
		return -1;

	return ret;
}
EXPORT_SYMBOL(bcm_memc_get_spd_mhz);
