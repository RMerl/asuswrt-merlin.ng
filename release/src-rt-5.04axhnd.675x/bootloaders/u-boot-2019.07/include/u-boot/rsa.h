/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013, Google Inc.
 *
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef _RSA_H
#define _RSA_H

#include <errno.h>
#include <image.h>

/**
 * struct rsa_public_key - holder for a public key
 *
 * An RSA public key consists of a modulus (typically called N), the inverse
 * and R^2, where R is 2^(# key bits).
 */

struct rsa_public_key {
	uint len;		/* len of modulus[] in number of uint32_t */
	uint32_t n0inv;		/* -1 / modulus[0] mod 2^32 */
	uint32_t *modulus;	/* modulus as little endian array */
	uint32_t *rr;		/* R^2 as little endian array */
	uint64_t exponent;	/* public exponent */
};

struct image_sign_info;

#if IMAGE_ENABLE_SIGN
/**
 * sign() - calculate and return signature for given input data
 *
 * @info:	Specifies key and FIT information
 * @data:	Pointer to the input data
 * @data_len:	Data length
 * @sigp:	Set to an allocated buffer holding the signature
 * @sig_len:	Set to length of the calculated hash
 *
 * This computes input data signature according to selected algorithm.
 * Resulting signature value is placed in an allocated buffer, the
 * pointer is returned as *sigp. The length of the calculated
 * signature is returned via the sig_len pointer argument. The caller
 * should free *sigp.
 *
 * @return: 0, on success, -ve on error
 */
int rsa_sign(struct image_sign_info *info,
	     const struct image_region region[],
	     int region_count, uint8_t **sigp, uint *sig_len);

/**
 * add_verify_data() - Add verification information to FDT
 *
 * Add public key information to the FDT node, suitable for
 * verification at run-time. The information added depends on the
 * algorithm being used.
 *
 * @info:	Specifies key and FIT information
 * @keydest:	Destination FDT blob for public key data
 * @return: 0, on success, -ENOSPC if the keydest FDT blob ran out of space,
		other -ve value on error
*/
int rsa_add_verify_data(struct image_sign_info *info, void *keydest);
#else
static inline int rsa_sign(struct image_sign_info *info,
		const struct image_region region[], int region_count,
		uint8_t **sigp, uint *sig_len)
{
	return -ENXIO;
}

static inline int rsa_add_verify_data(struct image_sign_info *info,
				      void *keydest)
{
	return -ENXIO;
}
#endif

#if IMAGE_ENABLE_VERIFY
/**
 * rsa_verify() - Verify a signature against some data
 *
 * Verify a RSA PKCS1.5 signature against an expected hash.
 *
 * @info:	Specifies key and FIT information
 * @data:	Pointer to the input data
 * @data_len:	Data length
 * @sig:	Signature
 * @sig_len:	Number of bytes in signature
 * @return 0 if verified, -ve on error
 */
int rsa_verify(struct image_sign_info *info,
	       const struct image_region region[], int region_count,
	       uint8_t *sig, uint sig_len);

int padding_pkcs_15_verify(struct image_sign_info *info,
			   uint8_t *msg, int msg_len,
			   const uint8_t *hash, int hash_len);

#ifdef CONFIG_FIT_ENABLE_RSASSA_PSS_SUPPORT
int padding_pss_verify(struct image_sign_info *info,
		       uint8_t *msg, int msg_len,
		       const uint8_t *hash, int hash_len);
#endif /* CONFIG_FIT_ENABLE_RSASSA_PSS_SUPPORT */
#else
static inline int rsa_verify(struct image_sign_info *info,
		const struct image_region region[], int region_count,
		uint8_t *sig, uint sig_len)
{
	return -ENXIO;
}

static inline int padding_pkcs_15_verify(struct image_sign_info *info,
					 uint8_t *msg, int msg_len,
					 const uint8_t *hash, int hash_len)
{
	return -ENXIO;
}

#ifdef CONFIG_FIT_ENABLE_RSASSA_PSS_SUPPORT
static inline int padding_pss_verify(struct image_sign_info *info,
				     uint8_t *msg, int msg_len,
				     const uint8_t *hash, int hash_len)
{
	return -ENXIO;
}
#endif /* CONFIG_FIT_ENABLE_RSASSA_PSS_SUPPORT */
#endif

#define RSA_DEFAULT_PADDING_NAME		"pkcs-1.5"

#define RSA2048_BYTES	(2048 / 8)
#define RSA4096_BYTES	(4096 / 8)

/* This is the minimum/maximum key size we support, in bits */
#define RSA_MIN_KEY_BITS	2048
#define RSA_MAX_KEY_BITS	4096

/* This is the maximum signature length that we support, in bits */
#define RSA_MAX_SIG_BITS	4096

#endif
