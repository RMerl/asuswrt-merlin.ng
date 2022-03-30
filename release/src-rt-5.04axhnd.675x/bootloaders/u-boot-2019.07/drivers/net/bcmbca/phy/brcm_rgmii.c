// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2019 Broadcom Corporation
   All Rights Reserved

    
*/

#include "phy_drv.h"
#include "dt_access.h"
#include "brcm_rgmii.h"
#include "access_macros.h"

#ifdef __UBOOT__
#include <dm/device.h>
#include <dm/uclass.h>
#include <dm/read.h>
#include <linux/io.h>
#include <linux/ioport.h>
#else
#include <linux/of_platform.h>
#include <linux/io.h>
#include <linux/module.h>
#endif

#define MAX_RGMII_INSTANCES     5
static void __iomem *rgmii_base[MAX_RGMII_INSTANCES];
static void __iomem *rgmii_pad_base[MAX_RGMII_INSTANCES]; // used by v3, v5
static void __iomem *gpio_base;
static void __iomem *top_rgmii_base;                      // used by v6 

#define     RGMII_VER1  1   /* for PON devices          rgmii_ctrl   gpio_pad_ctrl */
#define     RGMII_VER2  2   /* for 63146 & 4912 devices rgmii_ctrl   gpio_pad_ctrl */
#define     RGMII_VER3  3   /* for 63178 device         rgmii_ctrl   rgmii_pad_ctrl */
#define     RGMII_VER4  4   /* for 47622 device         rgmii_ctrl   gpio_pad_ctrl */
#define     RGMII_VER5  5   /* for 63158,148,138 device rgmii_ctrl[] rgmii_pad_ctrl[] */
#define     RGMII_VER6  6   /* for 4908 device          rgmii_ctrl   gpio_pad_ctrl      top_rgmii_ctrl */
#define     RGMII_VER7  7   /* for 6756 device          rgmii_ctrl   gpio_pad_ctrl */
#define     RGMII_VER8  8   /* for 6855 device          rgmii_ctrl   gpio_pad_ctrl      top_rgmii_ctrl */
static int rgmii_ver = RGMII_VER1;

#define RGMII_CTRL_REG              (rgmii_base[params->instance] + 0x0000)
#define RGMII_IB_STATUS             (rgmii_base[params->instance] + 0x0004)
#define RGMII_RX_CLOCK_DELAY_CNTRL  (rgmii_base[params->instance] + 0x0008)

#define GPIO_PAD_CTRL               (gpio_base + 0x0040)
#define GPIO_TestPortBlkDataMsb     (gpio_base + 0x0054)
#define GPIO_TestPortBlkDataLsb     (gpio_base + 0x0058)
#define GPIO_TestPortCommand        (gpio_base + 0x005c)

#define LOAD_PAD_CTRL_CMD           0x22

#define RGMII_PAD_CTRL              (rgmii_pad_base[params->instance])
#define TOP_RGMII_CTRL              (top_rgmii_base + 0x0000)

static void bcm_set_padctrl(rgmii_params *params, unsigned int pin_num, unsigned int pad_ctrl)
{
    unsigned int tp_blk_data_msb, tp_blk_data_lsb, tp_cmd;

    printk("set RGMII pad ctrl for GPIO %d to 0x%08x\n", pin_num, pad_ctrl);
    tp_cmd = LOAD_PAD_CTRL_CMD;
    tp_blk_data_msb = 0;
    tp_blk_data_lsb = 0;
    tp_blk_data_lsb |= pin_num;
    tp_blk_data_lsb |= pad_ctrl;

    WRITE_32(GPIO_TestPortBlkDataMsb, tp_blk_data_msb);
    WRITE_32(GPIO_TestPortBlkDataLsb, tp_blk_data_lsb);
    WRITE_32(GPIO_TestPortCommand, tp_cmd);
}

static void bcm_misc_hw_xmii_pads_init(rgmii_params *params)
{
    int num;
    uint32_t tp_data;
    uint8_t drive_stength = 6; /* 14mA */

    for (num = 0; num < params->num_pins ;num++)
    {
        tp_data = 0;
        if (rgmii_ver == RGMII_VER1 || rgmii_ver == RGMII_VER8)
        {
            tp_data |= ((num < 6 ? 0 : 1) << 16);                       /* pad_ind - 0 for RX pads, 1 for TX pads */
            tp_data |= ((params->is_1p8v && num < 6 ? 1 : 0) << 15);    /* pad_amp_en - 1 for RX pads */
        }
        else if (rgmii_ver == RGMII_VER6)
        {
            tp_data |= ((params->is_1p8v ? 1 : 0) << 15);   /* pad_amp_en */
            tp_data |= ((params->is_3p3v ? 1 : 0) << 17);   /* pad_sel_gmii */
        }
        else if ((rgmii_ver == RGMII_VER2) || (rgmii_ver == RGMII_VER7)) 
        {
            tp_data |= (((params->is_1p8v && (num < 6)) ? 1 : 0) << 15);    /* pad_amp_en */
            tp_data |= ((num < 6 ? 0 : 1) << 16);       /*pad_ind - 0 for RX pads, 1 for TX pads */
        }
        else if (rgmii_ver == RGMII_VER4)
        {
            tp_data |= ((params->is_1p8v ? 1 : 0) << 15);   /* pad_amp_en */
            tp_data |= ((params->is_3p3v ? 1 : 0) << 17);   /* pad_sel_gmii */
            tp_data |= ((num < 6 ? 0 : 1) << 16);       /*pad_ind - 0 for RX pads, 1 for TX pads */
        }
        else
        {
            drive_stength = 3; /* 8mA */
            tp_data |= ((num < 6 ? 0 : 1) << 16);       /*pad_ind - 0 for RX pads, 1 for TX pads */
        }

        tp_data |= (drive_stength << 12);

        bcm_set_padctrl(params, params->pins[num], params->is_disabled ? 0 : tp_data);
    }
}

#include "asm/arch/BPCM.h"
#include "asm/arch/pmc_addr.h"
#include "pmc_drv.h"

static void pmc_rgmii_clk_en(void)
{
#if defined(CONFIG_BCM963146)
    { // turn on RGMII clk 250 en
        uint32_t data;
        ReadBPCMRegister(PMB_ADDR_EGPHY, BPCMETHRegOffset(rgmii_cntrl), &data);
        data |= (1 << 1);  // bpcm_clk_en
        WriteBPCMRegister(PMB_ADDR_EGPHY, BPCMETHRegOffset(rgmii_cntrl), data);
    }
#endif
#if defined(CONFIG_BCM94912)
    { // turn on RGMII clk 250 en
        uint32_t data;
        ReadBPCMRegister(PMB_ADDR_ETH, BPCMETHRegOffset(rgmii_cntrl), &data);
        // bpcm_pwrdown_n(b4)=1 bpcm_reset_n(b3)=1 bpcm_rgmii_en(b2)=0 bpcm_clk_en(b1)=1 bpcm_mux_sel(b0)=0
        data |= (1 << 1);  // bpcm_clk_en
        WriteBPCMRegister(PMB_ADDR_ETH, BPCMETHRegOffset(rgmii_cntrl), data);
    }
#endif
}

int rgmii_attach(rgmii_params *params)
{
    uint32_t val;

    if (params->instance < 0)
    {
        if (rgmii_ver == RGMII_VER5)
        {
            printk("Missing rgmii-intf entry\n");
            return -1;
        }
        params->instance = 0;
    }

    if (!rgmii_base[params->instance] || !gpio_base)
        return -1;

    if (rgmii_ver == RGMII_VER2)
        pmc_rgmii_clk_en();

    READ_32(RGMII_CTRL_REG, val);
    val |= (1 << 0); /* RGMII_MODE_EN=1 */
    val &= ~(7 << 2); /* Clear PORT_MODE */
    val |= (3 << 2); /* RGMII mode */

    if (params->delay_tx)
        val &= ~(1 << 1); /* ID_MODE_DIS=0 */
    else
        val |= (1 << 1); /* ID_MODE_DIS=1 */

    if (rgmii_ver == RGMII_VER2)
    {
        if (params->is_1p8v)
            val &= ~(1 << 17); /* MODE_HV=0 */
        else
            val |= (1 << 17); /* MODE_HV=1 */
    }

    WRITE_32(RGMII_CTRL_REG, val);

    switch (rgmii_ver) {
    case RGMII_VER1:
    case RGMII_VER8:
        val = (params->delay_rx) ? 0x08 : 0x28; break;
    case RGMII_VER7:
    case RGMII_VER2:    val = (params->delay_rx) ? 0xc8 : 0xe8 /*RXCLK_DLY_MODE_BYPASS*/; break;
    default:            val = (params->delay_rx) ? 0xc8 : 0xf8 /*ETHSW_RXCLK_IDDQ|ETHSW_RXCLK_BYPASS*/; break;
    }
    WRITE_32(RGMII_RX_CLOCK_DELAY_CNTRL, val);

    switch (rgmii_ver) {
    case RGMII_VER1:
            val = params->is_1p8v ? 0 : (1<<6);
            WRITE_32(TOP_RGMII_CTRL, val);
            bcm_misc_hw_xmii_pads_init(params);
            break;
    case RGMII_VER2:
            bcm_misc_hw_xmii_pads_init(params);
            break;
    case RGMII_VER3:
            READ_32(RGMII_PAD_CTRL, val);
            if (params->is_1p8v)
                val = (val & ~(1<<6)) | (1<<3);             // 1.8v: & ~MISC_XMII_PAD_MODEHV | MISC_XMII_PAD_AMP_EN
            else if (params->is_3p3v)
                val = (val | (1<<6)) | (1<<4) & ~(1<<3);    // 3.3v: | MISC_XMII_PAD_MODEHV | MISC_XMII_PAD_SEL_GMII & ~MISC_XMII_PAD_AMP_EN
            else
                val = (val | (1<<6)) & ~(1<<4) & ~(1<<3);   // 2.5v: | MISC_XMII_PAD_MODEHV & ~MISC_XMII_PAD_SEL_GMII & ~MISC_XMII_PAD_AMP_EN
            WRITE_32(RGMII_PAD_CTRL, val);
            break;
    case RGMII_VER5:
            if (params->is_1p8v)
                val = (1<<3);                   // 1.8v: MISC_XMII_PAD_AMP_EN
            else if (params->is_3p3v)
                val = (1<<6) | (1<<4);          // 3.3v: | MISC_XMII_PAD_MODEHV | MISC_XMII_PAD_SEL_GMII
            else
                val = (1<<6);                   // 2.5v: | MISC_XMII_PAD_MODEHV
            val |= 6 /* 14mA */;
            WRITE_32(RGMII_PAD_CTRL, val);
            break;
    case RGMII_VER4:
            READ_32(GPIO_PAD_CTRL, val);
            if (params->is_1p8v)
                val = (val & ~(1<<8)) | (1<<10);             // 1.8v: & ~MISC_XMII_PAD_MODEHV | MISC_XMII_PAD_AMP_EN
            else if (params->is_3p3v)
                val = (val | (1<<8)) | (1<<9) & ~(1<<10);    // 3.3v: | MISC_XMII_PAD_MODEHV | MISC_XMII_PAD_SEL_GMII & ~MISC_XMII_PAD_AMP_EN
            else
                val = (val | (1<<8)) & ~(1<<9) & ~(1<<10);   // 2.5v: | MISC_XMII_PAD_MODEHV & ~MISC_XMII_PAD_SEL_GMII & ~MISC_XMII_PAD_AMP_EN
            WRITE_32(GPIO_PAD_CTRL, val);
            bcm_misc_hw_xmii_pads_init(params);
            break;
    case RGMII_VER6:
            val = (1<<1);           /* RGMII_PAD_ENABLE */
            if (!params->is_1p8v)
                val |= (1<<0);      /* RGMII_PAD_MODEHV */
            WRITE_32(TOP_RGMII_CTRL, val);
            bcm_misc_hw_xmii_pads_init(params);
            break;
    case RGMII_VER7:
            READ_32(GPIO_PAD_CTRL, val);
            if (params->is_1p8v)
                val &= ~(1 << 8);
            else
                val |= (1 << 8); /* rgmii_0_pad_modehv = 1 */
            WRITE_32(GPIO_PAD_CTRL, val);
            bcm_misc_hw_xmii_pads_init(params);
            break;
    case RGMII_VER8:
            val = params->is_1p8v ? 0 : 1;
            WRITE_32(TOP_RGMII_CTRL, val);
            bcm_misc_hw_xmii_pads_init(params);
            break;
    }

    return 0;
}
EXPORT_SYMBOL(rgmii_attach);

int rgmii_ib_status_override(int instance, int speed, int duplex)
{
    uint32_t val = 0;
    rgmii_params _params = { .instance = instance };
    rgmii_params *params = &_params;

    val |= (1 << 4); /* IB_STATUS_OVRD=1 */

    if (speed != PHY_SPEED_UNKNOWN)
        val |= (1 << 3); /* LINK_DECODE=1 */

    if (duplex == PHY_DUPLEX_FULL)
        val |= (1 << 2); /* DUPLEX_DECODE=1 */

    if (speed == PHY_SPEED_10)
        val |= (0 << 0); /* SPEED_DECODE=0 */

    if (speed == PHY_SPEED_100)
        val |= (1 << 0); /* SPEED_DECODE=1 */

    if (speed == PHY_SPEED_1000)
        val |= (2 << 0); /* SPEED_DECODE=2 */

    if (speed == PHY_SPEED_2500)
        val |= (3 << 0); /* SPEED_DECODE=3 */

    WRITE_32(RGMII_IB_STATUS, val);

    return 0;
}
EXPORT_SYMBOL(rgmii_ib_status_override);

static int rgmii_probe(dt_device_t *pdev)
{
    int ret, instance = 0;

    rgmii_ver = (uint32_t) pdev->driver_data;
    rgmii_base[instance] = dt_dev_remap_resource(pdev, 0);
    if (IS_ERR(rgmii_base[instance]))
    {
        ret = PTR_ERR(rgmii_base[instance]);
        goto Exit;
    }

    gpio_base = dt_dev_remap(pdev, 1);
    if (IS_ERR(gpio_base))
    {
        ret = PTR_ERR(gpio_base);
        goto Exit;
    }

    if (rgmii_ver == RGMII_VER1 || rgmii_ver == RGMII_VER6 || rgmii_ver == RGMII_VER8)
    {
        top_rgmii_base = dt_dev_remap(pdev, 2);
        
        if (IS_ERR(top_rgmii_base))
        {
            ret = PTR_ERR(top_rgmii_base);
            goto Exit;
        }
    }
    else if (rgmii_ver == RGMII_VER5)
    {
        rgmii_pad_base[instance] = gpio_base;
        for (instance=1; instance<MAX_RGMII_INSTANCES;instance++)
        {
            rgmii_base[instance] = dt_dev_remap_resource(pdev, instance*2);
            if (IS_ERR(rgmii_base[instance]))
            {
                ret = 0;
                break;
            }

            rgmii_pad_base[instance] = dt_dev_remap(pdev, instance*2+1);
            if (IS_ERR(rgmii_pad_base[instance]))
            {
                ret = 0;
                break;
            }
        }
    }
    else if (rgmii_ver == RGMII_VER3)
    {
        rgmii_pad_base[instance] = gpio_base;
    }

    dev_info(&pdev->dev, "registered\n");

    return 0;

Exit:
    gpio_base = NULL;
    rgmii_base[instance] = NULL;
    return ret;
}

static const struct udevice_id rgmii_ids[] = {
    { .compatible = "brcm,rgmii1", .data = (const void *)RGMII_VER1 },
    { .compatible = "brcm,rgmii2", .data = (const void *)RGMII_VER2 },
    { .compatible = "brcm,rgmii3", .data = (const void *)RGMII_VER3 },
    { .compatible = "brcm,rgmii4", .data = (const void *)RGMII_VER4 },
    { .compatible = "brcm,rgmii5", .data = (const void *)RGMII_VER5 },
    { .compatible = "brcm,rgmii6", .data = (const void *)RGMII_VER6 },
    { .compatible = "brcm,rgmii7", .data = (const void *)RGMII_VER7 },
    { .compatible = "brcm,rgmii8", .data = (const void *)RGMII_VER8 },
    { /* end of list */ },
};

U_BOOT_DRIVER(brcm_rgmii) = {
    .name	= "brcm-rgmii",
    .id	= UCLASS_MISC,
    .of_match = rgmii_ids,
    .probe = rgmii_probe,
};
