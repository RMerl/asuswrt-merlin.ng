/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#ifndef _RSA_MOD_EXP_H
#define _RSA_MOD_EXP_H

#include <errno.h>
#include <image.h>

struct udevice;

/**
 * struct key_prop - holder for a public key properties
 *
 * The struct has pointers to modulus (Typically called N),
 * The inverse, R^2, exponent. These can be typecasted and
 * used as byte arrays or converted to the required format
 * as per requirement of RSA implementation.
 */
struct key_prop {
	const void *rr;		/* R^2 can be treated as byte array */
	const void *modulus;	/* modulus as byte array */
	const void *public_exponent; /* public exponent as byte array */
	uint32_t n0inv;		/* -1 / modulus[0] mod 2^32 */
	int num_bits;		/* Key length in bits */
	uint32_t exp_len;	/* Exponent length in number of uint8_t */
};

/**
 * rsa_gen_key_prop() - Generate key properties of RSA public key
 * @key:	Specifies key data in DER format
 * @keylen:	Length of @key
 * @prop:	Generated key property
 *
 * This function takes a blob of encoded RSA public key data in DER
 * format, parse it and generate all the relevant properties
 * in key_prop structure.
 * Return a pointer to struct key_prop in @prop on success.
 *
 * Return:	0 on success, negative on error
 */
int rsa_gen_key_prop(const void *key, uint32_t keylen, struct key_prop **proc);

/**
 * rsa_free_key_prop() - Free key properties
 * @prop:	Pointer to struct key_prop
 *
 * This function frees all the memories allocated by rsa_gen_key_prop().
 */
void rsa_free_key_prop(struct key_prop *prop);

/**
 * rsa_mod_exp_sw() - Perform RSA Modular Exponentiation in sw
 *
 * Operation: out[] = sig ^ exponent % modulus
 *
 * @sig:	RSA PKCS1.5 signature
 * @sig_len:	Length of signature in number of bytes
 * @node:	Node with RSA key elements like modulus, exponent, R^2, n0inv
 * @out:	Result in form of byte array of len equal to sig_len
 */
int rsa_mod_exp_sw(const uint8_t *sig, uint32_t sig_len,
		struct key_prop *node, uint8_t *out);

int rsa_mod_exp(struct udevice *dev, const uint8_t *sig, uint32_t sig_len,
		struct key_prop *node, uint8_t *out);

#if defined(CONFIG_CMD_ZYNQ_RSA)
int zynq_pow_mod(uint32_t *keyptr, uint32_t *inout);
#endif

/**
 * struct struct mod_exp_ops - Driver model for RSA Modular Exponentiation
 *				operations
 *
 * The uclass interface is implemented by all crypto devices which use
 * driver model.
 */
struct mod_exp_ops {
	/**
	 * Perform Modular Exponentiation
	 *
	 * Operation: out[] = sig ^ exponent % modulus
	 *
	 * @dev:	RSA Device
	 * @sig:	RSA PKCS1.5 signature
	 * @sig_len:	Length of signature in number of bytes
	 * @node:	Node with RSA key elements like modulus, exponent,
	 *		R^2, n0inv
	 * @out:	Result in form of byte array of len equal to sig_len
	 *
	 * This function computes exponentiation over the signature.
	 * Returns: 0 if exponentiation is successful, or a negative value
	 * if it wasn't.
	 */
	int (*mod_exp)(struct udevice *dev, const uint8_t *sig,
			   uint32_t sig_len, struct key_prop *node,
			   uint8_t *outp);
};

#endif
