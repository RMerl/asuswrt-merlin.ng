// SPDX-License-Identifier: GPL-2.0+
/*
 * Test device path functions
 *
 * Copyright (c) 2020 Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#include <common.h>
#include <efi_loader.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

static int lib_test_efi_dp_check_length(struct unit_test_state *uts)
{
	/* end of device path */
	u8 d1[] __aligned(2) = {
		0x7f, 0xff, 0x04, 0x00 };
	/* device path node with length less then 4 */
	u8 d2[] __aligned(2) = {
		0x01, 0x02, 0x02, 0x00, 0x04, 0x00, 0x7f, 0xff, 0x04, 0x00 };
	/* well formed device path */
	u8 d3[] __aligned(2) = {
		0x03, 0x02, 0x08, 0x00, 0x01, 0x00, 0x01, 0x00,
		0x7f, 0xff, 0x04, 0x00 };

	struct efi_device_path *p1 = (struct efi_device_path *)d1;
	struct efi_device_path *p2 = (struct efi_device_path *)d2;
	struct efi_device_path *p3 = (struct efi_device_path *)d3;

	ut_asserteq((ssize_t)-EINVAL, efi_dp_check_length(p1, SIZE_MAX));
	ut_asserteq((ssize_t)sizeof(d1), efi_dp_check_length(p1, sizeof(d1)));
	ut_asserteq((ssize_t)sizeof(d1),
		    efi_dp_check_length(p1, sizeof(d1) + 4));
	ut_asserteq((ssize_t)-1, efi_dp_check_length(p1, sizeof(d1) - 1));

	ut_asserteq((ssize_t)-1, efi_dp_check_length(p2, sizeof(d2)));

	ut_asserteq((ssize_t)-1, efi_dp_check_length(p3, sizeof(d3) - 1));
	ut_asserteq((ssize_t)sizeof(d3), efi_dp_check_length(p3, sizeof(d3)));
	ut_asserteq((ssize_t)sizeof(d3), efi_dp_check_length(p3, SSIZE_MAX));
	ut_asserteq((ssize_t)-EINVAL,
		    efi_dp_check_length(p3, (size_t)SSIZE_MAX + 1));
	ut_asserteq((ssize_t)sizeof(d3),
		    efi_dp_check_length(p3, sizeof(d3) + 4));

	return 0;
}

LIB_TEST(lib_test_efi_dp_check_length, 0);
