/*
 * Copyright (C) 2020 Tobias Brunner
 * Copyright (C) 2020-2021 Pascal Knecht
 * Copyright (C) 2020 Méline Sieber
 * Copyright (C) 2010-2014 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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
#include "tls_hkdf.h"

#include <utils/debug.h>
#include <plugins/plugin_feature.h>
#include <collections/hashtable.h>
#include <collections/array.h>

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
ENUM_NEXT(tls_cipher_suite_names, TLS_AES_128_GCM_SHA256,
								  TLS_AES_128_CCM_8_SHA256,
								  TLS_EMPTY_RENEGOTIATION_INFO_SCSV,
	"TLS_AES_128_GCM_SHA256",
	"TLS_AES_256_GCM_SHA384",
	"TLS_CHACHA20_POLY1305_SHA256",
	"TLS_AES_128_CCM_SHA256",
	"TLS_AES_128_CCM_8_SHA256");
ENUM_NEXT(tls_cipher_suite_names, TLS_ECDH_ECDSA_WITH_NULL_SHA,
								  TLS_ECDHE_PSK_WITH_NULL_SHA384,
								  TLS_AES_128_CCM_8_SHA256,
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
ENUM_NEXT(tls_cipher_suite_names, TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256,
		TLS_DHE_RSA_WITH_CHACHA20_POLY1305_SHA256,
		TLS_ECDHE_PSK_WITH_NULL_SHA384,
	"TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256",
	"TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256",
	"TLS_DHE_RSA_WITH_CHACHA20_POLY1305_SHA256");
ENUM_END(tls_cipher_suite_names, TLS_DHE_RSA_WITH_CHACHA20_POLY1305_SHA256);


ENUM(tls_hash_algorithm_names, TLS_HASH_NONE, TLS_HASH_SHA512,
	"NONE",
	"MD5",
	"SHA1",
	"SHA224",
	"SHA256",
	"SHA384",
	"SHA512",
);

ENUM_BEGIN(tls_signature_scheme_names,
		   TLS_SIG_RSA_PKCS1_SHA1, TLS_SIG_RSA_PKCS1_SHA1,
	"RSA_PKCS1_SHA1");
ENUM_NEXT(tls_signature_scheme_names,
		  TLS_SIG_ECDSA_SHA1, TLS_SIG_ECDSA_SHA1, TLS_SIG_RSA_PKCS1_SHA1,
	"ECDSA_SHA1");
ENUM_NEXT(tls_signature_scheme_names,
		  TLS_SIG_RSA_PKCS1_SHA224, TLS_SIG_ECDSA_SHA224, TLS_SIG_ECDSA_SHA1,
	"RSA_PKCS1_SHA224",
	"DSA_SHA224",
	"ECDSA_SHA224");
ENUM_NEXT(tls_signature_scheme_names,
		  TLS_SIG_RSA_PKCS1_SHA256, TLS_SIG_ECDSA_SHA256, TLS_SIG_ECDSA_SHA224,
	"RSA_PKCS1_SHA256",
	"DSA_SHA256",
	"ECDSA_SHA256");
ENUM_NEXT(tls_signature_scheme_names,
		  TLS_SIG_RSA_PKCS1_SHA384, TLS_SIG_ECDSA_SHA384, TLS_SIG_ECDSA_SHA256,
	"RSA_PKCS1_SHA384",
	"DSA_SHA384",
	"ECDSA_SHA384");
ENUM_NEXT(tls_signature_scheme_names,
		  TLS_SIG_RSA_PKCS1_SHA512, TLS_SIG_ECDSA_SHA512, TLS_SIG_ECDSA_SHA384,
	"RSA_PKCS1_SHA512",
	"DSA_SHA512",
	"ECDSA_SHA512");
ENUM_NEXT(tls_signature_scheme_names,
		  TLS_SIG_RSA_PSS_RSAE_SHA256, TLS_SIG_RSA_PSS_PSS_SHA512, TLS_SIG_ECDSA_SHA512,
	"RSA_PSS_RSAE_SHA256",
	"RSA_PSS_RSAE_SHA384",
	"RSA_PSS_RSAE_SHA512",
	"ED25519",
	"ED448",
	"RSA_PSS_PSS_SHA256",
	"RSA_PSS_PSS_SHA384",
	"RSA_PSS_PSS_SHA512",
);
ENUM_END(tls_signature_scheme_names, TLS_SIG_RSA_PSS_PSS_SHA512);

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

ENUM_BEGIN(tls_named_group_names, TLS_SECT163K1, TLS_SECP521R1,
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
ENUM_NEXT(tls_named_group_names, TLS_CURVE25519, TLS_CURVE448, TLS_SECP521R1,
	"CURVE25519",
	"CURVE448",
);
ENUM_NEXT(tls_named_group_names, TLS_FFDHE2048, TLS_FFDHE8192, TLS_CURVE448,
	"FFDHE2048",
	"FFDHE3072",
	"FFDHE4096",
	"FFDHE6144",
	"FFDHE8192",
);
ENUM_END(tls_named_group_names, TLS_FFDHE8192);

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
	 * HKDF for TLS 1.3
	 */
	tls_hkdf_t *hkdf;

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
	key_exchange_method_t dh;
	hash_algorithm_t hash;
	pseudo_random_function_t prf;
	integrity_algorithm_t mac;
	encryption_algorithm_t encr;
	size_t encr_size;
	tls_version_t min_version;
	tls_version_t max_version;
} suite_algs_t;

/**
 * Mapping suites to a set of algorithms
 *
 * The order represents the descending preference of cipher suites and follows
 * this rule set:
 *
 *   1. TLS 1.3 > Legacy TLS
 *   2. AES > CAMELLIA > NULL
 *   3. AES256 > AES128
 *   4. GCM > CBC
 *   5. ECDHE > DHE > NULL
 *   6. ECDSA > RSA
 *   7. SHA384 > SHA256 > SHA1
 *
 */
static suite_algs_t suite_algs[] = {
	/* Cipher suites of TLS 1.3: key exchange and authentication
	 * delegated to extensions, therefore KEY_ANY, KE_NONE, PRF_UNDEFINED */
	{ TLS_AES_256_GCM_SHA384,
		KEY_ANY, KE_NONE,
		HASH_SHA384, PRF_UNDEFINED,
		AUTH_HMAC_SHA2_384_384, ENCR_AES_GCM_ICV16, 32,
		TLS_1_3, TLS_1_3,
	},
	{ TLS_AES_128_GCM_SHA256,
		KEY_ANY, KE_NONE,
		HASH_SHA256, PRF_UNDEFINED,
		AUTH_HMAC_SHA2_256_256, ENCR_AES_GCM_ICV16, 16,
		TLS_1_3, TLS_1_3,
	},
	{ TLS_CHACHA20_POLY1305_SHA256,
		KEY_ANY, KE_NONE,
		HASH_SHA256, PRF_UNDEFINED,
		AUTH_HMAC_SHA2_256_256, ENCR_CHACHA20_POLY1305, 32,
		TLS_1_3, TLS_1_3,
	},
	{ TLS_AES_128_CCM_SHA256,
		KEY_ANY, KE_NONE,
		HASH_SHA256, PRF_UNDEFINED,
		AUTH_HMAC_SHA2_256_256, ENCR_AES_CCM_ICV16, 16,
		TLS_1_3, TLS_1_3,
	},
	{ TLS_AES_128_CCM_8_SHA256,
		KEY_ANY, KE_NONE,
		HASH_SHA256, PRF_UNDEFINED,
		AUTH_HMAC_SHA2_256_256, ENCR_AES_CCM_ICV8, 16,
		TLS_1_3, TLS_1_3,
	},
	/* Legacy TLS cipher suites */
	{ TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,
		KEY_ECDSA, ECP_384_BIT,
		HASH_SHA384, PRF_HMAC_SHA2_384,
		AUTH_UNDEFINED, ENCR_AES_GCM_ICV16, 32,
		TLS_1_2, TLS_1_2,
	},
	{ TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384,
		KEY_ECDSA, ECP_384_BIT,
		HASH_SHA384, PRF_HMAC_SHA2_384,
		AUTH_HMAC_SHA2_384_384, ENCR_AES_CBC, 32,
		TLS_1_2, TLS_1_2,
	},
	{ TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA,
		KEY_ECDSA, ECP_384_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_AES_CBC, 32,
		TLS_1_0, TLS_1_2,
	},
	{ TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
		KEY_ECDSA, ECP_256_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_UNDEFINED, ENCR_AES_GCM_ICV16, 16,
		TLS_1_2, TLS_1_2,
	},
	{ TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256,
		KEY_ECDSA, ECP_256_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA2_256_256, ENCR_AES_CBC, 16,
		TLS_1_2, TLS_1_2,
	},
	{ TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA,
		KEY_ECDSA, ECP_256_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_AES_CBC, 16,
		TLS_1_0, TLS_1_2,
	},
	{ TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384,
		KEY_RSA, ECP_384_BIT,
		HASH_SHA384, PRF_HMAC_SHA2_384,
		AUTH_UNDEFINED, ENCR_AES_GCM_ICV16, 32,
		TLS_1_2, TLS_1_2,
	},
	{ TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384,
		KEY_RSA, ECP_384_BIT,
		HASH_SHA384, PRF_HMAC_SHA2_384,
		AUTH_HMAC_SHA2_384_384, ENCR_AES_CBC, 32,
		TLS_1_2, TLS_1_2,
	},
	{ TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA,
		KEY_RSA, ECP_384_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_AES_CBC, 32,
		TLS_1_0, TLS_1_2,
	},
	{ TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,
		KEY_RSA, ECP_256_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_UNDEFINED, ENCR_AES_GCM_ICV16, 16,
		TLS_1_2, TLS_1_2,
	},
	{ TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256,
		KEY_RSA, ECP_256_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA2_256_256, ENCR_AES_CBC, 16,
		TLS_1_2, TLS_1_2,
	},
	{ TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA,
		KEY_RSA, ECP_256_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_AES_CBC, 16,
		TLS_1_0, TLS_1_2,
	},
	{ TLS_DHE_RSA_WITH_AES_256_GCM_SHA384,
		KEY_RSA, MODP_4096_BIT,
		HASH_SHA384, PRF_HMAC_SHA2_384,
		AUTH_UNDEFINED, ENCR_AES_GCM_ICV16, 32,
		TLS_1_2, TLS_1_2,
	},
	{ TLS_DHE_RSA_WITH_AES_256_CBC_SHA256,
		KEY_RSA, MODP_4096_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA2_256_256, ENCR_AES_CBC, 32,
		TLS_1_2, TLS_1_2,
	},
	{ TLS_DHE_RSA_WITH_AES_256_CBC_SHA,
		KEY_RSA, MODP_3072_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_AES_CBC, 32,
		SSL_3_0, TLS_1_2,
	},
	{ TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA256,
		KEY_RSA, MODP_4096_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA2_256_256, ENCR_CAMELLIA_CBC, 32,
		TLS_1_2, TLS_1_2,
	},
	{ TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA,
		KEY_RSA, MODP_3072_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_CAMELLIA_CBC, 32,
		SSL_3_0, TLS_1_2,
	},
	{ TLS_DHE_RSA_WITH_AES_128_GCM_SHA256,
		KEY_RSA, MODP_3072_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_UNDEFINED, ENCR_AES_GCM_ICV16, 16,
		TLS_1_2, TLS_1_2,
	},
	{ TLS_DHE_RSA_WITH_AES_128_CBC_SHA256,
		KEY_RSA, MODP_3072_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA2_256_256, ENCR_AES_CBC, 16,
		TLS_1_2, TLS_1_2,
	},
	{ TLS_DHE_RSA_WITH_AES_128_CBC_SHA,
		KEY_RSA, MODP_2048_BIT,
		HASH_SHA256,PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_AES_CBC, 16,
		SSL_3_0, TLS_1_2,
	},
	{ TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA256,
		KEY_RSA, MODP_3072_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA2_256_256, ENCR_CAMELLIA_CBC, 16,
		TLS_1_2, TLS_1_2,
	},
	{ TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA,
		KEY_RSA, MODP_2048_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_CAMELLIA_CBC, 16,
		SSL_3_0, TLS_1_2,
	},
	{ TLS_RSA_WITH_AES_256_GCM_SHA384,
		KEY_RSA, KE_NONE,
		HASH_SHA384, PRF_HMAC_SHA2_384,
		AUTH_UNDEFINED, ENCR_AES_GCM_ICV16, 32,
		TLS_1_2, TLS_1_2,
	},
	{ TLS_RSA_WITH_AES_256_CBC_SHA256,
		KEY_RSA, KE_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA2_256_256, ENCR_AES_CBC, 32,
		TLS_1_2, TLS_1_2,
	},
	{ TLS_RSA_WITH_AES_256_CBC_SHA,
		KEY_RSA, KE_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_AES_CBC, 32,
		SSL_3_0, TLS_1_2,
	},
	{ TLS_RSA_WITH_AES_128_GCM_SHA256,
		KEY_RSA, KE_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_UNDEFINED, ENCR_AES_GCM_ICV16, 16,
		TLS_1_2, TLS_1_2,
	},
	{ TLS_RSA_WITH_AES_128_CBC_SHA256,
		KEY_RSA, KE_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA2_256_256, ENCR_AES_CBC, 16,
		TLS_1_2, TLS_1_2,
	},
	{ TLS_RSA_WITH_AES_128_CBC_SHA,
		KEY_RSA, KE_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_AES_CBC, 16,
		SSL_3_0, TLS_1_2,
	},
	{ TLS_RSA_WITH_CAMELLIA_256_CBC_SHA256,
		KEY_RSA, KE_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA2_256_256, ENCR_CAMELLIA_CBC, 32,
		TLS_1_2, TLS_1_2,
	},
	{ TLS_RSA_WITH_CAMELLIA_256_CBC_SHA,
		KEY_RSA, KE_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_CAMELLIA_CBC, 32,
		SSL_3_0, TLS_1_2,
	},
	{ TLS_RSA_WITH_CAMELLIA_128_CBC_SHA256,
		KEY_RSA, KE_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA2_256_256, ENCR_CAMELLIA_CBC, 16,
		TLS_1_2, TLS_1_2,
	},
	{ TLS_RSA_WITH_CAMELLIA_128_CBC_SHA,
		KEY_RSA, KE_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_CAMELLIA_CBC, 16,
		SSL_3_0, TLS_1_2,
	},
	{ TLS_ECDHE_ECDSA_WITH_NULL_SHA,
		KEY_ECDSA, ECP_256_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_NULL, 0,
		TLS_1_0, TLS_1_2,
	},
	{ TLS_ECDHE_RSA_WITH_NULL_SHA,
		KEY_ECDSA, ECP_256_BIT,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_NULL, 0,
		TLS_1_0, TLS_1_2,
	},
	{ TLS_RSA_WITH_NULL_SHA256,
		KEY_RSA, KE_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA2_256_256, ENCR_NULL, 0,
		TLS_1_2, TLS_1_2,
	},
	{ TLS_RSA_WITH_NULL_SHA,
		KEY_RSA, KE_NONE,
		HASH_SHA256, PRF_HMAC_SHA2_256,
		AUTH_HMAC_SHA1_160, ENCR_NULL, 0,
		SSL_3_0, TLS_1_2,
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
			{
				if (suites[i].prf != PRF_UNDEFINED)
				{
					/* skip, PRF does not match nor is it undefined */
					continue;
				}
			}
			if (current.hash && current.hash != suites[i].hash)
			{	/* skip, hash does not match */
				continue;
			}
			if (current.dh && current.dh != suites[i].dh)
			{
				if (suites[i].dh != KE_NONE &&
					!(key_exchange_is_ecdh(current.dh) &&
					  key_exchange_is_ecdh(suites[i].dh)))
				{	/* skip DH group, does not match nor NONE nor both ECDH */
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
					key_exchange_is_ecdh(suites[i].dh) &&
					suites[i].key == KEY_ECDSA)
				{
					suites[remaining++] = suites[i];
					break;
				}
				if (strcaseeq(token, "ecdhe-rsa") &&
					key_exchange_is_ecdh(suites[i].dh) &&
					suites[i].key == KEY_RSA)
				{
					suites[remaining++] = suites[i];
					break;
				}
				if (strcaseeq(token, "dhe-rsa") &&
					!key_exchange_is_ecdh(suites[i].dh) &&
					suites[i].dh != KE_NONE &&
					suites[i].key == KEY_RSA)
				{
					suites[remaining++] = suites[i];
					break;
				}
				if (strcaseeq(token, "rsa") &&
					suites[i].dh == KE_NONE &&
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
				const proposal_token_t *tok;

				tok = lib->proposal->get_token(lib->proposal, token);
				if (tok != NULL && tok->type == ENCRYPTION_ALGORITHM &&
					suites[i].encr == tok->algorithm &&
					(!tok->keysize || suites[i].encr_size == tok->keysize / 8))
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
 * Filter key exchange curves by curve user config
 */
static bool filter_curve_config(tls_named_group_t curve)
{
	enumerator_t *enumerator;
	char *token, *config;

	config = lib->settings->get_str(lib->settings, "%s.tls.ke_group", NULL,
									lib->ns);
	if (config)
	{
		enumerator = enumerator_create_token(config, ",", " ");
		while (enumerator->enumerate(enumerator, &token))
		{
			const proposal_token_t *tok;

			tok = lib->proposal->get_token(lib->proposal, token);
			if (tok != NULL && tok->type == KEY_EXCHANGE_METHOD &&
				curve == tls_ec_group_to_curve(tok->algorithm))
			{
				enumerator->destroy(enumerator);
				return TRUE;
			}
		}
		enumerator->destroy(enumerator);
	}
	return !config;
}

/**
 * Filter out unsupported suites on given suite array
 */
static void filter_unsupported_suites(suite_algs_t suites[], int *count)
{
	/* filter suite list by each algorithm */
	filter_suite(suites, count, offsetof(suite_algs_t, encr),
				 lib->crypto->create_aead_enumerator);
	filter_suite(suites, count, offsetof(suite_algs_t, prf),
				 lib->crypto->create_prf_enumerator);
	filter_suite(suites, count, offsetof(suite_algs_t, encr),
				 lib->crypto->create_crypter_enumerator);
	filter_suite(suites, count, offsetof(suite_algs_t, mac),
				 lib->crypto->create_signer_enumerator);
	filter_suite(suites, count, offsetof(suite_algs_t, hash),
				 lib->crypto->create_hasher_enumerator);
	filter_suite(suites, count, offsetof(suite_algs_t, dh),
				 lib->crypto->create_ke_enumerator);
}

/**
 * Initialize the cipher suite list
 */
static void build_cipher_suite_list(private_tls_crypto_t *this)
{
	suite_algs_t suites[countof(suite_algs)] = {};
	tls_version_t min_version, max_version, new_min_version, new_max_version;
	bool require_encryption = TRUE;
	int count = 0, i;

	switch (this->tls->get_purpose(this->tls))
	{
		case TLS_PURPOSE_EAP_TLS:
			require_encryption = FALSE;
			break;
		case TLS_PURPOSE_GENERIC:
			if (this->tls->get_flags(this->tls) & TLS_FLAG_ENCRYPTION_OPTIONAL)
			{
				require_encryption = FALSE;
			}
			break;
		default:
			break;
	}

	min_version = this->tls->get_version_min(this->tls);
	max_version = this->tls->get_version_max(this->tls);

	/* copy all suites appropriate for the current min/max versions */
	for (i = 0; i < countof(suite_algs); i++)
	{
		if (suite_algs[i].min_version <= max_version &&
			suite_algs[i].max_version >= min_version)
		{
			suites[count++] = suite_algs[i];
		}
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
	new_min_version = max_version;
	new_max_version = min_version;
	for (i = 0; i < count; i++)
	{
		DBG2(DBG_TLS, "  %N", tls_cipher_suite_names, suites[i].suite);
		this->suites[i] = suites[i].suite;

		/* set TLS min/max versions appropriate to the final cipher suites */
		new_max_version = max(new_max_version, suites[i].max_version);
		new_min_version = min(new_min_version, suites[i].min_version);
	}
	new_max_version = min(new_max_version, max_version);
	new_min_version = max(new_min_version, min_version);

	if ((min_version != new_min_version || max_version != new_max_version) &&
		this->tls->set_version(this->tls, new_min_version, new_max_version))
	{
		DBG2(DBG_TLS, "TLS min/max %N/%N according to the cipher suites",
			 tls_numeric_version_names, new_min_version,
			 tls_numeric_version_names, new_max_version);
	}
}

METHOD(tls_crypto_t, get_cipher_suites, int,
	private_tls_crypto_t *this, tls_cipher_suite_t **suites)
{
	if (!this->suites)
	{
		build_cipher_suite_list(this);
	}
	if (suites)
	{
		*suites = this->suites;
	}
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
	if (this->tls->get_version_max(this->tls) < TLS_1_1)
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
	if (this->tls->get_version_max(this->tls) < TLS_1_3)
	{
		this->aead_in = tls_aead_create_aead(algs->encr, algs->encr_size);
		this->aead_out = tls_aead_create_aead(algs->encr, algs->encr_size);
	}
	else
	{
		this->aead_in = tls_aead_create_seq(algs->encr, algs->encr_size);
		this->aead_out = tls_aead_create_seq(algs->encr, algs->encr_size);
	}
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
	DESTROY_IF(this->hkdf);
	DESTROY_IF(this->prf);
	if (this->tls->get_version_max(this->tls) < TLS_1_3)
	{
		if (this->tls->get_version_max(this->tls) < TLS_1_2)
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
	}
	else
	{
		this->hkdf = tls_hkdf_create(algs->hash, chunk_empty);
		if (!this->hkdf)
		{
			DBG1(DBG_TLS, "TLS HKDF creation unsuccessful");
			return FALSE;
		}
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
					if (key == KEY_ANY || key == algs->key ||
						(algs->key == KEY_ECDSA && key == KEY_ED25519) ||
						(algs->key == KEY_ECDSA && key == KEY_ED448))
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

/**
 * Parameters for RSA/PSS signature schemes
 */
#define PSS_PARAMS(bits) static rsa_pss_params_t pss_params_sha##bits = { \
	.hash = HASH_SHA##bits, \
	.mgf1_hash = HASH_SHA##bits, \
	.salt_len = HASH_SIZE_SHA##bits, \
}

PSS_PARAMS(256);
PSS_PARAMS(384);
PSS_PARAMS(512);

typedef struct {
	tls_signature_scheme_t sig;
	signature_params_t params;
	/* min/max versions for use in CertificateVerify */
	tls_version_t min_version;
	tls_version_t max_version;
} scheme_algs_t;

/**
 * Map TLS signature schemes, ordered by preference
 */
static scheme_algs_t schemes[] = {
	{ TLS_SIG_ECDSA_SHA256, { .scheme = SIGN_ECDSA_WITH_SHA256_DER },
		TLS_1_0, TLS_1_3 },
	{ TLS_SIG_ECDSA_SHA384, { .scheme = SIGN_ECDSA_WITH_SHA384_DER },
		TLS_1_0, TLS_1_3 },
	{ TLS_SIG_ECDSA_SHA512, { .scheme = SIGN_ECDSA_WITH_SHA512_DER },
		TLS_1_0, TLS_1_3 },
	{ TLS_SIG_ED25519, { .scheme = SIGN_ED25519 },
		TLS_1_0, TLS_1_3 },
	{ TLS_SIG_ED448, { .scheme = SIGN_ED448 },
		TLS_1_0, TLS_1_3 },
	{ TLS_SIG_RSA_PSS_RSAE_SHA256, { .scheme = SIGN_RSA_EMSA_PSS, .params = &pss_params_sha256, },
		TLS_1_2, TLS_1_3 },
	{ TLS_SIG_RSA_PSS_RSAE_SHA384, { .scheme = SIGN_RSA_EMSA_PSS, .params = &pss_params_sha384, },
		TLS_1_2, TLS_1_3 },
	{ TLS_SIG_RSA_PSS_RSAE_SHA512, { .scheme = SIGN_RSA_EMSA_PSS, .params = &pss_params_sha512, },
		TLS_1_2, TLS_1_3 },
	/* the parameters for the next three should actually be taken from the
	 * public key, we currently don't have an API for that, so assume defaults */
	{ TLS_SIG_RSA_PSS_PSS_SHA256, { .scheme = SIGN_RSA_EMSA_PSS, .params = &pss_params_sha256, },
		TLS_1_2, TLS_1_3 },
	{ TLS_SIG_RSA_PSS_PSS_SHA384, { .scheme = SIGN_RSA_EMSA_PSS, .params = &pss_params_sha384, },
		TLS_1_2, TLS_1_3 },
	{ TLS_SIG_RSA_PSS_PSS_SHA512, { .scheme = SIGN_RSA_EMSA_PSS, .params = &pss_params_sha512, },
		TLS_1_2, TLS_1_3 },
	{ TLS_SIG_RSA_PKCS1_SHA256, { .scheme = SIGN_RSA_EMSA_PKCS1_SHA2_256 },
		TLS_1_0, TLS_1_2 },
	{ TLS_SIG_RSA_PKCS1_SHA384, { .scheme = SIGN_RSA_EMSA_PKCS1_SHA2_384 },
		TLS_1_0, TLS_1_2 },
	{ TLS_SIG_RSA_PKCS1_SHA512, { .scheme = SIGN_RSA_EMSA_PKCS1_SHA2_512 },
		TLS_1_0, TLS_1_2 },
};

/**
 * Filter signature scheme config
 */
static bool filter_signature_scheme_config(tls_signature_scheme_t signature)
{
	enumerator_t *enumerator;
	char *token, *config;

	config = lib->settings->get_str(lib->settings, "%s.tls.signature", NULL,
									lib->ns);
	if (config)
	{
		enumerator = enumerator_create_token(config, ",", " ");
		while (enumerator->enumerate(enumerator, &token))
		{
			tls_signature_scheme_t sig;

			if (enum_from_name(tls_signature_scheme_names, token, &sig) &&
				sig == signature)
			{
				enumerator->destroy(enumerator);
				return TRUE;
			}
		}
		enumerator->destroy(enumerator);
	}
	return !config;
}

METHOD(tls_crypto_t, get_signature_algorithms, void,
	private_tls_crypto_t *this, bio_writer_t *writer, bool cert)
{
	bio_writer_t *supported;
	tls_version_t min_version, max_version;
	int i;

	supported = bio_writer_create(32);

	if (!cert)
	{
		min_version = this->tls->get_version_min(this->tls);
		max_version = this->tls->get_version_max(this->tls);
	}

	for (i = 0; i < countof(schemes); i++)
	{
		if ((cert || (schemes[i].min_version <= max_version &&
					  schemes[i].max_version >= min_version)) &&
			lib->plugins->has_feature(lib->plugins,
					PLUGIN_PROVIDE(PUBKEY_VERIFY, schemes[i].params.scheme)) &&
			filter_signature_scheme_config(schemes[i].sig))
		{
			supported->write_uint16(supported, schemes[i].sig);
		}
	}

	writer->write_data16(writer, supported->get_buf(supported));
	supported->destroy(supported);
}

/**
 * Get the signature parameters from a TLS signature scheme
 */
static signature_params_t *params_for_scheme(tls_signature_scheme_t sig,
											 bool sign)
{
	int i;

	for (i = 0; i < countof(schemes); i++)
	{
		/* strongSwan supports only RSA_PSS_RSAE schemes for signing but can
		 * verify public keys in rsaEncryption as well as rsassaPss encoding. */
		if (sign && (sig == TLS_SIG_RSA_PSS_PSS_SHA256 ||
					 sig == TLS_SIG_RSA_PSS_PSS_SHA384 ||
					 sig == TLS_SIG_RSA_PSS_PSS_SHA512))
		{
			continue;
		}
		if (schemes[i].sig == sig)
		{
			return &schemes[i].params;
		}
	}
	return NULL;
}

/**
 * Mapping groups to TLS named curves
 */
static struct {
	key_exchange_method_t group;
	tls_named_group_t curve;
} curves[] = {
	{ ECP_256_BIT, TLS_SECP256R1},
	{ ECP_384_BIT, TLS_SECP384R1},
	{ ECP_521_BIT, TLS_SECP521R1},
	{ ECP_224_BIT, TLS_SECP224R1},
	{ ECP_192_BIT, TLS_SECP192R1},
	{ CURVE_25519, TLS_CURVE25519},
	{ CURVE_448,   TLS_CURVE448},
};

CALLBACK(group_filter, bool,
	void *null, enumerator_t *orig, va_list args)
{
	key_exchange_method_t group, *group_out;
	tls_named_group_t curve, *curve_out;
	char *plugin;

	VA_ARGS_VGET(args, group_out, curve_out);

	while (orig->enumerate(orig, &group, &plugin))
	{
		curve = tls_ec_group_to_curve(group);
		if (curve)
		{
			if (group_out)
			{
				*group_out = group;
			}
			if (curve_out)
			{
				*curve_out = curve;
			}
			return TRUE;
		}
	}
	return FALSE;
}

CALLBACK(config_filter, bool,
	void *null, enumerator_t *orig, va_list args)
{
	key_exchange_method_t group, *group_out;
	tls_named_group_t curve, *curve_out;

	VA_ARGS_VGET(args, group_out, curve_out);

	while (orig->enumerate(orig, &group, &curve))
	{
		if (filter_curve_config(curve))

		{
			if (group_out)
			{
				*group_out = group;
			}
			if (curve_out)
			{
				*curve_out = curve;
			}
			return TRUE;
		}
	}
	return FALSE;
}

METHOD(tls_crypto_t, create_ec_enumerator, enumerator_t*,
	private_tls_crypto_t *this)
{
	return enumerator_create_filter(
							enumerator_create_filter(
								lib->crypto->create_ke_enumerator(lib->crypto),
								group_filter, NULL, NULL),
							config_filter, NULL, NULL);
}

/**
 * Check if the given ECDH group is supported or return the first one we
 * actually do support.
 */
static key_exchange_method_t supported_ec_group(private_tls_crypto_t *this,
												key_exchange_method_t orig)
{
	key_exchange_method_t current, first = KE_NONE;
	enumerator_t *enumerator;

	enumerator = create_ec_enumerator(this);
	while (enumerator->enumerate(enumerator, &current, NULL))
	{
		if (current == orig)
		{
			enumerator->destroy(enumerator);
			return orig;
		}
		else if (first == KE_NONE)
		{
			first = current;
		}
	}
	enumerator->destroy(enumerator);
	return first;
}

METHOD(tls_crypto_t, get_dh_group, key_exchange_method_t,
	private_tls_crypto_t *this)
{
	suite_algs_t *algs;

	algs = find_suite(this->suite);
	if (algs)
	{
		if (key_exchange_is_ecdh(algs->dh))
		{
			return supported_ec_group(this, algs->dh);
		}
		return algs->dh;
	}
	return KE_NONE;
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
	if (this->tls->get_version_max(this->tls) >= TLS_1_2)
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

METHOD(tls_crypto_t, hash_handshake, bool,
	private_tls_crypto_t *this, chunk_t *out)
{
	chunk_t hash;

	if (!hash_data(this, this->handshake, &hash))
	{
		return FALSE;
	}

	chunk_free(&this->handshake);
	append_handshake(this, TLS_MESSAGE_HASH, hash);

	if (out)
	{
		*out = hash;
	}
	else
	{
		free(hash.ptr);
	}
	return TRUE;
}

/**
 * TLS 1.3 static part of the data the server signs (64 spaces followed by the
 * context string "TLS 1.3, server CertificateVerify" and a 0 byte).
 */
static chunk_t tls13_sig_data_server = chunk_from_chars(
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x54, 0x4c, 0x53, 0x20, 0x31, 0x2e, 0x33, 0x2c,
	0x20, 0x73, 0x65, 0x72, 0x76, 0x65, 0x72, 0x20,
	0x43, 0x65, 0x72, 0x74, 0x69, 0x66, 0x69, 0x63,
	0x61, 0x74, 0x65, 0x56, 0x65, 0x72, 0x69, 0x66,
	0x79, 0x00,
);

/**
 * TLS 1.3 static part of the data the peer signs (64 spaces followed by the
 * context string "TLS 1.3, client CertificateVerify" and a 0 byte).
 */
static chunk_t tls13_sig_data_client = chunk_from_chars(
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x54, 0x4c, 0x53, 0x20, 0x31, 0x2e, 0x33, 0x2c,
	0x20, 0x63, 0x6c, 0x69, 0x65, 0x6e, 0x74, 0x20,
	0x43, 0x65, 0x72, 0x74, 0x69, 0x66, 0x69, 0x63,
	0x61, 0x74, 0x65, 0x56, 0x65, 0x72, 0x69, 0x66,
	0x79, 0x00,
);

METHOD(tls_crypto_t, sign, bool,
	private_tls_crypto_t *this, private_key_t *key, bio_writer_t *writer,
	chunk_t data, chunk_t hashsig)
{
	if (this->tls->get_version_max(this->tls) >= TLS_1_2)
	{
		/* fallback to SHA1/RSA and SHA1/ECDSA */
		const chunk_t hashsig_def = chunk_from_chars(0x02, 0x01, 0x02, 0x03);
		signature_params_t *params;
		key_type_t type;
		uint16_t scheme = 0, hashsig_scheme;
		bio_reader_t *reader;
		chunk_t sig;
		bool done = FALSE;


		if (this->tls->get_version_max(this->tls) >= TLS_1_3)
		{
			chunk_t transcript_hash;

			if (!hash_data(this, data, &transcript_hash))
			{
				DBG1(DBG_TLS, "unable to create transcript hash");
				return FALSE;
			}
			if (this->tls->is_server(this->tls))
			{
				data = chunk_cata("cm", tls13_sig_data_server, transcript_hash);
			}
			else
			{
				data = chunk_cata("cm", tls13_sig_data_client, transcript_hash);
			}
		}

		if (!hashsig.len)
		{	/* fallback if none given */
			hashsig = hashsig_def;
		}

		/* Determine TLS signature scheme if unique */
		type = key->get_type(key);
		switch (type)
		{
			case KEY_ED448:
				scheme = TLS_SIG_ED448;
				break;
			case KEY_ED25519:
				scheme = TLS_SIG_ED25519;
				break;
			case KEY_ECDSA:
				switch (key->get_keysize(key))
				{
					case 256:
						scheme = TLS_SIG_ECDSA_SHA256;
						break;
					case 384:
						scheme = TLS_SIG_ECDSA_SHA384;
						break;
					case 521:
						scheme = TLS_SIG_ECDSA_SHA512;
						break;
					default:
						DBG1(DBG_TLS, "%d bit ECDSA private key size not supported",
							key->get_keysize(key));
						return FALSE;
				}
				break;
			case KEY_RSA:
				/* Several TLS signature schemes possible, select later on */
				break;
			default:
				DBG1(DBG_TLS, "%N private key type not supported",
							   key_type_names, type);
				return FALSE;
		}

		reader = bio_reader_create(hashsig);
		while (reader->remaining(reader) >= 2)
		{
			if (reader->read_uint16(reader, &hashsig_scheme))
			{
				params = params_for_scheme(hashsig_scheme, TRUE);

				/**
				 * All key types except RSA have a single fixed signature scheme
				 * RSA signature schemes are tried until sign() is successful
				 */
				if (params && (scheme == hashsig_scheme ||
				   (!scheme &&
				    type == key_type_from_signature_scheme(params->scheme))))
				{
				    if (key->sign(key, params->scheme, params->params, data, &sig))
					{
						done = TRUE;
						scheme = hashsig_scheme;
						break;
					}
				}
			}
		}
		reader->destroy(reader);
		if (!done)
		{
			DBG1(DBG_TLS, "none of the proposed hash/sig algorithms supported");
			return FALSE;
		}
		DBG1(DBG_TLS, "created signature with %N", tls_signature_scheme_names,
			 scheme);
		writer->write_uint16(writer, scheme);
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
			case KEY_ED25519:
				if (!key->sign(key, SIGN_ED25519, NULL, data, &sig))
				{
					return FALSE;
				}
				DBG2(DBG_TLS, "created signature with Ed25519");
				break;
			case KEY_ED448:
				if (!key->sign(key, SIGN_ED448, NULL, data, &sig))
				{
					return FALSE;
				}
				DBG2(DBG_TLS, "created signature with Ed448");
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
	if (this->tls->get_version_max(this->tls) >= TLS_1_2)
	{
		signature_params_t *params;
		uint16_t scheme;
		chunk_t sig;

		if (!reader->read_uint16(reader, &scheme) ||
			!reader->read_data16(reader, &sig))
		{
			DBG1(DBG_TLS, "received invalid signature");
			return FALSE;
		}
		params = params_for_scheme(scheme, FALSE);
		if (!params)
		{
			DBG1(DBG_TLS, "signature algorithms %N not supported",
				 tls_signature_scheme_names, scheme);
			return FALSE;
		}
		if (this->tls->get_version_max(this->tls) >= TLS_1_3)
		{
			chunk_t transcript_hash;

			if (!hash_data(this, data, &transcript_hash))
			{
				DBG1(DBG_TLS, "Unable to create transcript hash");
				return FALSE;
			}

			if (this->tls->is_server(this->tls))
			{
				data = chunk_cata("cm", tls13_sig_data_client, transcript_hash);
			}
			else
			{
				data = chunk_cata("cm", tls13_sig_data_server, transcript_hash);
			}
		}
		if (!key->verify(key, params->scheme, params->params, data, sig))
		{
			DBG1(DBG_TLS, "signature verification with %N failed",
				 tls_signature_scheme_names, scheme);
			return FALSE;
		}
		DBG2(DBG_TLS, "verified signature with %N",
			 tls_signature_scheme_names, scheme);
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
			case KEY_ED25519:
				if (!key->verify(key, SIGN_ED25519, NULL, data, sig))
				{
					return FALSE;
				}
				DBG2(DBG_TLS, "verified signature with Ed25519");
				break;
			case KEY_ED448:
				if (!key->verify(key, SIGN_ED448, NULL, data, sig))
				{
					return FALSE;
				}
				DBG2(DBG_TLS, "verified signature with Ed448");
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

METHOD(tls_crypto_t, calculate_finished_legacy, bool,
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

METHOD(tls_crypto_t, calculate_finished, bool,
	private_tls_crypto_t *this, bool server, chunk_t *out)
{
	chunk_t finished_key, finished_hash;

	if (!this->hkdf)
	{
		return FALSE;
	}
	if (!hash_data(this, this->handshake, &finished_hash))
	{
		DBG1(DBG_TLS, "creating hash of handshake failed");
		return FALSE;
	}
	if (!this->hkdf->derive_finished(this->hkdf, server, &finished_key))
	{
		DBG1(DBG_TLS, "generating finished key failed");
		chunk_clear(&finished_hash);
		return FALSE;
	}
	if (!this->hkdf->allocate_bytes(this->hkdf, finished_key, finished_hash, out))
	{
		DBG1(DBG_TLS, "generating finished HMAC failed");
		chunk_clear(&finished_key);
		chunk_clear(&finished_hash);
		return FALSE;
	}
	chunk_clear(&finished_key);
	chunk_clear(&finished_hash);
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

/**
 * Derive and configure the client/server key/IV on an AEAD using a given label.
 */
static bool derive_labeled_key(private_tls_crypto_t *this, bool server,
							   tls_hkdf_label_t label, tls_aead_t *aead)
{
	chunk_t key = chunk_empty, iv = chunk_empty;
	bool success = FALSE;

	if (!this->hkdf->generate_secret(this->hkdf, label, this->handshake,
									 NULL) ||
		!this->hkdf->derive_key(this->hkdf, server,
								aead->get_encr_key_size(aead), &key) ||
		!this->hkdf->derive_iv(this->hkdf, server,
							   aead->get_iv_size(aead), &iv))
	{
		DBG1(DBG_TLS, "deriving key material failed");
		goto out;
	}

	if (!aead->set_keys(aead, chunk_empty, key, iv))
	{
		DBG1(DBG_TLS, "setting AEAD key material failed");
		goto out;
	}
	success = TRUE;

out:
	chunk_clear(&key);
	chunk_clear(&iv);
	return success;
}

/**
 * Derive and configure the keys/IVs using the given labels.
 */
static bool derive_labeled_keys(private_tls_crypto_t *this,
								tls_hkdf_label_t client_label,
								tls_hkdf_label_t server_label)
{
	tls_aead_t *aead_c, *aead_s;
	suite_algs_t *algs;

	algs = find_suite(this->suite);
	destroy_aeads(this);
	if (!create_aead(this, algs))
	{
		return FALSE;
	}
	aead_c = this->aead_out;
	aead_s = this->aead_in;
	if (this->tls->is_server(this->tls))
	{
		aead_c = this->aead_in;
		aead_s = this->aead_out;
	}
	return derive_labeled_key(this, FALSE, client_label, aead_c) &&
		   derive_labeled_key(this, TRUE, server_label, aead_s);
}

METHOD(tls_crypto_t, derive_handshake_keys, bool,
	private_tls_crypto_t *this, chunk_t shared_secret)
{
	this->hkdf->set_shared_secret(this->hkdf, shared_secret);
	return derive_labeled_keys(this, TLS_HKDF_C_HS_TRAFFIC,
							   TLS_HKDF_S_HS_TRAFFIC);
}

METHOD(tls_crypto_t, derive_app_keys, bool,
	private_tls_crypto_t *this)
{
	if (!derive_labeled_keys(this, TLS_HKDF_C_AP_TRAFFIC,
							 TLS_HKDF_S_AP_TRAFFIC))
	{
		return FALSE;
	}

	/* EAP-MSK */
	if (this->msk_label)
	{
		uint8_t type;

		switch (this->tls->get_purpose(this->tls))
		{
			case TLS_PURPOSE_EAP_TLS:
				type = EAP_TLS;
				break;
			case TLS_PURPOSE_EAP_PEAP:
				type = EAP_PEAP;
				break;
			case TLS_PURPOSE_EAP_TTLS:
				type = EAP_TTLS;
				break;
			default:
				return FALSE;
		}
		/* because the length is encoded when expanding key material, we
		 * request MSK and EMSK even if we don't use the latter */
		if (!this->hkdf->export(this->hkdf, "EXPORTER_EAP_TLS_Key_Material",
								chunk_from_thing(type), this->handshake, 128,
								&this->msk))
		{
			return FALSE;
		}
		this->msk.len = 64;
	}
	return TRUE;
}

METHOD(tls_crypto_t, update_app_keys, bool,
	private_tls_crypto_t *this, bool inbound)
{
	suite_algs_t *algs;
	tls_hkdf_label_t label = TLS_HKDF_UPD_C_TRAFFIC;

	algs = find_suite(this->suite);
	destroy_aeads(this);
	if (!create_aead(this, algs))
	{
		return FALSE;
	}
	if (this->tls->is_server(this->tls) != inbound)
	{
		label = TLS_HKDF_UPD_S_TRAFFIC;
	}
	return derive_labeled_key(this, label == TLS_HKDF_UPD_S_TRAFFIC, label,
							  inbound ? this->aead_in : this->aead_out);
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
			this->aead_in = NULL;
		}
		else
		{
			this->protection->set_cipher(this->protection, FALSE, this->aead_out);
			this->aead_out = NULL;
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
	DESTROY_IF(this->hkdf);
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
			.hash_handshake = _hash_handshake,
			.sign = _sign,
			.verify = _verify,
			.sign_handshake = _sign_handshake,
			.verify_handshake = _verify_handshake,
			.calculate_finished_legacy = _calculate_finished_legacy,
			.calculate_finished = _calculate_finished,
			.derive_secrets = _derive_secrets,
			.derive_handshake_keys = _derive_handshake_keys,
			.derive_app_keys = _derive_app_keys,
			.update_app_keys = _update_app_keys,
			.resume_session = _resume_session,
			.get_session = _get_session,
			.change_cipher = _change_cipher,
			.get_eap_msk = _get_eap_msk,
			.destroy = _destroy,
		},
		.tls = tls,
		.cache = cache,
	);

	/* FIXME: EDDSA keys are currently treated like ECDSA keys. A cleaner
	 * separation would be welcome. */
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
				case KEY_ED25519:
				case KEY_ED448:
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
			break;
		case TLS_PURPOSE_EAP_PEAP:
			this->msk_label = "client EAP encryption";
			break;
		case TLS_PURPOSE_EAP_TTLS:
			/* MSK PRF ASCII constant label according to EAP-TTLS RFC 5281 */
			this->msk_label = "ttls keying material";
			break;
		default:
			break;
	}
	return &this->public;
}

/**
 * See header.
 */
int tls_crypto_get_supported_suites(bool null, tls_version_t version,
									tls_cipher_suite_t **out)
{
	suite_algs_t suites[countof(suite_algs)] = {};
	int count = 0, i;

	/* initialize copy of suite list */
	for (i = 0; i < countof(suite_algs); i++)
	{
		if (suite_algs[i].min_version <= version &&
			suite_algs[i].max_version >= version)
		{
			suites[count++] = suite_algs[i];
		}
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

/**
 * See header.
 */
int tls_crypto_get_supported_groups(key_exchange_method_t **out)
{
	enumerator_t *enumerator;
	key_exchange_method_t groups[countof(curves)];
	key_exchange_method_t group;
	tls_named_group_t curve;
	int count = 0, i;

	enumerator = enumerator_create_filter(
							lib->crypto->create_ke_enumerator(lib->crypto),
							group_filter, NULL, NULL);

	while (enumerator->enumerate(enumerator, &group, &curve))
	{
		groups[count++] = group;
	}
	enumerator->destroy(enumerator);

	if (out)
	{
		*out = calloc(count, sizeof(key_exchange_method_t));
		for (i = 0; i < count; i++)
		{
			(*out)[i] = groups[i];
		}
	}
	return count;
}

/**
 * See header.
 */
int tls_crypto_get_supported_signatures(tls_version_t version,
										tls_signature_scheme_t **out)
{
	scheme_algs_t sigs[countof(schemes)];
	int count = 0, i;

	/* initialize copy of signature scheme list */
	for (i = 0; i < countof(schemes); i++)
	{
		/* only RSA_PSS_RSAE schemes supported for signing and verifying */
		if (schemes[i].sig == TLS_SIG_RSA_PSS_PSS_SHA256 ||
			schemes[i].sig == TLS_SIG_RSA_PSS_PSS_SHA384 ||
			schemes[i].sig == TLS_SIG_RSA_PSS_PSS_SHA512)
		{
			continue;
		}
		if (schemes[i].min_version <= version &&
			schemes[i].max_version >= version &&
			lib->plugins->has_feature(lib->plugins,
					PLUGIN_PROVIDE(PUBKEY_VERIFY, schemes[i].params.scheme)))
		{
			sigs[count++] = schemes[i];
		}
	}

	if (out)
	{
		*out = calloc(count, sizeof(tls_signature_scheme_t));
		for (i = 0; i < count; i++)
		{
			(*out)[i] = sigs[i].sig;
		}
	}
	return count;
}

/**
 * See header.
 */
tls_named_group_t tls_ec_group_to_curve(key_exchange_method_t group)
{
	int i;

	for (i = 0; i < countof(curves); i++)
	{
		if (curves[i].group == group)
		{
			return curves[i].curve;
		}
	}
	return 0;
}

/**
 * See header.
 */
key_type_t tls_signature_scheme_to_key_type(tls_signature_scheme_t sig)
{
	int i;

	for (i = 0; i < countof(schemes); i++)
	{
		if (schemes[i].sig == sig)
		{
			return key_type_from_signature_scheme(schemes[i].params.scheme);
		}
	}
	return 0;
}

/**
 * Hashtable hash function
 */
static u_int hash_key_type(key_type_t *type)
{
	return chunk_hash(chunk_from_thing(*type));
}

/**
 * Hashtable equals function
 */
static bool equals_key_type(key_type_t *key1, key_type_t *key2)
{
	return *key1 == *key2;
}

CALLBACK(filter_key_types, bool,
	void *data, enumerator_t *orig, va_list args)
{
	key_type_t *key_type, *out;

	VA_ARGS_VGET(args, out);

	if (orig->enumerate(orig, NULL, &key_type))
	{
		*out = *key_type;
		return TRUE;
	}
	return FALSE;
}

CALLBACK(destroy_key_types, void,
	hashtable_t *ht)
{
	ht->destroy_function(ht, (void*)free);
}

/**
 * Create an enumerator over supported key types within a specific TLS range
 */
static enumerator_t *get_supported_key_types(tls_version_t min_version,
										  tls_version_t max_version)
{
	hashtable_t *ht;
	key_type_t *type, lookup;
	int i;

	ht = hashtable_create((hashtable_hash_t)hash_key_type,
						  (hashtable_equals_t)equals_key_type, 4);
	for (i = 0; i < countof(schemes); i++)
	{
		if (schemes[i].min_version <= max_version &&
			schemes[i].max_version >= min_version)
		{
			lookup = key_type_from_signature_scheme(schemes[i].params.scheme);
			if (!ht->get(ht, &lookup))
			{
				type = malloc_thing(key_type_t);
				*type = lookup;
				ht->put(ht, type, type);
			}
		}
	}
	return enumerator_create_filter(ht->create_enumerator(ht),
									filter_key_types, ht, destroy_key_types);
}

/**
 * Create an array of an intersection of server and peer supported key types
 */
static array_t *create_common_key_types(enumerator_t *enumerator, chunk_t hashsig)
{
	array_t *key_types;
	key_type_t v, lookup;
	uint16_t sig_scheme;

	key_types = array_create(sizeof(key_type_t), 8);
	while (enumerator->enumerate(enumerator, &v))
	{
		bio_reader_t *reader;

		reader = bio_reader_create(hashsig);
		while (reader->remaining(reader) &&
			   reader->read_uint16(reader, &sig_scheme))
		{
			lookup = tls_signature_scheme_to_key_type(sig_scheme);
			if (v == lookup)
			{
				array_insert(key_types, ARRAY_TAIL, &lookup);
				break;
			}
		}
		reader->destroy(reader);
	}
	return key_types;
}

typedef struct {
	enumerator_t public;
	array_t *key_types;
	identification_t *peer;
	private_key_t *key;
	auth_cfg_t *auth;
} private_key_enumerator_t;

METHOD(enumerator_t, private_key_enumerate, bool,
	private_key_enumerator_t *this, va_list args)
{
	key_type_t type;
	auth_cfg_t **auth_out;
	private_key_t **key_out;

	VA_ARGS_VGET(args, key_out, auth_out);

	DESTROY_IF(this->key);
	DESTROY_IF(this->auth);
	this->auth = auth_cfg_create();

	while (array_remove(this->key_types, ARRAY_HEAD, &type))
	{
		this->key = lib->credmgr->get_private(lib->credmgr, type, this->peer,
											  this->auth);
		if (this->key)
		{
			*key_out = this->key;
			if (auth_out)
			{
				*auth_out = this->auth;
			}
			return TRUE;
		}
	}
	return FALSE;
}

METHOD(enumerator_t, private_key_destroy, void,
	private_key_enumerator_t *this)
{
	DESTROY_IF(this->key);
	DESTROY_IF(this->auth);
	array_destroy(this->key_types);
	free(this);
}

/**
 * See header.
 */
enumerator_t *tls_create_private_key_enumerator(tls_version_t min_version,
												tls_version_t max_version,
												chunk_t hashsig,
												identification_t *peer)
{
	private_key_enumerator_t *enumerator;
	enumerator_t *key_types;

	key_types = get_supported_key_types(min_version, max_version);

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _private_key_enumerate,
			.destroy = _private_key_destroy,
		},
		.key_types = create_common_key_types(key_types, hashsig),
		.peer = peer,
	);
	key_types->destroy(key_types);

	if (!array_count(enumerator->key_types))
	{
		private_key_destroy(enumerator);
		return NULL;
	}
	return &enumerator->public;
}
