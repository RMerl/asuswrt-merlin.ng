// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for memory 'cp' command
 */

#include <command.h>
#include <console.h>
#include <mapmem.h>
#include <dm/test.h>
#include <test/ut.h>

#define BUF_SIZE	256

/* Declare a new mem test */
#define MEM_TEST(_name)	UNIT_TEST(_name, 0, mem_test)

struct param {
	int d, s, count;
};

static int do_test(struct unit_test_state *uts,
		   const char *suffix, int d, int s, int count)
{
	const long addr = 0x1000;
	u8 shadow[BUF_SIZE];
	u8 *buf;
	int i, w, bytes;

	buf = map_sysmem(addr, BUF_SIZE);

	/* Fill with distinct bytes. */
	for (i = 0; i < BUF_SIZE; ++i)
		buf[i] = shadow[i] = i;

	/* Parameter sanity checking. */
	w = cmd_get_data_size(suffix, 4);
	ut_assert(w == 1 || w == 2 || w == 4 || (MEM_SUPPORT_64BIT_DATA && w == 8));

	bytes = count * w;
	ut_assert(d < BUF_SIZE);
	ut_assert(d + bytes <= BUF_SIZE);
	ut_assert(s < BUF_SIZE);
	ut_assert(s + bytes <= BUF_SIZE);

	/* This is exactly what we expect to happen to "buf" */
	memmove(shadow + d, shadow + s, bytes);

	run_commandf("cp%s 0x%lx 0x%lx 0x%x", suffix, addr + s, addr + d, count);

	ut_asserteq(0, memcmp(buf, shadow, BUF_SIZE));

	unmap_sysmem(buf);

	return 0;
}

static int mem_test_cp_b(struct unit_test_state *uts)
{
	static const struct param tests[] = {
		{ 0, 128, 128 },
		{ 128, 0, 128 },
		{ 0, 16, 32 },
		{ 16, 0, 32 },
		{ 60, 100, 100 },
		{ 100, 60, 100 },
		{ 123, 54, 96 },
		{ 54, 123, 96 },
	};
	const struct param *p;
	int ret, i;

	for (i = 0; i < ARRAY_SIZE(tests); ++i) {
		p = &tests[i];
		ret = do_test(uts, ".b", p->d, p->s, p->count);
		if (ret)
			return ret;
	}

	return 0;
}
MEM_TEST(mem_test_cp_b);

static int mem_test_cp_w(struct unit_test_state *uts)
{
	static const struct param tests[] = {
		{ 0, 128, 64 },
		{ 128, 0, 64 },
		{ 0, 16, 16 },
		{ 16, 0, 16 },
		{ 60, 100, 50 },
		{ 100, 60, 50 },
		{ 123, 54, 48 },
		{ 54, 123, 48 },
	};
	const struct param *p;
	int ret, i;

	for (i = 0; i < ARRAY_SIZE(tests); ++i) {
		p = &tests[i];
		ret = do_test(uts, ".w", p->d, p->s, p->count);
		if (ret)
			return ret;
	}

	return 0;
}
MEM_TEST(mem_test_cp_w);

static int mem_test_cp_l(struct unit_test_state *uts)
{
	static const struct param tests[] = {
		{ 0, 128, 32 },
		{ 128, 0, 32 },
		{ 0, 16, 8 },
		{ 16, 0, 8 },
		{ 60, 100, 25 },
		{ 100, 60, 25 },
		{ 123, 54, 24 },
		{ 54, 123, 24 },
	};
	const struct param *p;
	int ret, i;

	for (i = 0; i < ARRAY_SIZE(tests); ++i) {
		p = &tests[i];
		ret = do_test(uts, ".l", p->d, p->s, p->count);
		if (ret)
			return ret;
	}

	for (i = 0; i < ARRAY_SIZE(tests); ++i) {
		p = &tests[i];
		ret = do_test(uts, "", p->d, p->s, p->count);
		if (ret)
			return ret;
	}

	return 0;
}
MEM_TEST(mem_test_cp_l);

#if MEM_SUPPORT_64BIT_DATA
static int mem_test_cp_q(struct unit_test_state *uts)
{
	static const struct param tests[] = {
		{ 0, 128, 16 },
		{ 128, 0, 16 },
		{ 0, 16, 8 },
		{ 16, 0, 8 },
		{ 60, 100, 15 },
		{ 100, 60, 15 },
		{ 123, 54, 12 },
		{ 54, 123, 12 },
	};
	const struct param *p;
	int ret, i;

	for (i = 0; i < ARRAY_SIZE(tests); ++i) {
		p = &tests[i];
		ret = do_test(uts, ".q", p->d, p->s, p->count);
		if (ret)
			return ret;
	}

	return 0;
}
MEM_TEST(mem_test_cp_q);
#endif
