// SPDX-License-Identifier: GPL-2.0+
/*
 * Functional tests for UCLASS_FFA  class
 *
 * Copyright 2022-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * Authors:
 *   Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 */

#include <common.h>
#include <uuid.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

/* test UUID */
#define TEST_SVC_UUID	"ed32d533-4209-99e6-2d72-cdd998a79cc0"

#define UUID_SIZE 16

/* The UUID binary data (little-endian format) */
static const u8 ref_uuid_bin[UUID_SIZE] = {
	0x33, 0xd5, 0x32, 0xed,
	0x09, 0x42, 0xe6, 0x99,
	0x72, 0x2d, 0xc0, 0x9c,
	0xa7, 0x98, 0xd9, 0xcd
};

static int lib_test_uuid_to_le(struct unit_test_state *uts)
{
	const char *uuid_str = TEST_SVC_UUID;
	u8 ret_uuid_bin[UUID_SIZE] = {0};

	ut_assertok(uuid_str_to_le_bin(uuid_str, ret_uuid_bin));
	ut_asserteq_mem(ref_uuid_bin, ret_uuid_bin, UUID_SIZE);

	return 0;
}

LIB_TEST(lib_test_uuid_to_le, 0);
