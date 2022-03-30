// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2006
 * Markus Klotzbuecher, DENX Software Engineering <mk@denx.de>
 */

#include <common.h>

#if defined(CONFIG_USB_OHCI_NEW) && defined(CONFIG_SYS_USB_OHCI_CPU_INIT)
# if defined(CONFIG_CPU_MONAHANS) || defined(CONFIG_CPU_PXA27X)

#include <asm/arch/pxa-regs.h>
#include <asm/io.h>
#include <usb.h>

int usb_cpu_init(void)
{
#if defined(CONFIG_CPU_MONAHANS)
	/* Enable USB host clock. */
	writel(readl(CKENA) | CKENA_2_USBHOST | CKENA_20_UDC, CKENA);
	udelay(100);
#endif
#if defined(CONFIG_CPU_PXA27X)
	/* Enable USB host clock. */
	writel(readl(CKEN) | CKEN10_USBHOST, CKEN);
#endif

#if defined(CONFIG_CPU_MONAHANS)
	/* Configure Port 2 for Host (USB Client Registers) */
	writel(0x3000c, UP2OCR);
#endif

	writel(readl(UHCHR) | UHCHR_FHR, UHCHR);
	mdelay(11);
	writel(readl(UHCHR) & ~UHCHR_FHR, UHCHR);

	writel(readl(UHCHR) | UHCHR_FSBIR, UHCHR);
	while (readl(UHCHR) & UHCHR_FSBIR)
		udelay(1);

#if defined(CONFIG_CPU_MONAHANS) || defined(CONFIG_PXA27X)
	writel(readl(UHCHR) & ~UHCHR_SSEP0, UHCHR);
#endif
#if defined(CONFIG_CPU_PXA27X)
	writel(readl(UHCHR) & ~UHCHR_SSEP2, UHCHR);
#endif
	writel(readl(UHCHR) & ~(UHCHR_SSEP1 | UHCHR_SSE), UHCHR);

	return 0;
}

int usb_cpu_stop(void)
{
	writel(readl(UHCHR) | UHCHR_FHR, UHCHR);
	udelay(11);
	writel(readl(UHCHR) & ~UHCHR_FHR, UHCHR);

	writel(readl(UHCCOMS) | UHCCOMS_HCR, UHCCOMS);
	udelay(10);

#if defined(CONFIG_CPU_MONAHANS) || defined(CONFIG_PXA27X)
	writel(readl(UHCHR) | UHCHR_SSEP0, UHCHR);
#endif
#if defined(CONFIG_CPU_PXA27X)
	writel(readl(UHCHR) | UHCHR_SSEP2, UHCHR);
#endif
	writel(readl(UHCHR) | UHCHR_SSEP1 | UHCHR_SSE, UHCHR);

#if defined(CONFIG_CPU_MONAHANS)
	/* Disable USB host clock. */
	writel(readl(CKENA) & ~(CKENA_2_USBHOST | CKENA_20_UDC), CKENA);
	udelay(100);
#endif
#if defined(CONFIG_CPU_PXA27X)
	/* Disable USB host clock. */
	writel(readl(CKEN) & ~CKEN10_USBHOST, CKEN);
#endif

	return 0;
}

int usb_cpu_init_fail(void)
{
	return usb_cpu_stop();
}

# endif /* defined(CONFIG_CPU_MONAHANS) || defined(CONFIG_CPU_PXA27X) */
#endif /* defined(CONFIG_USB_OHCI) && defined(CONFIG_SYS_USB_OHCI_CPU_INIT) */
