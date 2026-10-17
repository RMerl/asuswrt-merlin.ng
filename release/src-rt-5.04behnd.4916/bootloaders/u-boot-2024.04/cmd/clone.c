// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 John Chau <john@harmon.hk>
 *
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <part.h>
#include <blk.h>
#include <vsprintf.h>

#define BUFSIZE (1 * 1024 * 1024)
static int do_clone(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	int srcdev, destdev;
	struct blk_desc *srcdesc, *destdesc;
	int srcbz, destbz, ret;
	char *unit, *buf;
	unsigned long wrcnt, rdcnt, requested, srcblk, destblk;
	unsigned long timer;
	const unsigned long buffersize = 1024 * 1024;

	if (argc < 6)
		return CMD_RET_USAGE;

	srcdev = blk_get_device_by_str(argv[1], argv[2], &srcdesc);
	destdev = blk_get_device_by_str(argv[3], argv[4], &destdesc);
	if (srcdev < 0) {
		printf("Unable to open source device\n");
		return 1;
	} else if (destdev < 0) {
		printf("Unable to open destination device\n");
		return 1;
	}
	requested = dectoul(argv[5], &unit);
	srcbz = srcdesc->blksz;
	destbz = destdesc->blksz;

	if ((srcbz * (buffersize / srcbz) != buffersize) ||
	    (destbz * (buffersize / destbz) != buffersize)) {
		printf("failed: cannot match device block sizes\n");
		return 1;
	}
	if (requested == 0) {
		unsigned long a = srcdesc->lba * srcdesc->blksz;
		unsigned long b = destdesc->lba * destdesc->blksz;

		if (a > b)
			requested = a;
		else
			requested = b;
	} else {
		switch (unit[0]) {
		case 'g':
		case 'G':
			requested *= 1024 * 1024 * 1024;
			break;
		case 'm':
		case 'M':
			requested *= 1024 * 1024;
			break;
		case 'k':
		case 'K':
			requested *= 1024;
			break;
		}
	}
	printf("Copying %ld bytes from %s:%s to %s:%s\n",
	       requested, argv[1], argv[2], argv[3], argv[4]);
	wrcnt = 0;
	rdcnt = 0;
	buf = (char *)malloc(BUFSIZE);
	srcblk = 0;
	destblk = 0;
	timer = get_timer(0);
	while (wrcnt < requested) {
		unsigned long toread = BUFSIZE / srcbz;
		unsigned long towrite = BUFSIZE / destbz;
		unsigned long offset = 0;

read:
		ret = blk_dread(srcdesc, srcblk, toread, buf + offset);
		if (ret < 0) {
			printf("Src read error @blk %ld\n", srcblk);
			goto exit;
		}
		rdcnt += ret * srcbz;
		srcblk += ret;
		if (ret < toread) {
			toread -= ret;
			offset += ret * srcbz;
			goto read;
		}
		offset = 0;
write:
		ret = blk_dwrite(destdesc, destblk, towrite, buf + offset);
		if (ret < 0) {
			printf("Dest write error @blk %ld\n", srcblk);
			goto exit;
		}
		wrcnt += ret * destbz;
		destblk += ret;
		if (ret < towrite) {
			towrite -= ret;
			offset += ret * destbz;
			goto write;
		}
	}

exit:
	timer = get_timer(timer);
	timer = 1000 * timer / CONFIG_SYS_HZ;
	printf("%ld read\n", rdcnt);
	printf("%ld written\n", wrcnt);
	printf("%ldms, %ldkB/s\n", timer, wrcnt / timer);
	free(buf);

	return 0;
}

U_BOOT_CMD(
	clone, 6, 1, do_clone,
	"simple storage cloning",
	"<src interface> <src dev> <dest interface> <dest dev> <size[K/M/G]>\n"
	"clone storage from 'src dev' on 'src interface' to 'dest dev' on 'dest interface' with maximum 'size' bytes (or 0 for clone to end)"
);
