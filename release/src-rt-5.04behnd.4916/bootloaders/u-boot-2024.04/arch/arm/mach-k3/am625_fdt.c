// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2023 Toradex - https://www.toradex.com/
 */

#include <asm/hardware.h>
#include "common_fdt.h"
#include <fdt_support.h>

static void fdt_fixup_cores_nodes_am625(void *blob, int core_nr)
{
	char node_path[32];

	if (core_nr < 1)
		return;

	for (; core_nr < 4; core_nr++) {
		snprintf(node_path, sizeof(node_path), "/cpus/cpu@%d", core_nr);
		fdt_del_node_path(blob, node_path);
		snprintf(node_path, sizeof(node_path), "/cpus/cpu-map/cluster0/core%d", core_nr);
		fdt_del_node_path(blob, node_path);
		snprintf(node_path, sizeof(node_path), "/bus@f0000/watchdog@e0%d0000", core_nr);
		fdt_del_node_path(blob, node_path);
	}
}

static void fdt_fixup_gpu_nodes_am625(void *blob, int has_gpu)
{
	if (!has_gpu) {
		fdt_del_node_path(blob, "/bus@f0000/gpu@fd00000");
		fdt_del_node_path(blob, "/bus@f0000/watchdog@e0f0000");
	}
}

static void fdt_fixup_pru_node_am625(void *blob, int has_pru)
{
	if (!has_pru)
		fdt_del_node_path(blob, "/bus@f0000/pruss@30040000");
}

int ft_system_setup(void *blob, struct bd_info *bd)
{
	fdt_fixup_cores_nodes_am625(blob, k3_get_core_nr());
	fdt_fixup_gpu_nodes_am625(blob, k3_has_gpu());
	fdt_fixup_pru_node_am625(blob, k3_has_pru());

	return 0;
}
