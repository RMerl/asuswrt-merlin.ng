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

#include "clk_rst.h"
#include "pmc_drv.h"
#include "bl_os_wraper.h"

int pll_vco_freq_set(unsigned int pll_addr, struct PLL_DIVIDERS *divs)
{
	int ret = 0;
	uint32 val;
	
	ret |= WriteBPCMRegister(pll_addr, 7, (1 << 31) | (divs->ndiv_frac << 10) | divs->ndiv_int);
	ret |= WriteBPCMRegister(pll_addr, 8, (1 << 31) | divs->pdiv);
	ret |= WriteBPCMRegister(pll_addr, 9, (divs->kp << 12) | (divs->ki << 8) | (divs->ka << 4));
	//ret |= WriteBPCMRegister(pll_addr, 7, (divs->ndiv_frac << 10) | divs->ndiv_int);
	//ret |= WriteBPCMRegister(pll_addr, 8, divs->pdiv);
	
	do
	{
		ret = ReadBPCMRegister(pll_addr, 15, &val);
	}
	while( !ret && !(val & (1 << 31)));
	
	return ret;
}

int pll_ch_freq_set(unsigned int pll_addr, unsigned int ch, unsigned int mdiv)
{
	int ret = 0;
	uint32 val;
	unsigned int reg = 11;
	unsigned int off = 0;
	
	switch(ch)
	{
		case 0:
		case 1:
			reg = 11;
			break;
		case 2:
		case 3:
			reg = 12;
			break;
		case 4:
		case 5:
			reg = 13;
			break;
	}
	
	switch(ch)
	{
		case 1:
		case 3:
		case 5:
			off = 16;
			break;
	}

	ret = ReadBPCMRegister(pll_addr, reg, &val);
	if(!ret)
	{
		// reset LOAD_EN_CH0
		val &= ~(1 << (10+off));
		ret |= WriteBPCMRegister(pll_addr, reg, val);
		// set MDIV
		val &= ~(0xff << off);
		val |= (mdiv << off) | (1 << (15+off));
		ret |= WriteBPCMRegister(pll_addr, reg, val);
		// set LOAD_EN_CH0
		val |= (1 << (10+off));
		ret |= WriteBPCMRegister(pll_addr, reg, val);
	}
		
	return ret;
}

static struct DDR_DIVIDERS
{
	struct PLL_DIVIDERS pll;
	unsigned int mdiv;
} ddr_divs[] =
{
	{{1, 96, 0, 1, 2, 9}, 18},	//133MHz
	{{1, 96, 0, 1, 2, 9}, 9},	//266MHz
	{{2, 159, 0, 1, 1, 5}, 6},	//333MHz
	{{1, 48, 0, 1, 2, 7}, 3},	//400MHz
	{{2, 149, 0, 1, 1, 5}, 4},	//466MHz
	{{1, 85, 0, 1, 2, 9}, 4},	//533MHz
};

int ddr_freq_set(unsigned long freq)
{
	int ret = 0;
	int i = 5;

	switch(freq)
	{
		case 133:
			i = 0;
			break;

		case 266:
			i = 1;
			break;

		case 333:
			i = 2;
			break;

		case 400:
			i = 3;
			break;

		case 466:
			i = 4;
			break;

		case 533:
			i = 5;
			break;
	}
	
	ret = pll_vco_freq_set(PMB_ADDR_SYSPLL1, &(ddr_divs[i].pll));
	ret |= pll_ch_freq_set(PMB_ADDR_SYSPLL1, 0, ddr_divs[i].mdiv);
	
	return ret;
}

#define VCO0_FREQ	1200
#define VCO2_FREQ	1600

int viper_freq_set(unsigned long freq)
{
	return pll_ch_freq_set(PMB_ADDR_SYSPLL0, 0, VCO0_FREQ/freq);

}

struct PLL_DIVIDERS rdp_divs = {1, 48, 0, 1, 2, 7};

int rdp_freq_set(unsigned long freq)
{
	int ret =0;
	
	if(freq == -1)
	{
		ret = pll_vco_freq_set(PMB_ADDR_SYSPLL2, &rdp_divs);
		ret |= pll_ch_freq_set(PMB_ADDR_SYSPLL2, 0, 2);
		ret |= pll_ch_freq_set(PMB_ADDR_SYSPLL2, 1, 4);
	}
	else
	{
		ret = pll_ch_freq_set(PMB_ADDR_SYSPLL2, 0, VCO2_FREQ/freq);
	}
	return ret;
}

unsigned long get_rdp_freq(unsigned int* rdp_freq)
{
	uint32 val, vco2_freq;
    int ret = 0;
    unsigned int mdiv = 0;
    
	ret = ReadBPCMRegister(PMB_ADDR_SYSPLL2, 7, &val);
	vco2_freq = 25 * (val & 0x3ff);
	
    ret |= ReadBPCMRegister(PMB_ADDR_SYSPLL2, 11, &val);
    if (val & (1 << 15))
        mdiv = val & 0xff;

	if(mdiv)
	{
		*rdp_freq = vco2_freq/mdiv;
	}
	else
	{
		*rdp_freq = 0;
		ret = -1;
	}

    return ret;
}
EXPORT_SYMBOL(get_rdp_freq);

