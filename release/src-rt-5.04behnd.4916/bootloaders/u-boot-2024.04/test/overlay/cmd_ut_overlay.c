// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 NextThing Co
 * Copyright (c) 2016 Free Electrons
 */

#include <common.h>
#include <command.h>
#include <errno.h>
#include <fdt_support.h>
#include <image.h>
#include <log.h>
#include <malloc.h>

#include <linux/sizes.h>

#include <test/ut.h>
#include <test/overlay.h>
#include <test/suites.h>

/* 4k ought to be enough for anybody */
#define FDT_COPY_SIZE	(4 * SZ_1K)

extern u32 __dtb_test_fdt_base_begin;
extern u32 __dtb_test_fdt_overlay_begin;
extern u32 __dtb_test_fdt_overlay_stacked_begin;

static void *fdt;

static int ut_fdt_getprop_u32_by_index(void *fdt, const char *path,
				    const char *name, int index,
				    u32 *out)
{
	const fdt32_t *val;
	int node_off;
	int len;

	node_off = fdt_path_offset(fdt, path);
	if (node_off < 0)
		return node_off;

	val = fdt_getprop(fdt, node_off, name, &len);
	if (!val || (len < (sizeof(uint32_t) * (index + 1))))
		return -FDT_ERR_NOTFOUND;

	*out = fdt32_to_cpu(*(val + index));

	return 0;
}

static int ut_fdt_getprop_u32(void *fdt, const char *path, const char *name,
			   u32 *out)
{
	return ut_fdt_getprop_u32_by_index(fdt, path, name, 0, out);
}

static int fdt_getprop_str(void *fdt, const char *path, const char *name,
			   const char **out)
{
	int node_off;
	int len;

	node_off = fdt_path_offset(fdt, path);
	if (node_off < 0)
		return node_off;

	*out = fdt_stringlist_get(fdt, node_off, name, 0, &len);

	return len < 0 ? len : 0;
}

static int fdt_overlay_change_int_property(struct unit_test_state *uts)
{
	u32 val = 0;

	ut_assertok(ut_fdt_getprop_u32(fdt, "/test-node", "test-int-property",
				    &val));
	ut_asserteq(43, val);

	return CMD_RET_SUCCESS;
}
OVERLAY_TEST(fdt_overlay_change_int_property, 0);

static int fdt_overlay_change_str_property(struct unit_test_state *uts)
{
	const char *val = NULL;

	ut_assertok(fdt_getprop_str(fdt, "/test-node", "test-str-property",
				    &val));
	ut_asserteq_str("foobar", val);

	return CMD_RET_SUCCESS;
}
OVERLAY_TEST(fdt_overlay_change_str_property, 0);

static int fdt_overlay_add_str_property(struct unit_test_state *uts)
{
	const char *val = NULL;

	ut_assertok(fdt_getprop_str(fdt, "/test-node", "test-str-property-2",
				    &val));
	ut_asserteq_str("foobar2", val);

	return CMD_RET_SUCCESS;
}
OVERLAY_TEST(fdt_overlay_add_str_property, 0);

static int fdt_overlay_add_node_by_phandle(struct unit_test_state *uts)
{
	int off;

	off = fdt_path_offset(fdt, "/test-node/new-node");
	ut_assert(off >= 0);

	ut_assertnonnull(fdt_getprop(fdt, off, "new-property", NULL));

	return CMD_RET_SUCCESS;
}
OVERLAY_TEST(fdt_overlay_add_node_by_phandle, 0);

static int fdt_overlay_add_node_by_path(struct unit_test_state *uts)
{
	int off;

	off = fdt_path_offset(fdt, "/new-node");
	ut_assert(off >= 0);

	ut_assertnonnull(fdt_getprop(fdt, off, "new-property", NULL));

	return CMD_RET_SUCCESS;
}
OVERLAY_TEST(fdt_overlay_add_node_by_path, 0);

static int fdt_overlay_add_subnode_property(struct unit_test_state *uts)
{
	int off;

	off = fdt_path_offset(fdt, "/test-node/sub-test-node");
	ut_assert(off >= 0);

	ut_assertnonnull(fdt_getprop(fdt, off, "sub-test-property", NULL));
	ut_assertnonnull(fdt_getprop(fdt, off, "new-sub-test-property", NULL));

	return CMD_RET_SUCCESS;
}
OVERLAY_TEST(fdt_overlay_add_subnode_property, 0);

static int fdt_overlay_local_phandle(struct unit_test_state *uts)
{
	uint32_t local_phandle;
	u32 val = 0;
	int off;

	off = fdt_path_offset(fdt, "/new-local-node");
	ut_assert(off >= 0);

	local_phandle = fdt_get_phandle(fdt, off);
	ut_assert(local_phandle);

	ut_assertok(ut_fdt_getprop_u32_by_index(fdt, "/", "test-several-phandle",
					     0, &val));
	ut_asserteq(local_phandle, val);

	ut_assertok(ut_fdt_getprop_u32_by_index(fdt, "/", "test-several-phandle",
					     1, &val));
	ut_asserteq(local_phandle, val);

	return CMD_RET_SUCCESS;
}
OVERLAY_TEST(fdt_overlay_local_phandle, 0);

static int fdt_overlay_local_phandles(struct unit_test_state *uts)
{
	uint32_t local_phandle, test_phandle;
	u32 val = 0;
	int off;

	off = fdt_path_offset(fdt, "/new-local-node");
	ut_assert(off >= 0);

	local_phandle = fdt_get_phandle(fdt, off);
	ut_assert(local_phandle);

	off = fdt_path_offset(fdt, "/test-node");
	ut_assert(off >= 0);

	test_phandle = fdt_get_phandle(fdt, off);
	ut_assert(test_phandle);

	ut_assertok(ut_fdt_getprop_u32_by_index(fdt, "/", "test-phandle", 0,
					     &val));
	ut_asserteq(test_phandle, val);

	ut_assertok(ut_fdt_getprop_u32_by_index(fdt, "/", "test-phandle", 1,
					     &val));
	ut_asserteq(local_phandle, val);

	return CMD_RET_SUCCESS;
}
OVERLAY_TEST(fdt_overlay_local_phandles, 0);

static int fdt_overlay_stacked(struct unit_test_state *uts)
{
	u32 val = 0;

	ut_assertok(ut_fdt_getprop_u32(fdt, "/new-local-node",
				       "stacked-test-int-property", &val));
	ut_asserteq(43, val);

	return CMD_RET_SUCCESS;
}
OVERLAY_TEST(fdt_overlay_stacked, 0);

int do_ut_overlay(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(overlay_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(overlay_test);
	struct unit_test_state *uts;
	void *fdt_base = &__dtb_test_fdt_base_begin;
	void *fdt_overlay = &__dtb_test_fdt_overlay_begin;
	void *fdt_overlay_stacked = &__dtb_test_fdt_overlay_stacked_begin;
	void *fdt_overlay_copy, *fdt_overlay_stacked_copy;
	int ret = -ENOMEM;

	uts = calloc(1, sizeof(*uts));
	if (!uts)
		return -ENOMEM;

	ut_assertok(fdt_check_header(fdt_base));
	ut_assertok(fdt_check_header(fdt_overlay));

	fdt = malloc(FDT_COPY_SIZE);
	if (!fdt)
		goto err1;

	fdt_overlay_copy = malloc(FDT_COPY_SIZE);
	if (!fdt_overlay_copy)
		goto err2;

	fdt_overlay_stacked_copy = malloc(FDT_COPY_SIZE);
	if (!fdt_overlay_stacked_copy)
		goto err3;

	/*
	 * Resize the FDT to 4k so that we have room to operate on
	 *
	 * (and relocate it since the memory might be mapped
	 * read-only)
	 */
	ut_assertok(fdt_open_into(fdt_base, fdt, FDT_COPY_SIZE));

	/*
	 * Resize the overlay to 4k so that we have room to operate on
	 *
	 * (and relocate it since the memory might be mapped
	 * read-only)
	 */
	ut_assertok(fdt_open_into(fdt_overlay, fdt_overlay_copy,
				  FDT_COPY_SIZE));

	/*
	 * Resize the stacked overlay to 4k so that we have room to operate on
	 *
	 * (and relocate it since the memory might be mapped
	 * read-only)
	 */
	ut_assertok(fdt_open_into(fdt_overlay_stacked, fdt_overlay_stacked_copy,
				  FDT_COPY_SIZE));

	/* Apply the overlay */
	ut_assertok(fdt_overlay_apply(fdt, fdt_overlay_copy));

	/* Apply the stacked overlay */
	ut_assertok(fdt_overlay_apply(fdt, fdt_overlay_stacked_copy));

	ret = cmd_ut_category("overlay", "", tests, n_ents, argc, argv);

	free(fdt_overlay_stacked_copy);
err3:
	free(fdt_overlay_copy);
err2:
	free(fdt);
err1:
	return ret;
}
