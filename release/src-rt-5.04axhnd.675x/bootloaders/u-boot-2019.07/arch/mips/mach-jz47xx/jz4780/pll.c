// SPDX-License-Identifier: GPL-2.0+
/*
 * JZ4780 PLL setup
 *
 * Copyright (c) 2013 Imagination Technologies
 * Author: Paul Burton <paul.burton@imgtec.com>
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <mach/jz4780.h>

#define CPM_CPCCR		0x00
#define CPM_LCR			0x04
#define CPM_RSR			0x08
#define CPM_CPPCR		0x0c
#define CPM_CPAPCR		0x10
#define CPM_CPMPCR		0x14
#define CPM_CPEPCR		0x18
#define CPM_CPVPCR		0x1c
#define CPM_CLKGR0		0x20
#define CPM_OPCR		0x24
#define CPM_CLKGR1		0x28
#define CPM_DDCDR		0x2c
#define CPM_VPUCDR		0x30
#define CPM_CPSPR		0x34
#define CPM_CPSPPR		0x38
#define CPM_USBPCR		0x3c
#define CPM_USBRDT		0x40
#define CPM_USBVBFIL		0x44
#define CPM_USBPCR1		0x48
#define CPM_USBCDR		0x50
#define CPM_LPCDR		0x54
#define CPM_I2SCDR		0x60
#define CPM_LPCDR1		0x64
#define CPM_MSCCDR		0x68
#define CPM_UHCCDR		0x6c
#define CPM_SSICDR		0x74
#define CPM_CIMCDR		0x7c
#define CPM_PCMCDR		0x84
#define CPM_GPUCDR		0x88
#define CPM_HDMICDR		0x8c
#define CPM_I2S1CDR		0xa0
#define CPM_MSCCDR1		0xa4
#define CPM_MSCCDR2		0xa8
#define CPM_BCHCDR		0xac
#define CPM_SPCR0		0xb8
#define CPM_SPCR1		0xbc
#define CPM_CPCSR		0xd4
#define CPM_PSWCST(n)		((0x4 * (n)) + 0x90)

/* Clock control register */
#define CPM_CPCCR_SEL_SRC_BIT		30
#define CPM_CPCCR_SEL_SRC_MASK		(0x3 << CPM_CPCCR_SEL_SRC_BIT)
#define CPM_SRC_SEL_STOP		0
#define CPM_SRC_SEL_APLL		1
#define CPM_SRC_SEL_EXCLK		2
#define CPM_SRC_SEL_RTCLK		3
#define CPM_CPCCR_SEL_CPLL_BIT		28
#define CPM_CPCCR_SEL_CPLL_MASK		(0x3 << CPM_CPCCR_SEL_CPLL_BIT)
#define CPM_CPCCR_SEL_H0PLL_BIT		26
#define CPM_CPCCR_SEL_H0PLL_MASK	(0x3 << CPM_CPCCR_SEL_H0PLL_BIT)
#define CPM_CPCCR_SEL_H2PLL_BIT		24
#define CPM_CPCCR_SEL_H2PLL_MASK	(0x3 << CPM_CPCCR_SEL_H2PLL_BIT)
#define CPM_PLL_SEL_STOP		0
#define CPM_PLL_SEL_SRC			1
#define CPM_PLL_SEL_MPLL		2
#define CPM_PLL_SEL_EPLL		3
#define CPM_CPCCR_CE_CPU		(0x1 << 22)
#define CPM_CPCCR_CE_AHB0		(0x1 << 21)
#define CPM_CPCCR_CE_AHB2		(0x1 << 20)
#define CPM_CPCCR_PDIV_BIT		16
#define CPM_CPCCR_PDIV_MASK		(0xf << CPM_CPCCR_PDIV_BIT)
#define CPM_CPCCR_H2DIV_BIT		12
#define CPM_CPCCR_H2DIV_MASK		(0xf << CPM_CPCCR_H2DIV_BIT)
#define CPM_CPCCR_H0DIV_BIT		8
#define CPM_CPCCR_H0DIV_MASK		(0x0f << CPM_CPCCR_H0DIV_BIT)
#define CPM_CPCCR_L2DIV_BIT		4
#define CPM_CPCCR_L2DIV_MASK		(0x0f << CPM_CPCCR_L2DIV_BIT)
#define CPM_CPCCR_CDIV_BIT		0
#define CPM_CPCCR_CDIV_MASK		(0x0f << CPM_CPCCR_CDIV_BIT)

/* Clock Status register */
#define CPM_CPCSR_H2DIV_BUSY		BIT(2)
#define CPM_CPCSR_H0DIV_BUSY		BIT(1)
#define CPM_CPCSR_CDIV_BUSY		BIT(0)

/* PLL control register */
#define CPM_CPPCR_PLLST_BIT		0
#define CPM_CPPCR_PLLST_MASK		(0xff << CPM_CPPCR_PLLST_BIT)

/* XPLL control register */
#define CPM_CPXPCR_XPLLM_BIT		19
#define CPM_CPXPCR_XPLLM_MASK		(0x1fff << CPM_CPXPCR_XPLLM_BIT)
#define CPM_CPXPCR_XPLLN_BIT		13
#define CPM_CPXPCR_XPLLN_MASK		(0x3f << CPM_CPXPCR_XPLLN_BIT)
#define CPM_CPXPCR_XPLLOD_BIT		9
#define CPM_CPXPCR_XPLLOD_MASK		(0xf << CPM_CPXPCR_XPLLOD_BIT)
#define CPM_CPXPCR_XLOCK		BIT(6)
#define CPM_CPXPCR_XPLL_ON		BIT(4)
#define CPM_CPXPCR_XF_MODE		BIT(3)
#define CPM_CPXPCR_XPLLBP		BIT(1)
#define CPM_CPXPCR_XPLLEN		BIT(0)

/* CPM scratch protected register */
#define CPM_CPSPPR_BIT			0
#define CPM_CPSPPR_MASK			(0xffff << CPM_CPSPPR_BIT)

/* USB parameter control register */
#define CPM_USBPCR_USB_MODE		BIT(31)  /* 1: OTG, 0: UDC*/
#define CPM_USBPCR_AVLD_REG		BIT(30)
#define CPM_USBPCR_IDPULLUP_MASK_BIT	28
#define CPM_USBPCR_IDPULLUP_MASK_MASK	(0x02 << IDPULLUP_MASK_BIT)
#define CPM_USBPCR_INCR_MASK		BIT(27)
#define CPM_USBPCR_CLK12_EN		BIT(26)
#define CPM_USBPCR_COMMONONN		BIT(25)
#define CPM_USBPCR_VBUSVLDEXT		BIT(24)
#define CPM_USBPCR_VBUSVLDEXTSEL	BIT(23)
#define CPM_USBPCR_POR			BIT(22)
#define CPM_USBPCR_SIDDQ		BIT(21)
#define CPM_USBPCR_OTG_DISABLE		BIT(20)
#define CPM_USBPCR_COMPDISTUNE_BIT	17
#define CPM_USBPCR_COMPDISTUNE_MASK	(0x07 << COMPDISTUNE_BIT)
#define CPM_USBPCR_OTGTUNE_BIT		14
#define CPM_USBPCR_OTGTUNE_MASK		(0x07 << OTGTUNE_BIT)
#define CPM_USBPCR_SQRXTUNE_BIT		11
#define CPM_USBPCR_SQRXTUNE_MASK	(0x7x << SQRXTUNE_BIT)
#define CPM_USBPCR_TXFSLSTUNE_BIT	7
#define CPM_USBPCR_TXFSLSTUNE_MASK	(0x0f << TXFSLSTUNE_BIT)
#define CPM_USBPCR_TXPREEMPHTUNE	BIT(6)
#define CPM_USBPCR_TXRISETUNE_BIT	4
#define CPM_USBPCR_TXRISETUNE_MASK	(0x03 << TXRISETUNE_BIT)
#define CPM_USBPCR_TXVREFTUNE_BIT	0
#define CPM_USBPCR_TXVREFTUNE_MASK	(0x0f << TXVREFTUNE_BIT)

/* DDR memory clock divider register */
#define CPM_DDRCDR_DCS_BIT		30
#define CPM_DDRCDR_DCS_MASK		(0x3 << CPM_DDRCDR_DCS_BIT)
#define CPM_DDRCDR_DCS_STOP		(0x0 << CPM_DDRCDR_DCS_BIT)
#define CPM_DDRCDR_DCS_SRC		(0x1 << CPM_DDRCDR_DCS_BIT)
#define CPM_DDRCDR_DCS_MPLL		(0x2 << CPM_DDRCDR_DCS_BIT)
#define CPM_DDRCDR_CE_DDR		BIT(29)
#define CPM_DDRCDR_DDR_BUSY		BIT(28)
#define CPM_DDRCDR_DDR_STOP		BIT(27)
#define CPM_DDRCDR_DDRDIV_BIT		0
#define CPM_DDRCDR_DDRDIV_MASK		(0xf << CPM_DDRCDR_DDRDIV_BIT)

/* USB reset detect timer register */
#define CPM_USBRDT_VBFIL_LD_EN		BIT(25)
#define CPM_USBRDT_IDDIG_EN		BIT(24)
#define CPM_USBRDT_IDDIG_REG		BIT(23)
#define CPM_USBRDT_USBRDT_BIT		0
#define CPM_USBRDT_USBRDT_MASK		(0x7fffff << CPM_USBRDT_USBRDT_BIT)

/* USB OTG PHY clock divider register */
#define CPM_USBCDR_UCS			BIT(31)
#define CPM_USBCDR_UPCS			BIT(30)
#define CPM_USBCDR_CEUSB		BIT(29)
#define CPM_USBCDR_USB_BUSY		BIT(28)
#define CPM_USBCDR_OTGDIV_BIT		0
#define CPM_USBCDR_OTGDIV_MASK		(0xff << CPM_USBCDR_OTGDIV_BIT)

/* I2S device clock divider register */
#define CPM_I2SCDR_I2CS			BIT(31)
#define CPM_I2SCDR_I2PCS		BIT(30)
#define CPM_I2SCDR_I2SDIV_BIT		0
#define CPM_I2SCDR_I2SDIV_MASK		(0x1ff << CPM_I2SCDR_I2SDIV_BIT)

/* LCD0 pix clock divider register */
#define CPM_LPCDR_LPCS_BIT		30
#define CPM_LPCDR_LPCS_MASK		(0x3 << CPM_LPCDR_LPCS_BIT)
#define CPM_LPCDR_CELCD			BIT(28)
#define CPM_LPCDR_LCD_BUSY		BIT(27)
#define CPM_LPCDR_LCD_STOP		BIT(26)
#define CPM_LPCDR_PIXDIV_BIT		0
#define CPM_LPCDR_PIXDIV_MASK		(0xff << CPM_LPCDR_PIXDIV_BIT)

/* MSC clock divider register */
#define CPM_MSCCDR_MPCS_BIT		30
#define CPM_MSCCDR_MPCS_MASK		(3 << CPM_MSCCDR_MPCS_BIT)
#define CPM_MSCCDR_MPCS_STOP		(0x0 << CPM_MSCCDR_MPCS_BIT)
#define CPM_MSCCDR_MPCS_SRC		(0x1 << CPM_MSCCDR_MPCS_BIT)
#define CPM_MSCCDR_MPCS_MPLL		(0x2 << CPM_MSCCDR_MPCS_BIT)
#define CPM_MSCCDR_CE			BIT(29)
#define CPM_MSCCDR_MSC_BUSY		BIT(28)
#define CPM_MSCCDR_MSC_STOP		BIT(27)
#define CPM_MSCCDR_MSC_CLK0_SEL		BIT(15)
#define CPM_MSCCDR_MSCDIV_BIT		0
#define CPM_MSCCDR_MSCDIV_MASK		(0xff << CPM_MSCCDR_MSCDIV_BIT)

/* UHC 48M clock divider register */
#define CPM_UHCCDR_UHCS_BIT		30
#define CPM_UHCCDR_UHCS_MASK		(0x3 << CPM_UHCCDR_UHCS_BIT)
#define CPM_UHCCDR_UHCS_SRC		(0x0 << CPM_UHCCDR_UHCS_BIT)
#define CPM_UHCCDR_UHCS_MPLL		(0x1 << CPM_UHCCDR_UHCS_BIT)
#define CPM_UHCCDR_UHCS_EPLL		(0x2 << CPM_UHCCDR_UHCS_BIT)
#define CPM_UHCCDR_UHCS_OTG		(0x3 << CPM_UHCCDR_UHCS_BIT)
#define CPM_UHCCDR_CE_UHC		BIT(29)
#define CPM_UHCCDR_UHC_BUSY		BIT(28)
#define CPM_UHCCDR_UHC_STOP		BIT(27)
#define CPM_UHCCDR_UHCDIV_BIT		0
#define CPM_UHCCDR_UHCDIV_MASK		(0xff << CPM_UHCCDR_UHCDIV_BIT)

/* SSI clock divider register */
#define CPM_SSICDR_SCS			BIT(31)
#define CPM_SSICDR_SSIDIV_BIT		0
#define CPM_SSICDR_SSIDIV_MASK		(0x3f << CPM_SSICDR_SSIDIV_BIT)

/* CIM MCLK clock divider register */
#define CPM_CIMCDR_CIMDIV_BIT		0
#define CPM_CIMCDR_CIMDIV_MASK		(0xff << CPM_CIMCDR_CIMDIV_BIT)

/* GPS clock divider register */
#define CPM_GPSCDR_GPCS			BIT(31)
#define CPM_GPSCDR_GPSDIV_BIT		0
#define CPM_GSPCDR_GPSDIV_MASK		(0xf << CPM_GPSCDR_GPSDIV_BIT)

/* PCM device clock divider register */
#define CPM_PCMCDR_PCMS			BIT(31)
#define CPM_PCMCDR_PCMPCS		BIT(30)
#define CPM_PCMCDR_PCMDIV_BIT		0
#define CPM_PCMCDR_PCMDIV_MASK		(0x1ff << CPM_PCMCDR_PCMDIV_BIT)

/* GPU clock divider register */
#define CPM_GPUCDR_GPCS			BIT(31)
#define CPM_GPUCDR_GPUDIV_BIT		0
#define CPM_GPUCDR_GPUDIV_MASK		(0x7 << CPM_GPUCDR_GPUDIV_BIT)

/* HDMI clock divider register */
#define CPM_HDMICDR_HPCS_BIT		30
#define CPM_HDMICDR_HPCS_MASK		(0x3 << CPM_HDMICDR_HPCS_BIT)
#define CPM_HDMICDR_CEHDMI		BIT(29)
#define CPM_HDMICDR_HDMI_BUSY		BIT(28)
#define CPM_HDMICDR_HDMI_STOP		BIT(26)
#define CPM_HDMICDR_HDMIDIV_BIT		0
#define CPM_HDMICDR_HDMIDIV_MASK	(0xff << CPM_HDMICDR_HDMIDIV_BIT)

/* Low Power Control Register */
#define CPM_LCR_PD_SCPU			BIT(31)
#define CPM_LCR_PD_VPU			BIT(30)
#define CPM_LCR_PD_GPU			BIT(29)
#define CPM_LCR_PD_GPS			BIT(28)
#define CPM_LCR_SCPUS			BIT(27)
#define CPM_LCR_VPUS			BIT(26)
#define CPM_LCR_GPUS			BIT(25)
#define CPM_LCR_GPSS			BIT(24)
#define CPM_LCR_GPU_IDLE		BIT(20)
#define CPM_LCR_PST_BIT			8
#define CPM_LCR_PST_MASK		(0xfff << CPM_LCR_PST_BIT)
#define CPM_LCR_DOZE_DUTY_BIT		3
#define CPM_LCR_DOZE_DUTY_MASK		(0x1f << CPM_LCR_DOZE_DUTY_BIT)
#define CPM_LCR_DOZE_ON			BIT(2)
#define CPM_LCR_LPM_BIT			0
#define CPM_LCR_LPM_MASK		(0x3 << CPM_LCR_LPM_BIT)
#define CPM_LCR_LPM_IDLE		(0x0 << CPM_LCR_LPM_BIT)
#define CPM_LCR_LPM_SLEEP		(0x1 << CPM_LCR_LPM_BIT)

/* Clock Gate Register0 */
#define CPM_CLKGR0_DDR1			BIT(31)
#define CPM_CLKGR0_DDR0			BIT(30)
#define CPM_CLKGR0_IPU			BIT(29)
#define CPM_CLKGR0_LCD1			BIT(28)
#define CPM_CLKGR0_LCD			BIT(27)
#define CPM_CLKGR0_CIM			BIT(26)
#define CPM_CLKGR0_I2C2			BIT(25)
#define CPM_CLKGR0_UHC			BIT(24)
#define CPM_CLKGR0_MAC			BIT(23)
#define CPM_CLKGR0_GPS			BIT(22)
#define CPM_CLKGR0_PDMAC		BIT(21)
#define CPM_CLKGR0_SSI2			BIT(20)
#define CPM_CLKGR0_SSI1			BIT(19)
#define CPM_CLKGR0_UART3		BIT(18)
#define CPM_CLKGR0_UART2		BIT(17)
#define CPM_CLKGR0_UART1		BIT(16)
#define CPM_CLKGR0_UART0		BIT(15)
#define CPM_CLKGR0_SADC			BIT(14)
#define CPM_CLKGR0_KBC			BIT(13)
#define CPM_CLKGR0_MSC2			BIT(12)
#define CPM_CLKGR0_MSC1			BIT(11)
#define CPM_CLKGR0_OWI			BIT(10)
#define CPM_CLKGR0_TSSI			BIT(9)
#define CPM_CLKGR0_AIC			BIT(8)
#define CPM_CLKGR0_SCC			BIT(7)
#define CPM_CLKGR0_I2C1			BIT(6)
#define CPM_CLKGR0_I2C0			BIT(5)
#define CPM_CLKGR0_SSI0			BIT(4)
#define CPM_CLKGR0_MSC0			BIT(3)
#define CPM_CLKGR0_OTG			BIT(2)
#define CPM_CLKGR0_BCH			BIT(1)
#define CPM_CLKGR0_NEMC			BIT(0)

/* Clock Gate Register1 */
#define CPM_CLKGR1_P1			BIT(15)
#define CPM_CLKGR1_X2D			BIT(14)
#define CPM_CLKGR1_DES			BIT(13)
#define CPM_CLKGR1_I2C4			BIT(12)
#define CPM_CLKGR1_AHB			BIT(11)
#define CPM_CLKGR1_UART4		BIT(10)
#define CPM_CLKGR1_HDMI			BIT(9)
#define CPM_CLKGR1_OTG1			BIT(8)
#define CPM_CLKGR1_GPVLC		BIT(7)
#define CPM_CLKGR1_AIC1			BIT(6)
#define CPM_CLKGR1_COMPRES		BIT(5)
#define CPM_CLKGR1_GPU			BIT(4)
#define CPM_CLKGR1_PCM			BIT(3)
#define CPM_CLKGR1_VPU			BIT(2)
#define CPM_CLKGR1_TSSI1		BIT(1)
#define CPM_CLKGR1_I2C3			BIT(0)

/* Oscillator and Power Control Register */
#define CPM_OPCR_O1ST_BIT		8
#define CPM_OPCR_O1ST_MASK		(0xff << CPM_OPCR_O1ST_BIT)
#define CPM_OPCR_SPENDN			BIT(7)
#define CPM_OPCR_GPSEN			BIT(6)
#define CPM_OPCR_SPENDH			BIT(5)
#define CPM_OPCR_O1SE			BIT(4)
#define CPM_OPCR_ERCS			BIT(2) /* 0: select EXCLK/512 clock, 1: RTCLK clock */
#define CPM_OPCR_USBM			BIT(0) /* 0: select EXCLK/512 clock, 1: RTCLK clock */

/* Reset Status Register */
#define CPM_RSR_P0R			BIT(2)
#define CPM_RSR_WR			BIT(1)
#define CPM_RSR_PR			BIT(0)

/* BCH clock divider register */
#define CPM_BCHCDR_BPCS_BIT		30
#define CPM_BCHCDR_BPCS_MASK		(0x3 << CPM_BCHCDR_BPCS_BIT)
#define CPM_BCHCDR_BPCS_STOP		(0X0 << CPM_BCHCDR_BPCS_BIT)
#define CPM_BCHCDR_BPCS_SRC_CLK		(0x1 << CPM_BCHCDR_BPCS_BIT)
#define CPM_BCHCDR_BPCS_MPLL		(0x2 << CPM_BCHCDR_BPCS_BIT)
#define CPM_BCHCDR_BPCS_EPLL		(0x3 << CPM_BCHCDR_BPCS_BIT)
#define CPM_BCHCDR_CE_BCH		BIT(29)
#define CPM_BCHCDR_BCH_BUSY		BIT(28)
#define CPM_BCHCDR_BCH_STOP		BIT(27)
#define CPM_BCHCDR_BCHCDR_BIT		0
#define CPM_BCHCDR_BCHCDR_MASK		(0x7 << CPM_BCHCDR_BCHCDR_BIT)

/* CPM scratch pad protected register(CPSPPR) */
#define CPSPPR_CPSPR_WRITABLE		0x00005a5a
#define RECOVERY_SIGNATURE		0x1a1a	/* means "RECY" */
#define RECOVERY_SIGNATURE_SEC		0x800	/* means "RECY" */

#define REBOOT_SIGNATURE		0x3535	/* means reboot */

/* XPLL control register */
#define XLOCK		(1 << 6)
#define XPLL_ON		(1 << 4)
#define XF_MODE		(1 << 3)
#define XPLLBP		(1 << 1)
#define XPLLEN		(1 << 0)

enum PLLS {
	EXTCLK = 0,
	APLL,
	MPLL,
	EPLL,
	VPLL,
};

#define M_N_OD(m, n, od)		\
		((((m) - 1) << 19) | (((n) - 1) << 13) | (((od) - 1) << 9))

struct cgu_pll_select {
	u8	reg;
	u8	pll;
	u8	pll_shift;
};

static void pll_init_one(int pll, int m, int n, int od)
{
	void __iomem *cpm_regs = (void __iomem *)CPM_BASE;
	void __iomem *pll_reg = cpm_regs + CPM_CPAPCR + ((pll - 1) * 4);

	setbits_le32(pll_reg, M_N_OD(m, n, od) | XPLLEN);

	/* FIXME */
	while (!(readl(pll_reg) & XPLL_ON))
		;
}

static void cpu_mux_select(int pll)
{
	void __iomem *cpm_regs = (void __iomem *)CPM_BASE;
	u32 clk_ctrl;
	unsigned int selectplls[] = {
		CPM_PLL_SEL_STOP,
		CPM_PLL_SEL_SRC,
		CPM_PLL_SEL_MPLL,
		CPM_PLL_SEL_EPLL
	};

	/* Init CPU, L2CACHE, AHB0, AHB2, APB clock */
	clk_ctrl = CPM_CPCCR_CE_CPU | CPM_CPCCR_CE_AHB0 | CPM_CPCCR_CE_AHB2 |
			((6 - 1) << CPM_CPCCR_H2DIV_BIT) |
			((3 - 1) << CPM_CPCCR_H0DIV_BIT) |
			((2 - 1) << CPM_CPCCR_L2DIV_BIT) |
			((1 - 1) << CPM_CPCCR_CDIV_BIT);

	if (CONFIG_SYS_MHZ >= 1000)
		clk_ctrl |= (12 - 1) << CPM_CPCCR_PDIV_BIT;
	else
		clk_ctrl |= (6 - 1) << CPM_CPCCR_PDIV_BIT;

	clrsetbits_le32(cpm_regs + CPM_CPCCR, 0x00ffffff, clk_ctrl);

	while (readl(cpm_regs + CPM_CPCSR) & (CPM_CPCSR_CDIV_BUSY |
	       CPM_CPCSR_H0DIV_BUSY | CPM_CPCSR_H2DIV_BUSY))
		;

	clk_ctrl = (selectplls[pll] << CPM_CPCCR_SEL_CPLL_BIT) |
		   (selectplls[MPLL] << CPM_CPCCR_SEL_H0PLL_BIT) |
		   (selectplls[MPLL] << CPM_CPCCR_SEL_H2PLL_BIT);
	if (pll == APLL)
		clk_ctrl |= CPM_PLL_SEL_SRC << CPM_CPCCR_SEL_SRC_BIT;
	else
		clk_ctrl |= CPM_SRC_SEL_EXCLK << CPM_CPCCR_SEL_SRC_BIT;

	clrsetbits_le32(cpm_regs + CPM_CPCCR, 0xff << 24, clk_ctrl);
}

static void ddr_mux_select(int pll)
{
	void __iomem *cpm_regs = (void __iomem *)CPM_BASE;
	int selectplls[] = { CPM_DDRCDR_DCS_STOP,
			     CPM_DDRCDR_DCS_SRC,
			     CPM_DDRCDR_DCS_MPLL};

	writel(selectplls[pll] | CPM_DDRCDR_CE_DDR | (JZ4780_SYS_MEM_DIV - 1),
	       cpm_regs + CPM_DDCDR);

	while (readl(cpm_regs + CPM_DDCDR) & CPM_DDRCDR_DDR_BUSY)
		;

	clrbits_le32(cpm_regs + CPM_CLKGR0, CPM_CLKGR0_DDR0);

	mdelay(200);
}

static void cgu_mux_init(struct cgu_pll_select *cgu, unsigned int num)
{
	void __iomem *cpm_regs = (void __iomem *)CPM_BASE;
	unsigned int selectplls[] = {0, 1, 2, 3, 2, 6};
	int i;

	for (i = 0; i < num; i++)
		writel(selectplls[cgu[i].pll] << cgu[i].pll_shift,
		       cpm_regs + cgu[i].reg);
}

void pll_init(void)
{
	void __iomem *cpm_regs = (void __iomem *)CPM_BASE;
	struct cgu_pll_select cgu_mux[] = {
		{ CPM_MSCCDR,  MPLL, 30 },
		{ CPM_LPCDR,   VPLL, 30 },
		{ CPM_LPCDR1,  VPLL, 30 },
		{ CPM_GPUCDR,  MPLL, 30 },
		{ CPM_HDMICDR, VPLL, 30 },
		{ CPM_I2SCDR,  EPLL, 30 },
		{ CPM_BCHCDR,  MPLL, 30 },
		{ CPM_VPUCDR,  0x1,  30 },
		{ CPM_UHCCDR,  0x3,  30 },
		{ CPM_CIMCDR,  0x1,  31 },
		{ CPM_PCMCDR,  0x5,  29 },
		{ CPM_SSICDR,  0x3,  30 },
	};

	/* PLL stable time set to default -- 1ms */
	clrsetbits_le32(cpm_regs + CPM_CPPCR, 0xfffff, (16 << 8) | 0x20);

	pll_init_one(APLL, JZ4780_APLL_M, JZ4780_APLL_N, JZ4780_APLL_OD);
	pll_init_one(MPLL, JZ4780_MPLL_M, JZ4780_MPLL_N, JZ4780_MPLL_OD);
	pll_init_one(VPLL, JZ4780_VPLL_M, JZ4780_VPLL_N, JZ4780_VPLL_OD);
	pll_init_one(EPLL, JZ4780_EPLL_M, JZ4780_EPLL_N, JZ4780_EPLL_OD);

	cpu_mux_select(MPLL);
	ddr_mux_select(MPLL);
	cgu_mux_init(cgu_mux, ARRAY_SIZE(cgu_mux));
}

const u32 jz4780_clk_get_efuse_clk(void)
{
	void __iomem *cpm_regs = (void __iomem *)CPM_BASE;
	u32 cpccr = readl(cpm_regs + CPM_CPCCR);
	u32 ahb2_div = ((cpccr & CPM_CPCCR_H2DIV_MASK) >>
			CPM_CPCCR_H2DIV_BIT) + 1;
	return JZ4780_SYS_MEM_SPEED / ahb2_div;
}

void jz4780_clk_ungate_ethernet(void)
{
	void __iomem *cpm_regs = (void __iomem *)CPM_BASE;

	clrbits_le32(cpm_regs + CPM_CLKGR0, CPM_CLKGR0_MAC);
	clrbits_le32(cpm_regs + CPM_CLKGR0, CPM_CLKGR0_NEMC);
}

void jz4780_clk_ungate_mmc(void)
{
	void __iomem *cpm_regs = (void __iomem *)CPM_BASE;
	u32 msc_cdr = JZ4780_SYS_MEM_SPEED / 24000000 / 2 - 1;

	msc_cdr |= CPM_MSCCDR_MPCS_MPLL | CPM_MSCCDR_CE;
	writel(msc_cdr, cpm_regs + CPM_MSCCDR);
	writel(msc_cdr, cpm_regs + CPM_MSCCDR1);
	writel(msc_cdr, cpm_regs + CPM_MSCCDR2);

	/* The wait_for_bit() won't fit, thus unbounded loop here. */
	while (readl(cpm_regs + CPM_MSCCDR1) & CPM_MSCCDR_MSC_BUSY)
		;
}

void jz4780_clk_ungate_uart(const unsigned int uart)
{
	void __iomem *cpm_regs = (void __iomem *)CPM_BASE;

	if (uart == 0)
		clrbits_le32(cpm_regs + CPM_CLKGR0, CPM_CLKGR0_UART0);
	else if (uart == 1)
		clrbits_le32(cpm_regs + CPM_CLKGR0, CPM_CLKGR0_UART1);
	else if (uart == 2)
		clrbits_le32(cpm_regs + CPM_CLKGR0, CPM_CLKGR0_UART2);
	else if (uart == 3)
		clrbits_le32(cpm_regs + CPM_CLKGR0, CPM_CLKGR0_UART3);
	else if (uart == 4)
		clrbits_le32(cpm_regs + CPM_CLKGR1, CPM_CLKGR1_UART4);
	else
		printf("%s[%i]: Invalid UART %d\n", __func__, __LINE__, uart);
}
