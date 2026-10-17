// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 NXP
 */

#include <common.h>
#include <dm.h>
#include <asm/global_data.h>
#include <dm/of_extra.h>
#include <dm/test.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

static int dm_test_fdtdec_set_carveout(struct unit_test_state *uts)
{
	struct fdt_memory resv;
	void *blob;
	const fdt32_t *prop;
	int blob_sz, len, offset;

	blob_sz = fdt_totalsize(gd->fdt_blob) + 4096;
	blob = malloc(blob_sz);
	ut_assertnonnull(blob);

	/* Make a writable copy of the fdt blob */
	ut_assertok(fdt_open_into(gd->fdt_blob, blob, blob_sz));

	resv.start = 0x1000;
	resv.end = 0x2000;
	ut_assertok(fdtdec_set_carveout(blob, "/a-test", "memory-region", 2,
					&resv, "test_resv1", NULL, 0, 0));

	resv.start = 0x10000;
	resv.end = 0x20000;
	ut_assertok(fdtdec_set_carveout(blob, "/a-test", "memory-region", 1,
					&resv, "test_resv2", NULL, 0, 0));

	resv.start = 0x100000;
	resv.end = 0x200000;
	ut_assertok(fdtdec_set_carveout(blob, "/a-test", "memory-region", 0,
					&resv, "test_resv3", NULL, 0, 0));

	offset = fdt_path_offset(blob, "/a-test");
	ut_assert(offset > 0);
	prop = fdt_getprop(blob, offset, "memory-region", &len);
	ut_assertnonnull(prop);

	ut_asserteq(len, 12);
	ut_assert(fdt_node_offset_by_phandle(blob, fdt32_to_cpu(prop[0])) > 0);
	ut_assert(fdt_node_offset_by_phandle(blob, fdt32_to_cpu(prop[1])) > 0);
	ut_assert(fdt_node_offset_by_phandle(blob, fdt32_to_cpu(prop[2])) > 0);

	free(blob);

	return 0;
}
DM_TEST(dm_test_fdtdec_set_carveout,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT | UT_TESTF_FLAT_TREE);

static int dm_test_fdtdec_add_reserved_memory(struct unit_test_state *uts)
{
	struct fdt_memory resv;
	fdt_addr_t addr;
	fdt_size_t size;
	void *blob;
	unsigned long flags = FDTDEC_RESERVED_MEMORY_NO_MAP;
	int blob_sz, parent, subnode;
	uint32_t phandle, phandle1;

	blob_sz = fdt_totalsize(gd->fdt_blob) + 128;
	blob = malloc(blob_sz);
	ut_assertnonnull(blob);

	/* Make a writable copy of the fdt blob */
	ut_assertok(fdt_open_into(gd->fdt_blob, blob, blob_sz));

	/* Insert a memory region in /reserved-memory node */
	resv.start = 0x1000;
	resv.end = 0x1fff;
	ut_assertok(fdtdec_add_reserved_memory(blob, "rsvd_region", &resv,
					       NULL, 0, &phandle, 0));

	/* Test /reserve-memory and its subnode should exist */
	parent = fdt_path_offset(blob, "/reserved-memory");
	ut_assert(parent > 0);
	subnode = fdt_path_offset(blob, "/reserved-memory/rsvd_region");
	ut_assert(subnode > 0);

	/* Test reg property of /reserved-memory/rsvd_region node */
	addr = fdtdec_get_addr_size_auto_parent(blob, parent, subnode,
						"reg", 0, &size, false);
	ut_assert(addr == resv.start);
	ut_assert(size == resv.end -  resv.start + 1);

	/* Insert another memory region in /reserved-memory node */
	subnode = fdt_path_offset(blob, "/reserved-memory/rsvd_region1");
	ut_assert(subnode < 0);

	resv.start = 0x2000;
	resv.end = 0x2fff;
	ut_assertok(fdtdec_add_reserved_memory(blob, "rsvd_region1", &resv,
					       NULL, 0, &phandle1, flags));
	subnode = fdt_path_offset(blob, "/reserved-memory/rsvd_region1");
	ut_assert(subnode > 0);

	/* check that no-map property is present */
	ut_assert(fdt_getprop(blob, subnode, "no-map", NULL) > 0);

	/* phandles must be different */
	ut_assert(phandle != phandle1);

	/*
	 * Insert a 3rd memory region with the same addr/size as the 1st one,
	 * but a new node should not be inserted due to the same addr/size.
	 */
	resv.start = 0x1000;
	resv.end = 0x1fff;
	ut_assertok(fdtdec_add_reserved_memory(blob, "rsvd_region2", &resv,
					       NULL, 0, &phandle1, 0));
	subnode = fdt_path_offset(blob, "/reserved-memory/rsvd_region2");
	ut_assert(subnode < 0);

	/* phandle must be same as the 1st one */
	ut_assert(phandle == phandle1);

	free(blob);

	return 0;
}
DM_TEST(dm_test_fdtdec_add_reserved_memory,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT | UT_TESTF_FLAT_TREE);
