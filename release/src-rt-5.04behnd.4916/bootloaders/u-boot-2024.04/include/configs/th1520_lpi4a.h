/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2023 Yixun Lan <dlan@gentoo.org>
 *
 */

#ifndef __TH1520_LPI4A_H
#define __TH1520_LPI4A_H

#include <linux/sizes.h>

#define CFG_SYS_SDRAM_BASE		0x00000000

#define UART_BASE	0xffe7014000
#define UART_REG_WIDTH  32

/* Environment options */

#define CFG_EXTRA_ENV_SETTINGS \
	"PS1=[LPi4A]# \0"

#endif /* __TH1520_LPI4A_H */
