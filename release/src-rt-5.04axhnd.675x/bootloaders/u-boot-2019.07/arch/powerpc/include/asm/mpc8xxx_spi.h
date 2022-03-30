/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Freescale non-CPM SPI Controller
 *
 * Copyright 2008 Qstreams Networks, Inc.
 */

#ifndef _ASM_MPC8XXX_SPI_H_
#define _ASM_MPC8XXX_SPI_H_

#include <asm/types.h>

#if defined(CONFIG_ARCH_MPC8308) || \
	defined(CONFIG_ARCH_MPC8313) || \
	defined(CONFIG_ARCH_MPC8315) || \
	defined(CONFIG_ARCH_MPC834X) || \
	defined(CONFIG_ARCH_MPC837X)

typedef struct spi8xxx {
	u8 res0[0x20];	/* 0x0-0x01f reserved */
	u32 mode;	/* mode register  */
	u32 event;	/* event register */
	u32 mask;	/* mask register  */
	u32 com;	/* command register */
	u32 tx;		/* transmit register */
	u32 rx;		/* receive register */
	u8 res1[0xFC8];	/* fill up to 0x1000 */
} spi8xxx_t;

#endif

#endif	/* _ASM_MPC8XXX_SPI_H_ */
