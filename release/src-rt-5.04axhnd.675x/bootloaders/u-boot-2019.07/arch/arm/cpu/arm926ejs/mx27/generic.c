// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (c) 2008 Eric Jarrige <eric.jarrige@armadeus.org>
 *  Copyright (c) 2009 Ilya Yanok <yanok@emcraft.com>
 */

#include <common.h>
#include <div64.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/mach-imx/sys_proto.h>
#ifdef CONFIG_MMC_MXC
#include <asm/arch/mxcmmc.h>
#endif

/*
 *  get the system pll clock in Hz
 *
 *                  mfi + mfn / (mfd +1)
 *  f = 2 * f_ref * --------------------
 *                        pd + 1
 */
static unsigned int imx_decode_pll(unsigned int pll, unsigned int f_ref)
{
	unsigned int mfi = (pll >> 10) & 0xf;
	unsigned int mfn = pll & 0x3ff;
	unsigned int mfd = (pll >> 16) & 0x3ff;
	unsigned int pd =  (pll >> 26) & 0xf;

	mfi = mfi <= 5 ? 5 : mfi;

	return lldiv(2 * (u64)f_ref * (mfi * (mfd + 1) + mfn),
			(mfd + 1) * (pd + 1));
}

static ulong clk_in_32k(void)
{
	return 1024 * CONFIG_MX27_CLK32;
}

static ulong clk_in_26m(void)
{
	struct pll_regs *pll = (struct pll_regs *)IMX_PLL_BASE;

	if (readl(&pll->cscr) & CSCR_OSC26M_DIV1P5) {
		/* divide by 1.5 */
		return 26000000 * 2 / 3;
	} else {
		return 26000000;
	}
}

static ulong imx_get_mpllclk(void)
{
	struct pll_regs *pll = (struct pll_regs *)IMX_PLL_BASE;
	ulong cscr = readl(&pll->cscr);
	ulong fref;

	if (cscr & CSCR_MCU_SEL)
		fref = clk_in_26m();
	else
		fref = clk_in_32k();

	return imx_decode_pll(readl(&pll->mpctl0), fref);
}

static ulong imx_get_armclk(void)
{
	struct pll_regs *pll = (struct pll_regs *)IMX_PLL_BASE;
	ulong cscr = readl(&pll->cscr);
	ulong fref = imx_get_mpllclk();
	ulong div;

	if (!(cscr & CSCR_ARM_SRC_MPLL))
		fref = lldiv((fref * 2), 3);

	div = ((cscr >> 12) & 0x3) + 1;

	return lldiv(fref, div);
}

static ulong imx_get_ahbclk(void)
{
	struct pll_regs *pll = (struct pll_regs *)IMX_PLL_BASE;
	ulong cscr = readl(&pll->cscr);
	ulong fref = imx_get_mpllclk();
	ulong div;

	div = ((cscr >> 8) & 0x3) + 1;

	return lldiv(fref * 2, 3 * div);
}

static __attribute__((unused)) ulong imx_get_spllclk(void)
{
	struct pll_regs *pll = (struct pll_regs *)IMX_PLL_BASE;
	ulong cscr = readl(&pll->cscr);
	ulong fref;

	if (cscr & CSCR_SP_SEL)
		fref = clk_in_26m();
	else
		fref = clk_in_32k();

	return imx_decode_pll(readl(&pll->spctl0), fref);
}

static ulong imx_decode_perclk(ulong div)
{
	return lldiv((imx_get_mpllclk() * 2), (div * 3));
}

static ulong imx_get_perclk1(void)
{
	struct pll_regs *pll = (struct pll_regs *)IMX_PLL_BASE;

	return imx_decode_perclk((readl(&pll->pcdr1) & 0x3f) + 1);
}

static ulong imx_get_perclk2(void)
{
	struct pll_regs *pll = (struct pll_regs *)IMX_PLL_BASE;

	return imx_decode_perclk(((readl(&pll->pcdr1) >> 8) & 0x3f) + 1);
}

static __attribute__((unused)) ulong imx_get_perclk3(void)
{
	struct pll_regs *pll = (struct pll_regs *)IMX_PLL_BASE;

	return imx_decode_perclk(((readl(&pll->pcdr1) >> 16) & 0x3f) + 1);
}

static __attribute__((unused)) ulong imx_get_perclk4(void)
{
	struct pll_regs *pll = (struct pll_regs *)IMX_PLL_BASE;

	return imx_decode_perclk(((readl(&pll->pcdr1) >> 24) & 0x3f) + 1);
}

unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_ARM_CLK:
		return imx_get_armclk();
	case MXC_I2C_CLK:
		return imx_get_ahbclk()/2;
	case MXC_UART_CLK:
		return imx_get_perclk1();
	case MXC_FEC_CLK:
		return imx_get_ahbclk();
	case MXC_ESDHC_CLK:
		return imx_get_perclk2();
	}
	return -1;
}


u32 get_cpu_rev(void)
{
	return MXC_CPU_MX27 << 12;
}

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo (void)
{
	char buf[32];

	printf("CPU:   Freescale i.MX27 at %s MHz\n\n",
			strmhz(buf, imx_get_mpllclk()));
	return 0;
}
#endif

int cpu_eth_init(bd_t *bis)
{
#if defined(CONFIG_FEC_MXC)
	struct pll_regs *pll = (struct pll_regs *)IMX_PLL_BASE;

	/* enable FEC clock */
	writel(readl(&pll->pccr1) | PCCR1_HCLK_FEC, &pll->pccr1);
	writel(readl(&pll->pccr0) | PCCR0_FEC_EN, &pll->pccr0);
	return fecmxc_initialize(bis);
#else
	return 0;
#endif
}

/*
 * Initializes on-chip MMC controllers.
 * to override, implement board_mmc_init()
 */
int cpu_mmc_init(bd_t *bis)
{
#ifdef CONFIG_MMC_MXC
	return mxc_mmc_init(bis);
#else
	return 0;
#endif
}

void imx_gpio_mode(int gpio_mode)
{
	struct gpio_port_regs *regs = (struct gpio_port_regs *)IMX_GPIO_BASE;
	unsigned int pin = gpio_mode & GPIO_PIN_MASK;
	unsigned int port = (gpio_mode & GPIO_PORT_MASK) >> GPIO_PORT_SHIFT;
	unsigned int ocr = (gpio_mode & GPIO_OCR_MASK) >> GPIO_OCR_SHIFT;
	unsigned int aout = (gpio_mode & GPIO_AOUT_MASK) >> GPIO_AOUT_SHIFT;
	unsigned int bout = (gpio_mode & GPIO_BOUT_MASK) >> GPIO_BOUT_SHIFT;
	unsigned int tmp;

	/* Pullup enable */
	if (gpio_mode & GPIO_PUEN) {
		writel(readl(&regs->port[port].puen) | (1 << pin),
				&regs->port[port].puen);
	} else {
		writel(readl(&regs->port[port].puen) & ~(1 << pin),
				&regs->port[port].puen);
	}

	/* Data direction */
	if (gpio_mode & GPIO_OUT) {
		writel(readl(&regs->port[port].gpio_dir) | 1 << pin,
				&regs->port[port].gpio_dir);
	} else {
		writel(readl(&regs->port[port].gpio_dir) & ~(1 << pin),
				&regs->port[port].gpio_dir);
	}

	/* Primary / alternate function */
	if (gpio_mode & GPIO_AF) {
		writel(readl(&regs->port[port].gpr) | (1 << pin),
				&regs->port[port].gpr);
	} else {
		writel(readl(&regs->port[port].gpr) & ~(1 << pin),
				&regs->port[port].gpr);
	}

	/* use as gpio? */
	if (!(gpio_mode & (GPIO_PF | GPIO_AF))) {
		writel(readl(&regs->port[port].gius) | (1 << pin),
				&regs->port[port].gius);
	} else {
		writel(readl(&regs->port[port].gius) & ~(1 << pin),
				&regs->port[port].gius);
	}

	/* Output / input configuration */
	if (pin < 16) {
		tmp = readl(&regs->port[port].ocr1);
		tmp &= ~(3 << (pin * 2));
		tmp |= (ocr << (pin * 2));
		writel(tmp, &regs->port[port].ocr1);

		writel(readl(&regs->port[port].iconfa1) & ~(3 << (pin * 2)),
				&regs->port[port].iconfa1);
		writel(readl(&regs->port[port].iconfa1) | aout << (pin * 2),
				&regs->port[port].iconfa1);
		writel(readl(&regs->port[port].iconfb1) & ~(3 << (pin * 2)),
				&regs->port[port].iconfb1);
		writel(readl(&regs->port[port].iconfb1) | bout << (pin * 2),
				&regs->port[port].iconfb1);
	} else {
		pin -= 16;

		tmp = readl(&regs->port[port].ocr2);
		tmp &= ~(3 << (pin * 2));
		tmp |= (ocr << (pin * 2));
		writel(tmp, &regs->port[port].ocr2);

		writel(readl(&regs->port[port].iconfa2) & ~(3 << (pin * 2)),
				&regs->port[port].iconfa2);
		writel(readl(&regs->port[port].iconfa2) | aout << (pin * 2),
				&regs->port[port].iconfa2);
		writel(readl(&regs->port[port].iconfb2) & ~(3 << (pin * 2)),
				&regs->port[port].iconfb2);
		writel(readl(&regs->port[port].iconfb2) | bout << (pin * 2),
				&regs->port[port].iconfb2);
	}
}

#ifdef CONFIG_MXC_UART
void mx27_uart1_init_pins(void)
{
	int i;
	unsigned int mode[] = {
		PE12_PF_UART1_TXD,
		PE13_PF_UART1_RXD,
	};

	for (i = 0; i < ARRAY_SIZE(mode); i++)
		imx_gpio_mode(mode[i]);

}
#endif /* CONFIG_MXC_UART */

#ifdef CONFIG_FEC_MXC
void mx27_fec_init_pins(void)
{
	int i;
	unsigned int mode[] = {
		PD0_AIN_FEC_TXD0,
		PD1_AIN_FEC_TXD1,
		PD2_AIN_FEC_TXD2,
		PD3_AIN_FEC_TXD3,
		PD4_AOUT_FEC_RX_ER,
		PD5_AOUT_FEC_RXD1,
		PD6_AOUT_FEC_RXD2,
		PD7_AOUT_FEC_RXD3,
		PD8_AF_FEC_MDIO,
		PD9_AIN_FEC_MDC | GPIO_PUEN,
		PD10_AOUT_FEC_CRS,
		PD11_AOUT_FEC_TX_CLK,
		PD12_AOUT_FEC_RXD0,
		PD13_AOUT_FEC_RX_DV,
		PD14_AOUT_FEC_CLR,
		PD15_AOUT_FEC_COL,
		PD16_AIN_FEC_TX_ER,
		PF23_AIN_FEC_TX_EN,
	};

	for (i = 0; i < ARRAY_SIZE(mode); i++)
		imx_gpio_mode(mode[i]);
}

void imx_get_mac_from_fuse(int dev_id, unsigned char *mac)
{
	int i;
	struct iim_regs *iim = (struct iim_regs *)IMX_IIM_BASE;
	struct fuse_bank *bank = &iim->bank[0];
	struct fuse_bank0_regs *fuse =
			(struct fuse_bank0_regs *)bank->fuse_regs;

	for (i = 0; i < 6; i++)
		mac[6 - 1 - i] = readl(&fuse->mac_addr[i]) & 0xff;
}
#endif /* CONFIG_FEC_MXC */

#ifdef CONFIG_MMC_MXC
void mx27_sd1_init_pins(void)
{
	int i;
	unsigned int mode[] = {
		PE18_PF_SD1_D0,
		PE19_PF_SD1_D1,
		PE20_PF_SD1_D2,
		PE21_PF_SD1_D3,
		PE22_PF_SD1_CMD,
		PE23_PF_SD1_CLK,
	};

	for (i = 0; i < ARRAY_SIZE(mode); i++)
		imx_gpio_mode(mode[i]);

}

void mx27_sd2_init_pins(void)
{
	int i;
	unsigned int mode[] = {
		PB4_PF_SD2_D0,
		PB5_PF_SD2_D1,
		PB6_PF_SD2_D2,
		PB7_PF_SD2_D3,
		PB8_PF_SD2_CMD,
		PB9_PF_SD2_CLK,
	};

	for (i = 0; i < ARRAY_SIZE(mode); i++)
		imx_gpio_mode(mode[i]);

}
#endif /* CONFIG_MMC_MXC */
