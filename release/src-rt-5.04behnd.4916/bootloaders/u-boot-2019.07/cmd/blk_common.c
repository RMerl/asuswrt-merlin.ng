// SPDX-License-Identifier: GPL-2.0+
/*
 * Handling of common block commands
 *
 * Copyright (c) 2017 Google, Inc
 *
 * (C) Copyright 2000-2011
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <blk.h>

#ifdef CONFIG_HAVE_BLOCK_DEVICE
int blk_common_cmd(int argc, char * const argv[], enum if_type if_type,
		   int *cur_devnump)
{
	const char *if_name = blk_get_if_type_name(if_type);

	switch (argc) {
	case 0:
	case 1:
		return CMD_RET_USAGE;
	case 2:
		if (strncmp(argv[1], "inf", 3) == 0) {
			blk_list_devices(if_type);
			return 0;
		} else if (strncmp(argv[1], "dev", 3) == 0) {
			if (blk_print_device_num(if_type, *cur_devnump)) {
				printf("\nno %s devices available\n", if_name);
				return CMD_RET_FAILURE;
			}
			return 0;
		} else if (strncmp(argv[1], "part", 4) == 0) {
			if (blk_list_part(if_type))
				printf("\nno %s devices available\n", if_name);
			return 0;
		}
		return CMD_RET_USAGE;
	case 3:
		if (strncmp(argv[1], "dev", 3) == 0) {
			int dev = (int)simple_strtoul(argv[2], NULL, 10);

			if (!blk_show_device(if_type, dev)) {
				*cur_devnump = dev;
				printf("... is now current device\n");
			} else {
				return CMD_RET_FAILURE;
			}
			return 0;
		} else if (strncmp(argv[1], "part", 4) == 0) {
			int dev = (int)simple_strtoul(argv[2], NULL, 10);

			if (blk_print_part_devnum(if_type, dev)) {
				printf("\n%s device %d not available\n",
				       if_name, dev);
				return CMD_RET_FAILURE;
			}
			return 0;
		}
		return CMD_RET_USAGE;

	default: /* at least 4 args */
		if (strcmp(argv[1], "read") == 0) {
			ulong addr = simple_strtoul(argv[2], NULL, 16);
			lbaint_t blk = simple_strtoul(argv[3], NULL, 16);
			ulong cnt = simple_strtoul(argv[4], NULL, 16);
			ulong n;

			printf("\n%s read: device %d block # "LBAFU", count %lu ... ",
			       if_name, *cur_devnump, blk, cnt);

			n = blk_read_devnum(if_type, *cur_devnump, blk, cnt,
					    (ulong *)addr);

			printf("%ld blocks read: %s\n", n,
			       n == cnt ? "OK" : "ERROR");
			return n == cnt ? 0 : 1;
		} else if (strcmp(argv[1], "write") == 0) {
			ulong addr = simple_strtoul(argv[2], NULL, 16);
			lbaint_t blk = simple_strtoul(argv[3], NULL, 16);
			ulong cnt = simple_strtoul(argv[4], NULL, 16);
			ulong n;

			printf("\n%s write: device %d block # "LBAFU", count %lu ... ",
			       if_name, *cur_devnump, blk, cnt);

			n = blk_write_devnum(if_type, *cur_devnump, blk, cnt,
					     (ulong *)addr);

			printf("%ld blocks written: %s\n", n,
			       n == cnt ? "OK" : "ERROR");
			return n == cnt ? 0 : 1;
		} else {
			return CMD_RET_USAGE;
		}
	}
}
#endif
