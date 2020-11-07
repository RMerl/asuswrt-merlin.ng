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
    if((cfg->ctrl >= _13BITS_MAX_VAL_) ||
       (cfg->pwrdn >= _1BITS_MAX_VAL_) ||
       (cfg->diag_on >= _1BITS_MAX_VAL_) ||
       (cfg->rstb >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_cfg = RU_FIELD_SET(0, RESCAL, CFG, CTRL, reg_cfg, cfg->ctrl);
    reg_cfg = RU_FIELD_SET(0, RESCAL, CFG, PWRDN, reg_cfg, cfg->pwrdn);
    reg_cfg = RU_FIELD_SET(0, RESCAL, CFG, DIAG_ON, reg_cfg, cfg->diag_on);
    reg_cfg = RU_FIELD_SET(0, RESCAL, CFG, RSTB, reg_cfg, cfg->rstb);

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

    cfg->ctrl = RU_FIELD_GET(0, RESCAL, CFG, CTRL, reg_cfg);
    cfg->pwrdn = RU_FIELD_GET(0, RESCAL, CFG, PWRDN, reg_cfg);
    cfg->diag_on = RU_FIELD_GET(0, RESCAL, CFG, DIAG_ON, reg_cfg);
    cfg->rstb = RU_FIELD_GET(0, RESCAL, CFG, RSTB, reg_cfg);

    return 0;
}

int ag_drv_rescal_status_0_get(rescal_status0 *status0)
{
    uint32_t reg_status0=0;

#ifdef VALIDATE_PARMS
    if(!status0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, RESCAL, STATUS0, reg_status0);

    status0->valid = RU_FIELD_GET(0, RESCAL, STATUS0, VALID, reg_status0);
    status0->comp = RU_FIELD_GET(0, RESCAL, STATUS0, COMP, reg_status0);
    status0->state = RU_FIELD_GET(0, RESCAL, STATUS0, STATE, reg_status0);
    status0->ctrl_dfs = RU_FIELD_GET(0, RESCAL, STATUS0, CTRL_DFS, reg_status0);
    status0->prev_comp_cnt = RU_FIELD_GET(0, RESCAL, STATUS0, PREV_COMP_CNT, reg_status0);
    status0->pon = RU_FIELD_GET(0, RESCAL, STATUS0, PON, reg_status0);
    status0->done = RU_FIELD_GET(0, RESCAL, STATUS0, DONE, reg_status0);

    return 0;
}

int ag_drv_rescal_status_1_get(uint8_t *curr_comp_cnt)
{
    uint32_t reg_status1=0;

#ifdef VALIDATE_PARMS
    if(!curr_comp_cnt)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, RESCAL, STATUS1, reg_status1);

    *curr_comp_cnt = RU_FIELD_GET(0, RESCAL, STATUS1, CURR_COMP_CNT, reg_status1);

    return 0;
}

