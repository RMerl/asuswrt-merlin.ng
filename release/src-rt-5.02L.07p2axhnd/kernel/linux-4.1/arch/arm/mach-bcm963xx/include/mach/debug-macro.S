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
/* FIXME!! this one needs to be adjusted */

/*
 * Macros used for EARLY_PRINTK, in low-level UART debug console
 */
#include <mach/hardware.h>
//#include <bcm_map_part.h>

#define DEBUG_UART_VA		IO_ADDRESS(CONFIG_DEBUG_UART_ADDR)

	.macro addruart, rp, rv, tmp
	ldr	\rv, =DEBUG_UART_VA	@ virtual
	ldr	\rp, =CONFIG_DEBUG_UART_ADDR	@ physical
	.endm

/* FIXME!! try to use a defined value for the address below */
#if CONFIG_DEBUG_UART_ADDR==0x80019000
//#if CONFIG_DEBUG_UART_ADDR==ARM_UART_PHYS_BASE
	#ifdef CONFIG_PLAT_BCM63XX_AMBA_PL011
		#include <asm/hardware/debug-pl01x.S>
	#else
		#ifdef EARLY_PRINTK
			/* FIXME! Print a compiling warning message saying */
			/* that there is no device for early_printk */
		#endif /* EARLY_PRINTK */
	#endif /* CONFIG_PLAT_BCM63XX_AMBA_PL011 */
#else
/* using PERIPH Uart for debug.S */
#define BCM63XX_UART_CTRL	0x00
#define BCM63XX_UART_BDWD	0x04
#define BCM63XX_UART_MCCTL	0x08
#define BCM63XX_UART_EIPT	0x0C
#define BCM63XX_UART_INT	0x10
#define BCM63XX_UART_DATA	0x14

#define BCM63XX_UART_TXFIFOEMP	0x0020
#define BCM63XX_UART_TXFIFOTHOLD	0x0008

	.macro senduart, rd, rx
	/* byte access doesn't work, has to write word */
	strb	\rd, [\rx, #BCM63XX_UART_DATA]
	.endm

	.macro waituart, rd, rx
1001:	ldr	\rd, [\rx, #BCM63XX_UART_INT]
	tst	\rd, #BCM63XX_UART_TXFIFOEMP
	beq	1001b
	.endm

	.macro busyuart, rd, rx
1002:	ldr	\rd, [\rx, #BCM63XX_UART_INT]
	tst	\rd, #BCM63XX_UART_TXFIFOEMP
	beq	1002b
	.endm
#endif /* CONFIG_DEBUG_UART_ADDR=0x8001900 */
#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
