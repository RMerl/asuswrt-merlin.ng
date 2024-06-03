// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013, Google Inc.
 */

#include "mkimage.h"
#include <stdio.h>
#include <string.h>
#include <image.h>
#include <time.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/engine.h>

#if OPENSSL_VERSION_NUMBER >= 0x10000000L
#define HAVE_ERR_REMOVE_THREAD_STATE
#endif

#if OPENSSL_VERSION_NUMBER < 0x10100000L || \
	(defined(LIBRESSL_VERSION_NUMBER) && LIBRESSL_VERSION_NUMBER < 0x02070000fL)
static void RSA_get0_key(const RSA *r,
                 const BIGNUM **n, const BIGNUM **e, const BIGNUM **d)
{
   if (n != NULL)
       *n = r->n;
   if (e != NULL)
       *e = r->e;
   if (d != NULL)
       *d = r->d;
}
#endif

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
 * @rsap	Returns RSA object, or NULL on failure
 * @return 0 if ok, -ve on error (in which case *rsap will be set to NULL)
 */
static int rsa_pem_get_pub_key(const char *keydir, const char *name, RSA **rsap)
{
	char path[1024];
	EVP_PKEY *key;
	X509 *cert;
	RSA *rsa;
	FILE *f;
	int ret;

	*rsap = NULL;
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

	/* Convert to a RSA_style key. */
	rsa = EVP_PKEY_get1_RSA(key);
	if (!rsa) {
		rsa_err("Couldn't convert to a RSA style key");
		ret = -EINVAL;
		goto err_rsa;
	}
	fclose(f);
	EVP_PKEY_free(key);
	X509_free(cert);
	*rsap = rsa;

	return 0;

err_rsa:
	EVP_PKEY_free(key);
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
 * @rsap	Returns RSA object, or NULL on failure
 * @return 0 if ok, -ve on error (in which case *rsap will be set to NULL)
 */
static int rsa_engine_get_pub_key(const char *keydir, const char *name,
				  ENGINE *engine, RSA **rsap)
{
	const char *engine_id;
	char key_id[1024];
	EVP_PKEY *key;
	RSA *rsa;
	int ret;

	*rsap = NULL;

	engine_id = ENGINE_get_id(engine);

	if (engine_id && !strcmp(engine_id, "pkcs11")) {
		if (keydir)
			snprintf(key_id, sizeof(key_id),
				 "pkcs11:%s;object=%s;type=public",
				 keydir, name);
		else
			snprintf(key_id, sizeof(key_id),
				 "pkcs11:object=%s;type=public",
				 name);
	} else {
		fprintf(stderr, "Engine not supported\n");
		return -ENOTSUP;
	}

	key = ENGINE_load_public_key(engine, key_id, NULL, NULL);
	if (!key)
		return rsa_err("Failure loading public key from engine");

	/* Convert to a RSA_style key. */
	rsa = EVP_PKEY_get1_RSA(key);
	if (!rsa) {
		rsa_err("Couldn't convert to a RSA style key");
		ret = -EINVAL;
		goto err_rsa;
	}

	EVP_PKEY_free(key);
	*rsap = rsa;

	return 0;

err_rsa:
	EVP_PKEY_free(key);
	return ret;
}

/**
 * rsa_get_pub_key() - read a public key
 *
 * @keydir:	Directory containing the key (PEM file) or key prefix (engine)
 * @name	Name of key file (will have a .crt extension)
 * @engine	Engine to use
 * @rsap	Returns RSA object, or NULL on failure
 * @return 0 if ok, -ve on error (in which case *rsap will be set to NULL)
 */
static int rsa_get_pub_key(const char *keydir, const char *name,
			   ENGINE *engine, RSA **rsap)
{
	if (engine)
		return rsa_engine_get_pub_key(keydir, name, engine, rsap);
	return rsa_pem_get_pub_key(keydir, name, rsap);
}

/**
 * rsa_pem_get_priv_key() - read a private key from a .key file
 *
 * @keydir:	Directory containing the key
 * @name	Name of key file (will have a .key extension)
 * @rsap	Returns RSA object, or NULL on failure
 * @return 0 if ok, -ve on error (in which case *rsap will be set to NULL)
 */
static int rsa_pem_get_priv_key(const char *keydir, const char *name,
				RSA **rsap)
{
	char path[1024];
	RSA *rsa;
	FILE *f;

	*rsap = NULL;
	snprintf(path, sizeof(path), "%s/%s.key", keydir, name);
	f = fopen(path, "r");
	if (!f) {
		fprintf(stderr, "Couldn't open RSA private key: '%s': %s\n",
			path, strerror(errno));
		return -ENOENT;
	}

	rsa = PEM_read_RSAPrivateKey(f, 0, NULL, path);
	if (!rsa) {
		rsa_err("Failure reading private key");
		fclose(f);
		return -EPROTO;
	}
	fclose(f);
	*rsap = rsa;

	return 0;
}

/**
 * rsa_engine_get_priv_key() - read a private key from given engine
 *
 * @keydir:	Key prefix
 * @name	Name of key
 * @engine	Engine to use
 * @rsap	Returns RSA object, or NULL on failure
 * @return 0 if ok, -ve on error (in which case *rsap will be set to NULL)
 */
static int rsa_engine_get_priv_key(const char *keydir, const char *name,
				   ENGINE *engine, RSA **rsap)
{
	const char *engine_id;
	char key_id[1024];
	EVP_PKEY *key;
	RSA *rsa;
	int ret;

	*rsap = NULL;

	engine_id = ENGINE_get_id(engine);

	if (engine_id && !strcmp(engine_id, "pkcs11")) {
		if (keydir)
			snprintf(key_id, sizeof(key_id),
				 "pkcs11:%s;object=%s;type=private",
				 keydir, name);
		else
			snprintf(key_id, sizeof(key_id),
				 "pkcs11:object=%s;type=private",
				 name);
	} else {
		fprintf(stderr, "Engine not supported\n");
		return -ENOTSUP;
	}

	key = ENGINE_load_private_key(engine, key_id, NULL, NULL);
	if (!key)
		return rsa_err("Failure loading private key from engine");

	/* Convert to a RSA_style key. */
	rsa = EVP_PKEY_get1_RSA(key);
	if (!rsa) {
		rsa_err("Couldn't convert to a RSA style key");
		ret = -EINVAL;
		goto err_rsa;
	}

	EVP_PKEY_free(key);
	*rsap = rsa;

	return 0;

err_rsa:
	EVP_PKEY_free(key);
	return ret;
}

/**
 * rsa_get_priv_key() - read a private key
 *
 * @keydir:	Directory containing the key (PEM file) or key prefix (engine)
 * @name	Name of key
 * @engine	Engine to use for signing
 * @rsap	Returns RSA object, or NULL on failure
 * @return 0 if ok, -ve on error (in which case *rsap will be set to NULL)
 */
static int rsa_get_priv_key(const char *keydir, const char *name,
			    ENGINE *engine, RSA **rsap)
{
	if (engine)
		return rsa_engine_get_priv_key(keydir, name, engine, rsap);
	return rsa_pem_get_priv_key(keydir, name, rsap);
}

static int rsa_init(void)
{
	int ret;

#if OPENSSL_VERSION_NUMBER < 0x10100000L || \
	(defined(LIBRESSL_VERSION_NUMBER) && LIBRESSL_VERSION_NUMBER < 0x02070000fL)
	ret = SSL_library_init();
#else
	ret = OPENSSL_init_ssl(0, NULL);
#endif
	if (!ret) {
		fprintf(stderr, "Failure to init SSL library\n");
		return -1;
	}
#if OPENSSL_VERSION_NUMBER < 0x10100000L || \
	(defined(LIBRESSL_VERSION_NUMBER) && LIBRESSL_VERSION_NUMBER < 0x02070000fL)
	SSL_load_error_strings();

	OpenSSL_add_all_algorithms();
	OpenSSL_add_all_digests();
	OpenSSL_add_all_ciphers();
#endif

	return 0;
}

static int rsa_engine_init(const char *engine_id, ENGINE **pe)
{
	ENGINE *e;
	int ret;

	ENGINE_load_builtin_engines();

	e = ENGINE_by_id(engine_id);
	if (!e) {
		fprintf(stderr, "Engine isn't available\n");
		ret = -1;
		goto err_engine_by_id;
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

	*pe = e;

	return 0;

err_set_rsa:
	ENGINE_finish(e);
err_engine_init:
	ENGINE_free(e);
err_engine_by_id:
#if OPENSSL_VERSION_NUMBER < 0x10100000L || \
	(defined(LIBRESSL_VERSION_NUMBER) && LIBRESSL_VERSION_NUMBER < 0x02070000fL)
	ENGINE_cleanup();
#endif
	return ret;
}

static void rsa_remove(void)
{
#if OPENSSL_VERSION_NUMBER < 0x10100000L || \
	(defined(LIBRESSL_VERSION_NUMBER) && LIBRESSL_VERSION_NUMBER < 0x02070000fL)
	CRYPTO_cleanup_all_ex_data();
	ERR_free_strings();
#ifdef HAVE_ERR_REMOVE_THREAD_STATE
	ERR_remove_thread_state(NULL);
#else
	ERR_remove_state(0);
#endif
	EVP_cleanup();
#endif
}

static void rsa_engine_remove(ENGINE *e)
{
	if (e) {
		ENGINE_finish(e);
		ENGINE_free(e);
	}
}

static int rsa_sign_with_key(RSA *rsa, struct padding_algo *padding_algo,
			     struct checksum_algo *checksum_algo,
		const struct image_region region[], int region_count,
		uint8_t **sigp, uint *sig_size)
{
	EVP_PKEY *key;
	EVP_PKEY_CTX *ckey;
	EVP_MD_CTX *context;
	int ret = 0;
	size_t size;
	uint8_t *sig;
	int i;

	key = EVP_PKEY_new();
	if (!key)
		return rsa_err("EVP_PKEY object creation failed");

	if (!EVP_PKEY_set1_RSA(key, rsa)) {
		ret = rsa_err("EVP key setup failed");
		goto err_set;
	}

	size = EVP_PKEY_size(key);
	sig = malloc(size);
	if (!sig) {
		fprintf(stderr, "Out of memory for signature (%zu bytes)\n",
			size);
		ret = -ENOMEM;
		goto err_alloc;
	}

	context = EVP_MD_CTX_create();
	if (!context) {
		ret = rsa_err("EVP context creation failed");
		goto err_create;
	}
	EVP_MD_CTX_init(context);

	ckey = EVP_PKEY_CTX_new(key, NULL);
	if (!ckey) {
		ret = rsa_err("EVP key context creation failed");
		goto err_create;
	}

	if (EVP_DigestSignInit(context, &ckey,
			       checksum_algo->calculate_sign(),
			       NULL, key) <= 0) {
		ret = rsa_err("Signer setup failed");
		goto err_sign;
	}

#ifdef CONFIG_FIT_ENABLE_RSASSA_PSS_SUPPORT
	if (padding_algo && !strcmp(padding_algo->name, "pss")) {
		if (EVP_PKEY_CTX_set_rsa_padding(ckey,
						 RSA_PKCS1_PSS_PADDING) <= 0) {
			ret = rsa_err("Signer padding setup failed");
			goto err_sign;
		}
	}
#endif /* CONFIG_FIT_ENABLE_RSASSA_PSS_SUPPORT */

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

	#if OPENSSL_VERSION_NUMBER < 0x10100000L || \
		(defined(LIBRESSL_VERSION_NUMBER) && LIBRESSL_VERSION_NUMBER < 0x02070000fL)
		EVP_MD_CTX_cleanup(context);
	#else
		EVP_MD_CTX_reset(context);
	#endif
	EVP_MD_CTX_destroy(context);
	EVP_PKEY_free(key);

	debug("Got signature: %d bytes, expected %zu\n", *sig_size, size);
	*sigp = sig;
	*sig_size = size;

	return 0;

err_sign:
	EVP_MD_CTX_destroy(context);
err_create:
	free(sig);
err_alloc:
err_set:
	EVP_PKEY_free(key);
	return ret;
}

int rsa_sign(struct image_sign_info *info,
	     const struct image_region region[], int region_count,
	     uint8_t **sigp, uint *sig_len)
{
	RSA *rsa;
	ENGINE *e = NULL;
	int ret;

	ret = rsa_init();
	if (ret)
		return ret;

	if (info->engine_id) {
		ret = rsa_engine_init(info->engine_id, &e);
		if (ret)
			goto err_engine;
	}

	ret = rsa_get_priv_key(info->keydir, info->keyname, e, &rsa);
	if (ret)
		goto err_priv;
	ret = rsa_sign_with_key(rsa, info->padding, info->checksum, region,
				region_count, sigp, sig_len);
	if (ret)
		goto err_sign;

	RSA_free(rsa);
	if (info->engine_id)
		rsa_engine_remove(e);
	rsa_remove();

	return ret;

err_sign:
	RSA_free(rsa);
err_priv:
	if (info->engine_id)
		rsa_engine_remove(e);
err_engine:
	rsa_remove();
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

static int fdt_add_bignum(void *blob, int noffset, const char *prop_name,
			  BIGNUM *num, int num_bits)
{
	int nwords = num_bits / 32;
	int size;
	uint32_t *buf, *ptr;
	BIGNUM *tmp, *big2, *big32, *big2_32;
	BN_CTX *ctx;
	int ret;

	tmp = BN_new();
	big2 = BN_new();
	big32 = BN_new();
	big2_32 = BN_new();

	/*
	 * Note: This code assumes that all of the above succeed, or all fail.
	 * In practice memory allocations generally do not fail (unless the
	 * process is killed), so it does not seem worth handling each of these
	 * as a separate case. Technicaly this could leak memory on failure,
	 * but a) it won't happen in practice, and b) it doesn't matter as we
	 * will immediately exit with a failure code.
	 */
	if (!tmp || !big2 || !big32 || !big2_32) {
		fprintf(stderr, "Out of memory (bignum)\n");
		return -ENOMEM;
	}
	ctx = BN_CTX_new();
	if (!tmp) {
		fprintf(stderr, "Out of memory (bignum context)\n");
		return -ENOMEM;
	}
	BN_set_word(big2, 2L);
	BN_set_word(big32, 32L);
	BN_exp(big2_32, big2, big32, ctx); /* B = 2^32 */

	size = nwords * sizeof(uint32_t);
	buf = malloc(size);
	if (!buf) {
		fprintf(stderr, "Out of memory (%d bytes)\n", size);
		return -ENOMEM;
	}

	/* Write out modulus as big endian array of integers */
	for (ptr = buf + nwords - 1; ptr >= buf; ptr--) {
		BN_mod(tmp, num, big2_32, ctx); /* n = N mod B */
		*ptr = cpu_to_fdt32(BN_get_word(tmp));
		BN_rshift(num, num, 32); /*  N = N/B */
	}

	/*
	 * We try signing with successively increasing size values, so this
	 * might fail several times
	 */
	ret = fdt_setprop(blob, noffset, prop_name, buf, size);
	free(buf);
	BN_free(tmp);
	BN_free(big2);
	BN_free(big32);
	BN_free(big2_32);

	return ret ? -FDT_ERR_NOSPACE : 0;
}

int rsa_add_verify_data(struct image_sign_info *info, void *keydest)
{
	BIGNUM *modulus, *r_squared;
	uint64_t exponent;
	uint32_t n0_inv;
	int parent, node;
	char name[100];
	int ret;
	int bits;
	RSA *rsa;
	ENGINE *e = NULL;

	debug("%s: Getting verification data\n", __func__);
	if (info->engine_id) {
		ret = rsa_engine_init(info->engine_id, &e);
		if (ret)
			return ret;
	}
	ret = rsa_get_pub_key(info->keydir, info->keyname, e, &rsa);
	if (ret)
		goto err_get_pub_key;
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
		ret = fdt_setprop_string(keydest, node, "key-name-hint",
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
		ret = fdt_setprop_string(keydest, node, "required",
					 info->require_keys);
	}
done:
	BN_free(modulus);
	BN_free(r_squared);
	if (ret)
		ret = ret == -FDT_ERR_NOSPACE ? -ENOSPC : -EIO;
err_get_params:
	RSA_free(rsa);
err_get_pub_key:
	if (info->engine_id)
		rsa_engine_remove(e);

	return ret;
}
