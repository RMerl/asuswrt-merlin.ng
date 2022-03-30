// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved

    
*/

/*
 *  Created on: June 2017
 *      Author: dima.mamut@broadcom.com
 */

/*
 * MDIO driver for BCM96846 6856  6878 96855
 */

#include "os_dep.h"
#include "mdio_drv_impl5.h"
#include "dt_access.h"

static uintptr_t mdio_base;
static uintptr_t pMdioMasterSelect;

static int mdio_probe(dt_device_t *pdev)
{
    int ret;

    mdio_base = dt_dev_remap_resource(pdev, 0);
    if (IS_ERR(mdio_base))
    {
        ret = PTR_ERR(mdio_base);
        mdio_base = NULL;
        dev_err(&pdev->dev, "Missing mdio_base entry\n");
        goto Exit;
    }

    pMdioMasterSelect = dt_dev_remap_resource(pdev, 1);
    if (IS_ERR(pMdioMasterSelect))
    {
        ret = PTR_ERR(pMdioMasterSelect);
        pMdioMasterSelect = NULL;
        dev_err(&pdev->dev, "Missing MdioMasterSelect entry\n");
        goto Exit;
    }

    dev_dbg(&pdev->dev, "mdio_base=0x%lx\n", mdio_base);
    dev_dbg(&pdev->dev, "pMdioMasterSelect=0x%lx\n", pMdioMasterSelect);

    dev_info(&pdev->dev, "registered\n");

    return 0;

Exit:
    return ret;
}

#ifdef __UBOOT__
static const struct udevice_id mdio_ids[] = {
#if defined(CONFIG_BCM63146) || defined(CONFIG_BCM4912) || defined(CONFIG_BCM6813)
    { .compatible = "brcm,mdio-sf2" },      //FIXME! redirect mdio-sf2 to mdio5 for now
#else
    { .compatible = "brcm,mdio5" },
#endif
    { /* end of list */ },
};

U_BOOT_DRIVER(brcm_mdio) = {
    .name	= "brcm-mdio",
    .id	= UCLASS_MISC,
    .of_match = mdio_ids,
    .probe = mdio_probe,
};
#else
static const struct of_device_id of_platform_table[] = {
    { .compatible = "brcm,mdio5" },
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
#endif

#define MDIO_CMD        (void *)mdio_base

static DEFINE_SPINLOCK(mdio_access);

static void mdio_cfg_type(mdio_type_t type)
{
#if defined(CONFIG_BCM63146) || defined(CONFIG_BCM4912) || defined(CONFIG_BCM6813)
    if (type != MDIO_INT)
#else
    if (type == MDIO_INT)
#endif
        *(uint32_t *)pMdioMasterSelect = 0;
    else
        *(uint32_t *)pMdioMasterSelect = 1;
}

int32_t mdio_read_c22_register(mdio_type_t type, uint32_t addr, uint32_t reg, uint16_t *val)
{
    int ret = MDIO_OK;

    spin_lock_bh(&mdio_access);
    mdio_cfg_type(type);
    ret =  mdio_cmd_read_22(MDIO_CMD, addr, reg, val);
    spin_unlock_bh(&mdio_access);

    return ret;
}
EXPORT_SYMBOL(mdio_read_c22_register);

int32_t mdio_write_c22_register(mdio_type_t type, uint32_t addr, uint32_t reg, uint16_t val)
{
    int ret = MDIO_OK;

    spin_lock_bh(&mdio_access);
    mdio_cfg_type(type);
    ret = mdio_cmd_write_22(MDIO_CMD, addr, reg, val);
    spin_unlock_bh(&mdio_access);

    return ret;
}
EXPORT_SYMBOL(mdio_write_c22_register);

int32_t mdio_read_c45_register(mdio_type_t type, uint32_t addr, uint32_t dev, uint16_t reg, uint16_t *val)
{
    int ret = MDIO_OK;

    spin_lock_bh(&mdio_access);
    mdio_cfg_type(type);
    ret = mdio_cmd_read_45(MDIO_CMD, addr, dev, reg, val);
    spin_unlock_bh(&mdio_access);

    return ret;
}
EXPORT_SYMBOL(mdio_read_c45_register);

int32_t mdio_write_c45_register(mdio_type_t type, uint32_t addr, uint32_t dev, uint16_t reg, uint16_t val)
{
    int ret = MDIO_OK;

    spin_lock_bh(&mdio_access);
    mdio_cfg_type(type);
    ret = mdio_cmd_write_45(MDIO_CMD, addr, dev, reg, val);
    spin_unlock_bh(&mdio_access);

    return ret;
}
EXPORT_SYMBOL(mdio_write_c45_register);


