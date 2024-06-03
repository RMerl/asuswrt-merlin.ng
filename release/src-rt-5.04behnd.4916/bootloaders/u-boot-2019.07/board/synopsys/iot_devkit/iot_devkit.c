// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Synopsys, Inc. All rights reserved.
 */

#include <common.h>
#include <malloc.h>
#include <dwmmc.h>
#include <linux/libfdt.h>
#include <fdtdec.h>

#include <asm/arcregs.h>

DECLARE_GLOBAL_DATA_PTR;

#define SYSCON_BASE	0xf000a000
#define AHBCKDIV	(void *)(SYSCON_BASE + 0x04)
#define APBCKDIV	(void *)(SYSCON_BASE + 0x08)
#define APBCKEN		(void *)(SYSCON_BASE + 0x0C)
#define RESET_REG	(void *)(SYSCON_BASE + 0x18)
#define CLKSEL		(void *)(SYSCON_BASE + 0x24)
#define CLKSTAT		(void *)(SYSCON_BASE + 0x28)
#define PLLCON		(void *)(SYSCON_BASE + 0x2C)
#define APBCKSEL	(void *)(SYSCON_BASE + 0x30)
#define AHBCKEN		(void *)(SYSCON_BASE + 0x34)
#define USBPHY_PLL	(void *)(SYSCON_BASE + 0x78)
#define USBCFG		(void *)(SYSCON_BASE + 0x7c)

#define PLL_MASK_0	0xffcfffff
#define PLL_MASK_1	0xffcfff00
#define PLL_MASK_2	0xfbcfff00

#define CLKSEL_DEFAULT	0x5a690000

static int set_cpu_freq(unsigned int clk)
{
	clk /= 1000000;

	/* Set clk to ext Xtal (LSN value 0) */
	writel(CLKSEL_DEFAULT, CLKSEL);

	switch (clk) {
	case 16:
		/* Bypass mode */
		return 0;

	case 50:
		writel(readl(PLLCON) & PLL_MASK_0, PLLCON);
		/* pll_off=1, M=25, N=1, OD=3, PLL_OUT_CLK=50M */
		writel((readl(PLLCON) & PLL_MASK_1) | 0x300191, PLLCON);
		/* pll_off=0, M=25, N=1, OD=3, PLL_OUT_CLK=50M */
		writel((readl(PLLCON) & PLL_MASK_2) | 0x300191, PLLCON);
		break;

	case 72:
		writel(readl(PLLCON) & PLL_MASK_0, PLLCON);
		/* pll_off=1, M=18, N=1, OD=2, PLL_OUT_CLK=72M */
		writel((readl(PLLCON) & PLL_MASK_1) | 0x200121, PLLCON);
		/* pll_off=0, M=18, N=1, OD=2, PLL_OUT_CLK=72M */
		writel((readl(PLLCON) & PLL_MASK_2) | 0x200121, PLLCON);
		break;

	case 100:
		writel(readl(PLLCON) & PLL_MASK_0, PLLCON);
		/* pll_off=1,M=25, N=1, OD=2, PLL_OUT_CLK=100M */
		writel((readl(PLLCON) & PLL_MASK_1) | 0x200191, PLLCON);
		/* pll_off=0,M=25, N=1, OD=2, PLL_OUT_CLK=100M */
		writel((readl(PLLCON) & PLL_MASK_2) | 0x200191, PLLCON);
		break;

	case 136:
		writel(readl(PLLCON) & PLL_MASK_0, PLLCON);
		/* pll_off=1, M=17, N=1, OD=1, PLL_OUT_CLK=136M */
		writel((readl(PLLCON) & PLL_MASK_1) | 0x100111, PLLCON);
		/* pll_off=0, M=17, N=1, OD=1, PLL_OUT_CLK=136M */
		writel((readl(PLLCON) & PLL_MASK_2) | 0x100111, PLLCON);
		break;

	case 144:
		writel(readl(PLLCON) & PLL_MASK_0, PLLCON);
		/* pll_off=1, M=18, N=1, OD=1, PLL_OUT_CLK=144M */
		writel((readl(PLLCON) & PLL_MASK_1) | 0x100121, PLLCON);
		/* pll_off=0, M=18, N=1, OD=1, PLL_OUT_CLK=144M */
		writel((readl(PLLCON) & PLL_MASK_2) | 0x100121, PLLCON);
		break;

	default:
		return -EINVAL;
	}

	while (!(readl(CLKSTAT) & 0x4))
		;

	/* Set clk from PLL on bus (LSN = 1) */
	writel(CLKSEL_DEFAULT | BIT(0), CLKSEL);

	return 0;
}

extern u8 __rom_end[];
extern u8 __ram_start[];
extern u8 __ram_end[];

/*
 * Use mach_cpu_init() for .data section copy as board_early_init_f() will be
 * too late: initf_dm() will use a value of "av_" variable from not yet
 * initialized (by copy) area.
 */
int mach_cpu_init(void)
{
	int offset;

	/* Don't relocate U-Boot */
	gd->flags |= GD_FLG_SKIP_RELOC;

	/* Copy data from ROM to RAM */
	u8 *src = __rom_end;
	u8 *dst = __ram_start;

	while (dst < __ram_end)
		*dst++ = *src++;

	/* Enable debug uart */
#define DEBUG_UART_BASE		0x80014000
#define DEBUG_UART_DLF_OFFSET	0xc0
	write_aux_reg(DEBUG_UART_BASE + DEBUG_UART_DLF_OFFSET, 1);

	offset = fdt_path_offset(gd->fdt_blob, "/cpu_card/core_clk");
	if (offset < 0)
		return offset;

	gd->cpu_clk = fdtdec_get_int(gd->fdt_blob, offset, "clock-frequency", 0);
	if (!gd->cpu_clk)
		return -EINVAL;

	/* If CPU freq > 100 MHz, divide eFLASH clock by 2 */
	if (gd->cpu_clk > 100000000) {
		u32 reg = readl(AHBCKDIV);

		reg &= ~(0xF << 8);
		reg |= 2 << 8;
		writel(reg, AHBCKDIV);
	}

	return set_cpu_freq(gd->cpu_clk);
}

#define ARC_PERIPHERAL_BASE	0xF0000000
#define SDIO_BASE		(ARC_PERIPHERAL_BASE + 0xB000)

int board_mmc_init(bd_t *bis)
{
	struct dwmci_host *host = NULL;

	host = malloc(sizeof(struct dwmci_host));
	if (!host) {
		printf("dwmci_host malloc fail!\n");
		return -ENOMEM;
	}

	memset(host, 0, sizeof(struct dwmci_host));
	host->name = "Synopsys Mobile storage";
	host->ioaddr = (void *)SDIO_BASE;
	host->buswidth = 4;
	host->dev_index = 0;
	host->bus_hz = 50000000;

	add_dwmci(host, host->bus_hz / 2, 400000);

	return 0;
}

int board_mmc_getcd(struct mmc *mmc)
{
	struct dwmci_host *host = mmc->priv;

	return !(dwmci_readl(host, DWMCI_CDETECT) & 1);
}

#define IOTDK_RESET_SEQ		0x55AA6699

void reset_cpu(ulong addr)
{
	writel(IOTDK_RESET_SEQ, RESET_REG);
}

int checkboard(void)
{
	puts("Board: Synopsys IoT Development Kit\n");
	return 0;
};
