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
#include "xrdp_drv_crdp_ds_cmn_sbpm_ag.h"

#define BLOCK_ADDR_COUNT_BITS 0
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_init_free_list_set(uint8_t sbpm_id, uint16_t init_max_bn, bdmf_boolean ram_init_start, bdmf_boolean bsy, bdmf_boolean rdy)
{
    uint32_t reg_regs_init_free_list = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_id >= BLOCK_ADDR_COUNT) ||
       (init_max_bn >= _14BITS_MAX_VAL_) ||
       (ram_init_start >= _1BITS_MAX_VAL_) ||
       (bsy >= _1BITS_MAX_VAL_) ||
       (rdy >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_init_free_list = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_INIT_FREE_LIST, INIT_MAX_BN, reg_regs_init_free_list, init_max_bn);
    reg_regs_init_free_list = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_INIT_FREE_LIST, RAM_INIT_START, reg_regs_init_free_list, ram_init_start);
    reg_regs_init_free_list = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_INIT_FREE_LIST, BSY, reg_regs_init_free_list, bsy);
    reg_regs_init_free_list = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_INIT_FREE_LIST, RDY, reg_regs_init_free_list, rdy);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, REGS_INIT_FREE_LIST, reg_regs_init_free_list);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_init_free_list_get(uint8_t sbpm_id, uint16_t *init_max_bn, bdmf_boolean *ram_init_start, bdmf_boolean *bsy, bdmf_boolean *rdy)
{
    uint32_t reg_regs_init_free_list;

#ifdef VALIDATE_PARMS
    if (!init_max_bn || !ram_init_start || !bsy || !rdy)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_INIT_FREE_LIST, reg_regs_init_free_list);

    *init_max_bn = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_INIT_FREE_LIST, INIT_MAX_BN, reg_regs_init_free_list);
    *ram_init_start = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_INIT_FREE_LIST, RAM_INIT_START, reg_regs_init_free_list);
    *bsy = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_INIT_FREE_LIST, BSY, reg_regs_init_free_list);
    *rdy = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_INIT_FREE_LIST, RDY, reg_regs_init_free_list);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bn_alloc_set(uint8_t sbpm_id, bdmf_boolean req)
{
    uint32_t reg_regs_bn_alloc = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_id >= BLOCK_ADDR_COUNT) ||
       (req >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_bn_alloc = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_ALLOC, REQ, reg_regs_bn_alloc, req);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_ALLOC, reg_regs_bn_alloc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bn_alloc_get(uint8_t sbpm_id, bdmf_boolean *req)
{
    uint32_t reg_regs_bn_alloc;

#ifdef VALIDATE_PARMS
    if (!req)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_ALLOC, reg_regs_bn_alloc);

    *req = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_ALLOC, REQ, reg_regs_bn_alloc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bn_alloc_rply_get(uint8_t sbpm_id, crdp_ds_cmn_sbpm_regs_bn_alloc_rply *regs_bn_alloc_rply)
{
    uint32_t reg_regs_bn_alloc_rply;

#ifdef VALIDATE_PARMS
    if (!regs_bn_alloc_rply)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_ALLOC_RPLY, reg_regs_bn_alloc_rply);

    regs_bn_alloc_rply->alloc_bn_valid = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_ALLOC_RPLY, ALLOC_BN_VALID, reg_regs_bn_alloc_rply);
    regs_bn_alloc_rply->alloc_bn = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_ALLOC_RPLY, ALLOC_BN, reg_regs_bn_alloc_rply);
    regs_bn_alloc_rply->ack = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_ALLOC_RPLY, ACK, reg_regs_bn_alloc_rply);
    regs_bn_alloc_rply->nack = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_ALLOC_RPLY, NACK, reg_regs_bn_alloc_rply);
    regs_bn_alloc_rply->excl_high = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_ALLOC_RPLY, EXCL_HIGH, reg_regs_bn_alloc_rply);
    regs_bn_alloc_rply->excl_med = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_ALLOC_RPLY, EXCL_MED, reg_regs_bn_alloc_rply);
    regs_bn_alloc_rply->excl_low = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_ALLOC_RPLY, EXCL_LOW, reg_regs_bn_alloc_rply);
    regs_bn_alloc_rply->busy = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_ALLOC_RPLY, BUSY, reg_regs_bn_alloc_rply);
    regs_bn_alloc_rply->rdy = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_ALLOC_RPLY, RDY, reg_regs_bn_alloc_rply);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_mcst_inc_set(uint8_t sbpm_id, uint16_t bn, uint8_t mcst_val, bdmf_boolean ack_req)
{
    uint32_t reg_regs_mcst_inc = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_id >= BLOCK_ADDR_COUNT) ||
       (bn >= _14BITS_MAX_VAL_) ||
       (mcst_val >= _3BITS_MAX_VAL_) ||
       (ack_req >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_mcst_inc = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_MCST_INC, BN, reg_regs_mcst_inc, bn);
    reg_regs_mcst_inc = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_MCST_INC, MCST_VAL, reg_regs_mcst_inc, mcst_val);
    reg_regs_mcst_inc = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_MCST_INC, ACK_REQ, reg_regs_mcst_inc, ack_req);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, REGS_MCST_INC, reg_regs_mcst_inc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_mcst_inc_get(uint8_t sbpm_id, uint16_t *bn, uint8_t *mcst_val, bdmf_boolean *ack_req)
{
    uint32_t reg_regs_mcst_inc;

#ifdef VALIDATE_PARMS
    if (!bn || !mcst_val || !ack_req)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_MCST_INC, reg_regs_mcst_inc);

    *bn = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_MCST_INC, BN, reg_regs_mcst_inc);
    *mcst_val = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_MCST_INC, MCST_VAL, reg_regs_mcst_inc);
    *ack_req = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_MCST_INC, ACK_REQ, reg_regs_mcst_inc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_mcst_inc_rply_get(uint8_t sbpm_id, bdmf_boolean *mcst_ack, uint8_t *mc_res, bdmf_boolean *bsy, bdmf_boolean *rdy)
{
    uint32_t reg_regs_mcst_inc_rply;

#ifdef VALIDATE_PARMS
    if (!mcst_ack || !mc_res || !bsy || !rdy)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_MCST_INC_RPLY, reg_regs_mcst_inc_rply);

    *mcst_ack = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_MCST_INC_RPLY, MCST_ACK, reg_regs_mcst_inc_rply);
    *mc_res = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_MCST_INC_RPLY, MC_RES, reg_regs_mcst_inc_rply);
    *bsy = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_MCST_INC_RPLY, BSY, reg_regs_mcst_inc_rply);
    *rdy = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_MCST_INC_RPLY, RDY, reg_regs_mcst_inc_rply);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bn_connect_set(uint8_t sbpm_id, uint16_t bn, bdmf_boolean ack_req, uint16_t pointed_bn)
{
    uint32_t reg_regs_bn_connect = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_id >= BLOCK_ADDR_COUNT) ||
       (bn >= _14BITS_MAX_VAL_) ||
       (ack_req >= _1BITS_MAX_VAL_) ||
       (pointed_bn >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_bn_connect = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_CONNECT, BN, reg_regs_bn_connect, bn);
    reg_regs_bn_connect = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_CONNECT, ACK_REQ, reg_regs_bn_connect, ack_req);
    reg_regs_bn_connect = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_CONNECT, POINTED_BN, reg_regs_bn_connect, pointed_bn);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_CONNECT, reg_regs_bn_connect);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bn_connect_get(uint8_t sbpm_id, uint16_t *bn, bdmf_boolean *ack_req, uint16_t *pointed_bn)
{
    uint32_t reg_regs_bn_connect;

#ifdef VALIDATE_PARMS
    if (!bn || !ack_req || !pointed_bn)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_CONNECT, reg_regs_bn_connect);

    *bn = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_CONNECT, BN, reg_regs_bn_connect);
    *ack_req = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_CONNECT, ACK_REQ, reg_regs_bn_connect);
    *pointed_bn = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_CONNECT, POINTED_BN, reg_regs_bn_connect);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bn_connect_rply_get(uint8_t sbpm_id, bdmf_boolean *connect_ack, bdmf_boolean *busy, bdmf_boolean *rdy)
{
    uint32_t reg_regs_bn_connect_rply;

#ifdef VALIDATE_PARMS
    if (!connect_ack || !busy || !rdy)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_CONNECT_RPLY, reg_regs_bn_connect_rply);

    *connect_ack = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_CONNECT_RPLY, CONNECT_ACK, reg_regs_bn_connect_rply);
    *busy = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_CONNECT_RPLY, BUSY, reg_regs_bn_connect_rply);
    *rdy = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_CONNECT_RPLY, RDY, reg_regs_bn_connect_rply);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_get_next_set(uint8_t sbpm_id, uint16_t bn)
{
    uint32_t reg_regs_get_next = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_id >= BLOCK_ADDR_COUNT) ||
       (bn >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_get_next = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GET_NEXT, BN, reg_regs_get_next, bn);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GET_NEXT, reg_regs_get_next);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_get_next_get(uint8_t sbpm_id, uint16_t *bn)
{
    uint32_t reg_regs_get_next;

#ifdef VALIDATE_PARMS
    if (!bn)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GET_NEXT, reg_regs_get_next);

    *bn = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GET_NEXT, BN, reg_regs_get_next);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_get_next_rply_get(uint8_t sbpm_id, bdmf_boolean *bn_valid, uint16_t *next_bn, bdmf_boolean *busy, bdmf_boolean *rdy)
{
    uint32_t reg_regs_get_next_rply;

#ifdef VALIDATE_PARMS
    if (!bn_valid || !next_bn || !busy || !rdy)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GET_NEXT_RPLY, reg_regs_get_next_rply);

    *bn_valid = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GET_NEXT_RPLY, BN_VALID, reg_regs_get_next_rply);
    *next_bn = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GET_NEXT_RPLY, NEXT_BN, reg_regs_get_next_rply);
    *busy = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GET_NEXT_RPLY, BUSY, reg_regs_get_next_rply);
    *rdy = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GET_NEXT_RPLY, RDY, reg_regs_get_next_rply);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_clk_gate_cntrl_set(uint8_t sbpm_id, const crdp_ds_cmn_sbpm_regs_sbpm_clk_gate_cntrl *regs_sbpm_clk_gate_cntrl)
{
    uint32_t reg_regs_sbpm_clk_gate_cntrl = 0;

#ifdef VALIDATE_PARMS
    if(!regs_sbpm_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT) ||
       (regs_sbpm_clk_gate_cntrl->bypass_clk_gate >= _1BITS_MAX_VAL_) ||
       (regs_sbpm_clk_gate_cntrl->keep_alive_en >= _1BITS_MAX_VAL_) ||
       (regs_sbpm_clk_gate_cntrl->keep_alive_intervl >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_sbpm_clk_gate_cntrl = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_regs_sbpm_clk_gate_cntrl, regs_sbpm_clk_gate_cntrl->bypass_clk_gate);
    reg_regs_sbpm_clk_gate_cntrl = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_CLK_GATE_CNTRL, TIMER_VAL, reg_regs_sbpm_clk_gate_cntrl, regs_sbpm_clk_gate_cntrl->timer_val);
    reg_regs_sbpm_clk_gate_cntrl = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_regs_sbpm_clk_gate_cntrl, regs_sbpm_clk_gate_cntrl->keep_alive_en);
    reg_regs_sbpm_clk_gate_cntrl = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_CLK_GATE_CNTRL, KEEP_ALIVE_INTERVL, reg_regs_sbpm_clk_gate_cntrl, regs_sbpm_clk_gate_cntrl->keep_alive_intervl);
    reg_regs_sbpm_clk_gate_cntrl = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_regs_sbpm_clk_gate_cntrl, regs_sbpm_clk_gate_cntrl->keep_alive_cyc);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_CLK_GATE_CNTRL, reg_regs_sbpm_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_clk_gate_cntrl_get(uint8_t sbpm_id, crdp_ds_cmn_sbpm_regs_sbpm_clk_gate_cntrl *regs_sbpm_clk_gate_cntrl)
{
    uint32_t reg_regs_sbpm_clk_gate_cntrl;

#ifdef VALIDATE_PARMS
    if (!regs_sbpm_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_CLK_GATE_CNTRL, reg_regs_sbpm_clk_gate_cntrl);

    regs_sbpm_clk_gate_cntrl->bypass_clk_gate = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_regs_sbpm_clk_gate_cntrl);
    regs_sbpm_clk_gate_cntrl->timer_val = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_CLK_GATE_CNTRL, TIMER_VAL, reg_regs_sbpm_clk_gate_cntrl);
    regs_sbpm_clk_gate_cntrl->keep_alive_en = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_regs_sbpm_clk_gate_cntrl);
    regs_sbpm_clk_gate_cntrl->keep_alive_intervl = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_CLK_GATE_CNTRL, KEEP_ALIVE_INTERVL, reg_regs_sbpm_clk_gate_cntrl);
    regs_sbpm_clk_gate_cntrl->keep_alive_cyc = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_regs_sbpm_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg1_set(uint8_t sbpm_id, uint16_t plen, uint16_t sop_offset, bdmf_boolean calc)
{
    uint32_t reg_regs_bn_free_without_contxt_reg1 = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_id >= BLOCK_ADDR_COUNT) ||
       (plen >= _14BITS_MAX_VAL_) ||
       (sop_offset >= _10BITS_MAX_VAL_) ||
       (calc >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_bn_free_without_contxt_reg1 = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_REG1, PLEN, reg_regs_bn_free_without_contxt_reg1, plen);
    reg_regs_bn_free_without_contxt_reg1 = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_REG1, SOP_OFFSET, reg_regs_bn_free_without_contxt_reg1, sop_offset);
    reg_regs_bn_free_without_contxt_reg1 = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_REG1, CALC, reg_regs_bn_free_without_contxt_reg1, calc);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_REG1, reg_regs_bn_free_without_contxt_reg1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg1_get(uint8_t sbpm_id, uint16_t *plen, uint16_t *sop_offset, bdmf_boolean *calc)
{
    uint32_t reg_regs_bn_free_without_contxt_reg1;

#ifdef VALIDATE_PARMS
    if (!plen || !sop_offset || !calc)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_REG1, reg_regs_bn_free_without_contxt_reg1);

    *plen = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_REG1, PLEN, reg_regs_bn_free_without_contxt_reg1);
    *sop_offset = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_REG1, SOP_OFFSET, reg_regs_bn_free_without_contxt_reg1);
    *calc = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_REG1, CALC, reg_regs_bn_free_without_contxt_reg1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg2_set(uint8_t sbpm_id, uint16_t first_bn, uint8_t num_packet)
{
    uint32_t reg_regs_bn_free_without_contxt_reg2 = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_id >= BLOCK_ADDR_COUNT) ||
       (first_bn >= _14BITS_MAX_VAL_) ||
       (num_packet >= _7BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_bn_free_without_contxt_reg2 = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_REG2, FIRST_BN, reg_regs_bn_free_without_contxt_reg2, first_bn);
    reg_regs_bn_free_without_contxt_reg2 = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_REG2, NUM_PACKET, reg_regs_bn_free_without_contxt_reg2, num_packet);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_REG2, reg_regs_bn_free_without_contxt_reg2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg2_get(uint8_t sbpm_id, uint16_t *first_bn, uint8_t *num_packet)
{
    uint32_t reg_regs_bn_free_without_contxt_reg2;

#ifdef VALIDATE_PARMS
    if (!first_bn || !num_packet)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_REG2, reg_regs_bn_free_without_contxt_reg2);

    *first_bn = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_REG2, FIRST_BN, reg_regs_bn_free_without_contxt_reg2);
    *num_packet = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_REG2, NUM_PACKET, reg_regs_bn_free_without_contxt_reg2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_rply_get(uint8_t sbpm_id, crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_rply *regs_bn_free_without_contxt_rply)
{
    uint32_t reg_regs_bn_free_without_contxt_rply;

#ifdef VALIDATE_PARMS
    if (!regs_bn_free_without_contxt_rply)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_RPLY, reg_regs_bn_free_without_contxt_rply);

    regs_bn_free_without_contxt_rply->free_ack = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_RPLY, FREE_ACK, reg_regs_bn_free_without_contxt_rply);
    regs_bn_free_without_contxt_rply->ack_stat = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_RPLY, ACK_STAT, reg_regs_bn_free_without_contxt_rply);
    regs_bn_free_without_contxt_rply->nack_stat = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_RPLY, NACK_STAT, reg_regs_bn_free_without_contxt_rply);
    regs_bn_free_without_contxt_rply->excl_high_stat = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_RPLY, EXCL_HIGH_STAT, reg_regs_bn_free_without_contxt_rply);
    regs_bn_free_without_contxt_rply->excl_med_stat = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_RPLY, EXCL_MED_STAT, reg_regs_bn_free_without_contxt_rply);
    regs_bn_free_without_contxt_rply->excl_low_stat = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_RPLY, EXCL_LOW_STAT, reg_regs_bn_free_without_contxt_rply);
    regs_bn_free_without_contxt_rply->bsy = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_RPLY, BSY, reg_regs_bn_free_without_contxt_rply);
    regs_bn_free_without_contxt_rply->rdy = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_RPLY, RDY, reg_regs_bn_free_without_contxt_rply);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_gl_trsh_set(uint8_t sbpm_id, uint16_t gl_bat, uint16_t gl_bah)
{
    uint32_t reg_regs_gl_trsh = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_id >= BLOCK_ADDR_COUNT) ||
       (gl_bat >= _15BITS_MAX_VAL_) ||
       (gl_bah >= _15BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_gl_trsh = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_TRSH, GL_BAT, reg_regs_gl_trsh, gl_bat);
    reg_regs_gl_trsh = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_TRSH, GL_BAH, reg_regs_gl_trsh, gl_bah);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_TRSH, reg_regs_gl_trsh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_gl_trsh_get(uint8_t sbpm_id, uint16_t *gl_bat, uint16_t *gl_bah)
{
    uint32_t reg_regs_gl_trsh;

#ifdef VALIDATE_PARMS
    if (!gl_bat || !gl_bah)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_TRSH, reg_regs_gl_trsh);

    *gl_bat = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_TRSH, GL_BAT, reg_regs_gl_trsh);
    *gl_bah = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_TRSH, GL_BAH, reg_regs_gl_trsh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_dbg_set(uint8_t sbpm_id, uint8_t select_bus, uint16_t window_start)
{
    uint32_t reg_regs_sbpm_dbg = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_id >= BLOCK_ADDR_COUNT) ||
       (select_bus >= _4BITS_MAX_VAL_) ||
       (window_start >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_sbpm_dbg = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_DBG, SELECT_BUS, reg_regs_sbpm_dbg, select_bus);
    reg_regs_sbpm_dbg = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_DBG, WINDOW_START, reg_regs_sbpm_dbg, window_start);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_DBG, reg_regs_sbpm_dbg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_dbg_get(uint8_t sbpm_id, uint8_t *select_bus, uint16_t *window_start)
{
    uint32_t reg_regs_sbpm_dbg;

#ifdef VALIDATE_PARMS
    if (!select_bus || !window_start)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_DBG, reg_regs_sbpm_dbg);

    *select_bus = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_DBG, SELECT_BUS, reg_regs_sbpm_dbg);
    *window_start = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_DBG, WINDOW_START, reg_regs_sbpm_dbg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_gl_bac_get(uint8_t sbpm_id, uint16_t *val)
{
    uint32_t reg_regs_gl_bac;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_BAC, reg_regs_gl_bac);

    *val = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_BAC, VAL, reg_regs_gl_bac);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_high_trsh_set(uint8_t sbpm_id, uint16_t exclt, uint16_t exclh)
{
    uint32_t reg_regs_gl_excl_high_trsh = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_id >= BLOCK_ADDR_COUNT) ||
       (exclt >= _15BITS_MAX_VAL_) ||
       (exclh >= _15BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_gl_excl_high_trsh = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_EXCL_HIGH_TRSH, EXCLT, reg_regs_gl_excl_high_trsh, exclt);
    reg_regs_gl_excl_high_trsh = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_EXCL_HIGH_TRSH, EXCLH, reg_regs_gl_excl_high_trsh, exclh);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_EXCL_HIGH_TRSH, reg_regs_gl_excl_high_trsh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_high_trsh_get(uint8_t sbpm_id, uint16_t *exclt, uint16_t *exclh)
{
    uint32_t reg_regs_gl_excl_high_trsh;

#ifdef VALIDATE_PARMS
    if (!exclt || !exclh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_EXCL_HIGH_TRSH, reg_regs_gl_excl_high_trsh);

    *exclt = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_EXCL_HIGH_TRSH, EXCLT, reg_regs_gl_excl_high_trsh);
    *exclh = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_EXCL_HIGH_TRSH, EXCLH, reg_regs_gl_excl_high_trsh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_low_trsh_set(uint8_t sbpm_id, uint16_t exclt, uint16_t exclh)
{
    uint32_t reg_regs_gl_excl_low_trsh = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_id >= BLOCK_ADDR_COUNT) ||
       (exclt >= _15BITS_MAX_VAL_) ||
       (exclh >= _15BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_gl_excl_low_trsh = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_EXCL_LOW_TRSH, EXCLT, reg_regs_gl_excl_low_trsh, exclt);
    reg_regs_gl_excl_low_trsh = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_EXCL_LOW_TRSH, EXCLH, reg_regs_gl_excl_low_trsh, exclh);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_EXCL_LOW_TRSH, reg_regs_gl_excl_low_trsh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_low_trsh_get(uint8_t sbpm_id, uint16_t *exclt, uint16_t *exclh)
{
    uint32_t reg_regs_gl_excl_low_trsh;

#ifdef VALIDATE_PARMS
    if (!exclt || !exclh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_EXCL_LOW_TRSH, reg_regs_gl_excl_low_trsh);

    *exclt = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_EXCL_LOW_TRSH, EXCLT, reg_regs_gl_excl_low_trsh);
    *exclh = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_EXCL_LOW_TRSH, EXCLH, reg_regs_gl_excl_low_trsh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_med_trsh_set(uint8_t sbpm_id, uint16_t exclt, uint16_t exclh)
{
    uint32_t reg_regs_gl_excl_med_trsh = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_id >= BLOCK_ADDR_COUNT) ||
       (exclt >= _15BITS_MAX_VAL_) ||
       (exclh >= _15BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_gl_excl_med_trsh = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_EXCL_MED_TRSH, EXCLT, reg_regs_gl_excl_med_trsh, exclt);
    reg_regs_gl_excl_med_trsh = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_EXCL_MED_TRSH, EXCLH, reg_regs_gl_excl_med_trsh, exclh);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_EXCL_MED_TRSH, reg_regs_gl_excl_med_trsh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_med_trsh_get(uint8_t sbpm_id, uint16_t *exclt, uint16_t *exclh)
{
    uint32_t reg_regs_gl_excl_med_trsh;

#ifdef VALIDATE_PARMS
    if (!exclt || !exclh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_EXCL_MED_TRSH, reg_regs_gl_excl_med_trsh);

    *exclt = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_EXCL_MED_TRSH, EXCLT, reg_regs_gl_excl_med_trsh);
    *exclh = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_GL_EXCL_MED_TRSH, EXCLH, reg_regs_gl_excl_med_trsh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_gl_status_get(uint8_t sbpm_id, bdmf_boolean *ack_stts, bdmf_boolean *excl_high_stts, bdmf_boolean *excl_med_stts, bdmf_boolean *excl_low_stts)
{
    uint32_t reg_regs_sbpm_gl_status;

#ifdef VALIDATE_PARMS
    if (!ack_stts || !excl_high_stts || !excl_med_stts || !excl_low_stts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_GL_STATUS, reg_regs_sbpm_gl_status);

    *ack_stts = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_GL_STATUS, ACK_STTS, reg_regs_sbpm_gl_status);
    *excl_high_stts = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_GL_STATUS, EXCL_HIGH_STTS, reg_regs_sbpm_gl_status);
    *excl_med_stts = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_GL_STATUS, EXCL_MED_STTS, reg_regs_sbpm_gl_status);
    *excl_low_stts = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_GL_STATUS, EXCL_LOW_STTS, reg_regs_sbpm_gl_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_iir_addr_get(uint8_t sbpm_id, uint16_t *cmd_sa, uint8_t *cmd_ta, uint16_t *bn_src)
{
    uint32_t reg_regs_iir_addr;

#ifdef VALIDATE_PARMS
    if (!cmd_sa || !cmd_ta || !bn_src)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_IIR_ADDR, reg_regs_iir_addr);

    *cmd_sa = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_IIR_ADDR, CMD_SA, reg_regs_iir_addr);
    *cmd_ta = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_IIR_ADDR, CMD_TA, reg_regs_iir_addr);
    *bn_src = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_IIR_ADDR, BN_SRC, reg_regs_iir_addr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_iir_low_get(uint8_t sbpm_id, uint32_t *cmd_data_0to31)
{
    uint32_t reg_regs_sbpm_iir_low;

#ifdef VALIDATE_PARMS
    if (!cmd_data_0to31)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_IIR_LOW, reg_regs_sbpm_iir_low);

    *cmd_data_0to31 = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_IIR_LOW, CMD_DATA_0TO31, reg_regs_sbpm_iir_low);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_iir_high_get(uint8_t sbpm_id, uint32_t *cmd_data_32to63)
{
    uint32_t reg_regs_sbpm_iir_high;

#ifdef VALIDATE_PARMS
    if (!cmd_data_32to63)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_IIR_HIGH, reg_regs_sbpm_iir_high);

    *cmd_data_32to63 = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_IIR_HIGH, CMD_DATA_32TO63, reg_regs_sbpm_iir_high);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_dbg_vec0_get(uint8_t sbpm_id, uint32_t *sbpm_fifo_empty)
{
    uint32_t reg_regs_sbpm_dbg_vec0;

#ifdef VALIDATE_PARMS
    if (!sbpm_fifo_empty)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_DBG_VEC0, reg_regs_sbpm_dbg_vec0);

    *sbpm_fifo_empty = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_DBG_VEC0, SBPM_FIFO_EMPTY, reg_regs_sbpm_dbg_vec0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_dbg_vec1_get(uint8_t sbpm_id, uint32_t *sbpm_fifo_full)
{
    uint32_t reg_regs_sbpm_dbg_vec1;

#ifdef VALIDATE_PARMS
    if (!sbpm_fifo_full)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_DBG_VEC1, reg_regs_sbpm_dbg_vec1);

    *sbpm_fifo_full = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_DBG_VEC1, SBPM_FIFO_FULL, reg_regs_sbpm_dbg_vec1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg1_set(uint8_t sbpm_id, uint32_t overwr_ra)
{
    uint32_t reg_regs_sbpm_raddr_decoder_reg1 = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_id >= BLOCK_ADDR_COUNT) ||
       (overwr_ra >= _25BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_sbpm_raddr_decoder_reg1 = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_RADDR_DECODER_REG1, OVERWR_RA, reg_regs_sbpm_raddr_decoder_reg1, overwr_ra);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_RADDR_DECODER_REG1, reg_regs_sbpm_raddr_decoder_reg1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg1_get(uint8_t sbpm_id, uint32_t *overwr_ra)
{
    uint32_t reg_regs_sbpm_raddr_decoder_reg1;

#ifdef VALIDATE_PARMS
    if (!overwr_ra)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_RADDR_DECODER_REG1, reg_regs_sbpm_raddr_decoder_reg1);

    *overwr_ra = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_RADDR_DECODER_REG1, OVERWR_RA, reg_regs_sbpm_raddr_decoder_reg1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg2_set(uint8_t sbpm_id, uint16_t id_2overwr, bdmf_boolean overwr_valid)
{
    uint32_t reg_regs_sbpm_raddr_decoder_reg2 = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_id >= BLOCK_ADDR_COUNT) ||
       (id_2overwr >= _9BITS_MAX_VAL_) ||
       (overwr_valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_sbpm_raddr_decoder_reg2 = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_RADDR_DECODER_REG2, ID_2OVERWR, reg_regs_sbpm_raddr_decoder_reg2, id_2overwr);
    reg_regs_sbpm_raddr_decoder_reg2 = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_RADDR_DECODER_REG2, OVERWR_VALID, reg_regs_sbpm_raddr_decoder_reg2, overwr_valid);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_RADDR_DECODER_REG2, reg_regs_sbpm_raddr_decoder_reg2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg2_get(uint8_t sbpm_id, uint16_t *id_2overwr, bdmf_boolean *overwr_valid)
{
    uint32_t reg_regs_sbpm_raddr_decoder_reg2;

#ifdef VALIDATE_PARMS
    if (!id_2overwr || !overwr_valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_RADDR_DECODER_REG2, reg_regs_sbpm_raddr_decoder_reg2);

    *id_2overwr = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_RADDR_DECODER_REG2, ID_2OVERWR, reg_regs_sbpm_raddr_decoder_reg2);
    *overwr_valid = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_RADDR_DECODER_REG2, OVERWR_VALID, reg_regs_sbpm_raddr_decoder_reg2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bac_max_set(uint8_t sbpm_id, uint16_t val, bdmf_boolean clr)
{
    uint32_t reg_regs_bac_max = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_id >= BLOCK_ADDR_COUNT) ||
       (val >= _15BITS_MAX_VAL_) ||
       (clr >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_bac_max = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BAC_MAX, VAL, reg_regs_bac_max, val);
    reg_regs_bac_max = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BAC_MAX, CLR, reg_regs_bac_max, clr);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BAC_MAX, reg_regs_bac_max);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bac_max_get(uint8_t sbpm_id, uint16_t *val, bdmf_boolean *clr)
{
    uint32_t reg_regs_bac_max;

#ifdef VALIDATE_PARMS
    if (!val || !clr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BAC_MAX, reg_regs_bac_max);

    *val = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BAC_MAX, VAL, reg_regs_bac_max);
    *clr = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_BAC_MAX, CLR, reg_regs_bac_max);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_spare_set(uint8_t sbpm_id, uint32_t spr)
{
    uint32_t reg_regs_sbpm_spare = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_sbpm_spare = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_SPARE, SPR, reg_regs_sbpm_spare, spr);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_SPARE, reg_regs_sbpm_spare);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_spare_get(uint8_t sbpm_id, uint32_t *spr)
{
    uint32_t reg_regs_sbpm_spare;

#ifdef VALIDATE_PARMS
    if (!spr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_SPARE, reg_regs_sbpm_spare);

    *spr = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SBPM_SPARE, SPR, reg_regs_sbpm_spare);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_cfg_bn_size_set(uint8_t sbpm_id, uint8_t size)
{
    uint32_t reg_regs_cfg_bn_size = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_id >= BLOCK_ADDR_COUNT) ||
       (size >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_cfg_bn_size = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_CFG_BN_SIZE, SIZE, reg_regs_cfg_bn_size, size);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, REGS_CFG_BN_SIZE, reg_regs_cfg_bn_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_cfg_bn_size_get(uint8_t sbpm_id, uint8_t *size)
{
    uint32_t reg_regs_cfg_bn_size;

#ifdef VALIDATE_PARMS
    if (!size)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_CFG_BN_SIZE, reg_regs_cfg_bn_size);

    *size = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_CFG_BN_SIZE, SIZE, reg_regs_cfg_bn_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_ll_weight_arb_set(uint8_t sbpm_id, uint32_t val)
{
    uint32_t reg_regs_ll_weight_arb = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_id >= BLOCK_ADDR_COUNT) ||
       (val >= _18BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_ll_weight_arb = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_LL_WEIGHT_ARB, VAL, reg_regs_ll_weight_arb, val);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, REGS_LL_WEIGHT_ARB, reg_regs_ll_weight_arb);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_ll_weight_arb_get(uint8_t sbpm_id, uint32_t *val)
{
    uint32_t reg_regs_ll_weight_arb;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_LL_WEIGHT_ARB, reg_regs_ll_weight_arb);

    *val = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_LL_WEIGHT_ARB, VAL, reg_regs_ll_weight_arb);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sp_rnr_g0_set(uint8_t sbpm_id, uint16_t start, uint16_t end)
{
    uint32_t reg_regs_sp_rnr_g0 = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_id >= BLOCK_ADDR_COUNT) ||
       (start >= _9BITS_MAX_VAL_) ||
       (end >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_sp_rnr_g0 = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SP_RNR_G0, START, reg_regs_sp_rnr_g0, start);
    reg_regs_sp_rnr_g0 = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SP_RNR_G0, END, reg_regs_sp_rnr_g0, end);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SP_RNR_G0, reg_regs_sp_rnr_g0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sp_rnr_g0_get(uint8_t sbpm_id, uint16_t *start, uint16_t *end)
{
    uint32_t reg_regs_sp_rnr_g0;

#ifdef VALIDATE_PARMS
    if (!start || !end)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SP_RNR_G0, reg_regs_sp_rnr_g0);

    *start = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SP_RNR_G0, START, reg_regs_sp_rnr_g0);
    *end = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SP_RNR_G0, END, reg_regs_sp_rnr_g0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sp_rnr_g1_set(uint8_t sbpm_id, uint16_t start, uint16_t end, bdmf_boolean valid)
{
    uint32_t reg_regs_sp_rnr_g1 = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_id >= BLOCK_ADDR_COUNT) ||
       (start >= _9BITS_MAX_VAL_) ||
       (end >= _9BITS_MAX_VAL_) ||
       (valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_sp_rnr_g1 = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SP_RNR_G1, START, reg_regs_sp_rnr_g1, start);
    reg_regs_sp_rnr_g1 = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SP_RNR_G1, END, reg_regs_sp_rnr_g1, end);
    reg_regs_sp_rnr_g1 = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SP_RNR_G1, VALID, reg_regs_sp_rnr_g1, valid);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SP_RNR_G1, reg_regs_sp_rnr_g1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sp_rnr_g1_get(uint8_t sbpm_id, uint16_t *start, uint16_t *end, bdmf_boolean *valid)
{
    uint32_t reg_regs_sp_rnr_g1;

#ifdef VALIDATE_PARMS
    if (!start || !end || !valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SP_RNR_G1, reg_regs_sp_rnr_g1);

    *start = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SP_RNR_G1, START, reg_regs_sp_rnr_g1);
    *end = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SP_RNR_G1, END, reg_regs_sp_rnr_g1);
    *valid = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_SP_RNR_G1, VALID, reg_regs_sp_rnr_g1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_nack_mask_get(uint8_t sbpm_id, uint16_t *nack_mask, bdmf_boolean *valid)
{
    uint32_t reg_regs_nack_mask;

#ifdef VALIDATE_PARMS
    if (!nack_mask || !valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_NACK_MASK, reg_regs_nack_mask);

    *nack_mask = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_NACK_MASK, NACK_MASK, reg_regs_nack_mask);
    *valid = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_NACK_MASK, VALID, reg_regs_nack_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_excl_mask_set(uint8_t sbpm_id, uint16_t excl_mask)
{
    uint32_t reg_regs_excl_mask = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_id >= BLOCK_ADDR_COUNT) ||
       (excl_mask >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_regs_excl_mask = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_EXCL_MASK, EXCL_MASK, reg_regs_excl_mask, excl_mask);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, REGS_EXCL_MASK, reg_regs_excl_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_excl_mask_get(uint8_t sbpm_id, uint16_t *excl_mask)
{
    uint32_t reg_regs_excl_mask;

#ifdef VALIDATE_PARMS
    if (!excl_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, REGS_EXCL_MASK, reg_regs_excl_mask);

    *excl_mask = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, REGS_EXCL_MASK, EXCL_MASK, reg_regs_excl_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_isr_set(uint8_t sbpm_id, const crdp_ds_cmn_sbpm_intr_ctrl_isr *intr_ctrl_isr)
{
    uint32_t reg_intr_ctrl_isr = 0;

#ifdef VALIDATE_PARMS
    if(!intr_ctrl_isr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT) ||
       (intr_ctrl_isr->free_list_non_alloc_bn >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->get2free_non_alloc_bn >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->mc2free_non_alloc_bn >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->alloc_alrdy_alloc_bn >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->get_non_alloc_bn >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->get_res_non_alloc_bn >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->connect_non_alloc_bn >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->connect_to_non_alloc_pointed_bn >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->mc_inc_non_alloc_bn >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->free_wo_context_comb >= _6BITS_MAX_VAL_) ||
       (intr_ctrl_isr->bac_underrun >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->mcst_overflow >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->empty_free_list >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->error_bb_cmd_msg >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_intr_ctrl_isr = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, FREE_LIST_NON_ALLOC_BN, reg_intr_ctrl_isr, intr_ctrl_isr->free_list_non_alloc_bn);
    reg_intr_ctrl_isr = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, GET2FREE_NON_ALLOC_BN, reg_intr_ctrl_isr, intr_ctrl_isr->get2free_non_alloc_bn);
    reg_intr_ctrl_isr = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, MC2FREE_NON_ALLOC_BN, reg_intr_ctrl_isr, intr_ctrl_isr->mc2free_non_alloc_bn);
    reg_intr_ctrl_isr = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, ALLOC_ALRDY_ALLOC_BN, reg_intr_ctrl_isr, intr_ctrl_isr->alloc_alrdy_alloc_bn);
    reg_intr_ctrl_isr = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, GET_NON_ALLOC_BN, reg_intr_ctrl_isr, intr_ctrl_isr->get_non_alloc_bn);
    reg_intr_ctrl_isr = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, GET_RES_NON_ALLOC_BN, reg_intr_ctrl_isr, intr_ctrl_isr->get_res_non_alloc_bn);
    reg_intr_ctrl_isr = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, CONNECT_NON_ALLOC_BN, reg_intr_ctrl_isr, intr_ctrl_isr->connect_non_alloc_bn);
    reg_intr_ctrl_isr = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, CONNECT_TO_NON_ALLOC_POINTED_BN, reg_intr_ctrl_isr, intr_ctrl_isr->connect_to_non_alloc_pointed_bn);
    reg_intr_ctrl_isr = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, MC_INC_NON_ALLOC_BN, reg_intr_ctrl_isr, intr_ctrl_isr->mc_inc_non_alloc_bn);
    reg_intr_ctrl_isr = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, FREE_WO_CONTEXT_COMB, reg_intr_ctrl_isr, intr_ctrl_isr->free_wo_context_comb);
    reg_intr_ctrl_isr = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, BAC_UNDERRUN, reg_intr_ctrl_isr, intr_ctrl_isr->bac_underrun);
    reg_intr_ctrl_isr = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, MCST_OVERFLOW, reg_intr_ctrl_isr, intr_ctrl_isr->mcst_overflow);
    reg_intr_ctrl_isr = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, EMPTY_FREE_LIST, reg_intr_ctrl_isr, intr_ctrl_isr->empty_free_list);
    reg_intr_ctrl_isr = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, ERROR_BB_CMD_MSG, reg_intr_ctrl_isr, intr_ctrl_isr->error_bb_cmd_msg);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, reg_intr_ctrl_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_isr_get(uint8_t sbpm_id, crdp_ds_cmn_sbpm_intr_ctrl_isr *intr_ctrl_isr)
{
    uint32_t reg_intr_ctrl_isr;

#ifdef VALIDATE_PARMS
    if (!intr_ctrl_isr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, reg_intr_ctrl_isr);

    intr_ctrl_isr->free_list_non_alloc_bn = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, FREE_LIST_NON_ALLOC_BN, reg_intr_ctrl_isr);
    intr_ctrl_isr->get2free_non_alloc_bn = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, GET2FREE_NON_ALLOC_BN, reg_intr_ctrl_isr);
    intr_ctrl_isr->mc2free_non_alloc_bn = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, MC2FREE_NON_ALLOC_BN, reg_intr_ctrl_isr);
    intr_ctrl_isr->alloc_alrdy_alloc_bn = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, ALLOC_ALRDY_ALLOC_BN, reg_intr_ctrl_isr);
    intr_ctrl_isr->get_non_alloc_bn = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, GET_NON_ALLOC_BN, reg_intr_ctrl_isr);
    intr_ctrl_isr->get_res_non_alloc_bn = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, GET_RES_NON_ALLOC_BN, reg_intr_ctrl_isr);
    intr_ctrl_isr->connect_non_alloc_bn = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, CONNECT_NON_ALLOC_BN, reg_intr_ctrl_isr);
    intr_ctrl_isr->connect_to_non_alloc_pointed_bn = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, CONNECT_TO_NON_ALLOC_POINTED_BN, reg_intr_ctrl_isr);
    intr_ctrl_isr->mc_inc_non_alloc_bn = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, MC_INC_NON_ALLOC_BN, reg_intr_ctrl_isr);
    intr_ctrl_isr->free_wo_context_comb = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, FREE_WO_CONTEXT_COMB, reg_intr_ctrl_isr);
    intr_ctrl_isr->bac_underrun = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, BAC_UNDERRUN, reg_intr_ctrl_isr);
    intr_ctrl_isr->mcst_overflow = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, MCST_OVERFLOW, reg_intr_ctrl_isr);
    intr_ctrl_isr->empty_free_list = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, EMPTY_FREE_LIST, reg_intr_ctrl_isr);
    intr_ctrl_isr->error_bb_cmd_msg = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISR, ERROR_BB_CMD_MSG, reg_intr_ctrl_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_ism_get(uint8_t sbpm_id, uint32_t *ism)
{
    uint32_t reg_intr_ctrl_ism;

#ifdef VALIDATE_PARMS
    if (!ism)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISM, reg_intr_ctrl_ism);

    *ism = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ISM, ISM, reg_intr_ctrl_ism);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_ier_set(uint8_t sbpm_id, uint32_t iem)
{
    uint32_t reg_intr_ctrl_ier = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_intr_ctrl_ier = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_IER, IEM, reg_intr_ctrl_ier, iem);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_IER, reg_intr_ctrl_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_ier_get(uint8_t sbpm_id, uint32_t *iem)
{
    uint32_t reg_intr_ctrl_ier;

#ifdef VALIDATE_PARMS
    if (!iem)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_IER, reg_intr_ctrl_ier);

    *iem = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_IER, IEM, reg_intr_ctrl_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_itr_set(uint8_t sbpm_id, uint32_t ist)
{
    uint32_t reg_intr_ctrl_itr = 0;

#ifdef VALIDATE_PARMS
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_intr_ctrl_itr = RU_FIELD_SET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ITR, IST, reg_intr_ctrl_itr, ist);

    RU_REG_WRITE(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ITR, reg_intr_ctrl_itr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_itr_get(uint8_t sbpm_id, uint32_t *ist)
{
    uint32_t reg_intr_ctrl_itr;

#ifdef VALIDATE_PARMS
    if (!ist)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sbpm_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ITR, reg_intr_ctrl_itr);

    *ist = RU_FIELD_GET(sbpm_id, CRDP_DS_CMN_SBPM, INTR_CTRL_ITR, IST, reg_intr_ctrl_itr);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_regs_init_free_list,
    bdmf_address_regs_bn_alloc,
    bdmf_address_regs_bn_alloc_rply,
    bdmf_address_regs_mcst_inc,
    bdmf_address_regs_mcst_inc_rply,
    bdmf_address_regs_bn_connect,
    bdmf_address_regs_bn_connect_rply,
    bdmf_address_regs_get_next,
    bdmf_address_regs_get_next_rply,
    bdmf_address_regs_sbpm_clk_gate_cntrl,
    bdmf_address_regs_bn_free_without_contxt_reg1,
    bdmf_address_regs_bn_free_without_contxt_reg2,
    bdmf_address_regs_bn_free_without_contxt_rply,
    bdmf_address_regs_gl_trsh,
    bdmf_address_regs_sbpm_dbg,
    bdmf_address_regs_gl_bac,
    bdmf_address_regs_gl_excl_high_trsh,
    bdmf_address_regs_gl_excl_low_trsh,
    bdmf_address_regs_gl_excl_med_trsh,
    bdmf_address_regs_sbpm_gl_status,
    bdmf_address_regs_iir_addr,
    bdmf_address_regs_sbpm_iir_low,
    bdmf_address_regs_sbpm_iir_high,
    bdmf_address_regs_sbpm_dbg_vec0,
    bdmf_address_regs_sbpm_dbg_vec1,
    bdmf_address_regs_sbpm_raddr_decoder_reg1,
    bdmf_address_regs_sbpm_raddr_decoder_reg2,
    bdmf_address_regs_bac_max,
    bdmf_address_regs_sbpm_spare,
    bdmf_address_regs_cfg_bn_size,
    bdmf_address_regs_ll_weight_arb,
    bdmf_address_regs_sp_rnr_g0,
    bdmf_address_regs_sp_rnr_g1,
    bdmf_address_regs_nack_mask,
    bdmf_address_regs_excl_mask,
    bdmf_address_intr_ctrl_isr,
    bdmf_address_intr_ctrl_ism,
    bdmf_address_intr_ctrl_ier,
    bdmf_address_intr_ctrl_itr,
}
bdmf_address;

static int ag_drv_crdp_ds_cmn_sbpm_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_crdp_ds_cmn_sbpm_regs_init_free_list:
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_init_free_list_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_crdp_ds_cmn_sbpm_regs_bn_alloc:
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bn_alloc_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_crdp_ds_cmn_sbpm_regs_mcst_inc:
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_mcst_inc_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_crdp_ds_cmn_sbpm_regs_bn_connect:
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bn_connect_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_crdp_ds_cmn_sbpm_regs_get_next:
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_get_next_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_crdp_ds_cmn_sbpm_regs_sbpm_clk_gate_cntrl:
    {
        crdp_ds_cmn_sbpm_regs_sbpm_clk_gate_cntrl regs_sbpm_clk_gate_cntrl = { .bypass_clk_gate = parm[2].value.unumber, .timer_val = parm[3].value.unumber, .keep_alive_en = parm[4].value.unumber, .keep_alive_intervl = parm[5].value.unumber, .keep_alive_cyc = parm[6].value.unumber};
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_clk_gate_cntrl_set(parm[1].value.unumber, &regs_sbpm_clk_gate_cntrl);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg1:
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg2:
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_sbpm_regs_gl_trsh:
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_gl_trsh_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_sbpm_regs_sbpm_dbg:
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_dbg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_sbpm_regs_gl_excl_high_trsh:
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_high_trsh_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_sbpm_regs_gl_excl_low_trsh:
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_low_trsh_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_sbpm_regs_gl_excl_med_trsh:
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_med_trsh_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg1:
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg1_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg2:
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_sbpm_regs_bac_max:
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bac_max_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_sbpm_regs_sbpm_spare:
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_spare_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_crdp_ds_cmn_sbpm_regs_cfg_bn_size:
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_cfg_bn_size_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_crdp_ds_cmn_sbpm_regs_ll_weight_arb:
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_ll_weight_arb_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_crdp_ds_cmn_sbpm_regs_sp_rnr_g0:
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sp_rnr_g0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_sbpm_regs_sp_rnr_g1:
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sp_rnr_g1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_crdp_ds_cmn_sbpm_regs_excl_mask:
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_excl_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_crdp_ds_cmn_sbpm_intr_ctrl_isr:
    {
        crdp_ds_cmn_sbpm_intr_ctrl_isr intr_ctrl_isr = { .free_list_non_alloc_bn = parm[2].value.unumber, .get2free_non_alloc_bn = parm[3].value.unumber, .mc2free_non_alloc_bn = parm[4].value.unumber, .alloc_alrdy_alloc_bn = parm[5].value.unumber, .get_non_alloc_bn = parm[6].value.unumber, .get_res_non_alloc_bn = parm[7].value.unumber, .connect_non_alloc_bn = parm[8].value.unumber, .connect_to_non_alloc_pointed_bn = parm[9].value.unumber, .mc_inc_non_alloc_bn = parm[10].value.unumber, .free_wo_context_comb = parm[11].value.unumber, .bac_underrun = parm[12].value.unumber, .mcst_overflow = parm[13].value.unumber, .empty_free_list = parm[14].value.unumber, .error_bb_cmd_msg = parm[15].value.unumber};
        ag_err = ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_isr_set(parm[1].value.unumber, &intr_ctrl_isr);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_intr_ctrl_ier:
        ag_err = ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_ier_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_crdp_ds_cmn_sbpm_intr_ctrl_itr:
        ag_err = ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_itr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_crdp_ds_cmn_sbpm_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_crdp_ds_cmn_sbpm_regs_init_free_list:
    {
        uint16_t init_max_bn;
        bdmf_boolean ram_init_start;
        bdmf_boolean bsy;
        bdmf_boolean rdy;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_init_free_list_get(parm[1].value.unumber, &init_max_bn, &ram_init_start, &bsy, &rdy);
        bdmf_session_print(session, "init_max_bn = %u = 0x%x\n", init_max_bn, init_max_bn);
        bdmf_session_print(session, "ram_init_start = %u = 0x%x\n", ram_init_start, ram_init_start);
        bdmf_session_print(session, "bsy = %u = 0x%x\n", bsy, bsy);
        bdmf_session_print(session, "rdy = %u = 0x%x\n", rdy, rdy);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_bn_alloc:
    {
        bdmf_boolean req;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bn_alloc_get(parm[1].value.unumber, &req);
        bdmf_session_print(session, "req = %u = 0x%x\n", req, req);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_bn_alloc_rply:
    {
        crdp_ds_cmn_sbpm_regs_bn_alloc_rply regs_bn_alloc_rply;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bn_alloc_rply_get(parm[1].value.unumber, &regs_bn_alloc_rply);
        bdmf_session_print(session, "alloc_bn_valid = %u = 0x%x\n", regs_bn_alloc_rply.alloc_bn_valid, regs_bn_alloc_rply.alloc_bn_valid);
        bdmf_session_print(session, "alloc_bn = %u = 0x%x\n", regs_bn_alloc_rply.alloc_bn, regs_bn_alloc_rply.alloc_bn);
        bdmf_session_print(session, "ack = %u = 0x%x\n", regs_bn_alloc_rply.ack, regs_bn_alloc_rply.ack);
        bdmf_session_print(session, "nack = %u = 0x%x\n", regs_bn_alloc_rply.nack, regs_bn_alloc_rply.nack);
        bdmf_session_print(session, "excl_high = %u = 0x%x\n", regs_bn_alloc_rply.excl_high, regs_bn_alloc_rply.excl_high);
        bdmf_session_print(session, "excl_med = %u = 0x%x\n", regs_bn_alloc_rply.excl_med, regs_bn_alloc_rply.excl_med);
        bdmf_session_print(session, "excl_low = %u = 0x%x\n", regs_bn_alloc_rply.excl_low, regs_bn_alloc_rply.excl_low);
        bdmf_session_print(session, "busy = %u = 0x%x\n", regs_bn_alloc_rply.busy, regs_bn_alloc_rply.busy);
        bdmf_session_print(session, "rdy = %u = 0x%x\n", regs_bn_alloc_rply.rdy, regs_bn_alloc_rply.rdy);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_mcst_inc:
    {
        uint16_t bn;
        uint8_t mcst_val;
        bdmf_boolean ack_req;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_mcst_inc_get(parm[1].value.unumber, &bn, &mcst_val, &ack_req);
        bdmf_session_print(session, "bn = %u = 0x%x\n", bn, bn);
        bdmf_session_print(session, "mcst_val = %u = 0x%x\n", mcst_val, mcst_val);
        bdmf_session_print(session, "ack_req = %u = 0x%x\n", ack_req, ack_req);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_mcst_inc_rply:
    {
        bdmf_boolean mcst_ack;
        uint8_t mc_res;
        bdmf_boolean bsy;
        bdmf_boolean rdy;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_mcst_inc_rply_get(parm[1].value.unumber, &mcst_ack, &mc_res, &bsy, &rdy);
        bdmf_session_print(session, "mcst_ack = %u = 0x%x\n", mcst_ack, mcst_ack);
        bdmf_session_print(session, "mc_res = %u = 0x%x\n", mc_res, mc_res);
        bdmf_session_print(session, "bsy = %u = 0x%x\n", bsy, bsy);
        bdmf_session_print(session, "rdy = %u = 0x%x\n", rdy, rdy);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_bn_connect:
    {
        uint16_t bn;
        bdmf_boolean ack_req;
        uint16_t pointed_bn;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bn_connect_get(parm[1].value.unumber, &bn, &ack_req, &pointed_bn);
        bdmf_session_print(session, "bn = %u = 0x%x\n", bn, bn);
        bdmf_session_print(session, "ack_req = %u = 0x%x\n", ack_req, ack_req);
        bdmf_session_print(session, "pointed_bn = %u = 0x%x\n", pointed_bn, pointed_bn);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_bn_connect_rply:
    {
        bdmf_boolean connect_ack;
        bdmf_boolean busy;
        bdmf_boolean rdy;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bn_connect_rply_get(parm[1].value.unumber, &connect_ack, &busy, &rdy);
        bdmf_session_print(session, "connect_ack = %u = 0x%x\n", connect_ack, connect_ack);
        bdmf_session_print(session, "busy = %u = 0x%x\n", busy, busy);
        bdmf_session_print(session, "rdy = %u = 0x%x\n", rdy, rdy);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_get_next:
    {
        uint16_t bn;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_get_next_get(parm[1].value.unumber, &bn);
        bdmf_session_print(session, "bn = %u = 0x%x\n", bn, bn);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_get_next_rply:
    {
        bdmf_boolean bn_valid;
        uint16_t next_bn;
        bdmf_boolean busy;
        bdmf_boolean rdy;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_get_next_rply_get(parm[1].value.unumber, &bn_valid, &next_bn, &busy, &rdy);
        bdmf_session_print(session, "bn_valid = %u = 0x%x\n", bn_valid, bn_valid);
        bdmf_session_print(session, "next_bn = %u = 0x%x\n", next_bn, next_bn);
        bdmf_session_print(session, "busy = %u = 0x%x\n", busy, busy);
        bdmf_session_print(session, "rdy = %u = 0x%x\n", rdy, rdy);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_sbpm_clk_gate_cntrl:
    {
        crdp_ds_cmn_sbpm_regs_sbpm_clk_gate_cntrl regs_sbpm_clk_gate_cntrl;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_clk_gate_cntrl_get(parm[1].value.unumber, &regs_sbpm_clk_gate_cntrl);
        bdmf_session_print(session, "bypass_clk_gate = %u = 0x%x\n", regs_sbpm_clk_gate_cntrl.bypass_clk_gate, regs_sbpm_clk_gate_cntrl.bypass_clk_gate);
        bdmf_session_print(session, "timer_val = %u = 0x%x\n", regs_sbpm_clk_gate_cntrl.timer_val, regs_sbpm_clk_gate_cntrl.timer_val);
        bdmf_session_print(session, "keep_alive_en = %u = 0x%x\n", regs_sbpm_clk_gate_cntrl.keep_alive_en, regs_sbpm_clk_gate_cntrl.keep_alive_en);
        bdmf_session_print(session, "keep_alive_intervl = %u = 0x%x\n", regs_sbpm_clk_gate_cntrl.keep_alive_intervl, regs_sbpm_clk_gate_cntrl.keep_alive_intervl);
        bdmf_session_print(session, "keep_alive_cyc = %u = 0x%x\n", regs_sbpm_clk_gate_cntrl.keep_alive_cyc, regs_sbpm_clk_gate_cntrl.keep_alive_cyc);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg1:
    {
        uint16_t plen;
        uint16_t sop_offset;
        bdmf_boolean calc;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg1_get(parm[1].value.unumber, &plen, &sop_offset, &calc);
        bdmf_session_print(session, "plen = %u = 0x%x\n", plen, plen);
        bdmf_session_print(session, "sop_offset = %u = 0x%x\n", sop_offset, sop_offset);
        bdmf_session_print(session, "calc = %u = 0x%x\n", calc, calc);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg2:
    {
        uint16_t first_bn;
        uint8_t num_packet;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg2_get(parm[1].value.unumber, &first_bn, &num_packet);
        bdmf_session_print(session, "first_bn = %u = 0x%x\n", first_bn, first_bn);
        bdmf_session_print(session, "num_packet = %u = 0x%x\n", num_packet, num_packet);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_rply:
    {
        crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_rply regs_bn_free_without_contxt_rply;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_rply_get(parm[1].value.unumber, &regs_bn_free_without_contxt_rply);
        bdmf_session_print(session, "free_ack = %u = 0x%x\n", regs_bn_free_without_contxt_rply.free_ack, regs_bn_free_without_contxt_rply.free_ack);
        bdmf_session_print(session, "ack_stat = %u = 0x%x\n", regs_bn_free_without_contxt_rply.ack_stat, regs_bn_free_without_contxt_rply.ack_stat);
        bdmf_session_print(session, "nack_stat = %u = 0x%x\n", regs_bn_free_without_contxt_rply.nack_stat, regs_bn_free_without_contxt_rply.nack_stat);
        bdmf_session_print(session, "excl_high_stat = %u = 0x%x\n", regs_bn_free_without_contxt_rply.excl_high_stat, regs_bn_free_without_contxt_rply.excl_high_stat);
        bdmf_session_print(session, "excl_med_stat = %u = 0x%x\n", regs_bn_free_without_contxt_rply.excl_med_stat, regs_bn_free_without_contxt_rply.excl_med_stat);
        bdmf_session_print(session, "excl_low_stat = %u = 0x%x\n", regs_bn_free_without_contxt_rply.excl_low_stat, regs_bn_free_without_contxt_rply.excl_low_stat);
        bdmf_session_print(session, "bsy = %u = 0x%x\n", regs_bn_free_without_contxt_rply.bsy, regs_bn_free_without_contxt_rply.bsy);
        bdmf_session_print(session, "rdy = %u = 0x%x\n", regs_bn_free_without_contxt_rply.rdy, regs_bn_free_without_contxt_rply.rdy);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_gl_trsh:
    {
        uint16_t gl_bat;
        uint16_t gl_bah;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_gl_trsh_get(parm[1].value.unumber, &gl_bat, &gl_bah);
        bdmf_session_print(session, "gl_bat = %u = 0x%x\n", gl_bat, gl_bat);
        bdmf_session_print(session, "gl_bah = %u = 0x%x\n", gl_bah, gl_bah);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_sbpm_dbg:
    {
        uint8_t select_bus;
        uint16_t window_start;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_dbg_get(parm[1].value.unumber, &select_bus, &window_start);
        bdmf_session_print(session, "select_bus = %u = 0x%x\n", select_bus, select_bus);
        bdmf_session_print(session, "window_start = %u = 0x%x\n", window_start, window_start);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_gl_bac:
    {
        uint16_t val;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_gl_bac_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_gl_excl_high_trsh:
    {
        uint16_t exclt;
        uint16_t exclh;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_high_trsh_get(parm[1].value.unumber, &exclt, &exclh);
        bdmf_session_print(session, "exclt = %u = 0x%x\n", exclt, exclt);
        bdmf_session_print(session, "exclh = %u = 0x%x\n", exclh, exclh);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_gl_excl_low_trsh:
    {
        uint16_t exclt;
        uint16_t exclh;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_low_trsh_get(parm[1].value.unumber, &exclt, &exclh);
        bdmf_session_print(session, "exclt = %u = 0x%x\n", exclt, exclt);
        bdmf_session_print(session, "exclh = %u = 0x%x\n", exclh, exclh);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_gl_excl_med_trsh:
    {
        uint16_t exclt;
        uint16_t exclh;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_med_trsh_get(parm[1].value.unumber, &exclt, &exclh);
        bdmf_session_print(session, "exclt = %u = 0x%x\n", exclt, exclt);
        bdmf_session_print(session, "exclh = %u = 0x%x\n", exclh, exclh);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_sbpm_gl_status:
    {
        bdmf_boolean ack_stts;
        bdmf_boolean excl_high_stts;
        bdmf_boolean excl_med_stts;
        bdmf_boolean excl_low_stts;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_gl_status_get(parm[1].value.unumber, &ack_stts, &excl_high_stts, &excl_med_stts, &excl_low_stts);
        bdmf_session_print(session, "ack_stts = %u = 0x%x\n", ack_stts, ack_stts);
        bdmf_session_print(session, "excl_high_stts = %u = 0x%x\n", excl_high_stts, excl_high_stts);
        bdmf_session_print(session, "excl_med_stts = %u = 0x%x\n", excl_med_stts, excl_med_stts);
        bdmf_session_print(session, "excl_low_stts = %u = 0x%x\n", excl_low_stts, excl_low_stts);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_iir_addr:
    {
        uint16_t cmd_sa;
        uint8_t cmd_ta;
        uint16_t bn_src;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_iir_addr_get(parm[1].value.unumber, &cmd_sa, &cmd_ta, &bn_src);
        bdmf_session_print(session, "cmd_sa = %u = 0x%x\n", cmd_sa, cmd_sa);
        bdmf_session_print(session, "cmd_ta = %u = 0x%x\n", cmd_ta, cmd_ta);
        bdmf_session_print(session, "bn_src = %u = 0x%x\n", bn_src, bn_src);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_sbpm_iir_low:
    {
        uint32_t cmd_data_0to31;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_iir_low_get(parm[1].value.unumber, &cmd_data_0to31);
        bdmf_session_print(session, "cmd_data_0to31 = %u = 0x%x\n", cmd_data_0to31, cmd_data_0to31);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_sbpm_iir_high:
    {
        uint32_t cmd_data_32to63;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_iir_high_get(parm[1].value.unumber, &cmd_data_32to63);
        bdmf_session_print(session, "cmd_data_32to63 = %u = 0x%x\n", cmd_data_32to63, cmd_data_32to63);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_sbpm_dbg_vec0:
    {
        uint32_t sbpm_fifo_empty;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_dbg_vec0_get(parm[1].value.unumber, &sbpm_fifo_empty);
        bdmf_session_print(session, "sbpm_fifo_empty = %u = 0x%x\n", sbpm_fifo_empty, sbpm_fifo_empty);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_sbpm_dbg_vec1:
    {
        uint32_t sbpm_fifo_full;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_dbg_vec1_get(parm[1].value.unumber, &sbpm_fifo_full);
        bdmf_session_print(session, "sbpm_fifo_full = %u = 0x%x\n", sbpm_fifo_full, sbpm_fifo_full);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg1:
    {
        uint32_t overwr_ra;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg1_get(parm[1].value.unumber, &overwr_ra);
        bdmf_session_print(session, "overwr_ra = %u = 0x%x\n", overwr_ra, overwr_ra);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg2:
    {
        uint16_t id_2overwr;
        bdmf_boolean overwr_valid;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg2_get(parm[1].value.unumber, &id_2overwr, &overwr_valid);
        bdmf_session_print(session, "id_2overwr = %u = 0x%x\n", id_2overwr, id_2overwr);
        bdmf_session_print(session, "overwr_valid = %u = 0x%x\n", overwr_valid, overwr_valid);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_bac_max:
    {
        uint16_t val;
        bdmf_boolean clr;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bac_max_get(parm[1].value.unumber, &val, &clr);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        bdmf_session_print(session, "clr = %u = 0x%x\n", clr, clr);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_sbpm_spare:
    {
        uint32_t spr;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_spare_get(parm[1].value.unumber, &spr);
        bdmf_session_print(session, "spr = %u = 0x%x\n", spr, spr);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_cfg_bn_size:
    {
        uint8_t size;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_cfg_bn_size_get(parm[1].value.unumber, &size);
        bdmf_session_print(session, "size = %u = 0x%x\n", size, size);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_ll_weight_arb:
    {
        uint32_t val;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_ll_weight_arb_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_sp_rnr_g0:
    {
        uint16_t start;
        uint16_t end;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sp_rnr_g0_get(parm[1].value.unumber, &start, &end);
        bdmf_session_print(session, "start = %u = 0x%x\n", start, start);
        bdmf_session_print(session, "end = %u = 0x%x\n", end, end);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_sp_rnr_g1:
    {
        uint16_t start;
        uint16_t end;
        bdmf_boolean valid;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sp_rnr_g1_get(parm[1].value.unumber, &start, &end, &valid);
        bdmf_session_print(session, "start = %u = 0x%x\n", start, start);
        bdmf_session_print(session, "end = %u = 0x%x\n", end, end);
        bdmf_session_print(session, "valid = %u = 0x%x\n", valid, valid);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_nack_mask:
    {
        uint16_t nack_mask;
        bdmf_boolean valid;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_nack_mask_get(parm[1].value.unumber, &nack_mask, &valid);
        bdmf_session_print(session, "nack_mask = %u = 0x%x\n", nack_mask, nack_mask);
        bdmf_session_print(session, "valid = %u = 0x%x\n", valid, valid);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_regs_excl_mask:
    {
        uint16_t excl_mask;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_excl_mask_get(parm[1].value.unumber, &excl_mask);
        bdmf_session_print(session, "excl_mask = %u = 0x%x\n", excl_mask, excl_mask);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_intr_ctrl_isr:
    {
        crdp_ds_cmn_sbpm_intr_ctrl_isr intr_ctrl_isr;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_isr_get(parm[1].value.unumber, &intr_ctrl_isr);
        bdmf_session_print(session, "free_list_non_alloc_bn = %u = 0x%x\n", intr_ctrl_isr.free_list_non_alloc_bn, intr_ctrl_isr.free_list_non_alloc_bn);
        bdmf_session_print(session, "get2free_non_alloc_bn = %u = 0x%x\n", intr_ctrl_isr.get2free_non_alloc_bn, intr_ctrl_isr.get2free_non_alloc_bn);
        bdmf_session_print(session, "mc2free_non_alloc_bn = %u = 0x%x\n", intr_ctrl_isr.mc2free_non_alloc_bn, intr_ctrl_isr.mc2free_non_alloc_bn);
        bdmf_session_print(session, "alloc_alrdy_alloc_bn = %u = 0x%x\n", intr_ctrl_isr.alloc_alrdy_alloc_bn, intr_ctrl_isr.alloc_alrdy_alloc_bn);
        bdmf_session_print(session, "get_non_alloc_bn = %u = 0x%x\n", intr_ctrl_isr.get_non_alloc_bn, intr_ctrl_isr.get_non_alloc_bn);
        bdmf_session_print(session, "get_res_non_alloc_bn = %u = 0x%x\n", intr_ctrl_isr.get_res_non_alloc_bn, intr_ctrl_isr.get_res_non_alloc_bn);
        bdmf_session_print(session, "connect_non_alloc_bn = %u = 0x%x\n", intr_ctrl_isr.connect_non_alloc_bn, intr_ctrl_isr.connect_non_alloc_bn);
        bdmf_session_print(session, "connect_to_non_alloc_pointed_bn = %u = 0x%x\n", intr_ctrl_isr.connect_to_non_alloc_pointed_bn, intr_ctrl_isr.connect_to_non_alloc_pointed_bn);
        bdmf_session_print(session, "mc_inc_non_alloc_bn = %u = 0x%x\n", intr_ctrl_isr.mc_inc_non_alloc_bn, intr_ctrl_isr.mc_inc_non_alloc_bn);
        bdmf_session_print(session, "free_wo_context_comb = %u = 0x%x\n", intr_ctrl_isr.free_wo_context_comb, intr_ctrl_isr.free_wo_context_comb);
        bdmf_session_print(session, "bac_underrun = %u = 0x%x\n", intr_ctrl_isr.bac_underrun, intr_ctrl_isr.bac_underrun);
        bdmf_session_print(session, "mcst_overflow = %u = 0x%x\n", intr_ctrl_isr.mcst_overflow, intr_ctrl_isr.mcst_overflow);
        bdmf_session_print(session, "empty_free_list = %u = 0x%x\n", intr_ctrl_isr.empty_free_list, intr_ctrl_isr.empty_free_list);
        bdmf_session_print(session, "error_bb_cmd_msg = %u = 0x%x\n", intr_ctrl_isr.error_bb_cmd_msg, intr_ctrl_isr.error_bb_cmd_msg);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_intr_ctrl_ism:
    {
        uint32_t ism;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_ism_get(parm[1].value.unumber, &ism);
        bdmf_session_print(session, "ism = %u = 0x%x\n", ism, ism);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_intr_ctrl_ier:
    {
        uint32_t iem;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_ier_get(parm[1].value.unumber, &iem);
        bdmf_session_print(session, "iem = %u = 0x%x\n", iem, iem);
        break;
    }
    case cli_crdp_ds_cmn_sbpm_intr_ctrl_itr:
    {
        uint32_t ist;
        ag_err = ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_itr_get(parm[1].value.unumber, &ist);
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
static int ag_drv_crdp_ds_cmn_sbpm_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t sbpm_id = parm[1].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        uint16_t init_max_bn = gtmv(m, 14);
        bdmf_boolean ram_init_start = gtmv(m, 1);
        bdmf_boolean bsy = gtmv(m, 1);
        bdmf_boolean rdy = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_init_free_list_set(%u %u %u %u %u)\n", sbpm_id,
            init_max_bn, ram_init_start, bsy, rdy);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_init_free_list_set(sbpm_id, init_max_bn, ram_init_start, bsy, rdy);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_init_free_list_get(sbpm_id, &init_max_bn, &ram_init_start, &bsy, &rdy);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_init_free_list_get(%u %u %u %u %u)\n", sbpm_id,
                init_max_bn, ram_init_start, bsy, rdy);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (init_max_bn != gtmv(m, 14) || ram_init_start != gtmv(m, 1) || bsy != gtmv(m, 1) || rdy != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean req = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_bn_alloc_set(%u %u)\n", sbpm_id,
            req);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bn_alloc_set(sbpm_id, req);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bn_alloc_get(sbpm_id, &req);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_bn_alloc_get(%u %u)\n", sbpm_id,
                req);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (req != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        crdp_ds_cmn_sbpm_regs_bn_alloc_rply regs_bn_alloc_rply = {.alloc_bn_valid = gtmv(m, 1), .alloc_bn = gtmv(m, 14), .ack = gtmv(m, 1), .nack = gtmv(m, 1), .excl_high = gtmv(m, 1), .excl_med = gtmv(m, 1), .excl_low = gtmv(m, 1), .busy = gtmv(m, 1), .rdy = gtmv(m, 1)};
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bn_alloc_rply_get(sbpm_id, &regs_bn_alloc_rply);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_bn_alloc_rply_get(%u %u %u %u %u %u %u %u %u %u)\n", sbpm_id,
                regs_bn_alloc_rply.alloc_bn_valid, regs_bn_alloc_rply.alloc_bn, regs_bn_alloc_rply.ack, regs_bn_alloc_rply.nack, 
                regs_bn_alloc_rply.excl_high, regs_bn_alloc_rply.excl_med, regs_bn_alloc_rply.excl_low, regs_bn_alloc_rply.busy, 
                regs_bn_alloc_rply.rdy);
        }
    }

    {
        uint16_t bn = gtmv(m, 14);
        uint8_t mcst_val = gtmv(m, 3);
        bdmf_boolean ack_req = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_mcst_inc_set(%u %u %u %u)\n", sbpm_id,
            bn, mcst_val, ack_req);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_mcst_inc_set(sbpm_id, bn, mcst_val, ack_req);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_mcst_inc_get(sbpm_id, &bn, &mcst_val, &ack_req);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_mcst_inc_get(%u %u %u %u)\n", sbpm_id,
                bn, mcst_val, ack_req);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bn != gtmv(m, 14) || mcst_val != gtmv(m, 3) || ack_req != gtmv(m, 1))
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
        uint8_t mc_res = gtmv(m, 3);
        bdmf_boolean bsy = gtmv(m, 1);
        bdmf_boolean rdy = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_mcst_inc_rply_get(sbpm_id, &mcst_ack, &mc_res, &bsy, &rdy);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_mcst_inc_rply_get(%u %u %u %u %u)\n", sbpm_id,
                mcst_ack, mc_res, bsy, rdy);
        }
    }

    {
        uint16_t bn = gtmv(m, 14);
        bdmf_boolean ack_req = gtmv(m, 1);
        uint16_t pointed_bn = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_bn_connect_set(%u %u %u %u)\n", sbpm_id,
            bn, ack_req, pointed_bn);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bn_connect_set(sbpm_id, bn, ack_req, pointed_bn);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bn_connect_get(sbpm_id, &bn, &ack_req, &pointed_bn);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_bn_connect_get(%u %u %u %u)\n", sbpm_id,
                bn, ack_req, pointed_bn);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bn != gtmv(m, 14) || ack_req != gtmv(m, 1) || pointed_bn != gtmv(m, 14))
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
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bn_connect_rply_get(sbpm_id, &connect_ack, &busy, &rdy);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_bn_connect_rply_get(%u %u %u %u)\n", sbpm_id,
                connect_ack, busy, rdy);
        }
    }

    {
        uint16_t bn = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_get_next_set(%u %u)\n", sbpm_id,
            bn);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_get_next_set(sbpm_id, bn);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_get_next_get(sbpm_id, &bn);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_get_next_get(%u %u)\n", sbpm_id,
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
        bdmf_boolean bn_valid = gtmv(m, 1);
        uint16_t next_bn = gtmv(m, 14);
        bdmf_boolean busy = gtmv(m, 1);
        bdmf_boolean rdy = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_get_next_rply_get(sbpm_id, &bn_valid, &next_bn, &busy, &rdy);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_get_next_rply_get(%u %u %u %u %u)\n", sbpm_id,
                bn_valid, next_bn, busy, rdy);
        }
    }

    {
        crdp_ds_cmn_sbpm_regs_sbpm_clk_gate_cntrl regs_sbpm_clk_gate_cntrl = {.bypass_clk_gate = gtmv(m, 1), .timer_val = gtmv(m, 8), .keep_alive_en = gtmv(m, 1), .keep_alive_intervl = gtmv(m, 3), .keep_alive_cyc = gtmv(m, 8)};
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_clk_gate_cntrl_set(%u %u %u %u %u %u)\n", sbpm_id,
            regs_sbpm_clk_gate_cntrl.bypass_clk_gate, regs_sbpm_clk_gate_cntrl.timer_val, regs_sbpm_clk_gate_cntrl.keep_alive_en, regs_sbpm_clk_gate_cntrl.keep_alive_intervl, 
            regs_sbpm_clk_gate_cntrl.keep_alive_cyc);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_clk_gate_cntrl_set(sbpm_id, &regs_sbpm_clk_gate_cntrl);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_clk_gate_cntrl_get(sbpm_id, &regs_sbpm_clk_gate_cntrl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_clk_gate_cntrl_get(%u %u %u %u %u %u)\n", sbpm_id,
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
        uint16_t plen = gtmv(m, 14);
        uint16_t sop_offset = gtmv(m, 10);
        bdmf_boolean calc = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg1_set(%u %u %u %u)\n", sbpm_id,
            plen, sop_offset, calc);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg1_set(sbpm_id, plen, sop_offset, calc);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg1_get(sbpm_id, &plen, &sop_offset, &calc);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg1_get(%u %u %u %u)\n", sbpm_id,
                plen, sop_offset, calc);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (plen != gtmv(m, 14) || sop_offset != gtmv(m, 10) || calc != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t first_bn = gtmv(m, 14);
        uint8_t num_packet = gtmv(m, 7);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg2_set(%u %u %u)\n", sbpm_id,
            first_bn, num_packet);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg2_set(sbpm_id, first_bn, num_packet);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg2_get(sbpm_id, &first_bn, &num_packet);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg2_get(%u %u %u)\n", sbpm_id,
                first_bn, num_packet);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (first_bn != gtmv(m, 14) || num_packet != gtmv(m, 7))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_rply regs_bn_free_without_contxt_rply = {.free_ack = gtmv(m, 1), .ack_stat = gtmv(m, 1), .nack_stat = gtmv(m, 1), .excl_high_stat = gtmv(m, 1), .excl_med_stat = gtmv(m, 1), .excl_low_stat = gtmv(m, 1), .bsy = gtmv(m, 1), .rdy = gtmv(m, 1)};
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_rply_get(sbpm_id, &regs_bn_free_without_contxt_rply);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_rply_get(%u %u %u %u %u %u %u %u %u)\n", sbpm_id,
                regs_bn_free_without_contxt_rply.free_ack, regs_bn_free_without_contxt_rply.ack_stat, regs_bn_free_without_contxt_rply.nack_stat, regs_bn_free_without_contxt_rply.excl_high_stat, 
                regs_bn_free_without_contxt_rply.excl_med_stat, regs_bn_free_without_contxt_rply.excl_low_stat, regs_bn_free_without_contxt_rply.bsy, regs_bn_free_without_contxt_rply.rdy);
        }
    }

    {
        uint16_t gl_bat = gtmv(m, 15);
        uint16_t gl_bah = gtmv(m, 15);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_gl_trsh_set(%u %u %u)\n", sbpm_id,
            gl_bat, gl_bah);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_gl_trsh_set(sbpm_id, gl_bat, gl_bah);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_gl_trsh_get(sbpm_id, &gl_bat, &gl_bah);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_gl_trsh_get(%u %u %u)\n", sbpm_id,
                gl_bat, gl_bah);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (gl_bat != gtmv(m, 15) || gl_bah != gtmv(m, 15))
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
        uint16_t window_start = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_dbg_set(%u %u %u)\n", sbpm_id,
            select_bus, window_start);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_dbg_set(sbpm_id, select_bus, window_start);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_dbg_get(sbpm_id, &select_bus, &window_start);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_dbg_get(%u %u %u)\n", sbpm_id,
                select_bus, window_start);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (select_bus != gtmv(m, 4) || window_start != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t val = gtmv(m, 15);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_gl_bac_get(sbpm_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_gl_bac_get(%u %u)\n", sbpm_id,
                val);
        }
    }

    {
        uint16_t exclt = gtmv(m, 15);
        uint16_t exclh = gtmv(m, 15);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_high_trsh_set(%u %u %u)\n", sbpm_id,
            exclt, exclh);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_high_trsh_set(sbpm_id, exclt, exclh);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_high_trsh_get(sbpm_id, &exclt, &exclh);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_high_trsh_get(%u %u %u)\n", sbpm_id,
                exclt, exclh);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (exclt != gtmv(m, 15) || exclh != gtmv(m, 15))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t exclt = gtmv(m, 15);
        uint16_t exclh = gtmv(m, 15);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_low_trsh_set(%u %u %u)\n", sbpm_id,
            exclt, exclh);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_low_trsh_set(sbpm_id, exclt, exclh);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_low_trsh_get(sbpm_id, &exclt, &exclh);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_low_trsh_get(%u %u %u)\n", sbpm_id,
                exclt, exclh);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (exclt != gtmv(m, 15) || exclh != gtmv(m, 15))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t exclt = gtmv(m, 15);
        uint16_t exclh = gtmv(m, 15);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_med_trsh_set(%u %u %u)\n", sbpm_id,
            exclt, exclh);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_med_trsh_set(sbpm_id, exclt, exclh);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_med_trsh_get(sbpm_id, &exclt, &exclh);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_med_trsh_get(%u %u %u)\n", sbpm_id,
                exclt, exclh);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (exclt != gtmv(m, 15) || exclh != gtmv(m, 15))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean ack_stts = gtmv(m, 1);
        bdmf_boolean excl_high_stts = gtmv(m, 1);
        bdmf_boolean excl_med_stts = gtmv(m, 1);
        bdmf_boolean excl_low_stts = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_gl_status_get(sbpm_id, &ack_stts, &excl_high_stts, &excl_med_stts, &excl_low_stts);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_gl_status_get(%u %u %u %u %u)\n", sbpm_id,
                ack_stts, excl_high_stts, excl_med_stts, excl_low_stts);
        }
    }

    {
        uint16_t cmd_sa = gtmv(m, 9);
        uint8_t cmd_ta = gtmv(m, 3);
        uint16_t bn_src = gtmv(m, 14);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_iir_addr_get(sbpm_id, &cmd_sa, &cmd_ta, &bn_src);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_iir_addr_get(%u %u %u %u)\n", sbpm_id,
                cmd_sa, cmd_ta, bn_src);
        }
    }

    {
        uint32_t cmd_data_0to31 = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_iir_low_get(sbpm_id, &cmd_data_0to31);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_iir_low_get(%u %u)\n", sbpm_id,
                cmd_data_0to31);
        }
    }

    {
        uint32_t cmd_data_32to63 = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_iir_high_get(sbpm_id, &cmd_data_32to63);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_iir_high_get(%u %u)\n", sbpm_id,
                cmd_data_32to63);
        }
    }

    {
        uint32_t sbpm_fifo_empty = gtmv(m, 18);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_dbg_vec0_get(sbpm_id, &sbpm_fifo_empty);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_dbg_vec0_get(%u %u)\n", sbpm_id,
                sbpm_fifo_empty);
        }
    }

    {
        uint32_t sbpm_fifo_full = gtmv(m, 18);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_dbg_vec1_get(sbpm_id, &sbpm_fifo_full);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_dbg_vec1_get(%u %u)\n", sbpm_id,
                sbpm_fifo_full);
        }
    }

    {
        uint32_t overwr_ra = gtmv(m, 25);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg1_set(%u %u)\n", sbpm_id,
            overwr_ra);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg1_set(sbpm_id, overwr_ra);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg1_get(sbpm_id, &overwr_ra);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg1_get(%u %u)\n", sbpm_id,
                overwr_ra);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (overwr_ra != gtmv(m, 25))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t id_2overwr = gtmv(m, 9);
        bdmf_boolean overwr_valid = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg2_set(%u %u %u)\n", sbpm_id,
            id_2overwr, overwr_valid);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg2_set(sbpm_id, id_2overwr, overwr_valid);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg2_get(sbpm_id, &id_2overwr, &overwr_valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg2_get(%u %u %u)\n", sbpm_id,
                id_2overwr, overwr_valid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (id_2overwr != gtmv(m, 9) || overwr_valid != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t val = gtmv(m, 15);
        bdmf_boolean clr = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_bac_max_set(%u %u %u)\n", sbpm_id,
            val, clr);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bac_max_set(sbpm_id, val, clr);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_bac_max_get(sbpm_id, &val, &clr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_bac_max_get(%u %u %u)\n", sbpm_id,
                val, clr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 15) || clr != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t spr = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_spare_set(%u %u)\n", sbpm_id,
            spr);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_spare_set(sbpm_id, spr);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_spare_get(sbpm_id, &spr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_spare_get(%u %u)\n", sbpm_id,
                spr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (spr != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t size = gtmv(m, 2);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_cfg_bn_size_set(%u %u)\n", sbpm_id,
            size);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_cfg_bn_size_set(sbpm_id, size);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_cfg_bn_size_get(sbpm_id, &size);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_cfg_bn_size_get(%u %u)\n", sbpm_id,
                size);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (size != gtmv(m, 2))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t val = gtmv(m, 18);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_ll_weight_arb_set(%u %u)\n", sbpm_id,
            val);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_ll_weight_arb_set(sbpm_id, val);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_ll_weight_arb_get(sbpm_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_ll_weight_arb_get(%u %u)\n", sbpm_id,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 18))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t start = gtmv(m, 9);
        uint16_t end = gtmv(m, 9);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_sp_rnr_g0_set(%u %u %u)\n", sbpm_id,
            start, end);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sp_rnr_g0_set(sbpm_id, start, end);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sp_rnr_g0_get(sbpm_id, &start, &end);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_sp_rnr_g0_get(%u %u %u)\n", sbpm_id,
                start, end);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (start != gtmv(m, 9) || end != gtmv(m, 9))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t start = gtmv(m, 9);
        uint16_t end = gtmv(m, 9);
        bdmf_boolean valid = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_sp_rnr_g1_set(%u %u %u %u)\n", sbpm_id,
            start, end, valid);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sp_rnr_g1_set(sbpm_id, start, end, valid);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_sp_rnr_g1_get(sbpm_id, &start, &end, &valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_sp_rnr_g1_get(%u %u %u %u)\n", sbpm_id,
                start, end, valid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (start != gtmv(m, 9) || end != gtmv(m, 9) || valid != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t nack_mask = gtmv(m, 9);
        bdmf_boolean valid = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_nack_mask_get(sbpm_id, &nack_mask, &valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_nack_mask_get(%u %u %u)\n", sbpm_id,
                nack_mask, valid);
        }
    }

    {
        uint16_t excl_mask = gtmv(m, 9);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_excl_mask_set(%u %u)\n", sbpm_id,
            excl_mask);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_excl_mask_set(sbpm_id, excl_mask);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_regs_excl_mask_get(sbpm_id, &excl_mask);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_regs_excl_mask_get(%u %u)\n", sbpm_id,
                excl_mask);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (excl_mask != gtmv(m, 9))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        crdp_ds_cmn_sbpm_intr_ctrl_isr intr_ctrl_isr = {.free_list_non_alloc_bn = gtmv(m, 1), .get2free_non_alloc_bn = gtmv(m, 1), .mc2free_non_alloc_bn = gtmv(m, 1), .alloc_alrdy_alloc_bn = gtmv(m, 1), .get_non_alloc_bn = gtmv(m, 1), .get_res_non_alloc_bn = gtmv(m, 1), .connect_non_alloc_bn = gtmv(m, 1), .connect_to_non_alloc_pointed_bn = gtmv(m, 1), .mc_inc_non_alloc_bn = gtmv(m, 1), .free_wo_context_comb = gtmv(m, 6), .bac_underrun = gtmv(m, 1), .mcst_overflow = gtmv(m, 1), .empty_free_list = gtmv(m, 1), .error_bb_cmd_msg = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_isr_set(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", sbpm_id,
            intr_ctrl_isr.free_list_non_alloc_bn, intr_ctrl_isr.get2free_non_alloc_bn, intr_ctrl_isr.mc2free_non_alloc_bn, intr_ctrl_isr.alloc_alrdy_alloc_bn, 
            intr_ctrl_isr.get_non_alloc_bn, intr_ctrl_isr.get_res_non_alloc_bn, intr_ctrl_isr.connect_non_alloc_bn, intr_ctrl_isr.connect_to_non_alloc_pointed_bn, 
            intr_ctrl_isr.mc_inc_non_alloc_bn, intr_ctrl_isr.free_wo_context_comb, intr_ctrl_isr.bac_underrun, intr_ctrl_isr.mcst_overflow, 
            intr_ctrl_isr.empty_free_list, intr_ctrl_isr.error_bb_cmd_msg);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_isr_set(sbpm_id, &intr_ctrl_isr);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_isr_get(sbpm_id, &intr_ctrl_isr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_isr_get(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", sbpm_id,
                intr_ctrl_isr.free_list_non_alloc_bn, intr_ctrl_isr.get2free_non_alloc_bn, intr_ctrl_isr.mc2free_non_alloc_bn, intr_ctrl_isr.alloc_alrdy_alloc_bn, 
                intr_ctrl_isr.get_non_alloc_bn, intr_ctrl_isr.get_res_non_alloc_bn, intr_ctrl_isr.connect_non_alloc_bn, intr_ctrl_isr.connect_to_non_alloc_pointed_bn, 
                intr_ctrl_isr.mc_inc_non_alloc_bn, intr_ctrl_isr.free_wo_context_comb, intr_ctrl_isr.bac_underrun, intr_ctrl_isr.mcst_overflow, 
                intr_ctrl_isr.empty_free_list, intr_ctrl_isr.error_bb_cmd_msg);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (intr_ctrl_isr.free_list_non_alloc_bn != gtmv(m, 1) || intr_ctrl_isr.get2free_non_alloc_bn != gtmv(m, 1) || intr_ctrl_isr.mc2free_non_alloc_bn != gtmv(m, 1) || intr_ctrl_isr.alloc_alrdy_alloc_bn != gtmv(m, 1) || intr_ctrl_isr.get_non_alloc_bn != gtmv(m, 1) || intr_ctrl_isr.get_res_non_alloc_bn != gtmv(m, 1) || intr_ctrl_isr.connect_non_alloc_bn != gtmv(m, 1) || intr_ctrl_isr.connect_to_non_alloc_pointed_bn != gtmv(m, 1) || intr_ctrl_isr.mc_inc_non_alloc_bn != gtmv(m, 1) || intr_ctrl_isr.free_wo_context_comb != gtmv(m, 6) || intr_ctrl_isr.bac_underrun != gtmv(m, 1) || intr_ctrl_isr.mcst_overflow != gtmv(m, 1) || intr_ctrl_isr.empty_free_list != gtmv(m, 1) || intr_ctrl_isr.error_bb_cmd_msg != gtmv(m, 1))
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
            ag_err = ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_ism_get(sbpm_id, &ism);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_ism_get(%u %u)\n", sbpm_id,
                ism);
        }
    }

    {
        uint32_t iem = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_ier_set(%u %u)\n", sbpm_id,
            iem);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_ier_set(sbpm_id, iem);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_ier_get(sbpm_id, &iem);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_ier_get(%u %u)\n", sbpm_id,
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
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_itr_set(%u %u)\n", sbpm_id,
            ist);
        ag_err = ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_itr_set(sbpm_id, ist);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_itr_get(sbpm_id, &ist);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_itr_get(%u %u)\n", sbpm_id,
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
static int ag_drv_crdp_ds_cmn_sbpm_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_regs_init_free_list: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_INIT_FREE_LIST); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_bn_alloc: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_BN_ALLOC); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_bn_alloc_rply: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_BN_ALLOC_RPLY); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_mcst_inc: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_MCST_INC); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_mcst_inc_rply: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_MCST_INC_RPLY); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_bn_connect: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_BN_CONNECT); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_bn_connect_rply: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_BN_CONNECT_RPLY); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_get_next: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_GET_NEXT); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_get_next_rply: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_GET_NEXT_RPLY); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_sbpm_clk_gate_cntrl: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_SBPM_CLK_GATE_CNTRL); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_bn_free_without_contxt_reg1: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_REG1); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_bn_free_without_contxt_reg2: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_REG2); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_bn_free_without_contxt_rply: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_BN_FREE_WITHOUT_CONTXT_RPLY); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_gl_trsh: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_GL_TRSH); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_sbpm_dbg: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_SBPM_DBG); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_gl_bac: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_GL_BAC); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_gl_excl_high_trsh: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_GL_EXCL_HIGH_TRSH); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_gl_excl_low_trsh: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_GL_EXCL_LOW_TRSH); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_gl_excl_med_trsh: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_GL_EXCL_MED_TRSH); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_sbpm_gl_status: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_SBPM_GL_STATUS); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_iir_addr: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_IIR_ADDR); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_sbpm_iir_low: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_SBPM_IIR_LOW); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_sbpm_iir_high: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_SBPM_IIR_HIGH); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_sbpm_dbg_vec0: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_SBPM_DBG_VEC0); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_sbpm_dbg_vec1: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_SBPM_DBG_VEC1); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_sbpm_raddr_decoder_reg1: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_SBPM_RADDR_DECODER_REG1); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_sbpm_raddr_decoder_reg2: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_SBPM_RADDR_DECODER_REG2); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_bac_max: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_BAC_MAX); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_sbpm_spare: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_SBPM_SPARE); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_cfg_bn_size: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_CFG_BN_SIZE); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_ll_weight_arb: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_LL_WEIGHT_ARB); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_sp_rnr_g0: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_SP_RNR_G0); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_sp_rnr_g1: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_SP_RNR_G1); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_nack_mask: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_NACK_MASK); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_regs_excl_mask: reg = &RU_REG(CRDP_DS_CMN_SBPM, REGS_EXCL_MASK); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_intr_ctrl_isr: reg = &RU_REG(CRDP_DS_CMN_SBPM, INTR_CTRL_ISR); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_intr_ctrl_ism: reg = &RU_REG(CRDP_DS_CMN_SBPM, INTR_CTRL_ISM); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_intr_ctrl_ier: reg = &RU_REG(CRDP_DS_CMN_SBPM, INTR_CTRL_IER); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
    case bdmf_address_intr_ctrl_itr: reg = &RU_REG(CRDP_DS_CMN_SBPM, INTR_CTRL_ITR); blk = &RU_BLK(CRDP_DS_CMN_SBPM); break;
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

bdmfmon_handle_t ag_drv_crdp_ds_cmn_sbpm_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "crdp_ds_cmn_sbpm", "crdp_ds_cmn_sbpm", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_regs_init_free_list[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("init_max_bn", "init_max_bn", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_init_start", "ram_init_start", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bsy", "bsy", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rdy", "rdy", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_bn_alloc[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("req", "req", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_mcst_inc[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bn", "bn", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mcst_val", "mcst_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ack_req", "ack_req", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_bn_connect[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bn", "bn", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ack_req", "ack_req", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pointed_bn", "pointed_bn", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_get_next[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bn", "bn", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_clk_gate_cntrl[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bypass_clk_gate", "bypass_clk_gate", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("timer_val", "timer_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_en", "keep_alive_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_intervl", "keep_alive_intervl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_cyc", "keep_alive_cyc", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_bn_free_without_contxt_reg1[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("plen", "plen", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sop_offset", "sop_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("calc", "calc", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_bn_free_without_contxt_reg2[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("first_bn", "first_bn", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("num_packet", "num_packet", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_gl_trsh[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gl_bat", "gl_bat", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gl_bah", "gl_bah", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_dbg[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("select_bus", "select_bus", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("window_start", "window_start", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_gl_excl_high_trsh[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("exclt", "exclt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("exclh", "exclh", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_gl_excl_low_trsh[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("exclt", "exclt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("exclh", "exclh", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_gl_excl_med_trsh[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("exclt", "exclt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("exclh", "exclh", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_raddr_decoder_reg1[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("overwr_ra", "overwr_ra", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_raddr_decoder_reg2[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("id_2overwr", "id_2overwr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("overwr_valid", "overwr_valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_bac_max[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("clr", "clr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sbpm_spare[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("spr", "spr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_cfg_bn_size[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("size", "size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_ll_weight_arb[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sp_rnr_g0[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("start", "start", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("end", "end", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_sp_rnr_g1[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("start", "start", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("end", "end", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("valid", "valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regs_excl_mask[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("excl_mask", "excl_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_intr_ctrl_isr[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("free_list_non_alloc_bn", "free_list_non_alloc_bn", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("get2free_non_alloc_bn", "get2free_non_alloc_bn", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mc2free_non_alloc_bn", "mc2free_non_alloc_bn", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("alloc_alrdy_alloc_bn", "alloc_alrdy_alloc_bn", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("get_non_alloc_bn", "get_non_alloc_bn", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("get_res_non_alloc_bn", "get_res_non_alloc_bn", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("connect_non_alloc_bn", "connect_non_alloc_bn", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("connect_to_non_alloc_pointed_bn", "connect_to_non_alloc_pointed_bn", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mc_inc_non_alloc_bn", "mc_inc_non_alloc_bn", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("free_wo_context_comb", "free_wo_context_comb", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bac_underrun", "bac_underrun", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mcst_overflow", "mcst_overflow", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("empty_free_list", "empty_free_list", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("error_bb_cmd_msg", "error_bb_cmd_msg", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_intr_ctrl_ier[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("iem", "iem", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_intr_ctrl_itr[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ist", "ist", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "regs_init_free_list", .val = cli_crdp_ds_cmn_sbpm_regs_init_free_list, .parms = set_regs_init_free_list },
            { .name = "regs_bn_alloc", .val = cli_crdp_ds_cmn_sbpm_regs_bn_alloc, .parms = set_regs_bn_alloc },
            { .name = "regs_mcst_inc", .val = cli_crdp_ds_cmn_sbpm_regs_mcst_inc, .parms = set_regs_mcst_inc },
            { .name = "regs_bn_connect", .val = cli_crdp_ds_cmn_sbpm_regs_bn_connect, .parms = set_regs_bn_connect },
            { .name = "regs_get_next", .val = cli_crdp_ds_cmn_sbpm_regs_get_next, .parms = set_regs_get_next },
            { .name = "regs_sbpm_clk_gate_cntrl", .val = cli_crdp_ds_cmn_sbpm_regs_sbpm_clk_gate_cntrl, .parms = set_regs_sbpm_clk_gate_cntrl },
            { .name = "regs_bn_free_without_contxt_reg1", .val = cli_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg1, .parms = set_regs_bn_free_without_contxt_reg1 },
            { .name = "regs_bn_free_without_contxt_reg2", .val = cli_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg2, .parms = set_regs_bn_free_without_contxt_reg2 },
            { .name = "regs_gl_trsh", .val = cli_crdp_ds_cmn_sbpm_regs_gl_trsh, .parms = set_regs_gl_trsh },
            { .name = "regs_sbpm_dbg", .val = cli_crdp_ds_cmn_sbpm_regs_sbpm_dbg, .parms = set_regs_sbpm_dbg },
            { .name = "regs_gl_excl_high_trsh", .val = cli_crdp_ds_cmn_sbpm_regs_gl_excl_high_trsh, .parms = set_regs_gl_excl_high_trsh },
            { .name = "regs_gl_excl_low_trsh", .val = cli_crdp_ds_cmn_sbpm_regs_gl_excl_low_trsh, .parms = set_regs_gl_excl_low_trsh },
            { .name = "regs_gl_excl_med_trsh", .val = cli_crdp_ds_cmn_sbpm_regs_gl_excl_med_trsh, .parms = set_regs_gl_excl_med_trsh },
            { .name = "regs_sbpm_raddr_decoder_reg1", .val = cli_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg1, .parms = set_regs_sbpm_raddr_decoder_reg1 },
            { .name = "regs_sbpm_raddr_decoder_reg2", .val = cli_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg2, .parms = set_regs_sbpm_raddr_decoder_reg2 },
            { .name = "regs_bac_max", .val = cli_crdp_ds_cmn_sbpm_regs_bac_max, .parms = set_regs_bac_max },
            { .name = "regs_sbpm_spare", .val = cli_crdp_ds_cmn_sbpm_regs_sbpm_spare, .parms = set_regs_sbpm_spare },
            { .name = "regs_cfg_bn_size", .val = cli_crdp_ds_cmn_sbpm_regs_cfg_bn_size, .parms = set_regs_cfg_bn_size },
            { .name = "regs_ll_weight_arb", .val = cli_crdp_ds_cmn_sbpm_regs_ll_weight_arb, .parms = set_regs_ll_weight_arb },
            { .name = "regs_sp_rnr_g0", .val = cli_crdp_ds_cmn_sbpm_regs_sp_rnr_g0, .parms = set_regs_sp_rnr_g0 },
            { .name = "regs_sp_rnr_g1", .val = cli_crdp_ds_cmn_sbpm_regs_sp_rnr_g1, .parms = set_regs_sp_rnr_g1 },
            { .name = "regs_excl_mask", .val = cli_crdp_ds_cmn_sbpm_regs_excl_mask, .parms = set_regs_excl_mask },
            { .name = "intr_ctrl_isr", .val = cli_crdp_ds_cmn_sbpm_intr_ctrl_isr, .parms = set_intr_ctrl_isr },
            { .name = "intr_ctrl_ier", .val = cli_crdp_ds_cmn_sbpm_intr_ctrl_ier, .parms = set_intr_ctrl_ier },
            { .name = "intr_ctrl_itr", .val = cli_crdp_ds_cmn_sbpm_intr_ctrl_itr, .parms = set_intr_ctrl_itr },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_crdp_ds_cmn_sbpm_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "regs_init_free_list", .val = cli_crdp_ds_cmn_sbpm_regs_init_free_list, .parms = get_default },
            { .name = "regs_bn_alloc", .val = cli_crdp_ds_cmn_sbpm_regs_bn_alloc, .parms = get_default },
            { .name = "regs_bn_alloc_rply", .val = cli_crdp_ds_cmn_sbpm_regs_bn_alloc_rply, .parms = get_default },
            { .name = "regs_mcst_inc", .val = cli_crdp_ds_cmn_sbpm_regs_mcst_inc, .parms = get_default },
            { .name = "regs_mcst_inc_rply", .val = cli_crdp_ds_cmn_sbpm_regs_mcst_inc_rply, .parms = get_default },
            { .name = "regs_bn_connect", .val = cli_crdp_ds_cmn_sbpm_regs_bn_connect, .parms = get_default },
            { .name = "regs_bn_connect_rply", .val = cli_crdp_ds_cmn_sbpm_regs_bn_connect_rply, .parms = get_default },
            { .name = "regs_get_next", .val = cli_crdp_ds_cmn_sbpm_regs_get_next, .parms = get_default },
            { .name = "regs_get_next_rply", .val = cli_crdp_ds_cmn_sbpm_regs_get_next_rply, .parms = get_default },
            { .name = "regs_sbpm_clk_gate_cntrl", .val = cli_crdp_ds_cmn_sbpm_regs_sbpm_clk_gate_cntrl, .parms = get_default },
            { .name = "regs_bn_free_without_contxt_reg1", .val = cli_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg1, .parms = get_default },
            { .name = "regs_bn_free_without_contxt_reg2", .val = cli_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg2, .parms = get_default },
            { .name = "regs_bn_free_without_contxt_rply", .val = cli_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_rply, .parms = get_default },
            { .name = "regs_gl_trsh", .val = cli_crdp_ds_cmn_sbpm_regs_gl_trsh, .parms = get_default },
            { .name = "regs_sbpm_dbg", .val = cli_crdp_ds_cmn_sbpm_regs_sbpm_dbg, .parms = get_default },
            { .name = "regs_gl_bac", .val = cli_crdp_ds_cmn_sbpm_regs_gl_bac, .parms = get_default },
            { .name = "regs_gl_excl_high_trsh", .val = cli_crdp_ds_cmn_sbpm_regs_gl_excl_high_trsh, .parms = get_default },
            { .name = "regs_gl_excl_low_trsh", .val = cli_crdp_ds_cmn_sbpm_regs_gl_excl_low_trsh, .parms = get_default },
            { .name = "regs_gl_excl_med_trsh", .val = cli_crdp_ds_cmn_sbpm_regs_gl_excl_med_trsh, .parms = get_default },
            { .name = "regs_sbpm_gl_status", .val = cli_crdp_ds_cmn_sbpm_regs_sbpm_gl_status, .parms = get_default },
            { .name = "regs_iir_addr", .val = cli_crdp_ds_cmn_sbpm_regs_iir_addr, .parms = get_default },
            { .name = "regs_sbpm_iir_low", .val = cli_crdp_ds_cmn_sbpm_regs_sbpm_iir_low, .parms = get_default },
            { .name = "regs_sbpm_iir_high", .val = cli_crdp_ds_cmn_sbpm_regs_sbpm_iir_high, .parms = get_default },
            { .name = "regs_sbpm_dbg_vec0", .val = cli_crdp_ds_cmn_sbpm_regs_sbpm_dbg_vec0, .parms = get_default },
            { .name = "regs_sbpm_dbg_vec1", .val = cli_crdp_ds_cmn_sbpm_regs_sbpm_dbg_vec1, .parms = get_default },
            { .name = "regs_sbpm_raddr_decoder_reg1", .val = cli_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg1, .parms = get_default },
            { .name = "regs_sbpm_raddr_decoder_reg2", .val = cli_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg2, .parms = get_default },
            { .name = "regs_bac_max", .val = cli_crdp_ds_cmn_sbpm_regs_bac_max, .parms = get_default },
            { .name = "regs_sbpm_spare", .val = cli_crdp_ds_cmn_sbpm_regs_sbpm_spare, .parms = get_default },
            { .name = "regs_cfg_bn_size", .val = cli_crdp_ds_cmn_sbpm_regs_cfg_bn_size, .parms = get_default },
            { .name = "regs_ll_weight_arb", .val = cli_crdp_ds_cmn_sbpm_regs_ll_weight_arb, .parms = get_default },
            { .name = "regs_sp_rnr_g0", .val = cli_crdp_ds_cmn_sbpm_regs_sp_rnr_g0, .parms = get_default },
            { .name = "regs_sp_rnr_g1", .val = cli_crdp_ds_cmn_sbpm_regs_sp_rnr_g1, .parms = get_default },
            { .name = "regs_nack_mask", .val = cli_crdp_ds_cmn_sbpm_regs_nack_mask, .parms = get_default },
            { .name = "regs_excl_mask", .val = cli_crdp_ds_cmn_sbpm_regs_excl_mask, .parms = get_default },
            { .name = "intr_ctrl_isr", .val = cli_crdp_ds_cmn_sbpm_intr_ctrl_isr, .parms = get_default },
            { .name = "intr_ctrl_ism", .val = cli_crdp_ds_cmn_sbpm_intr_ctrl_ism, .parms = get_default },
            { .name = "intr_ctrl_ier", .val = cli_crdp_ds_cmn_sbpm_intr_ctrl_ier, .parms = get_default },
            { .name = "intr_ctrl_itr", .val = cli_crdp_ds_cmn_sbpm_intr_ctrl_itr, .parms = get_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_crdp_ds_cmn_sbpm_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_crdp_ds_cmn_sbpm_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0)            BDMFMON_MAKE_PARM("sbpm_id", "sbpm_id", BDMFMON_PARM_UNUMBER, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "REGS_INIT_FREE_LIST", .val = bdmf_address_regs_init_free_list },
            { .name = "REGS_BN_ALLOC", .val = bdmf_address_regs_bn_alloc },
            { .name = "REGS_BN_ALLOC_RPLY", .val = bdmf_address_regs_bn_alloc_rply },
            { .name = "REGS_MCST_INC", .val = bdmf_address_regs_mcst_inc },
            { .name = "REGS_MCST_INC_RPLY", .val = bdmf_address_regs_mcst_inc_rply },
            { .name = "REGS_BN_CONNECT", .val = bdmf_address_regs_bn_connect },
            { .name = "REGS_BN_CONNECT_RPLY", .val = bdmf_address_regs_bn_connect_rply },
            { .name = "REGS_GET_NEXT", .val = bdmf_address_regs_get_next },
            { .name = "REGS_GET_NEXT_RPLY", .val = bdmf_address_regs_get_next_rply },
            { .name = "REGS_SBPM_CLK_GATE_CNTRL", .val = bdmf_address_regs_sbpm_clk_gate_cntrl },
            { .name = "REGS_BN_FREE_WITHOUT_CONTXT_REG1", .val = bdmf_address_regs_bn_free_without_contxt_reg1 },
            { .name = "REGS_BN_FREE_WITHOUT_CONTXT_REG2", .val = bdmf_address_regs_bn_free_without_contxt_reg2 },
            { .name = "REGS_BN_FREE_WITHOUT_CONTXT_RPLY", .val = bdmf_address_regs_bn_free_without_contxt_rply },
            { .name = "REGS_GL_TRSH", .val = bdmf_address_regs_gl_trsh },
            { .name = "REGS_SBPM_DBG", .val = bdmf_address_regs_sbpm_dbg },
            { .name = "REGS_GL_BAC", .val = bdmf_address_regs_gl_bac },
            { .name = "REGS_GL_EXCL_HIGH_TRSH", .val = bdmf_address_regs_gl_excl_high_trsh },
            { .name = "REGS_GL_EXCL_LOW_TRSH", .val = bdmf_address_regs_gl_excl_low_trsh },
            { .name = "REGS_GL_EXCL_MED_TRSH", .val = bdmf_address_regs_gl_excl_med_trsh },
            { .name = "REGS_SBPM_GL_STATUS", .val = bdmf_address_regs_sbpm_gl_status },
            { .name = "REGS_IIR_ADDR", .val = bdmf_address_regs_iir_addr },
            { .name = "REGS_SBPM_IIR_LOW", .val = bdmf_address_regs_sbpm_iir_low },
            { .name = "REGS_SBPM_IIR_HIGH", .val = bdmf_address_regs_sbpm_iir_high },
            { .name = "REGS_SBPM_DBG_VEC0", .val = bdmf_address_regs_sbpm_dbg_vec0 },
            { .name = "REGS_SBPM_DBG_VEC1", .val = bdmf_address_regs_sbpm_dbg_vec1 },
            { .name = "REGS_SBPM_RADDR_DECODER_REG1", .val = bdmf_address_regs_sbpm_raddr_decoder_reg1 },
            { .name = "REGS_SBPM_RADDR_DECODER_REG2", .val = bdmf_address_regs_sbpm_raddr_decoder_reg2 },
            { .name = "REGS_BAC_MAX", .val = bdmf_address_regs_bac_max },
            { .name = "REGS_SBPM_SPARE", .val = bdmf_address_regs_sbpm_spare },
            { .name = "REGS_CFG_BN_SIZE", .val = bdmf_address_regs_cfg_bn_size },
            { .name = "REGS_LL_WEIGHT_ARB", .val = bdmf_address_regs_ll_weight_arb },
            { .name = "REGS_SP_RNR_G0", .val = bdmf_address_regs_sp_rnr_g0 },
            { .name = "REGS_SP_RNR_G1", .val = bdmf_address_regs_sp_rnr_g1 },
            { .name = "REGS_NACK_MASK", .val = bdmf_address_regs_nack_mask },
            { .name = "REGS_EXCL_MASK", .val = bdmf_address_regs_excl_mask },
            { .name = "INTR_CTRL_ISR", .val = bdmf_address_intr_ctrl_isr },
            { .name = "INTR_CTRL_ISM", .val = bdmf_address_intr_ctrl_ism },
            { .name = "INTR_CTRL_IER", .val = bdmf_address_intr_ctrl_ier },
            { .name = "INTR_CTRL_ITR", .val = bdmf_address_intr_ctrl_itr },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_crdp_ds_cmn_sbpm_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index1", "sbpm_id", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
