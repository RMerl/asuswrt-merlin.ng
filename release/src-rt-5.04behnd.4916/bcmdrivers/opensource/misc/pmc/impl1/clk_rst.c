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
#include "pmc_wan.h" //for pmc_wan_init
#include "clk_rst.h"
#include "BPCM.h"

#define VCO0_FREQ	1200
#define VCO2_FREQ	1600

#if IS_BCMCHIP(6765) || IS_BCMCHIP(6766) || IS_BCMCHIP(6764)
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
#define PLL_BPCM_REG_OFFSET(pll_addr, reg) PLLCLASSICBPCMRegOffset(reg)
#elif IS_BCMCHIP(6846)
#define PLL_BPCM_REG_OFFSET(pll_addr, reg) (pll_addr==PMB_ADDR_BIU_PLL) ? PLLCLASSICBPCMRegOffset(reg) : PLLBPCMRegOffset(reg)
#elif IS_BCMCHIP(6878)
#define PLL_BPCM_REG_OFFSET(pll_addr, reg) (pll_addr==PMB_ADDR_BIU_PLL) ? PLLBPCMRegOffset(reg) : PLLCLASSICBPCMRegOffset(reg)
#else
#define PLL_BPCM_REG_OFFSET(pll_addr, reg) PLLBPCMRegOffset(reg)
#endif

#if IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813) || IS_BCMCHIP(6888)
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
			
int pll_vco_freq_get(unsigned int pll_addr, unsigned int *fvco)
{
	int ret = 0;
	PLL_DECNDIV_REG pll_decndiv;
	PLL_DECPDIV_REG pll_decpdiv;
#if !IS_BCMCHIP(6858) && !IS_BCMCHIP(6856)
	PLL_NDIV_REG ndiv_reg;
	PLL_PDIV_REG pdiv_reg;
#endif

	ret =
	    ReadBPCMRegister(pll_addr, PLL_BPCM_REG_OFFSET(pll_addr, decndiv),
			     &pll_decndiv.Reg32);
	ret |=
	    ReadBPCMRegister(pll_addr, PLL_BPCM_REG_OFFSET(pll_addr, decpdiv),
			     &pll_decpdiv.Reg32);
	if (ret != 0)
		return -1;

	// Let's ignore ndiv_frac, it is set to zero anyway by HW.
	*fvco =
	    (PLL_REFCLK * (pll_decndiv.Bits.ndiv_int)) / pll_decpdiv.Bits.pdiv;

#if !IS_BCMCHIP(6858) && !IS_BCMCHIP(6856)
	if (!ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(pdiv), &pdiv_reg.Reg32)
	    && pdiv_reg.Bits.ndiv_pdiv_override
	    && !ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ndiv),
				 &ndiv_reg.Reg32))
		*fvco =
		    (PLL_REFCLK * (ndiv_reg.Bits.ndiv_int)) /
		    pdiv_reg.Bits.pdiv;
#endif

	return 0;
}

static int pll_ch_freq_get(unsigned int pll_addr, unsigned int ch, unsigned int *freq)
{
	int ret;
	unsigned int fvco, mdiv;
	PLL_DECPDIV_REG pll_decpdiv;
	PLL_DECCH25_REG pll_decch25;
	PLL_CHCFG_REG ch_cfg;

#if IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813) || IS_BCMCHIP(6888)
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
				     PLL_BPCM_REG_OFFSET(pll_addr, ch01_cfg),
				     &ch_cfg.Reg32);
		/* Check if default value is overitten */
		if (ch_cfg.Bits.mdiv_override0)
			mdiv = ch_cfg.Bits.mdiv0;	/* Use the new value */
		else {		/* If not, read from the default */
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLL_BPCM_REG_OFFSET(pll_addr, decpdiv),
					     &pll_decpdiv.Reg32);
			mdiv = pll_decpdiv.Bits.mdiv0;
		}
		break;
	case 1:
		ret =
		    ReadBPCMRegister(pll_addr,
				     PLL_BPCM_REG_OFFSET(pll_addr, ch01_cfg),
				     &ch_cfg.Reg32);
		if (ch_cfg.Bits.mdiv_override1)
			mdiv = ch_cfg.Bits.mdiv1;
		else {
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLL_BPCM_REG_OFFSET(pll_addr, decpdiv),
					     &pll_decpdiv.Reg32);
			mdiv = pll_decpdiv.Bits.mdiv1;
		}
		break;
	case 2:
		ret =
		    ReadBPCMRegister(pll_addr,
				     PLL_BPCM_REG_OFFSET(pll_addr, ch23_cfg),
				     &ch_cfg.Reg32);
		if (ch_cfg.Bits.mdiv_override0)
			mdiv = ch_cfg.Bits.mdiv0;
		else {
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLL_BPCM_REG_OFFSET(pll_addr, decch25),
					     &pll_decch25.Reg32);
			mdiv = pll_decch25.Bits.mdiv2;
		}
		break;
	case 3:
		ret =
		    ReadBPCMRegister(pll_addr,
				     PLL_BPCM_REG_OFFSET(pll_addr, ch23_cfg),
				     &ch_cfg.Reg32);
		if (ch_cfg.Bits.mdiv_override1)
			mdiv = ch_cfg.Bits.mdiv1;
		else {
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLL_BPCM_REG_OFFSET(pll_addr, decch25),
					     &pll_decch25.Reg32);
			mdiv = pll_decch25.Bits.mdiv3;
		}
		break;
	case 4:
		ret =
		    ReadBPCMRegister(pll_addr,
				     PLL_BPCM_REG_OFFSET(pll_addr, ch45_cfg),
				     &ch_cfg.Reg32);
		if (ch_cfg.Bits.mdiv_override0)
			mdiv = ch_cfg.Bits.mdiv0;
		else {
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLL_BPCM_REG_OFFSET(pll_addr, decch25),
					     &pll_decch25.Reg32);
			mdiv = pll_decch25.Bits.mdiv4;
		}
		break;
	case 5:
		ret =
		    ReadBPCMRegister(pll_addr,
				     PLL_BPCM_REG_OFFSET(pll_addr, ch45_cfg),
				     &ch_cfg.Reg32);
		if (ch_cfg.Bits.mdiv_override1)
			mdiv = ch_cfg.Bits.mdiv1;
		else {
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLL_BPCM_REG_OFFSET(pll_addr, decch25),
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

int biu_ch_freq_get(unsigned int ch, unsigned int *freq)
{
    return pll_ch_freq_get(PMB_ADDR_BIU_PLL, ch, freq);
}
EXPORT_SYMBOL(biu_ch_freq_get);

unsigned long get_rdp_freq(unsigned int *rdp_freq)
{
	int ret = -1;
#if IS_BCMCHIP(6858) || IS_BCMCHIP(63158) || IS_BCMCHIP(6856) || \
	IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813) || IS_BCMCHIP(6888)
	ret =
	    pll_ch_freq_get(PMB_ADDR_RDPPLL, XRDPPLL_RUNNER_CHANNEL, rdp_freq);
#if IS_BCMCHIP(6856)
	*rdp_freq /= 2;
#endif

#elif IS_BCMCHIP(6855)
	ret = pll_ch_freq_get(PMB_ADDR_SYSPLL, XRDPPLL_RUNNER_CHANNEL, rdp_freq);
#endif

	return ret;
}
EXPORT_SYMBOL(get_rdp_freq);

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

#if IS_BCMCHIP(6858)
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

#if defined(__KERNEL__) && IS_BCMCHIP(63158)
void clk_divide_50mhz_to_25mhz(void)
{
	uint32_t data;
	int ret;

	ret = ReadBPCMRegister(PMB_ADDR_CHIP_CLKRST,
			       CLKRSTBPCMRegOffset(xtal_control), &data);
	if (ret) {
		printk("Failed to ReadBPCMRegister CHIP_CLKRST block "
		       "CLKRST_XTAL_CNTL. Error=%d\n", ret);
		return;
	}

	/* Divide clock by 2. From 50mhz to 25mhz */
	data |= (0x1 << 24);

	ret =
	    WriteBPCMRegister(PMB_ADDR_CHIP_CLKRST,
			      CLKRSTBPCMRegOffset(xtal_control), data);
	if (ret)
		printk("Failed to writeBPCMRegister CHIP_CLKRST block "
		       "CLKRST_XTAL_CNTL. Error=%d\n", ret);
}
EXPORT_SYMBOL(clk_divide_50mhz_to_25mhz);
#endif

#if IS_BCMCHIP(6858) || IS_BCMCHIP(6856) || IS_BCMCHIP(63158)\
	|| IS_BCMCHIP(4912) || IS_BCMCHIP(6813) || IS_BCMCHIP(63146)
#define PMD_CLOCK_REG xtal_control
#if IS_BCMCHIP(4912) || IS_BCMCHIP(6813) || IS_BCMCHIP(63146)
#define CLOCK_RESET_XTAL_CONTROL_BIT_PD_DRV (20)
#else
#define CLOCK_RESET_XTAL_CONTROL_BIT_PD_DRV (17)
#endif
int bcm_enable_xtal_clk(void)
{
	uint32_t data;
	int ret = 0;

	ret = ReadBPCMRegister(PMB_ADDR_CHIP_CLKRST,
			       CLKRSTBPCMRegOffset(PMD_CLOCK_REG), &data);
#if IS_BCMCHIP(4912) || IS_BCMCHIP(6813) || IS_BCMCHIP(63146)
	data |= (0x1 << CLOCK_RESET_XTAL_CONTROL_BIT_PD_DRV);
#else	
	data &= ~(0x1 << CLOCK_RESET_XTAL_CONTROL_BIT_PD_DRV);
#endif
	ret |= WriteBPCMRegister(PMB_ADDR_CHIP_CLKRST,
				CLKRSTBPCMRegOffset(PMD_CLOCK_REG), data);


	if (ret)
		printk("Failed to enable xtal clk\n");

	return ret;
}
EXPORT_SYMBOL(bcm_enable_xtal_clk);

int bcm_disable_xtal_clk(void)
{
	uint32_t data;
	int ret = 0;

	ret = ReadBPCMRegister(PMB_ADDR_CHIP_CLKRST,
			       CLKRSTBPCMRegOffset(PMD_CLOCK_REG), &data);
	/* Only 4912 and 63146 need disable function */
#if IS_BCMCHIP(4912) || IS_BCMCHIP(6813) || IS_BCMCHIP(63146)
	data &= ~(0x1 << CLOCK_RESET_XTAL_CONTROL_BIT_PD_DRV);
#endif
	ret |= WriteBPCMRegister(PMB_ADDR_CHIP_CLKRST,
				CLKRSTBPCMRegOffset(PMD_CLOCK_REG), data);

	if (ret)
		printk("Failed to disable xtal clk\n");

	return ret;
}
EXPORT_SYMBOL(bcm_disable_xtal_clk); 
#endif

void set_vreg_clk(void)
{
#if IS_BCMCHIP(63178) || IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813) || IS_BCMCHIP(6765) || IS_BCMCHIP(6766)
	int ret;
	BPCM_CLKRST_VREG_CONTROL vreg_control_reg;

	vreg_control_reg.Bits.enable = 1;
	vreg_control_reg.Bits.counter = 0x24;
#if IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813)
	vreg_control_reg.Bits.counter = 0x32;
#elif IS_BCMCHIP(6765) || IS_BCMCHIP(6766)
	vreg_control_reg.Bits.counter = 0x3c;
#endif
	ret = WriteBPCMRegister(PMB_ADDR_CHIP_CLKRST,
				CLKRSTBPCMRegOffset(vreg_control),
				vreg_control_reg.Reg32);
	if (ret)
		printk("Failed to writeBPCMRegister CHIP_CLKRST block "
		       "VREG_CONTROL. Error=%d\n", ret);
#endif
}

#if IS_BCMCHIP(6858)

#if IS_BCMCHIP(6858)
#define C0_CLK_CONTROL (0x70 >> 2)
#define C0_CLK_PATTERN (0x78 >> 2)
#endif

uint32_t get_cluster_clk_pattern(void)
{
    uint32_t pattern = 0;
#if IS_BCMCHIP(6858) 
	ReadBPCMRegister(PMB_ADDR_BIU_BPCM, C0_CLK_PATTERN, &pattern);
#endif
    return pattern;
}

void set_cluster_clk_pattern(uint32_t pattern)
{
#if IS_BCMCHIP(6858) 
	WriteBPCMRegister(PMB_ADDR_BIU_BPCM, C0_CLK_PATTERN, pattern);
#endif
}

void reset_cluster_clock(void)
{
#if IS_BCMCHIP(6858) 
	pmc_initmode();
	// cluster clock control: unreset
	WriteBPCMRegister(PMB_ADDR_BIU_BPCM, C0_CLK_CONTROL, 0x80000000);
	// cluster clock pattern: full-speed pattern
	WriteBPCMRegister(PMB_ADDR_BIU_BPCM, C0_CLK_PATTERN, ~0);
	// cluster clock control: enable pattern
	WriteBPCMRegister(PMB_ADDR_BIU_BPCM, C0_CLK_CONTROL, 0x80000010);
#endif
}
#endif

void bcm_set_vreg_sync(void)
{
#if !defined(CONFIG_BRCM_QEMU)
#if IS_BCMCHIP(63178) || IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813) || IS_BCMCHIP(6765) || IS_BCMCHIP(6766)
    set_vreg_clk();
#elif IS_BCMCHIP(63158)
    /* Power on WAN block to access the VR divider reg.  Must set this register before
       we can enable pinmux for vreg pin */
#define VREG_CFG_VREG_CLK_BYPASS_SHIFT 9
#define VREG_CFG_VREG_CLK_BYPASS_MASK (0x1 << VREG_CFG_VREG_CLK_BYPASS_SHIFT)
#define VREG_CFG_VREG_CLK_SRC_SHIFT 8
#define VREG_CFG_VREG_CLK_SRC_MASK (0x1 << VREG_CFG_VREG_CLK_SRC_SHIFT)
#define VREG_CFG_VREG_DIV_SHIFT 0
#define VREG_CFG_VREG_DIV_MASK (0xff << VREG_CFG_VREG_DIV_SHIFT)
#define WAN_VOLTAGE_REGULATOR_DIVIDER_OFFSET 0x801440c8
    uint32_t reg;
    void *wan_top_v_reg_div = ioremap(WAN_VOLTAGE_REGULATOR_DIVIDER_OFFSET, 4);

    pmc_wan_init();

    reg = readl(wan_top_v_reg_div);
    reg &= ~(VREG_CFG_VREG_CLK_BYPASS_MASK);
    writel(reg, wan_top_v_reg_div);

    reg = readl(wan_top_v_reg_div);
    reg &= ~(VREG_CFG_VREG_CLK_SRC_MASK);
    writel(reg, wan_top_v_reg_div);

    reg = readl(wan_top_v_reg_div);
    reg &= ~(VREG_CFG_VREG_DIV_MASK);
    reg |= 0x32;
    writel(reg, wan_top_v_reg_div);
    iounmap(wan_top_v_reg_div);
#endif
#endif
}
EXPORT_SYMBOL(bcm_set_vreg_sync);

void set_spike_mitigation(unsigned int spike_us)
{
#if !defined(PMC_ON_HOSTCPU) && !defined(PMC_DIRECT_MODE_ONLY)
	volatile Pmc *pmc = (volatile Pmc *)g_pmc->pmc_base;

	// tmrctl = enable | microseconds | spike_us 
	pmc->ctrl.gpTmr0Ctl = (1 << 31) | (1 << 29) | spike_us;
	while (pmc->ctrl.gpTmr0Ctl & (1 << 31));
#endif
}

#if IS_BCMCHIP(6858) || IS_BCMCHIP(63158) || IS_BCMCHIP(6856) || IS_BCMCHIP(6855) || IS_BCMCHIP(6878) || \
	IS_BCMCHIP(6846) || IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813) || IS_BCMCHIP(6888)
int pll_ch_mdiv_get(unsigned int pll_addr, unsigned int ch, unsigned int *pmdiv)
{
	int ret, mdiv;
	PLL_DECPDIV_REG pll_decpdiv;
	PLL_DECCH25_REG pll_decch25;
	PLL_CHCFG_REG ch_cfg;

	// The pll may include up to 6 channels.
	switch (ch) {
	case 0:
		ret =
		    ReadBPCMRegister(pll_addr,
				     PLL_BPCM_REG_OFFSET(pll_addr, ch01_cfg),
				     &ch_cfg.Reg32);
		/* Check if default value is overitten */
		if (ch_cfg.Bits.mdiv_override0)
			mdiv = ch_cfg.Bits.mdiv0;	/* Use the new value */
		else {		/* If not, read from the default */
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLL_BPCM_REG_OFFSET(pll_addr, decpdiv),
					     &pll_decpdiv.Reg32);
			mdiv = pll_decpdiv.Bits.mdiv0;
		}
		break;
	case 1:
		ret =
		    ReadBPCMRegister(pll_addr,
				     PLL_BPCM_REG_OFFSET(pll_addr, ch01_cfg),
				     &ch_cfg.Reg32);
		if (ch_cfg.Bits.mdiv_override1)
			mdiv = ch_cfg.Bits.mdiv1;
		else {
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLL_BPCM_REG_OFFSET(pll_addr, decpdiv),
					     &pll_decpdiv.Reg32);
			mdiv = pll_decpdiv.Bits.mdiv1;
		}
		break;
	case 2:
		ret =
		    ReadBPCMRegister(pll_addr,
				     PLL_BPCM_REG_OFFSET(pll_addr, ch23_cfg),
				     &ch_cfg.Reg32);
		if (ch_cfg.Bits.mdiv_override0)
			mdiv = ch_cfg.Bits.mdiv0;
		else {
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLL_BPCM_REG_OFFSET(pll_addr, decch25),
					     &pll_decch25.Reg32);
			mdiv = pll_decch25.Bits.mdiv2;
		}
		break;
	case 3:
		ret =
		    ReadBPCMRegister(pll_addr,
				     PLL_BPCM_REG_OFFSET(pll_addr, ch23_cfg),
				     &ch_cfg.Reg32);
		if (ch_cfg.Bits.mdiv_override1)
			mdiv = ch_cfg.Bits.mdiv1;
		else {
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLL_BPCM_REG_OFFSET(pll_addr, decch25),
					     &pll_decch25.Reg32);
			mdiv = pll_decch25.Bits.mdiv3;
		}
		break;
	case 4:
		ret =
		    ReadBPCMRegister(pll_addr,
				     PLL_BPCM_REG_OFFSET(pll_addr, ch45_cfg),
				     &ch_cfg.Reg32);
		if (ch_cfg.Bits.mdiv_override0)
			mdiv = ch_cfg.Bits.mdiv0;
		else {
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLL_BPCM_REG_OFFSET(pll_addr, decch25),
					     &pll_decch25.Reg32);
			mdiv = pll_decch25.Bits.mdiv4;
		}
		break;
	case 5:
		ret =
		    ReadBPCMRegister(pll_addr,
				     PLL_BPCM_REG_OFFSET(pll_addr, ch45_cfg),
				     &ch_cfg.Reg32);
		if (ch_cfg.Bits.mdiv_override1)
			mdiv = ch_cfg.Bits.mdiv1;
		else {
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLL_BPCM_REG_OFFSET(pll_addr, decch25),
					     &pll_decch25.Reg32);
			mdiv = pll_decch25.Bits.mdiv5;			
		}
		break;
	default:
		return -1;
	};

	if (ret != 0)
		return -1;

	*pmdiv = mdiv;

	return 0;
}

int pll_ch_mdiv_set(unsigned int pll_addr, unsigned int ch, unsigned int mdiv)
{
	int ret;
	PLL_CHCFG_REG ch_cfg;

	// The pll may include up to 6 channels.
	switch (ch) {
	case 0:
		ret =
		    ReadBPCMRegister(pll_addr, PLL_BPCM_REG_OFFSET(pll_addr, ch01_cfg),
				     &ch_cfg.Reg32);
		ch_cfg.Bits.mdiv0 = mdiv;
		ch_cfg.Bits.mdiv_override0 = 1;
		ret |=
		    WriteBPCMRegister(pll_addr, PLL_BPCM_REG_OFFSET(pll_addr, ch01_cfg),
				      ch_cfg.Reg32);
		break;
	case 1:
		ret =
		    ReadBPCMRegister(pll_addr, PLL_BPCM_REG_OFFSET(pll_addr, ch01_cfg),
				     &ch_cfg.Reg32);
		ch_cfg.Bits.mdiv1 = mdiv;
		ch_cfg.Bits.mdiv_override1 = 1;
		ret |=
		    WriteBPCMRegister(pll_addr, PLL_BPCM_REG_OFFSET(pll_addr, ch01_cfg),
				      ch_cfg.Reg32);
		break;
	case 2:
		ret =
		    ReadBPCMRegister(pll_addr, PLL_BPCM_REG_OFFSET(pll_addr, ch23_cfg),
				     &ch_cfg.Reg32);
		ch_cfg.Bits.mdiv0 = mdiv;
		ch_cfg.Bits.mdiv_override0 = 1;
		ret |=
		    WriteBPCMRegister(pll_addr, PLL_BPCM_REG_OFFSET(pll_addr, ch23_cfg),
				      ch_cfg.Reg32);
		break;
	case 3:
		ret =
		    ReadBPCMRegister(pll_addr, PLL_BPCM_REG_OFFSET(pll_addr, ch23_cfg),
				     &ch_cfg.Reg32);
		ch_cfg.Bits.mdiv1 = mdiv;
		ch_cfg.Bits.mdiv_override1 = 1;
		ret |=
		    WriteBPCMRegister(pll_addr, PLL_BPCM_REG_OFFSET(pll_addr, ch23_cfg),
				      ch_cfg.Reg32);
		break;
	case 4:
		ret =
		    ReadBPCMRegister(pll_addr, PLL_BPCM_REG_OFFSET(pll_addr, ch45_cfg),
				     &ch_cfg.Reg32);
		ch_cfg.Bits.mdiv0 = mdiv;
		ch_cfg.Bits.mdiv_override0 = 1;
		ret |=
		    WriteBPCMRegister(pll_addr, PLL_BPCM_REG_OFFSET(pll_addr, ch45_cfg),
				      ch_cfg.Reg32);
		break;
	case 5:
		ret =
		    ReadBPCMRegister(pll_addr, PLL_BPCM_REG_OFFSET(pll_addr, ch45_cfg),
				     &ch_cfg.Reg32);
		ch_cfg.Bits.mdiv1 = mdiv;
		ch_cfg.Bits.mdiv_override1 = 1;
		ret |=
		    WriteBPCMRegister(pll_addr, PLL_BPCM_REG_OFFSET(pll_addr, ch45_cfg),
				      ch_cfg.Reg32);
		break;
	default:
		return -1;
	};

	return ret;
}
#endif

static int _xrdp_div_get(unsigned int *div)
{
    int ret = -1;

#if IS_BCMCHIP(6858) || IS_BCMCHIP(63158) || IS_BCMCHIP(6856) || \
	IS_BCMCHIP(6846) || IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813) || IS_BCMCHIP(6888)
    ret = pll_ch_mdiv_get(PMB_ADDR_RDPPLL, XRDPPLL_RUNNER_CHANNEL, div);
#elif IS_BCMCHIP(6855)
    ret = pll_ch_mdiv_get(PMB_ADDR_SYSPLL, XRDPPLL_RUNNER_CHANNEL, div);
#elif IS_BCMCHIP(6878)
    ret = pll_ch_mdiv_get(PMB_ADDR_SYSPLL, SYSPLL_RUNNER_CHANNEL, div);
#endif

    return ret;
}

static int _xrdp_div_set(unsigned int div)
{
    int ret = -1;

#if IS_BCMCHIP(6858) || IS_BCMCHIP(63158) || IS_BCMCHIP(6856) || \
	IS_BCMCHIP(6846) || IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813) || IS_BCMCHIP(6888)
    ret = pll_ch_mdiv_set(PMB_ADDR_RDPPLL, XRDPPLL_RUNNER_CHANNEL, div);
#elif IS_BCMCHIP(6855)
    ret = pll_ch_mdiv_set(PMB_ADDR_SYSPLL, XRDPPLL_RUNNER_CHANNEL, div);
#elif IS_BCMCHIP(6878)
    ret = pll_ch_mdiv_set(PMB_ADDR_SYSPLL, SYSPLL_RUNNER_CHANNEL, div);
#endif

    return ret;
}

static unsigned int xrdp_divider;

int xrdp_div_get(int *enable)
{
    unsigned int _xrdp_divider = 0;

    if (!xrdp_divider)
        _xrdp_div_get(&xrdp_divider);

    if (_xrdp_div_get(&_xrdp_divider))
        return -1;

    *enable = (_xrdp_divider != xrdp_divider);

    return 0;
}
EXPORT_SYMBOL(xrdp_div_get);

int xrdp_div_set(int enable)
{
    unsigned int _xrdp_divider;

    if (!xrdp_divider)
        _xrdp_div_get(&xrdp_divider);

    _xrdp_divider = enable ? xrdp_divider * 4 : xrdp_divider;
    if (_xrdp_div_set(_xrdp_divider))
        return -1;

    return 0;
}
EXPORT_SYMBOL(xrdp_div_set);
