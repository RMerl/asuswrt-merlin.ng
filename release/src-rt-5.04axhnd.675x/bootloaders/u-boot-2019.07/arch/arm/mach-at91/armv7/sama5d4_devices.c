// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Atmel
 *		      Bo Shen <voice.shen@atmel.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/clk.h>
#include <asm/arch/sama5_sfr.h>
#include <asm/arch/sama5d4.h>

char *get_cpu_name()
{
	unsigned int extension_id = get_extension_chip_id();

	if (cpu_is_sama5d4())
		switch (extension_id) {
		case ARCH_EXID_SAMA5D41:
			return "SAMA5D41";
		case ARCH_EXID_SAMA5D42:
			return "SAMA5D42";
		case ARCH_EXID_SAMA5D43:
			return "SAMA5D43";
		case ARCH_EXID_SAMA5D44:
			return "SAMA5D44";
		default:
			return "Unknown CPU type";
		}
	else
		return "Unknown CPU type";
}

#ifdef CONFIG_USB_GADGET_ATMEL_USBA
void at91_udp_hw_init(void)
{
	/* Enable UPLL clock */
	at91_upll_clk_enable();
	/* Enable UDPHS clock */
	at91_periph_clk_enable(ATMEL_ID_UDPHS);
}
#endif
