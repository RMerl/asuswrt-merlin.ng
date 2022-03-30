// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Xilinx, Inc.
 */
#include <common.h>
#include <command.h>
#include <clk.h>
#if defined(CONFIG_DM) && defined(CONFIG_CLK)
#include <dm.h>
#include <dm/device-internal.h>
#endif

int __weak soc_clk_dump(void)
{
#if defined(CONFIG_DM) && defined(CONFIG_CLK)
	struct udevice *dev;
	struct uclass *uc;
	struct clk clk;
	int ret;
	ulong rate;

	/* Device addresses start at 1 */
	ret = uclass_get(UCLASS_CLK, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(dev, uc) {
		memset(&clk, 0, sizeof(clk));
		ret = device_probe(dev);
		if (ret)
			goto noclk;

		ret = clk_request(dev, &clk);
		if (ret)
			goto noclk;

		rate = clk_get_rate(&clk);
		clk_free(&clk);

		if (rate == -ENODEV)
			goto noclk;

		printf("%-30.30s : %lu Hz\n", dev->name, rate);
		continue;
	noclk:
		printf("%-30.30s : ? Hz\n", dev->name);
	}

	return 0;
#else
	puts("Not implemented\n");
	return 1;
#endif
}

static int do_clk_dump(cmd_tbl_t *cmdtp, int flag, int argc,
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

static cmd_tbl_t cmd_clk_sub[] = {
	U_BOOT_CMD_MKENT(dump, 1, 1, do_clk_dump, "", ""),
};

static int do_clk(cmd_tbl_t *cmdtp, int flag, int argc,
		  char *const argv[])
{
	cmd_tbl_t *c;

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

#ifdef CONFIG_SYS_LONGHELP
static char clk_help_text[] =
	"dump - Print clock frequencies";
#endif

U_BOOT_CMD(clk, 2, 1, do_clk, "CLK sub-system", clk_help_text);
