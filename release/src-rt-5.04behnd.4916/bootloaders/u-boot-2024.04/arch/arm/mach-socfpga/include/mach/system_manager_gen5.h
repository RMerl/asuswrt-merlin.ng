/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013-2017 Altera Corporation <www.altera.com>
 */

#ifndef _SYSTEM_MANAGER_GEN5_H_
#define _SYSTEM_MANAGER_GEN5_H_

#ifndef __ASSEMBLY__

void sysmgr_pinmux_init(void);
void sysmgr_config_warmrstcfgio(int enable);

void sysmgr_get_pinmux_table(const u8 **table, unsigned int *table_len);

#define SYSMGR_GEN5_WDDBG			0x10
#define SYSMGR_GEN5_BOOTINFO			0x14
#define SYSMGR_GEN5_FPGAINFGRP_GBL		0x20
#define SYSMGR_GEN5_FPGAINFGRP_INDIV		0x24
#define SYSMGR_GEN5_FPGAINFGRP_MODULE		0x28
#define SYSMGR_GEN5_SCANMGRGRP_CTRL		0x30
#define SYSMGR_GEN5_ISWGRP_HANDOFF		0x80
#define SYSMGR_GEN5_ROMCODEGRP_CTRL		0xc0
#define SYSMGR_GEN5_WARMRAMGRP_EN		0xe0
#define SYSMGR_GEN5_SDMMC			0x108
#define SYSMGR_GEN5_ECCGRP_OCRAM		0x144
#define SYSMGR_GEN5_EMACIO			0x400
#define SYSMGR_GEN5_NAND_USEFPGA		0x6f0
#define SYSMGR_GEN5_RGMII1_USEFPGA		0x6f8
#define SYSMGR_GEN5_SDMMC_USEFPGA		0x708
#define SYSMGR_GEN5_RGMII0_USEFPGA		0x714
#define SYSMGR_GEN5_SPIM1_USEFPGA		0x730
#define SYSMGR_GEN5_SPIM0_USEFPGA		0x738

#define SYSMGR_SDMMC				SYSMGR_GEN5_SDMMC

#define SYSMGR_ISWGRP_HANDOFF_OFFSET(i)	\
	SYSMGR_GEN5_ISWGRP_HANDOFF + ((i) * sizeof(u32))
#endif

#define SYSMGR_SDMMC_SMPLSEL_SHIFT	3
#define SYSMGR_BOOTINFO_BSEL_SHIFT	0

#endif /* _SYSTEM_MANAGER_GEN5_H_ */
