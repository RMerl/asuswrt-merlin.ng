// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013, Google Inc.
 */

#define OPENSSL_API_COMPAT 0x10101000L

#include "mkimage.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <image.h>
#include <time.h>
#include <u-boot/fdt-libcrypto.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/engine.h>

static int rsa_err(const char *msg)
{
	unsigned long sslErr = ERR_get_error();

	fprintf(stderr, "%s", msg);
	fprintf(stderr, ": %s\n",
		ERR_error_string(sslErr, 0));

	return -1;
}

/**
 * rsa_pem_get_pub_key() - read a public key from a .crt file
 *
 * @keydir:	Directory containins the key
 * @name	Name of key file (will have a .crt extension)
 * @evpp	Returns EVP_PKEY object, or NULL on failure
 * Return: 0 if ok, -ve on error (in which case *evpp will be set to NULL)
 */
static int rsa_pem_get_pub_key(const char *keydir, const char *name, EVP_PKEY **evpp)
{
	char path[1024];
	EVP_PKEY *key = NULL;
	X509 *cert;
	FILE *f;
	int ret;

	if (!evpp)
		return -EINVAL;

	*evpp = NULL;
	snprintf(path, sizeof(path), "%s/%s.crt", keydir, name);
	f = fopen(path, "r");
	if (!f) {
		fprintf(stderr, "Couldn't open RSA certificate: '%s': %s\n",
			path, strerror(errno));
		return -EACCES;
	}

	/* Read the certificate */
	cert = NULL;
	if (!PEM_read_X509(f, &cert, NULL, NULL)) {
		rsa_err("Couldn't read certificate");
		ret = -EINVAL;
		goto err_cert;
	}

	/* Get the public key from the certificate. */
	key = X509_get_pubkey(cert);
	if (!key) {
		rsa_err("Couldn't read public key\n");
		ret = -EINVAL;
		goto err_pubkey;
	}

	fclose(f);
	*evpp = key;
	X509_free(cert);

	return 0;

err_pubkey:
	X509_free(cert);
err_cert:
	fclose(f);
	return ret;
}

/**
 * rsa_engine_get_pub_key() - read a public key from given engine
 *
 * @keydir:	Key prefix
 * @name	Name of key
 * @engine	Engine to use
 * @evpp	Returns EVP_PKEY object, or NULL on failure
 * Return: 0 if ok, -ve on error (in which case *evpp will be set to NULL)
 */
static int rsa_engine_get_pub_key(const char *keydir, const char *name,
				  ENGINE *engine, EVP_PKEY **evpp)
{
	const char *engine_id;
	char key_id[1024];
	EVP_PKEY *key = NULL;
	const char *const pkcs11_schema = "pkcs11:";
	const char *pkcs11_uri_prepend = "";

	if (!evpp)
		return -EINVAL;

	*evpp = NULL;

	engine_id = ENGINE_get_id(engine);

	if (engine_id && !strcmp(engine_id, "pkcs11")) {
		if (keydir) {
			// Check for legacy keydir spec and prepend
			if (strncmp(pkcs11_schema, keydir, strlen(pkcs11_schema))) {
				pkcs11_uri_prepend = pkcs11_schema;
				fprintf(stderr, "WARNING: Legacy URI specified. Please add '%s'.\n", pkcs11_schema);
			}

			if (strstr(keydir, "object="))
				snprintf(key_id, sizeof(key_id),
					 "%s%s;type=public",
					 pkcs11_uri_prepend, keydir);
			else
				snprintf(key_id, sizeof(key_id),
					 "%s%s;object=%s;type=public",
					 pkcs11_uri_prepend, keydir, name);
		} else {
			snprintf(key_id, sizeof(key_id),
				 "pkcs11:object=%s;type=public",
				 name);
		}
	} else if (engine_id) {
		if (keydir)
			snprintf(key_id, sizeof(key_id),
				 "%s%s",
				 keydir, name);
		else
			snprintf(key_id, sizeof(key_id),
				 "%s",
				 name);
	} else {
		fprintf(stderr, "Engine not supported\n");
		return -ENOTSUP;
	}

	key = ENGINE_load_public_key(engine, key_id, NULL, NULL);
	if (!key)
		return rsa_err("Failure loading public key from engine");

	*evpp = key;

	return 0;
}

/**
 * rsa_get_pub_key() - read a public key
 *
 * @keydir:	Directory containing the key (PEM file) or key prefix (engine)
 * @name	Name of key file (will have a .crt extension)
 * @engine	Engine to use
 * @evpp	Returns EVP_PKEY object, or NULL on failure
 * Return: 0 if ok, -ve on error (in which case *evpp will be set to NULL)
 */
static int rsa_get_pub_key(const char *keydir, const char *name,
			   ENGINE *engine, EVP_PKEY **evpp)
{
	if (engine)
		return rsa_engine_get_pub_key(keydir, name, engine, evpp);
	return rsa_pem_get_pub_key(keydir, name, evpp);
}

/**
 * rsa_pem_get_priv_key() - read a private key from a .key file
 *
 * @keydir:	Directory containing the key
 * @name	Name of key file (will have a .key extension)
 * @evpp	Returns EVP_PKEY object, or NULL on failure
 * Return: 0 if ok, -ve on error (in which case *evpp will be set to NULL)
 */
static int rsa_pem_get_priv_key(const char *keydir, const char *name,
				const char *keyfile, EVP_PKEY **evpp)
{
	char path[1024] = {0};
	FILE *f = NULL;

	if (!evpp)
		return -EINVAL;

	*evpp = NULL;
	if (keydir && name)
		snprintf(path, sizeof(path), "%s/%s.key", keydir, name);
	else if (keyfile)
		snprintf(path, sizeof(path), "%s", keyfile);
	else
		return -EINVAL;

	f = fopen(path, "r");
	if (!f) {
		fprintf(stderr, "Couldn't open RSA private key: '%s': %s\n",
			path, strerror(errno));
		return -ENOENT;
	}

	if (!PEM_read_PrivateKey(f, evpp, NULL, path)) {
		rsa_err("Failure reading private key");
		fclose(f);
		return -EPROTO;
	}
	fclose(f);

	return 0;
}

/**
 * rsa_engine_get_priv_key() - read a private key from given engine
 *
 * @keydir:	Key prefix
 * @name	Name of key
 * @engine	Engine to use
 * @evpp	Returns EVP_PKEY object, or NULL on failure
 * Return: 0 if ok, -ve on error (in which case *evpp will be set to NULL)
 */
static int rsa_engine_get_priv_key(const char *keydir, const char *name,
				   const char *keyfile,
				   ENGINE *engine, EVP_PKEY **evpp)
{
	const char *engine_id;
	char key_id[1024];
	EVP_PKEY *key = NULL;
	const char *const pkcs11_schema = "pkcs11:";
	const char *pkcs11_uri_prepend = "";

	if (!evpp)
		return -EINVAL;

	engine_id = ENGINE_get_id(engine);

	if (engine_id && !strcmp(engine_id, "pkcs11")) {
		if (!keydir && !name) {
			fprintf(stderr, "Please use 'keydir' with PKCS11\n");
			return -EINVAL;
		}
		if (keydir) {
			// Check for legacy keydir spec and prepend
			if (strncmp(pkcs11_schema, keydir, strlen(pkcs11_schema))) {
				pkcs11_uri_prepend = pkcs11_schema;
				fprintf(stderr, "WARNING: Legacy URI specified. Please add '%s'.\n", pkcs11_schema);
			}

			if (strstr(keydir, "object="))
				snprintf(key_id, sizeof(key_id),
					 "%s%s;type=private",
					 pkcs11_uri_prepend, keydir);
			else
				snprintf(key_id, sizeof(key_id),
					 "%s%s;object=%s;type=private",
					 pkcs11_uri_prepend, keydir, name);
		} else {
			snprintf(key_id, sizeof(key_id),
				 "pkcs11:object=%s;type=private",
				 name);
		}
	} else if (engine_id) {
		if (keydir && name)
			snprintf(key_id, sizeof(key_id),
				 "%s%s",
				 keydir, name);
		else if (name)
			snprintf(key_id, sizeof(key_id),
				 "%s",
				 name ? name : "");
		else if (keyfile)
			snprintf(key_id, sizeof(key_id), "%s", keyfile);
		else
			return -EINVAL;

	} else {
		fprintf(stderr, "Engine not supported\n");
		return -ENOTSUP;
	}

	key = ENGINE_load_private_key(engine, key_id, NULL, NULL);
	if (!key)
		return rsa_err("Failure loading private key from engine");

	*evpp = key;

	return 0;
}

/**
 * rsa_get_priv_key() - read a private key
 *
 * @keydir:	Directory containing the key (PEM file) or key prefix (engine)
 * @name	Name of key
 * @engine	Engine to use for signing
 * @evpp	Returns EVP_PKEY object, or NULL on failure
 * Return: 0 if ok, -ve on error (in which case *evpp will be set to NULL)
 */
static int rsa_get_priv_key(const char *keydir, const char *name,
			    const char *keyfile, ENGINE *engine, EVP_PKEY **evpp)
{
	if (engine)
		return rsa_engine_get_priv_key(keydir, name, keyfile, engine,
					       evpp);
	return rsa_pem_get_priv_key(keydir, name, keyfile, evpp);
}

static int rsa_init(void)
{
	int ret;

	ret = OPENSSL_init_ssl(0, NULL);
	if (!ret) {
		fprintf(stderr, "Failure to init SSL library\n");
		return -1;
	}

	return 0;
}

static int rsa_engine_init(const char *engine_id, ENGINE **pe)
{
	const char *key_pass;
	ENGINE *e;
	int ret;

	ENGINE_load_builtin_engines();

	e = ENGINE_by_id(engine_id);
	if (!e) {
		fprintf(stderr, "Engine '%s' isn't available\n", engine_id);
		ERR_print_errors_fp(stderr);
		return -1;
	}

	if (!ENGINE_init(e)) {
		fprintf(stderr, "Couldn't initialize engine\n");
		ret = -1;
		goto err_engine_init;
	}

	if (!ENGINE_set_default_RSA(e)) {
		fprintf(stderr, "Couldn't set engine as default for RSA\n");
		ret = -1;
		goto err_set_rsa;
	}

	key_pass = getenv("MKIMAGE_SIGN_PIN");
	if (key_pass) {
		if (!ENGINE_ctrl_cmd_string(e, "PIN", key_pass, 0)) {
			fprintf(stderr, "Couldn't set PIN\n");
			ret = -1;
			goto err_set_pin;
		}
	}

	*pe = e;

	return 0;

err_set_pin:
err_set_rsa:
	ENGINE_finish(e);
err_engine_init:
	ENGINE_free(e);
	return ret;
}

static void rsa_engine_remove(ENGINE *e)
{
	if (e) {
		ENGINE_finish(e);
		ENGINE_free(e);
	}
}

static int rsa_sign_with_key(EVP_PKEY *pkey, struct padding_algo *padding_algo,
			     struct checksum_algo *checksum_algo,
		const struct image_region region[], int region_count,
		uint8_t **sigp, uint *sig_size)
{
	EVP_PKEY_CTX *ckey;
	EVP_MD_CTX *context;
	int ret = 0;
	size_t size;
	uint8_t *sig;
	int i;

	size = EVP_PKEY_size(pkey);
	sig = malloc(size);
	if (!sig) {
		fprintf(stderr, "Out of memory for signature (%zu bytes)\n",
			size);
		ret = -ENOMEM;
		goto err_alloc;
	}

	context = EVP_MD_CTX_new();
	if (!context) {
		ret = rsa_err("EVP context creation failed");
		goto err_create;
	}

	ckey = EVP_PKEY_CTX_new(pkey, NULL);
	if (!ckey) {
		ret = rsa_err("EVP key context creation failed");
		goto err_create;
	}

	if (EVP_DigestSignInit(context, &ckey,
			       checksum_algo->calculate_sign(),
			       NULL, pkey) <= 0) {
		ret = rsa_err("Signer setup failed");
		goto err_sign;
	}

	if (CONFIG_IS_ENABLED(FIT_RSASSA_PSS) && padding_algo &&
	    !strcmp(padding_algo->name, "pss")) {
		if (EVP_PKEY_CTX_set_rsa_padding(ckey,
						 RSA_PKCS1_PSS_PADDING) <= 0) {
			ret = rsa_err("Signer padding setup failed");
			goto err_sign;
		}
	}

	for (i = 0; i < region_count; i++) {
		if (!EVP_DigestSignUpdate(context, region[i].data,
					  region[i].size)) {
			ret = rsa_err("Signing data failed");
			goto err_sign;
		}
	}

	if (!EVP_DigestSignFinal(context, sig, &size)) {
		ret = rsa_err("Could not obtain signature");
		goto err_sign;
	}

	EVP_MD_CTX_free(context);

	debug("Got signature: %zu bytes, expected %d\n", size, EVP_PKEY_size(pkey));
	*sigp = sig;
	*sig_size = size;

	return 0;

err_sign:
	EVP_MD_CTX_free(context);
err_create:
	free(sig);
err_alloc:
	return ret;
}

int rsa_sign(struct image_sign_info *info,
	     const struct image_region region[], int region_count,
	     uint8_t **sigp, uint *sig_len)
{
	EVP_PKEY *pkey = NULL;
	ENGINE *e = NULL;
	int ret;

	ret = rsa_init();
	if (ret)
		return ret;

	if (info->engine_id) {
		ret = rsa_engine_init(info->engine_id, &e);
		if (ret)
			return ret;
	}

	ret = rsa_get_priv_key(info->keydir, info->keyname, info->keyfile,
			       e, &pkey);
	if (ret)
		goto err_priv;
	ret = rsa_sign_with_key(pkey, info->padding, info->checksum, region,
				region_count, sigp, sig_len);
	if (ret)
		goto err_sign;

	EVP_PKEY_free(pkey);
	if (info->engine_id)
		rsa_engine_remove(e);

	return ret;

err_sign:
	EVP_PKEY_free(pkey);
err_priv:
	if (info->engine_id)
		rsa_engine_remove(e);
	return ret;
}

/*
 * rsa_get_exponent(): - Get the public exponent from an RSA key
 */
static int rsa_get_exponent(RSA *key, uint64_t *e)
{
	int ret;
	BIGNUM *bn_te;
	const BIGNUM *key_e;
	uint64_t te;

	ret = -EINVAL;
	bn_te = NULL;

	if (!e)
		goto cleanup;

	RSA_get0_key(key, NULL, &key_e, NULL);
	if (BN_num_bits(key_e) > 64)
		goto cleanup;

	*e = BN_get_word(key_e);

	if (BN_num_bits(key_e) < 33) {
		ret = 0;
		goto cleanup;
	}

	bn_te = BN_dup(key_e);
	if (!bn_te)
		goto cleanup;

	if (!BN_rshift(bn_te, bn_te, 32))
		goto cleanup;

	if (!BN_mask_bits(bn_te, 32))
		goto cleanup;

	te = BN_get_word(bn_te);
	te <<= 32;
	*e |= te;
	ret = 0;

cleanup:
	if (bn_te)
		BN_free(bn_te);

	return ret;
}

/*
 * rsa_get_params(): - Get the important parameters of an RSA public key
 */
int rsa_get_params(RSA *key, uint64_t *exponent, uint32_t *n0_invp,
		   BIGNUM **modulusp, BIGNUM **r_squaredp)
{
	BIGNUM *big1, *big2, *big32, *big2_32;
	BIGNUM *n, *r, *r_squared, *tmp;
	const BIGNUM *key_n;
	BN_CTX *bn_ctx = BN_CTX_new();
	int ret = 0;

	/* Initialize BIGNUMs */
	big1 = BN_new();
	big2 = BN_new();
	big32 = BN_new();
	r = BN_new();
	r_squared = BN_new();
	tmp = BN_new();
	big2_32 = BN_new();
	n = BN_new();
	if (!big1 || !big2 || !big32 || !r || !r_squared || !tmp || !big2_32 ||
	    !n) {
		fprintf(stderr, "Out of memory (bignum)\n");
		return -ENOMEM;
	}

	if (0 != rsa_get_exponent(key, exponent))
		ret = -1;

	RSA_get0_key(key, &key_n, NULL, NULL);
	if (!BN_copy(n, key_n) || !BN_set_word(big1, 1L) ||
	    !BN_set_word(big2, 2L) || !BN_set_word(big32, 32L))
		ret = -1;

	/* big2_32 = 2^32 */
	if (!BN_exp(big2_32, big2, big32, bn_ctx))
		ret = -1;

	/* Calculate n0_inv = -1 / n[0] mod 2^32 */
	if (!BN_mod_inverse(tmp, n, big2_32, bn_ctx) ||
	    !BN_sub(tmp, big2_32, tmp))
		ret = -1;
	*n0_invp = BN_get_word(tmp);

	/* Calculate R = 2^(# of key bits) */
	if (!BN_set_word(tmp, BN_num_bits(n)) ||
	    !BN_exp(r, big2, tmp, bn_ctx))
		ret = -1;

	/* Calculate r_squared = R^2 mod n */
	if (!BN_copy(r_squared, r) ||
	    !BN_mul(tmp, r_squared, r, bn_ctx) ||
	    !BN_mod(r_squared, tmp, n, bn_ctx))
		ret = -1;

	*modulusp = n;
	*r_squaredp = r_squared;

	BN_free(big1);
	BN_free(big2);
	BN_free(big32);
	BN_free(r);
	BN_free(tmp);
	BN_free(big2_32);
	if (ret) {
		fprintf(stderr, "Bignum operations failed\n");
		return -ENOMEM;
	}

	return ret;
}

int rsa_add_verify_data(struct image_sign_info *info, void *keydest)
{
	BIGNUM *modulus, *r_squared;
	uint64_t exponent;
	uint32_t n0_inv;
	int parent, node = -FDT_ERR_NOTFOUND;
	char name[100];
	int ret;
	int bits;
	RSA *rsa;
	EVP_PKEY *pkey = NULL;
	ENGINE *e = NULL;

	debug("%s: Getting verification data\n", __func__);
	if (info->engine_id) {
		ret = rsa_engine_init(info->engine_id, &e);
		if (ret)
			return ret;
	}
	ret = rsa_get_pub_key(info->keydir, info->keyname, e, &pkey);
	if (ret)
		goto err_get_pub_key;

	rsa = (RSA *)EVP_PKEY_get0_RSA(pkey);
	ret = rsa_get_params(rsa, &exponent, &n0_inv, &modulus, &r_squared);
	if (ret)
		goto err_get_params;
	bits = BN_num_bits(modulus);
	parent = fdt_subnode_offset(keydest, 0, FIT_SIG_NODENAME);
	if (parent == -FDT_ERR_NOTFOUND) {
		parent = fdt_add_subnode(keydest, 0, FIT_SIG_NODENAME);
		if (parent < 0) {
			ret = parent;
			if (ret != -FDT_ERR_NOSPACE) {
				fprintf(stderr, "Couldn't create signature node: %s\n",
					fdt_strerror(parent));
			}
		}
	}
	if (ret)
		goto done;

	/* Either create or overwrite the named key node */
	snprintf(name, sizeof(name), "key-%s", info->keyname);
	node = fdt_subnode_offset(keydest, parent, name);
	if (node == -FDT_ERR_NOTFOUND) {
		node = fdt_add_subnode(keydest, parent, name);
		if (node < 0) {
			ret = node;
			if (ret != -FDT_ERR_NOSPACE) {
				fprintf(stderr, "Could not create key subnode: %s\n",
					fdt_strerror(node));
			}
		}
	} else if (node < 0) {
		fprintf(stderr, "Cannot select keys parent: %s\n",
			fdt_strerror(node));
		ret = node;
	}

	if (!ret) {
		ret = fdt_setprop_string(keydest, node, FIT_KEY_HINT,
					 info->keyname);
	}
	if (!ret)
		ret = fdt_setprop_u32(keydest, node, "rsa,num-bits", bits);
	if (!ret)
		ret = fdt_setprop_u32(keydest, node, "rsa,n0-inverse", n0_inv);
	if (!ret) {
		ret = fdt_setprop_u64(keydest, node, "rsa,exponent", exponent);
	}
	if (!ret) {
		ret = fdt_add_bignum(keydest, node, "rsa,modulus", modulus,
				     bits);
	}
	if (!ret) {
		ret = fdt_add_bignum(keydest, node, "rsa,r-squared", r_squared,
				     bits);
	}
	if (!ret) {
		ret = fdt_setprop_string(keydest, node, FIT_ALGO_PROP,
					 info->name);
	}
	if (!ret && info->require_keys) {
		ret = fdt_setprop_string(keydest, node, FIT_KEY_REQUIRED,
					 info->require_keys);
	}
done:
	BN_free(modulus);
	BN_free(r_squared);
	if (ret)
		ret = ret == -FDT_ERR_NOSPACE ? -ENOSPC : -EIO;
err_get_params:
	EVP_PKEY_free(pkey);
err_get_pub_key:
	if (info->engine_id)
		rsa_engine_remove(e);

	if (ret)
		return ret;

	return node;
}
