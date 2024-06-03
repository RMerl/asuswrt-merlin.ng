/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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
 *  Created on: Dec 2016
 *      Author: steven.hsieh@broadcom.com
 */

/*
 * Phy drivers for 63138, 63148, 4908
 */

 #include "phy_drv_dsl_phy.h"
 #include "phy_drv_brcm.h"

sw_base_t sw_mmap_base;
EXPORT_SYMBOL(sw_mmap_base);
int gphy_base_addr;

#include "bcm_ioremap_shared.h"

static int swblks_probe(dt_device_t *pdev)
{
    struct resource *res;
    dt_handle_t node = dt_dev_get_handle(pdev);
    
    if (!pdev) return 0;

    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "switchcore-base");
    if (res && !IS_ERR((void*)res->start)) {
        sw_mmap_base.core_base_phy = (void *)res->start;
        sw_mmap_base.core_base = devm_ioremap_resource(&pdev->dev, res);
    }

    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "switchreg-base");
    if (res && !IS_ERR((void*)res->start))
        sw_mmap_base.swreg_base = devm_ioremap_shared_resource (&pdev->dev, res);

    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "switchmdio-base");
    if (res && !IS_ERR((void*)res->start))
        sw_mmap_base.mdio_base = devm_ioremap_shared_resource (&pdev->dev, res);
        
    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "switchfcb-base");
    if (res && !IS_ERR((void*)res->start))
        sw_mmap_base.fcb_base = devm_ioremap_resource(&pdev->dev, res);
        
    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "switchacb-base");
    if (res && !IS_ERR((void*)res->start))
        sw_mmap_base.acb_base = devm_ioremap_resource(&pdev->dev, res);
        
    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "switchlfh-base");
    if (res && !IS_ERR((void*)res->start))
        sw_mmap_base.lfh_base = devm_ioremap_resource(&pdev->dev, res);
        
    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "xbar-ctrl");
    if (res && !IS_ERR((void*)res->start))
        sw_mmap_base.xbar_ctrl = devm_ioremap_resource(&pdev->dev, res);

    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "phy-test-ctrl");
    if (res && !IS_ERR((void*)res->start))
        sw_mmap_base.phy_test_ctrl = devm_ioremap_resource(&pdev->dev, res);

    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "qphy-ctrl");
    if (res && !IS_ERR((void*)res->start))
        sw_mmap_base.qphy_ctrl = devm_ioremap_resource(&pdev->dev, res);

    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "xport0-clk-ctrl");
    if (res && !IS_ERR((void*)res->start))
        sw_mmap_base.xport0_clk_ctrl = devm_ioremap_resource(&pdev->dev, res);

    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "sphy-ctrl");
    if (res && !IS_ERR((void*)res->start))
        sw_mmap_base.sphy_ctrl = devm_ioremap_resource(&pdev->dev, res);

    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "bcast-ctrl");
    if (res && !IS_ERR((void*)res->start))
    {
        void __iomem *bcast_ctrl = devm_ioremap_resource(&pdev->dev, res);
        uint32_t val = *((uint32_t *)bcast_ctrl);
        val &= ~(1<<8); // disable bcast mode
        *((uint32_t *)bcast_ctrl) = val;
        devm_iounmap(&pdev->dev, bcast_ctrl);
    }

    gphy_base_addr = dt_property_read_u32_default(node, "phy_base", 1);

    dev_info(&pdev->dev, "registered\n");

    return 0;
}

static const struct of_device_id of_platform_table[] = {
    { .compatible = "brcm,swblks" },
    { /* end of list */ },
};

static struct platform_driver of_platform_driver = {
    .driver = {
        .name = "swblks",
        .of_match_table = of_platform_table,
    },
    .probe = swblks_probe,
};
module_platform_driver(of_platform_driver);


#if defined(QPHY_CNTRL)
 static phy_dev_t *qphy_devs[4];
 #define QPHY0_ADDR          gphy_base_addr
 #define IS_QPHY(phy_dev)    (phy_dev->addr-gphy_base_addr<4)
#else
 #define IS_QPHY(phy_dev)    0
#endif
#if defined(SPHY_CNTRL)
 static phy_dev_t *sphy_dev;
 #if defined(QPHY_CNTRL)
  #define SPHY_ADDR          (gphy_base_addr+4)
  #define IS_SPHY(phy_dev)   (phy_dev->addr==gphy_base_addr+4)
 #else
  #define SPHY_ADDR          gphy_base_addr
  #define IS_SPHY(phy_dev)   (phy_dev->addr==gphy_base_addr+0)
 #endif
#else
 #define IS_SPHY(phy_dev)    0
#endif

// QGPHYs and SPHY after reset will only advertise speed with full duplex
// setup phy to advertise half duplex also.
static void _advertise_supported_caps(phy_dev_t *phy_dev)
{
    uint32_t caps, adv;

    phy_dev_caps_get(phy_dev, CAPS_TYPE_SUPPORTED, &caps);
    // backward compatible disable PAUSE, PAUSE_ASM
    caps &= ~ (PHY_CAP_PAUSE | PHY_CAP_PAUSE_ASYM);
    
    phy_dev_caps_get(phy_dev, CAPS_TYPE_ADVERTISE, &adv);
    phy_dev_caps_set(phy_dev, caps|adv);
}

int dsl_phy_reset(phy_dev_t *phy_dev)
{
    u16 v16;
    u32 reg;
    int i=0;

    /* Reset PHY to clear status first */

#if defined(PHY_EXT3)
    if (IsC45Phy(phy_dev))
    {
        reg = CL45_REG_IEEE_CTRL;
        v16= MII_CONTROL_RESET;
        phy_bus_c45_write32(phy_dev, reg, v16);
        for(phy_bus_c45_read32(phy_dev, reg, &v16); v16 & MII_CONTROL_RESET;
                phy_bus_c45_read32(phy_dev, reg, &v16))
        {
            if (++i > 20) {printk("Failed to reset 0x%x\n", phy_dev->addr); return 0;}
            msleep(100);
        }
    }
    else
#endif
    {
        reg = MII_CONTROL;
        v16= MII_CONTROL_RESET;
        phy_bus_write(phy_dev, reg, v16);
        for(phy_bus_read(phy_dev, reg, &v16); v16 & MII_CONTROL_RESET;
                phy_bus_read(phy_dev, reg, &v16))
        {
            if (++i > 20) {printk("Failed to reset 0x%x\n", phy_dev->addr); return 0;}
            msleep(100);
        }

        if (phy_dev->phy_drv->phy_type == PHY_TYPE_DSL_GPHY) {
            _advertise_supported_caps(phy_dev);
            brcm_shadow_18_force_auto_mdix_set(phy_dev, 1);
        }
    }

    return 1;
}

#if defined(CONFIG_BCM963138)

#define _phy_afe_cfg(phy_dev) {\
        phy_bus_write(phy_dev, MII_CONTROL, MII_CONTROL_RESET|MII_CONTROL_AN_ENABLE|MII_CONTROL_DUPLEX_MODE|MII_CONTROL_SPEED_SEL6); \
        udelay(100); \
        brcm_misc_write(phy_dev, 0x38, 0x1, 0x9b2f);     /*AFE_RXCONFIG_1 Provide more margin for INL/DNL measurement on ATE */ \
        brcm_misc_write(phy_dev, 0x39, 0x0, 0x0431);     /*AFE_TX_CONFIG Set 100BT Cfeed=011 to improve rise/fall time */ \
        brcm_misc_write(phy_dev, 0x39, 0x1, 0xa7da);     /*AFE_VDAC_ICTRL_0 Set Iq=1101 instead of 0111 for improving AB symmetry */ \
        brcm_misc_write(phy_dev, 0x3a, 0x0, 0x00e3); }   /*AFE_HPF_TRIM_OTHERS Set 100Tx/10BT to -4.5% swing & Set rCal offset for HT=0 code */

static void dsl_phy_afe_pll_setup(phy_dev_t *phy_dev)
{
    int i;
    static int qphy_afe_pll_done = 0;

    if (!IS_QPHY(phy_dev) && !IS_SPHY(phy_dev))
        return;
    if (IS_QPHY(phy_dev) && qphy_afe_pll_done)
        return;

    // reset PHY
    if (phy_dev == sphy_dev)
        _phy_afe_cfg(phy_dev)
    else
        for (i=0; i<4; i++)
            if (qphy_devs[i])
                _phy_afe_cfg(qphy_devs[i])

    //CORE_BASE1E Force trim overwrite and set I_ext trim to 0000
    phy_bus_write(phy_dev, 0x1e, 0x10);

    if (phy_dev == sphy_dev)
        brcm_misc_write(phy_dev, 0xa, 0x0, 0x011b);      //Adjust bias current trim by +4% swing, +2 tick 'DSP_TAP10
    else
        for (i=0; i<4; i++)
            if (qphy_devs[i])
                brcm_misc_write(qphy_devs[i], 0xa, 0x0, 0x011b);      //Adjust bias current trim by +4% swing, +2 tick 'DSP_TAP10

    brcm_exp_write(phy_dev, 0xb0, 0x10);   //Reset R_CAL/RC_CAL Engine 'CORE_EXPB0
    brcm_exp_write(phy_dev, 0xb0, 0x0);    //Disable Reset R_CAL/RC_CAL Engine 'CORE_EXPB0

    if (phy_dev != sphy_dev)
        qphy_afe_pll_done = 1;
}

#elif defined(CONFIG_BCM963148)

#define _phy_afe_cfg(phy_dev) {\
        phy_bus_write(phy_dev, MII_CONTROL, MII_CONTROL_RESET|MII_CONTROL_AN_ENABLE|MII_CONTROL_DUPLEX_MODE|MII_CONTROL_SPEED_SEL6); \
        udelay(100); \
        brcm_misc_write(phy_dev, 0x38, 0x0, 0xeb15);    /*AFE_RXCONFIG_0 */ \
        brcm_misc_write(phy_dev, 0x38, 0x1, 0x9b2f);    /*AFE_RXCONFIG_1. Replacing the previously suggested 0x9AAF for SS part. See JIRA HW63148-31 */ \
        brcm_misc_write(phy_dev, 0x38, 0x2, 0x2003);    /*AFE_RXCONFIG_2 */ \
        brcm_misc_write(phy_dev, 0x38, 0x3, 0x7fc0);    /*AFE_RX_LP_COUNTER */ \
        brcm_misc_write(phy_dev, 0x39, 0x0, 0x0060);    /*AFE_TX_CONFIG */ \
        brcm_misc_write(phy_dev, 0x39, 0x1, 0xa7da);    /*AFE_VDAC_ICTRL_0 */ \
        brcm_misc_write(phy_dev, 0x39, 0x3, 0xa020);    /*AFE_VDAC_OTHERS_0 */ \
        brcm_misc_write(phy_dev, 0x3a, 0x0, 0x00e3); }  /*AFE_HPF_TRIM_OTHERS */ 

static void dsl_phy_afe_pll_setup(phy_dev_t *phy_dev)
{
    int i;
    static int qphy_afe_pll_done = 0;

    if (!IS_QPHY(phy_dev) && !IS_SPHY(phy_dev))
        return;
    if (IS_QPHY(phy_dev) && qphy_afe_pll_done)
        return;

    // reset PHY
    if (phy_dev == sphy_dev)
        _phy_afe_cfg(phy_dev)
    else
        for (i=0; i<4; i++)
            if (qphy_devs[i])
                _phy_afe_cfg(qphy_devs[i])

    //CORE_BASE1E Force trim overwrite and set I_ext trim to 0000
    phy_bus_write(phy_dev, 0x1e, 0x10);

    if (phy_dev == sphy_dev)
        brcm_misc_write(phy_dev, 0xa, 0x0, 0x111b);      //Adjust bias current trim by +4% swing, +2 tick, increase PLL BW in GPHY link start up training 'DSP_TAP10
    else
        for (i=0; i<4; i++)
            if (qphy_devs[i])
                brcm_misc_write(qphy_devs[i], 0xa, 0x0, 0x111b);      //Adjust bias current trim by +4% swing, +2 tick, increase PLL BW in GPHY link start up training 'DSP_TAP10

    brcm_exp_write(phy_dev, 0xb0, 0x10);   //Reset R_CAL/RC_CAL Engine 'CORE_EXPB0
    brcm_exp_write(phy_dev, 0xb0, 0x0);    //Disable Reset R_CAL/RC_CAL Engine 'CORE_EXPB0

    if (phy_dev != sphy_dev)
        qphy_afe_pll_done = 1;
}

#elif defined(CONFIG_BCM94908)

#define _phy_afe_cfg(phy_dev) {\
        phy_bus_write(phy_dev, MII_CONTROL, MII_CONTROL_RESET|MII_CONTROL_AN_ENABLE|MII_CONTROL_DUPLEX_MODE|MII_CONTROL_SPEED_SEL6); \
        udelay(100); \
        brcm_misc_write(phy_dev, 0x38, 0x1, 0x9b2f);    /*AFE_RXCONFIG_1 Provide more margin for INL/DNL measurement on ATE */ \
        brcm_misc_write(phy_dev, 0x39, 0x0, 0x0431);    /*AFE_TX_CONFIG Set 1000BT Cfeed=011 to improve rise/fall time */ \
        brcm_misc_write(phy_dev, 0x39, 0x1, 0xa7da);    /*AFE_VDAC_ICTRL_0 Set Iq=1101 instead of 0111 for improving AB symmetry */ \
        brcm_misc_write(phy_dev, 0x3a, 0x0, 0x00e3);    /*AFE_HPF_TRIM_OTHERS Set 100Tx/10BT to -4.5% swing & Set rCal offset for HT=0 code */ \
        brcm_misc_write(phy_dev, 0x0a, 0x0, 0x011b); }  /*DSP_TAP10  Adjust I_int bias current trim by +0% swing, +0 tick */

static void dsl_phy_afe_pll_setup(phy_dev_t *phy_dev)
{
    int i;
    static int qphy_afe_pll_done = 0;

    if (!IS_QPHY(phy_dev) && !IS_SPHY(phy_dev))
        return;
    if (IS_QPHY(phy_dev) && qphy_afe_pll_done)
        return;

    // reset PHY
    if (phy_dev == sphy_dev)
        _phy_afe_cfg(phy_dev)
    else
        for (i=0; i<4; i++)
            if (qphy_devs[i])
                _phy_afe_cfg(qphy_devs[i])

    brcm_exp_write(phy_dev, 0xb0, 0x10);   //Reset R_CAL/RC_CAL Engine 'CORE_EXPB0
    brcm_exp_write(phy_dev, 0xb0, 0x0);    //Disable Reset R_CAL/RC_CAL Engine 'CORE_EXPB0

    if (phy_dev != sphy_dev)
        qphy_afe_pll_done = 1;
}

#elif defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622) || defined(CONFIG_BCM96756)

#define _phy_afe_cfg(phy_dev) {\
        phy_bus_write(phy_dev, MII_CONTROL, MII_CONTROL_RESET|MII_CONTROL_AN_ENABLE|MII_CONTROL_DUPLEX_MODE|MII_CONTROL_SPEED_SEL6); \
        udelay(100); \
        brcm_misc_write( phy_dev, 0x39, 0x1, 0x0000 );     /*AFE_TX_CONFIG_0 Turn off AOF */ \
        brcm_misc_write( phy_dev, 0x3a, 0x2, 0x0BCC );     /*AFE_TX_CONFIG_1 1g AB symmetry Iq */ \
        brcm_misc_write( phy_dev, 0x39, 0x0, 0x233F );     /*AFE_TX_IQ_RX_LP LPF BW */ \
        brcm_misc_write( phy_dev, 0x3b, 0x0, 0xAD40 );     /*AFE_TEMPSEN_OTHERS  RCAL +6LSB to make impedance from 112 to 100ohm */ \
        brcm_misc_write( phy_dev, 0x0a, 0x0, 0x091B );     /*DSP_TAP10  since rcal make R smaller, make master current -4% */ \
        /*From EEE excel config file for Vitesse fix */ \
        brcm_misc_write( phy_dev, 0x21, 0x2, 0x87F6 );     /* rx_on_tune 8 -> 0xf */ \
        brcm_misc_write( phy_dev, 0x22, 0x2, 0x017D );     /* 100tx EEE bandwidth */ \
        brcm_misc_write( phy_dev, 0x26, 0x2, 0x0015 ); }   /* enable ffe zero det for Vitesse interop */

static void dsl_phy_afe_pll_setup(phy_dev_t *phy_dev)
{
#if defined(QPHY_CNTRL)
    int i;
#endif
    static int qphy_afe_pll_done = 0;

    if (!IS_QPHY(phy_dev) && !IS_SPHY(phy_dev))
        return;
    if (IS_QPHY(phy_dev) && qphy_afe_pll_done)
        return;

    // reset PHY
    if (phy_dev == sphy_dev)
        _phy_afe_cfg(phy_dev)
#if defined(QPHY_CNTRL)
    else
        for (i=0; i<4; i++)
            if (qphy_devs[i])
                _phy_afe_cfg(qphy_devs[i])
#endif

    brcm_exp_write(phy_dev, 0xb0, 0x10);   //Reset R_CAL/RC_CAL Engine 'CORE_EXPB0
    brcm_exp_write(phy_dev, 0xb0, 0x0);    //Disable Reset R_CAL/RC_CAL Engine 'CORE_EXPB0

    if (phy_dev != sphy_dev)
        qphy_afe_pll_done = 1;
}

#elif defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)

static void _phy_run_cal(phy_dev_t *phy_dev, u16 *rcalnewcode11_p)
{
    u16 expa9, rcalcode, rcalnewcodelp;

    // Run RCAL on AFE_CAL_CONFIG_2 = 0x3 
    brcm_misc_write(phy_dev, 0x39, 0x3, 0x38);      // enable max averaging
    brcm_misc_write(phy_dev, 0x39, 0x3, 0x3b);      // no reset, analog powerup
    udelay(1000);
    brcm_misc_write(phy_dev, 0x39, 0x3, 0x3f);      // start calibration
    udelay(1000);

    // Run RCCAL on AFE_CAL_CONFIG_0 = 0x1 
    brcm_misc_write(phy_dev, 0x39, 0x1, 0x1c82);    // Vref=1000, Target=10, averaging enabled
    brcm_misc_write(phy_dev, 0x39, 0x1, 0x9e82);    // no reset, analog powerup
    udelay(1000);
    brcm_misc_write(phy_dev, 0x39, 0x1, 0x9f82);    // start calibration
    udelay(1000);
    brcm_misc_write(phy_dev, 0x39, 0x1, 0x9e86);    // clear start calibration, set HiBW
    udelay(1000);
    brcm_misc_write(phy_dev, 0x39, 0x1, 0x9f86);    // start calibration with hi BW mode set
    udelay(1000);

    // TX Amplitude finetune
#if defined(CONFIG_BCM963146)
    brcm_misc_write(phy_dev, 0x38, 0x1, 0xe3ea); // AFE_BIAS_CONFIG_0, Adjust 10BT amplitude additional +7% and 100BT +2%
    brcm_misc_write(phy_dev, 0x38, 0x2, 0xede0); // AFE_BIAS_CONFIG_1, Adjust 1G mode amplitude and 1G testmode1
#else
    //brcm_misc_write(phy_dev, 0x38, 0x1, 0xe3ea); // AFE_BIAS_CONFIG_0, Adjust 10BT amplitude additional +7%, 100BT= E3 Commented as it is reduntant, as it is overwritten a little later below.
    brcm_misc_write(phy_dev, 0x38, 0x2, 0xefe2); // AFE_BIAS_CONFIG_1, Adjust 1G mode amplitude and 1G testmode1, TM1=EF (4915B0), 1G=E2 (4915B0)
#endif

    // Adjust 10BT bias and RCAL settings 
    // read CORE_EXPA9 
    brcm_exp_read(phy_dev, 0xa9, &expa9);           // read CORE_EXPA9
    rcalcode = (expa9 & 0x007e) >> 1;               // expA9<6:1> is rcalcode<5:0>
    rcalnewcodelp = rcalcode + 16;
    *rcalnewcode11_p = rcalcode + 10;
    if (rcalnewcodelp > 0x003f)                     // saturate RCAL code if necessary
        rcalnewcodelp = 0x003f; 
    if (*rcalnewcode11_p > 0x003f)                  // saturate RCAL code if necessary
        *rcalnewcode11_p = 0x003f;

    // AFE_BIAS_CONFIG_0 
    brcm_misc_write(phy_dev, 0x39, 0x3, (rcalnewcodelp << 8) + 0xf8);   // REXT=1 BYP=1 RCAL_st1<5:0>=new rcal code
    brcm_misc_write(phy_dev, 0x38, 0x1, 0xe3e4);    // AFE_BIAS_CONFIG_0 10BT bias code, bias = 0xe4; 100BTbias: E3 same as EPHY
}

static void _phy_afe_cfg(phy_dev_t *phy_dev, u16 rcalnewcode11)
{
    u16 txcfgch0, txcfg2;

    // AFE_RXCONFIG_3
    brcm_misc_write(phy_dev, 0x3b, 0x00, 0x8002);   // invert adc clock output and 'adc refp ldo current To correct default
    // AFE_TX_CONFIG_1
    brcm_misc_write(phy_dev, 0x3c, 0x03, 0xf882);   // 100BT stair case, high BW, 1G stair case, alternate encode
    // AFE_TX_CONFIG_2
    brcm_misc_write(phy_dev, 0x3d, 0x00, 0x3201);   // 1000BT DAC transition method per Erol, bits[32], DAC Shuffle sequence 1 + 10BT imp adjust bits
    /// AFE_RXCONFIG_1
    brcm_misc_write(phy_dev, 0x3a, 0x02, 0x0c00);   // some non-overlap fix per Erol

    // RX Full scale calibration 
    // pwdb override (rxconfig<5>) to turn on RX LDO indpendent of pwdb controls from DSP */
    brcm_misc_write(phy_dev, 0x3a, 0x01, 0x0020);   // AFE_RXCONFIG_0 

    // Specify target offset for each channel 
    brcm_misc_write(phy_dev, 0x3b, 0x02, 0x0000);   // AFE_RXCONFIG_CH0
    brcm_misc_write(phy_dev, 0x3b, 0x03, 0x0000);   // AFE_RXCONFIG_CH1
    brcm_misc_write(phy_dev, 0x3c, 0x00, 0x0000);   // AFE_RXCONFIG_CH2
    brcm_misc_write(phy_dev, 0x3c, 0x01, 0x0000);   // AFE_RXCONFIG_CH3

    // Set cal_bypassb bit rxconfig<43> 
    brcm_misc_write(phy_dev, 0x003a, 0x03, 0x0800);   // AFE_RXCONFIG_2

    // At least 2us delay needed  before this line is executed.
    udelay(1000);

    // Revert pwdb_override (rxconfig<5>) to 0 so that the RX pwr is controlled by DSP.
    brcm_misc_write(phy_dev, 0x003a, 0x01, 0x0000);   // AFE_RXCONFIG_0

    // Adjust 10BT bias and RCAL settings 
    brcm_misc_read(phy_dev, 0x3d, 0x1, &txcfgch0);    // read AFE_TX_CONFIG_CH0
    // clear bits <11:5>, set txcfg_ch0<5>=1 (enable + set local rcal)
    txcfgch0 = (txcfgch0 & ~(0xfe0)) | 0x0020 | ((rcalnewcode11 &0xfffe) << 5);
    brcm_misc_write(phy_dev, 0x3d, 0x1, txcfgch0);    // write AFE_TX_CONFIG_CH0
    brcm_misc_write(phy_dev, 0x3d, 0x2, txcfgch0);    // write AFE_TX_CONFIG_CH1 for MDIX

    // AFE_TX_CONFIG_2
    brcm_misc_read(phy_dev, 0x3d, 0x0, &txcfg2);      // read AFE_TX_CONFIG_2
    //* set txcfg<45:44>=11 (enable Rextra + invert fullscaledetect) 
    txcfg2 = (txcfg2 & ~(0x3000)) | 0x3000;
    brcm_misc_write(phy_dev, 0x3d, 0x0, txcfg2);      // write AFE_TX_CONFIG_2

     // Note that the following Registers are not set by default on our GPHY DVT Boards (GPHY Standalone Mode)
     // AutoNeg Advert (10/100/1000BT) And Restart AutoNeg, which is required For the GPHY Standalone Boards
    phy_bus_write(phy_dev, 0x4, 0x1e1);
    phy_bus_write(phy_dev, 0x9, 0x300);
    phy_bus_write(phy_dev, MII_CONTROL, MII_CONTROL_AN_ENABLE|MII_CONTROL_RESTART_AUTONEG|MII_CONTROL_DUPLEX_MODE|MII_CONTROL_SPEED_SEL6);
}

#define _phy_afe_reset(phy_dev) { \
        brcm_exp_write(phy_dev, 0x03, 0x00006);     /* Reset AFE and PLL */ \
        udelay(300); \
        brcm_exp_write(phy_dev, 0x03, 0x00000); }   /* release Reset */

static void dsl_phy_afe_pll_setup(phy_dev_t *phy_dev)
{
    u16 rcalnewcode11;
    int i;
    static int qphy_afe_pll_done = 0;

    if (!IS_QPHY(phy_dev) && !IS_SPHY(phy_dev))
        return;
    if (IS_QPHY(phy_dev)) {
        if (qphy_afe_pll_done) 
            return;
    }
    
    // reset PHY
    if (IS_SPHY(phy_dev))
        phy_bus_write(phy_dev, MII_CONTROL, MII_CONTROL_RESET|MII_CONTROL_AN_ENABLE|MII_CONTROL_DUPLEX_MODE|MII_CONTROL_SPEED_SEL6);
    else
        for (i=0; i<4; i++)
            if (qphy_devs[i])
                phy_bus_write(qphy_devs[i], MII_CONTROL, MII_CONTROL_RESET|MII_CONTROL_AN_ENABLE|MII_CONTROL_DUPLEX_MODE|MII_CONTROL_SPEED_SEL6);
    // reset AFE and PLL
    if (IS_SPHY(phy_dev))
        _phy_afe_reset(phy_dev)
    else
        for (i=0; i<4; i++)
            if (qphy_devs[i])
                _phy_afe_reset(qphy_devs[i])

    i = (IS_QPHY(phy_dev) && (phy_dev->addr != QPHY0_ADDR)) ? phy_dev->addr : 0;
    if (i) phy_dev->addr = QPHY0_ADDR;         // need to do following  QPHY init using base addr
    
    //Write PLL/AFE control registers
    brcm_misc_write(phy_dev, 0x30, 1, 0x0000); // reset PLL
    brcm_misc_write(phy_dev, 0x31, 0, 0x0050); // ndiv_integer
    brcm_misc_write(phy_dev, 0x33, 2, 0x0001); // pdiv=0; auto-configured
    brcm_misc_write(phy_dev, 0x31, 1, 0x0000); // ndiv_fraction
    brcm_misc_write(phy_dev, 0x31, 2, 0x0000); // frequency nudge factor
    brcm_misc_write(phy_dev, 0x30, 3, 0x0032); // reference frequency, mode 0
    brcm_misc_write(phy_dev, 0x32, 3, 0x0000); // init bypass mode - default setting
    brcm_misc_write(phy_dev, 0x33, 0, 0x0002); // bypass code - default, vcoclk enabled
    brcm_misc_write(phy_dev, 0x30, 2, 0x01c0); // LDOs at default settings
    brcm_misc_write(phy_dev, 0x30, 1, 0x0001); // release PLL reset
    // AFE_BG_CONFIG
    brcm_misc_write(phy_dev, 0x38, 0, 0x0010); // Bandgap curvature correction to correct default -- Erol
    //RCAL and RCCAL
    _phy_run_cal(phy_dev, &rcalnewcode11);

    if (i) phy_dev->addr = i;                  // restore back to current addr

    if (IS_SPHY(phy_dev))
        _phy_afe_cfg(phy_dev, rcalnewcode11);
    else
        for (i=0; i<4; i++)
            if (qphy_devs[i])
                _phy_afe_cfg(qphy_devs[i], rcalnewcode11);

    if (!IS_SPHY(phy_dev))
        qphy_afe_pll_done = 1;
}

#else

static void dsl_phy_afe_pll_setup(phy_dev_t *phy_dev)
{
}

#endif


static int phy_init (phy_dev_t *phy_dev)
{
    /* 
        Reset External GPHY; 
    */
    if (phy_dev->phy_drv->phy_type == PHY_TYPE_DSL_GPHY)
    {
        if (PhyIsExtPhyId(phy_dev))
        {
            dsl_phy_reset(phy_dev);
        }
        else
        {
            dsl_phy_afe_pll_setup(phy_dev);
            _advertise_supported_caps(phy_dev);
            brcm_shadow_18_force_auto_mdix_set(phy_dev, 1);
        }
#if defined(CONFIG_BCM_JUMBO_FRAME)
        brcm_shadow_18_ext_pkt_len_set(phy_dev, 1);
#endif         
    }
    phy_dev->an_enabled = 1; /* We always use AN even for specific speed */

    if (phy_dev->mii_type == PHY_MII_TYPE_RGMII)
        brcm_shadow_rgmii_init(phy_dev);

    if (0 && phy_dev_cable_diag_is_supported(phy_dev))
        phy_dev_cable_diag_set(phy_dev, 1);

    phy_dev_cable_diag_init(phy_dev);

    return 0;
}

static int _phy_read_status(phy_dev_t *phy_dev)
{
    if (phy_dev->mii_type==PHY_MII_TYPE_RGMII)      // update RGMII_IB_STATUS
        return brcm_read_status_rgmii_ib_override(phy_dev);
    else
        return brcm_read_status(phy_dev);
}

#if defined(QPHY_CNTRL)
#if !defined(ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK)
#define ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK 0
#endif
static void qphy_ctrl_adjust(uint32_t ext_pwr_down)
{
    uint32_t phy_ctrl;

#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
    // 28nm GPHY power workaround
     /* To ensure minimum power consumption, the following steps should be
     * applied to the GPHY control signals. This should be done immediately
     * upon startup, before the decision is made to enable the GPHY or not,
     * in order to cover all cases.
     */
    /* Assert reset_n (active low) */
    phy_ctrl = (*QPHY_CNTRL) | ETHSW_QPHY_CTRL_RESET_MASK;
    *QPHY_CNTRL = phy_ctrl;
    udelay(100);
    
    /* Assert phy_test_cntrl */
    phy_ctrl = (*PHY_TEST_CNTRL) | ETHSW_PHY_TEST_CTRL_EN_MASK;
    *PHY_TEST_CNTRL = phy_ctrl;
    udelay(100);
    
    /* Deassert iddq_global_pwr and iddq_bias */
    phy_ctrl = (*QPHY_CNTRL) & ~(ETHSW_QPHY_CTRL_IDDQ_BIAS_MASK | ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK);
    *QPHY_CNTRL = phy_ctrl;
    udelay(100);

    /* Assert iddq_global_pwr and iddq_bias */
    phy_ctrl = (*QPHY_CNTRL) | (ETHSW_QPHY_CTRL_IDDQ_BIAS_MASK | ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK);
    *QPHY_CNTRL = phy_ctrl;
    udelay(100);

    /* Deassert reset_n */
    phy_ctrl = (*QPHY_CNTRL) & ~ETHSW_QPHY_CTRL_RESET_MASK;
    *QPHY_CNTRL = phy_ctrl;
    udelay(100);
    
    /* Deassert phy_test_nctrl */
    phy_ctrl = (*PHY_TEST_CNTRL) & ~ETHSW_PHY_TEST_CTRL_EN_MASK;
    *PHY_TEST_CNTRL = phy_ctrl;
    udelay(100);
    
#endif

    phy_ctrl = (*QPHY_CNTRL) & ~(ETHSW_QPHY_CTRL_PHYAD_BASE_MASK | ETHSW_QPHY_CTRL_RESET_MASK | ETHSW_QPHY_CTRL_IDDQ_BIAS_MASK |
            ETHSW_QPHY_CTRL_EXT_PWR_DOWN_MASK | ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK | ETHSW_QPHY_CTRL_CK25_DIS_MASK);
    phy_ctrl |= (QPHY0_ADDR << ETHSW_QPHY_CTRL_PHYAD_BASE_SHIFT);
#if defined(ETHSW_QPHY_CTRL_REF_CLK_50MHZ)
    phy_ctrl |= ETHSW_QPHY_CTRL_REF_CLK_50MHZ;
#endif 
    if (ext_pwr_down == 0xf)  // all QPHY ports down
    {
        phy_ctrl |= ETHSW_QPHY_CTRL_RESET_MASK | ETHSW_QPHY_CTRL_IDDQ_BIAS_MASK| ETHSW_QPHY_CTRL_EXT_PWR_DOWN_MASK |
                    ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK | ETHSW_QPHY_CTRL_CK25_DIS_MASK;
    }
    else
    {
        *QPHY_CNTRL = phy_ctrl | ETHSW_QPHY_CTRL_IDDQ_BIAS_MASK | ETHSW_QPHY_CTRL_EXT_PWR_DOWN_MASK |
                        ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK | ETHSW_QPHY_CTRL_RESET_MASK;
        udelay(40);
        *QPHY_CNTRL = phy_ctrl |  ETHSW_QPHY_CTRL_RESET_MASK;
        udelay(100);
        phy_ctrl |= ext_pwr_down << ETHSW_QPHY_CTRL_EXT_PWR_DOWN_SHIFT;
    }
    *QPHY_CNTRL = phy_ctrl;
    printk("Adjusted QGPHY: qphy_ctrl=0x%08x ext_pwr_down=0x%x\n", *QPHY_CNTRL, ext_pwr_down);
}
#endif //QPHY_CNTRL

#if !defined(ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK)
#define ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK 0
#endif

#if defined(SPHY_CNTRL)
static void sphy_ctrl_adjust(uint32_t ext_pwr_down)
{
    uint32_t phy_ctrl;

#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622) || defined(CONFIG_BCM96756)
    // 28nm GPHY power workaround
     /* To ensure minimum power consumption, the following steps should be
     * applied to the GPHY control signals. This should be done immediately
     * upon startup, before the decision is made to enable the GPHY or not,
     * in order to cover all cases.
     */
    /* Assert reset_n (active low) */
    phy_ctrl = (*SPHY_CNTRL) | ETHSW_SPHY_CTRL_RESET_MASK;
    *SPHY_CNTRL = phy_ctrl;
    udelay(100);
    
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
    /* Assert phy_test_cntrl */
    phy_ctrl = (*PHY_TEST_CNTRL) | ETHSW_PHY_TEST_CTRL_EN_MASK;
    *PHY_TEST_CNTRL = phy_ctrl;
    udelay(100);

#elif defined(CONFIG_BCM947622) || defined(CONFIG_BCM96756)
    /* Set GPHY's pll_clk125_250_sel to 250MHz */
    phy_ctrl = (*SPHY_CNTRL) | ETHSW_SPHY_CTRL_PLL_CLK_250_MASK;
    *SPHY_CNTRL = phy_ctrl;
    udelay(100);
#endif 
    
    /* Deassert iddq_global_pwr and iddq_bias */
    phy_ctrl = (*SPHY_CNTRL) & ~(ETHSW_SPHY_CTRL_IDDQ_BIAS_MASK | ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK);
    *SPHY_CNTRL = phy_ctrl;
    udelay(100);

    /* Assert iddq_global_pwr and iddq_bias */
    phy_ctrl = (*SPHY_CNTRL) | (ETHSW_SPHY_CTRL_IDDQ_BIAS_MASK | ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK);
    *SPHY_CNTRL = phy_ctrl;
    udelay(100);

    /* Deassert reset_n */
    phy_ctrl = (*SPHY_CNTRL) & ~ETHSW_SPHY_CTRL_RESET_MASK;
    *SPHY_CNTRL = phy_ctrl;
    udelay(100);

#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
    /* Deassert phy_test_nctrl */
    phy_ctrl = (*PHY_TEST_CNTRL) & ~ETHSW_PHY_TEST_CTRL_EN_MASK;
    *PHY_TEST_CNTRL = phy_ctrl;
    udelay(100);
#endif 
   
#endif

    phy_ctrl = (*SPHY_CNTRL) & ~(ETHSW_SPHY_CTRL_RESET_MASK | ETHSW_SPHY_CTRL_IDDQ_BIAS_MASK| ETHSW_SPHY_CTRL_EXT_PWR_DOWN_MASK | 
            ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK | ETHSW_SPHY_CTRL_CK25_DIS_MASK);
    phy_ctrl |= SPHY_ADDR << ETHSW_SPHY_CTRL_PHYAD_SHIFT;
#if defined(ETHSW_SPHY_CTRL_REF_CLK_50MHZ)
    phy_ctrl |= ETHSW_SPHY_CTRL_REF_CLK_50MHZ;
#endif
    if (ext_pwr_down)  // SPHY port down
    {
        *SPHY_CNTRL = phy_ctrl | ETHSW_SPHY_CTRL_RESET_MASK | ETHSW_SPHY_CTRL_IDDQ_BIAS_MASK| ETHSW_SPHY_CTRL_EXT_PWR_DOWN_MASK | 
            ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK | ETHSW_SPHY_CTRL_CK25_DIS_MASK;
    }
    else
    {
#if defined(CONFIG_BCM96756)
        // GPHY init power workaround
        *SPHY_CNTRL = phy_ctrl | ETHSW_SPHY_CTRL_EXT_PWR_DOWN_MASK;   // 0x00010X02
        udelay(100);
        *SPHY_CNTRL = phy_ctrl | ETHSW_SPHY_CTRL_EXT_PWR_DOWN_MASK | ETHSW_SPHY_CTRL_RESET_MASK;    // 0x00010X22
        udelay(100);
        *SPHY_CNTRL = phy_ctrl | ETHSW_SPHY_CTRL_EXT_PWR_DOWN_MASK;   // 0x00010X02
        udelay(100);
        *SPHY_CNTRL = phy_ctrl | ETHSW_SPHY_CTRL_EXT_PWR_DOWN_MASK | ETHSW_SPHY_CTRL_IDDQ_BIAS_MASK | ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK; // 0x00010X0b
        udelay(100);

        // GPHY init
        *SPHY_CNTRL = phy_ctrl | ETHSW_SPHY_CTRL_RESET_MASK;    // 0x00010X20
        udelay(100);
        *SPHY_CNTRL = phy_ctrl;                                 // 0x00010X00
        udelay(100);
#else
        *SPHY_CNTRL = phy_ctrl |  ETHSW_SPHY_CTRL_IDDQ_BIAS_MASK | ETHSW_SPHY_CTRL_EXT_PWR_DOWN_MASK | 
            ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK | ETHSW_SPHY_CTRL_RESET_MASK;
        udelay(40);
        *SPHY_CNTRL = phy_ctrl |  ETHSW_SPHY_CTRL_RESET_MASK;
        udelay(100);
        *SPHY_CNTRL = phy_ctrl;
#endif
    }
    printk("Adjusted SGPHY: sphy_ctrl=0x%08x ext_pwr_down=0x%x\n", *SPHY_CNTRL, ext_pwr_down);
}
#endif //SPHY_CNTRL


#if defined(QPHY_CNTRL) || defined(SPHY_CNTRL)
static uint32_t enabled_phys;
#endif

static int _phy_dev_add(phy_dev_t *phy_dev)
{
#if defined(QPHY_CNTRL) || defined(SPHY_CNTRL)
    unsigned short ba = gphy_base_addr;

    if (phy_dev->addr < ba)
        return 0;
    ba = phy_dev->addr - ba;
#if !defined(QPHY_CNTRL) && defined(SPHY_CNTRL)
    ba +=4;
#endif
    enabled_phys |= 1 << ba;
#if defined(QPHY_CNTRL)
    if (ba < 4) qphy_devs[ba] = phy_dev;
#endif
#if defined(SPHY_CNTRL)
    if (ba == 4) sphy_dev = phy_dev;
#endif
#endif
    return 0;
}

static int _phy_drv_init(phy_drv_t *phy_drv)
{
#if defined(QPHY_CNTRL)
    qphy_ctrl_adjust(~enabled_phys & 0x0f);
#endif
#if defined(SPHY_CNTRL)
    sphy_ctrl_adjust(~enabled_phys & 0x10);
#endif

    phy_drv->initialized = 1;
    return 0;
}

int dsl_phy_exp_op(phy_dev_t *phy_dev, int op, va_list ap)
{
    int reg = va_arg(ap, int);
    int val;
    uint16_t *valp;
    int ret = 0;

    switch(op)
    {
        case PHY_OP_RD_MDIO:
            valp = va_arg(ap, uint16_t *);
            ret = ethsw_phy_exp_read(phy_dev, reg, valp);
            break;
        case PHY_OP_WR_MDIO:
            val = va_arg(ap, int);
            ret = ethsw_phy_exp_write(phy_dev, reg, val);
            break;
    }

    return ret;
}

void phy_bus_probe(bus_drv_t *bus_drv)
{
    int i;
    uint16_t phyid1, phyid2;
    uint16_t pkgid1, pkgid2;
    phy_dev_t *phy_dev;
    static int probed;
    char *name;
    int no_name;
    bus_cfg_t bus_cfg;

    if (probed || !bus_drv || !bus_drv->c45_read)
        return;

    bus_cfg_get(bus_drv, &bus_cfg);
    bus_cfg.probe_mode = 1;
    bus_cfg_set(bus_drv, &bus_cfg);

    printknotice("Probing Copper PHYs located at bus:");
    for (i = 0; i < 32; i++)
    {
        phy_dev = phy_dev_get(PHY_TYPE_UNKNOWN, i);
        if (phy_dev && phy_dev->shared_ref_clk_mhz)
            phy_dev_shared_clock_set(phy_dev);

        if ((bus_drv->c45_read(i, 0x1, 0x0002, &phyid1)) || (bus_drv->c45_read(i, 0x1, 0x0003, &phyid2)))
        {
            if (phy_dev)
                printk("   PHY defined in DT at address %2d PHY Type: %d is not detected!!\n", 
                        i, phy_dev->phy_drv->phy_type);
            continue;
        }

        if (!phy_dev)
        {
                printk("    Detected Copper PHY at address %2d with PHY ID 0x%04x.0x%04x: Not Defined in DT.\n", 
                        i, phyid1, phyid2);
            continue;
        }

        bus_drv->c45_read(i, 0x1, 0x000e, &pkgid1);
		bus_drv->c45_read(i, 0x1, 0x000f, &pkgid2);

        printk("    Detected Copper PHY at address %2d with PHY ID 0x%04x.0x%04x: ", i, phyid1, phyid2);
		if (pkgid1 || pkgid2)
			pr_cont("Package ID: 0x%04x.0x%04x: ", pkgid1, pkgid2);
			
        no_name = *phy_dev->name == 0;
        name = phy_dev_get_phy_name(phy_dev);
        pr_cont("%s\n", name);
        if (no_name)
            phy_dev->name[0] = 0;   /* For PHYs firmware version requesting firmware loading after probing */

    }
    pr_cont("\n");

    bus_cfg_get(bus_drv, &bus_cfg);
    bus_cfg.probe_mode = 0;
    bus_cfg_set(bus_drv, &bus_cfg);
    probed = 1;
}

phy_drv_t phy_drv_dsl_gphy =
{
    .phy_type = PHY_TYPE_DSL_GPHY,
    .name = "EGPHY",

    .power_get = mii_power_get,
    .power_set = mii_power_set,
    .apd_get = brcm_egphy_apd_get,
    .apd_set = brcm_egphy_apd_set,
    .eee_set = brcm_egphy_eee_set,
    .eee_get = brcm_egphy_eee_get,
    .eee_resolution_get = brcm_egphy_eee_resolution_get,
    .wirespeed_set = brcm_egphy_eth_wirespeed_set,
    .wirespeed_get = brcm_egphy_eth_wirespeed_get,
    .read_status = _phy_read_status,
    .speed_set = mii_speed_set,
    .caps_get = mii_caps_get,
    .caps_set = mii_caps_set,
    .phyid_get = mii_phyid_get,
    .isolate_phy = mii_isolate_phy,

    .read = brcm_egphy_read,
    .write = brcm_egphy_write,

    .init = phy_init,

    .dev_add = _phy_dev_add,
    .drv_init = _phy_drv_init,
    .loopback_set = brcm_loopback_set,
    .loopback_get = brcm_loopback_get,
    .apd_get = brcm_egphy_apd_get,
    .apd_set = brcm_egphy_apd_set,
    .cable_diag_run = brcm_cable_diag_run,
    .leds_init = dsl_phy_leds_init,	
    .priv_fun = dsl_phy_exp_op,
};


static int mac2mac_phy_init(phy_dev_t *phy_dev)
{
    // setup default speed, duplex and link state
    phy_dev->link = 0;
    phy_dev->speed = PHY_SPEED_1000;
    phy_dev->duplex = PHY_DUPLEX_FULL;
    return 0;
}

static int phy_drv_mac2mac_power_get(phy_dev_t *phy_dev, int *enable)
{
    *enable = 1;
    return 1;
}

phy_drv_t phy_drv_mac2mac =
{
    .phy_type = PHY_TYPE_MAC2MAC,
    .name = "MAC2MAC",
    .init = mac2mac_phy_init,
    .power_get = phy_drv_mac2mac_power_get,
};

