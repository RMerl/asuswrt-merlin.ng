// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007 by OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 */

#include <common.h>
#include <command.h>
#include <gzip.h>
#include <malloc.h>

#include "license_data_gz.h"
#include "license_data_size.h"

static int do_license(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	char *dst;
	unsigned long len = data_size;
	int ret = CMD_RET_SUCCESS;

	dst = malloc(data_size + 1);
	if (!dst)
		return CMD_RET_FAILURE;

	ret = gunzip(dst, data_size, (unsigned char *)data_gz, &len);
	if (ret) {
		printf("Error uncompressing license text\n");
		ret = CMD_RET_FAILURE;
		goto free;
	}

	dst[data_size] = 0;
	puts(dst);

free:
	free(dst);

	return ret;
}

U_BOOT_CMD(
	license, 1, 1, do_license,
	"print GPL license text",
	""
);
