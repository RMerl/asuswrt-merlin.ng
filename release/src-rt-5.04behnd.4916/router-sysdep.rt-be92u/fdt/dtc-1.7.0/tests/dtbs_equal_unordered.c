// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * libfdt - Flat Device Tree manipulation
 *	Tests if two given dtbs are structurally equal (including order)
 * Copyright (C) 2007 David Gibson, IBM Corporation.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#include <libfdt.h>

#include "tests.h"
#include "testdata.h"

static int notequal; /* = 0 */
static int ignore_memrsv; /* = 0 */

#define MISMATCH(fmt, ...)			\
	do { \
		if (notequal) \
			PASS(); \
		else \
			FAIL(fmt, ##__VA_ARGS__);	\
	} while (0)

#define MATCH()			\
	do { \
		if (!notequal) \
			PASS(); \
		else \
			FAIL("Trees match which shouldn't");	\
	} while (0)

#define CHECK(code) \
	{ \
		err = (code); \
		if (err) \
			FAIL(#code ": %s", fdt_strerror(err)); \
	}

static int mem_rsv_cmp(const void *p1, const void *p2)
{
	const struct fdt_reserve_entry *re1 = p1;
	const struct fdt_reserve_entry *re2 = p2;

	if (fdt64_to_cpu(re1->address) < fdt64_to_cpu(re2->address))
		return -1;
	else if (fdt64_to_cpu(re1->address) > fdt64_to_cpu(re2->address))
		return 1;

	if (fdt64_to_cpu(re1->size) < fdt64_to_cpu(re2->size))
		return -1;
	else if (fdt64_to_cpu(re1->size) > fdt64_to_cpu(re2->size))
		return 1;

	return 0;
}

static void compare_mem_rsv(void *fdt1, void *fdt2)
{
	int i;
	uint64_t addr1, size1, addr2, size2;
	int err;

	if (fdt_num_mem_rsv(fdt1) != fdt_num_mem_rsv(fdt2))
		MISMATCH("Trees have different number of reserve entries");

	qsort((char *)fdt1 + fdt_off_mem_rsvmap(fdt1), fdt_num_mem_rsv(fdt1),
	      sizeof(struct fdt_reserve_entry), mem_rsv_cmp);
	qsort((char *)fdt2 + fdt_off_mem_rsvmap(fdt2), fdt_num_mem_rsv(fdt2),
	      sizeof(struct fdt_reserve_entry), mem_rsv_cmp);

	for (i = 0; i < fdt_num_mem_rsv(fdt1); i++) {
		CHECK(fdt_get_mem_rsv(fdt1, i, &addr1, &size1));
		CHECK(fdt_get_mem_rsv(fdt2, i, &addr2, &size2));

		if ((addr1 != addr2) || (size1 != size2))
			MISMATCH("Mismatch in reserve entry %d: "
			     "(0x%llx, 0x%llx) != (0x%llx, 0x%llx)", i,
			     (unsigned long long)addr1,
			     (unsigned long long)size1,
			     (unsigned long long)addr2,
			     (unsigned long long)size2);
	}
}

static void compare_properties(const void *fdt1, int offset1,
			       const void *fdt2, int offset2)
{
	int offset = offset1;

	/* Check the properties */
	for (offset = fdt_first_property_offset(fdt1, offset1);
	     offset >= 0;
	     offset = fdt_next_property_offset(fdt1, offset)) {
		const char *name;
		int len1, len2;
		const void *data1, *data2;
		int i;

		data1 = fdt_getprop_by_offset(fdt1, offset, &name, &len1);
		if (!data1)
			FAIL("fdt_getprop_by_offset(): %s\n",
			     fdt_strerror(len1));

		verbose_printf("Property '%s'\n", name);

		data2 = fdt_getprop(fdt2, offset2, name, &len2);
		if (!data2) {
			if (len2 == -FDT_ERR_NOTFOUND)
				MISMATCH("Property '%s' missing\n", name);
			else
				FAIL("fdt_get_property(): %s\n",
				     fdt_strerror(len2));
		}

		verbose_printf("len1=%d data1=", len1);
		for (i = 0; i < len1; i++)
			verbose_printf(" %02x", ((const char *)data1)[i]);
		verbose_printf("\nlen2=%d data2=", len2);
		for (i = 0; i < len1; i++)
			verbose_printf(" %02x", ((const char *)data2)[i]);
		verbose_printf("\n");

		if (len1 != len2)
			MISMATCH("Property '%s' mismatched length %d vs. %d\n",
			     name, len1, len2);
		else if (memcmp(data1, data2, len1) != 0)
			MISMATCH("Property '%s' mismatched value\n", name);
	}
}

static void compare_node(const void *fdt1, int offset1,
			 const void *fdt2, int offset2);

static void compare_subnodes(const void *fdt1, int offset1,
			     const void *fdt2, int offset2,
			     int recurse)
{
	int coffset1, coffset2, depth;

	for (depth = 0, coffset1 = offset1;
	     (coffset1 >= 0) && (depth >= 0);
	      coffset1 = fdt_next_node(fdt1, coffset1, &depth))
		if (depth == 1) {
			const char *name = fdt_get_name(fdt1, coffset1, NULL);

			verbose_printf("Subnode %s\n", name);
			coffset2 = fdt_subnode_offset(fdt2, offset2, name);
			if (coffset2 == -FDT_ERR_NOTFOUND)
				MISMATCH("Subnode %s missing\n", name);
			else if (coffset2 < 0)
				FAIL("fdt_subnode_offset(): %s\n",
				     fdt_strerror(coffset2));

			if (recurse)
				compare_node(fdt1, coffset1, fdt2, coffset2);
		}
}

static void compare_node(const void *fdt1, int offset1,
			 const void *fdt2, int offset2)
{
	int err;
	char path1[PATH_MAX], path2[PATH_MAX];

	CHECK(fdt_get_path(fdt1, offset1, path1, sizeof(path1)));
	CHECK(fdt_get_path(fdt2, offset2, path2, sizeof(path2)));

	if (!streq(path1, path2))
		TEST_BUG("Path mismatch %s vs. %s\n", path1, path2);

	verbose_printf("Checking %s\n", path1);

	compare_properties(fdt1, offset1, fdt2, offset2);
	compare_properties(fdt2, offset2, fdt1, offset1);

	compare_subnodes(fdt1, offset1, fdt2, offset2, 1);
	compare_subnodes(fdt2, offset2, fdt1, offset1, 0);
}

static void badargs(char **argv)
{
	CONFIG("Usage: %s [-n] [-m] <dtb file> <dtb file>", argv[0]);
}

int main(int argc, char *argv[])
{
	void *fdt1, *fdt2;
	uint32_t cpuid1, cpuid2;
	char **args;
	int argsleft;

	test_init(argc, argv);

	args = &argv[1];
	argsleft = argc - 1;

	while (argsleft > 2) {
		if (streq(args[0], "-n"))
			notequal = 1;
		else if (streq(args[0], "-m"))
			ignore_memrsv = 1;
		else
			badargs(argv);
		args++;
		argsleft--;
	}
	if (argsleft != 2)
		badargs(argv);

	fdt1 = load_blob(args[0]);
	fdt2 = load_blob(args[1]);

	if (!ignore_memrsv)
		compare_mem_rsv(fdt1, fdt2);
	compare_node(fdt1, 0, fdt2, 0);

	cpuid1 = fdt_boot_cpuid_phys(fdt1);
	cpuid2 = fdt_boot_cpuid_phys(fdt2);
	if (cpuid1 != cpuid2)
		MISMATCH("boot_cpuid_phys mismatch 0x%x != 0x%x",
		     cpuid1, cpuid2);

	MATCH();
}
