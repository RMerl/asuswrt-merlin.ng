// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2011
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * IDE support
 */

#include <common.h>
#include <blk.h>
#include <config.h>
#include <watchdog.h>
#include <command.h>
#include <image.h>
#include <asm/byteorder.h>
#include <asm/io.h>

#if defined(CONFIG_IDE_PCMCIA)
# include <pcmcia.h>
#endif

#include <ide.h>
#include <ata.h>

#ifdef CONFIG_LED_STATUS
# include <status_led.h>
#endif

/* Current I/O Device	*/
static int curr_device;

int do_ide(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc == 2) {
		if (strncmp(argv[1], "res", 3) == 0) {
			puts("\nReset IDE: ");
			ide_init();
			return 0;
		}
	}

	return blk_common_cmd(argc, argv, IF_TYPE_IDE, &curr_device);
}

int do_diskboot(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	return common_diskboot(cmdtp, "ide", argc, argv);
}

U_BOOT_CMD(ide, 5, 1, do_ide,
	   "IDE sub-system",
	   "reset - reset IDE controller\n"
	   "ide info  - show available IDE devices\n"
	   "ide device [dev] - show or set current device\n"
	   "ide part [dev] - print partition table of one or all IDE devices\n"
	   "ide read  addr blk# cnt\n"
	   "ide write addr blk# cnt - read/write `cnt'"
	   " blocks starting at block `blk#'\n"
	   "    to/from memory address `addr'");

U_BOOT_CMD(diskboot, 3, 1, do_diskboot,
	   "boot from IDE device", "loadAddr dev:part");
