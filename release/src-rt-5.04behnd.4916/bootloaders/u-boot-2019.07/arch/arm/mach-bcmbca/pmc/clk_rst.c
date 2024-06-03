// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */
/*
 
*/

#include "pmc_drv.h"
#include "clk_rst.h"
#include "asm/arch/BPCM.h"

#define VCO0_FREQ	1200
#define VCO2_FREQ	1600

#if IS_BCMCHIP(6765)
#define PLL_REFCLK	80
#else
#define PLL_REFCLK	50
#endif

#if IS_BCMCHIP(6855)
int pll_vco_config(unsigned int pll_addr, unsigned int ndivider, unsigned int pdivider)
{
	int ret = 0;
	PLL_CTRL_REG pll_ctrl;
	PLL_STAT_REG pll_stat;
	PLL_NDIV_REG pll_ndiv;
	PLL_PDIV_REG pll_pdiv;

	//  reset pll
	ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(resets), &pll_ctrl.Reg32);
	pll_ctrl.Bits.resetb = 0;
	pll_ctrl.Bits.post_resetb = 0;
	WriteBPCMRegister(pll_addr, PLLBPCMRegOffset(resets), pll_ctrl.Reg32);

	// change ndiv and pdiv
	ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(pdiv), &pll_pdiv.Reg32);
	pll_pdiv.Bits.pdiv = pdivider;
	pll_pdiv.Bits.ndiv_pdiv_override = 1;
	ret |= WriteBPCMRegister(pll_addr, PLLBPCMRegOffset(pdiv), pll_pdiv.Reg32);

	ret = ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ndiv), &pll_ndiv.Reg32);
	pll_ndiv.Bits.ndiv_int = ndivider;
	pll_ndiv.Bits.ndiv_override = 1;
	ret |= WriteBPCMRegister(pll_addr, PLLBPCMRegOffset(ndiv), pll_ndiv.Reg32);

	// release restb
	ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(resets), &pll_ctrl.Reg32);
	pll_ctrl.Bits.resetb = 1;
	WriteBPCMRegister(pll_addr, PLLBPCMRegOffset(resets), pll_ctrl.Reg32);

	// wait untill pll is locked
	do {
		ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(stat), &pll_stat.Reg32);
	} while (pll_stat.Bits.lock == 0);

	// release post_resetb
	ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(resets), &pll_ctrl.Reg32);
	pll_ctrl.Bits.post_resetb = 1;
	WriteBPCMRegister(pll_addr, PLLBPCMRegOffset(resets), pll_ctrl.Reg32);

	return ret;
}
#endif


#if IS_BCMCHIP(6856)
#define PLL_BPCM_REG_OFFSET PLLCLASSICBPCMRegOffset
#else
#define PLL_BPCM_REG_OFFSET PLLBPCMRegOffset
#endif

#if IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813) || IS_BCMCHIP(6888) || IS_BCMCHIP(68880) || \
    IS_BCMCHIP(6837)
int rdppll_vco_freq_get(unsigned int pll_addr, unsigned int *fvco)
{
	int ret = 0;
	PLL_DECNDIV_REG pll_decndiv;
	RDPPLL_DECPDIV_REG pll_decpdiv;
	PLL_NDIV_REG ndiv_reg;
	PLL_PDIV_REG pdiv_reg;

	ret =
	    ReadBPCMRegister(pll_addr, RDPPLLBPCMRegOffset(decndiv),
			     &pll_decndiv.Reg32);
	ret |=
	    ReadBPCMRegister(pll_addr, RDPPLLBPCMRegOffset(decpdiv),
			     &pll_decpdiv.Reg32);
	if (ret != 0)
		return -1;

	// Let's ignore ndiv_frac, it is set to zero anyway by HW.
	*fvco =
	    (PLL_REFCLK * (pll_decndiv.Bits.ndiv_int)) / pll_decpdiv.Bits.pdiv;

	if (!ReadBPCMRegister(pll_addr, RDPPLLBPCMRegOffset(pdiv), &pdiv_reg.Reg32)
	    && pdiv_reg.Bits.ndiv_pdiv_override
	    && !ReadBPCMRegister(pll_addr, RDPPLLBPCMRegOffset(ndiv),
							 &ndiv_reg.Reg32)) {
		*fvco =
		    (PLL_REFCLK * (ndiv_reg.Bits.ndiv_int)) /
		    pdiv_reg.Bits.pdiv;
	}
	return 0;
}

int rdppll_ch_freq_get(unsigned int pll_addr, unsigned int ch, unsigned int *freq)
{
	int ret;
	unsigned int fvco, mdiv;
	RDPPLL_DECPDIV_REG pll_decpdiv;
	RDPPLL_DECCH25_REG pll_decch25;
	PLL_CHCFG_REG ch_cfg;

	ret = rdppll_vco_freq_get(pll_addr, &fvco);

	if (ret != 0)
		return -1;

	// The pll may include up to 6 channels.
	switch (ch) {
	case 0:
		ret =
		    ReadBPCMRegister(pll_addr,
				     RDPPLLBPCMRegOffset(ch01_cfg),
				     &ch_cfg.Reg32);
		/* Check if default value is overitten */
		if (ch_cfg.Bits.mdiv_override0)
			mdiv = ch_cfg.Bits.mdiv0;	/* Use the new value */
		else {		/* If not, read from the default */
			ret =
			    ReadBPCMRegister(pll_addr,
					     RDPPLLBPCMRegOffset(decpdiv),
					     &pll_decpdiv.Reg32);
			mdiv = pll_decpdiv.Bits.mdiv0;
		}
		break;
	case 1:
		ret =
		    ReadBPCMRegister(pll_addr,
				     RDPPLLBPCMRegOffset(ch01_cfg),
				     &ch_cfg.Reg32);
		if (ch_cfg.Bits.mdiv_override1)
			mdiv = ch_cfg.Bits.mdiv1;
		else {
			ret =
			    ReadBPCMRegister(pll_addr,
					     RDPPLLBPCMRegOffset(decpdiv),
					     &pll_decpdiv.Reg32);
			mdiv = pll_decpdiv.Bits.mdiv1;
		}
		break;
	case 2:
		ret =
		    ReadBPCMRegister(pll_addr,
				     RDPPLLBPCMRegOffset(ch23_cfg),
				     &ch_cfg.Reg32);
		if (ch_cfg.Bits.mdiv_override0)
			mdiv = ch_cfg.Bits.mdiv0;
		else {
			ret =
			    ReadBPCMRegister(pll_addr,
					     RDPPLLBPCMRegOffset(decch25),
					     &pll_decch25.Reg32);
			mdiv = pll_decch25.Bits.mdiv2;
		}
		break;
	case 3:
		ret =
		    ReadBPCMRegister(pll_addr,
				     RDPPLLBPCMRegOffset(ch23_cfg),
				     &ch_cfg.Reg32);
		if (ch_cfg.Bits.mdiv_override1)
			mdiv = ch_cfg.Bits.mdiv1;
		else {
			ret =
			    ReadBPCMRegister(pll_addr,
					     RDPPLLBPCMRegOffset(decch25),
					     &pll_decch25.Reg32);
			mdiv = pll_decch25.Bits.mdiv3;
		}
		break;
	case 4:
		ret =
		    ReadBPCMRegister(pll_addr,
				     RDPPLLBPCMRegOffset(ch45_cfg),
				     &ch_cfg.Reg32);
		if (ch_cfg.Bits.mdiv_override0)
			mdiv = ch_cfg.Bits.mdiv0;
		else {
			ret =
			    ReadBPCMRegister(pll_addr,
					     RDPPLLBPCMRegOffset(decch25),
					     &pll_decch25.Reg32);
			mdiv = pll_decch25.Bits.mdiv4;
		}
		break;
	case 5:
		ret =
		    ReadBPCMRegister(pll_addr,
				     RDPPLLBPCMRegOffset(ch45_cfg),
				     &ch_cfg.Reg32);
		if (ch_cfg.Bits.mdiv_override1)
			mdiv = ch_cfg.Bits.mdiv1;
		else {
			ret =
			    ReadBPCMRegister(pll_addr,
					     RDPPLLBPCMRegOffset(decpdiv),
					     &pll_decpdiv.Reg32);
			mdiv = pll_decpdiv.Bits.mdiv5;					  
		}
		break;
	default:
		return -1;
	};

	if (ret != 0)
		return -1;

	*freq = fvco / mdiv;

	return 0;
}
#endif

#if IS_BCMCHIP(6858) || IS_BCMCHIP(63158) || IS_BCMCHIP(6856) || \
	IS_BCMCHIP(6846) || IS_BCMCHIP(63178) || IS_BCMCHIP(47622) || \
	IS_BCMCHIP(6878) || IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || \
	IS_BCMCHIP(6855) || IS_BCMCHIP(6756)  || IS_BCMCHIP(6813) || IS_BCMCHIP(6888) || \
    IS_BCMCHIP(6837) || IS_BCMCHIP(68880) || IS_BCMCHIP(6765) || IS_BCMCHIP(6766)
int pll_vco_freq_get(unsigned int pll_addr, unsigned int *fvco)
{

	int ret = 0;
#if IS_BCMCHIP(6837) || IS_BCMCHIP(68880)
	PLL_PDIV_REG pdiv;
	PLL_NDIV_REG ndiv;

    ret = ReadBPCMRegister(pll_addr, ORIONPLLBPCMRegOffset(pdiv), &pdiv.Reg32);
    ret = ReadBPCMRegister(pll_addr, ORIONPLLBPCMRegOffset(ndiv), &ndiv.Reg32);

    if (ret != 0)
		return -1;
    if (pdiv.Bits.ndiv_pdiv_override)
    {
        if (pdiv.Bits.pdiv)
            *fvco = (PLL_REFCLK/pdiv.Bits.pdiv) * (ndiv.Bits.ndiv_int + (ndiv.Bits.ndiv_frac / 0x100000));
        else
            *fvco = (PLL_REFCLK) * ((ndiv.Bits.ndiv_int + (ndiv.Bits.ndiv_frac / 0x100000)) / 2);
    }
    else
        *fvco = (PLL_REFCLK) * (ndiv.Bits.ndiv_int + (ndiv.Bits.ndiv_frac / 0x100000));
#else
	PLL_DECNDIV_REG pll_decndiv;
	PLL_DECPDIV_REG pll_decpdiv;
#if IS_BCMCHIP(63158) || IS_BCMCHIP(63178) || IS_BCMCHIP(47622) || \
	IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6855) || \
	IS_BCMCHIP(6756)  || IS_BCMCHIP(6813) || IS_BCMCHIP(6888) || \
	IS_BCMCHIP(6765) || IS_BCMCHIP(6766)

	PLL_NDIV_REG ndiv_reg;
	PLL_PDIV_REG pdiv_reg;
#endif

	ret =
	    ReadBPCMRegister(pll_addr, PLL_BPCM_REG_OFFSET(decndiv),
			     &pll_decndiv.Reg32);
	ret |=
	    ReadBPCMRegister(pll_addr, PLL_BPCM_REG_OFFSET(decpdiv),
			     &pll_decpdiv.Reg32);
	if (ret != 0)
		return -1;

	// Let's ignore ndiv_frac, it is set to zero anyway by HW.
	*fvco =
	    (PLL_REFCLK * (pll_decndiv.Bits.ndiv_int)) / pll_decpdiv.Bits.pdiv;

#if IS_BCMCHIP(63158) || IS_BCMCHIP(63178) || IS_BCMCHIP(47622) || \
	IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6855) || \
	IS_BCMCHIP(6756)  || IS_BCMCHIP(6813) || IS_BCMCHIP(6888) || \
	IS_BCMCHIP(6765) || IS_BCMCHIP(6766)
	if (!ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(pdiv), &pdiv_reg.Reg32)
	    && pdiv_reg.Bits.ndiv_pdiv_override
	    && !ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ndiv),
				 &ndiv_reg.Reg32))
		*fvco =
		    (PLL_REFCLK * (ndiv_reg.Bits.ndiv_int)) /
		    pdiv_reg.Bits.pdiv;
#endif
#endif

	return 0;
}

#if IS_BCMCHIP(6837) || IS_BCMCHIP(68880)
int pll_4phase_ch_freq_get(unsigned int pll_addr, unsigned int ch, unsigned int *freq)
{
	int ret;
	unsigned int fvco;
    unsigned int mdiv2, rate;
    float mdiv1;
    PLL_ORION_CHCFG_REG ch_cfg;
    PLL_ORION_4PHASE_CFG0_REG phase_cfg0;


	ret = pll_vco_freq_get(pll_addr, &fvco);
	if (ret != 0)
		return -1;

    if ((ch == 1) || (ch == 2))
    {
        ret = ReadBPCMRegister(pll_addr, ORIONPLLBPCMRegOffset(ch12_cfg), &ch_cfg.Reg32);
        if (ch == 1)
        {
            mdiv1 = 1 + (0.25 * ch_cfg.Bits.mdiv1_ch1);
            mdiv2 = (1 << ch_cfg.Bits.mdiv2_ch1);
        }
        else
        {
            mdiv1 = 1 + (0.25 * ch_cfg.Bits.mdiv1_ch2);
            mdiv2 = (1 << ch_cfg.Bits.mdiv2_ch2);
        }

        *freq = fvco / (2 * mdiv1 * mdiv2);
    }
    else if ((ch == 0) || (ch == 90) || (ch == 180) ||(ch == 270))
    {
        ret = ReadBPCMRegister(pll_addr, ORIONPLLBPCMRegOffset(phase_cfg0), &phase_cfg0.Reg32);
        if (ch == 0)
        {
            mdiv2 = phase_cfg0.Bits.ph0_cfg_mdiv2 + 1;
            rate =  phase_cfg0.Bits.ph0_cfg_rate;
        }
        else if (ch == 90)
        {
            mdiv2 = phase_cfg0.Bits.ph90_cfg_mdiv2 + 1;
            rate =  phase_cfg0.Bits.ph90_cfg_rate;
        }
        else if (ch == 180)
        {
            mdiv2 = phase_cfg0.Bits.ph180_cfg_mdiv2 + 1;
            rate =  phase_cfg0.Bits.ph180_cfg_rate;
        }
        else
        {
            mdiv2 = phase_cfg0.Bits.ph270_cfg_mdiv2 + 1;
            rate =  phase_cfg0.Bits.ph270_cfg_rate;
        }
        
        if (rate)
            *freq = (fvco * rate)/(64 * mdiv2);
        else
            *freq = fvco/2;
    }
    else
    {
        return -1;
    }
    
    return 0;
}
#endif

int pll_ch_freq_get(unsigned int pll_addr, unsigned int ch, unsigned int *freq)
{
	int ret;
	unsigned int fvco, mdiv;
	PLL_DECPDIV_REG pll_decpdiv;
	PLL_DECCH25_REG pll_decch25;
	PLL_CHCFG_REG ch_cfg;

#if IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813) || IS_BCMCHIP(6888) || IS_BCMCHIP(68880) ||\
    IS_BCMCHIP(6837)
	if (pll_addr == PMB_ADDR_RDPPLL)
		return rdppll_ch_freq_get(pll_addr, ch, freq);
#endif

	ret = pll_vco_freq_get(pll_addr, &fvco);

	if (ret != 0)
		return -1;

	// The pll may include up to 6 channels.
	switch (ch) {
	case 0:
		ret =
		    ReadBPCMRegister(pll_addr,
				     PLL_BPCM_REG_OFFSET(ch01_cfg),
				     &ch_cfg.Reg32);
		/* Check if default value is overitten */
		if (ch_cfg.Bits.mdiv_override0)
			mdiv = ch_cfg.Bits.mdiv0;	/* Use the new value */
		else {		/* If not, read from the default */
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLL_BPCM_REG_OFFSET(decpdiv),
					     &pll_decpdiv.Reg32);
			mdiv = pll_decpdiv.Bits.mdiv0;
		}
		break;
	case 1:
		ret =
		    ReadBPCMRegister(pll_addr,
				     PLL_BPCM_REG_OFFSET(ch01_cfg),
				     &ch_cfg.Reg32);
		if (ch_cfg.Bits.mdiv_override1)
			mdiv = ch_cfg.Bits.mdiv1;
		else {
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLL_BPCM_REG_OFFSET(decpdiv),
					     &pll_decpdiv.Reg32);
			mdiv = pll_decpdiv.Bits.mdiv1;
		}
		break;
	case 2:
		ret =
		    ReadBPCMRegister(pll_addr,
				     PLL_BPCM_REG_OFFSET(ch23_cfg),
				     &ch_cfg.Reg32);
		if (ch_cfg.Bits.mdiv_override0)
			mdiv = ch_cfg.Bits.mdiv0;
		else {
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLL_BPCM_REG_OFFSET(decch25),
					     &pll_decch25.Reg32);
			mdiv = pll_decch25.Bits.mdiv2;
		}
		break;
	case 3:
		ret =
		    ReadBPCMRegister(pll_addr,
				     PLL_BPCM_REG_OFFSET(ch23_cfg),
				     &ch_cfg.Reg32);
		if (ch_cfg.Bits.mdiv_override1)
			mdiv = ch_cfg.Bits.mdiv1;
		else {
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLL_BPCM_REG_OFFSET(decch25),
					     &pll_decch25.Reg32);
			mdiv = pll_decch25.Bits.mdiv3;
		}
		break;
	case 4:
		ret =
		    ReadBPCMRegister(pll_addr,
				     PLL_BPCM_REG_OFFSET(ch45_cfg),
				     &ch_cfg.Reg32);
		if (ch_cfg.Bits.mdiv_override0)
			mdiv = ch_cfg.Bits.mdiv0;
		else {
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLL_BPCM_REG_OFFSET(decch25),
					     &pll_decch25.Reg32);
			mdiv = pll_decch25.Bits.mdiv4;
		}
		break;
	case 5:
		ret =
		    ReadBPCMRegister(pll_addr,
				     PLL_BPCM_REG_OFFSET(ch45_cfg),
				     &ch_cfg.Reg32);
		if (ch_cfg.Bits.mdiv_override1)
			mdiv = ch_cfg.Bits.mdiv1;
		else {
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLL_BPCM_REG_OFFSET(decch25),
					     &pll_decch25.Reg32);
			mdiv = pll_decch25.Bits.mdiv5;			
		}
		break;
	default:
		return -1;
	};

	if (ret != 0)
		return -1;

	*freq = fvco / mdiv;

	return 0;
}
#endif

int biu_ch_freq_get(unsigned int ch, unsigned int *freq)
{
#if IS_BCMCHIP(63138) || IS_BCMCHIP(63148) || IS_BCMCHIP(4908)
	printf("biu_ch_freq_get not supported!\n");
	*freq = 0;
	return -1;
#elif IS_BCMCHIP(68880) || IS_BCMCHIP(6837)
    return pll_4phase_ch_freq_get(PMB_ADDR_BIU_PLL, ch, freq);
#elif IS_BCMCHIP(6766)
	printf("biu_ch_freq_get not supported!\n");
	return -1;//brcm anand todo
#else
	return pll_ch_freq_get(PMB_ADDR_BIU_PLL, ch, freq);
#endif
}

unsigned long get_rdp_freq(unsigned int *rdp_freq)
{
	int ret = -1;
#if IS_BCMCHIP(6878)
	unsigned int fvco, mdiv;
	PLL_DECNDIV_REG pll_decndiv;
	PLL_DECPDIV_REG pll_decpdiv;

	ret =
	    ReadBPCMRegister(PMB_ADDR_SYSPLL, PLLCLASSICBPCMRegOffset(decndiv),
			     &pll_decndiv.Reg32);
	ret |=
	    ReadBPCMRegister(PMB_ADDR_SYSPLL, PLLCLASSICBPCMRegOffset(decpdiv),
			     &pll_decpdiv.Reg32);
	// Let's ignore ndiv_frac, it is set to zero anyway by HW.
	fvco =
	    (PLL_REFCLK * (pll_decndiv.Bits.ndiv_int)) / pll_decpdiv.Bits.pdiv;

	// read mdiv of channel 0
	ret =
	    ReadBPCMRegister(PMB_ADDR_SYSPLL, PLLCLASSICBPCMRegOffset(decpdiv),
			     &pll_decpdiv.Reg32);
	mdiv = pll_decpdiv.Bits.mdiv0;

	*rdp_freq = fvco / mdiv;


#elif IS_BCMCHIP(6858) || IS_BCMCHIP(63158) || IS_BCMCHIP(6856) || \
	IS_BCMCHIP(6846) || IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || \
	IS_BCMCHIP(6813) || IS_BCMCHIP(6888) || IS_BCMCHIP(68880) || \
    IS_BCMCHIP(6837)
	ret =
	    pll_ch_freq_get(PMB_ADDR_RDPPLL, XRDPPLL_RUNNER_CHANNEL, rdp_freq);
#if IS_BCMCHIP(6856) || IS_BCMCHIP(6846)
	*rdp_freq /= 2;
#endif
#elif IS_BCMCHIP(6855)

	ret = pll_ch_freq_get(PMB_ADDR_SYSPLL, XRDPPLL_RUNNER_CHANNEL, rdp_freq);

#elif IS_BCMCHIP(63138) || IS_BCMCHIP(63148) || IS_BCMCHIP(4908)
/* FIXME!! when knowing the real frequency info for 4908/62118 RDP */

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
 * F_clkout,0 = 700 MHz -> runner
 */

	PLL_NDIV_REG pll_ndiv;
	PLL_PDIV_REG pll_pdiv;
	PLL_CHCFG_REG pll_ch01_cfg;

	ret = ReadBPCMRegister(PMB_ADDR_RDPPLL, PLLBPCMRegOffset(ndiv),
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
#endif

	return ret;
}

#if IS_BCMCHIP(6858) || IS_BCMCHIP(6856) || IS_BCMCHIP(6846) || \
	IS_BCMCHIP(63158) || IS_BCMCHIP(63178) || IS_BCMCHIP(47622) || \
	IS_BCMCHIP(6878) || IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || \
	IS_BCMCHIP(6855) || IS_BCMCHIP(6756) || IS_BCMCHIP(6888) || \
	IS_BCMCHIP(6813) || IS_BCMCHIP(6765) || IS_BCMCHIP(6766)
int pll_ch_reset(unsigned int pll_addr, unsigned int ch, unsigned int pll_reg_offset)
{
	int ret;
	PLL_CHCFG_REG ch_cfg;

	// The pll may include up to 6 channels.
	switch (ch) {
	case 0:
	case 2:
	case 4:
		ret =
		    ReadBPCMRegister(pll_addr, pll_reg_offset,
				     &ch_cfg.Reg32);
		ch_cfg.Bits.mdiv0 = 0;
		ch_cfg.Bits.mdiv_override0 = 0;
		ret |=
		    WriteBPCMRegister(pll_addr, pll_reg_offset,
				      ch_cfg.Reg32);
		break;
	case 1:
	case 3:
	case 5:
		ret =
		    ReadBPCMRegister(pll_addr, pll_reg_offset,
				     &ch_cfg.Reg32);
		ch_cfg.Bits.mdiv1 = 0;
		ch_cfg.Bits.mdiv_override1 = 0;
		ret |=
		    WriteBPCMRegister(pll_addr, pll_reg_offset,
				      ch_cfg.Reg32);
		break;
	default:
		return -1;
	};

	return ret;
}

int pll_ch_freq_set_offs(unsigned int pll_addr, unsigned int ch, unsigned int mdiv, unsigned int offset)
{
	int ret;
	PLL_CHCFG_REG ch_cfg;

	// The pll may include up to 6 channels.
	switch (ch) {
	case 0:
	case 2:
	case 4:
		ret = ReadBPCMRegister(pll_addr, offset, &ch_cfg.Reg32);
		ch_cfg.Bits.mdiv0 = mdiv;
		ch_cfg.Bits.mdiv_override0 = 1;
		ret |= WriteBPCMRegister(pll_addr, offset, ch_cfg.Reg32);
		break;
	case 1:
	case 3:
	case 5:
		ret = ReadBPCMRegister(pll_addr, offset, &ch_cfg.Reg32);
		ch_cfg.Bits.mdiv1 = mdiv;
		ch_cfg.Bits.mdiv_override1 = 1;
		ret |= WriteBPCMRegister(pll_addr, offset, ch_cfg.Reg32);
		break;
	default:
		return -1;
	};
	return ret;
}

int pll_ch_freq_set(unsigned int pll_addr, unsigned int ch, unsigned int mdiv)
{
	int ret=0;

	// The pll may include up to 6 channels.
	switch (ch) {
	case 0:
	case 1:
		ret |= pll_ch_freq_set_offs( pll_addr, ch, mdiv, PLLBPCMRegOffset(ch01_cfg));
		break;
	case 2:
	case 3:
		ret |= pll_ch_freq_set_offs( pll_addr, ch, mdiv, PLLBPCMRegOffset(ch23_cfg));
		break;
	case 4:
	case 5:
		ret |= pll_ch_freq_set_offs( pll_addr, ch, mdiv, PLLBPCMRegOffset(ch45_cfg));
		break;
	default:
		return -1;
	};

	return ret;
}

int pll_ch_freq_vco_set(unsigned int pll_addr, unsigned int ch,
			unsigned int mdiv, unsigned int use_vco)
{
	int ret;
	PLL_CHCFG_REG ch_cfg;

	// The pll may include up to 6 channels.
	switch (ch) {
	case 0:
		ret =
		    ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch01_cfg),
				     &ch_cfg.Reg32);
		ch_cfg.Bits.mdiv0 = mdiv;
		ch_cfg.Bits.mdiv_override0 = 1;
		ch_cfg.Bits.reserved0 = use_vco ? 1 : 0;
		ret |=
		    WriteBPCMRegister(pll_addr, PLLBPCMRegOffset(ch01_cfg),
				      ch_cfg.Reg32);
		break;
	case 1:
		ret =
		    ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch01_cfg),
				     &ch_cfg.Reg32);
		ch_cfg.Bits.mdiv1 = mdiv;
		ch_cfg.Bits.mdiv_override1 = 1;
		ch_cfg.Bits.reserved1 = use_vco ? 1 : 0;
		ret |=
		    WriteBPCMRegister(pll_addr, PLLBPCMRegOffset(ch01_cfg),
				      ch_cfg.Reg32);
		break;
	case 2:
		ret =
		    ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch23_cfg),
				     &ch_cfg.Reg32);
		ch_cfg.Bits.mdiv0 = mdiv;
		ch_cfg.Bits.mdiv_override0 = 1;
		ch_cfg.Bits.reserved0 = use_vco ? 1 : 0;
		ret |=
		    WriteBPCMRegister(pll_addr, PLLBPCMRegOffset(ch23_cfg),
				      ch_cfg.Reg32);
		break;
	case 3:
		ret =
		    ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch23_cfg),
				     &ch_cfg.Reg32);
		ch_cfg.Bits.mdiv1 = mdiv;
		ch_cfg.Bits.mdiv_override1 = 1;
		ch_cfg.Bits.reserved1 = use_vco ? 1 : 0;
		ret |=
		    WriteBPCMRegister(pll_addr, PLLBPCMRegOffset(ch23_cfg),
				      ch_cfg.Reg32);
		break;
	case 4:
		ret =
		    ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch45_cfg),
				     &ch_cfg.Reg32);
		ch_cfg.Bits.mdiv0 = mdiv;
		ch_cfg.Bits.mdiv_override0 = 1;
		ch_cfg.Bits.reserved0 = use_vco ? 1 : 0;
		ret |=
		    WriteBPCMRegister(pll_addr, PLLBPCMRegOffset(ch45_cfg),
				      ch_cfg.Reg32);
		break;
	case 5:
		ret =
		    ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch45_cfg),
				     &ch_cfg.Reg32);
		ch_cfg.Bits.mdiv1 = mdiv;
		ch_cfg.Bits.mdiv_override1 = 1;
		ch_cfg.Bits.reserved1 = use_vco ? 1 : 0;
		ret |=
		    WriteBPCMRegister(pll_addr, PLLBPCMRegOffset(ch45_cfg),
				      ch_cfg.Reg32);
		break;
	default:
		return -1;
	};

	return ret;
}
#endif

#if IS_BCMCHIP(6858)
int bcm_change_cpu_clk(BCM_CPU_CLK clock)
{
	int ret = 0;
	PLL_CTRL_REG ctrl_reg;

	if (ReadBPCMRegister
	    (PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets), &ctrl_reg.Reg32))
		return -1;

	if (clock == BCM_CPU_CLK_HIGH)
		ctrl_reg.Bits.byp_wait = 0;
	else if (clock == BCM_CPU_CLK_LOW)
		ctrl_reg.Bits.byp_wait = 1;
	else
		ret = -1;

	ret = WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets),
				ctrl_reg.Reg32);

	return ret;
}
#endif

void set_vreg_clk(void)
{
#if IS_BCMCHIP(63178) || IS_BCMCHIP(63146)
	int ret;
	BPCM_CLKRST_VREG_CONTROL vreg_control_reg;

	vreg_control_reg.Bits.enable = 1;
	vreg_control_reg.Bits.counter = 0x24;
#if IS_BCMCHIP(63146)
	vreg_control_reg.Bits.counter = 0x32;
#endif
	ret = WriteBPCMRegister(PMB_ADDR_CHIP_CLKRST,
				CLKRSTBPCMRegOffset(vreg_control),
				vreg_control_reg.Reg32);
	if (ret)
		printk("Failed to writeBPCMRegister CHIP_CLKRST block "
		       "VREG_CONTROL. Error=%d\n", ret);
#endif
}
