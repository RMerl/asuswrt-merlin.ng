/* SPDX-License-Identifier: GPL-2.0+
*  *
*   *  Copyright 2019 Broadcom Ltd.
*    */

#include <common.h>

void phy_adjust_afe(unsigned int phy_id_base, int is_quad)
{
	unsigned int phy_id;
	unsigned int phy_id_end = is_quad ? (phy_id_base + 4) : (phy_id_base + 1);

	for( phy_id = phy_id_base; phy_id < phy_id_end; phy_id++ )
	{
		//reset phy
		bcm_ethsw_phy_write_reg(phy_id, 0x0, 0x9140);
	}
	udelay(100);

	for( phy_id = phy_id_base; phy_id < phy_id_end; phy_id++ )
	{
		//Turn off AOF
		bcm_ethsw_phy_write_misc_reg(  phy_id,  0x39, 0x1, 0x0000 );//AFE_TX_CONFIG_0

		//1g AB symmetry Iq
		bcm_ethsw_phy_write_misc_reg(  phy_id,  0x3a, 0x2, 0x0BCC );//AFE_TX_CONFIG_1

		//LPF BW
		bcm_ethsw_phy_write_misc_reg(  phy_id,  0x39, 0x0, 0x233F );//AFE_TX_IQ_RX_LP

		//RCAL +6LSB to make impedance from 112 to 100ohm
		bcm_ethsw_phy_write_misc_reg(  phy_id,  0x3b, 0x0, 0xAD40 );//AFE_TEMPSEN_OTHERS

		//since rcal make R smaller, make master current -4%
		bcm_ethsw_phy_write_misc_reg(  phy_id,  0x0a, 0x0, 0x091B );//DSP_TAP10

		//From EEE excel config file for Vitesse fix
		bcm_ethsw_phy_write_misc_reg(  phy_id,  0x0021, 0x0002, 0x87F6 );// rx_on_tune 8 -> 0xf
		bcm_ethsw_phy_write_misc_reg(  phy_id,  0x0022, 0x0002, 0x017D );// 100tx EEE bandwidth
		bcm_ethsw_phy_write_misc_reg(  phy_id,  0x0026, 0x0002, 0x0015 );// enable ffe zero det for Vitesse interop

	}

	//Reset R_CAL/RC_CAL Engine 'CORE_EXPB0
	bcm_ethsw_phy_write_exp_reg(phy_id_base, 0xb0, 0x10);
	//Disable Reset R_CAL/RC_CAL Engine 'CORE_EXPB0
	bcm_ethsw_phy_write_exp_reg(phy_id_base, 0xb0, 0x0);

	return;

}

