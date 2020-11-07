/*
* <:copyright-BRCM:2013-2015:proprietary:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
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
 :>
*/

/*
 * rdpa_vlan_action.c
 *
 *  Created on: Aug 20, 2012
 *      Author: igort
 */

#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_common.h"
#include "rdd.h"
#include "rdpa_int.h"
#if !defined(LEGACY_RDP) && !defined(XRDP)
#include "rdd_multicast_processing.h"
#endif
#include "rdpa_vlan_action_ex.h"

/***************************************************************************
 * vlan_action object type
 **************************************************************************/

/* vlan_action object private data */
typedef struct {
    bdmf_index index;              /**< vlan_action index */
    rdpa_traffic_dir dir;          /**< US / DS */
    rdpa_vlan_action_cfg_t cfg;    /**< Action */
    rdd_vlan_action_cl_t cl;       /**< Command list. Used for XRDP only */
    int is_configured;
} vlan_action_drv_priv_t;


/* Static data */
static struct bdmf_object *us_vlan_action_objects[RDPA_MAX_VLAN_ACTION];
static struct bdmf_object *ds_vlan_action_objects[RDPA_MAX_VLAN_ACTION];


/* TPID Replace */

#define RDPA_VLAN_ACTION_TPID_DUMMY 0x0
#define RDPA_VLAN_ACTION_TPID_SZ    (RDD_TPID_ID_7 + 1)

typedef struct
{
    uint16_t vals[RDPA_VLAN_ACTION_TPID_SZ];
    uint8_t ref_cntrs[RDPA_VLAN_ACTION_TPID_SZ];
} vlan_action_tpid_replace_t;

static vlan_action_tpid_replace_t vlan_action_tpid_replace;

#if defined __RDPA_VLAN_ACTION_DBG__
static void vlan_action_tpid_dump(void)
{
    uint8_t cntr;

    for (cntr = 0; cntr < RDPA_VLAN_ACTION_TPID_SZ; ++cntr)
    {
        bdmf_trace("__DBG> vlan_action_tpid_replace.vals[%u]: %u; vlan_action_tpid_replace.ref_cntrs[%u]: %u\n",
            cntr, vlan_action_tpid_replace.vals[cntr], cntr, vlan_action_tpid_replace.ref_cntrs[cntr]);
    }
}
#endif /* __RDPA_VLAN_ACTION_DBG__ */

static void vlan_action_get_tpid_info(const struct bdmf_object * const mo, uint32_t cmd,
    bdmf_boolean * const is_replace_supported, uint8_t * const tag_indx)
{
    bdmf_boolean is_pbit_transparent;
    *is_replace_supported = 0;
    *tag_indx = RDPA_VLAN_TAG_OUT;

    /* If VLAN command is one of PUSH family and PBIT command is transparent, force it to be COPY */
    if ((cmd & RDPA_VLAN_CMD_GROUP_PUSH) && !(cmd & RDPA_VLAN_CMD_GROUP_PBIT))
        cmd |= RDPA_VLAN_CMD_DSCP;
    is_pbit_transparent = (cmd & RDPA_VLAN_CMD_GROUP_PBIT) == 0;

    if ((cmd & RDPA_VLAN_CMD_PUSH_ALWAYS) && (cmd & (RDPA_VLAN_CMD_DSCP | RDPA_VLAN_CMD_REMARK)))
    {
        *is_replace_supported = 1;
        *tag_indx = RDPA_VLAN_TAG_IN;
    }
    else if ((((cmd & RDPA_VLAN_CMD_GROUP_PUSH_REPLACE) == RDPA_VLAN_CMD_GROUP_PUSH_REPLACE) &&
                (cmd & (RDPA_VLAN_CMD_REMARK | RDPA_VLAN_CMD_REMAP | RDPA_VLAN_CMD_DSCP | RDPA_VLAN_CMD_TPID_REMARK))) ||
               ((cmd & RDPA_VLAN_CMD_REPLACE2) &&
                ((cmd & RDPA_VLAN_CMD_REMARK) || is_pbit_transparent))
            )
    {
        *is_replace_supported = 1;  
        *tag_indx = RDPA_VLAN_TAG_IN; 
    }
    else if (cmd & (RDPA_VLAN_CMD_POP | RDPA_VLAN_CMD_REPLACE))
    {
        *is_replace_supported = 1;
        *tag_indx = RDPA_VLAN_TAG_OUT;
    }
}

static void vlan_action_handle_tpid_rdd(void)
{
    rdpa_rdd_tpid_set(vlan_action_tpid_replace.vals,
        sizeof(vlan_action_tpid_replace.vals) / sizeof(vlan_action_tpid_replace.vals[0]));
}

static int vlan_action_handle_tpid_in_ref(uint16_t tpid, uint16_t * const indx)
{
    int indx_empty = -1;
    /* Check if the tpid value is 0xffff than do nothing */
    if (tpid == RDPA_VLAN_ACTION_TPID_DONT_CARE)
        return 0;

    /* Locate index */
    for (*indx = 0; *indx < RDPA_VLAN_ACTION_TPID_SZ; ++(*indx))
    {
        if (vlan_action_tpid_replace.vals[*indx] == tpid) /* Found */
            break;

        if (vlan_action_tpid_replace.vals[*indx] == RDPA_VLAN_ACTION_TPID_DUMMY) /* Empty */
        {
            if (indx_empty == -1) /* First */
                indx_empty = *indx;
        }
    }

    /* Set value */
    if (*indx == RDPA_VLAN_ACTION_TPID_SZ) /* End */
    {
        if (indx_empty == -1) /* No empty found */
            return BDMF_ERR_NOENT;

        *indx = indx_empty;

        /* Update value */
        vlan_action_tpid_replace.vals[*indx] = tpid;
        vlan_action_handle_tpid_rdd();
    }

    /* Increment reference counter */
    ++(vlan_action_tpid_replace.ref_cntrs[*indx]);
    return 0;
}

static void vlan_action_handle_tpid_out_ref(uint16_t tpid)
{
    uint8_t cntr;

    for (cntr = 0; cntr < RDPA_VLAN_ACTION_TPID_SZ; ++cntr)
    {
        if (vlan_action_tpid_replace.vals[cntr] == tpid) /* Found */
        {
            /* Decrement reference counter */
            --(vlan_action_tpid_replace.ref_cntrs[cntr]);
            if (!vlan_action_tpid_replace.ref_cntrs[cntr]) /* Last */
            {
                vlan_action_tpid_replace.vals[cntr] = RDPA_VLAN_ACTION_TPID_DUMMY;
                vlan_action_handle_tpid_rdd();
            }
            break;
        }
    }
}

static int vlan_action_test_tpid_in(const struct bdmf_object *mo, rdpa_vlan_action_cfg_t *cfg)
{
    bdmf_boolean is_replace_supported;
    uint8_t tag_indx;

    vlan_action_get_tpid_info(mo, cfg->cmd, &is_replace_supported, &tag_indx);
    if (!is_replace_supported)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "TPID replace is not supported for this command\n");

    if (cfg->parm[tag_indx].tpid == RDPA_VLAN_ACTION_TPID_DUMMY)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "No value set; Tag expected: %s\n",
            (tag_indx == RDPA_VLAN_TAG_OUT) ? "Outer" : "Inner");
    }

    return 0;
}

static int vlan_action_handle_tpid_in(const struct bdmf_object * const mo, rdpa_vlan_action_cfg_t * const cfg,
    uint16_t *const indx, uint8_t *tag_indx)
{
    bdmf_boolean is_replace_supported;
    int rc;

    vlan_action_get_tpid_info(mo, cfg->cmd, &is_replace_supported, tag_indx);
    if (is_replace_supported)
    {
        rc = vlan_action_handle_tpid_in_ref(cfg->parm[*tag_indx].tpid, indx);
        if (rc < 0)
            return rc;

        cfg->parm[(*tag_indx == RDPA_VLAN_TAG_OUT) ? RDPA_VLAN_TAG_IN : RDPA_VLAN_TAG_OUT].tpid =
            RDPA_VLAN_ACTION_TPID_DUMMY;
    }

    return 0;
}

static void vlan_action_handle_tpid_out(const struct bdmf_object *mo, rdpa_vlan_action_cfg_t *cfg)
{
    bdmf_boolean is_replace_supported;
    uint8_t tag_indx;

    vlan_action_get_tpid_info(mo, cfg->cmd, &is_replace_supported, &tag_indx);
    if (is_replace_supported)
    {
        vlan_action_handle_tpid_out_ref(cfg->parm[tag_indx].tpid);

        /* Clear tag: Current */
        cfg->parm[tag_indx].tpid = RDPA_VLAN_ACTION_TPID_DUMMY;
    }
}

static void vlan_action_handle_tpid_out_remark(const struct bdmf_object * const mo,
    rdpa_vlan_action_cfg_t * const cfg, uint8_t tag_indx)
{
    vlan_action_handle_tpid_out_ref(cfg->parm[tag_indx].tpid);

    /* Clear tag: Current */
    cfg->parm[tag_indx].tpid = RDPA_VLAN_ACTION_TPID_DUMMY;
}

/* Validate and configure VLAN action */
static int vlan_action_configure(struct bdmf_object *mo, rdpa_vlan_action_cfg_t *cfg)
{
    vlan_action_drv_priv_t *vlan_action = (vlan_action_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_rdd_vlan_action_params_t vlan_params;
    uint32_t cmd = cfg->cmd;

    uint16_t tpid_indx = 0xFFFF; /* For RDPA_VLAN_CMD_TPID command*/
    uint16_t otpid_indx = 0xFFFF;
    uint16_t itpid_indx = 0xFFFF;
    /* TPID location for RDPA_VLAN_CMD_TPID command */
    uint8_t tpid_location;
    int rc;

    vlan_params.outer_tpid_overwrite_enable = 0;
    vlan_params.inner_tpid_overwrite_enable = 0;
    vlan_params.outer_tpid_id = 0;
    vlan_params.inner_tpid_id = 0;
    vlan_params.index = vlan_action->index;
    vlan_params.dir = vlan_action->dir;
    vlan_params.cfg = *cfg;

    /* Validations */
    /* If there is action in 2-vid group, no more vid actions are supported */
    if ((0 != (cmd & RDPA_VLAN_CMD_GROUP_1VID) && 0 != (cmd & RDPA_VLAN_CMD_GROUP_2VID)) ||
        ((cmd & RDPA_VLAN_CMD_GROUP_2VID) & ((cmd & RDPA_VLAN_CMD_GROUP_2VID) - 1)))
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "%08x: 2 VID command can't be combined with 1 VID\n", cmd);
    }

    /* Only 1 bit in PBIT group can be set */
    if ((cmd & RDPA_VLAN_CMD_GROUP_PBIT) & ((cmd & RDPA_VLAN_CMD_GROUP_PBIT) - 1))
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "%08x: PBIT commands can not be combined\n", cmd);

    rc = rdpa_rdd_vlan_action_validate(mo, cfg);
    if (rc)
        return rc;

    /* Handle TPID */
    /* Verify TPID RDPA_VLAN_CMD_TPID_REMARK or RDPA_VLAN_CMD_TPID not both */
    if ((cfg->cmd & RDPA_VLAN_CMD_TPID) && (cfg->cmd & RDPA_VLAN_CMD_TPID_REMARK))
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "not supported combination of TPID and TPID_REMARK\n");
    }

    /* VLAN_CMD_TPID_REMARK Handling */
    if (cfg->cmd & RDPA_VLAN_CMD_TPID_REMARK)
    {
        /* Outgoing: Handle */
        /* TPID flag is set and Skip for newly created objects*/
        if ((vlan_action->cfg.cmd & RDPA_VLAN_CMD_TPID_REMARK) && mo->state != bdmf_state_inactive)
        {
            /* Outgoing: Handle outer */
            vlan_action_handle_tpid_out_remark(mo, &(vlan_action->cfg), RDPA_VLAN_TAG_OUT);

            /* Outgoing: Handle inner */
            vlan_action_handle_tpid_out_remark(mo, &(vlan_action->cfg), RDPA_VLAN_TAG_IN);
        }

        /* Incoming: Handle */
        rc = vlan_action_handle_tpid_in_ref(cfg->parm[RDPA_VLAN_TAG_OUT].tpid, &otpid_indx);
        if (rc < 0)
            return rc;
        rc = vlan_action_handle_tpid_in_ref(cfg->parm[RDPA_VLAN_TAG_IN].tpid, &itpid_indx);
        if (rc < 0)
            return rc;
        /* Update the RDD struct */
    	vlan_params.outer_tpid_id = otpid_indx&7;
        vlan_params.inner_tpid_id = itpid_indx&7;
    	/* Overwrite tpid value only if valid */
    	vlan_params.outer_tpid_overwrite_enable = 
            (cfg->parm[RDPA_VLAN_TAG_OUT].tpid == RDPA_VLAN_ACTION_TPID_DONT_CARE ? 0 : 1);
    	vlan_params.inner_tpid_overwrite_enable = 
            (cfg->parm[RDPA_VLAN_TAG_IN].tpid == RDPA_VLAN_ACTION_TPID_DONT_CARE ? 0 : 1);
    }

    /* Incoming: Test */
    if (cfg->cmd & RDPA_VLAN_CMD_TPID) /* TPID flag is set */
    {
        /* Test */
        rc = vlan_action_test_tpid_in(mo, cfg);
        if (rc)
            return rc;
    }

    /* Outgoing: Handle */
    /* TPID flag is set and Skip for newly created objects*/
    if ((vlan_action->cfg.cmd & RDPA_VLAN_CMD_TPID) && mo->state != bdmf_state_inactive)
    {
        vlan_action_handle_tpid_out(mo, &vlan_action->cfg);

#if defined __RDPA_VLAN_ACTION_DBG__
        vlan_action_tpid_dump();
#endif /* __RDPA_VLAN_ACTION_DBG__ */
    }

    /* Incoming: Handle */
    if (cfg->cmd & RDPA_VLAN_CMD_TPID) /* TPID flag is set */
    {
        rc = vlan_action_handle_tpid_in(mo, cfg, &tpid_indx, &tpid_location);
        if (rc < 0)
            return rc;

        /* RDPA_VLAN_CMD_TPID Backward compatibility */
        if (tpid_location == RDPA_VLAN_TAG_OUT)
            vlan_params.outer_tpid_overwrite_enable = (tpid_indx != 0xFFFF) ? 1 : 0;
        else if (tpid_location == RDPA_VLAN_TAG_IN)
            vlan_params.inner_tpid_overwrite_enable = (tpid_indx != 0xFFFF) ? 1 : 0;

        vlan_params.outer_tpid_id = tpid_indx;
        vlan_params.inner_tpid_id = tpid_indx;

#if defined __RDPA_VLAN_ACTION_DBG__
        vlan_action_tpid_dump();
#endif /* __RDPA_VLAN_ACTION_DBG__ */
    }

    /* Call RDD API */
    rc = rdpa_rdd_vlan_action_set(mo, &vlan_params, &vlan_action->cl);
    if (!rc)
    {
        vlan_action->is_configured = 1;
    }

    return rc;
}

/** This optional callback is called called at object init time
 *  before initial attributes are set.
 *  If function returns error code !=0, object creation is aborted
 */
static int vlan_action_pre_init(struct bdmf_object *mo)
{
    vlan_action_drv_priv_t *vlan_action = (vlan_action_drv_priv_t *)bdmf_obj_data(mo);

    vlan_action->index = BDMF_INDEX_UNASSIGNED;

    return 0;
}

/** This optional callback is called at object init time
 * after initial attributes are set.
 */
static int vlan_action_post_init(struct bdmf_object *mo)
{
    vlan_action_drv_priv_t *vlan_action = (vlan_action_drv_priv_t *)bdmf_obj_data(mo);
    struct bdmf_object **vlan_action_objects = (vlan_action->dir == rdpa_dir_ds) ?
        ds_vlan_action_objects : us_vlan_action_objects;
    int rc;

    /* If vlan_action index is set - make sure it is unique.
     * Otherwise, assign free
     */
    if (vlan_action->index < 0)
    {
        int i;
        /* Find and assign free index */
        for (i = 0; i < RDPA_MAX_VLAN_ACTION; i++)
        {
            if (!vlan_action_objects[i])
            {
                vlan_action->index = i;
                break;
            }
        }
    }
    if (vlan_action->index < 0)
        BDMF_TRACE_RET(BDMF_ERR_TOO_MANY, "Can't create more than %d vlan_action objects\n", RDPA_MAX_VLAN_ACTION);
    else if (vlan_action->index >= RDPA_MAX_VLAN_ACTION)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "vlan_action index %u is out of range\n", (unsigned)vlan_action->index);
    if (vlan_action_objects[vlan_action->index])
        BDMF_TRACE_RET(BDMF_ERR_ALREADY, "vlan_action %u already exists\n", (unsigned)vlan_action->index);

    vlan_action_objects[vlan_action->index] = mo;
    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "vlan_action/dir=%s,index=%ld",
        bdmf_attr_get_enum_text_hlp(&rdpa_traffic_dir_enum_table, vlan_action->dir), vlan_action->index);

    rc = vlan_action_configure(mo, &vlan_action->cfg);

    return rc;
}

static void vlan_action_destroy(struct bdmf_object *mo)
{
    vlan_action_drv_priv_t *vlan_action = (vlan_action_drv_priv_t *)bdmf_obj_data(mo);
    struct bdmf_object **vlan_action_objects = (vlan_action->dir == rdpa_dir_ds) ?
        ds_vlan_action_objects : us_vlan_action_objects;

    if ((unsigned)vlan_action->index >= RDPA_MAX_VLAN_ACTION ||
        vlan_action_objects[vlan_action->index] != mo)
    {
        return;
    }

    /* Set as transparent */
    if (vlan_action->is_configured)
    {
        rdpa_vlan_action_cfg_t cfg = {};

        vlan_action_configure(mo, &cfg);
    }

    vlan_action_objects[vlan_action->index] = NULL;
}


/*
 * vlan_action attribute access
 */

/* "action" attribute "write" callback */
static int vlan_action_attr_action_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    vlan_action_drv_priv_t *priv = (vlan_action_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_vlan_action_cfg_t *cfg = (rdpa_vlan_action_cfg_t *)val;
    int rc = 0;

    /* notify runner if update rather than init */
    if (mo->state == bdmf_state_active)
        rc = vlan_action_configure(mo, cfg);

    if (rc < 0)
        return rc;

    priv->cfg = *cfg;

    return 0;
}

/* vlan_cmd type enum values */
static bdmf_attr_enum_table_t vlan_action_cmd_enum_table = {
    .type_name = "rdpa_vlan_command", .help = "VLAN command",
    .values = {
        { "push", RDPA_VLAN_CMD_BIT_PUSH},
        { "push_always", RDPA_VLAN_CMD_BIT_PUSH_ALWAYS},
        { "pop", RDPA_VLAN_CMD_BIT_POP},
        { "replace", RDPA_VLAN_CMD_BIT_REPLACE},
        { "push2", RDPA_VLAN_CMD_BIT_PUSH2},
        { "push2_always", RDPA_VLAN_CMD_BIT_PUSH2_ALWAYS},
        { "pop2", RDPA_VLAN_CMD_BIT_POP2},
        { "replace2", RDPA_VLAN_CMD_BIT_REPLACE2},
        { "remark", RDPA_VLAN_CMD_BIT_REMARK},
        { "remap", RDPA_VLAN_CMD_BIT_REMAP},
        { "dscp", RDPA_VLAN_CMD_BIT_DSCP},
        { "tpid", RDPA_VLAN_CMD_BIT_TPID},
        { "tpid_rem", RDPA_VLAN_CMD_BIT_TPID_REMARK},
        {NULL, 0}
    }
};

/*  vlan_action aggregate type */
struct bdmf_aggr_type vlan_action_type = {
    .name = "vlan_action", .struct_name = "rdpa_vlan_action_cfg_t",
    .help = "VLAN action parameters",
    .fields = (struct bdmf_attr[]) {
        { .name = "cmd", .help = "Command", .type = bdmf_attr_enum_mask, .ts.enum_table = &vlan_action_cmd_enum_table,
            .size = sizeof(uint32_t), .offset = offsetof(rdpa_vlan_action_cfg_t, cmd)
        },
        { .name = "ovid", .help = "Outer VID", .type = bdmf_attr_number,
            .size = sizeof(uint16_t), .offset = offsetof(rdpa_vlan_action_cfg_t, parm[RDPA_VLAN_TAG_OUT].vid),
            .max_val = 4095
        },
        { .name = "opbit", .help = "Outer PBIT", .type = bdmf_attr_number,
            .size = sizeof(rdpa_pbit), .offset = offsetof(rdpa_vlan_action_cfg_t, parm[RDPA_VLAN_TAG_OUT].pbit),
            .max_val = 31
        },
        { .name = "otpid", .help = "Outer TPID", .type = bdmf_attr_number,
            .size = sizeof(uint16_t), .offset = offsetof(rdpa_vlan_action_cfg_t, parm[RDPA_VLAN_TAG_OUT].tpid),
            .flags = BDMF_ATTR_HEX_FORMAT
        },
        { .name = "ivid", .help = "Inner VID", .type = bdmf_attr_number,
            .size = sizeof(uint16_t), .offset = offsetof(rdpa_vlan_action_cfg_t, parm[RDPA_VLAN_TAG_IN].vid),
            .max_val = 4095
        },
        { .name = "ipbit", .help = "Inner PBIT", .type = bdmf_attr_number,
            .size = sizeof(rdpa_pbit), .offset = offsetof(rdpa_vlan_action_cfg_t, parm[RDPA_VLAN_TAG_IN].pbit),
            .max_val = 31
        },
        { .name = "itpid", .help = "Inner TPID", .type = bdmf_attr_number,
            .size = sizeof(uint16_t), .offset = offsetof(rdpa_vlan_action_cfg_t, parm[RDPA_VLAN_TAG_IN].tpid),
            .flags = BDMF_ATTR_HEX_FORMAT
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(vlan_action_type);

/* Object attribute descriptors */
static struct bdmf_attr vlan_action_attrs[] = {
    { .name = "dir", .help = "Traffic Direction",
        .type = bdmf_attr_enum, .ts.enum_table = &rdpa_traffic_dir_enum_table,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG |
            BDMF_ATTR_KEY | BDMF_ATTR_MANDATORY,
        .size = sizeof(rdpa_traffic_dir), .offset = offsetof(vlan_action_drv_priv_t, dir)
    },
    { .name = "index", .help = "VLAN Action Index", .type = bdmf_attr_number,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_KEY,
        .size = sizeof(bdmf_index), .offset = offsetof(vlan_action_drv_priv_t, index)
    },
    { .name = "action", .help = "Action and parameters",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "vlan_action",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .offset = offsetof(vlan_action_drv_priv_t, cfg),
        .write = vlan_action_attr_action_write,
    },
    BDMF_ATTR_LAST
};

static int vlan_action_drv_init(struct bdmf_type *drv);
static void vlan_action_drv_exit(struct bdmf_type *drv);

struct bdmf_type vlan_action_drv = {
    .name = "vlan_action",
    .parent = "system",
    .description = "VLAN Action (header operations)",
    .drv_init = vlan_action_drv_init,
    .drv_exit = vlan_action_drv_exit,
    .pre_init = vlan_action_pre_init,
    .post_init = vlan_action_post_init,
    .destroy = vlan_action_destroy,
    .extra_size = sizeof(vlan_action_drv_priv_t),
    .aattr = vlan_action_attrs,
    .max_objs = RDPA_MAX_VLAN_ACTION+RDPA_MAX_VLAN_ACTION,
};
DECLARE_BDMF_TYPE(rdpa_vlan_action, vlan_action_drv);

/* Init/exit module. Cater for GPL layer */
static int vlan_action_drv_init(struct bdmf_type *drv)
{
    uint8_t cntr;

#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_vlan_action_drv = rdpa_vlan_action_drv;
    f_rdpa_vlan_action_get = rdpa_vlan_action_get;
#endif

    /* Initialize TPID Replace data */
    for (cntr = 0; cntr < RDPA_VLAN_ACTION_TPID_SZ; ++cntr)
    {
        vlan_action_tpid_replace.vals[cntr] = RDPA_VLAN_ACTION_TPID_DUMMY;
        vlan_action_tpid_replace.ref_cntrs[cntr] = 0;
    }

    return 0;
}

static void vlan_action_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_vlan_action_drv = NULL;
    f_rdpa_vlan_action_get = NULL;
#endif
}

/** Internal Get vlan_action object by key */
int _rdpa_vlan_action_get(const rdpa_vlan_action_key_t *_key_, bdmf_object_handle *_obj_)
{
    struct bdmf_object **objects = (_key_->dir == rdpa_dir_ds) ? ds_vlan_action_objects : us_vlan_action_objects;

    if ((unsigned)_key_->index >= RDPA_MAX_VLAN_ACTION)
        return BDMF_ERR_RANGE;

    *_obj_ = objects[_key_->index];
    if (!(*_obj_))
        return BDMF_ERR_NOENT;
    return 0;
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/

/** Get vlan_action object by key */
int rdpa_vlan_action_get(const rdpa_vlan_action_key_t *_key_, bdmf_object_handle *_obj_)
{
    struct bdmf_object **objects = (_key_->dir == rdpa_dir_ds) ? ds_vlan_action_objects : us_vlan_action_objects;
    return rdpa_obj_get(objects, RDPA_MAX_VLAN_ACTION, _key_->index, _obj_);
}

/***************************************************************************
 * Additional helpers used by other drivers
 **************************************************************************/
bdmf_error_t rdpa_vlan_action_cl_get(struct bdmf_object *mo, rdd_vlan_action_cl_t *cl)
{
    vlan_action_drv_priv_t *vlan_action = (vlan_action_drv_priv_t *)bdmf_obj_data(mo);
    memcpy(cl, &vlan_action->cl, sizeof(rdd_vlan_action_cl_t));
    return BDMF_ERR_OK;
}
