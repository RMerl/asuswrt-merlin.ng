// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) Nelson Integration, LLC 2016
 * Author: Eric Nelson<eric@nelint.com>
 *
 */
#include <config.h>
#include <common.h>
#include <malloc.h>
#include <part.h>

static int blkc_show(cmd_tbl_t *cmdtp, int flag,
		     int argc, char * const argv[])
{
	struct block_cache_stats stats;
	blkcache_stats(&stats);

	printf("hits: %u\n"
	       "misses: %u\n"
	       "entries: %u\n"
	       "max blocks/entry: %u\n"
	       "max cache entries: %u\n",
	       stats.hits, stats.misses, stats.entries,
	       stats.max_blocks_per_entry, stats.max_entries);
	return 0;
}

static int blkc_configure(cmd_tbl_t *cmdtp, int flag,
			  int argc, char * const argv[])
{
	unsigned blocks_per_entry, max_entries;
	if (argc != 3)
		return CMD_RET_USAGE;

	blocks_per_entry = simple_strtoul(argv[1], 0, 0);
	max_entries = simple_strtoul(argv[2], 0, 0);
	blkcache_configure(blocks_per_entry, max_entries);
	printf("changed to max of %u entries of %u blocks each\n",
	       max_entries, blocks_per_entry);
	return 0;
}

static cmd_tbl_t cmd_blkc_sub[] = {
	U_BOOT_CMD_MKENT(show, 0, 0, blkc_show, "", ""),
	U_BOOT_CMD_MKENT(configure, 3, 0, blkc_configure, "", ""),
};

static __maybe_unused void blkc_reloc(void)
{
	static int relocated;

	if (!relocated) {
		fixup_cmdtable(cmd_blkc_sub, ARRAY_SIZE(cmd_blkc_sub));
		relocated = 1;
	};
}

static int do_blkcache(cmd_tbl_t *cmdtp, int flag,
		       int argc, char * const argv[])
{
	cmd_tbl_t *c;

#ifdef CONFIG_NEEDS_MANUAL_RELOC
	blkc_reloc();
#endif
	if (argc < 2)
		return CMD_RET_USAGE;

	/* Strip off leading argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_blkc_sub[0], ARRAY_SIZE(cmd_blkc_sub));

	if (!c)
		return CMD_RET_USAGE;

	return c->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	blkcache, 4, 0, do_blkcache,
	"block cache diagnostics and control",
	"show - show and reset statistics\n"
	"blkcache configure blocks entries\n"
);
