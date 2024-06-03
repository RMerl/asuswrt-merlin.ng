/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

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

struct device;
int sfp_mon_read(struct device *dev, enum bcmsfp_mon_attr attr, int channel, long *value);
int sfp_mon_read_buf(struct device *dev, enum bcmsfp_mon_attr attr, int channel, char **buf, int *len);
int sfp_mon_write(struct device *dev, enum bcmsfp_mon_attr attr, int channel, long value);
int _sfp_mon_write(struct device *dev, enum bcmsfp_mon_attr attr, int channel, long value);
int sfp_mon_write_buf(struct device *dev, enum bcmsfp_mon_attr attr, int channel, char *buf, int count);
#if defined(CONFIG_BCM_ETHTOOL)
struct ethtool_eeprom;
struct ethtool_modinfo;
int bcmsfp_module_info(struct device *dev, struct ethtool_modinfo *modinfo);
int bcmsfp_module_eeprom(struct device *dev, struct ethtool_eeprom *ee, u8 *data);
#endif
#endif
