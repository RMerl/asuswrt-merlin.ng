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
 * rdpa_vlan_action_ex.c
 *
 */

#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_common.h"
#include "rdd.h"
#include "rdpa_int.h"
#ifndef LEGACY_RDP
#include "rdd_multicast_processing.h"
#endif
#include "rdpa_vlan_action_ex.h"


/* Map rdpa vlan command to rdd */
static bdmf_error_t vlan_action_map_to_rdd(struct bdmf_object *mo, uint32_t cmd,
    rdd_bridge_vlan_cmd_t *rdd_vlan_cmd, rdd_bridge_pbits_cmd_t *rdd_pbit_cmd)
{
    *rdd_vlan_cmd = RDD_VLAN_CMD_TRANSPARENT;
    *rdd_pbit_cmd = RDD_PBITS_CMD_TRANSPARENT;

    if (!cmd)
        return 0;

    /* Handle Tag command */
    if (vlan_action_is_1tag(cmd)) /* Single-tag command */
    {
        if ((cmd & RDPA_VLAN_CMD_PUSH))
            *rdd_vlan_cmd = RDD_VLAN_CMD_ADD_TAG;
        else if ((cmd & RDPA_VLAN_CMD_PUSH_ALWAYS))
        {
            if ((cmd & RDPA_VLAN_CMD_REMARK))
                *rdd_vlan_cmd = RDD_VLAN_CMD_ADD_TAG_ALWAYS;
            else
                *rdd_vlan_cmd = RDD_VLAN_CMD_ADD_3RD_TAG;
        }
        else if ((cmd & RDPA_VLAN_CMD_POP))
        {
            if ((cmd & RDPA_VLAN_CMD_DSCP))
                *rdd_vlan_cmd = RDD_VLAN_CMD_REMOVE_OUTER_TAG_COPY;
            else
                *rdd_vlan_cmd = RDD_VLAN_CMD_REMOVE_TAG;
        }
        else if ((cmd & RDPA_VLAN_CMD_REPLACE))
            *rdd_vlan_cmd = RDD_VLAN_CMD_REPLACE_TAG;
    }
    else /* Double-tag command */
    {
        if ((cmd & RDPA_VLAN_CMD_PUSH2))
            *rdd_vlan_cmd = RDD_VLAN_CMD_ADD_TWO_TAGS;
        else if (((cmd & RDPA_VLAN_CMD_GROUP_PUSH_REPLACE) == RDPA_VLAN_CMD_GROUP_PUSH_REPLACE) ||
                 ((cmd & RDPA_VLAN_CMD_GROUP_PUSH_ALWAYS_REPLACE) == RDPA_VLAN_CMD_GROUP_PUSH_ALWAYS_REPLACE))
            *rdd_vlan_cmd = RDD_VLAN_CMD_ADD_OUTER_TAG_REPLACE_INNER_TAG;
        else if ((cmd & RDPA_VLAN_CMD_GROUP_POP_REPLACE) == RDPA_VLAN_CMD_GROUP_POP_REPLACE)
            *rdd_vlan_cmd = RDD_VLAN_CMD_REMOVE_OUTER_TAG_REPLACE_INNER_TAG;
        else if ((cmd & RDPA_VLAN_CMD_POP2))
            *rdd_vlan_cmd = RDD_VLAN_CMD_REMOVE_TWO_TAGS;
        else if ((cmd & RDPA_VLAN_CMD_REPLACE2))
            *rdd_vlan_cmd = RDD_VLAN_CMD_REPLACE_OUTER_TAG_REPLACE_INNER_TAG;
        else
            BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "%08x: Command is not supported\n", cmd);
    }

    /* Handle P-Bits command */

    /* If VLAN command is one of PUSH family and PBIT command is transparent, force it to be COPY */
    if ((cmd & RDPA_VLAN_CMD_GROUP_PUSH) && !(cmd & RDPA_VLAN_CMD_GROUP_PBIT))
        cmd |= RDPA_VLAN_CMD_DSCP;

    if ((cmd & RDPA_VLAN_CMD_REMARK))
        *rdd_pbit_cmd = RDD_PBITS_CMD_CONFIGURED;
    else if ((cmd & RDPA_VLAN_CMD_REMAP))
        *rdd_pbit_cmd = RDD_PBITS_CMD_REMAP;
    else if ((*rdd_vlan_cmd == RDD_VLAN_CMD_TRANSPARENT) && (cmd & RDPA_VLAN_CMD_TPID_REMARK))
        *rdd_pbit_cmd = RDD_PBITS_CMD_REMAP;
    else if ((cmd & RDPA_VLAN_CMD_DSCP) && !((cmd & RDPA_VLAN_CMD_GROUP_VID) == RDPA_VLAN_CMD_POP))
        *rdd_pbit_cmd = RDD_PBITS_CMD_COPY;

    return 0;
}

/* Validate VLAN command */
bdmf_error_t rdpa_rdd_vlan_action_validate(struct bdmf_object *mo, const rdpa_vlan_action_cfg_t *cfg)
{
    rdd_bridge_vlan_cmd_t rdd_vlan_cmd;
    rdd_bridge_pbits_cmd_t rdd_pbit_cmd;

    return vlan_action_map_to_rdd(mo, cfg->cmd, &rdd_vlan_cmd, &rdd_pbit_cmd);
}

/* Apply TPID map */
void rdpa_rdd_tpid_set(uint16_t tpid_array[], uint8_t size)
{
    /* Downstream */
    rdd_tpid_overwrite_table_cfg(tpid_array, rdpa_dir_ds);

    /* Upstream */
    rdd_tpid_overwrite_table_cfg(tpid_array, rdpa_dir_us);
}

/* Apply VLAN action */
bdmf_error_t rdpa_rdd_vlan_action_set(struct bdmf_object *mo, const rdpa_rdd_vlan_action_params_t *params,
    rdd_vlan_action_cl_t *cl)
{
    rdd_bridge_vlan_cmd_t rdd_vlan_cmd;
    rdd_bridge_pbits_cmd_t rdd_pbit_cmd;
    rdd_vlan_cmd_param_t vlan_params;
    bdmf_error_t err;
    int rdd_rc;

    vlan_action_map_to_rdd(mo, params->cfg.cmd, &rdd_vlan_cmd, &rdd_pbit_cmd);

    vlan_params.vlan_command_id = params->index;
    vlan_params.vlan_command = rdd_vlan_cmd;
    vlan_params.pbits_command = rdd_pbit_cmd;
    vlan_params.outer_vid = params->cfg.parm[RDPA_VLAN_TAG_OUT].vid;
    vlan_params.inner_vid = params->cfg.parm[RDPA_VLAN_TAG_IN].vid;
    vlan_params.outer_pbits = params->cfg.parm[RDPA_VLAN_TAG_OUT].pbit;
    vlan_params.inner_pbits = params->cfg.parm[RDPA_VLAN_TAG_IN].pbit;
    vlan_params.outer_tpid_overwrite_enable = params->outer_tpid_overwrite_enable;
    vlan_params.inner_tpid_overwrite_enable = params->inner_tpid_overwrite_enable;
    vlan_params.outer_tpid_id = params->outer_tpid_id;
    vlan_params.inner_tpid_id = params->inner_tpid_id;

    rdd_rc = rdd_vlan_cmd_cfg(params->dir, &vlan_params);
    err = (rdd_rc == 0) ? BDMF_ERR_OK : BDMF_ERR_INTERNAL;
    BDMF_TRACE_RET_OBJ(err, mo, "rdd_vlan_command_config(%d, %d, %d, %d, 0x%x, 0x%x, 0x%x, 0x%x, %x) -> %d\n",
        (int)params->dir, (int)params->index,
        (int)rdd_vlan_cmd, (int)rdd_pbit_cmd,
        (unsigned)vlan_params.inner_vid, (unsigned)vlan_params.outer_vid,
        (unsigned)vlan_params.inner_pbits, (unsigned)vlan_params.outer_pbits,
        (unsigned)vlan_params.outer_tpid_id, rdd_rc);

    return (rdd_rc == 0) ? BDMF_ERR_OK : BDMF_ERR_INTERNAL;
}
