// SPDX-License-Identifier: GPL-2.0+
/*
 *  DM tag test
 *
 *  Copyright (c) 2021 Linaro Limited
 *  Author: AKASHI Takahiro
 */

#include <common.h>
#include <dm/tag.h>
#include <dm/test.h> /* DM_TEST() */
#include <test/test.h> /* struct unit_test_state */
#include <test/ut.h> /* assertions */

/*
 * Test dm_tag_ptr() API
 */
static int dm_test_tag_ptr(struct unit_test_state *uts)
{
	ulong val;
	void *ptr = NULL;

	ut_assertok(dev_tag_set_ptr(uts->root, DM_TAG_EFI, &val));

	ut_assertok(dev_tag_get_ptr(uts->root, DM_TAG_EFI, &ptr));

	ut_asserteq_ptr(&val, ptr);

	ut_assertok(dev_tag_del(uts->root, DM_TAG_EFI));

	return 0;
}

DM_TEST(dm_test_tag_ptr, 0);

/*
 * Test dm_tag_val() API
 */
static int dm_test_tag_val(struct unit_test_state *uts)
{
	ulong val1 = 0x12345678, val2 = 0;

	ut_assertok(dev_tag_set_val(uts->root, DM_TAG_EFI, val1));

	ut_assertok(dev_tag_get_val(uts->root, DM_TAG_EFI, &val2));

	ut_asserteq_64(val1, val2);

	ut_assertok(dev_tag_del(uts->root, DM_TAG_EFI));

	return 0;
}

DM_TEST(dm_test_tag_val, 0);

/*
 * Test against an invalid tag
 */
static int dm_test_tag_inval(struct unit_test_state *uts)
{
	ulong val;

	ut_asserteq(-EINVAL, dev_tag_set_ptr(uts->root, DM_TAG_COUNT, &val));

	return 0;
}

DM_TEST(dm_test_tag_inval, 0);

/*
 * Test dm_tag_del_all() AP:
 */
static int dm_test_tag_del_all(struct unit_test_state *uts)
{
	ulong val;

	ut_assertok(dev_tag_set_ptr(uts->root, DM_TAG_EFI, &val));

	ut_assertok(dev_tag_del_all(uts->root));

	return 0;
}

DM_TEST(dm_test_tag_del_all, 0);
