/*
 * arch/arm/mach-netx/include/mach/irqs.h
 *
 * Copyright (C) 2005 Sascha Hauer <s.hauer@pengutronix.de>, Pengutronix
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#define NETX_IRQ_VIC_START	64
#define NETX_IRQ_SOFTINT	(NETX_IRQ_VIC_START + 0)
#define NETX_IRQ_TIMER0		(NETX_IRQ_VIC_START + 1)
#define NETX_IRQ_TIMER1		(NETX_IRQ_VIC_START + 2)
#define NETX_IRQ_TIMER2		(NETX_IRQ_VIC_START + 3)
#define NETX_IRQ_SYSTIME_NS	(NETX_IRQ_VIC_START + 4)
#define NETX_IRQ_SYSTIME_S	(NETX_IRQ_VIC_START + 5)
#define NETX_IRQ_GPIO_15	(NETX_IRQ_VIC_START + 6)
#define NETX_IRQ_WATCHDOG	(NETX_IRQ_VIC_START + 7)
#define NETX_IRQ_UART0		(NETX_IRQ_VIC_START + 8)
#define NETX_IRQ_UART1		(NETX_IRQ_VIC_START + 9)
#define NETX_IRQ_UART2		(NETX_IRQ_VIC_START + 10)
#define NETX_IRQ_USB		(NETX_IRQ_VIC_START + 11)
#define NETX_IRQ_SPI		(NETX_IRQ_VIC_START + 12)
#define NETX_IRQ_I2C		(NETX_IRQ_VIC_START + 13)
#define NETX_IRQ_LCD		(NETX_IRQ_VIC_START + 14)
#define NETX_IRQ_HIF		(NETX_IRQ_VIC_START + 15)
#define NETX_IRQ_GPIO_0_14	(NETX_IRQ_VIC_START + 16)
#define NETX_IRQ_XPEC0		(NETX_IRQ_VIC_START + 17)
#define NETX_IRQ_XPEC1		(NETX_IRQ_VIC_START + 18)
#define NETX_IRQ_XPEC2		(NETX_IRQ_VIC_START + 19)
#define NETX_IRQ_XPEC3		(NETX_IRQ_VIC_START + 20)
#define NETX_IRQ_XPEC(no)	(NETX_IRQ_VIC_START + 17 + (no))
#define NETX_IRQ_MSYNC0		(NETX_IRQ_VIC_START + 21)
#define NETX_IRQ_MSYNC1		(NETX_IRQ_VIC_START + 22)
#define NETX_IRQ_MSYNC2		(NETX_IRQ_VIC_START + 23)
#define NETX_IRQ_MSYNC3		(NETX_IRQ_VIC_START + 24)
#define NETX_IRQ_IRQ_PHY	(NETX_IRQ_VIC_START + 25)
#define NETX_IRQ_ISO_AREA	(NETX_IRQ_VIC_START + 26)
/* int 27 is reserved */
/* int 28 is reserved */
#define NETX_IRQ_TIMER3		(NETX_IRQ_VIC_START + 29)
#define NETX_IRQ_TIMER4		(NETX_IRQ_VIC_START + 30)
/* int 31 is reserved */

#define NETX_IRQS 		(NETX_IRQ_VIC_START + 32)

/* for multiplexed irqs on gpio 0..14 */
#define NETX_IRQ_GPIO(x) (NETX_IRQS + (x))
#define NETX_IRQ_GPIO_LAST NETX_IRQ_GPIO(14)

/* Host interface interrupts */
#define NETX_IRQ_HIF_CHAINED(x)    (NETX_IRQ_GPIO_LAST + 1 + (x))
#define NETX_IRQ_HIF_PIO35         NETX_IRQ_HIF_CHAINED(0)
#define NETX_IRQ_HIF_PIO36         NETX_IRQ_HIF_CHAINED(1)
#define NETX_IRQ_HIF_PIO40         NETX_IRQ_HIF_CHAINED(2)
#define NETX_IRQ_HIF_PIO47         NETX_IRQ_HIF_CHAINED(3)
#define NETX_IRQ_HIF_PIO72         NETX_IRQ_HIF_CHAINED(4)
#define NETX_IRQ_HIF_LAST          NETX_IRQ_HIF_CHAINED(4)

#define NR_IRQS (NETX_IRQ_HIF_LAST + 1)
