// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021, Foundries.IO
 *
 */

#include <common.h>
#include <command.h>
#include <env.h>
#include <scp03.h>

int do_scp03_enable(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	if (argc != 1)
		return CMD_RET_USAGE;

	if (tee_enable_scp03()) {
		printf("TEE failed to enable SCP03\n");
		return CMD_RET_FAILURE;
	}

	printf("SCP03 is enabled\n");

	return CMD_RET_SUCCESS;
}

int do_scp03_provision(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	if (argc != 1)
		return CMD_RET_USAGE;

	if (tee_provision_scp03()) {
		printf("TEE failed to provision SCP03 keys\n");
		return CMD_RET_FAILURE;
	}

	printf("SCP03 is provisioned\n");

	return CMD_RET_SUCCESS;
}

U_BOOT_LONGHELP(scp03,
	"provides a command to enable SCP03 and provision the SCP03 keys\n"
	" enable    - enable SCP03 on the TEE\n"
	" provision - provision SCP03 on the TEE\n");

U_BOOT_CMD_WITH_SUBCMDS(scp03, "Secure Channel Protocol 03 control",
	scp03_help_text,
	U_BOOT_SUBCMD_MKENT(enable, 1, 1, do_scp03_enable),
	U_BOOT_SUBCMD_MKENT(provision, 1, 1, do_scp03_provision));
