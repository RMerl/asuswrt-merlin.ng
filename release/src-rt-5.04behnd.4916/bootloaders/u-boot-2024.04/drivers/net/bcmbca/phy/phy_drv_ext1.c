// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved
*/

/*
 *	Created on: Dec 2015
 *		Author: yuval.raviv@broadcom.com
 */

/*
 * Phy driver for external 1G phys: BCM5461S, BCM54810
 */

#include "bus_drv.h"
#include "phy_drv.h"
#include "phy_drv_mii.h"
#include "phy_drv_brcm.h"
#include "xrdp_led_init.h"

static int _phy_init(phy_dev_t *phy_dev)
{
	int ret;

	/* Access IEEE register set instead of LRE */
	ret = phy_bus_write(phy_dev, 0x0e, 0x06);
	if (ret)
		goto Exit;

	ret = mii_init(phy_dev);
	if (ret)
		goto Exit;

	/* Disable BroadR-Reach function */
	ret = brcm_exp_write(phy_dev, 0x90, 0x00);
	if (ret)
		goto Exit;

	if (phy_dev->mii_type == PHY_MII_TYPE_RGMII) {
		ret = brcm_shadow_rgmii_init(phy_dev);
		if (ret)
			goto Exit;
	}

	ret = brcm_shadow_18_force_auto_mdix_set(phy_dev, 1);
	if (ret)
		goto Exit;

	ret = brcm_shadow_18_eth_wirespeed_set(phy_dev, 1);
	if (ret)
		goto Exit;

Exit:
	return ret;
}

static int _phy_leds_init(phy_dev_t *phy_dev, void *leds_info)
{
	return xrdp_leds_init(leds_info);
}

phy_drv_t phy_drv_ext1 = {
	.phy_type = PHY_TYPE_EXT1,
	.name = "EXT1",
	.power_get = mii_power_get,
	.power_set = mii_power_set,
	.apd_get = brcm_shadow_1c_apd_get,
	.apd_set = brcm_shadow_1c_apd_set,
	.read_status = brcm_read_status_rgmii_ib_override,
	.speed_set = mii_speed_set,
	.caps_get = mii_caps_get,
	.caps_set = mii_caps_set,
	.phyid_get = mii_phyid_get,
	.mdix_mode_set = brcm_shadow_18_mdix_mode_set,
	.mdix_mode_get = brcm_shadow_18_mdix_mode_get,
	.mdix_status_get = brcm_shadow_18_mdix_status_get,
	.wirespeed_set = brcm_shadow_18_eth_wirespeed_set,
	.wirespeed_get = brcm_shadow_18_eth_wirespeed_get,
	.leds_init = _phy_leds_init,
	.init = _phy_init,
};
