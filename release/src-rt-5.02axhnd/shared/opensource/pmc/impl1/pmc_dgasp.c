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

void pmc_dgasp_init( uint16_t afe_reg0_val, uint16_t afe_bgbias_val )
{
	int status = 0;
	uint32 sr_ctrl;
	int dev = PMB_ADDR_VDSL3_PMD;
	int afe_bgbias_cntl_reg_val = afe_reg0_val << 16 | afe_bgbias_val;
	int phy_cntl_reg_val;

	/* Enable dg register programming override */
	status = ReadBPCMRegister(dev, BPCM_PHY_CNTL_OFFSET, &phy_cntl_reg_val);
	if(status == kPMC_NO_ERROR)
	{
		phy_cntl_reg_val |= BPCM_PHY_CNTL_OVERRIDE;
		status = WriteBPCMRegister(dev, BPCM_PHY_CNTL_OFFSET, phy_cntl_reg_val);
	}

	/* Program DG registers */
	if(status == kPMC_NO_ERROR)
		status = WriteBPCMRegister(dev, BPCM_AFEREG_BGBIAS_CNTL_OFFSET, afe_bgbias_cntl_reg_val);
	
	BUG_ON(status != kPMC_NO_ERROR);
}

#ifndef _CFE_
EXPORT_SYMBOL(pmc_dgasp_init);
#endif

