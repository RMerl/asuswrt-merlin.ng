/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

#include <linux/types.h>

void sdelay(unsigned long);

/* return_to_fel() - Return to BROM from SPL
 *
 * This returns back into the BROM after U-Boot SPL has performed its initial
 * init. It uses the provided lr and sp to do so.
 *
 * @lr:		BROM link register value (return address)
 * @sp:		BROM stack pointer
 */
void return_to_fel(uint32_t lr, uint32_t sp);

/* Board / SoC level designware gmac init */
#if !defined CONFIG_SPL_BUILD && defined CONFIG_SUN7I_GMAC
void eth_init_board(void);
#else
static inline void eth_init_board(void) {}
#endif

#endif
