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
#include <asm/div64.h>

typedef struct {
    uint32_t pll_addr;
    uint32_t ch;
    uint32_t offset;
} SDHCI_PLL_MAP;

static SDHCI_PLL_MAP pll_map =  
#if defined(CONFIG_BCM963146)
	{PMB_ADDR_RDPPLL,1,RDPPLLBPCMRegOffset(ch01_cfg)};
#elif defined(CONFIG_BCM94912) 
	{PMB_ADDR_BIU_PLL,5,PLLBPCMRegOffset(ch45_cfg)}; 
#elif defined(CONFIG_BCM96813)
	{PMB_ADDR_BIU_PLL,5,PLLBPCMRegOffset(ch45_cfg)};
#elif defined(CONFIG_BCM96756)
	{PMB_ADDR_BIU_PLL,4,PLLBPCMRegOffset(ch45_cfg)};
#elif defined(CONFIG_BCM96888)
	{PMB_ADDR_SYSPLL,3,PLLBPCMRegOffset(ch23_cfg)};
#elif defined(CONFIG_BCM96765)
	{PMB_ADDR_SYSPLL,3,SYSPLLBPCMRegOffset(ch23_cfg)};
#else 
	{0,0,0}; 
#endif

int pmc_sdhci_set_base_clk( uint64_t freq, struct device_node *node )
{
	int ret = -1;
	uint32_t tmp, i, baseclk, num_freqs, mdiv;
	
	if(node)
	{
		if(of_property_read_u32(node, "sdhci-pll-baseclk-mhz", &baseclk))
		{
			return ret;
		}
		else
		{
			printk("%s: PLL Baseclk freq: %uMhz\n", __FUNCTION__, baseclk);
		}

		/* Determine size of valid o/p freq array */
		if (!of_get_property(node, "sdhci-pll-valid-op-mhz", &tmp))
			return ret;

		num_freqs = tmp / (sizeof(uint32_t));
		do_div(freq,1000000);
	
		for (i = 0; i < num_freqs; i++) 
		{
			if (of_property_read_u32_index(node, "sdhci-pll-valid-op-mhz", i, &tmp))
			{
				return ret;
			}
			else
			{
				printk("%s: Supported freq: %uMhz\n", __FUNCTION__, tmp);
				if( tmp == freq)
				{
					/* Matched supported frequency */
					mdiv = baseclk/tmp;
					printk("%s: Setting sdhci base clock freq to: %lluMhz, mdiv: %d\n", __FUNCTION__, freq, mdiv);
					ret = pll_ch_freq_set_offs(pll_map.pll_addr, 
						pll_map.ch, 
						mdiv,
						pll_map.offset); 
					break;
				}
			}
		}
	}
	return ret;
}
EXPORT_SYMBOL(pmc_sdhci_set_base_clk);

