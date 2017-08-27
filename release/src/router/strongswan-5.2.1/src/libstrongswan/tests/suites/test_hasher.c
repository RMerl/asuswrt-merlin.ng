/*
 * Copyright (C) 2013 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
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

#include "test_suite.h"

#include <crypto/hashers/hasher.h>
#include <crypto/prfs/prf.h>
#include <crypto/signers/signer.h>
#include <asn1/oid.h>
#include <utils/test.h>

typedef struct {
	int oid;
	hash_algorithm_t alg;
	key_type_t key;
}hasher_oid_t;

static hasher_oid_t oids[] = {
	{ OID_MD2, HASH_MD2, KEY_ANY },
	{ OID_MD5, HASH_MD5, KEY_ANY },
	{ OID_SHA1, HASH_SHA1, KEY_ANY },
	{ OID_SHA224, HASH_SHA224, KEY_ANY },
	{ OID_SHA256, HASH_SHA256, KEY_ANY },
	{ OID_SHA384, HASH_SHA384, KEY_ANY },
	{ OID_SHA512, HASH_SHA512, KEY_ANY },
	{ OID_UNKNOWN, HASH_UNKNOWN, KEY_ANY },
	{ OID_MD2_WITH_RSA, HASH_MD2, KEY_RSA },
	{ OID_MD5_WITH_RSA, HASH_MD5, KEY_RSA },
	{ OID_SHA1_WITH_RSA, HASH_SHA1, KEY_RSA },
	{ OID_SHA224_WITH_RSA, HASH_SHA224, KEY_RSA },
	{ OID_SHA256_WITH_RSA, HASH_SHA256, KEY_RSA },
	{ OID_SHA384_WITH_RSA, HASH_SHA384, KEY_RSA },
	{ OID_SHA512_WITH_RSA, HASH_SHA512, KEY_RSA },
	{ OID_UNKNOWN, HASH_UNKNOWN, KEY_RSA },
	{ OID_ECDSA_WITH_SHA1, HASH_SHA1, KEY_ECDSA },
	{ OID_ECDSA_WITH_SHA256, HASH_SHA256, KEY_ECDSA },
	{ OID_ECDSA_WITH_SHA384, HASH_SHA384, KEY_ECDSA },
	{ OID_ECDSA_WITH_SHA512, HASH_SHA512, KEY_ECDSA },
	{ OID_UNKNOWN, HASH_UNKNOWN, KEY_ECDSA }
};

START_TEST(test_hasher_from_oid)
{
	ck_assert(hasher_algorithm_from_oid(oids[_i].oid) == oids[_i].alg);
}
END_TEST

START_TEST(test_hasher_to_oid)
{
	ck_assert(hasher_algorithm_to_oid(oids[_i].alg) == oids[_i].oid);
}
END_TEST

START_TEST(test_hasher_sig_to_oid)
{
	ck_assert(hasher_signature_algorithm_to_oid(oids[_i].alg,
												oids[_i].key) == oids[_i].oid);
}
END_TEST

typedef struct {
	pseudo_random_function_t prf;
	hash_algorithm_t alg;
}hasher_prf_t;

static hasher_prf_t prfs[] = {
	{ PRF_HMAC_MD5, HASH_MD5 },
	{ PRF_HMAC_SHA1, HASH_SHA1 },
	{ PRF_FIPS_SHA1_160, HASH_SHA1 },
	{ PRF_KEYED_SHA1, HASH_SHA1 },
	{ PRF_HMAC_SHA2_256, HASH_SHA256 },
	{ PRF_HMAC_SHA2_384, HASH_SHA384 },
	{ PRF_HMAC_SHA2_512, HASH_SHA512 },
	{ PRF_HMAC_TIGER, HASH_UNKNOWN },
	{ PRF_AES128_XCBC, HASH_UNKNOWN },
	{ PRF_AES128_CMAC, HASH_UNKNOWN },
	{ PRF_FIPS_DES, HASH_UNKNOWN },
	{ PRF_CAMELLIA128_XCBC, HASH_UNKNOWN },
	{ PRF_UNDEFINED, HASH_UNKNOWN },
	{ 0, HASH_UNKNOWN }
};

START_TEST(test_hasher_from_prf)
{
	ck_assert(hasher_algorithm_from_prf(prfs[_i].prf) == prfs[_i].alg);
}
END_TEST

typedef struct {
	integrity_algorithm_t auth;
	hash_algorithm_t alg;
	size_t length;
}hasher_auth_t;

static hasher_auth_t auths[] = {
	{ AUTH_UNDEFINED, HASH_MD2, 0 },
	{ AUTH_UNDEFINED, HASH_MD4, 0 },
	{ AUTH_UNDEFINED, HASH_SHA224, 0 },
	{ AUTH_UNDEFINED, 9, 0 },
	{ AUTH_UNDEFINED, HASH_UNKNOWN, 0 },
	{ AUTH_HMAC_MD5_96, HASH_MD5, 12 },
	{ AUTH_HMAC_SHA1_96, HASH_SHA1, 12 },
	{ AUTH_HMAC_SHA2_256_96, HASH_SHA256, 12 },
	{ AUTH_HMAC_MD5_128, HASH_MD5, 16 },
	{ AUTH_HMAC_SHA1_128, HASH_SHA1, 16 },
	{ AUTH_HMAC_SHA2_256_128, HASH_SHA256, 16 },
	{ AUTH_HMAC_SHA1_160, HASH_SHA1, 20 },
	{ AUTH_HMAC_SHA2_384_192, HASH_SHA384, 24 },
	{ AUTH_HMAC_SHA2_256_256, HASH_SHA256, 32 },
	{ AUTH_HMAC_SHA2_512_256, HASH_SHA512, 32 },
	{ AUTH_HMAC_SHA2_384_384, HASH_SHA384, 48 },
	{ AUTH_HMAC_SHA2_512_512, HASH_SHA512, 64 },
	{ AUTH_AES_CMAC_96, HASH_UNKNOWN, 0 },
	{ AUTH_AES_128_GMAC, HASH_UNKNOWN, 0 },
	{ AUTH_AES_192_GMAC, HASH_UNKNOWN, 0 },
	{ AUTH_AES_256_GMAC, HASH_UNKNOWN, 0 },
	{ AUTH_AES_XCBC_96, HASH_UNKNOWN, 0 },
	{ AUTH_DES_MAC, HASH_UNKNOWN, 0 },
	{ AUTH_CAMELLIA_XCBC_96, HASH_UNKNOWN, 0 },
	{ 0, HASH_UNKNOWN, 0 }
};

START_TEST(test_hasher_from_integrity)
{
	size_t length;

	length = 0;
	ck_assert(hasher_algorithm_from_integrity(auths[_i].auth, NULL) == 
											  auths[_i].alg);
	ck_assert(hasher_algorithm_from_integrity(auths[_i].auth, &length) == 
											  auths[_i].alg);
	ck_assert(length == auths[_i].length);
}
END_TEST

START_TEST(test_hasher_to_integrity)
{
	ck_assert(hasher_algorithm_to_integrity(
						auths[_i].alg, auths[_i].length) == auths[_i].auth);
	ck_assert(hasher_algorithm_to_integrity(
						auths[_i].alg, 0) == AUTH_UNDEFINED);
}
END_TEST

Suite *hasher_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("hasher");

	tc = tcase_create("from_oid");
	tcase_add_loop_test(tc, test_hasher_from_oid, 0, 15);
	suite_add_tcase(s, tc);

	tc = tcase_create("to_oid");
	tcase_add_loop_test(tc, test_hasher_to_oid, 0, 8);
	suite_add_tcase(s, tc);

	tc = tcase_create("sig_to_oid");
	tcase_add_loop_test(tc, test_hasher_sig_to_oid, 7, countof(oids));
	suite_add_tcase(s, tc);

	tc = tcase_create("from_prf");
	tcase_add_loop_test(tc, test_hasher_from_prf, 0, countof(prfs));
	suite_add_tcase(s, tc);

	tc = tcase_create("from_integrity");
	tcase_add_loop_test(tc, test_hasher_from_integrity, 4, countof(auths));
	suite_add_tcase(s, tc);

	tc = tcase_create("to_integrity");
	tcase_add_loop_test(tc, test_hasher_to_integrity, 0, 17);
	suite_add_tcase(s, tc);

	return s;
}
