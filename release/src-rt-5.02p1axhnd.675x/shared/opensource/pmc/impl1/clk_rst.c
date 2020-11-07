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
#include "BPCM.h"
#include "bcm_map_part.h"

#ifndef _CFE_
#include <linux/module.h>
#include <linux/delay.h>
#else
#include "lib_printf.h"
#define printk       xprintf
#endif

#if defined(_BCM96838_) || defined(CONFIG_BCM96838)
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
#endif

#if defined(_BCM96838_) || defined(CONFIG_BCM96838)
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
#endif

#define VCO0_FREQ	1200
#define VCO2_FREQ	1600

#if defined(_BCM96848_) || defined(CONFIG_BCM96848) || defined(_BCM96858_) || defined(CONFIG_BCM96858)    || \
    defined(_BCM963158_) || defined(CONFIG_BCM963158) || defined(_BCM96856_) || defined(CONFIG_BCM96856) || \
    defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM963178_) || defined(CONFIG_BCM963178) || \
    defined(_BCM947622_) || defined(CONFIG_BCM947622) || defined(_BCM96878_) || defined(CONFIG_BCM96878)
#define PLL_REFCLK  50 
int pll_vco_freq_get(unsigned int pll_addr, unsigned int* fvco)
{
	int ret = 0;
	PLL_DECNDIV_REG pll_decndiv;
	PLL_DECPDIV_REG pll_decpdiv;
#if defined(_BCM963158_) || defined(CONFIG_BCM963158) || defined(_BCM963178_) || defined(CONFIG_BCM963178) || \
    defined(_BCM947622_) || defined(CONFIG_BCM947622)
	PLL_NDIV_REG ndiv_reg;
	PLL_PDIV_REG pdiv_reg;
#endif

#if defined(_BCM96856_) || defined(CONFIG_BCM96856)
	ret  = ReadBPCMRegister(pll_addr, PLLCLASSICBPCMRegOffset(decndiv), &pll_decndiv.Reg32);
	ret |= ReadBPCMRegister(pll_addr, PLLCLASSICBPCMRegOffset(decpdiv), &pll_decpdiv.Reg32);
#else
	ret  = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(decndiv), &pll_decndiv.Reg32);
	ret |= ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(decpdiv), &pll_decpdiv.Reg32);
#endif
	if (ret != 0)
		return -1;

	// Let's ignore ndiv_frac, it is set to zero anyway by HW.
	*fvco = (PLL_REFCLK * (pll_decndiv.Bits.ndiv_int))/pll_decpdiv.Bits.pdiv;

#if defined(_BCM963158_) || defined(CONFIG_BCM963158) || defined(_BCM963178_) || defined(CONFIG_BCM963178) || \
    defined(_BCM947622_) || defined(CONFIG_BCM947622)
	if (!ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(pdiv), &pdiv_reg.Reg32) &&
		pdiv_reg.Bits.ndiv_pdiv_override &&
		!ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ndiv), &ndiv_reg.Reg32))
		*fvco = (PLL_REFCLK * (ndiv_reg.Bits.ndiv_int))/pdiv_reg.Bits.pdiv;
#endif

	return 0;
}

int pll_ch_freq_get(unsigned int pll_addr, unsigned int ch, unsigned int* freq)
{
    int ret;
    unsigned int fvco, mdiv;
    PLL_DECPDIV_REG pll_decpdiv;
    PLL_DECCH25_REG pll_decch25;
    PLL_CHCFG_REG ch_cfg;

#if defined(_BCM96848_) || defined(CONFIG_BCM96848)
    if ( (pll_addr==5) && (ch==1)  )
    {
        uint32 data;
        ret = ReadBPCMRegister(5, 11, &data);
        if (ret != 0)
            return -1;
        if (data & 0x80000000)
        {
            *freq = 428;
            return 0;
        }

    }
#endif

    ret = pll_vco_freq_get(pll_addr, &fvco);

    if (ret != 0)
        return -1;

    // The pll may include up to 6 channels.
    switch (ch)
    {
    case 0:
#if defined(_BCM96856_) || defined(CONFIG_BCM96856)
        ret = ReadBPCMRegister(pll_addr, PLLCLASSICBPCMRegOffset(ch01_cfg), &ch_cfg.Reg32);
#else
        ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch01_cfg), &ch_cfg.Reg32);
#endif
        if (ch_cfg.Bits.mdiv_override0) /* Check if default value is overitten */
            mdiv = ch_cfg.Bits.mdiv0; /* Use the new value */
        else /* If not, read from the default */
        {
#if defined(_BCM96856_) || defined(CONFIG_BCM96856)
            ret = ReadBPCMRegister(pll_addr, PLLCLASSICBPCMRegOffset(decpdiv), &pll_decpdiv.Reg32);
#else
            ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(decpdiv), &pll_decpdiv.Reg32);
#endif
            mdiv = pll_decpdiv.Bits.mdiv0;
        }
        break;
    case 1:
#if defined(_BCM96856_) || defined(CONFIG_BCM96856)
        ret = ReadBPCMRegister(pll_addr, PLLCLASSICBPCMRegOffset(ch01_cfg), &ch_cfg.Reg32);
#else
        ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch01_cfg), &ch_cfg.Reg32);
#endif
        if (ch_cfg.Bits.mdiv_override1) 
            mdiv = ch_cfg.Bits.mdiv1; 
        else 
        {
#if defined(_BCM96856_) || defined(CONFIG_BCM96856)
            ret = ReadBPCMRegister(pll_addr, PLLCLASSICBPCMRegOffset(decpdiv), &pll_decpdiv.Reg32);
#else
            ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(decpdiv), &pll_decpdiv.Reg32);
#endif
            mdiv = pll_decpdiv.Bits.mdiv1;
        }
        break;
    case 2:
#if defined(_BCM96856_) || defined(CONFIG_BCM96856)
        ret = ReadBPCMRegister(pll_addr, PLLCLASSICBPCMRegOffset(ch23_cfg), &ch_cfg.Reg32);
#else
        ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch23_cfg), &ch_cfg.Reg32);
#endif
        if (ch_cfg.Bits.mdiv_override0) 
            mdiv = ch_cfg.Bits.mdiv0; 
        else 
        {
#if defined(_BCM96856_) || defined(CONFIG_BCM96856)
            ret = ReadBPCMRegister(pll_addr, PLLCLASSICBPCMRegOffset(decch25), &pll_decch25.Reg32);
#else
            ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(decch25), &pll_decch25.Reg32);
#endif
            mdiv = pll_decch25.Bits.mdiv2;
        }
        break;
    case 3:
#if defined(_BCM96856_) || defined(CONFIG_BCM96856)
        ret = ReadBPCMRegister(pll_addr, PLLCLASSICBPCMRegOffset(ch23_cfg), &ch_cfg.Reg32);
#else
        ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch23_cfg), &ch_cfg.Reg32);
#endif
        if (ch_cfg.Bits.mdiv_override1) 
            mdiv = ch_cfg.Bits.mdiv1; 
        else 
        {
#if defined(_BCM96856_) || defined(CONFIG_BCM96856)
            ret = ReadBPCMRegister(pll_addr, PLLCLASSICBPCMRegOffset(decch25), &pll_decch25.Reg32);
#else
            ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(decch25), &pll_decch25.Reg32);
#endif
            mdiv = pll_decch25.Bits.mdiv3;
        }
        break;
    case 4:
#if defined(_BCM96856_) || defined(CONFIG_BCM96856)
        ret = ReadBPCMRegister(pll_addr, PLLCLASSICBPCMRegOffset(ch45_cfg), &ch_cfg.Reg32);
#else
        ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch45_cfg), &ch_cfg.Reg32);
#endif
        if (ch_cfg.Bits.mdiv_override0) 
            mdiv = ch_cfg.Bits.mdiv0;
        else 
        {
#if defined(_BCM96856_) || defined(CONFIG_BCM96856)
            ret = ReadBPCMRegister(pll_addr, PLLCLASSICBPCMRegOffset(decch25), &pll_decch25.Reg32);
#else
            ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(decch25), &pll_decch25.Reg32);
#endif
            mdiv = pll_decch25.Bits.mdiv4;
        }
        break;
    case 5:
#if defined(_BCM96856_) || defined(CONFIG_BCM96856)
        ret = ReadBPCMRegister(pll_addr, PLLCLASSICBPCMRegOffset(ch45_cfg), &ch_cfg.Reg32);
#else
        ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch45_cfg), &ch_cfg.Reg32);
#endif
        if (ch_cfg.Bits.mdiv_override1) 
            mdiv = ch_cfg.Bits.mdiv1; 
        else 
        {
#if defined(_BCM96856_) || defined(CONFIG_BCM96856)
            ret = ReadBPCMRegister(pll_addr, PLLCLASSICBPCMRegOffset(decch25), &pll_decch25.Reg32);
#else
            ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(decch25), &pll_decch25.Reg32);
#endif
            mdiv = pll_decch25.Bits.mdiv5;
        }
        break;
    default:
        return -1;
    };

    if (ret != 0)
        return -1;

    *freq = fvco/mdiv;

    return 0;
}
#ifndef _CFE_
EXPORT_SYMBOL(pll_ch_freq_get);
#endif
#endif

#if defined(_BCM96838_) || defined(CONFIG_BCM96838)
int viper_freq_set(unsigned long freq)
{
	return pll_ch_freq_set(PMB_ADDR_SYSPLL0, 0, VCO0_FREQ/freq);

}

int rdp_freq_set(unsigned long freq)
{
	return pll_ch_freq_set(PMB_ADDR_SYSPLL2, 0, VCO2_FREQ/freq);
}
#endif

#if defined(_BCM96838_) || defined(CONFIG_BCM96838)
unsigned long get_rdp_freq(unsigned int* rdp_freq)
{
	uint32 val;
	int ret = 0;
	unsigned int mdiv = 0;

	ret |= ReadBPCMRegister(PMB_ADDR_SYSPLL2, 11, &val);
	if (val & (1 << 15))
		mdiv = val & 0xff;

	if (mdiv)
		*rdp_freq = VCO2_FREQ/mdiv;
	else {
		*rdp_freq = 0;
		ret = -1;
	}

	return ret;
}
#elif defined(_BCM96848_) || defined(CONFIG_BCM96848)
unsigned long get_rdp_freq(unsigned int* rdp_freq)
{
	return pll_ch_freq_get(PMB_ADDR_SYSPLL0, SYSPLL0_RUNNER_CHANNEL, rdp_freq);
}
#elif defined(_BCM96878_) || defined(CONFIG_BCM96878)
unsigned long get_rdp_freq(unsigned int* rdp_freq)
{
    int ret;
    unsigned int fvco, mdiv;
    PLL_DECNDIV_REG pll_decndiv;
    PLL_DECPDIV_REG pll_decpdiv;

    ret  = ReadBPCMRegister(PMB_ADDR_SYSPLL, PLLCLASSICBPCMRegOffset(decndiv), &pll_decndiv.Reg32);
    ret |= ReadBPCMRegister(PMB_ADDR_SYSPLL, PLLCLASSICBPCMRegOffset(decpdiv), &pll_decpdiv.Reg32);
    // Let's ignore ndiv_frac, it is set to zero anyway by HW.
    fvco = (PLL_REFCLK * (pll_decndiv.Bits.ndiv_int))/pll_decpdiv.Bits.pdiv;

    // read mdiv of channel 0
    ret = ReadBPCMRegister(PMB_ADDR_SYSPLL, PLLCLASSICBPCMRegOffset(decpdiv), &pll_decpdiv.Reg32);
    mdiv = pll_decpdiv.Bits.mdiv0;
    
    *rdp_freq = fvco/mdiv;
    return ret;
}
#elif defined(_BCM96858_) || defined(CONFIG_BCM96858)   || \
      defined(_BCM963158_) || defined(CONFIG_BCM963158) || \
      defined(_BCM96856_) || defined(CONFIG_BCM96856)   || \
      defined(_BCM96846_) || defined(CONFIG_BCM96846)
unsigned long get_rdp_freq(unsigned int* rdp_freq)
{
    int ret;
    
    ret = pll_ch_freq_get(PMB_ADDR_RDPPLL, XRDPPLL_RUNNER_CHANNEL, rdp_freq);

#if defined(_BCM96856_) || defined(CONFIG_BCM96856) || defined(_BCM96846_) || defined(CONFIG_BCM96846)
    *rdp_freq /= 2;
#endif

    return ret;
}

#elif defined(_BCM963178_) || defined(CONFIG_BCM963178)
unsigned long get_rdp_freq(unsigned int* rdp_freq)
{
    return -1;
}
#elif defined(_BCM947622_) || defined(CONFIG_BCM947622)
unsigned long get_rdp_freq(unsigned int* rdp_freq)
{
    return -1;
}
/* FIXME!! when knowing the real frequency info for 4908/62118 RDP */
#elif defined(_BCM963138_) || defined(CONFIG_BCM963138) || defined(_BCM963148_) || defined(CONFIG_BCM963148) || defined(_BCM94908_) || defined(CONFIG_BCM94908)
#define RDP_PLL_REFCLK		50	/* 50 MHz for 63138 */
/* the formula here is
 * F_vco = (1 / pdiv) * (ndiv_int + ndiv_frac / (2 ^ 20)) * F_ref
 * F_clkout,n = (F_vco / mdiv_n)
 * ch#0 connects to runner block
 * ch#1 connects to test block
 * ch#2 connects to ipsec & rng block
 *
 * default values are:
 * ndiv_int = 0x8c (140)
 * ndiv_frac = 0
 * pdiv = 2
 * mdiv[0] = 0x5
 * mdiv[1] = 0xa
 * mdiv[2] = 0x23 (35).
 *
 * F_vco = 3500 MHz
 * F_clkout,0 = 700 MHz -> runner */
unsigned long get_rdp_freq(unsigned int *rdp_freq)
{
	int ret = 0;
	PLL_NDIV_REG pll_ndiv;
	PLL_PDIV_REG pll_pdiv;
	PLL_CHCFG_REG pll_ch01_cfg;

	ret |= ReadBPCMRegister(PMB_ADDR_RDPPLL, PLLBPCMRegOffset(ndiv),
			&pll_ndiv.Reg32);
	ret |= ReadBPCMRegister(PMB_ADDR_RDPPLL, PLLBPCMRegOffset(pdiv),
			&pll_pdiv.Reg32);
	ret |= ReadBPCMRegister(PMB_ADDR_RDPPLL, PLLBPCMRegOffset(ch01_cfg),
			&pll_ch01_cfg.Reg32);
	if (ret != 0)
		return -1;

	*rdp_freq = RDP_PLL_REFCLK * (pll_ndiv.Bits.ndiv_int);
	// FIXME! for simplicity, ndiv_frac is taken out.  Otherwise, the value
	// will be in form of double.
	*rdp_freq = *rdp_freq / pll_pdiv.Bits.pdiv / pll_ch01_cfg.Bits.mdiv0;

	return ret;
}
#endif

#if defined(_BCM96858_) || defined(CONFIG_BCM96858) || defined(_BCM96856_) || defined(CONFIG_BCM96856) || defined(_BCM96846_) || defined(CONFIG_BCM96846) || \
   defined(_BCM963158_) || defined(CONFIG_BCM963158) || defined(_BCM963178_) || defined(CONFIG_BCM963178) || defined(_BCM947622_) || defined(CONFIG_BCM947622) || \
   defined(_BCM96878_) || defined(CONFIG_BCM96878)
int pll_ch_freq_set(unsigned int pll_addr, unsigned int ch, unsigned int mdiv)
{
    int ret;
	PLL_CHCFG_REG ch_cfg;

    // The pll may include up to 6 channels.
    switch (ch)
    {
    case 0:
    	ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch01_cfg), &ch_cfg.Reg32);
    	ch_cfg.Bits.mdiv0 = mdiv;
        ch_cfg.Bits.mdiv_override0 = 1;
        ret |= WriteBPCMRegister(pll_addr, PLLBPCMRegOffset(ch01_cfg), ch_cfg.Reg32);
    	break;
    case 1:
    	ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch01_cfg), &ch_cfg.Reg32);
    	ch_cfg.Bits.mdiv1 = mdiv;
        ch_cfg.Bits.mdiv_override1 = 1;
        ret |= WriteBPCMRegister(pll_addr, PLLBPCMRegOffset(ch01_cfg), ch_cfg.Reg32);
    	break;
    case 2:
    	ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch23_cfg), &ch_cfg.Reg32);
    	ch_cfg.Bits.mdiv0 = mdiv;
        ch_cfg.Bits.mdiv_override0 = 1;
        ret |= WriteBPCMRegister(pll_addr, PLLBPCMRegOffset(ch23_cfg), ch_cfg.Reg32);
    	break;
    case 3:
    	ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch23_cfg), &ch_cfg.Reg32);
    	ch_cfg.Bits.mdiv1 = mdiv;
        ch_cfg.Bits.mdiv_override1 = 1;
        ret |= WriteBPCMRegister(pll_addr, PLLBPCMRegOffset(ch23_cfg), ch_cfg.Reg32);
    	break;
    case 4:
    	ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch45_cfg), &ch_cfg.Reg32);
    	ch_cfg.Bits.mdiv0 = mdiv;
        ch_cfg.Bits.mdiv_override0 = 1;
        ret |= WriteBPCMRegister(pll_addr, PLLBPCMRegOffset(ch45_cfg), ch_cfg.Reg32);
    	break;
    case 5:
    	ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch45_cfg), &ch_cfg.Reg32);
    	ch_cfg.Bits.mdiv1 = mdiv;
        ch_cfg.Bits.mdiv_override1 = 1;
        ret |= WriteBPCMRegister(pll_addr, PLLBPCMRegOffset(ch45_cfg), ch_cfg.Reg32);
    	break;
    default:
    	return -1;
    };

    return ret;
}

int pll_ch_freq_vco_set(unsigned int pll_addr, unsigned int ch, unsigned int mdiv, unsigned int use_vco)
{
    int ret;
	PLL_CHCFG_REG ch_cfg;

    // The pll may include up to 6 channels.
    switch (ch)
    {
    case 0:
    	ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch01_cfg), &ch_cfg.Reg32);
    	ch_cfg.Bits.mdiv0 = mdiv;
        ch_cfg.Bits.mdiv_override0 = 1;
        ch_cfg.Bits.reserved0 = use_vco ? 1:0;
        ret |= WriteBPCMRegister(pll_addr, PLLBPCMRegOffset(ch01_cfg), ch_cfg.Reg32);
    	break;
    case 1:
    	ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch01_cfg), &ch_cfg.Reg32);
    	ch_cfg.Bits.mdiv1 = mdiv;
        ch_cfg.Bits.mdiv_override1 = 1;
        ch_cfg.Bits.reserved1 = use_vco ? 1:0;
        ret |= WriteBPCMRegister(pll_addr, PLLBPCMRegOffset(ch01_cfg), ch_cfg.Reg32);
    	break;
    case 2:
    	ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch23_cfg), &ch_cfg.Reg32);
    	ch_cfg.Bits.mdiv0 = mdiv;
        ch_cfg.Bits.mdiv_override0 = 1;
        ch_cfg.Bits.reserved0 = use_vco ? 1:0;
        ret |= WriteBPCMRegister(pll_addr, PLLBPCMRegOffset(ch23_cfg), ch_cfg.Reg32);
    	break;
    case 3:
    	ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch23_cfg), &ch_cfg.Reg32);
    	ch_cfg.Bits.mdiv1 = mdiv;
        ch_cfg.Bits.mdiv_override1 = 1;
        ch_cfg.Bits.reserved1 = use_vco ? 1:0;
        ret |= WriteBPCMRegister(pll_addr, PLLBPCMRegOffset(ch23_cfg), ch_cfg.Reg32);
    	break;
    case 4:
    	ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch45_cfg), &ch_cfg.Reg32);
    	ch_cfg.Bits.mdiv0 = mdiv;
        ch_cfg.Bits.mdiv_override0 = 1;
        ch_cfg.Bits.reserved0 = use_vco ? 1:0;
        ret |= WriteBPCMRegister(pll_addr, PLLBPCMRegOffset(ch45_cfg), ch_cfg.Reg32);
    	break;
    case 5:
    	ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch45_cfg), &ch_cfg.Reg32);
    	ch_cfg.Bits.mdiv1 = mdiv;
        ch_cfg.Bits.mdiv_override1 = 1;
        ch_cfg.Bits.reserved1 = use_vco ? 1:0;
        ret |= WriteBPCMRegister(pll_addr, PLLBPCMRegOffset(ch45_cfg), ch_cfg.Reg32);
    	break;
    default:
    	return -1;
    };
    
    return ret;
}
#endif

#if defined(CONFIG_BCM96858)
int bcm_change_cpu_clk(BCM_CPU_CLK clock)
{
    int ret = 0;
    PLL_CTRL_REG ctrl_reg;

    if (ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets), &ctrl_reg.Reg32))
        return -1;

    if (clock == BCM_CPU_CLK_HIGH)
        ctrl_reg.Bits.byp_wait = 0;
    else if (clock == BCM_CPU_CLK_LOW)
        ctrl_reg.Bits.byp_wait = 1;
    else
        ret = -1;

    ret = WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets), ctrl_reg.Reg32);

    return ret;
}
#endif

#ifndef _CFE_
#if defined(CONFIG_BCM963158)
#if (CONFIG_BRCM_CHIP_REV==0x63158A0)
int pll_sar_set_divider(void)
{
   int ret = 0;
   /* For A0 version, it is set to MAX through pll.
   ** For B0, the default is slightly less than the divider here but qualified
   ** as a more stable number for production version.
   */
   PLL_CHCFG_REG pll_ch23_cfg;
   if(ReadBPCMRegister(PMB_ADDR_SYNC_PLL, PLLBPCMRegOffset(ch23_cfg), &pll_ch23_cfg.Reg32))
      return -1;
   printk("%s:Updating the sar pll %x\n",__FUNCTION__,pll_ch23_cfg.Reg32);
   pll_ch23_cfg.Bits.mdiv0 = 7;  /* SAR PLL is from 3GHz, divider of 7 will bring it to ~429 MHz */
   ret = WriteBPCMRegister(PMB_ADDR_SYNC_PLL, PLLBPCMRegOffset(ch23_cfg), pll_ch23_cfg.Reg32);
   if(ret != 0)
      return ret;
   mdelay(1);
   pll_ch23_cfg.Bits.mdiv_override0 = 1;
   printk("%s:Updating the sar pll %x\n",__FUNCTION__,pll_ch23_cfg.Reg32);
   ret = WriteBPCMRegister(PMB_ADDR_SYNC_PLL, PLLBPCMRegOffset(ch23_cfg), pll_ch23_cfg.Reg32);
   mdelay(1);
    return ret;
}
EXPORT_SYMBOL(pll_sar_set_divider);
#endif /* 158A0 */
#endif /* 158 */
#endif /* cfe */

#if defined(CONFIG_BCM963158)
void clk_divide_50mhz_to_25mhz(void)
{
    uint32 data;
    int ret;

    ret = ReadBPCMRegister(PMB_ADDR_CHIP_CLKRST, CLKRSTBPCMRegOffset(xtal_control), &data);
    if (ret)
    {
        printk("Failed to ReadBPCMRegister CHIP_CLKRST block CLKRST_XTAL_CNTL. Error=%d\n", ret);
        return;
    }

    /* Divide clock by 2. From 50mhz to 25mhz */
    data |= (0x1<<24);

    ret = WriteBPCMRegister(PMB_ADDR_CHIP_CLKRST, CLKRSTBPCMRegOffset(xtal_control), data);
    if (ret)
        printk("Failed to writeBPCMRegister CHIP_CLKRST block CLKRST_XTAL_CNTL. Error=%d\n", ret);
}
#ifndef _CFE_
EXPORT_SYMBOL(clk_divide_50mhz_to_25mhz);
#endif
#endif

#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM96878)
#if defined(CONFIG_BCM96878)
#define PMD_CLOCK_REG pmd_xtal_cntl
#define CLOCK_RESET_XTAL_CONTROL_BIT_PD_DRV (2)
#define CLOCK_RESET_XTAL_CONTROL_BIT_PWRON  (27)
#define PMD_CLOCK_REG2 pmd_xtal_cntl2
#define CLOCK_RESET_XTAL_CONTROL2_BIT_PD    (0)
#else
#define PMD_CLOCK_REG xtal_control
#define CLOCK_RESET_XTAL_CONTROL_BIT_PD_DRV (17)
#endif

void disable_25mhz_clk_to_pmd(void)
{
    uint32 data;
    int ret;

    ret = ReadBPCMRegister(PMB_ADDR_CHIP_CLKRST, CLKRSTBPCMRegOffset(PMD_CLOCK_REG), &data);
    if (ret)
    {
        printk("Failed to ReadBPCMRegister CHIP_CLKRST block CLKRST_XTAL_CNTL. Error=%d\n", ret);
        return;
    }

    data |=  (0x1 << CLOCK_RESET_XTAL_CONTROL_BIT_PD_DRV);
#if defined(CONFIG_BCM96878)
    data &= ~(0x1 << CLOCK_RESET_XTAL_CONTROL_BIT_PWRON);
#endif

    ret = WriteBPCMRegister(PMB_ADDR_CHIP_CLKRST, CLKRSTBPCMRegOffset(PMD_CLOCK_REG), data);
    if (ret)
        printk("Failed to writeBPCMRegister CHIP_CLKRST block CLKRST_XTAL_CNTL. Error=%d\n", ret);

#if defined(CONFIG_BCM96878)
    ret = ReadBPCMRegister(PMB_ADDR_CHIP_CLKRST, CLKRSTBPCMRegOffset(PMD_CLOCK_REG2), &data);
    if (ret)
    {
        printk("Failed to ReadBPCMRegister CHIP_CLKRST block PMD_XTAL_CNTL2. Error=%d\n", ret);
        return;
    }

    data |=  (0x1 << CLOCK_RESET_XTAL_CONTROL2_BIT_PD);

    ret = WriteBPCMRegister(PMB_ADDR_CHIP_CLKRST, CLKRSTBPCMRegOffset(PMD_CLOCK_REG2), data);
    if (ret)
        printk("Failed to writeBPCMRegister CHIP_CLKRST block CLKRST_XTAL_CNTL2. Error=%d\n", ret);

#endif
}
#ifndef _CFE_
EXPORT_SYMBOL(disable_25mhz_clk_to_pmd);
#endif
#endif

#ifndef _CFE_
EXPORT_SYMBOL(get_rdp_freq);
#endif

void set_vreg_clk(void)
{
#if defined(_BCM963178_) || defined(CONFIG_BCM963178)
    int ret;
    BPCM_CLKRST_VREG_CONTROL vreg_control_reg;

    vreg_control_reg.Bits.enable=1;
    vreg_control_reg.Bits.counter=0x24;
    ret = WriteBPCMRegister(PMB_ADDR_CHIP_CLKRST, CLKRSTBPCMRegOffset(vreg_control), vreg_control_reg.Reg32);
    if (ret)
        printk("Failed to writeBPCMRegister CHIP_CLKRST block VREG_CONTROL. Error=%d\n", ret);
#endif

}


#ifndef _CFE_
EXPORT_SYMBOL(set_vreg_clk);
#endif
