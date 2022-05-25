/*
   Copyright (c) 2013 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2013:DUAL/GPL:standard

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

/*
 *  Created on: Nov 2016
 *      Author: steven.hsieh@broadcom.com
 */

/*
 * MDIO driver for StarFighter2 - 4908/63138/63148/63158
 */

#include "mdio_drv_sf2.h"
#include "dt_access.h"

#include <linux/spinlock.h>
DEFINE_SPINLOCK(mdio_access);

static uintptr_t switch_mdio_base;
static uintptr_t switch_reg_base;

static int mdio_probe(dt_device_t *pdev)
{
    int ret;

    switch_mdio_base = dt_dev_remap_resource(pdev, 0);
    if (IS_ERR(switch_mdio_base))
    {
        ret = PTR_ERR(switch_mdio_base);
        switch_mdio_base = NULL;
        dev_err(&pdev->dev, "Missing switch_mdio_base entry\n");
        goto Exit;
    }
    dev_dbg(&pdev->dev, "switch_mdio_base=0x%lx\n", switch_mdio_base);

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908)
    switch_reg_base = dt_dev_remap_resource(pdev, 1);
    if (IS_ERR(switch_reg_base))
    {
        ret = PTR_ERR(switch_reg_base);
        switch_reg_base = NULL;
        dev_err(&pdev->dev, "Missing switch_reg_base entry\n");
        goto Exit;
    }
    dev_dbg(&pdev->dev, "switch_reg_base=0x%lx\n", switch_reg_base);
#endif

    dev_info(&pdev->dev, "registered\n");

    return 0;

Exit:
    return ret;
}

static const struct of_device_id of_platform_table[] = {
    { .compatible = "brcm,mdio-sf2" },
    { /* end of list */ },
};

static struct platform_driver of_platform_driver = {
    .driver = {
        .name = "brcm-mdio",
        .of_match_table = of_platform_table,
    },
    .probe = mdio_probe,
};
module_platform_driver(of_platform_driver);

#define SWITCH_MDIO_BASE        (void *)switch_mdio_base
#define SWITCH_REG_BASE         (void *)switch_reg_base

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908)
#define SF2_MDIO_MASTER                     0x01

static void sf2_mdio_master_enable(void)
{
    volatile uint32_t *sw_ctrl_reg = (void *)(SWITCH_REG_BASE);
    uint32_t val32 = *sw_ctrl_reg;
    
    val32 |= SF2_MDIO_MASTER;
    *sw_ctrl_reg = val32;
}

static void sf2_mdio_master_disable(void)
{
    volatile uint32_t *sw_ctrl_reg = (void *)(SWITCH_REG_BASE);
    uint32_t val32 = *sw_ctrl_reg;
    val32 &= ~SF2_MDIO_MASTER;
    *sw_ctrl_reg = val32;
}
#else
#define sf2_mdio_master_enable()
#define sf2_mdio_master_disable()
#endif

int32_t mdio_read_c22_register(mdio_type_t type, uint32_t addr, uint32_t reg, uint16_t *val)
{
    int ret = MDIO_OK;

    spin_lock_bh(&mdio_access);
    sf2_mdio_master_disable();

    /* Read twice for DSL based chip requested by hardware */
    ret = mdio_cmd_read_22((void *)SWITCH_MDIO_BASE, addr, reg, val);
    ret += mdio_cmd_read_22((void *)SWITCH_MDIO_BASE, addr, reg, val);

    sf2_mdio_master_enable();
    spin_unlock_bh(&mdio_access);

    return ret;
}
EXPORT_SYMBOL(mdio_read_c22_register);

int32_t mdio_write_c22_register(mdio_type_t type, uint32_t addr, uint32_t reg, uint16_t val)
{
    int ret = MDIO_OK;

    spin_lock_bh(&mdio_access);
    sf2_mdio_master_disable();

    ret = mdio_cmd_write_22((void *)SWITCH_MDIO_BASE, addr, reg, val);

    sf2_mdio_master_enable();
    spin_unlock_bh(&mdio_access);

    return ret;
}
EXPORT_SYMBOL(mdio_write_c22_register);

int32_t mdio_read_c45_register(mdio_type_t type, uint32_t addr, uint32_t dev, uint16_t reg, uint16_t *val)
{
    int ret = MDIO_OK;

    spin_lock_bh(&mdio_access);
    sf2_mdio_master_disable();

    ret = mdio_cmd_read_45((void *)SWITCH_MDIO_BASE, addr, dev, reg, val);

    sf2_mdio_master_enable();
    spin_unlock_bh(&mdio_access);

    return ret;
}
EXPORT_SYMBOL(mdio_read_c45_register);

int32_t mdio_write_c45_register(mdio_type_t type, uint32_t addr, uint32_t dev, uint16_t reg, uint16_t val)
{
    int ret = MDIO_OK;

    spin_lock_bh(&mdio_access);
    sf2_mdio_master_disable();

    ret = mdio_cmd_write_45((void *)SWITCH_MDIO_BASE, addr, dev, reg, val);

    sf2_mdio_master_enable();
    spin_unlock_bh(&mdio_access);

    return ret;
}
EXPORT_SYMBOL(mdio_write_c45_register);
