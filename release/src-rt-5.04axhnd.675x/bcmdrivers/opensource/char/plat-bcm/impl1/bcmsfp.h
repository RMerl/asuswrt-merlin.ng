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
    bcmsfp_mon_present,
    bcmsfp_mon_los,
    bcmsfp_mon_tx_enable,
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
    bcmsfp_mon_id_optical_wavelength,

    bcmsfp_mon_rx_power,
    bcmsfp_mon_tx_power,
    bcmsfp_mon_temp,
    bcmsfp_mon_vcc,
    bcmsfp_mon_bias_current,
    bcmsfp_mon_xfp_rx_channel,
    bcmsfp_mon_xfp_tx_channel,
    bcmsfp_mon_xfp_password,
};

struct device;
extern int sfp_mon_read(struct device *dev, enum bcmsfp_mon_attr attr, int channel, long *value);
extern int sfp_mon_read_buf(struct device *dev, enum bcmsfp_mon_attr attr, int channel, char **buf, int *len);
extern int sfp_mon_write(struct device *dev, enum bcmsfp_mon_attr attr, int channel, long value);
extern int sfp_mon_write_buf(struct device *dev, enum bcmsfp_mon_attr attr, int channel, char *buf, int count);

#endif

