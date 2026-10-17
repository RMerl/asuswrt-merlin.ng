/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2020, Alexandru Gagniuc <mr.nuke.me@gmail.com>
 */

#include <dm/device.h>

/**
 * struct ecdsa_public_key - ECDSA public key properties
 *
 * The struct has pointers to the (x, y) curve coordinates to an ECDSA public
 * key, as well as the name of the ECDSA curve. The size of the key is inferred
 * from the 'curve_name'
 */
struct ecdsa_public_key {
	const char *curve_name;	/* Name of curve, e.g. "prime256v1" */
	const void *x;		/* x coordinate of public key */
	const void *y;		/* y coordinate of public key */
	unsigned int size_bits;	/* key size in bits, derived from curve name */
};

struct ecdsa_ops {
	/**
	 * Verify signature of hash against given public key
	 *
	 * @dev:	ECDSA Device
	 * @pubkey:	ECDSA public key
	 * @hash:	Hash of binary image
	 * @hash_len:	Length of hash in bytes
	 * @signature:	Signature in a raw (R, S) point pair
	 * @sig_len:	Length of signature in bytes
	 *
	 * This function verifies that the 'signature' of the given 'hash' was
	 * signed by the private key corresponding to 'pubkey'.
	 */
	int (*verify)(struct udevice *dev, const struct ecdsa_public_key *pubkey,
		      const void *hash, size_t hash_len,
		      const void *signature, size_t sig_len);
};
