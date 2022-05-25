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
#include "pmc_wan.h" //for pmc_wan_power_up
#include "clk_rst.h"
#include "BPCM.h"

#define VCO0_FREQ	1200
#define VCO2_FREQ	1600

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


#if IS_BCMCHIP(6858) || IS_BCMCHIP(63158) || IS_BCMCHIP(6856) || \
	IS_BCMCHIP(6846) || IS_BCMCHIP(63178) || IS_BCMCHIP(47622) || \
	IS_BCMCHIP(6878) || IS_BCMCHIP(63146) || IS_BCMCHIP(6855) || \
	IS_BCMCHIP(4912) || IS_BCMCHIP(6813)  || IS_BCMCHIP(6756)
#define PLL_REFCLK  50
int pll_vco_freq_get(unsigned int pll_addr, unsigned int *fvco)
{
	int ret = 0;
	PLL_DECNDIV_REG pll_decndiv;
	PLL_DECPDIV_REG pll_decpdiv;
#if IS_BCMCHIP(63158) || IS_BCMCHIP(63178) || IS_BCMCHIP(47622) || \
	IS_BCMCHIP(63146) || IS_BCMCHIP(6855) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813) ||  \
        IS_BCMCHIP(6756)
	PLL_NDIV_REG ndiv_reg;
	PLL_PDIV_REG pdiv_reg;
#endif

#if IS_BCMCHIP(6856)
	ret =
	    ReadBPCMRegister(pll_addr, PLLCLASSICBPCMRegOffset(decndiv),
			     &pll_decndiv.Reg32);
	ret |=
	    ReadBPCMRegister(pll_addr, PLLCLASSICBPCMRegOffset(decpdiv),
			     &pll_decpdiv.Reg32);
#else
	ret =
	    ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(decndiv),
			     &pll_decndiv.Reg32);
	ret |=
	    ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(decpdiv),
			     &pll_decpdiv.Reg32);
#endif
	if (ret != 0)
		return -1;

	// Let's ignore ndiv_frac, it is set to zero anyway by HW.
	*fvco =
	    (PLL_REFCLK * (pll_decndiv.Bits.ndiv_int)) / pll_decpdiv.Bits.pdiv;

#if IS_BCMCHIP(63158) || IS_BCMCHIP(63178) || IS_BCMCHIP(47622) || \
	IS_BCMCHIP(63146)  || IS_BCMCHIP(6855) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813) ||  \
	IS_BCMCHIP(6756)
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

	ret = pll_vco_freq_get(pll_addr, &fvco);

	if (ret != 0)
		return -1;

	// The pll may include up to 6 channels.
	switch (ch) {
	case 0:
#if IS_BCMCHIP(6856)
		ret =
		    ReadBPCMRegister(pll_addr,
				     PLLCLASSICBPCMRegOffset(ch01_cfg),
				     &ch_cfg.Reg32);
#else
		ret =
		    ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch01_cfg),
				     &ch_cfg.Reg32);
#endif
		/* Check if default value is overitten */
		if (ch_cfg.Bits.mdiv_override0)
			mdiv = ch_cfg.Bits.mdiv0;	/* Use the new value */
		else {		/* If not, read from the default */

#if IS_BCMCHIP(6856)
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLLCLASSICBPCMRegOffset(decpdiv),
					     &pll_decpdiv.Reg32);
#else
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLLBPCMRegOffset(decpdiv),
					     &pll_decpdiv.Reg32);
#endif
			mdiv = pll_decpdiv.Bits.mdiv0;
		}
		break;
	case 1:
#if IS_BCMCHIP(6856)
		ret =
		    ReadBPCMRegister(pll_addr,
				     PLLCLASSICBPCMRegOffset(ch01_cfg),
				     &ch_cfg.Reg32);
#else
		ret =
		    ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch01_cfg),
				     &ch_cfg.Reg32);
#endif
		if (ch_cfg.Bits.mdiv_override1)
			mdiv = ch_cfg.Bits.mdiv1;
		else {
#if IS_BCMCHIP(6856)
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLLCLASSICBPCMRegOffset(decpdiv),
					     &pll_decpdiv.Reg32);
#else
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLLBPCMRegOffset(decpdiv),
					     &pll_decpdiv.Reg32);
#endif
			mdiv = pll_decpdiv.Bits.mdiv1;
		}
		break;
	case 2:
#if IS_BCMCHIP(6856)
		ret =
		    ReadBPCMRegister(pll_addr,
				     PLLCLASSICBPCMRegOffset(ch23_cfg),
				     &ch_cfg.Reg32);
#else
		ret =
		    ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch23_cfg),
				     &ch_cfg.Reg32);
#endif
		if (ch_cfg.Bits.mdiv_override0)
			mdiv = ch_cfg.Bits.mdiv0;
		else {
#if IS_BCMCHIP(6856)
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLLCLASSICBPCMRegOffset(decch25),
					     &pll_decch25.Reg32);
#else
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLLBPCMRegOffset(decch25),
					     &pll_decch25.Reg32);
#endif
			mdiv = pll_decch25.Bits.mdiv2;
		}
		break;
	case 3:
#if IS_BCMCHIP(6856)
		ret =
		    ReadBPCMRegister(pll_addr,
				     PLLCLASSICBPCMRegOffset(ch23_cfg),
				     &ch_cfg.Reg32);
#else
		ret =
		    ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch23_cfg),
				     &ch_cfg.Reg32);
#endif
		if (ch_cfg.Bits.mdiv_override1)
			mdiv = ch_cfg.Bits.mdiv1;
		else {
#if IS_BCMCHIP(6856)
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLLCLASSICBPCMRegOffset(decch25),
					     &pll_decch25.Reg32);
#else
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLLBPCMRegOffset(decch25),
					     &pll_decch25.Reg32);
#endif
			mdiv = pll_decch25.Bits.mdiv3;
		}
		break;
	case 4:
#if IS_BCMCHIP(6856)
		ret =
		    ReadBPCMRegister(pll_addr,
				     PLLCLASSICBPCMRegOffset(ch45_cfg),
				     &ch_cfg.Reg32);
#else
		ret =
		    ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch45_cfg),
				     &ch_cfg.Reg32);
#endif
		if (ch_cfg.Bits.mdiv_override0)
			mdiv = ch_cfg.Bits.mdiv0;
		else {
#if IS_BCMCHIP(6856)
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLLCLASSICBPCMRegOffset(decch25),
					     &pll_decch25.Reg32);
#else
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLLBPCMRegOffset(decch25),
					     &pll_decch25.Reg32);
#endif
			mdiv = pll_decch25.Bits.mdiv4;
		}
		break;
	case 5:
#if IS_BCMCHIP(6856)
		ret =
		    ReadBPCMRegister(pll_addr,
				     PLLCLASSICBPCMRegOffset(ch45_cfg),
				     &ch_cfg.Reg32);
#else
		ret =
		    ReadBPCMRegister(pll_addr, PLLBPCMRegOffset(ch45_cfg),
				     &ch_cfg.Reg32);
#endif
		if (ch_cfg.Bits.mdiv_override1)
			mdiv = ch_cfg.Bits.mdiv1;
		else {
#if IS_BCMCHIP(6856)
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLLCLASSICBPCMRegOffset(decch25),
					     &pll_decch25.Reg32);
			mdiv = pll_decch25.Bits.mdiv5;			
#elif IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813)
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLLBPCMRegOffset(decpdiv),
					     &pll_decpdiv.Reg32);
			mdiv = pll_decpdiv.Bits.mdiv5;			
#else			
			ret =
			    ReadBPCMRegister(pll_addr,
					     PLLBPCMRegOffset(decch25),
					     &pll_decch25.Reg32);
			mdiv = pll_decch25.Bits.mdiv5;			
#endif
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

#endif

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
	IS_BCMCHIP(6846)
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

#elif IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813)
	/* FIXME */
	/* Use RDPPLLBPCMRegOffset macro here. Its definitio is different than 
	   the main PLL */
	ret = 0;
	*rdp_freq = 1250;

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

#if IS_BCMCHIP(6858) || IS_BCMCHIP(6856) || IS_BCMCHIP(6846) || \
	IS_BCMCHIP(63158) || IS_BCMCHIP(63178) || IS_BCMCHIP(47622) || \
	IS_BCMCHIP(6878) || IS_BCMCHIP(63146)  || IS_BCMCHIP(6855) || \
	IS_BCMCHIP(4912) || IS_BCMCHIP(6813)   || IS_BCMCHIP(6756)
int pll_ch_freq_set(unsigned int pll_addr, unsigned int ch, unsigned int mdiv)
{
	int ret;

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

#if IS_BCMCHIP(6858) || IS_BCMCHIP(4908)
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

#if IS_BCMCHIP(6858) || IS_BCMCHIP(6846) || IS_BCMCHIP(6856) || IS_BCMCHIP(6878) || IS_BCMCHIP(63158)\
	|| IS_BCMCHIP(4912) || IS_BCMCHIP(6813) || IS_BCMCHIP(63146)
#if IS_BCMCHIP(6878)
#define PMD_CLOCK_REG pmd_xtal_cntl
#define CLOCK_RESET_XTAL_CONTROL_BIT_PD_DRV (2)
#define CLOCK_RESET_XTAL_CONTROL_BIT_PWRON  (27)
#define PMD_CLOCK_REG2 pmd_xtal_cntl2
#define CLOCK_RESET_XTAL_CONTROL2_BIT_PD    (0)
#else
#define PMD_CLOCK_REG xtal_control
#if IS_BCMCHIP(4912) || IS_BCMCHIP(6813) || IS_BCMCHIP(63146)
#define CLOCK_RESET_XTAL_CONTROL_BIT_PD_DRV (20)
#else
#define CLOCK_RESET_XTAL_CONTROL_BIT_PD_DRV (17)
#endif
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
#if IS_BCMCHIP(6878)
	data |= (0x1 << CLOCK_RESET_XTAL_CONTROL_BIT_PWRON);
#endif
#endif
	ret |= WriteBPCMRegister(PMB_ADDR_CHIP_CLKRST,
				CLKRSTBPCMRegOffset(PMD_CLOCK_REG), data);

#if IS_BCMCHIP(6878)
	ret |= ReadBPCMRegister(PMB_ADDR_CHIP_CLKRST,
			     CLKRSTBPCMRegOffset(PMD_CLOCK_REG2), &data);

	data &= ~(0x1 << CLOCK_RESET_XTAL_CONTROL2_BIT_PD);

	ret |= WriteBPCMRegister(PMB_ADDR_CHIP_CLKRST,
				CLKRSTBPCMRegOffset(PMD_CLOCK_REG2), data);
#endif

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
#if IS_BCMCHIP(63178) || IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813)
	int ret;
	BPCM_CLKRST_VREG_CONTROL vreg_control_reg;

	vreg_control_reg.Bits.enable = 1;
	vreg_control_reg.Bits.counter = 0x24;
#if IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813)
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

#if IS_BCMCHIP(4908) || IS_BCMCHIP(6858)

#if IS_BCMCHIP(6858)
#define C0_CLK_CONTROL (0x70 >> 2)
#define C0_CLK_PATTERN (0x78 >> 2)
#endif

uint32_t get_cluster_clk_pattern(void)
{
    uint32_t pattern = 0;
#if IS_BCMCHIP(6858) 
	ReadBPCMRegister(PMB_ADDR_BIU_BPCM, C0_CLK_PATTERN, &pattern);
#elif IS_BCMCHIP(4908)
    void __iomem *cpu_cluster_base = ioremap(BIU_CLUSTER_CLK_BASE, 0x10);
	pattern = readl(cpu_cluster_base + BIU_CLUSTER_CLK_PATERN_OFFSET);
    iounmap(cpu_cluster_base);
#endif
    return pattern;
}

void set_cluster_clk_pattern(uint32_t pattern)
{
#if IS_BCMCHIP(6858) 
	WriteBPCMRegister(PMB_ADDR_BIU_BPCM, C0_CLK_PATTERN, pattern);
#elif IS_BCMCHIP(4908)
    void __iomem *cpu_cluster_base = ioremap(BIU_CLUSTER_CLK_BASE, 0x10);
	writel(pattern, (cpu_cluster_base + BIU_CLUSTER_CLK_PATERN_OFFSET));
    iounmap(cpu_cluster_base);
#endif
}

void reset_cluster_clock(void)
{
	pmc_initmode();
#if IS_BCMCHIP(6858) 
	// cluster clock control: unreset
	WriteBPCMRegister(PMB_ADDR_BIU_BPCM, C0_CLK_CONTROL, 0x80000000);
	// cluster clock pattern: full-speed pattern
	WriteBPCMRegister(PMB_ADDR_BIU_BPCM, C0_CLK_PATTERN, ~0);
	// cluster clock control: enable pattern
	WriteBPCMRegister(PMB_ADDR_BIU_BPCM, C0_CLK_CONTROL, 0x80000010);
#elif IS_BCMCHIP(4908)
	{
		uint32_t clk_ctrl, clk_pattern;
        void __iomem *cpu_cluster_base = ioremap(BIU_CLUSTER_CLK_BASE, 0x10);
		clk_ctrl = readl(cpu_cluster_base + BIU_CLUSTER_CLK_CTRL_OFFSET);

		if ((clk_ctrl & (1 << 4)) == 0)
		{
			clk_pattern = ~0;
			clk_ctrl = (1<<4);
			writel(clk_pattern, (cpu_cluster_base + BIU_CLUSTER_CLK_PATERN_OFFSET));	// full-speed user clock-pattern
			writel(clk_ctrl, (cpu_cluster_base + BIU_CLUSTER_CLK_CTRL_OFFSET));	// enable user clock-patterns
		}
        iounmap(cpu_cluster_base);
	}
#endif
}
#endif

#if IS_BCMCHIP(63148)
void set_b15_mdiv(unsigned int value)
{
	PLL_CHCFG_REG ch01_cfg;
	ReadBPCMRegister(PMB_ADDR_B15_PLL, PLLBPCMRegOffset(ch01_cfg), &ch01_cfg.Reg32);
	if (ch01_cfg.Bits.mdiv0 != value ) 
    {
		printk("%s: set CPU PLL mdiv to %d and start cpu freq at 1.5GHz\n", __func__, value);
		ch01_cfg.Bits.mdiv0 = value;
		WriteBPCMRegister(PMB_ADDR_B15_PLL, PLLBPCMRegOffset(ch01_cfg), ch01_cfg.Reg32);
	}
}
#endif

void bcm_set_vreg_sync(void)
{
#if !defined(CONFIG_BRCM_QEMU)
#if defined(CONFIG_BCM963178) || defined(CONFIG_BCM963146) ||  defined(CONFIG_BCM94912) || IS_BCMCHIP(6813)
    set_vreg_clk();
#elif defined(CONFIG_BCM963158)
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

    pmc_wan_power_up();

    reg = readl(wan_top_v_reg_div);
    reg &= ~(VREG_CFG_VREG_CLK_BYPASS_MASK);
    writel(reg, wan_top_v_reg_div);

    reg = readl(wan_top_v_reg_div);
    reg &= ~(VREG_CFG_VREG_CLK_SRC_MASK);
    writel(reg, wan_top_v_reg_div);

    reg = readl(wan_top_v_reg_div);
    reg &= ~(VREG_CFG_VREG_DIV_MASK);
    reg |= 0x53;
    writel(reg, wan_top_v_reg_div);
    iounmap(wan_top_v_reg_div);
#endif
#endif
}
EXPORT_SYMBOL(bcm_set_vreg_sync);

void set_spike_mitigation(unsigned int spike_us)
{
#if !defined(PMC_ON_HOSTCPU)
	volatile Pmc *pmc = (volatile Pmc *)g_pmc->pmc_base;

	// tmrctl = enable | microseconds | spike_us 
	pmc->ctrl.gpTmr0Ctl = (1 << 31) | (1 << 29) | spike_us;
	while (pmc->ctrl.gpTmr0Ctl & (1 << 31));
#endif
}
