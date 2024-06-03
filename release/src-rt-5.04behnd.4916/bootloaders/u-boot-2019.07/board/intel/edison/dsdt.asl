/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Intel Corporation
 *
 * Partially based on dsdt.asl for other x86 boards
 */

DefinitionBlock("dsdt.aml", "DSDT", 2, "U-BOOT", "U-BOOTBL", 0x00010000)
{
	/* platform specific */
	#include <asm/arch/acpi/platform.asl>
}
