/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2010 - 2011 NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _AES_REF_H_
#define _AES_REF_H_

#ifdef USE_HOSTCC
/* Define compat stuff for use in fw_* tools. */
typedef unsigned char u8;
typedef unsigned int u32;
#define debug(...) do {} while (0)
#endif

/*
 * AES encryption library, with small code size, supporting only 128-bit AES
 *
 * AES is a stream cipher which works a block at a time, with each block
 * in this case being AES_BLOCK_LENGTH bytes.
 */

enum {
	AES_STATECOLS	= 4,	/* columns in the state & expanded key */
	AES128_KEYCOLS	= 4,	/* columns in a key for aes128 */
	AES192_KEYCOLS	= 6,	/* columns in a key for aes128 */
	AES256_KEYCOLS	= 8,	/* columns in a key for aes128 */
	AES128_ROUNDS	= 10,	/* rounds in encryption for aes128 */
	AES192_ROUNDS	= 12,	/* rounds in encryption for aes192 */
	AES256_ROUNDS	= 14,	/* rounds in encryption for aes256 */
	AES128_KEY_LENGTH	= 128 / 8,
	AES192_KEY_LENGTH	= 192 / 8,
	AES256_KEY_LENGTH	= 256 / 8,
	AES128_EXPAND_KEY_LENGTH = 4 * AES_STATECOLS * (AES128_ROUNDS + 1),
	AES192_EXPAND_KEY_LENGTH = 4 * AES_STATECOLS * (AES192_ROUNDS + 1),
	AES256_EXPAND_KEY_LENGTH = 4 * AES_STATECOLS * (AES256_ROUNDS + 1),
	AES_BLOCK_LENGTH	= 128 / 8,
};

/**
 * aes_expand_key() - Expand the AES key
 *
 * Expand a key into a key schedule, which is then used for the other
 * operations.
 *
 * @key		Key
 * @key_size	Size of the key (in bits)
 * @expkey	Buffer to place expanded key, AES_EXPAND_KEY_LENGTH
 */
void aes_expand_key(u8 *key, u32 key_size, u8 *expkey);

/**
 * aes_encrypt() - Encrypt single block of data with AES 128
 *
 * @key_size	Size of the aes key (in bits)
 * @in		Input data
 * @expkey	Expanded key to use for encryption (from aes_expand_key())
 * @out		Output data
 */
void aes_encrypt(u32 key_size, u8 *in, u8 *expkey, u8 *out);

/**
 * aes_decrypt() - Decrypt single block of data with AES 128
 *
 * @key_size	Size of the aes key (in bits)
 * @in		Input data
 * @expkey	Expanded key to use for decryption (from aes_expand_key())
 * @out		Output data
 */
void aes_decrypt(u32 key_size, u8 *in, u8 *expkey, u8 *out);

/**
 * Apply chain data to the destination using EOR
 *
 * Each array is of length AES_BLOCK_LENGTH.
 *
 * @cbc_chain_data	Chain data
 * @src			Source data
 * @dst			Destination data, which is modified here
 */
void aes_apply_cbc_chain_data(u8 *cbc_chain_data, u8 *src, u8 *dst);

/**
 * aes_cbc_encrypt_blocks() - Encrypt multiple blocks of data with AES CBC.
 *
 * @key_size		Size of the aes key (in bits)
 * @key_exp		Expanded key to use
 * @iv			Initialization vector
 * @src			Source data to encrypt
 * @dst			Destination buffer
 * @num_aes_blocks	Number of AES blocks to encrypt
 */
void aes_cbc_encrypt_blocks(u32 key_size, u8 *key_exp, u8 *iv, u8 *src, u8 *dst,
			    u32 num_aes_blocks);

/**
 * Decrypt multiple blocks of data with AES CBC.
 *
 * @key_size		Size of the aes key (in bits)
 * @key_exp		Expanded key to use
 * @iv			Initialization vector
 * @src			Source data to decrypt
 * @dst			Destination buffer
 * @num_aes_blocks	Number of AES blocks to decrypt
 */
void aes_cbc_decrypt_blocks(u32 key_size, u8 *key_exp, u8 *iv, u8 *src, u8 *dst,
			    u32 num_aes_blocks);

#endif /* _AES_REF_H_ */
