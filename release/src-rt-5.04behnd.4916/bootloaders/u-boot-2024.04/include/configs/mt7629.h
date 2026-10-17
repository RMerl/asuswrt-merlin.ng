/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Configuration for MediaTek MT7629 SoC
 *
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 */

#ifndef __MT7629_H
#define __MT7629_H

/* SPL */
#define CFG_SYS_UBOOT_BASE		(0x30000000 + CONFIG_SPL_PAD_TO)

/* DRAM */
#define CFG_SYS_SDRAM_BASE		0x40000000

#endif
