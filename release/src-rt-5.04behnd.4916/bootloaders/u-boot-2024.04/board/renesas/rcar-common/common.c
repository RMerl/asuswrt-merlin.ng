// SPDX-License-Identifier: GPL-2.0
/*
 * board/renesas/rcar-common/common.c
 *
 * Copyright (C) 2013 Renesas Electronics Corporation
 * Copyright (C) 2013 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * Copyright (C) 2015 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 */

#include <dm.h>
#include <fdt_support.h>
#include <hang.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <dm/uclass-internal.h>
#include <asm/arch/renesas.h>
#include <linux/libfdt.h>

#ifdef CONFIG_RCAR_64

DECLARE_GLOBAL_DATA_PTR;

/* If the firmware passed a device tree use it for e.g. U-Boot DRAM setup. */
extern u64 rcar_atf_boot_args[];

#define FDT_RPC_PATH	"/soc/spi@ee200000"

static void apply_atf_overlay(void *fdt_blob)
{
	void *atf_fdt_blob = (void *)(rcar_atf_boot_args[1]);

	if (fdt_magic(atf_fdt_blob) == FDT_MAGIC)
		fdt_overlay_apply_node(fdt_blob, 0, atf_fdt_blob, 0);
}

int fdtdec_board_setup(const void *fdt_blob)
{
	apply_atf_overlay((void *)fdt_blob);

	return 0;
}

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	fdtdec_setup_memory_banksize();

	return 0;
}

int __weak board_init(void)
{
	return 0;
}

#if defined(CONFIG_RCAR_GEN3)
#define RST_BASE	0xE6160000
#define RST_CA57RESCNT	(RST_BASE + 0x40)
#define RST_CA53RESCNT	(RST_BASE + 0x44)
#define RST_RSTOUTCR	(RST_BASE + 0x58)
#define RST_CA57_CODE	0xA5A5000F
#define RST_CA53_CODE	0x5A5A000F

void __weak reset_cpu(void)
{
	unsigned long midr, cputype;

	asm volatile("mrs %0, midr_el1" : "=r" (midr));
	cputype = (midr >> 4) & 0xfff;

	if (cputype == 0xd03)
		writel(RST_CA53_CODE, RST_CA53RESCNT);
	else if (cputype == 0xd07)
		writel(RST_CA57_CODE, RST_CA57RESCNT);
	else
		hang();
}
#elif defined(CONFIG_RCAR_GEN4)
#define RST_BASE	0xE6160000 /* Domain0 */
#define RST_SRESCR0	(RST_BASE + 0x18)
#define RST_SPRES	0x5AA58000

void __weak reset_cpu(void)
{
	writel(RST_SPRES, RST_SRESCR0);
}
#else
#error Neither CONFIG_RCAR_GEN3 nor CONFIG_RCAR_GEN4 are set
#endif

#if defined(CONFIG_OF_BOARD_SETUP)
static int is_mem_overlap(void *blob, int first_mem_node, int curr_mem_node)
{
	struct fdt_resource first_mem_res, curr_mem_res;
	int curr_mem_reg, first_mem_reg = 0;
	int ret;

	for (;;) {
		ret = fdt_get_resource(blob, first_mem_node, "reg",
				       first_mem_reg++, &first_mem_res);
		if (ret) /* No more entries, no overlap found */
			return 0;

		curr_mem_reg = 0;
		for (;;) {
			ret = fdt_get_resource(blob, curr_mem_node, "reg",
					       curr_mem_reg++, &curr_mem_res);
			if (ret) /* No more entries, check next tuple */
				break;

			if (curr_mem_res.end < first_mem_res.start)
				continue;

			if (curr_mem_res.start >= first_mem_res.end)
				continue;

			log_debug("Overlap found: 0x%llx..0x%llx / 0x%llx..0x%llx\n",
				  first_mem_res.start, first_mem_res.end,
				  curr_mem_res.start, curr_mem_res.end);

			return 1;
		}
	}

	return 0;
}

static void scrub_duplicate_memory(void *blob)
{
	/*
	 * Scrub duplicate /memory@* node entries here. Some R-Car DTs might
	 * contain multiple /memory@* nodes, however fdt_fixup_memory_banks()
	 * either generates single /memory node or updates the first /memory
	 * node. Any remaining memory nodes are thus potential duplicates.
	 *
	 * However, it is not possible to delete all the memory nodes right
	 * away, since some of those might not be DRAM memory nodes, but some
	 * sort of other memory. Thus, delete only the memory nodes which are
	 * in the R-Car3 DBSC ranges.
	 */
	int mem = 0, first_mem_node = 0;

	for (;;) {
		mem = fdt_node_offset_by_prop_value(blob, mem,
						    "device_type", "memory", 7);
		if (mem < 0)
			break;
		if (!fdtdec_get_is_enabled(blob, mem))
			continue;

		/* First memory node, patched by U-Boot */
		if (!first_mem_node) {
			first_mem_node = mem;
			continue;
		}

		/* Check the remaining nodes and delete duplicates */
		if (!is_mem_overlap(blob, first_mem_node, mem))
			continue;

		/* Delete duplicate node, start again */
		fdt_del_node(blob, mem);
		first_mem_node = 0;
		mem = 0;
	}
}

static void update_rpc_status(void *blob)
{
	void *atf_fdt_blob = (void *)(rcar_atf_boot_args[1]);
	int offset, enabled;

	/*
	 * Check if the DT fragment received from TF-A had its RPC-IF device node
	 * enabled.
	 */
	if (fdt_magic(atf_fdt_blob) != FDT_MAGIC)
		return;

	offset = fdt_path_offset(atf_fdt_blob, FDT_RPC_PATH);
	if (offset < 0)
		return;

	enabled = fdtdec_get_is_enabled(atf_fdt_blob, offset);
	if (!enabled)
		return;

	/*
	 * Find the RPC-IF device node, and enable it if it has a flash subnode.
	 */
	offset = fdt_path_offset(blob, FDT_RPC_PATH);
	if (offset < 0)
		return;

	if (fdt_subnode_offset(blob, offset, "flash") < 0)
		return;

	fdt_status_okay(blob, offset);
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	apply_atf_overlay(blob);
	scrub_duplicate_memory(blob);
	update_rpc_status(blob);

	return 0;
}
#endif
#endif
