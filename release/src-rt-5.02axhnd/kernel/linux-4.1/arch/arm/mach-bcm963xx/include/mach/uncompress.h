#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
#include <mach/hardware.h>
#include <bcm_map_part.h>

#define AMBA_UART_DR(base)	(*(volatile unsigned char *)((base) + 0x00))
#define AMBA_UART_LCRH(base)	(*(volatile unsigned char *)((base) + 0x2c))
#define AMBA_UART_CR(base)	(*(volatile unsigned char *)((base) + 0x30))
#define AMBA_UART_FR(base)	(*(volatile unsigned char *)((base) + 0x18))

#if defined(ARM_UART_PHYS_BASE) && (CONFIG_DEBUG_UART_ADDR==ARM_UART_PHYS_BASE)
/*
 * This does not append a newline
 */
static inline void putc(int c)
{
	unsigned long base = CONFIG_DEBUG_UART_ADDR;

	while (AMBA_UART_FR(base) & (1 << 5))
		barrier();

	AMBA_UART_DR(base) = c;
}

static inline void flush(void)
{
	unsigned long base = CONFIG_DEBUG_UART_ADDR;

	while (AMBA_UART_FR(base) & (1 << 3))
		barrier();
}
#elif CONFIG_DEBUG_UART_ADDR==UART0_PHYS_BASE

#define PERIPH_UART_DATA(base)	(*(volatile unsigned char *)((base) + 0x14))
#define PERIPH_UART_STS(base)	(*(volatile unsigned long *)((base) + 0x10))

static inline void putc(int c)
{
#if 0	/* FIXME! TXFIFOTHOLD might not work */
	while (!(PERIPH_UART_STS(UART0_PHYS_BASE) & TXFIFOTHOLD))
		barrier();
#endif
	PERIPH_UART_DATA(UART0_PHYS_BASE) = (unsigned char)c;
}

static inline void flush(void)
{
	while (!(PERIPH_UART_STS(UART0_PHYS_BASE) & TXFIFOEMT))
		barrier();
}
#endif

/*
 * nothing to do
 */
#define arch_decomp_setup()
#define arch_decomp_wdog()
#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
