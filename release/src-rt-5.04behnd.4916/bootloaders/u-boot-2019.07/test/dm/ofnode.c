// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <dm.h>
#include <dm/of_extra.h>
#include <dm/test.h>
#include <test/ut.h>

static int dm_test_ofnode_compatible(struct unit_test_state *uts)
{
	ofnode root_node = ofnode_path("/");

	ut_assert(ofnode_valid(root_node));
	ut_assert(ofnode_device_is_compatible(root_node, "sandbox"));

	return 0;
}
DM_TEST(dm_test_ofnode_compatible, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

static int dm_test_ofnode_by_prop_value(struct unit_test_state *uts)
{
	const char propname[] = "compatible";
	const char propval[] = "denx,u-boot-fdt-test";
	const char *str;
	ofnode node = ofnode_null();

	/* Find first matching node, there should be at least one */
	node = ofnode_by_prop_value(node, propname, propval, sizeof(propval));
	ut_assert(ofnode_valid(node));
	str = ofnode_read_string(node, propname);
	ut_assert(str && !strcmp(str, propval));

	/* Find the rest of the matching nodes */
	while (true) {
		node = ofnode_by_prop_value(node, propname, propval,
					    sizeof(propval));
		if (!ofnode_valid(node))
			break;
		str = ofnode_read_string(node, propname);
		ut_assert(str && !strcmp(str, propval));
	}

	return 0;
}
DM_TEST(dm_test_ofnode_by_prop_value, DM_TESTF_SCAN_FDT);

static int dm_test_ofnode_fmap(struct unit_test_state *uts)
{
	struct fmap_entry entry;
	ofnode node;

	node = ofnode_path("/cros-ec/flash");
	ut_assert(ofnode_valid(node));
	ut_assertok(ofnode_read_fmap_entry(node, &entry));
	ut_asserteq(0x08000000, entry.offset);
	ut_asserteq(0x20000, entry.length);

	return 0;
}
DM_TEST(dm_test_ofnode_fmap, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
