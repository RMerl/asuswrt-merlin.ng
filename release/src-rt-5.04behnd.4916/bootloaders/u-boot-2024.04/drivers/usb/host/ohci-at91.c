// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2006
 * DENX Software Engineering <mk@denx.de>
 */

#include <common.h>
#include <asm/arch/clk.h>

int usb_cpu_init(void)
{
#ifdef CONFIG_USB_ATMEL_CLK_SEL_PLLB
	if (at91_pllb_clk_enable(get_pllb_init()))
		return -1;

#ifdef CONFIG_AT91SAM9N12
	at91_usb_clk_init(AT91_PMC_USBS_USB_PLLB | AT91_PMC_USB_DIV_2);
#endif
#elif defined(CONFIG_USB_ATMEL_CLK_SEL_UPLL)
	if (at91_upll_clk_enable())
		return -1;

	at91_usb_clk_init(AT91_PMC_USBS_USB_UPLL | AT91_PMC_USBDIV_10);
#endif

	at91_periph_clk_enable(ATMEL_ID_UHP);

	at91_system_clk_enable(ATMEL_PMC_UHP);
#if defined(CONFIG_AT91SAM9261) || defined(CONFIG_AT91SAM9G10)
	at91_system_clk_enable(AT91_PMC_HCK0);
#endif

	return 0;
}

int usb_cpu_stop(void)
{
	at91_periph_clk_disable(ATMEL_ID_UHP);

	at91_system_clk_disable(ATMEL_PMC_UHP);
#if defined(CONFIG_AT91SAM9261) || defined(CONFIG_AT91SAM9G10)
	at91_system_clk_disable(AT91_PMC_HCK0);
#endif

#ifdef CONFIG_USB_ATMEL_CLK_SEL_PLLB
#ifdef CONFIG_AT91SAM9N12
	at91_usb_clk_init(0);
#endif

	if (at91_pllb_clk_disable())
		return -1;

#elif defined(CONFIG_USB_ATMEL_CLK_SEL_UPLL)
	if (at91_upll_clk_disable())
		return -1;
#endif

	return 0;
}

int usb_cpu_init_fail(void)
{
	return usb_cpu_stop();
}
