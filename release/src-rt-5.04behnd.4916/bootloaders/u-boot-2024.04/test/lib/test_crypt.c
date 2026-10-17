// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Steffen Jaeckel
 *
 * Unit test for crypt-style password hashing
 */

#include <common.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

#include <crypt.h>

/**
 * lib_crypt() - unit test for crypt-style password hashing
 *
 * @uts:	unit test state
 * Return:	0 = success, 1 = failure
 */
static int lib_crypt(struct unit_test_state *uts)
{
	int equals = 0;
	int err;

	err = crypt_compare("", "password", &equals);
	ut_assertf(err != 0, "crypt_compare successful but should not\n");
	ut_assertf(equals != 1,
		   "crypt_compare password hash matched but should not\n");

	if (IS_ENABLED(CONFIG_CRYPT_PW_SHA256)) {
		err = crypt_compare("$5$", "password", &equals);
		ut_assertf(err == 0, "crypt-sha256 not successful\n");
		ut_assertf(
			equals != 1,
			"crypt-sha256 password hash matched but should not\n");

		err = crypt_compare(
			"$5$rounds=640000$TM4lL4zXDG7F4aRX$JM7a9wmvodnA0WasjTztj6mxg.KVuk6doQ/eBhdcapB",
			"password", &equals);
		ut_assertf(err == 0, "crypt-sha256 failed: %d\n", err);
		ut_assertf(equals == 1,
			   "crypt-sha256 password hash didn't match\n");
	}
	equals = 0;
	if (IS_ENABLED(CONFIG_CRYPT_PW_SHA512)) {
		err = crypt_compare("$6$", "password", &equals);
		ut_assertf(err == 0, "crypt-sha512 not successful\n");
		ut_assertf(
			equals != 1,
			"crypt-sha512 password hash matched but should not\n");

		err = crypt_compare(
			"$6$rounds=640000$fCTP1F0N5JLq2eND$z5EzK5KZJA9JnOaj5d1Gg/2v6VqFOQJ3bVekWuCPauabutBt/8qzV1exJnytUyhbq3H0bSBXtodwNbtGEi/Tm/",
			"password", &equals);
		ut_assertf(err == 0, "crypt-sha512 failed: %d\n", err);
		ut_assertf(equals == 1,
			   "crypt-sha512 password hash didn't match\n");
	}

	return CMD_RET_SUCCESS;
}

LIB_TEST(lib_crypt, 0);
