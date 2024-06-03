// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2020 Broadcom Corporation
 * Joel Peshkin, Broadcom Corporation, joel.peshkin@broadcom.com
 */

#define DEBUG

#include <common.h>
#include <command.h>
#include <environment.h>
#include <hexdump.h>
#include <linux/ctype.h>
#include <linux/stddef.h>
#include <errno.h>
#include <bca_sdk.h>

DECLARE_GLOBAL_DATA_PTR;

/* From Jedec Spec */
#define RES_RPMB_KEY_NOT_PROGRAMMED 	0x07
#define RPMB_DFLT_KEY 			"key_rpmb_auth"
#define RPMB_MMC_PART			3
#define MIN_RPMB_KEY_LEN		32

static char rpmb_usage[] =
"check_set_key <optional key_name>\n"
" - Check if rpmb key has been set, if not set, then set key pointed to by key_name\n"
" - If key_name is specified, a key data must exist in dtb at /trust/key_name\n"
" - If not specified then default keyname of key_rpmb_auth will be used\n"
;

static int do_rpmb_check_set(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	char temp_buf[256];
	int off = 0;
	int len = 0;
	const char *val = NULL;
	unsigned long counter;
	int ret = CMD_RET_SUCCESS;
	struct mmc *mmc = find_mmc_device(0);

	/* Get key node */
	snprintf(temp_buf, 256, "/trust/%s", (argc==2)?argv[1]:RPMB_DFLT_KEY);

	/* Get key value */
	off = fdt_path_offset(gd->fdt_blob, temp_buf);
	if (off < 0) {
		printf("ERROR: Can't find %s node in boot DTB!\n", temp_buf);
		return CMD_RET_FAILURE;
	} else {
		printf("SUCCESS:Found %s node in boot DTB!\n", temp_buf);
		val = (char*)(fdt_getprop(gd->fdt_blob, off, "value", &len));
		if( !val || len < MIN_RPMB_KEY_LEN ) {
			printf("ERROR: Can't find valid %d byte value for %s in boot DTB!\n", MIN_RPMB_KEY_LEN, temp_buf);
			return CMD_RET_FAILURE;
		}
		
	}

	/* Switch to rpmb partition */
	sprintf(temp_buf, "mmc dev 0 %d", RPMB_MMC_PART);
	if ( run_command(temp_buf, 0) ) {
		printf("Error: Cannot switch to RPMB partition!\n");
		return CMD_RET_FAILURE;
	}

	/* Check if rpmb key has been programmed already */
	if (mmc_rpmb_get_counter(mmc, &counter) == RES_RPMB_KEY_NOT_PROGRAMMED) {
		/* Program rpmb key */
		printf("Programming rpmb key\n");
		if (mmc_rpmb_set_key(mmc, (void*)val)) {
			printf("ERROR - Key already programmed ?\n");
			ret = CMD_RET_FAILURE;
		}
	} else {
		printf("rpmb key already programmed!\n");
	}

	/* Switch back to userdata partition */
	run_command("mmc dev 0 0", 0); 

	return ret;

}
U_BOOT_CMD_WITH_SUBCMDS(bca_rpmb, "Broadcom rpmb commands", rpmb_usage,
	U_BOOT_SUBCMD_MKENT(check_set_key, 2, 0, do_rpmb_check_set));
