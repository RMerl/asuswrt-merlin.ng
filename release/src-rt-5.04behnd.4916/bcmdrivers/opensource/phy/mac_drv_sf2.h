/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard
    
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

/*
 *  Created on: Jun 2017
 *      Author: steven.hsieh@broadcom.com
 */

#ifndef __MAC_DRV_SF2_H__
#define __MAC_DRV_SF2_H__

/* definition for mac_drv priv flags */
#define SF2MAC_DRV_PRIV_FLAG_MGMT               (1<<1)
#define SF2MAC_DRV_PRIV_FLAG_SHRINK_IPG         (1<<2)  // for IMP port with clock speed can't support full line rate due to brcm tag
#define SF2MAC_DRV_PRIV_FLAG_SW_EXT             (1<<3)  // for SF2_DUAL indicating MAC/port is on external switch1
#define SF2MAC_DRV_PRIV_FLAG_EXTSW_CONNECTED    (1<<4)  // for SF2_DUAL indicating MAC/port is connected to external switch
#define SF2MAC_DRV_PRIV_FLAG_RMT_LPBK_EN        (1<<5)  // remote loopback is enabled for this MAC
#define SF2MAC_DRV_PRIV_FLAG_64_40BIT_MIB       (1<<6)  // 64/40bit mib counters instead of 64/32bit 

typedef struct
{
    unsigned long priv_flags;
    void (*rreg)(int page, int reg, void *data_out, int len);
    void (*wreg)(int page, int reg, void *data_in,  int len);
    uint16_t saved_pmap;    // save port based vlan map to be restored when remote loopback is disabled
    mac_stats_t mac_stats;  // accumulated software counters
    mac_stats_t last_mac_stats; // last hw counters
    mac_stats_t scratch_stats; // current hw counters
} sf2_mac_dev_priv_data_t;

#endif

