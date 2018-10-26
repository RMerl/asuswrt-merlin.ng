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

#include "bcm6858_drivers_lport_ag.h"
#include "bcm6858_lport_mdio_ag.h"
int ag_drv_lport_mdio_control_set(const lport_mdio_control *control)
{
    uint32_t reg_cmd=0;

#ifdef VALIDATE_PARMS
    if(!control)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((control->start_busy >= _1BITS_MAX_VAL_) ||
       (control->fail >= _1BITS_MAX_VAL_) ||
       (control->op_code >= _2BITS_MAX_VAL_) ||
       (control->phy_prt_addr >= _5BITS_MAX_VAL_) ||
       (control->reg_dev_addr >= _5BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_cmd = RU_FIELD_SET(0, LPORT_MDIO, CMD, START_BUSY, reg_cmd, control->start_busy);
    reg_cmd = RU_FIELD_SET(0, LPORT_MDIO, CMD, FAIL, reg_cmd, control->fail);
    reg_cmd = RU_FIELD_SET(0, LPORT_MDIO, CMD, OP_CODE, reg_cmd, control->op_code);
    reg_cmd = RU_FIELD_SET(0, LPORT_MDIO, CMD, PHY_PRT_ADDR, reg_cmd, control->phy_prt_addr);
    reg_cmd = RU_FIELD_SET(0, LPORT_MDIO, CMD, REG_DEV_ADDR, reg_cmd, control->reg_dev_addr);
    reg_cmd = RU_FIELD_SET(0, LPORT_MDIO, CMD, DATA_ADDR, reg_cmd, control->data_addr);

    RU_REG_WRITE(0, LPORT_MDIO, CMD, reg_cmd);

    return 0;
}

int ag_drv_lport_mdio_control_get(lport_mdio_control *control)
{
    uint32_t reg_cmd=0;

#ifdef VALIDATE_PARMS
    if(!control)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_MDIO, CMD, reg_cmd);

    control->start_busy = RU_FIELD_GET(0, LPORT_MDIO, CMD, START_BUSY, reg_cmd);
    control->fail = RU_FIELD_GET(0, LPORT_MDIO, CMD, FAIL, reg_cmd);
    control->op_code = RU_FIELD_GET(0, LPORT_MDIO, CMD, OP_CODE, reg_cmd);
    control->phy_prt_addr = RU_FIELD_GET(0, LPORT_MDIO, CMD, PHY_PRT_ADDR, reg_cmd);
    control->reg_dev_addr = RU_FIELD_GET(0, LPORT_MDIO, CMD, REG_DEV_ADDR, reg_cmd);
    control->data_addr = RU_FIELD_GET(0, LPORT_MDIO, CMD, DATA_ADDR, reg_cmd);

    return 0;
}

int ag_drv_lport_mdio_cfg_set(uint8_t free_run_clk_enable, uint8_t supress_preamble, uint8_t mdio_clk_divider, uint8_t mdio_clause)
{
    uint32_t reg_cfg=0;

#ifdef VALIDATE_PARMS
    if((supress_preamble >= _1BITS_MAX_VAL_) ||
       (mdio_clause >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_cfg = RU_FIELD_SET(0, LPORT_MDIO, CFG, FREE_RUN_CLK_ENABLE, reg_cfg, free_run_clk_enable);
    reg_cfg = RU_FIELD_SET(0, LPORT_MDIO, CFG, SUPRESS_PREAMBLE, reg_cfg, supress_preamble);
    reg_cfg = RU_FIELD_SET(0, LPORT_MDIO, CFG, MDIO_CLK_DIVIDER, reg_cfg, mdio_clk_divider);
    reg_cfg = RU_FIELD_SET(0, LPORT_MDIO, CFG, MDIO_CLAUSE, reg_cfg, mdio_clause);

    RU_REG_WRITE(0, LPORT_MDIO, CFG, reg_cfg);

    return 0;
}

int ag_drv_lport_mdio_cfg_get(uint8_t *free_run_clk_enable, uint8_t *supress_preamble, uint8_t *mdio_clk_divider, uint8_t *mdio_clause)
{
    uint32_t reg_cfg=0;

#ifdef VALIDATE_PARMS
    if(!supress_preamble || !mdio_clk_divider || !mdio_clause)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_MDIO, CFG, reg_cfg);

    *free_run_clk_enable = RU_FIELD_GET(0, LPORT_MDIO, CFG, FREE_RUN_CLK_ENABLE, reg_cfg);
    *supress_preamble = RU_FIELD_GET(0, LPORT_MDIO, CFG, SUPRESS_PREAMBLE, reg_cfg);
    *mdio_clk_divider = RU_FIELD_GET(0, LPORT_MDIO, CFG, MDIO_CLK_DIVIDER, reg_cfg);
    *mdio_clause = RU_FIELD_GET(0, LPORT_MDIO, CFG, MDIO_CLAUSE, reg_cfg);

    return 0;
}

