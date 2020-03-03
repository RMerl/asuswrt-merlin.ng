/*
 * linux/arch/arm/mach-footbridge/personal.c
 *
 * Personal server (Skiff) machine fixup
 */
#include <linux/init.h>
#include <linux/spinlock.h>

#include <asm/hardware/dec21285.h>
#include <asm/mach-types.h>

#include <asm/mach/arch.h>

#include "common.h"

MACHINE_START(PERSONAL_SERVER, "Compaq-PersonalServer")
	/* Maintainer: Jamey Hicks / George France */
	.atag_offset	= 0x100,
	.map_io		= footbridge_map_io,
	.init_irq	= footbridge_init_irq,
	.init_time	= footbridge_timer_init,
	.restart	= footbridge_restart,
MACHINE_END

