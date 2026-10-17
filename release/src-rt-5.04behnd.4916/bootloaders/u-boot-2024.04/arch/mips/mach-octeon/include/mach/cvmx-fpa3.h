/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Interface to the CN78XX Free Pool Allocator, a.k.a. FPA3
 */

#include "cvmx-address.h"
#include "cvmx-fpa-defs.h"
#include "cvmx-scratch.h"

#ifndef __CVMX_FPA3_H__
#define __CVMX_FPA3_H__

typedef struct {
	unsigned res0 : 6;
	unsigned node : 2;
	unsigned res1 : 2;
	unsigned lpool : 6;
	unsigned valid_magic : 16;
} cvmx_fpa3_pool_t;

typedef struct {
	unsigned res0 : 6;
	unsigned node : 2;
	unsigned res1 : 6;
	unsigned laura : 10;
	unsigned valid_magic : 16;
} cvmx_fpa3_gaura_t;

#define CVMX_FPA3_VALID_MAGIC	0xf9a3
#define CVMX_FPA3_INVALID_GAURA ((cvmx_fpa3_gaura_t){ 0, 0, 0, 0, 0 })
#define CVMX_FPA3_INVALID_POOL	((cvmx_fpa3_pool_t){ 0, 0, 0, 0, 0 })

static inline bool __cvmx_fpa3_aura_valid(cvmx_fpa3_gaura_t aura)
{
	if (aura.valid_magic != CVMX_FPA3_VALID_MAGIC)
		return false;
	return true;
}

static inline bool __cvmx_fpa3_pool_valid(cvmx_fpa3_pool_t pool)
{
	if (pool.valid_magic != CVMX_FPA3_VALID_MAGIC)
		return false;
	return true;
}

static inline cvmx_fpa3_gaura_t __cvmx_fpa3_gaura(int node, int laura)
{
	cvmx_fpa3_gaura_t aura;

	if (node < 0)
		node = cvmx_get_node_num();
	if (laura < 0)
		return CVMX_FPA3_INVALID_GAURA;

	aura.node = node;
	aura.laura = laura;
	aura.valid_magic = CVMX_FPA3_VALID_MAGIC;
	return aura;
}

static inline cvmx_fpa3_pool_t __cvmx_fpa3_pool(int node, int lpool)
{
	cvmx_fpa3_pool_t pool;

	if (node < 0)
		node = cvmx_get_node_num();
	if (lpool < 0)
		return CVMX_FPA3_INVALID_POOL;

	pool.node = node;
	pool.lpool = lpool;
	pool.valid_magic = CVMX_FPA3_VALID_MAGIC;
	return pool;
}

#undef CVMX_FPA3_VALID_MAGIC

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
	struct {
		u64 scraddr : 8;
		u64 len : 8;
		u64 did : 8;
		u64 node : 4;
		u64 red : 1;
		u64 reserved2 : 9;
		u64 aura : 10;
		u64 reserved3 : 16;
	} cn78xx;
} cvmx_fpa3_iobdma_data_t;

/**
 * Struct describing load allocate operation addresses for FPA pool.
 */
union cvmx_fpa3_load_data {
	u64 u64;
	struct {
		u64 seg : 2;
		u64 reserved1 : 13;
		u64 io : 1;
		u64 did : 8;
		u64 node : 4;
		u64 red : 1;
		u64 reserved2 : 9;
		u64 aura : 10;
		u64 reserved3 : 16;
	};
};

typedef union cvmx_fpa3_load_data cvmx_fpa3_load_data_t;

/**
 * Struct describing store free operation addresses from FPA pool.
 */
union cvmx_fpa3_store_addr {
	u64 u64;
	struct {
		u64 seg : 2;
		u64 reserved1 : 13;
		u64 io : 1;
		u64 did : 8;
		u64 node : 4;
		u64 reserved2 : 10;
		u64 aura : 10;
		u64 fabs : 1;
		u64 reserved3 : 3;
		u64 dwb_count : 9;
		u64 reserved4 : 3;
	};
};

typedef union cvmx_fpa3_store_addr cvmx_fpa3_store_addr_t;

enum cvmx_fpa3_pool_alignment_e {
	FPA_NATURAL_ALIGNMENT,
	FPA_OFFSET_ALIGNMENT,
	FPA_OPAQUE_ALIGNMENT
};

#define CVMX_FPA3_AURAX_LIMIT_MAX ((1ull << 40) - 1)

/**
 * @INTERNAL
 * Accessor functions to return number of POOLS in an FPA3
 * depending on SoC model.
 * The number is per-node for models supporting multi-node configurations.
 */
static inline int cvmx_fpa3_num_pools(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		return 64;
	if (OCTEON_IS_MODEL(OCTEON_CNF75XX))
		return 32;
	if (OCTEON_IS_MODEL(OCTEON_CN73XX))
		return 32;
	printf("ERROR: %s: Unknowm model\n", __func__);
	return -1;
}

/**
 * @INTERNAL
 * Accessor functions to return number of AURAS in an FPA3
 * depending on SoC model.
 * The number is per-node for models supporting multi-node configurations.
 */
static inline int cvmx_fpa3_num_auras(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		return 1024;
	if (OCTEON_IS_MODEL(OCTEON_CNF75XX))
		return 512;
	if (OCTEON_IS_MODEL(OCTEON_CN73XX))
		return 512;
	printf("ERROR: %s: Unknowm model\n", __func__);
	return -1;
}

/**
 * Get the FPA3 POOL underneath FPA3 AURA, containing all its buffers
 *
 */
static inline cvmx_fpa3_pool_t cvmx_fpa3_aura_to_pool(cvmx_fpa3_gaura_t aura)
{
	cvmx_fpa3_pool_t pool;
	cvmx_fpa_aurax_pool_t aurax_pool;

	aurax_pool.u64 = cvmx_read_csr_node(aura.node, CVMX_FPA_AURAX_POOL(aura.laura));

	pool = __cvmx_fpa3_pool(aura.node, aurax_pool.s.pool);
	return pool;
}

/**
 * Get a new block from the FPA pool
 *
 * @param aura  - aura number
 * Return: pointer to the block or NULL on failure
 */
static inline void *cvmx_fpa3_alloc(cvmx_fpa3_gaura_t aura)
{
	u64 address;
	cvmx_fpa3_load_data_t load_addr;

	load_addr.u64 = 0;
	load_addr.seg = CVMX_MIPS_SPACE_XKPHYS;
	load_addr.io = 1;
	load_addr.did = 0x29; /* Device ID. Indicates FPA. */
	load_addr.node = aura.node;
	load_addr.red = 0; /* Perform RED on allocation.
				  * FIXME to use config option
				  */
	load_addr.aura = aura.laura;

	address = cvmx_read64_uint64(load_addr.u64);
	if (!address)
		return NULL;
	return cvmx_phys_to_ptr(address);
}

/**
 * Asynchronously get a new block from the FPA
 *
 * The result of cvmx_fpa_async_alloc() may be retrieved using
 * cvmx_fpa_async_alloc_finish().
 *
 * @param scr_addr Local scratch address to put response in.  This is a byte
 *		   address but must be 8 byte aligned.
 * @param aura     Global aura to get the block from
 */
static inline void cvmx_fpa3_async_alloc(u64 scr_addr, cvmx_fpa3_gaura_t aura)
{
	cvmx_fpa3_iobdma_data_t data;

	/* Hardware only uses 64 bit aligned locations, so convert from byte
	 * address to 64-bit index
	 */
	data.u64 = 0ull;
	data.cn78xx.scraddr = scr_addr >> 3;
	data.cn78xx.len = 1;
	data.cn78xx.did = 0x29;
	data.cn78xx.node = aura.node;
	data.cn78xx.aura = aura.laura;
	cvmx_scratch_write64(scr_addr, 0ull);

	CVMX_SYNCW;
	cvmx_send_single(data.u64);
}

/**
 * Retrieve the result of cvmx_fpa3_async_alloc
 *
 * @param scr_addr The Local scratch address.  Must be the same value
 * passed to cvmx_fpa_async_alloc().
 *
 * @param aura Global aura the block came from.  Must be the same value
 * passed to cvmx_fpa_async_alloc.
 *
 * Return: Pointer to the block or NULL on failure
 */
static inline void *cvmx_fpa3_async_alloc_finish(u64 scr_addr, cvmx_fpa3_gaura_t aura)
{
	u64 address;

	CVMX_SYNCIOBDMA;

	address = cvmx_scratch_read64(scr_addr);
	if (cvmx_likely(address))
		return cvmx_phys_to_ptr(address);
	else
		/* Try regular alloc if async failed */
		return cvmx_fpa3_alloc(aura);
}

/**
 * Free a pointer back to the pool.
 *
 * @param aura   global aura number
 * @param ptr    physical address of block to free.
 * @param num_cache_lines Cache lines to invalidate
 */
static inline void cvmx_fpa3_free(void *ptr, cvmx_fpa3_gaura_t aura, unsigned int num_cache_lines)
{
	cvmx_fpa3_store_addr_t newptr;
	cvmx_addr_t newdata;

	newdata.u64 = cvmx_ptr_to_phys(ptr);

	/* Make sure that any previous writes to memory go out before we free
	   this buffer. This also serves as a barrier to prevent GCC from
	   reordering operations to after the free. */
	CVMX_SYNCWS;

	newptr.u64 = 0;
	newptr.seg = CVMX_MIPS_SPACE_XKPHYS;
	newptr.io = 1;
	newptr.did = 0x29; /* Device id, indicates FPA */
	newptr.node = aura.node;
	newptr.aura = aura.laura;
	newptr.fabs = 0; /* Free absolute. FIXME to use config option */
	newptr.dwb_count = num_cache_lines;

	cvmx_write_io(newptr.u64, newdata.u64);
}

/**
 * Free a pointer back to the pool without flushing the write buffer.
 *
 * @param aura   global aura number
 * @param ptr    physical address of block to free.
 * @param num_cache_lines Cache lines to invalidate
 */
static inline void cvmx_fpa3_free_nosync(void *ptr, cvmx_fpa3_gaura_t aura,
					 unsigned int num_cache_lines)
{
	cvmx_fpa3_store_addr_t newptr;
	cvmx_addr_t newdata;

	newdata.u64 = cvmx_ptr_to_phys(ptr);

	/* Prevent GCC from reordering writes to (*ptr) */
	asm volatile("" : : : "memory");

	newptr.u64 = 0;
	newptr.seg = CVMX_MIPS_SPACE_XKPHYS;
	newptr.io = 1;
	newptr.did = 0x29; /* Device id, indicates FPA */
	newptr.node = aura.node;
	newptr.aura = aura.laura;
	newptr.fabs = 0; /* Free absolute. FIXME to use config option */
	newptr.dwb_count = num_cache_lines;

	cvmx_write_io(newptr.u64, newdata.u64);
}

static inline int cvmx_fpa3_pool_is_enabled(cvmx_fpa3_pool_t pool)
{
	cvmx_fpa_poolx_cfg_t pool_cfg;

	if (!__cvmx_fpa3_pool_valid(pool))
		return -1;

	pool_cfg.u64 = cvmx_read_csr_node(pool.node, CVMX_FPA_POOLX_CFG(pool.lpool));
	return pool_cfg.cn78xx.ena;
}

static inline int cvmx_fpa3_config_red_params(unsigned int node, int qos_avg_en, int red_lvl_dly,
					      int avg_dly)
{
	cvmx_fpa_gen_cfg_t fpa_cfg;
	cvmx_fpa_red_delay_t red_delay;

	fpa_cfg.u64 = cvmx_read_csr_node(node, CVMX_FPA_GEN_CFG);
	fpa_cfg.s.avg_en = qos_avg_en;
	fpa_cfg.s.lvl_dly = red_lvl_dly;
	cvmx_write_csr_node(node, CVMX_FPA_GEN_CFG, fpa_cfg.u64);

	red_delay.u64 = cvmx_read_csr_node(node, CVMX_FPA_RED_DELAY);
	red_delay.s.avg_dly = avg_dly;
	cvmx_write_csr_node(node, CVMX_FPA_RED_DELAY, red_delay.u64);
	return 0;
}

/**
 * Gets the buffer size of the specified pool,
 *
 * @param aura Global aura number
 * Return: Returns size of the buffers in the specified pool.
 */
static inline int cvmx_fpa3_get_aura_buf_size(cvmx_fpa3_gaura_t aura)
{
	cvmx_fpa3_pool_t pool;
	cvmx_fpa_poolx_cfg_t pool_cfg;
	int block_size;

	pool = cvmx_fpa3_aura_to_pool(aura);

	pool_cfg.u64 = cvmx_read_csr_node(pool.node, CVMX_FPA_POOLX_CFG(pool.lpool));
	block_size = pool_cfg.cn78xx.buf_size << 7;
	return block_size;
}

/**
 * Return the number of available buffers in an AURA
 *
 * @param aura to receive count for
 * Return: available buffer count
 */
static inline long long cvmx_fpa3_get_available(cvmx_fpa3_gaura_t aura)
{
	cvmx_fpa3_pool_t pool;
	cvmx_fpa_poolx_available_t avail_reg;
	cvmx_fpa_aurax_cnt_t cnt_reg;
	cvmx_fpa_aurax_cnt_limit_t limit_reg;
	long long ret;

	pool = cvmx_fpa3_aura_to_pool(aura);

	/* Get POOL available buffer count */
	avail_reg.u64 = cvmx_read_csr_node(pool.node, CVMX_FPA_POOLX_AVAILABLE(pool.lpool));

	/* Get AURA current available count */
	cnt_reg.u64 = cvmx_read_csr_node(aura.node, CVMX_FPA_AURAX_CNT(aura.laura));
	limit_reg.u64 = cvmx_read_csr_node(aura.node, CVMX_FPA_AURAX_CNT_LIMIT(aura.laura));

	if (limit_reg.cn78xx.limit < cnt_reg.cn78xx.cnt)
		return 0;

	/* Calculate AURA-based buffer allowance */
	ret = limit_reg.cn78xx.limit - cnt_reg.cn78xx.cnt;

	/* Use POOL real buffer availability when less then allowance */
	if (ret > (long long)avail_reg.cn78xx.count)
		ret = avail_reg.cn78xx.count;

	return ret;
}

/**
 * Configure the QoS parameters of an FPA3 AURA
 *
 * @param aura is the FPA3 AURA handle
 * @param ena_bp enables backpressure when outstanding count exceeds 'bp_thresh'
 * @param ena_red enables random early discard when outstanding count exceeds 'pass_thresh'
 * @param pass_thresh is the maximum count to invoke flow control
 * @param drop_thresh is the count threshold to begin dropping packets
 * @param bp_thresh is the back-pressure threshold
 *
 */
static inline void cvmx_fpa3_setup_aura_qos(cvmx_fpa3_gaura_t aura, bool ena_red, u64 pass_thresh,
					    u64 drop_thresh, bool ena_bp, u64 bp_thresh)
{
	unsigned int shift = 0;
	u64 shift_thresh;
	cvmx_fpa_aurax_cnt_limit_t limit_reg;
	cvmx_fpa_aurax_cnt_levels_t aura_level;

	if (!__cvmx_fpa3_aura_valid(aura))
		return;

	/* Get AURAX count limit for validation */
	limit_reg.u64 = cvmx_read_csr_node(aura.node, CVMX_FPA_AURAX_CNT_LIMIT(aura.laura));

	if (pass_thresh < 256)
		pass_thresh = 255;

	if (drop_thresh <= pass_thresh || drop_thresh > limit_reg.cn78xx.limit)
		drop_thresh = limit_reg.cn78xx.limit;

	if (bp_thresh < 256 || bp_thresh > limit_reg.cn78xx.limit)
		bp_thresh = limit_reg.cn78xx.limit >> 1;

	shift_thresh = (bp_thresh > drop_thresh) ? bp_thresh : drop_thresh;

	/* Calculate shift so that the largest threshold fits in 8 bits */
	for (shift = 0; shift < (1 << 6); shift++) {
		if (0 == ((shift_thresh >> shift) & ~0xffull))
			break;
	};

	aura_level.u64 = cvmx_read_csr_node(aura.node, CVMX_FPA_AURAX_CNT_LEVELS(aura.laura));
	aura_level.s.pass = pass_thresh >> shift;
	aura_level.s.drop = drop_thresh >> shift;
	aura_level.s.bp = bp_thresh >> shift;
	aura_level.s.shift = shift;
	aura_level.s.red_ena = ena_red;
	aura_level.s.bp_ena = ena_bp;
	cvmx_write_csr_node(aura.node, CVMX_FPA_AURAX_CNT_LEVELS(aura.laura), aura_level.u64);
}

cvmx_fpa3_gaura_t cvmx_fpa3_reserve_aura(int node, int desired_aura_num);
int cvmx_fpa3_release_aura(cvmx_fpa3_gaura_t aura);
cvmx_fpa3_pool_t cvmx_fpa3_reserve_pool(int node, int desired_pool_num);
int cvmx_fpa3_release_pool(cvmx_fpa3_pool_t pool);
int cvmx_fpa3_is_aura_available(int node, int aura_num);
int cvmx_fpa3_is_pool_available(int node, int pool_num);

cvmx_fpa3_pool_t cvmx_fpa3_setup_fill_pool(int node, int desired_pool, const char *name,
					   unsigned int block_size, unsigned int num_blocks,
					   void *buffer);

/**
 * Function to attach an aura to an existing pool
 *
 * @param node - configure fpa on this node
 * @param pool - configured pool to attach aura to
 * @param desired_aura - pointer to aura to use, set to -1 to allocate
 * @param name - name to register
 * @param block_size - size of buffers to use
 * @param num_blocks - number of blocks to allocate
 *
 * Return: configured gaura on success, CVMX_FPA3_INVALID_GAURA on failure
 */
cvmx_fpa3_gaura_t cvmx_fpa3_set_aura_for_pool(cvmx_fpa3_pool_t pool, int desired_aura,
					      const char *name, unsigned int block_size,
					      unsigned int num_blocks);

/**
 * Function to setup and initialize a pool.
 *
 * @param node - configure fpa on this node
 * @param desired_aura - aura to use, -1 for dynamic allocation
 * @param name - name to register
 * @param block_size - size of buffers in pool
 * @param num_blocks - max number of buffers allowed
 */
cvmx_fpa3_gaura_t cvmx_fpa3_setup_aura_and_pool(int node, int desired_aura, const char *name,
						void *buffer, unsigned int block_size,
						unsigned int num_blocks);

int cvmx_fpa3_shutdown_aura_and_pool(cvmx_fpa3_gaura_t aura);
int cvmx_fpa3_shutdown_aura(cvmx_fpa3_gaura_t aura);
int cvmx_fpa3_shutdown_pool(cvmx_fpa3_pool_t pool);
const char *cvmx_fpa3_get_pool_name(cvmx_fpa3_pool_t pool);
int cvmx_fpa3_get_pool_buf_size(cvmx_fpa3_pool_t pool);
const char *cvmx_fpa3_get_aura_name(cvmx_fpa3_gaura_t aura);

#endif /* __CVMX_FPA3_H__ */
