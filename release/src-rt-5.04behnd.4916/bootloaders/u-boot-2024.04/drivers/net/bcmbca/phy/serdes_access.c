// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved

*/

/*
 *	Created on: Apr 2021
 *		Author: yuval.raviv@broadcom.com
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
#include <dm/device_compat.h>
#if IS_BCMCHIP(68375)
#include "smc_merlin.h"
#endif

#define WRITE_32(a, r)				(*(volatile uint32_t *)(a) = *(uint32_t *)&(r))
#define READ_32(a, r)				(*(volatile uint32_t *)&(r) = *(volatile uint32_t *)(a))

#define SERDES_BASE					serdes_base

static void __iomem *serdes_base;

static int serdes_probe(dt_device_t *pdev)
{
	int ret;

	serdes_base = dt_dev_remap(pdev, 0);
	if (IS_ERR(serdes_base)) {
		ret = PTR_ERR(serdes_base);
		serdes_base = NULL;
		dev_err(pdev, "Missing serdes_base entry\n");
		goto Exit;
	}

	dev_dbg(pdev, "serdes_base=0x%p\n", serdes_base);
	dev_info(pdev, "registered\n");

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
	u8 core_id = phy_dev->core_index;

	READ_32(SERDES_BASE + SERDES_CONTROL[core_id], *serdes_control);
}
#endif

static void serdes_control_write(phy_dev_t *phy_dev, serdes_control_t *serdes_control)
{
	u8 core_id = phy_dev->core_index;

	WRITE_32(SERDES_BASE + SERDES_CONTROL[core_id], *serdes_control);
}

static void serdes_status_read(phy_dev_t *phy_dev, serdes_status_t *serdes_status)
{
	u8 core_id = phy_dev->core_index;

	READ_32(SERDES_BASE + SERDES_STATUS[core_id], *serdes_status);
}

static void serdes_an_status_read(phy_dev_t *phy_dev, serdes_an_status_t *serdes_an_status)
{
	u8 core_id = phy_dev->core_index;

	READ_32(SERDES_BASE + SERDES_AN_STATUS[core_id], *serdes_an_status);
}

static void serdes_indirect_access_control_read(phy_dev_t *phy_dev,
	serdes_indirect_access_control_t *serdes_indirect_access_control)
{
	u8 core_id = phy_dev->core_index;

	READ_32(SERDES_BASE + SERDES_INDIR_ACC_CNTRL[core_id], *serdes_indirect_access_control);
}

static void serdes_indirect_access_control_write(phy_dev_t *phy_dev,
	serdes_indirect_access_control_t *serdes_indirect_access_control)
{
	u8 core_id = phy_dev->core_index;

	WRITE_32(SERDES_BASE + SERDES_INDIR_ACC_CNTRL[core_id], *serdes_indirect_access_control);
}

static void serdes_indirect_access_address_write(phy_dev_t *phy_dev, u32 val)
{
	u8 core_id = phy_dev->core_index;

	WRITE_32(SERDES_BASE + SERDES_INDIR_ACC_ADDR[core_id], val);
}

static void serdes_indirect_access_mask_write(phy_dev_t *phy_dev, u32 val)
{
	u8 core_id = phy_dev->core_index;

	WRITE_32(SERDES_BASE + SERDES_INDIR_ACC_MASK[core_id], val);
}

static void serdes_status_1_read(phy_dev_t *phy_dev, serdes_status_1_t *serdes_status_1)
{
	u8 core_id = phy_dev->core_index;

	READ_32(SERDES_BASE + SERDES_STATUS_1[core_id], *serdes_status_1);
}

static inline u32 serdes_indirect_access_encode_address(u8 lane_id, u16 dev, u16 reg)
{
	return (dev << DEV_TYPE_OFFSET) | (lane_id << LANE_ADDRESS_OFFSET) | reg;
}

#define SERDES_ACCESS_RETRIES 100
static int serdes_access_wait_for_status_ready(phy_dev_t *phy_dev)
{
	serdes_indirect_access_control_t serdes_indirect_access_control;
	u32 retries = SERDES_ACCESS_RETRIES;

	while (retries--) {
		serdes_indirect_access_control_read(phy_dev, &serdes_indirect_access_control);

		if (!serdes_indirect_access_control_status(&serdes_indirect_access_control))
			return 0;

		udelay(1);
	}

	return 1;
}

int serdes_access_read_mask(phy_dev_t *phy_dev, u16 dev, u16 reg, u16 mask, u16 shift, u16 *val)
{
	u8 core_id = phy_dev->core_index;
	u8 lane_id = phy_dev->lane_index;
	serdes_indirect_access_control_t serdes_indirect_access_control;
	u32 addr = serdes_indirect_access_encode_address(lane_id, dev, reg);

	serdes_indirect_access_control_init(&serdes_indirect_access_control, 1, 0); /* Means read operation */
	serdes_indirect_access_address_write(phy_dev, addr);
	serdes_indirect_access_control_write(phy_dev, &serdes_indirect_access_control);

	/* Validate no error after reading */
	if (serdes_access_wait_for_status_ready(phy_dev)) {
		pr_err("Error while validating read from SerDes %d\n", core_id);
		return -1;
	}

	serdes_indirect_access_control_read(phy_dev, &serdes_indirect_access_control);
	*val = serdes_indirect_access_control.reg_data & mask;

	return 0;
}

int serdes_access_write_mask(phy_dev_t *phy_dev, u16 dev, u16 reg, u16 mask, u16 shift, u16 val)
{
	u8 core_id = phy_dev->core_index;
	u8 lane_id = phy_dev->lane_index;
	serdes_indirect_access_control_t serdes_indirect_access_control;
	u32 addr = serdes_indirect_access_encode_address(lane_id, dev, reg);

	serdes_indirect_access_control_init(&serdes_indirect_access_control, 0, val << shift); /* Means write operation */
	serdes_indirect_access_address_write(phy_dev, addr);
	serdes_indirect_access_mask_write(phy_dev, ~mask);
	serdes_indirect_access_control_write(phy_dev, &serdes_indirect_access_control);

	/* Validate no error after writing */
	if (serdes_access_wait_for_status_ready(phy_dev)) {
		pr_err("Error while validating write to SerDes %d\n", core_id);
		return -1;
	}

	return 0;
}

int serdes_access_read(phy_dev_t *phy_dev, u16 dev, u16 reg, u16 *val)
{
	return serdes_access_read_mask(phy_dev, dev, reg, MASK_ALL_BITS_16, 0, val);
}

int serdes_access_write(phy_dev_t *phy_dev, u16 dev, u16 reg, u16 val)
{
	return serdes_access_write_mask(phy_dev, dev, reg, MASK_ALL_BITS_16, 0, val);
}

int serdes_access_wait_pll_lock(phy_dev_t *phy_dev)
{
	serdes_status_t serdes_status = {};
	u8 core_id = phy_dev->core_index;
	u32 retries = SERDES_ACCESS_RETRIES;

	do {
		mdelay(10);
		serdes_status_read(phy_dev, &serdes_status);
	} while (!serdes_status.pll_lock && --retries);

	if (!serdes_status.pll_lock) {
		pr_err("Failed to get PLL lock in SerDes %d\n", core_id);
		return -1;
	}

	return 0;
}

int serdes_access_wait_link_status(phy_dev_t *phy_dev)
{
	serdes_status_t serdes_status = {};
	u8 core_id = phy_dev->core_index;
	u8 lane_id = phy_dev->lane_index;
	u8 mask = (1 << lane_id);
	u32 retries = SERDES_ACCESS_RETRIES;

	do {
		mdelay(10);
		serdes_status_read(phy_dev, &serdes_status);
	} while (!(serdes_status.link_status & mask) && --retries);

	if (!(serdes_status.link_status & mask)) {
		pr_err("Failed to get link in SerDes %d Lane %d\n", core_id, lane_id);
		return -1;
	}

	return 0;
}

int serdes_access_config(phy_dev_t *phy_dev, int enable)
{
	serdes_control_t serdes_control = {0};
	int disable = !enable;

#if IS_BCMCHIP(68375)
	if (enable)
		smc_merlin_power_up(phy_dev->core_index);
#endif

	/* Step 1 */
	serdes_control.iddq = 1;
	serdes_control.refclk_reset = 1;
	serdes_control.serdes_reset = 1;
#if IS_BCMCHIP(68880) || IS_BCMCHIP(6837) || IS_BCMCHIP(68375)
	serdes_control.ref_cmos_clk_hz = 1;
	serdes_control.pd_cml_refclk_chout = 1;
	serdes_control.pd_cml_lcrefout = 1;
	serdes_control.iso_enable = 1;
#endif
#if IS_BCMCHIP(6813) || IS_BCMCHIP(6764) || IS_BCMCHIP(6765) || IS_BCMCHIP(6766)
	serdes_control.comclk_enable = 1;
#endif
#if defined(PHY_SHARED_REF_CLK)
	if (phy_dev->shared_ref_clk_mhz) {
		serdes_control.serdes_testsel = 1;
		serdes_control.iso_enable = 0;
	} else
		serdes_control.serdes_testsel = 0;
#endif
	serdes_control_write(phy_dev, &serdes_control);
	mdelay(10);

	/* Step 2 */
	serdes_control.iddq = disable;
#if IS_BCMCHIP(68880) || IS_BCMCHIP(6837) || IS_BCMCHIP(68375)
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

#if IS_BCMCHIP(68375)
	if (!enable)
		smc_merlin_power_down(phy_dev->core_index);
#endif

	return 0;
}

int serdes_access_get_status(phy_dev_t *phy_dev, u8 *link_status, u8 *module_detect)
{
	u8 lane_id = phy_dev->lane_index;
	serdes_status_t serdes_status;
	u8 mask = (1 << lane_id);

	serdes_status_read(phy_dev, &serdes_status);
	*link_status = (serdes_status.link_status & mask) ? 1 : 0;
	*module_detect = !(serdes_status.serdes_mod_def0 & mask) ? 1 : 0;

	return 0;
}

int serdes_access_get_an_status(phy_dev_t *phy_dev, u8 *an_link_status)
{
	u8 lane_id = phy_dev->lane_index;
	serdes_an_status_t serdes_an_status;
	u8 mask = (1 << lane_id);

	if (sizeof(serdes_an_status) < 4)
		return -1;

	serdes_an_status_read(phy_dev, &serdes_an_status);
	*an_link_status = (serdes_an_status.an_link_status & mask) ? 1 : 0;

	return 0;
}

int serdes_access_get_an_pause_resolution(phy_dev_t *phy_dev, int *pause_rx, int *pause_tx)
{
	u8 lane_id = phy_dev->lane_index;
	serdes_an_status_t serdes_an_status;
	u8 mask = (1 << lane_id);

	if (sizeof(serdes_an_status) < 4)
		return -1;

	serdes_an_status_read(phy_dev, &serdes_an_status);
	*pause_rx = (serdes_an_status.an_hcd_rx_pause & mask) > 0;
	*pause_tx = (serdes_an_status.an_hcd_tx_pause & mask) > 0;

	return 0;
}

int serdes_access_get_speed(phy_dev_t *phy_dev, phy_speed_t *speed)
{
	u8 lane_id = phy_dev->lane_index;
	serdes_status_1_t serdes_status_1;

	serdes_status_1_read(phy_dev, &serdes_status_1);
	serdes_access_parse_speed(&serdes_status_1, lane_id, speed);

	return 0;
}

int serdes_access_lane_enable(phy_dev_t *phy_dev, int enable)
{
	if (dt_gpio_exists(phy_dev->gpiod_rx_power))
		dt_gpio_set_value(phy_dev->gpiod_rx_power, enable);

	if (dt_gpio_exists(phy_dev->gpiod_tx_power))
		dt_gpio_set_value(phy_dev->gpiod_tx_power, enable);

	if (dt_gpio_exists(phy_dev->gpiod_tx_disable))
		dt_gpio_set_value(phy_dev->gpiod_tx_disable, !enable);

	return 0;
}
