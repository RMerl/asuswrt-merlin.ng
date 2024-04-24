/*
 * Copyright (C) 2019 Sean Parkinson, wolfSSL Inc.
 * Copyright (C) 2021 Andreas Steffen, strongSec GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "wolfssl_common.h"

#include <library.h>
#include <utils/debug.h>

#include "wolfssl_plugin.h"
#include "wolfssl_aead.h"
#include "wolfssl_crypter.h"
#include "wolfssl_diffie_hellman.h"
#include "wolfssl_ec_diffie_hellman.h"
#include "wolfssl_ec_private_key.h"
#include "wolfssl_ec_public_key.h"
#include "wolfssl_ed_private_key.h"
#include "wolfssl_ed_public_key.h"
#include "wolfssl_hasher.h"
#include "wolfssl_hmac.h"
#include "wolfssl_kdf.h"
#include "wolfssl_rsa_private_key.h"
#include "wolfssl_rsa_public_key.h"
#include "wolfssl_rng.h"
#include "wolfssl_sha1_prf.h"
#include "wolfssl_x_diffie_hellman.h"
#include "wolfssl_xof.h"

#include <wolfssl/ssl.h>

#ifndef FIPS_MODE
#define FIPS_MODE 0
#endif

typedef struct private_wolfssl_plugin_t private_wolfssl_plugin_t;

/**
 * Private data of wolfssl_plugin
 */
struct private_wolfssl_plugin_t {

	/**
	 * Public interface
	 */
	wolfssl_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_wolfssl_plugin_t *this)
{
	return "wolfssl";
}

METHOD(plugin_t, get_features, int,
	private_wolfssl_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		/* crypters */
		PLUGIN_REGISTER(CRYPTER, wolfssl_crypter_create),
#if !defined(NO_AES) && !defined(NO_AES_CTR)
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CTR, 16),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CTR, 24),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CTR, 32),
#endif
#if !defined(NO_AES) && !defined(NO_AES_CBC)
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CBC, 16),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CBC, 24),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CBC, 32),
#endif
#if !defined(NO_AES) && defined(HAVE_AES_ECB)
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_ECB, 16),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_ECB, 24),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_ECB, 32),
#endif
#if !defined(NO_AES) && defined(WOLFSSL_AES_CFB)
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CFB, 16),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CFB, 24),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CFB, 32),
#endif
#ifdef HAVE_CAMELLIA
			PLUGIN_PROVIDE(CRYPTER, ENCR_CAMELLIA_CBC, 16),
			PLUGIN_PROVIDE(CRYPTER, ENCR_CAMELLIA_CBC, 24),
			PLUGIN_PROVIDE(CRYPTER, ENCR_CAMELLIA_CBC, 32),
#endif
#ifndef NO_DES3
			PLUGIN_PROVIDE(CRYPTER, ENCR_3DES, 24),
			PLUGIN_PROVIDE(CRYPTER, ENCR_DES, 8),
	#ifdef WOLFSSL_DES_ECB
			PLUGIN_PROVIDE(CRYPTER, ENCR_DES_ECB, 8),
	#endif
#endif
			PLUGIN_PROVIDE(CRYPTER, ENCR_NULL, 0),
		/* hashers */
		PLUGIN_REGISTER(HASHER, wolfssl_hasher_create),
#ifndef NO_MD5
			PLUGIN_PROVIDE(HASHER, HASH_MD5),
#endif
#ifndef NO_SHA
			PLUGIN_PROVIDE(HASHER, HASH_SHA1),
#endif
#ifdef WOLFSSL_SHA224
			PLUGIN_PROVIDE(HASHER, HASH_SHA224),
#endif
#ifndef NO_SHA256
			PLUGIN_PROVIDE(HASHER, HASH_SHA256),
#endif
#ifdef WOLFSSL_SHA384
			PLUGIN_PROVIDE(HASHER, HASH_SHA384),
#endif
#ifdef WOLFSSL_SHA512
			PLUGIN_PROVIDE(HASHER, HASH_SHA512),
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_224)
			PLUGIN_PROVIDE(HASHER, HASH_SHA3_224),
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_256)
			PLUGIN_PROVIDE(HASHER, HASH_SHA3_256),
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_384)
			PLUGIN_PROVIDE(HASHER, HASH_SHA3_384),
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_512)
			PLUGIN_PROVIDE(HASHER, HASH_SHA3_512),
#endif
#if defined(WOLFSSL_SHAKE256) && LIBWOLFSSL_VERSION_HEX >= 0x04007001
		PLUGIN_REGISTER(XOF, wolfssl_xof_create),
			PLUGIN_PROVIDE(XOF, XOF_SHAKE_256),
#endif
#ifndef NO_SHA
		/* keyed sha1 hasher (aka prf) */
		PLUGIN_REGISTER(PRF, wolfssl_sha1_prf_create),
			PLUGIN_PROVIDE(PRF, PRF_KEYED_SHA1),
#endif
#ifndef NO_HMAC
		PLUGIN_REGISTER(PRF, wolfssl_hmac_prf_create),
#ifndef NO_MD5
			PLUGIN_PROVIDE(PRF, PRF_HMAC_MD5),
#endif
#ifndef NO_SHA
			PLUGIN_PROVIDE(PRF, PRF_HMAC_SHA1),
#endif
#ifndef NO_SHA256
			PLUGIN_PROVIDE(PRF, PRF_HMAC_SHA2_256),
#endif
#ifdef WOLFSSL_SHA384
			PLUGIN_PROVIDE(PRF, PRF_HMAC_SHA2_384),
#endif
#ifdef WOLFSSL_SHA512
			PLUGIN_PROVIDE(PRF, PRF_HMAC_SHA2_512),
#endif
		PLUGIN_REGISTER(SIGNER, wolfssl_hmac_signer_create),
#ifndef NO_MD5
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_MD5_96),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_MD5_128),
#endif
#ifndef NO_SHA
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA1_96),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA1_128),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA1_160),
#endif
#ifndef NO_SHA256
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_256_128),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_256_256),
#endif
#ifdef WOLFSSL_SHA384
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_384_192),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_384_384),
#endif
#ifdef WOLFSSL_SHA512
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_512_256),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_512_512),
#endif
#ifdef HAVE_HKDF
		PLUGIN_REGISTER(KDF, wolfssl_kdf_create),
			PLUGIN_PROVIDE(KDF, KDF_PRF),
			PLUGIN_PROVIDE(KDF, KDF_PRF_PLUS),
#endif
#endif /* NO_HMAC */
#if (!defined(NO_AES) && (defined(HAVE_AESGCM) || defined(HAVE_AESCCM))) || \
								(defined(HAVE_CHACHA) && defined(HAVE_POLY1305))
		PLUGIN_REGISTER(AEAD, wolfssl_aead_create),
#if !defined(NO_AES) && defined(HAVE_AESGCM)
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV16, 16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV16, 24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV16, 32),
	#if WOLFSSL_MIN_AUTH_TAG_SZ <= 12
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV12, 16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV12, 24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV12, 32),
	#endif
	#if WOLFSSL_MIN_AUTH_TAG_SZ <= 8
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV8,  16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV8,  24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV8,  32),
	#endif
#endif /* !NO_AES && HAVE_AESGCM */
#if !defined(NO_AES) && defined(HAVE_AESCCM)
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV16, 16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV16, 24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV16, 32),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV12, 16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV12, 24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV12, 32),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV8,  16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV8,  24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV8,  32),
#endif /* !NO_AES && HAVE_AESCCM */
#if defined(HAVE_CHACHA) && defined(HAVE_POLY1305)
			PLUGIN_PROVIDE(AEAD, ENCR_CHACHA20_POLY1305, 32),
#endif /* HAVE_CHACHA && HAVE_POLY1305 */
#endif
#ifdef HAVE_ECC_DHE
		/* EC DH groups */
		PLUGIN_REGISTER(KE, wolfssl_ec_diffie_hellman_create),
	#if (!defined(NO_ECC256) || defined(HAVE_ALL_CURVES)) && \
		(!defined(ECC_MIN_KEY_SZ) || ECC_MIN_KEY_SZ <= 256)
			PLUGIN_PROVIDE(KE, ECP_256_BIT),
	#endif
	#if (defined(HAVE_ECC384) || defined(HAVE_ALL_CURVES)) && \
		(!defined(ECC_MIN_KEY_SZ) || ECC_MIN_KEY_SZ <= 384)
			PLUGIN_PROVIDE(KE, ECP_384_BIT),
	#endif
	#if (defined(HAVE_ECC521) || defined(HAVE_ALL_CURVES)) && \
		(!defined(ECC_MIN_KEY_SZ) || ECC_MIN_KEY_SZ <= 521)
			PLUGIN_PROVIDE(KE, ECP_521_BIT),
	#endif
	#if (defined(HAVE_ECC224) || defined(HAVE_ALL_CURVES)) && \
		 (!defined(ECC_MIN_KEY_SZ) || ECC_MIN_KEY_SZ <= 224)
			PLUGIN_PROVIDE(KE, ECP_224_BIT),
	#endif
	#if (defined(HAVE_ECC192) || defined(HAVE_ALL_CURVES)) && \
		 (!defined(ECC_MIN_KEY_SZ) || ECC_MIN_KEY_SZ <= 192)
			PLUGIN_PROVIDE(KE, ECP_192_BIT),
	#endif
	#ifdef HAVE_ECC_BRAINPOOL
		#if (!defined(NO_ECC256) || defined(HAVE_ALL_CURVES)) && \
			(!defined(ECC_MIN_KEY_SZ) || ECC_MIN_KEY_SZ <= 256)
			PLUGIN_PROVIDE(KE, ECP_256_BP),
		#endif
		#if (defined(HAVE_ECC384) || defined(HAVE_ALL_CURVES))  && \
			(!defined(ECC_MIN_KEY_SZ) || ECC_MIN_KEY_SZ <= 384)
			PLUGIN_PROVIDE(KE, ECP_384_BP),
		#endif
		#if (defined(HAVE_ECC512) || defined(HAVE_ALL_CURVES)) && \
			(!defined(ECC_MIN_KEY_SZ) || ECC_MIN_KEY_SZ <= 512)
			PLUGIN_PROVIDE(KE, ECP_512_BP),
		#endif
		#if (defined(HAVE_ECC224) || defined(HAVE_ALL_CURVES)) && \
			(!defined(ECC_MIN_KEY_SZ) || ECC_MIN_KEY_SZ <= 224)
			PLUGIN_PROVIDE(KE, ECP_224_BP),
		#endif
	#endif
#endif /* HAVE_ECC_DHE */
#ifndef NO_DH
		/* MODP DH groups */
		PLUGIN_REGISTER(KE, wolfssl_diffie_hellman_create),
	#if (defined(USE_FAST_MATH) && FP_MAX_BITS >= (3072 * 2)) || \
		(defined(WOLFSSL_SP_MATH_ALL) && SP_INT_BITS >= 3072) || \
		defined(USE_INTEGER_HEAP_MATH)
			PLUGIN_PROVIDE(KE, MODP_3072_BIT),
	#endif
	#if (defined(USE_FAST_MATH) && FP_MAX_BITS >= (4096 * 2)) || \
		(defined(WOLFSSL_SP_MATH_ALL) && SP_INT_BITS >= 4096) || \
		defined(USE_INTEGER_HEAP_MATH)
			PLUGIN_PROVIDE(KE, MODP_4096_BIT),
	#endif
	#if (defined(USE_FAST_MATH) && FP_MAX_BITS >= (6144 * 2)) || \
		(defined(WOLFSSL_SP_MATH_ALL) && SP_INT_BITS >= 6144) || \
		defined(USE_INTEGER_HEAP_MATH)
			PLUGIN_PROVIDE(KE, MODP_6144_BIT),
	#endif
	#if (defined(USE_FAST_MATH) && FP_MAX_BITS >= (8192 * 2)) || \
		(defined(WOLFSSL_SP_MATH_ALL) && SP_INT_BITS >= 8192) || \
		defined(USE_INTEGER_HEAP_MATH)
			PLUGIN_PROVIDE(KE, MODP_8192_BIT),
	#endif
	#if (defined(USE_FAST_MATH) && FP_MAX_BITS >= (2048 * 2)) || \
		(defined(WOLFSSL_SP_MATH_ALL) && SP_INT_BITS >= 2048) || \
		defined(USE_INTEGER_HEAP_MATH)
			PLUGIN_PROVIDE(KE, MODP_2048_BIT),
			PLUGIN_PROVIDE(KE, MODP_2048_224),
			PLUGIN_PROVIDE(KE, MODP_2048_256),
	#endif
	#if (defined(USE_FAST_MATH) && FP_MAX_BITS >= (1536 * 2)) || \
		(defined(WOLFSSL_SP_MATH_ALL) && SP_INT_BITS >= 1536) || \
		defined(USE_INTEGER_HEAP_MATH)
			PLUGIN_PROVIDE(KE, MODP_1536_BIT),
	#endif
	#if (defined(USE_FAST_MATH) && FP_MAX_BITS >= (1024 * 2)) || \
		(defined(WOLFSSL_SP_MATH_ALL) && SP_INT_BITS >= 1024) || \
		defined(USE_INTEGER_HEAP_MATH)
			PLUGIN_PROVIDE(KE, MODP_1024_BIT),
			PLUGIN_PROVIDE(KE, MODP_1024_160),
	#endif
	#if (defined(USE_FAST_MATH) && FP_MAX_BITS >= (768 * 2)) || \
		(defined(WOLFSSL_SP_MATH_ALL) && SP_INT_BITS >= 768) || \
		defined(USE_INTEGER_HEAP_MATH)
			PLUGIN_PROVIDE(KE, MODP_768_BIT),
	#endif
			PLUGIN_PROVIDE(KE, MODP_CUSTOM),
#endif /* NO_DH */
#ifndef NO_RSA
		/* RSA private/public key loading */
		PLUGIN_REGISTER(PRIVKEY, wolfssl_rsa_private_key_load, TRUE),
			PLUGIN_PROVIDE(PRIVKEY, KEY_RSA),
			PLUGIN_PROVIDE(PRIVKEY, KEY_ANY),
		PLUGIN_REGISTER(PUBKEY, wolfssl_rsa_public_key_load, TRUE),
			PLUGIN_PROVIDE(PUBKEY, KEY_RSA),
	#ifdef WOLFSSL_KEY_GEN
		PLUGIN_REGISTER(PRIVKEY_GEN, wolfssl_rsa_private_key_gen, FALSE),
			PLUGIN_PROVIDE(PRIVKEY_GEN, KEY_RSA),
	#endif
		/* signature/encryption schemes */
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_NULL),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_NULL),
	#ifdef WC_RSA_PSS
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PSS),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PSS),
	#endif
	#ifndef NO_SHA
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA1),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA1),
	#endif
	#ifdef WOLFSSL_SHA224
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA2_224),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA2_224),
	#endif
	#ifndef NO_SHA256
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA2_256),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA2_256),
	#endif
	#ifdef WOLFSSL_SHA384
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA2_384),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA2_384),
	#endif
	#ifdef WOLFSSL_SHA512
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA2_512),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA2_512),
	#endif
	#if defined(WOLFSSL_SHA3) && LIBWOLFSSL_VERSION_HEX >= 0x04007001
	#ifndef WOLFSSL_NOSHA3_224
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA3_224),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA3_224),
	#endif
	#ifndef WOLFSSL_NOSHA3_256
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA3_256),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA3_256),
	#endif
	#ifndef WOLFSSL_NOSHA3_384
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA3_384),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA3_384),
	#endif
	#ifndef WOLFSSL_NOSHA3_512
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA3_512),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA3_512),
	#endif
	#endif /* WOLFSSL_SHA3 */
		PLUGIN_PROVIDE(PRIVKEY_DECRYPT, ENCRYPT_RSA_PKCS1),
		PLUGIN_PROVIDE(PUBKEY_ENCRYPT, ENCRYPT_RSA_PKCS1),
	#ifndef NO_MD5
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_MD5),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_MD5),
	#endif
	#ifndef WC_NO_RSA_OAEP
	#ifndef NO_SHA
		PLUGIN_PROVIDE(PUBKEY_ENCRYPT, ENCRYPT_RSA_OAEP_SHA1),
		PLUGIN_PROVIDE(PRIVKEY_DECRYPT, ENCRYPT_RSA_OAEP_SHA1),
	#endif
	#ifdef WOLFSSL_SHA224
		PLUGIN_PROVIDE(PUBKEY_ENCRYPT, ENCRYPT_RSA_OAEP_SHA224),
		PLUGIN_PROVIDE(PRIVKEY_DECRYPT, ENCRYPT_RSA_OAEP_SHA224),
	#endif
	#ifndef NO_SHA256
		PLUGIN_PROVIDE(PUBKEY_ENCRYPT, ENCRYPT_RSA_OAEP_SHA256),
		PLUGIN_PROVIDE(PRIVKEY_DECRYPT, ENCRYPT_RSA_OAEP_SHA256),
	#endif
	#ifdef WOLFSSL_SHA384
		PLUGIN_PROVIDE(PUBKEY_ENCRYPT, ENCRYPT_RSA_OAEP_SHA384),
		PLUGIN_PROVIDE(PRIVKEY_DECRYPT, ENCRYPT_RSA_OAEP_SHA384),
	#endif
	#ifdef WOLFSSL_SHA512
		PLUGIN_PROVIDE(PUBKEY_ENCRYPT, ENCRYPT_RSA_OAEP_SHA512),
		PLUGIN_PROVIDE(PRIVKEY_DECRYPT, ENCRYPT_RSA_OAEP_SHA512),
	#endif
	#endif /* !WC_NO_RSA_OAEP */
#endif /* !NO_RSA */
#ifdef HAVE_ECC
	#ifdef HAVE_ECC_KEY_IMPORT
		/* EC private/public key loading */
		PLUGIN_REGISTER(PRIVKEY, wolfssl_ec_private_key_load, TRUE),
			PLUGIN_PROVIDE(PRIVKEY, KEY_ECDSA),
			PLUGIN_PROVIDE(PRIVKEY, KEY_ANY),
	#endif
	#ifdef HAVE_ECC_DHE
		PLUGIN_REGISTER(PRIVKEY_GEN, wolfssl_ec_private_key_gen, FALSE),
			PLUGIN_PROVIDE(PRIVKEY_GEN, KEY_ECDSA),
	#endif
	#ifdef HAVE_ECC_KEY_IMPORT
		PLUGIN_REGISTER(PUBKEY, wolfssl_ec_public_key_load, TRUE),
			PLUGIN_PROVIDE(PUBKEY, KEY_ECDSA),
	#endif
	#ifdef HAVE_ECC_SIGN
		/* signature encryption schemes */
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_WITH_NULL),
		#ifndef NO_SHA
			PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_WITH_SHA1_DER),
		#endif
		#ifndef NO_SHA256
			PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_WITH_SHA256_DER),
			PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_256),
		#endif
		#ifdef WOLFSSL_SHA384
			PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_WITH_SHA384_DER),
			PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_384),
		#endif
		#ifdef WOLFSSL_SHA512
			PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_WITH_SHA512_DER),
			PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_521),
		#endif
	#endif /* HAVE_ECC_SIGN */
	#ifdef HAVE_ECC_VERIFY
		/* signature encryption schemes */
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_WITH_NULL),
		#ifndef NO_SHA
			PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_WITH_SHA1_DER),
		#endif
		#ifndef NO_SHA256
			PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_WITH_SHA256_DER),
			PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_256),
		#endif
		#ifdef WOLFSSL_SHA384
			PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_WITH_SHA384_DER),
			PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_384),
		#endif
		#ifdef WOLFSSL_SHA512
			PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_WITH_SHA512_DER),
			PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_521),
		#endif
	#endif /* HAVE_ECC_VERIFY */
#endif /* HAVE_ECC */
#if defined (HAVE_CURVE25519) || defined(HAVE_CURVE448)
		PLUGIN_REGISTER(KE, wolfssl_x_diffie_hellman_create),
	#ifdef HAVE_CURVE25519
			PLUGIN_PROVIDE(KE, CURVE_25519),
	#endif
	#ifdef HAVE_CURVE448
			PLUGIN_PROVIDE(KE, CURVE_448),
	#endif
#endif /* HAVE_CURVE25519 || HAVE_CURVE448 */
#if defined(HAVE_ED25519) || defined(HAVE_ED448)
		/* EdDSA private/public key loading */
		PLUGIN_REGISTER(PUBKEY, wolfssl_ed_public_key_load, TRUE),
	#ifdef HAVE_ED25519
			PLUGIN_PROVIDE(PUBKEY, KEY_ED25519),
	#endif
	#ifdef HAVE_ED448
			PLUGIN_PROVIDE(PUBKEY, KEY_ED448),
	#endif
		PLUGIN_REGISTER(PRIVKEY, wolfssl_ed_private_key_load, TRUE),
	#ifdef HAVE_ED25519
			PLUGIN_PROVIDE(PRIVKEY, KEY_ED25519),
	#endif
	#ifdef HAVE_ED448
			PLUGIN_PROVIDE(PRIVKEY, KEY_ED448),
	#endif
		PLUGIN_REGISTER(PRIVKEY_GEN, wolfssl_ed_private_key_gen, FALSE),
	#ifdef HAVE_ED25519
			PLUGIN_PROVIDE(PRIVKEY_GEN, KEY_ED25519),
	#endif
	#ifdef HAVE_ED448
			PLUGIN_PROVIDE(PRIVKEY_GEN, KEY_ED448),
	#endif
	#ifdef HAVE_ED25519_SIGN
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ED25519),
	#endif
	#ifdef HAVE_ED25519_VERIFY
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ED25519),
	#endif
	#ifdef HAVE_ED448_SIGN
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ED448),
	#endif
	#ifdef HAVE_ED448_VERIFY
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ED448),
	#endif
		/* register a pro forma identity hasher, never instantiated */
		PLUGIN_REGISTER(HASHER, return_null),
			PLUGIN_PROVIDE(HASHER, HASH_IDENTITY),
#endif /* HAVE_ED25519 || HAVE_ED448 */
#ifndef WC_NO_RNG
		/* generic key loader */
		PLUGIN_REGISTER(RNG, wolfssl_rng_create),
			PLUGIN_PROVIDE(RNG, RNG_STRONG),
			PLUGIN_PROVIDE(RNG, RNG_WEAK),
#endif
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_wolfssl_plugin_t *this)
{
#ifndef WC_NO_RNG
	wolfssl_rng_global_final();
#endif
	wolfSSL_Cleanup();

	free(this);
}

/*
 * Described in header
 */
plugin_t *wolfssl_plugin_create()
{
	private_wolfssl_plugin_t *this;
	bool fips_mode;

	fips_mode = lib->settings->get_bool(lib->settings,
								"%s.plugins.wolfssl.fips_mode", FALSE, lib->ns);
#ifdef HAVE_FIPS
	if (fips_mode)
	{
		int ret = wolfCrypt_GetStatus_fips();
		if (ret != 0)
		{
			DBG1(DBG_LIB, "wolfssl FIPS mode unavailable (%d)", ret);
			return NULL;
		}
	}
#else
	if (fips_mode)
	{
		DBG1(DBG_LIB, "wolfssl FIPS mode unavailable");
		return NULL;
	}
#endif

	wolfSSL_Init();
#ifndef WC_NO_RNG
	if (!wolfssl_rng_global_init())
	{
		return NULL;
	}
#endif

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
	);

	return &this->public.plugin;
}
