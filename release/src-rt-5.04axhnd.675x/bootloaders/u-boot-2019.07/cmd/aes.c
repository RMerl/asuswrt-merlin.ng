// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Marek Vasut <marex@denx.de>
 *
 * Command for en/de-crypting block of memory with AES-128-CBC cipher.
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <uboot_aes.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <linux/compiler.h>

/**
 * do_aes() - Handle the "aes" command-line command
 * @cmdtp:	Command data struct pointer
 * @flag:	Command flag
 * @argc:	Command-line argument count
 * @argv:	Array of command-line arguments
 *
 * Returns zero on success, CMD_RET_USAGE in case of misuse and negative
 * on error.
 */
static int do_aes(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	uint32_t key_addr, iv_addr, src_addr, dst_addr, len;
	uint8_t *key_ptr, *iv_ptr, *src_ptr, *dst_ptr;
	uint8_t key_exp[AES_EXPAND_KEY_LENGTH];
	uint32_t aes_blocks;
	int enc;

	if (argc != 7)
		return CMD_RET_USAGE;

	if (!strncmp(argv[1], "enc", 3))
		enc = 1;
	else if (!strncmp(argv[1], "dec", 3))
		enc = 0;
	else
		return CMD_RET_USAGE;

	key_addr = simple_strtoul(argv[2], NULL, 16);
	iv_addr = simple_strtoul(argv[3], NULL, 16);
	src_addr = simple_strtoul(argv[4], NULL, 16);
	dst_addr = simple_strtoul(argv[5], NULL, 16);
	len = simple_strtoul(argv[6], NULL, 16);

	key_ptr = (uint8_t *)key_addr;
	iv_ptr = (uint8_t *)iv_addr;
	src_ptr = (uint8_t *)src_addr;
	dst_ptr = (uint8_t *)dst_addr;

	/* First we expand the key. */
	aes_expand_key(key_ptr, key_exp);

	/* Calculate the number of AES blocks to encrypt. */
	aes_blocks = DIV_ROUND_UP(len, AES_KEY_LENGTH);

	if (enc)
		aes_cbc_encrypt_blocks(key_exp, iv_ptr, src_ptr, dst_ptr,
				       aes_blocks);
	else
		aes_cbc_decrypt_blocks(key_exp, iv_ptr, src_ptr, dst_ptr,
				       aes_blocks);

	return 0;
}

/***************************************************/
#ifdef CONFIG_SYS_LONGHELP
static char aes_help_text[] =
	"enc key iv src dst len - Encrypt block of data $len bytes long\n"
	"                             at address $src using a key at address\n"
	"                             $key with initialization vector at address\n"
	"                             $iv. Store the result at address $dst.\n"
	"                             The $len size must be multiple of 16 bytes.\n"
	"                             The $key and $iv must be 16 bytes long.\n"
	"aes dec key iv src dst len - Decrypt block of data $len bytes long\n"
	"                             at address $src using a key at address\n"
	"                             $key with initialization vector at address\n"
	"                             $iv. Store the result at address $dst.\n"
	"                             The $len size must be multiple of 16 bytes.\n"
	"                             The $key and $iv must be 16 bytes long.";
#endif

U_BOOT_CMD(
	aes, 7, 1, do_aes,
	"AES 128 CBC encryption",
	aes_help_text
);
