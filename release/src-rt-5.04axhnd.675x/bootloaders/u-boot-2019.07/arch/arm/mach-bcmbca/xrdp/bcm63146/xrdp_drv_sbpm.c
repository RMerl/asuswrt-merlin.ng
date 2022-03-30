// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    
*/

#include "rdp_common.h"
#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_sbpm.h"

bdmf_error_t ag_drv_sbpm_regs_bn_alloc_set(uint8_t sa)
{
    uint32_t reg_regs_bn_alloc=0;

#ifdef VALIDATE_PARMS
    if((sa >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_bn_alloc = RU_FIELD_SET(0, SBPM, REGS_BN_ALLOC, SA, reg_regs_bn_alloc, sa);

    RU_REG_WRITE(0, SBPM, REGS_BN_ALLOC, reg_regs_bn_alloc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_bn_alloc_rply_get(sbpm_regs_bn_alloc_rply *regs_bn_alloc_rply)
{
    uint32_t reg_regs_bn_alloc_rply;

#ifdef VALIDATE_PARMS
    if(!regs_bn_alloc_rply)
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

bdmf_error_t ag_drv_sbpm_regs_bn_connect_set(uint16_t bn, bdmf_boolean ack_req, bdmf_boolean wr_req, uint16_t pointed_bn)
{
    uint32_t reg_regs_bn_connect=0;

#ifdef VALIDATE_PARMS
    if((bn >= _14BITS_MAX_VAL_) ||
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

bdmf_error_t ag_drv_sbpm_regs_bn_connect_rply_get(bdmf_boolean *connect_ack, bdmf_boolean *busy, bdmf_boolean *rdy)
{
    uint32_t reg_regs_bn_connect_rply;

#ifdef VALIDATE_PARMS
    if(!connect_ack || !busy || !rdy)
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
    uint32_t reg_regs_get_next=0;

#ifdef VALIDATE_PARMS
    if((bn >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_get_next = RU_FIELD_SET(0, SBPM, REGS_GET_NEXT, BN, reg_regs_get_next, bn);

    RU_REG_WRITE(0, SBPM, REGS_GET_NEXT, reg_regs_get_next);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_sbpm_regs_get_next_rply_get(sbpm_regs_get_next_rply *regs_get_next_rply)
{
    uint32_t reg_regs_get_next_rply;

#ifdef VALIDATE_PARMS
    if(!regs_get_next_rply)
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

bdmf_error_t ag_drv_sbpm_regs_bn_free_without_contxt_set(uint16_t head_bn, uint8_t sa, bdmf_boolean ack_req)
{
    uint32_t reg_regs_bn_free_without_contxt=0;

#ifdef VALIDATE_PARMS
    if((head_bn >= _14BITS_MAX_VAL_) ||
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

bdmf_error_t ag_drv_sbpm_regs_bn_free_without_contxt_rply_get(sbpm_regs_bn_free_without_contxt_rply *regs_bn_free_without_contxt_rply)
{
    uint32_t reg_regs_bn_free_without_contxt_rply;

#ifdef VALIDATE_PARMS
    if(!regs_bn_free_without_contxt_rply)
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

