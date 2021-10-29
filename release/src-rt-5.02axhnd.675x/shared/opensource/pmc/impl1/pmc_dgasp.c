/*
<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:> 
*/
#ifndef _CFE_
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#endif

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
#define DG_HYS_SHIFT 		4
#define DG_THRESH_SHIFT		6
#define DG_ENABLE		0x08
#define DG_BITS_MASK	 	0xF8
#define DG_AFE_REG_SHIFT	16
#define DG_BGBIAS_REG_MASK      0xFFFF
#define DG_BGBIAS_VAL		0x04CD

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
	status = ReadBPCMRegister(dev, BPCMVDSLRegOffset(BPCM_VDSL_AFE_CTL_REG), (uint32 *)&afe_bgbias_cntl_reg_val);
#if DGASP_DBG	
	printk("PMB addr 0x%08x: Read 0x%08x @offset 0x%08x, ret:%d\n", 
		dev << 12, afe_bgbias_cntl_reg_val, BPCMVDSLOffset(BPCM_VDSL_AFE_CTL_REG), status);
#endif	

	if(status == kPMC_NO_ERROR)
	{
		/* Set proper values for DGASP threshold and hysterysis */
        	afe_reg0_val = afe_bgbias_cntl_reg_val >> DG_AFE_REG_SHIFT;
		afe_reg0_val &= ~DG_BITS_MASK;
#if defined(CONFIG_BCM963178)
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
	status = ReadBPCMRegister(dev, BPCMVDSLRegOffset(BPCM_VDSL_PHY_CTL_REG), (uint32 *)&phy_cntl_reg_val);
#if DGASP_DBG	
	printk("PMB addr 0x%08x: Read 0x%08x @offset 0x%08x,  ret:%d\n", 
		dev << 12, phy_cntl_reg_val, BPCMVDSLOffset(BPCM_VDSL_PHY_CTL_REG), status);
#endif	

	if(status == kPMC_NO_ERROR)
	{
		phy_cntl_reg_val |= BPCM_PHY_CNTL_OVERRIDE;
		status = WriteBPCMRegister(dev, BPCMVDSLRegOffset(BPCM_VDSL_PHY_CTL_REG), phy_cntl_reg_val);
#if DGASP_DBG	
		printk("PMB addr 0x%08x: Wrote 0x%08x @offset 0x%08x, ret:%d\n", 
			dev << 12, phy_cntl_reg_val, BPCMVDSLOffset(BPCM_VDSL_PHY_CTL_REG), status);
#endif	
	}

#if defined(NO_DSL) && defined(CONFIG_BCM963158)
	/* If this is a NODSL build, power off most of the AFE */
	status = ReadBPCMRegister(PMB_ADDR_CHIP_CLKRST, CLKRSTBPCMRegOffset(clkrst_control), (uint32 *)&clkrst_val);
	if(status == kPMC_NO_ERROR)
	{
		clkrst_val |= BPCM_CLKRST_AFE_PWRDWN;
		status = WriteBPCMRegister(PMB_ADDR_CHIP_CLKRST, CLKRSTBPCMRegOffset(clkrst_control), clkrst_val);
	}
#if DGASP_DBG	
	printk("PMB addr 0x%08x: Wrote 0x%08x @offset 0x%08x, ret:%d\n", PMB_ADDR_CHIP_CLKRST << 12, clkrst_val, CLKRSTBPCMOffset(clkrst_control), status);
#endif	
#endif /* defined(NO_DSL) && defined(CONFIG_BCM963158) */	

	BUG_ON(status != kPMC_NO_ERROR);
}

#ifndef _CFE_
EXPORT_SYMBOL(pmc_dgasp_init);
#endif

void pmc_dgasp_override_disable(void)
{
	int status = 0;
	int dev = PMB_ADDR_VDSL_DGASP_PMD;
	volatile int phy_cntl_reg_val;
	status = ReadBPCMRegister(dev, BPCMVDSLRegOffset(BPCM_VDSL_PHY_CTL_REG), (uint32 *)&phy_cntl_reg_val);
#if DGASP_DBG	
	printk("PMB addr 0x%08x: Read 0x%08x @offset 0x%08x, ret:%d\n", 
		dev << 12, phy_cntl_reg_val, BPCMVDSLOffset(BPCM_VDSL_PHY_CTL_REG), status);
#endif	

	if(status == kPMC_NO_ERROR)
	{
		phy_cntl_reg_val &= ~BPCM_PHY_CNTL_OVERRIDE;
		status = WriteBPCMRegister(dev, BPCMVDSLRegOffset(BPCM_VDSL_PHY_CTL_REG), phy_cntl_reg_val);
#if DGASP_DBG	
		printk("PMB addr 0x%08x: Wrote 0x%08x @offset 0x%08x, ret:%d\n", 
			dev << 12, phy_cntl_reg_val, BPCMVDSLOffset(BPCM_VDSL_PHY_CTL_REG), status);
#endif	
	}
	BUG_ON(status != kPMC_NO_ERROR);
}
#ifndef _CFE_
EXPORT_SYMBOL(pmc_dgasp_override_disable);
#endif

void pmc_dgasp_get_config( unsigned int * afe_reg0, unsigned int * bg_bias0 )
{
	int status = 0;
	int dev = PMB_ADDR_VDSL_DGASP_PMD;
	volatile unsigned int afe_bgbias_cntl_reg_val = 0;

	/* Read DG registers */
	status = ReadBPCMRegister(dev, BPCMVDSLRegOffset(BPCM_VDSL_AFE_CTL_REG), (uint32 *)&afe_bgbias_cntl_reg_val);
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
#ifndef _CFE_
EXPORT_SYMBOL(pmc_dgasp_get_config);
#endif
