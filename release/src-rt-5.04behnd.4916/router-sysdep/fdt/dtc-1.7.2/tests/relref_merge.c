// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * libfdt - Flat Device Tree manipulation
 *	Testcase for label relative child references in dtc
 * Copyright (C) 2006 David Gibson, IBM Corporation.
 * Copyright (C) 2020 Ahmad Fatoum, Pengutronix.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <libfdt.h>

#include "tests.h"
#include "testdata.h"

static void check_exist(void *fdt, const char *path)
{
	int sn = fdt_path_offset(fdt, path);
	if (sn < 0)
		FAIL("%s expected but not found: %s", path, fdt_strerror(sn));
}

static void check_doesnt_exist(void *fdt, const char *path)
{
	int sn = fdt_path_offset(fdt, path);
	if (sn >= 0)
		FAIL("%s found but not expected %d", path, sn);
}

int main(int argc, char *argv[])
{
	void *fdt;

	test_init(argc, argv);
	fdt = load_blob_arg(argc, argv);

	check_exist(fdt, "/node/subnode1");
	check_exist(fdt, "/node/keep-me");
	check_doesnt_exist(fdt, "/node/remove-me");

	check_doesnt_exist(fdt, "/node2");
	check_doesnt_exist(fdt, "/node/subnode3");

	check_exist(fdt, "/node/subnode4");

	check_exist(fdt, "/node/subnode1/add-me");

	PASS();
}
