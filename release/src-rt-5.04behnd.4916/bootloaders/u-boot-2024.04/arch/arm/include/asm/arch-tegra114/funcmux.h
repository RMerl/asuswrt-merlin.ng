/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010-2013, NVIDIA CORPORATION.  All rights reserved.
 */

/* Tegra114 high-level function multiplexing */

#ifndef _TEGRA114_FUNCMUX_H_
#define _TEGRA114_FUNCMUX_H_

#include <asm/arch-tegra/funcmux.h>

/* Configs supported by the func mux */
enum {
	FUNCMUX_DEFAULT = 0,	/* default config */

	/* UART configs */
	FUNCMUX_UART4_GMI = 0,
};
#endif	/* _TEGRA114_FUNCMUX_H_ */
