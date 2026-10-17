/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG EXYNOS5420 SoC
 */

#ifndef __CONFIG_EXYNOS5420_H
#define __CONFIG_EXYNOS5420_H

#define CFG_IRAM_TOP			0x02074000

#define CFG_PHY_IRAM_BASE		0x02020000

/*
 * Low Power settings
 */
#define CFG_LOWPOWER_FLAG		0x02020028
#define CFG_LOWPOWER_ADDR		0x0202002C

#endif	/* __CONFIG_EXYNOS5420_H */
