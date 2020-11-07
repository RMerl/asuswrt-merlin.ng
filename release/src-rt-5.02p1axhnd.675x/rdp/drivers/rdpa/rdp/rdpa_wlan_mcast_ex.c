/*
* <:copyright-BRCM:2016:proprietary:standard
* 
*    Copyright (c) 2016 Broadcom 
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

#include "bdmf_dev.h"
#include "rdd.h"
#include "rdpa_wlan_mcast_ex.h"
#include "rdpa_int.h"
#include "rdpa_rdd_inline.h"

/***************************************************************************
 * wlan_mcast object type
 **************************************************************************/

#if (RDPA_WLAN_MCAST_MAX_FLOWS != RDD_WLAN_MCAST_FWD_TABLE_SIZE)
#error "RDPA_WLAN_MCAST_MAX_FLOWS != RDD_WLAN_MCAST_FWD_TABLE_SIZE"
#endif

#if (RDPA_WLAN_MCAST_MAX_FLOWS != RDD_WLAN_MCAST_DHD_LIST_TABLE_SIZE)
#error "RDPA_WLAN_MCAST_MAX_FLOWS != RDD_WLAN_MCAST_DHD_LIST_TABLE_SIZE"
#endif

#if (RDPA_WLAN_MCAST_MAX_DHD_STATIONS != RDD_WLAN_MCAST_DHD_LIST_ENTRY_ARRAY_DHD_STATION_NUMBER)
#error "RDPA_WLAN_MCAST_MAX_DHD_STATIONS != RDD_WLAN_MCAST_DHD_LIST_ENTRY_ARRAY_DHD_STATION_NUMBER"
#endif

#if (RDPA_WLAN_MCAST_MAX_SSID_MAC_ADDRESSES != RDD_WLAN_MCAST_SSID_MAC_ADDRESS_TABLE_SIZE)
#error "RDPA_WLAN_MCAST_MAX_SSID_MAC_ADDRESSES != RDD_WLAN_MCAST_SSID_MAC_ADDRESS_TABLE_SIZE"
#endif
#if (RDPA_WLAN_MCAST_MAX_SSID_STATS != RDD_WLAN_MCAST_SSID_STATS_TABLE_SIZE)
#error "RDPA_WLAN_MCAST_MAX_SSID_STATS != RDD_WLAN_MCAST_SSID_STATS_TABLE_SIZE"
#endif

void wlan_mcast_destroy_ex(struct bdmf_object *mo)
{
/*    wlan_mcast_drv_priv_t *wlan_mcast = (wlan_mcast_drv_priv_t *)bdmf_obj_data(mo); */
    /* XXX: Cleanup all tables! */
}

static void __fwd_table_entry_get(rdpa_wlan_mcast_fwd_table_t *rdpa_fwd_table,
                                  RDD_WLAN_MCAST_FWD_ENTRY_DTS *rdd_fwd_entry)
{
    rdpa_fwd_table->is_proxy_enabled = rdd_fwd_entry->is_proxy_enabled;
    rdpa_fwd_table->wfd_tx_priority = rdd_fwd_entry->tx_priority;
    rdpa_fwd_table->wfd_0_priority = rdd_fwd_entry->wfd_0_priority;
    rdpa_fwd_table->wfd_1_priority = rdd_fwd_entry->wfd_1_priority;
    rdpa_fwd_table->wfd_2_priority = rdd_fwd_entry->wfd_2_priority;
    rdpa_fwd_table->wfd_0_ssid_vector = rdd_fwd_entry->wfd_0_ssid_vector;
    rdpa_fwd_table->wfd_1_ssid_vector = rdd_fwd_entry->wfd_1_ssid_vector;
    rdpa_fwd_table->wfd_2_ssid_vector = rdd_fwd_entry->wfd_2_ssid_vector;

    rdpa_fwd_table->dhd_station_index = 0xFF;
    rdpa_fwd_table->dhd_station_list_size = rdd_fwd_entry->dhd_list_size;
}

static void __fwd_table_entry_set(RDD_WLAN_MCAST_FWD_ENTRY_DTS *rdd_fwd_entry,
                                  RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS *rdd_dhd_list_entry,
                                  rdpa_wlan_mcast_fwd_table_t *rdpa_fwd_table)
{
    rdd_fwd_entry->is_proxy_enabled = rdpa_fwd_table->is_proxy_enabled;
    rdd_fwd_entry->tx_priority = rdpa_fwd_table->wfd_tx_priority;
    rdd_fwd_entry->wfd_0_priority = rdpa_fwd_table->wfd_0_priority;
    rdd_fwd_entry->wfd_1_priority = rdpa_fwd_table->wfd_1_priority;
    rdd_fwd_entry->wfd_2_priority = rdpa_fwd_table->wfd_2_priority;
    rdd_fwd_entry->wfd_0_ssid_vector = rdpa_fwd_table->wfd_0_ssid_vector;
    rdd_fwd_entry->wfd_1_ssid_vector = rdpa_fwd_table->wfd_1_ssid_vector;
    rdd_fwd_entry->wfd_2_ssid_vector = rdpa_fwd_table->wfd_2_ssid_vector;

    rdd_dhd_list_entry->valid = (rdpa_fwd_table->dhd_station_index !=
                                RDPA_WLAN_MCAST_DHD_STATION_INDEX_INVALID) ? 1 : 0;
    rdd_dhd_list_entry->index = rdpa_fwd_table->dhd_station_index;

    rdd_fwd_entry->dhd_list_size = rdpa_fwd_table->dhd_station_list_size;
}

/* "fwd_table" attribute "read" callback */
int wlan_mcast_attr_fwd_table_read_ex(struct bdmf_object *mo, bdmf_index index,
    rdpa_wlan_mcast_fwd_table_t *rdpa_fwd_table)
{
    RDD_WLAN_MCAST_FWD_ENTRY_DTS rdd_fwd_entry;
    int rc;

    /* read the flow data from the RDD */
    rc = rdd_wlan_mcast_fwd_entry_read(index, &rdd_fwd_entry,
        (RDD_WLAN_MCAST_DHD_LIST_ENTRY_ARRAY_DTS *)rdpa_fwd_table->dhd_station_list,
        &rdpa_fwd_table->dhd_station_count);
    if (rc)
        return rc;

    /* Data fill the RDPA structure */
    __fwd_table_entry_get(rdpa_fwd_table, &rdd_fwd_entry);

    return BDMF_ERR_OK;
}

/* "fwd_table" attribute write callback */
int wlan_mcast_attr_fwd_table_write_ex(struct bdmf_object *mo, bdmf_index index,
    rdpa_wlan_mcast_fwd_table_t *rdpa_fwd_table)
{
    RDD_WLAN_MCAST_FWD_ENTRY_DTS rdd_fwd_entry;
    RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS rdd_dhd_list_entry;
    uint8_t prev_dhd_station_count;
    uint32_t dhd_list_add;
    int ret;

    /* Data fill the RDD structure */
    __fwd_table_entry_set(&rdd_fwd_entry, &rdd_dhd_list_entry, rdpa_fwd_table);

    ret = rdd_wlan_mcast_fwd_entry_dhd_station_count_get(index, &prev_dhd_station_count);
    if (ret)
    {
        BDMF_TRACE_ERR("Could not rdd_wlan_mcast_fwd_entry_dhd_station_count_get\n");

        return ret;
    }

    if (rdpa_fwd_table->dhd_station_count > prev_dhd_station_count)
        dhd_list_add = 1; /* add */
    else if (rdpa_fwd_table->dhd_station_count == prev_dhd_station_count)
        dhd_list_add = 2; /* test existence */
    else
        dhd_list_add = 0; /* delete */

    return rdd_wlan_mcast_fwd_entry_write(index, &rdd_fwd_entry, rdd_dhd_list_entry, dhd_list_add);
}

/* "fwd_table" attribute add callback */
int wlan_mcast_attr_fwd_table_add_ex(struct bdmf_object *mo, bdmf_index *index,
    rdpa_wlan_mcast_fwd_table_t *rdpa_fwd_table)
{
    RDD_WLAN_MCAST_FWD_ENTRY_DTS rdd_fwd_entry;
    RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS rdd_dhd_list_entry;

    /* Data fill the RDD structure */
    __fwd_table_entry_set(&rdd_fwd_entry, &rdd_dhd_list_entry, rdpa_fwd_table);

    return rdd_wlan_mcast_fwd_entry_add(&rdd_fwd_entry, rdd_dhd_list_entry, (uint32_t *)index);
}

/* "fwd_table" attribute delete callback */
int wlan_mcast_attr_fwd_table_delete_ex(struct bdmf_object *mo, bdmf_index index)
{
    return rdd_wlan_mcast_fwd_entry_delete(index);
}

/*
 * wlan_mcast dhd_station attribute access
 */

#ifdef CONFIG_DHD_RUNNER
static void __dhd_station_entry_get(rdpa_wlan_mcast_dhd_station_t *rdpa_dhd_station,
                                    RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS *rdd_dhd_station)
{
    uint16_t *rdpa_mac_address_high = (uint16_t *)&rdpa_dhd_station->mac_address.b[0];
    uint16_t *rdpa_mac_address_mid  = (uint16_t *)&rdpa_dhd_station->mac_address.b[2];
    uint16_t *rdpa_mac_address_low  = (uint16_t *)&rdpa_dhd_station->mac_address.b[4];

    *rdpa_mac_address_high = ntohs(rdd_dhd_station->mac_address_high);
    *rdpa_mac_address_mid = ntohs(rdd_dhd_station->mac_address_mid);
    *rdpa_mac_address_low = ntohs(rdd_dhd_station->mac_address_low);
    rdpa_dhd_station->radio_index = rdd_dhd_station->radio_index;
    rdpa_dhd_station->ssid = rdd_dhd_station->ssid;
    rdpa_dhd_station->flowring_index = rdd_dhd_station->flowring_index;
    rdpa_dhd_station->tx_priority = rdd_dhd_station->tx_priority;
    rdpa_dhd_station->reference_count = rdd_dhd_station->reference_count;
}
#endif

int wlan_mcast_attr_dhd_station_read_ex(struct bdmf_object *mo, bdmf_index index,
    rdpa_wlan_mcast_dhd_station_t *rdpa_dhd_station)
{
#ifdef CONFIG_DHD_RUNNER
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS rdd_dhd_station;
    int rc;

    /* read the flow data from the RDD */
    rc = rdd_wlan_mcast_dhd_station_read(index, &rdd_dhd_station);
    if (rc)
        return rc;

    /* Data fill the RDPA structure */
    __dhd_station_entry_get(rdpa_dhd_station, &rdd_dhd_station);

    return BDMF_ERR_OK;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

int wlan_mcast_attr_dhd_station_add_ex(struct bdmf_object *mo, bdmf_index *index,
    rdpa_wlan_mcast_dhd_station_t *rdpa_dhd_station)
{
#ifdef CONFIG_DHD_RUNNER
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS rdd_dhd_station;

    /* Data fill the RDD structure */
    __dhd_station_entry_set(&rdd_dhd_station, rdpa_dhd_station);

    return rdd_wlan_mcast_dhd_station_add(&rdd_dhd_station, (uint32_t *)index);
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

int wlan_mcast_attr_dhd_station_delete_ex(struct bdmf_object *mo, bdmf_index index)
{
#ifdef CONFIG_DHD_RUNNER
    return rdd_wlan_mcast_dhd_station_delete(index);
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

/* "dhd_station" attribute find callback */
int wlan_mcast_attr_dhd_station_find_ex(struct bdmf_object *mo, bdmf_index *index,
    rdpa_wlan_mcast_dhd_station_t *rdpa_dhd_station)
{
#ifdef CONFIG_DHD_RUNNER
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS rdd_dhd_station;

    /* Data fill the RDD structure */
    __dhd_station_entry_set(&rdd_dhd_station, rdpa_dhd_station);

    return rdd_wlan_mcast_dhd_station_find(&rdd_dhd_station, (uint32_t *)index);
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}
