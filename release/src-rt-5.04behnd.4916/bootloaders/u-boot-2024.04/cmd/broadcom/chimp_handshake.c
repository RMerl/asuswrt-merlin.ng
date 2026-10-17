// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 Broadcom
 */

#include <common.h>
#include <command.h>
#include <broadcom/chimp.h>

/* This command should be called after loading the nitro binaries */
static int do_chimp_hs(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	int ret = CMD_RET_USAGE;
	u32 hstatus;

	/* Returns 1, if handshake call is success */
	if (chimp_handshake_status_optee(0, &hstatus))
		ret = CMD_RET_SUCCESS;

	if (hstatus == CHIMP_HANDSHAKE_SUCCESS)
		printf("ChiMP Handshake successful\n");
	else
		printf("ERROR: ChiMP Handshake status 0x%x\n", hstatus);

	return ret;
}

U_BOOT_CMD
	(chimp_hs, 1, 1, do_chimp_hs,
	 "Verify the Chimp handshake",
	 "chimp_hs\n"
);
