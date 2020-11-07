/*
* <:copyright-BRCM:2015:proprietary:standard
* 
*    Copyright (c) 2015 Broadcom 
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

/*
 * rdpa_wlan_mcast.c
 *
 *  Created on: April 30, 2015
 *      Author: mdemaria
 */

#include <bdmf_dev.h>
#include "rdd.h"
#include "rdpa_wlan_mcast_ex.h"
#include "rdd_wlan_mcast_common.h"

const bdmf_attr_enum_table_t rdpa_wlan_mcast_wfd0_enum_table =
{
    .type_name = "wlan_ssid", .help = "WLAN mcast SSID wfd 0",
    .values = {
        {"ssid_0_0", rdpa_wlan_ssid0},
        {"ssid_0_1", rdpa_wlan_ssid1},
        {"ssid_0_2", rdpa_wlan_ssid2},
        {"ssid_0_3", rdpa_wlan_ssid3},
        {"ssid_0_4", rdpa_wlan_ssid4},
        {"ssid_0_5", rdpa_wlan_ssid5},
        {"ssid_0_6", rdpa_wlan_ssid6},
        {"ssid_0_7", rdpa_wlan_ssid7},
        {"ssid_0_8", rdpa_wlan_ssid8},
        {"ssid_0_9", rdpa_wlan_ssid9},
        {"ssid_0_10", rdpa_wlan_ssid10},
        {"ssid_0_11", rdpa_wlan_ssid11},
        {"ssid_0_12", rdpa_wlan_ssid12},
        {"ssid_0_13", rdpa_wlan_ssid13},
        {"ssid_0_14", rdpa_wlan_ssid14},
        {"ssid_0_15", rdpa_wlan_ssid15},
        {NULL, 0}
    }
};

const bdmf_attr_enum_table_t rdpa_wlan_mcast_wfd1_enum_table =
{
    .type_name = "wlan_ssid", .help = "WLAN mcast SSID wfd 1",
    .values = {
        {"ssid_1_0", rdpa_wlan_ssid0},
        {"ssid_1_1", rdpa_wlan_ssid1},
        {"ssid_1_2", rdpa_wlan_ssid2},
        {"ssid_1_3", rdpa_wlan_ssid3},
        {"ssid_1_4", rdpa_wlan_ssid4},
        {"ssid_1_5", rdpa_wlan_ssid5},
        {"ssid_1_6", rdpa_wlan_ssid6},
        {"ssid_1_7", rdpa_wlan_ssid7},
        {"ssid_1_8", rdpa_wlan_ssid8},
        {"ssid_1_9", rdpa_wlan_ssid9},
        {"ssid_1_10", rdpa_wlan_ssid10},
        {"ssid_1_11", rdpa_wlan_ssid11},
        {"ssid_1_12", rdpa_wlan_ssid12},
        {"ssid_1_13", rdpa_wlan_ssid13},
        {"ssid_1_14", rdpa_wlan_ssid14},
        {"ssid_1_15", rdpa_wlan_ssid15},
        {NULL, 0}
    }
};

const bdmf_attr_enum_table_t rdpa_wlan_mcast_wfd2_enum_table =
{
    .type_name = "wlan_ssid", .help = "WLAN mcast SSID wfd 2",
    .values = {
        {"ssid_2_0", rdpa_wlan_ssid0},
        {"ssid_2_1", rdpa_wlan_ssid1},
        {"ssid_2_2", rdpa_wlan_ssid2},
        {"ssid_2_3", rdpa_wlan_ssid3},
        {"ssid_2_4", rdpa_wlan_ssid4},
        {"ssid_2_5", rdpa_wlan_ssid5},
        {"ssid_2_6", rdpa_wlan_ssid6},
        {"ssid_2_7", rdpa_wlan_ssid7},
        {"ssid_2_8", rdpa_wlan_ssid8},
        {"ssid_2_9", rdpa_wlan_ssid9},
        {"ssid_2_10", rdpa_wlan_ssid10},
        {"ssid_2_11", rdpa_wlan_ssid11},
        {"ssid_2_12", rdpa_wlan_ssid12},
        {"ssid_2_13", rdpa_wlan_ssid13},
        {"ssid_2_14", rdpa_wlan_ssid14},
        {"ssid_2_15", rdpa_wlan_ssid15},
        {NULL, 0}
    }
};

struct bdmf_object *wlan_mcast_object;
static DEFINE_BDMF_FASTLOCK(wlan_mcast_lock);

/*
 * mcast object callback funtions
 */

/** This optional callback is called at object init time
 * after initial attributes are set.
 * Its work is:
 * - make sure that all necessary attributes are set and make sense
 * - allocate dynamic resources if any
 * - assign object name if not done in pre_init
 * - finalise object creation
 * If function returns error code !=0, object creation is aborted
 */

static int wlan_mcast_post_init(struct bdmf_object *mo)
{
    wlan_mcast_drv_priv_t *wlan_mcast = (wlan_mcast_drv_priv_t *)bdmf_obj_data(mo);
    int rc;

#ifdef CONFIG_DHD_RUNNER
    wlan_mcast->dhd_list_table.virt_p = bdmf_alloc_uncached(WLAN_MCAST_DHD_LIST_TABLE_SIZE,
        &wlan_mcast->dhd_list_table.phys_addr);
    if (!wlan_mcast->dhd_list_table.virt_p)
        return BDMF_ERR_NOMEM;

    memset(wlan_mcast->dhd_list_table.virt_p, 0, WLAN_MCAST_DHD_LIST_TABLE_SIZE);
#endif

    rc = rdd_wlan_mcast_init(&wlan_mcast->dhd_list_table);
    if (rc)
        return rc;

    /* save pointer to the mcast object */
    wlan_mcast_object = mo;

    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "mcast");
    return 0;
}

static void wlan_mcast_destroy(struct bdmf_object *mo)
{
    wlan_mcast_drv_priv_t *wlan_mcast = (wlan_mcast_drv_priv_t *)bdmf_obj_data(mo);

    wlan_mcast_destroy_ex(mo);

#ifdef CONFIG_DHD_RUNNER
    if (wlan_mcast->dhd_list_table.virt_p)
    {
        bdmf_free_uncached(wlan_mcast->dhd_list_table.virt_p, wlan_mcast->dhd_list_table.phys_addr,
            WLAN_MCAST_DHD_LIST_TABLE_SIZE);
    }
#else
    wlan_mcast->dhd_list_table.virt_p = NULL;
    wlan_mcast->dhd_list_table.phys_addr = 0;
#endif

    wlan_mcast_object = NULL;
}

/** find mcast object */
static int wlan_mcast_get(struct bdmf_type *drv, struct bdmf_object *owner,
                          const char *discr, struct bdmf_object **pmo)
{
    if (wlan_mcast_object == NULL)
        return BDMF_ERR_NOENT;

    *pmo = wlan_mcast_object;
    return 0;
}

/*
 * wlan_mcast fwd_table attribute access
 */

/* "fwd_table" attribute "read" callback */
static int wlan_mcast_attr_fwd_table_read(struct bdmf_object *mo, struct bdmf_attr *ad,
                                          bdmf_index index, void *val, uint32_t size)
{
    rdpa_wlan_mcast_fwd_table_t *rdpa_fwd_table = (rdpa_wlan_mcast_fwd_table_t *)val;

    if (size != sizeof(rdpa_wlan_mcast_fwd_table_t))
    {
        BDMF_TRACE_ERR("%s() : Size difference <%d:%d>\n", __FUNCTION__,
            (int)size, (int)sizeof(rdpa_wlan_mcast_fwd_table_t));

        return BDMF_ERR_NOENT;
    }
    return wlan_mcast_attr_fwd_table_read_ex(mo, index, rdpa_fwd_table);
}

/* "fwd_table" attribute write callback */
static int wlan_mcast_attr_fwd_table_write(struct bdmf_object *mo, struct bdmf_attr *ad,
                                           bdmf_index index, const void *val, uint32_t size)
{
    rdpa_wlan_mcast_fwd_table_t *rdpa_fwd_table = (rdpa_wlan_mcast_fwd_table_t *)val;

    if (mo->state != bdmf_state_active)
        return BDMF_ERR_INVALID_OP;

    if (size != sizeof(rdpa_wlan_mcast_fwd_table_t))
    {
        BDMF_TRACE_ERR("%s() : Size difference <%d:%d>\n", __FUNCTION__,
            (int)size, (int)sizeof(rdpa_wlan_mcast_fwd_table_t));

        return BDMF_ERR_NOENT;
    }
    return wlan_mcast_attr_fwd_table_write_ex(mo, index, rdpa_fwd_table);
}

/* "fwd_table" attribute add callback */
static int wlan_mcast_attr_fwd_table_add(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index *index, const void *val, uint32_t size)
{
    wlan_mcast_drv_priv_t *wlan_mcast = (wlan_mcast_drv_priv_t *)bdmf_obj_data(wlan_mcast_object);
    rdpa_wlan_mcast_fwd_table_t *rdpa_fwd_table = (rdpa_wlan_mcast_fwd_table_t *)val;
    int rc;

    rc = wlan_mcast_attr_fwd_table_add_ex(mo, index, rdpa_fwd_table);
    if (rc)
        return rc;

    bdmf_fastlock_lock(&wlan_mcast_lock);
    wlan_mcast->fwd_table_entries++;
    bdmf_fastlock_unlock(&wlan_mcast_lock);

    return BDMF_ERR_OK;
}

/* "fwd_table" attribute delete callback */
static int wlan_mcast_attr_fwd_table_delete(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index)
{
    wlan_mcast_drv_priv_t *wlan_mcast = (wlan_mcast_drv_priv_t *)bdmf_obj_data(wlan_mcast_object);
    int rc;

    rc = wlan_mcast_attr_fwd_table_delete_ex(mo, index);
    if (rc)
        return rc;

    bdmf_fastlock_lock(&wlan_mcast_lock);
    wlan_mcast->fwd_table_entries--;
    bdmf_fastlock_unlock(&wlan_mcast_lock);

    return BDMF_ERR_OK;
}

/*
 * wlan_mcast dhd_station attribute access
 */
#ifdef CONFIG_DHD_RUNNER
void __dhd_station_entry_set(RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS *rdd_dhd_station,
    rdpa_wlan_mcast_dhd_station_t *rdpa_dhd_station)
{
    uint16_t *rdpa_mac_address_high = (uint16_t *)&rdpa_dhd_station->mac_address.b[0];
    uint16_t *rdpa_mac_address_mid  = (uint16_t *)&rdpa_dhd_station->mac_address.b[2];
    uint16_t *rdpa_mac_address_low  = (uint16_t *)&rdpa_dhd_station->mac_address.b[4];

    rdd_dhd_station->mac_address_high = htons(*rdpa_mac_address_high);
    rdd_dhd_station->mac_address_mid = htons(*rdpa_mac_address_mid);
    rdd_dhd_station->mac_address_low = htons(*rdpa_mac_address_low);
    rdd_dhd_station->radio_index = rdpa_dhd_station->radio_index;
    rdd_dhd_station->flowring_index = rdpa_dhd_station->flowring_index;
    rdd_dhd_station->tx_priority = rdpa_dhd_station->tx_priority;
#ifdef XRDP
    rdd_dhd_station->valid = rdpa_dhd_station->reference_count > 0 ? 1 : 0;
#else
    rdd_dhd_station->ssid = rdpa_dhd_station->ssid;
    rdd_dhd_station->reference_count = rdpa_dhd_station->reference_count;
#endif
}
#endif /* CONFIG_DHD_RUNNER */

/* "dhd_station" attribute "read" callback */
static int wlan_mcast_attr_dhd_station_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
#ifdef CONFIG_DHD_RUNNER
    rdpa_wlan_mcast_dhd_station_t *rdpa_dhd_station = (rdpa_wlan_mcast_dhd_station_t *)val;

    if (size != sizeof(rdpa_wlan_mcast_dhd_station_t))
    {
        BDMF_TRACE_ERR("%s() : Size difference <%d:%d>\n", __FUNCTION__,
            (int)size, (int)sizeof(rdpa_wlan_mcast_dhd_station_t));

        return BDMF_ERR_NOENT;
    }

    return wlan_mcast_attr_dhd_station_read_ex(mo, index, rdpa_dhd_station);
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

/* "dhd_station" attribute add callback */
static int wlan_mcast_attr_dhd_station_add(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index *index, const void *val, uint32_t size)
{
#ifdef CONFIG_DHD_RUNNER
    wlan_mcast_drv_priv_t *wlan_mcast = (wlan_mcast_drv_priv_t *)bdmf_obj_data(wlan_mcast_object);
    rdpa_wlan_mcast_dhd_station_t *rdpa_dhd_station = (rdpa_wlan_mcast_dhd_station_t *)val;
    int rc;

    rc = wlan_mcast_attr_dhd_station_add_ex(mo, index, rdpa_dhd_station);
    if (rc)
        return rc;

    bdmf_fastlock_lock(&wlan_mcast_lock);
    wlan_mcast->dhd_station_entries++;
    bdmf_fastlock_unlock(&wlan_mcast_lock);

    return BDMF_ERR_OK;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

/* "dhd_station" attribute delete callback */
static int wlan_mcast_attr_dhd_station_delete(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index)
{
#ifdef CONFIG_DHD_RUNNER
    wlan_mcast_drv_priv_t *wlan_mcast = (wlan_mcast_drv_priv_t *)bdmf_obj_data(wlan_mcast_object);
    int rc;

    rc = wlan_mcast_attr_dhd_station_delete_ex(mo, index);
    if (rc)
        return rc;

    bdmf_fastlock_lock(&wlan_mcast_lock);
    wlan_mcast->dhd_station_entries--;
    bdmf_fastlock_unlock(&wlan_mcast_lock);

    return BDMF_ERR_OK;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

/* "dhd_station" attribute find callback */
static int wlan_mcast_attr_dhd_station_find(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index *index, void *val, uint32_t size)
{
#ifdef CONFIG_DHD_RUNNER
    rdpa_wlan_mcast_dhd_station_t *rdpa_dhd_station = (rdpa_wlan_mcast_dhd_station_t *)val;

    return wlan_mcast_attr_dhd_station_find_ex(mo, index, rdpa_dhd_station);
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

#ifdef CONFIG_DHD_RUNNER
static void __ssid_mac_address_entry_get(rdpa_wlan_mcast_ssid_mac_address_t *rdpa_ssid_mac_address,
    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_DTS *rdd_ssid_mac_address, bdmf_index index)
{
    uint32_t *rdpa_mac_address_high = (uint32_t *)&rdpa_ssid_mac_address->mac_address.b[0];
    uint16_t *rdpa_mac_address_low = (uint16_t *)&rdpa_ssid_mac_address->mac_address.b[4];

    rdpa_ssid_mac_address->radio_index = RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_RADIO_INDEX(index);
    rdpa_ssid_mac_address->ssid = RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_SSID(index);

    *rdpa_mac_address_high = ntohl(rdd_ssid_mac_address->mac_address_high);
    *rdpa_mac_address_low = ntohs(rdd_ssid_mac_address->mac_address_low);
    rdpa_ssid_mac_address->reference_count = rdd_ssid_mac_address->reference_count;
}

static void __ssid_mac_address_entry_set(RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_DTS *rdd_ssid_mac_address,
    rdpa_wlan_mcast_ssid_mac_address_t *rdpa_ssid_mac_address)
{
    uint32_t *rdpa_mac_address_high = (uint32_t *)&rdpa_ssid_mac_address->mac_address.b[0];
    uint16_t *rdpa_mac_address_low = (uint16_t *)&rdpa_ssid_mac_address->mac_address.b[4];

    rdd_ssid_mac_address->mac_address_high = htonl(*rdpa_mac_address_high);
    rdd_ssid_mac_address->mac_address_low = htons(*rdpa_mac_address_low);
}
#endif /* CONFIG_DHD_RUNNER */

/* "ssid_mac_address" attribute "read" callback */
static int wlan_mcast_attr_ssid_mac_address_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
#ifdef CONFIG_DHD_RUNNER
    rdpa_wlan_mcast_ssid_mac_address_t *rdpa_ssid_mac_address = (rdpa_wlan_mcast_ssid_mac_address_t *)val;
    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_DTS rdd_ssid_mac_address;
    int rc;

    if (size != sizeof(rdpa_wlan_mcast_ssid_mac_address_t))
    {
        BDMF_TRACE_ERR("%s() : Size difference <%d:%d>\n", __FUNCTION__,
            (int)size, (int)sizeof(rdpa_wlan_mcast_ssid_mac_address_t));

        return BDMF_ERR_NOENT;
    }

    /* read the flow data from the RDD */
    rc = rdd_wlan_mcast_ssid_mac_address_read(index, &rdd_ssid_mac_address);
    if (rc)
        return rc;

    /* Data fill the RDPA structure */
    __ssid_mac_address_entry_get(rdpa_ssid_mac_address, &rdd_ssid_mac_address, index);
    return 0;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

/* "ssid_mac_address" attribute add callback */
static int wlan_mcast_attr_ssid_mac_address_add(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index *index, const void *val, uint32_t size)
{
#ifdef CONFIG_DHD_RUNNER
    wlan_mcast_drv_priv_t *wlan_mcast = (wlan_mcast_drv_priv_t *)bdmf_obj_data(wlan_mcast_object);
    rdpa_wlan_mcast_ssid_mac_address_t *rdpa_ssid_mac_address = (rdpa_wlan_mcast_ssid_mac_address_t *)val;
    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_DTS rdd_ssid_mac_address;
    int rc;

    /* Data fill the RDD structure */
    __ssid_mac_address_entry_set(&rdd_ssid_mac_address, rdpa_ssid_mac_address);

    rc = rdd_wlan_mcast_ssid_mac_address_add(rdpa_ssid_mac_address->radio_index,
        rdpa_ssid_mac_address->ssid, &rdd_ssid_mac_address, (uint32_t *)index);
    if (rc)
        return rc;

    bdmf_fastlock_lock(&wlan_mcast_lock);
    wlan_mcast->ssid_mac_address_entries++;
    bdmf_fastlock_unlock(&wlan_mcast_lock);
    return 0;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

/* "ssid_mac_address" attribute delete callback */
static int wlan_mcast_attr_ssid_mac_address_delete(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index)
{
#ifdef CONFIG_DHD_RUNNER
    wlan_mcast_drv_priv_t *wlan_mcast = (wlan_mcast_drv_priv_t *)bdmf_obj_data(wlan_mcast_object);
    int rc;

    rc = rdd_wlan_mcast_ssid_mac_address_delete(index);
    if (rc)
        return rc;

    bdmf_fastlock_lock(&wlan_mcast_lock);
    wlan_mcast->ssid_mac_address_entries--;
    bdmf_fastlock_unlock(&wlan_mcast_lock);
    return 0;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

/*
 * wlan_mcast ssid_stats attribute access
 */

/* "ssid_stats" attribute "read" callback */
static int wlan_mcast_attr_ssid_stats_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
#ifdef CONFIG_DHD_RUNNER
    RDD_WLAN_MCAST_SSID_STATS_ENTRY_DTS rdd_ssid_stats;
    int rc;
    rdpa_wlan_mcast_ssid_stats_t *rdpa_ssid_stats = (rdpa_wlan_mcast_ssid_stats_t *)val;

    /* read the ip flow result from the RDD */
    rc = rdd_wlan_mcast_ssid_stats_read(index, &rdd_ssid_stats);
    if (rc)
        return rc;

    rdpa_ssid_stats->radio_index = RDD_WLAN_MCAST_SSID_STATS_ENTRY_RADIO_INDEX(index);
    rdpa_ssid_stats->ssid = RDD_WLAN_MCAST_SSID_STATS_ENTRY_SSID(index);
    rdpa_ssid_stats->packets = rdd_ssid_stats.packets;
    rdpa_ssid_stats->bytes = rdd_ssid_stats.bytes;

    return BDMF_ERR_OK;
#else
    return BDMF_ERR_NOENT;
#endif
}

/*
 * wlan_mcast object definition
 */

struct bdmf_aggr_type wlan_mcast_fwd_table_type = {
    .name = "fwd_table_entry", .struct_name = "rdpa_wlan_mcast_fwd_table_t",
    .help = "WLAN Multicast Forwarding Table Entry",
    .size = sizeof(rdpa_wlan_mcast_fwd_table_t),
    .fields = (struct bdmf_attr[]) {
        { .name = "is_proxy_enabled", .help = "Proxy Enabled Flag", .type = bdmf_attr_number,
          .size = sizeof(uint8_t), .offset = offsetof(rdpa_wlan_mcast_fwd_table_t, is_proxy_enabled),
          .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "wfd_tx_priority", .help = "WFD Tx Priority", .type = bdmf_attr_number,
          .size = sizeof(uint8_t), .offset = offsetof(rdpa_wlan_mcast_fwd_table_t, wfd_tx_priority),
          .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "wfd_0_priority", .help = "WFD 0 Queue Priority", .type = bdmf_attr_number,
          .size = sizeof(uint8_t), .offset = offsetof(rdpa_wlan_mcast_fwd_table_t, wfd_0_priority),
          .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "wfd_1_priority", .help = "WFD 1 Queue Priority", .type = bdmf_attr_number,
          .size = sizeof(uint8_t), .offset = offsetof(rdpa_wlan_mcast_fwd_table_t, wfd_1_priority),
          .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "wfd_2_priority", .help = "WFD 2 Queue Priority", .type = bdmf_attr_number,
          .size = sizeof(uint8_t), .offset = offsetof(rdpa_wlan_mcast_fwd_table_t, wfd_2_priority),
          .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "wfd_0_ssid_vector", .help = "WFD 0 SSID Forwarding Vector",
            .type = bdmf_attr_enum_mask,
            .ts.enum_table = &rdpa_wlan_mcast_wfd0_enum_table,
            .size = sizeof(uint16_t),
            .offset = offsetof(rdpa_wlan_mcast_fwd_table_t, wfd_0_ssid_vector)
        },
        { .name = "wfd_1_ssid_vector", .help = "WFD 1 SSID Forwarding Vector",
            .type = bdmf_attr_enum_mask,
            .ts.enum_table = &rdpa_wlan_mcast_wfd1_enum_table,
            .size = sizeof(uint16_t),
            .offset = offsetof(rdpa_wlan_mcast_fwd_table_t, wfd_1_ssid_vector)
        },
        { .name = "wfd_2_ssid_vector", .help = "WFD 2 SSID Forwarding Vector",
            .type = bdmf_attr_enum_mask,
            .ts.enum_table = &rdpa_wlan_mcast_wfd2_enum_table,
            .size = sizeof(uint16_t),
            .offset = offsetof(rdpa_wlan_mcast_fwd_table_t, wfd_2_ssid_vector)
        },
        { .name = "dhd_station_index", .help = "New DHD Station Index", .type = bdmf_attr_number,
          .size = sizeof(bdmf_index), .offset = offsetof(rdpa_wlan_mcast_fwd_table_t, dhd_station_index),
          .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_HIDDEN
        },
        { .name = "dhd_station_count", .help = "DHD Station Count", .type = bdmf_attr_number,
          .size = sizeof(uint8_t), .offset = offsetof(rdpa_wlan_mcast_fwd_table_t, dhd_station_count),
          .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_READ
        },
        { .name = "dhd_station_list_size", .help = "DHD Station List Size", .type = bdmf_attr_number,
          .size = sizeof(uint8_t), .offset = offsetof(rdpa_wlan_mcast_fwd_table_t, dhd_station_list_size),
          .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_READ
        },
        { .name = "dhd_station_list", .help = "DHD Station List", .size = RDPA_WLAN_MCAST_MAX_DHD_STATIONS,
          .type = bdmf_attr_buffer, .offset = offsetof(rdpa_wlan_mcast_fwd_table_t, dhd_station_list),
          .flags = BDMF_ATTR_READ,
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(wlan_mcast_fwd_table_type);

struct bdmf_aggr_type wlan_mcast_dhd_station_type = {
    .name = "dhd_station_entry", .struct_name = "rdpa_wlan_mcast_dhd_station_t",
    .help = "WLAN Multicast Forwarding Table Entry",
    .size = sizeof(rdpa_wlan_mcast_dhd_station_t),
    .fields = (struct bdmf_attr[]) {
        { .name = "mac_address", .help = "DHD Station MAC address", .type = bdmf_attr_ether_addr,
          .size = sizeof(bdmf_mac_t), .offset = offsetof(rdpa_wlan_mcast_dhd_station_t, mac_address)
        },
        { .name = "radio_index", .help = "DHD Station Radio Index", .type = bdmf_attr_number,
          .size = sizeof(uint8_t), .offset = offsetof(rdpa_wlan_mcast_dhd_station_t, radio_index),
          .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "ssid", .help = "DHD Station SSID", .type = bdmf_attr_number,
          .size = sizeof(uint8_t), .offset = offsetof(rdpa_wlan_mcast_dhd_station_t, ssid),
          .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "flowring_index", .help = "DHD Station FlowRing Index", .type = bdmf_attr_number,
          .size = sizeof(uint16_t), .offset = offsetof(rdpa_wlan_mcast_dhd_station_t, flowring_index),
          .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "tx_priority", .help = "DHD Station Tx Priority", .type = bdmf_attr_number,
          .size = sizeof(uint8_t), .offset = offsetof(rdpa_wlan_mcast_dhd_station_t, tx_priority),
          .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "reference_count", .help = "References to this DHD Station", .type = bdmf_attr_number,
          .size = sizeof(uint8_t), .offset = offsetof(rdpa_wlan_mcast_dhd_station_t, reference_count),
          .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_READ
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(wlan_mcast_dhd_station_type);

struct bdmf_aggr_type wlan_mcast_ssid_mac_address_type = {
    .name = "ssid_mac_address_entry", .struct_name = "rdpa_wlan_mcast_ssid_mac_address_t",
    .help = "SSID MAC Address Table Entry",
    .size = sizeof(rdpa_wlan_mcast_ssid_mac_address_t),
    .fields = (struct bdmf_attr[]) {
        { .name = "radio_index", .help = "Radio Index", .type = bdmf_attr_number,
          .size = sizeof(uint8_t), .offset = offsetof(rdpa_wlan_mcast_ssid_mac_address_t, radio_index),
          .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "ssid", .help = "SSID", .type = bdmf_attr_number,
          .size = sizeof(uint8_t), .offset = offsetof(rdpa_wlan_mcast_ssid_mac_address_t, ssid),
          .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "mac_address", .help = "SSID MAC address", .type = bdmf_attr_ether_addr,
          .size = sizeof(bdmf_mac_t), .offset = offsetof(rdpa_wlan_mcast_ssid_mac_address_t, mac_address)
        },
        { .name = "reference_count", .help = "References to this DHD Station", .type = bdmf_attr_number,
          .size = sizeof(uint8_t), .offset = offsetof(rdpa_wlan_mcast_ssid_mac_address_t, reference_count),
          .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_READ
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(wlan_mcast_ssid_mac_address_type);

struct bdmf_aggr_type wlan_mcast_ssid_stats_type = {
    .name = "ssid_stats_entry", .struct_name = "rdpa_wlan_mcast_ssid_stats_t",
    .help = "SSID MAC Address Table Entry",
    .size = sizeof(rdpa_wlan_mcast_ssid_stats_t),
    .fields = (struct bdmf_attr[]) {
        { .name = "radio_index", .help = "Radio Index", .type = bdmf_attr_number,
          .size = sizeof(uint8_t), .offset = offsetof(rdpa_wlan_mcast_ssid_stats_t, radio_index),
          .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "ssid", .help = "SSID", .type = bdmf_attr_number,
          .size = sizeof(uint8_t), .offset = offsetof(rdpa_wlan_mcast_ssid_stats_t, ssid),
          .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "packets", .help = "Cumulative Packet Count", .type = bdmf_attr_number,
          .size = sizeof(uint32_t), .offset = offsetof(rdpa_wlan_mcast_ssid_stats_t, packets)
        },
        { .name = "bytes", .help = "Cumulative Byte Count", .type = bdmf_attr_number,
          .size = sizeof(uint32_t), .offset = offsetof(rdpa_wlan_mcast_ssid_stats_t, bytes)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(wlan_mcast_ssid_stats_type);

/* Object attribute descriptors */
static struct bdmf_attr wlan_mcast_attrs[] = {
    { .name = "fwd_table_entries", .help = "Number of configured Forwarding Table entries",
      .type = bdmf_attr_number, .flags = BDMF_ATTR_READ | BDMF_ATTR_CONFIG,
      .size = sizeof(uint32_t), .offset = offsetof(wlan_mcast_drv_priv_t, fwd_table_entries)
    },
    { .name = "fwd_table", .help = "Forwarding Table",
      .type = bdmf_attr_aggregate, .ts.aggr_type_name = "fwd_table_entry",
      .array_size = RDPA_WLAN_MCAST_MAX_FLOWS,
      .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NOLOCK,
      .read = wlan_mcast_attr_fwd_table_read, .write = wlan_mcast_attr_fwd_table_write,
      .add = wlan_mcast_attr_fwd_table_add, .del = wlan_mcast_attr_fwd_table_delete,
    },
    { .name = "dhd_station_entries", .help = "Number of configured DHD Stations",
      .type = bdmf_attr_number, .flags = BDMF_ATTR_READ | BDMF_ATTR_CONFIG,
      .size = sizeof(uint32_t), .offset = offsetof(wlan_mcast_drv_priv_t, dhd_station_entries)
    },
    { .name = "dhd_station", .help = "DHD Station Table",
      .type = bdmf_attr_aggregate, .ts.aggr_type_name = "dhd_station_entry",
      .array_size = RDPA_WLAN_MCAST_MAX_DHD_STATIONS,
      .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NOLOCK,
      .read = wlan_mcast_attr_dhd_station_read,
      .add = wlan_mcast_attr_dhd_station_add, .del = wlan_mcast_attr_dhd_station_delete,
      .find = wlan_mcast_attr_dhd_station_find
    },
    { .name = "ssid_mac_address_entries", .help = "Number of configured SSID MAC Addresses",
      .type = bdmf_attr_number, .flags = BDMF_ATTR_READ | BDMF_ATTR_CONFIG,
      .size = sizeof(uint32_t), .offset = offsetof(wlan_mcast_drv_priv_t, ssid_mac_address_entries)
    },
    { .name = "ssid_mac_address", .help = "SSID MAC Address Table",
      .type = bdmf_attr_aggregate, .ts.aggr_type_name = "ssid_mac_address_entry",
      .array_size = RDPA_WLAN_MCAST_MAX_SSID_MAC_ADDRESSES,
      .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NOLOCK,
      .read = wlan_mcast_attr_ssid_mac_address_read,
      .add = wlan_mcast_attr_ssid_mac_address_add, .del = wlan_mcast_attr_ssid_mac_address_delete
    },
    { .name = "ssid_stats", .help = "SSID Statistics",
      .type = bdmf_attr_aggregate, .ts.aggr_type_name = "ssid_stats_entry",
      .array_size = RDPA_WLAN_MCAST_MAX_SSID_STATS,
      .flags = BDMF_ATTR_READ | BDMF_ATTR_NOLOCK,
      .read = wlan_mcast_attr_ssid_stats_read
    },
    BDMF_ATTR_LAST
};


static int wlan_mcast_drv_init(struct bdmf_type *drv);
static void wlan_mcast_drv_exit(struct bdmf_type *drv);

struct bdmf_type wlan_mcast_drv = {
    .name = "wlan_mcast",
    .parent = "system",
    .description = "WLAN Multicast Table Manager",
    .drv_init = wlan_mcast_drv_init,
    .drv_exit = wlan_mcast_drv_exit,
    .post_init = wlan_mcast_post_init,
    .destroy = wlan_mcast_destroy,
    .get = wlan_mcast_get,
    .extra_size = sizeof(wlan_mcast_drv_priv_t),
    .aattr = wlan_mcast_attrs,
    .max_objs = 1,
};
DECLARE_BDMF_TYPE(rdpa_wlan_mcast, wlan_mcast_drv);

/* Init module. Cater for GPL layer */
static int wlan_mcast_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_wlan_mcast_drv = rdpa_wlan_mcast_drv;
    f_rdpa_wlan_mcast_get = rdpa_wlan_mcast_get;
#endif
    return 0;
}

/* Exit module. Cater for GPL layer */
static void wlan_mcast_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_wlan_mcast_drv = NULL;
    f_rdpa_wlan_mcast_get = NULL;
#endif
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/

/** Get mcast object by key
 * \return  Object handle or NULL if not found
 */
int rdpa_wlan_mcast_get(bdmf_object_handle *_obj_)
{
    if (!wlan_mcast_object || wlan_mcast_object->state == bdmf_state_deleted)
        return BDMF_ERR_NOENT;

    bdmf_get(wlan_mcast_object);
    *_obj_ = wlan_mcast_object;

    return 0;
}

