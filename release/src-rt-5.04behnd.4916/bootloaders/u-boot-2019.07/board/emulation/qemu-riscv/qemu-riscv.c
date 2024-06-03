// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <virtio_types.h>
#include <virtio.h>

int board_init(void)
{
	/*
	 * Make sure virtio bus is enumerated so that peripherals
	 * on the virtio bus can be discovered by their drivers
	 */
	virtio_init();

	return 0;
}

int board_late_init(void)
{
	ulong kernel_start;
	ofnode chosen_node;
	int ret;

	chosen_node = ofnode_path("/chosen");
	if (!ofnode_valid(chosen_node)) {
		debug("No chosen node found, can't get kernel start address\n");
		return 0;
	}

#ifdef CONFIG_ARCH_RV64I
	ret = ofnode_read_u64(chosen_node, "riscv,kernel-start",
			      (u64 *)&kernel_start);
#else
	ret = ofnode_read_u32(chosen_node, "riscv,kernel-start",
			      (u32 *)&kernel_start);
#endif
	if (ret) {
		debug("Can't find kernel start address in device tree\n");
		return 0;
	}

	env_set_hex("kernel_start", kernel_start);

	return 0;
}

/*
 * QEMU specifies the location of Linux (supplied with the -kernel argument)
 * in the device tree using the riscv,kernel-start and riscv,kernel-end
 * properties. We currently rely on the SBI implementation of BBL to run
 * Linux and therefore embed Linux as payload in BBL. This causes an issue,
 * because BBL detects the kernel properties in the device tree and ignores
 * the Linux payload as a result. To work around this issue, we clear the
 * kernel properties before booting Linux.
 *
 * This workaround can be removed, once we do not require BBL for its SBI
 * implementation anymore.
 */
int ft_board_setup(void *blob, bd_t *bd)
{
	int chosen_offset, ret;

	chosen_offset = fdt_path_offset(blob, "/chosen");
	if (chosen_offset < 0)
		return 0;

#ifdef CONFIG_ARCH_RV64I
	ret = fdt_setprop_u64(blob, chosen_offset, "riscv,kernel-start", 0);
#else
	ret = fdt_setprop_u32(blob, chosen_offset, "riscv,kernel-start", 0);
#endif
	if (ret)
		return ret;

#ifdef CONFIG_ARCH_RV64I
	ret = fdt_setprop_u64(blob, chosen_offset, "riscv,kernel-end", 0);
#else
	ret = fdt_setprop_u32(blob, chosen_offset, "riscv,kernel-end", 0);
#endif
	if (ret)
		return ret;

	return 0;
}
