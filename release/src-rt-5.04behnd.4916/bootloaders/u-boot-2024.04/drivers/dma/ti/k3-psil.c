// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com
 *  Author: Peter Ujfalusi <peter.ujfalusi@ti.com>
 */

#include <linux/kernel.h>
#include <linux/err.h>

#include "k3-psil-priv.h"

static const struct psil_ep_map *soc_ep_map;

struct psil_endpoint_config *psil_get_ep_config(u32 thread_id)
{
	int i;

	if (!soc_ep_map) {
		if (IS_ENABLED(CONFIG_SOC_K3_AM654))
			soc_ep_map = &am654_ep_map;
		else if (IS_ENABLED(CONFIG_SOC_K3_J721E))
			soc_ep_map = &j721e_ep_map;
		else if (IS_ENABLED(CONFIG_SOC_K3_J721S2))
			soc_ep_map = &j721s2_ep_map;
		else if (IS_ENABLED(CONFIG_SOC_K3_AM642))
			soc_ep_map = &am64_ep_map;
		else if (IS_ENABLED(CONFIG_SOC_K3_AM625))
			soc_ep_map = &am62_ep_map;
		else if (IS_ENABLED(CONFIG_SOC_K3_AM62A7))
			soc_ep_map = &am62a_ep_map;
	}

	if (thread_id & K3_PSIL_DST_THREAD_ID_OFFSET && soc_ep_map->dst) {
		/* check in destination thread map */
		for (i = 0; i < soc_ep_map->dst_count; i++) {
			if (soc_ep_map->dst[i].thread_id == thread_id)
				return &soc_ep_map->dst[i].ep_config;
		}
	}

	thread_id &= ~K3_PSIL_DST_THREAD_ID_OFFSET;
	if (soc_ep_map->src) {
		for (i = 0; i < soc_ep_map->src_count; i++) {
			if (soc_ep_map->src[i].thread_id == thread_id)
				return &soc_ep_map->src[i].ep_config;
		}
	}

	return ERR_PTR(-ENOENT);
}
