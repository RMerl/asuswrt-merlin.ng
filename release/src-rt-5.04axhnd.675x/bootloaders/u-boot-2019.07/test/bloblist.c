// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2018, Google Inc. All rights reserved.
 */

#include <common.h>
#include <bloblist.h>
#include <log.h>
#include <mapmem.h>
#include <test/suites.h>
#include <test/test.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

/* Declare a new compression test */
#define BLOBLIST_TEST(_name, _flags) \
		UNIT_TEST(_name, _flags, bloblist_test)

enum {
	TEST_TAG		= 1,
	TEST_TAG2		= 2,
	TEST_TAG_MISSING	= 3,

	TEST_SIZE		= 10,
	TEST_SIZE2		= 20,

	TEST_ADDR		= CONFIG_BLOBLIST_ADDR,
	TEST_BLOBLIST_SIZE	= 0x100,
};

static struct bloblist_hdr *clear_bloblist(void)
{
	struct bloblist_hdr *hdr;

	/* Clear out any existing bloblist so we have a clean slate */
	hdr = map_sysmem(CONFIG_BLOBLIST_ADDR, TEST_BLOBLIST_SIZE);
	memset(hdr, '\0', TEST_BLOBLIST_SIZE);

	return hdr;
}

static int bloblist_test_init(struct unit_test_state *uts)
{
	struct bloblist_hdr *hdr;

	hdr = clear_bloblist();
	ut_asserteq(-ENOENT, bloblist_check(TEST_ADDR, TEST_BLOBLIST_SIZE));
	ut_assertok(bloblist_new(TEST_ADDR, TEST_BLOBLIST_SIZE, 0));
	hdr->version++;
	ut_asserteq(-EPROTONOSUPPORT, bloblist_check(TEST_ADDR,
						     TEST_BLOBLIST_SIZE));

	ut_asserteq(-ENOSPC, bloblist_new(TEST_ADDR, 0x10, 0));
	ut_asserteq(-EFAULT, bloblist_new(1, TEST_BLOBLIST_SIZE, 0));
	ut_assertok(bloblist_new(TEST_ADDR, TEST_BLOBLIST_SIZE, 0));

	ut_asserteq(-EIO, bloblist_check(TEST_ADDR, TEST_BLOBLIST_SIZE));
	ut_assertok(bloblist_finish());
	ut_assertok(bloblist_check(TEST_ADDR, TEST_BLOBLIST_SIZE));
	hdr->flags++;
	ut_asserteq(-EIO, bloblist_check(TEST_ADDR, TEST_BLOBLIST_SIZE));

	return 1;
}
BLOBLIST_TEST(bloblist_test_init, 0);

static int bloblist_test_blob(struct unit_test_state *uts)
{
	struct bloblist_hdr *hdr;
	struct bloblist_rec *rec, *rec2;
	char *data;

	/* At the start there should be no records */
	hdr = clear_bloblist();
	ut_assertnull(bloblist_find(TEST_TAG, TEST_BLOBLIST_SIZE));
	ut_assertok(bloblist_new(TEST_ADDR, TEST_BLOBLIST_SIZE, 0));

	/* Add a record and check that we can find it */
	data = bloblist_add(TEST_TAG, TEST_SIZE);
	rec = (void *)(hdr + 1);
	ut_asserteq_ptr(rec + 1, data);
	data = bloblist_find(TEST_TAG, TEST_SIZE);
	ut_asserteq_ptr(rec + 1, data);

	/* Check the 'ensure' method */
	ut_asserteq_ptr(data, bloblist_ensure(TEST_TAG, TEST_SIZE));
	ut_assertnull(bloblist_ensure(TEST_TAG, TEST_SIZE2));
	rec2 = (struct bloblist_rec *)(data + ALIGN(TEST_SIZE, BLOBLIST_ALIGN));

	/* Check for a non-existent record */
	ut_asserteq_ptr(data, bloblist_ensure(TEST_TAG, TEST_SIZE));
	ut_asserteq_ptr(rec2 + 1, bloblist_ensure(TEST_TAG2, TEST_SIZE2));
	ut_assertnull(bloblist_find(TEST_TAG_MISSING, 0));

	return 0;
}
BLOBLIST_TEST(bloblist_test_blob, 0);

static int bloblist_test_bad_blob(struct unit_test_state *uts)
{
	struct bloblist_hdr *hdr;
	void *data;

	hdr = clear_bloblist();
	ut_assertok(bloblist_new(TEST_ADDR, TEST_BLOBLIST_SIZE, 0));
	data = hdr + 1;
	data += sizeof(struct bloblist_rec);
	ut_asserteq_ptr(data, bloblist_ensure(TEST_TAG, TEST_SIZE));
	ut_asserteq_ptr(data, bloblist_ensure(TEST_TAG, TEST_SIZE));

	return 0;
}
BLOBLIST_TEST(bloblist_test_bad_blob, 0);

static int bloblist_test_checksum(struct unit_test_state *uts)
{
	struct bloblist_hdr *hdr;
	char *data, *data2;

	hdr = clear_bloblist();
	ut_assertok(bloblist_new(TEST_ADDR, TEST_BLOBLIST_SIZE, 0));
	ut_assertok(bloblist_finish());
	ut_assertok(bloblist_check(TEST_ADDR, TEST_BLOBLIST_SIZE));

	/*
	 * Now change things amd make sure that the checksum notices. We cannot
	 * change the size or alloced fields, since that will crash the code.
	 * It has to rely on these being correct.
	 */
	hdr->flags--;
	ut_asserteq(-EIO, bloblist_check(TEST_ADDR, TEST_BLOBLIST_SIZE));
	hdr->flags++;

	hdr->size--;
	ut_asserteq(-EFBIG, bloblist_check(TEST_ADDR, TEST_BLOBLIST_SIZE));
	hdr->size++;

	hdr->spare++;
	ut_asserteq(-EIO, bloblist_check(TEST_ADDR, TEST_BLOBLIST_SIZE));
	hdr->spare--;

	hdr->chksum++;
	ut_asserteq(-EIO, bloblist_check(TEST_ADDR, TEST_BLOBLIST_SIZE));
	hdr->chksum--;

	/* Make sure the checksum changes when we add blobs */
	data = bloblist_add(TEST_TAG, TEST_SIZE);
	ut_asserteq(-EIO, bloblist_check(TEST_ADDR, TEST_BLOBLIST_SIZE));

	data2 = bloblist_add(TEST_TAG2, TEST_SIZE2);
	ut_asserteq(-EIO, bloblist_check(TEST_ADDR, TEST_BLOBLIST_SIZE));
	ut_assertok(bloblist_finish());

	/* It should also change if we change the data */
	ut_assertok(bloblist_check(TEST_ADDR, TEST_BLOBLIST_SIZE));
	*data += 1;
	ut_asserteq(-EIO, bloblist_check(TEST_ADDR, TEST_BLOBLIST_SIZE));
	*data -= 1;

	ut_assertok(bloblist_check(TEST_ADDR, TEST_BLOBLIST_SIZE));
	*data2 += 1;
	ut_asserteq(-EIO, bloblist_check(TEST_ADDR, TEST_BLOBLIST_SIZE));
	*data2 -= 1;

	/*
	 * Changing data outside the range of valid data should not affect
	 * the checksum.
	 */
	ut_assertok(bloblist_check(TEST_ADDR, TEST_BLOBLIST_SIZE));
	data[TEST_SIZE]++;
	data2[TEST_SIZE2]++;
	ut_assertok(bloblist_check(TEST_ADDR, TEST_BLOBLIST_SIZE));

	return 0;
}

BLOBLIST_TEST(bloblist_test_checksum, 0);

int do_ut_bloblist(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = ll_entry_start(struct unit_test,
						 bloblist_test);
	const int n_ents = ll_entry_count(struct unit_test, bloblist_test);

	return cmd_ut_category("bloblist", tests, n_ents, argc, argv);
}
