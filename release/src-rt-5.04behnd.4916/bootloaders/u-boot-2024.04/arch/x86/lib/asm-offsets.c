// SPDX-License-Identifier: GPL-2.0+
/*
 * Adapted from Linux v2.6.36 kernel: arch/powerpc/kernel/asm-offsets.c
 *
 * This program is used to generate definitions needed by
 * assembly language modules.
 *
 * We use the technique used in the OSF Mach kernel code:
 * generate asm statements containing #defines,
 * compile this file to assembler, and then extract the
 * #defines from the assembly-language output.
 */

#include <common.h>
#include <asm/global_data.h>
#include <linux/kbuild.h>

int main(void)
{
	DEFINE(GD_BIST, offsetof(gd_t, arch.bist));
#ifdef CONFIG_USE_HOB
	DEFINE(GD_HOB_LIST, offsetof(gd_t, arch.hob_list));
#endif
	DEFINE(GD_TABLE, offsetof(gd_t, arch.table));
	return 0;
}
