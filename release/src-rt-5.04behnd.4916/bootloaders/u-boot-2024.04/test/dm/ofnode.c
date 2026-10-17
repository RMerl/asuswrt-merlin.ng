// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Google LLC
 *
 * There are two types of tests in this file:
 * - normal ones which act on the control FDT (gd->fdt_blob or gd->of_root)
 * - 'other' ones which act on the 'other' FDT (other.dts)
 *
 * The 'other' ones have an _ot suffix.
 *
 * The latter are used to check behaviour with multiple device trees,
 * particularly with flat tree, where a tree ID is included in ofnode as part of
 * the node offset. These tests are typically just for making sure that the
 * offset makes it to libfdt correctly and that the resulting return value is
 * correctly turned into an ofnode. The 'other' tests do not fully check the
 * behaviour of each ofnode function, since that is done by the normal ones.
 */

#include <common.h>
#include <abuf.h>
#include <dm.h>
#include <log.h>
#include <of_live.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/of_extra.h>
#include <dm/root.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <linux/sizes.h>
#include <test/test.h>
#include <test/ut.h>

/**
 * get_other_oftree() - Convert a flat tree into an oftree object
 *
 * @uts: Test state
 * @return: oftree object for the 'other' FDT (see sandbox' other.dts)
 */
oftree get_other_oftree(struct unit_test_state *uts)
{
	oftree tree;

	if (of_live_active())
		tree = oftree_from_np(uts->of_other);
	else
		tree = oftree_from_fdt(uts->other_fdt);

	/* An invalid tree may cause failure or crashes */
	if (!oftree_valid(tree))
		ut_reportf("test needs the UT_TESTF_OTHER_FDT flag");

	return tree;
}

/**
 * get_oftree() - Convert a flat tree into an oftree object
 *
 * @uts: Test state
 * @fdt: Pointer to flat tree
 * @treep: Returns the tree, on success
 * Return: 0 if OK, 1 if the tree failed to unflatten, -EOVERFLOW if there are
 * too many flat trees to allow another one to be registers (see
 * oftree_ensure())
 */
int get_oftree(struct unit_test_state *uts, void *fdt, oftree *treep)
{
	oftree tree;

	if (of_live_active()) {
		struct device_node *root;

		ut_assertok(unflatten_device_tree(fdt, &root));
		tree = oftree_from_np(root);
	} else {
		tree = oftree_from_fdt(fdt);
		if (!oftree_valid(tree))
			return -EOVERFLOW;
	}
	*treep = tree;

	return 0;
}

/**
 * free_oftree() - Free memory used by get_oftree()
 *
 * @tree: Tree to free
 */
void free_oftree(oftree tree)
{
	if (of_live_active())
		free(tree.np);
}

/* test ofnode_device_is_compatible() */
static int dm_test_ofnode_compatible(struct unit_test_state *uts)
{
	ofnode root_node = ofnode_path("/");

	ut_assert(ofnode_valid(root_node));
	ut_assert(ofnode_device_is_compatible(root_node, "sandbox"));

	return 0;
}
DM_TEST(dm_test_ofnode_compatible,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* check ofnode_device_is_compatible() with the 'other' FDT */
static int dm_test_ofnode_compatible_ot(struct unit_test_state *uts)
{
	oftree otree = get_other_oftree(uts);
	ofnode oroot = oftree_root(otree);

	ut_assert(ofnode_valid(oroot));
	ut_assert(ofnode_device_is_compatible(oroot, "sandbox-other"));

	return 0;
}
DM_TEST(dm_test_ofnode_compatible_ot, UT_TESTF_SCAN_FDT | UT_TESTF_OTHER_FDT);

static int dm_test_ofnode_get_by_phandle(struct unit_test_state *uts)
{
	/* test invalid phandle */
	ut_assert(!ofnode_valid(ofnode_get_by_phandle(0)));
	ut_assert(!ofnode_valid(ofnode_get_by_phandle(-1)));

	/* test first valid phandle */
	ut_assert(ofnode_valid(ofnode_get_by_phandle(1)));

	/* test unknown phandle */
	ut_assert(!ofnode_valid(ofnode_get_by_phandle(0x1000000)));

	ut_assert(ofnode_valid(oftree_get_by_phandle(oftree_default(), 1)));

	return 0;
}
DM_TEST(dm_test_ofnode_get_by_phandle, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* test oftree_get_by_phandle() with a the 'other' oftree */
static int dm_test_ofnode_get_by_phandle_ot(struct unit_test_state *uts)
{
	oftree otree = get_other_oftree(uts);
	ofnode node;

	ut_assert(ofnode_valid(oftree_get_by_phandle(oftree_default(), 1)));
	node = oftree_get_by_phandle(otree, 1);
	ut_assert(ofnode_valid(node));
	ut_asserteq_str("target", ofnode_get_name(node));

	return 0;
}
DM_TEST(dm_test_ofnode_get_by_phandle_ot,
	UT_TESTF_SCAN_FDT | UT_TESTF_OTHER_FDT);

static int check_prop_values(struct unit_test_state *uts, ofnode start,
			     const char *propname, const char *propval,
			     int expect_count)
{
	int proplen = strlen(propval) + 1;
	const char *str;
	ofnode node;
	int count;

	/* Find first matching node, there should be at least one */
	node = ofnode_by_prop_value(start, propname, propval, proplen);
	ut_assert(ofnode_valid(node));
	str = ofnode_read_string(node, propname);
	ut_assert(str && !strcmp(str, propval));

	/* Find the rest of the matching nodes */
	count = 1;
	while (true) {
		node = ofnode_by_prop_value(node, propname, propval, proplen);
		if (!ofnode_valid(node))
			break;
		str = ofnode_read_string(node, propname);
		ut_asserteq_str(propval, str);
		count++;
	}
	ut_asserteq(expect_count, count);

	return 0;
}

static int dm_test_ofnode_by_prop_value(struct unit_test_state *uts)
{
	ut_assertok(check_prop_values(uts, ofnode_null(), "compatible",
				      "denx,u-boot-fdt-test", 11));

	return 0;
}
DM_TEST(dm_test_ofnode_by_prop_value, UT_TESTF_SCAN_FDT);

/* test ofnode_by_prop_value() with a the 'other' oftree */
static int dm_test_ofnode_by_prop_value_ot(struct unit_test_state *uts)
{
	oftree otree = get_other_oftree(uts);

	ut_assertok(check_prop_values(uts, oftree_root(otree), "str-prop",
				      "other", 2));

	return 0;
}
DM_TEST(dm_test_ofnode_by_prop_value_ot,
	UT_TESTF_SCAN_FDT | UT_TESTF_OTHER_FDT);

/* test ofnode_read_fmap_entry() */
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
DM_TEST(dm_test_ofnode_fmap, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* test ofnode_read_prop() */
static int dm_test_ofnode_read(struct unit_test_state *uts)
{
	const u32 *val;
	ofnode node;
	int size;

	node = oftree_path(oftree_default(), "/");
	ut_assert(ofnode_valid(node));

	node = ofnode_path("/a-test");
	ut_assert(ofnode_valid(node));

	val = ofnode_read_prop(node, "int-value", &size);
	ut_assertnonnull(val);
	ut_asserteq(4, size);
	ut_asserteq(1234, fdt32_to_cpu(val[0]));

	val = ofnode_read_prop(node, "missing", &size);
	ut_assertnull(val);
	ut_asserteq(-FDT_ERR_NOTFOUND, size);

	/* Check it works without a size parameter */
	val = ofnode_read_prop(node, "missing", NULL);
	ut_assertnull(val);

	return 0;
}
DM_TEST(dm_test_ofnode_read, UT_TESTF_SCAN_FDT);

/* test ofnode_read_prop() with the 'other' tree */
static int dm_test_ofnode_read_ot(struct unit_test_state *uts)
{
	oftree otree = get_other_oftree(uts);
	const char *val;
	ofnode node;
	int size;

	node = oftree_path(otree, "/");
	ut_assert(ofnode_valid(node));

	node = oftree_path(otree, "/node/subnode");
	ut_assert(ofnode_valid(node));

	val = ofnode_read_prop(node, "str-prop", &size);
	ut_assertnonnull(val);
	ut_asserteq_str("other", val);
	ut_asserteq(6, size);

	return 0;
}
DM_TEST(dm_test_ofnode_read_ot, UT_TESTF_SCAN_FDT | UT_TESTF_OTHER_FDT);

/* test ofnode_count_/parse_phandle_with_args() */
static int dm_test_ofnode_phandle(struct unit_test_state *uts)
{
	struct ofnode_phandle_args args;
	ofnode node;
	int ret;
	const char prop[] = "test-gpios";
	const char cell[] = "#gpio-cells";
	const char prop2[] = "phandle-value";

	node = ofnode_path("/a-test");
	ut_assert(ofnode_valid(node));

	/* Test ofnode_count_phandle_with_args with cell name */
	ret = ofnode_count_phandle_with_args(node, "missing", cell, 0);
	ut_asserteq(-ENOENT, ret);
	ret = ofnode_count_phandle_with_args(node, prop, "#invalid", 0);
	ut_asserteq(-EINVAL, ret);
	ret = ofnode_count_phandle_with_args(node, prop, cell, 0);
	ut_asserteq(5, ret);

	/* Test ofnode_parse_phandle_with_args with cell name */
	ret = ofnode_parse_phandle_with_args(node, "missing", cell, 0, 0,
					     &args);
	ut_asserteq(-ENOENT, ret);
	ret = ofnode_parse_phandle_with_args(node, prop, "#invalid", 0, 0,
					     &args);
	ut_asserteq(-EINVAL, ret);
	ret = ofnode_parse_phandle_with_args(node, prop, cell, 0, 0, &args);
	ut_assertok(ret);
	ut_asserteq(1, args.args_count);
	ut_asserteq(1, args.args[0]);
	ret = ofnode_parse_phandle_with_args(node, prop, cell, 0, 1, &args);
	ut_assertok(ret);
	ut_asserteq(1, args.args_count);
	ut_asserteq(4, args.args[0]);
	ret = ofnode_parse_phandle_with_args(node, prop, cell, 0, 2, &args);
	ut_assertok(ret);
	ut_asserteq(5, args.args_count);
	ut_asserteq(5, args.args[0]);
	ut_asserteq(1, args.args[4]);
	ret = ofnode_parse_phandle_with_args(node, prop, cell, 0, 3, &args);
	ut_asserteq(-ENOENT, ret);
	ret = ofnode_parse_phandle_with_args(node, prop, cell, 0, 4, &args);
	ut_assertok(ret);
	ut_asserteq(1, args.args_count);
	ut_asserteq(12, args.args[0]);
	ret = ofnode_parse_phandle_with_args(node, prop, cell, 0, 5, &args);
	ut_asserteq(-ENOENT, ret);

	/* Test ofnode_count_phandle_with_args with cell count */
	ret = ofnode_count_phandle_with_args(node, "missing", NULL, 2);
	ut_asserteq(-ENOENT, ret);
	ret = ofnode_count_phandle_with_args(node, prop2, NULL, 1);
	ut_asserteq(3, ret);

	/* Test ofnode_parse_phandle_with_args with cell count */
	ret = ofnode_parse_phandle_with_args(node, prop2, NULL, 1, 0, &args);
	ut_assertok(ret);
	ut_asserteq(1, ofnode_valid(args.node));
	ut_asserteq(1, args.args_count);
	ut_asserteq(10, args.args[0]);
	ret = ofnode_parse_phandle_with_args(node, prop2, NULL, 1, 1, &args);
	ut_asserteq(-EINVAL, ret);
	ret = ofnode_parse_phandle_with_args(node, prop2, NULL, 1, 2, &args);
	ut_assertok(ret);
	ut_asserteq(1, ofnode_valid(args.node));
	ut_asserteq(1, args.args_count);
	ut_asserteq(30, args.args[0]);
	ret = ofnode_parse_phandle_with_args(node, prop2, NULL, 1, 3, &args);
	ut_asserteq(-ENOENT, ret);

	return 0;
}
DM_TEST(dm_test_ofnode_phandle, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* test ofnode_count_/parse_phandle_with_args() with 'other' tree */
static int dm_test_ofnode_phandle_ot(struct unit_test_state *uts)
{
	oftree otree = get_other_oftree(uts);
	struct ofnode_phandle_args args;
	ofnode node;
	int ret;

	node = oftree_path(otree, "/node");

	/* Test ofnode_count_phandle_with_args with cell name */
	ret = ofnode_count_phandle_with_args(node, "missing", "#gpio-cells", 0);
	ut_asserteq(-ENOENT, ret);
	ret = ofnode_count_phandle_with_args(node, "target", "#invalid", 0);
	ut_asserteq(-EINVAL, ret);
	ret = ofnode_count_phandle_with_args(node, "target", "#gpio-cells", 0);
	ut_asserteq(1, ret);

	ret = ofnode_parse_phandle_with_args(node, "target", "#gpio-cells", 0,
					     0, &args);
	ut_assertok(ret);
	ut_asserteq(2, args.args_count);
	ut_asserteq(3, args.args[0]);
	ut_asserteq(4, args.args[1]);

	return 0;
}
DM_TEST(dm_test_ofnode_phandle_ot, UT_TESTF_OTHER_FDT);

/* test ofnode_read_chosen_string/node/prop() */
static int dm_test_ofnode_read_chosen(struct unit_test_state *uts)
{
	const char *str;
	const u32 *val;
	ofnode node;
	int size;

	str = ofnode_read_chosen_string("setting");
	ut_assertnonnull(str);
	ut_asserteq_str("sunrise ohoka", str);
	ut_asserteq_ptr(NULL, ofnode_read_chosen_string("no-setting"));

	node = ofnode_get_chosen_node("other-node");
	ut_assert(ofnode_valid(node));
	ut_asserteq_str("c-test@5", ofnode_get_name(node));

	node = ofnode_get_chosen_node("setting");
	ut_assert(!ofnode_valid(node));

	val = ofnode_read_chosen_prop("int-values", &size);
	ut_assertnonnull(val);
	ut_asserteq(8, size);
	ut_asserteq(0x1937, fdt32_to_cpu(val[0]));
	ut_asserteq(72993, fdt32_to_cpu(val[1]));

	return 0;
}
DM_TEST(dm_test_ofnode_read_chosen, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* test ofnode_get_aliases_node/prop() */
static int dm_test_ofnode_read_aliases(struct unit_test_state *uts)
{
	const void *val;
	ofnode node;
	int size;

	node = ofnode_get_aliases_node("ethernet3");
	ut_assert(ofnode_valid(node));
	ut_asserteq_str("sbe5", ofnode_get_name(node));

	node = ofnode_get_aliases_node("unknown");
	ut_assert(!ofnode_valid(node));

	val = ofnode_read_aliases_prop("spi0", &size);
	ut_assertnonnull(val);
	ut_asserteq(7, size);
	ut_asserteq_str("/spi@0", (const char *)val);

	return 0;
}
DM_TEST(dm_test_ofnode_read_aliases, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_ofnode_get_child_count(struct unit_test_state *uts)
{
	ofnode node, child_node;
	u32 val;

	node = ofnode_path("/i-test");
	ut_assert(ofnode_valid(node));

	val = ofnode_get_child_count(node);
	ut_asserteq(3, val);

	child_node = ofnode_first_subnode(node);
	ut_assert(ofnode_valid(child_node));
	val = ofnode_get_child_count(child_node);
	ut_asserteq(0, val);

	return 0;
}
DM_TEST(dm_test_ofnode_get_child_count,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* test ofnode_get_child_count() with 'other' tree */
static int dm_test_ofnode_get_child_count_ot(struct unit_test_state *uts)
{
	oftree otree = get_other_oftree(uts);
	ofnode node, child_node;
	u32 val;

	node = oftree_path(otree, "/node");
	ut_assert(ofnode_valid(node));

	val = ofnode_get_child_count(node);
	ut_asserteq(2, val);

	child_node = ofnode_first_subnode(node);
	ut_assert(ofnode_valid(child_node));
	val = ofnode_get_child_count(child_node);
	ut_asserteq(0, val);

	return 0;
}
DM_TEST(dm_test_ofnode_get_child_count_ot,
	UT_TESTF_SCAN_FDT | UT_TESTF_OTHER_FDT);

static int dm_test_ofnode_is_enabled(struct unit_test_state *uts)
{
	ofnode root_node = ofnode_path("/");
	ofnode node = ofnode_path("/usb@0");

	ut_assert(ofnode_is_enabled(root_node));
	ut_assert(!ofnode_is_enabled(node));

	return 0;
}
DM_TEST(dm_test_ofnode_is_enabled, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* test ofnode_is_enabled() with 'other' tree */
static int dm_test_ofnode_is_enabled_ot(struct unit_test_state *uts)
{
	oftree otree = get_other_oftree(uts);
	ofnode root_node = oftree_root(otree);
	ofnode node = oftree_path(otree, "/target");

	ut_assert(ofnode_is_enabled(root_node));
	ut_assert(!ofnode_is_enabled(node));

	return 0;
}
DM_TEST(dm_test_ofnode_is_enabled_ot, UT_TESTF_OTHER_FDT);

/* test ofnode_get_addr/size() */
static int dm_test_ofnode_get_reg(struct unit_test_state *uts)
{
	ofnode node;
	fdt_addr_t addr;
	fdt_size_t size;

	node = ofnode_path("/translation-test@8000");
	ut_assert(ofnode_valid(node));
	addr = ofnode_get_addr(node);
	size = ofnode_get_size(node);
	ut_asserteq(0x8000, addr);
	ut_asserteq(0x4000, size);

	node = ofnode_path("/translation-test@8000/dev@1,100");
	ut_assert(ofnode_valid(node));
	addr = ofnode_get_addr(node);
	size = ofnode_get_size(node);
	ut_asserteq(0x9000, addr);
	ut_asserteq(0x1000, size);

	node = ofnode_path("/emul-mux-controller");
	ut_assert(ofnode_valid(node));
	addr = ofnode_get_addr(node);
	size = ofnode_get_size(node);
	ut_asserteq_64(FDT_ADDR_T_NONE, addr);
	ut_asserteq(FDT_SIZE_T_NONE, size);

	node = ofnode_path("/translation-test@8000/noxlatebus@3,300/dev@42");
	ut_assert(ofnode_valid(node));
	addr = ofnode_get_addr_size_index_notrans(node, 0, &size);
	ut_asserteq_64(0x42, addr);

	return 0;
}
DM_TEST(dm_test_ofnode_get_reg, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* test ofnode_get_addr() with 'other' tree */
static int dm_test_ofnode_get_reg_ot(struct unit_test_state *uts)
{
	oftree otree = get_other_oftree(uts);
	ofnode node = oftree_path(otree, "/target");
	fdt_addr_t addr;

	addr = ofnode_get_addr(node);
	ut_asserteq(0x8000, addr);

	return 0;
}
DM_TEST(dm_test_ofnode_get_reg_ot, UT_TESTF_SCAN_FDT | UT_TESTF_OTHER_FDT);

static int dm_test_ofnode_get_path(struct unit_test_state *uts)
{
	const char *path = "/translation-test@8000/noxlatebus@3,300/dev@42";
	char buf[64];
	ofnode node;

	node = ofnode_path(path);
	ut_assert(ofnode_valid(node));

	ut_assertok(ofnode_get_path(node, buf, sizeof(buf)));
	ut_asserteq_str(path, buf);

	ut_asserteq(-ENOSPC, ofnode_get_path(node, buf, 32));

	ut_assertok(ofnode_get_path(ofnode_root(), buf, 32));
	ut_asserteq_str("/", buf);

	return 0;
}
DM_TEST(dm_test_ofnode_get_path, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* test ofnode_get_path() with 'other' tree */
static int dm_test_ofnode_get_path_ot(struct unit_test_state *uts)
{
	oftree otree = get_other_oftree(uts);
	const char *path = "/node/subnode";
	ofnode node = oftree_path(otree, path);
	char buf[64];

	ut_assert(ofnode_valid(node));

	ut_assertok(ofnode_get_path(node, buf, sizeof(buf)));
	ut_asserteq_str(path, buf);

	ut_assertok(ofnode_get_path(oftree_root(otree), buf, 32));
	ut_asserteq_str("/", buf);

	return 0;
}
DM_TEST(dm_test_ofnode_get_path_ot, UT_TESTF_SCAN_FDT | UT_TESTF_OTHER_FDT);

/* test ofnode_conf_read_bool/int/str() */
static int dm_test_ofnode_conf(struct unit_test_state *uts)
{
	ut_assert(!ofnode_conf_read_bool("missing"));
	ut_assert(ofnode_conf_read_bool("testing-bool"));

	ut_asserteq(123, ofnode_conf_read_int("testing-int", 0));
	ut_asserteq(6, ofnode_conf_read_int("missing", 6));

	ut_assertnull(ofnode_conf_read_str("missing"));
	ut_asserteq_str("testing", ofnode_conf_read_str("testing-str"));

	return 0;
}
DM_TEST(dm_test_ofnode_conf, UT_TESTF_SCAN_FDT);

static int dm_test_ofnode_options(struct unit_test_state *uts)
{
	u64 bootscr_address, bootscr_offset;
	u64 bootscr_flash_offset, bootscr_flash_size;

	ut_assertok(ofnode_read_bootscript_address(&bootscr_address,
						   &bootscr_offset));
	ut_asserteq_64(0, bootscr_address);
	ut_asserteq_64(0x12345678, bootscr_offset);

	ut_assertok(ofnode_read_bootscript_flash(&bootscr_flash_offset,
						 &bootscr_flash_size));
	ut_asserteq_64(0, bootscr_flash_offset);
	ut_asserteq_64(0x2000, bootscr_flash_size);

	return 0;
}
DM_TEST(dm_test_ofnode_options, 0);

static int dm_test_ofnode_for_each_compatible_node(struct unit_test_state *uts)
{
	const char compatible[] = "denx,u-boot-fdt-test";
	bool found = false;
	ofnode node;

	ofnode_for_each_compatible_node(node, compatible) {
		ut_assert(ofnode_device_is_compatible(node, compatible));
		found = true;
	}

	/* There should be at least one matching node */
	ut_assert(found);

	return 0;
}
DM_TEST(dm_test_ofnode_for_each_compatible_node, UT_TESTF_SCAN_FDT);

/* test dm_test_ofnode_string_count/index/list() */
static int dm_test_ofnode_string(struct unit_test_state *uts)
{
	const char **val;
	const char *out;
	ofnode node;

	node = ofnode_path("/a-test");
	ut_assert(ofnode_valid(node));

	/* single string */
	ut_asserteq(1, ofnode_read_string_count(node, "str-value"));
	ut_assertok(ofnode_read_string_index(node, "str-value", 0, &out));
	ut_asserteq_str("test string", out);
	ut_asserteq(0, ofnode_stringlist_search(node, "str-value",
						"test string"));
	ut_asserteq(1, ofnode_read_string_list(node, "str-value", &val));
	ut_asserteq_str("test string", val[0]);
	ut_assertnull(val[1]);
	free(val);

	/* list of strings */
	ut_asserteq(5, ofnode_read_string_count(node, "mux-control-names"));
	ut_assertok(ofnode_read_string_index(node, "mux-control-names", 0,
					     &out));
	ut_asserteq_str("mux0", out);
	ut_asserteq(0, ofnode_stringlist_search(node, "mux-control-names",
						"mux0"));
	ut_asserteq(5, ofnode_read_string_list(node, "mux-control-names",
					       &val));
	ut_asserteq_str("mux0", val[0]);
	ut_asserteq_str("mux1", val[1]);
	ut_asserteq_str("mux2", val[2]);
	ut_asserteq_str("mux3", val[3]);
	ut_asserteq_str("mux4", val[4]);
	ut_assertnull(val[5]);
	free(val);

	ut_assertok(ofnode_read_string_index(node, "mux-control-names", 4,
					     &out));
	ut_asserteq_str("mux4", out);
	ut_asserteq(4, ofnode_stringlist_search(node, "mux-control-names",
						"mux4"));

	return 0;
}
DM_TEST(dm_test_ofnode_string, UT_TESTF_SCAN_FDT);

/* test error returns from ofnode_read_string_count/index/list() */
static int dm_test_ofnode_string_err(struct unit_test_state *uts)
{
	const char **val;
	const char *out;
	ofnode node;

	/*
	 * Test error codes only on livetree, as they are different with
	 * flattree
	 */
	node = ofnode_path("/a-test");
	ut_assert(ofnode_valid(node));

	/* non-existent property */
	ut_asserteq(-EINVAL, ofnode_read_string_count(node, "missing"));
	ut_asserteq(-EINVAL, ofnode_read_string_index(node, "missing", 0,
						      &out));
	ut_asserteq(-EINVAL, ofnode_read_string_list(node, "missing", &val));

	/* empty property */
	ut_asserteq(-ENODATA, ofnode_read_string_count(node, "bool-value"));
	ut_asserteq(-ENODATA, ofnode_read_string_index(node, "bool-value", 0,
						       &out));
	ut_asserteq(-ENODATA, ofnode_read_string_list(node, "bool-value",
						     &val));

	/* badly formatted string list */
	ut_asserteq(-EILSEQ, ofnode_read_string_count(node, "int64-value"));
	ut_asserteq(-EILSEQ, ofnode_read_string_index(node, "int64-value", 0,
						       &out));
	ut_asserteq(-EILSEQ, ofnode_read_string_list(node, "int64-value",
						     &val));

	/* out of range / not found */
	ut_asserteq(-ENODATA, ofnode_read_string_index(node, "str-value", 1,
						       &out));
	ut_asserteq(-ENODATA, ofnode_stringlist_search(node, "str-value",
						       "other"));

	/* negative value for index is not allowed, so don't test for that */

	ut_asserteq(-ENODATA, ofnode_read_string_index(node,
						       "mux-control-names", 5,
						       &out));

	return 0;
}
DM_TEST(dm_test_ofnode_string_err, UT_TESTF_LIVE_TREE);

static int dm_test_ofnode_read_phy_mode(struct unit_test_state *uts)
{
	ofnode eth_node, phy_node;
	phy_interface_t mode;
	u32 reg;

	eth_node = ofnode_path("/phy-test-eth");
	ut_assert(ofnode_valid(eth_node));

	mode = ofnode_read_phy_mode(eth_node);
	ut_assert(mode == PHY_INTERFACE_MODE_2500BASEX);

	phy_node = ofnode_get_phy_node(eth_node);
	ut_assert(ofnode_valid(phy_node));

	reg = ofnode_read_u32_default(phy_node, "reg", -1U);
	ut_asserteq_64(0x1, reg);

	return 0;
}
DM_TEST(dm_test_ofnode_read_phy_mode, UT_TESTF_SCAN_FDT);

/**
 * make_ofnode_fdt() - Create an FDT for testing with ofnode
 *
 * The size is set to the minimum needed
 *
 * @uts: Test state
 * @fdt: Place to write FDT
 * @size: Maximum size of space for fdt
 * @id: id value to add to the tree ('id' property in root node)
 */
static int make_ofnode_fdt(struct unit_test_state *uts, void *fdt, int size,
			   int id)
{
	ut_assertok(fdt_create(fdt, size));
	ut_assertok(fdt_finish_reservemap(fdt));
	ut_assert(fdt_begin_node(fdt, "") >= 0);

	ut_assertok(fdt_property_u32(fdt, "id", id));

	ut_assert(fdt_begin_node(fdt, "aliases") >= 0);
	ut_assertok(fdt_property_string(fdt, "mmc0", "/new-mmc"));
	ut_assertok(fdt_end_node(fdt));

	ut_assert(fdt_begin_node(fdt, "new-mmc") >= 0);
	ut_assertok(fdt_end_node(fdt));

	ut_assertok(fdt_end_node(fdt));
	ut_assertok(fdt_finish(fdt));

	return 0;
}

/* Check that aliases work on the control FDT */
static int dm_test_ofnode_aliases(struct unit_test_state *uts)
{
	ofnode node;

	node = ofnode_get_aliases_node("ethernet3");
	ut_assert(ofnode_valid(node));
	ut_asserteq_str("sbe5", ofnode_get_name(node));

	ut_assert(!oftree_valid(oftree_null()));

	return 0;
}
DM_TEST(dm_test_ofnode_aliases, UT_TESTF_SCAN_FDT);

/**
 * dm_test_ofnode_root_mult() - Check aliaes on control and 'other' tree
 *
 * Check that aliases work only with the control FDT, not with 'other' tree.
 * This is not actually the desired behaviour. If aliases are implemented for
 * any tree, then this test should be changed.
 */
static int dm_test_ofnode_root_mult(struct unit_test_state *uts)
{
	char fdt[256];
	oftree tree;
	ofnode node;

	/* skip this test if multiple FDTs are not supported */
	if (!IS_ENABLED(CONFIG_OFNODE_MULTI_TREE))
		return -EAGAIN;

	ut_assertok(make_ofnode_fdt(uts, fdt, sizeof(fdt), 0));
	ut_assertok(get_oftree(uts, fdt, &tree));
	ut_assert(oftree_valid(tree));

	/* Make sure they don't work on this new tree */
	node = oftree_path(tree, "mmc0");
	ut_assert(!ofnode_valid(node));

	/* It should appear in the new tree */
	node = oftree_path(tree, "/new-mmc");
	ut_assert(ofnode_valid(node));

	/* ...and not in the control FDT */
	node = oftree_path(oftree_default(), "/new-mmc");
	ut_assert(!ofnode_valid(node));

	free_oftree(tree);

	return 0;
}
DM_TEST(dm_test_ofnode_root_mult, UT_TESTF_SCAN_FDT);

/* test ofnode_set_enabled(), ofnode_write_prop() on a livetree */
static int dm_test_ofnode_livetree_writing(struct unit_test_state *uts)
{
	struct udevice *dev;
	ofnode node;

	/* Test enabling devices */
	node = ofnode_path("/usb@2");

	ut_assert(!ofnode_is_enabled(node));
	ut_assertok(ofnode_set_enabled(node, true));
	ut_asserteq(true, ofnode_is_enabled(node));

	device_bind_driver_to_node(dm_root(), "usb_sandbox", "usb@2", node,
				   &dev);
	ut_assertok(uclass_find_device_by_seq(UCLASS_USB, 2, &dev));

	/* Test string property setting */
	ut_assert(device_is_compatible(dev, "sandbox,usb"));
	ofnode_write_string(node, "compatible", "gdsys,super-usb");
	ut_assert(device_is_compatible(dev, "gdsys,super-usb"));
	ofnode_write_string(node, "compatible", "sandbox,usb");
	ut_assert(device_is_compatible(dev, "sandbox,usb"));

	/* Test setting generic properties */

	/* Non-existent in DTB */
	ut_asserteq_64(FDT_ADDR_T_NONE, dev_read_addr(dev));
	/* reg = 0x42, size = 0x100 */
	ut_assertok(ofnode_write_prop(node, "reg",
				      "\x00\x00\x00\x42\x00\x00\x01\x00", 8,
				      false));
	ut_asserteq(0x42, dev_read_addr(dev));

	/* Test disabling devices */
	device_remove(dev, DM_REMOVE_NORMAL);
	device_unbind(dev);

	ut_assert(ofnode_is_enabled(node));
	ut_assertok(ofnode_set_enabled(node, false));
	ut_assert(!ofnode_is_enabled(node));

	return 0;
}
DM_TEST(dm_test_ofnode_livetree_writing,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int check_write_prop(struct unit_test_state *uts, ofnode node)
{
	char prop[] = "middle-name";
	char name[10];
	int len;

	strcpy(name, "cecil");
	len = strlen(name) + 1;
	ut_assertok(ofnode_write_prop(node, prop, name, len, false));
	ut_asserteq_str(name, ofnode_read_string(node, prop));

	/* change the underlying value, this should mess up the live tree */
	strcpy(name, "tony");
	if (of_live_active()) {
		ut_asserteq_str(name, ofnode_read_string(node, prop));
	} else {
		ut_asserteq_str("cecil", ofnode_read_string(node, prop));
	}

	/* try again, this time copying the property */
	strcpy(name, "mary");
	ut_assertok(ofnode_write_prop(node, prop, name, len, true));
	ut_asserteq_str(name, ofnode_read_string(node, prop));
	strcpy(name, "leah");

	/* both flattree and livetree behave the same */
	ut_asserteq_str("mary", ofnode_read_string(node, prop));

	return 0;
}

/* writing the tree with and without copying the property */
static int dm_test_ofnode_write_copy(struct unit_test_state *uts)
{
	ofnode node;

	node = ofnode_path("/a-test");
	ut_assertok(check_write_prop(uts, node));

	return 0;
}
DM_TEST(dm_test_ofnode_write_copy, UT_TESTF_SCAN_FDT);

/* test writing a property to the 'other' tree */
static int dm_test_ofnode_write_copy_ot(struct unit_test_state *uts)
{
	oftree otree = get_other_oftree(uts);
	ofnode node, check_node;

	node = oftree_path(otree, "/node");
	ut_assertok(check_write_prop(uts, node));

	/* make sure the control FDT is not touched */
	check_node = ofnode_path("/node");
	ut_assertnull(ofnode_read_string(check_node, "middle-name"));

	return 0;
}
DM_TEST(dm_test_ofnode_write_copy_ot, UT_TESTF_SCAN_FDT | UT_TESTF_OTHER_FDT);

/* test ofnode_read_u32_index/default() */
static int dm_test_ofnode_u32(struct unit_test_state *uts)
{
	ofnode node;
	u32 val;

	node = ofnode_path("/lcd");
	ut_assert(ofnode_valid(node));
	ut_asserteq(1366, ofnode_read_u32_default(node, "xres", 123));
	ut_assertok(ofnode_write_u32(node, "xres", 1367));
	ut_asserteq(1367, ofnode_read_u32_default(node, "xres", 123));
	ut_assertok(ofnode_write_u32(node, "xres", 1366));

	node = ofnode_path("/backlight");
	ut_assertok(ofnode_read_u32_index(node, "brightness-levels", 0, &val));
	ut_asserteq(0, val);
	ut_assertok(ofnode_read_u32_index(node, "brightness-levels", 1, &val));
	ut_asserteq(16, val);
	ut_assertok(ofnode_read_u32_index(node, "brightness-levels", 8, &val));
	ut_asserteq(255, val);
	ut_asserteq(-EOVERFLOW,
		    ofnode_read_u32_index(node, "brightness-levels", 9, &val));
	ut_asserteq(-EINVAL, ofnode_read_u32_index(node, "missing", 0, &val));

	return 0;
}
DM_TEST(dm_test_ofnode_u32, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* test ofnode_read_u32_array() */
static int dm_test_ofnode_u32_array(struct unit_test_state *uts)
{
	ofnode node;
	u32 val[10];

	node = ofnode_path("/a-test");
	ut_assert(ofnode_valid(node));
	ut_assertok(ofnode_read_u32_array(node, "int-value", val, 1));
	ut_asserteq(-EINVAL, ofnode_read_u32_array(node, "missing", val, 1));
	ut_asserteq(-EOVERFLOW, ofnode_read_u32_array(node, "bool-value", val,
						      1));

	memset(val, '\0', sizeof(val));
	ut_assertok(ofnode_read_u32_array(node, "int-array", val + 1, 3));
	ut_asserteq(0, val[0]);
	ut_asserteq(5678, val[1]);
	ut_asserteq(9123, val[2]);
	ut_asserteq(4567, val[3]);
	ut_asserteq(0, val[4]);
	ut_asserteq(-EOVERFLOW, ofnode_read_u32_array(node, "int-array", val,
						      4));

	return 0;
}
DM_TEST(dm_test_ofnode_u32_array, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* test ofnode_read_u64() and ofnode_write_u64() */
static int dm_test_ofnode_u64(struct unit_test_state *uts)
{
	ofnode node;
	u64 val;

	node = ofnode_path("/a-test");
	ut_assert(ofnode_valid(node));
	ut_assertok(ofnode_read_u64(node, "int64-value", &val));
	ut_asserteq_64(0x1111222233334444, val);
	ut_assertok(ofnode_write_u64(node, "new-int64-value", 0x9876543210));
	ut_assertok(ofnode_read_u64(node, "new-int64-value", &val));
	ut_asserteq_64(0x9876543210, val);

	ut_asserteq(-EINVAL, ofnode_read_u64(node, "missing", &val));

	ut_assertok(ofnode_read_u64_index(node, "int64-array", 0, &val));
	ut_asserteq_64(0x1111222233334444, val);
	ut_assertok(ofnode_read_u64_index(node, "int64-array", 1, &val));
	ut_asserteq_64(0x4444333322221111, val);
	ut_asserteq(-EOVERFLOW,
		    ofnode_read_u64_index(node, "int64-array", 2, &val));
	ut_asserteq(-EINVAL, ofnode_read_u64_index(node, "missing", 0, &val));

	ut_assertok(ofnode_write_u64(node, "int64-array", 0x9876543210));
	ut_assertok(ofnode_read_u64_index(node, "int64-array", 0, &val));
	ut_asserteq_64(0x9876543210, val);
	ut_asserteq(-EOVERFLOW,
		    ofnode_read_u64_index(node, "int64-array", 1, &val));

	return 0;
}
DM_TEST(dm_test_ofnode_u64, UT_TESTF_SCAN_FDT);

static int dm_test_ofnode_add_subnode(struct unit_test_state *uts)
{
	ofnode node, check, subnode;
	char buf[128];

	node = ofnode_path("/lcd");
	ut_assert(ofnode_valid(node));
	ut_assertok(ofnode_add_subnode(node, "edmund", &subnode));
	check = ofnode_path("/lcd/edmund");
	ut_asserteq(subnode.of_offset, check.of_offset);
	ut_assertok(ofnode_get_path(subnode, buf, sizeof(buf)));
	ut_asserteq_str("/lcd/edmund", buf);

	if (of_live_active()) {
		struct device_node *child;

		ut_assertok(of_add_subnode((void *)ofnode_to_np(node), "edmund",
					   2, &child));
		ut_asserteq_str("ed", child->name);
		ut_asserteq_str("/lcd/ed", child->full_name);
		check = ofnode_path("/lcd/ed");
		ut_asserteq_ptr(child, check.np);
		ut_assertok(ofnode_get_path(np_to_ofnode(child), buf,
					    sizeof(buf)));
		ut_asserteq_str("/lcd/ed", buf);
	}

	/* An existing node should be returned with -EEXIST */
	ut_asserteq(-EEXIST, ofnode_add_subnode(node, "edmund", &check));
	ut_asserteq(subnode.of_offset, check.of_offset);

	/* add a root node */
	node = ofnode_path("/");
	ut_assert(ofnode_valid(node));
	ut_assertok(ofnode_add_subnode(node, "lcd2", &subnode));
	check = ofnode_path("/lcd2");
	ut_asserteq(subnode.of_offset, check.of_offset);
	ut_assertok(ofnode_get_path(subnode, buf, sizeof(buf)));
	ut_asserteq_str("/lcd2", buf);

	if (of_live_active()) {
		ulong start;
		int i;

		/*
		 * Make sure each of the three malloc()checks in
		 * of_add_subnode() work
		 */
		for (i = 0; i < 3; i++) {
			malloc_enable_testing(i);
			start = ut_check_free();
			ut_asserteq(-ENOMEM, ofnode_add_subnode(node, "anthony",
								&check));
			ut_assertok(ut_check_delta(start));
		}

		/* This should pass since we allow 3 allocations */
		malloc_enable_testing(3);
		ut_assertok(ofnode_add_subnode(node, "anthony", &check));
		malloc_disable_testing();
	}

	/* write to the empty node */
	ut_assertok(ofnode_write_string(subnode, "example", "text"));

	return 0;
}
DM_TEST(dm_test_ofnode_add_subnode, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_ofnode_for_each_prop(struct unit_test_state *uts)
{
	ofnode node, subnode;
	struct ofprop prop;
	int count;

	node = ofnode_path("/ofnode-foreach");
	count = 0;

	/* we expect "compatible" for each node */
	ofnode_for_each_prop(prop, node)
		count++;
	ut_asserteq(1, count);

	/* there are two nodes, each with 2 properties */
	ofnode_for_each_subnode(subnode, node)
		ofnode_for_each_prop(prop, subnode)
			count++;
	ut_asserteq(5, count);

	return 0;
}
DM_TEST(dm_test_ofnode_for_each_prop, UT_TESTF_SCAN_FDT);

static int dm_test_ofnode_by_compatible(struct unit_test_state *uts)
{
	const char *compat = "denx,u-boot-fdt-test";
	ofnode node;
	int count;

	count = 0;
	for (node = ofnode_null();
	     node = ofnode_by_compatible(node, compat), ofnode_valid(node);)
		count++;
	ut_asserteq(11, count);

	return 0;
}
DM_TEST(dm_test_ofnode_by_compatible, UT_TESTF_SCAN_FDT);

/* check ofnode_by_compatible() on the 'other' tree */
static int dm_test_ofnode_by_compatible_ot(struct unit_test_state *uts)
{
	const char *compat = "sandbox-other2";
	oftree otree = get_other_oftree(uts);
	ofnode node;
	int count;

	count = 0;
	for (node = oftree_root(otree);
	     node = ofnode_by_compatible(node, compat), ofnode_valid(node);)
		count++;
	ut_asserteq(2, count);

	return 0;
}
DM_TEST(dm_test_ofnode_by_compatible_ot, UT_TESTF_SCAN_FDT | UT_TESTF_OTHER_FDT);

static int dm_test_ofnode_find_subnode(struct unit_test_state *uts)
{
	ofnode node, subnode;

	node = ofnode_path("/buttons");

	subnode = ofnode_find_subnode(node, "btn1");
	ut_assert(ofnode_valid(subnode));
	ut_asserteq_str("btn1", ofnode_get_name(subnode));

	subnode = ofnode_find_subnode(node, "btn");
	ut_assert(!ofnode_valid(subnode));

	return 0;
}
DM_TEST(dm_test_ofnode_find_subnode, UT_TESTF_SCAN_FDT);

/* test ofnode_find_subnode() on the 'other' tree */
static int dm_test_ofnode_find_subnode_ot(struct unit_test_state *uts)
{
	oftree otree = get_other_oftree(uts);
	ofnode node, subnode;

	node = oftree_path(otree, "/node");

	subnode = ofnode_find_subnode(node, "subnode");
	ut_assert(ofnode_valid(subnode));
	ut_asserteq_str("subnode", ofnode_get_name(subnode));

	subnode = ofnode_find_subnode(node, "btn");
	ut_assert(!ofnode_valid(subnode));

	return 0;
}
DM_TEST(dm_test_ofnode_find_subnode_ot, UT_TESTF_OTHER_FDT);

static int dm_test_ofnode_get_name(struct unit_test_state *uts)
{
	ofnode node;

	node = ofnode_path("/buttons");
	ut_assert(ofnode_valid(node));
	ut_asserteq_str("buttons", ofnode_get_name(node));
	ut_asserteq_str("", ofnode_get_name(ofnode_root()));

	return 0;
}
DM_TEST(dm_test_ofnode_get_name, UT_TESTF_SCAN_FDT);

/* try to access more FDTs than is supported */
static int dm_test_ofnode_too_many(struct unit_test_state *uts)
{
	const int max_trees = CONFIG_IS_ENABLED(OFNODE_MULTI_TREE,
					(CONFIG_OFNODE_MULTI_TREE_MAX), (1));
	const int fdt_size = 256;
	const int num_trees = max_trees + 1;
	char fdt[num_trees][fdt_size];
	int i;

	for (i = 0; i < num_trees; i++) {
		oftree tree;
		int ret;

		ut_assertok(make_ofnode_fdt(uts, fdt[i], fdt_size, i));
		ret = get_oftree(uts, fdt[i], &tree);

		/*
		 * With flat tree we have the control FDT using one slot. Live
		 * tree has no limit since it uses pointers, not integer tree
		 * IDs
		 */
		if (of_live_active() || i < max_trees - 1) {
			ut_assertok(ret);
		} else {
			/*
			 * tree should be invalid when we try to register too
			 * many trees
			 */
			ut_asserteq(-EOVERFLOW, ret);
		}
	}

	return 0;
}
DM_TEST(dm_test_ofnode_too_many, UT_TESTF_SCAN_FDT);

static int check_copy_props(struct unit_test_state *uts, ofnode dst, ofnode src)
{
	u32 reg[2], val;

	ut_assertok(ofnode_copy_props(dst, src));

	ut_assertok(ofnode_read_u32(dst, "ping-expect", &val));
	ut_asserteq(3, val);

	ut_asserteq_str("denx,u-boot-fdt-test",
			ofnode_read_string(dst, "compatible"));

	/* check that a property with the same name is overwritten */
	ut_assertok(ofnode_read_u32_array(dst, "reg", reg, ARRAY_SIZE(reg)));
	ut_asserteq(3, reg[0]);
	ut_asserteq(1, reg[1]);

	/* reset the compatible so the live tree does not change */
	ut_assertok(ofnode_write_string(dst, "compatible", "nothing"));

	return 0;
}

static int dm_test_ofnode_copy_props(struct unit_test_state *uts)
{
	ofnode src, dst;

	/*
	 * These nodes are chosen so that the src node is before the destination
	 * node in the tree. This doesn't matter with livetree, but with
	 * flattree any attempt to insert a property earlier in the tree will
	 * mess up the offsets after it.
	 */
	src = ofnode_path("/b-test");
	dst = ofnode_path("/some-bus");

	ut_assertok(check_copy_props(uts, dst, src));

	/* check a property that is in the destination already */
	ut_asserteq_str("mux0", ofnode_read_string(dst, "mux-control-names"));

	return 0;
}
DM_TEST(dm_test_ofnode_copy_props, UT_TESTF_SCAN_FDT);

/* test ofnode_copy_props() with the 'other' tree */
static int dm_test_ofnode_copy_props_ot(struct unit_test_state *uts)
{
	ofnode src, dst;
	oftree otree = get_other_oftree(uts);

	src = ofnode_path("/b-test");
	dst = oftree_path(otree, "/node/subnode2");
	ut_assertok(check_copy_props(uts, dst, src));

	return 0;
}
DM_TEST(dm_test_ofnode_copy_props_ot, UT_TESTF_SCAN_FDT | UT_TESTF_OTHER_FDT);

/* check that the livetree is aligned to a structure boundary */
static int dm_test_livetree_align(struct unit_test_state *uts)
{
	const int align = __alignof__(struct unit_test_state);
	struct device_node *node;
	u32 *sentinel;
	ulong start;

	start = (ulong)gd_of_root();
	ut_asserteq(start, ALIGN(start, align));

	node = gd_of_root();
	sentinel = (void *)node - sizeof(u32);

	/*
	 * The sentinel should be overwritten with the root node. If it isn't,
	 * then the root node is not at the very start of the livetree memory
	 * area, and free(root) will fail to free the memory used by the
	 * livetree.
	 */
	ut_assert(*sentinel != BAD_OF_ROOT);

	return 0;
}
DM_TEST(dm_test_livetree_align, UT_TESTF_SCAN_FDT | UT_TESTF_LIVE_TREE);

/* check that it is possible to load an arbitrary livetree */
static int dm_test_livetree_ensure(struct unit_test_state *uts)
{
	oftree tree;
	ofnode node;

	/* read from other.dtb */
	ut_assertok(test_load_other_fdt(uts));
	tree = oftree_from_fdt(uts->other_fdt);
	ut_assert(oftree_valid(tree));
	node = oftree_path(tree, "/node/subnode");
	ut_assert(ofnode_valid(node));
	ut_asserteq_str("sandbox-other2",
			ofnode_read_string(node, "compatible"));

	return 0;
}
DM_TEST(dm_test_livetree_ensure, UT_TESTF_SCAN_FDT);

static int dm_test_oftree_new(struct unit_test_state *uts)
{
	ofnode node, subnode, check;
	oftree tree;

	ut_assertok(oftree_new(&tree));
	node = oftree_root(tree);
	ut_assert(ofnode_valid(node));
	ut_assertok(ofnode_add_subnode(node, "edmund", &subnode));
	check = ofnode_find_subnode(node, "edmund");
	ut_asserteq(check.of_offset, subnode.of_offset);

	return 0;
}
DM_TEST(dm_test_oftree_new, UT_TESTF_SCAN_FDT);

static int check_copy_node(struct unit_test_state *uts, ofnode dst, ofnode src,
			   ofnode *nodep)
{
	u32 reg[2], val;
	ofnode node;

	ut_assertok(ofnode_copy_node(dst, "copy-test", src, &node));

	ut_assertok(ofnode_read_u32(node, "ping-expect", &val));
	ut_asserteq(3, val);

	ut_asserteq_str("denx,u-boot-fdt-test",
			ofnode_read_string(node, "compatible"));

	/* check that a property with the same name is overwritten */
	ut_assertok(ofnode_read_u32_array(node, "reg", reg, ARRAY_SIZE(reg)));
	ut_asserteq(3, reg[0]);
	ut_asserteq(1, reg[1]);

	/* reset the compatible so the live tree does not change */
	ut_assertok(ofnode_write_string(node, "compatible", "nothing"));
	*nodep = node;

	return 0;
}

static int dm_test_ofnode_copy_node(struct unit_test_state *uts)
{
	ofnode src, dst, node, try;

	/*
	 * These nodes are chosen so that the src node is before the destination
	 * node in the tree. This doesn't matter with livetree, but with
	 * flattree any attempt to insert a property earlier in the tree will
	 * mess up the offsets after it.
	 */
	src = ofnode_path("/b-test");
	dst = ofnode_path("/some-bus");

	ut_assertok(check_copy_node(uts, dst, src, &node));

	/* check trying to copy over an existing node */
	ut_asserteq(-EEXIST, ofnode_copy_node(dst, "copy-test", src, &try));
	ut_asserteq(try.of_offset, node.of_offset);

	return 0;
}
DM_TEST(dm_test_ofnode_copy_node, UT_TESTF_SCAN_FDT);

/* test ofnode_copy_node() with the 'other' tree */
static int dm_test_ofnode_copy_node_ot(struct unit_test_state *uts)
{
	oftree otree = get_other_oftree(uts);
	ofnode src, dst, node;

	src = ofnode_path("/b-test");
	dst = oftree_path(otree, "/node/subnode2");
	ut_assertok(check_copy_node(uts, dst, src, &node));

	return 0;
}
DM_TEST(dm_test_ofnode_copy_node_ot, UT_TESTF_SCAN_FDT | UT_TESTF_OTHER_FDT);

static int dm_test_ofnode_delete(struct unit_test_state *uts)
{
	ofnode node;

	/*
	 * At present the livetree is not restored after changes made in tests.
	 * See test_pre_run() for how this is done with the other FDT and
	 * dm_test_pre_run() where it sets up the root-tree pointer. So use
	 * nodes which don't matter to other tests.
	 *
	 * We could fix this by detecting livetree changes and regenerating it
	 * before the next test if needed.
	 */
	node = ofnode_path("/leds/iracibble");
	ut_assert(ofnode_valid(node));
	ut_assertok(ofnode_delete(&node));
	ut_assert(!ofnode_valid(node));
	ut_assert(!ofnode_valid(ofnode_path("/leds/iracibble")));

	node = ofnode_path("/leds/default_on");
	ut_assert(ofnode_valid(node));
	ut_assertok(ofnode_delete(&node));
	ut_assert(!ofnode_valid(node));
	ut_assert(!ofnode_valid(ofnode_path("/leds/default_on")));

	ut_asserteq(2, ofnode_get_child_count(ofnode_path("/leds")));

	return 0;
}
DM_TEST(dm_test_ofnode_delete, UT_TESTF_SCAN_FDT);

static int dm_test_oftree_to_fdt(struct unit_test_state *uts)
{
	oftree tree, check;
	struct abuf buf, buf2;

	tree = oftree_default();
	ut_assertok(oftree_to_fdt(tree, &buf));
	ut_assert(abuf_size(&buf) > SZ_16K);

	/* convert it back to a tree and see if it looks OK */
	check = oftree_from_fdt(abuf_data(&buf));
	ut_assert(oftree_valid(check));

	ut_assertok(oftree_to_fdt(check, &buf2));
	ut_assert(abuf_size(&buf2) > SZ_16K);
	ut_asserteq(abuf_size(&buf), abuf_size(&buf2));
	ut_asserteq_mem(abuf_data(&buf), abuf_data(&buf2), abuf_size(&buf));

	return 0;
}
DM_TEST(dm_test_oftree_to_fdt, UT_TESTF_SCAN_FDT);

/* test ofnode_read_bool() and ofnode_write_bool() */
static int dm_test_bool(struct unit_test_state *uts)
{
	const char *propname = "missing-bool-value";
	ofnode node;

	node = ofnode_path("/a-test");
	ut_assert(ofnode_read_bool(node, "bool-value"));
	ut_assert(!ofnode_read_bool(node, propname));
	ut_assert(!ofnode_has_property(node, propname));

	ut_assertok(ofnode_write_bool(node, propname, true));
	ut_assert(ofnode_read_bool(node, propname));
	ut_assert(ofnode_has_property(node, propname));
	ut_assert(ofnode_read_bool(node, "bool-value"));

	ut_assertok(ofnode_write_bool(node, propname, false));
	ut_assert(!ofnode_read_bool(node, propname));
	ut_assert(!ofnode_has_property(node, propname));
	ut_assert(ofnode_read_bool(node, "bool-value"));

	return 0;
}
DM_TEST(dm_test_bool, UT_TESTF_SCAN_FDT);
