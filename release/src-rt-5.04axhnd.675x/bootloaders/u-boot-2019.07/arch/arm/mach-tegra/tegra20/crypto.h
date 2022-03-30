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
int sign_data_block(u8 *source, unsigned length, u8 *signature);

#endif /* #ifndef _CRYPTO_H_ */
