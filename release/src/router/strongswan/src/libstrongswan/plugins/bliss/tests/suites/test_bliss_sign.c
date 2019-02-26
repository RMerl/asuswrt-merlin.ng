/*
 * Copyright (C) 2014-2015 Andreas Steffen
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

#include <bliss_private_key.h>
#include <bliss_public_key.h>

static u_int key_type[] = { 1, 3, 4 };
static u_int key_strength[] = { 128, 160, 192 };

START_TEST(test_bliss_sign_all)
{
	signature_scheme_t signature_scheme;
	private_key_t *privkey, *privkey1;
	public_key_t *pubkey, *pubkey1;
	chunk_t msg, signature, privkey_blob, pubkey_blob, pubkey_fp, privkey_fp;
	int k;

	for (k = 0; k < 4; k++)
	{
		int verify_count = 1000;

		switch (k)
		{
			case 1:
				signature_scheme = SIGN_BLISS_WITH_SHA2_256;
				break;
			case 2:
				signature_scheme = SIGN_BLISS_WITH_SHA2_384;
				break;
			default:
				signature_scheme = SIGN_BLISS_WITH_SHA2_512;
		}

		/* enforce BLISS-B key for k = 2, 3 */
		lib->settings->set_bool(lib->settings,
				"%s.plugins.bliss.use_bliss_b", k >= 2, lib->ns);

		msg = chunk_from_str("Hello Dolly!");

		/* generate private key */
		privkey = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_BLISS,
									 BUILD_KEY_SIZE, key_type[_i], BUILD_END);
		ck_assert(privkey);

		/* generate ASN.1 DER and PEM encoding of private key */
		ck_assert(privkey->get_encoding(privkey, (k % 2) ?
				  PRIVKEY_ASN1_DER : PRIVKEY_PEM, &privkey_blob));

		/* extract public key from private key */
		pubkey = privkey->get_public_key(privkey);
		ck_assert(pubkey);

		/* generate ASN.1 DER and PEM encodings of public key */
		ck_assert(pubkey->get_encoding(pubkey, (k % 2) ?
				  PUBKEY_SPKI_ASN1_DER : PUBKEY_PEM, &pubkey_blob));

		/* compare fingerprints of public and private key */
		ck_assert(pubkey->get_fingerprint(pubkey, (k % 2) ?
				  KEYID_PUBKEY_INFO_SHA1 : KEYID_PUBKEY_SHA1, &pubkey_fp));
		ck_assert(privkey->get_fingerprint(privkey, (k % 2) ?
				  KEYID_PUBKEY_INFO_SHA1 : KEYID_PUBKEY_SHA1, &privkey_fp));
		ck_assert(chunk_equals(pubkey_fp, privkey_fp));

		/* retrieve fingerprints of public and private key from cache */
		ck_assert(pubkey->get_fingerprint(pubkey, (k % 2) ?
				  KEYID_PUBKEY_INFO_SHA1 : KEYID_PUBKEY_SHA1, &pubkey_fp));
		ck_assert(privkey->get_fingerprint(privkey, (k % 2) ?
				  KEYID_PUBKEY_INFO_SHA1 : KEYID_PUBKEY_SHA1, &privkey_fp));

		/* get a reference of the private key and destroy both instances */
		privkey1 = privkey->get_ref(privkey);
		ck_assert(privkey1);
		ck_assert(privkey1 == privkey);
		privkey->destroy(privkey);
		privkey1->destroy(privkey1);

		/* get a reference of the public key and destroy both instances */
		pubkey1 = pubkey->get_ref(pubkey);
		ck_assert(pubkey1);
		ck_assert(pubkey1 == pubkey);
		pubkey->destroy(pubkey);
		pubkey1->destroy(pubkey1);

		/* enforce BLISS-B key for k = 1, 3 */
		lib->settings->set_bool(lib->settings,
				"%s.plugins.bliss.use_bliss_b", k % 2, lib->ns);

		/* load private key from ASN.1 blob */
		privkey = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_BLISS,
									 BUILD_BLOB, privkey_blob, BUILD_END);
		ck_assert(privkey);
		ck_assert(privkey->get_type(privkey) == KEY_BLISS);
		ck_assert(privkey->get_keysize(privkey) == key_strength[_i]);
		chunk_free(&privkey_blob);

		/* load public key from ASN.1 blob */
		pubkey = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_ANY,
									BUILD_BLOB, pubkey_blob, BUILD_END);
		ck_assert(pubkey);
		ck_assert(pubkey->get_type(pubkey) == KEY_BLISS);
		ck_assert(pubkey->get_keysize(pubkey) == key_strength[_i]);
		chunk_free(&pubkey_blob);

		/* generate and verify 1000 BLISS signatures */
		while (verify_count--)
		{
			ck_assert(privkey->sign(privkey, signature_scheme, NULL, msg,
									&signature));
			ck_assert(pubkey->verify(pubkey, signature_scheme, NULL, msg,
									 signature));
			free(signature.ptr);
		}
		privkey->destroy(privkey);
		pubkey->destroy(pubkey);
	}
}
END_TEST

START_TEST(test_bliss_sign_fail)
{
	private_key_t *privkey;
	public_key_t *pubkey;
	chunk_t msg = chunk_empty, signature, encoding, fp;

	/* generate non-supported BLISS-II private key */
	privkey = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_BLISS,
								 BUILD_KEY_SIZE, BLISS_II, BUILD_END);
	ck_assert(!privkey);

	/* generate non-supported BLISS-B-II private key */
	privkey = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_BLISS,
								 BUILD_KEY_SIZE, BLISS_B_II, BUILD_END);
	ck_assert(!privkey);

	/* generate supported BLISS-B-I private key */
	privkey = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_BLISS,
								 BUILD_KEY_SIZE, BLISS_B_I, BUILD_END);
	ck_assert(privkey);

	/* wrong private key encoding format */
	ck_assert(!privkey->get_encoding(privkey, PUBKEY_PEM, &encoding));

	/* wrong fingerprint encoding format */
	ck_assert(!privkey->get_fingerprint(privkey, KEYID_PGPV4, &fp));

	/* extract public key */
	pubkey = privkey->get_public_key(privkey);
	ck_assert(pubkey);

	/* wrong private key encoding format */
	ck_assert(!pubkey->get_encoding(pubkey, PRIVKEY_PEM, &encoding));

	/* wrong fingerprint encoding format */
	ck_assert(!pubkey->get_fingerprint(pubkey, KEYID_PGPV4, &fp));

	/* encryption / decryption operation is not defined for BLISS */
	ck_assert(!pubkey->encrypt(pubkey, ENCRYPT_UNKNOWN, chunk_empty, NULL));
	ck_assert(!privkey->decrypt(privkey, ENCRYPT_UNKNOWN, chunk_empty, NULL));

	/* sign with invalid signature scheme */
	ck_assert(!privkey->sign(privkey, SIGN_UNKNOWN, NULL, msg, &signature));

	/* generate valid signature */
	msg = chunk_from_str("Hello Dolly!");
	ck_assert(privkey->sign(privkey, SIGN_BLISS_WITH_SHA2_512, NULL, msg, &signature));

	/* verify with invalid signature scheme */
	ck_assert(!pubkey->verify(pubkey, SIGN_UNKNOWN, NULL, msg, signature));

	/* corrupt signature */
	signature.ptr[signature.len - 1] ^= 0x80;
	ck_assert(!pubkey->verify(pubkey, SIGN_BLISS_WITH_SHA2_512, NULL, msg, signature));

	free(signature.ptr);
	privkey->destroy(privkey);
	pubkey->destroy(pubkey);
}
END_TEST

Suite *bliss_sign_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("bliss_sign");

	tc = tcase_create("sign_all");
	test_case_set_timeout(tc, 30);
	tcase_add_loop_test(tc, test_bliss_sign_all, 0, countof(key_type));
	suite_add_tcase(s, tc);

	tc = tcase_create("sign_fail");
	tcase_add_test(tc, test_bliss_sign_fail);
	suite_add_tcase(s, tc);

	return s;
}
