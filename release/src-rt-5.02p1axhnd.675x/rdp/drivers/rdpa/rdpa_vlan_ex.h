/*
* <:copyright-BRCM:2012-2015:proprietary:standard
* 
*    Copyright (c) 2012-2015 Broadcom 
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
 * rdpa_vlan_ex.h
 *
 */

#ifndef _RDPA_VLAN_EX_H_
#define _RDPA_VLAN_EX_H_

#include <bdmf_dev.h>
#include <rdpa_api.h>
#include "rdpa_common.h"
#include "rdd.h"
#include "rdpa_int.h"

/***************************************************************************
 * vlan object type
 **************************************************************************/

typedef struct
{
    int16_t vid;
} rdpa_vlan_vid_cfg_t;

/* VLAN object private data */
typedef struct {
    char name[BDMF_OBJ_NAME_LEN];     /**< Object name */
    rdpa_vlan_vid_cfg_t vids[RDPA_MAX_VLANS];
    int common_init_completed;
    int is_linking;
    bdmf_object_handle linked_bridge;
    rdpa_filter_ctrl_t ingress_filters[RDPA_FILTERS_QUANT]; /** Ingress filters per vlan object */
    rdpa_mac_lookup_cfg_t mac_lkp_cfg;
    uint32_t proto_filters;          /**< Map of rdpa_proto_filtres_t */
    rdpa_discard_prty discard_prty;  /**< VLAN Ingress QoS priority */
    uint8_t ingress_filters_profile;
    uint8_t counter_idx;
    bdmf_boolean counter_id_valid;
    uint32_t options;                /**< Reserved flags \XRDP_LIMITED */
    bdmf_boolean is_default;     /**< Default VLAN VID flag, one per port is allowed \XRDP_LIMITED */
} vlan_drv_priv_t;

int vlan_vid_table_update_ex(struct bdmf_object *mo, int16_t vid, int is_add);
int vlan_lan_to_wan_link_ex(struct bdmf_object *mo, struct bdmf_object *other);
void vlan_lan_to_wan_unlink_ex(struct bdmf_object *mo, struct bdmf_object *other);

int vlan_update_aggr_all_vids(struct bdmf_object *lan_obj, struct bdmf_object *wan_obj, int is_add);
int vlan_update_aggr_all_links(struct bdmf_object *mo, int16_t lan_vid, int is_add);

int vlan_wan_aggr_add_ex(struct bdmf_object *lan_obj, struct bdmf_object *wan_obj, int16_t lan_vid);
void vlan_wan_aggr_del_ex(struct bdmf_object *lan_obj, struct bdmf_object *wan_obj, int16_t lan_vid);

static inline int vlan_get_next_vid_cfg(struct bdmf_object *mo, int *prev_idx)
{
    vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(mo);
    int i;

    for (i = *prev_idx + 1; i < RDPA_MAX_VLANS; i++)
    {
        if (vlan->vids[i].vid > 0)
        {
            *prev_idx = i;
            return vlan->vids[i].vid;
        }
    }
    return -1;
}

int vlan_attr_ingress_filter_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);
int vlan_attr_mac_lkp_cfg_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);
int vlan_attr_proto_filters_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);
int vlan_attr_stat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size);
int vlan_attr_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size);

int vlan_update_bridge_id_ex(struct bdmf_object *vlan_obj, bdmf_index *bridge_id);

int vlan_attr_options_write_ex(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size);

#if defined(XRDP) && defined(CONFIG_RNR_BRIDGE)
int vlan_is_any_vid_enabled(vlan_drv_priv_t *vlan);
int rdpa_vlan_hash_entry_read(rdpa_if port, int16_t vid, RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_and_vlan_ctx_lkp_result);
int rdpa_vlan_hash_entry_write(rdpa_if port, int16_t vid, RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_and_vlan_ctx_lkp_result);
int rdpa_vlan_hash_entry_delete(rdpa_if port, int16_t vid);

typedef int (*rdpa_vlan_hash_entry_modify_cb_t)(RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_and_vlan_ctx_lkp_result,
    void *modify_ctx);
int rdpa_vlan_hash_entry_modify(rdpa_if port, int16_t vid, rdpa_vlan_hash_entry_modify_cb_t modify_cb,
    void *modify_ctx);
int vlan_ctx_update_invoke(struct bdmf_object *vlan_obj, rdpa_vlan_hash_entry_modify_cb_t modify_cb, void *modify_ctx);

void rdpa_vlan_hash_entry_isolation_vector_set(rdd_vport_vector_t vport_mask,
    RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_and_vlan_ctx_lkp_result);

int rdpa_vlan_mac_lkp_cfg_modify_cb(RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_and_vlan_ctx_lkp_result,
    void *modify_ctx);
int rdpa_vlan_proto_filters_modify_cb(RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_and_vlan_ctx_lkp_result,
    void *modify_ctx);
int rdpa_vlan_ingress_filter_profile_modify_cb(RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_and_vlan_ctx_lkp_result,
    void *modify_ctx);
int rdpa_vlan_isolation_vector_modify_cb(RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_and_vlan_ctx_lkp_result,
    void *modify_ctx);
int rdpa_vlan_wan_aggregation_modify_cb(RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_and_vlan_ctx_lkp_result,
    void *modify_ctx);
int rdpa_vlan_bridge_id_modify_cb(RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS *bridge_and_vlan_ctx_lkp_result,
    void *modify_ctx);
#endif

#ifndef XRDP
/* Get LAN VID entry. Returns entry >=0 or -1 if no entry or if vlan_aware_switching mode and entry is not aggregated */
int _rdpa_vlan_vid2entry(int16_t vid);
/* Get VID by rdd_entry */
int _rdpa_vlan_entry2vid(int vid_entry);
#endif

int vlan_pre_init_ex(struct bdmf_object *mo);

int vlan_post_init_ex(struct bdmf_object *mo);

int vlan_get_vid_cfg_idx(vlan_drv_priv_t *vlan, uint16_t vid);

int vlan_update_high_priority_vids(int16_t vid, bdmf_boolean enable);

void vlan_drv_init_ex(void);

void vlan_remove_default_vid_ex(struct bdmf_object *mo);

#endif
