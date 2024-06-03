/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2016:DUAL/GPL:standard

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
 *  Created on: May 2021
 *      Author: yuval.raviv@broadcom.com
 */

/*
 * Ethernet PHY top registers access for 6888, 6813, 6837
 */


#include "memory_access.h"
#include "dt_access.h"
#include "clk_rst.h"
#include <linux/delay.h>
#include <bcm_bca_extintr.h>
#include <mac_drv.h>
#include <phy_drv.h>
#include "pmc_ethtop.h"
#include "pmc_shutdown.h"

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
#if defined(CONFIG_BCM96765)
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
    dt_handle = pdev->dev.of_node;

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

static const struct of_device_id of_platform_table[] = {
    { .compatible = "brcm,eth-phy-top" },
    { /* end of list */ },
};

static struct platform_driver of_platform_driver = {
    .driver = {
        .name = "brcm,eth-phy-top",
        .of_match_table = of_platform_table,
    },
    .probe = eth_phy_top_probe,
};
module_platform_driver(of_platform_driver);

#if IS_BCMCHIP(68880) || IS_BCMCHIP(6837) 
static int bcm_bca_extintr_request_and_mask(void *_dev, struct device_node *np, const char *consumer_name,
    irq_handler_t pfunc, void *param, const char *interrupt_name, irq_handler_t thread_fn, int *irq)
{
    int virq = bcm_bca_extintr_request(_dev, np, consumer_name, pfunc, param, interrupt_name, thread_fn);

    *irq = bcm_bca_extintr_get_hwirq(virq);

    return periph_intr_mask_set(1, *irq, 1);
}

static irqreturn_t dummy_isr(int irq, void *context)
{
    return IRQ_HANDLED;
}

int bcm_wake_on_lan(mac_dev_t *mac_dev, phy_dev_t *phy_dev, int is_internal, int is_lnk)
{
    int rc = 0;
    wake_type_t wake_type = WAKE_NONE;
    int wake_param = 0;
    eth_xphy_serdes_intrl2_cpu_t intr_bits = {};

    if (mac_dev->mac_drv->mac_type != MAC_TYPE_XPORT)
        return -1;

    if (is_internal)
    {
        int xport_num  = mac_dev->mac_id / 4;

        if (is_lnk)
        {
#ifdef PHY_XPHY
            int xphy_num = -1;

            if (xport_num == 0)
            {
                if (mac_dev->mac_id == 0) xphy_num = 0;
                if (mac_dev->mac_id == 2) xphy_num = 1;
            }

            if (xphy_num < 0)
            {
                printk("bcm_wake_on_lan() xport port number %d does not support link up interrupt\n", mac_dev->mac_id);
                return -1;
            }

            /* Enable the interrupts for internal XPHY link up events */
            intr_bits.xphy_link_up_intr_mask =  MASK_BIT(xphy_num);
#endif
        }
        else
        {
            /* Enable the interrupts for XPORT MPD events */
            intr_bits.mpd_intr_mask = MASK_BIT(xport_num);
        }

#if defined(ETH_XPHY_SERDES_INTRL2_CPU_CLEAR)
        WRITE_32(ETH_PHY_TOP_BASE + ETH_XPHY_SERDES_INTRL2_CPU_CLEAR, intr_bits);
        WRITE_32(ETH_PHY_TOP_BASE + ETH_XPHY_SERDES_INTRL2_CPU_MASK_CLEAR, intr_bits);
#endif
        wake_type = WAKE_XPORT;
        wake_param = xport_num;
    }
    else
    {
        int irq = 0;

        /* Enable the interrupts for external PHY MPD and link up events */
        if (is_lnk)
        {
            rc = bcm_bca_extintr_request_and_mask(&dt_dev->dev, dt_handle, "phy-link", dummy_isr,
                NULL, "PHY link up", NULL, &irq);

            if (rc)
            {
                printk("[%s:%d] Error\n",__FUNCTION__, __LINE__);
                goto Exit;
            }
        }
        else
        {
            rc = bcm_bca_extintr_request_and_mask(&dt_dev->dev, dt_handle, "phy-mpd", dummy_isr,
                NULL, "PHY MPD", NULL, &irq);

            if (rc)
            {
                printk("[%s:%d] Error\n",__FUNCTION__, __LINE__);
                goto Exit;
            }
        }

        wake_type = WAKE_IRQ;
        wake_param = irq;
    }

    if ((rc = periph_intr_mask_set(1, XPORT_0_IRQ, 1)))
    {
        printk("[%s:%d] Error\n",__FUNCTION__, __LINE__);
        goto Exit;
    }

    if ((rc = periph_intr_mask_set(1, XPORT_1_IRQ, 1)))
    {
        printk("[%s:%d] Error\n",__FUNCTION__, __LINE__);
        goto Exit;
    }

    if ((rc = periph_intr_mask_set(1, XPORT_2_IRQ, 1)))
    {
        printk("[%s:%d] Error\n",__FUNCTION__, __LINE__);
        goto Exit;
    }

    if ((rc = periph_intr_mask_set(1, ETH_PHY_TOP_0_IRQ, 1)))
    {
        printk("[%s:%d] Error\n",__FUNCTION__, __LINE__);
        goto Exit;
    }

    if ((rc = periph_intr_mask_set(1, ETH_PHY_TOP_1_IRQ, 1)))
    {
        printk("[%s:%d] Error\n",__FUNCTION__, __LINE__);
        goto Exit;
    }

    if ((rc = periph_intr_mask_set(1, ETH_PHY_TOP_2_IRQ, 1)))
    {
        printk("[%s:%d] Error\n",__FUNCTION__, __LINE__);
        goto Exit;
    }

    /* Enable the wake function required to reset the chip and then shut down
     * all the blocks except for SMC and the blocks waiting for wake signals (for example: XPORT) */
    phy_devices_shutdown(phy_dev);

    rc = pmc_shutdown(wake_type, wake_param);

Exit:
    return rc;
}
EXPORT_SYMBOL(bcm_wake_on_lan);

int bcm_wake_on_button(void)
{
    int irq = 0, rc = 0;

    rc = bcm_bca_extintr_request_and_mask(&dt_dev->dev, dt_handle, "button", dummy_isr,
        NULL, "Wake button", NULL, &irq);

    if (rc)
    {
        printk("[%s:%d] Error\n",__FUNCTION__, __LINE__);
        goto Exit;
    }

    phy_devices_shutdown(NULL);

    rc = pmc_shutdown(WAKE_IRQ, irq);

Exit:
    return rc;
}
EXPORT_SYMBOL(bcm_wake_on_button);

int bcm_wake_on_timer(int minutes)
{
    phy_devices_shutdown(NULL);

    return pmc_shutdown(WAKE_TIMER, minutes);
}
EXPORT_SYMBOL(bcm_wake_on_timer);

int bcm_wake_on_wan(void)
{
    printk("bcm_wake_on_wan\n");
    return 0;
}
EXPORT_SYMBOL(bcm_wake_on_wan);
#endif
