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
#include <linux/interrupt.h>
#include <bcm_bca_extintr.h>
#include <mac_drv.h>
#include <phy_drv.h>
#include "pmc_ethtop.h"
#include "pmc_shutdown.h"

#define MASK_BIT(a)         (1<<(a))

#if IS_BCMCHIP(68880) || IS_BCMCHIP(6813) || IS_BCMCHIP(6765)
#define XPHY_CORES          2
#endif
#if IS_BCMCHIP(68375)
#define XPHY_CORES          5
#endif

#if IS_BCMCHIP(68880) || IS_BCMCHIP(6837) 
#define XPORT_0_IRQ         71
#define XPORT_1_IRQ         72
#define XPORT_2_IRQ         73
#define ETH_PHY_TOP_0_IRQ   74
#define ETH_PHY_TOP_1_IRQ   75
#define ETH_PHY_TOP_2_IRQ   76
#endif
#if IS_BCMCHIP(68375) && 0
#define XPORT_0_IRQ         72
#define XPORT_1_IRQ         73
#define ETH_PHY_TOP_0_IRQ   75
#define ETH_PHY_TOP_1_IRQ   76
#define ETH_PHY_TOP_2_IRQ   77
#endif

static void __iomem *eth_phy_top_base;
static dt_device_t *dt_dev; 
static dt_handle_t dt_handle;
static int xphy0_enabled, xphy1_enabled, xphy2_enabled, xphy3_enabled, xphy4_enabled;
static int xphy0_addr, xphy1_addr, xphy2_addr, xphy3_addr, xphy4_addr;
#if IS_BCMCHIP(68880) || IS_BCMCHIP(6837) //|| IS_BCMCHIP(68375)
static int power_down_xphy = 1;
#endif

#define ETH_PHY_TOP_BASE                                    eth_phy_top_base
#if IS_BCMCHIP(6765)
#define ETH_PHY_TOP_REG_XPHY_CNTRL_0                        0x0004
#define ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_0                   0x000c
#define ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_1                   0x0010
#define ETH_PHY_TOP_REG_XPHY_MUX_SEL_CNTRL                  0x0024
#define ETH_PHY_TOP_REG_XPHY_CNTRL_1                        0xffff
#elif IS_BCMCHIP(6766) || IS_BCMCHIP(6764)
#define ETH_PHY_TOP_REG_R2PMI_LP_BCAST_MODE_CNTRL           0x01b0  /* Note, 6766 needs to have lower real PHY_TOP_BASE in dtsi, but not XPHY base like other chip */
#else
#define ETH_PHY_TOP_REG_R2PMI_LP_BCAST_MODE_CNTRL           0x0000
#define ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_0                   0x0234
#define ETH_PHY_TOP_REG_XPHY_CNTRL_0                        0x0238
#define ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_1                   0x0240
#define ETH_PHY_TOP_REG_XPHY_CNTRL_1                        0x0244
#if IS_BCMCHIP(68375) 
#define ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_2                   0x024c
#define ETH_PHY_TOP_REG_XPHY_CNTRL_2                        0x0250
#define ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_3                   0x0258
#define ETH_PHY_TOP_REG_XPHY_CNTRL_3                        0x025c
#define ETH_PHY_TOP_REG_XPHY_TEST_CNTRL_4                   0x0264
#define ETH_PHY_TOP_REG_XPHY_CNTRL_4                        0x0268
#endif
#if IS_BCMCHIP(6813)
#define ETH_PHY_TOP_REG_XPHY_MUX_SEL_CNTRL                  0x01fc
#elif IS_BCMCHIP(68375) 
#define ETH_PHY_TOP_REG_XPHY_MUX_SEL_CNTRL                  0x0270
#else
#define ETH_PHY_TOP_REG_XPHY_MUX_SEL_CNTRL                  0x024c
#endif

#if IS_BCMCHIP(68880) || IS_BCMCHIP(6837) || IS_BCMCHIP(68375)
#define ETH_XPHY_SERDES_INTRL2_CPU_STATUS                   0x0c80	
#define ETH_XPHY_SERDES_INTRL2_CPU_SET                      0x0c84	
#define ETH_XPHY_SERDES_INTRL2_CPU_CLEAR                    0x0c88	
#define ETH_XPHY_SERDES_INTRL2_CPU_MASK_STATUS              0x0c8c	
#define ETH_XPHY_SERDES_INTRL2_CPU_MASK_SET                 0x0c90	
#define ETH_XPHY_SERDES_INTRL2_CPU_MASK_CLEAR               0x0c94	

#define ETH_GPHY_RGMII_INTRL2_CPU_STATUS                    0x0c40	
#define ETH_GPHY_RGMII_INTRL2_CPU_SET                       0x0c44	
#define ETH_GPHY_RGMII_INTRL2_CPU_CLEAR                     0x0c48	
#define ETH_GPHY_RGMII_INTRL2_CPU_MASK_STATUS               0x0c4c	
#define ETH_GPHY_RGMII_INTRL2_CPU_MASK_SET                  0x0c50	
#define ETH_GPHY_RGMII_INTRL2_CPU_MASK_CLEAR                0x0c54	
#endif

#endif

#if IS_BCMCHIP(68880) || IS_BCMCHIP(6837) 
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

#pragma pack(push,1)
typedef struct
{
    uint32_t qphy_link_up_intr:4;
    uint32_t qphy_link_down_intr:4;
    uint32_t qgphy_energy_on_intr:4;
    uint32_t qgphy_energy_off_mask:4;
    uint32_t qgphy_intr_mask:4;
    uint32_t reserved1:5;
    uint32_t rgmii_link_up_intr_mask:1;
    uint32_t rgmii_link_down_intr_mask:1;
    uint32_t mdio_err_intr_mask:1;
    uint32_t mdio_done_intr_mask:1;
    uint32_t reserved2:3;
} eth_gphy_rgmii_intrl2_cpu_t;
#pragma pack(pop)
#endif

#if IS_BCMCHIP(68375) 
#pragma pack(push,1)
typedef struct
{
    uint32_t xphy_link_up_intr_mask:1;
    uint32_t xphy_link_down_intr_mask:1;
    uint32_t rbus_err_mask:1;
    uint32_t reserved1:2;
    uint32_t mpd_intr_mask:2;
    uint32_t reserved2:25;
} eth_xphy_serdes_intrl2_cpu_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t xphy_link_up_intr_mask:4;
    uint32_t xphy_link_down_intr_mask:4;
    uint32_t mdio_err_intr_mask:1;
    uint32_t mdio_done_intr_mask:1;
    uint32_t reserved2:22;
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
#ifdef XPHY_CORES
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
static void power_supplies_off(const char *name)
{
#if IS_BCMCHIP(68880) || IS_BCMCHIP(6837) || IS_BCMCHIP(68375)
    int i = 0;
    struct gpio_descs *gpios = NULL;

    gpios = devm_gpiod_get_array_optional(&dt_dev->dev, name, GPIOD_OUT_HIGH);
    if (IS_ERR_OR_NULL(gpios))
        return;

    printk("power_supplies_off: %s (%d)\n", name, gpios->ndescs);

    for (i = 0; i < gpios->ndescs; i++)
    {
        struct gpio_desc *gpiod = gpios->desc[i];

        dt_gpio_set_value(gpiod, 0);
    }

    devm_gpiod_put_array(&dt_dev->dev, gpios);
#endif
}

static int eth_phy_top_init(void)
{
    uint32_t val;
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

    power_supplies_off("power-default");

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

#if IS_BCMCHIP(68880) || IS_BCMCHIP(6837) //|| IS_BCMCHIP(68375)
static int bcm_bca_extintr_request_and_mask(void *_dev, struct device_node *np, const char *consumer_name,
    irq_handler_t pfunc, void *param, const char *interrupt_name, irq_handler_t thread_fn, int *irq)
{
#if defined(CONFIG_BCM_BCA_EXTINTR)
    int virq = bcm_bca_extintr_request(_dev, np, consumer_name, pfunc, param, interrupt_name, thread_fn);
#else
    struct device *dev = (struct device *)_dev;
    struct gpio_desc *gpiod;
    int virq, status;
    unsigned long irqflags = 0;
    struct of_phandle_args params;
    char propname[128];
    u32 gpio;

    if (!np)
        np = dev->of_node;
    snprintf(propname, 128, "%s%s", consumer_name, "-gpio");

    /* check if gpio is used by other button. eth wake up usually use existing button */
    status = of_parse_phandle_with_fixed_args(np, propname, 2, 0, &params);
    if (status) {
        dev_err(dev, "failed to get gpio args ret %d\n", status);
        return status;
    }

    gpio = params.args[0];
    virq = bcm_bca_extintr_get_virq_by_gpio(gpio);
    if (virq < 0) {
        gpiod = devm_gpiod_get_from_of_node(dev, np, propname, 0, GPIOD_IN, NULL);
        if (IS_ERR(gpiod)) {
            dev_err(dev, "failed to request %s, err %d, np %px, np name %s\n",
            propname, (int)(PTR_ERR(gpiod)), np, of_node_full_name(np));
            return -PTR_ERR(gpiod);
        }

        virq = gpiod_to_irq(gpiod);
        if (!virq) {
            dev_err(dev, "failed to request gpio irq for gpio %s\n", consumer_name);
            return -ENOENT;
        }
        /* phy port gpio signal always uses falling edge to trigger interrupt */
        irqflags |= IRQF_TRIGGER_FALLING;

        status = devm_request_threaded_irq(dev, virq, pfunc, thread_fn, irqflags, interrupt_name, param);
        if (status) {
            dev_err(dev, "failed to request extintr gpio %s irq %d err %d\n",
                consumer_name, virq, status);
            return status;
        }
    } else {
        dev_warn(dev, "gpio %s %d already associated with irq %d\n", propname, gpio, virq);
    }
#endif

    *irq = bcm_bca_extintr_get_hwirq(virq);

    return periph_intr_mask_set(3, *irq, 1);
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
    eth_xphy_serdes_intrl2_cpu_t intr_bits_xphy = {};
#if IS_BCMCHIP(68375) 
    eth_gphy_rgmii_intrl2_cpu_t intr_bits_gphy = {};
#endif
    mac_cfg_t mac_cfg = {};
    phy_speed_t phy_speed = PHY_SPEED_UNKNOWN;
    uint32_t caps;

    if (mac_dev->mac_drv->mac_type != MAC_TYPE_XPORT)
        return -1;

    if (is_internal)
    {
        int xlmac_id = mac_dev->mac_id / 4;
        int xphy_num = -1;

        if (is_lnk)
        {
#ifdef XPHY_CORES

#if IS_BCMCHIP(68375) 
            if (xlmac_id == 0)
                xphy_num = mac_dev->mac_id == 2 ? 4 : -1;
            else
                xphy_num = mac_dev->mac_id % 4;
#else
            if (xlmac_id == 0)
            {
                if (mac_dev->mac_id == 0) xphy_num = 0;
                if (mac_dev->mac_id == 2) xphy_num = 1;
            }
#endif

            if (xphy_num < 0)
            {
                printk("bcm_wake_on_lan() xport port number %d does not support link up interrupt\n", mac_dev->mac_id);
                return -1;
            }

            /* Enable the interrupts for internal XPHY link up events */
#if IS_BCMCHIP(68375) 
            if (xphy_num == 4)
                intr_bits_xphy.xphy_link_up_intr_mask = 1;
            else
                intr_bits_gphy.xphy_link_up_intr_mask = MASK_BIT(xphy_num);
#else
            intr_bits_xphy.xphy_link_up_intr_mask = MASK_BIT(xphy_num);
#endif
#else
            printk("bcm_wake_on_lan() xport does not support link up interrupt\n");
            return -1;
#endif
        }
        else
        {
            /* Enable the interrupts for XPORT MPD events */
            intr_bits_xphy.mpd_intr_mask = MASK_BIT(xlmac_id);
        }

        WRITE_32(ETH_PHY_TOP_BASE + ETH_XPHY_SERDES_INTRL2_CPU_CLEAR, intr_bits_xphy);
        WRITE_32(ETH_PHY_TOP_BASE + ETH_XPHY_SERDES_INTRL2_CPU_MASK_CLEAR, intr_bits_xphy);

#if IS_BCMCHIP(68375) 
        WRITE_32(ETH_PHY_TOP_BASE + ETH_GPHY_RGMII_INTRL2_CPU_CLEAR, intr_bits_gphy);
        WRITE_32(ETH_PHY_TOP_BASE + ETH_GPHY_RGMII_INTRL2_CPU_MASK_CLEAR, intr_bits_gphy);
#endif
        if ( xphy_num != -1)
            power_down_xphy = 0;

        wake_type = WAKE_XPORT;
        wake_param = xlmac_id;
    }
    else
    {
        int irq = 0;

        if (!dt_is_valid(phy_dev->dt_handle))
        {
            printk("[%s:%d] Error\n",__FUNCTION__, __LINE__);
            rc = -1;
            goto Exit;
        }

        /* Enable the interrupts for external PHY MPD and link up events */
        if (is_lnk)
        {
            rc = bcm_bca_extintr_request_and_mask(&dt_dev->dev, phy_dev->dt_handle, "phy-link", dummy_isr,
                NULL, "PHY link up", NULL, &irq);

            if (rc)
            {
                printk("[%s:%d] Error\n",__FUNCTION__, __LINE__);
                goto Exit;
            }
        }
        else
        {
            rc = bcm_bca_extintr_request_and_mask(&dt_dev->dev, phy_dev->dt_handle, "phy-magic", dummy_isr,
                NULL, "PHY MPD", NULL, &irq);

            if (rc)
            {
                printk("[%s:%d] Error\n",__FUNCTION__, __LINE__);
                goto Exit;
            }
        }

        wake_type = WAKE_IRQ_WOL;
        wake_param = irq;
    }


    if ((rc = phy_dev_caps_get(phy_dev, CAPS_TYPE_ADVERTISE, &caps)))
        goto Exit;

    phy_speed = phy_caps_to_max_speed(caps);

    msleep(200);
    printk("setting phy speed to %dMbps\n", phy_speed);
    cascade_phy_dev_power_set(phy_dev, 1);
    cascade_phy_dev_speed_set(phy_dev, phy_speed, PHY_DUPLEX_FULL);
    phy_dev->link = -1;
    phy_dev->speed = phy_speed;
    phy_dev->duplex = PHY_DUPLEX_FULL;

    msleep(200);
    printk("setting mac speed to %dMbps\n", phy_speed);
    mac_dev_disable(mac_dev);
    mac_dev_cfg_get(mac_dev, &mac_cfg);
    mac_cfg.speed = phy_speed;
    mac_cfg.duplex = MAC_DUPLEX_FULL;
    mac_cfg.flag |= phy_dev_is_xgmii_mode(phy_dev) ? MAC_FLAG_XGMII : 0;
    mac_dev_cfg_set(mac_dev, &mac_cfg);
    mac_dev_enable(mac_dev);

#ifdef XPORT_0_IRQ
    rc += periph_intr_mask_set(3, XPORT_0_IRQ, 1);
#endif
#ifdef XPORT_1_IRQ
    rc += periph_intr_mask_set(3, XPORT_1_IRQ, 1);
#endif
#ifdef XPORT_2_IRQ
    rc += periph_intr_mask_set(3, XPORT_2_IRQ, 1);
#endif
#ifdef ETH_PHY_TOP_0_IRQ
    rc += periph_intr_mask_set(3, ETH_PHY_TOP_0_IRQ, 1);
#endif
#ifdef ETH_PHY_TOP_1_IRQ
    rc += periph_intr_mask_set(3, ETH_PHY_TOP_1_IRQ, 1);
#endif
#ifdef ETH_PHY_TOP_2_IRQ
    rc += periph_intr_mask_set(3, ETH_PHY_TOP_2_IRQ, 1);
#endif

    /* Enable the wake function required to reset the chip and then shut down
     * all the blocks except for SMC and the blocks waiting for wake signals (for example: XPORT) */
    rc = pmc_setup_wake_trig(wake_type, wake_param);

Exit:
    return rc;
}
EXPORT_SYMBOL(bcm_wake_on_lan);

int bcm_wake_on_button(void)
{
    int irq = 0, rc = 0;
    static int wake_initialized = 0;

    if (wake_initialized)
        return 0;

    rc = bcm_bca_extintr_request_and_mask(&dt_dev->dev, dt_handle, "wakeup-trigger-pin", dummy_isr,
        NULL, "Wake button", NULL, &irq);

    if (rc)
    {
        printk("[%s:%d] Error\n",__FUNCTION__, __LINE__);
        goto Exit;
    }

    rc = pmc_setup_wake_trig(WAKE_IRQ, irq);

    if (!rc)
        wake_initialized = 1;

Exit:
    return rc;
}
EXPORT_SYMBOL(bcm_wake_on_button);

int bcm_wake_on_timer(int minutes)
{
    return pmc_setup_wake_trig(WAKE_TIMER, minutes);
}
EXPORT_SYMBOL(bcm_wake_on_timer);

int bcm_wake_on_wan(void)
{
    printk("bcm_wake_on_wan\n");
    return 0;
}
EXPORT_SYMBOL(bcm_wake_on_wan);


int bcm_deepsleep_start(void)
{
    /* Power down all PHYs power supplies except for the one waiting for wake signal */
    phy_devices_shutdown();

    /* Power down the WAN power supplies */
    power_supplies_off("power-wan");

    /* Power down the other power supplies */
    power_supplies_off("power-misc");

    /* Power down the XPHY power supplies if not required to for wake signal */
    if (power_down_xphy)
        power_supplies_off("power-xphy");

    printk("activating deep sleep mode\n");
    return pmc_deep_sleep();
}
EXPORT_SYMBOL(bcm_deepsleep_start);
#endif
