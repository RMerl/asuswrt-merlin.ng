/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * On-chip UART initializaion for low-level debugging
 *
 * Copyright (C) 2014-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <linux/serial_reg.h>
#include <linux/linkage.h>

#include "../bcu/bcu-regs.h"
#include "../sc-regs.h"
#include "../sg-regs.h"

#if !defined(CONFIG_DEBUG_SEMIHOSTING)
#include CONFIG_DEBUG_LL_INCLUDE
#endif

#define SG_REVISION_TYPE_SHIFT		16
#define SG_REVISION_TYPE_MASK		(0xff << SG_REVISION_TYPE_SHIFT)
#define BAUDRATE		115200
#define DIV_ROUND(x, d)		(((x) + ((d) / 2)) / (d))

.macro	sg_set_pinsel, pin, muxval, mux_bits, reg_stride, ra, rd
	ldr	\ra, =(SG_PINCTRL_BASE + \pin * \mux_bits / 32 * \reg_stride)
	ldr	\rd, [\ra]
	and	\rd, \rd, #~(((1 << \mux_bits) - 1) << (\pin * \mux_bits % 32))
	orr	\rd, \rd, #(\muxval << (\pin * \mux_bits % 32))
	str	\rd, [\ra]
.endm

ENTRY(debug_ll_init)
	ldr		r0, =SG_REVISION
	ldr		r1, [r0]
	and		r1, r1, #SG_REVISION_TYPE_MASK
	mov		r1, r1, lsr #SG_REVISION_TYPE_SHIFT

#if defined(CONFIG_ARCH_UNIPHIER_LD4)
#define UNIPHIER_LD4_UART_CLK		36864000
	cmp		r1, #0x26
	bne		ld4_end

	ldr		r0, =SG_IECTRL
	ldr		r1, [r0]
	orr		r1, r1, #1
	str		r1, [r0]

	sg_set_pinsel	88, 1, 8, 4, r0, r1	@ HSDOUT6 -> TXD0

	ldr		r3, =DIV_ROUND(UNIPHIER_LD4_UART_CLK, 16 * BAUDRATE)

	b		init_uart
ld4_end:
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO4)
#define UNIPHIER_PRO4_UART_CLK		73728000
	cmp		r1, #0x28
	bne		pro4_end

	sg_set_pinsel	128, 0, 4, 8, r0, r1	@ TXD0 -> TXD0

	ldr		r0, =SG_LOADPINCTRL
	mov		r1, #1
	str		r1, [r0]

	ldr		r0, =SC_CLKCTRL
	ldr		r1, [r0]
	orr		r1, r1, #SC_CLKCTRL_CEN_PERI
	str		r1, [r0]

	ldr		r3, =DIV_ROUND(UNIPHIER_PRO4_UART_CLK, 16 * BAUDRATE)

	b		init_uart
pro4_end:
#endif
#if defined(CONFIG_ARCH_UNIPHIER_SLD8)
#define UNIPHIER_SLD8_UART_CLK		80000000
	cmp		r1, #0x29
	bne		sld8_end

	ldr		r0, =SG_IECTRL
	ldr		r1, [r0]
	orr		r1, r1, #1
	str		r1, [r0]

	sg_set_pinsel	70, 3, 8, 4, r0, r1	@ HSDOUT0 -> TXD0

	ldr		r3, =DIV_ROUND(UNIPHIER_SLD8_UART_CLK, 16 * BAUDRATE)

	b		init_uart
sld8_end:
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO5)
#define UNIPHIER_PRO5_UART_CLK		73728000
	cmp		r1, #0x2A
	bne		pro5_end

	sg_set_pinsel	47, 0, 4, 8, r0, r1	@ TXD0 -> TXD0
	sg_set_pinsel	49, 0, 4, 8, r0, r1	@ TXD1 -> TXD1
	sg_set_pinsel	51, 0, 4, 8, r0, r1	@ TXD2 -> TXD2
	sg_set_pinsel	53, 0, 4, 8, r0, r1	@ TXD3 -> TXD3

	ldr		r0, =SG_LOADPINCTRL
	mov		r1, #1
	str		r1, [r0]

	ldr		r0, =SC_CLKCTRL
	ldr		r1, [r0]
	orr		r1, r1, #SC_CLKCTRL_CEN_PERI
	str		r1, [r0]

	ldr		r3, =DIV_ROUND(UNIPHIER_PRO5_UART_CLK, 16 * BAUDRATE)

	b		init_uart
pro5_end:
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PXS2)
#define UNIPHIER_PXS2_UART_CLK		88900000
	cmp		r1, #0x2E
	bne		pxs2_end

	ldr		r0, =SG_IECTRL
	ldr		r1, [r0]
	orr		r1, r1, #1
	str		r1, [r0]

	sg_set_pinsel	217, 8, 8, 4, r0, r1	@ TXD0 -> TXD0
	sg_set_pinsel	115, 8, 8, 4, r0, r1	@ TXD1 -> TXD1
	sg_set_pinsel	113, 8, 8, 4, r0, r1	@ TXD2 -> TXD2
	sg_set_pinsel	219, 8, 8, 4, r0, r1	@ TXD3 -> TXD3

	ldr		r0, =SC_CLKCTRL
	ldr		r1, [r0]
	orr		r1, r1, #SC_CLKCTRL_CEN_PERI
	str		r1, [r0]

	ldr		r3, =DIV_ROUND(UNIPHIER_PXS2_UART_CLK, 16 * BAUDRATE)

	b		init_uart
pxs2_end:
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD6B)
#define UNIPHIER_LD6B_UART_CLK		88900000
	cmp		r1, #0x2F
	bne		ld6b_end

	ldr		r0, =SG_IECTRL
	ldr		r1, [r0]
	orr		r1, r1, #1
	str		r1, [r0]

	sg_set_pinsel	135, 3, 8, 4, r0, r1	@ PORT10 -> TXD0
	sg_set_pinsel	115, 0, 8, 4, r0, r1	@ TXD1 -> TXD1
	sg_set_pinsel	113, 2, 8, 4, r0, r1	@ SBO0 -> TXD2

	ldr		r0, =SC_CLKCTRL
	ldr		r1, [r0]
	orr		r1, r1, #SC_CLKCTRL_CEN_PERI
	str		r1, [r0]

	ldr		r3, =DIV_ROUND(UNIPHIER_LD6B_UART_CLK, 16 * BAUDRATE)

	b		init_uart
ld6b_end:
#endif
	mov		pc, lr

init_uart:
	addruart	r0, r1, r2
	mov		r1, #UART_LCR_WLEN8 << 8
	str		r1, [r0, #0x10]
	str		r3, [r0, #0x24]

	mov		pc, lr
ENDPROC(debug_ll_init)
