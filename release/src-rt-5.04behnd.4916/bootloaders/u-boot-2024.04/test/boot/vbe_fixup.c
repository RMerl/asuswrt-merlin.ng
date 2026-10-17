// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for VBE device tree fix-ups
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm/ofnode.h>
#include <linux/libfdt.h>
#include <test/test.h>
#include <test/ut.h>
#include "bootstd_common.h"

/*
 * Basic test of reading nvdata and updating a fwupd node in the device tree
 * This test works when called from test_vbe.py and it must use the flat tree,
 * since device tree fix-ups do not yet support live tree.
 */
static int vbe_test_fixup_norun(struct unit_test_state *uts)
{
	ofnode chosen, node;
	const char *data;
	oftree tree;
	int size;

	tree = oftree_from_fdt(working_fdt);
	ut_assert(oftree_valid(tree));

	chosen = oftree_path(tree, "/chosen");
	ut_assert(ofnode_valid(chosen));

	/* check the things set up for the FIT in test_vbe.py */
	node = ofnode_find_subnode(chosen, "random");

	/* ignore if this test is run on its own */
	if (!ofnode_valid(node))
		return 0;
	data = ofnode_read_prop(node, "data", &size);
	ut_asserteq(0x40, size);

	node = ofnode_find_subnode(chosen, "aslr2");
	ut_assert(ofnode_valid(node));
	data = ofnode_read_prop(node, "data", &size);
	ut_asserteq(4, size);

	node = ofnode_find_subnode(chosen, "efi-runtime");
	ut_assert(ofnode_valid(node));
	data = ofnode_read_prop(node, "data", &size);
	ut_asserteq(4, size);

	return 0;
}
BOOTSTD_TEST(vbe_test_fixup_norun, UT_TESTF_DM | UT_TESTF_SCAN_FDT |
	     UT_TESTF_FLAT_TREE | UT_TESTF_MANUAL);
