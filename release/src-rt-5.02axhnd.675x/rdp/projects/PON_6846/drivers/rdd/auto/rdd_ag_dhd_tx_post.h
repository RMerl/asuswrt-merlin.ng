/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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



/* This is an automated file. Do not edit its contents. */


#ifndef _RDD_AG_DHD_TX_POST_H_
#define _RDD_AG_DHD_TX_POST_H_

#include "rdd.h"

int rdd_ag_dhd_tx_post_wlan_mcast_dhd_station_table_set(uint32_t _entry, uint16_t mac_address_high, uint16_t mac_address_mid, uint16_t mac_address_low, uint8_t radio_index, bdmf_boolean valid, uint8_t tx_priority, uint16_t flowring_index);
int rdd_ag_dhd_tx_post_wlan_mcast_dhd_station_table_get(uint32_t _entry, uint16_t *mac_address_high, uint16_t *mac_address_mid, uint16_t *mac_address_low, uint8_t *radio_index, bdmf_boolean *valid, uint8_t *tx_priority, uint16_t *flowring_index);
int rdd_ag_dhd_tx_post_wlan_mcast_dhd_station_ctx_table_set(uint32_t _entry, uint8_t ssid);
int rdd_ag_dhd_tx_post_wlan_mcast_dhd_station_ctx_table_get(uint32_t _entry, uint8_t *ssid);
int rdd_ag_dhd_tx_post_wlan_mcast_dft_list_size_set(uint32_t _entry, uint8_t bits);
int rdd_ag_dhd_tx_post_wlan_mcast_dft_list_size_get(uint32_t _entry, uint8_t *bits);
int rdd_ag_dhd_tx_post_wlan_mcast_dft_addr_set(uint32_t low, uint32_t high);
int rdd_ag_dhd_tx_post_wlan_mcast_dft_addr_get(uint32_t *low, uint32_t *high);
int rdd_ag_dhd_tx_post_wlan_mcast_ssid_stats_table_set(uint32_t _entry, uint32_t packets, uint32_t bytes);
int rdd_ag_dhd_tx_post_wlan_mcast_ssid_stats_table_get(uint32_t _entry, uint32_t *packets, uint32_t *bytes);

#endif /* _RDD_AG_DHD_TX_POST_H_ */
