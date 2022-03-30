// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <dm.h>

/*
 * cleanup_before_linux() is called just before we call linux
 * it prepares the processor for linux
 *
 * we disable interrupt and caches.
 */
int cleanup_before_linux(void)
{
	disable_interrupts();

	cache_flush();

	return 0;
}

/* To enumerate devices on the /soc/ node, create a "simple-bus" driver */
static const struct udevice_id riscv_virtio_soc_ids[] = {
	{ .compatible = "riscv-virtio-soc" },
	{ }
};

U_BOOT_DRIVER(riscv_virtio_soc) = {
	.name = "riscv_virtio_soc",
	.id = UCLASS_SIMPLE_BUS,
	.of_match = riscv_virtio_soc_ids,
	.flags = DM_FLAG_PRE_RELOC,
};
