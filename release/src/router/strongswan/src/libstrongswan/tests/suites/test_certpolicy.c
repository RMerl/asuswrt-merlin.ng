/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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

#include <asn1/asn1.h>
#include <credentials/sets/mem_cred.h>
#include <credentials/certificates/x509.h>

/**
 * RSA private key, so we don't have to generate one
 */
static char keydata[] = {
  0x30,0x82,0x02,0x5e,0x02,0x01,0x00,0x02,0x81,0x81,0x00,0xb1,0x9b,0xd4,0x51,0x24,
  0xfc,0x56,0x1d,0x3d,0xfb,0xa2,0xea,0x37,0x02,0x70,0x72,0x87,0x84,0x2f,0x3b,0x2d,
  0x6e,0x22,0xef,0x3f,0x37,0x04,0xb2,0x6f,0xb7,0xe7,0xd8,0x58,0x05,0xde,0x34,0xbf,
  0x99,0xe6,0x40,0x7a,0x56,0xa7,0x73,0xf5,0x98,0xcb,0xb0,0x37,0x90,0x5e,0xd1,0x3f,
  0xf4,0x73,0x50,0x7f,0x53,0x8e,0xf1,0x04,0x25,0xb4,0x77,0x22,0x4e,0x8a,0x9d,0x27,
  0x8f,0x6f,0xaf,0x59,0xbd,0xb0,0x0f,0xf0,0xaa,0x11,0x94,0x66,0x16,0x10,0x58,0xad,
  0x77,0xa1,0xac,0x58,0xb4,0xd0,0x0d,0xbc,0x11,0xe0,0xc0,0xe9,0x29,0xdc,0x42,0x63,
  0x01,0x23,0x4f,0x28,0x41,0x6d,0x34,0x9e,0x0c,0x4a,0xc8,0x62,0x83,0xb5,0x71,0x71,
  0x0b,0x51,0xc0,0x4c,0x37,0xd4,0x68,0x19,0x52,0x9a,0x8b,0x02,0x03,0x01,0x00,0x01,
  0x02,0x81,0x81,0x00,0x82,0xca,0x33,0x16,0xb2,0x3a,0xd4,0x1b,0x62,0x9a,0x9c,0xc5,
  0x07,0x4f,0x57,0x89,0x2f,0x7c,0x4a,0xdf,0xb4,0x3b,0xc7,0xa4,0x11,0x14,0x2d,0xf4,
  0x4c,0xca,0xcc,0x03,0x88,0x06,0x82,0x34,0xab,0xe7,0xe4,0x24,0x15,0x33,0x1c,0xcb,
  0x0a,0xcf,0xc3,0x27,0x78,0x33,0x6b,0x6f,0x82,0x3e,0x3c,0x70,0xc9,0xe2,0xb9,0x7f,
  0x88,0xc3,0x4f,0x59,0xb5,0x8e,0xa3,0x81,0xd9,0x88,0x1f,0xc0,0x38,0xbc,0xc8,0x93,
  0x40,0x0f,0x43,0xd8,0x72,0x12,0xb4,0xcc,0x6d,0x76,0x0a,0x6f,0x01,0x05,0xa8,0x88,
  0xf4,0x57,0x44,0xd2,0x05,0xc4,0x77,0xf5,0xfb,0x1b,0xf3,0xb2,0x0d,0x90,0xb8,0xb4,
  0x63,0x62,0x70,0x2c,0xe4,0x28,0xd8,0x20,0x10,0x85,0x4a,0x5e,0x63,0xa9,0xb0,0xdd,
  0xba,0xd0,0x32,0x49,0x02,0x41,0x00,0xdb,0x77,0xf1,0xdd,0x1a,0x12,0xc5,0xfb,0x2b,
  0x5b,0xb2,0xcd,0xb6,0xd0,0x4c,0xc4,0xe5,0x93,0xd6,0xf8,0x88,0xfc,0x18,0x40,0x21,
  0x9c,0xf7,0x2d,0x60,0x6f,0x91,0xf5,0x73,0x3c,0xf7,0x7f,0x67,0x1d,0x5b,0xb5,0xee,
  0x29,0xc1,0xd4,0xc6,0xdb,0x44,0x4c,0x40,0x05,0x63,0xaa,0x71,0x95,0x18,0x14,0xa7,
  0x23,0x9f,0x7a,0xee,0x7f,0xb5,0xc7,0x02,0x41,0x00,0xcf,0x2c,0x24,0x50,0x65,0xf4,
  0x94,0x7b,0xe9,0xf3,0x13,0x77,0xea,0x27,0x3c,0x6f,0x03,0x84,0xa7,0x7d,0xa2,0x54,
  0x40,0x97,0x82,0x0e,0xd9,0x09,0x9f,0x4a,0xa6,0x75,0xe5,0x66,0xe4,0x9c,0x59,0xd9,
  0x3a,0xe6,0xf7,0xd8,0x8b,0x68,0xb0,0x21,0x52,0x31,0xb3,0x4a,0xa0,0x2c,0x41,0xd7,
  0x1f,0x7b,0xe2,0x0f,0x15,0xc9,0x6e,0xc0,0xe5,0x1d,0x02,0x41,0x00,0x9c,0x1a,0x61,
  0x9f,0x89,0xc7,0x26,0xa9,0x33,0xba,0xe2,0xa0,0x6d,0xd3,0x15,0x77,0xcb,0x6f,0xef,
  0xad,0x12,0x0a,0x75,0xd9,0x4f,0xcf,0x4d,0x05,0x2a,0x9d,0xd1,0x2c,0xcb,0xcd,0xe6,
  0xa0,0xe9,0x20,0x39,0xb6,0x5a,0xf3,0xba,0x99,0xf4,0xe3,0xcb,0x5d,0x8d,0x00,0x08,
  0x57,0x18,0xb9,0x1a,0xca,0xbd,0xe3,0x99,0xb1,0x1f,0xe9,0x18,0xcb,0x02,0x40,0x65,
  0x35,0x1b,0x48,0x6b,0x86,0x60,0x43,0x68,0xb6,0xe6,0xfb,0xdd,0xd7,0xed,0x1e,0x0e,
  0x89,0xef,0x88,0xe0,0x94,0x68,0x39,0x9b,0xbf,0xc5,0x27,0x7e,0x39,0xe9,0xb8,0x0e,
  0xa9,0x85,0x65,0x1c,0x3f,0x93,0x16,0xe2,0x5d,0x57,0x3d,0x7d,0x4d,0xc9,0xe9,0x9d,
  0xbd,0x07,0x22,0x97,0xc7,0x90,0x09,0xe5,0x15,0x99,0x7f,0x1e,0x2b,0xfd,0xc1,0x02,
  0x41,0x00,0x92,0x78,0xfe,0x04,0xa0,0x53,0xed,0x36,0x97,0xbd,0x16,0xce,0x91,0x9b,
  0xbe,0x1f,0x8e,0x40,0x00,0x99,0x0c,0x49,0x15,0xca,0x59,0xd3,0xe3,0xd4,0xeb,0x71,
  0xcf,0xda,0xd7,0xc8,0x99,0x74,0xfc,0x6b,0xe8,0xfd,0xe5,0xe0,0x49,0x61,0xcb,0xda,
  0xe3,0xe7,0x8b,0x72,0xb5,0x69,0x73,0x2b,0x8b,0x54,0xcb,0xd9,0x48,0x6d,0x61,0x02,
  0x49,0xe8,
};

/**
 * Issue a certificate fr given policy, including extended flags
 */
static certificate_t* create_cert_ext(certificate_t *ca, char *subject,
									  char *oid, x509_flag_t flags,
									  char *map_s, char *map_i,
									  u_int require_explicit,
									  u_int inhibit_mapping,
									  u_int inhibit_any)
{
	private_key_t *privkey;
	public_key_t *pubkey;
	certificate_t *cert;
	identification_t *id;
	linked_list_t *policies, *maps;
	x509_cert_policy_t policy = {};
	x509_policy_mapping_t map = {};

	privkey = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_RSA,
								 BUILD_BLOB_ASN1_DER, chunk_from_thing(keydata),
								 BUILD_END);
	ck_assert(privkey);
	pubkey = privkey->get_public_key(privkey);
	ck_assert(pubkey);
	policies = linked_list_create();
	if (oid)
	{
		policy.oid = asn1_oid_from_string(oid);
		ck_assert(policy.oid.ptr);
		policies->insert_last(policies, &policy);
	}
	maps = linked_list_create();
	if (map_s && map_i)
	{
		map.subject = asn1_oid_from_string(map_s);
		ck_assert(map.subject.ptr);
		map.issuer = asn1_oid_from_string(map_i);
		ck_assert(map.issuer.ptr);
		maps->insert_last(maps, &map);
	}
	id = identification_create_from_string(subject);
	cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
						BUILD_SIGNING_KEY, privkey,
						BUILD_PUBLIC_KEY, pubkey,
						BUILD_SUBJECT, id,
						BUILD_X509_FLAG, flags,
						BUILD_CERTIFICATE_POLICIES, policies,
						BUILD_POLICY_MAPPINGS, maps,
						BUILD_SIGNING_CERT, ca,
						BUILD_POLICY_REQUIRE_EXPLICIT, require_explicit,
						BUILD_POLICY_INHIBIT_MAPPING, inhibit_mapping,
						BUILD_POLICY_INHIBIT_ANY, inhibit_any,
						BUILD_END);
	ck_assert(cert);
	id->destroy(id);
	policies->destroy(policies);
	maps->destroy(maps);
	privkey->destroy(privkey);
	pubkey->destroy(pubkey);
	free(policy.oid.ptr);
	free(map.subject.ptr);
	free(map.issuer.ptr);

	return cert;
}

/**
 * Issue a certificate with given certificate policy and flags
 */
static certificate_t* create_cert(certificate_t *ca, char *subject,
								  char *oid, x509_flag_t flags,
								  char *map_s, char *map_i)
{
	return create_cert_ext(ca, subject, oid, flags, map_s, map_i,
						   X509_NO_CONSTRAINT, X509_NO_CONSTRAINT,
						   X509_NO_CONSTRAINT);
}

/**
 * Check if a certificate with given subject has an oid
 */
static bool check_oid(identification_t *subject, char *oid)
{
	enumerator_t *certs, *auths;
	certificate_t *cert;
	auth_cfg_t *auth;
	bool found = FALSE;
	auth_rule_t type;
	char *current;

	certs = lib->credmgr->create_trusted_enumerator(lib->credmgr, KEY_ANY,
													subject, FALSE);
	if (!certs->enumerate(certs, &cert, &auth))
	{
		certs->destroy(certs);
		ck_assert_msg(FALSE, "no trusted certificate found for %Y", subject);
	}
	auths = auth->create_enumerator(auth);
	while (auths->enumerate(auths, &type, &current))
	{
		if (type == AUTH_RULE_CERT_POLICY)
		{
			if (streq(current, oid))
			{
				found = TRUE;
				break;
			}
		}
	}
	auths->destroy(auths);
	certs->destroy(certs);

	return found;
}

/**
 * Check if a certificate with given subject has a valid trustchain
 */
static bool check_trust(identification_t *subject)
{
	enumerator_t *certs;
	certificate_t *cert;
	bool trusted;

	certs = lib->credmgr->create_trusted_enumerator(lib->credmgr, KEY_ANY,
													subject, FALSE);
	trusted = certs->enumerate(certs, &cert, NULL);
	certs->destroy(certs);

	return trusted;
}

static mem_cred_t *creds;

static char *anyPolicy = "2.5.29.32.0";
static char *extended = "2.23.140.1.1";
static char *baseline = "2.23.140.1.2";

START_SETUP(setup)
{
	creds = mem_cred_create();
	lib->credmgr->add_set(lib->credmgr, &creds->set);
}
END_SETUP

START_TEARDOWN(teardown)
{
	lib->credmgr->remove_set(lib->credmgr, &creds->set);
	creds->destroy(creds);
	lib->credmgr->flush_cache(lib->credmgr, CERT_ANY);
}
END_TEARDOWN

START_TEST(test_valid_fixed)
{
	certificate_t *ca, *im, *sj;

	ca = create_cert(NULL, "CN=CA", baseline, X509_CA, NULL, NULL);
	im = create_cert(ca, "CN=IM", baseline, X509_CA, NULL, NULL);
	sj = create_cert(im, "CN=SJ", baseline, 0, NULL, NULL);

	creds->add_cert(creds, TRUE, ca);
	creds->add_cert(creds, FALSE, im);
	creds->add_cert(creds, FALSE, sj);

	ck_assert(check_oid(sj->get_subject(sj), baseline));
}
END_TEST

START_TEST(test_valid_any1)
{
	certificate_t *ca, *im, *sj;

	ca = create_cert(NULL, "CN=CA", anyPolicy, X509_CA, NULL, NULL);
	im = create_cert(ca, "CN=IM", baseline, X509_CA, NULL, NULL);
	sj = create_cert(im, "CN=SJ", baseline, 0, NULL, NULL);

	creds->add_cert(creds, TRUE, ca);
	creds->add_cert(creds, FALSE, im);
	creds->add_cert(creds, FALSE, sj);

	ck_assert(check_oid(sj->get_subject(sj), baseline));
}
END_TEST

START_TEST(test_valid_any2)
{
	certificate_t *ca, *im, *sj;

	ca = create_cert(NULL, "CN=CA", anyPolicy, X509_CA, NULL, NULL);
	im = create_cert(ca, "CN=IM", anyPolicy, X509_CA, NULL, NULL);
	sj = create_cert(im, "CN=SJ", baseline, 0, NULL, NULL);

	creds->add_cert(creds, TRUE, ca);
	creds->add_cert(creds, FALSE, im);
	creds->add_cert(creds, FALSE, sj);

	ck_assert(check_oid(sj->get_subject(sj), baseline));
}
END_TEST

START_TEST(test_invalid_missing)
{
	certificate_t *ca, *im, *sj;

	ca = create_cert(NULL, "CN=CA", baseline, X509_CA, NULL, NULL);
	im = create_cert(ca, "CN=IM", baseline, X509_CA, NULL, NULL);
	sj = create_cert(im, "CN=SJ", NULL, 0, NULL, NULL);

	creds->add_cert(creds, TRUE, ca);
	creds->add_cert(creds, FALSE, im);
	creds->add_cert(creds, FALSE, sj);

	ck_assert(!check_oid(sj->get_subject(sj), baseline));
}
END_TEST

START_TEST(test_invalid_wrong)
{
	certificate_t *ca, *im, *sj;

	ca = create_cert(NULL, "CN=CA", baseline, X509_CA, NULL, NULL);
	im = create_cert(ca, "CN=IM", baseline, X509_CA, NULL, NULL);
	sj = create_cert(im, "CN=SJ", baseline, 0, NULL, NULL);

	creds->add_cert(creds, TRUE, ca);
	creds->add_cert(creds, FALSE, im);
	creds->add_cert(creds, FALSE, sj);

	ck_assert(!check_oid(sj->get_subject(sj), extended));
}
END_TEST

START_TEST(test_invalid_any1)
{
	certificate_t *ca, *im, *sj;

	ca = create_cert(NULL, "CN=CA", anyPolicy, X509_CA, NULL, NULL);
	im = create_cert(ca, "CN=IM", anyPolicy, X509_CA, NULL, NULL);
	sj = create_cert(im, "CN=SJ", NULL, 0, NULL, NULL);

	creds->add_cert(creds, TRUE, ca);
	creds->add_cert(creds, FALSE, im);
	creds->add_cert(creds, FALSE, sj);

	ck_assert(!check_oid(sj->get_subject(sj), baseline));
}
END_TEST

START_TEST(test_invalid_any2)
{
	certificate_t *ca, *im, *sj;

	ca = create_cert(NULL, "CN=CA", anyPolicy, X509_CA, NULL, NULL);
	im = create_cert(ca, "CN=IM", anyPolicy, X509_CA, NULL, NULL);
	sj = create_cert(im, "CN=SJ", anyPolicy, 0, NULL, NULL);

	creds->add_cert(creds, TRUE, ca);
	creds->add_cert(creds, FALSE, im);
	creds->add_cert(creds, FALSE, sj);

	ck_assert(!check_oid(sj->get_subject(sj), baseline));
}
END_TEST

START_TEST(test_badchain_wrong)
{
	certificate_t *ca, *im, *sj;

	ca = create_cert(NULL, "CN=CA", baseline, X509_CA, NULL, NULL);
	im = create_cert(ca, "CN=IM", extended, X509_CA, NULL, NULL);
	sj = create_cert(im, "CN=SJ", extended, 0, NULL, NULL);

	creds->add_cert(creds, TRUE, ca);
	creds->add_cert(creds, FALSE, im);
	creds->add_cert(creds, FALSE, sj);

	ck_assert(!check_oid(sj->get_subject(sj), baseline));
	ck_assert(!check_oid(sj->get_subject(sj), extended));
}
END_TEST

START_TEST(test_badchain_gap)
{
	certificate_t *ca, *im, *sj;

	ca = create_cert(NULL, "CN=CA", baseline, X509_CA, NULL, NULL);
	im = create_cert(ca, "CN=IM", NULL, X509_CA, NULL, NULL);
	sj = create_cert(im, "CN=SJ", baseline, 0, NULL, NULL);

	creds->add_cert(creds, TRUE, ca);
	creds->add_cert(creds, FALSE, im);
	creds->add_cert(creds, FALSE, sj);

	ck_assert(!check_oid(sj->get_subject(sj), baseline));
}
END_TEST

START_TEST(test_badchain_any)
{
	certificate_t *ca, *im, *sj;

	ca = create_cert(NULL, "CN=CA", baseline, X509_CA, NULL, NULL);
	im = create_cert(ca, "CN=IM", anyPolicy, X509_CA, NULL, NULL);
	sj = create_cert(im, "CN=SJ", extended, 0, NULL, NULL);

	creds->add_cert(creds, TRUE, ca);
	creds->add_cert(creds, FALSE, im);
	creds->add_cert(creds, FALSE, sj);

	ck_assert(!check_oid(sj->get_subject(sj), extended));
}
END_TEST

START_TEST(test_valid_mapping)
{
	certificate_t *ca, *im, *sj;

	ca = create_cert(NULL, "CN=CA", anyPolicy, X509_CA, NULL, NULL);
	im = create_cert(ca, "CN=IM", extended, X509_CA, baseline, extended);
	sj = create_cert(im, "CN=SJ", baseline, 0, NULL, NULL);

	creds->add_cert(creds, TRUE, ca);
	creds->add_cert(creds, FALSE, im);
	creds->add_cert(creds, FALSE, sj);

	ck_assert(check_oid(sj->get_subject(sj), baseline));
}
END_TEST

START_TEST(test_valid_mapping_twice)
{
	certificate_t *ca, *im, *sj;

	ca = create_cert(NULL, "CN=CA", "2.23.140.1.3", X509_CA,
					 extended, "2.23.140.1.3");
	im = create_cert(ca, "CN=IM", extended, X509_CA, baseline, extended);
	sj = create_cert(im, "CN=SJ", baseline, 0, NULL, NULL);

	creds->add_cert(creds, TRUE, ca);
	creds->add_cert(creds, FALSE, im);
	creds->add_cert(creds, FALSE, sj);

	ck_assert(check_oid(sj->get_subject(sj), baseline));
}
END_TEST

START_TEST(test_invalid_mapping_loop)
{
	certificate_t *ca, *im, *sj;

	ca = create_cert(NULL, "CN=CA", anyPolicy, X509_CA, NULL, NULL);
	im = create_cert(ca, "CN=IM", extended, X509_CA, baseline, baseline);
	sj = create_cert(im, "CN=SJ", baseline, 0, NULL, NULL);

	creds->add_cert(creds, TRUE, ca);
	creds->add_cert(creds, FALSE, im);
	creds->add_cert(creds, FALSE, sj);

	ck_assert(!check_oid(sj->get_subject(sj), baseline));
}
END_TEST

START_TEST(test_invalid_mapping_notallowed)
{
	certificate_t *ca, *im, *sj;

	ca = create_cert(NULL, "CN=CA", baseline, X509_CA, NULL, NULL);
	im = create_cert(ca, "CN=IM", extended, X509_CA, baseline, extended);
	sj = create_cert(im, "CN=SJ", baseline, 0, NULL, NULL);

	creds->add_cert(creds, TRUE, ca);
	creds->add_cert(creds, FALSE, im);
	creds->add_cert(creds, FALSE, sj);

	ck_assert(!check_oid(sj->get_subject(sj), baseline));
}
END_TEST

START_TEST(test_invalid_mapping_nopolicy)
{
	certificate_t *ca, *im, *sj;

	ca = create_cert(NULL, "CN=CA", baseline, X509_CA, NULL, NULL);
	im = create_cert(ca, "CN=IM", "2.23.140.1.3", X509_CA, baseline, extended);
	sj = create_cert(im, "CN=SJ", baseline, 0, NULL, NULL);

	creds->add_cert(creds, TRUE, ca);
	creds->add_cert(creds, FALSE, im);
	creds->add_cert(creds, FALSE, sj);

	ck_assert(!check_oid(sj->get_subject(sj), baseline));
}
END_TEST

START_TEST(test_inhibit_mapping_good)
{
	certificate_t *ca, *im, *sj;

	ca = create_cert_ext(NULL, "CN=CA", extended, X509_CA, NULL, NULL,
						 X509_NO_CONSTRAINT, 1, X509_NO_CONSTRAINT);
	im = create_cert(ca, "CN=IM", extended, X509_CA, baseline, extended);
	sj = create_cert(im, "CN=SJ", baseline, 0, NULL, NULL);

	creds->add_cert(creds, TRUE, ca);
	creds->add_cert(creds, FALSE, im);
	creds->add_cert(creds, FALSE, sj);

	ck_assert(check_oid(sj->get_subject(sj), baseline));
}
END_TEST

START_TEST(test_inhibit_mapping_bad)
{
	certificate_t *ca, *i1, *i2, *sj;

	ca = create_cert_ext(NULL, "CN=CA", extended, X509_CA, NULL, NULL,
						 X509_NO_CONSTRAINT, 1, X509_NO_CONSTRAINT);
	i1 = create_cert(ca, "CN=IM1", extended, X509_CA, NULL, NULL);
	i2 = create_cert(i1, "CN=IM2", extended, X509_CA, baseline, extended);
	sj = create_cert(i2, "CN=SJ", baseline, 0, NULL, NULL);

	creds->add_cert(creds, TRUE, ca);
	creds->add_cert(creds, FALSE, i1);
	creds->add_cert(creds, FALSE, i2);
	creds->add_cert(creds, FALSE, sj);

	/* TODO: we currently reject the certificate completely, but should
	 * actually just invalidate the policy not mapped properly */
	ck_assert(!check_trust(sj->get_subject(sj)));
}
END_TEST

START_TEST(test_inhibit_any_good)
{
	certificate_t *ca, *im, *sj;

	ca = create_cert_ext(NULL, "CN=CA", anyPolicy, X509_CA, NULL, NULL,
						 X509_NO_CONSTRAINT, X509_NO_CONSTRAINT, 1);
	im = create_cert(ca, "CN=IM", anyPolicy, X509_CA, NULL, NULL);
	sj = create_cert(im, "CN=SJ", baseline, 0, NULL, NULL);

	creds->add_cert(creds, TRUE, ca);
	creds->add_cert(creds, FALSE, im);
	creds->add_cert(creds, FALSE, sj);

	ck_assert(check_oid(sj->get_subject(sj), baseline));
}
END_TEST

START_TEST(test_inhibit_any_bad)
{
	certificate_t *ca, *i1, *i2, *sj;

	ca = create_cert_ext(NULL, "CN=CA", anyPolicy, X509_CA, NULL, NULL,
						 X509_NO_CONSTRAINT, X509_NO_CONSTRAINT, 1);
	i1 = create_cert(ca, "CN=IM1", anyPolicy, X509_CA, NULL, NULL);
	i2 = create_cert(i1, "CN=IM2", anyPolicy, X509_CA, NULL, NULL);
	sj = create_cert(i2, "CN=SJ", baseline, 0, NULL, NULL);

	creds->add_cert(creds, TRUE, ca);
	creds->add_cert(creds, FALSE, i1);
	creds->add_cert(creds, FALSE, i2);
	creds->add_cert(creds, FALSE, sj);

	/* TODO: we currently reject the certificate completely, but should
	 * actually just invalidate the policy relying on inhibited anyPolicy */
	ck_assert(!check_trust(sj->get_subject(sj)));
}
END_TEST

START_TEST(test_require_explicit_good)
{
	certificate_t *ca, *im, *sj;

	ca = create_cert_ext(NULL, "CN=CA", anyPolicy, X509_CA, NULL, NULL,
						 1, X509_NO_CONSTRAINT, X509_NO_CONSTRAINT);
	im = create_cert(ca, "CN=IM", baseline, X509_CA, NULL, NULL);
	sj = create_cert(im, "CN=SJ", baseline, 0, NULL, NULL);

	creds->add_cert(creds, TRUE, ca);
	creds->add_cert(creds, FALSE, im);
	creds->add_cert(creds, FALSE, sj);

	ck_assert(check_oid(sj->get_subject(sj), baseline));
}
END_TEST

START_TEST(test_require_explicit_bad)
{
	certificate_t *ca, *i1, *i2, *sj;

	ca = create_cert_ext(NULL, "CN=CA", anyPolicy, X509_CA, NULL, NULL,
						 1, X509_NO_CONSTRAINT, X509_NO_CONSTRAINT);
	i1 = create_cert(ca, "CN=IM1", extended, X509_CA, NULL, NULL);
	i2 = create_cert(i1, "CN=IM2", extended, X509_CA, NULL, NULL);
	sj = create_cert(i2, "CN=SJ", baseline, 0, NULL, NULL);

	creds->add_cert(creds, TRUE, ca);
	creds->add_cert(creds, FALSE, i1);
	creds->add_cert(creds, FALSE, i2);
	creds->add_cert(creds, FALSE, sj);

	/* TODO: we currently reject the certificate completely, but should
	 * actually just invalidate the policy violating requireExplicit */
	ck_assert(!check_trust(sj->get_subject(sj)));
}
END_TEST

Suite *certpolicy_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("certpolicy");

	tc = tcase_create("policy valid");
	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_add_test(tc, test_valid_fixed);
	tcase_add_test(tc, test_valid_any1);
	tcase_add_test(tc, test_valid_any2);
	suite_add_tcase(s, tc);

	tc = tcase_create("policy invalid");
	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_add_test(tc, test_invalid_missing);
	tcase_add_test(tc, test_invalid_wrong);
	tcase_add_test(tc, test_invalid_any1);
	tcase_add_test(tc, test_invalid_any2);
	suite_add_tcase(s, tc);

	tc = tcase_create("policy badchain");
	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_add_test(tc, test_badchain_wrong);
	tcase_add_test(tc, test_badchain_gap);
	tcase_add_test(tc, test_badchain_any);
	suite_add_tcase(s, tc);

	tc = tcase_create("policy valid mapping");
	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_add_test(tc, test_valid_mapping);
	tcase_add_test(tc, test_valid_mapping_twice);
	suite_add_tcase(s, tc);

	tc = tcase_create("policy invalid mapping");
	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_add_test(tc, test_invalid_mapping_loop);
	tcase_add_test(tc, test_invalid_mapping_notallowed);
	tcase_add_test(tc, test_invalid_mapping_nopolicy);
	suite_add_tcase(s, tc);

	tc = tcase_create("inhibit policy mapping");
	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_add_test(tc, test_inhibit_mapping_good);
	tcase_add_test(tc, test_inhibit_mapping_bad);
	suite_add_tcase(s, tc);

	tc = tcase_create("inhibit any policy");
	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_add_test(tc, test_inhibit_any_good);
	tcase_add_test(tc, test_inhibit_any_bad);
	suite_add_tcase(s, tc);

	tc = tcase_create("require explicit policy");
	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_add_test(tc, test_require_explicit_good);
	tcase_add_test(tc, test_require_explicit_bad);
	suite_add_tcase(s, tc);

	return s;
}
