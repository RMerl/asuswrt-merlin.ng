// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013, Google Inc.
 */

#ifndef USE_HOSTCC
#include <common.h>
#include <fdtdec.h>
#include <asm/types.h>
#include <asm/byteorder.h>
#include <linux/errno.h>
#include <asm/types.h>
#include <asm/unaligned.h>
#include <dm.h>
#else
#include "fdt_host.h"
#include "mkimage.h"
#include <fdt_support.h>
#endif
#include <u-boot/rsa-mod-exp.h>
#include <u-boot/rsa.h>

/* Default public exponent for backward compatibility */
#define RSA_DEFAULT_PUBEXP	65537

/**
 * rsa_verify_padding() - Verify RSA message padding is valid
 *
 * Verify a RSA message's padding is consistent with PKCS1.5
 * padding as described in the RSA PKCS#1 v2.1 standard.
 *
 * @msg:	Padded message
 * @pad_len:	Number of expected padding bytes
 * @algo:	Checksum algo structure having information on DER encoding etc.
 * @return 0 on success, != 0 on failure
 */
static int rsa_verify_padding(const uint8_t *msg, const int pad_len,
			      struct checksum_algo *algo)
{
	int ff_len;
	int ret;

	/* first byte must be 0x00 */
	ret = *msg++;
	/* second byte must be 0x01 */
	ret |= *msg++ ^ 0x01;
	/* next ff_len bytes must be 0xff */
	ff_len = pad_len - algo->der_len - 3;
	ret |= *msg ^ 0xff;
	ret |= memcmp(msg, msg+1, ff_len-1);
	msg += ff_len;
	/* next byte must be 0x00 */
	ret |= *msg++;
	/* next der_len bytes must match der_prefix */
	ret |= memcmp(msg, algo->der_prefix, algo->der_len);

	return ret;
}

int padding_pkcs_15_verify(struct image_sign_info *info,
			   uint8_t *msg, int msg_len,
			   const uint8_t *hash, int hash_len)
{
	struct checksum_algo *checksum = info->checksum;
	int ret, pad_len = msg_len - checksum->checksum_len;

	/* Check pkcs1.5 padding bytes. */
	ret = rsa_verify_padding(msg, pad_len, checksum);
	if (ret) {
		debug("In RSAVerify(): Padding check failed!\n");
		return -EINVAL;
	}

	/* Check hash. */
	if (memcmp((uint8_t *)msg + pad_len, hash, msg_len - pad_len)) {
		debug("In RSAVerify(): Hash check failed!\n");
		return -EACCES;
	}

	return 0;
}

#ifdef CONFIG_FIT_ENABLE_RSASSA_PSS_SUPPORT
static void u32_i2osp(uint32_t val, uint8_t *buf)
{
	buf[0] = (uint8_t)((val >> 24) & 0xff);
	buf[1] = (uint8_t)((val >> 16) & 0xff);
	buf[2] = (uint8_t)((val >>  8) & 0xff);
	buf[3] = (uint8_t)((val >>  0) & 0xff);
}

/**
 * mask_generation_function1() - generate an octet string
 *
 * Generate an octet string used to check rsa signature.
 * It use an input octet string and a hash function.
 *
 * @checksum:	A Hash function
 * @seed:	Specifies an input variable octet string
 * @seed_len:	Size of the input octet string
 * @output:	Specifies the output octet string
 * @output_len:	Size of the output octet string
 * @return 0 if the octet string was correctly generated, others on error
 */
static int mask_generation_function1(struct checksum_algo *checksum,
				     uint8_t *seed, int seed_len,
				     uint8_t *output, int output_len)
{
	struct image_region region[2];
	int ret = 0, i, i_output = 0, region_count = 2;
	uint32_t counter = 0;
	uint8_t buf_counter[4], *tmp;
	int hash_len = checksum->checksum_len;

	memset(output, 0, output_len);

	region[0].data = seed;
	region[0].size = seed_len;
	region[1].data = &buf_counter[0];
	region[1].size = 4;

	tmp = malloc(hash_len);
	if (!tmp) {
		debug("%s: can't allocate array tmp\n", __func__);
		ret = -ENOMEM;
		goto out;
	}

	while (i_output < output_len) {
		u32_i2osp(counter, &buf_counter[0]);

		ret = checksum->calculate(checksum->name,
					  region, region_count,
					  tmp);
		if (ret < 0) {
			debug("%s: Error in checksum calculation\n", __func__);
			goto out;
		}

		i = 0;
		while ((i_output < output_len) && (i < hash_len)) {
			output[i_output] = tmp[i];
			i_output++;
			i++;
		}

		counter++;
	}

out:
	free(tmp);

	return ret;
}

static int compute_hash_prime(struct checksum_algo *checksum,
			      uint8_t *pad, int pad_len,
			      uint8_t *hash, int hash_len,
			      uint8_t *salt, int salt_len,
			      uint8_t *hprime)
{
	struct image_region region[3];
	int ret, region_count = 3;

	region[0].data = pad;
	region[0].size = pad_len;
	region[1].data = hash;
	region[1].size = hash_len;
	region[2].data = salt;
	region[2].size = salt_len;

	ret = checksum->calculate(checksum->name, region, region_count, hprime);
	if (ret < 0) {
		debug("%s: Error in checksum calculation\n", __func__);
		goto out;
	}

out:
	return ret;
}

int padding_pss_verify(struct image_sign_info *info,
		       uint8_t *msg, int msg_len,
		       const uint8_t *hash, int hash_len)
{
	uint8_t *masked_db = NULL;
	int masked_db_len = msg_len - hash_len - 1;
	uint8_t *h = NULL, *hprime = NULL;
	int h_len = hash_len;
	uint8_t *db_mask = NULL;
	int db_mask_len = masked_db_len;
	uint8_t *db = NULL, *salt = NULL;
	int db_len = masked_db_len, 
		salt_len = SHA256_SUM_LEN;
	uint8_t pad_zero[8] = { 0 };
	int ret, i, leftmost_bits = 1;
	uint8_t leftmost_mask;
	struct checksum_algo *checksum = info->checksum;

	/* first, allocate everything */
	masked_db = malloc(masked_db_len);
	h = malloc(h_len);
	db_mask = malloc(db_mask_len);
	db = malloc(db_len);
	salt = malloc(salt_len);
	hprime = malloc(hash_len);
	if (!masked_db || !h || !db_mask || !db || !salt || !hprime) {
		printf("%s: can't allocate some buffer\n", __func__);
		ret = -ENOMEM;
		goto out;
	}

	/* step 4: check if the last byte is 0xbc */
	if (msg[msg_len - 1] != 0xbc) {
		printf("%s: invalid pss padding (0xbc is missing)\n", __func__);
		ret = -EINVAL;
		goto out;
	}

	/* step 5 */
	memcpy(masked_db, msg, masked_db_len);
	memcpy(h, msg + masked_db_len, h_len);
	
	/* step 6 */
	leftmost_mask = (0xff >> (8 - leftmost_bits)) << (8 - leftmost_bits);
	if (masked_db[0] & leftmost_mask) {
		printf("%s: invalid pss padding ", __func__);
		printf("(leftmost bit of maskedDB not zero)\n");
		ret = -EINVAL;
		goto out;
	}

	/* step 7 */
	mask_generation_function1(checksum, h, h_len, db_mask, db_mask_len);
	
	/* step 8 */
	for (i = 0; i < db_len; i++)
		db[i] = masked_db[i] ^ db_mask[i];
	
	/* step 9 */
	db[db_len - (salt_len+1)] &= 0xff >> leftmost_bits;

	/* step 10 */
	if (db[db_len-(salt_len+1)] != 0x01) {
		printf("%s: invalid pss padding ", __func__);
		printf("(leftmost byte of db isn't 0x01)\n");
		ret = EINVAL;
		goto out;
	}

	/* step 11 */
	memcpy(salt, &db[db_len-salt_len], salt_len);

	/* step 12 & 13 */
	compute_hash_prime(checksum, pad_zero, 8,
			   (uint8_t *)hash, hash_len,
			   salt, salt_len, hprime);
	
	/* step 14 */
	ret = memcmp(h, hprime, hash_len);

out:
	free(hprime);
	free(salt);
	free(db);
	free(db_mask);
	free(h);
	free(masked_db);

	return ret;
}
#endif

/**
 * rsa_verify_key() - Verify a signature against some data using RSA Key
 *
 * Verify a RSA PKCS1.5 signature against an expected hash using
 * the RSA Key properties in prop structure.
 *
 * @info:	Specifies key and FIT information
 * @prop:	Specifies key
 * @sig:	Signature
 * @sig_len:	Number of bytes in signature
 * @hash:	Pointer to the expected hash
 * @key_len:	Number of bytes in rsa key
 * @return 0 if verified, -ve on error
 */
static int rsa_verify_key(struct image_sign_info *info,
			  struct key_prop *prop, const uint8_t *sig,
			  const uint32_t sig_len, const uint8_t *hash,
			  const uint32_t key_len)
{
	int ret;
#if !defined(USE_HOSTCC)
	struct udevice *mod_exp_dev;
#endif
	struct checksum_algo *checksum = info->checksum;
	struct padding_algo *padding = info->padding;
	int hash_len;

	if (!prop || !sig || !hash || !checksum)
		return -EIO;

	if (sig_len != (prop->num_bits / 8)) {
		debug("Signature is of incorrect length %d\n", sig_len);
		return -EINVAL;
	}

	debug("Checksum algorithm: %s", checksum->name);

	/* Sanity check for stack size */
	if (sig_len > RSA_MAX_SIG_BITS / 8) {
		debug("Signature length %u exceeds maximum %d\n", sig_len,
		      RSA_MAX_SIG_BITS / 8);
		return -EINVAL;
	}

	uint8_t buf[sig_len];
	hash_len = checksum->checksum_len;

#if !defined(USE_HOSTCC)
	ret = uclass_get_device(UCLASS_MOD_EXP, 0, &mod_exp_dev);
	if (ret) {
		printf("RSA: Can't find Modular Exp implementation\n");
		return -EINVAL;
	}

	ret = rsa_mod_exp(mod_exp_dev, sig, sig_len, prop, buf);
#else
	ret = rsa_mod_exp_sw(sig, sig_len, prop, buf);
#endif
	if (ret) {
		debug("Error in Modular exponentation\n");
		return ret;
	}

	ret = padding->verify(info, buf, key_len, hash, hash_len);
	if (ret) {
		debug("In RSAVerify(): padding check failed!\n");
		return ret;
	}

	return 0;
}

/**
 * rsa_verify_with_keynode() - Verify a signature against some data using
 * information in node with prperties of RSA Key like modulus, exponent etc.
 *
 * Parse sign-node and fill a key_prop structure with properties of the
 * key.  Verify a RSA PKCS1.5 signature against an expected hash using
 * the properties parsed
 *
 * @info:	Specifies key and FIT information
 * @hash:	Pointer to the expected hash
 * @sig:	Signature
 * @sig_len:	Number of bytes in signature
 * @node:	Node having the RSA Key properties
 * @return 0 if verified, -ve on error
 */
static int rsa_verify_with_keynode(struct image_sign_info *info,
				   const void *hash, uint8_t *sig,
				   uint sig_len, int node)
{
	const void *blob = info->fdt_blob;
	struct key_prop prop;
	int length;
	int ret = 0;

	if (node < 0) {
		debug("%s: Skipping invalid node", __func__);
		return -EBADF;
	}

	prop.num_bits = fdtdec_get_int(blob, node, "rsa,num-bits", 0);

	prop.n0inv = fdtdec_get_int(blob, node, "rsa,n0-inverse", 0);

	prop.public_exponent = fdt_getprop(blob, node, "rsa,exponent", &length);
	if (!prop.public_exponent || length < sizeof(uint64_t))
		prop.public_exponent = NULL;

	prop.exp_len = sizeof(uint64_t);

	prop.modulus = fdt_getprop(blob, node, "rsa,modulus", NULL);

	prop.rr = fdt_getprop(blob, node, "rsa,r-squared", NULL);

	if (!prop.num_bits || !prop.modulus) {
		debug("%s: Missing RSA key info", __func__);
		return -EFAULT;
	}

	ret = rsa_verify_key(info, &prop, sig, sig_len, hash,
			     info->crypto->key_len);

	return ret;
}

int rsa_verify(struct image_sign_info *info,
	       const struct image_region region[], int region_count,
	       uint8_t *sig, uint sig_len)
{
	const void *blob = info->fdt_blob;
	/* Reserve memory for maximum checksum-length */
	uint8_t hash[info->crypto->key_len];
	int ndepth, noffset;
	int sig_node, node;
	char name[100];
	int ret;

	/*
	 * Verify that the checksum-length does not exceed the
	 * rsa-signature-length
	 */
	if (info->checksum->checksum_len >
	    info->crypto->key_len) {
		debug("%s: invlaid checksum-algorithm %s for %s\n",
		      __func__, info->checksum->name, info->crypto->name);
		return -EINVAL;
	}

	sig_node = fdt_subnode_offset(blob, 0, FIT_SIG_NODENAME);
	if (sig_node < 0) {
		debug("%s: No signature node found\n", __func__);
		return -ENOENT;
	}

	/* Calculate checksum with checksum-algorithm */
	ret = info->checksum->calculate(info->checksum->name,
					region, region_count, hash);
	if (ret < 0) {
		debug("%s: Error in checksum calculation\n", __func__);
		return -EINVAL;
	}

	/* See if we must use a particular key */
	if (info->required_keynode != -1) {
		ret = rsa_verify_with_keynode(info, hash, sig, sig_len,
			info->required_keynode);
		if (!ret)
			return ret;
	}

	/* Look for a key that matches our hint */
	snprintf(name, sizeof(name), "key-%s", info->keyname);
	node = fdt_subnode_offset(blob, sig_node, name);
	ret = rsa_verify_with_keynode(info, hash, sig, sig_len, node);
	if (!ret)
		return ret;

	/* No luck, so try each of the keys in turn */
	for (ndepth = 0, noffset = fdt_next_node(info->fit, sig_node, &ndepth);
			(noffset >= 0) && (ndepth > 0);
			noffset = fdt_next_node(info->fit, noffset, &ndepth)) {
		if (ndepth == 1 && noffset != node) {
			ret = rsa_verify_with_keynode(info, hash, sig, sig_len,
						      noffset);
			if (!ret)
				break;
		}
	}

	return ret;
}
