// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003 - 2004
 * Sysgo Real-Time Solutions, AG <www.elinos.com>
 * Pavel Bartusek <pba@sysgo.com>
 */

/*
 * Reiserfs support
 */
#include <common.h>
#include <config.h>
#include <command.h>
#include <env.h>
#include <image.h>
#include <linux/ctype.h>
#include <asm/byteorder.h>
#include <reiserfs.h>
#include <part.h>

#if !CONFIG_IS_ENABLED(DOS_PARTITION)
#error DOS partition support must be selected
#endif

/* #define	REISER_DEBUG */

#ifdef	REISER_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

int do_reiserls(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	char *filename = "/";
	int dev, part;
	struct blk_desc *dev_desc = NULL;
	struct disk_partition info;

	if (argc < 3)
		return CMD_RET_USAGE;

	part = blk_get_device_part_str(argv[1], argv[2], &dev_desc, &info, 1);
	if (part < 0)
		return 1;

	if (argc == 4) {
	    filename = argv[3];
	}

	dev = dev_desc->devnum;
	PRINTF("Using device %s %d:%d, directory: %s\n", argv[1], dev, part, filename);

	reiserfs_set_blk_dev(dev_desc, &info);

	if (!reiserfs_mount(info.size)) {
		printf ("** Bad Reiserfs partition or disk - %s %d:%d **\n",  argv[1], dev, part);
		return 1;
	}

	if (reiserfs_ls (filename)) {
		printf ("** Error reiserfs_ls() **\n");
		return 1;
	};

	return 0;
}

U_BOOT_CMD(
	reiserls,	4,	1,	do_reiserls,
	"list files in a directory (default /)",
	"<interface> <dev[:part]> [directory]\n"
	"    - list files from 'dev' on 'interface' in a 'directory'"
);

/******************************************************************************
 * Reiserfs boot command intepreter. Derived from diskboot
 */
int do_reiserload(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	char *filename = NULL;
	int dev, part;
	ulong addr = 0, filelen;
	struct disk_partition info;
	struct blk_desc *dev_desc = NULL;
	unsigned long count;
	char *addr_str;

	switch (argc) {
	case 3:
		addr_str = env_get("loadaddr");
		if (addr_str != NULL) {
			addr = hextoul(addr_str, NULL);
		} else {
			addr = CONFIG_SYS_LOAD_ADDR;
		}
		filename = env_get("bootfile");
		count = 0;
		break;
	case 4:
		addr = hextoul(argv[3], NULL);
		filename = env_get("bootfile");
		count = 0;
		break;
	case 5:
		addr = hextoul(argv[3], NULL);
		filename = argv[4];
		count = 0;
		break;
	case 6:
		addr = hextoul(argv[3], NULL);
		filename = argv[4];
		count = hextoul(argv[5], NULL);
		break;

	default:
		return CMD_RET_USAGE;
	}

	if (!filename) {
		puts ("\n** No boot file defined **\n");
		return 1;
	}

	part = blk_get_device_part_str(argv[1], argv[2], &dev_desc, &info, 1);
	if (part < 0)
		return 1;

	dev = dev_desc->devnum;

	printf("Loading file \"%s\" from %s device %d%c%c\n",
		filename, argv[1], dev,
		part ? ':' : ' ', part ? part + '0' : ' ');

	reiserfs_set_blk_dev(dev_desc, &info);

	if (!reiserfs_mount(info.size)) {
		printf ("** Bad Reiserfs partition or disk - %s %d:%d **\n",  argv[1], dev, part);
		return 1;
	}

	filelen = reiserfs_open(filename);
	if (filelen < 0) {
		printf("** File not found %s **\n", filename);
		return 1;
	}
	if ((count < filelen) && (count != 0)) {
	    filelen = count;
	}

	if (reiserfs_read((char *)addr, filelen) != filelen) {
		printf("\n** Unable to read \"%s\" from %s %d:%d **\n", filename, argv[1], dev, part);
		return 1;
	}

	/* Loading ok, update default load address */
	image_load_addr = addr;

	printf ("\n%ld bytes read\n", filelen);
	env_set_hex("filesize", filelen);

	return filelen;
}

U_BOOT_CMD(
	reiserload,	6,	0,	do_reiserload,
	"load binary file from a Reiser filesystem",
	"<interface> <dev[:part]> [addr] [filename] [bytes]\n"
	"    - load binary file 'filename' from 'dev' on 'interface'\n"
	"      to address 'addr' from dos filesystem"
);
