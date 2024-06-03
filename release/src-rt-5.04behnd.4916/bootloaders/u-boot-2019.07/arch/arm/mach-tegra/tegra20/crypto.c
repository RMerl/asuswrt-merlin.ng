// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2010 - 2011 NVIDIA Corporation <www.nvidia.com>
 */

#include <common.h>
#include <linux/errno.h>
#include "crypto.h"
#include "uboot_aes.h"

static u8 zero_key[16];

#define AES_CMAC_CONST_RB 0x87  /* from RFC 4493, Figure 2.2 */

enum security_op {
	SECURITY_SIGN		= 1 << 0,	/* Sign the data */
	SECURITY_ENCRYPT	= 1 << 1,	/* Encrypt the data */
};

/**
 * Shift a vector left by one bit
 *
 * \param in	Input vector
 * \param out	Output vector
 * \param size	Length of vector in bytes
 */
static void left_shift_vector(u8 *in, u8 *out, int size)
{
	int carry = 0;
	int i;

	for (i = size - 1; i >= 0; i--) {
		out[i] = (in[i] << 1) | carry;
		carry = in[i] >> 7;	/* get most significant bit */
	}
}

/**
 * Sign a block of data, putting the result into dst.
 *
 * \param key			Input AES key, length AES_KEY_LENGTH
 * \param key_schedule		Expanded key to use
 * \param src			Source data of length 'num_aes_blocks' blocks
 * \param dst			Destination buffer, length AES_KEY_LENGTH
 * \param num_aes_blocks	Number of AES blocks to encrypt
 */
static void sign_object(u8 *key, u8 *key_schedule, u8 *src, u8 *dst,
			u32 num_aes_blocks)
{
	u8 tmp_data[AES_KEY_LENGTH];
	u8 iv[AES_KEY_LENGTH] = {0};
	u8 left[AES_KEY_LENGTH];
	u8 k1[AES_KEY_LENGTH];
	u8 *cbc_chain_data;
	unsigned i;

	cbc_chain_data = zero_key;	/* Convenient array of 0's for IV */

	/* compute K1 constant needed by AES-CMAC calculation */
	for (i = 0; i < AES_KEY_LENGTH; i++)
		tmp_data[i] = 0;

	aes_cbc_encrypt_blocks(key_schedule, iv, tmp_data, left, 1);

	left_shift_vector(left, k1, sizeof(left));

	if ((left[0] >> 7) != 0) /* get MSB of L */
		k1[AES_KEY_LENGTH-1] ^= AES_CMAC_CONST_RB;

	/* compute the AES-CMAC value */
	for (i = 0; i < num_aes_blocks; i++) {
		/* Apply the chain data */
		aes_apply_cbc_chain_data(cbc_chain_data, src, tmp_data);

		/* for the final block, XOR K1 into the IV */
		if (i == num_aes_blocks - 1)
			aes_apply_cbc_chain_data(tmp_data, k1, tmp_data);

		/* encrypt the AES block */
		aes_encrypt(tmp_data, key_schedule, dst);

		debug("sign_obj: block %d of %d\n", i, num_aes_blocks);

		/* Update pointers for next loop. */
		cbc_chain_data = dst;
		src += AES_KEY_LENGTH;
	}
}

/**
 * Encrypt and sign a block of data (depending on security mode).
 *
 * \param key		Input AES key, length AES_KEY_LENGTH
 * \param oper		Security operations mask to perform (enum security_op)
 * \param src		Source data
 * \param length	Size of source data
 * \param sig_dst	Destination address for signature, AES_KEY_LENGTH bytes
 */
static int encrypt_and_sign(u8 *key, enum security_op oper, u8 *src,
			    u32 length, u8 *sig_dst)
{
	u32 num_aes_blocks;
	u8 key_schedule[AES_EXPAND_KEY_LENGTH];
	u8 iv[AES_KEY_LENGTH] = {0};

	debug("encrypt_and_sign: length = %d\n", length);

	/*
	 * The only need for a key is for signing/checksum purposes, so
	 * if not encrypting, expand a key of 0s.
	 */
	aes_expand_key(oper & SECURITY_ENCRYPT ? key : zero_key, key_schedule);

	num_aes_blocks = (length + AES_KEY_LENGTH - 1) / AES_KEY_LENGTH;

	if (oper & SECURITY_ENCRYPT) {
		/* Perform this in place, resulting in src being encrypted. */
		debug("encrypt_and_sign: begin encryption\n");
		aes_cbc_encrypt_blocks(key_schedule, iv, src, src,
				       num_aes_blocks);
		debug("encrypt_and_sign: end encryption\n");
	}

	if (oper & SECURITY_SIGN) {
		/* encrypt the data, overwriting the result in signature. */
		debug("encrypt_and_sign: begin signing\n");
		sign_object(key, key_schedule, src, sig_dst, num_aes_blocks);
		debug("encrypt_and_sign: end signing\n");
	}

	return 0;
}

int sign_data_block(u8 *source, unsigned length, u8 *signature)
{
	return encrypt_and_sign(zero_key, SECURITY_SIGN, source,
				length, signature);
}
