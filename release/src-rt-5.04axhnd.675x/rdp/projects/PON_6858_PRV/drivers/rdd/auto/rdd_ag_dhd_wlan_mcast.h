/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
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



/* This is an automated file. Do not edit its contents. */


#ifndef _RDD_AG_DHD_WLAN_MCAST_H_
#define _RDD_AG_DHD_WLAN_MCAST_H_

#include "rdd.h"

int rdd_ag_dhd_wlan_mcast_wlan_mcast_dhd_station_table_set(uint32_t _entry, uint16_t mac_address_high, uint16_t mac_address_mid, uint16_t mac_address_low, uint8_t radio_index, bdmf_boolean valid, uint8_t tx_priority, uint16_t flowring_index);
int rdd_ag_dhd_wlan_mcast_wlan_mcast_dhd_station_table_get(uint32_t _entry, uint16_t *mac_address_high, uint16_t *mac_address_mid, uint16_t *mac_address_low, uint8_t *radio_index, bdmf_boolean *valid, uint8_t *tx_priority, uint16_t *flowring_index);
int rdd_ag_dhd_wlan_mcast_wlan_mcast_ssid_stats_table_set(uint32_t _entry, uint32_t packets, uint32_t bytes);
int rdd_ag_dhd_wlan_mcast_wlan_mcast_ssid_stats_table_get(uint32_t _entry, uint32_t *packets, uint32_t *bytes);
int rdd_ag_dhd_wlan_mcast_wlan_mcast_dhd_station_ctx_table_set(uint32_t _entry, uint8_t ssid);
int rdd_ag_dhd_wlan_mcast_wlan_mcast_dhd_station_ctx_table_get(uint32_t _entry, uint8_t *ssid);
int rdd_ag_dhd_wlan_mcast_wlan_mcast_dft_list_size_set(uint32_t _entry, uint8_t bits);
int rdd_ag_dhd_wlan_mcast_wlan_mcast_dft_list_size_get(uint32_t _entry, uint8_t *bits);
int rdd_ag_dhd_wlan_mcast_wlan_mcast_dft_addr_set(uint32_t low, uint32_t high);
int rdd_ag_dhd_wlan_mcast_wlan_mcast_dft_addr_get(uint32_t *low, uint32_t *high);

#endif /* _RDD_AG_DHD_WLAN_MCAST_H_ */
