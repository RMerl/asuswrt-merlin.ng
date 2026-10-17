// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013, Google Inc.
 */

#ifndef USE_HOSTCC
#include <fdtdec.h>
#include <log.h>
#include <malloc.h>
#include <asm/types.h>
#include <asm/byteorder.h>
#include <linux/errno.h>
#include <asm/types.h>
#include <asm/unaligned.h>
#include <dm.h>
#else
#include "fdt_host.h"
#include "mkimage.h"
#include <linux/kconfig.h>
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
 * Return: 0 on success, != 0 on failure
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
			   const uint8_t *msg, int msg_len,
			   const uint8_t *hash, int hash_len)
{
	struct checksum_algo *checksum = info->checksum;
	int ret, pad_len = msg_len - checksum->checksum_len;

	/* Check pkcs1.5 padding bytes */
	ret = rsa_verify_padding(msg, pad_len, checksum);
	if (ret) {
		debug("In RSAVerify(): Padding check failed!\n");
		return -EINVAL;
	}

	/* Check hash */
	if (memcmp((uint8_t *)msg + pad_len, hash, msg_len - pad_len)) {
		debug("In RSAVerify(): Hash check failed!\n");
		return -EACCES;
	}

	return 0;
}

#ifndef USE_HOSTCC
U_BOOT_PADDING_ALGO(pkcs_15) = {
	.name = "pkcs-1.5",
	.verify = padding_pkcs_15_verify,
};
#endif

#if CONFIG_IS_ENABLED(FIT_RSASSA_PSS)
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
 * Return: 0 if the octet string was correctly generated, others on error
 */
static int mask_generation_function1(struct checksum_algo *checksum,
				     const uint8_t *seed, int seed_len,
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
			      const uint8_t *pad, int pad_len,
			      const uint8_t *hash, int hash_len,
			      const uint8_t *salt, int salt_len,
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

/*
 * padding_pss_verify() - verify the pss padding of a signature
 *
 * Works with any salt length
 *
 * msg is a concatenation of : masked_db + h + 0xbc
 * Once unmasked, db is a concatenation of : [0x00]* + 0x01 + salt
 * Length of 0-padding at begin of db depends on salt length.
 *
 * @info:	Specifies key and FIT information
 * @msg:	byte array of message, len equal to msg_len
 * @msg_len:	Message length
 * @hash:	Pointer to the expected hash
 * @hash_len:	Length of the hash
 *
 * Return:	0 if padding is correct, non-zero otherwise
 */
#if !defined(CONFIG_BCMBCA_RSASSA_PSS)

int padding_pss_verify(struct image_sign_info *info,
		       const uint8_t *msg, int msg_len,
		       const uint8_t *hash, int hash_len)
{
	const uint8_t *masked_db = NULL;
	uint8_t *db_mask = NULL;
	uint8_t *db = NULL;
	int db_len = msg_len - hash_len - 1;
	const uint8_t *h = NULL;
	uint8_t *hprime = NULL;
	int h_len = hash_len;
	uint8_t *db_nopad = NULL, *salt = NULL;
	int db_padlen, salt_len;
	uint8_t pad_zero[8] = { 0 };
	int ret, i, leftmost_bits = 1;
	uint8_t leftmost_mask;
	struct checksum_algo *checksum = info->checksum;

	if (db_len <= 0)
		return -EINVAL;

	/* first, allocate everything */
	db_mask = malloc(db_len);
	db = malloc(db_len);
	hprime = malloc(hash_len);
	if (!db_mask || !db || !hprime) {
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
	masked_db = &msg[0];
	h = &msg[db_len];

	/* step 6 */
	leftmost_mask = (0xff >> (8 - leftmost_bits)) << (8 - leftmost_bits);
	if (masked_db[0] & leftmost_mask) {
		printf("%s: invalid pss padding ", __func__);
		printf("(leftmost bit of maskedDB not zero)\n");
		ret = -EINVAL;
		goto out;
	}

	/* step 7 */
	mask_generation_function1(checksum, h, h_len, db_mask, db_len);

	/* step 8 */
	for (i = 0; i < db_len; i++)
		db[i] = masked_db[i] ^ db_mask[i];

	/* step 9 */
	db[0] &= 0xff >> leftmost_bits;

	/* step 10 */
	db_padlen = 0;
	while (db[db_padlen] == 0x00 && db_padlen < (db_len - 1))
		db_padlen++;
	db_nopad = &db[db_padlen];
	if (db_nopad[0] != 0x01) {
		printf("%s: invalid pss padding ", __func__);
		printf("(leftmost byte of db after 0-padding isn't 0x01)\n");
		ret = EINVAL;
		goto out;
	}

	/* step 11 */
	salt_len = db_len - db_padlen - 1;
	salt = &db_nopad[1];

	/* step 12 & 13 */
	compute_hash_prime(checksum, pad_zero, 8,
			   hash, hash_len,
			   salt, salt_len, hprime);

	/* step 14 */
	ret = memcmp(h, hprime, hash_len);

out:
	free(hprime);
	free(db);
	free(db_mask);

	return ret;
}

#else

int padding_pss_verify(struct image_sign_info *info,
		       const uint8_t *msg, int msg_len,
		       const uint8_t *hash, int hash_len)
{
	return emsa_pss_verify(info, (uint8_t*)msg, msg_len, hash, hash_len, -2); 
}

int emsa_pss_verify(struct image_sign_info *info,
		       uint8_t *msg, int msg_len,
		       const uint8_t *hash, int hash_len, int rsa_pss_saltlen_type)
{	uint8_t *masked_db = NULL;
	int masked_db_len = msg_len - hash_len - 1;
	uint8_t *h = NULL, *hprime = NULL;
	int h_len = hash_len;
	uint8_t *db_mask = NULL;
	int db_mask_len = masked_db_len;
	uint8_t *db = NULL, *salt = NULL;
	int db_len = masked_db_len,
		salt_len = (rsa_pss_saltlen_type == -1 ? hash_len : msg_len - hash_len - 2);
	uint8_t pad_zero[8] = { 0 };
	int ret, i, leftmost_bits = 1;
	uint8_t leftmost_mask;
	struct checksum_algo *checksum = info->checksum;

	
	printf("%s salt_len 0x%x hash_len 0x%x\n",__FUNCTION__,salt_len,hash_len);
	if (msg_len < salt_len + hash_len + 2) {
		ret = -EINVAL;
		goto out;
	}
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
	db[(db_len - salt_len) - 1] &= 0xff >> leftmost_bits;

	/* step 10 in RFC 8017 the offset points to last non-salt element of DB 
 	* (the spec. points that out so don't get it confused with the first element of the salt) */
	if (db[(db_len - salt_len) - 1] != 0x01) {
		printf("%s: invalid pss padding ", __func__);
		printf("(leftmost byte of db isn't 0x01)\n");
		ret = EINVAL;
		goto out;
	}

	/* step 11 */
	memcpy(salt, &db[db_len - salt_len], salt_len);

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

#endif /*CONFIG_ARCH_BCMBCA*/

#ifndef USE_HOSTCC
U_BOOT_PADDING_ALGO(pss) = {
	.name = "pss",
	.verify = padding_pss_verify,
};
#endif

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
 * Return: 0 if verified, -ve on error
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

	if (!prop || !sig || !hash || !checksum || !padding)
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
 * rsa_verify_with_pkey() - Verify a signature against some data using
 * only modulus and exponent as RSA key properties.
 * @info:	Specifies key information
 * @hash:	Pointer to the expected hash
 * @sig:	Signature
 * @sig_len:	Number of bytes in signature
 *
 * Parse a RSA public key blob in DER format pointed to in @info and fill
 * a key_prop structure with properties of the key. Then verify a RSA PKCS1.5
 * signature against an expected hash using the calculated properties.
 *
 * Return	0 if verified, -ve on error
 */
int rsa_verify_with_pkey(struct image_sign_info *info,
			 const void *hash, uint8_t *sig, uint sig_len)
{
	struct key_prop *prop;
	int ret;

	if (!CONFIG_IS_ENABLED(RSA_VERIFY_WITH_PKEY))
		return -EACCES;

	/* Public key is self-described to fill key_prop */
	ret = rsa_gen_key_prop(info->key, info->keylen, &prop);
	if (ret) {
		debug("Generating necessary parameter for decoding failed\n");
		return ret;
	}

	ret = rsa_verify_key(info, prop, sig, sig_len, hash,
			     info->crypto->key_len);

	rsa_free_key_prop(prop);

	return ret;
}

#if CONFIG_IS_ENABLED(FIT_SIGNATURE)
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
 * Return: 0 if verified, -ve on error
 */
static int rsa_verify_with_keynode(struct image_sign_info *info,
				   const void *hash, uint8_t *sig,
				   uint sig_len, int node)
{
	const void *blob = info->fdt_blob;
	struct key_prop prop;
	int length;
	int ret = 0;
	const char *algo;

	if (node < 0) {
		debug("%s: Skipping invalid node", __func__);
		return -EBADF;
	}

	algo = fdt_getprop(blob, node, "algo", NULL);
	if (strcmp(info->name, algo)) {
		debug("%s: Wrong algo: have %s, expected %s", __func__,
		      info->name, algo);
		return -EFAULT;
	}

	prop.num_bits = fdtdec_get_int(blob, node, "rsa,num-bits", 0);

	prop.n0inv = fdtdec_get_int(blob, node, "rsa,n0-inverse", 0);

	prop.public_exponent = fdt_getprop(blob, node, "rsa,exponent", &length);
	if (!prop.public_exponent || length < sizeof(uint64_t))
		prop.public_exponent = NULL;

	prop.exp_len = sizeof(uint64_t);

	prop.modulus = fdt_getprop(blob, node, "rsa,modulus", NULL);

	prop.rr = fdt_getprop(blob, node, "rsa,r-squared", NULL);

	if (!prop.num_bits || !prop.modulus || !prop.rr) {
		debug("%s: Missing RSA key info", __func__);
		return -EFAULT;
	}

	ret = rsa_verify_key(info, &prop, sig, sig_len, hash,
			     info->crypto->key_len);

	return ret;
}
#else
static int rsa_verify_with_keynode(struct image_sign_info *info,
				   const void *hash, uint8_t *sig,
				   uint sig_len, int node)
{
	return -EACCES;
}
#endif

int rsa_verify_hash(struct image_sign_info *info,
		    const uint8_t *hash, uint8_t *sig, uint sig_len)
{
	int ret = -EACCES;

	/*
	 * Since host tools, like mkimage, make use of openssl library for
	 * RSA encryption, rsa_verify_with_pkey()/rsa_gen_key_prop() are
	 * of no use and should not be compiled in.
	 */
	if (!tools_build() && CONFIG_IS_ENABLED(RSA_VERIFY_WITH_PKEY) &&
			!info->fdt_blob) {
		/* don't rely on fdt properties */
		ret = rsa_verify_with_pkey(info, hash, sig, sig_len);
		if (ret)
			debug("%s: rsa_verify_with_pkey() failed\n", __func__);
		return ret;
	}

	if (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		const void *blob = info->fdt_blob;
		int ndepth, noffset;
		int sig_node, node;
		char name[100];

		sig_node = fdt_subnode_offset(blob, 0, FIT_SIG_NODENAME);
		if (sig_node < 0) {
			debug("%s: No signature node found\n", __func__);
			return -ENOENT;
		}

		/* See if we must use a particular key */
		if (info->required_keynode != -1) {
			ret = rsa_verify_with_keynode(info, hash, sig, sig_len,
						      info->required_keynode);
			if (ret)
				debug("%s: Failed to verify required_keynode\n",
				      __func__);
			return ret;
		}

		/* Look for a key that matches our hint */
		snprintf(name, sizeof(name), "key-%s", info->keyname);
		node = fdt_subnode_offset(blob, sig_node, name);
		ret = rsa_verify_with_keynode(info, hash, sig, sig_len, node);
		if (!ret)
			return ret;
		debug("%s: Could not verify key '%s', trying all\n", __func__,
		      name);

		/* No luck, so try each of the keys in turn */
		for (ndepth = 0, noffset = fdt_next_node(blob, sig_node,
							 &ndepth);
		     (noffset >= 0) && (ndepth > 0);
		     noffset = fdt_next_node(blob, noffset, &ndepth)) {
			if (ndepth == 1 && noffset != node) {
				ret = rsa_verify_with_keynode(info, hash,
							      sig, sig_len,
							      noffset);
				if (!ret)
					break;
			}
		}
	}
	debug("%s: Failed to verify by any means\n", __func__);

	return ret;
}

int rsa_verify(struct image_sign_info *info,
	       const struct image_region region[], int region_count,
	       uint8_t *sig, uint sig_len)
{
	/* Reserve memory for maximum checksum-length */
	uint8_t hash[info->crypto->key_len];
	int ret;

	/*
	 * Verify that the checksum-length does not exceed the
	 * rsa-signature-length
	 */
	if (info->checksum->checksum_len >
	    info->crypto->key_len) {
		debug("%s: invalid checksum-algorithm %s for %s\n",
		      __func__, info->checksum->name, info->crypto->name);
		return -EINVAL;
	}

	/* Calculate checksum with checksum-algorithm */
	ret = info->checksum->calculate(info->checksum->name,
					region, region_count, hash);
	if (ret < 0) {
		debug("%s: Error in checksum calculation\n", __func__);
		return -EINVAL;
	}

	return rsa_verify_hash(info, hash, sig, sig_len);
}

#ifndef USE_HOSTCC

U_BOOT_CRYPTO_ALGO(rsa2048) = {
	.name = "rsa2048",
	.key_len = RSA2048_BYTES,
	.verify = rsa_verify,
};

U_BOOT_CRYPTO_ALGO(rsa3072) = {
	.name = "rsa3072",
	.key_len = RSA3072_BYTES,
	.verify = rsa_verify,
};

U_BOOT_CRYPTO_ALGO(rsa4096) = {
	.name = "rsa4096",
	.key_len = RSA4096_BYTES,
	.verify = rsa_verify,
};

#endif
