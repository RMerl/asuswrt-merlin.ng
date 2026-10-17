// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018 Arm Ltd.
 * (C) Copyright 2020-2021 Samuel Holland <samuel@sholland.org>
 */

#define OPENSSL_API_COMPAT 0x10101000L

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/asn1t.h>
#include <openssl/bn.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

#include <image.h>
#include <sunxi_image.h>

#include "imagetool.h"
#include "mkimage.h"

/*
 * NAND requires 8K padding. For other devices, BROM requires only
 * 512B padding, but let's use the larger padding to cover everything.
 */
#define PAD_SIZE		8192

#define pr_fmt(fmt)		"mkimage (TOC0): %s: " fmt
#define pr_err(fmt, args...)	fprintf(stderr, pr_fmt(fmt), "error", ##args)
#define pr_warn(fmt, args...)	fprintf(stderr, pr_fmt(fmt), "warning", ##args)
#define pr_info(fmt, args...)	fprintf(stderr, pr_fmt(fmt), "info", ##args)

#if defined(LIBRESSL_VERSION_NUMBER) && LIBRESSL_VERSION_NUMBER < 0x3050000fL
#define RSA_get0_n(key) (key)->n
#define RSA_get0_e(key) (key)->e
#define RSA_get0_d(key) (key)->d
#endif

struct __packed toc0_key_item {
	__le32  vendor_id;
	__le32  key0_n_len;
	__le32  key0_e_len;
	__le32  key1_n_len;
	__le32  key1_e_len;
	__le32  sig_len;
	uint8_t key0[512];
	uint8_t key1[512];
	uint8_t reserved[32];
	uint8_t sig[256];
};

/*
 * This looks somewhat like an X.509 certificate, but it is not valid BER.
 *
 * Some differences:
 *  - Some X.509 certificate fields are missing or rearranged.
 *  - Some sequences have the wrong tag.
 *  - Zero-length sequences are accepted.
 *  - Large strings and integers must be an even number of bytes long.
 *  - Positive integers are not zero-extended to maintain their sign.
 *
 * See https://linux-sunxi.org/TOC0 for more information.
 */
struct __packed toc0_small_tag {
	uint8_t tag;
	uint8_t length;
};

typedef struct toc0_small_tag toc0_small_int;
typedef struct toc0_small_tag toc0_small_oct;
typedef struct toc0_small_tag toc0_small_seq;
typedef struct toc0_small_tag toc0_small_exp;

#define TOC0_SMALL_INT(len) { 0x02, (len) }
#define TOC0_SMALL_SEQ(len) { 0x30, (len) }
#define TOC0_SMALL_EXP(tag, len) { 0xa0 | (tag), len }

struct __packed toc0_large_tag {
	uint8_t tag;
	uint8_t prefix;
	uint8_t length_hi;
	uint8_t length_lo;
};

typedef struct toc0_large_tag toc0_large_int;
typedef struct toc0_large_tag toc0_large_bit;
typedef struct toc0_large_tag toc0_large_seq;

#define TOC0_LARGE_INT(len) { 0x02, 0x82, (len) >> 8, (len) & 0xff }
#define TOC0_LARGE_BIT(len) { 0x03, 0x82, (len) >> 8, (len) & 0xff }
#define TOC0_LARGE_SEQ(len) { 0x30, 0x82, (len) >> 8, (len) & 0xff }

struct __packed toc0_cert_item {
	toc0_large_seq tag_totalSequence;
	struct __packed toc0_totalSequence {
		toc0_large_seq tag_mainSequence;
		struct __packed toc0_mainSequence {
			toc0_small_exp tag_explicit0;
			struct __packed toc0_explicit0 {
				toc0_small_int tag_version;
				uint8_t version;
			} explicit0;
			toc0_small_int tag_serialNumber;
			uint8_t serialNumber;
			toc0_small_seq tag_signature;
			toc0_small_seq tag_issuer;
			toc0_small_seq tag_validity;
			toc0_small_seq tag_subject;
			toc0_large_seq tag_subjectPublicKeyInfo;
			struct __packed toc0_subjectPublicKeyInfo {
				toc0_small_seq tag_algorithm;
				toc0_large_seq tag_publicKey;
				struct __packed toc0_publicKey {
					toc0_large_int tag_n;
					uint8_t n[256];
					toc0_small_int tag_e;
					uint8_t e[3];
				} publicKey;
			} subjectPublicKeyInfo;
			toc0_small_exp tag_explicit3;
			struct __packed toc0_explicit3 {
				toc0_small_seq tag_extension;
				struct __packed toc0_extension {
					toc0_small_int tag_digest;
					uint8_t digest[32];
				} extension;
			} explicit3;
		} mainSequence;
		toc0_large_bit tag_sigSequence;
		struct __packed toc0_sigSequence {
			toc0_small_seq tag_algorithm;
			toc0_large_bit tag_signature;
			uint8_t signature[256];
		} sigSequence;
	} totalSequence;
};

#define sizeof_field(TYPE, MEMBER) sizeof((((TYPE *)0)->MEMBER))

static const struct toc0_cert_item cert_item_template = {
	TOC0_LARGE_SEQ(sizeof(struct toc0_totalSequence)),
	{
		TOC0_LARGE_SEQ(sizeof(struct toc0_mainSequence)),
		{
			TOC0_SMALL_EXP(0, sizeof(struct toc0_explicit0)),
			{
				TOC0_SMALL_INT(sizeof_field(struct toc0_explicit0, version)),
				0,
			},
			TOC0_SMALL_INT(sizeof_field(struct toc0_mainSequence, serialNumber)),
			0,
			TOC0_SMALL_SEQ(0),
			TOC0_SMALL_SEQ(0),
			TOC0_SMALL_SEQ(0),
			TOC0_SMALL_SEQ(0),
			TOC0_LARGE_SEQ(sizeof(struct toc0_subjectPublicKeyInfo)),
			{
				TOC0_SMALL_SEQ(0),
				TOC0_LARGE_SEQ(sizeof(struct toc0_publicKey)),
				{
					TOC0_LARGE_INT(sizeof_field(struct toc0_publicKey, n)),
					{},
					TOC0_SMALL_INT(sizeof_field(struct toc0_publicKey, e)),
					{},
				},
			},
			TOC0_SMALL_EXP(3, sizeof(struct toc0_explicit3)),
			{
				TOC0_SMALL_SEQ(sizeof(struct toc0_extension)),
				{
					TOC0_SMALL_INT(sizeof_field(struct toc0_extension, digest)),
					{},
				},
			},
		},
		TOC0_LARGE_BIT(sizeof(struct toc0_sigSequence)),
		{
			TOC0_SMALL_SEQ(0),
			TOC0_LARGE_BIT(sizeof_field(struct toc0_sigSequence, signature)),
			{},
		},
	},
};

#define TOC0_DEFAULT_NUM_ITEMS		3
#define TOC0_DEFAULT_HEADER_LEN						  \
	ALIGN(								  \
		sizeof(struct toc0_main_info)				+ \
		sizeof(struct toc0_item_info) *	TOC0_DEFAULT_NUM_ITEMS	+ \
		sizeof(struct toc0_cert_item)				+ \
		sizeof(struct toc0_key_item),				  \
	32)

static char *fw_key_file   = "fw_key.pem";
static char *key_item_file = "key_item.bin";
static char *root_key_file = "root_key.pem";

/*
 * Create a key item in @buf, containing the public keys @root_key and @fw_key,
 * and signed by the RSA key @root_key.
 */
static int toc0_create_key_item(uint8_t *buf, uint32_t *len,
				RSA *root_key, RSA *fw_key)
{
	struct toc0_key_item *key_item = (void *)buf;
	uint8_t digest[SHA256_DIGEST_LENGTH];
	int ret = EXIT_FAILURE;
	unsigned int sig_len;
	int n_len, e_len;

	/* Store key 0. */
	n_len = BN_bn2bin(RSA_get0_n(root_key), key_item->key0);
	e_len = BN_bn2bin(RSA_get0_e(root_key), key_item->key0 + n_len);
	if (n_len + e_len > sizeof(key_item->key0)) {
		pr_err("Root key is too big for key item\n");
		goto err;
	}
	key_item->key0_n_len = cpu_to_le32(n_len);
	key_item->key0_e_len = cpu_to_le32(e_len);

	/* Store key 1. */
	n_len = BN_bn2bin(RSA_get0_n(fw_key), key_item->key1);
	e_len = BN_bn2bin(RSA_get0_e(fw_key), key_item->key1 + n_len);
	if (n_len + e_len > sizeof(key_item->key1)) {
		pr_err("Firmware key is too big for key item\n");
		goto err;
	}
	key_item->key1_n_len = cpu_to_le32(n_len);
	key_item->key1_e_len = cpu_to_le32(e_len);

	/* Sign the key item. */
	key_item->sig_len = cpu_to_le32(RSA_size(root_key));
	SHA256(buf, key_item->sig - buf, digest);
	if (!RSA_sign(NID_sha256, digest, sizeof(digest),
		      key_item->sig, &sig_len, root_key)) {
		pr_err("Failed to sign key item\n");
		goto err;
	}
	if (sig_len != sizeof(key_item->sig)) {
		pr_err("Bad key item signature length\n");
		goto err;
	}

	*len = sizeof(*key_item);
	ret = EXIT_SUCCESS;

err:
	return ret;
}

/*
 * Verify the key item in @buf, containing two public keys @key0 and @key1,
 * and signed by the RSA key @key0. If @root_key is provided, only signatures
 * by that key will be accepted. @key1 is returned in @key.
 */
static int toc0_verify_key_item(const uint8_t *buf, uint32_t len,
				RSA *root_key, RSA **fw_key)
{
	struct toc0_key_item *key_item = (void *)buf;
	uint8_t digest[SHA256_DIGEST_LENGTH];
	int ret = EXIT_FAILURE;
	int n_len, e_len;
	RSA *key0 = NULL;
	RSA *key1 = NULL;
	BIGNUM *n, *e;

	if (len < sizeof(*key_item))
		goto err;

	/* Load key 0. */
	n_len = le32_to_cpu(key_item->key0_n_len);
	e_len = le32_to_cpu(key_item->key0_e_len);
	if (n_len + e_len > sizeof(key_item->key0)) {
		pr_err("Bad root key size in key item\n");
		goto err;
	}
	n = BN_bin2bn(key_item->key0, n_len, NULL);
	e = BN_bin2bn(key_item->key0 + n_len, e_len, NULL);
	key0 = RSA_new();
	if (!key0)
		goto err;
	if (!RSA_set0_key(key0, n, e, NULL))
		goto err;

	/* If a root key was provided, compare it to key 0. */
	if (root_key && (BN_cmp(n, RSA_get0_n(root_key)) ||
			 BN_cmp(e, RSA_get0_e(root_key)))) {
		pr_err("Wrong root key in key item\n");
		goto err;
	}

	/* Verify the key item signature. */
	SHA256(buf, key_item->sig - buf, digest);
	if (!RSA_verify(NID_sha256, digest, sizeof(digest),
			key_item->sig, le32_to_cpu(key_item->sig_len), key0)) {
		pr_err("Bad key item signature\n");
		goto err;
	}

	if (fw_key) {
		/* Load key 1. */
		n_len = le32_to_cpu(key_item->key1_n_len);
		e_len = le32_to_cpu(key_item->key1_e_len);
		if (n_len + e_len > sizeof(key_item->key1)) {
			pr_err("Bad firmware key size in key item\n");
			goto err;
		}
		n = BN_bin2bn(key_item->key1, n_len, NULL);
		e = BN_bin2bn(key_item->key1 + n_len, e_len, NULL);
		key1 = RSA_new();
		if (!key1)
			goto err;
		if (!RSA_set0_key(key1, n, e, NULL))
			goto err;

		if (*fw_key) {
			/* If a FW key was provided, compare it to key 1. */
			if (BN_cmp(n, RSA_get0_n(*fw_key)) ||
			    BN_cmp(e, RSA_get0_e(*fw_key))) {
				pr_err("Wrong firmware key in key item\n");
				goto err;
			}
		} else {
			/* Otherwise, send key1 back to the caller. */
			*fw_key = key1;
			key1 = NULL;
		}
	}

	ret = EXIT_SUCCESS;

err:
	RSA_free(key0);
	RSA_free(key1);

	return ret;
}

/*
 * Create a certificate in @buf, describing the firmware with SHA256 digest
 * @digest, and signed by the RSA key @fw_key.
 */
static int toc0_create_cert_item(uint8_t *buf, uint32_t *len, RSA *fw_key,
				 uint8_t digest[static SHA256_DIGEST_LENGTH])
{
	struct toc0_cert_item *cert_item = (void *)buf;
	uint8_t cert_digest[SHA256_DIGEST_LENGTH];
	struct toc0_totalSequence *totalSequence;
	struct toc0_sigSequence *sigSequence;
	struct toc0_extension *extension;
	struct toc0_publicKey *publicKey;
	int ret = EXIT_FAILURE;
	unsigned int sig_len;

	memcpy(cert_item, &cert_item_template, sizeof(*cert_item));
	*len = sizeof(*cert_item);

	/*
	 * Fill in the public key.
	 *
	 * Only 2048-bit RSA keys are supported. Since this uses a fixed-size
	 * structure, it may fail for non-standard exponents.
	 */
	totalSequence = &cert_item->totalSequence;
	publicKey = &totalSequence->mainSequence.subjectPublicKeyInfo.publicKey;
	if (BN_bn2binpad(RSA_get0_n(fw_key), publicKey->n, sizeof(publicKey->n)) < 0 ||
	    BN_bn2binpad(RSA_get0_e(fw_key), publicKey->e, sizeof(publicKey->e)) < 0) {
		pr_err("Firmware key is too big for certificate\n");
		goto err;
	}

	/* Fill in the firmware digest. */
	extension = &totalSequence->mainSequence.explicit3.extension;
	memcpy(&extension->digest, digest, SHA256_DIGEST_LENGTH);

	/*
	 * Sign the certificate.
	 *
	 * In older SBROM versions (and by default in newer versions),
	 * the last 4 bytes of the certificate are not signed.
	 *
	 * (The buffer passed to SHA256 starts at tag_mainSequence, but
	 *  the buffer size does not include the length of that tag.)
	 */
	SHA256((uint8_t *)totalSequence, sizeof(struct toc0_mainSequence), cert_digest);
	sigSequence = &totalSequence->sigSequence;
	if (!RSA_sign(NID_sha256, cert_digest, SHA256_DIGEST_LENGTH,
		      sigSequence->signature, &sig_len, fw_key)) {
		pr_err("Failed to sign certificate\n");
		goto err;
	}
	if (sig_len != sizeof(sigSequence->signature)) {
		pr_err("Bad certificate signature length\n");
		goto err;
	}

	ret = EXIT_SUCCESS;

err:
	return ret;
}

/*
 * Verify the certificate in @buf, describing the firmware with SHA256 digest
 * @digest, and signed by the RSA key contained within. If @fw_key is provided,
 * only that key will be accepted.
 *
 * This function is only expected to work with images created by mkimage.
 */
static int toc0_verify_cert_item(const uint8_t *buf, uint32_t len, RSA *fw_key,
				 uint8_t digest[static SHA256_DIGEST_LENGTH])
{
	const struct toc0_cert_item *cert_item = (const void *)buf;
	uint8_t cert_digest[SHA256_DIGEST_LENGTH];
	const struct toc0_totalSequence *totalSequence;
	const struct toc0_sigSequence *sigSequence;
	const struct toc0_extension *extension;
	const struct toc0_publicKey *publicKey;
	int ret = EXIT_FAILURE;
	RSA *key = NULL;
	BIGNUM *n, *e;

	/* Extract the public key from the certificate. */
	totalSequence = &cert_item->totalSequence;
	publicKey = &totalSequence->mainSequence.subjectPublicKeyInfo.publicKey;
	n = BN_bin2bn(publicKey->n, sizeof(publicKey->n), NULL);
	e = BN_bin2bn(publicKey->e, sizeof(publicKey->e), NULL);
	key = RSA_new();
	if (!key)
		goto err;
	if (!RSA_set0_key(key, n, e, NULL))
		goto err;

	/* If a key was provided, compare it to the embedded key. */
	if (fw_key && (BN_cmp(RSA_get0_n(key), RSA_get0_n(fw_key)) ||
		       BN_cmp(RSA_get0_e(key), RSA_get0_e(fw_key)))) {
		pr_err("Wrong firmware key in certificate\n");
		goto err;
	}

	/* If a digest was provided, compare it to the embedded digest. */
	extension = &totalSequence->mainSequence.explicit3.extension;
	if (digest && memcmp(&extension->digest, digest, SHA256_DIGEST_LENGTH)) {
		pr_err("Wrong firmware digest in certificate\n");
		goto err;
	}

	/* Verify the certificate's signature. See the comment above. */
	SHA256((uint8_t *)totalSequence, sizeof(struct toc0_mainSequence), cert_digest);
	sigSequence = &totalSequence->sigSequence;
	if (!RSA_verify(NID_sha256, cert_digest, SHA256_DIGEST_LENGTH,
			sigSequence->signature,
			sizeof(sigSequence->signature), key)) {
		pr_err("Bad certificate signature\n");
		goto err;
	}

	ret = EXIT_SUCCESS;

err:
	RSA_free(key);

	return ret;
}

/*
 * Always create a TOC0 containing 3 items. The extra item will be ignored on
 * SoCs which do not support it.
 */
static int toc0_create(uint8_t *buf, uint32_t len, RSA *root_key, RSA *fw_key,
		       uint8_t *key_item, uint32_t key_item_len,
		       uint8_t *fw_item, uint32_t fw_item_len, uint32_t fw_addr)
{
	struct toc0_main_info *main_info = (void *)buf;
	struct toc0_item_info *item_info = (void *)(main_info + 1);
	uint8_t digest[SHA256_DIGEST_LENGTH];
	uint32_t *buf32 = (void *)buf;
	RSA *orig_fw_key = fw_key;
	int ret = EXIT_FAILURE;
	uint32_t checksum = 0;
	uint32_t item_offset;
	uint32_t item_length;
	int i;

	/* Hash the firmware for inclusion in the certificate. */
	SHA256(fw_item, fw_item_len, digest);

	/* Create the main TOC0 header, containing three items. */
	memcpy(main_info->name, TOC0_MAIN_INFO_NAME, sizeof(main_info->name));
	main_info->magic	= cpu_to_le32(TOC0_MAIN_INFO_MAGIC);
	main_info->checksum	= cpu_to_le32(BROM_STAMP_VALUE);
	main_info->num_items	= cpu_to_le32(TOC0_DEFAULT_NUM_ITEMS);
	memcpy(main_info->end, TOC0_MAIN_INFO_END, sizeof(main_info->end));

	/* The first item links the ROTPK to the signing key. */
	item_offset = sizeof(*main_info) +
		      sizeof(*item_info) * TOC0_DEFAULT_NUM_ITEMS;
	/* Using an existing key item avoids needing the root private key. */
	if (key_item) {
		item_length = sizeof(*key_item);
		if (toc0_verify_key_item(key_item, item_length,
					 root_key, &fw_key))
			goto err;
		memcpy(buf + item_offset, key_item, item_length);
	} else if (toc0_create_key_item(buf + item_offset, &item_length,
					root_key, fw_key)) {
		goto err;
	}

	item_info->name		= cpu_to_le32(TOC0_ITEM_INFO_NAME_KEY);
	item_info->offset	= cpu_to_le32(item_offset);
	item_info->length	= cpu_to_le32(item_length);
	memcpy(item_info->end, TOC0_ITEM_INFO_END, sizeof(item_info->end));

	/* The second item contains a certificate signed by the firmware key. */
	item_offset = item_offset + item_length;
	if (toc0_create_cert_item(buf + item_offset, &item_length,
				  fw_key, digest))
		goto err;

	item_info++;
	item_info->name		= cpu_to_le32(TOC0_ITEM_INFO_NAME_CERT);
	item_info->offset	= cpu_to_le32(item_offset);
	item_info->length	= cpu_to_le32(item_length);
	memcpy(item_info->end, TOC0_ITEM_INFO_END, sizeof(item_info->end));

	/* The third item contains the actual boot code. */
	item_offset = ALIGN(item_offset + item_length, 32);
	item_length = fw_item_len;
	if (buf + item_offset != fw_item)
		memmove(buf + item_offset, fw_item, item_length);

	item_info++;
	item_info->name		= cpu_to_le32(TOC0_ITEM_INFO_NAME_FIRMWARE);
	item_info->offset	= cpu_to_le32(item_offset);
	item_info->length	= cpu_to_le32(item_length);
	item_info->load_addr	= cpu_to_le32(fw_addr);
	memcpy(item_info->end, TOC0_ITEM_INFO_END, sizeof(item_info->end));

	/* Pad to the required block size with 0xff to be flash-friendly. */
	item_offset = item_offset + item_length;
	item_length = ALIGN(item_offset, PAD_SIZE) - item_offset;
	memset(buf + item_offset, 0xff, item_length);

	/* Fill in the total padded file length. */
	item_offset = item_offset + item_length;
	main_info->length = cpu_to_le32(item_offset);

	/* Verify enough space was provided when creating the image. */
	assert(len >= item_offset);

	/* Calculate the checksum. Yes, it's that simple. */
	for (i = 0; i < item_offset / 4; ++i)
		checksum += le32_to_cpu(buf32[i]);
	main_info->checksum = cpu_to_le32(checksum);

	ret = EXIT_SUCCESS;

err:
	if (fw_key != orig_fw_key)
		RSA_free(fw_key);

	return ret;
}

static const struct toc0_item_info *
toc0_find_item(const struct toc0_main_info *main_info, uint32_t name,
	       uint32_t *offset, uint32_t *length)
{
	const struct toc0_item_info *item_info = (void *)(main_info + 1);
	uint32_t item_offset, item_length;
	uint32_t num_items, main_length;
	int i;

	num_items   = le32_to_cpu(main_info->num_items);
	main_length = le32_to_cpu(main_info->length);

	for (i = 0; i < num_items; ++i, ++item_info) {
		if (le32_to_cpu(item_info->name) != name)
			continue;

		item_offset = le32_to_cpu(item_info->offset);
		item_length = le32_to_cpu(item_info->length);

		if (item_offset > main_length ||
		    item_length > main_length - item_offset)
			continue;

		*offset = item_offset;
		*length = item_length;

		return item_info;
	}

	return NULL;
}

static int toc0_verify(const uint8_t *buf, uint32_t len, RSA *root_key)
{
	const struct toc0_main_info *main_info = (void *)buf;
	const struct toc0_item_info *item_info;
	uint8_t digest[SHA256_DIGEST_LENGTH];
	uint32_t main_length = le32_to_cpu(main_info->length);
	uint32_t checksum = BROM_STAMP_VALUE;
	uint32_t *buf32 = (void *)buf;
	uint32_t length, offset;
	int ret = EXIT_FAILURE;
	RSA *fw_key = NULL;
	int i;

	if (len < main_length)
		goto err;

	/* Verify the main header. */
	if (memcmp(main_info->name, TOC0_MAIN_INFO_NAME, sizeof(main_info->name)))
		goto err;
	if (le32_to_cpu(main_info->magic) != TOC0_MAIN_INFO_MAGIC)
		goto err;
	/* Verify the checksum without modifying the buffer. */
	for (i = 0; i < main_length / 4; ++i)
		checksum += le32_to_cpu(buf32[i]);
	if (checksum != 2 * le32_to_cpu(main_info->checksum))
		goto err;
	/* The length must be at least 512 byte aligned. */
	if (main_length % 512)
		goto err;
	if (memcmp(main_info->end, TOC0_MAIN_INFO_END, sizeof(main_info->end)))
		goto err;

	/* Verify the key item if present (it is optional). */
	item_info = toc0_find_item(main_info, TOC0_ITEM_INFO_NAME_KEY,
				   &offset, &length);
	if (!item_info)
		fw_key = root_key;
	else if (toc0_verify_key_item(buf + offset, length, root_key, &fw_key))
		goto err;

	/* Hash the firmware to compare with the certificate. */
	item_info = toc0_find_item(main_info, TOC0_ITEM_INFO_NAME_FIRMWARE,
				   &offset, &length);
	if (!item_info) {
		pr_err("Missing firmware item\n");
		goto err;
	}
	SHA256(buf + offset, length, digest);

	/* Verify the certificate item. */
	item_info = toc0_find_item(main_info, TOC0_ITEM_INFO_NAME_CERT,
				   &offset, &length);
	if (!item_info) {
		pr_err("Missing certificate item\n");
		goto err;
	}
	if (toc0_verify_cert_item(buf + offset, length, fw_key, digest))
		goto err;

	ret = EXIT_SUCCESS;

err:
	if (fw_key != root_key)
		RSA_free(fw_key);

	return ret;
}

static int toc0_check_params(struct image_tool_params *params)
{
	if (!params->dflag)
		return -EINVAL;

	/*
	 * If a key directory was provided, look for key files there.
	 * Otherwise, look for them in the current directory. The key files are
	 * the "quoted" terms in the description below.
	 *
	 * A summary of the chain of trust on most SoCs:
	 *  1) eFuse contains a SHA256 digest of the public "root key".
	 *  2) Private "root key" signs the certificate item (generated here).
	 *  3) Certificate item contains a SHA256 digest of the firmware item.
	 *
	 * A summary of the chain of trust on the H6 (by default; a bit in the
	 * BROM_CONFIG eFuse makes it work like above):
	 *  1) eFuse contains a SHA256 digest of the public "root key".
	 *  2) Private "root key" signs the "key item" (generated here).
	 *  3) "Key item" contains the public "root key" and public "fw key".
	 *  4) Private "fw key" signs the certificate item (generated here).
	 *  5) Certificate item contains a SHA256 digest of the firmware item.
	 *
	 * This means there are three valid ways to generate a TOC0:
	 *  1) Provide the private "root key" only. This works everywhere.
	 *     For H6, the "root key" will also be used as the "fw key".
	 *  2) FOR H6 ONLY: Provide the private "root key" and a separate
	 *     private "fw key".
	 *  3) FOR H6 ONLY: Provide the private "fw key" and a pre-existing
	 *     "key item" containing the corresponding  public "fw key".
	 *     In this case, the private "root key" can be kept offline. The
	 *     "key item" can be extracted from a TOC0 image generated using
	 *     method #2 above.
	 *
	 *  Note that until the ROTPK_HASH eFuse is programmed, any "root key"
	 *  will be accepted by the BROM.
	 */
	if (params->keydir) {
		if (asprintf(&fw_key_file, "%s/%s", params->keydir, fw_key_file) < 0)
			return -ENOMEM;
		if (asprintf(&key_item_file, "%s/%s", params->keydir, key_item_file) < 0)
			return -ENOMEM;
		if (asprintf(&root_key_file, "%s/%s", params->keydir, root_key_file) < 0)
			return -ENOMEM;
	}

	return 0;
}

static int toc0_verify_header(unsigned char *buf, int image_size,
			      struct image_tool_params *params)
{
	int ret = EXIT_FAILURE;
	RSA *root_key = NULL;
	FILE *fp;

	/* A root public key is optional. */
	fp = fopen(root_key_file, "rb");
	if (fp) {
		pr_info("Verifying image with existing root key\n");
		root_key = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL);
		if (!root_key)
			root_key = PEM_read_RSAPublicKey(fp, NULL, NULL, NULL);
		fclose(fp);
		if (!root_key) {
			pr_err("Failed to read public key from '%s'\n",
			       root_key_file);
			goto err;
		}
	}

	ret = toc0_verify(buf, image_size, root_key);

err:
	RSA_free(root_key);

	return ret;
}

static const char *toc0_item_name(uint32_t name)
{
	if (name == TOC0_ITEM_INFO_NAME_CERT)
		return "Certificate";
	if (name == TOC0_ITEM_INFO_NAME_FIRMWARE)
		return "Firmware";
	if (name == TOC0_ITEM_INFO_NAME_KEY)
		return "Key";
	return "(unknown)";
}

static void toc0_print_header(const void *buf, struct image_tool_params *params)
{
	const struct toc0_main_info *main_info = buf;
	const struct toc0_item_info *item_info = (void *)(main_info + 1);
	uint32_t head_length, main_length, num_items;
	uint32_t item_offset, item_length, item_name;
	int load_addr = -1;
	int i;

	num_items   = le32_to_cpu(main_info->num_items);
	head_length = sizeof(*main_info) + num_items * sizeof(*item_info);
	main_length = le32_to_cpu(main_info->length);

	printf("Allwinner TOC0 Image\n"
	       "Size: %d bytes\n"
	       "Contents: %d items\n"
	       " 00000000:%08x Headers\n",
	       main_length, num_items, head_length);

	for (i = 0; i < num_items; ++i, ++item_info) {
		item_offset = le32_to_cpu(item_info->offset);
		item_length = le32_to_cpu(item_info->length);
		item_name   = le32_to_cpu(item_info->name);

		if (item_name == TOC0_ITEM_INFO_NAME_FIRMWARE)
			load_addr = le32_to_cpu(item_info->load_addr);

		printf(" %08x:%08x %s\n",
		       item_offset, item_length,
		       toc0_item_name(item_name));
	}

	if (num_items && item_offset + item_length < main_length) {
		item_offset = item_offset + item_length;
		item_length = main_length - item_offset;

		printf(" %08x:%08x Padding\n",
		       item_offset, item_length);
	}

	if (load_addr != -1)
		printf("Load address: 0x%08x\n", load_addr);
}

static void toc0_set_header(void *buf, struct stat *sbuf, int ifd,
			    struct image_tool_params *params)
{
	uint32_t key_item_len = 0;
	uint8_t *key_item = NULL;
	int ret = EXIT_FAILURE;
	RSA *root_key = NULL;
	RSA *fw_key = NULL;
	FILE *fp;

	/* Either a key item or the root private key is required. */
	fp = fopen(key_item_file, "rb");
	if (fp) {
		pr_info("Creating image using existing key item\n");
		key_item_len = sizeof(struct toc0_key_item);
		key_item = OPENSSL_malloc(key_item_len);
		if (!key_item || fread(key_item, key_item_len, 1, fp) != 1) {
			pr_err("Failed to read key item from '%s'\n",
			       root_key_file);
			goto err;
		}
		fclose(fp);
		fp = NULL;
	}

	fp = fopen(root_key_file, "rb");
	if (fp) {
		root_key = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL);
		if (!root_key)
			root_key = PEM_read_RSAPublicKey(fp, NULL, NULL, NULL);
		fclose(fp);
		fp = NULL;
	}

	/* When using an existing key item, the root key is optional. */
	if (!key_item && (!root_key || !RSA_get0_d(root_key))) {
		pr_err("Failed to read private key from '%s'\n",
		       root_key_file);
		pr_info("Try 'openssl genrsa -out root_key.pem'\n");
		goto err;
	}

	/* The certificate/firmware private key is always required. */
	fp = fopen(fw_key_file, "rb");
	if (fp) {
		fw_key = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL);
		fclose(fp);
		fp = NULL;
	}
	if (!fw_key) {
		/* If the root key is a private key, it can be used instead. */
		if (root_key && RSA_get0_d(root_key)) {
			pr_info("Using root key as firmware key\n");
			fw_key = root_key;
		} else {
			pr_err("Failed to read private key from '%s'\n",
			       fw_key_file);
			goto err;
		}
	}

	/* Warn about potential compatibility issues. */
	if (key_item || fw_key != root_key)
		pr_warn("Only H6 supports separate root and firmware keys\n");

	ret = toc0_create(buf, params->file_size, root_key, fw_key,
			  key_item, key_item_len,
			  buf + TOC0_DEFAULT_HEADER_LEN,
			  params->orig_file_size, params->addr);

err:
	OPENSSL_free(key_item);
	OPENSSL_free(root_key);
	if (fw_key != root_key)
		OPENSSL_free(fw_key);
	if (fp)
		fclose(fp);

	if (ret != EXIT_SUCCESS)
		exit(ret);
}

static int toc0_check_image_type(uint8_t type)
{
	return type == IH_TYPE_SUNXI_TOC0 ? 0 : 1;
}

static int toc0_vrec_header(struct image_tool_params *params,
			    struct image_type_params *tparams)
{
	tparams->hdr = calloc(tparams->header_size, 1);

	/* Save off the unpadded data size for SHA256 calculation. */
	params->orig_file_size = params->file_size - TOC0_DEFAULT_HEADER_LEN;

	/* Return padding to 8K blocks. */
	return ALIGN(params->file_size, PAD_SIZE) - params->file_size;
}

U_BOOT_IMAGE_TYPE(
	sunxi_toc0,
	"Allwinner TOC0 Boot Image support",
	TOC0_DEFAULT_HEADER_LEN,
	NULL,
	toc0_check_params,
	toc0_verify_header,
	toc0_print_header,
	toc0_set_header,
	NULL,
	toc0_check_image_type,
	NULL,
	toc0_vrec_header
);
