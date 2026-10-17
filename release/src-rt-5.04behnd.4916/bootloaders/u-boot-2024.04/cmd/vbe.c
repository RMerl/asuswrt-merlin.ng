// SPDX-License-Identifier: GPL-2.0+
/*
 * Verified Boot for Embedded (VBE) command
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <bloblist.h>
#include <bootmeth.h>
#include <bootstd.h>
#include <command.h>
#include <spl.h>
#include <vbe.h>

static int do_vbe_list(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	vbe_list();

	return 0;
}

static int do_vbe_select(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct bootstd_priv *std;
	struct udevice *dev;
	int ret;

	ret = bootstd_get_priv(&std);
	if (ret)
		return CMD_RET_FAILURE;
	if (argc < 2) {
		std->vbe_bootmeth = NULL;
		return 0;
	}
	if (vbe_find_by_any(argv[1], &dev))
		return CMD_RET_FAILURE;

	std->vbe_bootmeth = dev;

	return 0;
}

static int do_vbe_info(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	struct bootstd_priv *std;
	char buf[256];
	int ret, len;

	ret = bootstd_get_priv(&std);
	if (ret)
		return CMD_RET_FAILURE;
	if (!std->vbe_bootmeth) {
		printf("No VBE bootmeth selected\n");
		return CMD_RET_FAILURE;
	}
	ret = bootmeth_get_state_desc(std->vbe_bootmeth, buf, sizeof(buf));
	if (ret) {
		printf("Failed (err=%d)\n", ret);
		return CMD_RET_FAILURE;
	}
	len = strnlen(buf, sizeof(buf));
	if (len >= sizeof(buf)) {
		printf("Buffer overflow\n");
		return CMD_RET_FAILURE;
	}

	puts(buf);
	if (buf[len] != '\n')
		putc('\n');

	return 0;
}

static int do_vbe_state(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	struct vbe_handoff *handoff = NULL;
	int i;

	if (IS_ENABLED(CONFIG_BLOBLIST)) {
		handoff = bloblist_find(BLOBLISTT_VBE,
					sizeof(struct vbe_handoff));
	}
	if (!handoff) {
		printf("No VBE state\n");
		return CMD_RET_FAILURE;
	}

	printf("Phases:");
	for (i = PHASE_NONE; i < PHASE_COUNT; i++) {
		if (handoff->phases & (1 << i))
			printf(" %s", spl_phase_name(i));

	}
	if (!handoff->phases)
		printf(" (none)");
	printf("\n");

	return 0;
}

U_BOOT_LONGHELP(vbe,
	"list   - list VBE bootmeths\n"
	"vbe select - select a VBE bootmeth by sequence or name\n"
	"vbe info   - show information about a VBE bootmeth\n"
	"vbe state  - show VBE state");

U_BOOT_CMD_WITH_SUBCMDS(vbe, "Verified Boot for Embedded", vbe_help_text,
	U_BOOT_SUBCMD_MKENT(list, 1, 1, do_vbe_list),
	U_BOOT_SUBCMD_MKENT(select, 2, 1, do_vbe_select),
	U_BOOT_SUBCMD_MKENT(state, 2, 1, do_vbe_state),
	U_BOOT_SUBCMD_MKENT(info, 2, 1, do_vbe_info));
