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
#include "xrdp_drv_natc_ddr_cfg_ag.h"

#define BLOCK_ADDR_COUNT_BITS 0
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_natc_ddr_cfg_natc_ddr_size_set(const natc_ddr_cfg_natc_ddr_size *natc_ddr_size)
{
    uint32_t reg_ddr_size = 0;

#ifdef VALIDATE_PARMS
    if(!natc_ddr_size)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((natc_ddr_size->ddr_size_tbl0 >= _3BITS_MAX_VAL_) ||
       (natc_ddr_size->ddr_size_tbl1 >= _3BITS_MAX_VAL_) ||
       (natc_ddr_size->ddr_size_tbl2 >= _3BITS_MAX_VAL_) ||
       (natc_ddr_size->ddr_size_tbl3 >= _3BITS_MAX_VAL_) ||
       (natc_ddr_size->ddr_size_tbl4 >= _3BITS_MAX_VAL_) ||
       (natc_ddr_size->ddr_size_tbl5 >= _3BITS_MAX_VAL_) ||
       (natc_ddr_size->ddr_size_tbl6 >= _3BITS_MAX_VAL_) ||
       (natc_ddr_size->ddr_size_tbl7 >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_ddr_size = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_SIZE, DDR_SIZE_TBL0, reg_ddr_size, natc_ddr_size->ddr_size_tbl0);
    reg_ddr_size = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_SIZE, DDR_SIZE_TBL1, reg_ddr_size, natc_ddr_size->ddr_size_tbl1);
    reg_ddr_size = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_SIZE, DDR_SIZE_TBL2, reg_ddr_size, natc_ddr_size->ddr_size_tbl2);
    reg_ddr_size = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_SIZE, DDR_SIZE_TBL3, reg_ddr_size, natc_ddr_size->ddr_size_tbl3);
    reg_ddr_size = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_SIZE, DDR_SIZE_TBL4, reg_ddr_size, natc_ddr_size->ddr_size_tbl4);
    reg_ddr_size = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_SIZE, DDR_SIZE_TBL5, reg_ddr_size, natc_ddr_size->ddr_size_tbl5);
    reg_ddr_size = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_SIZE, DDR_SIZE_TBL6, reg_ddr_size, natc_ddr_size->ddr_size_tbl6);
    reg_ddr_size = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_SIZE, DDR_SIZE_TBL7, reg_ddr_size, natc_ddr_size->ddr_size_tbl7);

    RU_REG_WRITE(0, NATC_DDR_CFG, DDR_SIZE, reg_ddr_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ddr_cfg_natc_ddr_size_get(natc_ddr_cfg_natc_ddr_size *natc_ddr_size)
{
    uint32_t reg_ddr_size;

#ifdef VALIDATE_PARMS
    if (!natc_ddr_size)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC_DDR_CFG, DDR_SIZE, reg_ddr_size);

    natc_ddr_size->ddr_size_tbl0 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_SIZE, DDR_SIZE_TBL0, reg_ddr_size);
    natc_ddr_size->ddr_size_tbl1 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_SIZE, DDR_SIZE_TBL1, reg_ddr_size);
    natc_ddr_size->ddr_size_tbl2 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_SIZE, DDR_SIZE_TBL2, reg_ddr_size);
    natc_ddr_size->ddr_size_tbl3 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_SIZE, DDR_SIZE_TBL3, reg_ddr_size);
    natc_ddr_size->ddr_size_tbl4 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_SIZE, DDR_SIZE_TBL4, reg_ddr_size);
    natc_ddr_size->ddr_size_tbl5 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_SIZE, DDR_SIZE_TBL5, reg_ddr_size);
    natc_ddr_size->ddr_size_tbl6 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_SIZE, DDR_SIZE_TBL6, reg_ddr_size);
    natc_ddr_size->ddr_size_tbl7 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_SIZE, DDR_SIZE_TBL7, reg_ddr_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_0_set(uint8_t ddr_bins_per_bucket_tbl0, uint8_t ddr_bins_per_bucket_tbl1, uint8_t ddr_bins_per_bucket_tbl2, uint8_t ddr_bins_per_bucket_tbl3)
{
    uint32_t reg_ddr_bins_per_bucket_0 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_ddr_bins_per_bucket_0 = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL0, reg_ddr_bins_per_bucket_0, ddr_bins_per_bucket_tbl0);
    reg_ddr_bins_per_bucket_0 = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL1, reg_ddr_bins_per_bucket_0, ddr_bins_per_bucket_tbl1);
    reg_ddr_bins_per_bucket_0 = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL2, reg_ddr_bins_per_bucket_0, ddr_bins_per_bucket_tbl2);
    reg_ddr_bins_per_bucket_0 = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL3, reg_ddr_bins_per_bucket_0, ddr_bins_per_bucket_tbl3);

    RU_REG_WRITE(0, NATC_DDR_CFG, DDR_BINS_PER_BUCKET_0, reg_ddr_bins_per_bucket_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_0_get(uint8_t *ddr_bins_per_bucket_tbl0, uint8_t *ddr_bins_per_bucket_tbl1, uint8_t *ddr_bins_per_bucket_tbl2, uint8_t *ddr_bins_per_bucket_tbl3)
{
    uint32_t reg_ddr_bins_per_bucket_0;

#ifdef VALIDATE_PARMS
    if (!ddr_bins_per_bucket_tbl0 || !ddr_bins_per_bucket_tbl1 || !ddr_bins_per_bucket_tbl2 || !ddr_bins_per_bucket_tbl3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC_DDR_CFG, DDR_BINS_PER_BUCKET_0, reg_ddr_bins_per_bucket_0);

    *ddr_bins_per_bucket_tbl0 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL0, reg_ddr_bins_per_bucket_0);
    *ddr_bins_per_bucket_tbl1 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL1, reg_ddr_bins_per_bucket_0);
    *ddr_bins_per_bucket_tbl2 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL2, reg_ddr_bins_per_bucket_0);
    *ddr_bins_per_bucket_tbl3 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL3, reg_ddr_bins_per_bucket_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_1_set(uint8_t ddr_bins_per_bucket_tbl4, uint8_t ddr_bins_per_bucket_tbl5, uint8_t ddr_bins_per_bucket_tbl6, uint8_t ddr_bins_per_bucket_tbl7)
{
    uint32_t reg_ddr_bins_per_bucket_1 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_ddr_bins_per_bucket_1 = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL4, reg_ddr_bins_per_bucket_1, ddr_bins_per_bucket_tbl4);
    reg_ddr_bins_per_bucket_1 = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL5, reg_ddr_bins_per_bucket_1, ddr_bins_per_bucket_tbl5);
    reg_ddr_bins_per_bucket_1 = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL6, reg_ddr_bins_per_bucket_1, ddr_bins_per_bucket_tbl6);
    reg_ddr_bins_per_bucket_1 = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL7, reg_ddr_bins_per_bucket_1, ddr_bins_per_bucket_tbl7);

    RU_REG_WRITE(0, NATC_DDR_CFG, DDR_BINS_PER_BUCKET_1, reg_ddr_bins_per_bucket_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_1_get(uint8_t *ddr_bins_per_bucket_tbl4, uint8_t *ddr_bins_per_bucket_tbl5, uint8_t *ddr_bins_per_bucket_tbl6, uint8_t *ddr_bins_per_bucket_tbl7)
{
    uint32_t reg_ddr_bins_per_bucket_1;

#ifdef VALIDATE_PARMS
    if (!ddr_bins_per_bucket_tbl4 || !ddr_bins_per_bucket_tbl5 || !ddr_bins_per_bucket_tbl6 || !ddr_bins_per_bucket_tbl7)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC_DDR_CFG, DDR_BINS_PER_BUCKET_1, reg_ddr_bins_per_bucket_1);

    *ddr_bins_per_bucket_tbl4 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL4, reg_ddr_bins_per_bucket_1);
    *ddr_bins_per_bucket_tbl5 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL5, reg_ddr_bins_per_bucket_1);
    *ddr_bins_per_bucket_tbl6 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL6, reg_ddr_bins_per_bucket_1);
    *ddr_bins_per_bucket_tbl7 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL7, reg_ddr_bins_per_bucket_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ddr_cfg_total_len_set(const natc_ddr_cfg_total_len *total_len)
{
    uint32_t reg_total_len = 0;

#ifdef VALIDATE_PARMS
    if(!total_len)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((total_len->total_len_tbl0 >= _3BITS_MAX_VAL_) ||
       (total_len->total_len_tbl1 >= _3BITS_MAX_VAL_) ||
       (total_len->total_len_tbl2 >= _3BITS_MAX_VAL_) ||
       (total_len->total_len_tbl3 >= _3BITS_MAX_VAL_) ||
       (total_len->total_len_tbl4 >= _3BITS_MAX_VAL_) ||
       (total_len->total_len_tbl5 >= _3BITS_MAX_VAL_) ||
       (total_len->total_len_tbl6 >= _3BITS_MAX_VAL_) ||
       (total_len->total_len_tbl7 >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_total_len = RU_FIELD_SET(0, NATC_DDR_CFG, TOTAL_LEN, TOTAL_LEN_TBL0, reg_total_len, total_len->total_len_tbl0);
    reg_total_len = RU_FIELD_SET(0, NATC_DDR_CFG, TOTAL_LEN, TOTAL_LEN_TBL1, reg_total_len, total_len->total_len_tbl1);
    reg_total_len = RU_FIELD_SET(0, NATC_DDR_CFG, TOTAL_LEN, TOTAL_LEN_TBL2, reg_total_len, total_len->total_len_tbl2);
    reg_total_len = RU_FIELD_SET(0, NATC_DDR_CFG, TOTAL_LEN, TOTAL_LEN_TBL3, reg_total_len, total_len->total_len_tbl3);
    reg_total_len = RU_FIELD_SET(0, NATC_DDR_CFG, TOTAL_LEN, TOTAL_LEN_TBL4, reg_total_len, total_len->total_len_tbl4);
    reg_total_len = RU_FIELD_SET(0, NATC_DDR_CFG, TOTAL_LEN, TOTAL_LEN_TBL5, reg_total_len, total_len->total_len_tbl5);
    reg_total_len = RU_FIELD_SET(0, NATC_DDR_CFG, TOTAL_LEN, TOTAL_LEN_TBL6, reg_total_len, total_len->total_len_tbl6);
    reg_total_len = RU_FIELD_SET(0, NATC_DDR_CFG, TOTAL_LEN, TOTAL_LEN_TBL7, reg_total_len, total_len->total_len_tbl7);

    RU_REG_WRITE(0, NATC_DDR_CFG, TOTAL_LEN, reg_total_len);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ddr_cfg_total_len_get(natc_ddr_cfg_total_len *total_len)
{
    uint32_t reg_total_len;

#ifdef VALIDATE_PARMS
    if (!total_len)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC_DDR_CFG, TOTAL_LEN, reg_total_len);

    total_len->total_len_tbl0 = RU_FIELD_GET(0, NATC_DDR_CFG, TOTAL_LEN, TOTAL_LEN_TBL0, reg_total_len);
    total_len->total_len_tbl1 = RU_FIELD_GET(0, NATC_DDR_CFG, TOTAL_LEN, TOTAL_LEN_TBL1, reg_total_len);
    total_len->total_len_tbl2 = RU_FIELD_GET(0, NATC_DDR_CFG, TOTAL_LEN, TOTAL_LEN_TBL2, reg_total_len);
    total_len->total_len_tbl3 = RU_FIELD_GET(0, NATC_DDR_CFG, TOTAL_LEN, TOTAL_LEN_TBL3, reg_total_len);
    total_len->total_len_tbl4 = RU_FIELD_GET(0, NATC_DDR_CFG, TOTAL_LEN, TOTAL_LEN_TBL4, reg_total_len);
    total_len->total_len_tbl5 = RU_FIELD_GET(0, NATC_DDR_CFG, TOTAL_LEN, TOTAL_LEN_TBL5, reg_total_len);
    total_len->total_len_tbl6 = RU_FIELD_GET(0, NATC_DDR_CFG, TOTAL_LEN, TOTAL_LEN_TBL6, reg_total_len);
    total_len->total_len_tbl7 = RU_FIELD_GET(0, NATC_DDR_CFG, TOTAL_LEN, TOTAL_LEN_TBL7, reg_total_len);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ddr_cfg_sm_status_set(const natc_ddr_cfg_sm_status *sm_status)
{
    uint32_t reg_sm_status = 0;

#ifdef VALIDATE_PARMS
    if(!sm_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((sm_status->nat_state >= _15BITS_MAX_VAL_) ||
       (sm_status->wb_state >= _1BITS_MAX_VAL_) ||
       (sm_status->runner_cmd_state >= _1BITS_MAX_VAL_) ||
       (sm_status->ddr_rep_state >= _3BITS_MAX_VAL_) ||
       (sm_status->ddr_req_state >= _2BITS_MAX_VAL_) ||
       (sm_status->apb_state >= _2BITS_MAX_VAL_) ||
       (sm_status->debug_sel >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_sm_status = RU_FIELD_SET(0, NATC_DDR_CFG, SM_STATUS, NAT_STATE, reg_sm_status, sm_status->nat_state);
    reg_sm_status = RU_FIELD_SET(0, NATC_DDR_CFG, SM_STATUS, WB_STATE, reg_sm_status, sm_status->wb_state);
    reg_sm_status = RU_FIELD_SET(0, NATC_DDR_CFG, SM_STATUS, RUNNER_CMD_STATE, reg_sm_status, sm_status->runner_cmd_state);
    reg_sm_status = RU_FIELD_SET(0, NATC_DDR_CFG, SM_STATUS, DDR_REP_STATE, reg_sm_status, sm_status->ddr_rep_state);
    reg_sm_status = RU_FIELD_SET(0, NATC_DDR_CFG, SM_STATUS, DDR_REQ_STATE, reg_sm_status, sm_status->ddr_req_state);
    reg_sm_status = RU_FIELD_SET(0, NATC_DDR_CFG, SM_STATUS, APB_STATE, reg_sm_status, sm_status->apb_state);
    reg_sm_status = RU_FIELD_SET(0, NATC_DDR_CFG, SM_STATUS, DEBUG_SEL, reg_sm_status, sm_status->debug_sel);

    RU_REG_WRITE(0, NATC_DDR_CFG, SM_STATUS, reg_sm_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ddr_cfg_sm_status_get(natc_ddr_cfg_sm_status *sm_status)
{
    uint32_t reg_sm_status;

#ifdef VALIDATE_PARMS
    if (!sm_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC_DDR_CFG, SM_STATUS, reg_sm_status);

    sm_status->nat_state = RU_FIELD_GET(0, NATC_DDR_CFG, SM_STATUS, NAT_STATE, reg_sm_status);
    sm_status->wb_state = RU_FIELD_GET(0, NATC_DDR_CFG, SM_STATUS, WB_STATE, reg_sm_status);
    sm_status->runner_cmd_state = RU_FIELD_GET(0, NATC_DDR_CFG, SM_STATUS, RUNNER_CMD_STATE, reg_sm_status);
    sm_status->ddr_rep_state = RU_FIELD_GET(0, NATC_DDR_CFG, SM_STATUS, DDR_REP_STATE, reg_sm_status);
    sm_status->ddr_req_state = RU_FIELD_GET(0, NATC_DDR_CFG, SM_STATUS, DDR_REQ_STATE, reg_sm_status);
    sm_status->apb_state = RU_FIELD_GET(0, NATC_DDR_CFG, SM_STATUS, APB_STATE, reg_sm_status);
    sm_status->debug_sel = RU_FIELD_GET(0, NATC_DDR_CFG, SM_STATUS, DEBUG_SEL, reg_sm_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ddr_cfg_ddr_hash_mode_set(const natc_ddr_cfg_ddr_hash_mode *ddr_hash_mode)
{
    uint32_t reg_ddr_hash_mode = 0;

#ifdef VALIDATE_PARMS
    if(!ddr_hash_mode)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((ddr_hash_mode->ddr_hash_mode_tbl0 >= _4BITS_MAX_VAL_) ||
       (ddr_hash_mode->ddr_hash_mode_tbl1 >= _4BITS_MAX_VAL_) ||
       (ddr_hash_mode->ddr_hash_mode_tbl2 >= _4BITS_MAX_VAL_) ||
       (ddr_hash_mode->ddr_hash_mode_tbl3 >= _4BITS_MAX_VAL_) ||
       (ddr_hash_mode->ddr_hash_mode_tbl4 >= _4BITS_MAX_VAL_) ||
       (ddr_hash_mode->ddr_hash_mode_tbl5 >= _4BITS_MAX_VAL_) ||
       (ddr_hash_mode->ddr_hash_mode_tbl6 >= _4BITS_MAX_VAL_) ||
       (ddr_hash_mode->ddr_hash_mode_tbl7 >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_ddr_hash_mode = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_HASH_MODE, DDR_HASH_MODE_TBL0, reg_ddr_hash_mode, ddr_hash_mode->ddr_hash_mode_tbl0);
    reg_ddr_hash_mode = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_HASH_MODE, DDR_HASH_MODE_TBL1, reg_ddr_hash_mode, ddr_hash_mode->ddr_hash_mode_tbl1);
    reg_ddr_hash_mode = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_HASH_MODE, DDR_HASH_MODE_TBL2, reg_ddr_hash_mode, ddr_hash_mode->ddr_hash_mode_tbl2);
    reg_ddr_hash_mode = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_HASH_MODE, DDR_HASH_MODE_TBL3, reg_ddr_hash_mode, ddr_hash_mode->ddr_hash_mode_tbl3);
    reg_ddr_hash_mode = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_HASH_MODE, DDR_HASH_MODE_TBL4, reg_ddr_hash_mode, ddr_hash_mode->ddr_hash_mode_tbl4);
    reg_ddr_hash_mode = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_HASH_MODE, DDR_HASH_MODE_TBL5, reg_ddr_hash_mode, ddr_hash_mode->ddr_hash_mode_tbl5);
    reg_ddr_hash_mode = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_HASH_MODE, DDR_HASH_MODE_TBL6, reg_ddr_hash_mode, ddr_hash_mode->ddr_hash_mode_tbl6);
    reg_ddr_hash_mode = RU_FIELD_SET(0, NATC_DDR_CFG, DDR_HASH_MODE, DDR_HASH_MODE_TBL7, reg_ddr_hash_mode, ddr_hash_mode->ddr_hash_mode_tbl7);

    RU_REG_WRITE(0, NATC_DDR_CFG, DDR_HASH_MODE, reg_ddr_hash_mode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ddr_cfg_ddr_hash_mode_get(natc_ddr_cfg_ddr_hash_mode *ddr_hash_mode)
{
    uint32_t reg_ddr_hash_mode;

#ifdef VALIDATE_PARMS
    if (!ddr_hash_mode)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC_DDR_CFG, DDR_HASH_MODE, reg_ddr_hash_mode);

    ddr_hash_mode->ddr_hash_mode_tbl0 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_HASH_MODE, DDR_HASH_MODE_TBL0, reg_ddr_hash_mode);
    ddr_hash_mode->ddr_hash_mode_tbl1 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_HASH_MODE, DDR_HASH_MODE_TBL1, reg_ddr_hash_mode);
    ddr_hash_mode->ddr_hash_mode_tbl2 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_HASH_MODE, DDR_HASH_MODE_TBL2, reg_ddr_hash_mode);
    ddr_hash_mode->ddr_hash_mode_tbl3 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_HASH_MODE, DDR_HASH_MODE_TBL3, reg_ddr_hash_mode);
    ddr_hash_mode->ddr_hash_mode_tbl4 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_HASH_MODE, DDR_HASH_MODE_TBL4, reg_ddr_hash_mode);
    ddr_hash_mode->ddr_hash_mode_tbl5 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_HASH_MODE, DDR_HASH_MODE_TBL5, reg_ddr_hash_mode);
    ddr_hash_mode->ddr_hash_mode_tbl6 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_HASH_MODE, DDR_HASH_MODE_TBL6, reg_ddr_hash_mode);
    ddr_hash_mode->ddr_hash_mode_tbl7 = RU_FIELD_GET(0, NATC_DDR_CFG, DDR_HASH_MODE, DDR_HASH_MODE_TBL7, reg_ddr_hash_mode);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_ddr_size,
    bdmf_address_ddr_bins_per_bucket_0,
    bdmf_address_ddr_bins_per_bucket_1,
    bdmf_address_total_len,
    bdmf_address_sm_status,
    bdmf_address_ddr_hash_mode,
}
bdmf_address;

static int ag_drv_natc_ddr_cfg_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_natc_ddr_cfg_natc_ddr_size:
    {
        natc_ddr_cfg_natc_ddr_size natc_ddr_size = { .ddr_size_tbl0 = parm[1].value.unumber, .ddr_size_tbl1 = parm[2].value.unumber, .ddr_size_tbl2 = parm[3].value.unumber, .ddr_size_tbl3 = parm[4].value.unumber, .ddr_size_tbl4 = parm[5].value.unumber, .ddr_size_tbl5 = parm[6].value.unumber, .ddr_size_tbl6 = parm[7].value.unumber, .ddr_size_tbl7 = parm[8].value.unumber};
        ag_err = ag_drv_natc_ddr_cfg_natc_ddr_size_set(&natc_ddr_size);
        break;
    }
    case cli_natc_ddr_cfg_ddr_bins_per_bucket_0:
        ag_err = ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_natc_ddr_cfg_ddr_bins_per_bucket_1:
        ag_err = ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_natc_ddr_cfg_total_len:
    {
        natc_ddr_cfg_total_len total_len = { .total_len_tbl0 = parm[1].value.unumber, .total_len_tbl1 = parm[2].value.unumber, .total_len_tbl2 = parm[3].value.unumber, .total_len_tbl3 = parm[4].value.unumber, .total_len_tbl4 = parm[5].value.unumber, .total_len_tbl5 = parm[6].value.unumber, .total_len_tbl6 = parm[7].value.unumber, .total_len_tbl7 = parm[8].value.unumber};
        ag_err = ag_drv_natc_ddr_cfg_total_len_set(&total_len);
        break;
    }
    case cli_natc_ddr_cfg_sm_status:
    {
        natc_ddr_cfg_sm_status sm_status = { .nat_state = parm[1].value.unumber, .wb_state = parm[2].value.unumber, .runner_cmd_state = parm[3].value.unumber, .ddr_rep_state = parm[4].value.unumber, .ddr_req_state = parm[5].value.unumber, .apb_state = parm[6].value.unumber, .debug_sel = parm[7].value.unumber};
        ag_err = ag_drv_natc_ddr_cfg_sm_status_set(&sm_status);
        break;
    }
    case cli_natc_ddr_cfg_ddr_hash_mode:
    {
        natc_ddr_cfg_ddr_hash_mode ddr_hash_mode = { .ddr_hash_mode_tbl0 = parm[1].value.unumber, .ddr_hash_mode_tbl1 = parm[2].value.unumber, .ddr_hash_mode_tbl2 = parm[3].value.unumber, .ddr_hash_mode_tbl3 = parm[4].value.unumber, .ddr_hash_mode_tbl4 = parm[5].value.unumber, .ddr_hash_mode_tbl5 = parm[6].value.unumber, .ddr_hash_mode_tbl6 = parm[7].value.unumber, .ddr_hash_mode_tbl7 = parm[8].value.unumber};
        ag_err = ag_drv_natc_ddr_cfg_ddr_hash_mode_set(&ddr_hash_mode);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_natc_ddr_cfg_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_natc_ddr_cfg_natc_ddr_size:
    {
        natc_ddr_cfg_natc_ddr_size natc_ddr_size;
        ag_err = ag_drv_natc_ddr_cfg_natc_ddr_size_get(&natc_ddr_size);
        bdmf_session_print(session, "ddr_size_tbl0 = %u = 0x%x\n", natc_ddr_size.ddr_size_tbl0, natc_ddr_size.ddr_size_tbl0);
        bdmf_session_print(session, "ddr_size_tbl1 = %u = 0x%x\n", natc_ddr_size.ddr_size_tbl1, natc_ddr_size.ddr_size_tbl1);
        bdmf_session_print(session, "ddr_size_tbl2 = %u = 0x%x\n", natc_ddr_size.ddr_size_tbl2, natc_ddr_size.ddr_size_tbl2);
        bdmf_session_print(session, "ddr_size_tbl3 = %u = 0x%x\n", natc_ddr_size.ddr_size_tbl3, natc_ddr_size.ddr_size_tbl3);
        bdmf_session_print(session, "ddr_size_tbl4 = %u = 0x%x\n", natc_ddr_size.ddr_size_tbl4, natc_ddr_size.ddr_size_tbl4);
        bdmf_session_print(session, "ddr_size_tbl5 = %u = 0x%x\n", natc_ddr_size.ddr_size_tbl5, natc_ddr_size.ddr_size_tbl5);
        bdmf_session_print(session, "ddr_size_tbl6 = %u = 0x%x\n", natc_ddr_size.ddr_size_tbl6, natc_ddr_size.ddr_size_tbl6);
        bdmf_session_print(session, "ddr_size_tbl7 = %u = 0x%x\n", natc_ddr_size.ddr_size_tbl7, natc_ddr_size.ddr_size_tbl7);
        break;
    }
    case cli_natc_ddr_cfg_ddr_bins_per_bucket_0:
    {
        uint8_t ddr_bins_per_bucket_tbl0;
        uint8_t ddr_bins_per_bucket_tbl1;
        uint8_t ddr_bins_per_bucket_tbl2;
        uint8_t ddr_bins_per_bucket_tbl3;
        ag_err = ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_0_get(&ddr_bins_per_bucket_tbl0, &ddr_bins_per_bucket_tbl1, &ddr_bins_per_bucket_tbl2, &ddr_bins_per_bucket_tbl3);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl0 = %u = 0x%x\n", ddr_bins_per_bucket_tbl0, ddr_bins_per_bucket_tbl0);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl1 = %u = 0x%x\n", ddr_bins_per_bucket_tbl1, ddr_bins_per_bucket_tbl1);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl2 = %u = 0x%x\n", ddr_bins_per_bucket_tbl2, ddr_bins_per_bucket_tbl2);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl3 = %u = 0x%x\n", ddr_bins_per_bucket_tbl3, ddr_bins_per_bucket_tbl3);
        break;
    }
    case cli_natc_ddr_cfg_ddr_bins_per_bucket_1:
    {
        uint8_t ddr_bins_per_bucket_tbl4;
        uint8_t ddr_bins_per_bucket_tbl5;
        uint8_t ddr_bins_per_bucket_tbl6;
        uint8_t ddr_bins_per_bucket_tbl7;
        ag_err = ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_1_get(&ddr_bins_per_bucket_tbl4, &ddr_bins_per_bucket_tbl5, &ddr_bins_per_bucket_tbl6, &ddr_bins_per_bucket_tbl7);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl4 = %u = 0x%x\n", ddr_bins_per_bucket_tbl4, ddr_bins_per_bucket_tbl4);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl5 = %u = 0x%x\n", ddr_bins_per_bucket_tbl5, ddr_bins_per_bucket_tbl5);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl6 = %u = 0x%x\n", ddr_bins_per_bucket_tbl6, ddr_bins_per_bucket_tbl6);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl7 = %u = 0x%x\n", ddr_bins_per_bucket_tbl7, ddr_bins_per_bucket_tbl7);
        break;
    }
    case cli_natc_ddr_cfg_total_len:
    {
        natc_ddr_cfg_total_len total_len;
        ag_err = ag_drv_natc_ddr_cfg_total_len_get(&total_len);
        bdmf_session_print(session, "total_len_tbl0 = %u = 0x%x\n", total_len.total_len_tbl0, total_len.total_len_tbl0);
        bdmf_session_print(session, "total_len_tbl1 = %u = 0x%x\n", total_len.total_len_tbl1, total_len.total_len_tbl1);
        bdmf_session_print(session, "total_len_tbl2 = %u = 0x%x\n", total_len.total_len_tbl2, total_len.total_len_tbl2);
        bdmf_session_print(session, "total_len_tbl3 = %u = 0x%x\n", total_len.total_len_tbl3, total_len.total_len_tbl3);
        bdmf_session_print(session, "total_len_tbl4 = %u = 0x%x\n", total_len.total_len_tbl4, total_len.total_len_tbl4);
        bdmf_session_print(session, "total_len_tbl5 = %u = 0x%x\n", total_len.total_len_tbl5, total_len.total_len_tbl5);
        bdmf_session_print(session, "total_len_tbl6 = %u = 0x%x\n", total_len.total_len_tbl6, total_len.total_len_tbl6);
        bdmf_session_print(session, "total_len_tbl7 = %u = 0x%x\n", total_len.total_len_tbl7, total_len.total_len_tbl7);
        break;
    }
    case cli_natc_ddr_cfg_sm_status:
    {
        natc_ddr_cfg_sm_status sm_status;
        ag_err = ag_drv_natc_ddr_cfg_sm_status_get(&sm_status);
        bdmf_session_print(session, "nat_state = %u = 0x%x\n", sm_status.nat_state, sm_status.nat_state);
        bdmf_session_print(session, "wb_state = %u = 0x%x\n", sm_status.wb_state, sm_status.wb_state);
        bdmf_session_print(session, "runner_cmd_state = %u = 0x%x\n", sm_status.runner_cmd_state, sm_status.runner_cmd_state);
        bdmf_session_print(session, "ddr_rep_state = %u = 0x%x\n", sm_status.ddr_rep_state, sm_status.ddr_rep_state);
        bdmf_session_print(session, "ddr_req_state = %u = 0x%x\n", sm_status.ddr_req_state, sm_status.ddr_req_state);
        bdmf_session_print(session, "apb_state = %u = 0x%x\n", sm_status.apb_state, sm_status.apb_state);
        bdmf_session_print(session, "debug_sel = %u = 0x%x\n", sm_status.debug_sel, sm_status.debug_sel);
        break;
    }
    case cli_natc_ddr_cfg_ddr_hash_mode:
    {
        natc_ddr_cfg_ddr_hash_mode ddr_hash_mode;
        ag_err = ag_drv_natc_ddr_cfg_ddr_hash_mode_get(&ddr_hash_mode);
        bdmf_session_print(session, "ddr_hash_mode_tbl0 = %u = 0x%x\n", ddr_hash_mode.ddr_hash_mode_tbl0, ddr_hash_mode.ddr_hash_mode_tbl0);
        bdmf_session_print(session, "ddr_hash_mode_tbl1 = %u = 0x%x\n", ddr_hash_mode.ddr_hash_mode_tbl1, ddr_hash_mode.ddr_hash_mode_tbl1);
        bdmf_session_print(session, "ddr_hash_mode_tbl2 = %u = 0x%x\n", ddr_hash_mode.ddr_hash_mode_tbl2, ddr_hash_mode.ddr_hash_mode_tbl2);
        bdmf_session_print(session, "ddr_hash_mode_tbl3 = %u = 0x%x\n", ddr_hash_mode.ddr_hash_mode_tbl3, ddr_hash_mode.ddr_hash_mode_tbl3);
        bdmf_session_print(session, "ddr_hash_mode_tbl4 = %u = 0x%x\n", ddr_hash_mode.ddr_hash_mode_tbl4, ddr_hash_mode.ddr_hash_mode_tbl4);
        bdmf_session_print(session, "ddr_hash_mode_tbl5 = %u = 0x%x\n", ddr_hash_mode.ddr_hash_mode_tbl5, ddr_hash_mode.ddr_hash_mode_tbl5);
        bdmf_session_print(session, "ddr_hash_mode_tbl6 = %u = 0x%x\n", ddr_hash_mode.ddr_hash_mode_tbl6, ddr_hash_mode.ddr_hash_mode_tbl6);
        bdmf_session_print(session, "ddr_hash_mode_tbl7 = %u = 0x%x\n", ddr_hash_mode.ddr_hash_mode_tbl7, ddr_hash_mode.ddr_hash_mode_tbl7);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_natc_ddr_cfg_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        natc_ddr_cfg_natc_ddr_size natc_ddr_size = {.ddr_size_tbl0 = gtmv(m, 3), .ddr_size_tbl1 = gtmv(m, 3), .ddr_size_tbl2 = gtmv(m, 3), .ddr_size_tbl3 = gtmv(m, 3), .ddr_size_tbl4 = gtmv(m, 3), .ddr_size_tbl5 = gtmv(m, 3), .ddr_size_tbl6 = gtmv(m, 3), .ddr_size_tbl7 = gtmv(m, 3)};
        bdmf_session_print(session, "ag_drv_natc_ddr_cfg_natc_ddr_size_set( %u %u %u %u %u %u %u %u)\n",
            natc_ddr_size.ddr_size_tbl0, natc_ddr_size.ddr_size_tbl1, natc_ddr_size.ddr_size_tbl2, natc_ddr_size.ddr_size_tbl3, 
            natc_ddr_size.ddr_size_tbl4, natc_ddr_size.ddr_size_tbl5, natc_ddr_size.ddr_size_tbl6, natc_ddr_size.ddr_size_tbl7);
        ag_err = ag_drv_natc_ddr_cfg_natc_ddr_size_set(&natc_ddr_size);
        if (!ag_err)
            ag_err = ag_drv_natc_ddr_cfg_natc_ddr_size_get(&natc_ddr_size);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_natc_ddr_cfg_natc_ddr_size_get( %u %u %u %u %u %u %u %u)\n",
                natc_ddr_size.ddr_size_tbl0, natc_ddr_size.ddr_size_tbl1, natc_ddr_size.ddr_size_tbl2, natc_ddr_size.ddr_size_tbl3, 
                natc_ddr_size.ddr_size_tbl4, natc_ddr_size.ddr_size_tbl5, natc_ddr_size.ddr_size_tbl6, natc_ddr_size.ddr_size_tbl7);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (natc_ddr_size.ddr_size_tbl0 != gtmv(m, 3) || natc_ddr_size.ddr_size_tbl1 != gtmv(m, 3) || natc_ddr_size.ddr_size_tbl2 != gtmv(m, 3) || natc_ddr_size.ddr_size_tbl3 != gtmv(m, 3) || natc_ddr_size.ddr_size_tbl4 != gtmv(m, 3) || natc_ddr_size.ddr_size_tbl5 != gtmv(m, 3) || natc_ddr_size.ddr_size_tbl6 != gtmv(m, 3) || natc_ddr_size.ddr_size_tbl7 != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t ddr_bins_per_bucket_tbl0 = gtmv(m, 8);
        uint8_t ddr_bins_per_bucket_tbl1 = gtmv(m, 8);
        uint8_t ddr_bins_per_bucket_tbl2 = gtmv(m, 8);
        uint8_t ddr_bins_per_bucket_tbl3 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_0_set( %u %u %u %u)\n",
            ddr_bins_per_bucket_tbl0, ddr_bins_per_bucket_tbl1, ddr_bins_per_bucket_tbl2, ddr_bins_per_bucket_tbl3);
        ag_err = ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_0_set(ddr_bins_per_bucket_tbl0, ddr_bins_per_bucket_tbl1, ddr_bins_per_bucket_tbl2, ddr_bins_per_bucket_tbl3);
        if (!ag_err)
            ag_err = ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_0_get(&ddr_bins_per_bucket_tbl0, &ddr_bins_per_bucket_tbl1, &ddr_bins_per_bucket_tbl2, &ddr_bins_per_bucket_tbl3);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_0_get( %u %u %u %u)\n",
                ddr_bins_per_bucket_tbl0, ddr_bins_per_bucket_tbl1, ddr_bins_per_bucket_tbl2, ddr_bins_per_bucket_tbl3);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ddr_bins_per_bucket_tbl0 != gtmv(m, 8) || ddr_bins_per_bucket_tbl1 != gtmv(m, 8) || ddr_bins_per_bucket_tbl2 != gtmv(m, 8) || ddr_bins_per_bucket_tbl3 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t ddr_bins_per_bucket_tbl4 = gtmv(m, 8);
        uint8_t ddr_bins_per_bucket_tbl5 = gtmv(m, 8);
        uint8_t ddr_bins_per_bucket_tbl6 = gtmv(m, 8);
        uint8_t ddr_bins_per_bucket_tbl7 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_1_set( %u %u %u %u)\n",
            ddr_bins_per_bucket_tbl4, ddr_bins_per_bucket_tbl5, ddr_bins_per_bucket_tbl6, ddr_bins_per_bucket_tbl7);
        ag_err = ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_1_set(ddr_bins_per_bucket_tbl4, ddr_bins_per_bucket_tbl5, ddr_bins_per_bucket_tbl6, ddr_bins_per_bucket_tbl7);
        if (!ag_err)
            ag_err = ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_1_get(&ddr_bins_per_bucket_tbl4, &ddr_bins_per_bucket_tbl5, &ddr_bins_per_bucket_tbl6, &ddr_bins_per_bucket_tbl7);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_1_get( %u %u %u %u)\n",
                ddr_bins_per_bucket_tbl4, ddr_bins_per_bucket_tbl5, ddr_bins_per_bucket_tbl6, ddr_bins_per_bucket_tbl7);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ddr_bins_per_bucket_tbl4 != gtmv(m, 8) || ddr_bins_per_bucket_tbl5 != gtmv(m, 8) || ddr_bins_per_bucket_tbl6 != gtmv(m, 8) || ddr_bins_per_bucket_tbl7 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        natc_ddr_cfg_total_len total_len = {.total_len_tbl0 = gtmv(m, 3), .total_len_tbl1 = gtmv(m, 3), .total_len_tbl2 = gtmv(m, 3), .total_len_tbl3 = gtmv(m, 3), .total_len_tbl4 = gtmv(m, 3), .total_len_tbl5 = gtmv(m, 3), .total_len_tbl6 = gtmv(m, 3), .total_len_tbl7 = gtmv(m, 3)};
        bdmf_session_print(session, "ag_drv_natc_ddr_cfg_total_len_set( %u %u %u %u %u %u %u %u)\n",
            total_len.total_len_tbl0, total_len.total_len_tbl1, total_len.total_len_tbl2, total_len.total_len_tbl3, 
            total_len.total_len_tbl4, total_len.total_len_tbl5, total_len.total_len_tbl6, total_len.total_len_tbl7);
        ag_err = ag_drv_natc_ddr_cfg_total_len_set(&total_len);
        if (!ag_err)
            ag_err = ag_drv_natc_ddr_cfg_total_len_get(&total_len);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_natc_ddr_cfg_total_len_get( %u %u %u %u %u %u %u %u)\n",
                total_len.total_len_tbl0, total_len.total_len_tbl1, total_len.total_len_tbl2, total_len.total_len_tbl3, 
                total_len.total_len_tbl4, total_len.total_len_tbl5, total_len.total_len_tbl6, total_len.total_len_tbl7);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (total_len.total_len_tbl0 != gtmv(m, 3) || total_len.total_len_tbl1 != gtmv(m, 3) || total_len.total_len_tbl2 != gtmv(m, 3) || total_len.total_len_tbl3 != gtmv(m, 3) || total_len.total_len_tbl4 != gtmv(m, 3) || total_len.total_len_tbl5 != gtmv(m, 3) || total_len.total_len_tbl6 != gtmv(m, 3) || total_len.total_len_tbl7 != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        natc_ddr_cfg_sm_status sm_status = {.nat_state = gtmv(m, 15), .wb_state = gtmv(m, 1), .runner_cmd_state = gtmv(m, 1), .ddr_rep_state = gtmv(m, 3), .ddr_req_state = gtmv(m, 2), .apb_state = gtmv(m, 2), .debug_sel = gtmv(m, 2)};
        bdmf_session_print(session, "ag_drv_natc_ddr_cfg_sm_status_set( %u %u %u %u %u %u %u)\n",
            sm_status.nat_state, sm_status.wb_state, sm_status.runner_cmd_state, sm_status.ddr_rep_state, 
            sm_status.ddr_req_state, sm_status.apb_state, sm_status.debug_sel);
        ag_err = ag_drv_natc_ddr_cfg_sm_status_set(&sm_status);
        if (!ag_err)
            ag_err = ag_drv_natc_ddr_cfg_sm_status_get(&sm_status);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_natc_ddr_cfg_sm_status_get( %u %u %u %u %u %u %u)\n",
                sm_status.nat_state, sm_status.wb_state, sm_status.runner_cmd_state, sm_status.ddr_rep_state, 
                sm_status.ddr_req_state, sm_status.apb_state, sm_status.debug_sel);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (sm_status.nat_state != gtmv(m, 15) || sm_status.wb_state != gtmv(m, 1) || sm_status.runner_cmd_state != gtmv(m, 1) || sm_status.ddr_rep_state != gtmv(m, 3) || sm_status.ddr_req_state != gtmv(m, 2) || sm_status.apb_state != gtmv(m, 2) || sm_status.debug_sel != gtmv(m, 2))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        natc_ddr_cfg_ddr_hash_mode ddr_hash_mode = {.ddr_hash_mode_tbl0 = gtmv(m, 4), .ddr_hash_mode_tbl1 = gtmv(m, 4), .ddr_hash_mode_tbl2 = gtmv(m, 4), .ddr_hash_mode_tbl3 = gtmv(m, 4), .ddr_hash_mode_tbl4 = gtmv(m, 4), .ddr_hash_mode_tbl5 = gtmv(m, 4), .ddr_hash_mode_tbl6 = gtmv(m, 4), .ddr_hash_mode_tbl7 = gtmv(m, 4)};
        bdmf_session_print(session, "ag_drv_natc_ddr_cfg_ddr_hash_mode_set( %u %u %u %u %u %u %u %u)\n",
            ddr_hash_mode.ddr_hash_mode_tbl0, ddr_hash_mode.ddr_hash_mode_tbl1, ddr_hash_mode.ddr_hash_mode_tbl2, ddr_hash_mode.ddr_hash_mode_tbl3, 
            ddr_hash_mode.ddr_hash_mode_tbl4, ddr_hash_mode.ddr_hash_mode_tbl5, ddr_hash_mode.ddr_hash_mode_tbl6, ddr_hash_mode.ddr_hash_mode_tbl7);
        ag_err = ag_drv_natc_ddr_cfg_ddr_hash_mode_set(&ddr_hash_mode);
        if (!ag_err)
            ag_err = ag_drv_natc_ddr_cfg_ddr_hash_mode_get(&ddr_hash_mode);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_natc_ddr_cfg_ddr_hash_mode_get( %u %u %u %u %u %u %u %u)\n",
                ddr_hash_mode.ddr_hash_mode_tbl0, ddr_hash_mode.ddr_hash_mode_tbl1, ddr_hash_mode.ddr_hash_mode_tbl2, ddr_hash_mode.ddr_hash_mode_tbl3, 
                ddr_hash_mode.ddr_hash_mode_tbl4, ddr_hash_mode.ddr_hash_mode_tbl5, ddr_hash_mode.ddr_hash_mode_tbl6, ddr_hash_mode.ddr_hash_mode_tbl7);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ddr_hash_mode.ddr_hash_mode_tbl0 != gtmv(m, 4) || ddr_hash_mode.ddr_hash_mode_tbl1 != gtmv(m, 4) || ddr_hash_mode.ddr_hash_mode_tbl2 != gtmv(m, 4) || ddr_hash_mode.ddr_hash_mode_tbl3 != gtmv(m, 4) || ddr_hash_mode.ddr_hash_mode_tbl4 != gtmv(m, 4) || ddr_hash_mode.ddr_hash_mode_tbl5 != gtmv(m, 4) || ddr_hash_mode.ddr_hash_mode_tbl6 != gtmv(m, 4) || ddr_hash_mode.ddr_hash_mode_tbl7 != gtmv(m, 4))
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
static int ag_drv_natc_ddr_cfg_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_ddr_size: reg = &RU_REG(NATC_DDR_CFG, DDR_SIZE); blk = &RU_BLK(NATC_DDR_CFG); break;
    case bdmf_address_ddr_bins_per_bucket_0: reg = &RU_REG(NATC_DDR_CFG, DDR_BINS_PER_BUCKET_0); blk = &RU_BLK(NATC_DDR_CFG); break;
    case bdmf_address_ddr_bins_per_bucket_1: reg = &RU_REG(NATC_DDR_CFG, DDR_BINS_PER_BUCKET_1); blk = &RU_BLK(NATC_DDR_CFG); break;
    case bdmf_address_total_len: reg = &RU_REG(NATC_DDR_CFG, TOTAL_LEN); blk = &RU_BLK(NATC_DDR_CFG); break;
    case bdmf_address_sm_status: reg = &RU_REG(NATC_DDR_CFG, SM_STATUS); blk = &RU_BLK(NATC_DDR_CFG); break;
    case bdmf_address_ddr_hash_mode: reg = &RU_REG(NATC_DDR_CFG, DDR_HASH_MODE); blk = &RU_BLK(NATC_DDR_CFG); break;
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

bdmfmon_handle_t ag_drv_natc_ddr_cfg_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "natc_ddr_cfg", "natc_ddr_cfg", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_natc_ddr_size[] = {
            BDMFMON_MAKE_PARM("ddr_size_tbl0", "ddr_size_tbl0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_size_tbl1", "ddr_size_tbl1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_size_tbl2", "ddr_size_tbl2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_size_tbl3", "ddr_size_tbl3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_size_tbl4", "ddr_size_tbl4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_size_tbl5", "ddr_size_tbl5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_size_tbl6", "ddr_size_tbl6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_size_tbl7", "ddr_size_tbl7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ddr_bins_per_bucket_0[] = {
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl0", "ddr_bins_per_bucket_tbl0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl1", "ddr_bins_per_bucket_tbl1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl2", "ddr_bins_per_bucket_tbl2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl3", "ddr_bins_per_bucket_tbl3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ddr_bins_per_bucket_1[] = {
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl4", "ddr_bins_per_bucket_tbl4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl5", "ddr_bins_per_bucket_tbl5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl6", "ddr_bins_per_bucket_tbl6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl7", "ddr_bins_per_bucket_tbl7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_total_len[] = {
            BDMFMON_MAKE_PARM("total_len_tbl0", "total_len_tbl0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("total_len_tbl1", "total_len_tbl1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("total_len_tbl2", "total_len_tbl2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("total_len_tbl3", "total_len_tbl3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("total_len_tbl4", "total_len_tbl4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("total_len_tbl5", "total_len_tbl5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("total_len_tbl6", "total_len_tbl6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("total_len_tbl7", "total_len_tbl7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_sm_status[] = {
            BDMFMON_MAKE_PARM("nat_state", "nat_state", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("wb_state", "wb_state", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("runner_cmd_state", "runner_cmd_state", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_rep_state", "ddr_rep_state", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_req_state", "ddr_req_state", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("apb_state", "apb_state", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("debug_sel", "debug_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ddr_hash_mode[] = {
            BDMFMON_MAKE_PARM("ddr_hash_mode_tbl0", "ddr_hash_mode_tbl0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_hash_mode_tbl1", "ddr_hash_mode_tbl1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_hash_mode_tbl2", "ddr_hash_mode_tbl2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_hash_mode_tbl3", "ddr_hash_mode_tbl3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_hash_mode_tbl4", "ddr_hash_mode_tbl4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_hash_mode_tbl5", "ddr_hash_mode_tbl5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_hash_mode_tbl6", "ddr_hash_mode_tbl6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_hash_mode_tbl7", "ddr_hash_mode_tbl7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "natc_ddr_size", .val = cli_natc_ddr_cfg_natc_ddr_size, .parms = set_natc_ddr_size },
            { .name = "ddr_bins_per_bucket_0", .val = cli_natc_ddr_cfg_ddr_bins_per_bucket_0, .parms = set_ddr_bins_per_bucket_0 },
            { .name = "ddr_bins_per_bucket_1", .val = cli_natc_ddr_cfg_ddr_bins_per_bucket_1, .parms = set_ddr_bins_per_bucket_1 },
            { .name = "total_len", .val = cli_natc_ddr_cfg_total_len, .parms = set_total_len },
            { .name = "sm_status", .val = cli_natc_ddr_cfg_sm_status, .parms = set_sm_status },
            { .name = "ddr_hash_mode", .val = cli_natc_ddr_cfg_ddr_hash_mode, .parms = set_ddr_hash_mode },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_natc_ddr_cfg_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "natc_ddr_size", .val = cli_natc_ddr_cfg_natc_ddr_size, .parms = get_default },
            { .name = "ddr_bins_per_bucket_0", .val = cli_natc_ddr_cfg_ddr_bins_per_bucket_0, .parms = get_default },
            { .name = "ddr_bins_per_bucket_1", .val = cli_natc_ddr_cfg_ddr_bins_per_bucket_1, .parms = get_default },
            { .name = "total_len", .val = cli_natc_ddr_cfg_total_len, .parms = get_default },
            { .name = "sm_status", .val = cli_natc_ddr_cfg_sm_status, .parms = get_default },
            { .name = "ddr_hash_mode", .val = cli_natc_ddr_cfg_ddr_hash_mode, .parms = get_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_natc_ddr_cfg_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_natc_ddr_cfg_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "DDR_SIZE", .val = bdmf_address_ddr_size },
            { .name = "DDR_BINS_PER_BUCKET_0", .val = bdmf_address_ddr_bins_per_bucket_0 },
            { .name = "DDR_BINS_PER_BUCKET_1", .val = bdmf_address_ddr_bins_per_bucket_1 },
            { .name = "TOTAL_LEN", .val = bdmf_address_total_len },
            { .name = "SM_STATUS", .val = bdmf_address_sm_status },
            { .name = "DDR_HASH_MODE", .val = bdmf_address_ddr_hash_mode },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_natc_ddr_cfg_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
