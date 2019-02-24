/*
 * Copyright (C) 2010-2014 Martin Willi
 * Copyright (C) 2010-2014 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "tls_crypto.h"

#include <utils/debug.h>
#include <plugins/plugin_feature.h>

ENUM_BEGIN(tls_cipher_suite_names, TLS_NULL_WITH_NULL_NULL,
								   TLS_DH_anon_WITH_3DES_EDE_CBC_SHA,
	"TLS_NULL_WITH_NULL_NULL",
	"TLS_RSA_WITH_NULL_MD5",
	"TLS_RSA_WITH_NULL_SHA",
	"TLS_RSA_EXPORT_WITH_RC4_40_MD5",
	"TLS_RSA_WITH_RC4_128_MD5",
	"TLS_RSA_WITH_RC4_128_SHA",
	"TLS_RSA_EXPORT_WITH_RC2_CBC_40_MD5",
	"TLS_RSA_WITH_IDEA_CBC_SHA",
	"TLS_RSA_EXPORT_WITH_DES40_CBC_SHA",
	"TLS_RSA_WITH_DES_CBC_SHA",
	"TLS_RSA_WITH_3DES_EDE_CBC_SHA",
	"TLS_DH_DSS_EXPORT_WITH_DES40_CBC_SHA",
	"TLS_DH_DSS_WITH_DES_CBC_SHA",
	"TLS_DH_DSS_WITH_3DES_EDE_CBC_SHA",
	"TLS_DH_RSA_EXPORT_WITH_DES40_CBC_SHA",
	"TLS_DH_RSA_WITH_DES_CBC_SHA",
	"TLS_DH_RSA_WITH_3DES_EDE_CBC_SHA",
	"TLS_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA",
	"TLS_DHE_DSS_WITH_DES_CBC_SHA",
	"TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA",
	"TLS_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA",
	"TLS_DHE_RSA_WITH_DES_CBC_SHA",
	"TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA",
	"TLS_DH_anon_EXPORT_WITH_RC4_40_MD5",
	"TLS_DH_anon_WITH_RC4_128_MD5",
	"TLS_DH_anon_EXPORT_WITH_DES40_CBC_SHA",
	"TLS_DH_anon_WITH_DES_CBC_SHA",
	"TLS_DH_anon_WITH_3DES_EDE_CBC_SHA");
ENUM_NEXT(tls_cipher_suite_names, TLS_KRB5_WITH_DES_CBC_SHA,
								  TLS_DH_anon_WITH_CAMELLIA_128_CBC_SHA,
								  TLS_DH_anon_WITH_3DES_EDE_CBC_SHA,
	"TLS_KRB5_WITH_DES_CBC_SHA",
	"TLS_KRB5_WITH_3DES_EDE_CBC_SHA",
	"TLS_KRB5_WITH_RC4_128_SHA",
	"TLS_KRB5_WITH_IDEA_CBC_SHA",
	"TLS_KRB5_WITH_DES_CBC_MD5",
	"TLS_KRB5_WITH_3DES_EDE_CBC_MD5",
	"TLS_KRB5_WITH_RC4_128_MD5",
	"TLS_KRB5_WITH_IDEA_CBC_MD5",
	"TLS_KRB5_EXPORT_WITH_DES_CBC_40_SHA",
	"TLS_KRB5_EXPORT_WITH_RC2_CBC_40_SHA",
	"TLS_KRB5_EXPORT_WITH_RC4_40_SHA",
	"TLS_KRB5_EXPORT_WITH_DES_CBC_40_MD5",
	"TLS_KRB5_EXPORT_WITH_RC2_CBC_40_MD5",
	"TLS_KRB5_EXPORT_WITH_RC4_40_MD5",
	"TLS_PSK_WITH_NULL_SHA",
	"TLS_DHE_PSK_WITH_NULL_SHA",
	"TLS_RSA_PSK_WITH_NULL_SHA",
	"TLS_RSA_WITH_AES_128_CBC_SHA",
	"TLS_DH_DSS_WITH_AES_128_CBC_SHA",
	"TLS_DH_RSA_WITH_AES_128_CBC_SHA",
	"TLS_DHE_DSS_WITH_AES_128_CBC_SHA",
	"TLS_DHE_RSA_WITH_AES_128_CBC_SHA",
	"TLS_DH_anon_WITH_AES_128_CBC_SHA",
	"TLS_RSA_WITH_AES_256_CBC_SHA",
	"TLS_DH_DSS_WITH_AES_256_CBC_SHA",
	"TLS_DH_RSA_WITH_AES_256_CBC_SHA",
	"TLS_DHE_DSS_WITH_AES_256_CBC_SHA",
	"TLS_DHE_RSA_WITH_AES_256_CBC_SHA",
	"TLS_DH_anon_WITH_AES_256_CBC_SHA",
	"TLS_RSA_WITH_NULL_SHA256",
	"TLS_RSA_WITH_AES_128_CBC_SHA256",
	"TLS_RSA_WITH_AES_256_CBC_SHA256",
	"TLS_DH_DSS_WITH_AES_128_CBC_SHA256",
	"TLS_DH_RSA_WITH_AES_128_CBC_SHA256",
	"TLS_DHE_DSS_WITH_AES_128_CBC_SHA256",
	"TLS_RSA_WITH_CAMELLIA_128_CBC_SHA",
	"TLS_DH_DSS_WITH_CAMELLIA_128_CBC_SHA",
	"TLS_DH_RSA_WITH_CAMELLIA_128_CBC_SHA",
	"TLS_DHE_DSS_WITH_CAMELLIA_128_CBC_SHA",
	"TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA",
	"TLS_DH_anon_WITH_CAMELLIA_128_CBC_SHA");
ENUM_NEXT(tls_cipher_suite_names, TLS_DHE_RSA_WITH_AES_128_CBC_SHA256,
								  TLS_DH_anon_WITH_AES_256_CBC_SHA256,
								  TLS_DH_anon_WITH_CAMELLIA_128_CBC_SHA,
	"TLS_DHE_RSA_WITH_AES_128_CBC_SHA256",
	"TLS_DH_DSS_WITH_AES_256_CBC_SHA256",
	"TLS_DH_RSA_WITH_AES_256_CBC_SHA256",
	"TLS_DHE_DSS_WITH_AES_256_CBC_SHA256",
	"TLS_DHE_RSA_WITH_AES_256_CBC_SHA256",
	"TLS_DH_anon_WITH_AES_128_CBC_SHA256",
	"TLS_DH_anon_WITH_AES_256_CBC_SHA256");
ENUM_NEXT(tls_cipher_suite_names, TLS_RSA_WITH_CAMELLIA_256_CBC_SHA,
								  TLS_DH_anon_WITH_CAMELLIA_256_CBC_SHA256,
								  TLS_DH_anon_WITH_AES_256_CBC_SHA256,
	"TLS_RSA_WITH_CAMELLIA_256_CBC_SHA",
	"TLS_DH_DSS_WITH_CAMELLIA_256_CBC_SHA",
	"TLS_DH_RSA_WITH_CAMELLIA_256_CBC_SHA",
	"TLS_DHE_DSS_WITH_CAMELLIA_256_CBC_SHA",
	"TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA",
	"TLS_DH_anon_WITH_CAMELLIA_256_CBC_SHA",
	"TLS_PSK_WITH_RC4_128_SHA",
	"TLS_PSK_WITH_3DES_EDE_CBC_SHA",
	"TLS_PSK_WITH_AES_128_CBC_SHA",
	"TLS_PSK_WITH_AES_256_CBC_SHA",
	"TLS_DHE_PSK_WITH_RC4_128_SHA",
	"TLS_DHE_PSK_WITH_3DES_EDE_CBC_SHA",
	"TLS_DHE_PSK_WITH_AES_128_CBC_SHA",
	"TLS_DHE_PSK_WITH_AES_256_CBC_SHA",
	"TLS_RSA_PSK_WITH_RC4_128_SHA",
	"TLS_RSA_PSK_WITH_3DES_EDE_CBC_SHA",
	"TLS_RSA_PSK_WITH_AES_128_CBC_SHA",
	"TLS_RSA_PSK_WITH_AES_256_CBC_SHA",
	"TLS_RSA_WITH_SEED_CBC_SHA",
	"TLS_DH_DSS_WITH_SEED_CBC_SHA",
	"TLS_DH_RSA_WITH_SEED_CBC_SHA",
	"TLS_DHE_DSS_WITH_SEED_CBC_SHA",
	"TLS_DHE_RSA_WITH_SEED_CBC_SHA",
	"TLS_DH_anon_WITH_SEED_CBC_SHA",
	"TLS_RSA_WITH_AES_128_GCM_SHA256",
	"TLS_RSA_WITH_AES_256_GCM_SHA384",
	"TLS_DHE_RSA_WITH_AES_128_GCM_SHA256",
	"TLS_DHE_RSA_WITH_AES_256_GCM_SHA384",
	"TLS_DH_RSA_WITH_AES_128_GCM_SHA256",
	"TLS_DH_RSA_WITH_AES_256_GCM_SHA384",
	"TLS_DHE_DSS_WITH_AES_128_GCM_SHA256",
	"TLS_DHE_DSS_WITH_AES_256_GCM_SHA384",
	"TLS_DH_DSS_WITH_AES_128_GCM_SHA256",
	"TLS_DH_DSS_WITH_AES_256_GCM_SHA384",
	"TLS_DH_anon_WITH_AES_128_GCM_SHA256",
	"TLS_DH_anon_WITH_AES_256_GCM_SHA384",
	"TLS_PSK_WITH_AES_128_GCM_SHA256",
	"TLS_PSK_WITH_AES_256_GCM_SHA384",
	"TLS_DHE_PSK_WITH_AES_128_GCM_SHA256",
	"TLS_DHE_PSK_WITH_AES_256_GCM_SHA384",
	"TLS_RSA_PSK_WITH_AES_128_GCM_SHA256",
	"TLS_RSA_PSK_WITH_AES_256_GCM_SHA384",
	"TLS_PSK_WITH_AES_128_CBC_SHA256",
	"TLS_PSK_WITH_AES_256_CBC_SHA384",
	"TLS_PSK_WITH_NULL_SHA256",
	"TLS_PSK_WITH_NULL_SHA384",
	"TLS_DHE_PSK_WITH_AES_128_CBC_SHA256",
	"TLS_DHE_PSK_WITH_AES_256_CBC_SHA384",
	"TLS_DHE_PSK_WITH_NULL_SHA256",
	"TLS_DHE_PSK_WITH_NULL_SHA384",
	"TLS_RSA_PSK_WITH_AES_128_CBC_SHA256",
	"TLS_RSA_PSK_WITH_AES_256_CBC_SHA384",
	"TLS_RSA_PSK_WITH_NULL_SHA256",
	"TLS_RSA_PSK_WITH_NULL_SHA384",
	"TLS_RSA_WITH_CAMELLIA_128_CBC_SHA256",
	"TLS_DH_DSS_WITH_CAMELLIA_128_CBC_SHA256",
	"TLS_DH_RSA_WITH_CAMELLIA_128_CBC_SHA256",
	"TLS_DHE_DSS_WITH_CAMELLIA_128_CBC_SHA256",
	"TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA256",
	"TLS_DH_anon_WITH_CAMELLIA_128_CBC_SHA256",
	"TLS_RSA_WITH_CAMELLIA_256_CBC_SHA256",
	"TLS_DH_DSS_WITH_CAMELLIA_256_CBC_SHA256",
	"TLS_DH_RSA_WITH_CAMELLIA_256_CBC_SHA256",
	"TLS_DHE_DSS_WITH_CAMELLIA_256_CBC_SHA256",
	"TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA256",
	"TLS_DH_anon_WITH_CAMELLIA_256_CBC_SHA256");
ENUM_NEXT(tls_cipher_suite_names, TLS_EMPTY_RENEGOTIATION_INFO_SCSV,
								  TLS_EMPTY_RENEGOTIATION_INFO_SCSV,
								  TLS_DH_anon_WITH_CAMELLIA_256_CBC_SHA256,
	"TLS_EMPTY_RENEGOTIATION_INFO_SCSV");
ENUM_NEXT(tls_cipher_suite_names, TLS_ECDH_ECDSA_WITH_NULL_SHA,
								  TLS_ECDHE_PSK_WITH_NULL_SHA384,
								  TLS_EMPTY_RENEGOTIATION_INFO_SCSV,
	"TLS_ECDH_ECDSA_WITH_NULL_SHA",
	"TLS_ECDH_ECDSA_WITH_RC4_128_SHA",
	"TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA",
	"TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA",
	"TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA",
	"TLS_ECDHE_ECDSA_WITH_NULL_SHA",
	"TLS_ECDHE_ECDSA_WITH_RC4_128_SHA",
	"TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA",
	"TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA",
	"TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA",
	"TLS_ECDH_RSA_WITH_NULL_SHA",
	"TLS_ECDH_RSA_WITH_RC4_128_SHA",
	"TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA",
	"TLS_ECDH_RSA_WITH_AES_128_CBC_SHA",
	"TLS_ECDH_RSA_WITH_AES_256_CBC_SHA",
	"TLS_ECDHE_RSA_WITH_NULL_SHA",
	"TLS_ECDHE_RSA_WITH_RC4_128_SHA",
	"TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA",
	"TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA",
	"TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA",
	"TLS_ECDH_anon_WITH_NULL_SHA",
	"TLS_ECDH_anon_WITH_RC4_128_SHA",
	"TLS_ECDH_anon_WITH_3DES_EDE_CBC_SHA",
	"TLS_ECDH_anon_WITH_AES_128_CBC_SHA",
	"TLS_ECDH_anon_WITH_AES_256_CBC_SHA",
	"TLS_SRP_SHA_WITH_3DES_EDE_CBC_SHA",
	"TLS_SRP_SHA_RSA_WITH_3DES_EDE_CBC_SHA",
	"TLS_SRP_SHA_DSS_WITH_3DES_EDE_CBC_SHA",
	"TLS_SRP_SHA_WITH_AES_128_CBC_SHA",
	"TLS_SRP_SHA_RSA_WITH_AES_128_CBC_SHA",
	"TLS_SRP_SHA_DSS_WITH_AES_128_CBC_SHA",
	"TLS_SRP_SHA_WITH_AES_256_CBC_SHA",
	"TLS_SRP_SHA_RSA_WITH_AES_256_CBC_SHA",
	"TLS_SRP_SHA_DSS_WITH_AES_256_CBC_SHA",
	"TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256",
	"TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384",
	"TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256",
	"TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384",
	"TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256",
	"TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384",
	"TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256",
	"TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384",
	"TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256",
	"TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384",
	"TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256",
	"TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384",
	"TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256",
	"TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384",
	"TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256",
	"TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384",
	"TLS_ECDHE_PSK_WITH_RC4_128_SHA",
	"TLS_ECDHE_PSK_WITH_3DES_EDE_CBC_SHA",
	"TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA",
	"TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA",
	"TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256",
	"TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA384",
	"TLS_ECDHE_PSK_WITH_NULL_SHA",
	"TLS_ECDHE_PSK_WITH_NULL_SHA256",
	"TLS_ECDHE_PSK_WITH_NULL_SHA384");
ENUM_END(tls_cipher_suite_names, TLS_ECDHE_PSK_WITH_NULL_SHA384);

ENUM(tls_hash_algorithm_names, TLS_HASH_NONE, TLS_HASH_SHA512,
	"NONE",
	"MD5",
	"SHA1",
	"SHA224",
	"SHA256",
	"SHA384",
	"SHA512",
);

ENUM(tls_signature_algorithm_names, TLS_SIG_RSA, TLS_SIG_ECDSA,
	"RSA",
	"DSA",
	"ECDSA",
);

ENUM_BEGIN(tls_client_certificate_type_names,
		   TLS_RSA_SIGN, TLS_DSS_EPHEMERAL_DH,
	"RSA_SIGN",
	"DSA_SIGN",
	"RSA_FIXED_DH",
	"DSS_FIXED_DH",
	"RSA_EPHEMERAL_DH",
	"DSS_EPHEMERAL_DH");
ENUM_NEXT(tls_client_certificate_type_names,
		  TLS_FORTEZZA_DMS, TLS_FORTEZZA_DMS, TLS_DSS_EPHEMERAL_DH,
	"FORTEZZA_DMS");
ENUM_NEXT(tls_client_certificate_type_names,
		  TLS_ECDSA_SIGN, TLS_ECDSA_FIXED_ECDH, TLS_FORTEZZA_DMS,
	"ECDSA_SIGN",
	"RSA_FIXED_ECDH",
	"ECDSA_FIXED_ECDH");
ENUM_END(tls_client_certificate_type_names, TLS_ECDSA_FIXED_ECDH);

ENUM(tls_ecc_curve_type_names, TLS_ECC_EXPLICIT_PRIME, TLS_ECC_NAMED_CURVE,
	"EXPLICIT_PRIME",
	"EXPLICIT_CHAR2",
	"NAMED_CURVE",
);

ENUM(tls_named_curve_names, TLS_SECT163K1, TLS_SECP521R1,
	"SECT163K1",
	"SECT163R1",
	"SECT163R2",
	"SECT193R1",
	"SECT193R2",
	"SECT233K1",
	"SECT233R1",
	"SECT239K1",
	"SECT283K1",
	"SECT283R1",
	"SECT409K1",
	"SECT409R1",
	"SECT571K1",
	"SECT571R1",
	"SECP160K1",
	"SECP160R1",
	"SECP160R2",
	"SECP192K1",
	"SECP192R1",
	"SECP224K1",
	"SECP224R1",
	"SECP256K1",
	"SECP256R1",
	"SECP384R1",
	"SECP521R1",
);

ENUM(tls_ansi_point_format_names, TLS_ANSI_COMPRESSED, TLS_ANSI_HYBRID_Y,
	"compressed",
	"compressed y",
	"uncompressed",
	"uncompressed y",
	"hybrid",
	"hybrid y",
);

ENUM(tls_ec_point_format_names,
	 TLS_EC_POINT_UNCOMPRESSED, TLS_EC_POINT_ANSIX962_COMPRESSED_CHAR2,
	"uncompressed",
	"ansiX962 compressed prime",
	"ansiX962 compressed char2",
);

typedef struct private_tls_crypto_t private_tls_crypto_t;

/**
 * Private data of an tls_crypto_t object.
 */
struct private_tls_crypto_t {

	/**
	 * Public tls_crypto_t interface.
	 */
	tls_crypto_t public;

	/**
	 * Protection layer
	 */
	tls_protection_t *protection;

	/**
	 * List of supported/acceptable cipher suites
	 */
	tls_cipher_suite_t *suites;

	/**
	 * Number of supported suites
	 */
	int suite_count;

	/**
	 * Selected cipher suite
	 */
	tls_cipher_suite_t suite;

	/**
	 * RSA supported?
	 */
	bool rsa;

	/**
	 * ECDSA supported?
	 */
	bool ecdsa;

	/**
	 * TLS context
	 */
	tls_t *tls;

	/**
	 * TLS session cache
	 */
	tls_cache_t *cache;

	/**
	 * All handshake data concatenated
	 */
	chunk_t handshake;

	/**
	 * Connection state TLS PRF
	 */
	tls_prf_t *prf;

	/**
	 * AEAD transform for inbound traffic
	 */
	tls_aead_t *aead_in;

	/**
	 * AEAD transform for outbound traffic
	 */
	tls_aead_t *aead_out;

	/**
	 * EAP-[T]TLS MSK
	 */
	chunk_t msk;

	/**
	 * ASCII string constant used as seed for EAP-[T]TLS MSK PRF
	 */
	char *msk_label;
};

typedef struct {
	tls_cipher_suite_t suite;
	key_type_t key;
	diffie_hellman_group_t dh;
	hash_algorithm_t hash;
	pseudo_random_function_t prf;
	integrity_algorithm_t mac;
	encryption_algorithm_t encr;
	size_t encr_size;
} suite_algs_t;

/**
 * Mapping suites to a set of algorithms
 */
static suite_algs_t suite_algs[] = {
	{ TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA,
		KEY_ECDSA, ECP_256_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_AES_CBC, 16
	},
	{ TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256,
		KEY_ECDSA, ECP_256_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA2_256_256, ENCR_AES_CBC, 16
	},
	{ TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA,
		KEY_ECDSA, ECP_384_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_AES_CBC, 32
	},
	{ TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384,
		KEY_ECDSA, ECP_384_BIT,
		HASH_SHA384, PRF_HMAC_SHA2_384,
		AUTH_HMAC_SHA2_384_384, ENCR_AES_CBC, 32
	},
	{ TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
		KEY_ECDSA, ECP_256_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_UNDEFINED, ENCR_AES_GCM_ICV16, 16
	},
	{ TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,
		KEY_ECDSA, ECP_384_BIT,
		HASH_SHA384, PRF_HMAC_SHA2_384,
		AUTH_UNDEFINED, ENCR_AES_GCM_ICV16, 32
	},
	{ TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA,
		KEY_RSA, ECP_256_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_AES_CBC, 16
	},
	{ TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256,
		KEY_RSA, ECP_256_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA2_256_256, ENCR_AES_CBC, 16
	},
	{ TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA,
		KEY_RSA, ECP_384_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_AES_CBC, 32
	},
	{ TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384,
		KEY_RSA, ECP_384_BIT,
		HASH_SHA384, PRF_HMAC_SHA2_384,
		AUTH_HMAC_SHA2_384_384, ENCR_AES_CBC, 32
	},
	{ TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,
		KEY_RSA, ECP_256_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_UNDEFINED, ENCR_AES_GCM_ICV16, 16
	},
	{ TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384,
		KEY_RSA, ECP_384_BIT,
		HASH_SHA384, PRF_HMAC_SHA2_384,
		AUTH_UNDEFINED, ENCR_AES_GCM_ICV16, 32
	},
	{ TLS_DHE_RSA_WITH_AES_128_CBC_SHA,
		KEY_RSA, MODP_2048_BIT,
		HASH_SHA256,PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_AES_CBC, 16
	},
	{ TLS_DHE_RSA_WITH_AES_128_CBC_SHA256,
		KEY_RSA, MODP_3072_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA2_256_256, ENCR_AES_CBC, 16
	},
	{ TLS_DHE_RSA_WITH_AES_256_CBC_SHA,
		KEY_RSA, MODP_3072_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_AES_CBC, 32
	},
	{ TLS_DHE_RSA_WITH_AES_256_CBC_SHA256,
		KEY_RSA, MODP_4096_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA2_256_256, ENCR_AES_CBC, 32
	},
	{ TLS_DHE_RSA_WITH_AES_128_GCM_SHA256,
		KEY_RSA, MODP_3072_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_UNDEFINED, ENCR_AES_GCM_ICV16, 16
	},
	{ TLS_DHE_RSA_WITH_AES_256_GCM_SHA384,
		KEY_RSA, MODP_4096_BIT,
		HASH_SHA384, PRF_HMAC_SHA2_384,
		AUTH_UNDEFINED, ENCR_AES_GCM_ICV16, 32
	},
	{ TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA,
		KEY_RSA, MODP_2048_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_CAMELLIA_CBC, 16
	},
	{ TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA256,
		KEY_RSA, MODP_3072_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA2_256_256, ENCR_CAMELLIA_CBC, 16
	},
	{ TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA,
		KEY_RSA, MODP_3072_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_CAMELLIA_CBC, 32
	},
	{ TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA256,
		KEY_RSA, MODP_4096_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA2_256_256, ENCR_CAMELLIA_CBC, 32
	},
	{ TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA,
		KEY_RSA, MODP_2048_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_3DES, 0
	},
	{ TLS_RSA_WITH_AES_128_CBC_SHA,
		KEY_RSA, MODP_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_AES_CBC, 16
	},
	{ TLS_RSA_WITH_AES_128_CBC_SHA256,
		KEY_RSA, MODP_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA2_256_256, ENCR_AES_CBC, 16
	},
	{ TLS_RSA_WITH_AES_256_CBC_SHA,
		KEY_RSA, MODP_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_AES_CBC, 32
	},
	{ TLS_RSA_WITH_AES_256_CBC_SHA256,
		KEY_RSA, MODP_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA2_256_256, ENCR_AES_CBC, 32
	},
	{ TLS_RSA_WITH_AES_128_GCM_SHA256,
		KEY_RSA, MODP_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_UNDEFINED, ENCR_AES_GCM_ICV16, 16
	},
	{ TLS_RSA_WITH_AES_256_GCM_SHA384,
		KEY_RSA, MODP_NONE,
		HASH_SHA384, PRF_HMAC_SHA2_384,
		AUTH_UNDEFINED, ENCR_AES_GCM_ICV16, 32
	},
	{ TLS_RSA_WITH_CAMELLIA_128_CBC_SHA,
		KEY_RSA, MODP_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_CAMELLIA_CBC, 16
	},
	{ TLS_RSA_WITH_CAMELLIA_128_CBC_SHA256,
		KEY_RSA, MODP_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA2_256_256, ENCR_CAMELLIA_CBC, 16
	},
	{ TLS_RSA_WITH_CAMELLIA_256_CBC_SHA,
		KEY_RSA, MODP_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_CAMELLIA_CBC, 32
	},
	{ TLS_RSA_WITH_CAMELLIA_256_CBC_SHA256,
		KEY_RSA, MODP_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA2_256_256, ENCR_CAMELLIA_CBC, 32
	},
	{ TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA,
		KEY_ECDSA, ECP_256_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_3DES, 0
	},
	{ TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA,
		KEY_RSA, ECP_256_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_3DES, 0
	},
	{ TLS_RSA_WITH_3DES_EDE_CBC_SHA,
		KEY_RSA, MODP_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_3DES, 0
	},
	{ TLS_ECDHE_ECDSA_WITH_NULL_SHA,
		KEY_ECDSA, ECP_256_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_NULL, 0
	},
	{ TLS_ECDHE_RSA_WITH_NULL_SHA,
		KEY_ECDSA, ECP_256_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_NULL, 0
	},
	{ TLS_RSA_WITH_NULL_SHA,
		KEY_RSA, MODP_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_NULL, 0
	},
	{ TLS_RSA_WITH_NULL_SHA256,
		KEY_RSA, MODP_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA2_256_256, ENCR_NULL, 0
	},
	{ TLS_RSA_WITH_NULL_MD5,
		KEY_RSA, MODP_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_MD5_128, ENCR_NULL, 0
	},
};

/**
 * Look up algorithms by a suite
 */
static suite_algs_t *find_suite(tls_cipher_suite_t suite)
{
	int i;

	for (i = 0; i < countof(suite_algs); i++)
	{
		if (suite_algs[i].suite == suite)
		{
			return &suite_algs[i];
		}
	}
	return NULL;
}

/**
 * Filter a suite list using a transform enumerator
 */
static void filter_suite(suite_algs_t suites[], int *count, int offset,
						 enumerator_t*(*create_enumerator)(crypto_factory_t*))
{
	const char *plugin_name;
	suite_algs_t current;
	int *current_alg, i, remaining = 0;
	enumerator_t *enumerator;

	memset(&current, 0, sizeof(current));
	current_alg = (int*)((char*)&current + offset);

	for (i = 0; i < *count; i++)
	{
		if (create_enumerator == lib->crypto->create_crypter_enumerator &&
			encryption_algorithm_is_aead(suites[i].encr))
		{	/* filtering crypters, but current suite uses an AEAD, apply */
			suites[remaining] = suites[i];
			remaining++;
			continue;
		}
		if (create_enumerator == lib->crypto->create_aead_enumerator &&
			!encryption_algorithm_is_aead(suites[i].encr))
		{	/* filtering AEADs, but current suite doesn't use one, apply */
			suites[remaining] = suites[i];
			remaining++;
			continue;
		}
		enumerator = create_enumerator(lib->crypto);
		while (enumerator->enumerate(enumerator, current_alg, &plugin_name))
		{
			if (current.encr && current.encr != suites[i].encr)
			{
				if (suites[i].encr != ENCR_NULL)
				{	/* skip, ENCR does not match nor is NULL */
					continue;
				}
			}
			if (current.mac && current.mac != suites[i].mac)
			{
				if (suites[i].mac != AUTH_UNDEFINED)
				{	/* skip, MAC does not match nor is it undefined */
					continue;
				}
			}
			if (current.prf && current.prf != suites[i].prf)
			{	/* skip, PRF does not match */
				continue;
			}
			if (current.hash && current.hash != suites[i].hash)
			{	/* skip, hash does not match */
				continue;
			}
			if (current.dh && current.dh != suites[i].dh)
			{
				if (suites[i].dh != MODP_NONE)
				{	/* skip DH group, does not match nor NONE */
					continue;
				}
			}
			/* suite supported, apply */
			suites[remaining] = suites[i];
			remaining++;
			break;
		}
		enumerator->destroy(enumerator);
	}
	*count = remaining;
}

/**
 * Purge NULL encryption cipher suites from list
 */
static void filter_null_suites(suite_algs_t suites[], int *count)
{
	int i, remaining = 0;

	for (i = 0; i < *count; i++)
	{
		if (suites[i].encr != ENCR_NULL)
		{
			suites[remaining] = suites[i];
			remaining++;
		}
	}
	*count = remaining;
}

/**
 * Purge suites using a given key type
 */
static void filter_key_suites(private_tls_crypto_t *this,
							  suite_algs_t suites[], int *count, key_type_t key)
{
	int i, remaining = 0;

	DBG2(DBG_TLS, "disabling %N suites, no backend found", key_type_names, key);
	for (i = 0; i < *count; i++)
	{
		if (suites[i].key != key)
		{
			suites[remaining] = suites[i];
			remaining++;
		}
	}
	*count = remaining;
}

/**
 * Filter suites by key exchange user config
 */
static void filter_key_exchange_config_suites(private_tls_crypto_t *this,
											  suite_algs_t suites[], int *count)
{
	enumerator_t *enumerator;
	int i, remaining = 0;
	char *token, *config;

	config = lib->settings->get_str(lib->settings, "%s.tls.key_exchange", NULL,
									lib->ns);
	if (config)
	{
		for (i = 0; i < *count; i++)
		{
			enumerator = enumerator_create_token(config, ",", " ");
			while (enumerator->enumerate(enumerator, &token))
			{
				if (strcaseeq(token, "ecdhe-ecdsa") &&
					diffie_hellman_group_is_ec(suites[i].dh) &&
					suites[i].key == KEY_ECDSA)
				{
					suites[remaining++] = suites[i];
					break;
				}
				if (strcaseeq(token, "ecdhe-rsa") &&
					diffie_hellman_group_is_ec(suites[i].dh) &&
					suites[i].key == KEY_RSA)
				{
					suites[remaining++] = suites[i];
					break;
				}
				if (strcaseeq(token, "dhe-rsa") &&
					!diffie_hellman_group_is_ec(suites[i].dh) &&
					suites[i].dh != MODP_NONE &&
					suites[i].key == KEY_RSA)
				{
					suites[remaining++] = suites[i];
					break;
				}
				if (strcaseeq(token, "rsa") &&
					suites[i].dh == MODP_NONE &&
					suites[i].key == KEY_RSA)
				{
					suites[remaining++] = suites[i];
					break;
				}
			}
			enumerator->destroy(enumerator);
		}
		*count = remaining;
	}
}

/**
 * Filter suites by cipher user config
 */
static void filter_cipher_config_suites(private_tls_crypto_t *this,
										suite_algs_t suites[], int *count)
{
	enumerator_t *enumerator;
	int i, remaining = 0;
	char *token, *config;

	config = lib->settings->get_str(lib->settings, "%s.tls.cipher", NULL,
									lib->ns);
	if (config)
	{
		for (i = 0; i < *count; i++)
		{
			enumerator = enumerator_create_token(config, ",", " ");
			while (enumerator->enumerate(enumerator, &token))
			{
				if (strcaseeq(token, "aes128") &&
					suites[i].encr == ENCR_AES_CBC &&
					suites[i].encr_size == 16)
				{
					suites[remaining++] = suites[i];
					break;
				}
				if (strcaseeq(token, "aes256") &&
					suites[i].encr == ENCR_AES_CBC &&
					suites[i].encr_size == 32)
				{
					suites[remaining++] = suites[i];
					break;
				}
				if (strcaseeq(token, "aes128gcm") &&
					suites[i].encr == ENCR_AES_GCM_ICV16 &&
					suites[i].encr_size == 16)
				{
					suites[remaining++] = suites[i];
					break;
				}
				if (strcaseeq(token, "aes256gcm") &&
					suites[i].encr == ENCR_AES_GCM_ICV16 &&
					suites[i].encr_size == 32)
				{
					suites[remaining++] = suites[i];
					break;
				}
				if (strcaseeq(token, "camellia128") &&
					suites[i].encr == ENCR_CAMELLIA_CBC &&
					suites[i].encr_size == 16)
				{
					suites[remaining++] = suites[i];
					break;
				}
				if (strcaseeq(token, "camellia256") &&
					suites[i].encr == ENCR_CAMELLIA_CBC &&
					suites[i].encr_size == 32)
				{
					suites[remaining++] = suites[i];
					break;
				}
				if (strcaseeq(token, "3des") &&
					suites[i].encr == ENCR_3DES)
				{
					suites[remaining++] = suites[i];
					break;
				}
				if (strcaseeq(token, "null") &&
					suites[i].encr == ENCR_NULL)
				{
					suites[remaining++] = suites[i];
					break;
				}
			}
			enumerator->destroy(enumerator);
		}
		*count = remaining;
	}
}

/**
 * Filter suites by mac user config
 */
static void filter_mac_config_suites(private_tls_crypto_t *this,
									 suite_algs_t suites[], int *count)
{
	enumerator_t *enumerator;
	int i, remaining = 0;
	char *token, *config;

	config = lib->settings->get_str(lib->settings, "%s.tls.mac", NULL,
									lib->ns);
	if (config)
	{
		for (i = 0; i < *count; i++)
		{
			enumerator = enumerator_create_token(config, ",", " ");
			while (enumerator->enumerate(enumerator, &token))
			{
				if (strcaseeq(token, "md5") &&
					suites[i].mac == AUTH_HMAC_MD5_128)
				{
					suites[remaining++] = suites[i];
					break;
				}
				if (strcaseeq(token, "sha1") &&
					suites[i].mac == AUTH_HMAC_SHA1_160)
				{
					suites[remaining++] = suites[i];
					break;
				}
				if (strcaseeq(token, "sha256") &&
					suites[i].mac == AUTH_HMAC_SHA2_256_256)
				{
					suites[remaining++] = suites[i];
					break;
				}
				if (strcaseeq(token, "sha384") &&
					suites[i].mac == AUTH_HMAC_SHA2_384_384)
				{
					suites[remaining++] = suites[i];
					break;
				}
			}
			enumerator->destroy(enumerator);
		}
		*count = remaining;
	}
}

/**
 * Filter for specific suites specified in strongswan.conf
 */
static void filter_specific_config_suites(private_tls_crypto_t *this,
										  suite_algs_t suites[], int *count)
{
	enumerator_t *enumerator;
	int i, remaining = 0, suite;
	char *token, *config;

	config = lib->settings->get_str(lib->settings, "%s.tls.suites", NULL,
									lib->ns);
	if (config)
	{
		for (i = 0; i < *count; i++)
		{
			enumerator = enumerator_create_token(config, ",", " ");
			while (enumerator->enumerate(enumerator, &token))
			{
				if (enum_from_name(tls_cipher_suite_names, token, &suite) &&
					suite == suites[i].suite)
				{
					suites[remaining++] = suites[i];
					break;
				}
			}
			enumerator->destroy(enumerator);
		}
		*count = remaining;
	}
}

/**
 * Filter out unsupported suites on given suite array
 */
static void filter_unsupported_suites(suite_algs_t suites[], int *count)
{
	/* filter suite list by each algorithm */
	filter_suite(suites, count, offsetof(suite_algs_t, encr),
				 lib->crypto->create_crypter_enumerator);
	filter_suite(suites, count, offsetof(suite_algs_t, encr),
				 lib->crypto->create_aead_enumerator);
	filter_suite(suites, count, offsetof(suite_algs_t, mac),
				 lib->crypto->create_signer_enumerator);
	filter_suite(suites, count, offsetof(suite_algs_t, prf),
				 lib->crypto->create_prf_enumerator);
	filter_suite(suites, count, offsetof(suite_algs_t, hash),
				 lib->crypto->create_hasher_enumerator);
	filter_suite(suites, count, offsetof(suite_algs_t, dh),
				 lib->crypto->create_dh_enumerator);
}

/**
 * Initialize the cipher suite list
 */
static void build_cipher_suite_list(private_tls_crypto_t *this,
									bool require_encryption)
{
	suite_algs_t suites[countof(suite_algs)];
	int count = countof(suite_algs), i;

	/* copy all suites */
	for (i = 0; i < count; i++)
	{
		suites[i] = suite_algs[i];
	}

	if (require_encryption)
	{
		filter_null_suites(suites, &count);
	}
	if (!this->rsa)
	{
		filter_key_suites(this, suites, &count, KEY_RSA);
	}
	if (!this->ecdsa)
	{
		filter_key_suites(this, suites, &count, KEY_ECDSA);
	}

	filter_unsupported_suites(suites, &count);

	/* filter suites with strongswan.conf options */
	filter_key_exchange_config_suites(this, suites, &count);
	filter_cipher_config_suites(this, suites, &count);
	filter_mac_config_suites(this, suites, &count);
	filter_specific_config_suites(this, suites, &count);

	free(this->suites);
	this->suite_count = count;
	this->suites = malloc(sizeof(tls_cipher_suite_t) * count);

	DBG2(DBG_TLS, "%d supported TLS cipher suites:", count);
	for (i = 0; i < count; i++)
	{
		DBG2(DBG_TLS, "  %N", tls_cipher_suite_names, suites[i].suite);
		this->suites[i] = suites[i].suite;
	}
}

METHOD(tls_crypto_t, get_cipher_suites, int,
	private_tls_crypto_t *this, tls_cipher_suite_t **suites)
{
	*suites = this->suites;
	return this->suite_count;
}

/**
 * Create NULL encryption transforms
 */
static bool create_null(private_tls_crypto_t *this, suite_algs_t *algs)
{
	this->aead_in = tls_aead_create_null(algs->mac);
	this->aead_out = tls_aead_create_null(algs->mac);
	if (!this->aead_in || !this->aead_out)
	{
		DBG1(DBG_TLS, "selected TLS MAC %N not supported",
			 integrity_algorithm_names, algs->mac);
		return FALSE;
	}
	return TRUE;
}

/**
 * Create traditional transforms
 */
static bool create_traditional(private_tls_crypto_t *this, suite_algs_t *algs)
{
	if (this->tls->get_version(this->tls) < TLS_1_1)
	{
		this->aead_in = tls_aead_create_implicit(algs->mac,
								algs->encr, algs->encr_size);
		this->aead_out = tls_aead_create_implicit(algs->mac,
								algs->encr, algs->encr_size);
	}
	else
	{
		this->aead_in = tls_aead_create_explicit(algs->mac,
								algs->encr, algs->encr_size);
		this->aead_out = tls_aead_create_explicit(algs->mac,
								algs->encr, algs->encr_size);
	}
	if (!this->aead_in || !this->aead_out)
	{
		DBG1(DBG_TLS, "selected TLS transforms %N-%u-%N not supported",
			 encryption_algorithm_names, algs->encr, algs->encr_size * 8,
			 integrity_algorithm_names, algs->mac);
		return FALSE;
	}
	return TRUE;
}

/**
 * Create AEAD transforms
 */
static bool create_aead(private_tls_crypto_t *this, suite_algs_t *algs)
{
	this->aead_in = tls_aead_create_aead(algs->encr, algs->encr_size);
	this->aead_out = tls_aead_create_aead(algs->encr, algs->encr_size);
	if (!this->aead_in || !this->aead_out)
	{
		DBG1(DBG_TLS, "selected TLS transforms %N-%u not supported",
			 encryption_algorithm_names, algs->encr, algs->encr_size * 8);
		return FALSE;
	}
	return TRUE;
}

/**
 * Clean up and unset AEAD transforms
 */
static void destroy_aeads(private_tls_crypto_t *this)
{
	DESTROY_IF(this->aead_in);
	DESTROY_IF(this->aead_out);
	this->aead_in = this->aead_out = NULL;
}

/**
 * Create crypto primitives
 */
static bool create_ciphers(private_tls_crypto_t *this, suite_algs_t *algs)
{
	destroy_aeads(this);
	DESTROY_IF(this->prf);
	if (this->tls->get_version(this->tls) < TLS_1_2)
	{
		this->prf = tls_prf_create_10();
	}
	else
	{
		this->prf = tls_prf_create_12(algs->prf);
	}
	if (!this->prf)
	{
		DBG1(DBG_TLS, "selected TLS PRF not supported");
		return FALSE;
	}
	if (algs->encr == ENCR_NULL)
	{
		if (create_null(this, algs))
		{
			return TRUE;
		}
	}
	else if (encryption_algorithm_is_aead(algs->encr))
	{
		if (create_aead(this, algs))
		{
			return TRUE;
		}
	}
	else
	{
		if (create_traditional(this, algs))
		{
			return TRUE;
		}
	}
	destroy_aeads(this);
	return FALSE;
}

METHOD(tls_crypto_t, select_cipher_suite, tls_cipher_suite_t,
	private_tls_crypto_t *this, tls_cipher_suite_t *suites, int count,
	key_type_t key)
{
	suite_algs_t *algs;
	int i, j;

	for (i = 0; i < this->suite_count; i++)
	{
		for (j = 0; j < count; j++)
		{
			if (this->suites[i] == suites[j])
			{
				algs = find_suite(this->suites[i]);
				if (algs)
				{
					if (key == KEY_ANY || key == algs->key)
					{
						if (create_ciphers(this, algs))
						{
							this->suite = this->suites[i];
							return this->suite;
						}
					}
				}
			}
		}
	}
	return 0;
}

METHOD(tls_crypto_t, get_dh_group, diffie_hellman_group_t,
	private_tls_crypto_t *this)
{
	suite_algs_t *algs;

	algs = find_suite(this->suite);
	if (algs)
	{
		return algs->dh;
	}
	return MODP_NONE;
}

/**
 * Map signature schemes to TLS key types and hashes, ordered by preference
 */
static struct {
	tls_signature_algorithm_t sig;
	tls_hash_algorithm_t hash;
	signature_scheme_t scheme;
} schemes[] = {
	{ TLS_SIG_ECDSA,	TLS_HASH_SHA256,	SIGN_ECDSA_WITH_SHA256_DER   },
	{ TLS_SIG_ECDSA,	TLS_HASH_SHA384,	SIGN_ECDSA_WITH_SHA384_DER   },
	{ TLS_SIG_ECDSA,	TLS_HASH_SHA512,	SIGN_ECDSA_WITH_SHA512_DER   },
	{ TLS_SIG_ECDSA,	TLS_HASH_SHA1,		SIGN_ECDSA_WITH_SHA1_DER     },
	{ TLS_SIG_RSA,		TLS_HASH_SHA256,	SIGN_RSA_EMSA_PKCS1_SHA2_256 },
	{ TLS_SIG_RSA,		TLS_HASH_SHA384,	SIGN_RSA_EMSA_PKCS1_SHA2_384 },
	{ TLS_SIG_RSA,		TLS_HASH_SHA512,	SIGN_RSA_EMSA_PKCS1_SHA2_512 },
	{ TLS_SIG_RSA,		TLS_HASH_SHA224,	SIGN_RSA_EMSA_PKCS1_SHA2_224 },
	{ TLS_SIG_RSA,		TLS_HASH_SHA1,		SIGN_RSA_EMSA_PKCS1_SHA1     },
	{ TLS_SIG_RSA,		TLS_HASH_MD5,		SIGN_RSA_EMSA_PKCS1_MD5      },
};

METHOD(tls_crypto_t, get_signature_algorithms, void,
	private_tls_crypto_t *this, bio_writer_t *writer)
{
	bio_writer_t *supported;
	int i;

	supported = bio_writer_create(32);

	for (i = 0; i < countof(schemes); i++)
	{
		if (schemes[i].sig == TLS_SIG_RSA && !this->rsa)
		{
			continue;
		}
		if (schemes[i].sig == TLS_SIG_ECDSA && !this->ecdsa)
		{
			continue;
		}
		if (!lib->plugins->has_feature(lib->plugins,
						PLUGIN_PROVIDE(PUBKEY_VERIFY, schemes[i].scheme)))
		{
			continue;
		}
		supported->write_uint8(supported, schemes[i].hash);
		supported->write_uint8(supported, schemes[i].sig);
	}

	supported->wrap16(supported);
	writer->write_data16(writer, supported->get_buf(supported));
	supported->destroy(supported);
}

/**
 * Get the signature scheme from a TLS 1.2 hash/sig algorithm pair
 */
static signature_scheme_t hashsig_to_scheme(key_type_t type,
											tls_hash_algorithm_t hash,
											tls_signature_algorithm_t sig)
{
	int i;

	if ((sig == TLS_SIG_RSA && type == KEY_RSA) ||
		(sig == TLS_SIG_ECDSA && type == KEY_ECDSA))
	{
		for (i = 0; i < countof(schemes); i++)
		{
			if (schemes[i].sig == sig && schemes[i].hash == hash)
			{
				return schemes[i].scheme;
			}
		}
	}
	return SIGN_UNKNOWN;
}

/**
 * Mapping groups to TLS named curves
 */
static struct {
	diffie_hellman_group_t group;
	tls_named_curve_t curve;
} curves[] = {
	{ ECP_256_BIT, TLS_SECP256R1},
	{ ECP_384_BIT, TLS_SECP384R1},
	{ ECP_521_BIT, TLS_SECP521R1},
	{ ECP_224_BIT, TLS_SECP224R1},
	{ ECP_192_BIT, TLS_SECP192R1},
};

CALLBACK(group_filter, bool,
	void *null, enumerator_t *orig, va_list args)
{
	diffie_hellman_group_t group, *out;
	tls_named_curve_t *curve;
	char *plugin;
	int i;

	VA_ARGS_VGET(args, out, curve);

	while (orig->enumerate(orig, &group, &plugin))
	{
		for (i = 0; i < countof(curves); i++)
		{
			if (curves[i].group == group)
			{
				if (out)
				{
					*out = curves[i].group;
				}
				if (curve)
				{
					*curve = curves[i].curve;
				}
				return TRUE;
			}
		}
	}
	return FALSE;
}

METHOD(tls_crypto_t, create_ec_enumerator, enumerator_t*,
	private_tls_crypto_t *this)
{
	return enumerator_create_filter(
							lib->crypto->create_dh_enumerator(lib->crypto),
							group_filter, NULL, NULL);
}

METHOD(tls_crypto_t, set_protection, void,
	private_tls_crypto_t *this, tls_protection_t *protection)
{
	this->protection = protection;
}

METHOD(tls_crypto_t, append_handshake, void,
	private_tls_crypto_t *this, tls_handshake_type_t type, chunk_t data)
{
	uint32_t header;

	/* reconstruct handshake header */
	header = htonl(data.len | (type << 24));
	this->handshake = chunk_cat("mcc", this->handshake,
								chunk_from_thing(header), data);
}

/**
 * Create a hash using the suites HASH algorithm
 */
static bool hash_data(private_tls_crypto_t *this, chunk_t data, chunk_t *hash)
{
	if (this->tls->get_version(this->tls) >= TLS_1_2)
	{
		hasher_t *hasher;
		suite_algs_t *alg;

		alg = find_suite(this->suite);
		if (!alg)
		{
			return FALSE;
		}
		hasher = lib->crypto->create_hasher(lib->crypto, alg->hash);
		if (!hasher || !hasher->allocate_hash(hasher, data, hash))
		{
			DBG1(DBG_TLS, "%N not supported", hash_algorithm_names, alg->hash);
			DESTROY_IF(hasher);
			return FALSE;
		}
		hasher->destroy(hasher);
	}
	else
	{
		hasher_t *md5, *sha1;
		char buf[HASH_SIZE_MD5 + HASH_SIZE_SHA1];

		md5 = lib->crypto->create_hasher(lib->crypto, HASH_MD5);
		if (!md5 || !md5->get_hash(md5, data, buf))
		{
			DBG1(DBG_TLS, "%N not supported", hash_algorithm_names, HASH_MD5);
			DESTROY_IF(md5);
			return FALSE;
		}
		md5->destroy(md5);
		sha1 = lib->crypto->create_hasher(lib->crypto, HASH_SHA1);
		if (!sha1 || !sha1->get_hash(sha1, data, buf + HASH_SIZE_MD5))
		{
			DBG1(DBG_TLS, "%N not supported", hash_algorithm_names, HASH_SHA1);
			DESTROY_IF(sha1);
			return FALSE;
		}
		sha1->destroy(sha1);

		*hash = chunk_clone(chunk_from_thing(buf));
	}
	return TRUE;
}

METHOD(tls_crypto_t, sign, bool,
	private_tls_crypto_t *this, private_key_t *key, bio_writer_t *writer,
	chunk_t data, chunk_t hashsig)
{
	if (this->tls->get_version(this->tls) >= TLS_1_2)
	{
		signature_scheme_t scheme;
		bio_reader_t *reader;
		uint8_t hash, alg;
		chunk_t sig;
		bool done = FALSE;

		if (!hashsig.len)
		{	/* fallback if none given */
			hashsig = chunk_from_chars(
				TLS_HASH_SHA1, TLS_SIG_RSA, TLS_HASH_SHA1, TLS_SIG_ECDSA);
		}
		reader = bio_reader_create(hashsig);
		while (reader->remaining(reader) >= 2)
		{
			if (reader->read_uint8(reader, &hash) &&
				reader->read_uint8(reader, &alg))
			{
				scheme = hashsig_to_scheme(key->get_type(key), hash, alg);
				if (scheme != SIGN_UNKNOWN &&
					key->sign(key, scheme, NULL, data, &sig))
				{
					done = TRUE;
					break;
				}
			}
		}
		reader->destroy(reader);
		if (!done)
		{
			DBG1(DBG_TLS, "none of the proposed hash/sig algorithms supported");
			return FALSE;
		}
		DBG2(DBG_TLS, "created signature with %N/%N",
			 tls_hash_algorithm_names, hash, tls_signature_algorithm_names, alg);
		writer->write_uint8(writer, hash);
		writer->write_uint8(writer, alg);
		writer->write_data16(writer, sig);
		free(sig.ptr);
	}
	else
	{
		chunk_t sig, hash;
		bool done;

		switch (key->get_type(key))
		{
			case KEY_RSA:
				if (!hash_data(this, data, &hash))
				{
					return FALSE;
				}
				done = key->sign(key, SIGN_RSA_EMSA_PKCS1_NULL, NULL, hash,
								 &sig);
				free(hash.ptr);
				if (!done)
				{
					return FALSE;
				}
				DBG2(DBG_TLS, "created signature with MD5+SHA1/RSA");
				break;
			case KEY_ECDSA:
				if (!key->sign(key, SIGN_ECDSA_WITH_SHA1_DER, NULL, data, &sig))
				{
					return FALSE;
				}
				DBG2(DBG_TLS, "created signature with SHA1/ECDSA");
				break;
			default:
				return FALSE;
		}
		writer->write_data16(writer, sig);
		free(sig.ptr);
	}
	return TRUE;
}

METHOD(tls_crypto_t, verify, bool,
	private_tls_crypto_t *this, public_key_t *key, bio_reader_t *reader,
	chunk_t data)
{
	if (this->tls->get_version(this->tls) >= TLS_1_2)
	{
		signature_scheme_t scheme = SIGN_UNKNOWN;
		uint8_t hash, alg;
		chunk_t sig;

		if (!reader->read_uint8(reader, &hash) ||
			!reader->read_uint8(reader, &alg) ||
			!reader->read_data16(reader, &sig))
		{
			DBG1(DBG_TLS, "received invalid signature");
			return FALSE;
		}
		scheme = hashsig_to_scheme(key->get_type(key), hash, alg);
		if (scheme == SIGN_UNKNOWN)
		{
			DBG1(DBG_TLS, "signature algorithms %N/%N not supported",
				 tls_hash_algorithm_names, hash,
				 tls_signature_algorithm_names, alg);
			return FALSE;
		}
		if (!key->verify(key, scheme, NULL, data, sig))
		{
			return FALSE;
		}
		DBG2(DBG_TLS, "verified signature with %N/%N",
			 tls_hash_algorithm_names, hash, tls_signature_algorithm_names, alg);
	}
	else
	{
		chunk_t sig, hash;
		bool done;

		if (!reader->read_data16(reader, &sig))
		{
			DBG1(DBG_TLS, "received invalid signature");
			return FALSE;
		}
		switch (key->get_type(key))
		{
			case KEY_RSA:
				if (!hash_data(this, data, &hash))
				{
					return FALSE;
				}
				done = key->verify(key, SIGN_RSA_EMSA_PKCS1_NULL, NULL, hash,
								   sig);
				free(hash.ptr);
				if (!done)
				{
					return FALSE;
				}
				DBG2(DBG_TLS, "verified signature data with MD5+SHA1/RSA");
				break;
			case KEY_ECDSA:
				if (!key->verify(key, SIGN_ECDSA_WITH_SHA1_DER, NULL, data,
								 sig))
				{
					return FALSE;
				}
				DBG2(DBG_TLS, "verified signature with SHA1/ECDSA");
				break;
			default:
				return FALSE;
		}
	}
	return TRUE;
}

METHOD(tls_crypto_t, sign_handshake, bool,
	private_tls_crypto_t *this, private_key_t *key, bio_writer_t *writer,
	chunk_t hashsig)
{
	return sign(this, key, writer, this->handshake, hashsig);
}

METHOD(tls_crypto_t, verify_handshake, bool,
	private_tls_crypto_t *this, public_key_t *key, bio_reader_t *reader)
{
	return verify(this, key, reader, this->handshake);
}

METHOD(tls_crypto_t, calculate_finished, bool,
	private_tls_crypto_t *this, char *label, char out[12])
{
	chunk_t seed;

	if (!this->prf)
	{
		return FALSE;
	}
	if (!hash_data(this, this->handshake, &seed))
	{
		return FALSE;
	}
	if (!this->prf->get_bytes(this->prf, label, seed, 12, out))
	{
		free(seed.ptr);
		return FALSE;
	}
	free(seed.ptr);
	return TRUE;
}

/**
 * Derive master secret from premaster, optionally save session
 */
static bool derive_master(private_tls_crypto_t *this, chunk_t premaster,
						  chunk_t session, identification_t *id,
						  chunk_t client_random, chunk_t server_random)
{
	char master[48];
	chunk_t seed;

	/* derive master secret */
	seed = chunk_cata("cc", client_random, server_random);

	if (!this->prf->set_key(this->prf, premaster) ||
		!this->prf->get_bytes(this->prf, "master secret", seed,
							  sizeof(master), master) ||
		!this->prf->set_key(this->prf, chunk_from_thing(master)))
	{
		return FALSE;
	}

	if (this->cache && session.len)
	{
		this->cache->create(this->cache, session, id, chunk_from_thing(master),
							this->suite);
	}
	memwipe(master, sizeof(master));
	return TRUE;
}

/**
 * Expand key material from master secret
 */
static bool expand_keys(private_tls_crypto_t *this,
						chunk_t client_random, chunk_t server_random)
{
	chunk_t seed, block;
	chunk_t cw_mac, cw, cw_iv;
	chunk_t sw_mac, sw, sw_iv;
	int mklen, eklen, ivlen;

	if (!this->aead_in || !this->aead_out)
	{
		return FALSE;
	}

	/* derive key block for key expansion */
	mklen = this->aead_in->get_mac_key_size(this->aead_in);
	eklen = this->aead_in->get_encr_key_size(this->aead_in);
	ivlen = this->aead_in->get_iv_size(this->aead_in);
	seed = chunk_cata("cc", server_random, client_random);
	block = chunk_alloca((mklen + eklen + ivlen) * 2);
	if (!this->prf->get_bytes(this->prf, "key expansion", seed,
							  block.len, block.ptr))
	{
		return FALSE;
	}

	/* client/server write signer keys */
	cw_mac = chunk_create(block.ptr, mklen);
	block = chunk_skip(block, mklen);
	sw_mac = chunk_create(block.ptr, mklen);
	block = chunk_skip(block, mklen);

	/* client/server write encryption keys */
	cw = chunk_create(block.ptr, eklen);
	block = chunk_skip(block, eklen);
	sw = chunk_create(block.ptr, eklen);
	block = chunk_skip(block, eklen);

	/* client/server write IV; TLS 1.0 implicit IVs or AEAD salt, if any */
	cw_iv = chunk_create(block.ptr, ivlen);
	block = chunk_skip(block, ivlen);
	sw_iv = chunk_create(block.ptr, ivlen);
	block = chunk_skip(block, ivlen);

	if (this->tls->is_server(this->tls))
	{
		if (!this->aead_in->set_keys(this->aead_in, cw_mac, cw, cw_iv) ||
			!this->aead_out->set_keys(this->aead_out, sw_mac, sw, sw_iv))
		{
			return FALSE;
		}
	}
	else
	{
		if (!this->aead_out->set_keys(this->aead_out, cw_mac, cw, cw_iv) ||
			!this->aead_in->set_keys(this->aead_in, sw_mac, sw, sw_iv))
		{
			return FALSE;
		}
	}

	/* EAP-MSK */
	if (this->msk_label)
	{
		seed = chunk_cata("cc", client_random, server_random);
		this->msk = chunk_alloc(64);
		if (!this->prf->get_bytes(this->prf, this->msk_label, seed,
								  this->msk.len, this->msk.ptr))
		{
			return FALSE;
		}
	}
	return TRUE;
}

METHOD(tls_crypto_t, derive_secrets, bool,
	private_tls_crypto_t *this, chunk_t premaster, chunk_t session,
	identification_t *id, chunk_t client_random, chunk_t server_random)
{
	return derive_master(this, premaster, session, id,
						 client_random, server_random) &&
		   expand_keys(this, client_random, server_random);
}

METHOD(tls_crypto_t, resume_session, tls_cipher_suite_t,
	private_tls_crypto_t *this, chunk_t session, identification_t *id,
	chunk_t client_random, chunk_t server_random)
{
	chunk_t master;

	if (this->cache && session.len)
	{
		this->suite = this->cache->lookup(this->cache, session, id, &master);
		if (this->suite)
		{
			this->suite = select_cipher_suite(this, &this->suite, 1, KEY_ANY);
			if (this->suite)
			{
				if (!this->prf->set_key(this->prf, master) ||
					!expand_keys(this, client_random, server_random))
				{
					this->suite = 0;
				}
			}
			chunk_clear(&master);
		}
		return this->suite;
	}
	return 0;
}

METHOD(tls_crypto_t, get_session, chunk_t,
	private_tls_crypto_t *this, identification_t *server)
{
	if (this->cache)
	{
		return this->cache->check(this->cache, server);
	}
	return chunk_empty;
}

METHOD(tls_crypto_t, change_cipher, void,
	private_tls_crypto_t *this, bool inbound)
{
	if (this->protection)
	{
		if (inbound)
		{
			this->protection->set_cipher(this->protection, TRUE, this->aead_in);
		}
		else
		{
			this->protection->set_cipher(this->protection, FALSE, this->aead_out);
		}
	}
}

METHOD(tls_crypto_t, get_eap_msk, chunk_t,
	private_tls_crypto_t *this)
{
	return this->msk;
}

METHOD(tls_crypto_t, destroy, void,
	private_tls_crypto_t *this)
{
	destroy_aeads(this);
	free(this->handshake.ptr);
	free(this->msk.ptr);
	DESTROY_IF(this->prf);
	free(this->suites);
	free(this);
}

/**
 * See header
 */
tls_crypto_t *tls_crypto_create(tls_t *tls, tls_cache_t *cache)
{
	private_tls_crypto_t *this;
	enumerator_t *enumerator;
	credential_type_t type;
	int subtype;

	INIT(this,
		.public = {
			.get_cipher_suites = _get_cipher_suites,
			.select_cipher_suite = _select_cipher_suite,
			.get_dh_group = _get_dh_group,
			.get_signature_algorithms = _get_signature_algorithms,
			.create_ec_enumerator = _create_ec_enumerator,
			.set_protection = _set_protection,
			.append_handshake = _append_handshake,
			.sign = _sign,
			.verify = _verify,
			.sign_handshake = _sign_handshake,
			.verify_handshake = _verify_handshake,
			.calculate_finished = _calculate_finished,
			.derive_secrets = _derive_secrets,
			.resume_session = _resume_session,
			.get_session = _get_session,
			.change_cipher = _change_cipher,
			.get_eap_msk = _get_eap_msk,
			.destroy = _destroy,
		},
		.tls = tls,
		.cache = cache,
	);

	enumerator = lib->creds->create_builder_enumerator(lib->creds);
	while (enumerator->enumerate(enumerator, &type, &subtype))
	{
		if (type == CRED_PUBLIC_KEY)
		{
			switch (subtype)
			{
				case KEY_RSA:
					this->rsa = TRUE;
					break;
				case KEY_ECDSA:
					this->ecdsa = TRUE;
					break;
				default:
					break;
			}
		}
	}
	enumerator->destroy(enumerator);

	switch (tls->get_purpose(tls))
	{
		case TLS_PURPOSE_EAP_TLS:
			/* MSK PRF ASCII constant label according to EAP-TLS RFC 5216 */
			this->msk_label = "client EAP encryption";
			build_cipher_suite_list(this, FALSE);
			break;
		case TLS_PURPOSE_EAP_PEAP:
			this->msk_label = "client EAP encryption";
			build_cipher_suite_list(this, TRUE);
			break;
		case TLS_PURPOSE_EAP_TTLS:
			/* MSK PRF ASCII constant label according to EAP-TTLS RFC 5281 */
			this->msk_label = "ttls keying material";
			build_cipher_suite_list(this, TRUE);
			break;
		case TLS_PURPOSE_GENERIC:
			build_cipher_suite_list(this, TRUE);
			break;
		case TLS_PURPOSE_GENERIC_NULLOK:
			build_cipher_suite_list(this, FALSE);
			break;
		default:
			break;
	}
	return &this->public;
}

/**
 * See header.
 */
int tls_crypto_get_supported_suites(bool null, tls_cipher_suite_t **out)
{
	suite_algs_t suites[countof(suite_algs)];
	int count = countof(suite_algs), i;

	/* initialize copy of suite list */
	for (i = 0; i < count; i++)
	{
		suites[i] = suite_algs[i];
	}

	filter_unsupported_suites(suites, &count);

	if (!null)
	{
		filter_null_suites(suites, &count);
	}

	if (out)
	{
		*out = calloc(count, sizeof(tls_cipher_suite_t));
		for (i = 0; i < count; i++)
		{
			(*out)[i] = suites[i].suite;
		}
	}
	return count;
}
