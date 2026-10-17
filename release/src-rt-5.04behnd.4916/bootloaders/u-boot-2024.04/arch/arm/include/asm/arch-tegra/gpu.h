/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2015
 *  NVIDIA Corporation <www.nvidia.com>
 */

#ifndef __ASM_ARCH_TEGRA_GPU_H
#define __ASM_ARCH_TEGRA_GPU_H

#if defined(CONFIG_TEGRA_GPU)

void tegra_gpu_config(void);

#else /* CONFIG_TEGRA_GPU */

static inline void tegra_gpu_config(void)
{
}

#endif /* CONFIG_TEGRA_GPU */

#if defined(CONFIG_OF_LIBFDT)

int tegra_gpu_enable_node(void *blob, const char *gpupath);

#else /* CONFIG_OF_LIBFDT */

static inline int tegra_gpu_enable_node(void *blob, const char *compat)
{
	return 0;
}

#endif /* CONFIG_OF_LIBFDT */

#endif	/* __ASM_ARCH_TEGRA_GPU_H */
