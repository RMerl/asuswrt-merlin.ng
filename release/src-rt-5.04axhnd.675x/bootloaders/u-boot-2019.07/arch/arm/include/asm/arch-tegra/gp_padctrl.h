/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2010-2015
 *  NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _TEGRA_GP_PADCTRL_H_
#define _TEGRA_GP_PADCTRL_H_

#define GP_HIDREV			0x804

/* bit fields definitions for APB_MISC_GP_HIDREV register */
#define HIDREV_CHIPID_SHIFT		8
#define HIDREV_CHIPID_MASK		(0xff << HIDREV_CHIPID_SHIFT)
#define HIDREV_MAJORPREV_SHIFT		4
#define HIDREV_MAJORPREV_MASK		(0xf << HIDREV_MAJORPREV_SHIFT)

/* CHIPID field returned from APB_MISC_GP_HIDREV register */
#define CHIPID_TEGRA20			0x20
#define CHIPID_TEGRA30			0x30
#define CHIPID_TEGRA114			0x35
#define CHIPID_TEGRA124			0x40
#define CHIPID_TEGRA210			0x21

#endif	/* _TEGRA_GP_PADCTRL_H_ */
