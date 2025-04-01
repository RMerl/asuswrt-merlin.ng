// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * libfdt - Flat Device Tree manipulation
 *	Testcase for fdt_delprop()
 * Copyright (C) 2006 David Gibson, IBM Corporation.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include <libfdt.h>

#include "tests.h"
#include "testdata.h"

int main(int argc, char *argv[])
{
	void *fdt;
	const uint32_t *intp;
	const char *strp;
	int err, lenerr;
	int oldsize, delsize, newsize;

	test_init(argc, argv);
	fdt = load_blob_arg(argc, argv);

	fdt = open_blob_rw(fdt);

	oldsize = fdt_totalsize(fdt);

	intp = check_getprop_cell(fdt, 0, "prop-int", TEST_VALUE_1);
	verbose_printf("int value was 0x%08x\n", *intp);

	err = fdt_delprop(fdt, 0, "prop-int");
	if (err)
		FAIL("Failed to delete \"prop-int\": %s", fdt_strerror(err));

	intp = fdt_getprop(fdt, 0, "prop-int", &lenerr);
	if (intp)
		FAIL("prop-int still present after deletion");
	if (lenerr != -FDT_ERR_NOTFOUND)
		FAIL("Unexpected error on second getprop: %s",
		     fdt_strerror(lenerr));

	strp = check_getprop(fdt, 0, "prop-str", strlen(TEST_STRING_1)+1,
			     TEST_STRING_1);
	verbose_printf("string value was \"%s\"\n", strp);
	err = fdt_delprop(fdt, 0, "prop-str");
	if (err)
		FAIL("Failed to delete \"prop-str\": %s", fdt_strerror(err));

	strp = fdt_getprop(fdt, 0, "prop-str", &lenerr);
	if (strp)
		FAIL("prop-str still present after deletion");
	if (lenerr != -FDT_ERR_NOTFOUND)
		FAIL("Unexpected error on second getprop: %s",
		     fdt_strerror(lenerr));

	delsize = fdt_totalsize(fdt);

	err = fdt_pack(fdt);
	if (err)
		FAIL("fdt_pack(): %s\n", fdt_strerror(err));

	newsize = fdt_totalsize(fdt);

	verbose_printf("oldsize = %d, delsize = %d, newsize = %d\n",
		       oldsize, delsize, newsize);

	if (newsize >= oldsize)
		FAIL("Tree failed to shrink after deletions");

	PASS();
}
