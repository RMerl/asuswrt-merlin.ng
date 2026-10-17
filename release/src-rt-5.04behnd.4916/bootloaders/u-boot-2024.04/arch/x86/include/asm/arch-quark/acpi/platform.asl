/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016, Bin Meng <bmeng.cn@gmail.com>
 */

#include <asm/acpi/statdef.asl>
#include <asm/arch/iomap.h>
#include <asm/arch/irq.h>

/*
 * The _PTS method (Prepare To Sleep) is called before the OS is
 * entering a sleep state. The sleep state number is passed in Arg0.
 */
Method(_PTS, 1)
{
}

/* The _WAK method is called on system wakeup */
Method(_WAK, 1)
{
	Return (Package() {0, 0})
}

/* ACPI global NVS */
#include "global_nvs.asl"

/* TODO: add CPU ASL support */

Scope (\_SB)
{
	#include "southcluster.asl"
}

/* Chipset specific sleep states */
#include <asm/acpi/sleepstates.asl>
