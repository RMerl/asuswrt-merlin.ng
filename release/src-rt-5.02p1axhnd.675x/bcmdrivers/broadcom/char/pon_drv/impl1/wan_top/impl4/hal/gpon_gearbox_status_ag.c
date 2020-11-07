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
#include "gpon_gearbox_status_ag.h"
int ag_drv_gpon_gearbox_status_gearbox_status_get(uint32_t *cr_rd_data_clx)
{
    uint32_t reg_gearbox_status=0;

#ifdef VALIDATE_PARMS
    if(!cr_rd_data_clx)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, GPON_GEARBOX_STATUS, GEARBOX_STATUS, reg_gearbox_status);

    *cr_rd_data_clx = RU_FIELD_GET(0, GPON_GEARBOX_STATUS, GEARBOX_STATUS, CR_RD_DATA_CLX, reg_gearbox_status);

    return 0;
}

int ag_drv_gpon_gearbox_status_gearbox_prbs_control_0_set(const gpon_gearbox_status_gearbox_prbs_control_0 *gearbox_prbs_control_0)
{
    uint32_t reg_gearbox_prbs_control_0=0;

#ifdef VALIDATE_PARMS
    if(!gearbox_prbs_control_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_sig_prbs_status_clr >= _1BITS_MAX_VAL_) ||
       (gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_inv >= _1BITS_MAX_VAL_) ||
       (gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_ool_cnt >= _5BITS_MAX_VAL_) ||
       (gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_lock_cnt >= _5BITS_MAX_VAL_) ||
       (gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_err_cnt_burst_mode >= _1BITS_MAX_VAL_) ||
       (gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_mode_sel >= _3BITS_MAX_VAL_) ||
       (gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_en_timeout >= _5BITS_MAX_VAL_) ||
       (gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_en_timer_mode >= _2BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gearbox_prbs_control_0 = RU_FIELD_SET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_SIG_PRBS_STATUS_CLR, reg_gearbox_prbs_control_0, gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_sig_prbs_status_clr);
    reg_gearbox_prbs_control_0 = RU_FIELD_SET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_INV, reg_gearbox_prbs_control_0, gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_inv);
    reg_gearbox_prbs_control_0 = RU_FIELD_SET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_OOL_CNT, reg_gearbox_prbs_control_0, gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_ool_cnt);
    reg_gearbox_prbs_control_0 = RU_FIELD_SET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_LOCK_CNT, reg_gearbox_prbs_control_0, gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_lock_cnt);
    reg_gearbox_prbs_control_0 = RU_FIELD_SET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_ERR_CNT_BURST_MODE, reg_gearbox_prbs_control_0, gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_err_cnt_burst_mode);
    reg_gearbox_prbs_control_0 = RU_FIELD_SET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_MODE_SEL, reg_gearbox_prbs_control_0, gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_mode_sel);
    reg_gearbox_prbs_control_0 = RU_FIELD_SET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_EN_TIMEOUT, reg_gearbox_prbs_control_0, gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_en_timeout);
    reg_gearbox_prbs_control_0 = RU_FIELD_SET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_EN_TIMER_MODE, reg_gearbox_prbs_control_0, gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_en_timer_mode);

    RU_REG_WRITE(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_0, reg_gearbox_prbs_control_0);

    return 0;
}

int ag_drv_gpon_gearbox_status_gearbox_prbs_control_0_get(gpon_gearbox_status_gearbox_prbs_control_0 *gearbox_prbs_control_0)
{
    uint32_t reg_gearbox_prbs_control_0=0;

#ifdef VALIDATE_PARMS
    if(!gearbox_prbs_control_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_0, reg_gearbox_prbs_control_0);

    gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_sig_prbs_status_clr = RU_FIELD_GET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_SIG_PRBS_STATUS_CLR, reg_gearbox_prbs_control_0);
    gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_inv = RU_FIELD_GET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_INV, reg_gearbox_prbs_control_0);
    gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_ool_cnt = RU_FIELD_GET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_OOL_CNT, reg_gearbox_prbs_control_0);
    gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_lock_cnt = RU_FIELD_GET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_LOCK_CNT, reg_gearbox_prbs_control_0);
    gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_err_cnt_burst_mode = RU_FIELD_GET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_ERR_CNT_BURST_MODE, reg_gearbox_prbs_control_0);
    gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_mode_sel = RU_FIELD_GET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_MODE_SEL, reg_gearbox_prbs_control_0);
    gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_en_timeout = RU_FIELD_GET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_EN_TIMEOUT, reg_gearbox_prbs_control_0);
    gearbox_prbs_control_0->cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_0_en_timer_mode = RU_FIELD_GET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_EN_TIMER_MODE, reg_gearbox_prbs_control_0);

    return 0;
}

int ag_drv_gpon_gearbox_status_gearbox_prbs_control_1_set(uint8_t cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_1_en, uint8_t cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_1_mode, uint32_t cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_1_timer_val)
{
    uint32_t reg_gearbox_prbs_control_1=0;

#ifdef VALIDATE_PARMS
    if((cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_1_en >= _1BITS_MAX_VAL_) ||
       (cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_1_mode >= _2BITS_MAX_VAL_) ||
       (cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_1_timer_val >= _20BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gearbox_prbs_control_1 = RU_FIELD_SET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_1, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_EN, reg_gearbox_prbs_control_1, cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_1_en);
    reg_gearbox_prbs_control_1 = RU_FIELD_SET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_1, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_MODE, reg_gearbox_prbs_control_1, cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_1_mode);
    reg_gearbox_prbs_control_1 = RU_FIELD_SET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_1, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_TIMER_VAL, reg_gearbox_prbs_control_1, cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_1_timer_val);

    RU_REG_WRITE(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_1, reg_gearbox_prbs_control_1);

    return 0;
}

int ag_drv_gpon_gearbox_status_gearbox_prbs_control_1_get(uint8_t *cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_1_en, uint8_t *cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_1_mode, uint32_t *cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_1_timer_val)
{
    uint32_t reg_gearbox_prbs_control_1=0;

#ifdef VALIDATE_PARMS
    if(!cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_1_en || !cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_1_mode || !cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_1_timer_val)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_1, reg_gearbox_prbs_control_1);

    *cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_1_en = RU_FIELD_GET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_1, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_EN, reg_gearbox_prbs_control_1);
    *cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_1_mode = RU_FIELD_GET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_1, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_MODE, reg_gearbox_prbs_control_1);
    *cr_xgwan_top_wan_misc_gpon_gearbox_rg_prbs_chk_ctrl_1_timer_val = RU_FIELD_GET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_CONTROL_1, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_TIMER_VAL, reg_gearbox_prbs_control_1);

    return 0;
}

int ag_drv_gpon_gearbox_status_gearbox_prbs_status_0_get(uint32_t *gpon_gearbox_prbs_stat_0_vector)
{
    uint32_t reg_gearbox_prbs_status_0=0;

#ifdef VALIDATE_PARMS
    if(!gpon_gearbox_prbs_stat_0_vector)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_STATUS_0, reg_gearbox_prbs_status_0);

    *gpon_gearbox_prbs_stat_0_vector = RU_FIELD_GET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_STATUS_0, GPON_GEARBOX_PRBS_STAT_0_VECTOR, reg_gearbox_prbs_status_0);

    return 0;
}

int ag_drv_gpon_gearbox_status_gearbox_prbs_status_1_get(uint8_t *gpon_gearbox_prbs_stat_1_vector)
{
    uint32_t reg_gearbox_prbs_status_1=0;

#ifdef VALIDATE_PARMS
    if(!gpon_gearbox_prbs_stat_1_vector)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_STATUS_1, reg_gearbox_prbs_status_1);

    *gpon_gearbox_prbs_stat_1_vector = RU_FIELD_GET(0, GPON_GEARBOX_STATUS, GEARBOX_PRBS_STATUS_1, GPON_GEARBOX_PRBS_STAT_1_VECTOR, reg_gearbox_prbs_status_1);

    return 0;
}

