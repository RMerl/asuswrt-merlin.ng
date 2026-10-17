// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2010-2016, NVIDIA CORPORATION.
 */

#include <common.h>
#include <fdtdec.h>
#include <stdlib.h>
#include <asm/arch-tegra/cboot.h>
#include <asm/arch-tegra/gpu.h>

/*
 * This function is called right before the kernel is booted. "blob" is the
 * device tree that will be passed to the kernel.
 */
int ft_system_setup(void *blob, struct bd_info *bd)
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

#if defined(CONFIG_ARM64)
void ft_mac_address_setup(void *fdt)
{
	const void *cboot_fdt = (const void *)cboot_boot_x0;
	uint8_t mac[ETH_ALEN], local_mac[ETH_ALEN];
	const char *path;
	int offset, err;

	err = cboot_get_ethaddr(cboot_fdt, local_mac);
	if (err < 0)
		memset(local_mac, 0, ETH_ALEN);

	path = fdt_get_alias(fdt, "ethernet");
	if (!path)
		return;

	debug("ethernet alias found: %s\n", path);

	offset = fdt_path_offset(fdt, path);
	if (offset < 0) {
		printf("ethernet alias points to absent node %s\n", path);
		return;
	}

	if (is_valid_ethaddr(local_mac)) {
		err = fdt_setprop(fdt, offset, "local-mac-address", local_mac,
				  ETH_ALEN);
		if (!err)
			debug("Local MAC address set: %pM\n", local_mac);
	}

	if (eth_env_get_enetaddr("ethaddr", mac)) {
		if (memcmp(local_mac, mac, ETH_ALEN) != 0) {
			err = fdt_setprop(fdt, offset, "mac-address", mac,
					  ETH_ALEN);
			if (!err)
				debug("MAC address set: %pM\n", mac);
		}
	}
}

static int ft_copy_carveout(void *dst, const void *src, const char *node)
{
	const char *names = "memory-region-names";
	struct fdt_memory carveout;
	unsigned int index = 0;
	int err, offset, len;
	const void *prop;

	while (true) {
		const char **compatibles = NULL;
		unsigned int num_compatibles;
		unsigned long flags;
		char *copy = NULL;
		const char *name;

		err = fdtdec_get_carveout(src, node, "memory-region", index,
					  &carveout, &name, &compatibles,
					  &num_compatibles, &flags);
		if (err < 0) {
			if (err != -FDT_ERR_NOTFOUND)
				printf("failed to get carveout for %s: %d\n",
				       node, err);
			else
				break;

			return err;
		}

		if (name) {
			const char *ptr = strchr(name, '@');

			if (ptr) {
				copy = strndup(name, ptr - name);
				name = copy;
			}
		} else {
			name = "carveout";
		}

		err = fdtdec_set_carveout(dst, node, "memory-region", index,
					  &carveout, name, compatibles,
					  num_compatibles, flags);
		if (err < 0) {
			printf("failed to set carveout for %s: %d\n", node,
			       err);
			return err;
		}

		if (copy)
			free(copy);

		index++;
	}

	offset = fdt_path_offset(src, node);
	if (offset < 0) {
		debug("failed to find source offset for %s: %s\n", node,
		      fdt_strerror(err));
		return err;
	}

	prop = fdt_getprop(src, offset, names, &len);
	if (prop) {
		offset = fdt_path_offset(dst, node);
		if (offset < 0) {
			debug("failed to find destination offset for %s: %s\n",
			      node, fdt_strerror(err));
			return err;
		}

		err = fdt_setprop(dst, offset, "memory-region-names", prop,
				  len);
		if (err < 0) {
			debug("failed to copy \"%s\" property: %s\n", names,
			      fdt_strerror(err));
			return err;
		}
	}

	return 0;
}

void ft_carveout_setup(void *fdt, const char * const *nodes, unsigned int count)
{
	const void *cboot_fdt = (const void *)cboot_boot_x0;
	unsigned int i;
	int err;

	for (i = 0; i < count; i++) {
		printf("copying carveout for %s...\n", nodes[i]);

		err = ft_copy_carveout(fdt, cboot_fdt, nodes[i]);
		if (err < 0) {
			if (err != -FDT_ERR_NOTFOUND)
				printf("failed to copy carveout for %s: %d\n",
				       nodes[i], err);

			continue;
		}
	}
}
#endif
