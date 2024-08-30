// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * libfdt - Flat Device Tree manipulation
 *	Testcase for fdt_path_offset()
 * Copyright (C) 2006 David Gibson, IBM Corporation.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#include <libfdt.h>

#include "tests.h"
#include "testdata.h"

static void vcheck_search(void *fdt, const char *propname, const void *propval,
		  int proplen, va_list ap)
{
	int offset = -1, target;

	do {
		target = va_arg(ap, int);
		verbose_printf("Searching (target = %d): %d ->",
			       target, offset);
		offset = fdt_node_offset_by_prop_value(fdt, offset, propname,
						       propval, proplen);
		verbose_printf("%d\n", offset);

		if (offset != target)
			FAIL("fdt_node_offset_by_prop_value() returns %d "
			     "instead of %d", offset, target);
	} while (target >= 0);
}

static void check_search(void *fdt, const char *propname, const void *propval,
		  int proplen, ...)
{
	va_list ap;

	va_start(ap, proplen);
	vcheck_search(fdt, propname, propval, proplen, ap);
	va_end(ap);
}

static void check_search_str(void *fdt, const char *propname,
			     const char *propval, ...)
{
	va_list ap;

	va_start(ap, propval);
	vcheck_search(fdt, propname, propval, strlen(propval)+1, ap);
	va_end(ap);
}

#define check_search_cell(fdt, propname, propval, ...) \
	{ \
		fdt32_t val = cpu_to_fdt32(propval); \
		check_search((fdt), (propname), &val, sizeof(val), \
			     ##__VA_ARGS__); \
	}

int main(int argc, char *argv[])
{
	void *fdt;
	int subnode1_offset, subnode2_offset;
	int subsubnode1_offset, subsubnode2_offset;

	test_init(argc, argv);
	fdt = load_blob_arg(argc, argv);

	subnode1_offset = fdt_path_offset(fdt, "/subnode@1");
	subnode2_offset = fdt_path_offset(fdt, "/subnode@2");
	subsubnode1_offset = fdt_path_offset(fdt, "/subnode@1/subsubnode");
	subsubnode2_offset = fdt_path_offset(fdt, "/subnode@2/subsubnode@0");

	if ((subnode1_offset < 0) || (subnode2_offset < 0)
	    || (subsubnode1_offset < 0) || (subsubnode2_offset < 0))
		FAIL("Can't find required nodes");

	check_search_cell(fdt, "prop-int", TEST_VALUE_1, 0, subnode1_offset,
			  subsubnode1_offset, -FDT_ERR_NOTFOUND);

	check_search_cell(fdt, "prop-int", TEST_VALUE_2, subnode2_offset,
			  subsubnode2_offset, -FDT_ERR_NOTFOUND);

	check_search_str(fdt, "prop-str", TEST_STRING_1, 0, -FDT_ERR_NOTFOUND);

	check_search_str(fdt, "prop-str", "no such string", -FDT_ERR_NOTFOUND);

	check_search_cell(fdt, "prop-int", TEST_VALUE_1+1, -FDT_ERR_NOTFOUND);

	check_search(fdt, "no-such-prop", NULL, 0, -FDT_ERR_NOTFOUND);

	PASS();
}
