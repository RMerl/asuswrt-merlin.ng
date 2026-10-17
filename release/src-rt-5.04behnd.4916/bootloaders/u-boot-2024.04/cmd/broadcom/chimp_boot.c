// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 Broadcom
 */

#include <common.h>
#include <command.h>
#include <broadcom/chimp.h>

static int do_chimp_fastboot_secure(struct cmd_tbl *cmdtp, int flag, int argc,
				    char *const argv[])
{
	u32 health = 0;

	if (chimp_health_status_optee(&health)) {
		pr_err("Chimp health command fail\n");
		return CMD_RET_FAILURE;
	}

	if (health == BCM_CHIMP_RUNNIG_GOOD) {
		printf("skip fastboot...\n");
		return CMD_RET_SUCCESS;
	}

	if (chimp_fastboot_optee()) {
		pr_err("Failed to load secure ChiMP image\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD
	(chimp_ld_secure, 1, 0, do_chimp_fastboot_secure,
	 "Invoke chimp fw load via optee",
	 "chimp_ld_secure\n"
);
