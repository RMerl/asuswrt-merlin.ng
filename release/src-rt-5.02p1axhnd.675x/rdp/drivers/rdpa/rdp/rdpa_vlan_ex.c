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
 * rdpa_vlan_ex.c
 *
 *  Created on: oct 26, 2016
 *      Author: danielc
 */

#include "rdpa_vlan_ex.h"

/* VID references (isolation) */
typedef struct
{
    rdpa_ports ports;   /* Eligible ports. 0=VID is unreferenced */
    int16_t vid;        /* vid */
    int wan_entry;      /* entry in vlan_wan_aggr_vids[] array for aggregation */
    int wan_ref_count;  /* Number of times this LAN VID refers to WAN VID */
    uint32_t rdd_entry; /* RDD table entry */

    int ref_count;      /* Reference counter, specifying the number     */
                        /* of times the RDD LAN VID entry is referenced */
                        /* by either VLAN or MAC tables.                */

    int ref_count_vlan; /* Reference counter, specifying the number     */
                        /* of times the RDD LAN VID entry is referenced */
                        /* by VLAN table.                               */
} vlan_vid_ref_t;

/* WAN VID aggregation reference structure */
typedef struct
{
    int16_t vid;            /* WAN VID */
    int ref_count;      /* WAN VID reference count */
} vlan_wan_aggr_ref_t;

vlan_vid_ref_t vlan_vid_refs_lan[RDPA_MAX_VLANS];
vlan_vid_ref_t vlan_vid_refs_wan[RDPA_MAX_VLANS];
vlan_wan_aggr_ref_t vlan_wan_aggr_vids[RDPA_MAX_AGGR_WAN_VLANS];
extern bdmf_fastlock vlan_vid_refs_fastlock;

static void vlan_handle_vid_entry_del(vlan_vid_ref_t *ref, bdmf_boolean is_lan);

/*
 * VLAN isolation, VLAN aggregation helpers
 */

static int vlan_wan_aggr_add_to_rdd(struct bdmf_object *lan_obj, vlan_vid_ref_t *vid_ref,
    vlan_wan_aggr_ref_t *wan_ref)
{
    rdd_lan_vid_cfg_t cfg = {
        .vid = vid_ref->vid,
#ifdef LEGACY_RDP
        .isolation_mode_port_vector = rdpa_ports2rdd_bridge_port_vector(vid_ref->ports & RDPA_PORT_ALL_LAN),
        .aggregation_mode_port_vector = rdpa_ports2rdd_bridge_port_vector(vid_ref->ports & RDPA_PORT_ALL_LAN),
#else
        .isolation_mode_port_vector = rdpa_ports2rdd_vport_vector(vid_ref->ports & RDPA_PORT_ALL_LAN),
        .aggregation_mode_port_vector = rdpa_ports2rdd_vport_vector(vid_ref->ports & RDPA_PORT_ALL_LAN),
#endif
        .aggregation_vid_index = wan_ref - vlan_wan_aggr_vids
    };
    int rdd_rc = rdd_lan_vid_entry_cfg(vid_ref->rdd_entry, &cfg);

    BDMF_TRACE_DBG_OBJ(lan_obj, "rdd_lan_vid_entry_cfg(%d, {vid=%d ag=0x%x ag_vid=%d ports=0x%x}) --> %d\n",
        (int)vid_ref->rdd_entry, (int)cfg.vid, (int)cfg.aggregation_mode_port_vector,
        (int)cfg.aggregation_vid_index, (unsigned)cfg.isolation_mode_port_vector, rdd_rc);
    if (rdd_rc)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, lan_obj, "rdd_lan_vid_entry_cfg() failed. rdd_rc=%d\n", rdd_rc);
    vid_ref->wan_entry = wan_ref - vlan_wan_aggr_vids;

    /* Configure WAN vid if necessary */
    if (!wan_ref->ref_count)
    {
        rdd_wan_vid_cfg(vid_ref->wan_entry, wan_ref->vid);
        BDMF_TRACE_DBG_OBJ(lan_obj, "rdd_wan_vid_config(%d, %d)\n", vid_ref->wan_entry, wan_ref->vid);
    }
    return 0;
}

static void vlan_wan_aggr_delete_to_rdd(struct bdmf_object *lan_obj, vlan_vid_ref_t *vid_ref, 
    vlan_wan_aggr_ref_t *wan_ref)
{
#ifdef BRIDGE_AGGR
    /* Remove MAC references */
    _rdpa_bridge_disable_lan_vid_aggregation(vid_ref->rdd_entry);
#endif
    /* Reconfigure VLAN entry */
    {
        rdd_lan_vid_cfg_t cfg = {
            .vid = vid_ref->vid,
#ifdef LEGACY_RDP
            .isolation_mode_port_vector = rdpa_ports2rdd_bridge_port_vector(vid_ref->ports & RDPA_PORT_ALL_LAN),
#else
            .isolation_mode_port_vector = rdpa_ports2rdd_vport_vector(vid_ref->ports & RDPA_PORT_ALL_LAN),
#endif
            .aggregation_mode_port_vector = 0,
            .aggregation_vid_index = RDPA_VLAN_AGGR_ENTRY_DONT_CARE
        };
        int rdd_rc;

        rdd_rc = rdd_lan_vid_entry_cfg(vid_ref->rdd_entry, &cfg);
        BDMF_TRACE_DBG_OBJ(lan_obj, "rdd_lan_vid_entry_cfg(%d, {vid=%d ag=0x%x ag_vid=%d ports=0x%x}) --> %d\n",
            (int)vid_ref->rdd_entry, (int)cfg.vid, (int)cfg.aggregation_mode_port_vector,
            (int)cfg.aggregation_vid_index, (unsigned)cfg.isolation_mode_port_vector, rdd_rc);
    }
}

/* Find global VID reference by VID */
vlan_vid_ref_t *vlan_vid_ref_find(const struct bdmf_object *mo, int16_t vid, bdmf_boolean allocate)
{
    vlan_vid_ref_t *ref = NULL;
    rdpa_if port;
    vlan_vid_ref_t *vlan_vid_refs = vlan_vid_refs_lan;
    int i;
 
    /* Locate reference array */
    if (mo) /* VID port: LAN or WAN */
    {
        rdpa_port_index_get(mo->owner, &port);
        if (rdpa_if_is_wan(port))
            vlan_vid_refs = vlan_vid_refs_wan;
    }

    /* Locate reference */
    if (!allocate)
    {
        for (i = 0; (i < RDPA_MAX_VLANS); ++i)
        {
            if (vlan_vid_refs[i].vid == vid)
            {
                ref = &vlan_vid_refs[i];
                break;
            }
        }	
    }
    else
    {
        for (i = 0; (i < RDPA_MAX_VLANS) && vlan_vid_refs[i].ref_count; ++i)
        {
            if (vlan_vid_refs[i].vid == vid)
            {
                ref = &vlan_vid_refs[i];
                break;
            }
        }	
    }

    if ((!ref) && allocate)
    {
        if (i >= RDPA_MAX_VLANS) /* Full */
            return NULL;

        ref = &vlan_vid_refs[i];
        ref->vid = vid;
    }

    return ref;
}


/* Update global VID table.
 * The function when VID is added/deleted to/from VLAN container.
 * It takes care of VLAN isolation and aggregation
 */
int vlan_vid_table_update_ex(struct bdmf_object *mo, int16_t vid, int is_add)
{
    rdpa_ports new_mask;
    const rdpa_system_init_cfg_t *syscfg = _rdpa_system_init_cfg_get();
    vlan_vid_ref_t *ref; 
    rdpa_if port;
    int rc;
    int rdd_rc = 0;
    bdmf_boolean is_ref_valid = 1;
     
    ref = vlan_vid_ref_find(mo, vid, is_add); /* VID port: LAN or WAN */
    if (!ref)
    {
        BDMF_TRACE_ERR_OBJ(mo, "VID %d is not found\n", vid);
        return BDMF_ERR_INTERNAL;
    }

    new_mask = ref->ports;
    /* Index of port that owns VLAN container */
    rdpa_port_index_get(mo->owner, &port);
    if (is_add)
        new_mask |= rdpa_if_id(port);
    else
        new_mask &= ~rdpa_if_id(port);

    /* Update VLAN isolation table in RDD if necessary */
    if (rdpa_if_is_lan(port) && syscfg->switching_mode != rdpa_switching_none)
    {
        rdd_lan_vid_cfg_t cfg = {
            .vid = ref->vid,
#ifdef LEGACY_RDP
            .isolation_mode_port_vector = rdpa_ports2rdd_bridge_port_vector(new_mask & RDPA_PORT_ALL_LAN),
#else
            .isolation_mode_port_vector = rdpa_ports2rdd_vport_vector(new_mask & RDPA_PORT_ALL_LAN),
#endif
            .aggregation_mode_port_vector = 0,
            .aggregation_vid_index = RDPA_VLAN_AGGR_ENTRY_DONT_CARE
        };

        if (syscfg->switching_mode == rdpa_vlan_aware_switching)
        {
            if (ref->wan_ref_count)
            {
                cfg.aggregation_mode_port_vector =
#ifdef LEGACY_RDP
                    rdpa_ports2rdd_bridge_port_vector(ref->ports & RDPA_PORT_ALL_LAN),
#else
                    rdpa_ports2rdd_vport_vector(ref->ports & RDPA_PORT_ALL_LAN),
#endif
                cfg.aggregation_vid_index = ref->wan_entry;
            }
        }
        else if (syscfg->switching_mode == rdpa_mac_based_switching)
        {
            cfg.aggregation_mode_port_vector = cfg.isolation_mode_port_vector;
        }
        else
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "Can't handle switching mode %d\n",
                (int)syscfg->switching_mode);
        }

        if (is_add) /* Add */
        {
            if (ref->ref_count == 0) /* First */
            {
                rdd_rc = rdd_lan_vid_entry_add(&cfg, &ref->rdd_entry);
                BDMF_TRACE_DBG_OBJ(mo, "rdd_lan_vid_entry_add({vid=%d ag=0x%x ag_vid=%d ports=0x%x}, %d) --> %d\n",
                    (int)cfg.vid, (int)cfg.aggregation_mode_port_vector,
                    (int)cfg.aggregation_vid_index, (unsigned)cfg.isolation_mode_port_vector,
                    (int)ref->rdd_entry, rdd_rc);
            }
            else
            {
                rdd_rc = rdd_lan_vid_entry_cfg(ref->rdd_entry, &cfg);
                BDMF_TRACE_DBG_OBJ(mo, "rdd_lan_vid_entry_cfg(%d, {vid=%d ag=%d ag_vid=%d ports=0x%x}) --> %d\n",
                    (int)ref->rdd_entry, (int)cfg.vid, (int)cfg.aggregation_mode_port_vector,
                    (int)cfg.aggregation_vid_index, (unsigned)cfg.isolation_mode_port_vector, rdd_rc);
            }

            if (!rdd_rc)
            {
                ++(ref->ref_count);
                ++(ref->ref_count_vlan);
            }

            /* Handle aggregation if any */
            if (syscfg->switching_mode == rdpa_vlan_aware_switching)
            {
                rc = vlan_update_aggr_all_links(mo, ref->vid, 1);
                if (rc)
                {
                    vlan_vid_table_update_ex(mo, ref->vid, 0);
                    return rc;
                }
            }
        }
        else /* Delete */
        {
            if (ref->ref_count == 1) /* Last */
            {
                /* Delete. 1st delete aggregation if any, then delete forward eligibility */
                if (syscfg->switching_mode == rdpa_vlan_aware_switching)
                    vlan_update_aggr_all_links(mo, ref->vid, 0);

                vlan_handle_vid_entry_del(ref, 1); /* LAN */

                is_ref_valid = 0;

                /* The 'delete' routine compacts the references array, thus leaving the 'ref' invalid:  */
                /* Do not decrement the reference counter                                               */
            }
            else
            {
                /* Aggregation is enabled if the RDD LAN VID entry is referenced by the VLAN table at least once */
                cfg.aggregation_mode_port_vector = (ref->ref_count_vlan == 1) ? 0 :
#ifdef LEGACY_RDP
                    rdpa_ports2rdd_bridge_port_vector(ref->ports & RDPA_PORT_ALL_LAN),
#else
                    rdpa_ports2rdd_vport_vector(ref->ports & RDPA_PORT_ALL_LAN),
#endif
                rdd_rc = rdd_lan_vid_entry_cfg(ref->rdd_entry, &cfg);
                BDMF_TRACE_DBG_OBJ(mo, "rdd_lan_vid_entry_cfg(%d, {vid=%d ag=%d ag_vid=%d ports=0x%x}) --> %d\n",
                    (int)ref->rdd_entry, (int)cfg.vid, (int)cfg.aggregation_mode_port_vector,
                    (int)cfg.aggregation_vid_index, (unsigned)cfg.isolation_mode_port_vector, rdd_rc);

                if (!rdd_rc)
                {
                    --(ref->ref_count);
                    --(ref->ref_count_vlan);
                }
            }
        }
        if (rdd_rc)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "rdd error %d\n", rdd_rc);
    }

    /* If WAN VLAN and vlan_aware switching - configure aggregation */
    if (rdpa_if_is_wan(port) && syscfg->switching_mode == rdpa_vlan_aware_switching)
    {
        rc = vlan_update_aggr_all_links(mo, ref->vid, is_add);
        if (rc)
            return rc;

        if (is_add) /* Add */
        {
            /* Increment reference counter */
            ++(ref->ref_count);
        }
        else /* Delete */
        {
            if (ref->ref_count == 1) /* Last */
            {
                vlan_handle_vid_entry_del(ref, 0); /* WAN */
                
                is_ref_valid = 0;

                /* The 'delete' routine compacts the references array, thus leaving the 'ref' invalid:  */
                /* Do not decrement the reference counter                                               */
            }
            else
            {
                --(ref->ref_count);
            }
        }
    }

    /* Reference valid: Update port mask */
    if (is_ref_valid)
        ref->ports = new_mask;

    return 0;
}

int vlan_lan_to_wan_link_ex(struct bdmf_object *mo, struct bdmf_object *other)
{
    if (_rdpa_system_init_cfg_get()->switching_mode != rdpa_vlan_aware_switching)
        {
            BDMF_TRACE_INFO_OBJ(mo, "VLAN Aggregation is disabled. Linking with %s is ignored\n", other->name);
            return 0;
        }
        /* Go over all VIDs in the LAN container and configure WAN_VID - LAN_VID aggregation,
         * aggregation if not configured yet */
    return vlan_update_aggr_all_vids(mo, other, 1);
}

void vlan_lan_to_wan_unlink_ex(struct bdmf_object *mo, struct bdmf_object *other)
{
    /* Stop here if aggregation isn't enabled */
    if (_rdpa_system_init_cfg_get()->switching_mode != rdpa_vlan_aware_switching)
        return;

    /* Go over all VIDs in the LAN container and remove WAN_VID - LAN_VID aggregation,
     * if no longer referenced */
    vlan_update_aggr_all_vids(mo, other, 0);
}

static void vlan_handle_vid_entry_del(vlan_vid_ref_t *ref, bdmf_boolean is_lan)
{
    vlan_vid_ref_t *vlan_vid_refs = is_lan ? vlan_vid_refs_lan : vlan_vid_refs_wan;

    if (is_lan)
    {
        /* Handle RDD LAN VID entry */
        rdd_lan_vid_entry_delete(ref->rdd_entry);
        BDMF_TRACE_DBG("rdd_lan_vid_entry_del(%d)\n", (int)ref->rdd_entry);
    }

    /* Compact references array */
    memmove(ref, ref+1, (long)&vlan_vid_refs[RDPA_MAX_VLANS] - (long)(ref + 1));
    /* Clear the last entry that became free */
    memset(&vlan_vid_refs[RDPA_MAX_VLANS-1], 0, sizeof(vlan_vid_ref_t));
    vlan_vid_refs[RDPA_MAX_VLANS-1].rdd_entry = -1;
}

/***************************************************************************
 * RDP:A-internal helpers
 **************************************************************************/

/* Handle RDD LAN VID entry */

int _rdpa_handle_rdd_lan_vid_entry(uint16_t vid, bdmf_boolean is_add,
    void *params_v, uint8_t *entry)
{
    int err = 0;
    rdd_mac_params_t *params = (rdd_mac_params_t *)params_v;
    vlan_vid_ref_t *ref = NULL;
    rdd_lan_vid_cfg_t lan_vid_params =
    {
        .vid = vid,
        .isolation_mode_port_vector = 0, /* No ports */
        .aggregation_mode_port_vector = 0, /* No ports */
        .aggregation_vid_index = RDPA_VLAN_AGGR_ENTRY_DONT_CARE
    };

    /* Verify: Aggregation mode is not disabled */
    if (params && /* When calling this API from the System object, no 'params' is provided */
        !params->aggregation_mode)
    {
        return 0;
    }

    bdmf_fastlock_lock(&vlan_vid_refs_fastlock);

    ref = vlan_vid_ref_find(NULL, vid, 1); /* VID port: LAN; Allocate */
    if (!ref)
    {
        bdmf_trace("VID table is full (%d VIDs)\n", RDPA_MAX_VLANS);
        err = BDMF_ERR_TOO_MANY;
        goto lbl_out;
    }

    if (is_add) /* Add */
    {
        if (ref->ref_count == 0) /* First */
        {
            err = rdd_lan_vid_entry_add(&lan_vid_params, &ref->rdd_entry);
            BDMF_TRACE_DBG("rdd_lan_vid_entry_add({vid=%d ag=%d ag_vid=%d ports=0x%x}, %d) --> %d\n",
                (int)lan_vid_params.vid, (int)lan_vid_params.aggregation_mode_port_vector,
                (int)lan_vid_params.aggregation_vid_index, (unsigned)lan_vid_params.isolation_mode_port_vector,
                (int)ref->rdd_entry, err);

            if (err)
                goto lbl_out;
        }

        if (entry) /* When calling this API from the System object, no 'entry' is provided */
            *entry = ref->rdd_entry;
        
        ++(ref->ref_count);
    }
    else /* Delete */
    {
        if (ref->ref_count == 1) /* Last */
        {
            vlan_handle_vid_entry_del(ref, 1); /* LAN */
            /* The 'delete' routine compacts the references array, thus leaving the 'ref' invalid:  */
            /* Do not decrement the reference counter                                               */
        }
        else
        {
            --(ref->ref_count);
        }
    }

lbl_out:
    bdmf_fastlock_unlock(&vlan_vid_refs_fastlock);
    return err;
}

int vlan_attr_ingress_filter_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int vlan_attr_mac_lkp_cfg_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int vlan_attr_proto_filters_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int vlan_attr_stat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int vlan_attr_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int vlan_pre_init_ex(struct bdmf_object *mo)
{
    return BDMF_ERR_OK;
}

int vlan_post_init_ex(struct bdmf_object *mo)
{
    return BDMF_ERR_OK;
}

int vlan_update_high_priority_vids(int16_t vid, bdmf_boolean enable)
{
    return BDMF_ERR_OK;
}

int vlan_update_bridge_id_ex(struct bdmf_object *vlan_obj, bdmf_index *bridge_id)
{
    return BDMF_ERR_OK;
}

int vlan_attr_options_write_ex(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    return BDMF_ERR_OK;
}

/***************************************************************************
 * RDP:A-internal helpers
 **************************************************************************/

/* Get LAN VID entry. Returns entry >=0 or -1 if no entry or if vlan_aware_switching mode and entry is not aggregated */
int _rdpa_vlan_vid2entry(int16_t vid)
{
    const rdpa_system_init_cfg_t *syscfg = _rdpa_system_init_cfg_get();
    static vlan_vid_ref_t *vid_ref;

    if (syscfg->switching_mode == rdpa_switching_none)
        return -1;

    vid_ref = vlan_vid_ref_find(NULL, vid, 0);
    if (!vid_ref)
        return -1;

    if (!vid_ref->wan_ref_count && syscfg->switching_mode == rdpa_vlan_aware_switching)
        return -1;

    return vid_ref->rdd_entry;
}

/* Get VID by rdd_entry */
int _rdpa_vlan_entry2vid(int vid_entry)
{
    int i;

    for (i = 0; i < RDPA_MAX_VLANS && vlan_vid_refs_lan[i].ref_count; i++)
    {
        if (vlan_vid_refs_lan[i].rdd_entry == vid_entry)
            return vlan_vid_refs_lan[i].vid;
    }
    return 0;
}

/* Add aggregation wan_obj --> lan vid_ref */
int vlan_wan_aggr_add_ex(struct bdmf_object *lan_obj, struct bdmf_object *wan_obj, int16_t lan_vid)
{
    vlan_drv_priv_t *wan_vlan = (vlan_drv_priv_t *)bdmf_obj_data(wan_obj);
    vlan_wan_aggr_ref_t *wan_ref = NULL;
    vlan_vid_ref_t *vid_ref;
    int i;
    int rc;

    vid_ref = vlan_vid_ref_find(NULL, lan_vid, 0);
    if (!vid_ref)
    {
        /* That's odd. Shouldn't have happened */
        BDMF_TRACE_ERR_OBJ(lan_obj, "Internal error. VID %d reference is not found\n", lan_vid);
        return BDMF_ERR_INTERNAL;
    }
 
    /* Do nothing if no VID in WAN container */
    if (wan_vlan->vids[0].vid == BDMF_INDEX_UNASSIGNED)
        return 0;

    /* Find/allocate wan aggregation reference */
    for (i = 0; i < RDPA_MAX_AGGR_WAN_VLANS; i++)
    {
        if (vlan_wan_aggr_vids[i].ref_count)
        {
            /* Check for match */
            if (vlan_wan_aggr_vids[i].vid == wan_vlan->vids[0].vid)
            {
                wan_ref = &vlan_wan_aggr_vids[i];
                break;
            }
        }
        else
        {
            /* Empty slot */
            if (!wan_ref)
                wan_ref = &vlan_wan_aggr_vids[i];
        }
    }
    if (!wan_ref)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_TOO_MANY, lan_obj, "%d WAN VIDs are already being aggregated\n",
            RDPA_MAX_AGGR_WAN_VLANS);
    }
    wan_ref->vid = wan_vlan->vids[0].vid;

    /* If LAN VID is already aggregated to another WAN VID - it is an error */
    if (vid_ref->wan_ref_count && wan_ref != &vlan_wan_aggr_vids[vid_ref->wan_entry])
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_ALREADY, lan_obj, "LAN VID %d is already aggregated to another WAN VID %d\n",
            vid_ref->vid, vlan_wan_aggr_vids[vid_ref->wan_entry].vid);
    }

    /* Notify RDD if LAN VID is not aggregated yet */
    if (!vid_ref->wan_ref_count)
    {
        rc = vlan_wan_aggr_add_to_rdd(lan_obj, vid_ref, wan_ref);
        if (rc)
            return rc;
    }   
    /* All good. Increment reference counts */
    ++vid_ref->wan_ref_count;
    ++wan_ref->ref_count;

    return 0;
}

/* Delete aggregation wan_obj --> lan vid_ref */
void vlan_wan_aggr_del_ex(struct bdmf_object *lan_obj, struct bdmf_object *wan_obj, int16_t lan_vid)
{
    vlan_drv_priv_t *wan_vlan = (vlan_drv_priv_t *)bdmf_obj_data(wan_obj);
    vlan_wan_aggr_ref_t *wan_ref;
    vlan_vid_ref_t *vid_ref;

    vid_ref = vlan_vid_ref_find(NULL, lan_vid, 0);
    if (!vid_ref)
        return;

    /* There are roll-back scenarios in which this function can be called for wrong WAN. Ignore */
    if (!vid_ref->wan_ref_count)
        return;
    wan_ref = &vlan_wan_aggr_vids[vid_ref->wan_entry];
    if (wan_ref->vid != wan_vlan->vids[0].vid)
        return;
    BUG_ON(!wan_ref->ref_count);

    /* The same LAN VID can be "aggregated" multiple times, from different VLAN containers.
     * Remove aggregation in RDD only when the last reference is removed
     */
    --vid_ref->wan_ref_count;
    if (vid_ref->wan_ref_count)
        return;

    vlan_wan_aggr_delete_to_rdd(lan_obj, vid_ref, wan_ref);

    /* Reduce number of LAN VIDs referencing the given WAN VID */
    --wan_ref->ref_count;
}

void vlan_drv_init_ex(void)
{
    int i;

    for (i = 0; i < RDPA_MAX_VLANS; i++)
    {
        vlan_vid_refs_lan[i].rdd_entry = -1;
        vlan_vid_refs_lan[i].ref_count = 0;

        vlan_vid_refs_wan[i].ref_count = 0;
        vlan_vid_refs_wan[i].ref_count_vlan = 0;
    }
}

void vlan_remove_default_vid_ex(struct bdmf_object *mo)
{
    ;/*nop*/
}
