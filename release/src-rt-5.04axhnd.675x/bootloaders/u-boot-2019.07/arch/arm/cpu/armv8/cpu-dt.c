// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 NXP Semiconductor, Inc.
 */

#include <common.h>
#include <asm/psci.h>
#include <asm/system.h>
#include <asm/armv8/sec_firmware.h>

#ifdef CONFIG_ARMV8_SEC_FIRMWARE_SUPPORT
int psci_update_dt(void *fdt)
{
	/*
	 * If the PSCI in SEC Firmware didn't work, avoid to update the
	 * device node of PSCI. But still return 0 instead of an error
	 * number to support detecting PSCI dynamically and then switching
	 * the SMP boot method between PSCI and spin-table.
	 */
	if (sec_firmware_support_psci_version() == PSCI_INVALID_VER)
		return 0;
	fdt_psci(fdt);

#if defined(CONFIG_ARMV8_PSCI) && !defined(CONFIG_ARMV8_SECURE_BASE)
	/* secure code lives in RAM, keep it alive */
	fdt_add_mem_rsv(fdt, (unsigned long)__secure_start,
			__secure_end - __secure_start);
#endif

	return 0;
}
#endif
