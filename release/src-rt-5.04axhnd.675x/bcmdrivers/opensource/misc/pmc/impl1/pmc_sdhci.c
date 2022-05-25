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
#include "pmc_drv.h"
#include "clk_rst.h"
#include "pmc_sdhci.h"
#include "BPCM.h"
#include "board.h"

typedef struct {
    uint64_t freq;
    uint32_t pll_addr;
    uint32_t ch;
    uint32_t offset;
    uint16_t mdiv;
} SDHCI_FREQ_MAP;


static SDHCI_FREQ_MAP freq_map[] = { 
#if defined(CONFIG_BCM963146)
	{100000000,PMB_ADDR_RDPPLL,1,RDPPLLBPCMRegOffset(ch01_cfg),0x32}, // 100Mhz
	{125000000,PMB_ADDR_RDPPLL,1,RDPPLLBPCMRegOffset(ch01_cfg),0x28}, // 125Mhz
	{147000000,PMB_ADDR_RDPPLL,1,RDPPLLBPCMRegOffset(ch01_cfg),0x22}, // 147Mhz
	{180000000,PMB_ADDR_RDPPLL,1,RDPPLLBPCMRegOffset(ch01_cfg),0x1c}, // 178Mhz
	{185000000,PMB_ADDR_RDPPLL,1,RDPPLLBPCMRegOffset(ch01_cfg),0x1b}, // 185Mhz
	{200000000,PMB_ADDR_RDPPLL,1,RDPPLLBPCMRegOffset(ch01_cfg),0x19}, // 200Mhz
#endif 
#if defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
	{100000000,PMB_ADDR_BIU_PLL,5,PLLBPCMRegOffset(ch45_cfg),0x28}, // 100Mhz
	{160000000,PMB_ADDR_BIU_PLL,5,PLLBPCMRegOffset(ch45_cfg),0x19}, // 160Mhz
	{180000000,PMB_ADDR_BIU_PLL,5,PLLBPCMRegOffset(ch45_cfg),0x16}, // 180Mhz
	{200000000,PMB_ADDR_BIU_PLL,5,PLLBPCMRegOffset(ch45_cfg),0x14}, // 200Mhz
#endif 
	{0,0,0,0} 
	};


int pmc_sdhci_set_base_clk( uint64_t freq )
{
	int ret = -1;
	SDHCI_FREQ_MAP * pll_freq_map = &freq_map[0];
	
	while( pll_freq_map->freq )
	{
		if( pll_freq_map->freq == freq )
		{
			printk("%s: Setting sdhci base clock freq to: %llu\n", __FUNCTION__, freq);
			ret = pll_ch_freq_set_offs(pll_freq_map->pll_addr, 
				pll_freq_map->ch, 
				pll_freq_map->mdiv,
				pll_freq_map->offset); 
#ifndef CONFIG_BRCM_QEMU
			kerSysSetSdhciBaseClkSrc(1);
#endif
			break;
		}
		pll_freq_map++;
	}
	
	return ret;
}
EXPORT_SYMBOL(pmc_sdhci_set_base_clk);

