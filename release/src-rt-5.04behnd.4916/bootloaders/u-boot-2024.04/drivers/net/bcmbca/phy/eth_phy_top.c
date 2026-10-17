// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved
*/

/*
 *  Created on: May 2021
 *      Author: yuval.raviv@broadcom.com
 */

/*
 * Ethernet PHY top registers access for 6888, 6813, 6837
 */

#include "memory_access.h"
#include "dt_access.h"
#include <linux/delay.h>
#include <dm/device_compat.h>
#include <mac_drv.h>
#include <phy_drv.h>
#include "pmc_ethtop.h"
#if IS_BCMCHIP(68375)
#include "smc_xphy.h"
#endif

#define MASK_BIT(a)			(1 << (a))

#if IS_BCMCHIP(68880) || IS_BCMCHIP(6813) || IS_BCMCHIP(6765)
#define XPHY_CORES			2
#endif
#if IS_BCMCHIP(68375)
#define XPHY_CORES			5
#endif

#if IS_BCMCHIP(68880) || IS_BCMCHIP(6837)
#define XPORT_0_IRQ			71
#define XPORT_1_IRQ			72
#define XPORT_2_IRQ			73
#define ETH_PHY_TOP_0_IRQ	74
#define ETH_PHY_TOP_1_IRQ	75
#define ETH_PHY_TOP_2_IRQ	76
#endif
#if IS_BCMCHIP(68375) && 0
#define XPORT_0_IRQ			72
#define XPORT_1_IRQ			73
#define ETH_PHY_TOP_0_IRQ	75
#define ETH_PHY_TOP_1_IRQ	76
#define ETH_PHY_TOP_2_IRQ	77
#endif

static void __iomem *eth_phy_top_base;
static dt_device_t *dt_dev;
static dt_handle_t dt_handle;
static int xphy0_enabled, xphy1_enabled, xphy2_enabled, xphy3_enabled, xphy4_enabled;
static int xphy0_addr, xphy1_addr, xphy2_addr, xphy3_addr, xphy4_addr;

#define ETH_PHY_TOP_BASE									eth_phy_top_base
#if IS_BCMCHIP(6765)
#define ETH_PHY_TOP_REG_XPHY_CNTRL_0						0x0004
#define ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_0					0x000c
#define ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_1					0x0010
#define ETH_PHY_TOP_REG_XPHY_MUX_SEL_CNTRL					0x0024
#define ETH_PHY_TOP_REG_XPHY_CNTRL_1						0xffff
#elif IS_BCMCHIP(6766) || IS_BCMCHIP(6764)
#define ETH_PHY_TOP_REG_R2PMI_LP_BCAST_MODE_CNTRL			0x01b0  /* Note, 6766 needs to have lower real PHY_TOP_BASE in dtsi, but not XPHY base like other chip */
#else
#define ETH_PHY_TOP_REG_R2PMI_LP_BCAST_MODE_CNTRL			0x0000
#define ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_0					0x0234
#define ETH_PHY_TOP_REG_XPHY_CNTRL_0						0x0238
#define ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_1					0x0240
#define ETH_PHY_TOP_REG_XPHY_CNTRL_1						0x0244
#if IS_BCMCHIP(68375)
#define ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_2					0x024c
#define ETH_PHY_TOP_REG_XPHY_CNTRL_2						0x0250
#define ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_3					0x0258
#define ETH_PHY_TOP_REG_XPHY_CNTRL_3						0x025c
#define ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_4					0x0264
#define ETH_PHY_TOP_REG_XPHY_CNTRL_4						0x0268
#endif
#if IS_BCMCHIP(6813)
#define ETH_PHY_TOP_REG_XPHY_MUX_SEL_CNTRL					0x01fc
#elif IS_BCMCHIP(68375)
#define ETH_PHY_TOP_REG_XPHY_MUX_SEL_CNTRL					0x0270
#else
#define ETH_PHY_TOP_REG_XPHY_MUX_SEL_CNTRL					0x024c
#endif

#if IS_BCMCHIP(68880) || IS_BCMCHIP(6837) || IS_BCMCHIP(68375)
#define ETH_XPHY_SERDES_INTRL2_CPU_STATUS					0x0c80
#define ETH_XPHY_SERDES_INTRL2_CPU_SET						0x0c84
#define ETH_XPHY_SERDES_INTRL2_CPU_CLEAR					0x0c88
#define ETH_XPHY_SERDES_INTRL2_CPU_MASK_STATUS				0x0c8c
#define ETH_XPHY_SERDES_INTRL2_CPU_MASK_SET					0x0c90
#define ETH_XPHY_SERDES_INTRL2_CPU_MASK_CLEAR				0x0c94

#define ETH_GPHY_RGMII_INTRL2_CPU_STATUS					0x0c40
#define ETH_GPHY_RGMII_INTRL2_CPU_SET						0x0c44
#define ETH_GPHY_RGMII_INTRL2_CPU_CLEAR						0x0c48
#define ETH_GPHY_RGMII_INTRL2_CPU_MASK_STATUS				0x0c4c
#define ETH_GPHY_RGMII_INTRL2_CPU_MASK_SET					0x0c50
#define ETH_GPHY_RGMII_INTRL2_CPU_MASK_CLEAR				0x0c54
#endif

#endif

#if IS_BCMCHIP(68880) || IS_BCMCHIP(6837)
#pragma pack(push, 1)
typedef struct {
	u32 link_up_intr_mask:1;
	u32 link_down_intr_mask:1;
	u32 serdes_sd_on_intr_mask:1;
	u32 serdes_sd_off_intr_mask:1;
	u32 serdes_mod_def0_event_intr_mask:1;
	u32 pmd_micro_ext_intr_mask:1;
	u32 rbus_err_mask:1;
	u32 serdes_an_link_status_intr_mask:4;
	u32 xphy_link_up_intr_mask:2;
	u32 xphy_link_down_intr_mask:2;
	u32 ms_intr_mask:1;
	u32 ms_timeout_intr_mask:1;
	u32 mpd_intr_mask:3;
	u32 reserved1:12;
} eth_xphy_serdes_intrl2_cpu_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
	u32 qphy_link_up_intr:4;
	u32 qphy_link_down_intr:4;
	u32 qgphy_energy_on_intr:4;
	u32 qgphy_energy_off_mask:4;
	u32 qgphy_intr_mask:4;
	u32 reserved1:5;
	u32 rgmii_link_up_intr_mask:1;
	u32 rgmii_link_down_intr_mask:1;
	u32 mdio_err_intr_mask:1;
	u32 mdio_done_intr_mask:1;
	u32 reserved2:3;
} eth_gphy_rgmii_intrl2_cpu_t;
#pragma pack(pop)
#endif

#if IS_BCMCHIP(68375)
#pragma pack(push, 1)
typedef struct {
	u32 xphy_link_up_intr_mask:1;
	u32 xphy_link_down_intr_mask:1;
	u32 rbus_err_mask:1;
	u32 reserved1:2;
	u32 mpd_intr_mask:2;
	u32 reserved2:25;
} eth_xphy_serdes_intrl2_cpu_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
	u32 xphy_link_up_intr_mask:4;
	u32 xphy_link_down_intr_mask:4;
	u32 mdio_err_intr_mask:1;
	u32 mdio_done_intr_mask:1;
	u32 reserved2:22;
} eth_gphy_rgmii_intrl2_cpu_t;
#pragma pack(pop)
#endif

#ifdef XPHY_CORES
static uintptr_t ETH_PHY_TOP_REG_XPHY_TEST_CNTRL[XPHY_CORES] = {
	ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_0,
	ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_1,
#if XPHY_CORES == 5
	ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_2,
	ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_3,
	ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_4,
#endif
};

static uintptr_t ETH_PHY_TOP_REG_XPHY_CNTRL[XPHY_CORES] = {
	ETH_PHY_TOP_REG_XPHY_CNTRL_0,
	ETH_PHY_TOP_REG_XPHY_CNTRL_1,
#if XPHY_CORES == 5
	ETH_PHY_TOP_REG_XPHY_CNTRL_2,
	ETH_PHY_TOP_REG_XPHY_CNTRL_3,
	ETH_PHY_TOP_REG_XPHY_CNTRL_4,
#endif
};

#pragma pack(push, 1)
typedef struct {
	u32 phy_test_en:1;
	u32 tmode_sel:3;
	u32 iso_enable:1;
	u32 tmode:1;
	u32 reserved1:26;
} eth_phy_top_reg_xphy_test_cntrl_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
	u32 super_isolate:1;
	u32 phy_reset:1;
	u32 phy_phyad:5;
	u32 refclk_sel:1;
	u32 xtal_bypass:1;
	u32 osc_ctrl:5;
	u32 led_invert:1;
	u32 reserved1:17;
} eth_phy_top_reg_xphy_cntrl_t;
#pragma pack(pop)

static void eth_phy_top_reg_xphy_test_cntrl_read(u32 xphy_index, eth_phy_top_reg_xphy_test_cntrl_t *xphy_test)
{
	READ_32(ETH_PHY_TOP_BASE + ETH_PHY_TOP_REG_XPHY_TEST_CNTRL[xphy_index], *xphy_test);
}

static void eth_phy_top_reg_xphy_test_cntrl_write(u32 xphy_index, eth_phy_top_reg_xphy_test_cntrl_t *xphy_test)
{
	WRITE_32(ETH_PHY_TOP_BASE + ETH_PHY_TOP_REG_XPHY_TEST_CNTRL[xphy_index], *xphy_test);
}

static void eth_phy_top_reg_xphy_cntrl_read(u32 xphy_index, eth_phy_top_reg_xphy_cntrl_t *xphy_control)
{
	READ_32(ETH_PHY_TOP_BASE + ETH_PHY_TOP_REG_XPHY_CNTRL[xphy_index], *xphy_control);
}

static void eth_phy_top_reg_xphy_cntrl_write(u32 xphy_index, eth_phy_top_reg_xphy_cntrl_t *xphy_control)
{
	WRITE_32(ETH_PHY_TOP_BASE + ETH_PHY_TOP_REG_XPHY_CNTRL[xphy_index], *xphy_control);
}
#endif

static int xphy_init(u32 xphy_id, u32 xphy_addr)
{
#ifdef XPHY_CORES
	eth_phy_top_reg_xphy_test_cntrl_t eth_phy_top_reg_xphy_test_cntrl;
	eth_phy_top_reg_xphy_cntrl_t eth_phy_top_reg_xphy_cntrl;

#if IS_BCMCHIP(68375)
	pmc_xphy_power_up(XPHY_PORT0 + xphy_id, XPHY_HIGH_SPEED);
#endif

	eth_phy_top_reg_xphy_cntrl_read(xphy_id, &eth_phy_top_reg_xphy_cntrl);
	eth_phy_top_reg_xphy_cntrl.phy_reset = 1;
	eth_phy_top_reg_xphy_cntrl_write(xphy_id, &eth_phy_top_reg_xphy_cntrl);
	mdelay(100);

	eth_phy_top_reg_xphy_test_cntrl_read(xphy_id, &eth_phy_top_reg_xphy_test_cntrl);
	eth_phy_top_reg_xphy_test_cntrl.iso_enable = 0;
	eth_phy_top_reg_xphy_test_cntrl.tmode = 0;
	eth_phy_top_reg_xphy_test_cntrl_write(xphy_id, &eth_phy_top_reg_xphy_test_cntrl);
	mdelay(100);

	eth_phy_top_reg_xphy_cntrl_read(xphy_id, &eth_phy_top_reg_xphy_cntrl);
	eth_phy_top_reg_xphy_cntrl.phy_phyad = xphy_addr;
	eth_phy_top_reg_xphy_cntrl.phy_reset = 0;
	eth_phy_top_reg_xphy_cntrl.super_isolate = 0;
	eth_phy_top_reg_xphy_cntrl_write(xphy_id, &eth_phy_top_reg_xphy_cntrl);
	mdelay(100);
#endif

	return 0;
}

static int eth_phy_top_init(void)
{
	u32 val;
	int ret = 0;

	ret |= pmc_ethtop_power_up(ETHTOP_COMMON);

	/* Read driver configuration from device tree */
	xphy0_enabled = dt_property_read_bool(dt_handle, "xphy0-enabled");
	xphy1_enabled = dt_property_read_bool(dt_handle, "xphy1-enabled");
	xphy2_enabled = dt_property_read_bool(dt_handle, "xphy2-enabled");
	xphy3_enabled = dt_property_read_bool(dt_handle, "xphy3-enabled");
	xphy4_enabled = dt_property_read_bool(dt_handle, "xphy4-enabled");
	xphy0_addr = dt_property_read_u32_default(dt_handle, "xphy0-addr", 0x9);
	xphy1_addr = dt_property_read_u32_default(dt_handle, "xphy1-addr", 0xa);
	xphy2_addr = dt_property_read_u32_default(dt_handle, "xphy2-addr", 0xb);
	xphy3_addr = dt_property_read_u32_default(dt_handle, "xphy3-addr", 0xc);
	xphy4_addr = dt_property_read_u32_default(dt_handle, "xphy4-addr", 0xd);

	/* Select the source of XPHY LED signal to LED controller */
#if defined(ETH_PHY_TOP_REG_XPHY_MUX_SEL_CNTRL)
	val = 1;
	WRITE_32(ETH_PHY_TOP_BASE + ETH_PHY_TOP_REG_XPHY_MUX_SEL_CNTRL, val);
#endif

	/* Enable XPHY0 */
	if (xphy0_enabled)
		ret |= xphy_init(0, xphy0_addr);

	/* Enable XPHY1 */
	if (xphy1_enabled)
		ret |= xphy_init(1, xphy1_addr);

	/* Enable XPHY2 */
	if (xphy2_enabled)
		ret |= xphy_init(2, xphy2_addr);

	/* Enable XPHY3 */
	if (xphy3_enabled)
		ret |= xphy_init(3, xphy3_addr);

	/* Enable XPHY4 */
	if (xphy4_enabled)
		ret |= xphy_init(4, xphy4_addr);

	/* Disable broadcast mode for SerDeses PMI */
#if defined(ETH_PHY_TOP_REG_R2PMI_LP_BCAST_MODE_CNTRL)
	READ_32(ETH_PHY_TOP_BASE + ETH_PHY_TOP_REG_R2PMI_LP_BCAST_MODE_CNTRL, val);
	val &= ~MASK_BIT(8);
	WRITE_32(ETH_PHY_TOP_BASE + ETH_PHY_TOP_REG_R2PMI_LP_BCAST_MODE_CNTRL, val);
#endif

	return ret;
}

static int eth_phy_top_probe(dt_device_t *pdev)
{
	int ret;

	dt_dev = pdev;
	dt_handle = pdev->node_;

	if (!dt_is_valid(dt_handle)) {
		dev_err(pdev, "Missing node entry\n");
		return -ENODEV;
	}

	eth_phy_top_base = dt_dev_remap(pdev, 0);
	if (IS_ERR(eth_phy_top_base)) {
		ret = PTR_ERR(eth_phy_top_base);
		eth_phy_top_base = NULL;
		dev_err(pdev, "Missing eth_phy_top_base entry\n");
		return ret;
	}

	dev_dbg(pdev, "eth_phy_top_base=0x%p\n", eth_phy_top_base);
	dev_info(pdev, "registered\n");

	return eth_phy_top_init();
}

static const struct udevice_id eth_phy_top_ids[] = {
	{ .compatible = "brcm,eth-phy-top" },
	{ /* end of list */ },
};

U_BOOT_DRIVER(brcm_eth_phy_top) = {
	.name	= "eth-phy-top",
	.id	= UCLASS_MISC,
	.of_match = eth_phy_top_ids,
	.probe = eth_phy_top_probe,
};

/*
 * 10G integrated phy(XPHY) has incorrect default led source that
 * can result into incorrect default state when led driver starts. The LED
 * will stay on until network driver starts and update the led source even
 * when network cable is not plugged in. This fixup correct the source and
 * will be called before the led driver starts.
 */
void xphy_led_fixup(void)
{
#if defined(XPHY_LED_FIXUP)
	u32 val = 1;
	ofnode node;

	if (!eth_phy_top_base) {
		node = ofnode_by_compatible(ofnode_null(), "brcm,eth-phy-top");
		if (ofnode_valid(node))
			eth_phy_top_base = (void __iomem *)ofnode_get_addr(node);
		else
			return;
	}

	/* Select the source of XPHY LED signal to LED controller */
	WRITE_32(ETH_PHY_TOP_BASE + ETH_PHY_TOP_REG_XPHY_MUX_SEL_CNTRL, val);
#endif
}
