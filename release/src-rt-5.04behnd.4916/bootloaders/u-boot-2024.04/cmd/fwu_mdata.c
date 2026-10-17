/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2022, Linaro Limited
 */

#include <command.h>
#include <dm.h>
#include <fwu.h>
#include <fwu_mdata.h>
#include <log.h>
#include <stdio.h>
#include <stdlib.h>

#include <linux/types.h>

static void print_mdata(struct fwu_mdata *mdata)
{
	int i, j;
	struct fwu_image_entry *img_entry;
	struct fwu_image_bank_info *img_info;

	printf("\tFWU Metadata\n");
	printf("crc32: %#x\n", mdata->crc32);
	printf("version: %#x\n", mdata->version);
	printf("active_index: %#x\n", mdata->active_index);
	printf("previous_active_index: %#x\n", mdata->previous_active_index);

	printf("\tImage Info\n");
	for (i = 0; i < CONFIG_FWU_NUM_IMAGES_PER_BANK; i++) {
		img_entry = &mdata->img_entry[i];
		printf("\nImage Type Guid: %pUL\n",
		       &img_entry->image_type_uuid);
		printf("Location Guid: %pUL\n", &img_entry->location_uuid);
		for (j = 0; j < CONFIG_FWU_NUM_BANKS; j++) {
			img_info = &img_entry->img_bank_info[j];
			printf("Image Guid:  %pUL\n", &img_info->image_uuid);
			printf("Image Acceptance: %s\n",
			       img_info->accepted == 0x1 ? "yes" : "no");
		}
	}
}

int do_fwu_mdata_read(struct cmd_tbl *cmdtp, int flag,
		     int argc, char * const argv[])
{
	int ret = CMD_RET_SUCCESS, res;
	struct fwu_mdata mdata;

	res = fwu_get_mdata(&mdata);
	if (res < 0) {
		log_err("Unable to get valid FWU metadata\n");
		ret = CMD_RET_FAILURE;
		goto out;
	}

	print_mdata(&mdata);

out:
	return ret;
}

U_BOOT_CMD(
	fwu_mdata_read,	1,	1,	do_fwu_mdata_read,
	"Read and print FWU metadata",
	""
);
