/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016, Bin Meng <bmeng.cn@gmail.com>
 */

DefinitionBlock("dsdt.aml", "DSDT", 2, "U-BOOT", "U-BOOTBL", 0x00010000)
{
	/* platform specific */
	#include <asm/arch/acpi/platform.asl>

	/* board specific */
	#include "acpi/mainboard.asl"
}
