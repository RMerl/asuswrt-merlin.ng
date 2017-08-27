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

#include <crypto/crypters/crypter.h>
#include <asn1/oid.h>
#include <utils/test.h>

typedef struct {
	int oid;
	encryption_algorithm_t alg;
	size_t key_size;
}crypter_oid_t;

static crypter_oid_t oids[] = {
	{ OID_UNKNOWN, ENCR_AES_CBC, 0 },
	{ OID_UNKNOWN, ENCR_CAMELLIA_CBC, 0 },
	{ OID_UNKNOWN, ENCR_UNDEFINED, 0 },
	{ OID_DES_CBC, ENCR_DES, 0 },
	{ OID_3DES_EDE_CBC, ENCR_3DES, 0 },
	{ OID_AES128_CBC, ENCR_AES_CBC, 128 },
	{ OID_AES192_CBC, ENCR_AES_CBC, 192 },
	{ OID_AES256_CBC, ENCR_AES_CBC, 256 },
	{ OID_CAMELLIA128_CBC, ENCR_CAMELLIA_CBC, 128 },
	{ OID_CAMELLIA192_CBC, ENCR_CAMELLIA_CBC, 192 },
	{ OID_CAMELLIA256_CBC, ENCR_CAMELLIA_CBC, 256 }
};

START_TEST(test_crypter_from_oid)
{
	size_t key_size;

	ck_assert(encryption_algorithm_from_oid(oids[_i].oid, NULL) ==
										    oids[_i].alg);
	ck_assert(encryption_algorithm_from_oid(oids[_i].oid, &key_size) ==
										    oids[_i].alg);
	ck_assert(key_size == oids[_i].key_size);
}
END_TEST

START_TEST(test_crypter_to_oid)
{
	ck_assert(encryption_algorithm_to_oid(oids[_i].alg,
									      oids[_i].key_size) == oids[_i].oid);
}
END_TEST

typedef struct {
	encryption_algorithm_t alg;
	bool is_aead;
}crypter_aead_t;

static crypter_aead_t aead[] = {
	{ ENCR_AES_CCM_ICV8, TRUE },
	{ ENCR_AES_CCM_ICV12, TRUE },
	{ ENCR_AES_CCM_ICV16, TRUE },
	{ ENCR_AES_GCM_ICV8, TRUE },
	{ ENCR_AES_GCM_ICV12, TRUE },
	{ ENCR_AES_GCM_ICV16, TRUE },
	{ ENCR_NULL_AUTH_AES_GMAC, TRUE },
	{ ENCR_CAMELLIA_CCM_ICV8, TRUE },
	{ ENCR_CAMELLIA_CCM_ICV12, TRUE },
	{ ENCR_CAMELLIA_CCM_ICV16, TRUE },
	{ ENCR_AES_CBC, FALSE },
	{ ENCR_CAMELLIA_CBC, FALSE }
};
     
START_TEST(test_crypter_is_aead)
{
	ck_assert(encryption_algorithm_is_aead(aead[_i].alg) == aead[_i].is_aead);
}
END_TEST

Suite *crypter_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("crypter");

	tc = tcase_create("from_oid");
	tcase_add_loop_test(tc, test_crypter_from_oid, 2, countof(oids));
	suite_add_tcase(s, tc);

	tc = tcase_create("to_oid");
	tcase_add_loop_test(tc, test_crypter_to_oid, 0, countof(oids));
	suite_add_tcase(s, tc);

	tc = tcase_create("is_aead");
	tcase_add_loop_test(tc, test_crypter_is_aead, 0, countof(aead));
	suite_add_tcase(s, tc);

	return s;
}
