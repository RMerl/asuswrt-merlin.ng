// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Support library for the hardware Free Pool Allocator.
 */

#include <errno.h>
#include <log.h>
#include <time.h>
#include <linux/delay.h>

#include <mach/cvmx-regs.h>
#include <mach/cvmx-csr.h>
#include <mach/cvmx-bootmem.h>
#include <mach/octeon-model.h>
#include <mach/cvmx-fuse.h>
#include <mach/octeon-feature.h>
#include <mach/cvmx-qlm.h>
#include <mach/octeon_qlm.h>
#include <mach/cvmx-pcie.h>
#include <mach/cvmx-coremask.h>
#include <mach/cvmx-range.h>
#include <mach/cvmx-global-resources.h>

#include <mach/cvmx-agl-defs.h>
#include <mach/cvmx-bgxx-defs.h>
#include <mach/cvmx-ciu-defs.h>
#include <mach/cvmx-gmxx-defs.h>
#include <mach/cvmx-gserx-defs.h>
#include <mach/cvmx-ilk-defs.h>
#include <mach/cvmx-ipd-defs.h>
#include <mach/cvmx-pcsx-defs.h>
#include <mach/cvmx-pcsxx-defs.h>
#include <mach/cvmx-pki-defs.h>
#include <mach/cvmx-pko-defs.h>
#include <mach/cvmx-xcv-defs.h>

#include <mach/cvmx-hwpko.h>
#include <mach/cvmx-ilk.h>
#include <mach/cvmx-pki.h>
#include <mach/cvmx-pko3.h>
#include <mach/cvmx-pko3-queue.h>
#include <mach/cvmx-pko3-resources.h>

#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-cfg.h>

#include <mach/cvmx-helper-bgx.h>
#include <mach/cvmx-helper-cfg.h>
#include <mach/cvmx-helper-util.h>
#include <mach/cvmx-helper-pki.h>

static const int debug;

/* Due to suspected errata, we may not be able to let the FPA_AURAX_CNT
 * get too close to 0, to avoid a spurious wrap-around error
 */
const unsigned int __cvmx_fpa3_cnt_offset = 32;

/* For advanced checks, a guard-band is created around the internal
 * stack, to make sure the stack is not overwritten.
 */
const u64 magic_pattern = 0xbab4faced095f00d;
const unsigned int guard_band_size = 0 << 10; /* 1KiB default*/

#define CVMX_CACHE_LINE_SHIFT (7)

#define CVMX_FPA3_NAME_LEN (16)

typedef struct {
	char name[CVMX_FPA3_NAME_LEN];
	u64 stack_paddr; /* Internal stack storage */
	u64 bufs_paddr;	 /* Buffer pool base address */
	u64 stack_psize; /* Internal stack storage size */
	u64 bufs_psize;	 /* Buffer pool raw size */
	u64 buf_count;	 /* Number of buffer filled */
	u64 buf_size;	 /* Buffer size */
} cvmx_fpa3_poolx_info_t;

typedef struct {
	char name[CVMX_FPA3_NAME_LEN];
	unsigned int buf_size; /* Buffer size */
} cvmx_fpa3_aurax_info_t;

typedef struct {
	char name[CVMX_FPA1_NAME_SIZE];
	u64 size; /* Block size of pool buffers */
	u64 buffer_count;
	u64 base_paddr; /* Base physical addr */
			/* if buffer is allocated at initialization */
} cvmx_fpa1_pool_info_t;

/**
 * FPA1/FPA3 info structure is stored in a named block
 * that is allocated once and shared among applications.
 */
static cvmx_fpa1_pool_info_t *cvmx_fpa1_pool_info;
static cvmx_fpa3_poolx_info_t *cvmx_fpa3_pool_info[CVMX_MAX_NODES];
static cvmx_fpa3_aurax_info_t *cvmx_fpa3_aura_info[CVMX_MAX_NODES];

/**
 * Return the size of buffers held in a POOL
 *
 * @param pool is the POOL handle
 * @return buffer size in bytes
 *
 */
int cvmx_fpa3_get_pool_buf_size(cvmx_fpa3_pool_t pool)
{
	cvmx_fpa_poolx_cfg_t pool_cfg;

	if (!__cvmx_fpa3_pool_valid(pool))
		return -1;

	pool_cfg.u64 = csr_rd_node(pool.node, CVMX_FPA_POOLX_CFG(pool.lpool));
	return pool_cfg.cn78xx.buf_size << CVMX_CACHE_LINE_SHIFT;
}

/**
 * Return the size of buffers held in a buffer pool
 *
 * @param pool is the pool number
 *
 * This function will work with CN78XX models in backward-compatible mode
 */
unsigned int cvmx_fpa_get_block_size(int pool)
{
	if (octeon_has_feature(OCTEON_FEATURE_FPA3)) {
		return cvmx_fpa3_get_pool_buf_size(cvmx_fpa3_aura_to_pool(
			cvmx_fpa1_pool_to_fpa3_aura(pool)));
	} else {
		if ((unsigned int)pool >= CVMX_FPA1_NUM_POOLS)
			return 0;
		if (!cvmx_fpa1_pool_info)
			cvmx_fpa_global_init_node(0);
		return cvmx_fpa1_pool_info[pool].size;
	}
}

static void cvmx_fpa3_set_aura_name(cvmx_fpa3_gaura_t aura, const char *name)
{
	cvmx_fpa3_aurax_info_t *pinfo;

	pinfo = cvmx_fpa3_aura_info[aura.node];
	if (!pinfo)
		return;
	pinfo += aura.laura;
	memset(pinfo->name, 0, sizeof(pinfo->name));
	if (name)
		strlcpy(pinfo->name, name, sizeof(pinfo->name));
}

static void cvmx_fpa3_set_pool_name(cvmx_fpa3_pool_t pool, const char *name)
{
	cvmx_fpa3_poolx_info_t *pinfo;

	pinfo = cvmx_fpa3_pool_info[pool.node];
	if (!pinfo)
		return;
	pinfo += pool.lpool;
	memset(pinfo->name, 0, sizeof(pinfo->name));
	if (name)
		strlcpy(pinfo->name, name, sizeof(pinfo->name));
}

static void cvmx_fpa_set_name(int pool_num, const char *name)
{
	if (octeon_has_feature(OCTEON_FEATURE_FPA3)) {
		cvmx_fpa3_set_aura_name(cvmx_fpa1_pool_to_fpa3_aura(pool_num),
					name);
	} else {
		cvmx_fpa1_pool_info_t *pinfo;

		if ((unsigned int)pool_num >= CVMX_FPA1_NUM_POOLS)
			return;
		if (!cvmx_fpa1_pool_info)
			cvmx_fpa_global_init_node(0);
		pinfo = &cvmx_fpa1_pool_info[pool_num];
		memset(pinfo->name, 0, sizeof(pinfo->name));
		if (name)
			strlcpy(pinfo->name, name, sizeof(pinfo->name));
	}
}

static int cvmx_fpa3_aura_cfg(cvmx_fpa3_gaura_t aura, cvmx_fpa3_pool_t pool,
			      u64 limit, u64 threshold, int ptr_dis)
{
	cvmx_fpa3_aurax_info_t *pinfo;
	cvmx_fpa_aurax_cfg_t aura_cfg;
	cvmx_fpa_poolx_cfg_t pool_cfg;
	cvmx_fpa_aurax_cnt_t cnt_reg;
	cvmx_fpa_aurax_cnt_limit_t limit_reg;
	cvmx_fpa_aurax_cnt_threshold_t thresh_reg;
	cvmx_fpa_aurax_int_t int_reg;
	unsigned int block_size;

	if (debug)
		debug("%s: AURA %u:%u POOL %u:%u\n", __func__, aura.node,
		      aura.laura, pool.node, pool.lpool);

	if (aura.node != pool.node) {
		printf("ERROR: %s: AURA/POOL node mismatch\n", __func__);
		return -1;
	}

	if (!__cvmx_fpa3_aura_valid(aura)) {
		printf("ERROR: %s: AURA invalid\n", __func__);
		return -1;
	}

	if (!__cvmx_fpa3_pool_valid(pool)) {
		printf("ERROR: %s: POOL invalid\n", __func__);
		return -1;
	}

	/* Record POOL block size in AURA info entry */
	pool_cfg.u64 = csr_rd_node(pool.node, CVMX_FPA_POOLX_CFG(pool.lpool));

	block_size = pool_cfg.cn78xx.buf_size << 7;
	pinfo = cvmx_fpa3_aura_info[aura.node];
	if (!pinfo)
		return -1;
	pinfo += aura.laura;

	pinfo->buf_size = block_size;

	/* block_size should be >0 except for POOL=0 which is never enabled*/
	if (pool_cfg.cn78xx.ena && block_size == 0) {
		printf("ERROR; %s: POOL buf_size invalid\n", __func__);
		return -1;
	}

	/* Initialize AURA count, limit and threshold registers */
	cnt_reg.u64 = 0;
	cnt_reg.cn78xx.cnt = 0 + __cvmx_fpa3_cnt_offset;

	limit_reg.u64 = 0;
	limit_reg.cn78xx.limit = limit;
	/* Apply count offset, unless it cases a wrap-around */
	if ((limit + __cvmx_fpa3_cnt_offset) < CVMX_FPA3_AURAX_LIMIT_MAX)
		limit_reg.cn78xx.limit += __cvmx_fpa3_cnt_offset;

	thresh_reg.u64 = 0;
	thresh_reg.cn78xx.thresh = threshold;
	/* Apply count offset, unless it cases a wrap-around */
	if ((threshold + __cvmx_fpa3_cnt_offset) < CVMX_FPA3_AURAX_LIMIT_MAX)
		thresh_reg.cn78xx.thresh += __cvmx_fpa3_cnt_offset;

	csr_wr_node(aura.node, CVMX_FPA_AURAX_CNT(aura.laura), cnt_reg.u64);
	csr_wr_node(aura.node, CVMX_FPA_AURAX_CNT_LIMIT(aura.laura),
		    limit_reg.u64);
	csr_wr_node(aura.node, CVMX_FPA_AURAX_CNT_THRESHOLD(aura.laura),
		    thresh_reg.u64);

	/* Clear any pending error interrupts */
	int_reg.u64 = 0;
	int_reg.cn78xx.thresh = 1;

	/* Follow a write to clear FPA_AURAX_INT[THRESH] with a read as
	 * a workaround to Errata FPA-23410. If FPA_AURAX_INT[THRESH]
	 * isn't clear, try again.
	 */
	do {
		csr_wr_node(aura.node, CVMX_FPA_AURAX_INT(aura.laura),
			    int_reg.u64);
		int_reg.u64 =
			csr_rd_node(aura.node, CVMX_FPA_AURAX_INT(aura.laura));
	} while (int_reg.s.thresh);

	/* Disable backpressure etc.*/
	csr_wr_node(aura.node, CVMX_FPA_AURAX_CNT_LEVELS(aura.laura), 0);
	csr_wr_node(aura.node, CVMX_FPA_AURAX_POOL_LEVELS(aura.laura), 0);

	aura_cfg.u64 = 0;
	aura_cfg.s.ptr_dis = ptr_dis;
	csr_wr_node(aura.node, CVMX_FPA_AURAX_CFG(aura.laura), aura_cfg.u64);
	csr_wr_node(aura.node, CVMX_FPA_AURAX_POOL(aura.laura), pool.lpool);

	return 0;
}

/**
 * @INTERNAL
 *
 * Fill a newly created FPA3 POOL with buffers
 * using a temporary AURA.
 */
static int cvmx_fpa3_pool_populate(cvmx_fpa3_pool_t pool, unsigned int buf_cnt,
				   unsigned int buf_sz, void *mem_ptr,
				   unsigned int mem_node)
{
	cvmx_fpa3_poolx_info_t *pinfo;
	cvmx_fpa3_gaura_t aura;
	cvmx_fpa3_pool_t zero_pool;
	cvmx_fpa_poolx_cfg_t pool_cfg;
	cvmx_fpa_poolx_start_addr_t pool_start_reg;
	cvmx_fpa_poolx_end_addr_t pool_end_reg;
	cvmx_fpa_poolx_available_t avail_reg;
	cvmx_fpa_poolx_threshold_t thresh_reg;
	cvmx_fpa_poolx_int_t int_reg;
	unsigned int block_size, align;
	unsigned long long mem_size;
	u64 paddr;
	unsigned int i;

	if (debug)
		debug("%s: POOL %u:%u buf_sz=%u count=%d\n", __func__,
		      pool.node, pool.lpool, buf_sz, buf_cnt);

	if (!__cvmx_fpa3_pool_valid(pool))
		return -1;

	zero_pool = __cvmx_fpa3_pool(pool.node, 0);

	pool_cfg.u64 = csr_rd_node(pool.node, CVMX_FPA_POOLX_CFG(pool.lpool));

	block_size = pool_cfg.cn78xx.buf_size << 7;

	if (pool_cfg.cn78xx.nat_align) {
		/* Assure block_size is legit */
		if (block_size > (1 << 17)) {
			printf("ERROR: %s: POOL %u:%u block size %u is not valid\n",
			       __func__, pool.node, pool.lpool, block_size);
			return -1;
		}
	}
	align = CVMX_CACHE_LINE_SIZE;

	pinfo = cvmx_fpa3_pool_info[pool.node];
	if (!pinfo)
		return -1;
	pinfo += pool.lpool;

	if (pinfo->buf_size != block_size || block_size != buf_sz) {
		printf("ERROR: %s: POOL %u:%u buffer size mismatch\n", __func__,
		       pool.node, pool.lpool);
		return -1;
	}

	if (!mem_ptr) {
		/* When allocating our own memory
		 * make sure at least 'buf_cnt' blocks
		 * will fit into it.
		 */
		mem_size = (long long)buf_cnt * block_size + (block_size - 128);

		mem_ptr = cvmx_helper_mem_alloc(mem_node, mem_size, align);

		if (!mem_ptr) {
			printf("ERROR: %s: POOL %u:%u out of memory, could not allocate %llu bytes\n",
			       __func__, pool.node, pool.lpool, mem_size);
			return -1;
		}

		/* Record memory base for use in shutdown */
		pinfo->bufs_paddr = cvmx_ptr_to_phys(mem_ptr);
	} else {
		/* caller-allocated memory is sized simply, may reduce count */
		mem_size = (long long)buf_cnt * block_size;
		/* caller responsable to free this memory too */
	}

	/* Recalculate buf_cnt after possible alignment adjustment */
	buf_cnt = mem_size / block_size;

	/* Get temporary AURA */
	aura = cvmx_fpa3_reserve_aura(pool.node, -1);
	if (!__cvmx_fpa3_aura_valid(aura))
		return -1;

	/* Attach the temporary AURA to the POOL */
	(void)cvmx_fpa3_aura_cfg(aura, pool, buf_cnt, buf_cnt + 1, 0);

	/* Set AURA count to buffer count to avoid wrap-around */
	csr_wr_node(aura.node, CVMX_FPA_AURAX_CNT(aura.laura), buf_cnt);

	/* Set POOL threshold just above buf count so it does not misfire */
	thresh_reg.u64 = 0;
	thresh_reg.cn78xx.thresh = buf_cnt + 1;
	csr_wr_node(pool.node, CVMX_FPA_POOLX_THRESHOLD(pool.lpool),
		    thresh_reg.u64);

	/* Set buffer memory region bounds checking */
	paddr = (cvmx_ptr_to_phys(mem_ptr) >> 7) << 7;
	pool_start_reg.u64 = 0;
	pool_end_reg.u64 = 0;
	pool_start_reg.cn78xx.addr = paddr >> 7;
	pool_end_reg.cn78xx.addr = (paddr + mem_size + 127) >> 7;

	csr_wr_node(pool.node, CVMX_FPA_POOLX_START_ADDR(pool.lpool),
		    pool_start_reg.u64);
	csr_wr_node(pool.node, CVMX_FPA_POOLX_END_ADDR(pool.lpool),
		    pool_end_reg.u64);

	/* Make sure 'paddr' is divisible by 'block_size' */
	i = (paddr % block_size);
	if (i > 0) {
		i = block_size - i;
		paddr += i;
		mem_size -= i;
	}

	/* The above alignment mimics how the FPA3 hardware
	 * aligns pointers to the buffer size, which only
	 * needs to be multiple of cache line size
	 */

	if (debug && paddr != cvmx_ptr_to_phys(mem_ptr))
		debug("%s: pool mem paddr %#llx adjusted to %#llx for block size %#x\n",
		      __func__, CAST_ULL(cvmx_ptr_to_phys(mem_ptr)),
		      CAST_ULL(paddr), block_size);

	for (i = 0; i < buf_cnt; i++) {
		void *ptr = cvmx_phys_to_ptr(paddr);

		cvmx_fpa3_free_nosync(ptr, aura, 0);

		paddr += block_size;

		if ((paddr + block_size - 1) >= (paddr + mem_size))
			break;
	}

	if (debug && i < buf_cnt) {
		debug("%s: buffer count reduced from %u to %u\n", __func__,
		      buf_cnt, i);
		buf_cnt = i;
	}

	/* Wait for all buffers to reach the POOL before removing temp AURA */
	do {
		CVMX_SYNC;
		avail_reg.u64 = csr_rd_node(
			pool.node, CVMX_FPA_POOLX_AVAILABLE(pool.lpool));
	} while (avail_reg.cn78xx.count < buf_cnt);

	/* Detach the temporary AURA */
	(void)cvmx_fpa3_aura_cfg(aura, zero_pool, 0, 0, 0);

	/* Release temporary AURA */
	(void)cvmx_fpa3_release_aura(aura);

	/* Clear all POOL interrupts */
	int_reg.u64 = 0;
	int_reg.cn78xx.ovfls = 1;
	int_reg.cn78xx.crcerr = 1;
	int_reg.cn78xx.range = 1;
	int_reg.cn78xx.thresh = 1;
	csr_wr_node(pool.node, CVMX_FPA_POOLX_INT(pool.lpool), int_reg.u64);

	/* Record buffer count for shutdown */
	pinfo->buf_count = buf_cnt;

	return buf_cnt;
}

/**
 * @INTERNAL
 *
 * Fill a legacy FPA pool with buffers
 */
static int cvmx_fpa1_fill_pool(cvmx_fpa1_pool_t pool, int num_blocks,
			       void *buffer)
{
	cvmx_fpa_poolx_start_addr_t pool_start_reg;
	cvmx_fpa_poolx_end_addr_t pool_end_reg;
	unsigned int block_size = cvmx_fpa_get_block_size(pool);
	unsigned int mem_size;
	char *bufp;

	if ((unsigned int)pool >= CVMX_FPA1_NUM_POOLS)
		return -1;

	mem_size = block_size * num_blocks;

	if (!buffer) {
		buffer = cvmx_helper_mem_alloc(0, mem_size,
					       CVMX_CACHE_LINE_SIZE);

		cvmx_fpa1_pool_info[pool].base_paddr = cvmx_ptr_to_phys(buffer);
	} else {
		/* Align user-supplied buffer to cache line size */
		unsigned int off =
			(CVMX_CACHE_LINE_SIZE - 1) & cvmx_ptr_to_phys(buffer);
		if (off > 0) {
			//			buffer += CVMX_CACHE_LINE_SIZE - off;
			buffer = (char *)buffer + CVMX_CACHE_LINE_SIZE - off;
			mem_size -= CVMX_CACHE_LINE_SIZE - off;
			num_blocks = mem_size / block_size;
		}
	}

	if (debug)
		debug("%s: memory at %p size %#x\n", __func__, buffer,
		      mem_size);

	pool_start_reg.u64 = 0;
	pool_end_reg.u64 = 0;

	/* buffer pointer range checks are highly recommended, but optional */
	pool_start_reg.cn61xx.addr = 1; /* catch NULL pointers */
	pool_end_reg.cn61xx.addr = (1ull << (40 - 7)) - 1; /* max paddr */
	if (!OCTEON_IS_MODEL(OCTEON_CN63XX)) {
		csr_wr(CVMX_FPA_POOLX_START_ADDR(pool), pool_start_reg.u64);
		csr_wr(CVMX_FPA_POOLX_END_ADDR(pool), pool_end_reg.u64);
	}

	bufp = (char *)buffer;
	while (num_blocks--) {
		cvmx_fpa1_free(bufp, pool, 0);
		cvmx_fpa1_pool_info[pool].buffer_count++;
		bufp += block_size;
	}
	return 0;
}

/**
 * @INTERNAL
 *
 * Setup a legacy FPA pool
 */
static int cvmx_fpa1_pool_init(cvmx_fpa1_pool_t pool_id, int num_blocks,
			       int block_size, void *buffer)
{
	int max_pool = cvmx_fpa_get_max_pools();

	if (pool_id < 0 || pool_id >= max_pool) {
		printf("ERROR: %s pool %d invalid\n", __func__, pool_id);
		return -1;
	}

	if (!cvmx_fpa1_pool_info)
		cvmx_fpa_global_init_node(0);

	if (debug)
		debug("%s: initializing info pool %d\n", __func__, pool_id);

	cvmx_fpa1_pool_info[pool_id].size = block_size;
	cvmx_fpa1_pool_info[pool_id].buffer_count = 0;

	if (debug)
		debug("%s: enabling unit for pool %d\n", __func__, pool_id);

	return 0;
}

/**
 * Initialize global configuration for FPA block for specified node.
 *
 * @param node is the node number
 *
 * @note this function sets the initial QoS averaging timing parameters,
 * for the entire FPA unit (per node), which may be overridden on a
 * per AURA basis.
 */
int cvmx_fpa_global_init_node(int node)
{
	/* There are just the initial parameter values */
#define FPA_RED_AVG_DLY 1
#define FPA_RED_LVL_DLY 3
#define FPA_QOS_AVRG	0
	/* Setting up avg_dly and prb_dly, enable bits */
	if (octeon_has_feature(OCTEON_FEATURE_FPA3)) {
		char pool_info_name[32] = "cvmx_fpa3_pools_";
		char aura_info_name[32] = "cvmx_fpa3_auras_";
		char ns[2] = "0";

		ns[0] += node;
		strcat(pool_info_name, ns);
		strcat(aura_info_name, ns);

		cvmx_fpa3_config_red_params(node, FPA_QOS_AVRG, FPA_RED_LVL_DLY,
					    FPA_RED_AVG_DLY);

		/* Allocate the pinfo named block */
		cvmx_fpa3_pool_info[node] = (cvmx_fpa3_poolx_info_t *)
			cvmx_bootmem_alloc_named_range_once(
				sizeof(cvmx_fpa3_pool_info[0][0]) *
					cvmx_fpa3_num_pools(),
				0, 0, 0, pool_info_name, NULL);

		cvmx_fpa3_aura_info[node] = (cvmx_fpa3_aurax_info_t *)
			cvmx_bootmem_alloc_named_range_once(
				sizeof(cvmx_fpa3_aura_info[0][0]) *
					cvmx_fpa3_num_auras(),
				0, 0, 0, aura_info_name, NULL);

		//XXX add allocation error check

		/* Setup zero_pool on this node */
		cvmx_fpa3_reserve_pool(node, 0);
		cvmx_fpa3_pool_info[node][0].buf_count = 0;
	} else {
		char pool_info_name[32] = "cvmx_fpa_pool";

		/* Allocate the pinfo named block */
		cvmx_fpa1_pool_info = (cvmx_fpa1_pool_info_t *)
			cvmx_bootmem_alloc_named_range_once(
				sizeof(cvmx_fpa1_pool_info[0]) *
					CVMX_FPA1_NUM_POOLS,
				0, 0, 0, pool_info_name, NULL);

		cvmx_fpa1_enable();
	}

	return 0;
}

static void __memset_u64(u64 *ptr, u64 pattern, unsigned int words)
{
	while (words--)
		*ptr++ = pattern;
}

/**
 * @INTERNAL
 * Initialize pool pointer-storage memory
 *
 * Unlike legacy FPA, which used free buffers to store pointers that
 * exceed on-chip memory, FPA3 requires a dedicated memory buffer for
 * free pointer stack back-store.
 *
 * @param pool - pool to initialize
 * @param mem_node - if memory should be allocated from a different node
 * @param max_buffer_cnt - maximum block capacity of pool
 * @param align - buffer alignment mode,
 *   current FPA_NATURAL_ALIGNMENT is supported
 * @param buffer_sz - size of buffers in pool
 */
static int cvmx_fpa3_pool_stack_init(cvmx_fpa3_pool_t pool,
				     unsigned int mem_node,
				     unsigned int max_buffer_cnt,
				     enum cvmx_fpa3_pool_alignment_e align,
				     unsigned int buffer_sz)
{
	cvmx_fpa3_poolx_info_t *pinfo;
	u64 stack_paddr;
	void *mem_ptr;
	unsigned int stack_memory_size;
	cvmx_fpa_poolx_cfg_t pool_cfg;
	cvmx_fpa_poolx_fpf_marks_t pool_fpf_marks;

	if (debug)
		debug("%s: POOL %u:%u bufsz=%u maxbuf=%u\n", __func__,
		      pool.node, pool.lpool, buffer_sz, max_buffer_cnt);

	if (!__cvmx_fpa3_pool_valid(pool)) {
		printf("ERROR: %s: POOL invalid\n", __func__);
		return -1;
	}

	pinfo = cvmx_fpa3_pool_info[pool.node];
	if (!pinfo) {
		printf("ERROR: %s: FPA on node#%u is not initialized\n",
		       __func__, pool.node);
		return -1;
	}
	pinfo += pool.lpool;

	/* Calculate stack size based on buffer count with one line to spare */
	stack_memory_size = (max_buffer_cnt * 128) / 29 + 128 + 127;

	/* Increase stack size by band guard */
	stack_memory_size += guard_band_size << 1;

	/* Align size to cache line */
	stack_memory_size = (stack_memory_size >> 7) << 7;

	/* Allocate internal stack */
	mem_ptr = cvmx_helper_mem_alloc(mem_node, stack_memory_size,
					CVMX_CACHE_LINE_SIZE);

	if (debug)
		debug("%s: stack_mem=%u ptr=%p\n", __func__, stack_memory_size,
		      mem_ptr);

	if (!mem_ptr) {
		debug("ERROR: %sFailed to allocate stack for POOL %u:%u\n",
		      __func__, pool.node, pool.lpool);
		return -1;
	}

	/* Initialize guard bands */
	if (guard_band_size > 0) {
		__memset_u64((u64 *)mem_ptr, magic_pattern,
			     guard_band_size >> 3);
		__memset_u64((u64 *)((char *)mem_ptr + stack_memory_size -
				     guard_band_size),
			     magic_pattern, guard_band_size >> 3);
	}

	pinfo->stack_paddr = cvmx_ptr_to_phys(mem_ptr);
	pinfo->stack_psize = stack_memory_size;

	/* Calculate usable stack start */
	stack_paddr = cvmx_ptr_to_phys((char *)mem_ptr + guard_band_size);

	csr_wr_node(pool.node, CVMX_FPA_POOLX_STACK_BASE(pool.lpool),
		    stack_paddr);
	csr_wr_node(pool.node, CVMX_FPA_POOLX_STACK_ADDR(pool.lpool),
		    stack_paddr);

	/* Calculate usable stack end  - start of last cache line */
	stack_paddr = stack_paddr + stack_memory_size - (guard_band_size << 1);

	csr_wr_node(pool.node, CVMX_FPA_POOLX_STACK_END(pool.lpool),
		    stack_paddr);

	if (debug)
		debug("%s: Stack paddr %#llx - %#llx\n", __func__,
		      CAST_ULL(csr_rd_node(pool.node, CVMX_FPA_POOLX_STACK_BASE(
							      pool.lpool))),
		      CAST_ULL(csr_rd_node(pool.node, CVMX_FPA_POOLX_STACK_END(
							      pool.lpool))));

	/* Setup buffer size for this pool until it is shutdown */
	pinfo->buf_size = buffer_sz;

	pool_cfg.u64 = 0;
	pool_cfg.cn78xx.buf_size = buffer_sz >> 7;
	pool_cfg.cn78xx.l_type = 0x2;
	pool_cfg.cn78xx.ena = 0;
	if (align == FPA_NATURAL_ALIGNMENT)
		pool_cfg.cn78xx.nat_align = 1;

	/* FPA-26117, FPA-22443 */
	pool_fpf_marks.u64 =
		csr_rd_node(pool.node, CVMX_FPA_POOLX_FPF_MARKS(pool.lpool));
	pool_fpf_marks.s.fpf_rd = 0x80;
	csr_wr_node(pool.node, CVMX_FPA_POOLX_FPF_MARKS(pool.lpool),
		    pool_fpf_marks.u64);

	csr_wr_node(pool.node, CVMX_FPA_POOLX_CFG(pool.lpool), pool_cfg.u64);
	pool_cfg.cn78xx.ena = 1;
	csr_wr_node(pool.node, CVMX_FPA_POOLX_CFG(pool.lpool), pool_cfg.u64);

	/* Pool is now ready to be filled up */
	return 0;
}

/**
 * Create an FPA POOL and fill it up with buffers
 *
 * @param node is the node number for the pool and memory location
 * @param desired_pool is the local pool number desired
 *	or -1 for first available
 * @param name is the symbolic name to assign the POOL
 * @param block_size is the size of all buffers held in this POOL
 * @param num_blocks is the number of free buffers to fill into the POOL
 * @param buffer is an optionally caller-supplied memory for the buffers
 *	or NULL to cause the buffer memory to be allocated automatically.
 * @return the POOL handle
 *
 * Note: if the buffer memory is supplied by caller, the application
 * will be responsable to free this memory.
 *
 * Only supported on CN78XX.
 */
cvmx_fpa3_pool_t cvmx_fpa3_setup_fill_pool(int node, int desired_pool,
					   const char *name,
					   unsigned int block_size,
					   unsigned int num_blocks,
					   void *buffer)
{
	cvmx_fpa3_pool_t pool;
	unsigned int mem_node;
	int rc;

	if (node < 0)
		node = cvmx_get_node_num();

	if (debug)
		debug("%s: desired pool=%d bufsize=%u cnt=%u '%s'\n", __func__,
		      desired_pool, block_size, num_blocks, name);

	/* Use memory from the node local to the AURA/POOL */
	mem_node = node;

	if (num_blocks == 0 || num_blocks > 1 << 30) {
		printf("ERROR: %s: invalid block count %u\n", __func__,
		       num_blocks);
		return CVMX_FPA3_INVALID_POOL;
	}

	/*
	 * Check for block size validity:
	 * With user-supplied buffer, can't increase block size,
	 * so make sure it is at least 128, and is aligned to 128
	 * For all cases make sure it is not too big
	 */
	if ((buffer && (block_size < CVMX_CACHE_LINE_SIZE ||
			(block_size & (CVMX_CACHE_LINE_SIZE - 1)))) ||
	    (block_size > (1 << 17))) {
		printf("ERROR: %s: invalid block size %u\n", __func__,
		       block_size);
		return CVMX_FPA3_INVALID_POOL;
	}

	if (block_size < CVMX_CACHE_LINE_SIZE)
		block_size = CVMX_CACHE_LINE_SIZE;

	/* Reserve POOL */
	pool = cvmx_fpa3_reserve_pool(node, desired_pool);

	if (!__cvmx_fpa3_pool_valid(pool)) {
		printf("ERROR: %s: POOL %u:%d not available\n", __func__, node,
		       desired_pool);
		return CVMX_FPA3_INVALID_POOL;
	}

	/* Initialize POOL with stack storage */
	rc = cvmx_fpa3_pool_stack_init(pool, mem_node, num_blocks,
				       FPA_NATURAL_ALIGNMENT, block_size);
	if (rc < 0) {
		printf("ERROR: %s: POOL %u:%u stack setup failed\n", __func__,
		       pool.node, pool.lpool);
		cvmx_fpa3_release_pool(pool);
		return CVMX_FPA3_INVALID_POOL;
	}

	/* Populate the POOL with buffers */
	rc = cvmx_fpa3_pool_populate(pool, num_blocks, block_size, buffer,
				     mem_node);
	if (rc < 0) {
		printf("ERROR: %s: POOL %u:%u memory fill failed\n", __func__,
		       pool.node, pool.lpool);
		cvmx_fpa3_release_pool(pool);
		return CVMX_FPA3_INVALID_POOL;
	}

	cvmx_fpa3_set_pool_name(pool, name);

	return pool;
}

/**
 * Attach an AURA to an existing POOL
 *
 * @param pool is the handle of the POOL to be attached
 * @param desired_aura is the number of the AURA resired
 *	or -1 for the AURA to be automatically assigned
 * @param name is a symbolic name for the new AURA
 * @param block_size is the size of all buffers that will be handed
 *	out by this AURA
 * @param num_blocks is the maximum number of buffers that can be
 *	handed out by this AURA, and can not exceed the number
 *	of buffers filled into the attached POOL
 * @return the AURA handle
 *
 * Only supported on CN78XX.
 */
cvmx_fpa3_gaura_t cvmx_fpa3_set_aura_for_pool(cvmx_fpa3_pool_t pool,
					      int desired_aura,
					      const char *name,
					      unsigned int block_size,
					      unsigned int num_blocks)
{
	cvmx_fpa3_gaura_t aura;
	cvmx_fpa_poolx_available_t avail_reg;
	const char *emsg;
	int rc;

	if (debug)
		debug("%s: aura=%d bufsize=%u cnt=%u '%s'\n", __func__,
		      desired_aura, block_size, num_blocks, name);

	if (!__cvmx_fpa3_pool_valid(pool)) {
		printf("ERROR: %s: POOL argument invalid\n", __func__);
		return CVMX_FPA3_INVALID_GAURA;
	}

	/* Verify the AURA buffer count limit is not above POOL buffer count */
	avail_reg.u64 =
		csr_rd_node(pool.node, CVMX_FPA_POOLX_AVAILABLE(pool.lpool));
	if (avail_reg.cn78xx.count < num_blocks) {
		printf("WARNING: %s: AURA %u:%u buffer count limit %u reduced to POOL available count %u\n",
		       __func__, aura.node, aura.laura, num_blocks,
		       (unsigned int)avail_reg.cn78xx.count);
		num_blocks = avail_reg.cn78xx.count;
	}

	/* Reserve an AURA number, follow desired number */
	aura = cvmx_fpa3_reserve_aura(pool.node, desired_aura);

	if (!__cvmx_fpa3_aura_valid(aura)) {
		printf("ERROR: %s: AURA %u:%d not available\n", __func__,
		       pool.node, desired_aura);
		return CVMX_FPA3_INVALID_GAURA;
	}

	/* Initialize AURA attached to the above POOL */
	rc = cvmx_fpa3_aura_cfg(aura, pool, num_blocks, num_blocks + 1, 0);
	if (rc < 0) {
		emsg = "AURA configuration";
		goto _fail;
	}

	cvmx_fpa3_set_aura_name(aura, name);

	return aura;

_fail:
	printf("ERROR: %s: %s\n", __func__, emsg);
	cvmx_fpa3_release_aura(aura);
	return CVMX_FPA3_INVALID_GAURA;
}

/**
 * Create a combination of an AURA and a POOL
 *
 * @param node is the node number for the pool and memory location
 * @param desired_aura is the number of the AURA resired
 *	or -1 for the AURA to be automatically assigned
 * @param name is a symbolic name for the new AURA
 * @param block_size is the size of all buffers that will be handed
 *	out by this AURA
 * @param num_blocks is the maximum number of buffers that can be
 *	handed out by this AURA, and can not exceed the number
 *	of buffers filled into the attached POOL
 * @param buffer is an optionally caller-supplied memory for the buffers
 *	or NULL to cause the buffer memory to be allocated automatically.
 *
 * @return the AURA handle
 *
 * Note: if the buffer memory is supplied by caller, the application
 * will be responsable to free this memory.
 * The POOL number is always automatically assigned.
 *
 * Only supported on CN78XX.
 */
cvmx_fpa3_gaura_t cvmx_fpa3_setup_aura_and_pool(int node, int desired_aura,
						const char *name, void *buffer,
						unsigned int block_size,
						unsigned int num_blocks)
{
	cvmx_fpa3_gaura_t aura = CVMX_FPA3_INVALID_GAURA;
	cvmx_fpa3_pool_t pool = CVMX_FPA3_INVALID_POOL;
	const char *emsg = "";
	unsigned int mem_node;
	int rc;

	if (debug)
		debug("%s: aura=%d size=%u cnt=%u '%s'\n", __func__,
		      desired_aura, block_size, num_blocks, name);

	if (node < 0)
		node = cvmx_get_node_num();

	if (num_blocks == 0 || num_blocks > 1 << 30) {
		printf("ERROR: %s: invalid block count %u\n", __func__,
		       num_blocks);
		return CVMX_FPA3_INVALID_GAURA;
	}

	/* Use memory from the node local to the AURA/POOL */
	mem_node = node;

	/* Reserve an AURA number, follow desired number */
	aura = cvmx_fpa3_reserve_aura(node, desired_aura);

	if (!__cvmx_fpa3_aura_valid(aura)) {
		emsg = "AURA not available";
		goto _fail;
	}

	/* Reserve POOL dynamically to underpin this AURA */
	pool = cvmx_fpa3_reserve_pool(node, -1);

	if (!__cvmx_fpa3_pool_valid(pool)) {
		emsg = "POOL not available";
		goto _fail;
	}

	/*
	 * Check for block size validity:
	 * With user-supplied buffer, can't increase block size,
	 * so make sure it is at least 128, and is aligned to 128
	 * For all cases make sure it is not too big
	 */
	if ((buffer && (block_size < CVMX_CACHE_LINE_SIZE ||
			(block_size & (CVMX_CACHE_LINE_SIZE - 1)))) ||
	    block_size > (1 << 17)) {
		printf("ERROR: %s: invalid block size %u\n", __func__,
		       block_size);
		emsg = "invalid block size";
		goto _fail;
	}

	if (block_size < CVMX_CACHE_LINE_SIZE)
		block_size = CVMX_CACHE_LINE_SIZE;

	/* Initialize POOL with stack storage */
	rc = cvmx_fpa3_pool_stack_init(pool, mem_node, num_blocks,
				       FPA_NATURAL_ALIGNMENT, block_size);
	if (rc < 0) {
		emsg = "POOL Stack setup";
		goto _fail;
	}

	/* Populate the AURA/POOL with buffers */
	rc = cvmx_fpa3_pool_populate(pool, num_blocks, block_size, buffer,
				     mem_node);
	if (rc < 0) {
		emsg = "POOL buffer memory";
		goto _fail;
	}

	/* Initialize AURA attached to the above POOL */
	rc = cvmx_fpa3_aura_cfg(aura, pool, num_blocks, num_blocks + 1, 0);
	if (rc < 0) {
		emsg = "AURA configuration";
		goto _fail;
	}

	cvmx_fpa3_set_aura_name(aura, name);
	cvmx_fpa3_set_pool_name(pool, name);

	if (debug)
		debug("%s: AURA %u:%u ready, avail=%lld\n", __func__, aura.node,
		      aura.laura, cvmx_fpa3_get_available(aura));

	return aura;

_fail:
	printf("ERROR: %s: Failed in %s\n", __func__, emsg);
	/* These will silently fail if POOL/AURA is not valid */
	cvmx_fpa3_release_aura(aura);
	cvmx_fpa3_release_pool(pool);
	return CVMX_FPA3_INVALID_GAURA;
}

/**
 * Setup a legacy FPA pool
 *
 * @param desired_pool is the POOL number desired or -1 for automatic
 *	assignment
 * @param name is the symbolic POOL name
 * @param block_size is the size of all buffers held in this POOL
 * @param num_blocks is the number of free buffers to fill into the POOL
 * @param buffer is an optionally caller-supplied memory for the buffers
 *	or NULL to cause the buffer memory to be allocated automatically.
 * @return pool number or -1 on error.
 *
 * Note: if the buffer memory is supplied by caller, the application
 * will be responsable to free this memory.
 */
int cvmx_fpa1_setup_pool(int desired_pool, const char *name, void *buffer,
			 unsigned int block_size, unsigned int num_blocks)
{
	cvmx_fpa1_pool_t pool = CVMX_FPA1_INVALID_POOL;
	int rc;

	if (debug)
		debug("%s: desired pool %d, name '%s', mem %p size %u count %u\n",
		      __func__, desired_pool, name, buffer, block_size,
		      num_blocks);

	/* Reserve desired pool or get one dynamically */
	pool = cvmx_fpa1_reserve_pool(desired_pool);

	/* Validate reserved pool, if successful */
	if (pool < 0 || pool >= cvmx_fpa_get_max_pools()) {
		/* global resources would have printed an error message here */
		return CVMX_FPA1_INVALID_POOL;
	}

	/* Initialize the pool */
	rc = cvmx_fpa1_pool_init(pool, num_blocks, block_size, buffer);
	if (rc < 0) {
		printf("ERROR: %s: failed pool %u init\n", __func__, pool);
		cvmx_fpa1_release_pool(pool);
		return CVMX_FPA1_INVALID_POOL;
	}

	rc = cvmx_fpa1_fill_pool(pool, num_blocks, buffer);
	if (rc < 0) {
		printf("ERROR: %s: failed pool %u memory\n", __func__, pool);
		cvmx_fpa1_release_pool(pool);
		return CVMX_FPA1_INVALID_POOL;
	}

	if (debug)
		debug("%s: pool %d filled up\b", __func__, pool);

	cvmx_fpa_set_name(pool, name);
	return pool;
}

/**
 * Setup an FPA pool with buffers
 *
 * @param pool is the POOL number desired or -1 for automatic assignment
 * @param name is the symbolic POOL name
 * @param buffer is an optionally caller-supplied memory for the buffers
 *	or NULL to cause the buffer memory to be allocated automatically.
 * @param block_size is the size of all buffers held in this POOL
 * @param num_blocks is the number of free buffers to fill into the POOL
 * @param buffer is an optionally caller-supplied memory for the buffers
 *	or NULL to cause the buffer memory to be allocated automatically.
 *
 * @return pool number or -1 on error.
 *
 * Note: if the buffer memory is supplied by caller, the application
 * will be responsable to free this memory.
 * This function will work with CN78XX models in backward-compatible mode
 */
int cvmx_fpa_setup_pool(int pool, const char *name, void *buffer,
			u64 block_size, u64 num_blocks)
{
	if (octeon_has_feature(OCTEON_FEATURE_FPA3)) {
		cvmx_fpa3_gaura_t aura;

		aura = cvmx_fpa3_setup_aura_and_pool(-1, pool, name, buffer,
						     block_size, num_blocks);
		if (!__cvmx_fpa3_aura_valid(aura))
			return -1;
		if (aura.laura >= CVMX_FPA1_NUM_POOLS && pool >= 0)
			printf("WARNING: %s: AURA %u is out of range for backward-compatible operation\n",
			       __func__, aura.laura);
		return aura.laura;
	} else {
		return cvmx_fpa1_setup_pool(pool, name, buffer, block_size,
					    num_blocks);
	}
}
