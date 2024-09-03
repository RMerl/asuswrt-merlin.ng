/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

#ifndef _BCMSFP_H_

enum bcmsfp_mon_attr
{
    bcmsfp_mon_has_txsd,
    bcmsfp_mon_rxsd_pin,	
    bcmsfp_mon_present,
    bcmsfp_mon_los,
    bcmsfp_mon_trx_compliance,
    bcmsfp_mon_tx_enable,
    bcmsfp_mon_rx_enable,
    bcmsfp_mon_tx_power_down,
    bcmsfp_mon_force_tx_power,
    bcmsfp_mon_smtc_tx_dis,

    bcmsfp_mon_id_phys_id,
    bcmsfp_mon_id_phys_ext_id,
    bcmsfp_mon_id_connector,
    bcmsfp_mon_id_vendor_name,
    bcmsfp_mon_id_vendor_pn,
    bcmsfp_mon_id_vendor_rev,
    bcmsfp_mon_id_vendor_sn,
    bcmsfp_mon_id_date_code,
    bcmsfp_mon_id_optical_wavelength,

    bcmsfp_mon_rx_power,
    bcmsfp_mon_tx_power,
    bcmsfp_mon_temp,
    bcmsfp_mon_vcc,
    bcmsfp_mon_bias_current,
    bcmsfp_mon_xfp_rx_channel,
    bcmsfp_mon_xfp_tx_channel,
    bcmsfp_mon_xfp_password,

    /* copper sfp ethernet phy reg access */
    bcmsfp_mon_phy_reg
};

enum bcmsfp_trx_compliance
{
    BCM_TRX_COMPLIANCE_UNKNOWN  = 0,
    BCM_SFF8472_CC_10GBASE_R    = (1 << 0),
    BCM_SFF8472_CC_1GBASE_X     = (1 << 1),
    BCM_SFF8024_ECC_10GBASE_T   = (1 << 2),
    BCM_SFF8024_ECC_5GBASE_T    = (1 << 3),
    BCM_SFF8024_ECC_2_5GBASE_T  = (1 << 4)
};

#define SFF8472_CC_10GBASE_R_BITS 0xf0
#define SFF8472_CC_1GBASE_X_BITS  0x0f

enum bcmsfp_connector
{
    SFF8024_CONNECTOR_COPPER_PIGTAIL    = 0x21,
    SFF8024_CONNECTOR_RJ45              = 0x22,
};

#define SFP_DIAGMON_ADDRMODE 0x4

struct sfp_data;
#endif
