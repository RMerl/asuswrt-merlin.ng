// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * libfdt - Flat Device Tree manipulation
 *	Testcase for fdt_set_name()
 * Copyright (C) 2006 David Gibson, IBM Corporation.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <libfdt.h>

#include "tests.h"
#include "testdata.h"

static void check_set_name(void *fdt, const char *path, const char *newname)
{
	int offset;
	const char *getname, *oldname;
	int len, err;

	oldname = strrchr(path, '/');
	if (!oldname)
		TEST_BUG();
	oldname += 1;

	offset = fdt_path_offset(fdt, path);
	if (offset < 0)
		FAIL("Couldn't find %s", path);

	getname = fdt_get_name(fdt, offset, &len);
	verbose_printf("fdt_get_name(%d) returns \"%s\" (len=%d)\n",
		       offset, getname, len);
	if (!getname)
		FAIL("fdt_get_name(%d): %s", offset, fdt_strerror(len));

	if (strcmp(getname, oldname) != 0)
		FAIL("fdt_get_name(%s) returned \"%s\" instead of \"%s\"",
		     path, getname, oldname);

	if (len < 0)
		FAIL("fdt_get_name(%s) returned negative length: %d",
		     path, len);

	if ((unsigned)len != strlen(getname))
		FAIL("fdt_get_name(%s) returned length %d instead of %zd",
		     path, len, strlen(getname));

	err = fdt_set_name(fdt, offset, newname);
	if (err)
		FAIL("fdt_set_name(%d, \"%s\"): %s", offset, newname,
		     fdt_strerror(err));

	getname = fdt_get_name(fdt, offset, &len);
	if (!getname)
		FAIL("fdt_get_name(%d): %s", offset, fdt_strerror(len));
	if (len < 0)
		FAIL("negative name length (%d) for returned node name\n", len);

	if (strcmp(getname, newname) != 0)
		FAIL("fdt_get_name(%s) returned \"%s\" instead of \"%s\"",
		     path, getname, newname);

	if ((unsigned)len != strlen(getname))
		FAIL("fdt_get_name(%s) returned length %d instead of %zd",
		     path, len, strlen(getname));
}

int main(int argc, char *argv[])
{
	void *fdt;

	test_init(argc, argv);
	fdt = load_blob_arg(argc, argv);
	fdt = open_blob_rw(fdt);

	check_set_name(fdt, "/subnode@1", "subnode@17");
	check_set_name(fdt, "/subnode@2/subsubnode@0", "fred@0");
	check_set_name(fdt, "/subnode@17/subsubnode", "something@0");

	PASS();
}
