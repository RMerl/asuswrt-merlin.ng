// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Linaro
 * Jon Medhurst <tixy@linaro.org>
 *
 * TC2 specific code for Versatile Express.
 */

#include <asm/armv7.h>
#include <asm/io.h>
#include <asm/u-boot.h>
#include <common.h>
#include <linux/libfdt.h>

#define SCC_BASE	0x7fff0000

bool armv7_boot_nonsec_default(void)
{
#ifdef CONFIG_ARMV7_BOOT_SEC_DEFAULT
	return false;
#else
	/*
	 * The Serial Configuration Controller (SCC) register at address 0x700
	 * contains flags for configuring the behaviour of the Boot Monitor
	 * (which CPUs execute from reset). Two of these bits are of interest:
	 *
	 * bit 12 = Use per-cpu mailboxes for power management
	 * bit 13 = Power down the non-boot cluster
	 *
	 * It is only when both of these are false that U-Boot's current
	 * implementation of 'nonsec' mode can work as expected because we
	 * rely on getting all CPUs to execute _nonsec_init, so let's check that.
	 */
	return (readl((u32 *)(SCC_BASE + 0x700)) & ((1 << 12) | (1 << 13))) == 0;
#endif
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *fdt, bd_t *bd)
{
	int offset, tmp, len;
	const struct fdt_property *prop;
	const char *cci_compatible = "arm,cci-400-ctrl-if";

#ifdef CONFIG_ARMV7_NONSEC
	if (!armv7_boot_nonsec())
		return 0;
#else
	return 0;
#endif
	/* Booting in nonsec mode, disable CCI access */
	offset = fdt_path_offset(fdt, "/cpus");
	if (offset < 0) {
		printf("couldn't find /cpus\n");
		return offset;
	}

	/* delete cci-control-port in each cpu node */
	for (tmp = fdt_first_subnode(fdt, offset); tmp >= 0;
	     tmp = fdt_next_subnode(fdt, tmp))
		fdt_delprop(fdt, tmp, "cci-control-port");

	/* disable all ace cci slave ports */
	offset = fdt_node_offset_by_prop_value(fdt, offset, "compatible",
					       cci_compatible, 20);
	while (offset > 0) {
		prop = fdt_get_property(fdt, offset, "interface-type",
					&len);
		if (!prop)
			continue;
		if (len < 4)
			continue;
		if (strcmp(prop->data, "ace"))
			continue;

		fdt_setprop_string(fdt, offset, "status", "disabled");

		offset = fdt_node_offset_by_prop_value(fdt, offset, "compatible",
						       cci_compatible, 20);
	}

	return 0;
}
#endif /* CONFIG_OF_BOARD_SETUP */
