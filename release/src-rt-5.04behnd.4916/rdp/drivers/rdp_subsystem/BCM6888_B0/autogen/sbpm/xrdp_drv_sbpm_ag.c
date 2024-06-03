/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

:>
*/


#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_sbpm_ag.h"

#define BLOCK_ADDR_COUNT_BITS 0
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_sbpm_nack_mask_get(uint32_t *sbpm_nack_mask_high, uint32_t *sbpm_nack_mask_low)
{
    uint32_t reg_regs_sbpm_nack_mask_high;
    uint32_t reg_regs_sbpm_nack_mask_low;

#ifdef VALIDATE_PARMS
    if (!sbpm_nack_mask_high || !sbpm_nack_mask_low)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_NACK_MASK_HIGH, reg_regs_sbpm_nack_mask_high);
    RU_REG_READ(0, SBPM, REGS_SBPM_NACK_MASK_LOW, reg_regs_sbpm_nack_mask_low);

    *sbpm_nack_mask_high = RU_FIELD_GET(0, SBPM, REGS_SBPM_NACK_MASK_HIGH, SBPM_NACK_MASK_HIGH, reg_regs_sbpm_nack_mask_high);
    *sbpm_nack_mask_low = RU_FIELD_GET(0, SBPM, REGS_SBPM_NACK_MASK_LOW, SBPM_NACK_MASK_LOW, reg_regs_sbpm_nack_mask_low);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_bac_get(uint16_t *bac, uint16_t *ug1bac, uint16_t *ug0bac)
{
    uint32_t reg_regs_sbpm_gl_bac;
    uint32_t reg_regs_sbpm_ug1_bac;
    uint32_t reg_regs_sbpm_ug0_bac;

#ifdef VALIDATE_PARMS
    if (!bac || !ug1bac || !ug0bac)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_GL_BAC, reg_regs_sbpm_gl_bac);
    RU_REG_READ(0, SBPM, REGS_SBPM_UG1_BAC, reg_regs_sbpm_ug1_bac);
    RU_REG_READ(0, SBPM, REGS_SBPM_UG0_BAC, reg_regs_sbpm_ug0_bac);

    *bac = RU_FIELD_GET(0, SBPM, REGS_SBPM_GL_BAC, BAC, reg_regs_sbpm_gl_bac);
    *ug1bac = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG1_BAC, UG1BAC, reg_regs_sbpm_ug1_bac);
    *ug0bac = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG0_BAC, UG0BAC, reg_regs_sbpm_ug0_bac);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_init_free_list_set(uint16_t init_base_addr, uint16_t init_offset, bdmf_boolean bsy, bdmf_boolean rdy)
{
    uint32_t reg_regs_init_free_list = 0;

#ifdef VALIDATE_PARMS
    if ((init_base_addr >= _14BITS_MAX_VAL_) ||
       (init_offset >= _14BITS_MAX_VAL_) ||
       (bsy >= _1BITS_MAX_VAL_) ||
       (rdy >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_init_free_list = RU_FIELD_SET(0, SBPM, REGS_INIT_FREE_LIST, INIT_BASE_ADDR, reg_regs_init_free_list, init_base_addr);
    reg_regs_init_free_list = RU_FIELD_SET(0, SBPM, REGS_INIT_FREE_LIST, INIT_OFFSET, reg_regs_init_free_list, init_offset);
    reg_regs_init_free_list = RU_FIELD_SET(0, SBPM, REGS_INIT_FREE_LIST, BSY, reg_regs_init_free_list, bsy);
    reg_regs_init_free_list = RU_FIELD_SET(0, SBPM, REGS_INIT_FREE_LIST, RDY, reg_regs_init_free_list, rdy);

    RU_REG_WRITE(0, SBPM, REGS_INIT_FREE_LIST, reg_regs_init_free_list);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_init_free_list_get(uint16_t *init_base_addr, uint16_t *init_offset, bdmf_boolean *bsy, bdmf_boolean *rdy)
{
    uint32_t reg_regs_init_free_list;

#ifdef VALIDATE_PARMS
    if (!init_base_addr || !init_offset || !bsy || !rdy)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_INIT_FREE_LIST, reg_regs_init_free_list);

    *init_base_addr = RU_FIELD_GET(0, SBPM, REGS_INIT_FREE_LIST, INIT_BASE_ADDR, reg_regs_init_free_list);
    *init_offset = RU_FIELD_GET(0, SBPM, REGS_INIT_FREE_LIST, INIT_OFFSET, reg_regs_init_free_list);
    *bsy = RU_FIELD_GET(0, SBPM, REGS_INIT_FREE_LIST, BSY, reg_regs_init_free_list);
    *rdy = RU_FIELD_GET(0, SBPM, REGS_INIT_FREE_LIST, RDY, reg_regs_init_free_list);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_bn_alloc_set(uint8_t sa)
{
    uint32_t reg_regs_bn_alloc = 0;

#ifdef VALIDATE_PARMS
    if ((sa >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_bn_alloc = RU_FIELD_SET(0, SBPM, REGS_BN_ALLOC, SA, reg_regs_bn_alloc, sa);

    RU_REG_WRITE(0, SBPM, REGS_BN_ALLOC, reg_regs_bn_alloc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_bn_alloc_get(uint8_t *sa)
{
    uint32_t reg_regs_bn_alloc;

#ifdef VALIDATE_PARMS
    if (!sa)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_BN_ALLOC, reg_regs_bn_alloc);

    *sa = RU_FIELD_GET(0, SBPM, REGS_BN_ALLOC, SA, reg_regs_bn_alloc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_bn_alloc_rply_get(sbpm_regs_bn_alloc_rply *regs_bn_alloc_rply)
{
    uint32_t reg_regs_bn_alloc_rply;

#ifdef VALIDATE_PARMS
    if (!regs_bn_alloc_rply)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_BN_ALLOC_RPLY, reg_regs_bn_alloc_rply);

    regs_bn_alloc_rply->alloc_bn_valid = RU_FIELD_GET(0, SBPM, REGS_BN_ALLOC_RPLY, ALLOC_BN_VALID, reg_regs_bn_alloc_rply);
    regs_bn_alloc_rply->alloc_bn = RU_FIELD_GET(0, SBPM, REGS_BN_ALLOC_RPLY, ALLOC_BN, reg_regs_bn_alloc_rply);
    regs_bn_alloc_rply->ack = RU_FIELD_GET(0, SBPM, REGS_BN_ALLOC_RPLY, ACK, reg_regs_bn_alloc_rply);
    regs_bn_alloc_rply->nack = RU_FIELD_GET(0, SBPM, REGS_BN_ALLOC_RPLY, NACK, reg_regs_bn_alloc_rply);
    regs_bn_alloc_rply->excl_high = RU_FIELD_GET(0, SBPM, REGS_BN_ALLOC_RPLY, EXCL_HIGH, reg_regs_bn_alloc_rply);
    regs_bn_alloc_rply->excl_low = RU_FIELD_GET(0, SBPM, REGS_BN_ALLOC_RPLY, EXCL_LOW, reg_regs_bn_alloc_rply);
    regs_bn_alloc_rply->busy = RU_FIELD_GET(0, SBPM, REGS_BN_ALLOC_RPLY, BUSY, reg_regs_bn_alloc_rply);
    regs_bn_alloc_rply->rdy = RU_FIELD_GET(0, SBPM, REGS_BN_ALLOC_RPLY, RDY, reg_regs_bn_alloc_rply);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_bn_free_with_contxt_low_set(uint16_t head_bn, uint8_t sa, uint8_t offset, bdmf_boolean ack)
{
    uint32_t reg_regs_bn_free_with_contxt_low = 0;

#ifdef VALIDATE_PARMS
    if ((head_bn >= _14BITS_MAX_VAL_) ||
       (sa >= _6BITS_MAX_VAL_) ||
       (offset >= _7BITS_MAX_VAL_) ||
       (ack >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_bn_free_with_contxt_low = RU_FIELD_SET(0, SBPM, REGS_BN_FREE_WITH_CONTXT_LOW, HEAD_BN, reg_regs_bn_free_with_contxt_low, head_bn);
    reg_regs_bn_free_with_contxt_low = RU_FIELD_SET(0, SBPM, REGS_BN_FREE_WITH_CONTXT_LOW, SA, reg_regs_bn_free_with_contxt_low, sa);
    reg_regs_bn_free_with_contxt_low = RU_FIELD_SET(0, SBPM, REGS_BN_FREE_WITH_CONTXT_LOW, OFFSET, reg_regs_bn_free_with_contxt_low, offset);
    reg_regs_bn_free_with_contxt_low = RU_FIELD_SET(0, SBPM, REGS_BN_FREE_WITH_CONTXT_LOW, ACK, reg_regs_bn_free_with_contxt_low, ack);

    RU_REG_WRITE(0, SBPM, REGS_BN_FREE_WITH_CONTXT_LOW, reg_regs_bn_free_with_contxt_low);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_bn_free_with_contxt_low_get(uint16_t *head_bn, uint8_t *sa, uint8_t *offset, bdmf_boolean *ack)
{
    uint32_t reg_regs_bn_free_with_contxt_low;

#ifdef VALIDATE_PARMS
    if (!head_bn || !sa || !offset || !ack)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_BN_FREE_WITH_CONTXT_LOW, reg_regs_bn_free_with_contxt_low);

    *head_bn = RU_FIELD_GET(0, SBPM, REGS_BN_FREE_WITH_CONTXT_LOW, HEAD_BN, reg_regs_bn_free_with_contxt_low);
    *sa = RU_FIELD_GET(0, SBPM, REGS_BN_FREE_WITH_CONTXT_LOW, SA, reg_regs_bn_free_with_contxt_low);
    *offset = RU_FIELD_GET(0, SBPM, REGS_BN_FREE_WITH_CONTXT_LOW, OFFSET, reg_regs_bn_free_with_contxt_low);
    *ack = RU_FIELD_GET(0, SBPM, REGS_BN_FREE_WITH_CONTXT_LOW, ACK, reg_regs_bn_free_with_contxt_low);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_bn_free_with_contxt_high_set(uint16_t last_bn)
{
    uint32_t reg_regs_bn_free_with_contxt_high = 0;

#ifdef VALIDATE_PARMS
    if ((last_bn >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_bn_free_with_contxt_high = RU_FIELD_SET(0, SBPM, REGS_BN_FREE_WITH_CONTXT_HIGH, LAST_BN, reg_regs_bn_free_with_contxt_high, last_bn);

    RU_REG_WRITE(0, SBPM, REGS_BN_FREE_WITH_CONTXT_HIGH, reg_regs_bn_free_with_contxt_high);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_bn_free_with_contxt_high_get(uint16_t *last_bn)
{
    uint32_t reg_regs_bn_free_with_contxt_high;

#ifdef VALIDATE_PARMS
    if (!last_bn)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_BN_FREE_WITH_CONTXT_HIGH, reg_regs_bn_free_with_contxt_high);

    *last_bn = RU_FIELD_GET(0, SBPM, REGS_BN_FREE_WITH_CONTXT_HIGH, LAST_BN, reg_regs_bn_free_with_contxt_high);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_mcst_inc_set(uint16_t bn, uint8_t mcst_val, bdmf_boolean ack_req)
{
    uint32_t reg_regs_mcst_inc = 0;

#ifdef VALIDATE_PARMS
    if ((bn >= _14BITS_MAX_VAL_) ||
       (ack_req >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_mcst_inc = RU_FIELD_SET(0, SBPM, REGS_MCST_INC, BN, reg_regs_mcst_inc, bn);
    reg_regs_mcst_inc = RU_FIELD_SET(0, SBPM, REGS_MCST_INC, MCST_VAL, reg_regs_mcst_inc, mcst_val);
    reg_regs_mcst_inc = RU_FIELD_SET(0, SBPM, REGS_MCST_INC, ACK_REQ, reg_regs_mcst_inc, ack_req);

    RU_REG_WRITE(0, SBPM, REGS_MCST_INC, reg_regs_mcst_inc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_mcst_inc_get(uint16_t *bn, uint8_t *mcst_val, bdmf_boolean *ack_req)
{
    uint32_t reg_regs_mcst_inc;

#ifdef VALIDATE_PARMS
    if (!bn || !mcst_val || !ack_req)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_MCST_INC, reg_regs_mcst_inc);

    *bn = RU_FIELD_GET(0, SBPM, REGS_MCST_INC, BN, reg_regs_mcst_inc);
    *mcst_val = RU_FIELD_GET(0, SBPM, REGS_MCST_INC, MCST_VAL, reg_regs_mcst_inc);
    *ack_req = RU_FIELD_GET(0, SBPM, REGS_MCST_INC, ACK_REQ, reg_regs_mcst_inc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_mcst_inc_rply_get(bdmf_boolean *mcst_ack, bdmf_boolean *bsy, bdmf_boolean *rdy)
{
    uint32_t reg_regs_mcst_inc_rply;

#ifdef VALIDATE_PARMS
    if (!mcst_ack || !bsy || !rdy)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_MCST_INC_RPLY, reg_regs_mcst_inc_rply);

    *mcst_ack = RU_FIELD_GET(0, SBPM, REGS_MCST_INC_RPLY, MCST_ACK, reg_regs_mcst_inc_rply);
    *bsy = RU_FIELD_GET(0, SBPM, REGS_MCST_INC_RPLY, BSY, reg_regs_mcst_inc_rply);
    *rdy = RU_FIELD_GET(0, SBPM, REGS_MCST_INC_RPLY, RDY, reg_regs_mcst_inc_rply);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_bn_connect_set(uint16_t bn, bdmf_boolean ack_req, bdmf_boolean wr_req, uint16_t pointed_bn)
{
    uint32_t reg_regs_bn_connect = 0;

#ifdef VALIDATE_PARMS
    if ((bn >= _14BITS_MAX_VAL_) ||
       (ack_req >= _1BITS_MAX_VAL_) ||
       (wr_req >= _1BITS_MAX_VAL_) ||
       (pointed_bn >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_bn_connect = RU_FIELD_SET(0, SBPM, REGS_BN_CONNECT, BN, reg_regs_bn_connect, bn);
    reg_regs_bn_connect = RU_FIELD_SET(0, SBPM, REGS_BN_CONNECT, ACK_REQ, reg_regs_bn_connect, ack_req);
    reg_regs_bn_connect = RU_FIELD_SET(0, SBPM, REGS_BN_CONNECT, WR_REQ, reg_regs_bn_connect, wr_req);
    reg_regs_bn_connect = RU_FIELD_SET(0, SBPM, REGS_BN_CONNECT, POINTED_BN, reg_regs_bn_connect, pointed_bn);

    RU_REG_WRITE(0, SBPM, REGS_BN_CONNECT, reg_regs_bn_connect);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_bn_connect_get(uint16_t *bn, bdmf_boolean *ack_req, bdmf_boolean *wr_req, uint16_t *pointed_bn)
{
    uint32_t reg_regs_bn_connect;

#ifdef VALIDATE_PARMS
    if (!bn || !ack_req || !wr_req || !pointed_bn)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_BN_CONNECT, reg_regs_bn_connect);

    *bn = RU_FIELD_GET(0, SBPM, REGS_BN_CONNECT, BN, reg_regs_bn_connect);
    *ack_req = RU_FIELD_GET(0, SBPM, REGS_BN_CONNECT, ACK_REQ, reg_regs_bn_connect);
    *wr_req = RU_FIELD_GET(0, SBPM, REGS_BN_CONNECT, WR_REQ, reg_regs_bn_connect);
    *pointed_bn = RU_FIELD_GET(0, SBPM, REGS_BN_CONNECT, POINTED_BN, reg_regs_bn_connect);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_bn_connect_rply_get(bdmf_boolean *connect_ack, bdmf_boolean *busy, bdmf_boolean *rdy)
{
    uint32_t reg_regs_bn_connect_rply;

#ifdef VALIDATE_PARMS
    if (!connect_ack || !busy || !rdy)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_BN_CONNECT_RPLY, reg_regs_bn_connect_rply);

    *connect_ack = RU_FIELD_GET(0, SBPM, REGS_BN_CONNECT_RPLY, CONNECT_ACK, reg_regs_bn_connect_rply);
    *busy = RU_FIELD_GET(0, SBPM, REGS_BN_CONNECT_RPLY, BUSY, reg_regs_bn_connect_rply);
    *rdy = RU_FIELD_GET(0, SBPM, REGS_BN_CONNECT_RPLY, RDY, reg_regs_bn_connect_rply);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_get_next_set(uint16_t bn)
{
    uint32_t reg_regs_get_next = 0;

#ifdef VALIDATE_PARMS
    if ((bn >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_get_next = RU_FIELD_SET(0, SBPM, REGS_GET_NEXT, BN, reg_regs_get_next, bn);

    RU_REG_WRITE(0, SBPM, REGS_GET_NEXT, reg_regs_get_next);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_get_next_get(uint16_t *bn)
{
    uint32_t reg_regs_get_next;

#ifdef VALIDATE_PARMS
    if (!bn)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_GET_NEXT, reg_regs_get_next);

    *bn = RU_FIELD_GET(0, SBPM, REGS_GET_NEXT, BN, reg_regs_get_next);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_get_next_rply_get(sbpm_regs_get_next_rply *regs_get_next_rply)
{
    uint32_t reg_regs_get_next_rply;

#ifdef VALIDATE_PARMS
    if (!regs_get_next_rply)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_GET_NEXT_RPLY, reg_regs_get_next_rply);

    regs_get_next_rply->bn_valid = RU_FIELD_GET(0, SBPM, REGS_GET_NEXT_RPLY, BN_VALID, reg_regs_get_next_rply);
    regs_get_next_rply->next_bn = RU_FIELD_GET(0, SBPM, REGS_GET_NEXT_RPLY, NEXT_BN, reg_regs_get_next_rply);
    regs_get_next_rply->bn_null = RU_FIELD_GET(0, SBPM, REGS_GET_NEXT_RPLY, BN_NULL, reg_regs_get_next_rply);
    regs_get_next_rply->mcnt_val = RU_FIELD_GET(0, SBPM, REGS_GET_NEXT_RPLY, MCNT_VAL, reg_regs_get_next_rply);
    regs_get_next_rply->busy = RU_FIELD_GET(0, SBPM, REGS_GET_NEXT_RPLY, BUSY, reg_regs_get_next_rply);
    regs_get_next_rply->rdy = RU_FIELD_GET(0, SBPM, REGS_GET_NEXT_RPLY, RDY, reg_regs_get_next_rply);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_clk_gate_cntrl_set(const sbpm_regs_sbpm_clk_gate_cntrl *regs_sbpm_clk_gate_cntrl)
{
    uint32_t reg_regs_sbpm_clk_gate_cntrl = 0;

#ifdef VALIDATE_PARMS
    if(!regs_sbpm_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((regs_sbpm_clk_gate_cntrl->bypass_clk_gate >= _1BITS_MAX_VAL_) ||
       (regs_sbpm_clk_gate_cntrl->keep_alive_en >= _1BITS_MAX_VAL_) ||
       (regs_sbpm_clk_gate_cntrl->keep_alive_intervl >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_sbpm_clk_gate_cntrl = RU_FIELD_SET(0, SBPM, REGS_SBPM_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_regs_sbpm_clk_gate_cntrl, regs_sbpm_clk_gate_cntrl->bypass_clk_gate);
    reg_regs_sbpm_clk_gate_cntrl = RU_FIELD_SET(0, SBPM, REGS_SBPM_CLK_GATE_CNTRL, TIMER_VAL, reg_regs_sbpm_clk_gate_cntrl, regs_sbpm_clk_gate_cntrl->timer_val);
    reg_regs_sbpm_clk_gate_cntrl = RU_FIELD_SET(0, SBPM, REGS_SBPM_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_regs_sbpm_clk_gate_cntrl, regs_sbpm_clk_gate_cntrl->keep_alive_en);
    reg_regs_sbpm_clk_gate_cntrl = RU_FIELD_SET(0, SBPM, REGS_SBPM_CLK_GATE_CNTRL, KEEP_ALIVE_INTERVL, reg_regs_sbpm_clk_gate_cntrl, regs_sbpm_clk_gate_cntrl->keep_alive_intervl);
    reg_regs_sbpm_clk_gate_cntrl = RU_FIELD_SET(0, SBPM, REGS_SBPM_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_regs_sbpm_clk_gate_cntrl, regs_sbpm_clk_gate_cntrl->keep_alive_cyc);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_CLK_GATE_CNTRL, reg_regs_sbpm_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_clk_gate_cntrl_get(sbpm_regs_sbpm_clk_gate_cntrl *regs_sbpm_clk_gate_cntrl)
{
    uint32_t reg_regs_sbpm_clk_gate_cntrl;

#ifdef VALIDATE_PARMS
    if (!regs_sbpm_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_CLK_GATE_CNTRL, reg_regs_sbpm_clk_gate_cntrl);

    regs_sbpm_clk_gate_cntrl->bypass_clk_gate = RU_FIELD_GET(0, SBPM, REGS_SBPM_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_regs_sbpm_clk_gate_cntrl);
    regs_sbpm_clk_gate_cntrl->timer_val = RU_FIELD_GET(0, SBPM, REGS_SBPM_CLK_GATE_CNTRL, TIMER_VAL, reg_regs_sbpm_clk_gate_cntrl);
    regs_sbpm_clk_gate_cntrl->keep_alive_en = RU_FIELD_GET(0, SBPM, REGS_SBPM_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_regs_sbpm_clk_gate_cntrl);
    regs_sbpm_clk_gate_cntrl->keep_alive_intervl = RU_FIELD_GET(0, SBPM, REGS_SBPM_CLK_GATE_CNTRL, KEEP_ALIVE_INTERVL, reg_regs_sbpm_clk_gate_cntrl);
    regs_sbpm_clk_gate_cntrl->keep_alive_cyc = RU_FIELD_GET(0, SBPM, REGS_SBPM_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_regs_sbpm_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_bn_free_without_contxt_set(uint16_t head_bn, uint8_t sa, bdmf_boolean ack_req)
{
    uint32_t reg_regs_bn_free_without_contxt = 0;

#ifdef VALIDATE_PARMS
    if ((head_bn >= _14BITS_MAX_VAL_) ||
       (sa >= _6BITS_MAX_VAL_) ||
       (ack_req >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_bn_free_without_contxt = RU_FIELD_SET(0, SBPM, REGS_BN_FREE_WITHOUT_CONTXT, HEAD_BN, reg_regs_bn_free_without_contxt, head_bn);
    reg_regs_bn_free_without_contxt = RU_FIELD_SET(0, SBPM, REGS_BN_FREE_WITHOUT_CONTXT, SA, reg_regs_bn_free_without_contxt, sa);
    reg_regs_bn_free_without_contxt = RU_FIELD_SET(0, SBPM, REGS_BN_FREE_WITHOUT_CONTXT, ACK_REQ, reg_regs_bn_free_without_contxt, ack_req);

    RU_REG_WRITE(0, SBPM, REGS_BN_FREE_WITHOUT_CONTXT, reg_regs_bn_free_without_contxt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_bn_free_without_contxt_get(uint16_t *head_bn, uint8_t *sa, bdmf_boolean *ack_req)
{
    uint32_t reg_regs_bn_free_without_contxt;

#ifdef VALIDATE_PARMS
    if (!head_bn || !sa || !ack_req)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_BN_FREE_WITHOUT_CONTXT, reg_regs_bn_free_without_contxt);

    *head_bn = RU_FIELD_GET(0, SBPM, REGS_BN_FREE_WITHOUT_CONTXT, HEAD_BN, reg_regs_bn_free_without_contxt);
    *sa = RU_FIELD_GET(0, SBPM, REGS_BN_FREE_WITHOUT_CONTXT, SA, reg_regs_bn_free_without_contxt);
    *ack_req = RU_FIELD_GET(0, SBPM, REGS_BN_FREE_WITHOUT_CONTXT, ACK_REQ, reg_regs_bn_free_without_contxt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_bn_free_without_contxt_rply_get(sbpm_regs_bn_free_without_contxt_rply *regs_bn_free_without_contxt_rply)
{
    uint32_t reg_regs_bn_free_without_contxt_rply;

#ifdef VALIDATE_PARMS
    if (!regs_bn_free_without_contxt_rply)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_BN_FREE_WITHOUT_CONTXT_RPLY, reg_regs_bn_free_without_contxt_rply);

    regs_bn_free_without_contxt_rply->free_ack = RU_FIELD_GET(0, SBPM, REGS_BN_FREE_WITHOUT_CONTXT_RPLY, FREE_ACK, reg_regs_bn_free_without_contxt_rply);
    regs_bn_free_without_contxt_rply->ack_stat = RU_FIELD_GET(0, SBPM, REGS_BN_FREE_WITHOUT_CONTXT_RPLY, ACK_STAT, reg_regs_bn_free_without_contxt_rply);
    regs_bn_free_without_contxt_rply->nack_stat = RU_FIELD_GET(0, SBPM, REGS_BN_FREE_WITHOUT_CONTXT_RPLY, NACK_STAT, reg_regs_bn_free_without_contxt_rply);
    regs_bn_free_without_contxt_rply->excl_high_stat = RU_FIELD_GET(0, SBPM, REGS_BN_FREE_WITHOUT_CONTXT_RPLY, EXCL_HIGH_STAT, reg_regs_bn_free_without_contxt_rply);
    regs_bn_free_without_contxt_rply->excl_low_stat = RU_FIELD_GET(0, SBPM, REGS_BN_FREE_WITHOUT_CONTXT_RPLY, EXCL_LOW_STAT, reg_regs_bn_free_without_contxt_rply);
    regs_bn_free_without_contxt_rply->bsy = RU_FIELD_GET(0, SBPM, REGS_BN_FREE_WITHOUT_CONTXT_RPLY, BSY, reg_regs_bn_free_without_contxt_rply);
    regs_bn_free_without_contxt_rply->rdy = RU_FIELD_GET(0, SBPM, REGS_BN_FREE_WITHOUT_CONTXT_RPLY, RDY, reg_regs_bn_free_without_contxt_rply);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_bn_free_with_contxt_rply_get(sbpm_regs_bn_free_with_contxt_rply *regs_bn_free_with_contxt_rply)
{
    uint32_t reg_regs_bn_free_with_contxt_rply;

#ifdef VALIDATE_PARMS
    if (!regs_bn_free_with_contxt_rply)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_BN_FREE_WITH_CONTXT_RPLY, reg_regs_bn_free_with_contxt_rply);

    regs_bn_free_with_contxt_rply->free_ack = RU_FIELD_GET(0, SBPM, REGS_BN_FREE_WITH_CONTXT_RPLY, FREE_ACK, reg_regs_bn_free_with_contxt_rply);
    regs_bn_free_with_contxt_rply->ack_state = RU_FIELD_GET(0, SBPM, REGS_BN_FREE_WITH_CONTXT_RPLY, ACK_STATE, reg_regs_bn_free_with_contxt_rply);
    regs_bn_free_with_contxt_rply->nack_state = RU_FIELD_GET(0, SBPM, REGS_BN_FREE_WITH_CONTXT_RPLY, NACK_STATE, reg_regs_bn_free_with_contxt_rply);
    regs_bn_free_with_contxt_rply->excl_high_state = RU_FIELD_GET(0, SBPM, REGS_BN_FREE_WITH_CONTXT_RPLY, EXCL_HIGH_STATE, reg_regs_bn_free_with_contxt_rply);
    regs_bn_free_with_contxt_rply->excl_low_state = RU_FIELD_GET(0, SBPM, REGS_BN_FREE_WITH_CONTXT_RPLY, EXCL_LOW_STATE, reg_regs_bn_free_with_contxt_rply);
    regs_bn_free_with_contxt_rply->busy = RU_FIELD_GET(0, SBPM, REGS_BN_FREE_WITH_CONTXT_RPLY, BUSY, reg_regs_bn_free_with_contxt_rply);
    regs_bn_free_with_contxt_rply->rdy = RU_FIELD_GET(0, SBPM, REGS_BN_FREE_WITH_CONTXT_RPLY, RDY, reg_regs_bn_free_with_contxt_rply);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_gl_trsh_set(uint16_t gl_bat, uint16_t gl_bah)
{
    uint32_t reg_regs_sbpm_gl_trsh = 0;

#ifdef VALIDATE_PARMS
    if ((gl_bat >= _14BITS_MAX_VAL_) ||
       (gl_bah >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_sbpm_gl_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_GL_TRSH, GL_BAT, reg_regs_sbpm_gl_trsh, gl_bat);
    reg_regs_sbpm_gl_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_GL_TRSH, GL_BAH, reg_regs_sbpm_gl_trsh, gl_bah);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_GL_TRSH, reg_regs_sbpm_gl_trsh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_gl_trsh_get(uint16_t *gl_bat, uint16_t *gl_bah)
{
    uint32_t reg_regs_sbpm_gl_trsh;

#ifdef VALIDATE_PARMS
    if (!gl_bat || !gl_bah)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_GL_TRSH, reg_regs_sbpm_gl_trsh);

    *gl_bat = RU_FIELD_GET(0, SBPM, REGS_SBPM_GL_TRSH, GL_BAT, reg_regs_sbpm_gl_trsh);
    *gl_bah = RU_FIELD_GET(0, SBPM, REGS_SBPM_GL_TRSH, GL_BAH, reg_regs_sbpm_gl_trsh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_ug0_trsh_set(uint16_t ug_bat, uint16_t ug_bah)
{
    uint32_t reg_regs_sbpm_ug0_trsh = 0;

#ifdef VALIDATE_PARMS
    if ((ug_bat >= _14BITS_MAX_VAL_) ||
       (ug_bah >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_sbpm_ug0_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG0_TRSH, UG_BAT, reg_regs_sbpm_ug0_trsh, ug_bat);
    reg_regs_sbpm_ug0_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG0_TRSH, UG_BAH, reg_regs_sbpm_ug0_trsh, ug_bah);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_UG0_TRSH, reg_regs_sbpm_ug0_trsh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_ug0_trsh_get(uint16_t *ug_bat, uint16_t *ug_bah)
{
    uint32_t reg_regs_sbpm_ug0_trsh;

#ifdef VALIDATE_PARMS
    if (!ug_bat || !ug_bah)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_UG0_TRSH, reg_regs_sbpm_ug0_trsh);

    *ug_bat = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG0_TRSH, UG_BAT, reg_regs_sbpm_ug0_trsh);
    *ug_bah = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG0_TRSH, UG_BAH, reg_regs_sbpm_ug0_trsh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_ug1_trsh_set(uint16_t ug_bat, uint16_t ug_bah)
{
    uint32_t reg_regs_sbpm_ug1_trsh = 0;

#ifdef VALIDATE_PARMS
    if ((ug_bat >= _14BITS_MAX_VAL_) ||
       (ug_bah >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_sbpm_ug1_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG1_TRSH, UG_BAT, reg_regs_sbpm_ug1_trsh, ug_bat);
    reg_regs_sbpm_ug1_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG1_TRSH, UG_BAH, reg_regs_sbpm_ug1_trsh, ug_bah);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_UG1_TRSH, reg_regs_sbpm_ug1_trsh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_ug1_trsh_get(uint16_t *ug_bat, uint16_t *ug_bah)
{
    uint32_t reg_regs_sbpm_ug1_trsh;

#ifdef VALIDATE_PARMS
    if (!ug_bat || !ug_bah)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_UG1_TRSH, reg_regs_sbpm_ug1_trsh);

    *ug_bat = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG1_TRSH, UG_BAT, reg_regs_sbpm_ug1_trsh);
    *ug_bah = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG1_TRSH, UG_BAH, reg_regs_sbpm_ug1_trsh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_dbg_set(uint8_t select_bus)
{
    uint32_t reg_regs_sbpm_dbg = 0;

#ifdef VALIDATE_PARMS
    if ((select_bus >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_sbpm_dbg = RU_FIELD_SET(0, SBPM, REGS_SBPM_DBG, SELECT_BUS, reg_regs_sbpm_dbg, select_bus);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_DBG, reg_regs_sbpm_dbg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_dbg_get(uint8_t *select_bus)
{
    uint32_t reg_regs_sbpm_dbg;

#ifdef VALIDATE_PARMS
    if (!select_bus)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_DBG, reg_regs_sbpm_dbg);

    *select_bus = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG, SELECT_BUS, reg_regs_sbpm_dbg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_ug0_excl_high_trsh_set(uint16_t exclt, uint16_t exclh)
{
    uint32_t reg_regs_sbpm_ug0_excl_high_trsh = 0;

#ifdef VALIDATE_PARMS
    if ((exclt >= _14BITS_MAX_VAL_) ||
       (exclh >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_sbpm_ug0_excl_high_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG0_EXCL_HIGH_TRSH, EXCLT, reg_regs_sbpm_ug0_excl_high_trsh, exclt);
    reg_regs_sbpm_ug0_excl_high_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG0_EXCL_HIGH_TRSH, EXCLH, reg_regs_sbpm_ug0_excl_high_trsh, exclh);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_UG0_EXCL_HIGH_TRSH, reg_regs_sbpm_ug0_excl_high_trsh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_ug0_excl_high_trsh_get(uint16_t *exclt, uint16_t *exclh)
{
    uint32_t reg_regs_sbpm_ug0_excl_high_trsh;

#ifdef VALIDATE_PARMS
    if (!exclt || !exclh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_UG0_EXCL_HIGH_TRSH, reg_regs_sbpm_ug0_excl_high_trsh);

    *exclt = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG0_EXCL_HIGH_TRSH, EXCLT, reg_regs_sbpm_ug0_excl_high_trsh);
    *exclh = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG0_EXCL_HIGH_TRSH, EXCLH, reg_regs_sbpm_ug0_excl_high_trsh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_ug1_excl_high_trsh_set(uint16_t exclt, uint16_t exclh)
{
    uint32_t reg_regs_sbpm_ug1_excl_high_trsh = 0;

#ifdef VALIDATE_PARMS
    if ((exclt >= _14BITS_MAX_VAL_) ||
       (exclh >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_sbpm_ug1_excl_high_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG1_EXCL_HIGH_TRSH, EXCLT, reg_regs_sbpm_ug1_excl_high_trsh, exclt);
    reg_regs_sbpm_ug1_excl_high_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG1_EXCL_HIGH_TRSH, EXCLH, reg_regs_sbpm_ug1_excl_high_trsh, exclh);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_UG1_EXCL_HIGH_TRSH, reg_regs_sbpm_ug1_excl_high_trsh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_ug1_excl_high_trsh_get(uint16_t *exclt, uint16_t *exclh)
{
    uint32_t reg_regs_sbpm_ug1_excl_high_trsh;

#ifdef VALIDATE_PARMS
    if (!exclt || !exclh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_UG1_EXCL_HIGH_TRSH, reg_regs_sbpm_ug1_excl_high_trsh);

    *exclt = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG1_EXCL_HIGH_TRSH, EXCLT, reg_regs_sbpm_ug1_excl_high_trsh);
    *exclh = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG1_EXCL_HIGH_TRSH, EXCLH, reg_regs_sbpm_ug1_excl_high_trsh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_ug0_excl_low_trsh_set(uint16_t exclt, uint16_t exclh)
{
    uint32_t reg_regs_sbpm_ug0_excl_low_trsh = 0;

#ifdef VALIDATE_PARMS
    if ((exclt >= _14BITS_MAX_VAL_) ||
       (exclh >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_sbpm_ug0_excl_low_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG0_EXCL_LOW_TRSH, EXCLT, reg_regs_sbpm_ug0_excl_low_trsh, exclt);
    reg_regs_sbpm_ug0_excl_low_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG0_EXCL_LOW_TRSH, EXCLH, reg_regs_sbpm_ug0_excl_low_trsh, exclh);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_UG0_EXCL_LOW_TRSH, reg_regs_sbpm_ug0_excl_low_trsh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_ug0_excl_low_trsh_get(uint16_t *exclt, uint16_t *exclh)
{
    uint32_t reg_regs_sbpm_ug0_excl_low_trsh;

#ifdef VALIDATE_PARMS
    if (!exclt || !exclh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_UG0_EXCL_LOW_TRSH, reg_regs_sbpm_ug0_excl_low_trsh);

    *exclt = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG0_EXCL_LOW_TRSH, EXCLT, reg_regs_sbpm_ug0_excl_low_trsh);
    *exclh = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG0_EXCL_LOW_TRSH, EXCLH, reg_regs_sbpm_ug0_excl_low_trsh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_ug1_excl_low_trsh_set(uint16_t exclt, uint16_t exclh)
{
    uint32_t reg_regs_sbpm_ug1_excl_low_trsh = 0;

#ifdef VALIDATE_PARMS
    if ((exclt >= _14BITS_MAX_VAL_) ||
       (exclh >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_sbpm_ug1_excl_low_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG1_EXCL_LOW_TRSH, EXCLT, reg_regs_sbpm_ug1_excl_low_trsh, exclt);
    reg_regs_sbpm_ug1_excl_low_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG1_EXCL_LOW_TRSH, EXCLH, reg_regs_sbpm_ug1_excl_low_trsh, exclh);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_UG1_EXCL_LOW_TRSH, reg_regs_sbpm_ug1_excl_low_trsh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_ug1_excl_low_trsh_get(uint16_t *exclt, uint16_t *exclh)
{
    uint32_t reg_regs_sbpm_ug1_excl_low_trsh;

#ifdef VALIDATE_PARMS
    if (!exclt || !exclh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_UG1_EXCL_LOW_TRSH, reg_regs_sbpm_ug1_excl_low_trsh);

    *exclt = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG1_EXCL_LOW_TRSH, EXCLT, reg_regs_sbpm_ug1_excl_low_trsh);
    *exclh = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG1_EXCL_LOW_TRSH, EXCLH, reg_regs_sbpm_ug1_excl_low_trsh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_ug_status_get(uint8_t *ug_ack_stts, uint8_t *ug_excl_high_stts, uint8_t *ug_excl_low_stts)
{
    uint32_t reg_regs_sbpm_ug_status;

#ifdef VALIDATE_PARMS
    if (!ug_ack_stts || !ug_excl_high_stts || !ug_excl_low_stts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_UG_STATUS, reg_regs_sbpm_ug_status);

    *ug_ack_stts = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG_STATUS, UG_ACK_STTS, reg_regs_sbpm_ug_status);
    *ug_excl_high_stts = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG_STATUS, UG_EXCL_HIGH_STTS, reg_regs_sbpm_ug_status);
    *ug_excl_low_stts = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG_STATUS, UG_EXCL_LOW_STTS, reg_regs_sbpm_ug_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_error_handling_params_set(uint8_t search_depth, bdmf_boolean max_search_en, bdmf_boolean chck_last_en, bdmf_boolean freeze_in_error)
{
    uint32_t reg_regs_error_handling_params = 0;

#ifdef VALIDATE_PARMS
    if ((search_depth >= _7BITS_MAX_VAL_) ||
       (max_search_en >= _1BITS_MAX_VAL_) ||
       (chck_last_en >= _1BITS_MAX_VAL_) ||
       (freeze_in_error >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_error_handling_params = RU_FIELD_SET(0, SBPM, REGS_ERROR_HANDLING_PARAMS, SEARCH_DEPTH, reg_regs_error_handling_params, search_depth);
    reg_regs_error_handling_params = RU_FIELD_SET(0, SBPM, REGS_ERROR_HANDLING_PARAMS, MAX_SEARCH_EN, reg_regs_error_handling_params, max_search_en);
    reg_regs_error_handling_params = RU_FIELD_SET(0, SBPM, REGS_ERROR_HANDLING_PARAMS, CHCK_LAST_EN, reg_regs_error_handling_params, chck_last_en);
    reg_regs_error_handling_params = RU_FIELD_SET(0, SBPM, REGS_ERROR_HANDLING_PARAMS, FREEZE_IN_ERROR, reg_regs_error_handling_params, freeze_in_error);

    RU_REG_WRITE(0, SBPM, REGS_ERROR_HANDLING_PARAMS, reg_regs_error_handling_params);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_error_handling_params_get(uint8_t *search_depth, bdmf_boolean *max_search_en, bdmf_boolean *chck_last_en, bdmf_boolean *freeze_in_error)
{
    uint32_t reg_regs_error_handling_params;

#ifdef VALIDATE_PARMS
    if (!search_depth || !max_search_en || !chck_last_en || !freeze_in_error)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_ERROR_HANDLING_PARAMS, reg_regs_error_handling_params);

    *search_depth = RU_FIELD_GET(0, SBPM, REGS_ERROR_HANDLING_PARAMS, SEARCH_DEPTH, reg_regs_error_handling_params);
    *max_search_en = RU_FIELD_GET(0, SBPM, REGS_ERROR_HANDLING_PARAMS, MAX_SEARCH_EN, reg_regs_error_handling_params);
    *chck_last_en = RU_FIELD_GET(0, SBPM, REGS_ERROR_HANDLING_PARAMS, CHCK_LAST_EN, reg_regs_error_handling_params);
    *freeze_in_error = RU_FIELD_GET(0, SBPM, REGS_ERROR_HANDLING_PARAMS, FREEZE_IN_ERROR, reg_regs_error_handling_params);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_iir_addr_get(uint8_t *cmd_sa, uint8_t *cmd_ta)
{
    uint32_t reg_regs_sbpm_iir_addr;

#ifdef VALIDATE_PARMS
    if (!cmd_sa || !cmd_ta)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_IIR_ADDR, reg_regs_sbpm_iir_addr);

    *cmd_sa = RU_FIELD_GET(0, SBPM, REGS_SBPM_IIR_ADDR, CMD_SA, reg_regs_sbpm_iir_addr);
    *cmd_ta = RU_FIELD_GET(0, SBPM, REGS_SBPM_IIR_ADDR, CMD_TA, reg_regs_sbpm_iir_addr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_iir_low_get(uint32_t *cmd_data_0to31)
{
    uint32_t reg_regs_sbpm_iir_low;

#ifdef VALIDATE_PARMS
    if (!cmd_data_0to31)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_IIR_LOW, reg_regs_sbpm_iir_low);

    *cmd_data_0to31 = RU_FIELD_GET(0, SBPM, REGS_SBPM_IIR_LOW, CMD_DATA_0TO31, reg_regs_sbpm_iir_low);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_iir_high_get(uint32_t *cmd_data_32to63)
{
    uint32_t reg_regs_sbpm_iir_high;

#ifdef VALIDATE_PARMS
    if (!cmd_data_32to63)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_IIR_HIGH, reg_regs_sbpm_iir_high);

    *cmd_data_32to63 = RU_FIELD_GET(0, SBPM, REGS_SBPM_IIR_HIGH, CMD_DATA_32TO63, reg_regs_sbpm_iir_high);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_dbg_vec0_get(sbpm_regs_sbpm_dbg_vec0 *regs_sbpm_dbg_vec0)
{
    uint32_t reg_regs_sbpm_dbg_vec0;

#ifdef VALIDATE_PARMS
    if (!regs_sbpm_dbg_vec0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_DBG_VEC0, reg_regs_sbpm_dbg_vec0);

    regs_sbpm_dbg_vec0->alloc_sm = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC0, ALLOC_SM, reg_regs_sbpm_dbg_vec0);
    regs_sbpm_dbg_vec0->cnnct_sm = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC0, CNNCT_SM, reg_regs_sbpm_dbg_vec0);
    regs_sbpm_dbg_vec0->mcint_sm = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC0, MCINT_SM, reg_regs_sbpm_dbg_vec0);
    regs_sbpm_dbg_vec0->free_w_cnxt_sm = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC0, FREE_W_CNXT_SM, reg_regs_sbpm_dbg_vec0);
    regs_sbpm_dbg_vec0->free_wo_cnxt_sm = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC0, FREE_WO_CNXT_SM, reg_regs_sbpm_dbg_vec0);
    regs_sbpm_dbg_vec0->gn_sm = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC0, GN_SM, reg_regs_sbpm_dbg_vec0);
    regs_sbpm_dbg_vec0->multi_gn_sm = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC0, MULTI_GN_SM, reg_regs_sbpm_dbg_vec0);
    regs_sbpm_dbg_vec0->free_lst_hd = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC0, FREE_LST_HD, reg_regs_sbpm_dbg_vec0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_dbg_vec1_get(sbpm_regs_sbpm_dbg_vec1 *regs_sbpm_dbg_vec1)
{
    uint32_t reg_regs_sbpm_dbg_vec1;

#ifdef VALIDATE_PARMS
    if (!regs_sbpm_dbg_vec1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_DBG_VEC1, reg_regs_sbpm_dbg_vec1);

    regs_sbpm_dbg_vec1->in2e_valid = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC1, IN2E_VALID, reg_regs_sbpm_dbg_vec1);
    regs_sbpm_dbg_vec1->multi_gn_valid = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC1, MULTI_GN_VALID, reg_regs_sbpm_dbg_vec1);
    regs_sbpm_dbg_vec1->ug_active = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC1, UG_ACTIVE, reg_regs_sbpm_dbg_vec1);
    regs_sbpm_dbg_vec1->tx_cmd_full = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC1, TX_CMD_FULL, reg_regs_sbpm_dbg_vec1);
    regs_sbpm_dbg_vec1->rx_fifo_pop = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC1, RX_FIFO_POP, reg_regs_sbpm_dbg_vec1);
    regs_sbpm_dbg_vec1->ram_init_start = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC1, RAM_INIT_START, reg_regs_sbpm_dbg_vec1);
    regs_sbpm_dbg_vec1->ram_init_done = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC1, RAM_INIT_DONE, reg_regs_sbpm_dbg_vec1);
    regs_sbpm_dbg_vec1->rx_fifo_data = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC1, RX_FIFO_DATA, reg_regs_sbpm_dbg_vec1);
    regs_sbpm_dbg_vec1->free_decode = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC1, FREE_DECODE, reg_regs_sbpm_dbg_vec1);
    regs_sbpm_dbg_vec1->in2e_decode = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC1, IN2E_DECODE, reg_regs_sbpm_dbg_vec1);
    regs_sbpm_dbg_vec1->free_wo_decode = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC1, FREE_WO_DECODE, reg_regs_sbpm_dbg_vec1);
    regs_sbpm_dbg_vec1->get_nxt_decode = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC1, GET_NXT_DECODE, reg_regs_sbpm_dbg_vec1);
    regs_sbpm_dbg_vec1->multi_get_nxt_decode = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC1, MULTI_GET_NXT_DECODE, reg_regs_sbpm_dbg_vec1);
    regs_sbpm_dbg_vec1->cnct_decode = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC1, CNCT_DECODE, reg_regs_sbpm_dbg_vec1);
    regs_sbpm_dbg_vec1->free_w_decode = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC1, FREE_W_DECODE, reg_regs_sbpm_dbg_vec1);
    regs_sbpm_dbg_vec1->mcin_decode = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC1, MCIN_DECODE, reg_regs_sbpm_dbg_vec1);
    regs_sbpm_dbg_vec1->alloc_decode = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC1, ALLOC_DECODE, reg_regs_sbpm_dbg_vec1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_dbg_vec2_get(sbpm_regs_sbpm_dbg_vec2 *regs_sbpm_dbg_vec2)
{
    uint32_t reg_regs_sbpm_dbg_vec2;

#ifdef VALIDATE_PARMS
    if (!regs_sbpm_dbg_vec2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_DBG_VEC2, reg_regs_sbpm_dbg_vec2);

    regs_sbpm_dbg_vec2->tx_data_full = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC2, TX_DATA_FULL, reg_regs_sbpm_dbg_vec2);
    regs_sbpm_dbg_vec2->tx_fifo_empty = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC2, TX_FIFO_EMPTY, reg_regs_sbpm_dbg_vec2);
    regs_sbpm_dbg_vec2->lcl_stts_full = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC2, LCL_STTS_FULL, reg_regs_sbpm_dbg_vec2);
    regs_sbpm_dbg_vec2->lcl_stts_empty = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC2, LCL_STTS_EMPTY, reg_regs_sbpm_dbg_vec2);
    regs_sbpm_dbg_vec2->tx_cmd_full = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC2, TX_CMD_FULL, reg_regs_sbpm_dbg_vec2);
    regs_sbpm_dbg_vec2->tx_cmd_fifo_empty = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC2, TX_CMD_FIFO_EMPTY, reg_regs_sbpm_dbg_vec2);
    regs_sbpm_dbg_vec2->bb_decoder_dest_id = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC2, BB_DECODER_DEST_ID, reg_regs_sbpm_dbg_vec2);
    regs_sbpm_dbg_vec2->tx_bbh_send_in_progress = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC2, TX_BBH_SEND_IN_PROGRESS, reg_regs_sbpm_dbg_vec2);
    regs_sbpm_dbg_vec2->sp_2send = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC2, SP_2SEND, reg_regs_sbpm_dbg_vec2);
    regs_sbpm_dbg_vec2->tx2data_fifo_taddr = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC2, TX2DATA_FIFO_TADDR, reg_regs_sbpm_dbg_vec2);
    regs_sbpm_dbg_vec2->cpu_access = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC2, CPU_ACCESS, reg_regs_sbpm_dbg_vec2);
    regs_sbpm_dbg_vec2->bbh_access = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC2, BBH_ACCESS, reg_regs_sbpm_dbg_vec2);
    regs_sbpm_dbg_vec2->rnr_access = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC2, RNR_ACCESS, reg_regs_sbpm_dbg_vec2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_dbg_vec3_get(sbpm_regs_sbpm_dbg_vec3 *regs_sbpm_dbg_vec3)
{
    uint32_t reg_regs_sbpm_dbg_vec3;

#ifdef VALIDATE_PARMS
    if (!regs_sbpm_dbg_vec3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_DBG_VEC3, reg_regs_sbpm_dbg_vec3);

    regs_sbpm_dbg_vec3->alloc_rply = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC3, ALLOC_RPLY, reg_regs_sbpm_dbg_vec3);
    regs_sbpm_dbg_vec3->bn_rply = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC3, BN_RPLY, reg_regs_sbpm_dbg_vec3);
    regs_sbpm_dbg_vec3->txfifo_alloc_ack = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC3, TXFIFO_ALLOC_ACK, reg_regs_sbpm_dbg_vec3);
    regs_sbpm_dbg_vec3->tx_fifo_mcinc_ack = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC3, TX_FIFO_MCINC_ACK, reg_regs_sbpm_dbg_vec3);
    regs_sbpm_dbg_vec3->txfifo_cnct_ack = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC3, TXFIFO_CNCT_ACK, reg_regs_sbpm_dbg_vec3);
    regs_sbpm_dbg_vec3->txfifo_gt_nxt_rply = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC3, TXFIFO_GT_NXT_RPLY, reg_regs_sbpm_dbg_vec3);
    regs_sbpm_dbg_vec3->txfifo_mlti_gt_nxt_rply = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC3, TXFIFO_MLTI_GT_NXT_RPLY, reg_regs_sbpm_dbg_vec3);
    regs_sbpm_dbg_vec3->tx_msg_pipe_sm = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC3, TX_MSG_PIPE_SM, reg_regs_sbpm_dbg_vec3);
    regs_sbpm_dbg_vec3->send_stt_sm = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC3, SEND_STT_SM, reg_regs_sbpm_dbg_vec3);
    regs_sbpm_dbg_vec3->txfifo_in2estts_chng = RU_FIELD_GET(0, SBPM, REGS_SBPM_DBG_VEC3, TXFIFO_IN2ESTTS_CHNG, reg_regs_sbpm_dbg_vec3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_bbh_low_set(uint32_t sbpm_sp_bbh_low)
{
    uint32_t reg_regs_sbpm_sp_bbh_low = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_regs_sbpm_sp_bbh_low = RU_FIELD_SET(0, SBPM, REGS_SBPM_SP_BBH_LOW, SBPM_SP_BBH_LOW, reg_regs_sbpm_sp_bbh_low, sbpm_sp_bbh_low);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_SP_BBH_LOW, reg_regs_sbpm_sp_bbh_low);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_bbh_low_get(uint32_t *sbpm_sp_bbh_low)
{
    uint32_t reg_regs_sbpm_sp_bbh_low;

#ifdef VALIDATE_PARMS
    if (!sbpm_sp_bbh_low)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_SP_BBH_LOW, reg_regs_sbpm_sp_bbh_low);

    *sbpm_sp_bbh_low = RU_FIELD_GET(0, SBPM, REGS_SBPM_SP_BBH_LOW, SBPM_SP_BBH_LOW, reg_regs_sbpm_sp_bbh_low);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_bbh_high_set(uint32_t sbpm_sp_bbh_high)
{
    uint32_t reg_regs_sbpm_sp_bbh_high = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_regs_sbpm_sp_bbh_high = RU_FIELD_SET(0, SBPM, REGS_SBPM_SP_BBH_HIGH, SBPM_SP_BBH_HIGH, reg_regs_sbpm_sp_bbh_high, sbpm_sp_bbh_high);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_SP_BBH_HIGH, reg_regs_sbpm_sp_bbh_high);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_bbh_high_get(uint32_t *sbpm_sp_bbh_high)
{
    uint32_t reg_regs_sbpm_sp_bbh_high;

#ifdef VALIDATE_PARMS
    if (!sbpm_sp_bbh_high)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_SP_BBH_HIGH, reg_regs_sbpm_sp_bbh_high);

    *sbpm_sp_bbh_high = RU_FIELD_GET(0, SBPM, REGS_SBPM_SP_BBH_HIGH, SBPM_SP_BBH_HIGH, reg_regs_sbpm_sp_bbh_high);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_rnr_low_set(uint32_t sbpm_sp_rnr_low)
{
    uint32_t reg_regs_sbpm_sp_rnr_low = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_regs_sbpm_sp_rnr_low = RU_FIELD_SET(0, SBPM, REGS_SBPM_SP_RNR_LOW, SBPM_SP_RNR_LOW, reg_regs_sbpm_sp_rnr_low, sbpm_sp_rnr_low);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_SP_RNR_LOW, reg_regs_sbpm_sp_rnr_low);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_rnr_low_get(uint32_t *sbpm_sp_rnr_low)
{
    uint32_t reg_regs_sbpm_sp_rnr_low;

#ifdef VALIDATE_PARMS
    if (!sbpm_sp_rnr_low)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_SP_RNR_LOW, reg_regs_sbpm_sp_rnr_low);

    *sbpm_sp_rnr_low = RU_FIELD_GET(0, SBPM, REGS_SBPM_SP_RNR_LOW, SBPM_SP_RNR_LOW, reg_regs_sbpm_sp_rnr_low);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_rnr_high_set(uint32_t sbpm_sp_rnr_high)
{
    uint32_t reg_regs_sbpm_sp_rnr_high = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_regs_sbpm_sp_rnr_high = RU_FIELD_SET(0, SBPM, REGS_SBPM_SP_RNR_HIGH, SBPM_SP_RNR_HIGH, reg_regs_sbpm_sp_rnr_high, sbpm_sp_rnr_high);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_SP_RNR_HIGH, reg_regs_sbpm_sp_rnr_high);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_rnr_high_get(uint32_t *sbpm_sp_rnr_high)
{
    uint32_t reg_regs_sbpm_sp_rnr_high;

#ifdef VALIDATE_PARMS
    if (!sbpm_sp_rnr_high)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_SP_RNR_HIGH, reg_regs_sbpm_sp_rnr_high);

    *sbpm_sp_rnr_high = RU_FIELD_GET(0, SBPM, REGS_SBPM_SP_RNR_HIGH, SBPM_SP_RNR_HIGH, reg_regs_sbpm_sp_rnr_high);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_ug_map_low_set(uint32_t sbpm_ug_map_low)
{
    uint32_t reg_regs_sbpm_ug_map_low = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_regs_sbpm_ug_map_low = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG_MAP_LOW, SBPM_UG_MAP_LOW, reg_regs_sbpm_ug_map_low, sbpm_ug_map_low);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_UG_MAP_LOW, reg_regs_sbpm_ug_map_low);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_ug_map_low_get(uint32_t *sbpm_ug_map_low)
{
    uint32_t reg_regs_sbpm_ug_map_low;

#ifdef VALIDATE_PARMS
    if (!sbpm_ug_map_low)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_UG_MAP_LOW, reg_regs_sbpm_ug_map_low);

    *sbpm_ug_map_low = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG_MAP_LOW, SBPM_UG_MAP_LOW, reg_regs_sbpm_ug_map_low);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_ug_map_high_set(uint32_t sbpm_ug_map_high)
{
    uint32_t reg_regs_sbpm_ug_map_high = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_regs_sbpm_ug_map_high = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG_MAP_HIGH, SBPM_UG_MAP_HIGH, reg_regs_sbpm_ug_map_high, sbpm_ug_map_high);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_UG_MAP_HIGH, reg_regs_sbpm_ug_map_high);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_ug_map_high_get(uint32_t *sbpm_ug_map_high)
{
    uint32_t reg_regs_sbpm_ug_map_high;

#ifdef VALIDATE_PARMS
    if (!sbpm_ug_map_high)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_UG_MAP_HIGH, reg_regs_sbpm_ug_map_high);

    *sbpm_ug_map_high = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG_MAP_HIGH, SBPM_UG_MAP_HIGH, reg_regs_sbpm_ug_map_high);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_excl_mask_low_set(uint32_t sbpm_excl_mask_low)
{
    uint32_t reg_regs_sbpm_excl_mask_low = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_regs_sbpm_excl_mask_low = RU_FIELD_SET(0, SBPM, REGS_SBPM_EXCL_MASK_LOW, SBPM_EXCL_MASK_LOW, reg_regs_sbpm_excl_mask_low, sbpm_excl_mask_low);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_EXCL_MASK_LOW, reg_regs_sbpm_excl_mask_low);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_excl_mask_low_get(uint32_t *sbpm_excl_mask_low)
{
    uint32_t reg_regs_sbpm_excl_mask_low;

#ifdef VALIDATE_PARMS
    if (!sbpm_excl_mask_low)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_EXCL_MASK_LOW, reg_regs_sbpm_excl_mask_low);

    *sbpm_excl_mask_low = RU_FIELD_GET(0, SBPM, REGS_SBPM_EXCL_MASK_LOW, SBPM_EXCL_MASK_LOW, reg_regs_sbpm_excl_mask_low);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_excl_mask_high_set(uint32_t sbpm_excl_mask_high)
{
    uint32_t reg_regs_sbpm_excl_mask_high = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_regs_sbpm_excl_mask_high = RU_FIELD_SET(0, SBPM, REGS_SBPM_EXCL_MASK_HIGH, SBPM_EXCL_MASK_HIGH, reg_regs_sbpm_excl_mask_high, sbpm_excl_mask_high);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_EXCL_MASK_HIGH, reg_regs_sbpm_excl_mask_high);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_excl_mask_high_get(uint32_t *sbpm_excl_mask_high)
{
    uint32_t reg_regs_sbpm_excl_mask_high;

#ifdef VALIDATE_PARMS
    if (!sbpm_excl_mask_high)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_EXCL_MASK_HIGH, reg_regs_sbpm_excl_mask_high);

    *sbpm_excl_mask_high = RU_FIELD_GET(0, SBPM, REGS_SBPM_EXCL_MASK_HIGH, SBPM_EXCL_MASK_HIGH, reg_regs_sbpm_excl_mask_high);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_raddr_decoder_set(uint8_t id_2overwr, uint16_t overwr_ra, bdmf_boolean overwr_valid)
{
    uint32_t reg_regs_sbpm_raddr_decoder = 0;

#ifdef VALIDATE_PARMS
    if ((id_2overwr >= _6BITS_MAX_VAL_) ||
       (overwr_ra >= _10BITS_MAX_VAL_) ||
       (overwr_valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_sbpm_raddr_decoder = RU_FIELD_SET(0, SBPM, REGS_SBPM_RADDR_DECODER, ID_2OVERWR, reg_regs_sbpm_raddr_decoder, id_2overwr);
    reg_regs_sbpm_raddr_decoder = RU_FIELD_SET(0, SBPM, REGS_SBPM_RADDR_DECODER, OVERWR_RA, reg_regs_sbpm_raddr_decoder, overwr_ra);
    reg_regs_sbpm_raddr_decoder = RU_FIELD_SET(0, SBPM, REGS_SBPM_RADDR_DECODER, OVERWR_VALID, reg_regs_sbpm_raddr_decoder, overwr_valid);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_RADDR_DECODER, reg_regs_sbpm_raddr_decoder);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_raddr_decoder_get(uint8_t *id_2overwr, uint16_t *overwr_ra, bdmf_boolean *overwr_valid)
{
    uint32_t reg_regs_sbpm_raddr_decoder;

#ifdef VALIDATE_PARMS
    if (!id_2overwr || !overwr_ra || !overwr_valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_RADDR_DECODER, reg_regs_sbpm_raddr_decoder);

    *id_2overwr = RU_FIELD_GET(0, SBPM, REGS_SBPM_RADDR_DECODER, ID_2OVERWR, reg_regs_sbpm_raddr_decoder);
    *overwr_ra = RU_FIELD_GET(0, SBPM, REGS_SBPM_RADDR_DECODER, OVERWR_RA, reg_regs_sbpm_raddr_decoder);
    *overwr_valid = RU_FIELD_GET(0, SBPM, REGS_SBPM_RADDR_DECODER, OVERWR_VALID, reg_regs_sbpm_raddr_decoder);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_wr_data_set(uint32_t sbpm_wr_data)
{
    uint32_t reg_regs_sbpm_wr_data = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_wr_data >= _22BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_sbpm_wr_data = RU_FIELD_SET(0, SBPM, REGS_SBPM_WR_DATA, SBPM_WR_DATA, reg_regs_sbpm_wr_data, sbpm_wr_data);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_WR_DATA, reg_regs_sbpm_wr_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_wr_data_get(uint32_t *sbpm_wr_data)
{
    uint32_t reg_regs_sbpm_wr_data;

#ifdef VALIDATE_PARMS
    if (!sbpm_wr_data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_WR_DATA, reg_regs_sbpm_wr_data);

    *sbpm_wr_data = RU_FIELD_GET(0, SBPM, REGS_SBPM_WR_DATA, SBPM_WR_DATA, reg_regs_sbpm_wr_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_ug_bac_max_set(uint16_t ug0bacmax, uint16_t ug1bacmax)
{
    uint32_t reg_regs_sbpm_ug_bac_max = 0;

#ifdef VALIDATE_PARMS
    if ((ug0bacmax >= _14BITS_MAX_VAL_) ||
       (ug1bacmax >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_sbpm_ug_bac_max = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG_BAC_MAX, UG0BACMAX, reg_regs_sbpm_ug_bac_max, ug0bacmax);
    reg_regs_sbpm_ug_bac_max = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG_BAC_MAX, UG1BACMAX, reg_regs_sbpm_ug_bac_max, ug1bacmax);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_UG_BAC_MAX, reg_regs_sbpm_ug_bac_max);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_ug_bac_max_get(uint16_t *ug0bacmax, uint16_t *ug1bacmax)
{
    uint32_t reg_regs_sbpm_ug_bac_max;

#ifdef VALIDATE_PARMS
    if (!ug0bacmax || !ug1bacmax)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_UG_BAC_MAX, reg_regs_sbpm_ug_bac_max);

    *ug0bacmax = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG_BAC_MAX, UG0BACMAX, reg_regs_sbpm_ug_bac_max);
    *ug1bacmax = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG_BAC_MAX, UG1BACMAX, reg_regs_sbpm_ug_bac_max);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_spare_set(bdmf_boolean gl_bac_clear_en)
{
    uint32_t reg_regs_sbpm_spare = 0;

#ifdef VALIDATE_PARMS
    if ((gl_bac_clear_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_sbpm_spare = RU_FIELD_SET(0, SBPM, REGS_SBPM_SPARE, GL_BAC_CLEAR_EN, reg_regs_sbpm_spare, gl_bac_clear_en);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_SPARE, reg_regs_sbpm_spare);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_spare_get(bdmf_boolean *gl_bac_clear_en)
{
    uint32_t reg_regs_sbpm_spare;

#ifdef VALIDATE_PARMS
    if (!gl_bac_clear_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_SPARE, reg_regs_sbpm_spare);

    *gl_bac_clear_en = RU_FIELD_GET(0, SBPM, REGS_SBPM_SPARE, GL_BAC_CLEAR_EN, reg_regs_sbpm_spare);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_bac_underrun_set(bdmf_boolean sbpm_cfg_bac_underrun_en)
{
    uint32_t reg_regs_sbpm_bac_underrun = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_cfg_bac_underrun_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_sbpm_bac_underrun = RU_FIELD_SET(0, SBPM, REGS_SBPM_BAC_UNDERRUN, SBPM_CFG_BAC_UNDERRUN_EN, reg_regs_sbpm_bac_underrun, sbpm_cfg_bac_underrun_en);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_BAC_UNDERRUN, reg_regs_sbpm_bac_underrun);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_sbpm_bac_underrun_get(bdmf_boolean *sbpm_cfg_bac_underrun_en)
{
    uint32_t reg_regs_sbpm_bac_underrun;

#ifdef VALIDATE_PARMS
    if (!sbpm_cfg_bac_underrun_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_BAC_UNDERRUN, reg_regs_sbpm_bac_underrun);

    *sbpm_cfg_bac_underrun_en = RU_FIELD_GET(0, SBPM, REGS_SBPM_BAC_UNDERRUN, SBPM_CFG_BAC_UNDERRUN_EN, reg_regs_sbpm_bac_underrun);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_intr_ctrl_isr_set(const sbpm_intr_ctrl_isr *intr_ctrl_isr)
{
    uint32_t reg_intr_ctrl_isr = 0;

#ifdef VALIDATE_PARMS
    if(!intr_ctrl_isr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((intr_ctrl_isr->bac_underrun >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->mcst_overflow >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->check_last_err >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->max_search_err >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->invalid_in2e >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->multi_get_next_null >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->cnct_null >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->alloc_null >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->invalid_in2e_overflow >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->invalid_in2e_underflow >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->bac_underrun_ug0 >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->bac_underrun_ug1 >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_intr_ctrl_isr = RU_FIELD_SET(0, SBPM, INTR_CTRL_ISR, BAC_UNDERRUN, reg_intr_ctrl_isr, intr_ctrl_isr->bac_underrun);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, SBPM, INTR_CTRL_ISR, MCST_OVERFLOW, reg_intr_ctrl_isr, intr_ctrl_isr->mcst_overflow);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, SBPM, INTR_CTRL_ISR, CHECK_LAST_ERR, reg_intr_ctrl_isr, intr_ctrl_isr->check_last_err);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, SBPM, INTR_CTRL_ISR, MAX_SEARCH_ERR, reg_intr_ctrl_isr, intr_ctrl_isr->max_search_err);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, SBPM, INTR_CTRL_ISR, INVALID_IN2E, reg_intr_ctrl_isr, intr_ctrl_isr->invalid_in2e);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, SBPM, INTR_CTRL_ISR, MULTI_GET_NEXT_NULL, reg_intr_ctrl_isr, intr_ctrl_isr->multi_get_next_null);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, SBPM, INTR_CTRL_ISR, CNCT_NULL, reg_intr_ctrl_isr, intr_ctrl_isr->cnct_null);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, SBPM, INTR_CTRL_ISR, ALLOC_NULL, reg_intr_ctrl_isr, intr_ctrl_isr->alloc_null);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, SBPM, INTR_CTRL_ISR, INVALID_IN2E_OVERFLOW, reg_intr_ctrl_isr, intr_ctrl_isr->invalid_in2e_overflow);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, SBPM, INTR_CTRL_ISR, INVALID_IN2E_UNDERFLOW, reg_intr_ctrl_isr, intr_ctrl_isr->invalid_in2e_underflow);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, SBPM, INTR_CTRL_ISR, BAC_UNDERRUN_UG0, reg_intr_ctrl_isr, intr_ctrl_isr->bac_underrun_ug0);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, SBPM, INTR_CTRL_ISR, BAC_UNDERRUN_UG1, reg_intr_ctrl_isr, intr_ctrl_isr->bac_underrun_ug1);

    RU_REG_WRITE(0, SBPM, INTR_CTRL_ISR, reg_intr_ctrl_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_intr_ctrl_isr_get(sbpm_intr_ctrl_isr *intr_ctrl_isr)
{
    uint32_t reg_intr_ctrl_isr;

#ifdef VALIDATE_PARMS
    if (!intr_ctrl_isr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, INTR_CTRL_ISR, reg_intr_ctrl_isr);

    intr_ctrl_isr->bac_underrun = RU_FIELD_GET(0, SBPM, INTR_CTRL_ISR, BAC_UNDERRUN, reg_intr_ctrl_isr);
    intr_ctrl_isr->mcst_overflow = RU_FIELD_GET(0, SBPM, INTR_CTRL_ISR, MCST_OVERFLOW, reg_intr_ctrl_isr);
    intr_ctrl_isr->check_last_err = RU_FIELD_GET(0, SBPM, INTR_CTRL_ISR, CHECK_LAST_ERR, reg_intr_ctrl_isr);
    intr_ctrl_isr->max_search_err = RU_FIELD_GET(0, SBPM, INTR_CTRL_ISR, MAX_SEARCH_ERR, reg_intr_ctrl_isr);
    intr_ctrl_isr->invalid_in2e = RU_FIELD_GET(0, SBPM, INTR_CTRL_ISR, INVALID_IN2E, reg_intr_ctrl_isr);
    intr_ctrl_isr->multi_get_next_null = RU_FIELD_GET(0, SBPM, INTR_CTRL_ISR, MULTI_GET_NEXT_NULL, reg_intr_ctrl_isr);
    intr_ctrl_isr->cnct_null = RU_FIELD_GET(0, SBPM, INTR_CTRL_ISR, CNCT_NULL, reg_intr_ctrl_isr);
    intr_ctrl_isr->alloc_null = RU_FIELD_GET(0, SBPM, INTR_CTRL_ISR, ALLOC_NULL, reg_intr_ctrl_isr);
    intr_ctrl_isr->invalid_in2e_overflow = RU_FIELD_GET(0, SBPM, INTR_CTRL_ISR, INVALID_IN2E_OVERFLOW, reg_intr_ctrl_isr);
    intr_ctrl_isr->invalid_in2e_underflow = RU_FIELD_GET(0, SBPM, INTR_CTRL_ISR, INVALID_IN2E_UNDERFLOW, reg_intr_ctrl_isr);
    intr_ctrl_isr->bac_underrun_ug0 = RU_FIELD_GET(0, SBPM, INTR_CTRL_ISR, BAC_UNDERRUN_UG0, reg_intr_ctrl_isr);
    intr_ctrl_isr->bac_underrun_ug1 = RU_FIELD_GET(0, SBPM, INTR_CTRL_ISR, BAC_UNDERRUN_UG1, reg_intr_ctrl_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_intr_ctrl_ism_get(uint32_t *ism)
{
    uint32_t reg_intr_ctrl_ism;

#ifdef VALIDATE_PARMS
    if (!ism)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, INTR_CTRL_ISM, reg_intr_ctrl_ism);

    *ism = RU_FIELD_GET(0, SBPM, INTR_CTRL_ISM, ISM, reg_intr_ctrl_ism);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_intr_ctrl_ier_set(uint32_t iem)
{
    uint32_t reg_intr_ctrl_ier = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_intr_ctrl_ier = RU_FIELD_SET(0, SBPM, INTR_CTRL_IER, IEM, reg_intr_ctrl_ier, iem);

    RU_REG_WRITE(0, SBPM, INTR_CTRL_IER, reg_intr_ctrl_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_intr_ctrl_ier_get(uint32_t *iem)
{
    uint32_t reg_intr_ctrl_ier;

#ifdef VALIDATE_PARMS
    if (!iem)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, INTR_CTRL_IER, reg_intr_ctrl_ier);

    *iem = RU_FIELD_GET(0, SBPM, INTR_CTRL_IER, IEM, reg_intr_ctrl_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_intr_ctrl_itr_set(uint32_t ist)
{
    uint32_t reg_intr_ctrl_itr = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_intr_ctrl_itr = RU_FIELD_SET(0, SBPM, INTR_CTRL_ITR, IST, reg_intr_ctrl_itr, ist);

    RU_REG_WRITE(0, SBPM, INTR_CTRL_ITR, reg_intr_ctrl_itr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_intr_ctrl_itr_get(uint32_t *ist)
{
    uint32_t reg_intr_ctrl_itr;

#ifdef VALIDATE_PARMS
    if (!ist)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, INTR_CTRL_ITR, reg_intr_ctrl_itr);

    *ist = RU_FIELD_GET(0, SBPM, INTR_CTRL_ITR, IST, reg_intr_ctrl_itr);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_regs_init_free_list,
    bdmf_address_regs_bn_alloc,
    bdmf_address_regs_bn_alloc_rply,
    bdmf_address_regs_bn_free_with_contxt_low,
    bdmf_address_regs_bn_free_with_contxt_high,
    bdmf_address_regs_mcst_inc,
    bdmf_address_regs_mcst_inc_rply,
    bdmf_address_regs_bn_connect,
    bdmf_address_regs_bn_connect_rply,
    bdmf_address_regs_get_next,
    bdmf_address_regs_get_next_rply,
    bdmf_address_regs_sbpm_clk_gate_cntrl,
    bdmf_address_regs_bn_free_without_contxt,
    bdmf_address_regs_bn_free_without_contxt_rply,
    bdmf_address_regs_bn_free_with_contxt_rply,
    bdmf_address_regs_sbpm_gl_trsh,
    bdmf_address_regs_sbpm_ug0_trsh,
    bdmf_address_regs_sbpm_ug1_trsh,
    bdmf_address_regs_sbpm_dbg,
    bdmf_address_regs_sbpm_ug0_bac,
    bdmf_address_regs_sbpm_ug1_bac,
    bdmf_address_regs_sbpm_gl_bac,
    bdmf_address_regs_sbpm_ug0_excl_high_trsh,
    bdmf_address_regs_sbpm_ug1_excl_high_trsh,
    bdmf_address_regs_sbpm_ug0_excl_low_trsh,
    bdmf_address_regs_sbpm_ug1_excl_low_trsh,
    bdmf_address_regs_sbpm_ug_status,
    bdmf_address_regs_error_handling_params,
    bdmf_address_regs_sbpm_iir_addr,
    bdmf_address_regs_sbpm_iir_low,
    bdmf_address_regs_sbpm_iir_high,
    bdmf_address_regs_sbpm_dbg_vec0,
    bdmf_address_regs_sbpm_dbg_vec1,
    bdmf_address_regs_sbpm_dbg_vec2,
    bdmf_address_regs_sbpm_dbg_vec3,
    bdmf_address_regs_sbpm_sp_bbh_low,
    bdmf_address_regs_sbpm_sp_bbh_high,
    bdmf_address_regs_sbpm_sp_rnr_low,
    bdmf_address_regs_sbpm_sp_rnr_high,
    bdmf_address_regs_sbpm_ug_map_low,
    bdmf_address_regs_sbpm_ug_map_high,
    bdmf_address_regs_sbpm_nack_mask_low,
    bdmf_address_regs_sbpm_nack_mask_high,
    bdmf_address_regs_sbpm_excl_mask_low,
    bdmf_address_regs_sbpm_excl_mask_high,
    bdmf_address_regs_sbpm_raddr_decoder,
    bdmf_address_regs_sbpm_wr_data,
    bdmf_address_regs_sbpm_ug_bac_max,
    bdmf_address_regs_sbpm_spare,
    bdmf_address_regs_sbpm_bac_underrun,
    bdmf_address_intr_ctrl_isr,
    bdmf_address_intr_ctrl_ism,
    bdmf_address_intr_ctrl_ier,
    bdmf_address_intr_ctrl_itr,
}
bdmf_address;

static int ag_drv_sbpm_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_sbpm_regs_init_free_list:
        ag_err = ag_drv_sbpm_regs_init_free_list_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_sbpm_regs_bn_alloc:
        ag_err = ag_drv_sbpm_regs_bn_alloc_set(parm[1].value.unumber);
        break;
    case cli_sbpm_regs_bn_free_with_contxt_low:
        ag_err = ag_drv_sbpm_regs_bn_free_with_contxt_low_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_sbpm_regs_bn_free_with_contxt_high:
        ag_err = ag_drv_sbpm_regs_bn_free_with_contxt_high_set(parm[1].value.unumber);
        break;
    case cli_sbpm_regs_mcst_inc:
        ag_err = ag_drv_sbpm_regs_mcst_inc_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_sbpm_regs_bn_connect:
        ag_err = ag_drv_sbpm_regs_bn_connect_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_sbpm_regs_get_next:
        ag_err = ag_drv_sbpm_regs_get_next_set(parm[1].value.unumber);
        break;
    case cli_sbpm_regs_sbpm_clk_gate_cntrl:
    {
        sbpm_regs_sbpm_clk_gate_cntrl regs_sbpm_clk_gate_cntrl = { .bypass_clk_gate = parm[1].value.unumber, .timer_val = parm[2].value.unumber, .keep_alive_en = parm[3].value.unumber, .keep_alive_intervl = parm[4].value.unumber, .keep_alive_cyc = parm[5].value.unumber};
        ag_err = ag_drv_sbpm_regs_sbpm_clk_gate_cntrl_set(&regs_sbpm_clk_gate_cntrl);
        break;
    }
    case cli_sbpm_regs_bn_free_without_contxt:
        ag_err = ag_drv_sbpm_regs_bn_free_without_contxt_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_sbpm_regs_sbpm_gl_trsh:
        ag_err = ag_drv_sbpm_regs_sbpm_gl_trsh_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_sbpm_regs_sbpm_ug0_trsh:
        ag_err = ag_drv_sbpm_regs_sbpm_ug0_trsh_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_sbpm_regs_sbpm_ug1_trsh:
        ag_err = ag_drv_sbpm_regs_sbpm_ug1_trsh_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_sbpm_regs_sbpm_dbg:
        ag_err = ag_drv_sbpm_regs_sbpm_dbg_set(parm[1].value.unumber);
        break;
    case cli_sbpm_regs_sbpm_ug0_excl_high_trsh:
        ag_err = ag_drv_sbpm_regs_sbpm_ug0_excl_high_trsh_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_sbpm_regs_sbpm_ug1_excl_high_trsh:
        ag_err = ag_drv_sbpm_regs_sbpm_ug1_excl_high_trsh_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_sbpm_regs_sbpm_ug0_excl_low_trsh:
        ag_err = ag_drv_sbpm_regs_sbpm_ug0_excl_low_trsh_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_sbpm_regs_sbpm_ug1_excl_low_trsh:
        ag_err = ag_drv_sbpm_regs_sbpm_ug1_excl_low_trsh_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_sbpm_regs_error_handling_params:
        ag_err = ag_drv_sbpm_regs_error_handling_params_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_sbpm_regs_sbpm_sp_bbh_low:
        ag_err = ag_drv_sbpm_regs_sbpm_sp_bbh_low_set(parm[1].value.unumber);
        break;
    case cli_sbpm_regs_sbpm_sp_bbh_high:
        ag_err = ag_drv_sbpm_regs_sbpm_sp_bbh_high_set(parm[1].value.unumber);
        break;
    case cli_sbpm_regs_sbpm_sp_rnr_low:
        ag_err = ag_drv_sbpm_regs_sbpm_sp_rnr_low_set(parm[1].value.unumber);
        break;
    case cli_sbpm_regs_sbpm_sp_rnr_high:
        ag_err = ag_drv_sbpm_regs_sbpm_sp_rnr_high_set(parm[1].value.unumber);
        break;
    case cli_sbpm_regs_sbpm_ug_map_low:
        ag_err = ag_drv_sbpm_regs_sbpm_ug_map_low_set(parm[1].value.unumber);
        break;
    case cli_sbpm_regs_sbpm_ug_map_high:
        ag_err = ag_drv_sbpm_regs_sbpm_ug_map_high_set(parm[1].value.unumber);
        break;
    case cli_sbpm_regs_sbpm_excl_mask_low:
        ag_err = ag_drv_sbpm_regs_sbpm_excl_mask_low_set(parm[1].value.unumber);
        break;
    case cli_sbpm_regs_sbpm_excl_mask_high:
        ag_err = ag_drv_sbpm_regs_sbpm_excl_mask_high_set(parm[1].value.unumber);
        break;
    case cli_sbpm_regs_sbpm_raddr_decoder:
        ag_err = ag_drv_sbpm_regs_sbpm_raddr_decoder_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_sbpm_regs_sbpm_wr_data:
        ag_err = ag_drv_sbpm_regs_sbpm_wr_data_set(parm[1].value.unumber);
        break;
    case cli_sbpm_regs_sbpm_ug_bac_max:
        ag_err = ag_drv_sbpm_regs_sbpm_ug_bac_max_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_sbpm_regs_sbpm_spare:
        ag_err = ag_drv_sbpm_regs_sbpm_spare_set(parm[1].value.unumber);
        break;
    case cli_sbpm_regs_sbpm_bac_underrun:
        ag_err = ag_drv_sbpm_regs_sbpm_bac_underrun_set(parm[1].value.unumber);
        break;
    case cli_sbpm_intr_ctrl_isr:
    {
        sbpm_intr_ctrl_isr intr_ctrl_isr = { .bac_underrun = parm[1].value.unumber, .mcst_overflow = parm[2].value.unumber, .check_last_err = parm[3].value.unumber, .max_search_err = parm[4].value.unumber, .invalid_in2e = parm[5].value.unumber, .multi_get_next_null = parm[6].value.unumber, .cnct_null = parm[7].value.unumber, .alloc_null = parm[8].value.unumber, .invalid_in2e_overflow = parm[9].value.unumber, .invalid_in2e_underflow = parm[10].value.unumber, .bac_underrun_ug0 = parm[11].value.unumber, .bac_underrun_ug1 = parm[12].value.unumber};
        ag_err = ag_drv_sbpm_intr_ctrl_isr_set(&intr_ctrl_isr);
        break;
    }
    case cli_sbpm_intr_ctrl_ier:
        ag_err = ag_drv_sbpm_intr_ctrl_ier_set(parm[1].value.unumber);
        break;
    case cli_sbpm_intr_ctrl_itr:
        ag_err = ag_drv_sbpm_intr_ctrl_itr_set(parm[1].value.unumber);
        break;
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_sbpm_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_sbpm_nack_mask:
    {
        uint32_t sbpm_nack_mask_high;
        uint32_t sbpm_nack_mask_low;
        ag_err = ag_drv_sbpm_nack_mask_get(&sbpm_nack_mask_high, &sbpm_nack_mask_low);
        bdmf_session_print(session, "sbpm_nack_mask_high = %u = 0x%x\n", sbpm_nack_mask_high, sbpm_nack_mask_high);
        bdmf_session_print(session, "sbpm_nack_mask_low = %u = 0x%x\n", sbpm_nack_mask_low, sbpm_nack_mask_low);
        break;
    }
    case cli_sbpm_bac:
    {
        uint16_t bac;
        uint16_t ug1bac;
        uint16_t ug0bac;
        ag_err = ag_drv_sbpm_bac_get(&bac, &ug1bac, &ug0bac);
        bdmf_session_print(session, "bac = %u = 0x%x\n", bac, bac);
        bdmf_session_print(session, "ug1bac = %u = 0x%x\n", ug1bac, ug1bac);
        bdmf_session_print(session, "ug0bac = %u = 0x%x\n", ug0bac, ug0bac);
        break;
    }
    case cli_sbpm_regs_init_free_list:
    {
        uint16_t init_base_addr;
        uint16_t init_offset;
        bdmf_boolean bsy;
        bdmf_boolean rdy;
        ag_err = ag_drv_sbpm_regs_init_free_list_get(&init_base_addr, &init_offset, &bsy, &rdy);
        bdmf_session_print(session, "init_base_addr = %u = 0x%x\n", init_base_addr, init_base_addr);
        bdmf_session_print(session, "init_offset = %u = 0x%x\n", init_offset, init_offset);
        bdmf_session_print(session, "bsy = %u = 0x%x\n", bsy, bsy);
        bdmf_session_print(session, "rdy = %u = 0x%x\n", rdy, rdy);
        break;
    }
    case cli_sbpm_regs_bn_alloc:
    {
        uint8_t sa;
        ag_err = ag_drv_sbpm_regs_bn_alloc_get(&sa);
        bdmf_session_print(session, "sa = %u = 0x%x\n", sa, sa);
        break;
    }
    case cli_sbpm_regs_bn_alloc_rply:
    {
        sbpm_regs_bn_alloc_rply regs_bn_alloc_rply;
        ag_err = ag_drv_sbpm_regs_bn_alloc_rply_get(&regs_bn_alloc_rply);
        bdmf_session_print(session, "alloc_bn_valid = %u = 0x%x\n", regs_bn_alloc_rply.alloc_bn_valid, regs_bn_alloc_rply.alloc_bn_valid);
        bdmf_session_print(session, "alloc_bn = %u = 0x%x\n", regs_bn_alloc_rply.alloc_bn, regs_bn_alloc_rply.alloc_bn);
        bdmf_session_print(session, "ack = %u = 0x%x\n", regs_bn_alloc_rply.ack, regs_bn_alloc_rply.ack);
        bdmf_session_print(session, "nack = %u = 0x%x\n", regs_bn_alloc_rply.nack, regs_bn_alloc_rply.nack);
        bdmf_session_print(session, "excl_high = %u = 0x%x\n", regs_bn_alloc_rply.excl_high, regs_bn_alloc_rply.excl_high);
        bdmf_session_print(session, "excl_low = %u = 0x%x\n", regs_bn_alloc_rply.excl_low, regs_bn_alloc_rply.excl_low);
        bdmf_session_print(session, "busy = %u = 0x%x\n", regs_bn_alloc_rply.busy, regs_bn_alloc_rply.busy);
        bdmf_session_print(session, "rdy = %u = 0x%x\n", regs_bn_alloc_rply.rdy, regs_bn_alloc_rply.rdy);
        break;
    }
    case cli_sbpm_regs_bn_free_with_contxt_low:
    {
        uint16_t head_bn;
        uint8_t sa;
        uint8_t offset;
        bdmf_boolean ack;
        ag_err = ag_drv_sbpm_regs_bn_free_with_contxt_low_get(&head_bn, &sa, &offset, &ack);
        bdmf_session_print(session, "head_bn = %u = 0x%x\n", head_bn, head_bn);
        bdmf_session_print(session, "sa = %u = 0x%x\n", sa, sa);
        bdmf_session_print(session, "offset = %u = 0x%x\n", offset, offset);
        bdmf_session_print(session, "ack = %u = 0x%x\n", ack, ack);
        break;
    }
    case cli_sbpm_regs_bn_free_with_contxt_high:
    {
        uint16_t last_bn;
        ag_err = ag_drv_sbpm_regs_bn_free_with_contxt_high_get(&last_bn);
        bdmf_session_print(session, "last_bn = %u = 0x%x\n", last_bn, last_bn);
        break;
    }
    case cli_sbpm_regs_mcst_inc:
    {
        uint16_t bn;
        uint8_t mcst_val;
        bdmf_boolean ack_req;
        ag_err = ag_drv_sbpm_regs_mcst_inc_get(&bn, &mcst_val, &ack_req);
        bdmf_session_print(session, "bn = %u = 0x%x\n", bn, bn);
        bdmf_session_print(session, "mcst_val = %u = 0x%x\n", mcst_val, mcst_val);
        bdmf_session_print(session, "ack_req = %u = 0x%x\n", ack_req, ack_req);
        break;
    }
    case cli_sbpm_regs_mcst_inc_rply:
    {
        bdmf_boolean mcst_ack;
        bdmf_boolean bsy;
        bdmf_boolean rdy;
        ag_err = ag_drv_sbpm_regs_mcst_inc_rply_get(&mcst_ack, &bsy, &rdy);
        bdmf_session_print(session, "mcst_ack = %u = 0x%x\n", mcst_ack, mcst_ack);
        bdmf_session_print(session, "bsy = %u = 0x%x\n", bsy, bsy);
        bdmf_session_print(session, "rdy = %u = 0x%x\n", rdy, rdy);
        break;
    }
    case cli_sbpm_regs_bn_connect:
    {
        uint16_t bn;
        bdmf_boolean ack_req;
        bdmf_boolean wr_req;
        uint16_t pointed_bn;
        ag_err = ag_drv_sbpm_regs_bn_connect_get(&bn, &ack_req, &wr_req, &pointed_bn);
        bdmf_session_print(session, "bn = %u = 0x%x\n", bn, bn);
        bdmf_session_print(session, "ack_req = %u = 0x%x\n", ack_req, ack_req);
        bdmf_session_print(session, "wr_req = %u = 0x%x\n", wr_req, wr_req);
        bdmf_session_print(session, "pointed_bn = %u = 0x%x\n", pointed_bn, pointed_bn);
        break;
    }
    case cli_sbpm_regs_bn_connect_rply:
    {
        bdmf_boolean connect_ack;
        bdmf_boolean busy;
        bdmf_boolean rdy;
        ag_err = ag_drv_sbpm_regs_bn_connect_rply_get(&connect_ack, &busy, &rdy);
        bdmf_session_print(session, "connect_ack = %u = 0x%x\n", connect_ack, connect_ack);
        bdmf_session_print(session, "busy = %u = 0x%x\n", busy, busy);
        bdmf_session_print(session, "rdy = %u = 0x%x\n", rdy, rdy);
        break;
    }
    case cli_sbpm_regs_get_next:
    {
        uint16_t bn;
        ag_err = ag_drv_sbpm_regs_get_next_get(&bn);
        bdmf_session_print(session, "bn = %u = 0x%x\n", bn, bn);
        break;
    }
    case cli_sbpm_regs_get_next_rply:
    {
        sbpm_regs_get_next_rply regs_get_next_rply;
        ag_err = ag_drv_sbpm_regs_get_next_rply_get(&regs_get_next_rply);
        bdmf_session_print(session, "bn_valid = %u = 0x%x\n", regs_get_next_rply.bn_valid, regs_get_next_rply.bn_valid);
        bdmf_session_print(session, "next_bn = %u = 0x%x\n", regs_get_next_rply.next_bn, regs_get_next_rply.next_bn);
        bdmf_session_print(session, "bn_null = %u = 0x%x\n", regs_get_next_rply.bn_null, regs_get_next_rply.bn_null);
        bdmf_session_print(session, "mcnt_val = %u = 0x%x\n", regs_get_next_rply.mcnt_val, regs_get_next_rply.mcnt_val);
        bdmf_session_print(session, "busy = %u = 0x%x\n", regs_get_next_rply.busy, regs_get_next_rply.busy);
        bdmf_session_print(session, "rdy = %u = 0x%x\n", regs_get_next_rply.rdy, regs_get_next_rply.rdy);
        break;
    }
    case cli_sbpm_regs_sbpm_clk_gate_cntrl:
    {
        sbpm_regs_sbpm_clk_gate_cntrl regs_sbpm_clk_gate_cntrl;
        ag_err = ag_drv_sbpm_regs_sbpm_clk_gate_cntrl_get(&regs_sbpm_clk_gate_cntrl);
        bdmf_session_print(session, "bypass_clk_gate = %u = 0x%x\n", regs_sbpm_clk_gate_cntrl.bypass_clk_gate, regs_sbpm_clk_gate_cntrl.bypass_clk_gate);
        bdmf_session_print(session, "timer_val = %u = 0x%x\n", regs_sbpm_clk_gate_cntrl.timer_val, regs_sbpm_clk_gate_cntrl.timer_val);
        bdmf_session_print(session, "keep_alive_en = %u = 0x%x\n", regs_sbpm_clk_gate_cntrl.keep_alive_en, regs_sbpm_clk_gate_cntrl.keep_alive_en);
        bdmf_session_print(session, "keep_alive_intervl = %u = 0x%x\n", regs_sbpm_clk_gate_cntrl.keep_alive_intervl, regs_sbpm_clk_gate_cntrl.keep_alive_intervl);
        bdmf_session_print(session, "keep_alive_cyc = %u = 0x%x\n", regs_sbpm_clk_gate_cntrl.keep_alive_cyc, regs_sbpm_clk_gate_cntrl.keep_alive_cyc);
        break;
    }
    case cli_sbpm_regs_bn_free_without_contxt:
    {
        uint16_t head_bn;
        uint8_t sa;
        bdmf_boolean ack_req;
        ag_err = ag_drv_sbpm_regs_bn_free_without_contxt_get(&head_bn, &sa, &ack_req);
        bdmf_session_print(session, "head_bn = %u = 0x%x\n", head_bn, head_bn);
        bdmf_session_print(session, "sa = %u = 0x%x\n", sa, sa);
        bdmf_session_print(session, "ack_req = %u = 0x%x\n", ack_req, ack_req);
        break;
    }
    case cli_sbpm_regs_bn_free_without_contxt_rply:
    {
        sbpm_regs_bn_free_without_contxt_rply regs_bn_free_without_contxt_rply;
        ag_err = ag_drv_sbpm_regs_bn_free_without_contxt_rply_get(&regs_bn_free_without_contxt_rply);
        bdmf_session_print(session, "free_ack = %u = 0x%x\n", regs_bn_free_without_contxt_rply.free_ack, regs_bn_free_without_contxt_rply.free_ack);
        bdmf_session_print(session, "ack_stat = %u = 0x%x\n", regs_bn_free_without_contxt_rply.ack_stat, regs_bn_free_without_contxt_rply.ack_stat);
        bdmf_session_print(session, "nack_stat = %u = 0x%x\n", regs_bn_free_without_contxt_rply.nack_stat, regs_bn_free_without_contxt_rply.nack_stat);
        bdmf_session_print(session, "excl_high_stat = %u = 0x%x\n", regs_bn_free_without_contxt_rply.excl_high_stat, regs_bn_free_without_contxt_rply.excl_high_stat);
        bdmf_session_print(session, "excl_low_stat = %u = 0x%x\n", regs_bn_free_without_contxt_rply.excl_low_stat, regs_bn_free_without_contxt_rply.excl_low_stat);
        bdmf_session_print(session, "bsy = %u = 0x%x\n", regs_bn_free_without_contxt_rply.bsy, regs_bn_free_without_contxt_rply.bsy);
        bdmf_session_print(session, "rdy = %u = 0x%x\n", regs_bn_free_without_contxt_rply.rdy, regs_bn_free_without_contxt_rply.rdy);
        break;
    }
    case cli_sbpm_regs_bn_free_with_contxt_rply:
    {
        sbpm_regs_bn_free_with_contxt_rply regs_bn_free_with_contxt_rply;
        ag_err = ag_drv_sbpm_regs_bn_free_with_contxt_rply_get(&regs_bn_free_with_contxt_rply);
        bdmf_session_print(session, "free_ack = %u = 0x%x\n", regs_bn_free_with_contxt_rply.free_ack, regs_bn_free_with_contxt_rply.free_ack);
        bdmf_session_print(session, "ack_state = %u = 0x%x\n", regs_bn_free_with_contxt_rply.ack_state, regs_bn_free_with_contxt_rply.ack_state);
        bdmf_session_print(session, "nack_state = %u = 0x%x\n", regs_bn_free_with_contxt_rply.nack_state, regs_bn_free_with_contxt_rply.nack_state);
        bdmf_session_print(session, "excl_high_state = %u = 0x%x\n", regs_bn_free_with_contxt_rply.excl_high_state, regs_bn_free_with_contxt_rply.excl_high_state);
        bdmf_session_print(session, "excl_low_state = %u = 0x%x\n", regs_bn_free_with_contxt_rply.excl_low_state, regs_bn_free_with_contxt_rply.excl_low_state);
        bdmf_session_print(session, "busy = %u = 0x%x\n", regs_bn_free_with_contxt_rply.busy, regs_bn_free_with_contxt_rply.busy);
        bdmf_session_print(session, "rdy = %u = 0x%x\n", regs_bn_free_with_contxt_rply.rdy, regs_bn_free_with_contxt_rply.rdy);
        break;
    }
    case cli_sbpm_regs_sbpm_gl_trsh:
    {
        uint16_t gl_bat;
        uint16_t gl_bah;
        ag_err = ag_drv_sbpm_regs_sbpm_gl_trsh_get(&gl_bat, &gl_bah);
        bdmf_session_print(session, "gl_bat = %u = 0x%x\n", gl_bat, gl_bat);
        bdmf_session_print(session, "gl_bah = %u = 0x%x\n", gl_bah, gl_bah);
        break;
    }
    case cli_sbpm_regs_sbpm_ug0_trsh:
    {
        uint16_t ug_bat;
        uint16_t ug_bah;
        ag_err = ag_drv_sbpm_regs_sbpm_ug0_trsh_get(&ug_bat, &ug_bah);
        bdmf_session_print(session, "ug_bat = %u = 0x%x\n", ug_bat, ug_bat);
        bdmf_session_print(session, "ug_bah = %u = 0x%x\n", ug_bah, ug_bah);
        break;
    }
    case cli_sbpm_regs_sbpm_ug1_trsh:
    {
        uint16_t ug_bat;
        uint16_t ug_bah;
        ag_err = ag_drv_sbpm_regs_sbpm_ug1_trsh_get(&ug_bat, &ug_bah);
        bdmf_session_print(session, "ug_bat = %u = 0x%x\n", ug_bat, ug_bat);
        bdmf_session_print(session, "ug_bah = %u = 0x%x\n", ug_bah, ug_bah);
        break;
    }
    case cli_sbpm_regs_sbpm_dbg:
    {
        uint8_t select_bus;
        ag_err = ag_drv_sbpm_regs_sbpm_dbg_get(&select_bus);
        bdmf_session_print(session, "select_bus = %u = 0x%x\n", select_bus, select_bus);
        break;
    }
    case cli_sbpm_regs_sbpm_ug0_excl_high_trsh:
    {
        uint16_t exclt;
        uint16_t exclh;
        ag_err = ag_drv_sbpm_regs_sbpm_ug0_excl_high_trsh_get(&exclt, &exclh);
        bdmf_session_print(session, "exclt = %u = 0x%x\n", exclt, exclt);
        bdmf_session_print(session, "exclh = %u = 0x%x\n", exclh, exclh);
        break;
    }
    case cli_sbpm_regs_sbpm_ug1_excl_high_trsh:
    {
        uint16_t exclt;
        uint16_t exclh;
        ag_err = ag_drv_sbpm_regs_sbpm_ug1_excl_high_trsh_get(&exclt, &exclh);
        bdmf_session_print(session, "exclt = %u = 0x%x\n", exclt, exclt);
        bdmf_session_print(session, "exclh = %u = 0x%x\n", exclh, exclh);
        break;
    }
    case cli_sbpm_regs_sbpm_ug0_excl_low_trsh:
    {
        uint16_t exclt;
        uint16_t exclh;
        ag_err = ag_drv_sbpm_regs_sbpm_ug0_excl_low_trsh_get(&exclt, &exclh);
        bdmf_session_print(session, "exclt = %u = 0x%x\n", exclt, exclt);
        bdmf_session_print(session, "exclh = %u = 0x%x\n", exclh, exclh);
        break;
    }
    case cli_sbpm_regs_sbpm_ug1_excl_low_trsh:
    {
        uint16_t exclt;
        uint16_t exclh;
        ag_err = ag_drv_sbpm_regs_sbpm_ug1_excl_low_trsh_get(&exclt, &exclh);
        bdmf_session_print(session, "exclt = %u = 0x%x\n", exclt, exclt);
        bdmf_session_print(session, "exclh = %u = 0x%x\n", exclh, exclh);
        break;
    }
    case cli_sbpm_regs_sbpm_ug_status:
    {
        uint8_t ug_ack_stts;
        uint8_t ug_excl_high_stts;
        uint8_t ug_excl_low_stts;
        ag_err = ag_drv_sbpm_regs_sbpm_ug_status_get(&ug_ack_stts, &ug_excl_high_stts, &ug_excl_low_stts);
        bdmf_session_print(session, "ug_ack_stts = %u = 0x%x\n", ug_ack_stts, ug_ack_stts);
        bdmf_session_print(session, "ug_excl_high_stts = %u = 0x%x\n", ug_excl_high_stts, ug_excl_high_stts);
        bdmf_session_print(session, "ug_excl_low_stts = %u = 0x%x\n", ug_excl_low_stts, ug_excl_low_stts);
        break;
    }
    case cli_sbpm_regs_error_handling_params:
    {
        uint8_t search_depth;
        bdmf_boolean max_search_en;
        bdmf_boolean chck_last_en;
        bdmf_boolean freeze_in_error;
        ag_err = ag_drv_sbpm_regs_error_handling_params_get(&search_depth, &max_search_en, &chck_last_en, &freeze_in_error);
        bdmf_session_print(session, "search_depth = %u = 0x%x\n", search_depth, search_depth);
        bdmf_session_print(session, "max_search_en = %u = 0x%x\n", max_search_en, max_search_en);
        bdmf_session_print(session, "chck_last_en = %u = 0x%x\n", chck_last_en, chck_last_en);
        bdmf_session_print(session, "freeze_in_error = %u = 0x%x\n", freeze_in_error, freeze_in_error);
        break;
    }
    case cli_sbpm_regs_sbpm_iir_addr:
    {
        uint8_t cmd_sa;
        uint8_t cmd_ta;
        ag_err = ag_drv_sbpm_regs_sbpm_iir_addr_get(&cmd_sa, &cmd_ta);
        bdmf_session_print(session, "cmd_sa = %u = 0x%x\n", cmd_sa, cmd_sa);
        bdmf_session_print(session, "cmd_ta = %u = 0x%x\n", cmd_ta, cmd_ta);
        break;
    }
    case cli_sbpm_regs_sbpm_iir_low:
    {
        uint32_t cmd_data_0to31;
        ag_err = ag_drv_sbpm_regs_sbpm_iir_low_get(&cmd_data_0to31);
        bdmf_session_print(session, "cmd_data_0to31 = %u = 0x%x\n", cmd_data_0to31, cmd_data_0to31);
        break;
    }
    case cli_sbpm_regs_sbpm_iir_high:
    {
        uint32_t cmd_data_32to63;
        ag_err = ag_drv_sbpm_regs_sbpm_iir_high_get(&cmd_data_32to63);
        bdmf_session_print(session, "cmd_data_32to63 = %u = 0x%x\n", cmd_data_32to63, cmd_data_32to63);
        break;
    }
    case cli_sbpm_regs_sbpm_dbg_vec0:
    {
        sbpm_regs_sbpm_dbg_vec0 regs_sbpm_dbg_vec0;
        ag_err = ag_drv_sbpm_regs_sbpm_dbg_vec0_get(&regs_sbpm_dbg_vec0);
        bdmf_session_print(session, "alloc_sm = %u = 0x%x\n", regs_sbpm_dbg_vec0.alloc_sm, regs_sbpm_dbg_vec0.alloc_sm);
        bdmf_session_print(session, "cnnct_sm = %u = 0x%x\n", regs_sbpm_dbg_vec0.cnnct_sm, regs_sbpm_dbg_vec0.cnnct_sm);
        bdmf_session_print(session, "mcint_sm = %u = 0x%x\n", regs_sbpm_dbg_vec0.mcint_sm, regs_sbpm_dbg_vec0.mcint_sm);
        bdmf_session_print(session, "free_w_cnxt_sm = %u = 0x%x\n", regs_sbpm_dbg_vec0.free_w_cnxt_sm, regs_sbpm_dbg_vec0.free_w_cnxt_sm);
        bdmf_session_print(session, "free_wo_cnxt_sm = %u = 0x%x\n", regs_sbpm_dbg_vec0.free_wo_cnxt_sm, regs_sbpm_dbg_vec0.free_wo_cnxt_sm);
        bdmf_session_print(session, "gn_sm = %u = 0x%x\n", regs_sbpm_dbg_vec0.gn_sm, regs_sbpm_dbg_vec0.gn_sm);
        bdmf_session_print(session, "multi_gn_sm = %u = 0x%x\n", regs_sbpm_dbg_vec0.multi_gn_sm, regs_sbpm_dbg_vec0.multi_gn_sm);
        bdmf_session_print(session, "free_lst_hd = %u = 0x%x\n", regs_sbpm_dbg_vec0.free_lst_hd, regs_sbpm_dbg_vec0.free_lst_hd);
        break;
    }
    case cli_sbpm_regs_sbpm_dbg_vec1:
    {
        sbpm_regs_sbpm_dbg_vec1 regs_sbpm_dbg_vec1;
        ag_err = ag_drv_sbpm_regs_sbpm_dbg_vec1_get(&regs_sbpm_dbg_vec1);
        bdmf_session_print(session, "in2e_valid = %u = 0x%x\n", regs_sbpm_dbg_vec1.in2e_valid, regs_sbpm_dbg_vec1.in2e_valid);
        bdmf_session_print(session, "multi_gn_valid = %u = 0x%x\n", regs_sbpm_dbg_vec1.multi_gn_valid, regs_sbpm_dbg_vec1.multi_gn_valid);
        bdmf_session_print(session, "ug_active = %u = 0x%x\n", regs_sbpm_dbg_vec1.ug_active, regs_sbpm_dbg_vec1.ug_active);
        bdmf_session_print(session, "tx_cmd_full = %u = 0x%x\n", regs_sbpm_dbg_vec1.tx_cmd_full, regs_sbpm_dbg_vec1.tx_cmd_full);
        bdmf_session_print(session, "rx_fifo_pop = %u = 0x%x\n", regs_sbpm_dbg_vec1.rx_fifo_pop, regs_sbpm_dbg_vec1.rx_fifo_pop);
        bdmf_session_print(session, "ram_init_start = %u = 0x%x\n", regs_sbpm_dbg_vec1.ram_init_start, regs_sbpm_dbg_vec1.ram_init_start);
        bdmf_session_print(session, "ram_init_done = %u = 0x%x\n", regs_sbpm_dbg_vec1.ram_init_done, regs_sbpm_dbg_vec1.ram_init_done);
        bdmf_session_print(session, "rx_fifo_data = %u = 0x%x\n", regs_sbpm_dbg_vec1.rx_fifo_data, regs_sbpm_dbg_vec1.rx_fifo_data);
        bdmf_session_print(session, "free_decode = %u = 0x%x\n", regs_sbpm_dbg_vec1.free_decode, regs_sbpm_dbg_vec1.free_decode);
        bdmf_session_print(session, "in2e_decode = %u = 0x%x\n", regs_sbpm_dbg_vec1.in2e_decode, regs_sbpm_dbg_vec1.in2e_decode);
        bdmf_session_print(session, "free_wo_decode = %u = 0x%x\n", regs_sbpm_dbg_vec1.free_wo_decode, regs_sbpm_dbg_vec1.free_wo_decode);
        bdmf_session_print(session, "get_nxt_decode = %u = 0x%x\n", regs_sbpm_dbg_vec1.get_nxt_decode, regs_sbpm_dbg_vec1.get_nxt_decode);
        bdmf_session_print(session, "multi_get_nxt_decode = %u = 0x%x\n", regs_sbpm_dbg_vec1.multi_get_nxt_decode, regs_sbpm_dbg_vec1.multi_get_nxt_decode);
        bdmf_session_print(session, "cnct_decode = %u = 0x%x\n", regs_sbpm_dbg_vec1.cnct_decode, regs_sbpm_dbg_vec1.cnct_decode);
        bdmf_session_print(session, "free_w_decode = %u = 0x%x\n", regs_sbpm_dbg_vec1.free_w_decode, regs_sbpm_dbg_vec1.free_w_decode);
        bdmf_session_print(session, "mcin_decode = %u = 0x%x\n", regs_sbpm_dbg_vec1.mcin_decode, regs_sbpm_dbg_vec1.mcin_decode);
        bdmf_session_print(session, "alloc_decode = %u = 0x%x\n", regs_sbpm_dbg_vec1.alloc_decode, regs_sbpm_dbg_vec1.alloc_decode);
        break;
    }
    case cli_sbpm_regs_sbpm_dbg_vec2:
    {
        sbpm_regs_sbpm_dbg_vec2 regs_sbpm_dbg_vec2;
        ag_err = ag_drv_sbpm_regs_sbpm_dbg_vec2_get(&regs_sbpm_dbg_vec2);
        bdmf_session_print(session, "tx_data_full = %u = 0x%x\n", regs_sbpm_dbg_vec2.tx_data_full, regs_sbpm_dbg_vec2.tx_data_full);
        bdmf_session_print(session, "tx_fifo_empty = %u = 0x%x\n", regs_sbpm_dbg_vec2.tx_fifo_empty, regs_sbpm_dbg_vec2.tx_fifo_empty);
        bdmf_session_print(session, "lcl_stts_full = %u = 0x%x\n", regs_sbpm_dbg_vec2.lcl_stts_full, regs_sbpm_dbg_vec2.lcl_stts_full);
        bdmf_session_print(session, "lcl_stts_empty = %u = 0x%x\n", regs_sbpm_dbg_vec2.lcl_stts_empty, regs_sbpm_dbg_vec2.lcl_stts_empty);
        bdmf_session_print(session, "tx_cmd_full = %u = 0x%x\n", regs_sbpm_dbg_vec2.tx_cmd_full, regs_sbpm_dbg_vec2.tx_cmd_full);
        bdmf_session_print(session, "tx_cmd_fifo_empty = %u = 0x%x\n", regs_sbpm_dbg_vec2.tx_cmd_fifo_empty, regs_sbpm_dbg_vec2.tx_cmd_fifo_empty);
        bdmf_session_print(session, "bb_decoder_dest_id = %u = 0x%x\n", regs_sbpm_dbg_vec2.bb_decoder_dest_id, regs_sbpm_dbg_vec2.bb_decoder_dest_id);
        bdmf_session_print(session, "tx_bbh_send_in_progress = %u = 0x%x\n", regs_sbpm_dbg_vec2.tx_bbh_send_in_progress, regs_sbpm_dbg_vec2.tx_bbh_send_in_progress);
        bdmf_session_print(session, "sp_2send = %u = 0x%x\n", regs_sbpm_dbg_vec2.sp_2send, regs_sbpm_dbg_vec2.sp_2send);
        bdmf_session_print(session, "tx2data_fifo_taddr = %u = 0x%x\n", regs_sbpm_dbg_vec2.tx2data_fifo_taddr, regs_sbpm_dbg_vec2.tx2data_fifo_taddr);
        bdmf_session_print(session, "cpu_access = %u = 0x%x\n", regs_sbpm_dbg_vec2.cpu_access, regs_sbpm_dbg_vec2.cpu_access);
        bdmf_session_print(session, "bbh_access = %u = 0x%x\n", regs_sbpm_dbg_vec2.bbh_access, regs_sbpm_dbg_vec2.bbh_access);
        bdmf_session_print(session, "rnr_access = %u = 0x%x\n", regs_sbpm_dbg_vec2.rnr_access, regs_sbpm_dbg_vec2.rnr_access);
        break;
    }
    case cli_sbpm_regs_sbpm_dbg_vec3:
    {
        sbpm_regs_sbpm_dbg_vec3 regs_sbpm_dbg_vec3;
        ag_err = ag_drv_sbpm_regs_sbpm_dbg_vec3_get(&regs_sbpm_dbg_vec3);
        bdmf_session_print(session, "alloc_rply = %u = 0x%x\n", regs_sbpm_dbg_vec3.alloc_rply, regs_sbpm_dbg_vec3.alloc_rply);
        bdmf_session_print(session, "bn_rply = %u = 0x%x\n", regs_sbpm_dbg_vec3.bn_rply, regs_sbpm_dbg_vec3.bn_rply);
        bdmf_session_print(session, "txfifo_alloc_ack = %u = 0x%x\n", regs_sbpm_dbg_vec3.txfifo_alloc_ack, regs_sbpm_dbg_vec3.txfifo_alloc_ack);
        bdmf_session_print(session, "tx_fifo_mcinc_ack = %u = 0x%x\n", regs_sbpm_dbg_vec3.tx_fifo_mcinc_ack, regs_sbpm_dbg_vec3.tx_fifo_mcinc_ack);
        bdmf_session_print(session, "txfifo_cnct_ack = %u = 0x%x\n", regs_sbpm_dbg_vec3.txfifo_cnct_ack, regs_sbpm_dbg_vec3.txfifo_cnct_ack);
        bdmf_session_print(session, "txfifo_gt_nxt_rply = %u = 0x%x\n", regs_sbpm_dbg_vec3.txfifo_gt_nxt_rply, regs_sbpm_dbg_vec3.txfifo_gt_nxt_rply);
        bdmf_session_print(session, "txfifo_mlti_gt_nxt_rply = %u = 0x%x\n", regs_sbpm_dbg_vec3.txfifo_mlti_gt_nxt_rply, regs_sbpm_dbg_vec3.txfifo_mlti_gt_nxt_rply);
        bdmf_session_print(session, "tx_msg_pipe_sm = %u = 0x%x\n", regs_sbpm_dbg_vec3.tx_msg_pipe_sm, regs_sbpm_dbg_vec3.tx_msg_pipe_sm);
        bdmf_session_print(session, "send_stt_sm = %u = 0x%x\n", regs_sbpm_dbg_vec3.send_stt_sm, regs_sbpm_dbg_vec3.send_stt_sm);
        bdmf_session_print(session, "txfifo_in2estts_chng = %u = 0x%x\n", regs_sbpm_dbg_vec3.txfifo_in2estts_chng, regs_sbpm_dbg_vec3.txfifo_in2estts_chng);
        break;
    }
    case cli_sbpm_regs_sbpm_sp_bbh_low:
    {
        uint32_t sbpm_sp_bbh_low;
        ag_err = ag_drv_sbpm_regs_sbpm_sp_bbh_low_get(&sbpm_sp_bbh_low);
        bdmf_session_print(session, "sbpm_sp_bbh_low = %u = 0x%x\n", sbpm_sp_bbh_low, sbpm_sp_bbh_low);
        break;
    }
    case cli_sbpm_regs_sbpm_sp_bbh_high:
    {
        uint32_t sbpm_sp_bbh_high;
        ag_err = ag_drv_sbpm_regs_sbpm_sp_bbh_high_get(&sbpm_sp_bbh_high);
        bdmf_session_print(session, "sbpm_sp_bbh_high = %u = 0x%x\n", sbpm_sp_bbh_high, sbpm_sp_bbh_high);
        break;
    }
    case cli_sbpm_regs_sbpm_sp_rnr_low:
    {
        uint32_t sbpm_sp_rnr_low;
        ag_err = ag_drv_sbpm_regs_sbpm_sp_rnr_low_get(&sbpm_sp_rnr_low);
        bdmf_session_print(session, "sbpm_sp_rnr_low = %u = 0x%x\n", sbpm_sp_rnr_low, sbpm_sp_rnr_low);
        break;
    }
    case cli_sbpm_regs_sbpm_sp_rnr_high:
    {
        uint32_t sbpm_sp_rnr_high;
        ag_err = ag_drv_sbpm_regs_sbpm_sp_rnr_high_get(&sbpm_sp_rnr_high);
        bdmf_session_print(session, "sbpm_sp_rnr_high = %u = 0x%x\n", sbpm_sp_rnr_high, sbpm_sp_rnr_high);
        break;
    }
    case cli_sbpm_regs_sbpm_ug_map_low:
    {
        uint32_t sbpm_ug_map_low;
        ag_err = ag_drv_sbpm_regs_sbpm_ug_map_low_get(&sbpm_ug_map_low);
        bdmf_session_print(session, "sbpm_ug_map_low = %u = 0x%x\n", sbpm_ug_map_low, sbpm_ug_map_low);
        break;
    }
    case cli_sbpm_regs_sbpm_ug_map_high:
    {
        uint32_t sbpm_ug_map_high;
        ag_err = ag_drv_sbpm_regs_sbpm_ug_map_high_get(&sbpm_ug_map_high);
        bdmf_session_print(session, "sbpm_ug_map_high = %u = 0x%x\n", sbpm_ug_map_high, sbpm_ug_map_high);
        break;
    }
    case cli_sbpm_regs_sbpm_excl_mask_low:
    {
        uint32_t sbpm_excl_mask_low;
        ag_err = ag_drv_sbpm_regs_sbpm_excl_mask_low_get(&sbpm_excl_mask_low);
        bdmf_session_print(session, "sbpm_excl_mask_low = %u = 0x%x\n", sbpm_excl_mask_low, sbpm_excl_mask_low);
        break;
    }
    case cli_sbpm_regs_sbpm_excl_mask_high:
    {
        uint32_t sbpm_excl_mask_high;
        ag_err = ag_drv_sbpm_regs_sbpm_excl_mask_high_get(&sbpm_excl_mask_high);
        bdmf_session_print(session, "sbpm_excl_mask_high = %u = 0x%x\n", sbpm_excl_mask_high, sbpm_excl_mask_high);
        break;
    }
    case cli_sbpm_regs_sbpm_raddr_decoder:
    {
        uint8_t id_2overwr;
        uint16_t overwr_ra;
        bdmf_boolean overwr_valid;
        ag_err = ag_drv_sbpm_regs_sbpm_raddr_decoder_get(&id_2overwr, &overwr_ra, &overwr_valid);
        bdmf_session_print(session, "id_2overwr = %u = 0x%x\n", id_2overwr, id_2overwr);
        bdmf_session_print(session, "overwr_ra = %u = 0x%x\n", overwr_ra, overwr_ra);
        bdmf_session_print(session, "overwr_valid = %u = 0x%x\n", overwr_valid, overwr_valid);
        break;
    }
    case cli_sbpm_regs_sbpm_wr_data:
    {
        uint32_t sbpm_wr_data;
        ag_err = ag_drv_sbpm_regs_sbpm_wr_data_get(&sbpm_wr_data);
        bdmf_session_print(session, "sbpm_wr_data = %u = 0x%x\n", sbpm_wr_data, sbpm_wr_data);
        break;
    }
    case cli_sbpm_regs_sbpm_ug_bac_max:
    {
        uint16_t ug0bacmax;
        uint16_t ug1bacmax;
        ag_err = ag_drv_sbpm_regs_sbpm_ug_bac_max_get(&ug0bacmax, &ug1bacmax);
        bdmf_session_print(session, "ug0bacmax = %u = 0x%x\n", ug0bacmax, ug0bacmax);
        bdmf_session_print(session, "ug1bacmax = %u = 0x%x\n", ug1bacmax, ug1bacmax);
        break;
    }
    case cli_sbpm_regs_sbpm_spare:
    {
        bdmf_boolean gl_bac_clear_en;
        ag_err = ag_drv_sbpm_regs_sbpm_spare_get(&gl_bac_clear_en);
        bdmf_session_print(session, "gl_bac_clear_en = %u = 0x%x\n", gl_bac_clear_en, gl_bac_clear_en);
        break;
    }
    case cli_sbpm_regs_sbpm_bac_underrun:
    {
        bdmf_boolean sbpm_cfg_bac_underrun_en;
        ag_err = ag_drv_sbpm_regs_sbpm_bac_underrun_get(&sbpm_cfg_bac_underrun_en);
        bdmf_session_print(session, "sbpm_cfg_bac_underrun_en = %u = 0x%x\n", sbpm_cfg_bac_underrun_en, sbpm_cfg_bac_underrun_en);
        break;
    }
    case cli_sbpm_intr_ctrl_isr:
    {
        sbpm_intr_ctrl_isr intr_ctrl_isr;
        ag_err = ag_drv_sbpm_intr_ctrl_isr_get(&intr_ctrl_isr);
        bdmf_session_print(session, "bac_underrun = %u = 0x%x\n", intr_ctrl_isr.bac_underrun, intr_ctrl_isr.bac_underrun);
        bdmf_session_print(session, "mcst_overflow = %u = 0x%x\n", intr_ctrl_isr.mcst_overflow, intr_ctrl_isr.mcst_overflow);
        bdmf_session_print(session, "check_last_err = %u = 0x%x\n", intr_ctrl_isr.check_last_err, intr_ctrl_isr.check_last_err);
        bdmf_session_print(session, "max_search_err = %u = 0x%x\n", intr_ctrl_isr.max_search_err, intr_ctrl_isr.max_search_err);
        bdmf_session_print(session, "invalid_in2e = %u = 0x%x\n", intr_ctrl_isr.invalid_in2e, intr_ctrl_isr.invalid_in2e);
        bdmf_session_print(session, "multi_get_next_null = %u = 0x%x\n", intr_ctrl_isr.multi_get_next_null, intr_ctrl_isr.multi_get_next_null);
        bdmf_session_print(session, "cnct_null = %u = 0x%x\n", intr_ctrl_isr.cnct_null, intr_ctrl_isr.cnct_null);
        bdmf_session_print(session, "alloc_null = %u = 0x%x\n", intr_ctrl_isr.alloc_null, intr_ctrl_isr.alloc_null);
        bdmf_session_print(session, "invalid_in2e_overflow = %u = 0x%x\n", intr_ctrl_isr.invalid_in2e_overflow, intr_ctrl_isr.invalid_in2e_overflow);
        bdmf_session_print(session, "invalid_in2e_underflow = %u = 0x%x\n", intr_ctrl_isr.invalid_in2e_underflow, intr_ctrl_isr.invalid_in2e_underflow);
        bdmf_session_print(session, "bac_underrun_ug0 = %u = 0x%x\n", intr_ctrl_isr.bac_underrun_ug0, intr_ctrl_isr.bac_underrun_ug0);
        bdmf_session_print(session, "bac_underrun_ug1 = %u = 0x%x\n", intr_ctrl_isr.bac_underrun_ug1, intr_ctrl_isr.bac_underrun_ug1);
        break;
    }
    case cli_sbpm_intr_ctrl_ism:
    {
        uint32_t ism;
        ag_err = ag_drv_sbpm_intr_ctrl_ism_get(&ism);
        bdmf_session_print(session, "ism = %u = 0x%x\n", ism, ism);
        break;
    }
    case cli_sbpm_intr_ctrl_ier:
    {
        uint32_t iem;
        ag_err = ag_drv_sbpm_intr_ctrl_ier_get(&iem);
        bdmf_session_print(session, "iem = %u = 0x%x\n", iem, iem);
        break;
    }
    case cli_sbpm_intr_ctrl_itr:
    {
        uint32_t ist;
        ag_err = ag_drv_sbpm_intr_ctrl_itr_get(&ist);
        bdmf_session_print(session, "ist = %u = 0x%x\n", ist, ist);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_sbpm_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        uint32_t sbpm_nack_mask_high = gtmv(m, 32);
        uint32_t sbpm_nack_mask_low = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_sbpm_nack_mask_get(&sbpm_nack_mask_high, &sbpm_nack_mask_low);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_nack_mask_get( %u %u)\n",
                sbpm_nack_mask_high, sbpm_nack_mask_low);
        }
    }

    {
        uint16_t bac = gtmv(m, 14);
        uint16_t ug1bac = gtmv(m, 14);
        uint16_t ug0bac = gtmv(m, 14);
        if (!ag_err)
            ag_err = ag_drv_sbpm_bac_get(&bac, &ug1bac, &ug0bac);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_bac_get( %u %u %u)\n",
                bac, ug1bac, ug0bac);
        }
    }

    {
        uint16_t init_base_addr = gtmv(m, 14);
        uint16_t init_offset = gtmv(m, 14);
        bdmf_boolean bsy = gtmv(m, 1);
        bdmf_boolean rdy = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_sbpm_regs_init_free_list_set( %u %u %u %u)\n",
            init_base_addr, init_offset, bsy, rdy);
        ag_err = ag_drv_sbpm_regs_init_free_list_set(init_base_addr, init_offset, bsy, rdy);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_init_free_list_get(&init_base_addr, &init_offset, &bsy, &rdy);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_init_free_list_get( %u %u %u %u)\n",
                init_base_addr, init_offset, bsy, rdy);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (init_base_addr != gtmv(m, 14) || init_offset != gtmv(m, 14) || bsy != gtmv(m, 1) || rdy != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t sa = gtmv(m, 6);
        bdmf_session_print(session, "ag_drv_sbpm_regs_bn_alloc_set( %u)\n",
            sa);
        ag_err = ag_drv_sbpm_regs_bn_alloc_set(sa);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_bn_alloc_get(&sa);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_bn_alloc_get( %u)\n",
                sa);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (sa != gtmv(m, 6))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        sbpm_regs_bn_alloc_rply regs_bn_alloc_rply = {.alloc_bn_valid = gtmv(m, 1), .alloc_bn = gtmv(m, 14), .ack = gtmv(m, 1), .nack = gtmv(m, 1), .excl_high = gtmv(m, 1), .excl_low = gtmv(m, 1), .busy = gtmv(m, 1), .rdy = gtmv(m, 1)};
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_bn_alloc_rply_get(&regs_bn_alloc_rply);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_bn_alloc_rply_get( %u %u %u %u %u %u %u %u)\n",
                regs_bn_alloc_rply.alloc_bn_valid, regs_bn_alloc_rply.alloc_bn, regs_bn_alloc_rply.ack, regs_bn_alloc_rply.nack, 
                regs_bn_alloc_rply.excl_high, regs_bn_alloc_rply.excl_low, regs_bn_alloc_rply.busy, regs_bn_alloc_rply.rdy);
        }
    }

    {
        uint16_t head_bn = gtmv(m, 14);
        uint8_t sa = gtmv(m, 6);
        uint8_t offset = gtmv(m, 7);
        bdmf_boolean ack = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_sbpm_regs_bn_free_with_contxt_low_set( %u %u %u %u)\n",
            head_bn, sa, offset, ack);
        ag_err = ag_drv_sbpm_regs_bn_free_with_contxt_low_set(head_bn, sa, offset, ack);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_bn_free_with_contxt_low_get(&head_bn, &sa, &offset, &ack);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_bn_free_with_contxt_low_get( %u %u %u %u)\n",
                head_bn, sa, offset, ack);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (head_bn != gtmv(m, 14) || sa != gtmv(m, 6) || offset != gtmv(m, 7) || ack != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t last_bn = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_sbpm_regs_bn_free_with_contxt_high_set( %u)\n",
            last_bn);
        ag_err = ag_drv_sbpm_regs_bn_free_with_contxt_high_set(last_bn);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_bn_free_with_contxt_high_get(&last_bn);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_bn_free_with_contxt_high_get( %u)\n",
                last_bn);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (last_bn != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t bn = gtmv(m, 14);
        uint8_t mcst_val = gtmv(m, 8);
        bdmf_boolean ack_req = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_sbpm_regs_mcst_inc_set( %u %u %u)\n",
            bn, mcst_val, ack_req);
        ag_err = ag_drv_sbpm_regs_mcst_inc_set(bn, mcst_val, ack_req);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_mcst_inc_get(&bn, &mcst_val, &ack_req);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_mcst_inc_get( %u %u %u)\n",
                bn, mcst_val, ack_req);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bn != gtmv(m, 14) || mcst_val != gtmv(m, 8) || ack_req != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean mcst_ack = gtmv(m, 1);
        bdmf_boolean bsy = gtmv(m, 1);
        bdmf_boolean rdy = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_mcst_inc_rply_get(&mcst_ack, &bsy, &rdy);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_mcst_inc_rply_get( %u %u %u)\n",
                mcst_ack, bsy, rdy);
        }
    }

    {
        uint16_t bn = gtmv(m, 14);
        bdmf_boolean ack_req = gtmv(m, 1);
        bdmf_boolean wr_req = gtmv(m, 1);
        uint16_t pointed_bn = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_sbpm_regs_bn_connect_set( %u %u %u %u)\n",
            bn, ack_req, wr_req, pointed_bn);
        ag_err = ag_drv_sbpm_regs_bn_connect_set(bn, ack_req, wr_req, pointed_bn);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_bn_connect_get(&bn, &ack_req, &wr_req, &pointed_bn);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_bn_connect_get( %u %u %u %u)\n",
                bn, ack_req, wr_req, pointed_bn);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bn != gtmv(m, 14) || ack_req != gtmv(m, 1) || wr_req != gtmv(m, 1) || pointed_bn != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean connect_ack = gtmv(m, 1);
        bdmf_boolean busy = gtmv(m, 1);
        bdmf_boolean rdy = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_bn_connect_rply_get(&connect_ack, &busy, &rdy);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_bn_connect_rply_get( %u %u %u)\n",
                connect_ack, busy, rdy);
        }
    }

    {
        uint16_t bn = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_sbpm_regs_get_next_set( %u)\n",
            bn);
        ag_err = ag_drv_sbpm_regs_get_next_set(bn);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_get_next_get(&bn);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_get_next_get( %u)\n",
                bn);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bn != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        sbpm_regs_get_next_rply regs_get_next_rply = {.bn_valid = gtmv(m, 1), .next_bn = gtmv(m, 14), .bn_null = gtmv(m, 1), .mcnt_val = gtmv(m, 8), .busy = gtmv(m, 1), .rdy = gtmv(m, 1)};
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_get_next_rply_get(&regs_get_next_rply);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_get_next_rply_get( %u %u %u %u %u %u)\n",
                regs_get_next_rply.bn_valid, regs_get_next_rply.next_bn, regs_get_next_rply.bn_null, regs_get_next_rply.mcnt_val, 
                regs_get_next_rply.busy, regs_get_next_rply.rdy);
        }
    }

    {
        sbpm_regs_sbpm_clk_gate_cntrl regs_sbpm_clk_gate_cntrl = {.bypass_clk_gate = gtmv(m, 1), .timer_val = gtmv(m, 8), .keep_alive_en = gtmv(m, 1), .keep_alive_intervl = gtmv(m, 3), .keep_alive_cyc = gtmv(m, 8)};
        bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_clk_gate_cntrl_set( %u %u %u %u %u)\n",
            regs_sbpm_clk_gate_cntrl.bypass_clk_gate, regs_sbpm_clk_gate_cntrl.timer_val, regs_sbpm_clk_gate_cntrl.keep_alive_en, regs_sbpm_clk_gate_cntrl.keep_alive_intervl, 
            regs_sbpm_clk_gate_cntrl.keep_alive_cyc);
        ag_err = ag_drv_sbpm_regs_sbpm_clk_gate_cntrl_set(&regs_sbpm_clk_gate_cntrl);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_clk_gate_cntrl_get(&regs_sbpm_clk_gate_cntrl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_clk_gate_cntrl_get( %u %u %u %u %u)\n",
                regs_sbpm_clk_gate_cntrl.bypass_clk_gate, regs_sbpm_clk_gate_cntrl.timer_val, regs_sbpm_clk_gate_cntrl.keep_alive_en, regs_sbpm_clk_gate_cntrl.keep_alive_intervl, 
                regs_sbpm_clk_gate_cntrl.keep_alive_cyc);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (regs_sbpm_clk_gate_cntrl.bypass_clk_gate != gtmv(m, 1) || regs_sbpm_clk_gate_cntrl.timer_val != gtmv(m, 8) || regs_sbpm_clk_gate_cntrl.keep_alive_en != gtmv(m, 1) || regs_sbpm_clk_gate_cntrl.keep_alive_intervl != gtmv(m, 3) || regs_sbpm_clk_gate_cntrl.keep_alive_cyc != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t head_bn = gtmv(m, 14);
        uint8_t sa = gtmv(m, 6);
        bdmf_boolean ack_req = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_sbpm_regs_bn_free_without_contxt_set( %u %u %u)\n",
            head_bn, sa, ack_req);
        ag_err = ag_drv_sbpm_regs_bn_free_without_contxt_set(head_bn, sa, ack_req);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_bn_free_without_contxt_get(&head_bn, &sa, &ack_req);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_bn_free_without_contxt_get( %u %u %u)\n",
                head_bn, sa, ack_req);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (head_bn != gtmv(m, 14) || sa != gtmv(m, 6) || ack_req != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        sbpm_regs_bn_free_without_contxt_rply regs_bn_free_without_contxt_rply = {.free_ack = gtmv(m, 1), .ack_stat = gtmv(m, 1), .nack_stat = gtmv(m, 1), .excl_high_stat = gtmv(m, 1), .excl_low_stat = gtmv(m, 1), .bsy = gtmv(m, 1), .rdy = gtmv(m, 1)};
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_bn_free_without_contxt_rply_get(&regs_bn_free_without_contxt_rply);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_bn_free_without_contxt_rply_get( %u %u %u %u %u %u %u)\n",
                regs_bn_free_without_contxt_rply.free_ack, regs_bn_free_without_contxt_rply.ack_stat, regs_bn_free_without_contxt_rply.nack_stat, regs_bn_free_without_contxt_rply.excl_high_stat, 
                regs_bn_free_without_contxt_rply.excl_low_stat, regs_bn_free_without_contxt_rply.bsy, regs_bn_free_without_contxt_rply.rdy);
        }
    }

    {
        sbpm_regs_bn_free_with_contxt_rply regs_bn_free_with_contxt_rply = {.free_ack = gtmv(m, 1), .ack_state = gtmv(m, 1), .nack_state = gtmv(m, 1), .excl_high_state = gtmv(m, 1), .excl_low_state = gtmv(m, 1), .busy = gtmv(m, 1), .rdy = gtmv(m, 1)};
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_bn_free_with_contxt_rply_get(&regs_bn_free_with_contxt_rply);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_bn_free_with_contxt_rply_get( %u %u %u %u %u %u %u)\n",
                regs_bn_free_with_contxt_rply.free_ack, regs_bn_free_with_contxt_rply.ack_state, regs_bn_free_with_contxt_rply.nack_state, regs_bn_free_with_contxt_rply.excl_high_state, 
                regs_bn_free_with_contxt_rply.excl_low_state, regs_bn_free_with_contxt_rply.busy, regs_bn_free_with_contxt_rply.rdy);
        }
    }

    {
        uint16_t gl_bat = gtmv(m, 14);
        uint16_t gl_bah = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_gl_trsh_set( %u %u)\n",
            gl_bat, gl_bah);
        ag_err = ag_drv_sbpm_regs_sbpm_gl_trsh_set(gl_bat, gl_bah);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_gl_trsh_get(&gl_bat, &gl_bah);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_gl_trsh_get( %u %u)\n",
                gl_bat, gl_bah);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (gl_bat != gtmv(m, 14) || gl_bah != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t ug_bat = gtmv(m, 14);
        uint16_t ug_bah = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_ug0_trsh_set( %u %u)\n",
            ug_bat, ug_bah);
        ag_err = ag_drv_sbpm_regs_sbpm_ug0_trsh_set(ug_bat, ug_bah);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_ug0_trsh_get(&ug_bat, &ug_bah);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_ug0_trsh_get( %u %u)\n",
                ug_bat, ug_bah);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ug_bat != gtmv(m, 14) || ug_bah != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t ug_bat = gtmv(m, 14);
        uint16_t ug_bah = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_ug1_trsh_set( %u %u)\n",
            ug_bat, ug_bah);
        ag_err = ag_drv_sbpm_regs_sbpm_ug1_trsh_set(ug_bat, ug_bah);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_ug1_trsh_get(&ug_bat, &ug_bah);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_ug1_trsh_get( %u %u)\n",
                ug_bat, ug_bah);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ug_bat != gtmv(m, 14) || ug_bah != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t select_bus = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_dbg_set( %u)\n",
            select_bus);
        ag_err = ag_drv_sbpm_regs_sbpm_dbg_set(select_bus);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_dbg_get(&select_bus);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_dbg_get( %u)\n",
                select_bus);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (select_bus != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t exclt = gtmv(m, 14);
        uint16_t exclh = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_ug0_excl_high_trsh_set( %u %u)\n",
            exclt, exclh);
        ag_err = ag_drv_sbpm_regs_sbpm_ug0_excl_high_trsh_set(exclt, exclh);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_ug0_excl_high_trsh_get(&exclt, &exclh);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_ug0_excl_high_trsh_get( %u %u)\n",
                exclt, exclh);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (exclt != gtmv(m, 14) || exclh != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t exclt = gtmv(m, 14);
        uint16_t exclh = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_ug1_excl_high_trsh_set( %u %u)\n",
            exclt, exclh);
        ag_err = ag_drv_sbpm_regs_sbpm_ug1_excl_high_trsh_set(exclt, exclh);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_ug1_excl_high_trsh_get(&exclt, &exclh);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_ug1_excl_high_trsh_get( %u %u)\n",
                exclt, exclh);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (exclt != gtmv(m, 14) || exclh != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t exclt = gtmv(m, 14);
        uint16_t exclh = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_ug0_excl_low_trsh_set( %u %u)\n",
            exclt, exclh);
        ag_err = ag_drv_sbpm_regs_sbpm_ug0_excl_low_trsh_set(exclt, exclh);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_ug0_excl_low_trsh_get(&exclt, &exclh);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_ug0_excl_low_trsh_get( %u %u)\n",
                exclt, exclh);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (exclt != gtmv(m, 14) || exclh != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t exclt = gtmv(m, 14);
        uint16_t exclh = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_ug1_excl_low_trsh_set( %u %u)\n",
            exclt, exclh);
        ag_err = ag_drv_sbpm_regs_sbpm_ug1_excl_low_trsh_set(exclt, exclh);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_ug1_excl_low_trsh_get(&exclt, &exclh);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_ug1_excl_low_trsh_get( %u %u)\n",
                exclt, exclh);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (exclt != gtmv(m, 14) || exclh != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t ug_ack_stts = gtmv(m, 2);
        uint8_t ug_excl_high_stts = gtmv(m, 2);
        uint8_t ug_excl_low_stts = gtmv(m, 2);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_ug_status_get(&ug_ack_stts, &ug_excl_high_stts, &ug_excl_low_stts);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_ug_status_get( %u %u %u)\n",
                ug_ack_stts, ug_excl_high_stts, ug_excl_low_stts);
        }
    }

    {
        uint8_t search_depth = gtmv(m, 7);
        bdmf_boolean max_search_en = gtmv(m, 1);
        bdmf_boolean chck_last_en = gtmv(m, 1);
        bdmf_boolean freeze_in_error = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_sbpm_regs_error_handling_params_set( %u %u %u %u)\n",
            search_depth, max_search_en, chck_last_en, freeze_in_error);
        ag_err = ag_drv_sbpm_regs_error_handling_params_set(search_depth, max_search_en, chck_last_en, freeze_in_error);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_error_handling_params_get(&search_depth, &max_search_en, &chck_last_en, &freeze_in_error);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_error_handling_params_get( %u %u %u %u)\n",
                search_depth, max_search_en, chck_last_en, freeze_in_error);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (search_depth != gtmv(m, 7) || max_search_en != gtmv(m, 1) || chck_last_en != gtmv(m, 1) || freeze_in_error != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t cmd_sa = gtmv(m, 6);
        uint8_t cmd_ta = gtmv(m, 3);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_iir_addr_get(&cmd_sa, &cmd_ta);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_iir_addr_get( %u %u)\n",
                cmd_sa, cmd_ta);
        }
    }

    {
        uint32_t cmd_data_0to31 = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_iir_low_get(&cmd_data_0to31);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_iir_low_get( %u)\n",
                cmd_data_0to31);
        }
    }

    {
        uint32_t cmd_data_32to63 = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_iir_high_get(&cmd_data_32to63);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_iir_high_get( %u)\n",
                cmd_data_32to63);
        }
    }

    {
        sbpm_regs_sbpm_dbg_vec0 regs_sbpm_dbg_vec0 = {.alloc_sm = gtmv(m, 2), .cnnct_sm = gtmv(m, 1), .mcint_sm = gtmv(m, 4), .free_w_cnxt_sm = gtmv(m, 4), .free_wo_cnxt_sm = gtmv(m, 4), .gn_sm = gtmv(m, 2), .multi_gn_sm = gtmv(m, 4), .free_lst_hd = gtmv(m, 11)};
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_dbg_vec0_get(&regs_sbpm_dbg_vec0);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_dbg_vec0_get( %u %u %u %u %u %u %u %u)\n",
                regs_sbpm_dbg_vec0.alloc_sm, regs_sbpm_dbg_vec0.cnnct_sm, regs_sbpm_dbg_vec0.mcint_sm, regs_sbpm_dbg_vec0.free_w_cnxt_sm, 
                regs_sbpm_dbg_vec0.free_wo_cnxt_sm, regs_sbpm_dbg_vec0.gn_sm, regs_sbpm_dbg_vec0.multi_gn_sm, regs_sbpm_dbg_vec0.free_lst_hd);
        }
    }

    {
        sbpm_regs_sbpm_dbg_vec1 regs_sbpm_dbg_vec1 = {.in2e_valid = gtmv(m, 1), .multi_gn_valid = gtmv(m, 4), .ug_active = gtmv(m, 2), .tx_cmd_full = gtmv(m, 1), .rx_fifo_pop = gtmv(m, 1), .ram_init_start = gtmv(m, 1), .ram_init_done = gtmv(m, 1), .rx_fifo_data = gtmv(m, 10), .free_decode = gtmv(m, 1), .in2e_decode = gtmv(m, 1), .free_wo_decode = gtmv(m, 1), .get_nxt_decode = gtmv(m, 1), .multi_get_nxt_decode = gtmv(m, 1), .cnct_decode = gtmv(m, 1), .free_w_decode = gtmv(m, 1), .mcin_decode = gtmv(m, 1), .alloc_decode = gtmv(m, 1)};
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_dbg_vec1_get(&regs_sbpm_dbg_vec1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_dbg_vec1_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
                regs_sbpm_dbg_vec1.in2e_valid, regs_sbpm_dbg_vec1.multi_gn_valid, regs_sbpm_dbg_vec1.ug_active, regs_sbpm_dbg_vec1.tx_cmd_full, 
                regs_sbpm_dbg_vec1.rx_fifo_pop, regs_sbpm_dbg_vec1.ram_init_start, regs_sbpm_dbg_vec1.ram_init_done, regs_sbpm_dbg_vec1.rx_fifo_data, 
                regs_sbpm_dbg_vec1.free_decode, regs_sbpm_dbg_vec1.in2e_decode, regs_sbpm_dbg_vec1.free_wo_decode, regs_sbpm_dbg_vec1.get_nxt_decode, 
                regs_sbpm_dbg_vec1.multi_get_nxt_decode, regs_sbpm_dbg_vec1.cnct_decode, regs_sbpm_dbg_vec1.free_w_decode, regs_sbpm_dbg_vec1.mcin_decode, 
                regs_sbpm_dbg_vec1.alloc_decode);
        }
    }

    {
        sbpm_regs_sbpm_dbg_vec2 regs_sbpm_dbg_vec2 = {.tx_data_full = gtmv(m, 1), .tx_fifo_empty = gtmv(m, 1), .lcl_stts_full = gtmv(m, 1), .lcl_stts_empty = gtmv(m, 1), .tx_cmd_full = gtmv(m, 1), .tx_cmd_fifo_empty = gtmv(m, 1), .bb_decoder_dest_id = gtmv(m, 6), .tx_bbh_send_in_progress = gtmv(m, 1), .sp_2send = gtmv(m, 6), .tx2data_fifo_taddr = gtmv(m, 3), .cpu_access = gtmv(m, 1), .bbh_access = gtmv(m, 1), .rnr_access = gtmv(m, 1)};
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_dbg_vec2_get(&regs_sbpm_dbg_vec2);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_dbg_vec2_get( %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
                regs_sbpm_dbg_vec2.tx_data_full, regs_sbpm_dbg_vec2.tx_fifo_empty, regs_sbpm_dbg_vec2.lcl_stts_full, regs_sbpm_dbg_vec2.lcl_stts_empty, 
                regs_sbpm_dbg_vec2.tx_cmd_full, regs_sbpm_dbg_vec2.tx_cmd_fifo_empty, regs_sbpm_dbg_vec2.bb_decoder_dest_id, regs_sbpm_dbg_vec2.tx_bbh_send_in_progress, 
                regs_sbpm_dbg_vec2.sp_2send, regs_sbpm_dbg_vec2.tx2data_fifo_taddr, regs_sbpm_dbg_vec2.cpu_access, regs_sbpm_dbg_vec2.bbh_access, 
                regs_sbpm_dbg_vec2.rnr_access);
        }
    }

    {
        sbpm_regs_sbpm_dbg_vec3 regs_sbpm_dbg_vec3 = {.alloc_rply = gtmv(m, 1), .bn_rply = gtmv(m, 11), .txfifo_alloc_ack = gtmv(m, 1), .tx_fifo_mcinc_ack = gtmv(m, 1), .txfifo_cnct_ack = gtmv(m, 1), .txfifo_gt_nxt_rply = gtmv(m, 1), .txfifo_mlti_gt_nxt_rply = gtmv(m, 1), .tx_msg_pipe_sm = gtmv(m, 2), .send_stt_sm = gtmv(m, 1), .txfifo_in2estts_chng = gtmv(m, 1)};
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_dbg_vec3_get(&regs_sbpm_dbg_vec3);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_dbg_vec3_get( %u %u %u %u %u %u %u %u %u %u)\n",
                regs_sbpm_dbg_vec3.alloc_rply, regs_sbpm_dbg_vec3.bn_rply, regs_sbpm_dbg_vec3.txfifo_alloc_ack, regs_sbpm_dbg_vec3.tx_fifo_mcinc_ack, 
                regs_sbpm_dbg_vec3.txfifo_cnct_ack, regs_sbpm_dbg_vec3.txfifo_gt_nxt_rply, regs_sbpm_dbg_vec3.txfifo_mlti_gt_nxt_rply, regs_sbpm_dbg_vec3.tx_msg_pipe_sm, 
                regs_sbpm_dbg_vec3.send_stt_sm, regs_sbpm_dbg_vec3.txfifo_in2estts_chng);
        }
    }

    {
        uint32_t sbpm_sp_bbh_low = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_sp_bbh_low_set( %u)\n",
            sbpm_sp_bbh_low);
        ag_err = ag_drv_sbpm_regs_sbpm_sp_bbh_low_set(sbpm_sp_bbh_low);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_sp_bbh_low_get(&sbpm_sp_bbh_low);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_sp_bbh_low_get( %u)\n",
                sbpm_sp_bbh_low);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (sbpm_sp_bbh_low != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t sbpm_sp_bbh_high = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_sp_bbh_high_set( %u)\n",
            sbpm_sp_bbh_high);
        ag_err = ag_drv_sbpm_regs_sbpm_sp_bbh_high_set(sbpm_sp_bbh_high);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_sp_bbh_high_get(&sbpm_sp_bbh_high);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_sp_bbh_high_get( %u)\n",
                sbpm_sp_bbh_high);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (sbpm_sp_bbh_high != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t sbpm_sp_rnr_low = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_sp_rnr_low_set( %u)\n",
            sbpm_sp_rnr_low);
        ag_err = ag_drv_sbpm_regs_sbpm_sp_rnr_low_set(sbpm_sp_rnr_low);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_sp_rnr_low_get(&sbpm_sp_rnr_low);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_sp_rnr_low_get( %u)\n",
                sbpm_sp_rnr_low);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (sbpm_sp_rnr_low != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t sbpm_sp_rnr_high = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_sp_rnr_high_set( %u)\n",
            sbpm_sp_rnr_high);
        ag_err = ag_drv_sbpm_regs_sbpm_sp_rnr_high_set(sbpm_sp_rnr_high);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_sp_rnr_high_get(&sbpm_sp_rnr_high);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_sp_rnr_high_get( %u)\n",
                sbpm_sp_rnr_high);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (sbpm_sp_rnr_high != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t sbpm_ug_map_low = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_ug_map_low_set( %u)\n",
            sbpm_ug_map_low);
        ag_err = ag_drv_sbpm_regs_sbpm_ug_map_low_set(sbpm_ug_map_low);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_ug_map_low_get(&sbpm_ug_map_low);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_ug_map_low_get( %u)\n",
                sbpm_ug_map_low);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (sbpm_ug_map_low != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t sbpm_ug_map_high = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_ug_map_high_set( %u)\n",
            sbpm_ug_map_high);
        ag_err = ag_drv_sbpm_regs_sbpm_ug_map_high_set(sbpm_ug_map_high);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_ug_map_high_get(&sbpm_ug_map_high);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_ug_map_high_get( %u)\n",
                sbpm_ug_map_high);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (sbpm_ug_map_high != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t sbpm_excl_mask_low = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_excl_mask_low_set( %u)\n",
            sbpm_excl_mask_low);
        ag_err = ag_drv_sbpm_regs_sbpm_excl_mask_low_set(sbpm_excl_mask_low);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_excl_mask_low_get(&sbpm_excl_mask_low);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_excl_mask_low_get( %u)\n",
                sbpm_excl_mask_low);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (sbpm_excl_mask_low != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t sbpm_excl_mask_high = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_excl_mask_high_set( %u)\n",
            sbpm_excl_mask_high);
        ag_err = ag_drv_sbpm_regs_sbpm_excl_mask_high_set(sbpm_excl_mask_high);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_excl_mask_high_get(&sbpm_excl_mask_high);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_excl_mask_high_get( %u)\n",
                sbpm_excl_mask_high);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (sbpm_excl_mask_high != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t id_2overwr = gtmv(m, 6);
        uint16_t overwr_ra = gtmv(m, 10);
        bdmf_boolean overwr_valid = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_raddr_decoder_set( %u %u %u)\n",
            id_2overwr, overwr_ra, overwr_valid);
        ag_err = ag_drv_sbpm_regs_sbpm_raddr_decoder_set(id_2overwr, overwr_ra, overwr_valid);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_raddr_decoder_get(&id_2overwr, &overwr_ra, &overwr_valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_raddr_decoder_get( %u %u %u)\n",
                id_2overwr, overwr_ra, overwr_valid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (id_2overwr != gtmv(m, 6) || overwr_ra != gtmv(m, 10) || overwr_valid != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t sbpm_wr_data = gtmv(m, 22);
        bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_wr_data_set( %u)\n",
            sbpm_wr_data);
        ag_err = ag_drv_sbpm_regs_sbpm_wr_data_set(sbpm_wr_data);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_wr_data_get(&sbpm_wr_data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_wr_data_get( %u)\n",
                sbpm_wr_data);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (sbpm_wr_data != gtmv(m, 22))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t ug0bacmax = gtmv(m, 14);
        uint16_t ug1bacmax = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_ug_bac_max_set( %u %u)\n",
            ug0bacmax, ug1bacmax);
        ag_err = ag_drv_sbpm_regs_sbpm_ug_bac_max_set(ug0bacmax, ug1bacmax);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_ug_bac_max_get(&ug0bacmax, &ug1bacmax);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_ug_bac_max_get( %u %u)\n",
                ug0bacmax, ug1bacmax);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ug0bacmax != gtmv(m, 14) || ug1bacmax != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean gl_bac_clear_en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_spare_set( %u)\n",
            gl_bac_clear_en);
        ag_err = ag_drv_sbpm_regs_sbpm_spare_set(gl_bac_clear_en);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_spare_get(&gl_bac_clear_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_spare_get( %u)\n",
                gl_bac_clear_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (gl_bac_clear_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean sbpm_cfg_bac_underrun_en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_bac_underrun_set( %u)\n",
            sbpm_cfg_bac_underrun_en);
        ag_err = ag_drv_sbpm_regs_sbpm_bac_underrun_set(sbpm_cfg_bac_underrun_en);
        if (!ag_err)
            ag_err = ag_drv_sbpm_regs_sbpm_bac_underrun_get(&sbpm_cfg_bac_underrun_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_regs_sbpm_bac_underrun_get( %u)\n",
                sbpm_cfg_bac_underrun_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (sbpm_cfg_bac_underrun_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        sbpm_intr_ctrl_isr intr_ctrl_isr = {.bac_underrun = gtmv(m, 1), .mcst_overflow = gtmv(m, 1), .check_last_err = gtmv(m, 1), .max_search_err = gtmv(m, 1), .invalid_in2e = gtmv(m, 1), .multi_get_next_null = gtmv(m, 1), .cnct_null = gtmv(m, 1), .alloc_null = gtmv(m, 1), .invalid_in2e_overflow = gtmv(m, 1), .invalid_in2e_underflow = gtmv(m, 1), .bac_underrun_ug0 = gtmv(m, 1), .bac_underrun_ug1 = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_sbpm_intr_ctrl_isr_set( %u %u %u %u %u %u %u %u %u %u %u %u)\n",
            intr_ctrl_isr.bac_underrun, intr_ctrl_isr.mcst_overflow, intr_ctrl_isr.check_last_err, intr_ctrl_isr.max_search_err, 
            intr_ctrl_isr.invalid_in2e, intr_ctrl_isr.multi_get_next_null, intr_ctrl_isr.cnct_null, intr_ctrl_isr.alloc_null, 
            intr_ctrl_isr.invalid_in2e_overflow, intr_ctrl_isr.invalid_in2e_underflow, intr_ctrl_isr.bac_underrun_ug0, intr_ctrl_isr.bac_underrun_ug1);
        ag_err = ag_drv_sbpm_intr_ctrl_isr_set(&intr_ctrl_isr);
        if (!ag_err)
            ag_err = ag_drv_sbpm_intr_ctrl_isr_get(&intr_ctrl_isr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_intr_ctrl_isr_get( %u %u %u %u %u %u %u %u %u %u %u %u)\n",
                intr_ctrl_isr.bac_underrun, intr_ctrl_isr.mcst_overflow, intr_ctrl_isr.check_last_err, intr_ctrl_isr.max_search_err, 
                intr_ctrl_isr.invalid_in2e, intr_ctrl_isr.multi_get_next_null, intr_ctrl_isr.cnct_null, intr_ctrl_isr.alloc_null, 
                intr_ctrl_isr.invalid_in2e_overflow, intr_ctrl_isr.invalid_in2e_underflow, intr_ctrl_isr.bac_underrun_ug0, intr_ctrl_isr.bac_underrun_ug1);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (intr_ctrl_isr.bac_underrun != gtmv(m, 1) || intr_ctrl_isr.mcst_overflow != gtmv(m, 1) || intr_ctrl_isr.check_last_err != gtmv(m, 1) || intr_ctrl_isr.max_search_err != gtmv(m, 1) || intr_ctrl_isr.invalid_in2e != gtmv(m, 1) || intr_ctrl_isr.multi_get_next_null != gtmv(m, 1) || intr_ctrl_isr.cnct_null != gtmv(m, 1) || intr_ctrl_isr.alloc_null != gtmv(m, 1) || intr_ctrl_isr.invalid_in2e_overflow != gtmv(m, 1) || intr_ctrl_isr.invalid_in2e_underflow != gtmv(m, 1) || intr_ctrl_isr.bac_underrun_ug0 != gtmv(m, 1) || intr_ctrl_isr.bac_underrun_ug1 != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t ism = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_sbpm_intr_ctrl_ism_get(&ism);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_intr_ctrl_ism_get( %u)\n",
                ism);
        }
    }

    {
        uint32_t iem = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_sbpm_intr_ctrl_ier_set( %u)\n",
            iem);
        ag_err = ag_drv_sbpm_intr_ctrl_ier_set(iem);
        if (!ag_err)
            ag_err = ag_drv_sbpm_intr_ctrl_ier_get(&iem);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_intr_ctrl_ier_get( %u)\n",
                iem);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (iem != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t ist = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_sbpm_intr_ctrl_itr_set( %u)\n",
            ist);
        ag_err = ag_drv_sbpm_intr_ctrl_itr_set(ist);
        if (!ag_err)
            ag_err = ag_drv_sbpm_intr_ctrl_itr_get(&ist);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_sbpm_intr_ctrl_itr_get( %u)\n",
                ist);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ist != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    bdmf_session_print(session, "successes=%u failures=%u\n", test_success_cnt, test_failure_cnt);

    return ag_err;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_sbpm_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    int chip_rev_idx = RU_CHIP_REV_IDX_GET();
    uint32_t i;
    uint32_t j;
    uint32_t index1_start = 0;
    uint32_t index1_stop;
    uint32_t index2_start = 0;
    uint32_t index2_stop;
    bdmfmon_cmd_parm_t *cliparm;
    const ru_reg_rec *reg;
    const ru_block_rec *blk;
    const char *enum_string = bdmfmon_enum_parm_stringval(session, &parm[0]);

    if(!enum_string)
        return BDMF_ERR_INTERNAL;

    switch (parm[0].value.unumber)
    {
    case bdmf_address_regs_init_free_list: reg = &RU_REG(SBPM, REGS_INIT_FREE_LIST); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_bn_alloc: reg = &RU_REG(SBPM, REGS_BN_ALLOC); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_bn_alloc_rply: reg = &RU_REG(SBPM, REGS_BN_ALLOC_RPLY); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_bn_free_with_contxt_low: reg = &RU_REG(SBPM, REGS_BN_FREE_WITH_CONTXT_LOW); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_bn_free_with_contxt_high: reg = &RU_REG(SBPM, REGS_BN_FREE_WITH_CONTXT_HIGH); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_mcst_inc: reg = &RU_REG(SBPM, REGS_MCST_INC); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_mcst_inc_rply: reg = &RU_REG(SBPM, REGS_MCST_INC_RPLY); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_bn_connect: reg = &RU_REG(SBPM, REGS_BN_CONNECT); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_bn_connect_rply: reg = &RU_REG(SBPM, REGS_BN_CONNECT_RPLY); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_get_next: reg = &RU_REG(SBPM, REGS_GET_NEXT); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_get_next_rply: reg = &RU_REG(SBPM, REGS_GET_NEXT_RPLY); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_clk_gate_cntrl: reg = &RU_REG(SBPM, REGS_SBPM_CLK_GATE_CNTRL); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_bn_free_without_contxt: reg = &RU_REG(SBPM, REGS_BN_FREE_WITHOUT_CONTXT); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_bn_free_without_contxt_rply: reg = &RU_REG(SBPM, REGS_BN_FREE_WITHOUT_CONTXT_RPLY); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_bn_free_with_contxt_rply: reg = &RU_REG(SBPM, REGS_BN_FREE_WITH_CONTXT_RPLY); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_gl_trsh: reg = &RU_REG(SBPM, REGS_SBPM_GL_TRSH); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_ug0_trsh: reg = &RU_REG(SBPM, REGS_SBPM_UG0_TRSH); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_ug1_trsh: reg = &RU_REG(SBPM, REGS_SBPM_UG1_TRSH); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_dbg: reg = &RU_REG(SBPM, REGS_SBPM_DBG); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_ug0_bac: reg = &RU_REG(SBPM, REGS_SBPM_UG0_BAC); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_ug1_bac: reg = &RU_REG(SBPM, REGS_SBPM_UG1_BAC); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_gl_bac: reg = &RU_REG(SBPM, REGS_SBPM_GL_BAC); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_ug0_excl_high_trsh: reg = &RU_REG(SBPM, REGS_SBPM_UG0_EXCL_HIGH_TRSH); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_ug1_excl_high_trsh: reg = &RU_REG(SBPM, REGS_SBPM_UG1_EXCL_HIGH_TRSH); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_ug0_excl_low_trsh: reg = &RU_REG(SBPM, REGS_SBPM_UG0_EXCL_LOW_TRSH); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_ug1_excl_low_trsh: reg = &RU_REG(SBPM, REGS_SBPM_UG1_EXCL_LOW_TRSH); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_ug_status: reg = &RU_REG(SBPM, REGS_SBPM_UG_STATUS); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_error_handling_params: reg = &RU_REG(SBPM, REGS_ERROR_HANDLING_PARAMS); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_iir_addr: reg = &RU_REG(SBPM, REGS_SBPM_IIR_ADDR); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_iir_low: reg = &RU_REG(SBPM, REGS_SBPM_IIR_LOW); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_iir_high: reg = &RU_REG(SBPM, REGS_SBPM_IIR_HIGH); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_dbg_vec0: reg = &RU_REG(SBPM, REGS_SBPM_DBG_VEC0); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_dbg_vec1: reg = &RU_REG(SBPM, REGS_SBPM_DBG_VEC1); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_dbg_vec2: reg = &RU_REG(SBPM, REGS_SBPM_DBG_VEC2); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_dbg_vec3: reg = &RU_REG(SBPM, REGS_SBPM_DBG_VEC3); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_sp_bbh_low: reg = &RU_REG(SBPM, REGS_SBPM_SP_BBH_LOW); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_sp_bbh_high: reg = &RU_REG(SBPM, REGS_SBPM_SP_BBH_HIGH); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_sp_rnr_low: reg = &RU_REG(SBPM, REGS_SBPM_SP_RNR_LOW); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_sp_rnr_high: reg = &RU_REG(SBPM, REGS_SBPM_SP_RNR_HIGH); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_ug_map_low: reg = &RU_REG(SBPM, REGS_SBPM_UG_MAP_LOW); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_ug_map_high: reg = &RU_REG(SBPM, REGS_SBPM_UG_MAP_HIGH); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_nack_mask_low: reg = &RU_REG(SBPM, REGS_SBPM_NACK_MASK_LOW); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_nack_mask_high: reg = &RU_REG(SBPM, REGS_SBPM_NACK_MASK_HIGH); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_excl_mask_low: reg = &RU_REG(SBPM, REGS_SBPM_EXCL_MASK_LOW); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_excl_mask_high: reg = &RU_REG(SBPM, REGS_SBPM_EXCL_MASK_HIGH); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_raddr_decoder: reg = &RU_REG(SBPM, REGS_SBPM_RADDR_DECODER); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_wr_data: reg = &RU_REG(SBPM, REGS_SBPM_WR_DATA); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_ug_bac_max: reg = &RU_REG(SBPM, REGS_SBPM_UG_BAC_MAX); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_spare: reg = &RU_REG(SBPM, REGS_SBPM_SPARE); blk = &RU_BLK(SBPM); break;
    case bdmf_address_regs_sbpm_bac_underrun: reg = &RU_REG(SBPM, REGS_SBPM_BAC_UNDERRUN); blk = &RU_BLK(SBPM); break;
    case bdmf_address_intr_ctrl_isr: reg = &RU_REG(SBPM, INTR_CTRL_ISR); blk = &RU_BLK(SBPM); break;
    case bdmf_address_intr_ctrl_ism: reg = &RU_REG(SBPM, INTR_CTRL_ISM); blk = &RU_BLK(SBPM); break;
    case bdmf_address_intr_ctrl_ier: reg = &RU_REG(SBPM, INTR_CTRL_IER); blk = &RU_BLK(SBPM); break;
    case bdmf_address_intr_ctrl_itr: reg = &RU_REG(SBPM, INTR_CTRL_ITR); blk = &RU_BLK(SBPM); break;
    default :
        return BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    if ((cliparm = bdmfmon_cmd_find(session, "index1")))
    {
        index1_start = cliparm->value.unumber;
        index1_stop = index1_start + 1;
    }
    else
        index1_stop = blk->addr_count;
    if ((cliparm = bdmfmon_cmd_find(session, "index2")))
    {
        index2_start = cliparm->value.unumber;
        index2_stop = index2_start + 1;
    }
    else
        index2_stop = reg->ram_count;
    if (index1_stop > blk->addr_count)
    {
        bdmf_session_print(session, "index1 (%u) is out of range (%u).\n", index1_stop, blk->addr_count);
        return BDMF_ERR_RANGE;
    }
    if (index2_stop > (reg->ram_count))
    {
        bdmf_session_print(session, "index2 (%u) is out of range (%u).\n", index2_stop, reg->ram_count);
        return BDMF_ERR_RANGE;
    }
    if (reg->ram_count)
        for (i = index1_start; i < index1_stop; i++)
        {
            bdmf_session_print(session, "index1 = %u\n", i);
            for (j = index2_start; j < index2_stop; j++)
                bdmf_session_print(session, TAB "(%5u) 0x%08X\n", j, ((blk->addr[i] + reg->addr[chip_rev_idx]) + j));
        }
    else
        for (i = index1_start; i < index1_stop; i++)
            bdmf_session_print(session, "(%3u) 0x%08X\n", i, blk->addr[i]+reg->addr[chip_rev_idx]);
    return 0;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

bdmfmon_handle_t ag_drv_sbpm_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "sbpm", "sbpm", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_regs_init_free_list[] = {
            BDMFMON_MAKE_PARM("init_base_addr", "init_base_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("init_offset", "init_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bsy", "bsy", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rdy", "rdy", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_bn_alloc[] = {
            BDMFMON_MAKE_PARM("sa", "sa", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_bn_free_with_contxt_low[] = {
            BDMFMON_MAKE_PARM("head_bn", "head_bn", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sa", "sa", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("offset", "offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ack", "ack", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_bn_free_with_contxt_high[] = {
            BDMFMON_MAKE_PARM("last_bn", "last_bn", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_mcst_inc[] = {
            BDMFMON_MAKE_PARM("bn", "bn", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mcst_val", "mcst_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ack_req", "ack_req", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_bn_connect[] = {
            BDMFMON_MAKE_PARM("bn", "bn", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ack_req", "ack_req", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("wr_req", "wr_req", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pointed_bn", "pointed_bn", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_get_next[] = {
            BDMFMON_MAKE_PARM("bn", "bn", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_clk_gate_cntrl[] = {
            BDMFMON_MAKE_PARM("bypass_clk_gate", "bypass_clk_gate", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("timer_val", "timer_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_en", "keep_alive_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_intervl", "keep_alive_intervl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_cyc", "keep_alive_cyc", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_bn_free_without_contxt[] = {
            BDMFMON_MAKE_PARM("head_bn", "head_bn", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sa", "sa", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ack_req", "ack_req", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_gl_trsh[] = {
            BDMFMON_MAKE_PARM("gl_bat", "gl_bat", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gl_bah", "gl_bah", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_ug0_trsh[] = {
            BDMFMON_MAKE_PARM("ug_bat", "ug_bat", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ug_bah", "ug_bah", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_ug1_trsh[] = {
            BDMFMON_MAKE_PARM("ug_bat", "ug_bat", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ug_bah", "ug_bah", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_dbg[] = {
            BDMFMON_MAKE_PARM("select_bus", "select_bus", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_ug0_excl_high_trsh[] = {
            BDMFMON_MAKE_PARM("exclt", "exclt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("exclh", "exclh", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_ug1_excl_high_trsh[] = {
            BDMFMON_MAKE_PARM("exclt", "exclt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("exclh", "exclh", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_ug0_excl_low_trsh[] = {
            BDMFMON_MAKE_PARM("exclt", "exclt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("exclh", "exclh", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_ug1_excl_low_trsh[] = {
            BDMFMON_MAKE_PARM("exclt", "exclt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("exclh", "exclh", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_error_handling_params[] = {
            BDMFMON_MAKE_PARM("search_depth", "search_depth", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("max_search_en", "max_search_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chck_last_en", "chck_last_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("freeze_in_error", "freeze_in_error", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_sp_bbh_low[] = {
            BDMFMON_MAKE_PARM("sbpm_sp_bbh_low", "sbpm_sp_bbh_low", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_sp_bbh_high[] = {
            BDMFMON_MAKE_PARM("sbpm_sp_bbh_high", "sbpm_sp_bbh_high", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_sp_rnr_low[] = {
            BDMFMON_MAKE_PARM("sbpm_sp_rnr_low", "sbpm_sp_rnr_low", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_sp_rnr_high[] = {
            BDMFMON_MAKE_PARM("sbpm_sp_rnr_high", "sbpm_sp_rnr_high", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_ug_map_low[] = {
            BDMFMON_MAKE_PARM("sbpm_ug_map_low", "sbpm_ug_map_low", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_ug_map_high[] = {
            BDMFMON_MAKE_PARM("sbpm_ug_map_high", "sbpm_ug_map_high", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_excl_mask_low[] = {
            BDMFMON_MAKE_PARM("sbpm_excl_mask_low", "sbpm_excl_mask_low", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_excl_mask_high[] = {
            BDMFMON_MAKE_PARM("sbpm_excl_mask_high", "sbpm_excl_mask_high", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_raddr_decoder[] = {
            BDMFMON_MAKE_PARM("id_2overwr", "id_2overwr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("overwr_ra", "overwr_ra", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("overwr_valid", "overwr_valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_wr_data[] = {
            BDMFMON_MAKE_PARM("sbpm_wr_data", "sbpm_wr_data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_ug_bac_max[] = {
            BDMFMON_MAKE_PARM("ug0bacmax", "ug0bacmax", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ug1bacmax", "ug1bacmax", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_spare[] = {
            BDMFMON_MAKE_PARM("gl_bac_clear_en", "gl_bac_clear_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_bac_underrun[] = {
            BDMFMON_MAKE_PARM("sbpm_cfg_bac_underrun_en", "sbpm_cfg_bac_underrun_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_intr_ctrl_isr[] = {
            BDMFMON_MAKE_PARM("bac_underrun", "bac_underrun", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mcst_overflow", "mcst_overflow", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("check_last_err", "check_last_err", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("max_search_err", "max_search_err", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("invalid_in2e", "invalid_in2e", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("multi_get_next_null", "multi_get_next_null", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cnct_null", "cnct_null", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("alloc_null", "alloc_null", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("invalid_in2e_overflow", "invalid_in2e_overflow", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("invalid_in2e_underflow", "invalid_in2e_underflow", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bac_underrun_ug0", "bac_underrun_ug0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bac_underrun_ug1", "bac_underrun_ug1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_intr_ctrl_ier[] = {
            BDMFMON_MAKE_PARM("iem", "iem", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_intr_ctrl_itr[] = {
            BDMFMON_MAKE_PARM("ist", "ist", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "regs_init_free_list", .val = cli_sbpm_regs_init_free_list, .parms = set_regs_init_free_list },
            { .name = "regs_bn_alloc", .val = cli_sbpm_regs_bn_alloc, .parms = set_regs_bn_alloc },
            { .name = "regs_bn_free_with_contxt_low", .val = cli_sbpm_regs_bn_free_with_contxt_low, .parms = set_regs_bn_free_with_contxt_low },
            { .name = "regs_bn_free_with_contxt_high", .val = cli_sbpm_regs_bn_free_with_contxt_high, .parms = set_regs_bn_free_with_contxt_high },
            { .name = "regs_mcst_inc", .val = cli_sbpm_regs_mcst_inc, .parms = set_regs_mcst_inc },
            { .name = "regs_bn_connect", .val = cli_sbpm_regs_bn_connect, .parms = set_regs_bn_connect },
            { .name = "regs_get_next", .val = cli_sbpm_regs_get_next, .parms = set_regs_get_next },
            { .name = "regs_sbpm_clk_gate_cntrl", .val = cli_sbpm_regs_sbpm_clk_gate_cntrl, .parms = set_regs_sbpm_clk_gate_cntrl },
            { .name = "regs_bn_free_without_contxt", .val = cli_sbpm_regs_bn_free_without_contxt, .parms = set_regs_bn_free_without_contxt },
            { .name = "regs_sbpm_gl_trsh", .val = cli_sbpm_regs_sbpm_gl_trsh, .parms = set_regs_sbpm_gl_trsh },
            { .name = "regs_sbpm_ug0_trsh", .val = cli_sbpm_regs_sbpm_ug0_trsh, .parms = set_regs_sbpm_ug0_trsh },
            { .name = "regs_sbpm_ug1_trsh", .val = cli_sbpm_regs_sbpm_ug1_trsh, .parms = set_regs_sbpm_ug1_trsh },
            { .name = "regs_sbpm_dbg", .val = cli_sbpm_regs_sbpm_dbg, .parms = set_regs_sbpm_dbg },
            { .name = "regs_sbpm_ug0_excl_high_trsh", .val = cli_sbpm_regs_sbpm_ug0_excl_high_trsh, .parms = set_regs_sbpm_ug0_excl_high_trsh },
            { .name = "regs_sbpm_ug1_excl_high_trsh", .val = cli_sbpm_regs_sbpm_ug1_excl_high_trsh, .parms = set_regs_sbpm_ug1_excl_high_trsh },
            { .name = "regs_sbpm_ug0_excl_low_trsh", .val = cli_sbpm_regs_sbpm_ug0_excl_low_trsh, .parms = set_regs_sbpm_ug0_excl_low_trsh },
            { .name = "regs_sbpm_ug1_excl_low_trsh", .val = cli_sbpm_regs_sbpm_ug1_excl_low_trsh, .parms = set_regs_sbpm_ug1_excl_low_trsh },
            { .name = "regs_error_handling_params", .val = cli_sbpm_regs_error_handling_params, .parms = set_regs_error_handling_params },
            { .name = "regs_sbpm_sp_bbh_low", .val = cli_sbpm_regs_sbpm_sp_bbh_low, .parms = set_regs_sbpm_sp_bbh_low },
            { .name = "regs_sbpm_sp_bbh_high", .val = cli_sbpm_regs_sbpm_sp_bbh_high, .parms = set_regs_sbpm_sp_bbh_high },
            { .name = "regs_sbpm_sp_rnr_low", .val = cli_sbpm_regs_sbpm_sp_rnr_low, .parms = set_regs_sbpm_sp_rnr_low },
            { .name = "regs_sbpm_sp_rnr_high", .val = cli_sbpm_regs_sbpm_sp_rnr_high, .parms = set_regs_sbpm_sp_rnr_high },
            { .name = "regs_sbpm_ug_map_low", .val = cli_sbpm_regs_sbpm_ug_map_low, .parms = set_regs_sbpm_ug_map_low },
            { .name = "regs_sbpm_ug_map_high", .val = cli_sbpm_regs_sbpm_ug_map_high, .parms = set_regs_sbpm_ug_map_high },
            { .name = "regs_sbpm_excl_mask_low", .val = cli_sbpm_regs_sbpm_excl_mask_low, .parms = set_regs_sbpm_excl_mask_low },
            { .name = "regs_sbpm_excl_mask_high", .val = cli_sbpm_regs_sbpm_excl_mask_high, .parms = set_regs_sbpm_excl_mask_high },
            { .name = "regs_sbpm_raddr_decoder", .val = cli_sbpm_regs_sbpm_raddr_decoder, .parms = set_regs_sbpm_raddr_decoder },
            { .name = "regs_sbpm_wr_data", .val = cli_sbpm_regs_sbpm_wr_data, .parms = set_regs_sbpm_wr_data },
            { .name = "regs_sbpm_ug_bac_max", .val = cli_sbpm_regs_sbpm_ug_bac_max, .parms = set_regs_sbpm_ug_bac_max },
            { .name = "regs_sbpm_spare", .val = cli_sbpm_regs_sbpm_spare, .parms = set_regs_sbpm_spare },
            { .name = "regs_sbpm_bac_underrun", .val = cli_sbpm_regs_sbpm_bac_underrun, .parms = set_regs_sbpm_bac_underrun },
            { .name = "intr_ctrl_isr", .val = cli_sbpm_intr_ctrl_isr, .parms = set_intr_ctrl_isr },
            { .name = "intr_ctrl_ier", .val = cli_sbpm_intr_ctrl_ier, .parms = set_intr_ctrl_ier },
            { .name = "intr_ctrl_itr", .val = cli_sbpm_intr_ctrl_itr, .parms = set_intr_ctrl_itr },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_sbpm_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "nack_mask", .val = cli_sbpm_nack_mask, .parms = get_default },
            { .name = "bac", .val = cli_sbpm_bac, .parms = get_default },
            { .name = "regs_init_free_list", .val = cli_sbpm_regs_init_free_list, .parms = get_default },
            { .name = "regs_bn_alloc", .val = cli_sbpm_regs_bn_alloc, .parms = get_default },
            { .name = "regs_bn_alloc_rply", .val = cli_sbpm_regs_bn_alloc_rply, .parms = get_default },
            { .name = "regs_bn_free_with_contxt_low", .val = cli_sbpm_regs_bn_free_with_contxt_low, .parms = get_default },
            { .name = "regs_bn_free_with_contxt_high", .val = cli_sbpm_regs_bn_free_with_contxt_high, .parms = get_default },
            { .name = "regs_mcst_inc", .val = cli_sbpm_regs_mcst_inc, .parms = get_default },
            { .name = "regs_mcst_inc_rply", .val = cli_sbpm_regs_mcst_inc_rply, .parms = get_default },
            { .name = "regs_bn_connect", .val = cli_sbpm_regs_bn_connect, .parms = get_default },
            { .name = "regs_bn_connect_rply", .val = cli_sbpm_regs_bn_connect_rply, .parms = get_default },
            { .name = "regs_get_next", .val = cli_sbpm_regs_get_next, .parms = get_default },
            { .name = "regs_get_next_rply", .val = cli_sbpm_regs_get_next_rply, .parms = get_default },
            { .name = "regs_sbpm_clk_gate_cntrl", .val = cli_sbpm_regs_sbpm_clk_gate_cntrl, .parms = get_default },
            { .name = "regs_bn_free_without_contxt", .val = cli_sbpm_regs_bn_free_without_contxt, .parms = get_default },
            { .name = "regs_bn_free_without_contxt_rply", .val = cli_sbpm_regs_bn_free_without_contxt_rply, .parms = get_default },
            { .name = "regs_bn_free_with_contxt_rply", .val = cli_sbpm_regs_bn_free_with_contxt_rply, .parms = get_default },
            { .name = "regs_sbpm_gl_trsh", .val = cli_sbpm_regs_sbpm_gl_trsh, .parms = get_default },
            { .name = "regs_sbpm_ug0_trsh", .val = cli_sbpm_regs_sbpm_ug0_trsh, .parms = get_default },
            { .name = "regs_sbpm_ug1_trsh", .val = cli_sbpm_regs_sbpm_ug1_trsh, .parms = get_default },
            { .name = "regs_sbpm_dbg", .val = cli_sbpm_regs_sbpm_dbg, .parms = get_default },
            { .name = "regs_sbpm_ug0_excl_high_trsh", .val = cli_sbpm_regs_sbpm_ug0_excl_high_trsh, .parms = get_default },
            { .name = "regs_sbpm_ug1_excl_high_trsh", .val = cli_sbpm_regs_sbpm_ug1_excl_high_trsh, .parms = get_default },
            { .name = "regs_sbpm_ug0_excl_low_trsh", .val = cli_sbpm_regs_sbpm_ug0_excl_low_trsh, .parms = get_default },
            { .name = "regs_sbpm_ug1_excl_low_trsh", .val = cli_sbpm_regs_sbpm_ug1_excl_low_trsh, .parms = get_default },
            { .name = "regs_sbpm_ug_status", .val = cli_sbpm_regs_sbpm_ug_status, .parms = get_default },
            { .name = "regs_error_handling_params", .val = cli_sbpm_regs_error_handling_params, .parms = get_default },
            { .name = "regs_sbpm_iir_addr", .val = cli_sbpm_regs_sbpm_iir_addr, .parms = get_default },
            { .name = "regs_sbpm_iir_low", .val = cli_sbpm_regs_sbpm_iir_low, .parms = get_default },
            { .name = "regs_sbpm_iir_high", .val = cli_sbpm_regs_sbpm_iir_high, .parms = get_default },
            { .name = "regs_sbpm_dbg_vec0", .val = cli_sbpm_regs_sbpm_dbg_vec0, .parms = get_default },
            { .name = "regs_sbpm_dbg_vec1", .val = cli_sbpm_regs_sbpm_dbg_vec1, .parms = get_default },
            { .name = "regs_sbpm_dbg_vec2", .val = cli_sbpm_regs_sbpm_dbg_vec2, .parms = get_default },
            { .name = "regs_sbpm_dbg_vec3", .val = cli_sbpm_regs_sbpm_dbg_vec3, .parms = get_default },
            { .name = "regs_sbpm_sp_bbh_low", .val = cli_sbpm_regs_sbpm_sp_bbh_low, .parms = get_default },
            { .name = "regs_sbpm_sp_bbh_high", .val = cli_sbpm_regs_sbpm_sp_bbh_high, .parms = get_default },
            { .name = "regs_sbpm_sp_rnr_low", .val = cli_sbpm_regs_sbpm_sp_rnr_low, .parms = get_default },
            { .name = "regs_sbpm_sp_rnr_high", .val = cli_sbpm_regs_sbpm_sp_rnr_high, .parms = get_default },
            { .name = "regs_sbpm_ug_map_low", .val = cli_sbpm_regs_sbpm_ug_map_low, .parms = get_default },
            { .name = "regs_sbpm_ug_map_high", .val = cli_sbpm_regs_sbpm_ug_map_high, .parms = get_default },
            { .name = "regs_sbpm_excl_mask_low", .val = cli_sbpm_regs_sbpm_excl_mask_low, .parms = get_default },
            { .name = "regs_sbpm_excl_mask_high", .val = cli_sbpm_regs_sbpm_excl_mask_high, .parms = get_default },
            { .name = "regs_sbpm_raddr_decoder", .val = cli_sbpm_regs_sbpm_raddr_decoder, .parms = get_default },
            { .name = "regs_sbpm_wr_data", .val = cli_sbpm_regs_sbpm_wr_data, .parms = get_default },
            { .name = "regs_sbpm_ug_bac_max", .val = cli_sbpm_regs_sbpm_ug_bac_max, .parms = get_default },
            { .name = "regs_sbpm_spare", .val = cli_sbpm_regs_sbpm_spare, .parms = get_default },
            { .name = "regs_sbpm_bac_underrun", .val = cli_sbpm_regs_sbpm_bac_underrun, .parms = get_default },
            { .name = "intr_ctrl_isr", .val = cli_sbpm_intr_ctrl_isr, .parms = get_default },
            { .name = "intr_ctrl_ism", .val = cli_sbpm_intr_ctrl_ism, .parms = get_default },
            { .name = "intr_ctrl_ier", .val = cli_sbpm_intr_ctrl_ier, .parms = get_default },
            { .name = "intr_ctrl_itr", .val = cli_sbpm_intr_ctrl_itr, .parms = get_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_sbpm_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            { .name = "high", .val = ag_drv_cli_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_sbpm_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "REGS_INIT_FREE_LIST", .val = bdmf_address_regs_init_free_list },
            { .name = "REGS_BN_ALLOC", .val = bdmf_address_regs_bn_alloc },
            { .name = "REGS_BN_ALLOC_RPLY", .val = bdmf_address_regs_bn_alloc_rply },
            { .name = "REGS_BN_FREE_WITH_CONTXT_LOW", .val = bdmf_address_regs_bn_free_with_contxt_low },
            { .name = "REGS_BN_FREE_WITH_CONTXT_HIGH", .val = bdmf_address_regs_bn_free_with_contxt_high },
            { .name = "REGS_MCST_INC", .val = bdmf_address_regs_mcst_inc },
            { .name = "REGS_MCST_INC_RPLY", .val = bdmf_address_regs_mcst_inc_rply },
            { .name = "REGS_BN_CONNECT", .val = bdmf_address_regs_bn_connect },
            { .name = "REGS_BN_CONNECT_RPLY", .val = bdmf_address_regs_bn_connect_rply },
            { .name = "REGS_GET_NEXT", .val = bdmf_address_regs_get_next },
            { .name = "REGS_GET_NEXT_RPLY", .val = bdmf_address_regs_get_next_rply },
            { .name = "REGS_SBPM_CLK_GATE_CNTRL", .val = bdmf_address_regs_sbpm_clk_gate_cntrl },
            { .name = "REGS_BN_FREE_WITHOUT_CONTXT", .val = bdmf_address_regs_bn_free_without_contxt },
            { .name = "REGS_BN_FREE_WITHOUT_CONTXT_RPLY", .val = bdmf_address_regs_bn_free_without_contxt_rply },
            { .name = "REGS_BN_FREE_WITH_CONTXT_RPLY", .val = bdmf_address_regs_bn_free_with_contxt_rply },
            { .name = "REGS_SBPM_GL_TRSH", .val = bdmf_address_regs_sbpm_gl_trsh },
            { .name = "REGS_SBPM_UG0_TRSH", .val = bdmf_address_regs_sbpm_ug0_trsh },
            { .name = "REGS_SBPM_UG1_TRSH", .val = bdmf_address_regs_sbpm_ug1_trsh },
            { .name = "REGS_SBPM_DBG", .val = bdmf_address_regs_sbpm_dbg },
            { .name = "REGS_SBPM_UG0_BAC", .val = bdmf_address_regs_sbpm_ug0_bac },
            { .name = "REGS_SBPM_UG1_BAC", .val = bdmf_address_regs_sbpm_ug1_bac },
            { .name = "REGS_SBPM_GL_BAC", .val = bdmf_address_regs_sbpm_gl_bac },
            { .name = "REGS_SBPM_UG0_EXCL_HIGH_TRSH", .val = bdmf_address_regs_sbpm_ug0_excl_high_trsh },
            { .name = "REGS_SBPM_UG1_EXCL_HIGH_TRSH", .val = bdmf_address_regs_sbpm_ug1_excl_high_trsh },
            { .name = "REGS_SBPM_UG0_EXCL_LOW_TRSH", .val = bdmf_address_regs_sbpm_ug0_excl_low_trsh },
            { .name = "REGS_SBPM_UG1_EXCL_LOW_TRSH", .val = bdmf_address_regs_sbpm_ug1_excl_low_trsh },
            { .name = "REGS_SBPM_UG_STATUS", .val = bdmf_address_regs_sbpm_ug_status },
            { .name = "REGS_ERROR_HANDLING_PARAMS", .val = bdmf_address_regs_error_handling_params },
            { .name = "REGS_SBPM_IIR_ADDR", .val = bdmf_address_regs_sbpm_iir_addr },
            { .name = "REGS_SBPM_IIR_LOW", .val = bdmf_address_regs_sbpm_iir_low },
            { .name = "REGS_SBPM_IIR_HIGH", .val = bdmf_address_regs_sbpm_iir_high },
            { .name = "REGS_SBPM_DBG_VEC0", .val = bdmf_address_regs_sbpm_dbg_vec0 },
            { .name = "REGS_SBPM_DBG_VEC1", .val = bdmf_address_regs_sbpm_dbg_vec1 },
            { .name = "REGS_SBPM_DBG_VEC2", .val = bdmf_address_regs_sbpm_dbg_vec2 },
            { .name = "REGS_SBPM_DBG_VEC3", .val = bdmf_address_regs_sbpm_dbg_vec3 },
            { .name = "REGS_SBPM_SP_BBH_LOW", .val = bdmf_address_regs_sbpm_sp_bbh_low },
            { .name = "REGS_SBPM_SP_BBH_HIGH", .val = bdmf_address_regs_sbpm_sp_bbh_high },
            { .name = "REGS_SBPM_SP_RNR_LOW", .val = bdmf_address_regs_sbpm_sp_rnr_low },
            { .name = "REGS_SBPM_SP_RNR_HIGH", .val = bdmf_address_regs_sbpm_sp_rnr_high },
            { .name = "REGS_SBPM_UG_MAP_LOW", .val = bdmf_address_regs_sbpm_ug_map_low },
            { .name = "REGS_SBPM_UG_MAP_HIGH", .val = bdmf_address_regs_sbpm_ug_map_high },
            { .name = "REGS_SBPM_NACK_MASK_LOW", .val = bdmf_address_regs_sbpm_nack_mask_low },
            { .name = "REGS_SBPM_NACK_MASK_HIGH", .val = bdmf_address_regs_sbpm_nack_mask_high },
            { .name = "REGS_SBPM_EXCL_MASK_LOW", .val = bdmf_address_regs_sbpm_excl_mask_low },
            { .name = "REGS_SBPM_EXCL_MASK_HIGH", .val = bdmf_address_regs_sbpm_excl_mask_high },
            { .name = "REGS_SBPM_RADDR_DECODER", .val = bdmf_address_regs_sbpm_raddr_decoder },
            { .name = "REGS_SBPM_WR_DATA", .val = bdmf_address_regs_sbpm_wr_data },
            { .name = "REGS_SBPM_UG_BAC_MAX", .val = bdmf_address_regs_sbpm_ug_bac_max },
            { .name = "REGS_SBPM_SPARE", .val = bdmf_address_regs_sbpm_spare },
            { .name = "REGS_SBPM_BAC_UNDERRUN", .val = bdmf_address_regs_sbpm_bac_underrun },
            { .name = "INTR_CTRL_ISR", .val = bdmf_address_intr_ctrl_isr },
            { .name = "INTR_CTRL_ISM", .val = bdmf_address_intr_ctrl_ism },
            { .name = "INTR_CTRL_IER", .val = bdmf_address_intr_ctrl_ier },
            { .name = "INTR_CTRL_ITR", .val = bdmf_address_intr_ctrl_itr },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_sbpm_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
