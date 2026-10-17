// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <command.h>
#include <env.h>
#include <tpm-v1.h>
#include <malloc.h>
#include <linux/ctype.h>
#include <asm/unaligned.h>

#include "hre.h"

int flush_keys(struct udevice *tpm)
{
	u16 key_count;
	u8 buf[288];
	u8 *ptr;
	u32 err;
	uint i;

	/* fetch list of already loaded keys in the TPM */
	err = tpm1_get_capability(tpm, TPM_CAP_HANDLE, TPM_RT_KEY, buf,
				  sizeof(buf));
	if (err)
		return -1;
	key_count = get_unaligned_be16(buf);
	ptr = buf + 2;
	for (i = 0; i < key_count; ++i, ptr += 4) {
		err = tpm1_flush_specific(tpm, get_unaligned_be32(ptr),
					  TPM_RT_KEY);
		if (err && err != TPM_KEY_OWNER_CONTROL)
			return err;
	}

	return 0;
}

int decode_hexstr(char *hexstr, u8 **result)
{
	int len = strlen(hexstr);
	int bytes = len / 2;
	int i;
	u8 acc = 0;

	if (len % 2 == 1)
		return 1;

	*result = (u8 *)malloc(bytes);

	for (i = 0; i < len; i++) {
		char cur = tolower(hexstr[i]);
		u8 val;

		if ((cur >= 'a' && cur <= 'f') || (cur >= '0' && cur <= '9')) {
			val = cur - (cur > '9' ? 87 : 48);

			if (i % 2 == 0)
				acc = 16 * val;
			else
				(*result)[i / 2] = acc + val;
		} else {
			free(*result);
			return 1;
		}
	}

	return 0;
}

int extract_subprogram(u8 **progdata, u32 expected_magic,
		       struct key_program **result)
{
	struct key_program *prog = *result;
	u32 magic, code_crc, code_size;

	magic = get_unaligned_be32(*progdata);
	code_crc = get_unaligned_be32(*progdata + 4);
	code_size = get_unaligned_be32(*progdata + 8);

	*progdata += 12;

	if (magic != expected_magic)
		return -1;

	*result = malloc(sizeof(struct key_program) + code_size);

	if (!*result)
		return -1;

	prog->magic = magic;
	prog->code_crc = code_crc;
	prog->code_size = code_size;
	memcpy(prog->code, *progdata, code_size);

	*progdata += code_size;

	if (hre_verify_program(prog)) {
		free(prog);
		return -1;
	}

	return 0;
}

struct key_program *parse_and_check_keyprog(u8 *progdata)
{
	struct key_program *result = NULL, *hmac = NULL;

	/* Part 1: Load key program */

	if (extract_subprogram(&progdata, MAGIC_KEY_PROGRAM, &result))
		return NULL;

	/* Part 2: Load hmac program */

	if (extract_subprogram(&progdata, MAGIC_HMAC, &hmac))
		return NULL;

	free(hmac);

	return result;
}

int load_and_run_keyprog(struct udevice *tpm)
{
	char *cmd = NULL;
	u8 *binprog = NULL;
	char *hexprog;
	struct key_program *prog;

	cmd = env_get("loadkeyprogram");

	if (!cmd || run_command(cmd, 0))
		return 1;

	hexprog = env_get("keyprogram");

	if (decode_hexstr(hexprog, &binprog))
		return 1;

	prog = parse_and_check_keyprog(binprog);
	free(binprog);

	if (!prog)
		return 1;

	if (hre_run_program(tpm, prog->code, prog->code_size)) {
		free(prog);
		return 1;
	}

	printf("\nSD code ran successfully\n");

	free(prog);

	return 0;
}
