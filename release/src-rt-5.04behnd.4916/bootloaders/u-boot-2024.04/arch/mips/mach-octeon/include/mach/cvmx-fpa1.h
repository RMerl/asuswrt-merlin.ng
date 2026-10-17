/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Interface to the hardware Free Pool Allocator on Octeon chips.
 * These are the legacy models, i.e. prior to CN78XX/CN76XX.
 */

#ifndef __CVMX_FPA1_HW_H__
#define __CVMX_FPA1_HW_H__

#include "cvmx-scratch.h"
#include "cvmx-fpa-defs.h"
#include "cvmx-fpa3.h"

/* Legacy pool range is 0..7 and 8 on CN68XX */
typedef int cvmx_fpa1_pool_t;

#define CVMX_FPA1_NUM_POOLS    8
#define CVMX_FPA1_INVALID_POOL ((cvmx_fpa1_pool_t)-1)
#define CVMX_FPA1_NAME_SIZE    16

/**
 * Structure describing the data format used for stores to the FPA.
 */
typedef union {
	u64 u64;
	struct {
		u64 scraddr : 8;
		u64 len : 8;
		u64 did : 8;
		u64 addr : 40;
	} s;
} cvmx_fpa1_iobdma_data_t;

/*
 * Allocate or reserve the specified fpa pool.
 *
 * @param pool	  FPA pool to allocate/reserve. If -1 it
 *                finds an empty pool to allocate.
 * Return:        Alloctaed pool number or CVMX_FPA1_POOL_INVALID
 *                if fails to allocate the pool
 */
cvmx_fpa1_pool_t cvmx_fpa1_reserve_pool(cvmx_fpa1_pool_t pool);

/**
 * Free the specified fpa pool.
 * @param pool	   Pool to free
 * Return:         0 for success -1 failure
 */
int cvmx_fpa1_release_pool(cvmx_fpa1_pool_t pool);

static inline void cvmx_fpa1_free(void *ptr, cvmx_fpa1_pool_t pool, u64 num_cache_lines)
{
	cvmx_addr_t newptr;

	newptr.u64 = cvmx_ptr_to_phys(ptr);
	newptr.sfilldidspace.didspace = CVMX_ADDR_DIDSPACE(CVMX_FULL_DID(CVMX_OCT_DID_FPA, pool));
	/* Make sure that any previous writes to memory go out before we free
	 * this buffer.  This also serves as a barrier to prevent GCC from
	 * reordering operations to after the free.
	 */
	CVMX_SYNCWS;
	/* value written is number of cache lines not written back */
	cvmx_write_io(newptr.u64, num_cache_lines);
}

static inline void cvmx_fpa1_free_nosync(void *ptr, cvmx_fpa1_pool_t pool,
					 unsigned int num_cache_lines)
{
	cvmx_addr_t newptr;

	newptr.u64 = cvmx_ptr_to_phys(ptr);
	newptr.sfilldidspace.didspace = CVMX_ADDR_DIDSPACE(CVMX_FULL_DID(CVMX_OCT_DID_FPA, pool));
	/* Prevent GCC from reordering around free */
	asm volatile("" : : : "memory");
	/* value written is number of cache lines not written back */
	cvmx_write_io(newptr.u64, num_cache_lines);
}

/**
 * Enable the FPA for use. Must be performed after any CSR
 * configuration but before any other FPA functions.
 */
static inline void cvmx_fpa1_enable(void)
{
	cvmx_fpa_ctl_status_t status;

	status.u64 = csr_rd(CVMX_FPA_CTL_STATUS);
	if (status.s.enb) {
		/*
		 * CN68XXP1 should not reset the FPA (doing so may break
		 * the SSO, so we may end up enabling it more than once.
		 * Just return and don't spew messages.
		 */
		return;
	}

	status.u64 = 0;
	status.s.enb = 1;
	csr_wr(CVMX_FPA_CTL_STATUS, status.u64);
}

/**
 * Reset FPA to disable. Make sure buffers from all FPA pools are freed
 * before disabling FPA.
 */
static inline void cvmx_fpa1_disable(void)
{
	cvmx_fpa_ctl_status_t status;

	if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS1))
		return;

	status.u64 = csr_rd(CVMX_FPA_CTL_STATUS);
	status.s.reset = 1;
	csr_wr(CVMX_FPA_CTL_STATUS, status.u64);
}

static inline void *cvmx_fpa1_alloc(cvmx_fpa1_pool_t pool)
{
	u64 address;

	for (;;) {
		address = csr_rd(CVMX_ADDR_DID(CVMX_FULL_DID(CVMX_OCT_DID_FPA, pool)));
		if (cvmx_likely(address)) {
			return cvmx_phys_to_ptr(address);
		} else {
			if (csr_rd(CVMX_FPA_QUEX_AVAILABLE(pool)) > 0)
				udelay(50);
			else
				return NULL;
		}
	}
}

/**
 * Asynchronously get a new block from the FPA
 * @INTERNAL
 *
 * The result of cvmx_fpa_async_alloc() may be retrieved using
 * cvmx_fpa_async_alloc_finish().
 *
 * @param scr_addr Local scratch address to put response in.  This is a byte
 *		   address but must be 8 byte aligned.
 * @param pool      Pool to get the block from
 */
static inline void cvmx_fpa1_async_alloc(u64 scr_addr, cvmx_fpa1_pool_t pool)
{
	cvmx_fpa1_iobdma_data_t data;

	/* Hardware only uses 64 bit aligned locations, so convert from byte
	 * address to 64-bit index
	 */
	data.u64 = 0ull;
	data.s.scraddr = scr_addr >> 3;
	data.s.len = 1;
	data.s.did = CVMX_FULL_DID(CVMX_OCT_DID_FPA, pool);
	data.s.addr = 0;

	cvmx_scratch_write64(scr_addr, 0ull);
	CVMX_SYNCW;
	cvmx_send_single(data.u64);
}

/**
 * Retrieve the result of cvmx_fpa_async_alloc
 * @INTERNAL
 *
 * @param scr_addr The Local scratch address.  Must be the same value
 * passed to cvmx_fpa_async_alloc().
 *
 * @param pool Pool the block came from.  Must be the same value
 * passed to cvmx_fpa_async_alloc.
 *
 * Return: Pointer to the block or NULL on failure
 */
static inline void *cvmx_fpa1_async_alloc_finish(u64 scr_addr, cvmx_fpa1_pool_t pool)
{
	u64 address;

	CVMX_SYNCIOBDMA;

	address = cvmx_scratch_read64(scr_addr);
	if (cvmx_likely(address))
		return cvmx_phys_to_ptr(address);
	else
		return cvmx_fpa1_alloc(pool);
}

static inline u64 cvmx_fpa1_get_available(cvmx_fpa1_pool_t pool)
{
	return csr_rd(CVMX_FPA_QUEX_AVAILABLE(pool));
}

#endif /* __CVMX_FPA1_HW_H__ */
