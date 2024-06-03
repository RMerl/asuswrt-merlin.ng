/// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
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
#include <mac_drv.h>
#include "pmc_ethtop.h"

#define MASK_BIT(a)         (1<<(a))
#define XPHY_CORES          2
#define XPORT_0_IRQ         71
#define XPORT_1_IRQ         72
#define XPORT_2_IRQ         73
#define ETH_PHY_TOP_0_IRQ   74
#define ETH_PHY_TOP_1_IRQ   75
#define ETH_PHY_TOP_2_IRQ   76

static void __iomem *eth_phy_top_base;
static dt_device_t *dt_dev; 
static dt_handle_t dt_handle;
static int xphy0_enabled, xphy1_enabled;
static int xphy0_addr, xphy1_addr;

#define ETH_PHY_TOP_BASE                                    eth_phy_top_base
#if defined(CONFIG_BCM6765)
#define ETH_PHY_TOP_REG_XPHY_CNTRL_0                        0x0004
#define ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_0                   0x000c
#define ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_1                   0x0010
#define ETH_PHY_TOP_REG_XPHY_MUX_SEL_CNTRL                  0x0024
#define ETH_PHY_TOP_REG_XPHY_CNTRL_1                        0xffff
#else
#define ETH_PHY_TOP_REG_R2PMI_LP_BCAST_MODE_CNTRL           0x0000
#define ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_0                   0x0234
#define ETH_PHY_TOP_REG_XPHY_CNTRL_0                        0x0238
#define ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_1                   0x0240
#define ETH_PHY_TOP_REG_XPHY_CNTRL_1                        0x0244
#if defined(CONFIG_BCM96813)
#define ETH_PHY_TOP_REG_XPHY_MUX_SEL_CNTRL                  0x01fc
#else
#define ETH_PHY_TOP_REG_XPHY_MUX_SEL_CNTRL                  0x024c
#endif

#define ETH_XPHY_SERDES_INTRL2_CPU_STATUS                   0x0c80	
#define ETH_XPHY_SERDES_INTRL2_CPU_SET                      0x0c84	
#define ETH_XPHY_SERDES_INTRL2_CPU_CLEAR                    0x0c88	
#define ETH_XPHY_SERDES_INTRL2_CPU_MASK_STATUS              0x0c8c	
#define ETH_XPHY_SERDES_INTRL2_CPU_MASK_SET                 0x0c90	
#define ETH_XPHY_SERDES_INTRL2_CPU_MASK_CLEAR               0x0c94	
#endif

#pragma pack(push,1)
typedef struct
{
    uint32_t link_up_intr_mask:1;
    uint32_t link_down_intr_mask:1;
    uint32_t serdes_sd_on_intr_mask:1;
    uint32_t serdes_sd_off_intr_mask:1;
    uint32_t serdes_mod_def0_event_intr_mask:1;
    uint32_t pmd_micro_ext_intr_mask:1;
    uint32_t rbus_err_mask:1;
    uint32_t serdes_an_link_status_intr_mask:4;
    uint32_t xphy_link_up_intr_mask:2;
    uint32_t xphy_link_down_intr_mask:2;
    uint32_t ms_intr_mask:1;
    uint32_t ms_timeout_intr_mask:1;
    uint32_t mpd_intr_mask:3;
    uint32_t reserved1:12;
} eth_xphy_serdes_intrl2_cpu_t;
#pragma pack(pop)

#ifdef PHY_XPHY
static uintptr_t ETH_PHY_TOP_REG_XPHY_TEST_CNTRL[XPHY_CORES] = {
    ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_0,
    ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_1,
};

static uintptr_t ETH_PHY_TOP_REG_XPHY_CNTRL[XPHY_CORES] = {
    ETH_PHY_TOP_REG_XPHY_CNTRL_0,
    ETH_PHY_TOP_REG_XPHY_CNTRL_1,
};

#pragma pack(push,1)
typedef struct
{
    uint32_t phy_test_en:1;
    uint32_t tmode_sel:3;
    uint32_t iso_enable:1;
    uint32_t tmode:1;
    uint32_t reserved1:26;
} eth_phy_top_reg_xphy_test_cntrl_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t super_isolate:1;
    uint32_t phy_reset:1;
    uint32_t phy_phyad:5;
    uint32_t refclk_sel:1;
    uint32_t xtal_bypass:1;
    uint32_t osc_ctrl:5;
    uint32_t led_invert:1;
    uint32_t reserved1:17;
} eth_phy_top_reg_xphy_cntrl_t;
#pragma pack(pop)

static void eth_phy_top_reg_xphy_test_cntrl_read(uint32_t xphy_index, eth_phy_top_reg_xphy_test_cntrl_t *xphy_test)
{
    READ_32(ETH_PHY_TOP_BASE + ETH_PHY_TOP_REG_XPHY_TEST_CNTRL[xphy_index], *xphy_test);
}

static void eth_phy_top_reg_xphy_test_cntrl_write(uint32_t xphy_index, eth_phy_top_reg_xphy_test_cntrl_t *xphy_test)
{
    WRITE_32(ETH_PHY_TOP_BASE + ETH_PHY_TOP_REG_XPHY_TEST_CNTRL[xphy_index], *xphy_test);
}

static void eth_phy_top_reg_xphy_cntrl_read(uint32_t xphy_index, eth_phy_top_reg_xphy_cntrl_t *xphy_control)
{
    READ_32(ETH_PHY_TOP_BASE + ETH_PHY_TOP_REG_XPHY_CNTRL[xphy_index], *xphy_control);
}

static void eth_phy_top_reg_xphy_cntrl_write(uint32_t xphy_index, eth_phy_top_reg_xphy_cntrl_t *xphy_control)
{
    WRITE_32(ETH_PHY_TOP_BASE + ETH_PHY_TOP_REG_XPHY_CNTRL[xphy_index], *xphy_control);
}
#endif

static int xphy_init(uint32_t xphy_id, uint32_t xphy_addr)
{
#ifdef PHY_XPHY
    eth_phy_top_reg_xphy_test_cntrl_t eth_phy_top_reg_xphy_test_cntrl;
    eth_phy_top_reg_xphy_cntrl_t eth_phy_top_reg_xphy_cntrl;

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
    uint32_t val;
    int ret = 0;

    ret |= pmc_ethtop_power_up(ETHTOP_COMMON);

    /* Read driver configuration from device tree */
    xphy0_enabled = dt_property_read_bool(dt_handle, "xphy0-enabled");
    xphy1_enabled = dt_property_read_bool(dt_handle, "xphy1-enabled");
    xphy0_addr = dt_property_read_u32_default(dt_handle, "xphy0-addr", 0x9);
    xphy1_addr = dt_property_read_u32_default(dt_handle, "xphy1-addr", 0xa);

	/* Select the source of XPHY LED signal to LED controller */
    val = 1;
    WRITE_32(ETH_PHY_TOP_BASE + ETH_PHY_TOP_REG_XPHY_MUX_SEL_CNTRL, val);

    /* Enable XPHY0 */
    if (xphy0_enabled)
        ret |= xphy_init(0, xphy0_addr);

    /* Enable XPHY1 */
    if (xphy1_enabled)
        ret |= xphy_init(1, xphy1_addr);

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
    dt_handle = pdev->node;

    if (!dt_is_valid(dt_handle))
    {
        dev_err(&pdev->dev, "Missing node entry\n");
        return -ENODEV;
    }

    eth_phy_top_base = dt_dev_remap(pdev, 0);
    if (IS_ERR(eth_phy_top_base))
    {
        ret = PTR_ERR(eth_phy_top_base);
        eth_phy_top_base = NULL;
        dev_err(&pdev->dev, "Missing eth_phy_top_base entry\n");
        return ret;
    }

    dev_dbg(&pdev->dev, "eth_phy_top_base=0x%p\n", eth_phy_top_base);
    dev_info(&pdev->dev, "registered\n");

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

