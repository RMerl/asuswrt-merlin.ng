// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 */

/*
 * SCSI support.
 */
#include <common.h>
#include <command.h>
#include <scsi.h>

static int scsi_curr_dev; /* current device */

/*
 * scsi boot command intepreter. Derived from diskboot
 */
static int do_scsiboot(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	return common_diskboot(cmdtp, "scsi", argc, argv);
}

/*
 * scsi command intepreter
 */
static int do_scsi(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;

	if (argc == 2) {
		if (strncmp(argv[1], "res", 3) == 0) {
			printf("\nReset SCSI\n");
#ifndef CONFIG_DM_SCSI
			scsi_bus_reset(NULL);
#endif
			ret = scsi_scan(true);
			if (ret)
				return CMD_RET_FAILURE;
			return ret;
		}
		if (strncmp(argv[1], "scan", 4) == 0) {
			ret = scsi_scan(true);
			if (ret)
				return CMD_RET_FAILURE;
			return ret;
		}
	}

	return blk_common_cmd(argc, argv, IF_TYPE_SCSI, &scsi_curr_dev);
}

U_BOOT_CMD(
	scsi, 5, 1, do_scsi,
	"SCSI sub-system",
	"reset - reset SCSI controller\n"
	"scsi info  - show available SCSI devices\n"
	"scsi scan  - (re-)scan SCSI bus\n"
	"scsi device [dev] - show or set current device\n"
	"scsi part [dev] - print partition table of one or all SCSI devices\n"
	"scsi read addr blk# cnt - read `cnt' blocks starting at block `blk#'\n"
	"     to memory address `addr'\n"
	"scsi write addr blk# cnt - write `cnt' blocks starting at block\n"
	"     `blk#' from memory address `addr'"
);

U_BOOT_CMD(
	scsiboot, 3, 1, do_scsiboot,
	"boot from SCSI device",
	"loadAddr dev:part"
);
