/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013
 * NVIDIA Corporation <www.nvidia.com>
 */

/* Tegra124 high-level function multiplexing */

#ifndef _TEGRA124_FUNCMUX_H_
#define _TEGRA124_FUNCMUX_H_

#include <asm/arch-tegra/funcmux.h>

/* Configs supported by the func mux */
enum {
	FUNCMUX_DEFAULT = 0,	/* default config */

	/* UART configs */
	FUNCMUX_UART1_KBC = 0,
	FUNCMUX_UART4_GPIO = 0,
};
#endif	/* _TEGRA124_FUNCMUX_H_ */
