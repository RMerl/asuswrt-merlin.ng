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

#ifdef CONFIG_WLAN_MCAST

#include "bdmf_dev.h"
#include "rdd.h"
#include "rdpa_wlan_mcast_ex.h"
#include "rdpa_int.h"
#ifdef CONFIG_DHD_RUNNER
#include "rdd_ag_dhd_tx_post.h"
#include "rdd_dhd_helper.h"
#endif

static rdpa_wlan_mcast_fwd_table_t *mcast_fwd_table[RDPA_WLAN_MCAST_MAX_FLOWS] = {};

rdpa_wlan_mcast_fwd_table_t *__wlan_mcast_fwd_table_get(int idx)
{
    if (idx == RDPA_WLAN_MCAST_MAX_FLOWS)
        return NULL;
    return mcast_fwd_table[idx];
}

#ifdef CONFIG_DHD_RUNNER
static rdpa_wlan_mcast_dhd_station_t dhd_stations[RDPA_WLAN_MCAST_MAX_DHD_STATIONS] = {};

static int __wlan_dhd_station_rdd_update(bdmf_index idx)
{
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS entry;
    int rc;

    /* Data fill the RDD structure */
    __dhd_station_entry_set(&entry, &dhd_stations[idx]);
    rc = rdd_ag_dhd_tx_post_wlan_mcast_dhd_station_table_set(idx, entry.mac_address_high, entry.mac_address_mid,
        entry.mac_address_low, entry.radio_index, entry.valid, entry.tx_priority, entry.flowring_index);

    rc = rc ? rc : rdd_ag_dhd_tx_post_wlan_mcast_dhd_station_ctx_table_set(idx, dhd_stations[idx].ssid); 

    return rc;
}

static int __wlan_mcast_dhd_list_entry_update(bdmf_index rdpa_fwd_table_index, bdmf_index dhd_station_index,
    uint32_t *dhd_list_size, int is_add)
{
    RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS dhd_list_entry;
    int rc;

    dhd_list_entry.valid = 1;
    dhd_list_entry.index = dhd_station_index;

    if (is_add)
    {
        if (*dhd_list_size && !rdd_wlan_mcast_dhd_list_entry_find(rdpa_fwd_table_index, &dhd_list_entry))
            return 0; /* Already present */

        rc = rdd_wlan_mcast_dhd_list_entry_add(rdpa_fwd_table_index, &dhd_list_entry, dhd_list_size);
    }
    else
        rc = rdd_wlan_mcast_dhd_list_entry_delete(rdpa_fwd_table_index, &dhd_list_entry, dhd_list_size);
    if (rc)
        return rc;
    return rdd_ag_dhd_tx_post_wlan_mcast_dft_list_size_set(rdpa_fwd_table_index, (uint8_t)(*dhd_list_size));
}
#endif

static int __wlan_mcast_fwd_table_free(int idx)
{
    if (!mcast_fwd_table[idx])
        return BDMF_ERR_NOENT;

#ifdef CONFIG_DHD_RUNNER
    /* If DHD table is not empty, free the table first */
    rdd_wlan_mcast_dhd_list_delete(idx);
#endif

    bdmf_free(mcast_fwd_table[idx]);
    mcast_fwd_table[idx] = NULL;
    return 0;
}

void wlan_mcast_destroy_ex(struct bdmf_object *mo)
{
    int i;

    for (i = 0; i < RDPA_WLAN_MCAST_MAX_FLOWS; i++)
        __wlan_mcast_fwd_table_free(i);

#ifdef CONFIG_DHD_RUNNER
    for (i = 0; i < RDPA_WLAN_MCAST_MAX_DHD_STATIONS; i++)
    {
        dhd_stations[i].reference_count = 0;
        __wlan_dhd_station_rdd_update(i);
    }

    rdd_wlan_mcast_dft_init(0);
#endif
}

int wlan_mcast_dhd_station_per_radio_cnt_read_ex(bdmf_index rdpa_fwd_table_index, uint8_t radio_index)
{
#ifdef CONFIG_DHD_RUNNER
    uint8_t dhd_station_list[RDPA_WLAN_MCAST_MAX_DHD_STATIONS];
    uint8_t dhd_station_count_total = 0;
    uint8_t dhd_station_count_per_radio = 0;
    RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS *dhd_list_entry_ptr = NULL;
    int i;

    dhd_station_count_total = rdd_wlan_mcast_dhd_list_scan(rdpa_fwd_table_index,
        (RDD_WLAN_MCAST_DHD_LIST_ENTRY_ARRAY_DTS *)&dhd_station_list);

    if (0 == dhd_station_count_total)
        return 0;

    for (i = 0; i < RDPA_WLAN_MCAST_MAX_DHD_STATIONS; ++i)
    {        
        dhd_list_entry_ptr = (RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS *)&dhd_station_list[i];
        if (dhd_list_entry_ptr->valid && 
            (radio_index == dhd_stations[dhd_list_entry_ptr->index].radio_index))
        {
            dhd_station_count_per_radio++;
        }
    }

    return dhd_station_count_per_radio;
#else
    return 0;
#endif
}


/* "fwd_table" attribute "read" callback */
int wlan_mcast_attr_fwd_table_read_ex(struct bdmf_object *mo, bdmf_index index,
    rdpa_wlan_mcast_fwd_table_t *rdpa_fwd_table)
{
    if (!mcast_fwd_table[index])
        return BDMF_ERR_NOENT;

    memcpy(rdpa_fwd_table, mcast_fwd_table[index], sizeof(rdpa_wlan_mcast_fwd_table_t));
#ifdef CONFIG_DHD_RUNNER
    rdpa_fwd_table->dhd_station_count = rdd_wlan_mcast_dhd_list_scan(index,
        (RDD_WLAN_MCAST_DHD_LIST_ENTRY_ARRAY_DTS *)&rdpa_fwd_table->dhd_station_list);
#else
    rdpa_fwd_table->dhd_station_count = 0;
#endif

    return 0;
}

/* "fwd_table" attribute write callback */
int wlan_mcast_attr_fwd_table_write_ex(struct bdmf_object *mo, bdmf_index index,
    rdpa_wlan_mcast_fwd_table_t *rdpa_fwd_table)
{
    if (!mcast_fwd_table[index])
        return BDMF_ERR_NOENT;

#ifdef CONFIG_DHD_RUNNER
    {
        uint32_t dhd_list_size;

        dhd_list_size = rdd_wlan_mcast_dhd_list_scan(index, NULL);
        if (rdpa_fwd_table->dhd_station_count > dhd_list_size)
        {
            /* If DHD station added, update the list */
            int rc = __wlan_mcast_dhd_list_entry_update(index, rdpa_fwd_table->dhd_station_index, &dhd_list_size, 1);
            if (rc)
                return rc;
        }
        else if ((rdpa_fwd_table->dhd_station_count == dhd_list_size) && 
             (rdpa_fwd_table->dhd_station_index != RDPA_WLAN_MCAST_DHD_STATION_INDEX_INVALID))
        {
            /* just test if the requested dhd_station already exists in requested fwd_table,
                    ** never change fwd_table.
                    */
            RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS dhd_list_entry;

            dhd_list_entry.valid = 1;
            dhd_list_entry.index = rdpa_fwd_table->dhd_station_index;

            if (dhd_list_size && !rdd_wlan_mcast_dhd_list_entry_find(index, &dhd_list_entry))
                return BDMF_ERR_ALREADY; /* Already present */
            else
                return BDMF_ERR_NOENT; /* Not present */
        }
        else if (dhd_list_size)
        {
            __wlan_mcast_dhd_list_entry_update(index, rdpa_fwd_table->dhd_station_index, &dhd_list_size, 0);
        }
        rdpa_fwd_table->dhd_station_list_size = dhd_list_size;
        rdpa_fwd_table->dhd_station_count = rdd_wlan_mcast_dhd_list_scan(index, NULL);
    }
#else
    rdpa_fwd_table->dhd_station_count = 0;
    rdpa_fwd_table->dhd_station_list_size = 0;
#endif

    memcpy(mcast_fwd_table[index], rdpa_fwd_table, sizeof(rdpa_wlan_mcast_fwd_table_t));
    return 0;
}

/* "fwd_table" attribute add callback */
int wlan_mcast_attr_fwd_table_add_ex(struct bdmf_object *mo, bdmf_index *index,
    rdpa_wlan_mcast_fwd_table_t *rdpa_fwd_table)
{
    int idx;

    for (idx = 0; idx < RDPA_WLAN_MCAST_MAX_FLOWS; idx++)
    {
        if (!mcast_fwd_table[idx])
            break;
    }
    if (idx == RDPA_WLAN_MCAST_MAX_FLOWS)
        return BDMF_ERR_NOMEM;
    mcast_fwd_table[idx] = (rdpa_wlan_mcast_fwd_table_t *)bdmf_alloc(sizeof(rdpa_wlan_mcast_fwd_table_t));
    if (!mcast_fwd_table[idx])
        return BDMF_ERR_NOMEM;

#ifdef CONFIG_DHD_RUNNER
    /* If DHD station added, create a list */
    if (rdpa_fwd_table->dhd_station_index != RDPA_WLAN_MCAST_DHD_STATION_INDEX_INVALID)
    {
        int rc;
        uint32_t dhd_list_size = 0;

        rc = __wlan_mcast_dhd_list_entry_update(idx, rdpa_fwd_table->dhd_station_index, &dhd_list_size, 1);
        if (rc)
        {
            bdmf_free(mcast_fwd_table[idx]);
            mcast_fwd_table[idx] = NULL;
            return rc;
        }
        rdpa_fwd_table->dhd_station_list_size = dhd_list_size;
    }
#else
    rdpa_fwd_table->dhd_station_count = 0;
    rdpa_fwd_table->dhd_station_list_size = 0;
#endif

    memcpy(mcast_fwd_table[idx], rdpa_fwd_table, sizeof(rdpa_wlan_mcast_fwd_table_t));

    *index = idx;
    return 0;
}

/* "fwd_table" attribute delete callback */
int wlan_mcast_attr_fwd_table_delete_ex(struct bdmf_object *mo, bdmf_index index)
{
    return __wlan_mcast_fwd_table_free((int)index);
}

int wlan_mcast_attr_dhd_station_read_ex(struct bdmf_object *mo, bdmf_index index,
    rdpa_wlan_mcast_dhd_station_t *rdpa_dhd_station)
{
#ifdef CONFIG_DHD_RUNNER
    if (!dhd_stations[index].reference_count)
        return BDMF_ERR_NOENT;

    memcpy(rdpa_dhd_station, &dhd_stations[index], sizeof(rdpa_wlan_mcast_dhd_station_t));
    return 0;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

int wlan_mcast_attr_dhd_station_add_ex(struct bdmf_object *mo, bdmf_index *index,
    rdpa_wlan_mcast_dhd_station_t *rdpa_dhd_station)
{
#ifdef CONFIG_DHD_RUNNER
    int i, rc;

    if (!wlan_mcast_attr_dhd_station_find_ex(mo, index, rdpa_dhd_station))
    {
        /* Entry already present, update reference count */
        dhd_stations[*index].reference_count++;
        return BDMF_ERR_OK;
    }
    /* Find empty slow */
    for (i = 0; i < RDPA_WLAN_MCAST_MAX_DHD_STATIONS && dhd_stations[i].reference_count; i++)
        ;
    if (i == RDPA_WLAN_MCAST_MAX_DHD_STATIONS)
        return BDMF_ERR_NORES;

    memcpy(&dhd_stations[i], rdpa_dhd_station, sizeof(rdpa_wlan_mcast_dhd_station_t));
    dhd_stations[i].reference_count = 1;

    rc = __wlan_dhd_station_rdd_update(i);
    if (rc)
        return rc;

    *index = i;
    return BDMF_ERR_OK;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

int wlan_mcast_attr_dhd_station_delete_ex(struct bdmf_object *mo, bdmf_index index)
{
#ifdef CONFIG_DHD_RUNNER
    if (!dhd_stations[index].reference_count)
        return BDMF_ERR_NOENT;

    if (--dhd_stations[index].reference_count)
        return BDMF_ERR_OK; /* Still valid */

    return __wlan_dhd_station_rdd_update(index);
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

/* "dhd_station" attribute find callback */
int wlan_mcast_attr_dhd_station_find_ex(struct bdmf_object *mo, bdmf_index *index,
    rdpa_wlan_mcast_dhd_station_t *rdpa_dhd_station)
{
#ifdef CONFIG_DHD_RUNNER
    int i;

    for (i = 0; i < RDPA_WLAN_MCAST_MAX_DHD_STATIONS; i++)
    {
        if (!dhd_stations[i].reference_count ||
            dhd_stations[i].radio_index != rdpa_dhd_station->radio_index ||
            dhd_stations[i].flowring_index != rdpa_dhd_station->flowring_index ||
            memcmp(&dhd_stations[i].mac_address, &rdpa_dhd_station->mac_address, sizeof(bdmf_mac_t)))
        {
            continue;
        }
        *index = i;
        return BDMF_ERR_OK;
    }

    return BDMF_ERR_NOENT;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

#endif
