#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
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
