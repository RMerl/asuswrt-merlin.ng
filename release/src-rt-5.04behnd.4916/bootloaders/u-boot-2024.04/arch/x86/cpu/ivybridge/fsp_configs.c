// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <log.h>
#include <asm/fsp1/fsp_support.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

void fsp_update_configs(struct fsp_config_data *config,
			struct fspinit_rtbuf *rt_buf)
{
	struct platform_config *plat_config = &config->plat_config;
	struct memory_config *mem_config = &config->mem_config;
	const void *blob = gd->fdt_blob;
	int node;

	node = fdtdec_next_compatible(blob, 0, COMPAT_INTEL_IVYBRIDGE_FSP);
	if (node < 0) {
		debug("%s: Cannot find FSP node\n", __func__);
		return;
	}

	plat_config->enable_ht =
		fdtdec_get_bool(blob, node, "fsp,enable-ht");
	plat_config->enable_turbo =
		fdtdec_get_bool(blob, node, "fsp,enable-turbo");
	plat_config->enable_memory_down =
		fdtdec_get_bool(blob, node, "fsp,enable-memory-down");
	plat_config->enable_fast_boot =
		fdtdec_get_bool(blob, node, "fsp,enable-fast-boot");

	/* Initialize runtime buffer for fsp_init() */
	rt_buf->stack_top = config->common.stack_top - 32;
	rt_buf->boot_mode = config->common.boot_mode;
	rt_buf->plat_config = plat_config;

	if (plat_config->enable_memory_down)
		rt_buf->mem_config = mem_config;
	else
		rt_buf->mem_config = NULL;
}
