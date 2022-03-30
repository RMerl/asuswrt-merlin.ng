// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 */

#include <common.h>

#define NIOS_MAGIC 0x534f494e /* enable command line and initrd passing */

int do_bootm_linux(int flag, int argc, char * const argv[], bootm_headers_t *images)
{
	void (*kernel)(int, int, int, char *) = (void *)images->ep;
	char *commandline = env_get("bootargs");
	ulong initrd_start = images->rd_start;
	ulong initrd_end = images->rd_end;
	char *of_flat_tree = NULL;
#if defined(CONFIG_OF_LIBFDT)
	/* did generic code already find a device tree? */
	if (images->ft_len)
		of_flat_tree = images->ft_addr;
#endif
	if (!of_flat_tree && argc > 1)
		of_flat_tree = (char *)simple_strtoul(argv[1], NULL, 16);
	if (of_flat_tree)
		initrd_end = (ulong)of_flat_tree;

	/*
	 * allow the PREP bootm subcommand, it is required for bootm to work
	 */
	if (flag & BOOTM_STATE_OS_PREP)
		return 0;

	if ((flag != 0) && (flag != BOOTM_STATE_OS_GO))
		return 1;

	/* flushes data and instruction caches before calling the kernel */
	disable_interrupts();
	flush_dcache_all();

	debug("bootargs=%s @ 0x%lx\n", commandline, (ulong)&commandline);
	debug("initrd=0x%lx-0x%lx\n", (ulong)initrd_start, (ulong)initrd_end);
	/* kernel parameters passing
	 * r4 : NIOS magic
	 * r5 : initrd start
	 * r6 : initrd end or fdt
	 * r7 : kernel command line
	 * fdt is passed to kernel via r6, the same as initrd_end. fdt will be
	 * verified with fdt magic. when both initrd and fdt are used at the
	 * same time, fdt must follow immediately after initrd.
	 */
	kernel(NIOS_MAGIC, initrd_start, initrd_end, commandline);
	/* does not return */

	return 1;
}
