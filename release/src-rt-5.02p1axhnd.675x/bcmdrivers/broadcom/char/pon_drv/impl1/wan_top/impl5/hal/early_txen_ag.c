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
#include "early_txen_ag.h"
int ag_drv_early_txen_txen_set(const early_txen_txen *txen)
{
    uint32_t reg_txen_cfg=0;

#ifdef VALIDATE_PARMS
    if(!txen)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((txen->cr_xgwan_top_wan_misc_early_txen_cfg_output_txen_polarity >= _1BITS_MAX_VAL_) ||
       (txen->cr_xgwan_top_wan_misc_early_txen_cfg_input_txen_polarity >= _1BITS_MAX_VAL_) ||
       (txen->cr_xgwan_top_wan_misc_early_txen_cfg_early_txen_bypass >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_txen_cfg = RU_FIELD_SET(0, EARLY_TXEN, TXEN_CFG, HOLD_TIME, reg_txen_cfg, txen->cr_xgwan_top_wan_misc_early_txen_cfg_hold_time);
    reg_txen_cfg = RU_FIELD_SET(0, EARLY_TXEN, TXEN_CFG, SETUP_TIME, reg_txen_cfg, txen->cr_xgwan_top_wan_misc_early_txen_cfg_setup_time);
    reg_txen_cfg = RU_FIELD_SET(0, EARLY_TXEN, TXEN_CFG, TOFF_TIME, reg_txen_cfg, txen->cr_xgwan_top_wan_misc_early_txen_cfg_toff_time);
    reg_txen_cfg = RU_FIELD_SET(0, EARLY_TXEN, TXEN_CFG, OUTPUT_TXEN_POLARITY, reg_txen_cfg, txen->cr_xgwan_top_wan_misc_early_txen_cfg_output_txen_polarity);
    reg_txen_cfg = RU_FIELD_SET(0, EARLY_TXEN, TXEN_CFG, INPUT_TXEN_POLARITY, reg_txen_cfg, txen->cr_xgwan_top_wan_misc_early_txen_cfg_input_txen_polarity);
    reg_txen_cfg = RU_FIELD_SET(0, EARLY_TXEN, TXEN_CFG, EARLY_TXEN_BYPASS, reg_txen_cfg, txen->cr_xgwan_top_wan_misc_early_txen_cfg_early_txen_bypass);

    RU_REG_WRITE(0, EARLY_TXEN, TXEN_CFG, reg_txen_cfg);

    return 0;
}

int ag_drv_early_txen_txen_get(early_txen_txen *txen)
{
    uint32_t reg_txen_cfg=0;

#ifdef VALIDATE_PARMS
    if(!txen)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, EARLY_TXEN, TXEN_CFG, reg_txen_cfg);

    txen->cr_xgwan_top_wan_misc_early_txen_cfg_hold_time = RU_FIELD_GET(0, EARLY_TXEN, TXEN_CFG, HOLD_TIME, reg_txen_cfg);
    txen->cr_xgwan_top_wan_misc_early_txen_cfg_setup_time = RU_FIELD_GET(0, EARLY_TXEN, TXEN_CFG, SETUP_TIME, reg_txen_cfg);
    txen->cr_xgwan_top_wan_misc_early_txen_cfg_toff_time = RU_FIELD_GET(0, EARLY_TXEN, TXEN_CFG, TOFF_TIME, reg_txen_cfg);
    txen->cr_xgwan_top_wan_misc_early_txen_cfg_output_txen_polarity = RU_FIELD_GET(0, EARLY_TXEN, TXEN_CFG, OUTPUT_TXEN_POLARITY, reg_txen_cfg);
    txen->cr_xgwan_top_wan_misc_early_txen_cfg_input_txen_polarity = RU_FIELD_GET(0, EARLY_TXEN, TXEN_CFG, INPUT_TXEN_POLARITY, reg_txen_cfg);
    txen->cr_xgwan_top_wan_misc_early_txen_cfg_early_txen_bypass = RU_FIELD_GET(0, EARLY_TXEN, TXEN_CFG, EARLY_TXEN_BYPASS, reg_txen_cfg);

    return 0;
}

