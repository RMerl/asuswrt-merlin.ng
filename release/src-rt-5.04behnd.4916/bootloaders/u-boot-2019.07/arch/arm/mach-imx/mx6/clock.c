// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010-2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <div64.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>

enum pll_clocks {
	PLL_SYS,	/* System PLL */
	PLL_BUS,	/* System Bus PLL*/
	PLL_USBOTG,	/* OTG USB PLL */
	PLL_ENET,	/* ENET PLL */
	PLL_AUDIO,	/* AUDIO PLL */
	PLL_VIDEO,	/* VIDEO PLL */
};

struct mxc_ccm_reg *imx_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

#ifdef CONFIG_MXC_OCOTP
void enable_ocotp_clk(unsigned char enable)
{
	u32 reg;

	reg = __raw_readl(&imx_ccm->CCGR2);
	if (enable)
		reg |= MXC_CCM_CCGR2_OCOTP_CTRL_MASK;
	else
		reg &= ~MXC_CCM_CCGR2_OCOTP_CTRL_MASK;
	__raw_writel(reg, &imx_ccm->CCGR2);
}
#endif

#ifdef CONFIG_NAND_MXS
void setup_gpmi_io_clk(u32 cfg)
{
	/* Disable clocks per ERR007177 from MX6 errata */
	clrbits_le32(&imx_ccm->CCGR4,
		     MXC_CCM_CCGR4_RAWNAND_U_BCH_INPUT_APB_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_BCH_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_GPMI_IO_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_INPUT_APB_MASK |
		     MXC_CCM_CCGR4_PL301_MX6QPER1_BCH_MASK);

#if defined(CONFIG_MX6SX)
	clrbits_le32(&imx_ccm->CCGR4, MXC_CCM_CCGR4_QSPI2_ENFC_MASK);

	clrsetbits_le32(&imx_ccm->cs2cdr,
			MXC_CCM_CS2CDR_QSPI2_CLK_PODF_MASK |
			MXC_CCM_CS2CDR_QSPI2_CLK_PRED_MASK |
			MXC_CCM_CS2CDR_QSPI2_CLK_SEL_MASK,
			cfg);

	setbits_le32(&imx_ccm->CCGR4, MXC_CCM_CCGR4_QSPI2_ENFC_MASK);
#else
	clrbits_le32(&imx_ccm->CCGR2, MXC_CCM_CCGR2_IOMUX_IPT_CLK_IO_MASK);

	clrsetbits_le32(&imx_ccm->cs2cdr,
			MXC_CCM_CS2CDR_ENFC_CLK_PODF_MASK |
			MXC_CCM_CS2CDR_ENFC_CLK_PRED_MASK |
			MXC_CCM_CS2CDR_ENFC_CLK_SEL_MASK,
			cfg);

	setbits_le32(&imx_ccm->CCGR2, MXC_CCM_CCGR2_IOMUX_IPT_CLK_IO_MASK);
#endif
	setbits_le32(&imx_ccm->CCGR4,
		     MXC_CCM_CCGR4_RAWNAND_U_BCH_INPUT_APB_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_BCH_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_GPMI_IO_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_INPUT_APB_MASK |
		     MXC_CCM_CCGR4_PL301_MX6QPER1_BCH_MASK);
}
#endif

void enable_usboh3_clk(unsigned char enable)
{
	u32 reg;

	reg = __raw_readl(&imx_ccm->CCGR6);
	if (enable)
		reg |= MXC_CCM_CCGR6_USBOH3_MASK;
	else
		reg &= ~(MXC_CCM_CCGR6_USBOH3_MASK);
	__raw_writel(reg, &imx_ccm->CCGR6);

}

#if defined(CONFIG_FEC_MXC) && !defined(CONFIG_MX6SX)
void enable_enet_clk(unsigned char enable)
{
	u32 mask, *addr;

	if (is_mx6ull()) {
		mask = MXC_CCM_CCGR0_ENET_CLK_ENABLE_MASK;
		addr = &imx_ccm->CCGR0;
	} else if (is_mx6ul()) {
		mask = MXC_CCM_CCGR3_ENET_MASK;
		addr = &imx_ccm->CCGR3;
	} else {
		mask = MXC_CCM_CCGR1_ENET_MASK;
		addr = &imx_ccm->CCGR1;
	}

	if (enable)
		setbits_le32(addr, mask);
	else
		clrbits_le32(addr, mask);
}
#endif

#ifdef CONFIG_MXC_UART
void enable_uart_clk(unsigned char enable)
{
	u32 mask;

	if (is_mx6ul() || is_mx6ull())
		mask = MXC_CCM_CCGR5_UART_MASK;
	else
		mask = MXC_CCM_CCGR5_UART_MASK | MXC_CCM_CCGR5_UART_SERIAL_MASK;

	if (enable)
		setbits_le32(&imx_ccm->CCGR5, mask);
	else
		clrbits_le32(&imx_ccm->CCGR5, mask);
}
#endif

#ifdef CONFIG_MMC
int enable_usdhc_clk(unsigned char enable, unsigned bus_num)
{
	u32 mask;

	if (bus_num > 3)
		return -EINVAL;

	mask = MXC_CCM_CCGR_CG_MASK << (bus_num * 2 + 2);
	if (enable)
		setbits_le32(&imx_ccm->CCGR6, mask);
	else
		clrbits_le32(&imx_ccm->CCGR6, mask);

	return 0;
}
#endif

#ifdef CONFIG_SYS_I2C_MXC
/* i2c_num can be from 0 - 3 */
int enable_i2c_clk(unsigned char enable, unsigned i2c_num)
{
	u32 reg;
	u32 mask;
	u32 *addr;

	if (i2c_num > 3)
		return -EINVAL;
	if (i2c_num < 3) {
		mask = MXC_CCM_CCGR_CG_MASK
			<< (MXC_CCM_CCGR2_I2C1_SERIAL_OFFSET
			+ (i2c_num << 1));
		reg = __raw_readl(&imx_ccm->CCGR2);
		if (enable)
			reg |= mask;
		else
			reg &= ~mask;
		__raw_writel(reg, &imx_ccm->CCGR2);
	} else {
		if (is_mx6sll())
			return -EINVAL;
		if (is_mx6sx() || is_mx6ul() || is_mx6ull()) {
			mask = MXC_CCM_CCGR6_I2C4_MASK;
			addr = &imx_ccm->CCGR6;
		} else {
			mask = MXC_CCM_CCGR1_I2C4_SERIAL_MASK;
			addr = &imx_ccm->CCGR1;
		}
		reg = __raw_readl(addr);
		if (enable)
			reg |= mask;
		else
			reg &= ~mask;
		__raw_writel(reg, addr);
	}
	return 0;
}
#endif

/* spi_num can be from 0 - SPI_MAX_NUM */
int enable_spi_clk(unsigned char enable, unsigned spi_num)
{
	u32 reg;
	u32 mask;

	if (spi_num > SPI_MAX_NUM)
		return -EINVAL;

	mask = MXC_CCM_CCGR_CG_MASK << (spi_num << 1);
	reg = __raw_readl(&imx_ccm->CCGR1);
	if (enable)
		reg |= mask;
	else
		reg &= ~mask;
	__raw_writel(reg, &imx_ccm->CCGR1);
	return 0;
}
static u32 decode_pll(enum pll_clocks pll, u32 infreq)
{
	u32 div, test_div, pll_num, pll_denom;

	switch (pll) {
	case PLL_SYS:
		div = __raw_readl(&imx_ccm->analog_pll_sys);
		div &= BM_ANADIG_PLL_SYS_DIV_SELECT;

		return (infreq * div) >> 1;
	case PLL_BUS:
		div = __raw_readl(&imx_ccm->analog_pll_528);
		div &= BM_ANADIG_PLL_528_DIV_SELECT;

		return infreq * (20 + (div << 1));
	case PLL_USBOTG:
		div = __raw_readl(&imx_ccm->analog_usb1_pll_480_ctrl);
		div &= BM_ANADIG_USB1_PLL_480_CTRL_DIV_SELECT;

		return infreq * (20 + (div << 1));
	case PLL_ENET:
		div = __raw_readl(&imx_ccm->analog_pll_enet);
		div &= BM_ANADIG_PLL_ENET_DIV_SELECT;

		return 25000000 * (div + (div >> 1) + 1);
	case PLL_AUDIO:
		div = __raw_readl(&imx_ccm->analog_pll_audio);
		if (!(div & BM_ANADIG_PLL_AUDIO_ENABLE))
			return 0;
		/* BM_ANADIG_PLL_AUDIO_BYPASS_CLK_SRC is ignored */
		if (div & BM_ANADIG_PLL_AUDIO_BYPASS)
			return MXC_HCLK;
		pll_num = __raw_readl(&imx_ccm->analog_pll_audio_num);
		pll_denom = __raw_readl(&imx_ccm->analog_pll_audio_denom);
		test_div = (div & BM_ANADIG_PLL_AUDIO_TEST_DIV_SELECT) >>
			BP_ANADIG_PLL_AUDIO_TEST_DIV_SELECT;
		div &= BM_ANADIG_PLL_AUDIO_DIV_SELECT;
		if (test_div == 3) {
			debug("Error test_div\n");
			return 0;
		}
		test_div = 1 << (2 - test_div);

		return infreq * (div + pll_num / pll_denom) / test_div;
	case PLL_VIDEO:
		div = __raw_readl(&imx_ccm->analog_pll_video);
		if (!(div & BM_ANADIG_PLL_VIDEO_ENABLE))
			return 0;
		/* BM_ANADIG_PLL_AUDIO_BYPASS_CLK_SRC is ignored */
		if (div & BM_ANADIG_PLL_VIDEO_BYPASS)
			return MXC_HCLK;
		pll_num = __raw_readl(&imx_ccm->analog_pll_video_num);
		pll_denom = __raw_readl(&imx_ccm->analog_pll_video_denom);
		test_div = (div & BM_ANADIG_PLL_VIDEO_POST_DIV_SELECT) >>
			BP_ANADIG_PLL_VIDEO_POST_DIV_SELECT;
		div &= BM_ANADIG_PLL_VIDEO_DIV_SELECT;
		if (test_div == 3) {
			debug("Error test_div\n");
			return 0;
		}
		test_div = 1 << (2 - test_div);

		return infreq * (div + pll_num / pll_denom) / test_div;
	default:
		return 0;
	}
	/* NOTREACHED */
}
static u32 mxc_get_pll_pfd(enum pll_clocks pll, int pfd_num)
{
	u32 div;
	u64 freq;

	switch (pll) {
	case PLL_BUS:
		if (!is_mx6ul() && !is_mx6ull()) {
			if (pfd_num == 3) {
				/* No PFD3 on PLL2 */
				return 0;
			}
		}
		div = __raw_readl(&imx_ccm->analog_pfd_528);
		freq = (u64)decode_pll(PLL_BUS, MXC_HCLK);
		break;
	case PLL_USBOTG:
		div = __raw_readl(&imx_ccm->analog_pfd_480);
		freq = (u64)decode_pll(PLL_USBOTG, MXC_HCLK);
		break;
	default:
		/* No PFD on other PLL					     */
		return 0;
	}

	return lldiv(freq * 18, (div & ANATOP_PFD_FRAC_MASK(pfd_num)) >>
			      ANATOP_PFD_FRAC_SHIFT(pfd_num));
}

static u32 get_mcu_main_clk(void)
{
	u32 reg, freq;

	reg = __raw_readl(&imx_ccm->cacrr);
	reg &= MXC_CCM_CACRR_ARM_PODF_MASK;
	reg >>= MXC_CCM_CACRR_ARM_PODF_OFFSET;
	freq = decode_pll(PLL_SYS, MXC_HCLK);

	return freq / (reg + 1);
}

u32 get_periph_clk(void)
{
	u32 reg, div = 0, freq = 0;

	reg = __raw_readl(&imx_ccm->cbcdr);
	if (reg & MXC_CCM_CBCDR_PERIPH_CLK_SEL) {
		div = (reg & MXC_CCM_CBCDR_PERIPH_CLK2_PODF_MASK) >>
		       MXC_CCM_CBCDR_PERIPH_CLK2_PODF_OFFSET;
		reg = __raw_readl(&imx_ccm->cbcmr);
		reg &= MXC_CCM_CBCMR_PERIPH_CLK2_SEL_MASK;
		reg >>= MXC_CCM_CBCMR_PERIPH_CLK2_SEL_OFFSET;

		switch (reg) {
		case 0:
			freq = decode_pll(PLL_USBOTG, MXC_HCLK);
			break;
		case 1:
		case 2:
			freq = MXC_HCLK;
			break;
		default:
			break;
		}
	} else {
		reg = __raw_readl(&imx_ccm->cbcmr);
		reg &= MXC_CCM_CBCMR_PRE_PERIPH_CLK_SEL_MASK;
		reg >>= MXC_CCM_CBCMR_PRE_PERIPH_CLK_SEL_OFFSET;

		switch (reg) {
		case 0:
			freq = decode_pll(PLL_BUS, MXC_HCLK);
			break;
		case 1:
			freq = mxc_get_pll_pfd(PLL_BUS, 2);
			break;
		case 2:
			freq = mxc_get_pll_pfd(PLL_BUS, 0);
			break;
		case 3:
			/* static / 2 divider */
			freq = mxc_get_pll_pfd(PLL_BUS, 2) / 2;
			break;
		default:
			break;
		}
	}

	return freq / (div + 1);
}

static u32 get_ipg_clk(void)
{
	u32 reg, ipg_podf;

	reg = __raw_readl(&imx_ccm->cbcdr);
	reg &= MXC_CCM_CBCDR_IPG_PODF_MASK;
	ipg_podf = reg >> MXC_CCM_CBCDR_IPG_PODF_OFFSET;

	return get_ahb_clk() / (ipg_podf + 1);
}

static u32 get_ipg_per_clk(void)
{
	u32 reg, perclk_podf;

	reg = __raw_readl(&imx_ccm->cscmr1);
	if (is_mx6sll() || is_mx6sl() || is_mx6sx() ||
	    is_mx6dqp() || is_mx6ul() || is_mx6ull()) {
		if (reg & MXC_CCM_CSCMR1_PER_CLK_SEL_MASK)
			return MXC_HCLK; /* OSC 24Mhz */
	}

	perclk_podf = reg & MXC_CCM_CSCMR1_PERCLK_PODF_MASK;

	return get_ipg_clk() / (perclk_podf + 1);
}

static u32 get_uart_clk(void)
{
	u32 reg, uart_podf;
	u32 freq = decode_pll(PLL_USBOTG, MXC_HCLK) / 6; /* static divider */
	reg = __raw_readl(&imx_ccm->cscdr1);

	if (is_mx6sl() || is_mx6sx() || is_mx6dqp() || is_mx6ul() ||
	    is_mx6sll() || is_mx6ull()) {
		if (reg & MXC_CCM_CSCDR1_UART_CLK_SEL)
			freq = MXC_HCLK;
	}

	reg &= MXC_CCM_CSCDR1_UART_CLK_PODF_MASK;
	uart_podf = reg >> MXC_CCM_CSCDR1_UART_CLK_PODF_OFFSET;

	return freq / (uart_podf + 1);
}

static u32 get_cspi_clk(void)
{
	u32 reg, cspi_podf;

	reg = __raw_readl(&imx_ccm->cscdr2);
	cspi_podf = (reg & MXC_CCM_CSCDR2_ECSPI_CLK_PODF_MASK) >>
		     MXC_CCM_CSCDR2_ECSPI_CLK_PODF_OFFSET;

	if (is_mx6dqp() || is_mx6sl() || is_mx6sx() || is_mx6ul() ||
	    is_mx6sll() || is_mx6ull()) {
		if (reg & MXC_CCM_CSCDR2_ECSPI_CLK_SEL_MASK)
			return MXC_HCLK / (cspi_podf + 1);
	}

	return	decode_pll(PLL_USBOTG, MXC_HCLK) / (8 * (cspi_podf + 1));
}

static u32 get_axi_clk(void)
{
	u32 root_freq, axi_podf;
	u32 cbcdr =  __raw_readl(&imx_ccm->cbcdr);

	axi_podf = cbcdr & MXC_CCM_CBCDR_AXI_PODF_MASK;
	axi_podf >>= MXC_CCM_CBCDR_AXI_PODF_OFFSET;

	if (cbcdr & MXC_CCM_CBCDR_AXI_SEL) {
		if (cbcdr & MXC_CCM_CBCDR_AXI_ALT_SEL)
			root_freq = mxc_get_pll_pfd(PLL_USBOTG, 1);
		else
			root_freq = mxc_get_pll_pfd(PLL_BUS, 2);
	} else
		root_freq = get_periph_clk();

	return  root_freq / (axi_podf + 1);
}

static u32 get_emi_slow_clk(void)
{
	u32 emi_clk_sel, emi_slow_podf, cscmr1, root_freq = 0;

	cscmr1 =  __raw_readl(&imx_ccm->cscmr1);
	emi_clk_sel = cscmr1 & MXC_CCM_CSCMR1_ACLK_EMI_SLOW_MASK;
	emi_clk_sel >>= MXC_CCM_CSCMR1_ACLK_EMI_SLOW_OFFSET;
	emi_slow_podf = cscmr1 & MXC_CCM_CSCMR1_ACLK_EMI_SLOW_PODF_MASK;
	emi_slow_podf >>= MXC_CCM_CSCMR1_ACLK_EMI_SLOW_PODF_OFFSET;

	switch (emi_clk_sel) {
	case 0:
		root_freq = get_axi_clk();
		break;
	case 1:
		root_freq = decode_pll(PLL_USBOTG, MXC_HCLK);
		break;
	case 2:
		root_freq =  mxc_get_pll_pfd(PLL_BUS, 2);
		break;
	case 3:
		root_freq =  mxc_get_pll_pfd(PLL_BUS, 0);
		break;
	}

	return root_freq / (emi_slow_podf + 1);
}

static u32 get_mmdc_ch0_clk(void)
{
	u32 cbcmr = __raw_readl(&imx_ccm->cbcmr);
	u32 cbcdr = __raw_readl(&imx_ccm->cbcdr);

	u32 freq, podf, per2_clk2_podf, pmu_misc2_audio_div;

	if (is_mx6sx() || is_mx6ul() || is_mx6ull() || is_mx6sl() ||
	    is_mx6sll()) {
		podf = (cbcdr & MXC_CCM_CBCDR_MMDC_CH1_PODF_MASK) >>
			MXC_CCM_CBCDR_MMDC_CH1_PODF_OFFSET;
		if (cbcdr & MXC_CCM_CBCDR_PERIPH2_CLK_SEL) {
			per2_clk2_podf = (cbcdr & MXC_CCM_CBCDR_PERIPH2_CLK2_PODF_MASK) >>
				MXC_CCM_CBCDR_PERIPH2_CLK2_PODF_OFFSET;
			if (is_mx6sl()) {
				if (cbcmr & MXC_CCM_CBCMR_PERIPH2_CLK2_SEL)
					freq = MXC_HCLK;
				else
					freq = decode_pll(PLL_USBOTG, MXC_HCLK);
			} else {
				if (cbcmr & MXC_CCM_CBCMR_PERIPH2_CLK2_SEL)
					freq = decode_pll(PLL_BUS, MXC_HCLK);
				else
					freq = decode_pll(PLL_USBOTG, MXC_HCLK);
			}
		} else {
			per2_clk2_podf = 0;
			switch ((cbcmr &
				MXC_CCM_CBCMR_PRE_PERIPH2_CLK_SEL_MASK) >>
				MXC_CCM_CBCMR_PRE_PERIPH2_CLK_SEL_OFFSET) {
			case 0:
				freq = decode_pll(PLL_BUS, MXC_HCLK);
				break;
			case 1:
				freq = mxc_get_pll_pfd(PLL_BUS, 2);
				break;
			case 2:
				freq = mxc_get_pll_pfd(PLL_BUS, 0);
				break;
			case 3:
				if (is_mx6sl()) {
					freq = mxc_get_pll_pfd(PLL_BUS, 2) >> 1;
					break;
				}

				pmu_misc2_audio_div = PMU_MISC2_AUDIO_DIV(__raw_readl(&imx_ccm->pmu_misc2));
				switch (pmu_misc2_audio_div) {
				case 0:
				case 2:
					pmu_misc2_audio_div = 1;
					break;
				case 1:
					pmu_misc2_audio_div = 2;
					break;
				case 3:
					pmu_misc2_audio_div = 4;
					break;
				}
				freq = decode_pll(PLL_AUDIO, MXC_HCLK) /
					pmu_misc2_audio_div;
				break;
			}
		}
		return freq / (podf + 1) / (per2_clk2_podf + 1);
	} else {
		podf = (cbcdr & MXC_CCM_CBCDR_MMDC_CH0_PODF_MASK) >>
			MXC_CCM_CBCDR_MMDC_CH0_PODF_OFFSET;
		return get_periph_clk() / (podf + 1);
	}
}

#if defined(CONFIG_VIDEO_MXS)
static int enable_pll_video(u32 pll_div, u32 pll_num, u32 pll_denom,
			    u32 post_div)
{
	u32 reg = 0;
	ulong start;

	debug("pll5 div = %d, num = %d, denom = %d\n",
	      pll_div, pll_num, pll_denom);

	/* Power up PLL5 video */
	writel(BM_ANADIG_PLL_VIDEO_POWERDOWN |
	       BM_ANADIG_PLL_VIDEO_BYPASS |
	       BM_ANADIG_PLL_VIDEO_DIV_SELECT |
	       BM_ANADIG_PLL_VIDEO_POST_DIV_SELECT,
	       &imx_ccm->analog_pll_video_clr);

	/* Set div, num and denom */
	switch (post_div) {
	case 1:
		writel(BF_ANADIG_PLL_VIDEO_DIV_SELECT(pll_div) |
		       BF_ANADIG_PLL_VIDEO_POST_DIV_SELECT(0x2),
		       &imx_ccm->analog_pll_video_set);
		break;
	case 2:
		writel(BF_ANADIG_PLL_VIDEO_DIV_SELECT(pll_div) |
		       BF_ANADIG_PLL_VIDEO_POST_DIV_SELECT(0x1),
		       &imx_ccm->analog_pll_video_set);
		break;
	case 4:
		writel(BF_ANADIG_PLL_VIDEO_DIV_SELECT(pll_div) |
		       BF_ANADIG_PLL_VIDEO_POST_DIV_SELECT(0x0),
		       &imx_ccm->analog_pll_video_set);
		break;
	default:
		puts("Wrong test_div!\n");
		return -EINVAL;
	}

	writel(BF_ANADIG_PLL_VIDEO_NUM_A(pll_num),
	       &imx_ccm->analog_pll_video_num);
	writel(BF_ANADIG_PLL_VIDEO_DENOM_B(pll_denom),
	       &imx_ccm->analog_pll_video_denom);

	/* Wait PLL5 lock */
	start = get_timer(0);	/* Get current timestamp */

	do {
		reg = readl(&imx_ccm->analog_pll_video);
		if (reg & BM_ANADIG_PLL_VIDEO_LOCK) {
			/* Enable PLL out */
			writel(BM_ANADIG_PLL_VIDEO_ENABLE,
			       &imx_ccm->analog_pll_video_set);
			return 0;
		}
	} while (get_timer(0) < (start + 10)); /* Wait 10ms */

	puts("Lock PLL5 timeout\n");

	return -ETIME;
}

/*
 * 24M--> PLL_VIDEO -> LCDIFx_PRED -> LCDIFx_PODF -> LCD
 *
 * 'freq' using KHz as unit, see driver/video/mxsfb.c.
 */
void mxs_set_lcdclk(u32 base_addr, u32 freq)
{
	u32 reg = 0;
	u32 hck = MXC_HCLK / 1000;
	/* DIV_SELECT ranges from 27 to 54 */
	u32 min = hck * 27;
	u32 max = hck * 54;
	u32 temp, best = 0;
	u32 i, j, max_pred = 8, max_postd = 8, pred = 1, postd = 1;
	u32 pll_div, pll_num, pll_denom, post_div = 1;

	debug("mxs_set_lcdclk, freq = %dKHz\n", freq);

	if (!is_mx6sx() && !is_mx6ul() && !is_mx6ull() && !is_mx6sl() &&
	    !is_mx6sll()) {
		debug("This chip not support lcd!\n");
		return;
	}

	if (!is_mx6sl()) {
		if (base_addr == LCDIF1_BASE_ADDR) {
			reg = readl(&imx_ccm->cscdr2);
			/* Can't change clocks when clock not from pre-mux */
			if ((reg & MXC_CCM_CSCDR2_LCDIF1_CLK_SEL_MASK) != 0)
				return;
		}
	}

	if (is_mx6sx()) {
		reg = readl(&imx_ccm->cscdr2);
		/* Can't change clocks when clock not from pre-mux */
		if ((reg & MXC_CCM_CSCDR2_LCDIF2_CLK_SEL_MASK) != 0)
			return;
	}

	temp = freq * max_pred * max_postd;
	if (temp < min) {
		/*
		 * Register: PLL_VIDEO
		 * Bit Field: POST_DIV_SELECT
		 * 00 — Divide by 4.
		 * 01 — Divide by 2.
		 * 10 — Divide by 1.
		 * 11 — Reserved
		 * No need to check post_div(1)
		 */
		for (post_div = 2; post_div <= 4; post_div <<= 1) {
			if ((temp * post_div) > min) {
				freq *= post_div;
				break;
			}
		}

		if (post_div > 4) {
			printf("Fail to set rate to %dkhz", freq);
			return;
		}
	}

	/* Choose the best pred and postd to match freq for lcd */
	for (i = 1; i <= max_pred; i++) {
		for (j = 1; j <= max_postd; j++) {
			temp = freq * i * j;
			if (temp > max || temp < min)
				continue;
			if (best == 0 || temp < best) {
				best = temp;
				pred = i;
				postd = j;
			}
		}
	}

	if (best == 0) {
		printf("Fail to set rate to %dKHz", freq);
		return;
	}

	debug("best %d, pred = %d, postd = %d\n", best, pred, postd);

	pll_div = best / hck;
	pll_denom = 1000000;
	pll_num = (best - hck * pll_div) * pll_denom / hck;

	/*
	 *                                  pll_num
	 *             (24MHz * (pll_div + --------- ))
	 *                                 pll_denom
	 *freq KHz =  --------------------------------
	 *             post_div * pred * postd * 1000
	 */

	if (base_addr == LCDIF1_BASE_ADDR) {
		if (enable_pll_video(pll_div, pll_num, pll_denom, post_div))
			return;

		enable_lcdif_clock(base_addr, 0);
		if (!is_mx6sl()) {
			/* Select pre-lcd clock to PLL5 and set pre divider */
			clrsetbits_le32(&imx_ccm->cscdr2,
					MXC_CCM_CSCDR2_LCDIF1_PRED_SEL_MASK |
					MXC_CCM_CSCDR2_LCDIF1_PRE_DIV_MASK,
					(0x2 << MXC_CCM_CSCDR2_LCDIF1_PRED_SEL_OFFSET) |
					((pred - 1) <<
					 MXC_CCM_CSCDR2_LCDIF1_PRE_DIV_OFFSET));

			/* Set the post divider */
			clrsetbits_le32(&imx_ccm->cbcmr,
					MXC_CCM_CBCMR_LCDIF1_PODF_MASK,
					((postd - 1) <<
					MXC_CCM_CBCMR_LCDIF1_PODF_OFFSET));
		} else {
			/* Select pre-lcd clock to PLL5 and set pre divider */
			clrsetbits_le32(&imx_ccm->cscdr2,
					MXC_CCM_CSCDR2_LCDIF_PIX_CLK_SEL_MASK |
					MXC_CCM_CSCDR2_LCDIF_PIX_PRE_DIV_MASK,
					(0x2 << MXC_CCM_CSCDR2_LCDIF_PIX_CLK_SEL_OFFSET) |
					((pred - 1) <<
					 MXC_CCM_CSCDR2_LCDIF_PIX_PRE_DIV_OFFSET));

			/* Set the post divider */
			clrsetbits_le32(&imx_ccm->cscmr1,
					MXC_CCM_CSCMR1_LCDIF_PIX_PODF_MASK,
					(((postd - 1)^0x6) <<
					 MXC_CCM_CSCMR1_LCDIF_PIX_PODF_OFFSET));
		}

		enable_lcdif_clock(base_addr, 1);
	} else if (is_mx6sx()) {
		/* Setting LCDIF2 for i.MX6SX */
		if (enable_pll_video(pll_div, pll_num, pll_denom, post_div))
			return;

		enable_lcdif_clock(base_addr, 0);
		/* Select pre-lcd clock to PLL5 and set pre divider */
		clrsetbits_le32(&imx_ccm->cscdr2,
				MXC_CCM_CSCDR2_LCDIF2_PRED_SEL_MASK |
				MXC_CCM_CSCDR2_LCDIF2_PRE_DIV_MASK,
				(0x2 << MXC_CCM_CSCDR2_LCDIF2_PRED_SEL_OFFSET) |
				((pred - 1) <<
				 MXC_CCM_CSCDR2_LCDIF2_PRE_DIV_OFFSET));

		/* Set the post divider */
		clrsetbits_le32(&imx_ccm->cscmr1,
				MXC_CCM_CSCMR1_LCDIF2_PODF_MASK,
				((postd - 1) <<
				 MXC_CCM_CSCMR1_LCDIF2_PODF_OFFSET));

		enable_lcdif_clock(base_addr, 1);
	}
}

int enable_lcdif_clock(u32 base_addr, bool enable)
{
	u32 reg = 0;
	u32 lcdif_clk_sel_mask, lcdif_ccgr3_mask;

	if (is_mx6sx()) {
		if ((base_addr != LCDIF1_BASE_ADDR) &&
		    (base_addr != LCDIF2_BASE_ADDR)) {
			puts("Wrong LCD interface!\n");
			return -EINVAL;
		}
		/* Set to pre-mux clock at default */
		lcdif_clk_sel_mask = (base_addr == LCDIF2_BASE_ADDR) ?
			MXC_CCM_CSCDR2_LCDIF2_CLK_SEL_MASK :
			MXC_CCM_CSCDR2_LCDIF1_CLK_SEL_MASK;
		lcdif_ccgr3_mask = (base_addr == LCDIF2_BASE_ADDR) ?
			(MXC_CCM_CCGR3_LCDIF2_PIX_MASK |
			 MXC_CCM_CCGR3_DISP_AXI_MASK) :
			(MXC_CCM_CCGR3_LCDIF1_PIX_MASK |
			 MXC_CCM_CCGR3_DISP_AXI_MASK);
	} else if (is_mx6ul() || is_mx6ull() || is_mx6sll()) {
		if (base_addr != LCDIF1_BASE_ADDR) {
			puts("Wrong LCD interface!\n");
			return -EINVAL;
		}
		/* Set to pre-mux clock at default */
		lcdif_clk_sel_mask = MXC_CCM_CSCDR2_LCDIF1_CLK_SEL_MASK;
		lcdif_ccgr3_mask =  MXC_CCM_CCGR3_LCDIF1_PIX_MASK;
	} else if (is_mx6sl()) {
		if (base_addr != LCDIF1_BASE_ADDR) {
			puts("Wrong LCD interface!\n");
			return -EINVAL;
		}

		reg = readl(&imx_ccm->CCGR3);
		reg &= ~(MXC_CCM_CCGR3_LCDIF_AXI_MASK |
			 MXC_CCM_CCGR3_LCDIF_PIX_MASK);
		writel(reg, &imx_ccm->CCGR3);

		if (enable) {
			reg = readl(&imx_ccm->cscdr3);
			reg &= ~MXC_CCM_CSCDR3_LCDIF_AXI_CLK_SEL_MASK;
			reg |= 1 << MXC_CCM_CSCDR3_LCDIF_AXI_CLK_SEL_OFFSET;
			writel(reg, &imx_ccm->cscdr3);

			reg = readl(&imx_ccm->CCGR3);
			reg |= MXC_CCM_CCGR3_LCDIF_AXI_MASK |
				MXC_CCM_CCGR3_LCDIF_PIX_MASK;
			writel(reg, &imx_ccm->CCGR3);
		}

		return 0;
	} else {
		return 0;
	}

	/* Gate LCDIF clock first */
	reg = readl(&imx_ccm->CCGR3);
	reg &= ~lcdif_ccgr3_mask;
	writel(reg, &imx_ccm->CCGR3);

	reg = readl(&imx_ccm->CCGR2);
	reg &= ~MXC_CCM_CCGR2_LCD_MASK;
	writel(reg, &imx_ccm->CCGR2);

	if (enable) {
		/* Select pre-mux */
		reg = readl(&imx_ccm->cscdr2);
		reg &= ~lcdif_clk_sel_mask;
		writel(reg, &imx_ccm->cscdr2);

		/* Enable the LCDIF pix clock */
		reg = readl(&imx_ccm->CCGR3);
		reg |= lcdif_ccgr3_mask;
		writel(reg, &imx_ccm->CCGR3);

		reg = readl(&imx_ccm->CCGR2);
		reg |= MXC_CCM_CCGR2_LCD_MASK;
		writel(reg, &imx_ccm->CCGR2);
	}

	return 0;
}
#endif

#ifdef CONFIG_FSL_QSPI
/* qspi_num can be from 0 - 1 */
void enable_qspi_clk(int qspi_num)
{
	u32 reg = 0;
	/* Enable QuadSPI clock */
	switch (qspi_num) {
	case 0:
		/* disable the clock gate */
		clrbits_le32(&imx_ccm->CCGR3, MXC_CCM_CCGR3_QSPI1_MASK);

		/* set 50M  : (50 = 396 / 2 / 4) */
		reg = readl(&imx_ccm->cscmr1);
		reg &= ~(MXC_CCM_CSCMR1_QSPI1_PODF_MASK |
			 MXC_CCM_CSCMR1_QSPI1_CLK_SEL_MASK);
		reg |= ((1 << MXC_CCM_CSCMR1_QSPI1_PODF_OFFSET) |
			(2 << MXC_CCM_CSCMR1_QSPI1_CLK_SEL_OFFSET));
		writel(reg, &imx_ccm->cscmr1);

		/* enable the clock gate */
		setbits_le32(&imx_ccm->CCGR3, MXC_CCM_CCGR3_QSPI1_MASK);
		break;
	case 1:
		/*
		 * disable the clock gate
		 * QSPI2 and GPMI_BCH_INPUT_GPMI_IO share the same clock gate,
		 * disable both of them.
		 */
		clrbits_le32(&imx_ccm->CCGR4, MXC_CCM_CCGR4_QSPI2_ENFC_MASK |
			     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_GPMI_IO_MASK);

		/* set 50M  : (50 = 396 / 2 / 4) */
		reg = readl(&imx_ccm->cs2cdr);
		reg &= ~(MXC_CCM_CS2CDR_QSPI2_CLK_PODF_MASK |
			 MXC_CCM_CS2CDR_QSPI2_CLK_PRED_MASK |
			 MXC_CCM_CS2CDR_QSPI2_CLK_SEL_MASK);
		reg |= (MXC_CCM_CS2CDR_QSPI2_CLK_PRED(0x1) |
			MXC_CCM_CS2CDR_QSPI2_CLK_SEL(0x3));
		writel(reg, &imx_ccm->cs2cdr);

		/*enable the clock gate*/
		setbits_le32(&imx_ccm->CCGR4, MXC_CCM_CCGR4_QSPI2_ENFC_MASK |
			     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_GPMI_IO_MASK);
		break;
	default:
		break;
	}
}
#endif

#ifdef CONFIG_FEC_MXC
int enable_fec_anatop_clock(int fec_id, enum enet_freq freq)
{
	u32 reg = 0;
	s32 timeout = 100000;

	struct anatop_regs __iomem *anatop =
		(struct anatop_regs __iomem *)ANATOP_BASE_ADDR;

	if (freq < ENET_25MHZ || freq > ENET_125MHZ)
		return -EINVAL;

	reg = readl(&anatop->pll_enet);

	if (fec_id == 0) {
		reg &= ~BM_ANADIG_PLL_ENET_DIV_SELECT;
		reg |= BF_ANADIG_PLL_ENET_DIV_SELECT(freq);
	} else if (fec_id == 1) {
		/* Only i.MX6SX/UL support ENET2 */
		if (!(is_mx6sx() || is_mx6ul() || is_mx6ull()))
			return -EINVAL;
		reg &= ~BM_ANADIG_PLL_ENET2_DIV_SELECT;
		reg |= BF_ANADIG_PLL_ENET2_DIV_SELECT(freq);
	} else {
		return -EINVAL;
	}

	if ((reg & BM_ANADIG_PLL_ENET_POWERDOWN) ||
	    (!(reg & BM_ANADIG_PLL_ENET_LOCK))) {
		reg &= ~BM_ANADIG_PLL_ENET_POWERDOWN;
		writel(reg, &anatop->pll_enet);
		while (timeout--) {
			if (readl(&anatop->pll_enet) & BM_ANADIG_PLL_ENET_LOCK)
				break;
		}
		if (timeout < 0)
			return -ETIMEDOUT;
	}

	/* Enable FEC clock */
	if (fec_id == 0)
		reg |= BM_ANADIG_PLL_ENET_ENABLE;
	else
		reg |= BM_ANADIG_PLL_ENET2_ENABLE;
	reg &= ~BM_ANADIG_PLL_ENET_BYPASS;
	writel(reg, &anatop->pll_enet);

#ifdef CONFIG_MX6SX
	/* Disable enet system clcok before switching clock parent */
	reg = readl(&imx_ccm->CCGR3);
	reg &= ~MXC_CCM_CCGR3_ENET_MASK;
	writel(reg, &imx_ccm->CCGR3);

	/*
	 * Set enet ahb clock to 200MHz
	 * pll2_pfd2_396m-> ENET_PODF-> ENET_AHB
	 */
	reg = readl(&imx_ccm->chsccdr);
	reg &= ~(MXC_CCM_CHSCCDR_ENET_PRE_CLK_SEL_MASK
		 | MXC_CCM_CHSCCDR_ENET_PODF_MASK
		 | MXC_CCM_CHSCCDR_ENET_CLK_SEL_MASK);
	/* PLL2 PFD2 */
	reg |= (4 << MXC_CCM_CHSCCDR_ENET_PRE_CLK_SEL_OFFSET);
	/* Div = 2*/
	reg |= (1 << MXC_CCM_CHSCCDR_ENET_PODF_OFFSET);
	reg |= (0 << MXC_CCM_CHSCCDR_ENET_CLK_SEL_OFFSET);
	writel(reg, &imx_ccm->chsccdr);

	/* Enable enet system clock */
	reg = readl(&imx_ccm->CCGR3);
	reg |= MXC_CCM_CCGR3_ENET_MASK;
	writel(reg, &imx_ccm->CCGR3);
#endif
	return 0;
}
#endif

static u32 get_usdhc_clk(u32 port)
{
	u32 root_freq = 0, usdhc_podf = 0, clk_sel = 0;
	u32 cscmr1 = __raw_readl(&imx_ccm->cscmr1);
	u32 cscdr1 = __raw_readl(&imx_ccm->cscdr1);

	if (is_mx6ul() || is_mx6ull()) {
		if (port > 1)
			return 0;
	}

	if (is_mx6sll()) {
		if (port > 2)
			return 0;
	}

	switch (port) {
	case 0:
		usdhc_podf = (cscdr1 & MXC_CCM_CSCDR1_USDHC1_PODF_MASK) >>
					MXC_CCM_CSCDR1_USDHC1_PODF_OFFSET;
		clk_sel = cscmr1 & MXC_CCM_CSCMR1_USDHC1_CLK_SEL;

		break;
	case 1:
		usdhc_podf = (cscdr1 & MXC_CCM_CSCDR1_USDHC2_PODF_MASK) >>
					MXC_CCM_CSCDR1_USDHC2_PODF_OFFSET;
		clk_sel = cscmr1 & MXC_CCM_CSCMR1_USDHC2_CLK_SEL;

		break;
	case 2:
		usdhc_podf = (cscdr1 & MXC_CCM_CSCDR1_USDHC3_PODF_MASK) >>
					MXC_CCM_CSCDR1_USDHC3_PODF_OFFSET;
		clk_sel = cscmr1 & MXC_CCM_CSCMR1_USDHC3_CLK_SEL;

		break;
	case 3:
		usdhc_podf = (cscdr1 & MXC_CCM_CSCDR1_USDHC4_PODF_MASK) >>
					MXC_CCM_CSCDR1_USDHC4_PODF_OFFSET;
		clk_sel = cscmr1 & MXC_CCM_CSCMR1_USDHC4_CLK_SEL;

		break;
	default:
		break;
	}

	if (clk_sel)
		root_freq = mxc_get_pll_pfd(PLL_BUS, 0);
	else
		root_freq = mxc_get_pll_pfd(PLL_BUS, 2);

	return root_freq / (usdhc_podf + 1);
}

u32 imx_get_uartclk(void)
{
	return get_uart_clk();
}

u32 imx_get_fecclk(void)
{
	return mxc_get_clock(MXC_IPG_CLK);
}

#if defined(CONFIG_SATA) || defined(CONFIG_PCIE_IMX)
static int enable_enet_pll(uint32_t en)
{
	struct mxc_ccm_reg *const imx_ccm
		= (struct mxc_ccm_reg *) CCM_BASE_ADDR;
	s32 timeout = 100000;
	u32 reg = 0;

	/* Enable PLLs */
	reg = readl(&imx_ccm->analog_pll_enet);
	reg &= ~BM_ANADIG_PLL_SYS_POWERDOWN;
	writel(reg, &imx_ccm->analog_pll_enet);
	reg |= BM_ANADIG_PLL_SYS_ENABLE;
	while (timeout--) {
		if (readl(&imx_ccm->analog_pll_enet) & BM_ANADIG_PLL_SYS_LOCK)
			break;
	}
	if (timeout <= 0)
		return -EIO;
	reg &= ~BM_ANADIG_PLL_SYS_BYPASS;
	writel(reg, &imx_ccm->analog_pll_enet);
	reg |= en;
	writel(reg, &imx_ccm->analog_pll_enet);
	return 0;
}
#endif

#ifdef CONFIG_SATA
static void ungate_sata_clock(void)
{
	struct mxc_ccm_reg *const imx_ccm =
		(struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* Enable SATA clock. */
	setbits_le32(&imx_ccm->CCGR5, MXC_CCM_CCGR5_SATA_MASK);
}

int enable_sata_clock(void)
{
	ungate_sata_clock();
	return enable_enet_pll(BM_ANADIG_PLL_ENET_ENABLE_SATA);
}

void disable_sata_clock(void)
{
	struct mxc_ccm_reg *const imx_ccm =
		(struct mxc_ccm_reg *)CCM_BASE_ADDR;

	clrbits_le32(&imx_ccm->CCGR5, MXC_CCM_CCGR5_SATA_MASK);
}
#endif

#ifdef CONFIG_PCIE_IMX
static void ungate_pcie_clock(void)
{
	struct mxc_ccm_reg *const imx_ccm =
		(struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* Enable PCIe clock. */
	setbits_le32(&imx_ccm->CCGR4, MXC_CCM_CCGR4_PCIE_MASK);
}

int enable_pcie_clock(void)
{
	struct anatop_regs *anatop_regs =
		(struct anatop_regs *)ANATOP_BASE_ADDR;
	struct mxc_ccm_reg *ccm_regs = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	u32 lvds1_clk_sel;

	/*
	 * Here be dragons!
	 *
	 * The register ANATOP_MISC1 is not documented in the Freescale
	 * MX6RM. The register that is mapped in the ANATOP space and
	 * marked as ANATOP_MISC1 is actually documented in the PMU section
	 * of the datasheet as PMU_MISC1.
	 *
	 * Switch LVDS clock source to SATA (0xb) on mx6q/dl or PCI (0xa) on
	 * mx6sx, disable clock INPUT and enable clock OUTPUT. This is important
	 * for PCI express link that is clocked from the i.MX6.
	 */
#define ANADIG_ANA_MISC1_LVDSCLK1_IBEN		(1 << 12)
#define ANADIG_ANA_MISC1_LVDSCLK1_OBEN		(1 << 10)
#define ANADIG_ANA_MISC1_LVDS1_CLK_SEL_MASK	0x0000001F
#define ANADIG_ANA_MISC1_LVDS1_CLK_SEL_PCIE_REF	0xa
#define ANADIG_ANA_MISC1_LVDS1_CLK_SEL_SATA_REF	0xb

	if (is_mx6sx())
		lvds1_clk_sel = ANADIG_ANA_MISC1_LVDS1_CLK_SEL_PCIE_REF;
	else
		lvds1_clk_sel = ANADIG_ANA_MISC1_LVDS1_CLK_SEL_SATA_REF;

	clrsetbits_le32(&anatop_regs->ana_misc1,
			ANADIG_ANA_MISC1_LVDSCLK1_IBEN |
			ANADIG_ANA_MISC1_LVDS1_CLK_SEL_MASK,
			ANADIG_ANA_MISC1_LVDSCLK1_OBEN | lvds1_clk_sel);

	/* PCIe reference clock sourced from AXI. */
	clrbits_le32(&ccm_regs->cbcmr, MXC_CCM_CBCMR_PCIE_AXI_CLK_SEL);

	/* Party time! Ungate the clock to the PCIe. */
#ifdef CONFIG_SATA
	ungate_sata_clock();
#endif
	ungate_pcie_clock();

	return enable_enet_pll(BM_ANADIG_PLL_ENET_ENABLE_SATA |
			       BM_ANADIG_PLL_ENET_ENABLE_PCIE);
}
#endif

#ifdef CONFIG_SECURE_BOOT
void hab_caam_clock_enable(unsigned char enable)
{
	u32 reg;

	if (is_mx6ull() || is_mx6sll()) {
		/* CG5, DCP clock */
		reg = __raw_readl(&imx_ccm->CCGR0);
		if (enable)
			reg |= MXC_CCM_CCGR0_DCP_CLK_MASK;
		else
			reg &= ~MXC_CCM_CCGR0_DCP_CLK_MASK;
		__raw_writel(reg, &imx_ccm->CCGR0);
	} else {
		/* CG4 ~ CG6, CAAM clocks */
		reg = __raw_readl(&imx_ccm->CCGR0);
		if (enable)
			reg |= (MXC_CCM_CCGR0_CAAM_WRAPPER_IPG_MASK |
				MXC_CCM_CCGR0_CAAM_WRAPPER_ACLK_MASK |
				MXC_CCM_CCGR0_CAAM_SECURE_MEM_MASK);
		else
			reg &= ~(MXC_CCM_CCGR0_CAAM_WRAPPER_IPG_MASK |
				MXC_CCM_CCGR0_CAAM_WRAPPER_ACLK_MASK |
				MXC_CCM_CCGR0_CAAM_SECURE_MEM_MASK);
		__raw_writel(reg, &imx_ccm->CCGR0);
	}

	/* EMI slow clk */
	reg = __raw_readl(&imx_ccm->CCGR6);
	if (enable)
		reg |= MXC_CCM_CCGR6_EMI_SLOW_MASK;
	else
		reg &= ~MXC_CCM_CCGR6_EMI_SLOW_MASK;
	__raw_writel(reg, &imx_ccm->CCGR6);
}
#endif

static void enable_pll3(void)
{
	struct anatop_regs __iomem *anatop =
		(struct anatop_regs __iomem *)ANATOP_BASE_ADDR;

	/* make sure pll3 is enabled */
	if ((readl(&anatop->usb1_pll_480_ctrl) &
			BM_ANADIG_USB1_PLL_480_CTRL_LOCK) == 0) {
		/* enable pll's power */
		writel(BM_ANADIG_USB1_PLL_480_CTRL_POWER,
		       &anatop->usb1_pll_480_ctrl_set);
		writel(0x80, &anatop->ana_misc2_clr);
		/* wait for pll lock */
		while ((readl(&anatop->usb1_pll_480_ctrl) &
			BM_ANADIG_USB1_PLL_480_CTRL_LOCK) == 0)
			;
		/* disable bypass */
		writel(BM_ANADIG_USB1_PLL_480_CTRL_BYPASS,
		       &anatop->usb1_pll_480_ctrl_clr);
		/* enable pll output */
		writel(BM_ANADIG_USB1_PLL_480_CTRL_ENABLE,
		       &anatop->usb1_pll_480_ctrl_set);
	}
}

void enable_thermal_clk(void)
{
	enable_pll3();
}

#ifdef CONFIG_MTD_NOR_FLASH
void enable_eim_clk(unsigned char enable)
{
	u32 reg;

	reg = __raw_readl(&imx_ccm->CCGR6);
	if (enable)
		reg |= MXC_CCM_CCGR6_EMI_SLOW_MASK;
	else
		reg &= ~MXC_CCM_CCGR6_EMI_SLOW_MASK;
	__raw_writel(reg, &imx_ccm->CCGR6);
}
#endif

unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_ARM_CLK:
		return get_mcu_main_clk();
	case MXC_PER_CLK:
		return get_periph_clk();
	case MXC_AHB_CLK:
		return get_ahb_clk();
	case MXC_IPG_CLK:
		return get_ipg_clk();
	case MXC_IPG_PERCLK:
	case MXC_I2C_CLK:
		return get_ipg_per_clk();
	case MXC_UART_CLK:
		return get_uart_clk();
	case MXC_CSPI_CLK:
		return get_cspi_clk();
	case MXC_AXI_CLK:
		return get_axi_clk();
	case MXC_EMI_SLOW_CLK:
		return get_emi_slow_clk();
	case MXC_DDR_CLK:
		return get_mmdc_ch0_clk();
	case MXC_ESDHC_CLK:
		return get_usdhc_clk(0);
	case MXC_ESDHC2_CLK:
		return get_usdhc_clk(1);
	case MXC_ESDHC3_CLK:
		return get_usdhc_clk(2);
	case MXC_ESDHC4_CLK:
		return get_usdhc_clk(3);
	case MXC_SATA_CLK:
		return get_ahb_clk();
	default:
		printf("Unsupported MXC CLK: %d\n", clk);
		break;
	}

	return 0;
}

#ifndef CONFIG_SPL_BUILD
/*
 * Dump some core clockes.
 */
int do_mx6_showclocks(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 freq;
	freq = decode_pll(PLL_SYS, MXC_HCLK);
	printf("PLL_SYS    %8d MHz\n", freq / 1000000);
	freq = decode_pll(PLL_BUS, MXC_HCLK);
	printf("PLL_BUS    %8d MHz\n", freq / 1000000);
	freq = decode_pll(PLL_USBOTG, MXC_HCLK);
	printf("PLL_OTG    %8d MHz\n", freq / 1000000);
	freq = decode_pll(PLL_ENET, MXC_HCLK);
	printf("PLL_NET    %8d MHz\n", freq / 1000000);

	printf("\n");
	printf("ARM        %8d kHz\n", mxc_get_clock(MXC_ARM_CLK) / 1000);
	printf("IPG        %8d kHz\n", mxc_get_clock(MXC_IPG_CLK) / 1000);
	printf("UART       %8d kHz\n", mxc_get_clock(MXC_UART_CLK) / 1000);
#ifdef CONFIG_MXC_SPI
	printf("CSPI       %8d kHz\n", mxc_get_clock(MXC_CSPI_CLK) / 1000);
#endif
	printf("AHB        %8d kHz\n", mxc_get_clock(MXC_AHB_CLK) / 1000);
	printf("AXI        %8d kHz\n", mxc_get_clock(MXC_AXI_CLK) / 1000);
	printf("DDR        %8d kHz\n", mxc_get_clock(MXC_DDR_CLK) / 1000);
	printf("USDHC1     %8d kHz\n", mxc_get_clock(MXC_ESDHC_CLK) / 1000);
	printf("USDHC2     %8d kHz\n", mxc_get_clock(MXC_ESDHC2_CLK) / 1000);
	printf("USDHC3     %8d kHz\n", mxc_get_clock(MXC_ESDHC3_CLK) / 1000);
	printf("USDHC4     %8d kHz\n", mxc_get_clock(MXC_ESDHC4_CLK) / 1000);
	printf("EMI SLOW   %8d kHz\n", mxc_get_clock(MXC_EMI_SLOW_CLK) / 1000);
	printf("IPG PERCLK %8d kHz\n", mxc_get_clock(MXC_IPG_PERCLK) / 1000);

	return 0;
}

#ifndef CONFIG_MX6SX
void enable_ipu_clock(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	int reg;
	reg = readl(&mxc_ccm->CCGR3);
	reg |= MXC_CCM_CCGR3_IPU1_IPU_MASK;
	writel(reg, &mxc_ccm->CCGR3);

	if (is_mx6dqp()) {
		setbits_le32(&mxc_ccm->CCGR6, MXC_CCM_CCGR6_PRG_CLK0_MASK);
		setbits_le32(&mxc_ccm->CCGR3, MXC_CCM_CCGR3_IPU2_IPU_MASK);
	}
}
#endif

#if defined(CONFIG_MX6Q) || defined(CONFIG_MX6D) || defined(CONFIG_MX6DL) || \
	defined(CONFIG_MX6S)
static void disable_ldb_di_clock_sources(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	int reg;

	/* Make sure PFDs are disabled at boot. */
	reg = readl(&mxc_ccm->analog_pfd_528);
	/* Cannot disable pll2_pfd2_396M, as it is the MMDC clock in iMX6DL */
	if (is_mx6sdl())
		reg |= 0x80008080;
	else
		reg |= 0x80808080;
	writel(reg, &mxc_ccm->analog_pfd_528);

	/* Disable PLL3 PFDs */
	reg = readl(&mxc_ccm->analog_pfd_480);
	reg |= 0x80808080;
	writel(reg, &mxc_ccm->analog_pfd_480);

	/* Disable PLL5 */
	reg = readl(&mxc_ccm->analog_pll_video);
	reg &= ~(1 << 13);
	writel(reg, &mxc_ccm->analog_pll_video);
}

static void enable_ldb_di_clock_sources(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	int reg;

	reg = readl(&mxc_ccm->analog_pfd_528);
	if (is_mx6sdl())
		reg &= ~(0x80008080);
	else
		reg &= ~(0x80808080);
	writel(reg, &mxc_ccm->analog_pfd_528);

	reg = readl(&mxc_ccm->analog_pfd_480);
	reg &= ~(0x80808080);
	writel(reg, &mxc_ccm->analog_pfd_480);
}

/*
 * Try call this function as early in the boot process as possible since the
 * function temporarily disables PLL2 PFD's, PLL3 PFD's and PLL5.
 */
void select_ldb_di_clock_source(enum ldb_di_clock clk)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	int reg;

	/*
	 * Need to follow a strict procedure when changing the LDB
	 * clock, else we can introduce a glitch. Things to keep in
	 * mind:
	 * 1. The current and new parent clocks must be disabled.
	 * 2. The default clock for ldb_dio_clk is mmdc_ch1 which has
	 * no CG bit.
	 * 3. In the RTL implementation of the LDB_DI_CLK_SEL mux
	 * the top four options are in one mux and the PLL3 option along
	 * with another option is in the second mux. There is third mux
	 * used to decide between the first and second mux.
	 * The code below switches the parent to the bottom mux first
	 * and then manipulates the top mux. This ensures that no glitch
	 * will enter the divider.
	 *
	 * Need to disable MMDC_CH1 clock manually as there is no CG bit
	 * for this clock. The only way to disable this clock is to move
	 * it to pll3_sw_clk and then to disable pll3_sw_clk
	 * Make sure periph2_clk2_sel is set to pll3_sw_clk
	 */

	/* Disable all ldb_di clock parents */
	disable_ldb_di_clock_sources();

	/* Set MMDC_CH1 mask bit */
	reg = readl(&mxc_ccm->ccdr);
	reg |= MXC_CCM_CCDR_MMDC_CH1_HS_MASK;
	writel(reg, &mxc_ccm->ccdr);

	/* Set periph2_clk2_sel to be sourced from PLL3_sw_clk */
	reg = readl(&mxc_ccm->cbcmr);
	reg &= ~MXC_CCM_CBCMR_PERIPH2_CLK2_SEL;
	writel(reg, &mxc_ccm->cbcmr);

	/*
	 * Set the periph2_clk_sel to the top mux so that
	 * mmdc_ch1 is from pll3_sw_clk.
	 */
	reg = readl(&mxc_ccm->cbcdr);
	reg |= MXC_CCM_CBCDR_PERIPH2_CLK_SEL;
	writel(reg, &mxc_ccm->cbcdr);

	/* Wait for the clock switch */
	while (readl(&mxc_ccm->cdhipr))
		;
	/* Disable pll3_sw_clk by selecting bypass clock source */
	reg = readl(&mxc_ccm->ccsr);
	reg |= MXC_CCM_CCSR_PLL3_SW_CLK_SEL;
	writel(reg, &mxc_ccm->ccsr);

	/* Set the ldb_di0_clk and ldb_di1_clk to 111b */
	reg = readl(&mxc_ccm->cs2cdr);
	reg |= ((7 << MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET)
	      | (7 << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET));
	writel(reg, &mxc_ccm->cs2cdr);

	/* Set the ldb_di0_clk and ldb_di1_clk to 100b */
	reg = readl(&mxc_ccm->cs2cdr);
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK
	      | MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK);
	reg |= ((4 << MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET)
	      | (4 << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET));
	writel(reg, &mxc_ccm->cs2cdr);

	/* Set the ldb_di0_clk and ldb_di1_clk to desired source */
	reg = readl(&mxc_ccm->cs2cdr);
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK
	      | MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK);
	reg |= ((clk << MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET)
	      | (clk << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET));
	writel(reg, &mxc_ccm->cs2cdr);

	/* Unbypass pll3_sw_clk */
	reg = readl(&mxc_ccm->ccsr);
	reg &= ~MXC_CCM_CCSR_PLL3_SW_CLK_SEL;
	writel(reg, &mxc_ccm->ccsr);

	/*
	 * Set the periph2_clk_sel back to the bottom mux so that
	 * mmdc_ch1 is from its original parent.
	 */
	reg = readl(&mxc_ccm->cbcdr);
	reg &= ~MXC_CCM_CBCDR_PERIPH2_CLK_SEL;
	writel(reg, &mxc_ccm->cbcdr);

	/* Wait for the clock switch */
	while (readl(&mxc_ccm->cdhipr))
		;
	/* Clear MMDC_CH1 mask bit */
	reg = readl(&mxc_ccm->ccdr);
	reg &= ~MXC_CCM_CCDR_MMDC_CH1_HS_MASK;
	writel(reg, &mxc_ccm->ccdr);

	enable_ldb_di_clock_sources();
}
#endif

/***************************************************/

U_BOOT_CMD(
	clocks,	CONFIG_SYS_MAXARGS, 1, do_mx6_showclocks,
	"display clocks",
	""
);
#endif
