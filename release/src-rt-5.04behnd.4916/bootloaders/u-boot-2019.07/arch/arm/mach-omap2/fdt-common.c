// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016-2017 Texas Instruments, Inc.
 */

#include <common.h>
#include <linux/libfdt.h>
#include <fdt_support.h>

#include <asm/omap_common.h>
#include <asm/omap_sec_common.h>

#ifdef CONFIG_TI_SECURE_DEVICE

/* Give zero values if not already defined */
#ifndef TI_OMAP5_SECURE_BOOT_RESV_SRAM_SZ
#define TI_OMAP5_SECURE_BOOT_RESV_SRAM_SZ (0)
#endif
#ifndef CONFIG_SECURE_RUNTIME_RESV_SRAM_SZ
#define CONFIG_SECURE_RUNTIME_RESV_SRAM_SZ (0)
#endif

int ft_hs_disable_rng(void *fdt, bd_t *bd)
{
	const char *path;
	int offs;
	int ret;

	/* Make HW RNG reserved for secure world use */
	path = "/ocp/rng";
	offs = fdt_path_offset(fdt, path);
	if (offs < 0) {
		debug("Node %s not found.\n", path);
		return 0;
	}
	ret = fdt_setprop_string(fdt, offs,
				 "status", "disabled");
	if (ret < 0) {
		printf("Could not add status property to node %s: %s\n",
		       path, fdt_strerror(ret));
		return ret;
	}
	return 0;
}

#if (CONFIG_TI_SECURE_EMIF_TOTAL_REGION_SIZE != 0)
/*
 * fdt_pack_reg - pack address and size array into the "reg"-suitable stream
 */
static int fdt_pack_reg(const void *fdt, void *buf, u64 address, u64 size)
{
	int address_cells = fdt_address_cells(fdt, 0);
	int size_cells = fdt_size_cells(fdt, 0);
	char *p = buf;

	if (address_cells == 2)
		*(fdt64_t *)p = cpu_to_fdt64(address);
	else
		*(fdt32_t *)p = cpu_to_fdt32(address);
	p += 4 * address_cells;

	if (size_cells == 2)
		*(fdt64_t *)p = cpu_to_fdt64(size);
	else
		*(fdt32_t *)p = cpu_to_fdt32(size);
	p += 4 * size_cells;

	return p - (char *)buf;
}

int ft_hs_fixup_dram(void *fdt, bd_t *bd)
{
	const char *path, *subpath;
	int offs, len;
	u32 sec_mem_start = get_sec_mem_start();
	u32 sec_mem_size = CONFIG_TI_SECURE_EMIF_TOTAL_REGION_SIZE;
	fdt32_t address_cells = cpu_to_fdt32(fdt_address_cells(fdt, 0));
	fdt32_t size_cells = cpu_to_fdt32(fdt_size_cells(fdt, 0));
	u8 temp[16]; /* Up to 64-bit address + 64-bit size */

	/* Delete any original secure_reserved node */
	path = "/reserved-memory/secure_reserved";
	offs = fdt_path_offset(fdt, path);
	if (offs >= 0)
		fdt_del_node(fdt, offs);

	/* Add new secure_reserved node */
	path = "/reserved-memory";
	offs = fdt_path_offset(fdt, path);
	if (offs < 0) {
		debug("Node %s not found\n", path);
		path = "/";
		subpath = "reserved-memory";
		offs = fdt_path_offset(fdt, path);
		offs = fdt_add_subnode(fdt, offs, subpath);
		if (offs < 0) {
			printf("Could not create %s%s node.\n", path, subpath);
			return 1;
		}
		path = "/reserved-memory";
		offs = fdt_path_offset(fdt, path);

		fdt_setprop(fdt, offs, "#address-cells", &address_cells, sizeof(address_cells));
		fdt_setprop(fdt, offs, "#size-cells", &size_cells, sizeof(size_cells));
		fdt_setprop(fdt, offs, "ranges", NULL, 0);
	}

	subpath = "secure_reserved";
	offs = fdt_add_subnode(fdt, offs, subpath);
	if (offs < 0) {
		printf("Could not create %s%s node.\n", path, subpath);
		return 1;
	}

	fdt_setprop_string(fdt, offs, "compatible", "ti,secure-memory");
	fdt_setprop_string(fdt, offs, "status", "okay");
	fdt_setprop(fdt, offs, "no-map", NULL, 0);
	len = fdt_pack_reg(fdt, temp, sec_mem_start, sec_mem_size);
	fdt_setprop(fdt, offs, "reg", temp, len);

	return 0;
}
#else
int ft_hs_fixup_dram(void *fdt, bd_t *bd) { return 0; }
#endif

int ft_hs_add_tee(void *fdt, bd_t *bd)
{
	const char *path, *subpath;
	int offs;

	extern int tee_loaded;
	if (!tee_loaded)
		return 0;

	path = "/firmware";
	offs = fdt_path_offset(fdt, path);
	if (offs < 0) {
		path = "/";
		offs = fdt_path_offset(fdt, path);
		if (offs < 0) {
			printf("Could not find root node.\n");
			return 1;
		}

		subpath = "firmware";
		offs = fdt_add_subnode(fdt, offs, subpath);
		if (offs < 0) {
			printf("Could not create %s node.\n", subpath);
			return 1;
		}
	}

	subpath = "optee";
	offs = fdt_add_subnode(fdt, offs, subpath);
	if (offs < 0) {
		printf("Could not create %s node.\n", subpath);
		return 1;
	}

	fdt_setprop_string(fdt, offs, "compatible", "linaro,optee-tz");
	fdt_setprop_string(fdt, offs, "method", "smc");

	return 0;
}

#endif
