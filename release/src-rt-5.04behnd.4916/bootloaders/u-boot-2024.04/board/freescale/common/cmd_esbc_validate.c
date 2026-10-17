// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <command.h>
#include <env.h>
#include <fsl_validate.h>

int do_esbc_halt(struct cmd_tbl *cmdtp, int flag, int argc,
		 char *const argv[])
{
	if (fsl_check_boot_mode_secure() == 0) {
		printf("Boot Mode is Non-Secure. Not entering spin loop.\n");
		return 0;
	}

	printf("Core is entering spin loop.\n");
loop:
	goto loop;

	return 0;
}

#ifndef CONFIG_SPL_BUILD
static int do_esbc_validate(struct cmd_tbl *cmdtp, int flag, int argc,
			    char *const argv[])
{
	char *hash_str = NULL;
	uintptr_t haddr;
	int ret;
	uintptr_t img_addr = 0;
	char buf[20];

	if (argc < 2)
		return cmd_usage(cmdtp);
	else if (argc > 2)
		/* Second arg - Optional - Hash Str*/
		hash_str = argv[2];

	/* First argument - header address -32/64bit */
	haddr = (uintptr_t)hextoul(argv[1], NULL);

	/* With esbc_validate command, Image address must be
	 * part of header. So, the function is called
	 * by passing this argument as 0.
	 */
	ret = fsl_secboot_validate(haddr, hash_str, &img_addr);

	/* Need to set "img_addr" even if validation failure.
	 * Required when SB_EN in RCW set and non-fatal error
	 * to continue U-Boot
	 */
	sprintf(buf, "%lx", img_addr);
	env_set("img_addr", buf);

	if (ret)
		return 1;

	printf("esbc_validate command successful\n");
	return 0;
}

/***************************************************/
static char esbc_validate_help_text[] =
	"esbc_validate hdr_addr <hash_val> - Validates signature using\n"
	"                          RSA verification\n"
	"                          $hdr_addr Address of header of the image\n"
	"                          to be validated.\n"
	"                          $hash_val -Optional\n"
	"                          It provides Hash of public/srk key to be\n"
	"                          used to verify signature.\n";

U_BOOT_CMD(
	esbc_validate,	3,	0,	do_esbc_validate,
	"Validates signature on a given image using RSA verification",
	esbc_validate_help_text
);

U_BOOT_CMD(
	esbc_halt,	1,	0,	do_esbc_halt,
	"Put the core in spin loop (Secure Boot Only)",
	""
);
#endif
