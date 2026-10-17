// SPDX-License-Identifier: GPL-2.0
/*
 * This file implements basic PSCI support for Renesas r8a779a0 SoC
 *
 * Copyright (C) 2020 Renesas Electronics Corp.
 *
 */

#include <asm/io.h>
#include <asm/psci.h>
#include <asm/secure.h>

int __secure psci_features(u32 function_id, u32 psci_fid)
{
	switch (psci_fid) {
	case ARM_PSCI_0_2_FN_PSCI_VERSION:
	case ARM_PSCI_0_2_FN_SYSTEM_RESET:
		return 0x0;
	}
	/* case ARM_PSCI_0_2_FN_CPU_ON: */
	/* case ARM_PSCI_0_2_FN_CPU_OFF: */
	/* case ARM_PSCI_0_2_FN_AFFINITY_INFO: */
	/* case ARM_PSCI_0_2_FN_MIGRATE_INFO_TYPE: */
	/* case ARM_PSCI_0_2_FN_SYSTEM_OFF: */
	return ARM_PSCI_RET_NI;
}

u32 __secure psci_version(void)
{
	return ARM_PSCI_VER_0_2;
}

#define RST_BASE	0xE6160000 /* Domain0 */
#define RST_SRESCR0	(RST_BASE + 0x18)
#define RST_SPRES	0x5AA58000

void __secure __noreturn psci_system_reset(void)
{
	writel(RST_SPRES, RST_SRESCR0);

	while (1)
		;
}

int psci_update_dt(void *fdt)
{
	return 0;
}
