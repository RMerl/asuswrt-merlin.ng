// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <command.h>
#include <malloc.h>

#include "config_data_gz.h"
#include "config_data_size.h"

static int do_config(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *dst;
	unsigned long len = data_size;
	int ret = CMD_RET_SUCCESS;

	dst = malloc(data_size + 1);
	if (!dst)
		return CMD_RET_FAILURE;

	ret = gunzip(dst, data_size, (unsigned char *)data_gz, &len);
	if (ret) {
		printf("failed to uncompress .config data\n");
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
	config, 1, 1, do_config,
	"print .config",
	""
);
