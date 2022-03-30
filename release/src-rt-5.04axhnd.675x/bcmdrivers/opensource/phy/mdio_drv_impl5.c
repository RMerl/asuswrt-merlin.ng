/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2016:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
:>
*/

/*
 *  Created on: June 2017
 *      Author: dima.mamut@broadcom.com
 */

/*
 * MDIO driver for BCM96846 6856 and 6878
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

#define MDIO_CMD        (void *)mdio_base

static DEFINE_SPINLOCK(mdio_access);

static void mdio_cfg_type(mdio_type_t type)
{
    if (type == MDIO_INT)
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


