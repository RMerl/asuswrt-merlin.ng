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


#include "rdd.h"
#include "rdd_wlan_mcast_common.h"

/*
 *  DHD List table management API
 */

wlan_mcast_dhd_list_table_t wlan_mcast_dhd_list_table_g = { NULL, 0 };

#ifdef CONFIG_DHD_RUNNER
int rdd_wlan_mcast_dhd_list_entry_add(uint32_t fwd_entry_index,
    RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS *dhd_list_entry, uint32_t *dhd_list_size)
{
    RDD_WLAN_MCAST_DHD_LIST_TABLE_DTS *dhd_list_table = wlan_mcast_dhd_list_table_g.virt_p;
    uint32_t list_size = 0;
    uint32_t i;
    uint32_t done = 0;

    for (i = 0; i < RDD_WLAN_MCAST_DHD_LIST_ENTRY_ARRAY_DHD_STATION_NUMBER; ++i)
    {
        RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS *dhd_list_entry_ptr = (RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS *)
            &dhd_list_table->entry[fwd_entry_index].dhd_station[i];
        uint32_t valid;

        RDD_WLAN_MCAST_DHD_LIST_ENTRY_VALID_READ(valid, dhd_list_entry_ptr);

        if (!valid && !done)
        {
            __debug("DHD List ADD: entry %p (%u) -> value %u\n", dhd_list_entry_ptr, i, dhd_list_entry->index);

            RDD_WLAN_MCAST_DHD_LIST_ENTRY_VALID_WRITE(1, dhd_list_entry_ptr);
            RDD_WLAN_MCAST_DHD_LIST_ENTRY_INDEX_WRITE(dhd_list_entry->index, dhd_list_entry_ptr);

            valid = 1;
            done = 1;
        }

        if (valid)
        {
            list_size = i + 1;
        }
    }

    *dhd_list_size = list_size;

    return (1 == done) ? BDMF_ERR_OK : BDMF_ERR_NORES;
}

int rdd_wlan_mcast_dhd_list_entry_delete(uint32_t fwd_entry_index,
    RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS *dhd_list_entry, uint32_t *dhd_list_size)
{
    RDD_WLAN_MCAST_DHD_LIST_TABLE_DTS *dhd_list_table = wlan_mcast_dhd_list_table_g.virt_p;
    uint32_t i;
    uint32_t list_size = 0;
    uint32_t done = 0;

    for (i = 0; i < RDD_WLAN_MCAST_DHD_LIST_ENTRY_ARRAY_DHD_STATION_NUMBER; ++i)
    {
        RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS *dhd_list_entry_ptr = (RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS *)
            &dhd_list_table->entry[fwd_entry_index].dhd_station[i];
        uint32_t valid;
        uint32_t index;

        RDD_WLAN_MCAST_DHD_LIST_ENTRY_VALID_READ(valid, dhd_list_entry_ptr);
        RDD_WLAN_MCAST_DHD_LIST_ENTRY_INDEX_READ(index, dhd_list_entry_ptr);

        if (valid && (index == dhd_list_entry->index) && !done)
        {
            __debug("DHD List DELETE: entry %p (%u) -> value %u\n", dhd_list_entry_ptr, i, dhd_list_entry->index);

            RDD_WLAN_MCAST_DHD_LIST_ENTRY_VALID_WRITE(0, dhd_list_entry_ptr);
            RDD_WLAN_MCAST_DHD_LIST_ENTRY_INDEX_WRITE(0, dhd_list_entry_ptr);

            valid = 0;
            done = 1;
        }

        if (valid)
        {
            list_size = i + 1;
        }
    }

    *dhd_list_size = list_size;

    return (1 == done) ? BDMF_ERR_OK : BDMF_ERR_NORES;
}

int rdd_wlan_mcast_dhd_list_entry_find(uint32_t fwd_entry_index,
    RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS *dhd_list_entry)
{
    RDD_WLAN_MCAST_DHD_LIST_TABLE_DTS *dhd_list_table = wlan_mcast_dhd_list_table_g.virt_p;
    uint32_t i;

    __debug("Entered, fwd_entry_index %d, dhd_list_entry->index = %d, dhd_list_entry->valid = %d\n",
        fwd_entry_index, dhd_list_entry->index, dhd_list_entry->valid);
    for (i = 0; i < RDD_WLAN_MCAST_DHD_LIST_ENTRY_ARRAY_DHD_STATION_NUMBER; ++i)
    {
        RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS *dhd_list_entry_ptr = (RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS *)
            &dhd_list_table->entry[fwd_entry_index].dhd_station[i];
        uint32_t valid;
        uint32_t index;

        RDD_WLAN_MCAST_DHD_LIST_ENTRY_VALID_READ(valid, dhd_list_entry_ptr);
        RDD_WLAN_MCAST_DHD_LIST_ENTRY_INDEX_READ(index, dhd_list_entry_ptr);

        if (valid && (index == dhd_list_entry->index))
        {
            __debug("DHD List FIND: entry %p (%u) -> value %u\n", dhd_list_entry_ptr, i, dhd_list_entry->index);

            return BDMF_ERR_OK;
        }
    }

    return BDMF_ERR_NORES;
}

void rdd_wlan_mcast_dhd_list_delete(uint32_t fwd_entry_index)
{
    uint32_t i;

    for (i = 0; i < RDD_WLAN_MCAST_DHD_LIST_ENTRY_ARRAY_DHD_STATION_NUMBER; ++i)
    {
        RDD_WLAN_MCAST_DHD_LIST_TABLE_DTS *dhd_list_table = wlan_mcast_dhd_list_table_g.virt_p;
        RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS *dhd_list_entry_ptr = (RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS *)
            &dhd_list_table->entry[fwd_entry_index].dhd_station[i];

        RDD_WLAN_MCAST_DHD_LIST_ENTRY_VALID_WRITE(0, dhd_list_entry_ptr);
    }
}

uint8_t rdd_wlan_mcast_dhd_list_scan(uint32_t fwd_entry_index,
    RDD_WLAN_MCAST_DHD_LIST_ENTRY_ARRAY_DTS *dhd_list_entry_array)
{
    RDD_WLAN_MCAST_DHD_LIST_TABLE_DTS *dhd_list_table = wlan_mcast_dhd_list_table_g.virt_p;
    uint8_t size = 0;
    uint32_t valid;
    int i;

    for (i = 0; i < RDD_WLAN_MCAST_DHD_LIST_ENTRY_ARRAY_DHD_STATION_NUMBER; ++i)
    {
        RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS *dhd_list_entry_ptr = (RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS *)
            &dhd_list_table->entry[fwd_entry_index].dhd_station[i];

        if (dhd_list_entry_array)
            dhd_list_entry_array->dhd_station[i] = dhd_list_table->entry[fwd_entry_index].dhd_station[i];

        RDD_WLAN_MCAST_DHD_LIST_ENTRY_VALID_READ(valid, dhd_list_entry_ptr);

        if (valid)
            (size)++;
    }

    return size;
}
#endif /* CONFIG_DHD_RUNNER */

/*
 *  Initialization API
 */

int rdd_wlan_mcast_init_common(wlan_mcast_dhd_list_table_t *table)
{
    /* Initialize the DHD FWD Table */
    if (table)
    {
        wlan_mcast_dhd_list_table_g.virt_p = table->virt_p;
        wlan_mcast_dhd_list_table_g.phys_addr = table->phys_addr;
    }
    else
    {
        wlan_mcast_dhd_list_table_g.virt_p = 0;
        wlan_mcast_dhd_list_table_g.phys_addr = 0;
    }
    return 0;
}

