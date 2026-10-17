// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <bootflow.h>
#include <bootstd.h>
#include <command.h>
#include <dm.h>

/*
 * show_bootmeths() - List available bootmeths
 *
 * We could refactor this to use do_bootmeth_list() if more detail (or ordering)
 * are needed
 */
static void show_bootmeths(void)
{
	struct udevice *dev;
	struct uclass *uc;

	printf("Bootmeths: ");
	uclass_id_foreach_dev(UCLASS_BOOTMETH, dev, uc)
		printf(" %s", dev->name);
	printf("\n");
}

int bootstd_prog_boot(void)
{
	struct bootflow_iter iter;
	struct bootflow bflow;
	int ret, flags, i;

	printf("Programmatic boot starting\n");
	show_bootmeths();
	flags = BOOTFLOWIF_HUNT | BOOTFLOWIF_SHOW | BOOTFLOWIF_SKIP_GLOBAL;

	bootstd_clear_glob();
	for (i = 0, ret = bootflow_scan_first(NULL, NULL, &iter, flags, &bflow);
	     i < 1000 && ret != -ENODEV;
	     i++, ret = bootflow_scan_next(&iter, &bflow)) {
		if (!bflow.err)
			bootflow_run_boot(&iter, &bflow);
		bootflow_free(&bflow);
	}

	return -EFAULT;
}
