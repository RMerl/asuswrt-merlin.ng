// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 The Chromium OS Authors.
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/spl.h>
#include <asm/arch/clk.h>

#define SIGNATURE	0xdeadbeef

/* Parameters of early board initialization in SPL */
static struct spl_machine_param machine_param
		__section(".machine_param") = {
	.signature	= SIGNATURE,
	.version	= 1,
	.params		= "vmubfasirM",
	.size		= sizeof(machine_param),

	.mem_iv_size	= 0x1f,
	.mem_type	= DDR_MODE_DDR3,

	/*
	 * Set uboot_size to 0x100000 bytes.
	 *
	 * This is an overly conservative value chosen to accommodate all
	 * possible U-Boot image.  You are advised to set this value to a
	 * smaller realistic size via scripts that modifies the .machine_param
	 * section of output U-Boot image.
	 */
	.uboot_size	= 0x100000,

	.boot_source	= BOOT_MODE_OM,
	.frequency_mhz	= 800,
	.arm_freq_mhz	= 900,
	.serial_base	= 0x12c30000,
	.i2c_base	= 0x12c60000,
	.mem_manuf	= MEM_MANUF_SAMSUNG,
};

struct spl_machine_param *spl_get_machine_params(void)
{
	if (machine_param.signature != SIGNATURE) {
		/* Will hang if SIGNATURE dont match */
		while (1)
			;
	}

	return &machine_param;
}
