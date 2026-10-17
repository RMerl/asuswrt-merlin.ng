/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2016 Google Inc.
 */

#include "variant_ec.h"
#include "variant_gpio.h"
#include <acpi/acpi_table.h>
#include <asm/acpi/global_nvs.h>

DefinitionBlock(
	"dsdt.aml",
	"DSDT",
	0x02,		// DSDT revision: ACPI v2.0 and up
	OEM_ID,
	OEM_TABLE_ID,
	0x20110725	// OEM revision
)
{
	/* global NVS and variables */
	#include <asm/arch/acpi/globalnvs.asl>

	/* CPU */
	#include <asm/acpi/cpu.asl>

	Scope (\_SB) {
		Device (PCI0)
		{
			#include <asm/arch/acpi/northbridge.asl>
			#include <asm/arch/acpi/southbridge.asl>
			#include <asm/arch/acpi/pch_hda.asl>
		}
	}

	/* Chrome OS specific */
	#include <asm/acpi/chromeos.asl>

	/* Chipset specific sleep states */
	#include <asm/acpi/sleepstates.asl>

	/* Chrome OS Embedded Controller */
	Scope (\_SB.PCI0.LPCB)
	{
		/* ACPI code for EC SuperIO functions */
		#include <asm/acpi/cros_ec/superio.asl>
		/* ACPI code for EC functions */
		#include <asm/acpi/cros_ec/ec.asl>
	}

	/* Dynamic Platform Thermal Framework */
	Scope (\_SB)
	{
		/* Per board variant specific definitions. */
		#include "variant_dptf.asl"
		/* Include soc specific DPTF changes */
		#include <asm/arch/acpi/dptf.asl>
		/* Include common dptf ASL files */
		#include <asm/acpi/dptf/dptf.asl>
	}
}
