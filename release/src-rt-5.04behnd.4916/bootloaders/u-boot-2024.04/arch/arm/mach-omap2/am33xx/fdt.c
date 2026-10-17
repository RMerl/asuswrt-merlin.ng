// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 Texas Instruments, Inc.
 */

#include <common.h>
#include <hang.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <malloc.h>

#include <asm/omap_common.h>
#include <asm/arch-am33xx/sys_proto.h>

#ifdef CONFIG_TI_SECURE_DEVICE

static void ft_hs_fixups(void *fdt, struct bd_info *bd)
{
	/* Check we are running on an HS/EMU device type */
	if (GP_DEVICE != get_device_type()) {
		if ((ft_hs_disable_rng(fdt, bd) == 0) &&
		    (ft_hs_fixup_dram(fdt, bd) == 0) &&
		    (ft_hs_add_tee(fdt, bd) == 0))
			return;
	} else {
		printf("ERROR: Incorrect device type (GP) detected!");
	}
	/* Fixup failed or wrong device type */
	hang();
}
#else
static void ft_hs_fixups(void *fdt, struct bd_info *bd) { }
#endif /* #ifdef CONFIG_TI_SECURE_DEVICE */

/*
 * Place for general cpu/SoC FDT fixups. Board specific
 * fixups should remain in the board files which is where
 * this function should be called from.
 */
void ft_cpu_setup(void *fdt, struct bd_info *bd)
{
	ft_hs_fixups(fdt, bd);
}
