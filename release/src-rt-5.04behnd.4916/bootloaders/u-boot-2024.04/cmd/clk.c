// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Xilinx, Inc.
 */
#include <common.h>
#include <command.h>
#include <clk.h>
#if defined(CONFIG_DM) && defined(CONFIG_CLK)
#include <dm.h>
#include <dm/device.h>
#include <dm/root.h>
#include <dm/device-internal.h>
#include <linux/clk-provider.h>
#endif

#if defined(CONFIG_DM) && defined(CONFIG_CLK)
static void show_clks(struct udevice *dev, int depth, int last_flag)
{
	int i, is_last;
	struct udevice *child;
	struct clk *clkp, *parent;
	u32 rate;

	clkp = dev_get_clk_ptr(dev);
	if (clkp) {
		parent = clk_get_parent(clkp);
		if (!IS_ERR(parent) && depth == -1)
			return;
		depth++;
		rate = clk_get_rate(clkp);

		printf(" %-12u  %8d        ", rate, clkp->enable_count);

		for (i = depth; i >= 0; i--) {
			is_last = (last_flag >> i) & 1;
			if (i) {
				if (is_last)
					printf("    ");
				else
					printf("|   ");
			} else {
				if (is_last)
					printf("`-- ");
				else
					printf("|-- ");
			}
		}

		printf("%s\n", dev->name);
	}

	device_foreach_child_probe(child, dev) {
		if (device_get_uclass_id(child) != UCLASS_CLK)
			continue;
		if (child == dev)
			continue;
		is_last = list_is_last(&child->sibling_node, &dev->child_head);
		show_clks(child, depth, (last_flag << 1) | is_last);
	}
}

static int soc_clk_dump(void)
{
	struct udevice *dev;
	const struct clk_ops *ops;

	printf(" Rate               Usecnt      Name\n");
	printf("------------------------------------------\n");

	uclass_foreach_dev_probe(UCLASS_CLK, dev)
		show_clks(dev, -1, 0);

	uclass_foreach_dev_probe(UCLASS_CLK, dev) {
		ops = dev_get_driver_ops(dev);
		if (ops && ops->dump) {
			printf("\n%s %s:\n", dev->driver->name, dev->name);
			ops->dump(dev);
		}
	}

	return 0;
}
#else
static int soc_clk_dump(void)
{
	puts("Not implemented\n");
	return 1;
}
#endif

static int do_clk_dump(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	int ret;

	ret = soc_clk_dump();
	if (ret < 0) {
		printf("Clock dump error %d\n", ret);
		ret = CMD_RET_FAILURE;
	}

	return ret;
}

#if CONFIG_IS_ENABLED(DM) && CONFIG_IS_ENABLED(CLK)
static int do_clk_setfreq(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	struct clk *clk = NULL;
	s32 freq;
	struct udevice *dev;

	if (argc != 3)
		return CMD_RET_USAGE;

	freq = dectoul(argv[2], NULL);

	if (!uclass_get_device_by_name(UCLASS_CLK, argv[1], &dev))
		clk = dev_get_clk_ptr(dev);

	if (!clk) {
		printf("clock '%s' not found.\n", argv[1]);
		return CMD_RET_FAILURE;
	}

	freq = clk_set_rate(clk, freq);
	if (freq < 0) {
		printf("set_rate failed: %d\n", freq);
		return CMD_RET_FAILURE;
	}

	printf("set_rate returns %u\n", freq);
	return 0;
}
#endif

static struct cmd_tbl cmd_clk_sub[] = {
	U_BOOT_CMD_MKENT(dump, 1, 1, do_clk_dump, "", ""),
#if CONFIG_IS_ENABLED(DM) && CONFIG_IS_ENABLED(CLK)
	U_BOOT_CMD_MKENT(setfreq, 3, 1, do_clk_setfreq, "", ""),
#endif
};

static int do_clk(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[])
{
	struct cmd_tbl *c;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* Strip off leading 'clk' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_clk_sub[0], ARRAY_SIZE(cmd_clk_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

U_BOOT_LONGHELP(clk,
	"dump - Print clock frequencies\n"
	"clk setfreq [clk] [freq] - Set clock frequency");

U_BOOT_CMD(clk, 4, 1, do_clk, "CLK sub-system", clk_help_text);
