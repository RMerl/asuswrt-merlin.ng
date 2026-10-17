// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <dm/of_extra.h>
#include <dm/test.h>
#include <test/ut.h>
#include <u-boot/sha256.h>

static int dm_test_ofnode_read_fmap_entry(struct unit_test_state *uts)
{
	const char hash_expect[] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
		0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	};
	struct fmap_entry entry;
	ofnode node;

	node = ofnode_path("/cros-ec/flash/wp-ro");
	ut_assertok(ofnode_read_fmap_entry(node, &entry));
	ut_asserteq(0xf000, entry.offset);
	ut_asserteq(0x1000, entry.length);
	ut_asserteq(0x884, entry.used);
	ut_asserteq(FMAP_COMPRESS_LZ4, entry.compress_algo);
	ut_asserteq(0xcf8, entry.unc_length);
	ut_asserteq(FMAP_HASH_SHA256, entry.hash_algo);
	ut_asserteq(SHA256_SUM_LEN, entry.hash_size);
	ut_asserteq_mem(hash_expect, entry.hash, SHA256_SUM_LEN);

	return 0;
}
DM_TEST(dm_test_ofnode_read_fmap_entry, 0);

static int dm_test_ofnode_phy_is_fixed_link(struct unit_test_state *uts)
{
	ofnode eth_node, phy_node, node;

	eth_node = ofnode_path("/dsa-test/ports/port@0");
	ut_assert(ofnode_phy_is_fixed_link(eth_node, &phy_node));
	node = ofnode_path("/dsa-test/ports/port@0/fixed-link");
	ut_asserteq_mem(&phy_node, &node, sizeof(ofnode));

	eth_node = ofnode_path("/dsa-test/ports/port@1");
	ut_assert(ofnode_phy_is_fixed_link(eth_node, &phy_node));
	node = eth_node;
	ut_asserteq_mem(&phy_node, &node, sizeof(ofnode));

	return 0;
}
DM_TEST(dm_test_ofnode_phy_is_fixed_link, 0);
