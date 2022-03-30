/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010-2012, NVIDIA CORPORATION.  All rights reserved.
 */

/* Tegra30 high-level function multiplexing */

#ifndef _TEGRA30_FUNCMUX_H_
#define _TEGRA30_FUNCMUX_H_

#include <asm/arch-tegra/funcmux.h>

/* Configs supported by the func mux */
enum {
	FUNCMUX_DEFAULT = 0,	/* default config */

	/* UART configs */
	FUNCMUX_UART1_ULPI = 0,
};
#endif	/* _TEGRA30_FUNCMUX_H_ */
