/*
 * Copyright (C) 2016 Tobias Brunner
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

#include <credentials/auth_cfg.h>

struct {
	char *constraints;
	signature_scheme_t sig[5];
	signature_scheme_t ike[5];
} sig_constraints_tests[] = {
	{ "rsa-sha256", { SIGN_RSA_EMSA_PKCS1_SHA2_256, 0 }, {0}},
	{ "rsa-sha256-sha512", { SIGN_RSA_EMSA_PKCS1_SHA2_256, SIGN_RSA_EMSA_PKCS1_SHA2_512, 0 }, {0}},
	{ "ecdsa-sha256", { SIGN_ECDSA_WITH_SHA256_DER, SIGN_ECDSA_256, 0 }, {0}},
	{ "rsa-sha256-ecdsa-sha256", { SIGN_RSA_EMSA_PKCS1_SHA2_256, SIGN_ECDSA_WITH_SHA256_DER, SIGN_ECDSA_256, 0 }, {0}},
	{ "pubkey-sha256", { SIGN_RSA_EMSA_PKCS1_SHA2_256, SIGN_ECDSA_WITH_SHA256_DER, SIGN_ECDSA_256, SIGN_BLISS_WITH_SHA2_256, 0 }, {0}},
	{ "ike:rsa-sha256", {0}, { SIGN_RSA_EMSA_PKCS1_SHA2_256, 0 }},
	{ "ike:rsa-sha256-rsa-sha256", { SIGN_RSA_EMSA_PKCS1_SHA2_256, 0 }, { SIGN_RSA_EMSA_PKCS1_SHA2_256, 0 }},
	{ "rsa-sha256-ike:rsa-sha256", { SIGN_RSA_EMSA_PKCS1_SHA2_256, 0 }, { SIGN_RSA_EMSA_PKCS1_SHA2_256, 0 }},
	{ "ike:pubkey-sha256", {0}, { SIGN_RSA_EMSA_PKCS1_SHA2_256, SIGN_ECDSA_WITH_SHA256_DER, SIGN_ECDSA_256, SIGN_BLISS_WITH_SHA2_256, 0 }},
	{ "rsa-ecdsa-sha256", { SIGN_ECDSA_WITH_SHA256_DER, SIGN_ECDSA_256, 0 }, {0}},
	{ "rsa-4096-ecdsa-sha256", { SIGN_ECDSA_WITH_SHA256_DER, SIGN_ECDSA_256, 0 }, {0}},
	{ "rsa-4096-ecdsa-256-sha256", { SIGN_ECDSA_WITH_SHA256_DER, SIGN_ECDSA_256, 0 }, {0}},
	{ "rsa-ecdsa256-sha256", { SIGN_RSA_EMSA_PKCS1_SHA2_256, 0 }, {0}},
	{ "rsa4096-sha256", {0}, {0}},
	{ "sha256", {0}, {0}},
	{ "ike:sha256", {0}, {0}},
};

static void check_sig_constraints(auth_cfg_t *cfg, auth_rule_t type,
								  signature_scheme_t expected[])
{
	enumerator_t *enumerator;
	auth_rule_t t;
	signature_params_t *value;
	int i = 0;

	enumerator = cfg->create_enumerator(cfg);
	while (enumerator->enumerate(enumerator, &t, &value))
	{
		if (t == type)
		{
			ck_assert(expected[i]);
			ck_assert_int_eq(expected[i], value->scheme);
			i++;
		}
	}
	enumerator->destroy(enumerator);
	ck_assert(!expected[i]);
}

START_TEST(test_sig_contraints)
{
	auth_cfg_t *cfg;
	signature_scheme_t none[] = {0};

	cfg = auth_cfg_create();
	cfg->add_pubkey_constraints(cfg, sig_constraints_tests[_i].constraints, FALSE);
	check_sig_constraints(cfg, AUTH_RULE_SIGNATURE_SCHEME, sig_constraints_tests[_i].sig);
	check_sig_constraints(cfg, AUTH_RULE_IKE_SIGNATURE_SCHEME, none);
	cfg->destroy(cfg);

	lib->settings->set_bool(lib->settings, "%s.signature_authentication_constraints",
							FALSE, lib->ns);

	cfg = auth_cfg_create();
	cfg->add_pubkey_constraints(cfg, sig_constraints_tests[_i].constraints, TRUE);
	check_sig_constraints(cfg, AUTH_RULE_SIGNATURE_SCHEME, sig_constraints_tests[_i].sig);
	check_sig_constraints(cfg, AUTH_RULE_IKE_SIGNATURE_SCHEME, sig_constraints_tests[_i].ike);
	cfg->destroy(cfg);
}
END_TEST

START_TEST(test_ike_contraints_fallback)
{
	auth_cfg_t *cfg;

	lib->settings->set_bool(lib->settings, "%s.signature_authentication_constraints",
							TRUE, lib->ns);

	cfg = auth_cfg_create();
	cfg->add_pubkey_constraints(cfg, sig_constraints_tests[_i].constraints, TRUE);
	check_sig_constraints(cfg, AUTH_RULE_SIGNATURE_SCHEME, sig_constraints_tests[_i].sig);
	if (sig_constraints_tests[_i].ike[0])
	{
		check_sig_constraints(cfg, AUTH_RULE_IKE_SIGNATURE_SCHEME, sig_constraints_tests[_i].ike);
	}
	else
	{
		check_sig_constraints(cfg, AUTH_RULE_IKE_SIGNATURE_SCHEME, sig_constraints_tests[_i].sig);
	}
	cfg->destroy(cfg);
}
END_TEST

typedef union {
	rsa_pss_params_t pss;
} signature_param_types_t;

struct {
	char *constraints;
	signature_scheme_t sig[5];
	signature_param_types_t p[5];
} sig_constraints_params_tests[] = {
	{ "rsa/pss-sha256", { SIGN_RSA_EMSA_PSS, 0 }, {
		{ .pss = { .hash = HASH_SHA256, .mgf1_hash = HASH_SHA256, .salt_len = HASH_SIZE_SHA256, }}}},
	{ "rsa/pss-sha256-sha384", { SIGN_RSA_EMSA_PSS, SIGN_RSA_EMSA_PSS, 0 }, {
		{ .pss = { .hash = HASH_SHA256, .mgf1_hash = HASH_SHA256, .salt_len = HASH_SIZE_SHA256, }},
		{ .pss = { .hash = HASH_SHA384, .mgf1_hash = HASH_SHA384, .salt_len = HASH_SIZE_SHA384, }}}},
	{ "rsa/pss-sha256-rsa-sha256", { SIGN_RSA_EMSA_PSS, SIGN_RSA_EMSA_PKCS1_SHA2_256, 0 }, {
		{ .pss = { .hash = HASH_SHA256, .mgf1_hash = HASH_SHA256, .salt_len = HASH_SIZE_SHA256, }}}},
	{ "rsa-sha256-rsa/pss-sha256", { SIGN_RSA_EMSA_PKCS1_SHA2_256, SIGN_RSA_EMSA_PSS, 0 }, {
		{},
		{ .pss = { .hash = HASH_SHA256, .mgf1_hash = HASH_SHA256, .salt_len = HASH_SIZE_SHA256, }}}},
	{ "rsa/pss", { 0 }, {}},
};

static void check_sig_constraints_params(auth_cfg_t *cfg, auth_rule_t type,
										 signature_scheme_t scheme[],
										 signature_param_types_t p[])
{
	enumerator_t *enumerator;
	auth_rule_t t;
	signature_params_t *value;
	int i = 0;

	enumerator = cfg->create_enumerator(cfg);
	while (enumerator->enumerate(enumerator, &t, &value))
	{
		if (t == type)
		{
			if (scheme[i] == SIGN_RSA_EMSA_PSS)
			{
				signature_params_t expected = {
					.scheme = scheme[i],
					.params = &p[i].pss,
				};
				ck_assert(signature_params_equal(value, &expected));
			}
			else
			{
				ck_assert(scheme[i]);
				ck_assert(!value->params);
				ck_assert_int_eq(scheme[i], value->scheme);
			}
			i++;
		}
	}
	enumerator->destroy(enumerator);
	ck_assert(!scheme[i]);
}

START_TEST(test_sig_contraints_params)
{
	auth_cfg_t *cfg;

	cfg = auth_cfg_create();
	cfg->add_pubkey_constraints(cfg, sig_constraints_params_tests[_i].constraints, TRUE);
	check_sig_constraints_params(cfg, AUTH_RULE_IKE_SIGNATURE_SCHEME,
								 sig_constraints_params_tests[_i].sig,
								 sig_constraints_params_tests[_i].p);
	cfg->destroy(cfg);
}
END_TEST

struct {
	char *constraints;
	signature_scheme_t sig[6];
	signature_param_types_t p[6];
} sig_constraints_rsa_pss_tests[] = {
	{ "pubkey-sha256", { SIGN_RSA_EMSA_PSS, SIGN_RSA_EMSA_PKCS1_SHA2_256, SIGN_ECDSA_WITH_SHA256_DER, SIGN_ECDSA_256, SIGN_BLISS_WITH_SHA2_256, 0 }, {
		{ .pss = { .hash = HASH_SHA256, .mgf1_hash = HASH_SHA256, .salt_len = HASH_SIZE_SHA256, }}, {}, {}, {}, {}}},
	{ "rsa-sha256", { SIGN_RSA_EMSA_PSS, SIGN_RSA_EMSA_PKCS1_SHA2_256, 0 }, {
		{ .pss = { .hash = HASH_SHA256, .mgf1_hash = HASH_SHA256, .salt_len = HASH_SIZE_SHA256, }}, {}}},
};

START_TEST(test_sig_contraints_rsa_pss)
{
	auth_cfg_t *cfg;

	lib->settings->set_bool(lib->settings, "%s.rsa_pss", TRUE, lib->ns);

	cfg = auth_cfg_create();
	cfg->add_pubkey_constraints(cfg, sig_constraints_rsa_pss_tests[_i].constraints, TRUE);
	check_sig_constraints_params(cfg, AUTH_RULE_IKE_SIGNATURE_SCHEME,
								 sig_constraints_rsa_pss_tests[_i].sig,
								 sig_constraints_rsa_pss_tests[_i].p);
	cfg->destroy(cfg);
}
END_TEST

Suite *auth_cfg_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("auth_cfg");

	tc = tcase_create("add_pubkey_constraints");
	tcase_add_loop_test(tc, test_sig_contraints, 0, countof(sig_constraints_tests));
	tcase_add_loop_test(tc, test_ike_contraints_fallback, 0, countof(sig_constraints_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("add_pubkey_constraints parameters");
	tcase_add_loop_test(tc, test_sig_contraints_params, 0, countof(sig_constraints_params_tests));
	tcase_add_loop_test(tc, test_sig_contraints_rsa_pss, 0, countof(sig_constraints_rsa_pss_tests));
	suite_add_tcase(s, tc);

	return s;
}
