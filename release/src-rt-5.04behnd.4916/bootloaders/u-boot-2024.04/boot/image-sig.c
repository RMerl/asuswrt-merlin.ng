// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013, Google Inc.
 */

#include <common.h>
#include <log.h>
#include <malloc.h>
#include <asm/global_data.h>
DECLARE_GLOBAL_DATA_PTR;
#include <image.h>
#include <relocate.h>
#include <u-boot/ecdsa.h>
#include <u-boot/rsa.h>
#include <u-boot/hash-checksum.h>

#define IMAGE_MAX_HASHED_NODES		100

struct checksum_algo checksum_algos[] = {
	{
		.name = "sha1",
		.checksum_len = SHA1_SUM_LEN,
		.der_len = SHA1_DER_LEN,
		.der_prefix = sha1_der_prefix,
		.calculate = hash_calculate,
	},
	{
		.name = "sha256",
		.checksum_len = SHA256_SUM_LEN,
		.der_len = SHA256_DER_LEN,
		.der_prefix = sha256_der_prefix,
		.calculate = hash_calculate,
	},
#ifdef CONFIG_SHA384
	{
		.name = "sha384",
		.checksum_len = SHA384_SUM_LEN,
		.der_len = SHA384_DER_LEN,
		.der_prefix = sha384_der_prefix,
		.calculate = hash_calculate,
	},
#endif
#ifdef CONFIG_SHA512
	{
		.name = "sha512",
		.checksum_len = SHA512_SUM_LEN,
		.der_len = SHA512_DER_LEN,
		.der_prefix = sha512_der_prefix,
		.calculate = hash_calculate,
	},
#endif

};

struct checksum_algo *image_get_checksum_algo(const char *full_name)
{
	int i;
	const char *name;

	for (i = 0; i < ARRAY_SIZE(checksum_algos); i++) {
		name = checksum_algos[i].name;
		/* Make sure names match and next char is a comma */
		if (!strncmp(name, full_name, strlen(name)) &&
		    full_name[strlen(name)] == ',')
			return &checksum_algos[i];
	}

	return NULL;
}

struct crypto_algo *image_get_crypto_algo(const char *full_name)
{
	struct crypto_algo *crypto, *end;
	const char *name;

	/* Move name to after the comma */
	name = strchr(full_name, ',');
	if (!name)
		return NULL;
	name += 1;

	crypto = ll_entry_start(struct crypto_algo, cryptos);
	end = ll_entry_end(struct crypto_algo, cryptos);
	for (; crypto < end; crypto++) {
		if (!strcmp(crypto->name, name))
			return crypto;
	}

	/* Not found */
	return NULL;
}

struct padding_algo *image_get_padding_algo(const char *name)
{
	struct padding_algo *padding, *end;

	if (!name)
		return NULL;

	padding = ll_entry_start(struct padding_algo, paddings);
	end = ll_entry_end(struct padding_algo, paddings);
	for (; padding < end; padding++) {
		if (!strcmp(padding->name, name))
			return padding;
	}

	return NULL;
}
