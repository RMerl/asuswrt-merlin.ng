/*
   Copyright (c) 2015 Broadcom Corporation
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

#include "drivers_common_ag.h"
#include "wan_prbs_ag.h"
bdmf_error_t ag_drv_wan_prbs_wan_prbs_chk_ctrl_0_set(const wan_prbs_wan_prbs_chk_ctrl_0 *wan_prbs_chk_ctrl_0)
{
    uint32_t reg_chk_ctrl_0=0;

#ifdef VALIDATE_PARMS
    if(!wan_prbs_chk_ctrl_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((wan_prbs_chk_ctrl_0->en_timer_mode >= _2BITS_MAX_VAL_) ||
       (wan_prbs_chk_ctrl_0->en_timeout >= _5BITS_MAX_VAL_) ||
       (wan_prbs_chk_ctrl_0->mode_sel >= _3BITS_MAX_VAL_) ||
       (wan_prbs_chk_ctrl_0->err_cnt_burst_mode >= _1BITS_MAX_VAL_) ||
       (wan_prbs_chk_ctrl_0->lock_cnt >= _5BITS_MAX_VAL_) ||
       (wan_prbs_chk_ctrl_0->ool_cnt >= _5BITS_MAX_VAL_) ||
       (wan_prbs_chk_ctrl_0->inv >= _1BITS_MAX_VAL_) ||
       (wan_prbs_chk_ctrl_0->sig_prbs_status_clr >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_chk_ctrl_0 = RU_FIELD_SET(0, WAN_PRBS, CHK_CTRL_0, EN_TIMER_MODE, reg_chk_ctrl_0, wan_prbs_chk_ctrl_0->en_timer_mode);
    reg_chk_ctrl_0 = RU_FIELD_SET(0, WAN_PRBS, CHK_CTRL_0, EN_TIMEOUT, reg_chk_ctrl_0, wan_prbs_chk_ctrl_0->en_timeout);
    reg_chk_ctrl_0 = RU_FIELD_SET(0, WAN_PRBS, CHK_CTRL_0, MODE_SEL, reg_chk_ctrl_0, wan_prbs_chk_ctrl_0->mode_sel);
    reg_chk_ctrl_0 = RU_FIELD_SET(0, WAN_PRBS, CHK_CTRL_0, ERR_CNT_BURST_MODE, reg_chk_ctrl_0, wan_prbs_chk_ctrl_0->err_cnt_burst_mode);
    reg_chk_ctrl_0 = RU_FIELD_SET(0, WAN_PRBS, CHK_CTRL_0, LOCK_CNT, reg_chk_ctrl_0, wan_prbs_chk_ctrl_0->lock_cnt);
    reg_chk_ctrl_0 = RU_FIELD_SET(0, WAN_PRBS, CHK_CTRL_0, OOL_CNT, reg_chk_ctrl_0, wan_prbs_chk_ctrl_0->ool_cnt);
    reg_chk_ctrl_0 = RU_FIELD_SET(0, WAN_PRBS, CHK_CTRL_0, INV, reg_chk_ctrl_0, wan_prbs_chk_ctrl_0->inv);
    reg_chk_ctrl_0 = RU_FIELD_SET(0, WAN_PRBS, CHK_CTRL_0, SIG_PRBS_STATUS_CLR, reg_chk_ctrl_0, wan_prbs_chk_ctrl_0->sig_prbs_status_clr);

    RU_REG_WRITE(0, WAN_PRBS, CHK_CTRL_0, reg_chk_ctrl_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_wan_prbs_wan_prbs_chk_ctrl_0_get(wan_prbs_wan_prbs_chk_ctrl_0 *wan_prbs_chk_ctrl_0)
{
    uint32_t reg_chk_ctrl_0=0;

#ifdef VALIDATE_PARMS
    if(!wan_prbs_chk_ctrl_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, WAN_PRBS, CHK_CTRL_0, reg_chk_ctrl_0);

    wan_prbs_chk_ctrl_0->en_timer_mode = RU_FIELD_GET(0, WAN_PRBS, CHK_CTRL_0, EN_TIMER_MODE, reg_chk_ctrl_0);
    wan_prbs_chk_ctrl_0->en_timeout = RU_FIELD_GET(0, WAN_PRBS, CHK_CTRL_0, EN_TIMEOUT, reg_chk_ctrl_0);
    wan_prbs_chk_ctrl_0->mode_sel = RU_FIELD_GET(0, WAN_PRBS, CHK_CTRL_0, MODE_SEL, reg_chk_ctrl_0);
    wan_prbs_chk_ctrl_0->err_cnt_burst_mode = RU_FIELD_GET(0, WAN_PRBS, CHK_CTRL_0, ERR_CNT_BURST_MODE, reg_chk_ctrl_0);
    wan_prbs_chk_ctrl_0->lock_cnt = RU_FIELD_GET(0, WAN_PRBS, CHK_CTRL_0, LOCK_CNT, reg_chk_ctrl_0);
    wan_prbs_chk_ctrl_0->ool_cnt = RU_FIELD_GET(0, WAN_PRBS, CHK_CTRL_0, OOL_CNT, reg_chk_ctrl_0);
    wan_prbs_chk_ctrl_0->inv = RU_FIELD_GET(0, WAN_PRBS, CHK_CTRL_0, INV, reg_chk_ctrl_0);
    wan_prbs_chk_ctrl_0->sig_prbs_status_clr = RU_FIELD_GET(0, WAN_PRBS, CHK_CTRL_0, SIG_PRBS_STATUS_CLR, reg_chk_ctrl_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_wan_prbs_wan_prbs_chk_ctrl_1_set(bdmf_boolean prbs_chk_en, uint8_t prbs_chk_mode, uint32_t prbs_timer_val)
{
    uint32_t reg_chk_ctrl_1=0;

#ifdef VALIDATE_PARMS
    if((prbs_timer_val >= _20BITS_MAX_VAL_) ||
       (prbs_chk_mode >= _2BITS_MAX_VAL_) ||
       (prbs_chk_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_chk_ctrl_1 = RU_FIELD_SET(0, WAN_PRBS, CHK_CTRL_1, PRBS_TIMER_VAL, reg_chk_ctrl_1, prbs_timer_val);
    reg_chk_ctrl_1 = RU_FIELD_SET(0, WAN_PRBS, CHK_CTRL_1, PRBS_CHK_MODE, reg_chk_ctrl_1, prbs_chk_mode);
    reg_chk_ctrl_1 = RU_FIELD_SET(0, WAN_PRBS, CHK_CTRL_1, PRBS_CHK_EN, reg_chk_ctrl_1, prbs_chk_en);

    RU_REG_WRITE(0, WAN_PRBS, CHK_CTRL_1, reg_chk_ctrl_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_wan_prbs_wan_prbs_chk_ctrl_1_get(bdmf_boolean *prbs_chk_en, uint8_t *prbs_chk_mode, uint32_t *prbs_timer_val)
{
    uint32_t reg_chk_ctrl_1=0;

#ifdef VALIDATE_PARMS
    if(!prbs_timer_val || !prbs_chk_mode || !prbs_chk_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, WAN_PRBS, CHK_CTRL_1, reg_chk_ctrl_1);

    *prbs_timer_val = RU_FIELD_GET(0, WAN_PRBS, CHK_CTRL_1, PRBS_TIMER_VAL, reg_chk_ctrl_1);
    *prbs_chk_mode = RU_FIELD_GET(0, WAN_PRBS, CHK_CTRL_1, PRBS_CHK_MODE, reg_chk_ctrl_1);
    *prbs_chk_en = RU_FIELD_GET(0, WAN_PRBS, CHK_CTRL_1, PRBS_CHK_EN, reg_chk_ctrl_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_wan_prbs_wan_prbs_status_0_get(bdmf_boolean *lock_lost_lh, uint32_t *err_cnt)
{
    uint32_t reg_status_0=0;

#ifdef VALIDATE_PARMS
    if(!err_cnt || !lock_lost_lh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, WAN_PRBS, STATUS_0, reg_status_0);

    *err_cnt = RU_FIELD_GET(0, WAN_PRBS, STATUS_0, ERR_CNT, reg_status_0);
    *lock_lost_lh = RU_FIELD_GET(0, WAN_PRBS, STATUS_0, LOCK_LOST_LH, reg_status_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_wan_prbs_wan_prbs_status_1_get(bdmf_boolean *any_err, bdmf_boolean *lock)
{
    uint32_t reg_status_1=0;

#ifdef VALIDATE_PARMS
    if(!lock || !any_err)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, WAN_PRBS, STATUS_1, reg_status_1);

    *lock = RU_FIELD_GET(0, WAN_PRBS, STATUS_1, LOCK, reg_status_1);
    *any_err = RU_FIELD_GET(0, WAN_PRBS, STATUS_1, ANY_ERR, reg_status_1);

    return BDMF_ERR_OK;
}

