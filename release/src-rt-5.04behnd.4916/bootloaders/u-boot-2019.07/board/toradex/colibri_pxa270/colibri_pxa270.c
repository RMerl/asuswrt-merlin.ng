// SPDX-License-Identifier: GPL-2.0+
/*
 * Toradex Colibri PXA270 Support
 *
 * Copyright (C) 2010 Marek Vasut <marek.vasut@gmail.com>
 * Copyright (C) 2016 Marcel Ziswiler <marcel.ziswiler@toradex.com>
 */

#include <common.h>
#include <dm.h>
#include <asm/arch/hardware.h>
#include <asm/arch/pxa.h>
#include <asm/arch/regs-mmc.h>
#include <asm/arch/regs-uart.h>
#include <asm/io.h>
#include <dm/platdata.h>
#include <dm/platform_data/serial_pxa.h>
#include <netdev.h>
#include <serial.h>
#include <usb.h>
#include <asm/mach-types.h>
#include "../common/tdx-common.h"

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	/* We have RAM, disable cache */
	dcache_disable();
	icache_disable();

	/* arch number of Toradex Colibri PXA270 */
	gd->bd->bi_arch_number = MACH_TYPE_COLIBRI;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0xa0000100;

	return 0;
}

int checkboard(void)
{
	puts("Model: Toradex Colibri PXA270\n");

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	return ft_common_board_setup(blob, bd);
}
#endif

int dram_init(void)
{
	pxa2xx_dram_init();
	gd->ram_size = PHYS_SDRAM_1_SIZE;
	return 0;
}

#ifdef	CONFIG_CMD_USB
int board_usb_init(int index, enum usb_init_type init)
{
	writel((readl(UHCHR) | UHCHR_PCPL | UHCHR_PSPL) &
		~(UHCHR_SSEP0 | UHCHR_SSEP1 | UHCHR_SSEP2 | UHCHR_SSE),
		UHCHR);

	writel(readl(UHCHR) | UHCHR_FSBIR, UHCHR);

	while (UHCHR & UHCHR_FSBIR)
		;

	writel(readl(UHCHR) & ~UHCHR_SSE, UHCHR);
	writel((UHCHIE_UPRIE | UHCHIE_RWIE), UHCHIE);

	/* Clear any OTG Pin Hold */
	if (readl(PSSR) & PSSR_OTGPH)
		writel(readl(PSSR) | PSSR_OTGPH, PSSR);

	writel(readl(UHCRHDA) & ~(0x200), UHCRHDA);
	writel(readl(UHCRHDA) | 0x100, UHCRHDA);

	/* Set port power control mask bits, only 3 ports. */
	writel(readl(UHCRHDB) | (0x7<<17), UHCRHDB);

	/* enable port 2 */
	writel(readl(UP2OCR) | UP2OCR_HXOE | UP2OCR_HXS |
		UP2OCR_DMPDE | UP2OCR_DPPDE, UP2OCR);

	return 0;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	return 0;
}

void usb_board_stop(void)
{
	writel(readl(UHCHR) | UHCHR_FHR, UHCHR);
	udelay(11);
	writel(readl(UHCHR) & ~UHCHR_FHR, UHCHR);

	writel(readl(UHCCOMS) | 1, UHCCOMS);
	udelay(10);

	writel(readl(CKEN) & ~CKEN10_USBHOST, CKEN);

	return;
}
#endif

#ifdef CONFIG_DRIVER_DM9000
int board_eth_init(bd_t *bis)
{
	return dm9000_initialize(bis);
}
#endif

#ifdef	CONFIG_CMD_MMC
int board_mmc_init(bd_t *bis)
{
	pxa_mmc_register(0);
	return 0;
}
#endif

static const struct pxa_serial_platdata serial_platdata = {
	.base = (struct pxa_uart_regs *)FFUART_BASE,
	.port = FFUART_INDEX,
	.baudrate = CONFIG_BAUDRATE,
};

U_BOOT_DEVICE(pxa_serials) = {
	.name = "serial_pxa",
	.platdata = &serial_platdata,
};
