// SPDX-License-Identifier: GPL-2.0+
/*
 * 2017 by Marek Behun <marek.behun@nic.cz>
 */

#include <common.h>
#include <command.h>
#include <btrfs.h>
#include <fs.h>

int do_btrsubvol(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc != 3)
		return CMD_RET_USAGE;

	if (fs_set_blk_dev(argv[1], argv[2], FS_TYPE_BTRFS))
		return 1;

	btrfs_list_subvols();
	return 0;
}

U_BOOT_CMD(btrsubvol, 3, 1, do_btrsubvol,
	"list subvolumes of a BTRFS filesystem",
	"<interface> <dev[:part]>\n"
	"     - List subvolumes of a BTRFS filesystem."
)
