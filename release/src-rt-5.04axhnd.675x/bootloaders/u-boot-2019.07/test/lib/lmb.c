// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018 Simon Goldschmidt
 */

#include <common.h>
#include <lmb.h>
#include <dm/test.h>
#include <test/ut.h>

static int check_lmb(struct unit_test_state *uts, struct lmb *lmb,
		     phys_addr_t ram_base, phys_size_t ram_size,
		     unsigned long num_reserved,
		     phys_addr_t base1, phys_size_t size1,
		     phys_addr_t base2, phys_size_t size2,
		     phys_addr_t base3, phys_size_t size3)
{
	if (ram_size) {
		ut_asserteq(lmb->memory.cnt, 1);
		ut_asserteq(lmb->memory.region[0].base, ram_base);
		ut_asserteq(lmb->memory.region[0].size, ram_size);
	}

	ut_asserteq(lmb->reserved.cnt, num_reserved);
	if (num_reserved > 0) {
		ut_asserteq(lmb->reserved.region[0].base, base1);
		ut_asserteq(lmb->reserved.region[0].size, size1);
	}
	if (num_reserved > 1) {
		ut_asserteq(lmb->reserved.region[1].base, base2);
		ut_asserteq(lmb->reserved.region[1].size, size2);
	}
	if (num_reserved > 2) {
		ut_asserteq(lmb->reserved.region[2].base, base3);
		ut_asserteq(lmb->reserved.region[2].size, size3);
	}
	return 0;
}

#define ASSERT_LMB(lmb, ram_base, ram_size, num_reserved, base1, size1, \
		   base2, size2, base3, size3) \
		   ut_assert(!check_lmb(uts, lmb, ram_base, ram_size, \
			     num_reserved, base1, size1, base2, size2, base3, \
			     size3))

/*
 * Test helper function that reserves 64 KiB somewhere in the simulated RAM and
 * then does some alloc + free tests.
 */
static int test_multi_alloc(struct unit_test_state *uts, const phys_addr_t ram,
			    const phys_size_t ram_size, const phys_addr_t ram0,
			    const phys_size_t ram0_size,
			    const phys_addr_t alloc_64k_addr)
{
	const phys_addr_t ram_end = ram + ram_size;
	const phys_addr_t alloc_64k_end = alloc_64k_addr + 0x10000;

	struct lmb lmb;
	long ret;
	phys_addr_t a, a2, b, b2, c, d;

	/* check for overflow */
	ut_assert(ram_end == 0 || ram_end > ram);
	ut_assert(alloc_64k_end > alloc_64k_addr);
	/* check input addresses + size */
	ut_assert(alloc_64k_addr >= ram + 8);
	ut_assert(alloc_64k_end <= ram_end - 8);

	lmb_init(&lmb);

	if (ram0_size) {
		ret = lmb_add(&lmb, ram0, ram0_size);
		ut_asserteq(ret, 0);
	}

	ret = lmb_add(&lmb, ram, ram_size);
	ut_asserteq(ret, 0);

	if (ram0_size) {
		ut_asserteq(lmb.memory.cnt, 2);
		ut_asserteq(lmb.memory.region[0].base, ram0);
		ut_asserteq(lmb.memory.region[0].size, ram0_size);
		ut_asserteq(lmb.memory.region[1].base, ram);
		ut_asserteq(lmb.memory.region[1].size, ram_size);
	} else {
		ut_asserteq(lmb.memory.cnt, 1);
		ut_asserteq(lmb.memory.region[0].base, ram);
		ut_asserteq(lmb.memory.region[0].size, ram_size);
	}

	/* reserve 64KiB somewhere */
	ret = lmb_reserve(&lmb, alloc_64k_addr, 0x10000);
	ut_asserteq(ret, 0);
	ASSERT_LMB(&lmb, 0, 0, 1, alloc_64k_addr, 0x10000,
		   0, 0, 0, 0);

	/* allocate somewhere, should be at the end of RAM */
	a = lmb_alloc(&lmb, 4, 1);
	ut_asserteq(a, ram_end - 4);
	ASSERT_LMB(&lmb, 0, 0, 2, alloc_64k_addr, 0x10000,
		   ram_end - 4, 4, 0, 0);
	/* alloc below end of reserved region -> below reserved region */
	b = lmb_alloc_base(&lmb, 4, 1, alloc_64k_end);
	ut_asserteq(b, alloc_64k_addr - 4);
	ASSERT_LMB(&lmb, 0, 0, 2,
		   alloc_64k_addr - 4, 0x10000 + 4, ram_end - 4, 4, 0, 0);

	/* 2nd time */
	c = lmb_alloc(&lmb, 4, 1);
	ut_asserteq(c, ram_end - 8);
	ASSERT_LMB(&lmb, 0, 0, 2,
		   alloc_64k_addr - 4, 0x10000 + 4, ram_end - 8, 8, 0, 0);
	d = lmb_alloc_base(&lmb, 4, 1, alloc_64k_end);
	ut_asserteq(d, alloc_64k_addr - 8);
	ASSERT_LMB(&lmb, 0, 0, 2,
		   alloc_64k_addr - 8, 0x10000 + 8, ram_end - 8, 8, 0, 0);

	ret = lmb_free(&lmb, a, 4);
	ut_asserteq(ret, 0);
	ASSERT_LMB(&lmb, 0, 0, 2,
		   alloc_64k_addr - 8, 0x10000 + 8, ram_end - 8, 4, 0, 0);
	/* allocate again to ensure we get the same address */
	a2 = lmb_alloc(&lmb, 4, 1);
	ut_asserteq(a, a2);
	ASSERT_LMB(&lmb, 0, 0, 2,
		   alloc_64k_addr - 8, 0x10000 + 8, ram_end - 8, 8, 0, 0);
	ret = lmb_free(&lmb, a2, 4);
	ut_asserteq(ret, 0);
	ASSERT_LMB(&lmb, 0, 0, 2,
		   alloc_64k_addr - 8, 0x10000 + 8, ram_end - 8, 4, 0, 0);

	ret = lmb_free(&lmb, b, 4);
	ut_asserteq(ret, 0);
	ASSERT_LMB(&lmb, 0, 0, 3,
		   alloc_64k_addr - 8, 4, alloc_64k_addr, 0x10000,
		   ram_end - 8, 4);
	/* allocate again to ensure we get the same address */
	b2 = lmb_alloc_base(&lmb, 4, 1, alloc_64k_end);
	ut_asserteq(b, b2);
	ASSERT_LMB(&lmb, 0, 0, 2,
		   alloc_64k_addr - 8, 0x10000 + 8, ram_end - 8, 4, 0, 0);
	ret = lmb_free(&lmb, b2, 4);
	ut_asserteq(ret, 0);
	ASSERT_LMB(&lmb, 0, 0, 3,
		   alloc_64k_addr - 8, 4, alloc_64k_addr, 0x10000,
		   ram_end - 8, 4);

	ret = lmb_free(&lmb, c, 4);
	ut_asserteq(ret, 0);
	ASSERT_LMB(&lmb, 0, 0, 2,
		   alloc_64k_addr - 8, 4, alloc_64k_addr, 0x10000, 0, 0);
	ret = lmb_free(&lmb, d, 4);
	ut_asserteq(ret, 0);
	ASSERT_LMB(&lmb, 0, 0, 1, alloc_64k_addr, 0x10000,
		   0, 0, 0, 0);

	if (ram0_size) {
		ut_asserteq(lmb.memory.cnt, 2);
		ut_asserteq(lmb.memory.region[0].base, ram0);
		ut_asserteq(lmb.memory.region[0].size, ram0_size);
		ut_asserteq(lmb.memory.region[1].base, ram);
		ut_asserteq(lmb.memory.region[1].size, ram_size);
	} else {
		ut_asserteq(lmb.memory.cnt, 1);
		ut_asserteq(lmb.memory.region[0].base, ram);
		ut_asserteq(lmb.memory.region[0].size, ram_size);
	}

	return 0;
}

static int test_multi_alloc_512mb(struct unit_test_state *uts,
				  const phys_addr_t ram)
{
	return test_multi_alloc(uts, ram, 0x20000000, 0, 0, ram + 0x10000000);
}

static int test_multi_alloc_512mb_x2(struct unit_test_state *uts,
				     const phys_addr_t ram,
				     const phys_addr_t ram0)
{
	return test_multi_alloc(uts, ram, 0x20000000, ram0, 0x20000000,
				ram + 0x10000000);
}

/* Create a memory region with one reserved region and allocate */
static int lib_test_lmb_simple(struct unit_test_state *uts)
{
	int ret;

	/* simulate 512 MiB RAM beginning at 1GiB */
	ret = test_multi_alloc_512mb(uts, 0x40000000);
	if (ret)
		return ret;

	/* simulate 512 MiB RAM beginning at 1.5GiB */
	return test_multi_alloc_512mb(uts, 0xE0000000);
}

DM_TEST(lib_test_lmb_simple, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Create two memory regions with one reserved region and allocate */
static int lib_test_lmb_simple_x2(struct unit_test_state *uts)
{
	int ret;

	/* simulate 512 MiB RAM beginning at 2GiB and 1 GiB */
	ret = test_multi_alloc_512mb_x2(uts, 0x80000000, 0x40000000);
	if (ret)
		return ret;

	/* simulate 512 MiB RAM beginning at 3.5GiB and 1 GiB */
	return test_multi_alloc_512mb_x2(uts, 0xE0000000, 0x40000000);
}

DM_TEST(lib_test_lmb_simple_x2,  DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Simulate 512 MiB RAM, allocate some blocks that fit/don't fit */
static int test_bigblock(struct unit_test_state *uts, const phys_addr_t ram)
{
	const phys_size_t ram_size = 0x20000000;
	const phys_size_t big_block_size = 0x10000000;
	const phys_addr_t ram_end = ram + ram_size;
	const phys_addr_t alloc_64k_addr = ram + 0x10000000;
	struct lmb lmb;
	long ret;
	phys_addr_t a, b;

	/* check for overflow */
	ut_assert(ram_end == 0 || ram_end > ram);

	lmb_init(&lmb);

	ret = lmb_add(&lmb, ram, ram_size);
	ut_asserteq(ret, 0);

	/* reserve 64KiB in the middle of RAM */
	ret = lmb_reserve(&lmb, alloc_64k_addr, 0x10000);
	ut_asserteq(ret, 0);
	ASSERT_LMB(&lmb, ram, ram_size, 1, alloc_64k_addr, 0x10000,
		   0, 0, 0, 0);

	/* allocate a big block, should be below reserved */
	a = lmb_alloc(&lmb, big_block_size, 1);
	ut_asserteq(a, ram);
	ASSERT_LMB(&lmb, ram, ram_size, 1, a,
		   big_block_size + 0x10000, 0, 0, 0, 0);
	/* allocate 2nd big block */
	/* This should fail, printing an error */
	b = lmb_alloc(&lmb, big_block_size, 1);
	ut_asserteq(b, 0);
	ASSERT_LMB(&lmb, ram, ram_size, 1, a,
		   big_block_size + 0x10000, 0, 0, 0, 0);

	ret = lmb_free(&lmb, a, big_block_size);
	ut_asserteq(ret, 0);
	ASSERT_LMB(&lmb, ram, ram_size, 1, alloc_64k_addr, 0x10000,
		   0, 0, 0, 0);

	/* allocate too big block */
	/* This should fail, printing an error */
	a = lmb_alloc(&lmb, ram_size, 1);
	ut_asserteq(a, 0);
	ASSERT_LMB(&lmb, ram, ram_size, 1, alloc_64k_addr, 0x10000,
		   0, 0, 0, 0);

	return 0;
}

static int lib_test_lmb_big(struct unit_test_state *uts)
{
	int ret;

	/* simulate 512 MiB RAM beginning at 1GiB */
	ret = test_bigblock(uts, 0x40000000);
	if (ret)
		return ret;

	/* simulate 512 MiB RAM beginning at 1.5GiB */
	return test_bigblock(uts, 0xE0000000);
}

DM_TEST(lib_test_lmb_big, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Simulate 512 MiB RAM, allocate a block without previous reservation */
static int test_noreserved(struct unit_test_state *uts, const phys_addr_t ram,
			   const phys_addr_t alloc_size, const ulong align)
{
	const phys_size_t ram_size = 0x20000000;
	const phys_addr_t ram_end = ram + ram_size;
	struct lmb lmb;
	long ret;
	phys_addr_t a, b;
	const phys_addr_t alloc_size_aligned = (alloc_size + align - 1) &
		~(align - 1);

	/* check for overflow */
	ut_assert(ram_end == 0 || ram_end > ram);

	lmb_init(&lmb);

	ret = lmb_add(&lmb, ram, ram_size);
	ut_asserteq(ret, 0);
	ASSERT_LMB(&lmb, ram, ram_size, 0, 0, 0, 0, 0, 0, 0);

	/* allocate a block */
	a = lmb_alloc(&lmb, alloc_size, align);
	ut_assert(a != 0);
	ASSERT_LMB(&lmb, ram, ram_size, 1, ram + ram_size - alloc_size_aligned,
		   alloc_size, 0, 0, 0, 0);
	/* allocate another block */
	b = lmb_alloc(&lmb, alloc_size, align);
	ut_assert(b != 0);
	if (alloc_size == alloc_size_aligned) {
		ASSERT_LMB(&lmb, ram, ram_size, 1, ram + ram_size -
			   (alloc_size_aligned * 2), alloc_size * 2, 0, 0, 0,
			   0);
	} else {
		ASSERT_LMB(&lmb, ram, ram_size, 2, ram + ram_size -
			   (alloc_size_aligned * 2), alloc_size, ram + ram_size
			   - alloc_size_aligned, alloc_size, 0, 0);
	}
	/* and free them */
	ret = lmb_free(&lmb, b, alloc_size);
	ut_asserteq(ret, 0);
	ASSERT_LMB(&lmb, ram, ram_size, 1, ram + ram_size - alloc_size_aligned,
		   alloc_size, 0, 0, 0, 0);
	ret = lmb_free(&lmb, a, alloc_size);
	ut_asserteq(ret, 0);
	ASSERT_LMB(&lmb, ram, ram_size, 0, 0, 0, 0, 0, 0, 0);

	/* allocate a block with base*/
	b = lmb_alloc_base(&lmb, alloc_size, align, ram_end);
	ut_assert(a == b);
	ASSERT_LMB(&lmb, ram, ram_size, 1, ram + ram_size - alloc_size_aligned,
		   alloc_size, 0, 0, 0, 0);
	/* and free it */
	ret = lmb_free(&lmb, b, alloc_size);
	ut_asserteq(ret, 0);
	ASSERT_LMB(&lmb, ram, ram_size, 0, 0, 0, 0, 0, 0, 0);

	return 0;
}

static int lib_test_lmb_noreserved(struct unit_test_state *uts)
{
	int ret;

	/* simulate 512 MiB RAM beginning at 1GiB */
	ret = test_noreserved(uts, 0x40000000, 4, 1);
	if (ret)
		return ret;

	/* simulate 512 MiB RAM beginning at 1.5GiB */
	return test_noreserved(uts, 0xE0000000, 4, 1);
}

DM_TEST(lib_test_lmb_noreserved, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

static int lib_test_lmb_unaligned_size(struct unit_test_state *uts)
{
	int ret;

	/* simulate 512 MiB RAM beginning at 1GiB */
	ret = test_noreserved(uts, 0x40000000, 5, 8);
	if (ret)
		return ret;

	/* simulate 512 MiB RAM beginning at 1.5GiB */
	return test_noreserved(uts, 0xE0000000, 5, 8);
}

DM_TEST(lib_test_lmb_unaligned_size, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
/*
 * Simulate a RAM that starts at 0 and allocate down to address 0, which must
 * fail as '0' means failure for the lmb_alloc functions.
 */
static int lib_test_lmb_at_0(struct unit_test_state *uts)
{
	const phys_addr_t ram = 0;
	const phys_size_t ram_size = 0x20000000;
	struct lmb lmb;
	long ret;
	phys_addr_t a, b;

	lmb_init(&lmb);

	ret = lmb_add(&lmb, ram, ram_size);
	ut_asserteq(ret, 0);

	/* allocate nearly everything */
	a = lmb_alloc(&lmb, ram_size - 4, 1);
	ut_asserteq(a, ram + 4);
	ASSERT_LMB(&lmb, ram, ram_size, 1, a, ram_size - 4,
		   0, 0, 0, 0);
	/* allocate the rest */
	/* This should fail as the allocated address would be 0 */
	b = lmb_alloc(&lmb, 4, 1);
	ut_asserteq(b, 0);
	/* check that this was an error by checking lmb */
	ASSERT_LMB(&lmb, ram, ram_size, 1, a, ram_size - 4,
		   0, 0, 0, 0);
	/* check that this was an error by freeing b */
	ret = lmb_free(&lmb, b, 4);
	ut_asserteq(ret, -1);
	ASSERT_LMB(&lmb, ram, ram_size, 1, a, ram_size - 4,
		   0, 0, 0, 0);

	ret = lmb_free(&lmb, a, ram_size - 4);
	ut_asserteq(ret, 0);
	ASSERT_LMB(&lmb, ram, ram_size, 0, 0, 0, 0, 0, 0, 0);

	return 0;
}

DM_TEST(lib_test_lmb_at_0, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Check that calling lmb_reserve with overlapping regions fails. */
static int lib_test_lmb_overlapping_reserve(struct unit_test_state *uts)
{
	const phys_addr_t ram = 0x40000000;
	const phys_size_t ram_size = 0x20000000;
	struct lmb lmb;
	long ret;

	lmb_init(&lmb);

	ret = lmb_add(&lmb, ram, ram_size);
	ut_asserteq(ret, 0);

	ret = lmb_reserve(&lmb, 0x40010000, 0x10000);
	ut_asserteq(ret, 0);
	ASSERT_LMB(&lmb, ram, ram_size, 1, 0x40010000, 0x10000,
		   0, 0, 0, 0);
	/* allocate overlapping region should fail */
	ret = lmb_reserve(&lmb, 0x40011000, 0x10000);
	ut_asserteq(ret, -1);
	ASSERT_LMB(&lmb, ram, ram_size, 1, 0x40010000, 0x10000,
		   0, 0, 0, 0);
	/* allocate 3nd region */
	ret = lmb_reserve(&lmb, 0x40030000, 0x10000);
	ut_asserteq(ret, 0);
	ASSERT_LMB(&lmb, ram, ram_size, 2, 0x40010000, 0x10000,
		   0x40030000, 0x10000, 0, 0);
	/* allocate 2nd region */
	ret = lmb_reserve(&lmb, 0x40020000, 0x10000);
	ut_assert(ret >= 0);
	ASSERT_LMB(&lmb, ram, ram_size, 1, 0x40010000, 0x30000,
		   0, 0, 0, 0);

	return 0;
}

DM_TEST(lib_test_lmb_overlapping_reserve,
	DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/*
 * Simulate 512 MiB RAM, reserve 3 blocks, allocate addresses in between.
 * Expect addresses outside the memory range to fail.
 */
static int test_alloc_addr(struct unit_test_state *uts, const phys_addr_t ram)
{
	const phys_size_t ram_size = 0x20000000;
	const phys_addr_t ram_end = ram + ram_size;
	const phys_size_t alloc_addr_a = ram + 0x8000000;
	const phys_size_t alloc_addr_b = ram + 0x8000000 * 2;
	const phys_size_t alloc_addr_c = ram + 0x8000000 * 3;
	struct lmb lmb;
	long ret;
	phys_addr_t a, b, c, d, e;

	/* check for overflow */
	ut_assert(ram_end == 0 || ram_end > ram);

	lmb_init(&lmb);

	ret = lmb_add(&lmb, ram, ram_size);
	ut_asserteq(ret, 0);

	/*  reserve 3 blocks */
	ret = lmb_reserve(&lmb, alloc_addr_a, 0x10000);
	ut_asserteq(ret, 0);
	ret = lmb_reserve(&lmb, alloc_addr_b, 0x10000);
	ut_asserteq(ret, 0);
	ret = lmb_reserve(&lmb, alloc_addr_c, 0x10000);
	ut_asserteq(ret, 0);
	ASSERT_LMB(&lmb, ram, ram_size, 3, alloc_addr_a, 0x10000,
		   alloc_addr_b, 0x10000, alloc_addr_c, 0x10000);

	/* allocate blocks */
	a = lmb_alloc_addr(&lmb, ram, alloc_addr_a - ram);
	ut_asserteq(a, ram);
	ASSERT_LMB(&lmb, ram, ram_size, 3, ram, 0x8010000,
		   alloc_addr_b, 0x10000, alloc_addr_c, 0x10000);
	b = lmb_alloc_addr(&lmb, alloc_addr_a + 0x10000,
			   alloc_addr_b - alloc_addr_a - 0x10000);
	ut_asserteq(b, alloc_addr_a + 0x10000);
	ASSERT_LMB(&lmb, ram, ram_size, 2, ram, 0x10010000,
		   alloc_addr_c, 0x10000, 0, 0);
	c = lmb_alloc_addr(&lmb, alloc_addr_b + 0x10000,
			   alloc_addr_c - alloc_addr_b - 0x10000);
	ut_asserteq(c, alloc_addr_b + 0x10000);
	ASSERT_LMB(&lmb, ram, ram_size, 1, ram, 0x18010000,
		   0, 0, 0, 0);
	d = lmb_alloc_addr(&lmb, alloc_addr_c + 0x10000,
			   ram_end - alloc_addr_c - 0x10000);
	ut_asserteq(d, alloc_addr_c + 0x10000);
	ASSERT_LMB(&lmb, ram, ram_size, 1, ram, ram_size,
		   0, 0, 0, 0);

	/* allocating anything else should fail */
	e = lmb_alloc(&lmb, 1, 1);
	ut_asserteq(e, 0);
	ASSERT_LMB(&lmb, ram, ram_size, 1, ram, ram_size,
		   0, 0, 0, 0);

	ret = lmb_free(&lmb, d, ram_end - alloc_addr_c - 0x10000);
	ut_asserteq(ret, 0);

	/* allocate at 3 points in free range */

	d = lmb_alloc_addr(&lmb, ram_end - 4, 4);
	ut_asserteq(d, ram_end - 4);
	ASSERT_LMB(&lmb, ram, ram_size, 2, ram, 0x18010000,
		   d, 4, 0, 0);
	ret = lmb_free(&lmb, d, 4);
	ut_asserteq(ret, 0);
	ASSERT_LMB(&lmb, ram, ram_size, 1, ram, 0x18010000,
		   0, 0, 0, 0);

	d = lmb_alloc_addr(&lmb, ram_end - 128, 4);
	ut_asserteq(d, ram_end - 128);
	ASSERT_LMB(&lmb, ram, ram_size, 2, ram, 0x18010000,
		   d, 4, 0, 0);
	ret = lmb_free(&lmb, d, 4);
	ut_asserteq(ret, 0);
	ASSERT_LMB(&lmb, ram, ram_size, 1, ram, 0x18010000,
		   0, 0, 0, 0);

	d = lmb_alloc_addr(&lmb, alloc_addr_c + 0x10000, 4);
	ut_asserteq(d, alloc_addr_c + 0x10000);
	ASSERT_LMB(&lmb, ram, ram_size, 1, ram, 0x18010004,
		   0, 0, 0, 0);
	ret = lmb_free(&lmb, d, 4);
	ut_asserteq(ret, 0);
	ASSERT_LMB(&lmb, ram, ram_size, 1, ram, 0x18010000,
		   0, 0, 0, 0);

	/* allocate at the bottom */
	ret = lmb_free(&lmb, a, alloc_addr_a - ram);
	ut_asserteq(ret, 0);
	ASSERT_LMB(&lmb, ram, ram_size, 1, ram + 0x8000000, 0x10010000,
		   0, 0, 0, 0);
	d = lmb_alloc_addr(&lmb, ram, 4);
	ut_asserteq(d, ram);
	ASSERT_LMB(&lmb, ram, ram_size, 2, d, 4,
		   ram + 0x8000000, 0x10010000, 0, 0);

	/* check that allocating outside memory fails */
	if (ram_end != 0) {
		ret = lmb_alloc_addr(&lmb, ram_end, 1);
		ut_asserteq(ret, 0);
	}
	if (ram != 0) {
		ret = lmb_alloc_addr(&lmb, ram - 1, 1);
		ut_asserteq(ret, 0);
	}

	return 0;
}

static int lib_test_lmb_alloc_addr(struct unit_test_state *uts)
{
	int ret;

	/* simulate 512 MiB RAM beginning at 1GiB */
	ret = test_alloc_addr(uts, 0x40000000);
	if (ret)
		return ret;

	/* simulate 512 MiB RAM beginning at 1.5GiB */
	return test_alloc_addr(uts, 0xE0000000);
}

DM_TEST(lib_test_lmb_alloc_addr, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Simulate 512 MiB RAM, reserve 3 blocks, check addresses in between */
static int test_get_unreserved_size(struct unit_test_state *uts,
				    const phys_addr_t ram)
{
	const phys_size_t ram_size = 0x20000000;
	const phys_addr_t ram_end = ram + ram_size;
	const phys_size_t alloc_addr_a = ram + 0x8000000;
	const phys_size_t alloc_addr_b = ram + 0x8000000 * 2;
	const phys_size_t alloc_addr_c = ram + 0x8000000 * 3;
	struct lmb lmb;
	long ret;
	phys_size_t s;

	/* check for overflow */
	ut_assert(ram_end == 0 || ram_end > ram);

	lmb_init(&lmb);

	ret = lmb_add(&lmb, ram, ram_size);
	ut_asserteq(ret, 0);

	/*  reserve 3 blocks */
	ret = lmb_reserve(&lmb, alloc_addr_a, 0x10000);
	ut_asserteq(ret, 0);
	ret = lmb_reserve(&lmb, alloc_addr_b, 0x10000);
	ut_asserteq(ret, 0);
	ret = lmb_reserve(&lmb, alloc_addr_c, 0x10000);
	ut_asserteq(ret, 0);
	ASSERT_LMB(&lmb, ram, ram_size, 3, alloc_addr_a, 0x10000,
		   alloc_addr_b, 0x10000, alloc_addr_c, 0x10000);

	/* check addresses in between blocks */
	s = lmb_get_free_size(&lmb, ram);
	ut_asserteq(s, alloc_addr_a - ram);
	s = lmb_get_free_size(&lmb, ram + 0x10000);
	ut_asserteq(s, alloc_addr_a - ram - 0x10000);
	s = lmb_get_free_size(&lmb, alloc_addr_a - 4);
	ut_asserteq(s, 4);

	s = lmb_get_free_size(&lmb, alloc_addr_a + 0x10000);
	ut_asserteq(s, alloc_addr_b - alloc_addr_a - 0x10000);
	s = lmb_get_free_size(&lmb, alloc_addr_a + 0x20000);
	ut_asserteq(s, alloc_addr_b - alloc_addr_a - 0x20000);
	s = lmb_get_free_size(&lmb, alloc_addr_b - 4);
	ut_asserteq(s, 4);

	s = lmb_get_free_size(&lmb, alloc_addr_c + 0x10000);
	ut_asserteq(s, ram_end - alloc_addr_c - 0x10000);
	s = lmb_get_free_size(&lmb, alloc_addr_c + 0x20000);
	ut_asserteq(s, ram_end - alloc_addr_c - 0x20000);
	s = lmb_get_free_size(&lmb, ram_end - 4);
	ut_asserteq(s, 4);

	return 0;
}

static int lib_test_lmb_get_free_size(struct unit_test_state *uts)
{
	int ret;

	/* simulate 512 MiB RAM beginning at 1GiB */
	ret = test_get_unreserved_size(uts, 0x40000000);
	if (ret)
		return ret;

	/* simulate 512 MiB RAM beginning at 1.5GiB */
	return test_get_unreserved_size(uts, 0xE0000000);
}

DM_TEST(lib_test_lmb_get_free_size,
	DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
