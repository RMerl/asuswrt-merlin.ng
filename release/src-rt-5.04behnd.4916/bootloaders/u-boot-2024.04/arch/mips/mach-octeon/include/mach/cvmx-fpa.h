/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Interface to the hardware Free Pool Allocator.
 */

#ifndef __CVMX_FPA_H__
#define __CVMX_FPA_H__

#include "cvmx-scratch.h"
#include "cvmx-fpa-defs.h"
#include "cvmx-fpa1.h"
#include "cvmx-fpa3.h"

#define CVMX_FPA_MIN_BLOCK_SIZE 128
#define CVMX_FPA_ALIGNMENT	128
#define CVMX_FPA_POOL_NAME_LEN	16

/* On CN78XX in backward-compatible mode, pool is mapped to AURA */
#define CVMX_FPA_NUM_POOLS                                                                         \
	(octeon_has_feature(OCTEON_FEATURE_FPA3) ? cvmx_fpa3_num_auras() : CVMX_FPA1_NUM_POOLS)

/**
 * Structure to store FPA pool configuration parameters.
 */
struct cvmx_fpa_pool_config {
	s64 pool_num;
	u64 buffer_size;
	u64 buffer_count;
};

typedef struct cvmx_fpa_pool_config cvmx_fpa_pool_config_t;

/**
 * Return the name of the pool
 *
 * @param pool_num   Pool to get the name of
 * Return: The name
 */
const char *cvmx_fpa_get_name(int pool_num);

/**
 * Initialize FPA per node
 */
int cvmx_fpa_global_init_node(int node);

/**
 * Enable the FPA
 */
static inline void cvmx_fpa_enable(void)
{
	if (!octeon_has_feature(OCTEON_FEATURE_FPA3))
		cvmx_fpa1_enable();
	else
		cvmx_fpa_global_init_node(cvmx_get_node_num());
}

/**
 * Disable the FPA
 */
static inline void cvmx_fpa_disable(void)
{
	if (!octeon_has_feature(OCTEON_FEATURE_FPA3))
		cvmx_fpa1_disable();
	/* FPA3 does not have a disable function */
}

/**
 * @INTERNAL
 * @deprecated OBSOLETE
 *
 * Kept for transition assistance only
 */
static inline void cvmx_fpa_global_initialize(void)
{
	cvmx_fpa_global_init_node(cvmx_get_node_num());
}

/**
 * @INTERNAL
 *
 * Convert FPA1 style POOL into FPA3 AURA in
 * backward compatibility mode.
 */
static inline cvmx_fpa3_gaura_t cvmx_fpa1_pool_to_fpa3_aura(cvmx_fpa1_pool_t pool)
{
	if ((octeon_has_feature(OCTEON_FEATURE_FPA3))) {
		unsigned int node = cvmx_get_node_num();
		cvmx_fpa3_gaura_t aura = __cvmx_fpa3_gaura(node, pool);
		return aura;
	}
	return CVMX_FPA3_INVALID_GAURA;
}

/**
 * Get a new block from the FPA
 *
 * @param pool   Pool to get the block from
 * Return: Pointer to the block or NULL on failure
 */
static inline void *cvmx_fpa_alloc(u64 pool)
{
	/* FPA3 is handled differently */
	if ((octeon_has_feature(OCTEON_FEATURE_FPA3))) {
		return cvmx_fpa3_alloc(cvmx_fpa1_pool_to_fpa3_aura(pool));
	} else {
		return cvmx_fpa1_alloc(pool);
	}
}

/**
 * Asynchronously get a new block from the FPA
 *
 * The result of cvmx_fpa_async_alloc() may be retrieved using
 * cvmx_fpa_async_alloc_finish().
 *
 * @param scr_addr Local scratch address to put response in.  This is a byte
 *		   address but must be 8 byte aligned.
 * @param pool      Pool to get the block from
 */
static inline void cvmx_fpa_async_alloc(u64 scr_addr, u64 pool)
{
	if ((octeon_has_feature(OCTEON_FEATURE_FPA3))) {
		return cvmx_fpa3_async_alloc(scr_addr, cvmx_fpa1_pool_to_fpa3_aura(pool));
	} else
		return cvmx_fpa1_async_alloc(scr_addr, pool);
}

/**
 * Retrieve the result of cvmx_fpa_async_alloc
 *
 * @param scr_addr The Local scratch address.  Must be the same value
 * passed to cvmx_fpa_async_alloc().
 *
 * @param pool Pool the block came from.  Must be the same value
 * passed to cvmx_fpa_async_alloc.
 *
 * Return: Pointer to the block or NULL on failure
 */
static inline void *cvmx_fpa_async_alloc_finish(u64 scr_addr, u64 pool)
{
	if ((octeon_has_feature(OCTEON_FEATURE_FPA3)))
		return cvmx_fpa3_async_alloc_finish(scr_addr, cvmx_fpa1_pool_to_fpa3_aura(pool));
	else
		return cvmx_fpa1_async_alloc_finish(scr_addr, pool);
}

/**
 * Free a block allocated with a FPA pool.
 * Does NOT provide memory ordering in cases where the memory block was
 * modified by the core.
 *
 * @param ptr    Block to free
 * @param pool   Pool to put it in
 * @param num_cache_lines
 *               Cache lines to invalidate
 */
static inline void cvmx_fpa_free_nosync(void *ptr, u64 pool, u64 num_cache_lines)
{
	/* FPA3 is handled differently */
	if ((octeon_has_feature(OCTEON_FEATURE_FPA3)))
		cvmx_fpa3_free_nosync(ptr, cvmx_fpa1_pool_to_fpa3_aura(pool), num_cache_lines);
	else
		cvmx_fpa1_free_nosync(ptr, pool, num_cache_lines);
}

/**
 * Free a block allocated with a FPA pool.  Provides required memory
 * ordering in cases where memory block was modified by core.
 *
 * @param ptr    Block to free
 * @param pool   Pool to put it in
 * @param num_cache_lines
 *               Cache lines to invalidate
 */
static inline void cvmx_fpa_free(void *ptr, u64 pool, u64 num_cache_lines)
{
	if ((octeon_has_feature(OCTEON_FEATURE_FPA3)))
		cvmx_fpa3_free(ptr, cvmx_fpa1_pool_to_fpa3_aura(pool), num_cache_lines);
	else
		cvmx_fpa1_free(ptr, pool, num_cache_lines);
}

/**
 * Setup a FPA pool to control a new block of memory.
 * This can only be called once per pool. Make sure proper
 * locking enforces this.
 *
 * @param pool       Pool to initialize
 * @param name       Constant character string to name this pool.
 *                   String is not copied.
 * @param buffer     Pointer to the block of memory to use. This must be
 *                   accessible by all processors and external hardware.
 * @param block_size Size for each block controlled by the FPA
 * @param num_blocks Number of blocks
 *
 * Return: the pool number on Success,
 *         -1 on failure
 */
int cvmx_fpa_setup_pool(int pool, const char *name, void *buffer, u64 block_size, u64 num_blocks);

int cvmx_fpa_shutdown_pool(int pool);

/**
 * Gets the block size of buffer in specified pool
 * @param pool	 Pool to get the block size from
 * Return:       Size of buffer in specified pool
 */
unsigned int cvmx_fpa_get_block_size(int pool);

int cvmx_fpa_is_pool_available(int pool_num);
u64 cvmx_fpa_get_pool_owner(int pool_num);
int cvmx_fpa_get_max_pools(void);
int cvmx_fpa_get_current_count(int pool_num);
int cvmx_fpa_validate_pool(int pool);

#endif /*  __CVM_FPA_H__ */
