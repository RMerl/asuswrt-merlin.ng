/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013-2015
 * NVIDIA Corporation <www.nvidia.com>
 */

/* Tegra210 high-level function multiplexing */

#ifndef _TEGRA210_FUNCMUX_H_
#define _TEGRA210_FUNCMUX_H_

#include <asm/arch-tegra/funcmux.h>

/* Configs supported by the func mux */
enum {
	FUNCMUX_DEFAULT = 0,	/* default config */

	/* UART configs */
	FUNCMUX_UART1_UART1 = 0,
	FUNCMUX_UART4_UART4 = 0,
};
#endif	/* _TEGRA210_FUNCMUX_H_ */
