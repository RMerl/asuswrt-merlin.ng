/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2014 Google, Inc
 *
 * From Coreboot file of the same name
 */

#ifndef _ASM_MTRR_H
#define _ASM_MTRR_H

/* MTRR region types */
#define MTRR_TYPE_UNCACHEABLE	0
#define MTRR_TYPE_WRCOMB	1
#define MTRR_TYPE_WRTHROUGH	4
#define MTRR_TYPE_WRPROT	5
#define MTRR_TYPE_WRBACK	6

#define MTRR_TYPE_COUNT		7

#define MTRR_CAP_MSR		0x0fe
#define MTRR_DEF_TYPE_MSR	0x2ff

#define MTRR_CAP_SMRR		(1 << 11)
#define MTRR_CAP_WC		(1 << 10)
#define MTRR_CAP_FIX		(1 << 8)
#define MTRR_CAP_VCNT_MASK	0xff

#define MTRR_DEF_TYPE_EN	(1 << 11)
#define MTRR_DEF_TYPE_FIX_EN	(1 << 10)

#define MTRR_PHYS_BASE_MSR(reg)	(0x200 + 2 * (reg))
#define MTRR_PHYS_MASK_MSR(reg)	(0x200 + 2 * (reg) + 1)

#define MTRR_PHYS_MASK_VALID	(1 << 11)

#define MTRR_BASE_TYPE_MASK	0x7

/* Number of MTRRs supported */
#define MTRR_COUNT		8

#define NUM_FIXED_MTRRS		11
#define RANGES_PER_FIXED_MTRR	8
#define NUM_FIXED_RANGES	(NUM_FIXED_MTRRS * RANGES_PER_FIXED_MTRR)

#define MTRR_FIX_64K_00000_MSR	0x250
#define MTRR_FIX_16K_80000_MSR	0x258
#define MTRR_FIX_16K_A0000_MSR	0x259
#define MTRR_FIX_4K_C0000_MSR	0x268
#define MTRR_FIX_4K_C8000_MSR	0x269
#define MTRR_FIX_4K_D0000_MSR	0x26a
#define MTRR_FIX_4K_D8000_MSR	0x26b
#define MTRR_FIX_4K_E0000_MSR	0x26c
#define MTRR_FIX_4K_E8000_MSR	0x26d
#define MTRR_FIX_4K_F0000_MSR	0x26e
#define MTRR_FIX_4K_F8000_MSR	0x26f

#define MTRR_FIX_TYPE(t)	((t << 24) | (t << 16) | (t << 8) | t)

#if !defined(__ASSEMBLER__)

/**
 * Information about the previous MTRR state, set up by mtrr_open()
 *
 * @deftype:		Previous value of MTRR_DEF_TYPE_MSR
 * @enable_cache:	true if cache was enabled
 */
struct mtrr_state {
	uint64_t deftype;
	bool enable_cache;
};

/**
 * mtrr_open() - Prepare to adjust MTRRs
 *
 * Use mtrr_open() passing in a structure - this function will init it. Then
 * when done, pass the same structure to mtrr_close() to re-enable MTRRs and
 * possibly the cache.
 *
 * @state:	Empty structure to pass in to hold settings
 * @do_caches:	true to disable caches before opening
 */
void mtrr_open(struct mtrr_state *state, bool do_caches);

/**
 * mtrr_open() - Clean up after adjusting MTRRs, and enable them
 *
 * This uses the structure containing information returned from mtrr_open().
 *
 * @state:	Structure from mtrr_open()
 * @state:	true to restore cache state to that before mtrr_open()
 */
void mtrr_close(struct mtrr_state *state, bool do_caches);

/**
 * mtrr_add_request() - Add a new MTRR request
 *
 * This adds a request for a memory region to be set up in a particular way.
 *
 * @type:	Requested type (MTRR_TYPE_)
 * @start:	Start address
 * @size:	Size
 *
 * @return:	0 on success, non-zero on failure
 */
int mtrr_add_request(int type, uint64_t start, uint64_t size);

/**
 * mtrr_commit() - set up the MTRR registers based on current requests
 *
 * This sets up MTRRs for the available DRAM and the requests received so far.
 * It must be called with caches disabled.
 *
 * @do_caches:	true if caches are currently on
 *
 * @return:	0 on success, non-zero on failure
 */
int mtrr_commit(bool do_caches);

#endif

#if ((CONFIG_XIP_ROM_SIZE & (CONFIG_XIP_ROM_SIZE - 1)) != 0)
# error "CONFIG_XIP_ROM_SIZE is not a power of 2"
#endif

#if ((CONFIG_CACHE_ROM_SIZE & (CONFIG_CACHE_ROM_SIZE - 1)) != 0)
# error "CONFIG_CACHE_ROM_SIZE is not a power of 2"
#endif

#define CACHE_ROM_BASE	(((1 << 20) - (CONFIG_CACHE_ROM_SIZE >> 12)) << 12)

#endif
