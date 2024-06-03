// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2004-2007 ARM Limited.
 * Copyright (C) 2008 Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 * Copyright (C) 2015 - 2016 Xilinx, Inc, Michal Simek
 *
 * As a special exception, if other files instantiate templates or use macros
 * or inline functions from this file, or you compile this file and link it
 * with other works to produce a work based on this file, this file does not
 * by itself cause the resulting work to be covered by the GNU General Public
 * License. However the source code for this file must still be made available
 * in accordance with section (3) of the GNU General Public License.

 * This exception does not invalidate any other reasons why a work based on
 * this file might be covered by the GNU General Public License.
 */

#include <common.h>
#include <dm.h>
#include <serial.h>

#if defined(CONFIG_CPU_V6) || defined(CONFIG_CPU_V7A) || defined(CONFIG_CPU_V7R)
/*
 * ARMV6 & ARMV7
 */
#define DCC_RBIT	(1 << 30)
#define DCC_WBIT	(1 << 29)

#define write_dcc(x)	\
		__asm__ volatile ("mcr p14, 0, %0, c0, c5, 0\n" : : "r" (x))

#define read_dcc(x)	\
		__asm__ volatile ("mrc p14, 0, %0, c0, c5, 0\n" : "=r" (x))

#define status_dcc(x)	\
		__asm__ volatile ("mrc p14, 0, %0, c0, c1, 0\n" : "=r" (x))

#elif defined(CONFIG_CPU_XSCALE)
/*
 * XSCALE
 */
#define DCC_RBIT	(1 << 31)
#define DCC_WBIT	(1 << 28)

#define write_dcc(x)	\
		__asm__ volatile ("mcr p14, 0, %0, c8, c0, 0\n" : : "r" (x))

#define read_dcc(x)	\
		__asm__ volatile ("mrc p14, 0, %0, c9, c0, 0\n" : "=r" (x))

#define status_dcc(x)	\
		__asm__ volatile ("mrc p14, 0, %0, c14, c0, 0\n" : "=r" (x))

#elif defined(CONFIG_CPU_ARMV8)
/*
 * ARMV8
 */
#define DCC_RBIT	(1 << 30)
#define DCC_WBIT	(1 << 29)

#define write_dcc(x)   \
		__asm__ volatile ("msr dbgdtrtx_el0, %0\n" : : "r" (x))

#define read_dcc(x)    \
		__asm__ volatile ("mrs %0, dbgdtrrx_el0\n" : "=r" (x))

#define status_dcc(x)  \
		__asm__ volatile ("mrs %0, mdccsr_el0\n" : "=r" (x))

#else
#define DCC_RBIT	(1 << 0)
#define DCC_WBIT	(1 << 1)

#define write_dcc(x)	\
		__asm__ volatile ("mcr p14, 0, %0, c1, c0, 0\n" : : "r" (x))

#define read_dcc(x)	\
		__asm__ volatile ("mrc p14, 0, %0, c1, c0, 0\n" : "=r" (x))

#define status_dcc(x)	\
		__asm__ volatile ("mrc p14, 0, %0, c0, c0, 0\n" : "=r" (x))

#endif

#define can_read_dcc(x)	do {	\
		status_dcc(x);	\
		x &= DCC_RBIT;	\
		} while (0);

#define can_write_dcc(x) do {	\
		status_dcc(x);	\
		x &= DCC_WBIT;	\
		x = (x == 0);	\
		} while (0);

#define TIMEOUT_COUNT 0x4000000

static int arm_dcc_getc(struct udevice *dev)
{
	int ch;
	register unsigned int reg;

	do {
		can_read_dcc(reg);
	} while (!reg);
	read_dcc(ch);

	return ch;
}

static int arm_dcc_putc(struct udevice *dev, char ch)
{
	register unsigned int reg;
	unsigned int timeout_count = TIMEOUT_COUNT;

	while (--timeout_count) {
		can_write_dcc(reg);
		if (reg)
			break;
	}
	if (timeout_count == 0)
		return -EAGAIN;
	else
		write_dcc(ch);

	return 0;
}

static int arm_dcc_pending(struct udevice *dev, bool input)
{
	register unsigned int reg;

	if (input) {
		can_read_dcc(reg);
	} else {
		can_write_dcc(reg);
	}

	return reg;
}

static const struct dm_serial_ops arm_dcc_ops = {
	.putc = arm_dcc_putc,
	.pending = arm_dcc_pending,
	.getc = arm_dcc_getc,
};

static const struct udevice_id arm_dcc_ids[] = {
	{ .compatible = "arm,dcc", },
	{ }
};

U_BOOT_DRIVER(serial_dcc) = {
	.name	= "arm_dcc",
	.id	= UCLASS_SERIAL,
	.of_match = arm_dcc_ids,
	.ops	= &arm_dcc_ops,
};

#ifdef CONFIG_DEBUG_UART_ARM_DCC

#include <debug_uart.h>

static inline void _debug_uart_init(void)
{
}

static inline void _debug_uart_putc(int ch)
{
	arm_dcc_putc(NULL, ch);
}

DEBUG_UART_FUNCS
#endif
