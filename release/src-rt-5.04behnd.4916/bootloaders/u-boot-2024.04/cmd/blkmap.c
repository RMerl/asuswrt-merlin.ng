// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Addiva Elektronik
 * Author: Tobias Waldekranz <tobias@waldekranz.com>
 */

#include <blk.h>
#include <blkmap.h>
#include <common.h>
#include <command.h>
#include <malloc.h>
#include <dm/device.h>

static int blkmap_curr_dev;

struct map_ctx {
	struct udevice *dev;
	lbaint_t blknr, blkcnt;
};

typedef int (*map_parser_fn)(struct map_ctx *ctx, int argc, char *const argv[]);

struct map_handler {
	const char *name;
	map_parser_fn fn;
};

static int do_blkmap_map_linear(struct map_ctx *ctx, int argc,
				char *const argv[])
{
	struct blk_desc *lbd;
	int err, ldevnum;
	lbaint_t lblknr;

	if (argc < 4)
		return CMD_RET_USAGE;

	ldevnum = dectoul(argv[2], NULL);
	lblknr = dectoul(argv[3], NULL);

	lbd = blk_get_devnum_by_uclass_idname(argv[1], ldevnum);
	if (!lbd) {
		printf("Found no device matching \"%s %d\"\n",
		       argv[1], ldevnum);
		return CMD_RET_FAILURE;
	}

	err = blkmap_map_linear(ctx->dev, ctx->blknr, ctx->blkcnt,
				lbd->bdev, lblknr);
	if (err) {
		printf("Unable to map \"%s %d\" at block 0x" LBAF ": %d\n",
		       argv[1], ldevnum, ctx->blknr, err);

		return CMD_RET_FAILURE;
	}

	printf("Block 0x" LBAF "+0x" LBAF " mapped to block 0x" LBAF " of \"%s %d\"\n",
	       ctx->blknr, ctx->blkcnt, lblknr, argv[1], ldevnum);
	return CMD_RET_SUCCESS;
}

static int do_blkmap_map_mem(struct map_ctx *ctx, int argc, char *const argv[])
{
	phys_addr_t addr;
	int err;

	if (argc < 2)
		return CMD_RET_USAGE;

	addr = hextoul(argv[1], NULL);

	err = blkmap_map_pmem(ctx->dev, ctx->blknr, ctx->blkcnt, addr);
	if (err) {
		printf("Unable to map %#llx at block 0x" LBAF ": %d\n",
		       (unsigned long long)addr, ctx->blknr, err);
		return CMD_RET_FAILURE;
	}

	printf("Block 0x" LBAF "+0x" LBAF " mapped to %#llx\n",
	       ctx->blknr, ctx->blkcnt, (unsigned long long)addr);
	return CMD_RET_SUCCESS;
}

static struct map_handler map_handlers[] = {
	{ .name = "linear", .fn = do_blkmap_map_linear },
	{ .name = "mem", .fn = do_blkmap_map_mem },

	{ .name = NULL }
};

static int do_blkmap_map(struct cmd_tbl *cmdtp, int flag,
			 int argc, char *const argv[])
{
	struct map_handler *handler;
	struct map_ctx ctx;

	if (argc < 5)
		return CMD_RET_USAGE;

	ctx.dev = blkmap_from_label(argv[1]);
	if (!ctx.dev) {
		printf("\"%s\" is not the name of any known blkmap\n", argv[1]);
		return CMD_RET_FAILURE;
	}

	ctx.blknr = hextoul(argv[2], NULL);
	ctx.blkcnt = hextoul(argv[3], NULL);
	argc -= 4;
	argv += 4;

	for (handler = map_handlers; handler->name; handler++) {
		if (!strcmp(handler->name, argv[0]))
			return handler->fn(&ctx, argc, argv);
	}

	printf("Unknown map type \"%s\"\n", argv[0]);
	return CMD_RET_USAGE;
}

static int do_blkmap_create(struct cmd_tbl *cmdtp, int flag,
			    int argc, char *const argv[])
{
	const char *label;
	int err;

	if (argc != 2)
		return CMD_RET_USAGE;

	label = argv[1];

	err = blkmap_create(label, NULL);
	if (err) {
		printf("Unable to create \"%s\": %d\n", label, err);
		return CMD_RET_FAILURE;
	}

	printf("Created \"%s\"\n", label);
	return CMD_RET_SUCCESS;
}

static int do_blkmap_destroy(struct cmd_tbl *cmdtp, int flag,
			     int argc, char *const argv[])
{
	struct udevice *dev;
	const char *label;
	int err;

	if (argc != 2)
		return CMD_RET_USAGE;

	label = argv[1];

	dev = blkmap_from_label(label);
	if (!dev) {
		printf("\"%s\" is not the name of any known blkmap\n", label);
		return CMD_RET_FAILURE;
	}

	err = blkmap_destroy(dev);
	if (err) {
		printf("Unable to destroy \"%s\": %d\n", label, err);
		return CMD_RET_FAILURE;
	}

	printf("Destroyed \"%s\"\n", label);
	return CMD_RET_SUCCESS;
}

static int do_blkmap_get(struct cmd_tbl *cmdtp, int flag,
			 int argc, char *const argv[])
{
	struct udevice *dev;
	const char *label;
	int err;

	if (argc < 3)
		return CMD_RET_USAGE;

	label = argv[1];

	dev = blkmap_from_label(label);
	if (!dev) {
		printf("\"%s\" is not the name of any known blkmap\n", label);
		return CMD_RET_FAILURE;
	}

	if (!strcmp(argv[2], "dev")) {
		if (argc == 3) {
			printf("%d\n", dev_seq(dev));
		} else {
			err = env_set_hex(argv[3], dev_seq(dev));
			if (err)
				return CMD_RET_FAILURE;
		}
	} else {
		return CMD_RET_USAGE;
	}

	return CMD_RET_SUCCESS;
}

static int do_blkmap_common(struct cmd_tbl *cmdtp, int flag,
			    int argc, char *const argv[])
{
	/* The subcommand parsing pops the original argv[0] ("blkmap")
	 * which blk_common_cmd expects. Push it back again.
	 */
	argc++;
	argv--;

	return blk_common_cmd(argc, argv, UCLASS_BLKMAP, &blkmap_curr_dev);
}

U_BOOT_CMD_WITH_SUBCMDS(
	blkmap, "Composeable virtual block devices",
	"info - list configured devices\n"
	"blkmap part - list available partitions on current blkmap device\n"
	"blkmap dev [<dev>] - show or set current blkmap device\n"
	"blkmap read <addr> <blk#> <cnt>\n"
	"blkmap write <addr> <blk#> <cnt>\n"
	"blkmap get <label> dev [<var>] - store device number in variable\n"
	"blkmap create <label> - create device\n"
	"blkmap destroy <label> - destroy device\n"
	"blkmap map <label> <blk#> <cnt> linear <interface> <dev> <blk#> - device mapping\n"
	"blkmap map <label> <blk#> <cnt> mem <addr> - memory mapping\n",
	U_BOOT_SUBCMD_MKENT(info, 2, 1, do_blkmap_common),
	U_BOOT_SUBCMD_MKENT(part, 2, 1, do_blkmap_common),
	U_BOOT_SUBCMD_MKENT(dev, 4, 1, do_blkmap_common),
	U_BOOT_SUBCMD_MKENT(read, 5, 1, do_blkmap_common),
	U_BOOT_SUBCMD_MKENT(write, 5, 1, do_blkmap_common),
	U_BOOT_SUBCMD_MKENT(get, 5, 1, do_blkmap_get),
	U_BOOT_SUBCMD_MKENT(create, 2, 1, do_blkmap_create),
	U_BOOT_SUBCMD_MKENT(destroy, 2, 1, do_blkmap_destroy),
	U_BOOT_SUBCMD_MKENT(map, 32, 1, do_blkmap_map));
