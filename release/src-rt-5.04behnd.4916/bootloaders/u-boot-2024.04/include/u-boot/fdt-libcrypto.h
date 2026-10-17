/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2020, Alexandru Gagniuc <mr.nuke.me@gmail.com>
 * Copyright (c) 2013, Google Inc.
 */

#ifndef _FDT_LIBCRYPTO_H
#define _FDT_LIBCRYPTO_H

#include <openssl/bn.h>

/**
 * fdt_add_bignum() - Write a libcrypto BIGNUM as an FDT property
 *
 * Convert a libcrypto BIGNUM * into a big endian array of integers.
 *
 * @blob:	FDT blob to modify
 * @noffset:	Offset of the FDT node
 * @prop_name:	What to call the property in the FDT
 * @num:	pointer to a libcrypto big number
 * @num_bits:	How big is 'num' in bits?
 * Return: 0 if all good all working, -ve on horror
 */
int fdt_add_bignum(void *blob, int noffset, const char *prop_name,
		   BIGNUM *num, int num_bits);

#endif /* _FDT_LIBCRYPTO_H */
