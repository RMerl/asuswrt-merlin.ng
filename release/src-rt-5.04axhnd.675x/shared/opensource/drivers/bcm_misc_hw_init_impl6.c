/*
 * <:copyright-BRCM:2015:DUAL/GPL:standard
 * 
 *    Copyright (c) 2015 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */

// BCMFORMAT: notabs reindent:uncrustify:bcm_minimal_i4.cfg
#include "boardparms.h"
#include "bcm_map_part.h"
#include "bcm_misc_hw_init.h"

#ifdef _CFE_
#include "lib_types.h"
#include "lib_printf.h"
#include "lib_string.h"
#include "cfe_timer.h"
#include "bcm_map.h"
#define printk  printf
#define udelay  cfe_usleep
#else // Linux
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/of.h>
#include "bcm_hwdefs.h"
#ifndef DT_SUPPORT_ONLY
#include "board.h"
#endif
#include "bcm_rsvmem.h"
#include "bcm_otp.h"
#if !defined(CONFIG_BCM963178) && !defined(_BCM963178_) && !defined(CONFIG_BCM947622) && !defined(_BCM947622_) && !defined(CONFIG_BCM96855) && !defined(CONFIG_BCM96756) && \
    (!defined(NONETWORK) || NONETWORK==0)
#include "rdpa_types.h"
#endif
#include "bcm_intr.h"
#endif
#include "bcm_ubus4.h"
#if defined(CONFIG_BCM963158) || defined(_BCM963158_) ||  defined(CONFIG_BCM963146) || defined(_BCM963146_) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
#include "pmc_wan.h"
#endif

#if defined(CONFIG_BCM963178) || defined(_BCM963178_) || defined(CONFIG_BCM963146) || defined(_BCM963146_) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
#include "clk_rst.h"
#endif

#if defined(__KERNEL__) && (defined(CONFIG_BCM_XRDP) || defined(CONFIG_BCM_RDP))
struct device *rdp_dummy_dev = NULL;
EXPORT_SYMBOL(rdp_dummy_dev);
#endif


#if defined(CONFIG_BCM94908) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813) || defined(CONFIG_BCM96756)
static void bcm_misc_hw_rcal(void)
{
    /* start the resistor calibrator by setting RSTB and then clearing the PWRDN bit */
#if defined(CONFIG_BCM94908) || defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813) || defined(CONFIG_BCM96756)
#if defined(CONFIG_BCM96756)
    int val = 0;

    /* only do calibration when chip enable rescal feature */
    bcm_otp_is_rescal_enabled(&val);
    if (!val)
        return;
#endif
    TOPCTRL->RescalIPCtrl |= RESCAL_RSTB;
    udelay(10);
    TOPCTRL->RescalIPCtrl &= ~RESCAL_PWRDN;
    udelay(10);
#endif
#if defined(CONFIG_BCM963158)
    WAN_TOP->WAN_TOP_RESCAL_CFG |= RESCAL_RSTB;
    udelay(10);
    WAN_TOP->WAN_TOP_RESCAL_CFG &= ~RESCAL_PWRDN;
    while(!(WAN_TOP->WAN_TOP_RESCAL_STATUS_0&RESCAL_DONE));
#endif
}
#endif


#if !defined(_CFE_) && !defined(CONFIG_BCM963178) && !defined(_BCM963178_) && !defined(CONFIG_BCM947622) && !defined(_BCM947622_) && \
    !defined(CONFIG_BCM96878) && !defined(CONFIG_BCM96855) && !defined(CONFIG_BCM96756)
int bcm_misc_xfi_port_get(void)
{
    struct device_node *np, *child, *sec;
    char *phy_mode;
    int port_index;
    const unsigned int *port_reg;
    rdpa_emac emac = rdpa_emac_none;

    if (!(np = of_find_compatible_node(NULL, NULL, "brcm,enet")))
        return rdpa_emac_none;

    for_each_available_child_of_node(np, sec)
    {
        for_each_available_child_of_node(sec, child)
        {
            port_reg = of_get_property(child, "reg", NULL);
            if (!port_reg)
                continue;

            port_index = be32_to_cpup(port_reg);
            if ((phy_mode = (char *)of_get_property(child, "phy-mode", NULL)) &&
                !strcmp("xfi", phy_mode))
            {
                emac = (rdpa_emac)(rdpa_emac0 + port_index);
                break;
            }
        }
    }

    of_node_put(np);
    return emac;
}
EXPORT_SYMBOL(bcm_misc_xfi_port_get);

uint32_t bcm_misc_g9991_phys_port_vec_get(void)
{
    struct device_node *np, *child, *sec;
    int port_index;
    const unsigned int *port_reg;
    uint32_t vec = 0;

    if (!(np = of_find_compatible_node(NULL, NULL, "brcm,enet")))
        return rdpa_emac_none;

    for_each_available_child_of_node(np, sec)
    {
        for_each_available_child_of_node(sec, child)
        {
            port_reg = of_get_property(child, "reg", NULL);
            if (!port_reg)
                continue;

            port_index = be32_to_cpup(port_reg);
            if (of_get_property(child, "link", NULL))
                vec |= (1 << port_index);
        }
    }

    of_node_put(np);
    return vec;
}
EXPORT_SYMBOL(bcm_misc_g9991_phys_port_vec_get);

int runner_reserved_memory_get(uint8_t **bm_base_addr,
                               uint8_t **bm_base_addr_phys,
                               unsigned int *bm_size,
                               uint8_t **fm_base_addr,
                               uint8_t **fm_base_addr_phys,
                               unsigned int *fm_size)
{
    int rc;
    phys_addr_t phy_addr;

    rc = BcmMemReserveGetByName(BUFFER_MEMORY_BASE_ADDR_STR,
                                (void **)bm_base_addr, &phy_addr, bm_size);
    if (unlikely(rc)) {
        printk("%s %s Failed to get buffer memory, rc(%d)\n",
               __FILE__, __func__, rc);
        return rc;
    }
    *bm_base_addr_phys = (uint8_t *)phy_addr;

    rc = BcmMemReserveGetByName(FLOW_MEMORY_BASE_ADDR_STR,
                                (void **)fm_base_addr, &phy_addr, fm_size);
    if (unlikely(rc)) {
        printk("Failed to get valid flow memory, rc = %d\n", rc);
        return rc;
    }
    *fm_base_addr_phys = (uint8_t *)phy_addr;

    memset(*bm_base_addr, 0x00, *bm_size);
    memset(*fm_base_addr, 0x00, *fm_size);

    printk("bm_base_addr 0x%px, size %u, bm_base_addr_phys 0x%px\n",
           *bm_base_addr, *bm_size, *bm_base_addr_phys);
   
    printk("fm_base_addr 0x%px, size %u, fm_base_addr_phys 0x%px\n",
           *fm_base_addr, *fm_size, *fm_base_addr_phys);

    *bm_size = *bm_size >> 20;	/* convert from Byte to MB */
    *fm_size = *fm_size >> 20;	/* convert from Byte to MB */

    return rc;
}
EXPORT_SYMBOL(runner_reserved_memory_get);
#endif

int rdp_shut_down(void)
{
    /*put all RDP modules in reset state*/
    // TBD. pmcPutAllRdpModulesInReset();
    return 0;
}
#ifndef _CFE_
EXPORT_SYMBOL(rdp_shut_down);
#endif

#if defined(__KERNEL__) && (defined(CONFIG_BCM_XRDP) || defined(CONFIG_BCM_RDP))
static void alloc_rdp_dummy_device(void)
{
    if (rdp_dummy_dev == NULL) {
        rdp_dummy_dev = kzalloc(sizeof(struct device), GFP_ATOMIC);

#ifdef CONFIG_BCM_GLB_COHERENCY
        arch_setup_dma_ops(rdp_dummy_dev, 0, 0, NULL, true);
#else
        arch_setup_dma_ops(rdp_dummy_dev, 0, 0, NULL, false);
#endif

#if defined(CONFIG_BCM96858)
        /* need to confirm how many bits we support in 6858 runner */
        dma_set_coherent_mask(rdp_dummy_dev, DMA_BIT_MASK(40));
#else
        dma_coerce_mask_and_coherent(rdp_dummy_dev, DMA_BIT_MASK(32));
#endif
    }
}
#endif
#if !defined(CONFIG_DT_SUPPORT_ONLY) && defined(CONFIG_BCM96858)
extern void bcm_gpio_set_data(unsigned int, unsigned int);
extern void bcm_gpio_set_dir(unsigned int gpio_num, unsigned int dir);

static void configure_xfi_optic_phy(void)
{
    bcm_gpio_set_dir(52, 1);
    bcm_gpio_set_data(52, 0);
}
#endif

int bcm_misc_hw_init(void)
{
#if defined(CONFIG_BCM94908) || defined(CONFIG_BCM963158) || \
    defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813) || \
    defined(CONFIG_BCM96756)
    bcm_misc_hw_rcal();
#endif

#if defined(__KERNEL__) && (defined(CONFIG_BCM_XRDP) || defined(CONFIG_BCM_RDP) || defined(CONFIG_BCM96855))
   alloc_rdp_dummy_device();
#endif

#if !defined(CONFIG_DT_SUPPORT_ONLY) && (defined(_BCM96858_) || defined(CONFIG_BCM96858))
    configure_xfi_optic_phy();
#endif

    return 0;
}

int bcm_ubus_init(void)
{
#if !defined(_BCM94908_) && !defined(CONFIG_BCM94908)
    bcm_ubus_config();
#endif

#if defined(CONFIG_BCM963178)
    configure_ubus_sar_reg_decode();
#endif

    return 0;
}
#ifndef _CFE_
arch_initcall(bcm_misc_hw_init);
subsys_initcall(bcm_ubus_init);
#endif
