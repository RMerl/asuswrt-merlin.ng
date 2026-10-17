// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <init.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Global declaration of gd.
 *
 * As we write to it before relocation we have to make sure it is not put into
 * a .bss section which may overlap a .rela section. Initialization forces it
 * into a .data section which cannot overlap any .rela section.
 */
struct global_data *global_data_ptr = (struct global_data *)~0;

void arch_setup_gd(gd_t *new_gd)
{
	global_data_ptr = new_gd;
}

int misc_init_r(void)
{
	return 0;
}

#ifndef CONFIG_SYS_COREBOOT
int checkcpu(void)
{
	return 0;
}

int print_cpuinfo(void)
{
	return 0;
}
#endif
