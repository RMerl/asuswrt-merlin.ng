// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

*/

/*
 *  Created on: Jun 2017
 *      Author: steven.hsieh@broadcom.com
 */

#ifndef __MAC_DRV_SF2_H__
#define __MAC_DRV_SF2_H__

/* definition for mac_drv priv flags */
#define SF2MAC_DRV_PRIV_FLAG_IMP                (1<<1)
#define SF2MAC_DRV_PRIV_FLAG_SHRINK_IPG         (1<<2)  // for IMP port with clock speed can't support full line rate due to brcm tag
#define SF2MAC_DRV_PRIV_FLAG_SW_EXT             (1<<3)  // for SF2_DUAL indicating MAC/port is on external switch1
typedef struct
{
    unsigned long priv_flags;
    void (*rreg)(int page, int reg, void *data_out, int len);
    void (*wreg)(int page, int reg, void *data_in,  int len);
    mac_stats_t mac_stats;
    mac_stats_t last_mac_stats;
} sf2_mac_dev_priv_data_t;

#endif

