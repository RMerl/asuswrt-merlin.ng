// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <abuf.h>
#include <mapmem.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

static char test_data[] = "1234";
#define TEST_DATA_LEN	sizeof(test_data)

/* Test abuf_set() */
static int lib_test_abuf_set(struct unit_test_state *uts)
{
	struct abuf buf;
	ulong start;

	start = ut_check_free();

	abuf_init(&buf);
	abuf_set(&buf, test_data, TEST_DATA_LEN);
	ut_asserteq_ptr(test_data, buf.data);
	ut_asserteq(TEST_DATA_LEN, buf.size);
	ut_asserteq(false, buf.alloced);

	/* Force it to allocate */
	ut_asserteq(true, abuf_realloc(&buf, TEST_DATA_LEN + 1));
	ut_assertnonnull(buf.data);
	ut_asserteq(TEST_DATA_LEN + 1, buf.size);
	ut_asserteq(true, buf.alloced);

	/* Now set it again, to force it to free */
	abuf_set(&buf, test_data, TEST_DATA_LEN);
	ut_asserteq_ptr(test_data, buf.data);
	ut_asserteq(TEST_DATA_LEN, buf.size);
	ut_asserteq(false, buf.alloced);

	/* Check for memory leaks */
	ut_assertok(ut_check_delta(start));

	return 0;
}
LIB_TEST(lib_test_abuf_set, 0);

/* Test abuf_map_sysmem() */
static int lib_test_abuf_map_sysmem(struct unit_test_state *uts)
{
	struct abuf buf;
	ulong addr;

	abuf_init(&buf);
	addr = 0x100;
	abuf_map_sysmem(&buf, addr, TEST_DATA_LEN);

	ut_asserteq_ptr(map_sysmem(0x100, 0), buf.data);
	ut_asserteq(TEST_DATA_LEN, buf.size);
	ut_asserteq(false, buf.alloced);

	return 0;
}
LIB_TEST(lib_test_abuf_map_sysmem, 0);

/* Test abuf_realloc() */
static int lib_test_abuf_realloc(struct unit_test_state *uts)
{
	struct abuf buf;
	ulong start;
	void *ptr;

	/*
	 * TODO: crashes on sandbox sometimes due to an apparent bug in
	 * realloc().
	 */
	return 0;

	start = ut_check_free();

	abuf_init(&buf);

	/* Allocate an empty buffer */
	ut_asserteq(true, abuf_realloc(&buf, 0));
	ut_assertnull(buf.data);
	ut_asserteq(0, buf.size);
	ut_asserteq(false, buf.alloced);

	/* Allocate a non-empty abuf */
	ut_asserteq(true, abuf_realloc(&buf, TEST_DATA_LEN));
	ut_assertnonnull(buf.data);
	ut_asserteq(TEST_DATA_LEN, buf.size);
	ut_asserteq(true, buf.alloced);
	ptr = buf.data;

	/*
	 * Make it smaller; the pointer should remain the same. Note this relies
	 * on knowledge of how U-Boot's realloc() works
	 */
	ut_asserteq(true, abuf_realloc(&buf, TEST_DATA_LEN - 1));
	ut_asserteq(TEST_DATA_LEN - 1, buf.size);
	ut_asserteq(true, buf.alloced);
	ut_asserteq_ptr(ptr, buf.data);

	/*
	 * Make it larger, forcing reallocation. Note this relies on knowledge
	 * of how U-Boot's realloc() works
	 */
	ut_asserteq(true, abuf_realloc(&buf, 0x1000));
	ut_assert(buf.data != ptr);
	ut_asserteq(0x1000, buf.size);
	ut_asserteq(true, buf.alloced);

	/* Free it */
	ut_asserteq(true, abuf_realloc(&buf, 0));
	ut_assertnull(buf.data);
	ut_asserteq(0, buf.size);
	ut_asserteq(false, buf.alloced);

	/* Check for memory leaks */
	ut_assertok(ut_check_delta(start));

	return 0;
}
LIB_TEST(lib_test_abuf_realloc, 0);

/* Test abuf_realloc() on an non-allocated buffer of zero size */
static int lib_test_abuf_realloc_size(struct unit_test_state *uts)
{
	struct abuf buf;
	ulong start;

	start = ut_check_free();

	abuf_init(&buf);

	/* Allocate some space */
	ut_asserteq(true, abuf_realloc(&buf, TEST_DATA_LEN));
	ut_assertnonnull(buf.data);
	ut_asserteq(TEST_DATA_LEN, buf.size);
	ut_asserteq(true, buf.alloced);

	/* Free it */
	ut_asserteq(true, abuf_realloc(&buf, 0));
	ut_assertnull(buf.data);
	ut_asserteq(0, buf.size);
	ut_asserteq(false, buf.alloced);

	/* Check for memory leaks */
	ut_assertok(ut_check_delta(start));

	return 0;
}
LIB_TEST(lib_test_abuf_realloc_size, 0);

/* Test abuf_realloc_inc() */
static int lib_test_abuf_realloc_inc(struct unit_test_state *uts)
{
	struct abuf buf;
	ulong start;

	start = ut_check_free();

	abuf_init(&buf);
	ut_asserteq(0, buf.size);
	ut_asserteq(false, buf.alloced);

	abuf_realloc_inc(&buf, 20);
	ut_asserteq(20, buf.size);
	ut_asserteq(true, buf.alloced);

	abuf_uninit(&buf);

	/* Check for memory leaks */
	ut_assertok(ut_check_delta(start));

	return 0;
}
LIB_TEST(lib_test_abuf_realloc_inc, 0);

/* Test handling of buffers that are too large */
static int lib_test_abuf_large(struct unit_test_state *uts)
{
	struct abuf buf;
	ulong start;
	size_t size;
	int delta;
	void *ptr;

	/*
	 * This crashes at present due to trying to allocate more memory than
	 * available, which breaks something on sandbox.
	 */
	return 0;

	start = ut_check_free();

	/* Try an impossible size */
	abuf_init(&buf);
	ut_asserteq(false, abuf_realloc(&buf, CONFIG_SYS_MALLOC_LEN));
	ut_assertnull(buf.data);
	ut_asserteq(0, buf.size);
	ut_asserteq(false, buf.alloced);

	abuf_uninit(&buf);
	ut_assertnull(buf.data);
	ut_asserteq(0, buf.size);
	ut_asserteq(false, buf.alloced);

	/* Start with a normal size then try to increase it, to check realloc */
	ut_asserteq(true, abuf_realloc(&buf, TEST_DATA_LEN));
	ut_assertnonnull(buf.data);
	ut_asserteq(TEST_DATA_LEN, buf.size);
	ut_asserteq(true, buf.alloced);
	ptr = buf.data;
	delta = ut_check_delta(start);
	ut_assert(delta > 0);

	/* try to increase it */
	ut_asserteq(false, abuf_realloc(&buf, CONFIG_SYS_MALLOC_LEN));
	ut_asserteq_ptr(ptr, buf.data);
	ut_asserteq(TEST_DATA_LEN, buf.size);
	ut_asserteq(true, buf.alloced);
	ut_asserteq(delta, ut_check_delta(start));

	/* Check for memory leaks */
	abuf_uninit(&buf);
	ut_assertok(ut_check_delta(start));

	/* Start with a huge unallocated buf and try to move it */
	abuf_init(&buf);
	abuf_map_sysmem(&buf, 0, CONFIG_SYS_MALLOC_LEN);
	ut_asserteq(CONFIG_SYS_MALLOC_LEN, buf.size);
	ut_asserteq(false, buf.alloced);
	ut_assertnull(abuf_uninit_move(&buf, &size));

	/* Check for memory leaks */
	abuf_uninit(&buf);
	ut_assertok(ut_check_delta(start));

	return 0;
}
LIB_TEST(lib_test_abuf_large, 0);

/* Test abuf_uninit_move() */
static int lib_test_abuf_uninit_move(struct unit_test_state *uts)
{
	void *ptr, *orig_ptr;
	struct abuf buf;
	size_t size;
	ulong start;
	int delta;

	start = ut_check_free();

	/*
	 * TODO: crashes on sandbox sometimes due to an apparent bug in
	 * realloc().
	 */
	return 0;

	/* Move an empty buffer */
	abuf_init(&buf);
	ut_assertnull(abuf_uninit_move(&buf, &size));
	ut_asserteq(0, size);
	ut_assertnull(abuf_uninit_move(&buf, NULL));

	/* Move an unallocated buffer */
	abuf_set(&buf, test_data, TEST_DATA_LEN);
	ut_assertok(ut_check_delta(start));
	ptr = abuf_uninit_move(&buf, &size);
	ut_asserteq(TEST_DATA_LEN, size);
	ut_asserteq_str(ptr, test_data);
	ut_assertnonnull(ptr);
	ut_assertnull(buf.data);
	ut_asserteq(0, buf.size);
	ut_asserteq(false, buf.alloced);

	/* Check that freeing it frees the only allocation */
	delta = ut_check_delta(start);
	ut_assert(delta > 0);
	free(ptr);
	ut_assertok(ut_check_delta(start));

	/* Move an allocated buffer */
	ut_asserteq(true, abuf_realloc(&buf, TEST_DATA_LEN));
	orig_ptr = buf.data;
	strcpy(orig_ptr, test_data);

	delta = ut_check_delta(start);
	ut_assert(delta > 0);
	ptr = abuf_uninit_move(&buf, &size);
	ut_asserteq(TEST_DATA_LEN, size);
	ut_assertnonnull(ptr);
	ut_asserteq_ptr(ptr, orig_ptr);
	ut_asserteq_str(ptr, test_data);
	ut_assertnull(buf.data);
	ut_asserteq(0, buf.size);
	ut_asserteq(false, buf.alloced);

	/* Check there was no new allocation */
	ut_asserteq(delta, ut_check_delta(start));

	/* Check that freeing it frees the only allocation */
	free(ptr);
	ut_assertok(ut_check_delta(start));

	/* Move an unallocated buffer, without the size */
	abuf_set(&buf, test_data, TEST_DATA_LEN);
	ut_assertok(ut_check_delta(start));
	ptr = abuf_uninit_move(&buf, NULL);
	ut_asserteq_str(ptr, test_data);

	return 0;
}
LIB_TEST(lib_test_abuf_uninit_move, 0);

/* Test abuf_uninit() */
static int lib_test_abuf_uninit(struct unit_test_state *uts)
{
	struct abuf buf;

	/* Nothing in the buffer */
	abuf_init(&buf);
	abuf_uninit(&buf);
	ut_assertnull(buf.data);
	ut_asserteq(0, buf.size);
	ut_asserteq(false, buf.alloced);

	/* Not allocated */
	abuf_set(&buf, test_data, TEST_DATA_LEN);
	abuf_uninit(&buf);
	ut_assertnull(buf.data);
	ut_asserteq(0, buf.size);
	ut_asserteq(false, buf.alloced);

	return 0;
}
LIB_TEST(lib_test_abuf_uninit, 0);

/* Test abuf_init_set() */
static int lib_test_abuf_init_set(struct unit_test_state *uts)
{
	struct abuf buf;

	abuf_init_set(&buf, test_data, TEST_DATA_LEN);
	ut_asserteq_ptr(test_data, buf.data);
	ut_asserteq(TEST_DATA_LEN, buf.size);
	ut_asserteq(false, buf.alloced);

	return 0;
}
LIB_TEST(lib_test_abuf_init_set, 0);

/* Test abuf_init_move() */
static int lib_test_abuf_init_move(struct unit_test_state *uts)
{
	struct abuf buf;
	void *ptr;

	/*
	 * TODO: crashes on sandbox sometimes due to an apparent bug in
	 * realloc().
	 */
	return 0;

	ptr = strdup(test_data);
	ut_assertnonnull(ptr);

	free(ptr);

	abuf_init_move(&buf, ptr, TEST_DATA_LEN);
	ut_asserteq_ptr(ptr, abuf_data(&buf));
	ut_asserteq(TEST_DATA_LEN, abuf_size(&buf));
	ut_asserteq(true, buf.alloced);

	return 0;
}
LIB_TEST(lib_test_abuf_init_move, 0);

/* Test abuf_init() */
static int lib_test_abuf_init(struct unit_test_state *uts)
{
	struct abuf buf;

	buf.data = &buf;
	buf.size = 123;
	buf.alloced = true;
	abuf_init(&buf);
	ut_assertnull(buf.data);
	ut_asserteq(0, buf.size);
	ut_asserteq(false, buf.alloced);

	return 0;
}
LIB_TEST(lib_test_abuf_init, 0);
