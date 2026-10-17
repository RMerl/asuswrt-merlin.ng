// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 */

#include <cpu_func.h>
#include <log.h>
#include <spl.h>

void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	typedef void __noreturn (*image_entry_noargs_t)(void);
	image_entry_noargs_t image_entry =
		(image_entry_noargs_t)spl_image->entry_point;

	/* Flush cache before jumping to application */
	flush_cache((unsigned long)spl_image->load_addr, spl_image->size);

	debug("image entry point: 0x%lx\n", spl_image->entry_point);
	image_entry();
}
