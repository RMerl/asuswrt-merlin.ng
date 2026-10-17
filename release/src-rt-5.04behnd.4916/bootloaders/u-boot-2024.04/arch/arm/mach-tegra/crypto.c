// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2010 - 2011 NVIDIA Corporation <www.nvidia.com>
 */

#include <common.h>
#include <log.h>
#include <linux/errno.h>
#include <asm/arch-tegra/crypto.h>
#include "uboot_aes.h"

static u8 zero_key[16];

#define AES_CMAC_CONST_RB 0x87  /* from RFC 4493, Figure 2.2 */

enum security_op {
	SECURITY_SIGN		= 1 << 0,	/* Sign the data */
	SECURITY_ENCRYPT	= 1 << 1,	/* Encrypt the data */
	SECURITY_DECRYPT	= 1 << 2,	/* Dectypt the data */
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
 * \param key			Input AES key, length AES128_KEY_LENGTH
 * \param key_schedule		Expanded key to use
 * \param src			Source data of length 'num_aes_blocks' blocks
 * \param dst			Destination buffer, length AES128_KEY_LENGTH
 * \param num_aes_blocks	Number of AES blocks to encrypt
 */
static void sign_object(u8 *key, u8 *key_schedule, u8 *src, u8 *dst,
			u32 num_aes_blocks)
{
	u8 tmp_data[AES128_KEY_LENGTH];
	u8 iv[AES128_KEY_LENGTH] = {0};
	u8 left[AES128_KEY_LENGTH];
	u8 k1[AES128_KEY_LENGTH];
	u8 *cbc_chain_data;
	unsigned int i;

	cbc_chain_data = zero_key;	/* Convenient array of 0's for IV */

	/* compute K1 constant needed by AES-CMAC calculation */
	for (i = 0; i < AES128_KEY_LENGTH; i++)
		tmp_data[i] = 0;

	aes_cbc_encrypt_blocks(AES128_KEY_LENGTH, key_schedule, iv,
			       tmp_data, left, 1);

	left_shift_vector(left, k1, sizeof(left));

	if ((left[0] >> 7) != 0) /* get MSB of L */
		k1[AES128_KEY_LENGTH - 1] ^= AES_CMAC_CONST_RB;

	/* compute the AES-CMAC value */
	for (i = 0; i < num_aes_blocks; i++) {
		/* Apply the chain data */
		aes_apply_cbc_chain_data(cbc_chain_data, src, tmp_data);

		/* for the final block, XOR K1 into the IV */
		if (i == num_aes_blocks - 1)
			aes_apply_cbc_chain_data(tmp_data, k1, tmp_data);

		/* encrypt the AES block */
		aes_encrypt(AES128_KEY_LENGTH, tmp_data,
			    key_schedule, dst);

		debug("sign_obj: block %d of %d\n", i, num_aes_blocks);

		/* Update pointers for next loop. */
		cbc_chain_data = dst;
		src += AES128_KEY_LENGTH;
	}
}

/**
 * Decrypt, encrypt or sign a block of data (depending on security mode).
 *
 * \param key		Input AES key, length AES128_KEY_LENGTH
 * \param oper		Security operations mask to perform (enum security_op)
 * \param src		Source data
 * \param length	Size of source data
 * \param sig_dst	Destination address for signature, AES128_KEY_LENGTH bytes
 */
static int tegra_crypto_core(u8 *key, enum security_op oper, u8 *src,
			     u32 length, u8 *sig_dst)
{
	u32 num_aes_blocks;
	u8 key_schedule[AES128_EXPAND_KEY_LENGTH];
	u8 iv[AES128_KEY_LENGTH] = {0};

	debug("%s: length = %d\n", __func__, length);

	aes_expand_key(key, AES128_KEY_LENGTH, key_schedule);

	num_aes_blocks = (length + AES128_KEY_LENGTH - 1) / AES128_KEY_LENGTH;

	if (oper & SECURITY_DECRYPT) {
		/* Perform this in place, resulting in src being decrypted. */
		debug("%s: begin decryption\n", __func__);
		aes_cbc_decrypt_blocks(AES128_KEY_LENGTH, key_schedule, iv, src,
				       src, num_aes_blocks);
		debug("%s: end decryption\n", __func__);
	}

	if (oper & SECURITY_ENCRYPT) {
		/* Perform this in place, resulting in src being encrypted. */
		debug("%s: begin encryption\n", __func__);
		aes_cbc_encrypt_blocks(AES128_KEY_LENGTH, key_schedule, iv, src,
				       src, num_aes_blocks);
		debug("%s: end encryption\n", __func__);
	}

	if (oper & SECURITY_SIGN) {
		/* encrypt the data, overwriting the result in signature. */
		debug("%s: begin signing\n", __func__);
		sign_object(key, key_schedule, src, sig_dst, num_aes_blocks);
		debug("%s: end signing\n", __func__);
	}

	return 0;
}

/**
 * Tegra crypto group
 */
int sign_data_block(u8 *source, unsigned int length, u8 *signature)
{
	return tegra_crypto_core(zero_key, SECURITY_SIGN, source,
				 length, signature);
}

int sign_enc_data_block(u8 *source, unsigned int length, u8 *signature, u8 *key)
{
	return tegra_crypto_core(key, SECURITY_SIGN, source,
				 length, signature);
}

int encrypt_data_block(u8 *source, unsigned int length, u8 *key)
{
	return tegra_crypto_core(key, SECURITY_ENCRYPT, source,
				 length, NULL);
}

int decrypt_data_block(u8 *source, unsigned int length, u8 *key)
{
	return tegra_crypto_core(key, SECURITY_DECRYPT, source,
				 length, NULL);
}
