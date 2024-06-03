// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 */

#include <common.h>
#include <dm/ofnode.h>
#include <dm/util.h>
#include <linux/libfdt.h>
#include <vsprintf.h>

#ifdef CONFIG_DM_WARN
void dm_warn(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}
#endif

int list_count_items(struct list_head *head)
{
	struct list_head *node;
	int count = 0;

	list_for_each(node, head)
		count++;

	return count;
}

bool dm_ofnode_pre_reloc(ofnode node)
{
#if defined(CONFIG_SPL_BUILD) || defined(CONFIG_TPL_BUILD)
	/* for SPL and TPL the remaining nodes after the fdtgrep 1st pass
	 * had property dm-pre-reloc or u-boot,dm-spl/tpl.
	 * They are removed in final dtb (fdtgrep 2nd pass)
	 */
	return true;
#else
	if (ofnode_read_bool(node, "u-boot,dm-pre-reloc"))
		return true;

	/*
	 * In regular builds individual spl and tpl handling both
	 * count as handled pre-relocation for later second init.
	 */
	if (ofnode_read_bool(node, "u-boot,dm-spl") ||
	    ofnode_read_bool(node, "u-boot,dm-tpl"))
		return true;

	return false;
#endif
}
