/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2017 Intel Corporation.
 * Take from coreboot project file of the same name
 */

#ifndef _ASM_ARCH_LPC_H
#define _ASM_ARCH_LPC_H

#include <linux/bitops.h>
#define LPC_SERIRQ_CTL			0x64
#define  LPC_SCNT_EN			BIT(7)
#define  LPC_SCNT_MODE			BIT(6)
#define LPC_IO_DECODE			0x80
#define  LPC_IOD_COMA_RANGE             (0 << 0) /* 0x3F8 - 0x3FF COMA*/
#define  LPC_IOD_COMB_RANGE             (1 << 4) /* 0x2F8 - 0x2FF COMB*/
/*
 * Use IO_<peripheral>_<IO port> style macros defined in lpc_lib.h
 * to enable decoding of I/O locations for a peripheral
 */
#define LPC_IO_ENABLES			0x82
#define LPC_GENERIC_IO_RANGE(n)		((((n) & 0x3) * 4) + 0x84)
#define  LPC_LGIR_AMASK_MASK		(0xfc << 16)
#define  LPC_LGIR_ADDR_MASK		0xfffc
#define  LPC_LGIR_EN			BIT(0)
#define LPC_LGIR_MAX_WINDOW_SIZE	256
#define LPC_GENERIC_MEM_RANGE		0x98
#define  LPC_LGMR_ADDR_MASK		0xffff0000
#define  LPC_LGMR_EN			BIT(0)
#define LPC_LGMR_WINDOW_SIZE		(64 * KiB)
#define LPC_BIOS_CNTL			0xdc
#define  LPC_BC_BILD			BIT(7)
#define  LPC_BC_LE			BIT(1)
#define  LPC_BC_EISS			BIT(5)
#define LPC_PCCTL			0xE0	/* PCI Clock Control */
#define  LPC_PCCTL_CLKRUN_EN		BIT(0)

/*
 * IO decode enable macros are in the format IO_<peripheral>_<IO port>.
 * For example, to open ports 0x60, 0x64 for the keyboard controller,
 * use IOE_KBC_60_64 macro. For IOE_ macros that do not specify a port range,
 * the port range is selectable via the IO decodes register.
 */
#define LPC_IOE_EC_4E_4F		BIT(13)
#define LPC_IOE_SUPERIO_2E_2F		BIT(12)
#define LPC_IOE_EC_62_66		BIT(11)
#define LPC_IOE_KBC_60_64		BIT(10)
#define LPC_IOE_HGE_208			BIT(9)
#define LPC_IOE_LGE_200			BIT(8)
#define LPC_IOE_FDD_EN			BIT(3)
#define LPC_IOE_LPT_EN			BIT(2)
#define LPC_IOE_COMB_EN			BIT(1)
#define LPC_IOE_COMA_EN			BIT(0)
#define LPC_NUM_GENERIC_IO_RANGES	4

#define LPC_IO_ENABLES			0x82

/**
 * lpc_enable_fixed_io_ranges() - enable the fixed I/O ranges
 *
 * @io_enables: Mask of things to enable (LPC_IOE_.)
 */
void lpc_enable_fixed_io_ranges(uint io_enables);

/**
 * lpc_open_pmio_window() - Open an IO port range
 *
 * @base: Base I/O address (e.g. 0x800)
 * @size: Size of window (e.g. 0x100)
 * Return: 0 if OK, -ENOSPC if there are no more windows available, -EALREADY
 *	if already set up
 */
int lpc_open_pmio_window(uint base, uint size);

/**
 * lpc_io_setup_comm_a_b() - Set up basic serial UARTs
 *
 * Set up the LPC to handle I/O to the COMA/COMB serial UART addresses
 * 2f8-2ff and 3f8-3ff.
 */
void lpc_io_setup_comm_a_b(void);

#endif
