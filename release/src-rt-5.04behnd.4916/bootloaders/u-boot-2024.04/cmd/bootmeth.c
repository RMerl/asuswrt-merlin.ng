// SPDX-License-Identifier: GPL-2.0+
/*
 * 'bootmeth' command
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <bootdev.h>
#include <bootmeth.h>
#include <bootstd.h>
#include <command.h>
#include <dm.h>
#include <malloc.h>
#include <dm/uclass-internal.h>

static int do_bootmeth_list(struct cmd_tbl *cmdtp, int flag, int argc,
			    char *const argv[])
{
	struct bootstd_priv *std;
	struct udevice *dev;
	bool use_order;
	bool all = false;
	int ret;
	int i;

	if (argc > 1 && *argv[1] == '-') {
		all = strchr(argv[1], 'a');
		argc--;
		argv++;
	}

	ret = bootstd_get_priv(&std);
	if (ret) {
		printf("Cannot get bootstd (err=%d)\n", ret);
		return CMD_RET_FAILURE;
	}

	printf("Order  Seq  Name                Description\n");
	printf("-----  ---  ------------------  ------------------\n");

	/*
	 * Use the ordering if we have one, so long as we are not trying to list
	 * all bootmethds
	 */
	use_order = std->bootmeth_count && !all;
	if (use_order)
		dev = std->bootmeth_order[0];
	else
		ret = uclass_find_first_device(UCLASS_BOOTMETH, &dev);

	for (i = 0; dev;) {
		struct bootmeth_uc_plat *ucp = dev_get_uclass_plat(dev);
		int order = i;

		/*
		 * With the -a flag we may list bootdevs that are not in the
		 * ordering. Find their place in the order
		 */
		if (all && std->bootmeth_count) {
			int j;

			/* Find the position of this bootmeth in the order */
			order = -1;
			for (j = 0; j < std->bootmeth_count; j++) {
				if (std->bootmeth_order[j] == dev)
					order = j;
			}
		}

		if (ucp->flags & BOOTMETHF_GLOBAL)
			printf("%5s", "glob");
		else if (order == -1)
			printf("%5s", "-");
		else
			printf("%5x", order);
		printf("  %3x  %-19.19s %s\n", dev_seq(dev), dev->name,
		       ucp->desc);
		i++;
		if (use_order)
			dev = std->bootmeth_order[i];
		else
			uclass_find_next_device(&dev);
	}
	printf("-----  ---  ------------------  ------------------\n");
	printf("(%d bootmeth%s)\n", i, i != 1 ? "s" : "");

	return 0;
}

static int do_bootmeth_order(struct cmd_tbl *cmdtp, int flag, int argc,
			     char *const argv[])
{
	int ret;

	ret = bootmeth_set_order(argv[1]);
	if (ret) {
		printf("Failed (err=%d)\n", ret);
		return CMD_RET_FAILURE;
	}
	env_set("bootmeths", argv[1]);

	return 0;
}

U_BOOT_LONGHELP(bootmeth,
	"list [-a]     - list available bootmeths (-a all)\n"
	"bootmeth order [<bd> ...]  - select bootmeth order / subset to use");

U_BOOT_CMD_WITH_SUBCMDS(bootmeth, "Boot methods", bootmeth_help_text,
	U_BOOT_SUBCMD_MKENT(list, 2, 1, do_bootmeth_list),
	U_BOOT_SUBCMD_MKENT(order, CONFIG_SYS_MAXARGS, 1, do_bootmeth_order));
