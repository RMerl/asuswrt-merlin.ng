/*
 * <:copyright-BRCM:2013:DUAL/GPL:standard
 * 
 *    Copyright (c) 2013 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
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
#elif defined(_CFE_)
#define __print(fmt, arg...) printf(fmt, ##arg)
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
    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_DTS *ssid_mac_address_entry,
    uint32_t *ssid_mac_address_index);

int rdd_wlan_mcast_ssid_mac_address_delete(uint32_t ssid_mac_address_index);

int rdd_wlan_mcast_ssid_mac_address_read(uint32_t ssid_mac_address_index,
    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_DTS *ssid_mac_address_entry);

int rdd_wlan_mcast_dhd_list_entry_add(uint32_t fwd_entry_index,
    RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS *dhd_list_entry, uint32_t *dhd_list_size);
int rdd_wlan_mcast_dhd_list_entry_delete(uint32_t fwd_entry_index,
    RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS *dhd_list_entry, uint32_t *dhd_list_size);
int rdd_wlan_mcast_dhd_list_entry_find(uint32_t fwd_entry_index,
    RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS *dhd_list_entry);

uint8_t rdd_wlan_mcast_dhd_list_scan(uint32_t fwd_entry_index,
    RDD_WLAN_MCAST_DHD_LIST_ENTRY_ARRAY_DTS *dhd_list_entry_array);
void rdd_wlan_mcast_dhd_list_delete(uint32_t fwd_entry_index);
#endif

int rdd_wlan_mcast_init(wlan_mcast_dhd_list_table_t *table);
int rdd_wlan_mcast_init_common(wlan_mcast_dhd_list_table_t *table);

int rdd_wlan_mcast_ssid_stats_read(uint32_t ssid_stats_index,
    RDD_WLAN_MCAST_SSID_STATS_ENTRY_DTS *ssid_stats_entry);

#endif /* _RDD_WLAN_MCAST_H_ */
