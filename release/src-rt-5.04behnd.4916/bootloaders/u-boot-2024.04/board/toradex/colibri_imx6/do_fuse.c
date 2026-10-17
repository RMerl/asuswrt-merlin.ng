// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014-2016, Toradex AG
 */

/*
 * Helpers for i.MX OTP fusing during module production
*/

#include <common.h>
#ifndef CONFIG_SPL_BUILD
#include <command.h>
#include <console.h>
#include <fuse.h>

static int mfgr_fuse(void)
{
	unsigned val, val6;

	fuse_sense(0, 5, &val);
	printf("Fuse 0, 5: %8x\n", val);
	fuse_sense(0, 6, &val6);
	printf("Fuse 0, 6: %8x\n", val6);
	fuse_sense(4, 3, &val);
	printf("Fuse 4, 3: %8x\n", val);
	fuse_sense(4, 2, &val);
	printf("Fuse 4, 2: %8x\n", val);
	if (val6 & 0x10) {
		puts("BT_FUSE_SEL already fused, will do nothing\n");
		return CMD_RET_FAILURE;
	}
	/* boot cfg */
	fuse_prog(0, 5, 0x00005062);
	/* BT_FUSE_SEL */
	fuse_prog(0, 6, 0x00000010);
	return CMD_RET_SUCCESS;
}

int do_mfgr_fuse(struct cmd_tbl *cmdtp, int flag, int argc,
		 char *const argv[])
{
	int ret;
	puts("Fusing...\n");
	ret = mfgr_fuse();
	if (ret == CMD_RET_SUCCESS)
		puts("done.\n");
	else
		puts("failed.\n");
	return ret;
}

int do_updt_fuse(struct cmd_tbl *cmdtp, int flag, int argc,
		 char *const argv[])
{
	unsigned val;
	int ret;
	int confirmed = argc >= 1 && !strcmp(argv[1], "-y");

	/* can be used in scripts for command availability check */
	if (argc >= 1 && !strcmp(argv[1], "-n"))
		return CMD_RET_SUCCESS;

	/* boot cfg */
	fuse_sense(0, 5, &val);
	printf("Fuse 0, 5: %8x\n", val);
	if (val & 0x10) {
		puts("Fast boot mode already fused, no need to fuse\n");
		return CMD_RET_SUCCESS;
	}
	if (!confirmed) {
		puts("Warning: Programming fuses is an irreversible operation!\n"
				"         Updating to fast boot mode prevents easy\n"
				"         downgrading to previous BSP versions.\n"
				"\nReally perform this fuse programming? <y/N>\n");
		if (!confirm_yesno())
			return CMD_RET_FAILURE;
	}
	puts("Fusing fast boot mode...\n");
	ret = fuse_prog(0, 5, 0x00005072);
	if (ret == CMD_RET_SUCCESS)
		puts("done.\n");
	else
		puts("failed.\n");
	return ret;
}

U_BOOT_CMD(
	mfgr_fuse, 1, 0, do_mfgr_fuse,
	"OTP fusing during module production",
	""
);

U_BOOT_CMD(
	updt_fuse, 2, 0, do_updt_fuse,
	"OTP fusing during module update",
	"updt_fuse [-n] [-y] - boot cfg fast boot mode fusing"
);
#endif /* CONFIG_SPL_BUILD */
