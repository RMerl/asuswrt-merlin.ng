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

