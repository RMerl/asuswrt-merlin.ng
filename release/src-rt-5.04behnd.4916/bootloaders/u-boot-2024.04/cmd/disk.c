// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2011
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */
#include <common.h>
#include <bootstage.h>
#include <command.h>
#include <cpu_func.h>
#include <image.h>
#include <log.h>
#include <part.h>

int common_diskboot(struct cmd_tbl *cmdtp, const char *intf, int argc,
		    char *const argv[])
{
	__maybe_unused int dev;
	int part;
	ulong addr = CONFIG_SYS_LOAD_ADDR;
	ulong cnt;
	struct disk_partition info;
#if defined(CONFIG_LEGACY_IMAGE_FORMAT)
	struct legacy_img_hdr *hdr;
#endif
	struct blk_desc *dev_desc;

#if CONFIG_IS_ENABLED(FIT)
	const void *fit_hdr = NULL;
#endif

	bootstage_mark(BOOTSTAGE_ID_IDE_START);
	if (argc > 3) {
		bootstage_error(BOOTSTAGE_ID_IDE_ADDR);
		return CMD_RET_USAGE;
	}
	bootstage_mark(BOOTSTAGE_ID_IDE_ADDR);

	if (argc > 1)
		addr = hextoul(argv[1], NULL);

	bootstage_mark(BOOTSTAGE_ID_IDE_BOOT_DEVICE);

	part = blk_get_device_part_str(intf, cmd_arg2(argc, argv),
				       &dev_desc, &info, 1);
	if (part < 0) {
		bootstage_error(BOOTSTAGE_ID_IDE_TYPE);
		return 1;
	}

	dev = dev_desc->devnum;
	bootstage_mark(BOOTSTAGE_ID_IDE_TYPE);

	printf("\nLoading from %s device %d, partition %d: "
	       "Name: %.32s  Type: %.32s\n", intf, dev, part, info.name,
	       info.type);

	debug("First Block: " LBAFU ",  # of blocks: " LBAFU
	      ", Block Size: %ld\n",
	      info.start, info.size, info.blksz);

	if (blk_dread(dev_desc, info.start, 1, (ulong *)addr) != 1) {
		printf("** Read error on %d:%d\n", dev, part);
		bootstage_error(BOOTSTAGE_ID_IDE_PART_READ);
		return 1;
	}
	bootstage_mark(BOOTSTAGE_ID_IDE_PART_READ);

	switch (genimg_get_format((void *) addr)) {
#if defined(CONFIG_LEGACY_IMAGE_FORMAT)
	case IMAGE_FORMAT_LEGACY:
		hdr = (struct legacy_img_hdr *)addr;

		bootstage_mark(BOOTSTAGE_ID_IDE_FORMAT);

		if (!image_check_hcrc(hdr)) {
			puts("\n** Bad Header Checksum **\n");
			bootstage_error(BOOTSTAGE_ID_IDE_CHECKSUM);
			return 1;
		}
		bootstage_mark(BOOTSTAGE_ID_IDE_CHECKSUM);

		image_print_contents(hdr);

		cnt = image_get_image_size(hdr);
		break;
#endif
#if CONFIG_IS_ENABLED(FIT)
	case IMAGE_FORMAT_FIT:
		fit_hdr = (const void *) addr;
		puts("Fit image detected...\n");

		cnt = fit_get_size(fit_hdr);
		break;
#endif
	default:
		bootstage_error(BOOTSTAGE_ID_IDE_FORMAT);
		puts("** Unknown image type\n");
		return 1;
	}

	cnt += info.blksz - 1;
	cnt /= info.blksz;
	cnt -= 1;

	if (blk_dread(dev_desc, info.start + 1, cnt,
		      (ulong *)(addr + info.blksz)) != cnt) {
		printf("** Read error on %d:%d\n", dev, part);
		bootstage_error(BOOTSTAGE_ID_IDE_READ);
		return 1;
	}
	bootstage_mark(BOOTSTAGE_ID_IDE_READ);

#if CONFIG_IS_ENABLED(FIT)
	/* This cannot be done earlier,
	 * we need complete FIT image in RAM first */
	if (genimg_get_format((void *) addr) == IMAGE_FORMAT_FIT) {
		if (fit_check_format(fit_hdr, IMAGE_SIZE_INVAL)) {
			bootstage_error(BOOTSTAGE_ID_IDE_FIT_READ);
			puts("** Bad FIT image format\n");
			return 1;
		}
		bootstage_mark(BOOTSTAGE_ID_IDE_FIT_READ_OK);
	}
#endif

	flush_cache(addr, (cnt+1)*info.blksz);

	/* Loading ok, update default load address */
	image_load_addr = addr;

	return bootm_maybe_autostart(cmdtp, argv[0]);
}
