/*
 * <:copyright-BRCM:2013:DUAL/GPL:standard
 * 
 *    Copyright (c) 2013 Broadcom 
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :> 
 */


#ifndef _RDD_WLAN_MCAST_COMMON_H_
#define _RDD_WLAN_MCAST_COMMON_H_

#include "rdd_data_structures_auto.h"

#ifdef XRDP

#define __debug(fmt, arg...) RDD_BTRACE(fmt, ##arg)
#define __print(fmt, arg...) RDD_TRACE(fmt, ##arg)

#else /* XRDP */

#define WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_STRUCT RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_DTS
#define WLAN_MCAST_DHD_LIST_ENTRY_STRUCT RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS
#define WLAN_MCAST_DHD_LIST_ENTRY_ARRAY_STRUCT RDD_WLAN_MCAST_DHD_LIST_ENTRY_ARRAY_DTS
#define WLAN_MCAST_SSID_STATS_ENTRY_STRUCT RDD_WLAN_MCAST_SSID_STATS_ENTRY_DTS
#define WLAN_MCAST_DHD_LIST_TABLE_STRUCT RDD_WLAN_MCAST_DHD_LIST_TABLE_DTS
#define WLAN_MCAST_DHD_STATION_ENTRY_STRUCT RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS

#if 0
#if defined(BDMF_SYSTEM_SIM)
#define __debug(fmt, arg...) printf("%s,%u: " fmt, __FUNCTION__, __LINE__, ##arg)
#else
#define __debug(fmt, arg...) bdmf_trace("%s,%u: " fmt, __FUNCTION__, __LINE__, ##arg)
#endif
#else
#define __debug(fmt, arg...)
#endif

#if defined(BDMF_SYSTEM_SIM)
#define __print(fmt, arg...)
#else
#define __print(fmt, arg...) printk(fmt, ##arg)
#endif

#endif /* XRDP */

typedef struct {
    void *virt_p;
    bdmf_phys_addr_t phys_addr;
} wlan_mcast_dhd_list_table_t;

#define RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_INDEX(_radio_index, _ssid) \
    ( (((_radio_index) & 0x3) << 4) | ((_ssid) & 0xF) )

#define RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_RADIO_INDEX(_index) \
    ( ((_index) >> 4) & 0x3 )

#define RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_SSID(_index) \
    ( (_index) & 0xF )

#define RDD_WLAN_MCAST_SSID_STATS_ENTRY_RADIO_INDEX(_index) \
    ( ((_index) >> 4) & 0x3 )

#define RDD_WLAN_MCAST_SSID_STATS_ENTRY_SSID(_index) \
    ( (_index) & 0xF )

#ifdef CONFIG_DHD_RUNNER
int rdd_wlan_mcast_ssid_mac_address_add(uint32_t radio_index, uint32_t ssid,
    WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_STRUCT *ssid_mac_address_entry,
    uint32_t *ssid_mac_address_index);

int rdd_wlan_mcast_ssid_mac_address_delete(uint32_t ssid_mac_address_index);

int rdd_wlan_mcast_ssid_mac_address_read(uint32_t ssid_mac_address_index,
    WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_STRUCT *ssid_mac_address_entry);

int rdd_wlan_mcast_dhd_list_entry_add(uint32_t fwd_entry_index,
    WLAN_MCAST_DHD_LIST_ENTRY_STRUCT *dhd_list_entry, uint32_t *dhd_list_size);
int rdd_wlan_mcast_dhd_list_entry_delete(uint32_t fwd_entry_index,
    WLAN_MCAST_DHD_LIST_ENTRY_STRUCT *dhd_list_entry, uint32_t *dhd_list_size);
int rdd_wlan_mcast_dhd_list_entry_find(uint32_t fwd_entry_index,
    WLAN_MCAST_DHD_LIST_ENTRY_STRUCT *dhd_list_entry);

uint8_t rdd_wlan_mcast_dhd_list_scan(uint32_t fwd_entry_index,
    WLAN_MCAST_DHD_LIST_ENTRY_ARRAY_STRUCT *dhd_list_entry_array);
void rdd_wlan_mcast_dhd_list_delete(uint32_t fwd_entry_index);
#endif

int rdd_wlan_mcast_init(wlan_mcast_dhd_list_table_t *table);
int rdd_wlan_mcast_init_common(wlan_mcast_dhd_list_table_t *table);

int rdd_wlan_mcast_ssid_stats_read(uint32_t ssid_stats_index,
    WLAN_MCAST_SSID_STATS_ENTRY_STRUCT *ssid_stats_entry);

#endif /* _RDD_WLAN_MCAST_H_ */
