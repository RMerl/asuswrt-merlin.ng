// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * libfdt - Flat Device Tree manipulation
 *	Testcase for fdt_next_tag()
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <libfdt.h>
#include "tests.h"
#include "testdata.h"

#define FDT_SIZE 65536
#define CHECK_ERR(err) \
({ if (err) \
	FAIL("%s: %d: %s", __FILE__, __LINE__, fdt_strerror(err)); \
})

static void fdt_next_tag_test(bool fdt_finished)
{
	struct fdt_property *prp;
	void *fdt;
	int nextoff = 0, offset, err;
	uint32_t tag;

	fdt = malloc(FDT_SIZE);
	if (!fdt)
		FAIL("Can't allocate memory");
	err = fdt_create(fdt, FDT_SIZE);
	CHECK_ERR(err);
	err = fdt_finish_reservemap(fdt);
	CHECK_ERR(err);
	/* Create a root node and add two properties */
	err = fdt_begin_node(fdt, "");
	CHECK_ERR(err);
	err = fdt_property_u32(fdt, "prop-int-32", 0x1234);
	CHECK_ERR(err);
	err = fdt_property_u32(fdt, "prop2-int-32", 0x4321);
	CHECK_ERR(err);
	err = fdt_end_node(fdt);
	CHECK_ERR(err);
	if (fdt_finished) {
		/* Call fdt_finish to set the correct fdt state. */
		err = fdt_finish(fdt);
		CHECK_ERR(err);
	}

	offset = fdt_first_property_offset(fdt, 0);
	if (offset <= 0)
		FAIL("Invalid offset %x, expected value greater than 0, finished=%d\n",
		     offset, fdt_finished);

	/* Normal case */
	tag = fdt_next_tag(fdt, offset, &nextoff);
	if (tag != FDT_PROP)
		FAIL("Invalid tag %x, expected FDT_PROP, finished=%d\n",
		     tag, fdt_finished);
	if (nextoff <= 0)
		FAIL("Invalid nextoff %d, expected value greater than 0, finished=%d",
		     nextoff, fdt_finished);

	/* Get a writable ptr to the first property and corrupt the length */
	prp = fdt_get_property_by_offset_w(fdt, offset, NULL);
	if (!prp)
		FAIL("Bad property pointer, finished=%d", fdt_finished);

	/* int overflow case */
	prp->len = cpu_to_fdt32(0xFFFFFFFA);
	tag = fdt_next_tag(fdt, offset, &nextoff);
	if (tag != FDT_END)
		FAIL("Invalid tag %x, expected premature FDT_END, finished=%d",
		     tag, fdt_finished);
	if (nextoff != -FDT_ERR_BADSTRUCTURE)
		FAIL("Invalid nextoff, expected error -FDT_ERR_BADSTRUCTURE, finished=%d",
		     fdt_finished);

	/* negative offset case */
	prp->len = cpu_to_fdt32(0x7FFFFFFA);
	tag = fdt_next_tag(fdt, offset, &nextoff);
	if (tag != FDT_END)
		FAIL("Invalid tag %x, expected premature FDT_END, finished=%d",
		     tag, fdt_finished);
	if (nextoff != -FDT_ERR_BADSTRUCTURE)
		FAIL("Invalid nextoff, expected -FDT_ERR_BADSTRUCTURE, finished=%d",
		     fdt_finished);

	free(fdt);
}

int main(int argc, char *argv[])
{
	test_init(argc, argv);

	fdt_next_tag_test(false);
	fdt_next_tag_test(true);

	PASS();
}
