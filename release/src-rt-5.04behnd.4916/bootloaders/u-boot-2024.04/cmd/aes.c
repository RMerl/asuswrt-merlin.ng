// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Marek Vasut <marex@denx.de>
 *
 * Command for en/de-crypting block of memory with AES-[128/192/256]-CBC cipher.
 */

#include <common.h>
#include <command.h>
#include <uboot_aes.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <linux/compiler.h>
#include <mapmem.h>

u32 aes_get_key_len(char *command)
{
	u32 key_len = AES128_KEY_LENGTH;

	if (!strcmp(command, "aes.192"))
		key_len = AES192_KEY_LENGTH;
	else if (!strcmp(command, "aes.256"))
		key_len = AES256_KEY_LENGTH;

	return key_len;
}

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
static int do_aes(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	uint32_t key_addr, iv_addr, src_addr, dst_addr, len;
	uint8_t *key_ptr, *iv_ptr, *src_ptr, *dst_ptr;
	u8 key_exp[AES256_EXPAND_KEY_LENGTH];
	u32 aes_blocks, key_len;
	int enc;

	if (argc != 7)
		return CMD_RET_USAGE;

	key_len = aes_get_key_len(argv[0]);

	if (!strncmp(argv[1], "enc", 3))
		enc = 1;
	else if (!strncmp(argv[1], "dec", 3))
		enc = 0;
	else
		return CMD_RET_USAGE;

	key_addr = hextoul(argv[2], NULL);
	iv_addr = hextoul(argv[3], NULL);
	src_addr = hextoul(argv[4], NULL);
	dst_addr = hextoul(argv[5], NULL);
	len = hextoul(argv[6], NULL);

	key_ptr = (uint8_t *)map_sysmem(key_addr, key_len);
	iv_ptr = (uint8_t *)map_sysmem(iv_addr, 128 / 8);
	src_ptr = (uint8_t *)map_sysmem(src_addr, len);
	dst_ptr = (uint8_t *)map_sysmem(dst_addr, len);

	/* First we expand the key. */
	aes_expand_key(key_ptr, key_len, key_exp);

	/* Calculate the number of AES blocks to encrypt. */
	aes_blocks = DIV_ROUND_UP(len, AES_BLOCK_LENGTH);

	if (enc)
		aes_cbc_encrypt_blocks(key_len, key_exp, iv_ptr, src_ptr,
				       dst_ptr, aes_blocks);
	else
		aes_cbc_decrypt_blocks(key_len, key_exp, iv_ptr, src_ptr,
				       dst_ptr, aes_blocks);

	unmap_sysmem(key_ptr);
	unmap_sysmem(iv_ptr);
	unmap_sysmem(src_ptr);
	unmap_sysmem(dst_ptr);

	return 0;
}

/***************************************************/
U_BOOT_LONGHELP(aes,
	"[.128,.192,.256] enc key iv src dst len - Encrypt block of data $len bytes long\n"
	"                             at address $src using a key at address\n"
	"                             $key with initialization vector at address\n"
	"                             $iv. Store the result at address $dst.\n"
	"                             The $len size must be multiple of 16 bytes.\n"
	"                             The $key and $iv must be 16 bytes long.\n"
	"aes [.128,.192,.256] dec key iv src dst len - Decrypt block of data $len bytes long\n"
	"                             at address $src using a key at address\n"
	"                             $key with initialization vector at address\n"
	"                             $iv. Store the result at address $dst.\n"
	"                             The $len size must be multiple of 16 bytes.\n"
	"                             The $key and $iv must be 16 bytes long.");

U_BOOT_CMD(
	aes, 7, 1, do_aes,
	"AES 128/192/256 CBC encryption",
	aes_help_text
);
