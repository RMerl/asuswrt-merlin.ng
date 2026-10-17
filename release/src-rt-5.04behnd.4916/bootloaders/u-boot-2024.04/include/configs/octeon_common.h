/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019-2020
 * Marvell <www.marvell.com>
 */

#ifndef __OCTEON_COMMON_H__
#define __OCTEON_COMMON_H__

#if defined(CONFIG_RAM_OCTEON)
#define CFG_SYS_INIT_SP_OFFSET	0x20180000
#else
/* No DDR init -> run in L2 cache with limited resources */
#define CFG_SYS_INIT_SP_OFFSET	0x00180000
#endif

#define CFG_SYS_SDRAM_BASE		0xffffffff80000000

#endif /* __OCTEON_COMMON_H__ */
