/* SPDX-License-Identifier: GPL-2.0+
*  *
*  *  Copyright 2019 Broadcom Ltd.
*  */

#include <common.h>

void phy_adjust_afe(unsigned int phy_id_base, int is_quad)
{
	unsigned int phy_id;
	unsigned int phy_id_end = is_quad ? (phy_id_base + 4) : (phy_id_base + 1);

	for( phy_id = phy_id_base; phy_id < phy_id_end; phy_id++ )
	{
		//reset phy
		bcm_ethsw_phy_write_reg(phy_id, 0x0, 0x9140);
		udelay(100);
		//Write analog control registers
		//AFE_RXCONFIG_0
		bcm_ethsw_phy_write_misc_reg(phy_id, 0x38, 0x0, 0xeb15);
		//AFE_RXCONFIG_1. Replacing the previously suggested 0x9AAF for SS part. See JIRA HW63148-31
		bcm_ethsw_phy_write_misc_reg(phy_id, 0x38, 0x1, 0x9b2f);
		//AFE_RXCONFIG_2
		bcm_ethsw_phy_write_misc_reg(phy_id, 0x38, 0x2, 0x2003);
		//AFE_RX_LP_COUNTER
		bcm_ethsw_phy_write_misc_reg(phy_id, 0x38, 0x3, 0x7fc0);
		//AFE_TX_CONFIG
		bcm_ethsw_phy_write_misc_reg(phy_id, 0x39, 0x0, 0x0060);
		//AFE_VDAC_ICTRL_0
		bcm_ethsw_phy_write_misc_reg(phy_id, 0x39, 0x1, 0xa7da);
		//AFE_VDAC_OTHERS_0
		bcm_ethsw_phy_write_misc_reg(phy_id, 0x39, 0x3, 0xa020);
		//AFE_HPF_TRIM_OTHERS
		bcm_ethsw_phy_write_misc_reg(phy_id, 0x3a, 0x0, 0x00e3);
	}

	//CORE_BASE1E Force trim overwrite and set I_ext trim to 0000
	bcm_ethsw_phy_write_reg(phy_id_base, 0x1e, 0x0010);
	for( phy_id = phy_id_base; phy_id < phy_id_end; phy_id++ )
	{
	   //Adjust bias current trim by +4% swing, +2 tick, increase PLL BW in GPHY link start up training 'DSP_TAP10
	   bcm_ethsw_phy_write_misc_reg(phy_id, 0xa, 0x0, 0x111b);
	}

	//Reset R_CAL/RC_CAL Engine 'CORE_EXPB0
	bcm_ethsw_phy_write_exp_reg(phy_id_base, 0xb0, 0x10);
	//Disable Reset R_CAL/RC_CAL Engine 'CORE_EXPB0
	bcm_ethsw_phy_write_exp_reg(phy_id_base, 0xb0, 0x0);

	return;
}
