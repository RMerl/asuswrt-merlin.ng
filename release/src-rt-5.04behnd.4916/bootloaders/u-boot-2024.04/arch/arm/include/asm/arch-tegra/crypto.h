/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2010 - 2011 NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _CRYPTO_H_
#define _CRYPTO_H_

/**
 * Sign a block of data
 *
 * \param source	Source data
 * \param length	Size of source data
 * \param signature	Destination address for signature, AES_KEY_LENGTH bytes
 */
int sign_data_block(u8 *source, unsigned int length, u8 *signature);

/**
 * Sign an encrypted block of data
 *
 * \param source	Source data
 * \param length	Size of source data
 * \param signature	Destination address for signature, AES_KEY_LENGTH bytes
 * \param key		AES128 encryption key
 */
int sign_enc_data_block(u8 *source, unsigned int length, u8 *signature, u8 *key);

/**
 * Encrypt a block of data
 *
 * \param source	Source data
 * \param length	Size of source data
 * \param key		AES128 encryption key
 */
int encrypt_data_block(u8 *source, unsigned int length, u8 *key);

/**
 * Decrypt a block of data
 *
 * \param source	Source data
 * \param length	Size of source data
 * \param key		AES128 encryption key
 */
int decrypt_data_block(u8 *source, unsigned int length, u8 *key);

#endif /* #ifndef _CRYPTO_H_ */
