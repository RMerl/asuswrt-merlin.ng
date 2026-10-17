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
#include <command.h>
#include <mapmem.h>

int blk_common_cmd(int argc, char *const argv[], enum uclass_id uclass_id,
		   int *cur_devnump)
{
	const char *if_name = blk_get_uclass_name(uclass_id);

	switch (argc) {
	case 0:
	case 1:
		return CMD_RET_USAGE;
	case 2:
		if (strncmp(argv[1], "inf", 3) == 0) {
			blk_list_devices(uclass_id);
			return CMD_RET_SUCCESS;
		} else if (strncmp(argv[1], "dev", 3) == 0) {
			if (blk_print_device_num(uclass_id, *cur_devnump)) {
				printf("\nno %s devices available\n", if_name);
				return CMD_RET_FAILURE;
			}
			return CMD_RET_SUCCESS;
		} else if (strncmp(argv[1], "part", 4) == 0) {
			if (blk_list_part(uclass_id))
				printf("\nno %s partition table available\n",
				       if_name);
			return CMD_RET_SUCCESS;
		}
		return CMD_RET_USAGE;
	case 3:
		if (strncmp(argv[1], "dev", 3) == 0) {
			int dev = (int)dectoul(argv[2], NULL);

			if (!blk_show_device(uclass_id, dev)) {
				*cur_devnump = dev;
				printf("... is now current device\n");
			} else {
				return CMD_RET_FAILURE;
			}
			return CMD_RET_SUCCESS;
		} else if (strncmp(argv[1], "part", 4) == 0) {
			int dev = (int)dectoul(argv[2], NULL);

			if (blk_print_part_devnum(uclass_id, dev)) {
				printf("\n%s device %d not available\n",
				       if_name, dev);
				return CMD_RET_FAILURE;
			}
			return CMD_RET_SUCCESS;
		}
		return CMD_RET_USAGE;

	default: /* at least 4 args */
		if (strcmp(argv[1], "read") == 0) {
			phys_addr_t paddr = hextoul(argv[2], NULL);
			lbaint_t blk = hextoul(argv[3], NULL);
			ulong cnt = hextoul(argv[4], NULL);
			struct blk_desc *desc;
			void *vaddr;
			ulong n;
			int ret;

			printf("\n%s read: device %d block # "LBAFU", count %lu ... ",
			       if_name, *cur_devnump, blk, cnt);

			ret = blk_get_desc(uclass_id, *cur_devnump, &desc);
			if (ret)
				return CMD_RET_FAILURE;
			vaddr = map_sysmem(paddr, desc->blksz * cnt);
			n = blk_dread(desc, blk, cnt, vaddr);
			unmap_sysmem(vaddr);

			printf("%ld blocks read: %s\n", n,
			       n == cnt ? "OK" : "ERROR");
			return n == cnt ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
		} else if (strcmp(argv[1], "write") == 0) {
			phys_addr_t paddr = hextoul(argv[2], NULL);
			lbaint_t blk = hextoul(argv[3], NULL);
			ulong cnt = hextoul(argv[4], NULL);
			struct blk_desc *desc;
			void *vaddr;
			ulong n;
			int ret;

			printf("\n%s write: device %d block # "LBAFU", count %lu ... ",
			       if_name, *cur_devnump, blk, cnt);

			ret = blk_get_desc(uclass_id, *cur_devnump, &desc);
			if (ret)
				return CMD_RET_FAILURE;
			vaddr = map_sysmem(paddr, desc->blksz * cnt);
			n = blk_dwrite(desc, blk, cnt, vaddr);
			unmap_sysmem(vaddr);

			printf("%ld blocks written: %s\n", n,
			       n == cnt ? "OK" : "ERROR");
			return n == cnt ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
		} else {
			return CMD_RET_USAGE;
		}
	}
}
