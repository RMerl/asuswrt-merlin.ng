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

#ifndef _RDPA_BRIDGE_EX_H_
#define _RDPA_BRIDGE_EX_H_

#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_common.h"
#include "rdpa_int.h"
#include "rdpa_port_int.h"
#include "rdpa_vlan_ex.h"

#define NO_BRIDGE_ID (-1)

#ifdef XRDP
struct bridge_occupancy_entry
{
    DLIST_ENTRY(bridge_occupancy_entry) list;
    rdpa_fdb_key_t key;
};
typedef struct bridge_occupancy_entry bridge_occupancy_entry_t;
DLIST_HEAD(bridge_occupancy_list_t, bridge_occupancy_entry);
#endif

/* Bridge object private data */
typedef struct {
    bdmf_index index;
    rdpa_bridge_cfg_t cfg;
    rdpa_bridge_fdb_limit_t fdb_limit;

    /* Enable/Disable local switching on the bridge, 
     * when disabled traffic from lan to lan interface will be dropped */
    bdmf_boolean local_switch_enable;

    /* number of times port is "referenced".
     * The same port can be referenced multiple times by linking it
     * directly to bridge and indirectly via VLANs */
    int port_refs[rdpa_if__number_of];

    /* Forward eligibility matrix.
     * For each port in contains bitmask of destination ports to which
     * tx is eligible
     */
    rdpa_ports port_fw_elig[rdpa_if__number_of];
#ifndef XRDP

    /* SSID mask - all SSIDs included in bridget */
    uint16_t ssid_mask[rdpa_if__number_of];
    rdd_bridge_port_t port_flooding_vector;
#else
    struct bridge_occupancy_list_t bridge_occupancy_list; /**<Bridge Occupancy list */
#endif
    bdmf_mac_t lan_mac;    
    uint16_t port_flooding_wifi_vector;
} bridge_drv_priv_t;

/***************************************************************************
 * bridge object type
 **************************************************************************/

int bridge_post_init_ex(struct bdmf_object *mo);
void bridge_destroy_ex(struct bdmf_object *mo);
int bridge_mac_read_ex(struct bdmf_object *mo, rdpa_fdb_key_t *key, rdpa_fdb_data_t *data);
int bridge_mac_check_ex(struct bdmf_object *mo, rdpa_fdb_key_t *key, rdpa_fdb_data_t *data, int *is_found);
int bridge_mac_delete_ex(struct bdmf_object *mo, rdpa_fdb_key_t *key);
int bridge_mac_add_modify_ex(struct bdmf_object *mo, rdpa_fdb_key_t *key,
    const rdpa_fdb_data_t *data);
int bridge_attr_mac_get_next_ex(struct bdmf_object *mo, struct bdmf_attr *ad, 
    bdmf_index *index);
int bridge_attr_mac_status_read_ex(struct bdmf_object *mo, rdpa_fdb_key_t *key,
    bdmf_boolean *status);
int bridge_attr_fw_eligible_write_ex(struct bdmf_object *mo,
    bdmf_index index, rdpa_ports fw_elig);
int bridge_attr_lan_mac_write_ex(struct bdmf_object *mo, bdmf_mac_t *mac);
int bridge_link_port_ex(bridge_drv_priv_t *bridge, bdmf_number port_index);
void bridge_unlink_port_ex(bridge_drv_priv_t *bridge, bdmf_number port_index);
int bridge_drv_init_ex(void);

int bridge_set_fw_elig_ex(struct bdmf_object *mo, rdpa_if port, rdpa_ports new_mask, struct bdmf_object *vlan_obj);
rdpa_ports bridge_get_global_fw_elig(const bridge_drv_priv_t *bridge, rdpa_if port);

static inline int mac_is_mcast(const bdmf_mac_t *mac)
{
    return (mac->b[0] & 0x1) != 0;
}

/* wan port is considered valid if it is wan0 or
 * in valid range. The check is done this way because
 * currently num_wan in system object is FFU and can be 0.
 */
static inline int bridge_is_wan_valid(rdpa_if port)
{
    if (!rdpa_if_is_wan(port))
        return 0;
    if (port != rdpa_if_wan0 &&
        port >= rdpa_if_wan0 + _rdpa_system_num_wan_get()) /* FIXME: MULTI-WAN XPON */
    {
        return 0;
    }
    return 1;
}

static inline rdpa_if bridge_vlan_to_if(struct bdmf_object *vlan)
{
    struct bdmf_object *parent = vlan->owner;
    rdpa_if port_index = rdpa_if_none;

    while (parent && parent->drv != rdpa_port_drv())
        parent = parent->owner;
    if (!parent)
    {
        BDMF_TRACE_ERR_OBJ(vlan,
            "Internal error. No port in vlan's parent hierarchy\n");
        return rdpa_if_none;
    }
    rdpa_port_index_get(parent, &port_index);

    return port_index;
}

#endif
