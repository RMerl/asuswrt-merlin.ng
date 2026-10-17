// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for vbe-simple bootmeth. All start with 'vbe_simple'
 *
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <bootmeth.h>
#include <dm.h>
#include <image.h>
#include <of_live.h>
#include <vbe.h>
#include <test/suites.h>
#include <test/ut.h>
#include "bootstd_common.h"

/*
 * Basic test of reading nvdata and updating a fwupd node in the device tree
 *
 * This sets up its own VBE info in the device, using bootstd_setup_for_tests()
 * then does a VBE fixup and checks that everything is present.
 */
static int vbe_simple_test_base(struct unit_test_state *uts)
{
	const char *version, *bl_version;
	struct event_ft_fixup fixup;
	struct udevice *dev;
	struct device_node *np;
	char fdt_buf[0x400];
	char info[100];
	int node_ofs;
	ofnode node;
	u32 vernum;

	/* Set up the VBE info */
	ut_assertok(bootstd_setup_for_tests());

	/* Read the version back */
	ut_assertok(vbe_find_by_any("firmware0", &dev));
	ut_assertok(bootmeth_get_state_desc(dev, info, sizeof(info)));
	ut_asserteq_str("Version: " TEST_VERSION "\nVernum: 1/2", info);

	ut_assertok(fdt_create_empty_tree(fdt_buf, sizeof(fdt_buf)));
	node_ofs = fdt_add_subnode(fdt_buf, 0, "chosen");
	ut_assert(node_ofs > 0);

	node_ofs = fdt_add_subnode(fdt_buf, node_ofs, "fwupd");
	ut_assert(node_ofs > 0);

	node_ofs = fdt_add_subnode(fdt_buf, node_ofs, "firmware0");
	ut_assert(node_ofs > 0);

	if (of_live_active()) {
		ut_assertok(unflatten_device_tree(fdt_buf, &np));
		fixup.tree = oftree_from_np(np);
	} else {
		fixup.tree = oftree_from_fdt(fdt_buf);
	}

	/*
	 * It would be better to call image_setup_libfdt() here, but that
	 * function does not allow passing an ofnode. We can pass fdt_buf but
	 * when it comes to send the event, it creates an ofnode that uses the
	 * control FDT, since it has no way of accessing the live tree created
	 * here.
	 *
	 * Two fix this we need image_setup_libfdt() is updated to use ofnode
	 */
	fixup.images = NULL;
	ut_assertok(event_notify(EVT_FT_FIXUP, &fixup, sizeof(fixup)));

	node = oftree_path(fixup.tree, "/chosen/fwupd/firmware0");

	version = ofnode_read_string(node, "cur-version");
	ut_assertnonnull(version);
	ut_asserteq_str(TEST_VERSION, version);

	ut_assertok(ofnode_read_u32(node, "cur-vernum", &vernum));
	ut_asserteq(TEST_VERNUM, vernum);

	bl_version = ofnode_read_string(node, "bootloader-version");
	ut_assertnonnull(bl_version);
	ut_asserteq_str(version_string + 7, bl_version);

	return 0;
}
BOOTSTD_TEST(vbe_simple_test_base, UT_TESTF_DM | UT_TESTF_SCAN_FDT);
