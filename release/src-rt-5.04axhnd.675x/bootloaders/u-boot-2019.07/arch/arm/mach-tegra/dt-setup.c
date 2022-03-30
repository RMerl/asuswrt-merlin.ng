// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2010-2016, NVIDIA CORPORATION.
 */

#include <common.h>
#include <asm/arch-tegra/gpu.h>

/*
 * This function is called right before the kernel is booted. "blob" is the
 * device tree that will be passed to the kernel.
 */
int ft_system_setup(void *blob, bd_t *bd)
{
	const char *gpu_compats[] = {
#if defined(CONFIG_TEGRA124)
		"nvidia,gk20a",
#endif
#if defined(CONFIG_TEGRA210)
		"nvidia,gm20b",
#endif
	};
	int i, ret;

	/* Enable GPU node if GPU setup has been performed */
	for (i = 0; i < ARRAY_SIZE(gpu_compats); i++) {
		ret = tegra_gpu_enable_node(blob, gpu_compats[i]);
		if (ret)
			return ret;
	}

	return 0;
}
