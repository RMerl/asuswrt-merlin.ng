/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013 Samsung Electronics
 * Rajeshwari Shinde <rajeshwari.s@samsung.com>
 */

#ifndef _EXYNOS_BOARD_H
#define _EXYNOS_BOARD_H

/*
 * Exynos baord specific changes for
 * board_init
 */
int exynos_init(void);

/*
 * Exynos board specific changes for
 * board_early_init_f
 */
int exynos_early_init_f(void);

/*
 * Exynos board specific changes for
 * board_power_init
 */
int exynos_power_init(void);

#endif	/* EXYNOS_BOARD_H */
