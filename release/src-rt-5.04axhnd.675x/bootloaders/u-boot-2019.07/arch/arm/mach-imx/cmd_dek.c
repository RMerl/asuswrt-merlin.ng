// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008-2015 Freescale Semiconductor, Inc.
 *
 * Command for encapsulating DEK blob
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <linux/compiler.h>
#include <fsl_sec.h>
#include <asm/arch/clock.h>
#include <mapmem.h>

/**
* blob_dek() - Encapsulate the DEK as a blob using CAM's Key
* @src: - Address of data to be encapsulated
* @dst: - Desination address of encapsulated data
* @len: - Size of data to be encapsulated
*
* Returns zero on success,and negative on error.
*/
static int blob_encap_dek(const u8 *src, u8 *dst, u32 len)
{
	int ret = 0;
	u32 jr_size = 4;

	u32 out_jr_size = sec_in32(CONFIG_SYS_FSL_JR0_ADDR + 0x102c);
	if (out_jr_size != jr_size) {
		hab_caam_clock_enable(1);
		sec_init();
	}

	if (!((len == 128) | (len == 192) | (len == 256))) {
		debug("Invalid DEK size. Valid sizes are 128, 192 and 256b\n");
		return -1;
	}

	len /= 8;
	ret = blob_dek(src, dst, len);

	return ret;
}

/**
 * do_dek_blob() - Handle the "dek_blob" command-line command
 * @cmdtp:  Command data struct pointer
 * @flag:   Command flag
 * @argc:   Command-line argument count
 * @argv:   Array of command-line arguments
 *
 * Returns zero on success, CMD_RET_USAGE in case of misuse and negative
 * on error.
 */
static int do_dek_blob(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	uint32_t src_addr, dst_addr, len;
	uint8_t *src_ptr, *dst_ptr;
	int ret = 0;

	if (argc != 4)
		return CMD_RET_USAGE;

	src_addr = simple_strtoul(argv[1], NULL, 16);
	dst_addr = simple_strtoul(argv[2], NULL, 16);
	len = simple_strtoul(argv[3], NULL, 10);

	src_ptr = map_sysmem(src_addr, len/8);
	dst_ptr = map_sysmem(dst_addr, BLOB_SIZE(len/8));

	ret = blob_encap_dek(src_ptr, dst_ptr, len);

	return ret;
}

/***************************************************/
static char dek_blob_help_text[] =
	"src dst len            - Encapsulate and create blob of data\n"
	"                         $len bits long at address $src and\n"
	"                         store the result at address $dst.\n";

U_BOOT_CMD(
	dek_blob, 4, 1, do_dek_blob,
	"Data Encryption Key blob encapsulation",
	dek_blob_help_text
);
