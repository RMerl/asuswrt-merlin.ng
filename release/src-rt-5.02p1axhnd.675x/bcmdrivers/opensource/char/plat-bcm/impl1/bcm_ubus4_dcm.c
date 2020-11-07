
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
#include <bcm_map_part.h>
#include <bcm_ubus4.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/errno.h>

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

//===========================================
// UBUS4 master registeration
//===========================================
extern ub_mst_addr_map_t ub_mst_addr_map_tbl[];

//===========================================
// Generic functions
//===========================================

static u32 dcm_enable_get(void)
{
    return ((UBUS4CLK->ClockCtrl & UBUS4_CLK_BYPASS_MASK) ? 0 : 1);
}

static void dcm_enable_set(u32 enable)
{
    u32 d32;
    u32 m32;

    m32 = UBUS4_CLK_BYPASS_MASK|UBUS4_CLK_CTRL_EN_MASK;

    if (enable==0) {	// disable : CLK_BYPASS=1, CLK_CTRL_EN=1
		d32 = (1<<UBUS4_CLK_BYPASS_SHIFT) | (1<<UBUS4_CLK_CTRL_EN_SHIFT); 
        UBUS4CLK->ClockCtrl     = (UBUS4CLK->ClockCtrl 	   & ~m32) | d32;
#if defined(UBUSSYSXRDP_REGISTRATION)
        UBUS4XRDPCLK->ClockCtrl = (UBUS4XRDPCLK->ClockCtrl & ~m32) | d32;
#endif
    } else {		//enable -> CLK_BYPASS=0, CLK_CTRL_EN=1
		d32 = 1<<UBUS4_CLK_CTRL_EN_SHIFT; 
        UBUS4CLK->ClockCtrl 	= (UBUS4CLK->ClockCtrl     & ~m32) | d32;
#if defined(UBUSSYSXRDP_REGISTRATION)
        UBUS4XRDPCLK->ClockCtrl = (UBUS4XRDPCLK->ClockCtrl & ~m32) | d32;
#endif
    }
}

static void dcm_divider_set(uint32 val)
{
    uint32_t d32=0, m32=0;

    m32 = (~(UBUS4_MID_CLK_SEL_MASK|UBUS4_MIN_CLK_SEL_MASK));
    d32 = (val<<UBUS4_MIN_CLK_SEL_SHIFT)|(val<<UBUS4_MID_CLK_SEL_SHIFT);
    UBUS4CLK->ClockCtrl		= (UBUS4CLK->ClockCtrl     & m32) | d32;
#if defined(UBUSSYSXRDP_REGISTRATION)
    UBUS4XRDPCLK->ClockCtrl = (UBUS4XRDPCLK->ClockCtrl & m32) | d32;
#endif
}

static u32 dcm_divider_get(void)
{
	return ((UBUS4CLK->ClockCtrl & UBUS4_MIN_CLK_SEL_MASK) >> 
			UBUS4_MIN_CLK_SEL_SHIFT);
}

// Controls how soon the clock scaling changed
// For each UBUS4 segment
static void dcm_thold_set(uint32 val1, uint32 val2)
{
	// switch to high freq
	UBUS4CLK    ->Min2Mid_threshhold = val1;
	UBUS4CLK    ->Mid2Max_threshhold = val1;
    // switch to low freq
    UBUS4CLK    ->Mid2Min_threshhold = val2;
    UBUS4CLK    ->Max2Mid_threshhold = val2;
#if defined(UBUSSYSXRDP_REGISTRATION)
    UBUS4XRDPCLK->Min2Mid_threshhold = val1;
    UBUS4XRDPCLK->Mid2Max_threshhold = val1;
    UBUS4XRDPCLK->Mid2Min_threshhold = val2;
    UBUS4XRDPCLK->Max2Mid_threshhold = val2;
#endif
}

static u32 dcm_thold_get(void)
{
    return (UBUS4CLK->Mid2Min_threshhold);
}

// set default values
static void dcm_config_set(void)
{
    u32 i=0;
    uint64_t  mst_registered;

    volatile Ubus4ModuleClientRegistration * ubus4_sys_client_registration = UBUSSYSTOP_REGISTRATION;
#if defined(UBUSSYSXRDP_REGISTRATION)
    volatile Ubus4ModuleClientRegistration * ubus4_xrdp_client_registration = UBUSSYSXRDP_REGISTRATION;
#endif

    // UBUS Congestion Threshold regs. 
    // Only writes to those mst nodes that are registered
    // Set to '0' to response to the 1st UBUS request

    // ubus system
    mst_registered = *((uint64_t*)ubus4_sys_client_registration->MstStatus);
    while (ub_mst_addr_map_tbl[i].port_id != -1)
    {
        if (mst_registered & (1UL<<ub_mst_addr_map_tbl[i].port_id))
            ubus_cong_threshold_wr(ub_mst_addr_map_tbl[i].port_id, 0);
        i++;
    }
    
#if defined(UBUSSYSXRDP_REGISTRATION)
    // ubus XRDP
    mst_registered = *((uint64_t*)ubus4_xrdp_client_registration->MstStatus);
    while (ub_mst_addr_map_tbl[i].port_id != -1)
    {
        if (mst_registered & (1UL<<ub_mst_addr_map_tbl[i].port_id))
            ubus_cong_threshold_wr(ub_mst_addr_map_tbl[i].port_id, 0);
        i++;
    }

    {
        u32 d32;
#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856)
        d32 = (1<<XRDP_RCQ_GENERAL_DDR_CONGEST_THRESHOLD_SHIFT) |
              (1<<XRDP_RCQ_GENERAL_PSRAM_CONGEST_THRESHOLD_SHIFT);
#else
        // Due to RTL design issue, it is required to set this to 1 instead of 0
        d32 = 1<<XRDP_RCQ_GENERAL_DMA_ARB_CFG_CONGEST_THRESHOLD_SHIFT;
#endif
        *((volatile uint32_t*) XRDP_RCQ_GEN_CFG) = d32;
    }
#endif
    
    // always use DCM clock
    UBUS4CLK->ClockCtrl 	&= ~UBUS4_CLK_BYPASS_MASK;  // always use DCM clock
#if defined(UBUSSYSXRDP_REGISTRATION)
    UBUS4XRDPCLK->ClockCtrl &= ~UBUS4_CLK_BYPASS_MASK;  // always use DCM clock
#endif
    dcm_divider_set(DCM_DIVIDER_DEFAULT);
    dcm_thold_set(DCM_THOLD_TO_HIGH_FREQ_DEFAULT, 
				  DCM_THOLD_TO_LOW_FREQ_DEFAULT);
        
}

#ifdef  DEBUG		
static void dcm_config_get(void)	
{

#define DCM_THOLD(base) \
    ((MstPortNode*) base )->port_cfg[DCM_UBUS_CONGESTION_THRESHOLD]

    printk("UBUS Congestion Threshold Register value for:\n");
    printk("  SYS_BASE       = 0x%08x\n",DCM_THOLD(UBUS_SYS_MODULE_BASE      ));
#if defined(UBUSSYSXRDP_REGISTRATION)
    printk("  SYS_XRDP_BASE= 0x%08x\n",DCM_THOLD(UBUS_SYS_MODULE_XRDP_BASE ));
#endif
    printk("  B53_BASE       = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_B53_BASE    ));
    printk("  PER_BASE       = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_PER_BASE    ));
    printk("  USB_BASE       = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_USB_BASE    ));
    printk("  PCIE0_BASE     = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_PCIE0_BASE  ));
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM96858)
    printk("  PMC_BASE       = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_PMC_BASE    ));
    printk("  SPU_BASE       = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_SPU_BASE    ));
    printk("  PCIE2_BASE     = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_PCIE2_BASE  ));
    printk("  PER_DMA_BASE   = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_PER_DMA_BASE));
#endif
#if defined(CONFIG_BCM963158)
    printk("  DSL_BASE       = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_DSL_BASE    ));
    printk("  PCIE3_BASE     = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_PCIE3_BASE  ));
    printk("  DSLCPU_BASE    = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_DSLCPU_BASE ));
#endif
    printk("  \n");
    printk("  QM_BASE        = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_QM_BASE     ));
    printk("  DQM_BASE       = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_DQM_BASE    ));
    printk("  DMA0_BASE      = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_DMA0_BASE   ));
    printk("  NATC_BASE      = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_NATC_BASE   ));
#if defined(CONFIG_BCM96846)
    printk("  RQ0_BASE       = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_RQ0_BASE    ));
#endif
#if defined(CONFIG_BCM96856)
    printk("  PCIE2_BASE     = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_PCIE2_BASE  ));
    printk("  DMA1_BASE      = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_DMA1_BASE   ));
    printk("  RQ0_BASE       = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_RQ0_BASE    ));
    printk("  RQ1_BASE       = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_RQ1_BASE    ));
#endif
#if defined(CONFIG_BCM96858)
    printk("  DMA1_BASE      = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_DMA1_BASE   ));
    printk("  TOP_BUFF_BASE  = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_TOP_BUFFER_BASE   ));
    printk("  XRDP_BUFF_BASE = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_XRDP_BUFFER_BASE   ));
    printk("  RQ0_BASE       = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_RQ0_BASE    ));
    printk("  RQ1_BASE       = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_RQ1_BASE    ));
    printk("  RQ2_BASE       = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_RQ2_BASE    ));
    printk("  RQ3_BASE       = 0x%08x\n",DCM_THOLD(MST_PORT_NODE_RQ3_BASE    ));
#endif
    printk("\n");
	printk("%s = 0x%08x\n", 
    	"RCQ_GENERAL_CONFIG_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_DMA_ARB_CFG", 
        (uint32_t) *((uint64_t*) XRDP_RCQ_GEN_CFG));
    printk("\n");
    printk("UBUS4CLK    ->ClockCtrl = 0x%08x\n", UBUS4CLK    ->ClockCtrl);
    printk("UBUS4XRDPCLK->ClockCtrl = 0x%08x\n", UBUS4XRDPCLK->ClockCtrl);
    printk("\n");
    printk("UBUS4CLK    ->Min2Mid_threshhold = 0x%04x\n", 
			UBUS4CLK    ->Min2Mid_threshhold);
    printk("UBUS4CLK    ->Mid2Max_threshhold = 0x%04x\n", 
			UBUS4CLK    ->Mid2Max_threshhold);
    printk("UBUS4XRDPCLK->Min2Mid_threshhold = 0x%04x\n", 
			UBUS4XRDPCLK->Min2Mid_threshhold);
    printk("UBUS4XRDPCLK->Mid2Max_threshhold = 0x%04x\n", 
			UBUS4XRDPCLK->Mid2Max_threshhold);
    printk("\n");
    printk("UBUS4CLK    ->Mid2Min_threshhold = 0x%04x\n", 
			UBUS4CLK    ->Mid2Min_threshhold);
    printk("UBUS4CLK    ->Max2Mid_threshhold = 0x%04x\n", 
			UBUS4CLK    ->Max2Mid_threshhold);
    printk("UBUS4XRDPCLK->Mid2Min_threshhold = 0x%04x\n", 
			UBUS4XRDPCLK->Mid2Min_threshhold);
    printk("UBUS4XRDPCLK->Max2Mid_threshhold = 0x%04x\n", 
			UBUS4XRDPCLK->Max2Mid_threshhold);
}
#endif // DEBUG

//===========================================
// function declaration in /sys/module/ubus4_dcm/parameter/* 
//===========================================

static int ubus4_dcm_enable_get (char *buffer, struct kernel_param *kp)
{
    return sprintf(buffer, "%d\t : valid value { 0, 1}", dcm_enable_get());
}

static int ubus4_dcm_enable_set(const char *val, struct kernel_param *kp)
{
    if (!strncmp(val, "0", 1)) {
		dcm_enable_set (UBUS_DCM_OFF);	
    }  else if (!strncmp(val, "1", 1)) {
		dcm_enable_set (UBUS_DCM_ON);	
	} else {
        return -EINVAL;
	}
    return 0;
}

static int ubus4_dcm_divider_get (char *buffer, struct kernel_param *kp)
{
    return sprintf(buffer, 
				"%d\t : valid value { 1, 2, 3, 4, 5, 6, 7}", 
				dcm_divider_get());
}

static int ubus4_dcm_divider_set(const char *val, struct kernel_param *kp)
{
	u32 ival;

	kstrtouint (val, 0, &ival);

    if (ival>=1 && ival <=7) {
		dcm_divider_set(ival);
    } else {
		return -EINVAL;	// invalid input
    }
    return 0;
}

static int ubus4_dcm_thold_get (char *buffer, struct kernel_param *kp)
{
    return sprintf(buffer, 
		"0x%04x\t : valid value {1..0xFFFF}",dcm_thold_get());
}

static int ubus4_dcm_thold_set(const char *val, struct kernel_param *kp)
{
	u32	ival;

	kstrtouint (val, 0, &ival);

    if ((ival&0xFFFF) && ((ival&0xFFFF) <= 0xFFFF)) {
		dcm_thold_set (1,ival);
	} else {
		return -EINVAL;	// invalid input
	}
    return 0;
}

#ifdef DEBUG
static int ubus4_dcm_config_get (char *buffer, struct kernel_param *kp)
{
	dcm_config_get();	// for debug purpose
    return sprintf(buffer, "valid value { any, don't care }");
}
#endif

static int ubus4_dcm_config_set(const char *val, struct kernel_param *kp)
{
	dcm_config_set();
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
	dcm_config_set();
	if (!strcmp(str, "disable")) {
	 	dcm_enable_set(UBUS_DCM_OFF);
	} else {
	 	dcm_enable_set(UBUS_DCM_ON);
	}
	return 1;
}

__setup("ubus4_dcm=",ubus_dcm_init_param);

static int __init ubus_dcm_init(void)
{
	dcm_config_set();
	dcm_enable_set(UBUS_DCM_ON);	// default ON 
	return 0;
}

late_initcall(ubus_dcm_init);

