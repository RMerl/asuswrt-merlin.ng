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
#include "rescal_ag.h"
int ag_drv_rescal_cfg_set(const rescal_cfg *cfg)
{
    uint32_t reg_cfg=0;

#ifdef VALIDATE_PARMS
    if(!cfg)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((cfg->cfg_wan_rescal_rstb >= _1BITS_MAX_VAL_) ||
       (cfg->cfg_wan_rescal_diag_on >= _1BITS_MAX_VAL_) ||
       (cfg->cfg_wan_rescal_pwrdn >= _1BITS_MAX_VAL_) ||
       (cfg->cfg_wan_rescal_ctrl >= _13BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_cfg = RU_FIELD_SET(0, RESCAL, CFG, CFG_WAN_RESCAL_RSTB, reg_cfg, cfg->cfg_wan_rescal_rstb);
    reg_cfg = RU_FIELD_SET(0, RESCAL, CFG, CFG_WAN_RESCAL_DIAG_ON, reg_cfg, cfg->cfg_wan_rescal_diag_on);
    reg_cfg = RU_FIELD_SET(0, RESCAL, CFG, CFG_WAN_RESCAL_PWRDN, reg_cfg, cfg->cfg_wan_rescal_pwrdn);
    reg_cfg = RU_FIELD_SET(0, RESCAL, CFG, CFG_WAN_RESCAL_CTRL, reg_cfg, cfg->cfg_wan_rescal_ctrl);

    RU_REG_WRITE(0, RESCAL, CFG, reg_cfg);

    return 0;
}

int ag_drv_rescal_cfg_get(rescal_cfg *cfg)
{
    uint32_t reg_cfg=0;

#ifdef VALIDATE_PARMS
    if(!cfg)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, RESCAL, CFG, reg_cfg);

    cfg->cfg_wan_rescal_rstb = RU_FIELD_GET(0, RESCAL, CFG, CFG_WAN_RESCAL_RSTB, reg_cfg);
    cfg->cfg_wan_rescal_diag_on = RU_FIELD_GET(0, RESCAL, CFG, CFG_WAN_RESCAL_DIAG_ON, reg_cfg);
    cfg->cfg_wan_rescal_pwrdn = RU_FIELD_GET(0, RESCAL, CFG, CFG_WAN_RESCAL_PWRDN, reg_cfg);
    cfg->cfg_wan_rescal_ctrl = RU_FIELD_GET(0, RESCAL, CFG, CFG_WAN_RESCAL_CTRL, reg_cfg);

    return 0;
}

int ag_drv_rescal_status_0_get(rescal_status_0 *status_0)
{
    uint32_t reg_status_0=0;

#ifdef VALIDATE_PARMS
    if(!status_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, RESCAL, STATUS_0, reg_status_0);

    status_0->wan_rescal_done = RU_FIELD_GET(0, RESCAL, STATUS_0, WAN_RESCAL_DONE, reg_status_0);
    status_0->wan_rescal_pon = RU_FIELD_GET(0, RESCAL, STATUS_0, WAN_RESCAL_PON, reg_status_0);
    status_0->wan_rescal_prev_comp_cnt = RU_FIELD_GET(0, RESCAL, STATUS_0, WAN_RESCAL_PREV_COMP_CNT, reg_status_0);
    status_0->wan_rescal_ctrl_dfs = RU_FIELD_GET(0, RESCAL, STATUS_0, WAN_RESCAL_CTRL_DFS, reg_status_0);
    status_0->wan_rescal_state = RU_FIELD_GET(0, RESCAL, STATUS_0, WAN_RESCAL_STATE, reg_status_0);
    status_0->wan_rescal_comp = RU_FIELD_GET(0, RESCAL, STATUS_0, WAN_RESCAL_COMP, reg_status_0);
    status_0->wan_rescal_valid = RU_FIELD_GET(0, RESCAL, STATUS_0, WAN_RESCAL_VALID, reg_status_0);

    return 0;
}

int ag_drv_rescal_status_1_get(uint8_t *wan_rescal_curr_comp_cnt)
{
    uint32_t reg_status_1=0;

#ifdef VALIDATE_PARMS
    if(!wan_rescal_curr_comp_cnt)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, RESCAL, STATUS_1, reg_status_1);

    *wan_rescal_curr_comp_cnt = RU_FIELD_GET(0, RESCAL, STATUS_1, WAN_RESCAL_CURR_COMP_CNT, reg_status_1);

    return 0;
}

