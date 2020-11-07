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
#include "rdpa_vlan_action_ex.h"

#ifdef RDD_LAYER2_HEADER_HAS_BCM_TAG
#define RDPA_MAC_HEADER_LENGTH          16
#else
#define RDPA_MAC_HEADER_LENGTH          12
#endif
#define RDPA_FIRST_ETYPE_OFFSET         (RDD_PACKET_HEADROOM_OFFSET + RDPA_MAC_HEADER_LENGTH)

#define VLAN_ACTION_FLAG(bit)   (1 << (bit))

#define RDD_VLAN_ACTION_MAKE_OP(opcode, len, flags)     \
    ((opcode << GPE_COMMAND_OPCODE_F_OFFSET) |          \
     (len << GPE_COMMAND_LENGTH_F_OFFSET)    |          \
     (flags << GPE_COMMAND_OFFSET_F_OFFSET))

/* Validate VLAN command */
bdmf_error_t rdpa_rdd_vlan_action_validate(struct bdmf_object *mo, const rdpa_vlan_action_cfg_t *cfg)
{
#if !defined(BCM63158)
    uint32_t cmd = cfg->cmd;
    uint32_t vid_bits = cmd & RDPA_VLAN_CMD_GROUP_VID;

    if ((vid_bits == RDPA_VLAN_CMD_PUSH)               ||
        (vid_bits == RDPA_VLAN_CMD_PUSH2)              ||
        (vid_bits == RDPA_VLAN_CMD_PUSH_ALWAYS)        ||
        (vid_bits == RDPA_VLAN_CMD_PUSH2_ALWAYS)       ||
        (vid_bits == RDPA_VLAN_CMD_GROUP_PUSH_REPLACE) ||
        (vid_bits == RDPA_VLAN_CMD_GROUP_PUSH_ALWAYS_REPLACE) ||
        (vid_bits == RDPA_VLAN_CMD_GROUP_POP_REPLACE)  ||
        (vid_bits == RDPA_VLAN_CMD_POP)                ||
        (vid_bits == RDPA_VLAN_CMD_POP2)               ||
        (vid_bits == RDPA_VLAN_CMD_REPLACE)            ||
        (vid_bits == RDPA_VLAN_CMD_REPLACE2)           ||
        (vid_bits == RDPA_VLAN_CMD_TRANSPARENT))
        return BDMF_ERR_OK;

    BDMF_TRACE_ERR_OBJ(mo, "%08x: Command is not supported\n", cmd);
#endif

    return BDMF_ERR_NOT_SUPPORTED;
}

/* Apply TPID map */
void rdpa_rdd_tpid_set(uint16_t tpid_array[], uint8_t size)
{
}


#if !defined(BCM63158)
static bdmf_error_t _rdd_vlan_action_cl_add(rdd_vlan_action_cl_t *cl, uint16_t data)
{
    if (cl->num_commands + 1 >= RDPA_RDD_MAX_VLAN_ACTION_CL_LEN)
        return BDMF_ERR_OVERFLOW;
    cl->commands[cl->num_commands++] = data;
    return BDMF_ERR_OK;
}

static uint16_t _rdd_vlan_action_make_tag(const rdpa_vtag_cmd_parm_t *tag_parm)
{
    uint16_t tag = tag_parm->vid | (tag_parm->pbit << VLAN_TAG_PBITS_F_OFFSET);
    return tag;
}

static uint16_t _rdd_vlan_action_cl_cmd(rdd_vlan_action_cl_t *cl, rdd_vlan_action_gpe_opcode op,
    uint8_t offset, uint8_t length)
{
    bdmf_error_t err;
    uint32_t cmd = (op << GPE_COMMAND_OPCODE_F_OFFSET)  |
        (length << GPE_COMMAND_LENGTH_F_OFFSET)         |
        (offset << GPE_COMMAND_OFFSET_F_OFFSET);
    err = _rdd_vlan_action_cl_add(cl, cmd);
    return err;
}

/*
 * Per GPE command helpers
 */
static bdmf_error_t _rdd_vlan_action_dscp(rdd_vlan_action_cl_t *cl, uint8_t offset)
{
    bdmf_error_t err;
    err = _rdd_vlan_action_cl_cmd(cl, VLAN_ACTION_GPE_OPCODE_DSCP, offset, 2);
    return err;
}

static bdmf_error_t _rdd_vlan_action_mac_hdr_copy(rdd_vlan_action_cl_t *cl, int16_t tag_delta)
{
    bdmf_error_t err;
    uint16_t offset = RDD_PACKET_HEADROOM_OFFSET - 4 * tag_delta;

    err = _rdd_vlan_action_cl_cmd(cl, VLAN_ACTION_GPE_OPCODE_MAC_HDR_COPY, offset, 4);
    err = err ? err : _rdd_vlan_action_cl_add(cl, tag_delta > 0 ? tag_delta : -tag_delta);
    return err;
}

static bdmf_error_t _rdd_vlan_action_push_32(rdd_vlan_action_cl_t *cl, uint8_t *p_offset,
    uint16_t data1, uint16_t data2)
{
    bdmf_error_t err;
    *p_offset -= 4;
    err = _rdd_vlan_action_cl_cmd(cl, VLAN_ACTION_GPE_OPCODE_REPLACE_32, *p_offset, 6);
    err = err ? err : _rdd_vlan_action_cl_add(cl, data1);
    err = err ? err : _rdd_vlan_action_cl_add(cl, data2);
    return BDMF_ERR_OK;
}

static bdmf_error_t _rdd_vlan_action_pull_32(rdd_vlan_action_cl_t *cl, uint8_t *p_offset)
{
    *p_offset += 4;
    return BDMF_ERR_OK;
}

static bdmf_error_t _rdd_vlan_action_replace_16(rdd_vlan_action_cl_t *cl, uint8_t offset, uint16_t data)
{
    bdmf_error_t err;
    err = _rdd_vlan_action_cl_cmd(cl, VLAN_ACTION_GPE_OPCODE_REPLACE_16, offset, 4);
    err = err ? err : _rdd_vlan_action_cl_add(cl, data);
    return err;
}

static bdmf_error_t _rdd_vlan_action_replace_32(rdd_vlan_action_cl_t *cl, uint8_t offset,
    uint16_t data1, uint16_t data2)
{
    bdmf_error_t err;
    err = _rdd_vlan_action_cl_cmd(cl, VLAN_ACTION_GPE_OPCODE_REPLACE_32, offset, 6);
    err = err ? err : _rdd_vlan_action_cl_add(cl, data1);
    err = err ? err : _rdd_vlan_action_cl_add(cl, data2);
    return err;
}

static bdmf_error_t _rdd_vlan_action_copy_bits_16(rdd_vlan_action_cl_t *cl, uint8_t offset,
    uint8_t src_offset, uint16_t mask)
{
    bdmf_error_t err;
    err = _rdd_vlan_action_cl_cmd(cl, VLAN_ACTION_GPE_OPCODE_COPY_BITS_16, offset, 6);
    err = err ? err : _rdd_vlan_action_cl_add(cl, mask);
    err = err ? err : _rdd_vlan_action_cl_add(cl, src_offset);
    return err;
}

static bdmf_error_t _rdd_vlan_action_replace_bits_16(rdd_vlan_action_cl_t *cl, uint8_t offset,
    uint16_t value, uint16_t mask)
{
    bdmf_error_t err;
    err = _rdd_vlan_action_cl_cmd(cl, VLAN_ACTION_GPE_OPCODE_REPLACE_BITS_16, offset, 6);
    err = err ? err : _rdd_vlan_action_cl_add(cl, ~mask);
    err = err ? err : _rdd_vlan_action_cl_add(cl, value & mask);
    return err;
}

/*
 * Build CL for command categories
 */

/* REPLACE command helper */
static bdmf_error_t _rdd_vlan_action_replace_helper(const rdpa_rdd_vlan_action_params_t *params, rdd_vlan_action_cl_t *cl,
    uint8_t offset, int replace_tag_idx)
{
    uint32_t cmd = params->cfg.cmd;
    bdmf_error_t err = BDMF_ERR_OK;

    if (0 != (cmd & RDPA_VLAN_CMD_REMARK))
    {
        if (0 != (cmd & RDPA_VLAN_CMD_TPID_REMARK))
        {
            err = _rdd_vlan_action_replace_32(cl, offset,
                params->cfg.parm[replace_tag_idx].tpid,
                _rdd_vlan_action_make_tag(&params->cfg.parm[replace_tag_idx]));
        }
        else
        {
            err = _rdd_vlan_action_replace_16(cl, offset+2,
                _rdd_vlan_action_make_tag(&params->cfg.parm[replace_tag_idx]));
        }
    }
    else
    {
        if (0 != (cmd & RDPA_VLAN_CMD_TPID_REMARK))
            err = _rdd_vlan_action_replace_16(cl, offset, params->cfg.parm[replace_tag_idx].tpid);
        err = err ? err : _rdd_vlan_action_replace_bits_16(cl, offset+2,
            params->cfg.parm[replace_tag_idx].vid, 0xfff);
    }
    return err;
}

/* PUSH-untagged command helper */
static bdmf_error_t _rdd_vlan_action_push_untagged_helper(const rdpa_rdd_vlan_action_params_t *params, rdd_vlan_action_cl_t *cl,
    uint8_t *p_offset, int push_tag_idx)
{
    uint32_t cmd = params->cfg.cmd;
    uint8_t offset = *p_offset;
    bdmf_error_t err = BDMF_ERR_OK;

    err = _rdd_vlan_action_push_32(cl, &offset,
        params->cfg.parm[push_tag_idx].tpid,
        _rdd_vlan_action_make_tag(&params->cfg.parm[push_tag_idx]));
    /* If REMARK is not set, map from DHCP */
    if (0 == (cmd & (RDPA_VLAN_CMD_REMARK | RDPA_VLAN_CMD_PUSH_ALWAYS)))
        err = err ? err : _rdd_vlan_action_dscp(cl, offset + 2);
    *p_offset = offset;

    return err;
}

/* PUSH-PTAG command helper */
static bdmf_error_t _rdd_vlan_action_push_ptag_helper(const rdpa_rdd_vlan_action_params_t *params, rdd_vlan_action_cl_t *cl,
    uint8_t *p_offset)
{
    uint32_t cmd = params->cfg.cmd;
    uint8_t offset = *p_offset;
    bdmf_error_t err = BDMF_ERR_OK;
    int ptag_replace_idx;

    /* Replace P-TAG if not PUSH_ALWAYS command */
    if (0 == (cmd & RDPA_VLAN_CMD_PUSH_ALWAYS) && 0 == (cmd & RDPA_VLAN_CMD_PUSH2_ALWAYS))
    {
        if (0 != (cmd & RDPA_VLAN_CMD_REPLACE) || 0 != (cmd & RDPA_VLAN_CMD_PUSH2))
            ptag_replace_idx = RDPA_VLAN_TAG_IN;
        else
            ptag_replace_idx = RDPA_VLAN_TAG_OUT;
        err = _rdd_vlan_action_replace_helper(params, cl, offset, ptag_replace_idx);
    }
    if (0 != (cmd & RDPA_VLAN_CMD_REPLACE) || 0 != (cmd & RDPA_VLAN_CMD_PUSH_ALWAYS))
    {
        /* We've already replaced inner. Now add outer */
        err = err ? err : _rdd_vlan_action_push_32(cl, &offset,
            params->cfg.parm[RDPA_VLAN_TAG_OUT].tpid,
            _rdd_vlan_action_make_tag(&params->cfg.parm[RDPA_VLAN_TAG_OUT]));
        if (0 == (cmd & RDPA_VLAN_CMD_REMARK))
        {
            /* NOT REMARK */
            err = err ? err : _rdd_vlan_action_copy_bits_16(cl, offset + 2, offset + 6, 0xe000);   /* Copy P-BITS */
        }
    }
    *p_offset = offset;

    return err;
}

/* PUSH-QTAG command helper */
static bdmf_error_t _rdd_vlan_action_push_qtag_helper(const rdpa_rdd_vlan_action_params_t *params, rdd_vlan_action_cl_t *cl,
    uint8_t *p_offset, int push_tag_idx)
{
    uint32_t cmd = params->cfg.cmd;
    uint8_t offset = *p_offset;
    bdmf_error_t err = BDMF_ERR_OK;

    /*
     * Create 1 Q-TAG list
     */
    if (0 != (cmd & RDPA_VLAN_CMD_REPLACE))
        err = _rdd_vlan_action_replace_helper(params, cl, offset, RDPA_VLAN_TAG_IN);

    err = err ? err : _rdd_vlan_action_push_32(cl, &offset,
        params->cfg.parm[push_tag_idx].tpid,
        _rdd_vlan_action_make_tag(&params->cfg.parm[push_tag_idx]));
    if (0 != (cmd & RDPA_VLAN_CMD_DSCP))
    {
        /* DSCP */
        err = err ? err : _rdd_vlan_action_dscp(cl, offset + 2);
    }
    if (0 == (cmd & RDPA_VLAN_CMD_REMARK))
    {
        /* NOT REMARK */
        err = err ? err : _rdd_vlan_action_copy_bits_16(cl, offset + 2, offset + 6, 0xe000);   /* Copy P-BITS */
    }
    /* TPID remark on inner tag */
    if (0 != (cmd & RDPA_VLAN_CMD_TPID_REMARK) && 0 == (cmd & RDPA_VLAN_CMD_REPLACE))
    {
        if (params->cfg.parm[RDPA_VLAN_TAG_IN].tpid != (uint16_t)RDPA_VALUE_UNASSIGNED)
            err = err ? err : _rdd_vlan_action_replace_16(cl, offset + 4, params->cfg.parm[RDPA_VLAN_TAG_IN].tpid);
    }
    *p_offset = offset;

    return err;
}


/* Create command list for (PUSH + xxx) group
 * - PUSH
 * - PUSH_ALWAYS
 * - PUSH + REPLACE
 * - PUSH_ALWAYS + REPLACE
 * - +DSCP
 * - +REMARK
 * - +TPID_REMARK
 */
static bdmf_error_t _rdd_vlan_action_make_push(const rdpa_rdd_vlan_action_params_t *params, rdd_vlan_action_cl_t *cl)
{
    uint32_t cmd = params->cfg.cmd;
    uint8_t offset;
    bdmf_error_t err = BDMF_ERR_OK;

    /* Untagged */
    if (0 == (cmd & RDPA_VLAN_CMD_REPLACE))
    {
        cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_UNTAGGED] = 0;
        offset = RDPA_FIRST_ETYPE_OFFSET;
        err = _rdd_vlan_action_mac_hdr_copy(cl, 1);
        err = err ? err : _rdd_vlan_action_push_untagged_helper(params, cl, &offset, RDPA_VLAN_TAG_OUT);
        err = err ? err : _rdd_vlan_action_cl_add(cl, GPE_COMMAND_LIST_TERMINATOR);
    }
    else
    {
        cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_UNTAGGED] = VLAN_ACTION_CL_OFFSET_TRANSPARENT;
    }

    /* P-TAG */
    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_PTAG] = cl->num_commands*sizeof(uint16_t);
    offset = RDPA_FIRST_ETYPE_OFFSET;
    if (0 != (cmd & (RDPA_VLAN_CMD_PUSH_ALWAYS | RDPA_VLAN_CMD_REPLACE)))
        err = err ? err : _rdd_vlan_action_mac_hdr_copy(cl, 1);
    err = err ? err : _rdd_vlan_action_push_ptag_helper(params, cl, &offset);
    err = err ? err : _rdd_vlan_action_cl_add(cl, GPE_COMMAND_LIST_TERMINATOR);

    /* Q-TAG */
    if (0 == (cmd & RDPA_VLAN_CMD_REPLACE))
    {
        cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_1TAG] = cl->num_commands*sizeof(uint16_t);
        offset = RDPA_FIRST_ETYPE_OFFSET;
        err = _rdd_vlan_action_mac_hdr_copy(cl, 1);
        err = err ? err : _rdd_vlan_action_push_qtag_helper(params, cl, &offset, RDPA_VLAN_TAG_OUT);
        err = err ? err : _rdd_vlan_action_cl_add(cl, GPE_COMMAND_LIST_TERMINATOR);
    }
    else
    {
        /* In case of PUSH+REPLACE / PUSH_ALWAYS QTAG and PTAG command lists are the same */
        cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_1TAG] =
            cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_PTAG];
    }

    /* 2 tags */
    if (0 != (cmd & (RDPA_VLAN_CMD_PUSH_ALWAYS | RDPA_VLAN_CMD_REPLACE)))
    {
        cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_2TAGS] =
            cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_1TAG];
    }
    else
    {
        /* Can't PUSH 3rd tag */
        cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_2TAGS] = VLAN_ACTION_CL_OFFSET_TRANSPARENT;
    }
    return err;
}

/* Create command list for (PUSH2 + xxx) group
 * - PUSH2
 * - PUSH2_ALWAYS
 * - +DSCP
 * - +REMARK
 */
static bdmf_error_t _rdd_vlan_action_make_push2(const rdpa_rdd_vlan_action_params_t *params, rdd_vlan_action_cl_t *cl)
{
    uint32_t cmd = params->cfg.cmd;
    uint8_t offset;
    bdmf_error_t err;

    /* Untagged */
    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_UNTAGGED] = 0;
    offset = RDPA_FIRST_ETYPE_OFFSET;
    err = _rdd_vlan_action_mac_hdr_copy(cl, 2);
    err = err ? err : _rdd_vlan_action_push_untagged_helper(params, cl, &offset, RDPA_VLAN_TAG_IN);
    err = err ? err : _rdd_vlan_action_push_qtag_helper(params, cl, &offset, RDPA_VLAN_TAG_OUT);
    err = err ? err : _rdd_vlan_action_cl_add(cl, GPE_COMMAND_LIST_TERMINATOR);

    /* P-TAG */
    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_PTAG] = cl->num_commands*sizeof(uint16_t);
    offset = RDPA_FIRST_ETYPE_OFFSET;
    if (0 != (cmd & RDPA_VLAN_CMD_PUSH_ALWAYS))
        err = err ? err : _rdd_vlan_action_mac_hdr_copy(cl, 2);
    else
        err = err ? err : _rdd_vlan_action_mac_hdr_copy(cl, 1);
    err = err ? err : _rdd_vlan_action_push_ptag_helper(params, cl, &offset);
    err = err ? err : _rdd_vlan_action_push_qtag_helper(params, cl, &offset, RDPA_VLAN_TAG_OUT);
    err = err ? err : _rdd_vlan_action_cl_add(cl, GPE_COMMAND_LIST_TERMINATOR);

    /* Q-TAG */
    if (0 != (cmd & RDPA_VLAN_CMD_PUSH2_ALWAYS))
    {
        cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_1TAG] = cl->num_commands*sizeof(uint16_t);
        offset = RDPA_FIRST_ETYPE_OFFSET;
        err = err ? err : _rdd_vlan_action_mac_hdr_copy(cl, 2);
        err = err ? err : _rdd_vlan_action_push_qtag_helper(params, cl, &offset, RDPA_VLAN_TAG_IN);
        err = err ? err : _rdd_vlan_action_push_qtag_helper(params, cl, &offset, RDPA_VLAN_TAG_OUT);
        err = err ? err : _rdd_vlan_action_cl_add(cl, GPE_COMMAND_LIST_TERMINATOR);
    }
    else
    {
        /* Can't PUSH 3rd tag */
        cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_1TAG] = VLAN_ACTION_CL_OFFSET_TRANSPARENT;
    }

    /* 2 tags */
    if (0 != (cmd & RDPA_VLAN_CMD_PUSH2_ALWAYS))
    {
        cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_2TAGS] =
            cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_1TAG];
    }
    else
    {
        /* Can't PUSH 4rd tag */
        cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_2TAGS] = VLAN_ACTION_CL_OFFSET_TRANSPARENT;
    }
    return err;
}

/* Create command list for (POP + xxx) group
 * - POP
 * - POP + REPLACE
 * - +DSCP
 * - +REMARK
 * - +TPID_REMARK
 */
static bdmf_error_t _rdd_vlan_action_make_pop(const rdpa_rdd_vlan_action_params_t *params, rdd_vlan_action_cl_t *cl)
{
    uint32_t cmd = params->cfg.cmd;
    uint8_t offset = RDPA_FIRST_ETYPE_OFFSET;
    bdmf_error_t err = BDMF_ERR_OK;

    /* Untagged */
    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_UNTAGGED] = VLAN_ACTION_CL_OFFSET_TRANSPARENT;

    /* P-TAG */
    if (0 == (cmd & RDPA_VLAN_CMD_REPLACE) && 0 == (cmd & RDPA_VLAN_CMD_DSCP))
    {
        cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_PTAG] = cl->num_commands*sizeof(uint16_t);
        offset = RDPA_FIRST_ETYPE_OFFSET;
        err = _rdd_vlan_action_pull_32(cl, &offset);
        err = err ? err : _rdd_vlan_action_mac_hdr_copy(cl, -1);
        err = err ? err : _rdd_vlan_action_cl_add(cl, GPE_COMMAND_LIST_TERMINATOR);
    }
    else
    {
        cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_PTAG] = VLAN_ACTION_CL_OFFSET_TRANSPARENT;
    }

    /* Q-TAG */
    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_1TAG] = cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_PTAG];

    /* 2 TAGS */
    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_2TAGS] = cl->num_commands*sizeof(uint16_t);
    offset = RDPA_FIRST_ETYPE_OFFSET;
    /* DSCP here works as COPY command. P-BITS are copies from the outer
     * that is about to be stripped to the inner */
    if (0 != (cmd & RDPA_VLAN_CMD_DSCP))
        err = err ? err : _rdd_vlan_action_copy_bits_16(cl, offset + 6, offset + 2, 0xe000);   /* Copy P-BITS from outer to inner*/
    err = err ? err : _rdd_vlan_action_pull_32(cl, &offset);
    if (0 != (cmd & RDPA_VLAN_CMD_REPLACE))
        err = err ? err : _rdd_vlan_action_replace_helper(params, cl, offset, RDPA_VLAN_TAG_OUT);
    else if (0 != (cmd & RDPA_VLAN_CMD_TPID_REMARK))
        err = err ? err : _rdd_vlan_action_replace_16(cl, offset, params->cfg.parm[RDPA_VLAN_TAG_OUT].tpid);
    err = err ? err : _rdd_vlan_action_mac_hdr_copy(cl, -1);
    err = err ? err : _rdd_vlan_action_cl_add(cl, GPE_COMMAND_LIST_TERMINATOR);

    return err;
}

/* Create command list for POP2 command
 */
static bdmf_error_t _rdd_vlan_action_make_pop2(const rdpa_rdd_vlan_action_params_t *params, rdd_vlan_action_cl_t *cl)
{
    uint8_t offset = RDPA_FIRST_ETYPE_OFFSET;
    bdmf_error_t err = BDMF_ERR_OK;

    /* Untagged */
    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_UNTAGGED] = VLAN_ACTION_CL_OFFSET_TRANSPARENT;

    /* Priority Tagged */
    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_PTAG] = cl->num_commands*sizeof(uint16_t);
    offset = RDPA_FIRST_ETYPE_OFFSET;
    err = _rdd_vlan_action_pull_32(cl, &offset);
    err = err ? err : _rdd_vlan_action_mac_hdr_copy(cl, -1);
    err = err ? err : _rdd_vlan_action_cl_add(cl, GPE_COMMAND_LIST_TERMINATOR);

    /*Q-TAG */
    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_1TAG] = cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_PTAG];

    /* 2 TAGS */
    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_2TAGS] = cl->num_commands*sizeof(uint16_t);

    err = _rdd_vlan_action_pull_32(cl, &offset);
    err = err ? err : _rdd_vlan_action_pull_32(cl, &offset);
    err = err ? err : _rdd_vlan_action_mac_hdr_copy(cl, -2);
    err = err ? err : _rdd_vlan_action_cl_add(cl, GPE_COMMAND_LIST_TERMINATOR);
    return err;
}

/* Create command list for REPLACE command
 */
static bdmf_error_t _rdd_vlan_action_make_replace(const rdpa_rdd_vlan_action_params_t *params, rdd_vlan_action_cl_t *cl)
{
    uint32_t cmd = params->cfg.cmd;
    uint8_t offset;
    bdmf_error_t err = BDMF_ERR_OK;

    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_UNTAGGED] = 0;
    offset = RDPA_FIRST_ETYPE_OFFSET;
    err = _rdd_vlan_action_mac_hdr_copy(cl, 1);
    err = err ? err : _rdd_vlan_action_push_untagged_helper(params, cl, &offset, RDPA_VLAN_TAG_OUT);
    err = err ? err : _rdd_vlan_action_cl_add(cl, GPE_COMMAND_LIST_TERMINATOR);

    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_PTAG] = cl->num_commands*sizeof(uint16_t);
    offset = RDPA_FIRST_ETYPE_OFFSET;
    err = err ? err : _rdd_vlan_action_replace_helper(params, cl, offset, RDPA_VLAN_TAG_OUT);
    if (0 != (cmd & RDPA_VLAN_CMD_DSCP))
    {
        /* DSCP */
        err = err ? err : _rdd_vlan_action_dscp(cl, offset + 2);
    }
    err = err ? err : _rdd_vlan_action_cl_add(cl, GPE_COMMAND_LIST_TERMINATOR);

    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_1TAG] = cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_PTAG];

    /* 2 tags*/
    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_2TAGS] = cl->num_commands*sizeof(uint16_t);
    offset = RDPA_FIRST_ETYPE_OFFSET;
    err = err ? err : _rdd_vlan_action_replace_helper(params, cl, offset, RDPA_VLAN_TAG_OUT);
    if (0 != (cmd & RDPA_VLAN_CMD_DSCP))
    {
        /* DSCP case two tags -> copy pbits */
        err = err ? err : _rdd_vlan_action_copy_bits_16(cl, offset + 2, offset + 6, 0xe000);   /* Copy P-BITS */
    }
    err = err ? err : _rdd_vlan_action_cl_add(cl, GPE_COMMAND_LIST_TERMINATOR);

    return err;
}

/* Create command list for REPLACE2 command
 */
static bdmf_error_t _rdd_vlan_action_make_replace2(const rdpa_rdd_vlan_action_params_t *params, rdd_vlan_action_cl_t *cl)
{
    bdmf_error_t err = BDMF_ERR_OK;

    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_UNTAGGED] = VLAN_ACTION_CL_OFFSET_TRANSPARENT;

    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_PTAG] = VLAN_ACTION_CL_OFFSET_TRANSPARENT;

    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_1TAG] = VLAN_ACTION_CL_OFFSET_TRANSPARENT;

    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_2TAGS] = cl->num_commands*sizeof(uint16_t);
    err = _rdd_vlan_action_replace_helper(params, cl, RDPA_FIRST_ETYPE_OFFSET, RDPA_VLAN_TAG_OUT);
    err = err ? err : _rdd_vlan_action_replace_helper(params, cl, RDPA_FIRST_ETYPE_OFFSET + 4, RDPA_VLAN_TAG_IN);
    err = err ? err : _rdd_vlan_action_cl_add(cl, GPE_COMMAND_LIST_TERMINATOR);

    return err;
}

/* Create command list for DSCP command
 */
static bdmf_error_t _rdd_vlan_action_make_dscp(const rdpa_rdd_vlan_action_params_t *params, rdd_vlan_action_cl_t *cl)
{
    uint8_t offset;
    bdmf_error_t err = BDMF_ERR_OK;

    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_UNTAGGED] = 0;
    offset = RDPA_FIRST_ETYPE_OFFSET;
    err = _rdd_vlan_action_mac_hdr_copy(cl, 1);
    err = err ? err : _rdd_vlan_action_push_untagged_helper(params, cl, &offset, RDPA_VLAN_TAG_OUT);
    err = err ? err : _rdd_vlan_action_cl_add(cl, GPE_COMMAND_LIST_TERMINATOR);

    offset = RDPA_FIRST_ETYPE_OFFSET;
    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_PTAG] = cl->num_commands*sizeof(uint16_t);
    err = err ? err : _rdd_vlan_action_dscp(cl, offset + 2);
    err = err ? err : _rdd_vlan_action_cl_add(cl, GPE_COMMAND_LIST_TERMINATOR);

    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_1TAG] = cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_PTAG];

    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_2TAGS] = VLAN_ACTION_CL_OFFSET_TRANSPARENT;

    return err;
}

/* Create command list for remark command
 */
static bdmf_error_t _rdd_vlan_action_make_remark(const rdpa_rdd_vlan_action_params_t *params, rdd_vlan_action_cl_t *cl)
{
    uint8_t offset = RDPA_FIRST_ETYPE_OFFSET;
    bdmf_error_t err = BDMF_ERR_OK;

    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_UNTAGGED] = VLAN_ACTION_CL_OFFSET_TRANSPARENT;

    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_PTAG] = cl->num_commands*sizeof(uint16_t);
    err = _rdd_vlan_action_replace_bits_16(cl, offset+2,
        params->cfg.parm[RDPA_VLAN_TAG_OUT].pbit << 13, 0xe000);
    err = err ? err : _rdd_vlan_action_cl_add(cl, GPE_COMMAND_LIST_TERMINATOR);

    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_1TAG] = cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_PTAG];

    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_2TAGS] = cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_PTAG];

    return err;
}

/* Create command list for tpid_remark command
 */
static bdmf_error_t _rdd_vlan_action_make_tpid_remark(const rdpa_rdd_vlan_action_params_t *params, rdd_vlan_action_cl_t *cl)
{
    uint8_t offset = RDPA_FIRST_ETYPE_OFFSET;
    bdmf_error_t err = BDMF_ERR_OK;

    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_UNTAGGED] = VLAN_ACTION_CL_OFFSET_TRANSPARENT;

    if (params->cfg.parm[RDPA_VLAN_TAG_OUT].tpid != (uint16_t)RDPA_VALUE_UNASSIGNED)
    {
        cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_PTAG] = cl->num_commands*sizeof(uint16_t);
        err = _rdd_vlan_action_replace_16(cl, offset, params->cfg.parm[RDPA_VLAN_TAG_OUT].tpid);
        err = err ? err : _rdd_vlan_action_cl_add(cl, GPE_COMMAND_LIST_TERMINATOR);
    }
    else
    {
        cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_PTAG] = VLAN_ACTION_CL_OFFSET_TRANSPARENT;
    }

    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_1TAG] = cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_PTAG];

    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_2TAGS] = cl->num_commands*sizeof(uint16_t);
    if (params->cfg.parm[RDPA_VLAN_TAG_OUT].tpid != (uint16_t)RDPA_VALUE_UNASSIGNED)
        err = _rdd_vlan_action_replace_16(cl, offset, params->cfg.parm[RDPA_VLAN_TAG_OUT].tpid);
    if (params->cfg.parm[RDPA_VLAN_TAG_IN].tpid != (uint16_t)RDPA_VALUE_UNASSIGNED)
        err = _rdd_vlan_action_replace_16(cl, offset+4, params->cfg.parm[RDPA_VLAN_TAG_IN].tpid);
    err = err ? err : _rdd_vlan_action_cl_add(cl, GPE_COMMAND_LIST_TERMINATOR);

    return err;
}

/* Create command list for TRANSPARENT command.
 * Usually it is not needed because transparent action is not stored in NATC.
 * However, we can get here due to race condition, when vlan_action is removed,
 * but ingress classifier is not yet updated
 */
static bdmf_error_t _rdd_vlan_action_make_transparent(rdd_vlan_action_cl_t *cl)
{
    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_UNTAGGED] = VLAN_ACTION_CL_OFFSET_TRANSPARENT;
    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_PTAG] = VLAN_ACTION_CL_OFFSET_TRANSPARENT;
    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_1TAG] = VLAN_ACTION_CL_OFFSET_TRANSPARENT;
    cl->tag_state_cl_offset[VLAN_ACTION_CL_OFFSET_2TAGS] = VLAN_ACTION_CL_OFFSET_TRANSPARENT;
    return BDMF_ERR_OK;
}
#endif

/* Get DROP action command list  */
bdmf_error_t rdpa_vlan_action_drop_cl_get(rdd_vlan_action_cl_t *cl)
{
#if !defined(BCM63158)
    uint16_t op;
    /* Create command list */
    cl->num_commands = 0;
    op = RDD_VLAN_ACTION_MAKE_OP(VLAN_ACTION_GPE_OPCODE_DROP, 2, 0);
    _rdd_vlan_action_cl_add(cl, op);
    _rdd_vlan_action_cl_add(cl, GPE_COMMAND_LIST_TERMINATOR);
#endif
    return 0;
}


/* Apply VLAN action */
bdmf_error_t rdpa_rdd_vlan_action_set(struct bdmf_object *mo, const rdpa_rdd_vlan_action_params_t *params,
    rdd_vlan_action_cl_t *cl)
{
#if defined(BCM63158)
    return BDMF_ERR_NOT_SUPPORTED;
#else
    uint32_t cmd = params->cfg.cmd;
    bdmf_error_t err = BDMF_ERR_OK;

    /* Create command list */
    cl->num_commands = 0;

    /* If VLAN command is one of PUSH family and PBIT command is transparent, force it to be COPY */
    if (0 != (cmd & (RDPA_VLAN_CMD_PUSH | RDPA_VLAN_CMD_PUSH_ALWAYS)))
    {
        err = _rdd_vlan_action_make_push(params, cl);
    }
    else if (0 != (cmd & (RDPA_VLAN_CMD_PUSH2 | RDPA_VLAN_CMD_PUSH2_ALWAYS)))
    {
        err = _rdd_vlan_action_make_push2(params, cl);
    }
    else if (0 != (cmd & RDPA_VLAN_CMD_POP))
    {
        err = _rdd_vlan_action_make_pop(params, cl);
    }
    else if (0 != (cmd & RDPA_VLAN_CMD_POP2))
    {
        err = _rdd_vlan_action_make_pop2(params, cl);
    }
    else if (0 != (cmd & RDPA_VLAN_CMD_REPLACE))
    {
        err = _rdd_vlan_action_make_replace(params, cl);
    }
    else if (0 != (cmd & RDPA_VLAN_CMD_REPLACE2))
    {
        err = _rdd_vlan_action_make_replace2(params, cl);
    }
    else if (0 != (cmd & RDPA_VLAN_CMD_DSCP))
    {
        err = _rdd_vlan_action_make_dscp(params, cl);
    }
    else if (0 != (cmd & RDPA_VLAN_CMD_REMARK))
    {
        err = _rdd_vlan_action_make_remark(params, cl);
    }
    else if (0 != (cmd & (RDPA_VLAN_CMD_TPID_REMARK | RDPA_VLAN_CMD_TPID)))
    {
        err = _rdd_vlan_action_make_tpid_remark(params, cl);
    }
    else if (!cmd)
    {
        /* We can be here due to race condition. Transparent action will be
         * removed from NATC shortly
         */
        err = _rdd_vlan_action_make_transparent(cl);
    }
    else
    {
        err = BDMF_ERR_NOT_SUPPORTED;
    }

    BDMF_TRACE_RET_OBJ(err, mo, "%d/%d rdd_vlan_action_config({%d, 0x%x, 0x%x, 0x%x, 0x%x, %x}, %u) -> %d\n",
        (int)params->dir, (int)params->index,
        (int)params->cfg.cmd,
        (unsigned)params->cfg.parm[RDPA_VLAN_TAG_IN].vid, (unsigned)params->cfg.parm[RDPA_VLAN_TAG_OUT].vid,
        (unsigned)params->cfg.parm[RDPA_VLAN_TAG_IN].pbit, (unsigned)params->cfg.parm[RDPA_VLAN_TAG_OUT].pbit,
        (unsigned)params->outer_tpid_id,
        cl->num_commands, err);

    return err;
#endif
}

