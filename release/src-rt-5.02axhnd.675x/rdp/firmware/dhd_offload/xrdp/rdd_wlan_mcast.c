/*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/


#ifdef CONFIG_WLAN_MCAST

#include "rdd.h"
#include "rdd_wlan_mcast_common.h"
#ifdef CONFIG_DHD_RUNNER
#include "rdd_dhd_helper.h"
#include "rdd_ag_dhd_tx_post.h"
#endif

#ifdef CONFIG_DHD_RUNNER
/* SSID Macs for proxy mode */

static int _rdd_wlan_mcast_ssid_mac_address_get(uint32_t ssid_mac_idx, RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_DTS *entry,
    int ref_count_only)
{
    if (ssid_mac_idx >= RDD_WLAN_MCAST_SSID_MAC_ADDRESS_TABLE_SIZE)
        return BDMF_ERR_RANGE;

    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_REFERENCE_COUNT_READ_G(entry->reference_count,
        RDD_WLAN_MCAST_SSID_MAC_ADDRESS_TABLE_ADDRESS_ARR, ssid_mac_idx);
    if (!entry->reference_count)
        return BDMF_ERR_NOENT;

    if (!ref_count_only)
    {
        RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_MAC_ADDRESS_HIGH_READ_G(entry->mac_address_high,
            RDD_WLAN_MCAST_SSID_MAC_ADDRESS_TABLE_ADDRESS_ARR, ssid_mac_idx);

        RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_MAC_ADDRESS_LOW_READ_G(entry->mac_address_low,
            RDD_WLAN_MCAST_SSID_MAC_ADDRESS_TABLE_ADDRESS_ARR, ssid_mac_idx);
    }

    return 0;
}

static void _rdd_wlan_mcast_ssid_mac_address_set(uint32_t ssid_mac_idx,
    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_DTS *entry)
{
    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_REFERENCE_COUNT_WRITE_G(entry->reference_count,
        RDD_WLAN_MCAST_SSID_MAC_ADDRESS_TABLE_ADDRESS_ARR, ssid_mac_idx);

    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_MAC_ADDRESS_HIGH_WRITE_G(entry->mac_address_high,
        RDD_WLAN_MCAST_SSID_MAC_ADDRESS_TABLE_ADDRESS_ARR, ssid_mac_idx);

    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_MAC_ADDRESS_LOW_WRITE_G(entry->mac_address_low,
        RDD_WLAN_MCAST_SSID_MAC_ADDRESS_TABLE_ADDRESS_ARR, ssid_mac_idx);
}

int rdd_wlan_mcast_ssid_mac_address_add(uint32_t radio_index, uint32_t ssid,
    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_DTS *entry, uint32_t *ssid_mac_idx)
{
    RDD_BTRACE("radio_index = %d, ssid = %d, entry = %p, ssid_mac_idx = %p\n",
        radio_index, ssid, entry, ssid_mac_idx);

    *ssid_mac_idx = RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_INDEX(radio_index, ssid);
    if (_rdd_wlan_mcast_ssid_mac_address_get(*ssid_mac_idx, entry, 1) == BDMF_ERR_RANGE)
        return BDMF_ERR_RANGE;

    entry->reference_count++;
    _rdd_wlan_mcast_ssid_mac_address_set(*ssid_mac_idx, entry);

    return 0;
}

int rdd_wlan_mcast_ssid_mac_address_delete(uint32_t ssid_mac_idx)
{
    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_DTS entry;
    int rc;

    RDD_BTRACE("ssid_mac_idx = %d\n", ssid_mac_idx);

    rc = _rdd_wlan_mcast_ssid_mac_address_get(ssid_mac_idx, &entry, 0);
    if (rc)
        return rc;

    if (!--entry.reference_count)
    {
        entry.mac_address_low = 0;
        entry.mac_address_high = 0;
    }
    _rdd_wlan_mcast_ssid_mac_address_set(ssid_mac_idx, &entry);
    return 0;
}

int rdd_wlan_mcast_ssid_mac_address_read(uint32_t ssid_mac_idx,
    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_DTS *entry)
{
    RDD_BTRACE("ssid_mac_idx = %d, entry = %p\n", ssid_mac_idx, entry);

    return _rdd_wlan_mcast_ssid_mac_address_get(ssid_mac_idx, entry, 0);
}

#endif /* CONFIG_DHD_RUNNER */

int rdd_wlan_mcast_init(wlan_mcast_dhd_list_table_t *table)
{
    int rc;

    RDD_BTRACE("DFT PTR: Virt = %p, Phys = %llx\n", table ? table->virt_p : NULL,
        table ? (long long)table->phys_addr : 0);
    rc = rdd_wlan_mcast_init_common(table);
    if (rc)
        return rc;

#ifdef CONFIG_DHD_RUNNER
    rdd_wlan_mcast_dft_init(table->phys_addr);
#endif
    return 0;
}

#ifdef CONFIG_DHD_RUNNER
void rdd_wlan_mcast_dft_init(bdmf_phys_addr_t dft_phys_addr)
{
    uint32_t addr_hi, addr_lo;
    int i;

    GET_ADDR_HIGH_LOW(addr_hi, addr_lo, dft_phys_addr);
    rdd_ag_dhd_tx_post_wlan_mcast_dft_addr_set(addr_lo, addr_hi);

    for (i = 0; i < RDD_WLAN_MCAST_DFT_LIST_SIZE_SIZE; i++)
        rdd_ag_dhd_tx_post_wlan_mcast_dft_list_size_set(i, 0);
}
#endif


#ifdef CONFIG_DHD_RUNNER
int rdd_wlan_mcast_ssid_stats_read(uint32_t ssid_stats_index,
                                   RDD_WLAN_MCAST_SSID_STATS_ENTRY_DTS *ssid_stats_entry)
{
    if (ssid_stats_index >= RDD_WLAN_MCAST_SSID_STATS_TABLE_SIZE)
        return BDMF_ERR_RANGE;

    RDD_WLAN_MCAST_SSID_STATS_ENTRY_PACKETS_READ_G(ssid_stats_entry->packets, RDD_WLAN_MCAST_SSID_STATS_TABLE_ADDRESS_ARR, ssid_stats_index);
    
    if (!ssid_stats_entry->packets)
        return BDMF_ERR_NOENT;

    RDD_WLAN_MCAST_SSID_STATS_ENTRY_BYTES_READ_G(ssid_stats_entry->bytes, RDD_WLAN_MCAST_SSID_STATS_TABLE_ADDRESS_ARR, ssid_stats_index);

    return BDMF_ERR_OK;
}
#endif

#endif

