// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved


*/

/*
 *  Created on: Apr 2021
 *      Author: yuval.raviv@broadcom.com
 */

/*
 * Merlin SerDes registers access for 6858 and 6888
 */


#ifndef __SERDES_ACCESS_C__
#define __SERDES_ACCESS_C__
#endif

#include "phy_drv.h"
#include "serdes_access.h"
#include "dt_access.h"
#include <linux/delay.h>

#define WRITE_32(a, r)              (*(volatile uint32_t*)(a) = *(uint32_t*)&(r))
#define READ_32(a, r)               (*(volatile uint32_t*)&(r) = *(volatile uint32_t*)(a))

#define SERDES_BASE                 serdes_base

static void __iomem *serdes_base;

static int serdes_probe(dt_device_t *pdev)
{
    int ret;

    serdes_base = dt_dev_remap(pdev, 0);
    if (IS_ERR(serdes_base))
    {
        ret = PTR_ERR(serdes_base);
        serdes_base = NULL;
        dev_err(&pdev->dev, "Missing serdes_base entry\n");
        goto Exit;
    }

    dev_dbg(&pdev->dev, "serdes_base=0x%p\n", serdes_base);
    dev_info(&pdev->dev, "registered\n");

    return 0;

Exit:
    return ret;
}

static const struct udevice_id serdes_ids[] = {
    { .compatible = "brcm,serdes1" },
    { /* end of list */ },
};

U_BOOT_DRIVER(brcm_serdes) = {
    .name	= "brcm-serdes1",
    .id	= UCLASS_MISC,
    .of_match = serdes_ids,
    .probe = serdes_probe,
};

#if 0
static void serdes_control_read(phy_dev_t *phy_dev, serdes_control_t *serdes_control)
{
    uint8_t core_id = phy_dev->core_index;

    READ_32(SERDES_BASE + SERDES_CONTROL[core_id], *serdes_control);
}
#endif

static void serdes_control_write(phy_dev_t *phy_dev, serdes_control_t *serdes_control)
{
    uint8_t core_id = phy_dev->core_index;

    WRITE_32(SERDES_BASE + SERDES_CONTROL[core_id], *serdes_control);
}

static void serdes_status_read(phy_dev_t *phy_dev, serdes_status_t *serdes_status)
{
    uint8_t core_id = phy_dev->core_index;

    READ_32(SERDES_BASE + SERDES_STATUS[core_id], *serdes_status);
}

static void serdes_an_status_read(phy_dev_t *phy_dev, serdes_an_status_t *serdes_an_status)
{
    uint8_t core_id = phy_dev->core_index;

    READ_32(SERDES_BASE + SERDES_AN_STATUS[core_id], *serdes_an_status);
}

static void serdes_indirect_access_control_read(phy_dev_t *phy_dev,
    serdes_indirect_access_control_t *serdes_indirect_access_control)
{
    uint8_t core_id = phy_dev->core_index;

    READ_32(SERDES_BASE + SERDES_INDIR_ACC_CNTRL[core_id], *serdes_indirect_access_control);
}

static void serdes_indirect_access_control_write(phy_dev_t *phy_dev,
    serdes_indirect_access_control_t *serdes_indirect_access_control)
{
    uint8_t core_id = phy_dev->core_index;

    WRITE_32(SERDES_BASE + SERDES_INDIR_ACC_CNTRL[core_id], *serdes_indirect_access_control);
}

static void serdes_indirect_access_address_write(phy_dev_t *phy_dev, uint32_t val)
{
    uint8_t core_id = phy_dev->core_index;

    WRITE_32(SERDES_BASE + SERDES_INDIR_ACC_ADDR[core_id], val);
}

static void serdes_indirect_access_mask_write(phy_dev_t *phy_dev, uint32_t val)
{
    uint8_t core_id = phy_dev->core_index;

    WRITE_32(SERDES_BASE + SERDES_INDIR_ACC_MASK[core_id], val);
}

static void serdes_status_1_read(phy_dev_t *phy_dev, serdes_status_1_t *serdes_status_1)
{
    uint8_t core_id = phy_dev->core_index;

    READ_32(SERDES_BASE + SERDES_STATUS_1[core_id], *serdes_status_1);
}

static inline uint32_t serdes_indirect_access_encode_address(uint8_t lane_id, uint16_t dev, uint16_t reg)
{
    return (dev << DEV_TYPE_OFFSET) | (lane_id << LANE_ADDRESS_OFFSET) | reg;
}

int serdes_access_read_mask(phy_dev_t *phy_dev, uint16_t dev, uint16_t reg, uint16_t mask, uint16_t shift, uint16_t *val)
{
    uint8_t core_id = phy_dev->core_index;
    uint8_t lane_id = phy_dev->lane_index;
    serdes_indirect_access_control_t serdes_indirect_access_control;
    uint32_t addr = serdes_indirect_access_encode_address(lane_id, dev, reg);

    serdes_indirect_access_control_init(&serdes_indirect_access_control, 1, 0); /* Means read operation */
    serdes_indirect_access_address_write(phy_dev, addr);
    serdes_indirect_access_control_write(phy_dev, &serdes_indirect_access_control);

    udelay(10);

    /* Validate no error after reading */
    serdes_indirect_access_control_read(phy_dev, &serdes_indirect_access_control);

    if (serdes_indirect_access_control_status(&serdes_indirect_access_control))
    {
        pr_err("Error while validating read from SerDes %d: busy=%d\n",
            core_id, serdes_indirect_access_control.start_busy);
        return -1;
    }

    *val = serdes_indirect_access_control.reg_data & mask;

    return 0;
}

int serdes_access_write_mask(phy_dev_t *phy_dev, uint16_t dev, uint16_t reg, uint16_t mask, uint16_t shift, uint16_t val)
{
    uint8_t core_id = phy_dev->core_index;
    uint8_t lane_id = phy_dev->lane_index;
    serdes_indirect_access_control_t serdes_indirect_access_control;
    uint32_t addr = serdes_indirect_access_encode_address(lane_id, dev, reg);

    serdes_indirect_access_control_init(&serdes_indirect_access_control, 0, val << shift); /* Means write operation */
    serdes_indirect_access_address_write(phy_dev, addr);
    serdes_indirect_access_mask_write(phy_dev, ~mask);
    serdes_indirect_access_control_write(phy_dev, &serdes_indirect_access_control);

    udelay(10);

    /* Validate no error after writing */
    serdes_indirect_access_control_read(phy_dev, &serdes_indirect_access_control);

    if (serdes_indirect_access_control_status(&serdes_indirect_access_control))
    {
        pr_err("Error while validating write to SerDes %d: busy=%d\n",
            core_id, serdes_indirect_access_control.start_busy);
        return -1;
    }

    return 0;
}

int serdes_access_read(phy_dev_t *phy_dev, uint16_t dev, uint16_t reg, uint16_t *val)
{
    return serdes_access_read_mask(phy_dev, dev, reg, MASK_ALL_BITS_16, 0, val); 
}

int serdes_access_write(phy_dev_t *phy_dev, uint16_t dev, uint16_t reg, uint16_t val)
{
    return serdes_access_write_mask(phy_dev, dev, reg, MASK_ALL_BITS_16, 0, val); 
}

#define SERDES_ACCESS_RETRIES 100
int serdes_access_wait_pll_lock(phy_dev_t *phy_dev)
{
    serdes_status_t serdes_status = {};
    uint8_t core_id = phy_dev->core_index;
    uint32_t retries = SERDES_ACCESS_RETRIES;

    do
    {
        mdelay(10);
        serdes_status_read(phy_dev, &serdes_status);
    } while (!serdes_status.pll_lock && --retries);

    if (!serdes_status.pll_lock)
    {
        pr_err("Failed to get PLL lock in SerDes %d\n", core_id);
        return -1;
    }

    return 0;
}

int serdes_access_wait_link_status(phy_dev_t *phy_dev)
{
    serdes_status_t serdes_status = {};
    uint8_t core_id = phy_dev->core_index;
    uint8_t lane_id = phy_dev->lane_index;
    uint8_t mask = (1 << lane_id);
    uint32_t retries = SERDES_ACCESS_RETRIES;

    do
    {
        mdelay(10);
        serdes_status_read(phy_dev, &serdes_status);
    } while (!(serdes_status.link_status & mask) && --retries);

    if (!(serdes_status.link_status & mask))
    {
        pr_err("Failed to get link in SerDes %d Lane %d\n", core_id, lane_id);
        return -1;
    }

    return 0;
}

int serdes_access_config(phy_dev_t *phy_dev, int enable)
{
    serdes_control_t serdes_control = {0};
    int disable = !enable;

    /* Step 1 */
    serdes_control.iddq = 1;
    serdes_control.refclk_reset = 1;
    serdes_control.serdes_reset = 1;
#if defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880) || defined(CONFIG_BCM96837)
    serdes_control.ref_cmos_clk_hz = 1;
    serdes_control.pd_cml_refclk_chout = 1;
    serdes_control.pd_cml_lcrefout = 1;
    serdes_control.iso_enable = 1;
#endif
#if defined(CONFIG_BCM96813) || defined(CONFIG_BCM96765)
    serdes_control.comclk_enable = 1;
#endif
#if defined(PHY_SHARED_REF_CLK)
    if (phy_dev->shared_ref_clk_mhz)
    {
        serdes_control.serdes_testsel = 1;
        serdes_control.iso_enable = 0;
    }
    else
        serdes_control.serdes_testsel = 0;
#endif
    serdes_control_write(phy_dev, &serdes_control);
    mdelay(10);

    /* Step 2 */
    serdes_control.iddq = disable;
#if defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880) || defined(CONFIG_BCM96837)
    serdes_control.ref_cmos_clk_hz = disable;
    serdes_control.pd_cml_refclk_chout = disable;
    serdes_control.pd_cml_lcrefout = disable;
    serdes_control.iso_enable = disable;
    serdes_control.comclk_en = 1;
#endif
    serdes_control_write(phy_dev, &serdes_control);
    mdelay(10);

    /* Step 3 */
    serdes_control.refclk_reset = disable;
    serdes_control.serdes_reset = disable;

    if (phy_dev->addr > 0x1f)
        serdes_control.serdes_test_en = 1;
    else
        serdes_control.serdes_prtad = phy_dev->addr;

    serdes_control_write(phy_dev, &serdes_control);
    mdelay(10);

    return 0;
}

int serdes_access_get_status(phy_dev_t *phy_dev, uint8_t *link_status, uint8_t *module_detect)
{
    uint8_t lane_id = phy_dev->lane_index;
    serdes_status_t serdes_status;
    uint8_t mask = (1 << lane_id);

    serdes_status_read(phy_dev, &serdes_status);
    *link_status = (serdes_status.link_status & mask) ? 1 : 0; 
    *module_detect = !(serdes_status.serdes_mod_def0 & mask) ? 1 : 0;

    return 0;
}
int serdes_access_get_an_status(phy_dev_t *phy_dev, uint8_t *an_link_status)
{
    uint8_t lane_id = phy_dev->lane_index;
    serdes_an_status_t serdes_an_status;
    uint8_t mask = (1 << lane_id);

    serdes_an_status_read(phy_dev, &serdes_an_status);
    *an_link_status = (serdes_an_status.an_link_status & mask) ? 1 : 0; 

    return 0;
}

int serdes_access_get_speed(phy_dev_t *phy_dev, phy_speed_t *speed)
{
    uint8_t lane_id = phy_dev->lane_index;
    serdes_status_1_t serdes_status_1;

    serdes_status_1_read(phy_dev, &serdes_status_1);
    serdes_access_parse_speed(&serdes_status_1, lane_id, speed);

    return 0;
}

int serdes_access_lane_tx_enable(phy_dev_t *phy_dev, int enable)
{
    dt_gpio_desc gpiod_tx_disable = phy_dev->gpiod_tx_disable;

    if (dt_gpio_exists(gpiod_tx_disable))
        dt_gpio_set_value(gpiod_tx_disable, enable ? 0 : 1);

    return 0;
}
