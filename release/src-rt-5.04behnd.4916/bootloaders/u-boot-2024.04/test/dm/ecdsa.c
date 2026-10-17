// SPDX-License-Identifier: GPL-2.0-or-later

#include <crypto/ecdsa-uclass.h>
#include <dm.h>
#include <dm/test.h>
#include <test/ut.h>
#include <u-boot/ecdsa.h>

/*
 * Basic test of the ECDSA uclass and ecdsa_verify()
 *
 * ECDSA implementations in u-boot are hardware-dependent. Until we have a
 * software implementation that can be compiled into the sandbox, all we can
 * test is the uclass support.
 *
 * The uclass_get() test is redundant since ecdsa_verify() would also fail. We
 * run both functions in order to isolate the cause more clearly. i.e. is
 * ecdsa_verify() failing because the UCLASS is absent/broken?
 */
static int dm_test_ecdsa_verify(struct unit_test_state *uts)
{
	struct uclass *ucp;

	struct checksum_algo algo = {
		.checksum_len = 256,
	};

	struct image_sign_info info = {
		.checksum = &algo,
	};

	ut_assertok(uclass_get(UCLASS_ECDSA, &ucp));
	ut_assertnonnull(ucp);
	ut_asserteq(-ENODEV, ecdsa_verify(&info, NULL, 0, NULL, 0));

	return 0;
}
DM_TEST(dm_test_ecdsa_verify, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
