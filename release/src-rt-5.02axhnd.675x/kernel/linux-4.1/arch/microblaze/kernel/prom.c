/*
 * Procedures for creating, accessing and interpreting the device tree.
 *
 * Paul Mackerras	August 1996.
 * Copyright (C) 1996-2005 Paul Mackerras.
 *
 *  Adapted for 64bit PowerPC by Dave Engebretsen and Peter Bergner.
 *    {engebret|bergner}@us.ibm.com
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 */

#include <stdarg.h>
#include <linux/export.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/threads.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/stringify.h>
#include <linux/delay.h>
#include <linux/initrd.h>
#include <linux/bitops.h>
#include <linux/kexec.h>
#include <linux/debugfs.h>
#include <linux/irq.h>
#include <linux/memblock.h>
#include <linux/of_fdt.h>

#include <asm/prom.h>
#include <asm/page.h>
#include <asm/processor.h>
#include <asm/irq.h>
#include <linux/io.h>
#include <asm/mmu.h>
#include <asm/pgtable.h>
#include <asm/sections.h>
#include <asm/pci-bridge.h>

#ifdef CONFIG_EARLY_PRINTK
static const char *stdout;

static int __init early_init_dt_scan_chosen_serial(unsigned long node,
				const char *uname, int depth, void *data)
{
	int l;
	const char *p;

	pr_debug("%s: depth: %d, uname: %s\n", __func__, depth, uname);

	if (depth == 1 && (strcmp(uname, "chosen") == 0 ||
				strcmp(uname, "chosen@0") == 0)) {
		p = of_get_flat_dt_prop(node, "linux,stdout-path", &l);
		if (p != NULL && l > 0)
			stdout = p; /* store pointer to stdout-path */
	}

	if (stdout && strstr(stdout, uname)) {
		p = of_get_flat_dt_prop(node, "compatible", &l);
		pr_debug("Compatible string: %s\n", p);

		if ((strncmp(p, "xlnx,xps-uart16550", 18) == 0) ||
			(strncmp(p, "xlnx,axi-uart16550", 18) == 0)) {
			unsigned int addr;

			*(u32 *)data = UART16550;

			addr = *(u32 *)of_get_flat_dt_prop(node, "reg", &l);
			addr += *(u32 *)of_get_flat_dt_prop(node,
							"reg-offset", &l);
			/* clear register offset */
			return be32_to_cpu(addr) & ~3;
		}
		if ((strncmp(p, "xlnx,xps-uartlite", 17) == 0) ||
				(strncmp(p, "xlnx,opb-uartlite", 17) == 0) ||
				(strncmp(p, "xlnx,axi-uartlite", 17) == 0) ||
				(strncmp(p, "xlnx,mdm", 8) == 0)) {
			const unsigned int *addrp;

			*(u32 *)data = UARTLITE;

			addrp = of_get_flat_dt_prop(node, "reg", &l);
			return be32_to_cpup(addrp); /* return address */
		}
	}
	return 0;
}

/* this function is looking for early console - Microblaze specific */
int __init of_early_console(void *version)
{
	return of_scan_flat_dt(early_init_dt_scan_chosen_serial, version);
}
#endif

void __init early_init_devtree(void *params)
{
	pr_debug(" -> early_init_devtree(%p)\n", params);

	early_init_dt_scan(params);
	if (!strlen(boot_command_line))
		strlcpy(boot_command_line, cmd_line, COMMAND_LINE_SIZE);

	parse_early_param();

	memblock_allow_resize();

	pr_debug("Phys. mem: %lx\n", (unsigned long) memblock_phys_mem_size());

	pr_debug(" <- early_init_devtree()\n");
}
