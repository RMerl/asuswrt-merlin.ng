/*
<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

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
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>

#include "pmc_drv.h"
#include "pmc_dgasp.h"
#include "BPCM.h"

#ifndef PMB_ADDR_VDSL_DGASP_PMD
#error "PMB_ADDR_VDSL_DGASP_PMD not defined in pmc_drv.h"
#endif
#ifndef BPCM_VDSL_PHY_CTL_REG
#error "BPCM_VDSL_PHY_CTL_REG not defined in pmc_drv.h"
#endif
#ifndef BPCM_VDSL_AFE_CTL_REG
#error "BPCM_VDSL_AFE_CTL_REG not defined in pmc_drv.h"
#endif

#define DGASP_DBG 0

/* Update these values if DGASP h/w changes */
#if defined(CONFIG_BCM963146)
#define DG_HYS_SHIFT 		1
#define DG_THRESH_SHIFT		3
#define DG_ENABLE		0x0001
#define DG_BITS_MASK	 	0x001F
#define DG_AFE_REG_SHIFT	16
#define DG_BGBIAS_REG_MASK      0xFFFF
#define DG_BGBIAS_VAL		0x0005
#else
#define DG_HYS_SHIFT 		4
#define DG_THRESH_SHIFT		6
#define DG_ENABLE		0x08
#define DG_BITS_MASK	 	0xF8
#define DG_AFE_REG_SHIFT	16
#define DG_BGBIAS_REG_MASK      0xFFFF
#define DG_BGBIAS_VAL		0x04CD
#endif

typedef enum
{
	DG_THRESH_1_25V=0,
	DG_THRESH_1_30V,
	DG_THRESH_1_15V,
	DG_THRESH_1_20V,
} DG_THRESH;

typedef enum
{
	DG_HYS_50mV=0,
	DG_HYS_100mV,
	DG_HYS_0mV,
	DG_HYS_25mV,
} DG_HYS;

void pmc_dgasp_init(void)
{
	uint16_t afe_reg0_val = 0;
	uint16_t afe_bgbias_val = 0;
	int status = 0;
	int dev = PMB_ADDR_VDSL_DGASP_PMD;
	volatile int afe_bgbias_cntl_reg_val = 0;
	volatile int phy_cntl_reg_val;
#if defined(NO_DSL) && defined(CONFIG_BCM963158)
	int clkrst_val;
#endif	

	/* Read DG registers */
	status = ReadBPCMRegister(dev, BPCMVDSLRegOffset(BPCM_VDSL_AFE_CTL_REG), (uint32_t*)&afe_bgbias_cntl_reg_val);
#if DGASP_DBG	
	printk("PMB addr 0x%08x: Read 0x%08x @offset 0x%08x, ret:%d\n", 
		dev << 12, afe_bgbias_cntl_reg_val, BPCMVDSLOffset(BPCM_VDSL_AFE_CTL_REG), status);
#endif	

	if(status == kPMC_NO_ERROR)
	{
		/* Set proper values for DGASP threshold and hysterysis */
        	afe_reg0_val = afe_bgbias_cntl_reg_val >> DG_AFE_REG_SHIFT;
		afe_reg0_val &= ~DG_BITS_MASK;
#if defined(CONFIG_BCM963178) || defined(CONFIG_BCM963146)
		afe_reg0_val |= ( (DG_THRESH_1_20V << DG_THRESH_SHIFT) 
#else		
		afe_reg0_val |= ( (DG_THRESH_1_25V << DG_THRESH_SHIFT) 
#endif		
		                | (DG_HYS_0mV << DG_HYS_SHIFT) 
				|  DG_ENABLE
				 );

		afe_bgbias_val = DG_BGBIAS_VAL;
		afe_bgbias_cntl_reg_val = afe_reg0_val << DG_AFE_REG_SHIFT | afe_bgbias_val;

		status = WriteBPCMRegister(dev, BPCMVDSLRegOffset(BPCM_VDSL_AFE_CTL_REG), afe_bgbias_cntl_reg_val);
#if DGASP_DBG	
		printk("PMB addr 0x%08x: Wrote 0x%08x @offset 0x%08x,  ret:%d\n", 
			dev << 12, afe_bgbias_cntl_reg_val, BPCMVDSLOffset(BPCM_VDSL_AFE_CTL_REG), status);
#endif	
	}
	
	/* Enable dg register programming override */
	status = ReadBPCMRegister(dev, BPCMVDSLRegOffset(BPCM_VDSL_PHY_CTL_REG), (uint32_t*)&phy_cntl_reg_val);
#if DGASP_DBG	
	printk("PMB addr 0x%08x: Read 0x%08x @offset 0x%08x,  ret:%d\n", 
		dev << 12, phy_cntl_reg_val, BPCMVDSLOffset(BPCM_VDSL_PHY_CTL_REG), status);
#endif	

	if(status == kPMC_NO_ERROR)
	{
		/* For 63146 the host always has control, setting this bit
		 * simply allows the phy to take control whenever it needs to
		 */
		phy_cntl_reg_val |= BPCM_PHY_CNTL_OVERRIDE;
		status = WriteBPCMRegister(dev, BPCMVDSLRegOffset(BPCM_VDSL_PHY_CTL_REG), phy_cntl_reg_val);
#if DGASP_DBG	
		printk("PMB addr 0x%08x: Wrote 0x%08x @offset 0x%08x, ret:%d\n", 
			dev << 12, phy_cntl_reg_val, BPCMVDSLOffset(BPCM_VDSL_PHY_CTL_REG), status);
#endif	
	}

#if defined(NO_DSL) && defined(CONFIG_BCM963158)
	/* If this is a NODSL build, power off most of the AFE */
	status = ReadBPCMRegister(PMB_ADDR_CHIP_CLKRST, CLKRSTBPCMRegOffset(clkrst_control), (uint32_t*)&clkrst_val);
	if(status == kPMC_NO_ERROR)
	{
		clkrst_val |= BPCM_CLKRST_AFE_PWRDWN;
		status = WriteBPCMRegister(PMB_ADDR_CHIP_CLKRST, CLKRSTBPCMRegOffset(clkrst_control), clkrst_val);
	}
#if DGASP_DBG	
	printk("PMB addr 0x%08x: Wrote 0x%08x @offset 0x%08x, ret:%d\n", PMB_ADDR_CHIP_CLKRST << 12, clkrst_val, CLKRSTBPCMOffset(clkrst_control), status);
#endif	
#endif /* defined(NO_DSL) && defined(CONFIG_BCM963158) */	

#if defined(NO_DSL) && defined(CONFIG_BCM963146)
	/* If this is a NODSL build, power off most of the AFE */
	status = ReadBPCMRegister(dev, BPCMVDSLRegOffset(BPCM_VDSL_PHY_CTL_REG), (uint32_t*)&phy_cntl_reg_val);
	if(status == kPMC_NO_ERROR)
	{
		phy_cntl_reg_val |= BPCM_PHY_CNTL_AFE_PWRDWN;
		status = WriteBPCMRegister(dev, BPCMVDSLRegOffset(BPCM_VDSL_PHY_CTL_REG), phy_cntl_reg_val);
	}
#if DGASP_DBG	
	printk("PMB addr 0x%08x: Wrote 0x%08x @offset 0x%08x, ret:%d\n", 
		dev << 12, phy_cntl_reg_val, BPCMVDSLOffset(BPCM_VDSL_PHY_CTL_REG), status);
#endif	
#endif /* defined(NO_DSL) && defined(CONFIG_BCM963146) */	
	BUG_ON(status != kPMC_NO_ERROR);
}

EXPORT_SYMBOL(pmc_dgasp_init);

void pmc_dgasp_override_disable(void)
{
	int status = 0;
	int dev = PMB_ADDR_VDSL_DGASP_PMD;
	volatile int phy_cntl_reg_val;
	status = ReadBPCMRegister(dev, BPCMVDSLRegOffset(BPCM_VDSL_PHY_CTL_REG), (uint32_t*)&phy_cntl_reg_val);
#if DGASP_DBG	
	printk("PMB addr 0x%08x: Read 0x%08x @offset 0x%08x, ret:%d\n", 
		dev << 12, phy_cntl_reg_val, BPCMVDSLOffset(BPCM_VDSL_PHY_CTL_REG), status);
#endif	

	if(status == kPMC_NO_ERROR)
	{
#if defined(CONFIG_BCM963146)
		/* For 63146 the host always has control, setting this bit
		 * simply allows the phy to take control whenever it needs to
		 */
		phy_cntl_reg_val |= BPCM_PHY_CNTL_OVERRIDE;
#else		
		phy_cntl_reg_val &= ~BPCM_PHY_CNTL_OVERRIDE;
#endif		
		status = WriteBPCMRegister(dev, BPCMVDSLRegOffset(BPCM_VDSL_PHY_CTL_REG), phy_cntl_reg_val);
#if DGASP_DBG	
		printk("PMB addr 0x%08x: Wrote 0x%08x @offset 0x%08x, ret:%d\n", 
			dev << 12, phy_cntl_reg_val, BPCMVDSLOffset(BPCM_VDSL_PHY_CTL_REG), status);
#endif	
	}
	BUG_ON(status != kPMC_NO_ERROR);
}
EXPORT_SYMBOL(pmc_dgasp_override_disable);

void pmc_dgasp_get_config( unsigned int * afe_reg0, unsigned int * bg_bias0 )
{
	int status = 0;
	int dev = PMB_ADDR_VDSL_DGASP_PMD;
	volatile unsigned int afe_bgbias_cntl_reg_val = 0;

	/* Read DG registers */
	status = ReadBPCMRegister(dev, BPCMVDSLRegOffset(BPCM_VDSL_AFE_CTL_REG), (uint32_t*)&afe_bgbias_cntl_reg_val);
#if DGASP_DBG	
	printk("PMB addr 0x%08x: Read 0x%08x @offset 0x%08x, ret:%d\n", 
		dev << 12, afe_bgbias_cntl_reg_val, BPCMVDSLOffset(BPCM_VDSL_AFE_CTL_REG), status);
#endif	

	if(status == kPMC_NO_ERROR)
	{
		/* Set proper values for DGASP threshold and hysterysis */
        	*afe_reg0 = afe_bgbias_cntl_reg_val >> DG_AFE_REG_SHIFT;
		*bg_bias0 = afe_bgbias_cntl_reg_val & DG_BGBIAS_REG_MASK;
	}
	BUG_ON(status != kPMC_NO_ERROR);
}
EXPORT_SYMBOL(pmc_dgasp_get_config);

