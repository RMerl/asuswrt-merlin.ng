/*
 * Copyright (C) 2015 Tobias Brunner
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

#include <config/ike_cfg.h>

static void assert_family(int expected, char *addr, bool local)
{
	ike_cfg_t *cfg;
	int family;

	cfg = ike_cfg_create(IKEV2, FALSE, FALSE, local ? addr : "%any", 500,
						 local ? "%any" : addr, 500, FRAGMENTATION_NO, 0);
	family = ike_cfg_get_family(cfg, local);
	ck_assert_msg(expected == family, "expected family %d != %d (addr: '%s')",
				  expected, family, addr);
	cfg->destroy(cfg);
}

START_TEST(test_get_address_family_empty)
{
	assert_family(AF_UNSPEC, "", _i);
}
END_TEST

START_TEST(test_get_address_family_addr)
{
	assert_family(AF_INET, "192.168.1.1", _i);
	assert_family(AF_INET6, "fec::1", _i);
}
END_TEST

START_TEST(test_get_address_family_multi)
{
	assert_family(AF_INET, "192.168.1.1,192.168.2.2", _i);
	assert_family(AF_INET6, "fec::1,fec::2", _i);

	assert_family(AF_UNSPEC, "192.168.1.1,fec::1", _i);
	assert_family(AF_UNSPEC, "fec::1,192.168.1.1", _i);
}
END_TEST

START_TEST(test_get_address_family_any)
{
	assert_family(AF_UNSPEC, "%any", _i);

	assert_family(AF_INET, "%any4", _i);
	assert_family(AF_INET, "0.0.0.0", _i);

	assert_family(AF_INET6, "%any6", _i);
	assert_family(AF_INET6, "::", _i);

	assert_family(AF_INET, "192.168.1.1,%any", _i);
	assert_family(AF_INET, "192.168.1.1,%any4", _i);
	assert_family(AF_UNSPEC, "192.168.1.1,%any6", _i);

	assert_family(AF_INET6, "fec::1,%any", _i);
	assert_family(AF_UNSPEC, "fec::1,%any4", _i);
	assert_family(AF_INET6, "fec::1,%any6", _i);
}
END_TEST

START_TEST(test_get_address_family_other)
{
	assert_family(AF_INET, "192.168.1.0", _i);
	assert_family(AF_UNSPEC, "192.168.1.0/24", _i);
	assert_family(AF_UNSPEC, "192.168.1.0-192.168.1.10", _i);

	assert_family(AF_INET, "192.168.1.0/24,192.168.2.1", _i);
	assert_family(AF_INET, "192.168.1.0-192.168.1.10,192.168.2.1", _i);
	assert_family(AF_INET6, "192.168.1.0/24,fec::1", _i);
	assert_family(AF_INET6, "192.168.1.0-192.168.1.10,fec::1", _i);

	assert_family(AF_INET6, "fec::", _i);
	assert_family(AF_UNSPEC, "fec::/64", _i);
	assert_family(AF_UNSPEC, "fec::1-fec::10", _i);

	assert_family(AF_INET6, "fec::/64,fed::1", _i);
	assert_family(AF_INET6, "fec::1-fec::10,fec::1", _i);
	assert_family(AF_INET, "fec::/64,192.168.1.1", _i);
	assert_family(AF_INET, "fec::1-fec::10,192.168.1.1", _i);

	assert_family(AF_UNSPEC, "strongswan.org", _i);
	assert_family(AF_INET, "192.168.1.0,strongswan.org", _i);
	assert_family(AF_INET6, "fec::1,strongswan.org", _i);
}
END_TEST

Suite *ike_cfg_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("ike_cfg");

	tc = tcase_create("ike_cfg_get_address_family");
	tcase_add_loop_test(tc, test_get_address_family_empty, 0, 2);
	tcase_add_loop_test(tc, test_get_address_family_addr, 0, 2);
	tcase_add_loop_test(tc, test_get_address_family_multi, 0, 2);
	tcase_add_loop_test(tc, test_get_address_family_any, 0, 2);
	tcase_add_loop_test(tc, test_get_address_family_other, 0, 2);
	suite_add_tcase(s, tc);

	return s;
}
