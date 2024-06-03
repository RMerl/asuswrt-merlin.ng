/*
<:copyright-BRCM:2017:GPL/GPL:standard

         Copyright (c) 2017 Broadcom
         All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published 
by the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, 
or by writing to the Free Software Foundation, Inc., 59 Temple Place - 
Suite 330, Boston, MA 02111-1307, USA.

:>
*/


#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/of_platform.h>
#include "bcm_ubus4.h"
#include "bcm_ubus_internal.h"

// magically creates /sys/module/ubus4_dcm/parameter directory
#ifdef MODULE_PARAM_PREFIX
#undef MODULE_PARAM_PREFIX
#endif
#define MODULE_PARAM_PREFIX "ubus4_dcm."

// local defines
#define	UBUS_DCM_ON			1
#define	UBUS_DCM_OFF		0
#define	DCM_DIVIDER_DEFAULT	3					// 1/8 of UBUS_CLK
#define DCM_THOLD_TO_HIGH_FREQ_DEFAULT	1		// 1  UBUS req
#define DCM_THOLD_TO_LOW_FREQ_DEFAULT	0x400	// 1K UBUS_CLK cycles

ubus_dcm_t *ubus_dcm_sys = NULL;
ubus_dcm_t *ubus_dcm_xrdp = NULL;

int dcm_enable = UBUS_DCM_ON;

#define DEBUG
//===========================================
// Generic functions
//===========================================

static u32 dcm_enable_get(ubus_dcm_t *ubus_dcm)
{
    volatile Ubus4ClkCtrlCfgRegs *ubus4_clk;

    if (ubus_dcm && ubus_dcm->clk) {
        ubus4_clk = (volatile Ubus4ClkCtrlCfgRegs *)ubus_dcm->clk;
        return ((ubus4_clk->ClockCtrl & UBUS4_CLK_BYPASS_MASK) ? 0 : 1);
    } else
        return 0;
}

static void dcm_enable_set(ubus_dcm_t *ubus_dcm, u32 enable)
{
    u32 d32;
    u32 m32;
    volatile Ubus4ClkCtrlCfgRegs *ubus4_clk;

    if (!ubus_dcm)
        return;
    ubus4_clk = (volatile Ubus4ClkCtrlCfgRegs *)ubus_dcm->clk;
    if (!ubus4_clk)
        return;

    m32 = UBUS4_CLK_BYPASS_MASK|UBUS4_CLK_CTRL_EN_MASK;

    if (enable==0) {	// disable : CLK_BYPASS=1, CLK_CTRL_EN=1
        d32 = (1<<UBUS4_CLK_BYPASS_SHIFT) | (1<<UBUS4_CLK_CTRL_EN_SHIFT); 
        ubus4_clk->ClockCtrl     = (ubus4_clk->ClockCtrl 	   & ~m32) | d32;
    } else {		//enable -> CLK_BYPASS=0, CLK_CTRL_EN=1
        d32 = 1<<UBUS4_CLK_CTRL_EN_SHIFT; 
        ubus4_clk->ClockCtrl 	= (ubus4_clk->ClockCtrl     & ~m32) | d32;
    }
}

static void dcm_divider_set(ubus_dcm_t *ubus_dcm, uint32_t val)
{
    uint32_t d32=0, m32=0;
    volatile Ubus4ClkCtrlCfgRegs *ubus4_clk;

    if (!ubus_dcm)
        return;
    ubus4_clk = (volatile Ubus4ClkCtrlCfgRegs *)ubus_dcm->clk;
    if (!ubus4_clk)
        return;

    m32 = (~(UBUS4_MID_CLK_SEL_MASK|UBUS4_MIN_CLK_SEL_MASK));
    d32 = (val<<UBUS4_MIN_CLK_SEL_SHIFT)|(val<<UBUS4_MID_CLK_SEL_SHIFT);
    ubus4_clk->ClockCtrl = (ubus4_clk->ClockCtrl     & m32) | d32;
}

static u32 dcm_divider_get(ubus_dcm_t *ubus_dcm)
{
    volatile Ubus4ClkCtrlCfgRegs *ubus4_clk;

    if (!ubus_dcm)
        return 0;
    ubus4_clk = (volatile Ubus4ClkCtrlCfgRegs *)ubus_dcm->clk;
    if (!ubus4_clk)
        return 0;

    return ((ubus4_clk->ClockCtrl & UBUS4_MIN_CLK_SEL_MASK) >> 
        UBUS4_MIN_CLK_SEL_SHIFT);
}

// Controls how soon the clock scaling changed
// For each UBUS4 segment
static void dcm_thold_set(ubus_dcm_t *ubus_dcm, uint32_t val1, uint32_t val2)
{
    volatile Ubus4ClkCtrlCfgRegs *ubus4_clk;

    if (!ubus_dcm)
        return;
    ubus4_clk = (volatile Ubus4ClkCtrlCfgRegs *)ubus_dcm->clk;
    if (!ubus4_clk)
        return;

    // switch to high freq
    ubus4_clk    ->Min2Mid_threshhold = val1;
    ubus4_clk    ->Mid2Max_threshhold = val1;
    // switch to low freq
    ubus4_clk    ->Mid2Min_threshhold = val2;
    ubus4_clk    ->Max2Mid_threshhold = val2;
}

static u32 dcm_thold_get(ubus_dcm_t *ubus_dcm)
{
    volatile Ubus4ClkCtrlCfgRegs *ubus4_clk;

    if (!ubus_dcm)
        return 0;
    ubus4_clk = (volatile Ubus4ClkCtrlCfgRegs *)ubus_dcm->clk;
    if (!ubus4_clk)
        return 0;

    return (ubus4_clk->Mid2Min_threshhold);
}

// set default values
static void dcm_config_set(ubus_dcm_t *ubus_dcm)
{
    u32 i=0;
    uint64_t  mst_registered;
    volatile Ubus4ModuleClientRegistration *ubus4_client_registration;
    volatile Ubus4ClkCtrlCfgRegs *ubus4_clk = ubus_dcm->clk;

    if (ubus_dcm->xrdp_module)
        ubus4_client_registration =
            ubus_xrdp ? (volatile Ubus4ModuleClientRegistration *)ubus_xrdp->registration : NULL;
    else
        ubus4_client_registration =
            ubus_sys ? (volatile Ubus4ModuleClientRegistration *)ubus_sys->registration : NULL;
    if (!ubus4_client_registration) {
        printk("dcm_config_set: ubus port registration reg is not setup!\n");
        return;
    }

    // UBUS Congestion Threshold regs.
    // Only writes to those mst nodes that are registered
    // Set to '0' to response to the 1st UBUS request

    // ubus system
    mst_registered = *((uint64_t*)ubus4_client_registration->MstStatus);
    while (ub_mst_addr_map_tbl[i].port_id != -1) {
        if (mst_registered & (1UL<<ub_mst_addr_map_tbl[i].port_id))
            ubus_cong_threshold_wr(ub_mst_addr_map_tbl[i].port_id, 0);
        i++;
    }

    /* Runner driver (data_path_init.c) set CONGEST_THRESHOLD to 0x2 in this register. 
       don't need to do it here in ubus driver */ 
#if 0
    if (ubus_dcm->rcq_gen) {
        u32 d32 = 0;
#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856)
        d32 = (1<<XRDP_RCQ_GENERAL_DDR_CONGEST_THRESHOLD_SHIFT) |
              (1<<XRDP_RCQ_GENERAL_PSRAM_CONGEST_THRESHOLD_SHIFT);
#elif defined(CONFIG_BCM963158) || defined(CONFIG_BCM96858)
        // Due to RTL design issue, it is required to set this to 1 instead of 0
        d32 = 1<<XRDP_RCQ_GENERAL_DMA_ARB_CFG_CONGEST_THRESHOLD_SHIFT;
#endif
        if (d32)
            *((volatile uint32_t*)ubus_dcm->rcq_gen) = d32;
    }
#endif
    // always use DCM clock
    ubus4_clk->ClockCtrl &= ~UBUS4_CLK_BYPASS_MASK;  // always use DCM clock
    dcm_divider_set(ubus_dcm, DCM_DIVIDER_DEFAULT);
    dcm_thold_set(ubus_dcm, DCM_THOLD_TO_HIGH_FREQ_DEFAULT,
        DCM_THOLD_TO_LOW_FREQ_DEFAULT);
}

#ifdef  DEBUG
static void dcm_config_get(void)
{
    volatile Ubus4ClkCtrlCfgRegs *ubus4_clk_sys = NULL, *ubus4_clk_xrdp = NULL;
    int i = 0;
    volatile uint32_t *rcq = NULL;

    if (ubus_dcm_sys) {
        if ( ubus_dcm_sys->clk)
            ubus4_clk_sys = (volatile Ubus4ClkCtrlCfgRegs *)ubus_dcm_sys->clk;
        if (ubus_dcm_sys->rcq_gen)
            rcq = (volatile uint32_t *)ubus_dcm_sys->rcq_gen;
    }

    if (ubus_dcm_xrdp) {
        if (ubus_dcm_xrdp->clk)
           ubus4_clk_xrdp = (volatile Ubus4ClkCtrlCfgRegs *)ubus_dcm_xrdp->clk;
        if (ubus_dcm_xrdp->rcq_gen)
            rcq = (volatile uint32_t *)ubus_dcm_xrdp->rcq_gen;
    }
#define DCM_THOLD(base) \
    ((MstPortNode*) base )->port_cfg[DCM_UBUS_CONGESTION_THRESHOLD]

    printk("UBUS Congestion Threshold Register value for:\n");
    if (ubus_sys && ubus_sys->systop)
        printk("  SYS_BASE\t= 0x%08x\n",DCM_THOLD(ubus_sys->systop));
    if (ubus_xrdp && ubus_xrdp->systop)
        printk("  SYS_XRDP_BASE\t= 0x%08x\n",DCM_THOLD(ubus_xrdp->systop));

     while (ub_mst_addr_map_tbl[i].port_id != -1) {
         printk("  %s\t= 0x%08x\n", ub_mst_addr_map_tbl[i].str,
            DCM_THOLD(ub_mst_addr_map_tbl[i].base));
        i++;
    }
    printk("\n");
    if (rcq)
    printk("%s = 0x%08x\n", 
        "RCQ_GENERAL_CONFIG_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_DMA_ARB_CFG", 
        (uint32_t) *(rcq));
    printk("\n");
    if (ubus4_clk_sys)
        printk("UBUS4CLK    ->ClockCtrl = 0x%08x\n", ubus4_clk_sys->ClockCtrl);
    if (ubus4_clk_xrdp)
        printk("UBUS4XRDPCLK->ClockCtrl = 0x%08x\n", ubus4_clk_xrdp->ClockCtrl);
    printk("\n");
    if (ubus4_clk_sys) {
        printk("UBUS4CLK    ->Min2Mid_threshhold = 0x%04x\n", 
            ubus4_clk_sys    ->Min2Mid_threshhold);
        printk("UBUS4CLK    ->Mid2Max_threshhold = 0x%04x\n", 
            ubus4_clk_sys    ->Mid2Max_threshhold);
    }
    if (ubus4_clk_xrdp) {
        printk("UBUS4XRDPCLK->Min2Mid_threshhold = 0x%04x\n", 
            ubus4_clk_xrdp->Min2Mid_threshhold);
        printk("UBUS4XRDPCLK->Mid2Max_threshhold = 0x%04x\n", 
            ubus4_clk_xrdp->Mid2Max_threshhold);
    }
    printk("\n");
    if (ubus4_clk_sys) {
        printk("UBUS4CLK    ->Mid2Min_threshhold = 0x%04x\n", 
            ubus4_clk_sys    ->Mid2Min_threshhold);
        printk("UBUS4CLK    ->Max2Mid_threshhold = 0x%04x\n", 
            ubus4_clk_sys    ->Max2Mid_threshhold);
    }
    if (ubus4_clk_xrdp) {
        printk("UBUS4XRDPCLK->Mid2Min_threshhold = 0x%04x\n", 
            ubus4_clk_xrdp->Mid2Min_threshhold);
        printk("UBUS4XRDPCLK->Max2Mid_threshhold = 0x%04x\n", 
            ubus4_clk_xrdp->Max2Mid_threshhold);
    }
}
#endif // DEBUG

//===========================================
// function declaration in /sys/module/ubus4_dcm/parameter/* 
//===========================================

static int ubus4_dcm_enable_get (char *buffer, const struct kernel_param *kp)
{
    return sprintf(buffer, "%d\t : valid value { 0, 1}", dcm_enable_get(ubus_dcm_sys));
}

static int ubus4_dcm_enable_set(const char *val, const struct kernel_param *kp)
{
    if (!strncmp(val, "0", 1)) {
        dcm_enable = UBUS_DCM_OFF;
    }  else if (!strncmp(val, "1", 1)) {
        dcm_enable = UBUS_DCM_ON;
    } else {
        return -EINVAL;
    }

    dcm_enable_set (ubus_dcm_sys, dcm_enable);
    dcm_enable_set (ubus_dcm_xrdp, dcm_enable);

    return 0;
}

static int ubus4_dcm_divider_get (char *buffer, const struct kernel_param *kp)
{
    return sprintf(buffer, 
        "%d\t : valid value { 1, 2, 3, 4, 5, 6, 7}", 
        dcm_divider_get(ubus_dcm_sys));
}

static int ubus4_dcm_divider_set(const char *val, const struct kernel_param *kp)
{
    u32 ival;

    if(kstrtouint (val, 0, &ival))
        printk("%s:%d failed to scan ival\n", __func__, __LINE__);

    if (ival>=1 && ival <=7) {
        dcm_divider_set(ubus_dcm_sys, ival);
        dcm_divider_set(ubus_dcm_xrdp, ival);
    } else {
        return -EINVAL; // invalid input
    }
    return 0;
}

static int ubus4_dcm_thold_get (char *buffer, const struct kernel_param *kp)
{
    return sprintf(buffer, 
        "0x%04x\t : valid value {1..0xFFFF}",dcm_thold_get(ubus_dcm_sys));
}

static int ubus4_dcm_thold_set(const char *val, const struct kernel_param *kp)
{
    u32 ival;

    if(kstrtouint (val, 0, &ival))
        printk("%s:%d failed to scan ival\n", __func__, __LINE__);

    if ((ival&0xFFFF) && ((ival&0xFFFF) <= 0xFFFF)) {
        dcm_thold_set (ubus_dcm_sys, 1, ival);
        dcm_thold_set (ubus_dcm_xrdp, 1, ival);
    } else {
        return -EINVAL;	// invalid input
    }
    return 0;
}

#ifdef DEBUG
static int ubus4_dcm_config_get (char *buffer, const struct kernel_param *kp)
{
    dcm_config_get(); // for debug purpose
    return sprintf(buffer, "valid value { any, don't care }");
}
#endif

static int ubus4_dcm_config_set(const char *val, const struct kernel_param *kp)
{
    dcm_config_set(ubus_dcm_sys);
    dcm_config_set(ubus_dcm_xrdp);
    return 0;
}

// registering parameters in /sys/module/ubus4_dcm/parameter directory
#define DCM_MPC(a,b,c,d,e) module_param_call(a,b,c,d,e)
DCM_MPC(enable,    ubus4_dcm_enable_set,   ubus4_dcm_enable_get,   NULL, 0644);
DCM_MPC(divider,   ubus4_dcm_divider_set,  ubus4_dcm_divider_get,  NULL, 0644);
DCM_MPC(threshold, ubus4_dcm_thold_set,    ubus4_dcm_thold_get,    NULL, 0644);
#ifdef DEBUG
DCM_MPC(config,  ubus4_dcm_config_set, ubus4_dcm_config_get, NULL, 0644);
#else
DCM_MPC(config,  ubus4_dcm_config_set, NULL, NULL, 0644);
#endif


// Note: IMPORTANT
// other kernel module codes come after may change default setup.
// it is recommanded to re-enable this feature when everything is up.

// called when kernal boot options is specified : ubus4_dcm=disable/enable
static int __init ubus_dcm_init_param(char *str)
{
    if (!strcmp(str, "disable")) {
        dcm_enable = UBUS_DCM_OFF;
    } else {
        dcm_enable = UBUS_DCM_ON;
    }

    printk("ubus_dcm_init_param enable=%d\n", dcm_enable);
    return 1;
}
__setup("ubus4_dcm=",ubus_dcm_init_param);

static void bcu_ubus_dcm_free(ubus_dcm_t *ubus_dcm)
{
    if (ubus_dcm) {
        if (!IS_ERR_OR_NULL(ubus_dcm->clk))
            iounmap(ubus_dcm->clk);
        if (!IS_ERR_OR_NULL(ubus_dcm->rcq_gen))
            iounmap(ubus_dcm->rcq_gen);
        kfree(ubus_dcm);
    }
}

static int bcm_ubus_dcm_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct device_node *np = dev->of_node;
    struct resource *res;
    uint32_t val = 0, ret = 0;
    ubus_dcm_t *ubus_dcm;

    ubus_dcm = kzalloc(sizeof(*ubus_dcm), GFP_KERNEL);
    if (!ubus_dcm) {
        printk("UBUS DCM failed to allocate memory \n");
        return -ENOMEM;
    }

	dev_info(dev, "Broadcom BCA UBUS DCM Driver\n");
	
    if (of_property_read_u32(np, "flags", &val) == 0 &&
        (val&UBUS_FLAGS_MODULE_XRDP)) {
        ubus_dcm_xrdp = ubus_dcm;
        ubus_dcm->xrdp_module = 1;
    } else {
        ubus_dcm_sys = ubus_dcm;
        ubus_dcm->xrdp_module = 0;
    }

    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "ubus_dcm_clk");
    if (!res) {
        dev_err(dev, "Failed to find ubus_dcm_clk resource\n");
        ret = -EINVAL;
        goto error;
    }

    ubus_dcm->clk = devm_ioremap_resource(dev, res);
    if (IS_ERR(ubus_dcm->clk)) {
        dev_err(dev, "Failed to map the ubus_dcm_clk resource\n");
        ret = -ENXIO;
        ubus_dcm->clk = NULL;
        goto error;
    }

	dev_info(dev, "ubus dcm clk resource %pr\n", res);
	dev_info(dev, "map to virt addr 0x%px\n", ubus_dcm->clk);
	
    /* optional resource for xrdp rcq general register */
    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "xrdp_rdp_gen");
    if (res) {
        ubus_dcm->rcq_gen = devm_ioremap_resource(dev, res);
        if (IS_ERR(ubus_dcm->rcq_gen)) {
            dev_err(dev, "Failed to map the xrdp_rdp_gen resource\n");
            ret = -ENXIO;
            ubus_dcm->rcq_gen = NULL;
            goto error;
        }
		dev_info(dev, "xrdp rcq resource %pr\n", res);
		dev_info(dev, "map to virt addr 0x%px\n", ubus_dcm->rcq_gen);
    } else
        ubus_dcm->rcq_gen = NULL;

    dcm_config_set(ubus_dcm);
    dcm_enable_set(ubus_dcm, dcm_enable);

    return ret;
error:
    bcu_ubus_dcm_free(ubus_dcm);
    return ret;
}

static const struct of_device_id bcm_ubus_dcm_of_match[] = {
    { .compatible = "brcm,bca-ubus4-dcm"},
    {},
};
MODULE_DEVICE_TABLE(of, bcm_ubus_dcm_of_match);


static struct platform_driver bcm_ubus_dcm_driver = {
    .probe = bcm_ubus_dcm_probe,
    .driver = {
        .name = "bcm-ubus-dcm",
        .of_match_table = bcm_ubus_dcm_of_match,
    },
};

static int __init bcm_ubus_dcm_module_init(void)
{
    return platform_driver_register(&bcm_ubus_dcm_driver);
}

static void __exit bcm_ubus_dcm_module_exit(void)
{
      bcu_ubus_dcm_free(ubus_dcm_sys);
    bcu_ubus_dcm_free(ubus_dcm_xrdp);
    platform_driver_unregister(&bcm_ubus_dcm_driver);
    return;
}

module_init(bcm_ubus_dcm_module_init);
module_exit(bcm_ubus_dcm_module_exit);

MODULE_DESCRIPTION("Broadcom BCA UBUS DCM Driver");
MODULE_LICENSE("GPL v2");
