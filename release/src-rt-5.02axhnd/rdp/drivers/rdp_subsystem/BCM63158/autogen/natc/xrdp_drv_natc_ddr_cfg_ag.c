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

#include "rdp_common.h"
#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_natc_ddr_cfg_ag.h"

bdmf_error_t ag_drv_natc_ddr_cfg_natc_ddr_size_set(const natc_ddr_cfg_natc_ddr_size *natc_ddr_size)
{
    uint32_t reg__ddr_size=0;

#ifdef VALIDATE_PARMS
    if(!natc_ddr_size)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((natc_ddr_size->ddr_size_tbl0 >= _3BITS_MAX_VAL_) ||
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

    reg__ddr_size = RU_FIELD_SET(0, NATC_DDR_CFG, _DDR_SIZE, DDR_SIZE_TBL0, reg__ddr_size, natc_ddr_size->ddr_size_tbl0);
    reg__ddr_size = RU_FIELD_SET(0, NATC_DDR_CFG, _DDR_SIZE, DDR_SIZE_TBL1, reg__ddr_size, natc_ddr_size->ddr_size_tbl1);
    reg__ddr_size = RU_FIELD_SET(0, NATC_DDR_CFG, _DDR_SIZE, DDR_SIZE_TBL2, reg__ddr_size, natc_ddr_size->ddr_size_tbl2);
    reg__ddr_size = RU_FIELD_SET(0, NATC_DDR_CFG, _DDR_SIZE, DDR_SIZE_TBL3, reg__ddr_size, natc_ddr_size->ddr_size_tbl3);
    reg__ddr_size = RU_FIELD_SET(0, NATC_DDR_CFG, _DDR_SIZE, DDR_SIZE_TBL4, reg__ddr_size, natc_ddr_size->ddr_size_tbl4);
    reg__ddr_size = RU_FIELD_SET(0, NATC_DDR_CFG, _DDR_SIZE, DDR_SIZE_TBL5, reg__ddr_size, natc_ddr_size->ddr_size_tbl5);
    reg__ddr_size = RU_FIELD_SET(0, NATC_DDR_CFG, _DDR_SIZE, DDR_SIZE_TBL6, reg__ddr_size, natc_ddr_size->ddr_size_tbl6);
    reg__ddr_size = RU_FIELD_SET(0, NATC_DDR_CFG, _DDR_SIZE, DDR_SIZE_TBL7, reg__ddr_size, natc_ddr_size->ddr_size_tbl7);

    RU_REG_WRITE(0, NATC_DDR_CFG, _DDR_SIZE, reg__ddr_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ddr_cfg_natc_ddr_size_get(natc_ddr_cfg_natc_ddr_size *natc_ddr_size)
{
    uint32_t reg__ddr_size;

#ifdef VALIDATE_PARMS
    if(!natc_ddr_size)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC_DDR_CFG, _DDR_SIZE, reg__ddr_size);

    natc_ddr_size->ddr_size_tbl0 = RU_FIELD_GET(0, NATC_DDR_CFG, _DDR_SIZE, DDR_SIZE_TBL0, reg__ddr_size);
    natc_ddr_size->ddr_size_tbl1 = RU_FIELD_GET(0, NATC_DDR_CFG, _DDR_SIZE, DDR_SIZE_TBL1, reg__ddr_size);
    natc_ddr_size->ddr_size_tbl2 = RU_FIELD_GET(0, NATC_DDR_CFG, _DDR_SIZE, DDR_SIZE_TBL2, reg__ddr_size);
    natc_ddr_size->ddr_size_tbl3 = RU_FIELD_GET(0, NATC_DDR_CFG, _DDR_SIZE, DDR_SIZE_TBL3, reg__ddr_size);
    natc_ddr_size->ddr_size_tbl4 = RU_FIELD_GET(0, NATC_DDR_CFG, _DDR_SIZE, DDR_SIZE_TBL4, reg__ddr_size);
    natc_ddr_size->ddr_size_tbl5 = RU_FIELD_GET(0, NATC_DDR_CFG, _DDR_SIZE, DDR_SIZE_TBL5, reg__ddr_size);
    natc_ddr_size->ddr_size_tbl6 = RU_FIELD_GET(0, NATC_DDR_CFG, _DDR_SIZE, DDR_SIZE_TBL6, reg__ddr_size);
    natc_ddr_size->ddr_size_tbl7 = RU_FIELD_GET(0, NATC_DDR_CFG, _DDR_SIZE, DDR_SIZE_TBL7, reg__ddr_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_0_set(uint8_t ddr_bins_per_bucket_tbl3, uint8_t ddr_bins_per_bucket_tbl2, uint8_t ddr_bins_per_bucket_tbl1, uint8_t ddr_bins_per_bucket_tbl0)
{
    uint32_t reg__ddr_bins_per_bucket_0=0;

#ifdef VALIDATE_PARMS
#endif

    reg__ddr_bins_per_bucket_0 = RU_FIELD_SET(0, NATC_DDR_CFG, _DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL3, reg__ddr_bins_per_bucket_0, ddr_bins_per_bucket_tbl3);
    reg__ddr_bins_per_bucket_0 = RU_FIELD_SET(0, NATC_DDR_CFG, _DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL2, reg__ddr_bins_per_bucket_0, ddr_bins_per_bucket_tbl2);
    reg__ddr_bins_per_bucket_0 = RU_FIELD_SET(0, NATC_DDR_CFG, _DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL1, reg__ddr_bins_per_bucket_0, ddr_bins_per_bucket_tbl1);
    reg__ddr_bins_per_bucket_0 = RU_FIELD_SET(0, NATC_DDR_CFG, _DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL0, reg__ddr_bins_per_bucket_0, ddr_bins_per_bucket_tbl0);

    RU_REG_WRITE(0, NATC_DDR_CFG, _DDR_BINS_PER_BUCKET_0, reg__ddr_bins_per_bucket_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_0_get(uint8_t *ddr_bins_per_bucket_tbl3, uint8_t *ddr_bins_per_bucket_tbl2, uint8_t *ddr_bins_per_bucket_tbl1, uint8_t *ddr_bins_per_bucket_tbl0)
{
    uint32_t reg__ddr_bins_per_bucket_0;

#ifdef VALIDATE_PARMS
    if(!ddr_bins_per_bucket_tbl3 || !ddr_bins_per_bucket_tbl2 || !ddr_bins_per_bucket_tbl1 || !ddr_bins_per_bucket_tbl0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC_DDR_CFG, _DDR_BINS_PER_BUCKET_0, reg__ddr_bins_per_bucket_0);

    *ddr_bins_per_bucket_tbl3 = RU_FIELD_GET(0, NATC_DDR_CFG, _DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL3, reg__ddr_bins_per_bucket_0);
    *ddr_bins_per_bucket_tbl2 = RU_FIELD_GET(0, NATC_DDR_CFG, _DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL2, reg__ddr_bins_per_bucket_0);
    *ddr_bins_per_bucket_tbl1 = RU_FIELD_GET(0, NATC_DDR_CFG, _DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL1, reg__ddr_bins_per_bucket_0);
    *ddr_bins_per_bucket_tbl0 = RU_FIELD_GET(0, NATC_DDR_CFG, _DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL0, reg__ddr_bins_per_bucket_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_1_set(uint8_t ddr_bins_per_bucket_tbl7, uint8_t ddr_bins_per_bucket_tbl6, uint8_t ddr_bins_per_bucket_tbl5, uint8_t ddr_bins_per_bucket_tbl4)
{
    uint32_t reg__ddr_bins_per_bucket_1=0;

#ifdef VALIDATE_PARMS
#endif

    reg__ddr_bins_per_bucket_1 = RU_FIELD_SET(0, NATC_DDR_CFG, _DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL7, reg__ddr_bins_per_bucket_1, ddr_bins_per_bucket_tbl7);
    reg__ddr_bins_per_bucket_1 = RU_FIELD_SET(0, NATC_DDR_CFG, _DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL6, reg__ddr_bins_per_bucket_1, ddr_bins_per_bucket_tbl6);
    reg__ddr_bins_per_bucket_1 = RU_FIELD_SET(0, NATC_DDR_CFG, _DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL5, reg__ddr_bins_per_bucket_1, ddr_bins_per_bucket_tbl5);
    reg__ddr_bins_per_bucket_1 = RU_FIELD_SET(0, NATC_DDR_CFG, _DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL4, reg__ddr_bins_per_bucket_1, ddr_bins_per_bucket_tbl4);

    RU_REG_WRITE(0, NATC_DDR_CFG, _DDR_BINS_PER_BUCKET_1, reg__ddr_bins_per_bucket_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_1_get(uint8_t *ddr_bins_per_bucket_tbl7, uint8_t *ddr_bins_per_bucket_tbl6, uint8_t *ddr_bins_per_bucket_tbl5, uint8_t *ddr_bins_per_bucket_tbl4)
{
    uint32_t reg__ddr_bins_per_bucket_1;

#ifdef VALIDATE_PARMS
    if(!ddr_bins_per_bucket_tbl7 || !ddr_bins_per_bucket_tbl6 || !ddr_bins_per_bucket_tbl5 || !ddr_bins_per_bucket_tbl4)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC_DDR_CFG, _DDR_BINS_PER_BUCKET_1, reg__ddr_bins_per_bucket_1);

    *ddr_bins_per_bucket_tbl7 = RU_FIELD_GET(0, NATC_DDR_CFG, _DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL7, reg__ddr_bins_per_bucket_1);
    *ddr_bins_per_bucket_tbl6 = RU_FIELD_GET(0, NATC_DDR_CFG, _DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL6, reg__ddr_bins_per_bucket_1);
    *ddr_bins_per_bucket_tbl5 = RU_FIELD_GET(0, NATC_DDR_CFG, _DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL5, reg__ddr_bins_per_bucket_1);
    *ddr_bins_per_bucket_tbl4 = RU_FIELD_GET(0, NATC_DDR_CFG, _DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL4, reg__ddr_bins_per_bucket_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ddr_cfg_total_len_set(const natc_ddr_cfg_total_len *total_len)
{
    uint32_t reg__total_len=0;

#ifdef VALIDATE_PARMS
    if(!total_len)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((total_len->total_len_tbl7 >= _3BITS_MAX_VAL_) ||
       (total_len->total_len_tbl6 >= _3BITS_MAX_VAL_) ||
       (total_len->total_len_tbl5 >= _3BITS_MAX_VAL_) ||
       (total_len->total_len_tbl4 >= _3BITS_MAX_VAL_) ||
       (total_len->total_len_tbl3 >= _3BITS_MAX_VAL_) ||
       (total_len->total_len_tbl2 >= _3BITS_MAX_VAL_) ||
       (total_len->total_len_tbl1 >= _3BITS_MAX_VAL_) ||
       (total_len->total_len_tbl0 >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg__total_len = RU_FIELD_SET(0, NATC_DDR_CFG, _TOTAL_LEN, TOTAL_LEN_TBL7, reg__total_len, total_len->total_len_tbl7);
    reg__total_len = RU_FIELD_SET(0, NATC_DDR_CFG, _TOTAL_LEN, TOTAL_LEN_TBL6, reg__total_len, total_len->total_len_tbl6);
    reg__total_len = RU_FIELD_SET(0, NATC_DDR_CFG, _TOTAL_LEN, TOTAL_LEN_TBL5, reg__total_len, total_len->total_len_tbl5);
    reg__total_len = RU_FIELD_SET(0, NATC_DDR_CFG, _TOTAL_LEN, TOTAL_LEN_TBL4, reg__total_len, total_len->total_len_tbl4);
    reg__total_len = RU_FIELD_SET(0, NATC_DDR_CFG, _TOTAL_LEN, TOTAL_LEN_TBL3, reg__total_len, total_len->total_len_tbl3);
    reg__total_len = RU_FIELD_SET(0, NATC_DDR_CFG, _TOTAL_LEN, TOTAL_LEN_TBL2, reg__total_len, total_len->total_len_tbl2);
    reg__total_len = RU_FIELD_SET(0, NATC_DDR_CFG, _TOTAL_LEN, TOTAL_LEN_TBL1, reg__total_len, total_len->total_len_tbl1);
    reg__total_len = RU_FIELD_SET(0, NATC_DDR_CFG, _TOTAL_LEN, TOTAL_LEN_TBL0, reg__total_len, total_len->total_len_tbl0);

    RU_REG_WRITE(0, NATC_DDR_CFG, _TOTAL_LEN, reg__total_len);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ddr_cfg_total_len_get(natc_ddr_cfg_total_len *total_len)
{
    uint32_t reg__total_len;

#ifdef VALIDATE_PARMS
    if(!total_len)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC_DDR_CFG, _TOTAL_LEN, reg__total_len);

    total_len->total_len_tbl7 = RU_FIELD_GET(0, NATC_DDR_CFG, _TOTAL_LEN, TOTAL_LEN_TBL7, reg__total_len);
    total_len->total_len_tbl6 = RU_FIELD_GET(0, NATC_DDR_CFG, _TOTAL_LEN, TOTAL_LEN_TBL6, reg__total_len);
    total_len->total_len_tbl5 = RU_FIELD_GET(0, NATC_DDR_CFG, _TOTAL_LEN, TOTAL_LEN_TBL5, reg__total_len);
    total_len->total_len_tbl4 = RU_FIELD_GET(0, NATC_DDR_CFG, _TOTAL_LEN, TOTAL_LEN_TBL4, reg__total_len);
    total_len->total_len_tbl3 = RU_FIELD_GET(0, NATC_DDR_CFG, _TOTAL_LEN, TOTAL_LEN_TBL3, reg__total_len);
    total_len->total_len_tbl2 = RU_FIELD_GET(0, NATC_DDR_CFG, _TOTAL_LEN, TOTAL_LEN_TBL2, reg__total_len);
    total_len->total_len_tbl1 = RU_FIELD_GET(0, NATC_DDR_CFG, _TOTAL_LEN, TOTAL_LEN_TBL1, reg__total_len);
    total_len->total_len_tbl0 = RU_FIELD_GET(0, NATC_DDR_CFG, _TOTAL_LEN, TOTAL_LEN_TBL0, reg__total_len);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ddr_cfg__sm_status_set(const natc_ddr_cfg__sm_status *_sm_status)
{
    uint32_t reg__sm_status=0;

#ifdef VALIDATE_PARMS
    if(!_sm_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((_sm_status->debug_sel >= _2BITS_MAX_VAL_) ||
       (_sm_status->apb_state >= _2BITS_MAX_VAL_) ||
       (_sm_status->ddr_req_state >= _2BITS_MAX_VAL_) ||
       (_sm_status->ddr_rep_state >= _3BITS_MAX_VAL_) ||
       (_sm_status->runner_cmd_state >= _1BITS_MAX_VAL_) ||
       (_sm_status->wb_state >= _1BITS_MAX_VAL_) ||
       (_sm_status->nat_state >= _11BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg__sm_status = RU_FIELD_SET(0, NATC_DDR_CFG, _SM_STATUS, DEBUG_SEL, reg__sm_status, _sm_status->debug_sel);
    reg__sm_status = RU_FIELD_SET(0, NATC_DDR_CFG, _SM_STATUS, APB_STATE, reg__sm_status, _sm_status->apb_state);
    reg__sm_status = RU_FIELD_SET(0, NATC_DDR_CFG, _SM_STATUS, DDR_REQ_STATE, reg__sm_status, _sm_status->ddr_req_state);
    reg__sm_status = RU_FIELD_SET(0, NATC_DDR_CFG, _SM_STATUS, DDR_REP_STATE, reg__sm_status, _sm_status->ddr_rep_state);
    reg__sm_status = RU_FIELD_SET(0, NATC_DDR_CFG, _SM_STATUS, RUNNER_CMD_STATE, reg__sm_status, _sm_status->runner_cmd_state);
    reg__sm_status = RU_FIELD_SET(0, NATC_DDR_CFG, _SM_STATUS, WB_STATE, reg__sm_status, _sm_status->wb_state);
    reg__sm_status = RU_FIELD_SET(0, NATC_DDR_CFG, _SM_STATUS, NAT_STATE, reg__sm_status, _sm_status->nat_state);

    RU_REG_WRITE(0, NATC_DDR_CFG, _SM_STATUS, reg__sm_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ddr_cfg__sm_status_get(natc_ddr_cfg__sm_status *_sm_status)
{
    uint32_t reg__sm_status;

#ifdef VALIDATE_PARMS
    if(!_sm_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC_DDR_CFG, _SM_STATUS, reg__sm_status);

    _sm_status->debug_sel = RU_FIELD_GET(0, NATC_DDR_CFG, _SM_STATUS, DEBUG_SEL, reg__sm_status);
    _sm_status->apb_state = RU_FIELD_GET(0, NATC_DDR_CFG, _SM_STATUS, APB_STATE, reg__sm_status);
    _sm_status->ddr_req_state = RU_FIELD_GET(0, NATC_DDR_CFG, _SM_STATUS, DDR_REQ_STATE, reg__sm_status);
    _sm_status->ddr_rep_state = RU_FIELD_GET(0, NATC_DDR_CFG, _SM_STATUS, DDR_REP_STATE, reg__sm_status);
    _sm_status->runner_cmd_state = RU_FIELD_GET(0, NATC_DDR_CFG, _SM_STATUS, RUNNER_CMD_STATE, reg__sm_status);
    _sm_status->wb_state = RU_FIELD_GET(0, NATC_DDR_CFG, _SM_STATUS, WB_STATE, reg__sm_status);
    _sm_status->nat_state = RU_FIELD_GET(0, NATC_DDR_CFG, _SM_STATUS, NAT_STATE, reg__sm_status);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
typedef enum
{
    bdmf_address__ddr_size,
    bdmf_address__ddr_bins_per_bucket_0,
    bdmf_address__ddr_bins_per_bucket_1,
    bdmf_address__total_len,
    bdmf_address__sm_status,
}
bdmf_address;

static int bcm_natc_ddr_cfg_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_natc_ddr_cfg_natc_ddr_size:
    {
        natc_ddr_cfg_natc_ddr_size natc_ddr_size = { .ddr_size_tbl0=parm[1].value.unumber, .ddr_size_tbl1=parm[2].value.unumber, .ddr_size_tbl2=parm[3].value.unumber, .ddr_size_tbl3=parm[4].value.unumber, .ddr_size_tbl4=parm[5].value.unumber, .ddr_size_tbl5=parm[6].value.unumber, .ddr_size_tbl6=parm[7].value.unumber, .ddr_size_tbl7=parm[8].value.unumber};
        err = ag_drv_natc_ddr_cfg_natc_ddr_size_set(&natc_ddr_size);
        break;
    }
    case cli_natc_ddr_cfg_ddr_bins_per_bucket_0:
        err = ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_natc_ddr_cfg_ddr_bins_per_bucket_1:
        err = ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_natc_ddr_cfg_total_len:
    {
        natc_ddr_cfg_total_len total_len = { .total_len_tbl7=parm[1].value.unumber, .total_len_tbl6=parm[2].value.unumber, .total_len_tbl5=parm[3].value.unumber, .total_len_tbl4=parm[4].value.unumber, .total_len_tbl3=parm[5].value.unumber, .total_len_tbl2=parm[6].value.unumber, .total_len_tbl1=parm[7].value.unumber, .total_len_tbl0=parm[8].value.unumber};
        err = ag_drv_natc_ddr_cfg_total_len_set(&total_len);
        break;
    }
    case cli_natc_ddr_cfg__sm_status:
    {
        natc_ddr_cfg__sm_status _sm_status = { .debug_sel=parm[1].value.unumber, .apb_state=parm[2].value.unumber, .ddr_req_state=parm[3].value.unumber, .ddr_rep_state=parm[4].value.unumber, .runner_cmd_state=parm[5].value.unumber, .wb_state=parm[6].value.unumber, .nat_state=parm[7].value.unumber};
        err = ag_drv_natc_ddr_cfg__sm_status_set(&_sm_status);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_natc_ddr_cfg_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_natc_ddr_cfg_natc_ddr_size:
    {
        natc_ddr_cfg_natc_ddr_size natc_ddr_size;
        err = ag_drv_natc_ddr_cfg_natc_ddr_size_get(&natc_ddr_size);
        bdmf_session_print(session, "ddr_size_tbl0 = %u (0x%x)\n", natc_ddr_size.ddr_size_tbl0, natc_ddr_size.ddr_size_tbl0);
        bdmf_session_print(session, "ddr_size_tbl1 = %u (0x%x)\n", natc_ddr_size.ddr_size_tbl1, natc_ddr_size.ddr_size_tbl1);
        bdmf_session_print(session, "ddr_size_tbl2 = %u (0x%x)\n", natc_ddr_size.ddr_size_tbl2, natc_ddr_size.ddr_size_tbl2);
        bdmf_session_print(session, "ddr_size_tbl3 = %u (0x%x)\n", natc_ddr_size.ddr_size_tbl3, natc_ddr_size.ddr_size_tbl3);
        bdmf_session_print(session, "ddr_size_tbl4 = %u (0x%x)\n", natc_ddr_size.ddr_size_tbl4, natc_ddr_size.ddr_size_tbl4);
        bdmf_session_print(session, "ddr_size_tbl5 = %u (0x%x)\n", natc_ddr_size.ddr_size_tbl5, natc_ddr_size.ddr_size_tbl5);
        bdmf_session_print(session, "ddr_size_tbl6 = %u (0x%x)\n", natc_ddr_size.ddr_size_tbl6, natc_ddr_size.ddr_size_tbl6);
        bdmf_session_print(session, "ddr_size_tbl7 = %u (0x%x)\n", natc_ddr_size.ddr_size_tbl7, natc_ddr_size.ddr_size_tbl7);
        break;
    }
    case cli_natc_ddr_cfg_ddr_bins_per_bucket_0:
    {
        uint8_t ddr_bins_per_bucket_tbl3;
        uint8_t ddr_bins_per_bucket_tbl2;
        uint8_t ddr_bins_per_bucket_tbl1;
        uint8_t ddr_bins_per_bucket_tbl0;
        err = ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_0_get(&ddr_bins_per_bucket_tbl3, &ddr_bins_per_bucket_tbl2, &ddr_bins_per_bucket_tbl1, &ddr_bins_per_bucket_tbl0);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl3 = %u (0x%x)\n", ddr_bins_per_bucket_tbl3, ddr_bins_per_bucket_tbl3);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl2 = %u (0x%x)\n", ddr_bins_per_bucket_tbl2, ddr_bins_per_bucket_tbl2);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl1 = %u (0x%x)\n", ddr_bins_per_bucket_tbl1, ddr_bins_per_bucket_tbl1);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl0 = %u (0x%x)\n", ddr_bins_per_bucket_tbl0, ddr_bins_per_bucket_tbl0);
        break;
    }
    case cli_natc_ddr_cfg_ddr_bins_per_bucket_1:
    {
        uint8_t ddr_bins_per_bucket_tbl7;
        uint8_t ddr_bins_per_bucket_tbl6;
        uint8_t ddr_bins_per_bucket_tbl5;
        uint8_t ddr_bins_per_bucket_tbl4;
        err = ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_1_get(&ddr_bins_per_bucket_tbl7, &ddr_bins_per_bucket_tbl6, &ddr_bins_per_bucket_tbl5, &ddr_bins_per_bucket_tbl4);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl7 = %u (0x%x)\n", ddr_bins_per_bucket_tbl7, ddr_bins_per_bucket_tbl7);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl6 = %u (0x%x)\n", ddr_bins_per_bucket_tbl6, ddr_bins_per_bucket_tbl6);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl5 = %u (0x%x)\n", ddr_bins_per_bucket_tbl5, ddr_bins_per_bucket_tbl5);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl4 = %u (0x%x)\n", ddr_bins_per_bucket_tbl4, ddr_bins_per_bucket_tbl4);
        break;
    }
    case cli_natc_ddr_cfg_total_len:
    {
        natc_ddr_cfg_total_len total_len;
        err = ag_drv_natc_ddr_cfg_total_len_get(&total_len);
        bdmf_session_print(session, "total_len_tbl7 = %u (0x%x)\n", total_len.total_len_tbl7, total_len.total_len_tbl7);
        bdmf_session_print(session, "total_len_tbl6 = %u (0x%x)\n", total_len.total_len_tbl6, total_len.total_len_tbl6);
        bdmf_session_print(session, "total_len_tbl5 = %u (0x%x)\n", total_len.total_len_tbl5, total_len.total_len_tbl5);
        bdmf_session_print(session, "total_len_tbl4 = %u (0x%x)\n", total_len.total_len_tbl4, total_len.total_len_tbl4);
        bdmf_session_print(session, "total_len_tbl3 = %u (0x%x)\n", total_len.total_len_tbl3, total_len.total_len_tbl3);
        bdmf_session_print(session, "total_len_tbl2 = %u (0x%x)\n", total_len.total_len_tbl2, total_len.total_len_tbl2);
        bdmf_session_print(session, "total_len_tbl1 = %u (0x%x)\n", total_len.total_len_tbl1, total_len.total_len_tbl1);
        bdmf_session_print(session, "total_len_tbl0 = %u (0x%x)\n", total_len.total_len_tbl0, total_len.total_len_tbl0);
        break;
    }
    case cli_natc_ddr_cfg__sm_status:
    {
        natc_ddr_cfg__sm_status _sm_status;
        err = ag_drv_natc_ddr_cfg__sm_status_get(&_sm_status);
        bdmf_session_print(session, "debug_sel = %u (0x%x)\n", _sm_status.debug_sel, _sm_status.debug_sel);
        bdmf_session_print(session, "apb_state = %u (0x%x)\n", _sm_status.apb_state, _sm_status.apb_state);
        bdmf_session_print(session, "ddr_req_state = %u (0x%x)\n", _sm_status.ddr_req_state, _sm_status.ddr_req_state);
        bdmf_session_print(session, "ddr_rep_state = %u (0x%x)\n", _sm_status.ddr_rep_state, _sm_status.ddr_rep_state);
        bdmf_session_print(session, "runner_cmd_state = %u (0x%x)\n", _sm_status.runner_cmd_state, _sm_status.runner_cmd_state);
        bdmf_session_print(session, "wb_state = %u (0x%x)\n", _sm_status.wb_state, _sm_status.wb_state);
        bdmf_session_print(session, "nat_state = %u (0x%x)\n", _sm_status.nat_state, _sm_status.nat_state);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_natc_ddr_cfg_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        natc_ddr_cfg_natc_ddr_size natc_ddr_size = {.ddr_size_tbl0=gtmv(m, 3), .ddr_size_tbl1=gtmv(m, 3), .ddr_size_tbl2=gtmv(m, 3), .ddr_size_tbl3=gtmv(m, 3), .ddr_size_tbl4=gtmv(m, 3), .ddr_size_tbl5=gtmv(m, 3), .ddr_size_tbl6=gtmv(m, 3), .ddr_size_tbl7=gtmv(m, 3)};
        if(!err) bdmf_session_print(session, "ag_drv_natc_ddr_cfg_natc_ddr_size_set( %u %u %u %u %u %u %u %u)\n", natc_ddr_size.ddr_size_tbl0, natc_ddr_size.ddr_size_tbl1, natc_ddr_size.ddr_size_tbl2, natc_ddr_size.ddr_size_tbl3, natc_ddr_size.ddr_size_tbl4, natc_ddr_size.ddr_size_tbl5, natc_ddr_size.ddr_size_tbl6, natc_ddr_size.ddr_size_tbl7);
        if(!err) ag_drv_natc_ddr_cfg_natc_ddr_size_set(&natc_ddr_size);
        if(!err) ag_drv_natc_ddr_cfg_natc_ddr_size_get( &natc_ddr_size);
        if(!err) bdmf_session_print(session, "ag_drv_natc_ddr_cfg_natc_ddr_size_get( %u %u %u %u %u %u %u %u)\n", natc_ddr_size.ddr_size_tbl0, natc_ddr_size.ddr_size_tbl1, natc_ddr_size.ddr_size_tbl2, natc_ddr_size.ddr_size_tbl3, natc_ddr_size.ddr_size_tbl4, natc_ddr_size.ddr_size_tbl5, natc_ddr_size.ddr_size_tbl6, natc_ddr_size.ddr_size_tbl7);
        if(err || natc_ddr_size.ddr_size_tbl0!=gtmv(m, 3) || natc_ddr_size.ddr_size_tbl1!=gtmv(m, 3) || natc_ddr_size.ddr_size_tbl2!=gtmv(m, 3) || natc_ddr_size.ddr_size_tbl3!=gtmv(m, 3) || natc_ddr_size.ddr_size_tbl4!=gtmv(m, 3) || natc_ddr_size.ddr_size_tbl5!=gtmv(m, 3) || natc_ddr_size.ddr_size_tbl6!=gtmv(m, 3) || natc_ddr_size.ddr_size_tbl7!=gtmv(m, 3))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t ddr_bins_per_bucket_tbl3=gtmv(m, 8);
        uint8_t ddr_bins_per_bucket_tbl2=gtmv(m, 8);
        uint8_t ddr_bins_per_bucket_tbl1=gtmv(m, 8);
        uint8_t ddr_bins_per_bucket_tbl0=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_0_set( %u %u %u %u)\n", ddr_bins_per_bucket_tbl3, ddr_bins_per_bucket_tbl2, ddr_bins_per_bucket_tbl1, ddr_bins_per_bucket_tbl0);
        if(!err) ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_0_set(ddr_bins_per_bucket_tbl3, ddr_bins_per_bucket_tbl2, ddr_bins_per_bucket_tbl1, ddr_bins_per_bucket_tbl0);
        if(!err) ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_0_get( &ddr_bins_per_bucket_tbl3, &ddr_bins_per_bucket_tbl2, &ddr_bins_per_bucket_tbl1, &ddr_bins_per_bucket_tbl0);
        if(!err) bdmf_session_print(session, "ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_0_get( %u %u %u %u)\n", ddr_bins_per_bucket_tbl3, ddr_bins_per_bucket_tbl2, ddr_bins_per_bucket_tbl1, ddr_bins_per_bucket_tbl0);
        if(err || ddr_bins_per_bucket_tbl3!=gtmv(m, 8) || ddr_bins_per_bucket_tbl2!=gtmv(m, 8) || ddr_bins_per_bucket_tbl1!=gtmv(m, 8) || ddr_bins_per_bucket_tbl0!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t ddr_bins_per_bucket_tbl7=gtmv(m, 8);
        uint8_t ddr_bins_per_bucket_tbl6=gtmv(m, 8);
        uint8_t ddr_bins_per_bucket_tbl5=gtmv(m, 8);
        uint8_t ddr_bins_per_bucket_tbl4=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_1_set( %u %u %u %u)\n", ddr_bins_per_bucket_tbl7, ddr_bins_per_bucket_tbl6, ddr_bins_per_bucket_tbl5, ddr_bins_per_bucket_tbl4);
        if(!err) ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_1_set(ddr_bins_per_bucket_tbl7, ddr_bins_per_bucket_tbl6, ddr_bins_per_bucket_tbl5, ddr_bins_per_bucket_tbl4);
        if(!err) ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_1_get( &ddr_bins_per_bucket_tbl7, &ddr_bins_per_bucket_tbl6, &ddr_bins_per_bucket_tbl5, &ddr_bins_per_bucket_tbl4);
        if(!err) bdmf_session_print(session, "ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_1_get( %u %u %u %u)\n", ddr_bins_per_bucket_tbl7, ddr_bins_per_bucket_tbl6, ddr_bins_per_bucket_tbl5, ddr_bins_per_bucket_tbl4);
        if(err || ddr_bins_per_bucket_tbl7!=gtmv(m, 8) || ddr_bins_per_bucket_tbl6!=gtmv(m, 8) || ddr_bins_per_bucket_tbl5!=gtmv(m, 8) || ddr_bins_per_bucket_tbl4!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        natc_ddr_cfg_total_len total_len = {.total_len_tbl7=gtmv(m, 3), .total_len_tbl6=gtmv(m, 3), .total_len_tbl5=gtmv(m, 3), .total_len_tbl4=gtmv(m, 3), .total_len_tbl3=gtmv(m, 3), .total_len_tbl2=gtmv(m, 3), .total_len_tbl1=gtmv(m, 3), .total_len_tbl0=gtmv(m, 3)};
        if(!err) bdmf_session_print(session, "ag_drv_natc_ddr_cfg_total_len_set( %u %u %u %u %u %u %u %u)\n", total_len.total_len_tbl7, total_len.total_len_tbl6, total_len.total_len_tbl5, total_len.total_len_tbl4, total_len.total_len_tbl3, total_len.total_len_tbl2, total_len.total_len_tbl1, total_len.total_len_tbl0);
        if(!err) ag_drv_natc_ddr_cfg_total_len_set(&total_len);
        if(!err) ag_drv_natc_ddr_cfg_total_len_get( &total_len);
        if(!err) bdmf_session_print(session, "ag_drv_natc_ddr_cfg_total_len_get( %u %u %u %u %u %u %u %u)\n", total_len.total_len_tbl7, total_len.total_len_tbl6, total_len.total_len_tbl5, total_len.total_len_tbl4, total_len.total_len_tbl3, total_len.total_len_tbl2, total_len.total_len_tbl1, total_len.total_len_tbl0);
        if(err || total_len.total_len_tbl7!=gtmv(m, 3) || total_len.total_len_tbl6!=gtmv(m, 3) || total_len.total_len_tbl5!=gtmv(m, 3) || total_len.total_len_tbl4!=gtmv(m, 3) || total_len.total_len_tbl3!=gtmv(m, 3) || total_len.total_len_tbl2!=gtmv(m, 3) || total_len.total_len_tbl1!=gtmv(m, 3) || total_len.total_len_tbl0!=gtmv(m, 3))
            return err ? err : BDMF_ERR_IO;
    }
    {
        natc_ddr_cfg__sm_status _sm_status = {.debug_sel=gtmv(m, 2), .apb_state=gtmv(m, 2), .ddr_req_state=gtmv(m, 2), .ddr_rep_state=gtmv(m, 3), .runner_cmd_state=gtmv(m, 1), .wb_state=gtmv(m, 1), .nat_state=gtmv(m, 11)};
        if(!err) bdmf_session_print(session, "ag_drv_natc_ddr_cfg__sm_status_set( %u %u %u %u %u %u %u)\n", _sm_status.debug_sel, _sm_status.apb_state, _sm_status.ddr_req_state, _sm_status.ddr_rep_state, _sm_status.runner_cmd_state, _sm_status.wb_state, _sm_status.nat_state);
        if(!err) ag_drv_natc_ddr_cfg__sm_status_set(&_sm_status);
        if(!err) ag_drv_natc_ddr_cfg__sm_status_get( &_sm_status);
        if(!err) bdmf_session_print(session, "ag_drv_natc_ddr_cfg__sm_status_get( %u %u %u %u %u %u %u)\n", _sm_status.debug_sel, _sm_status.apb_state, _sm_status.ddr_req_state, _sm_status.ddr_rep_state, _sm_status.runner_cmd_state, _sm_status.wb_state, _sm_status.nat_state);
        if(err || _sm_status.debug_sel!=gtmv(m, 2) || _sm_status.apb_state!=gtmv(m, 2) || _sm_status.ddr_req_state!=gtmv(m, 2) || _sm_status.ddr_rep_state!=gtmv(m, 3) || _sm_status.runner_cmd_state!=gtmv(m, 1) || _sm_status.wb_state!=gtmv(m, 1) || _sm_status.nat_state!=gtmv(m, 11))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_natc_ddr_cfg_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t i;
    uint32_t j;
    uint32_t index1_start=0;
    uint32_t index1_stop;
    uint32_t index2_start=0;
    uint32_t index2_stop;
    bdmfmon_cmd_parm_t * bdmf_parm;
    const ru_reg_rec * reg;
    const ru_block_rec * blk;
    const char * enum_string = bdmfmon_enum_parm_stringval(session, 0, parm[0].value.unumber);

    if(!enum_string)
        return BDMF_ERR_INTERNAL;

    switch (parm[0].value.unumber)
    {
    case bdmf_address__ddr_size : reg = &RU_REG(NATC_DDR_CFG, _DDR_SIZE); blk = &RU_BLK(NATC_DDR_CFG); break;
    case bdmf_address__ddr_bins_per_bucket_0 : reg = &RU_REG(NATC_DDR_CFG, _DDR_BINS_PER_BUCKET_0); blk = &RU_BLK(NATC_DDR_CFG); break;
    case bdmf_address__ddr_bins_per_bucket_1 : reg = &RU_REG(NATC_DDR_CFG, _DDR_BINS_PER_BUCKET_1); blk = &RU_BLK(NATC_DDR_CFG); break;
    case bdmf_address__total_len : reg = &RU_REG(NATC_DDR_CFG, _TOTAL_LEN); blk = &RU_BLK(NATC_DDR_CFG); break;
    case bdmf_address__sm_status : reg = &RU_REG(NATC_DDR_CFG, _SM_STATUS); blk = &RU_BLK(NATC_DDR_CFG); break;
    default :
        return BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    if((bdmf_parm = bdmfmon_find_named_parm(session,"index1")))
    {
        index1_start = bdmf_parm->value.unumber;
        index1_stop = index1_start + 1;
    }
    else
        index1_stop = blk->addr_count;
    if((bdmf_parm = bdmfmon_find_named_parm(session,"index2")))
    {
        index2_start = bdmf_parm->value.unumber;
        index2_stop = index2_start + 1;
    }
    else
        index2_stop = reg->ram_count + 1;
    if(index1_stop > blk->addr_count)
    {
        bdmf_session_print(session, "index1 (%u) is out of range (%u).\n", index1_stop, blk->addr_count);
        return BDMF_ERR_RANGE;
    }
    if(index2_stop > (reg->ram_count + 1))
    {
        bdmf_session_print(session, "index2 (%u) is out of range (%u).\n", index2_stop, reg->ram_count + 1);
        return BDMF_ERR_RANGE;
    }
    if(reg->ram_count)
        for (i = index1_start; i < index1_stop; i++)
        {
            bdmf_session_print(session, "index1 = %u\n", i);
            for (j = index2_start; j < index2_stop; j++)
                bdmf_session_print(session, 	 "(%5u) 0x%lX\n", j, (blk->addr[i] + reg->addr + j));
        }
    else
        for (i = index1_start; i < index1_stop; i++)
            bdmf_session_print(session, "(%3u) 0x%lX\n", i, blk->addr[i]+reg->addr);
    return 0;
}

bdmfmon_handle_t ag_drv_natc_ddr_cfg_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "natc_ddr_cfg"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "natc_ddr_cfg", "natc_ddr_cfg", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_natc_ddr_size[]={
            BDMFMON_MAKE_PARM("ddr_size_tbl0", "ddr_size_tbl0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_size_tbl1", "ddr_size_tbl1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_size_tbl2", "ddr_size_tbl2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_size_tbl3", "ddr_size_tbl3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_size_tbl4", "ddr_size_tbl4", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_size_tbl5", "ddr_size_tbl5", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_size_tbl6", "ddr_size_tbl6", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_size_tbl7", "ddr_size_tbl7", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ddr_bins_per_bucket_0[]={
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl3", "ddr_bins_per_bucket_tbl3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl2", "ddr_bins_per_bucket_tbl2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl1", "ddr_bins_per_bucket_tbl1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl0", "ddr_bins_per_bucket_tbl0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ddr_bins_per_bucket_1[]={
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl7", "ddr_bins_per_bucket_tbl7", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl6", "ddr_bins_per_bucket_tbl6", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl5", "ddr_bins_per_bucket_tbl5", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl4", "ddr_bins_per_bucket_tbl4", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_total_len[]={
            BDMFMON_MAKE_PARM("total_len_tbl7", "total_len_tbl7", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("total_len_tbl6", "total_len_tbl6", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("total_len_tbl5", "total_len_tbl5", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("total_len_tbl4", "total_len_tbl4", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("total_len_tbl3", "total_len_tbl3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("total_len_tbl2", "total_len_tbl2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("total_len_tbl1", "total_len_tbl1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("total_len_tbl0", "total_len_tbl0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set__sm_status[]={
            BDMFMON_MAKE_PARM("debug_sel", "debug_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("apb_state", "apb_state", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_req_state", "ddr_req_state", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_rep_state", "ddr_rep_state", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("runner_cmd_state", "runner_cmd_state", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("wb_state", "wb_state", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("nat_state", "nat_state", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="natc_ddr_size", .val=cli_natc_ddr_cfg_natc_ddr_size, .parms=set_natc_ddr_size },
            { .name="ddr_bins_per_bucket_0", .val=cli_natc_ddr_cfg_ddr_bins_per_bucket_0, .parms=set_ddr_bins_per_bucket_0 },
            { .name="ddr_bins_per_bucket_1", .val=cli_natc_ddr_cfg_ddr_bins_per_bucket_1, .parms=set_ddr_bins_per_bucket_1 },
            { .name="total_len", .val=cli_natc_ddr_cfg_total_len, .parms=set_total_len },
            { .name="_sm_status", .val=cli_natc_ddr_cfg__sm_status, .parms=set__sm_status },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_natc_ddr_cfg_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="natc_ddr_size", .val=cli_natc_ddr_cfg_natc_ddr_size, .parms=set_default },
            { .name="ddr_bins_per_bucket_0", .val=cli_natc_ddr_cfg_ddr_bins_per_bucket_0, .parms=set_default },
            { .name="ddr_bins_per_bucket_1", .val=cli_natc_ddr_cfg_ddr_bins_per_bucket_1, .parms=set_default },
            { .name="total_len", .val=cli_natc_ddr_cfg_total_len, .parms=set_default },
            { .name="_sm_status", .val=cli_natc_ddr_cfg__sm_status, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_natc_ddr_cfg_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_natc_ddr_cfg_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="_DDR_SIZE" , .val=bdmf_address__ddr_size },
            { .name="_DDR_BINS_PER_BUCKET_0" , .val=bdmf_address__ddr_bins_per_bucket_0 },
            { .name="_DDR_BINS_PER_BUCKET_1" , .val=bdmf_address__ddr_bins_per_bucket_1 },
            { .name="_TOTAL_LEN" , .val=bdmf_address__total_len },
            { .name="_SM_STATUS" , .val=bdmf_address__sm_status },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_natc_ddr_cfg_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

