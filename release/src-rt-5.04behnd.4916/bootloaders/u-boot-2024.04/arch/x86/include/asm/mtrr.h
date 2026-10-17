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

#define MTRR_DEF_TYPE_MASK	0xff
#define MTRR_DEF_TYPE_EN	(1 << 11)
#define MTRR_DEF_TYPE_FIX_EN	(1 << 10)

#define MTRR_PHYS_BASE_MSR(reg)	(0x200 + 2 * (reg))
#define MTRR_PHYS_MASK_MSR(reg)	(0x200 + 2 * (reg) + 1)

#define MTRR_PHYS_MASK_VALID	(1 << 11)

#define MTRR_BASE_TYPE_MASK	0x7

/* Maximum number of MTRRs supported - see also mtrr_get_var_count() */
#define MTRR_MAX_COUNT		10

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

#if !defined(__ASSEMBLY__)

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
 * struct mtrr - Information about a single MTRR
 *
 * @base: Base address and MTRR_BASE_TYPE_MASK
 * @mask: Mask and MTRR_PHYS_MASK_VALID
 */
struct mtrr {
	u64 base;
	u64 mask;
};

/**
 * struct mtrr_info - Information about all MTRRs
 *
 * @mtrr: Information about each mtrr
 */
struct mtrr_info {
	struct mtrr mtrr[MTRR_MAX_COUNT];
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
 * mtrr_close() - Clean up after adjusting MTRRs, and enable them
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
 * @size:	Size, must be power of 2
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

/**
 * mtrr_set_next_var() - set up a variable MTRR
 *
 * This finds the first free variable MTRR and sets to the given area
 *
 * @type:	Requested type (MTRR_TYPE_)
 * @start:	Start address
 * @size:	Size, must be power of 2
 * Return: 0 on success, -EINVAL if size is not power of 2,
 * -ENOSPC if there are no more MTRRs
 */
int mtrr_set_next_var(uint type, uint64_t base, uint64_t size);

/**
 * mtrr_read_all() - Save all the MTRRs
 *
 * This reads all MTRRs from the boot CPU into a struct so they can be loaded
 * onto other CPUs
 *
 * @info: Place to put the MTRR info
 */
void mtrr_read_all(struct mtrr_info *info);

/**
 * mtrr_set_valid() - Set the valid flag for a selected MTRR and CPU(s)
 *
 * @cpu_select: Selected CPUs (either a CPU number or MP_SELECT_...)
 * @reg: MTRR register to write (0-7)
 * @valid: Valid flag to write
 * Return: 0 on success, -ve on error
 */
int mtrr_set_valid(int cpu_select, int reg, bool valid);

/**
 * mtrr_set() - Set the base address and mask for a selected MTRR and CPU(s)
 *
 * @cpu_select: Selected CPUs (either a CPU number or MP_SELECT_...)
 * @reg: MTRR register to write (0-7)
 * @base: Base address and MTRR_BASE_TYPE_MASK
 * @mask: Mask and MTRR_PHYS_MASK_VALID
 * Return: 0 on success, -ve on error
 */
int mtrr_set(int cpu_select, int reg, u64 base, u64 mask);

/**
 * mtrr_get_var_count() - Get the number of variable MTRRs
 *
 * Some CPUs have more than 8 MTRRs. This function returns the actual number
 *
 * Return: number of variable MTRRs
 */
int mtrr_get_var_count(void);

/**
 * mtrr_list() - List the MTRRs
 *
 * Shows a list of all the MTRRs including their values
 *
 * @reg_count: Number of registers to show. You can use mtrr_get_var_count() for
 * this
 * @cpu_select: CPU to use. Use MP_SELECT_BSP for the boot CPU
 * Returns: 0 if OK, -ve if the CPU was not found
 */
int mtrr_list(int reg_count, int cpu_select);

/**
 * mtrr_get_type_by_name() - Get the type of an MTRR given its type name
 *
 * @typename: Name to check
 * Returns: MTRR type (MTRR_TYPE_...) or -EINVAL if invalid
 */
int mtrr_get_type_by_name(const char *typename);

#endif

#if ((CONFIG_XIP_ROM_SIZE & (CONFIG_XIP_ROM_SIZE - 1)) != 0)
# error "CONFIG_XIP_ROM_SIZE is not a power of 2"
#endif

#if ((CONFIG_CACHE_ROM_SIZE & (CONFIG_CACHE_ROM_SIZE - 1)) != 0)
# error "CONFIG_CACHE_ROM_SIZE is not a power of 2"
#endif

#define CACHE_ROM_BASE	(((1 << 20) - (CONFIG_CACHE_ROM_SIZE >> 12)) << 12)

#endif
