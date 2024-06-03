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
 * MDIO driver for PON chips
 */

#include "os_dep.h"
#include "mdio_drv_v1.h"
#include "dt_access.h"
#include "pmc_mdio.h"

static void __iomem * mdio_base;

#define MDIO_CMD (void *)(mdio_base)
#define MDIO_CFG (void *)(mdio_base + 4)

uint16_t slow_clock_divider;
uint16_t fast_clock_divider;

static int mdio_probe(dt_device_t *pdev)
{
    int ret;
    dt_handle_t dt_handle = dt_dev_get_handle(pdev);

    slow_clock_divider = dt_property_read_u32_default(dt_handle, "clock-divider", 12);
    fast_clock_divider = dt_property_read_u32_default(dt_handle, "clock-divider-fast", 4);

    mdio_base = dt_dev_remap(pdev, 0);
    if (IS_ERR(mdio_base))
    {
        ret = PTR_ERR(mdio_base);
        mdio_base = NULL;
        dev_err(&pdev->dev, "Missing mdio_base entry\n");
        goto Exit;
    }

    ret = pmc_mdio_power_up();
    if (ret)
    {
        mdio_base = NULL;
        dev_err(&pdev->dev, "Failed to power ON the MDIO block\n");
        goto Exit;
    }

    dev_dbg(&pdev->dev, "mdio_base=0x%px\n", mdio_base);
    dev_info(&pdev->dev, "clock-divider=%d\n", slow_clock_divider);
    dev_info(&pdev->dev, "clock-divider-fast=%d\n", fast_clock_divider);
    dev_info(&pdev->dev, "registered\n");

    return mdio_cfg_set(MDIO_CFG, 0, slow_clock_divider);

Exit:
    return ret;
}

static const struct udevice_id mdio_ids[] = {
#if defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
    { .compatible = "brcm,mdio-sf2" },      //FIXME! redirect mdio-sf2 to mdio5 for now
#else
    { .compatible = "brcm,mdio1" },
#endif
    { /* end of list */ },
};

U_BOOT_DRIVER(brcm_mdio) = {
    .name	= "brcm-mdio1",
    .id	= UCLASS_MISC,
    .of_match = mdio_ids,
    .probe = mdio_probe,
};

static DEFINE_SPINLOCK(mdio_access);

int32_t mdio_config_get(uint16_t *probe_mode, uint16_t *fast_mode)
{
    uint16_t _probe_mode, _clock_divider;

    mdio_cfg_get(MDIO_CFG, &_probe_mode, &_clock_divider);

    *probe_mode = _probe_mode;
    *fast_mode = _clock_divider != slow_clock_divider;

    return 0;
}

int32_t mdio_config_set(uint16_t probe_mode, uint16_t fast_mode)
{
    uint16_t _probe_mode, _clock_divider;

    _probe_mode = probe_mode;
    _clock_divider = fast_mode ? fast_clock_divider : slow_clock_divider;

    mdio_cfg_set(MDIO_CFG, _probe_mode, _clock_divider);

    return 0;
}

int32_t mdio_read_c22_register(uint32_t addr, uint32_t reg, uint16_t *val)
{
    int ret = MDIO_OK;

    spin_lock_bh(&mdio_access);
    ret =  mdio_cmd_read_22(MDIO_CMD, addr, reg, val);
    spin_unlock_bh(&mdio_access);

    return ret;
}
EXPORT_SYMBOL(mdio_read_c22_register);

int32_t mdio_write_c22_register(uint32_t addr, uint32_t reg, uint16_t val)
{
    int ret = MDIO_OK;

    spin_lock_bh(&mdio_access);
    ret = mdio_cmd_write_22(MDIO_CMD, addr, reg, val);
    spin_unlock_bh(&mdio_access);

    return ret;
}
EXPORT_SYMBOL(mdio_write_c22_register);

int32_t mdio_read_c45_register(uint32_t addr, uint32_t dev, uint16_t reg, uint16_t *val)
{
    int ret = MDIO_OK;

    spin_lock_bh(&mdio_access);
    ret = mdio_cmd_read_45(MDIO_CMD, addr, dev, reg, val);
    spin_unlock_bh(&mdio_access);

    return ret;
}
EXPORT_SYMBOL(mdio_read_c45_register);

int32_t mdio_comp_read_c45_register(uint32_t addr, uint32_t dev, uint16_t reg, uint16_t *val, int regIn[], int regOut[])
{
    int ret = MDIO_OK;
    int i;

    spin_lock_bh(&mdio_access);

    for (i=0; regIn[i] != -1; i += 3)
        ret += mdio_cmd_write_45(MDIO_CMD, addr, regIn[i], regIn[i+1], regIn[i+2]);

    ret += mdio_cmd_read_45(MDIO_CMD, addr, dev, reg, val);

    for (i=0; regOut[i] != -1; i += 3)
        ret += mdio_cmd_write_45(MDIO_CMD, addr, regOut[i], regOut[i+1], regOut[i+2]);

    spin_unlock_bh(&mdio_access);

    return ret;
}
EXPORT_SYMBOL(mdio_comp_read_c45_register);

int32_t mdio_write_c45_register(uint32_t addr, uint32_t dev, uint16_t reg, uint16_t val)
{
    int ret = MDIO_OK;

    spin_lock_bh(&mdio_access);
    ret = mdio_cmd_write_45(MDIO_CMD, addr, dev, reg, val);
    spin_unlock_bh(&mdio_access);

    return ret;
}
EXPORT_SYMBOL(mdio_write_c45_register);

int32_t mdio_comp_write_c45_register(uint32_t addr, uint32_t dev, uint16_t reg, uint16_t val, int regIn[], int regOut[])
{
    int ret = MDIO_OK;
    int i;

    spin_lock_bh(&mdio_access);

    for (i=0; regIn[i] != -1; i += 3)
        ret += mdio_cmd_write_45(MDIO_CMD, addr, regIn[i], regIn[i+1], regIn[i+2]);

    ret += mdio_cmd_write_45(MDIO_CMD, addr, dev, reg, val);

    for (i=0; regOut[i] != -1; i += 3)
        ret += mdio_cmd_write_45(MDIO_CMD, addr, regOut[i], regOut[i+1], regOut[i+2]);

    spin_unlock_bh(&mdio_access);

    return ret;
}
EXPORT_SYMBOL(mdio_comp_write_c45_register);


