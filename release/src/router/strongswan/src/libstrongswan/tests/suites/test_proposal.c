/*
 * Copyright (C) 2016-2018 Tobias Brunner
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

#include <crypto/proposal/proposal.h>

static struct {
	protocol_id_t proto;
	char *proposal;
	char *expected;
} create_data[] = {
	{ PROTO_IKE, "", NULL },
	{ PROTO_IKE, "sha256", NULL },
	{ PROTO_IKE, "sha256-modp3072", NULL },
	{ PROTO_IKE, "null-sha256-modp3072", "IKE:NULL/HMAC_SHA2_256_128/PRF_HMAC_SHA2_256/MODP_3072" },
	{ PROTO_IKE, "aes128", NULL },
	{ PROTO_IKE, "aes128-sha256", NULL },
	{ PROTO_IKE, "aes128-sha256-modpnone", NULL },
	{ PROTO_IKE, "aes128-prfsha256", NULL },
	{ PROTO_IKE, "aes128-prfsha256-modp2048", NULL },
	{ PROTO_IKE, "aes128-sha256-modp3072", "IKE:AES_CBC_128/HMAC_SHA2_256_128/PRF_HMAC_SHA2_256/MODP_3072" },
	{ PROTO_IKE, "aes128-sha256-prfsha384-modp3072", "IKE:AES_CBC_128/HMAC_SHA2_256_128/PRF_HMAC_SHA2_384/MODP_3072" },
	{ PROTO_IKE, "aes128gcm16-modp3072", NULL },
	{ PROTO_IKE, "aes128gcm16-prfsha256-modp3072", "IKE:AES_GCM_16_128/PRF_HMAC_SHA2_256/MODP_3072" },
	{ PROTO_IKE, "aes128gcm16-sha256-modp3072", "IKE:AES_GCM_16_128/PRF_HMAC_SHA2_256/MODP_3072" },
	{ PROTO_IKE, "aes128gcm16-aes128-modp3072", NULL },
	{ PROTO_IKE, "aes128gcm16-aes128-sha256-modp3072", NULL },
	{ PROTO_ESP, "", NULL },
	{ PROTO_ESP, "sha256", NULL },
	{ PROTO_ESP, "aes128-sha256", "ESP:AES_CBC_128/HMAC_SHA2_256_128/NO_EXT_SEQ" },
	{ PROTO_ESP, "aes128-sha256-esn", "ESP:AES_CBC_128/HMAC_SHA2_256_128/EXT_SEQ" },
	{ PROTO_ESP, "aes128-sha256-noesn", "ESP:AES_CBC_128/HMAC_SHA2_256_128/NO_EXT_SEQ" },
	{ PROTO_ESP, "aes128-sha256-esn-noesn", "ESP:AES_CBC_128/HMAC_SHA2_256_128/EXT_SEQ/NO_EXT_SEQ" },
	{ PROTO_ESP, "aes128-sha256-prfsha256-modp3072", "ESP:AES_CBC_128/HMAC_SHA2_256_128/MODP_3072/NO_EXT_SEQ" },
	{ PROTO_ESP, "aes128gcm16-aes128-sha256-modp3072", NULL },
	{ PROTO_ESP, "aes128gmac", "ESP:NULL_AES_GMAC_128/NO_EXT_SEQ" },
	{ PROTO_AH,  "", NULL },
	{ PROTO_AH,  "aes128", NULL },
	{ PROTO_AH,  "aes128-sha256", "AH:HMAC_SHA2_256_128/NO_EXT_SEQ" },
	{ PROTO_AH,  "sha256-sha1", "AH:HMAC_SHA2_256_128/HMAC_SHA1_96/NO_EXT_SEQ" },
	{ PROTO_AH,  "aes128gmac-sha256", "AH:AES_128_GMAC/HMAC_SHA2_256_128/NO_EXT_SEQ" },
	{ PROTO_AH,  "aes128gmac-sha256-prfsha256", "AH:AES_128_GMAC/HMAC_SHA2_256_128/NO_EXT_SEQ" },
	{ PROTO_AH,  "aes128gmac-aes256gmac-aes128-sha256", "AH:AES_128_GMAC/AES_256_GMAC/HMAC_SHA2_256_128/NO_EXT_SEQ" },
	{ PROTO_AH,  "sha256-esn", "AH:HMAC_SHA2_256_128/EXT_SEQ" },
	{ PROTO_AH,  "sha256-noesn", "AH:HMAC_SHA2_256_128/NO_EXT_SEQ" },
	{ PROTO_AH,  "sha256-esn-noesn", "AH:HMAC_SHA2_256_128/EXT_SEQ/NO_EXT_SEQ" },
};

static void assert_proposal_eq(proposal_t *proposal, char *expected)
{
	char str[BUF_LEN];

	if (!expected)
	{
		ck_assert(!proposal);
		return;
	}
	snprintf(str, sizeof(str), "%P", proposal);
	ck_assert_str_eq(expected, str);
}

START_TEST(test_create_from_string)
{
	proposal_t *proposal;

	proposal = proposal_create_from_string(create_data[_i].proto,
										   create_data[_i].proposal);
	assert_proposal_eq(proposal, create_data[_i].expected);
	DESTROY_IF(proposal);
}
END_TEST

static struct {
	protocol_id_t proto;
	char *self;
	char *other;
	char *expected;
} select_data[] = {
	{ PROTO_ESP, "aes128", "aes128", "aes128" },
	{ PROTO_ESP, "aes128", "aes256", NULL },
	{ PROTO_ESP, "aes128-aes256", "aes256-aes128", "aes128" },
	{ PROTO_ESP, "aes256-aes128", "aes128-aes256", "aes256" },
	{ PROTO_ESP, "aes128-aes256-sha1-sha256", "aes256-aes128-sha256-sha1", "aes128-sha1" },
	{ PROTO_ESP, "aes256-aes128-sha256-sha1", "aes128-aes256-sha1-sha256", "aes256-sha256" },
	{ PROTO_ESP, "aes128-sha256-modp3072", "aes128-sha256", NULL },
	{ PROTO_ESP, "aes128-sha256", "aes128-sha256-modp3072", NULL },
	{ PROTO_ESP, "aes128-sha256-modp3072", "aes128-sha256-modpnone", NULL },
	{ PROTO_ESP, "aes128-sha256-modpnone", "aes128-sha256-modp3072", NULL },
	{ PROTO_ESP, "aes128-sha256-modp3072-modpnone", "aes128-sha256", "aes128-sha256" },
	{ PROTO_ESP, "aes128-sha256", "aes128-sha256-modp3072-modpnone", "aes128-sha256" },
	{ PROTO_ESP, "aes128-sha256-modp3072-modpnone", "aes128-sha256-modpnone-modp3072", "aes128-sha256-modp3072" },
	{ PROTO_ESP, "aes128-sha256-modpnone-modp3072", "aes128-sha256-modp3072-modpnone", "aes128-sha256" },
	{ PROTO_ESP, "aes128-sha256-esn", "aes128-sha256-esn", "aes128-sha256-esn" },
	{ PROTO_ESP, "aes128-sha256-noesn", "aes128-sha256-esn", NULL },
	{ PROTO_ESP, "aes128-sha256-noesn-esn", "aes128-sha256-esn", "aes128-sha256-esn" },
	{ PROTO_ESP, "aes128-sha256-noesn-esn", "aes128-sha256", "aes128-sha256" },
	{ PROTO_ESP, "aes128-sha256-esn-noesn", "aes128-sha256-noesn-esn", "aes128-sha256-esn" },
	{ PROTO_IKE, "aes128-sha256-modp3072", "aes128-sha256-modp3072", "aes128-sha256-modp3072" },
	{ PROTO_IKE, "aes128-sha256-modp3072", "aes128-sha256-modp3072-modpnone", "aes128-sha256-modp3072" },
	{ PROTO_IKE, "aes128-sha256-modp3072-modpnone", "aes128-sha256-modp3072", "aes128-sha256-modp3072" },
};

START_TEST(test_select)
{
	proposal_t *self, *other, *selected, *expected;

	self = proposal_create_from_string(select_data[_i].proto,
									   select_data[_i].self);
	other = proposal_create_from_string(select_data[_i].proto,
										select_data[_i].other);
	selected = self->select(self, other, TRUE, FALSE);
	if (select_data[_i].expected)
	{
		expected = proposal_create_from_string(select_data[_i].proto,
											   select_data[_i].expected);
		ck_assert(selected);
		ck_assert_msg(expected->equals(expected, selected), "proposal %P does "
					  "not match expected %P", selected, expected);
		expected->destroy(expected);
	}
	else
	{
		ck_assert(!selected);
	}
	DESTROY_IF(selected);
	other->destroy(other);
	self->destroy(self);
}
END_TEST

START_TEST(test_select_spi)
{
	proposal_t *self, *other, *selected;

	self = proposal_create_from_string(PROTO_ESP, "aes128-sha256-modp3072");
	other = proposal_create_from_string(PROTO_ESP, "aes128-sha256-modp3072");
	other->set_spi(other, 0x12345678);

	selected = self->select(self, other, TRUE, FALSE);
	ck_assert(selected);
	ck_assert_int_eq(selected->get_spi(selected), other->get_spi(other));
	selected->destroy(selected);

	selected = self->select(self, other, FALSE, FALSE);
	ck_assert(selected);
	ck_assert_int_eq(selected->get_spi(selected), self->get_spi(self));
	selected->destroy(selected);

	other->destroy(other);
	self->destroy(self);
}
END_TEST

START_TEST(test_matches)
{
	proposal_t *self, *other;

	self = proposal_create_from_string(select_data[_i].proto,
									   select_data[_i].self);
	other = proposal_create_from_string(select_data[_i].proto,
										select_data[_i].other);
	if (select_data[_i].expected)
	{
		ck_assert(self->matches(self, other, FALSE));
		ck_assert(other->matches(other, self, FALSE));
	}
	else
	{
		ck_assert(!self->matches(self, other, FALSE));
		ck_assert(!other->matches(other, self, FALSE));
	}
	other->destroy(other);
	self->destroy(self);
}
END_TEST

START_TEST(test_promote_dh_group)
{
	proposal_t *proposal;

	proposal = proposal_create_from_string(PROTO_IKE,
										   "aes128-sha256-modp3072-ecp256");
	ck_assert(proposal->promote_dh_group(proposal, ECP_256_BIT));
	assert_proposal_eq(proposal, "IKE:AES_CBC_128/HMAC_SHA2_256_128/PRF_HMAC_SHA2_256/ECP_256/MODP_3072");
	proposal->destroy(proposal);
}
END_TEST

START_TEST(test_promote_dh_group_already_front)
{
	proposal_t *proposal;

	proposal = proposal_create_from_string(PROTO_IKE,
										   "aes128-sha256-modp3072-ecp256");
	ck_assert(proposal->promote_dh_group(proposal, MODP_3072_BIT));
	assert_proposal_eq(proposal, "IKE:AES_CBC_128/HMAC_SHA2_256_128/PRF_HMAC_SHA2_256/MODP_3072/ECP_256");
	proposal->destroy(proposal);
}
END_TEST

START_TEST(test_promote_dh_group_not_contained)
{
	proposal_t *proposal;

	proposal = proposal_create_from_string(PROTO_IKE,
										   "aes128-sha256-modp3072-ecp256");

	ck_assert(!proposal->promote_dh_group(proposal, MODP_2048_BIT));
	assert_proposal_eq(proposal, "IKE:AES_CBC_128/HMAC_SHA2_256_128/PRF_HMAC_SHA2_256/MODP_3072/ECP_256");
	proposal->destroy(proposal);
}
END_TEST

START_TEST(test_unknown_transform_types_print)
{
	proposal_t *proposal;

	proposal = proposal_create(PROTO_IKE, 0);
	proposal->add_algorithm(proposal, 242, 42, 128);
	assert_proposal_eq(proposal, "IKE:UNKNOWN_242_42_128");
	proposal->destroy(proposal);

	proposal = proposal_create_from_string(PROTO_IKE,
										   "aes128-sha256-ecp256");
	proposal->add_algorithm(proposal, 242, 42, 128);
	proposal->add_algorithm(proposal, 243, 1, 0);
	assert_proposal_eq(proposal, "IKE:AES_CBC_128/HMAC_SHA2_256_128/PRF_HMAC_SHA2_256/ECP_256/UNKNOWN_242_42_128/UNKNOWN_243_1");
	proposal->destroy(proposal);
}
END_TEST

START_TEST(test_unknown_transform_types_equals)
{
	proposal_t *self, *other;

	self = proposal_create_from_string(PROTO_IKE, "aes128-sha256-ecp256");
	other = proposal_create_from_string(PROTO_IKE, "aes128-sha256-ecp256");
	other->add_algorithm(other, 242, 42, 0);
	ck_assert(!self->equals(self, other));
	ck_assert(!other->equals(other, self));
	self->add_algorithm(self, 242, 42, 0);
	ck_assert(self->equals(self, other));
	ck_assert(other->equals(other, self));
	other->destroy(other);
	self->destroy(self);
}
END_TEST

START_TEST(test_unknown_transform_types_select_fail)
{
	proposal_t *self, *other, *selected;

	self = proposal_create_from_string(PROTO_IKE, "aes128-sha256-ecp256");
	other = proposal_create_from_string(PROTO_IKE, "aes128-sha256-ecp256");
	other->add_algorithm(other, 242, 42, 0);

	selected = self->select(self, other, TRUE, FALSE);
	ck_assert(!selected);
	other->destroy(other);
	self->destroy(self);
}
END_TEST

START_TEST(test_unknown_transform_types_select_fail_subtype)
{
	proposal_t *self, *other, *selected;

	self = proposal_create_from_string(PROTO_IKE, "aes128-sha256-ecp256");
	self->add_algorithm(self, 242, 8, 0);
	other = proposal_create_from_string(PROTO_IKE, "aes128-sha256-ecp256");
	other->add_algorithm(other, 242, 42, 0);

	selected = self->select(self, other, TRUE, FALSE);
	ck_assert(!selected);
	other->destroy(other);
	self->destroy(self);
}
END_TEST

START_TEST(test_unknown_transform_types_select_success)
{
	proposal_t *self, *other, *selected;

	self = proposal_create_from_string(PROTO_IKE, "aes128-sha256-ecp256");
	self->add_algorithm(self, 242, 42, 128);
	other = proposal_create_from_string(PROTO_IKE, "aes128-sha256-ecp256");
	other->add_algorithm(other, 242, 42, 128);
	other->add_algorithm(other, 242, 1, 0);

	selected = self->select(self, other, TRUE, FALSE);
	ck_assert(selected);
	assert_proposal_eq(selected, "IKE:AES_CBC_128/HMAC_SHA2_256_128/PRF_HMAC_SHA2_256/ECP_256/UNKNOWN_242_42_128");
	selected->destroy(selected);
	other->destroy(other);
	self->destroy(self);
}
END_TEST

START_TEST(test_chacha20_poly1305_key_length)
{
	proposal_t *proposal;
	uint16_t alg, ks;

	proposal = proposal_create_from_string(PROTO_IKE, "chacha20poly1305-prfsha256-ecp256");
	proposal->get_algorithm(proposal, ENCRYPTION_ALGORITHM, &alg, &ks);
	ck_assert_int_eq(alg, ENCR_CHACHA20_POLY1305);
	ck_assert_int_eq(ks, 0);
	assert_proposal_eq(proposal, "IKE:CHACHA20_POLY1305/PRF_HMAC_SHA2_256/ECP_256");
	proposal->destroy(proposal);
}
END_TEST


Suite *proposal_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("proposal");

	tc = tcase_create("create_from_string");
	tcase_add_loop_test(tc, test_create_from_string, 0, countof(create_data));
	suite_add_tcase(s, tc);

	tc = tcase_create("select");
	tcase_add_loop_test(tc, test_select, 0, countof(select_data));
	tcase_add_test(tc, test_select_spi);
	suite_add_tcase(s, tc);

	tc = tcase_create("matches");
	tcase_add_loop_test(tc, test_matches, 0, countof(select_data));
	suite_add_tcase(s, tc);

	tc = tcase_create("promote_dh_group");
	tcase_add_test(tc, test_promote_dh_group);
	tcase_add_test(tc, test_promote_dh_group_already_front);
	tcase_add_test(tc, test_promote_dh_group_not_contained);
	suite_add_tcase(s, tc);

	tc = tcase_create("unknown transform types");
	tcase_add_test(tc, test_unknown_transform_types_print);
	tcase_add_test(tc, test_unknown_transform_types_equals);
	tcase_add_test(tc, test_unknown_transform_types_select_fail);
	tcase_add_test(tc, test_unknown_transform_types_select_fail_subtype);
	tcase_add_test(tc, test_unknown_transform_types_select_success);
	suite_add_tcase(s, tc);

	tc = tcase_create("chacha20/poly1305");
	tcase_add_test(tc, test_chacha20_poly1305_key_length);
	suite_add_tcase(s, tc);

	return s;
}
