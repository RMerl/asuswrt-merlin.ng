// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Support functions for managing command queues used for
 * various hardware blocks.
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

#include <mach/cvmx-fpa.h>
#include <mach/cvmx-cmd-queue.h>

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

#include <mach/cvmx-helper-util.h>
#include <mach/cvmx-dpi-defs.h>
#include <mach/cvmx-npei-defs.h>
#include <mach/cvmx-pexp-defs.h>

/**
 * This application uses this pointer to access the global queue
 * state. It points to a bootmem named block.
 */
__cvmx_cmd_queue_all_state_t *__cvmx_cmd_queue_state_ptrs[CVMX_MAX_NODES];

/**
 * @INTERNAL
 * Initialize the Global queue state pointer.
 *
 * @return CVMX_CMD_QUEUE_SUCCESS or a failure code
 */
cvmx_cmd_queue_result_t __cvmx_cmd_queue_init_state_ptr(unsigned int node)
{
	const char *alloc_name = "cvmx_cmd_queues\0\0";
	char s[4] = "_0";
	const struct cvmx_bootmem_named_block_desc *block_desc = NULL;
	unsigned int size;
	u64 paddr_min = 0, paddr_max = 0;
	void *ptr;

	if (cvmx_likely(__cvmx_cmd_queue_state_ptrs[node]))
		return CVMX_CMD_QUEUE_SUCCESS;

	/* Add node# to block name */
	if (node > 0) {
		s[1] += node;
		strcat((char *)alloc_name, s);
	}

	/* Find the named block in case it has been created already */
	block_desc = cvmx_bootmem_find_named_block(alloc_name);
	if (block_desc) {
		__cvmx_cmd_queue_state_ptrs[node] =
			(__cvmx_cmd_queue_all_state_t *)cvmx_phys_to_ptr(
				block_desc->base_addr);
		return CVMX_CMD_QUEUE_SUCCESS;
	}

	size = sizeof(*__cvmx_cmd_queue_state_ptrs[node]);

	/* Rest f the code is to allocate a new named block */

	/* Atomically allocate named block once, and zero it by default */
	ptr = cvmx_bootmem_alloc_named_range_once(size, paddr_min, paddr_max,
						  128, alloc_name, NULL);

	if (ptr) {
		__cvmx_cmd_queue_state_ptrs[node] =
			(__cvmx_cmd_queue_all_state_t *)ptr;
	} else {
		debug("ERROR: %s: Unable to get named block %s.\n", __func__,
		      alloc_name);
		return CVMX_CMD_QUEUE_NO_MEMORY;
	}
	return CVMX_CMD_QUEUE_SUCCESS;
}

/**
 * Initialize a command queue for use. The initial FPA buffer is
 * allocated and the hardware unit is configured to point to the
 * new command queue.
 *
 * @param queue_id  Hardware command queue to initialize.
 * @param max_depth Maximum outstanding commands that can be queued.
 * @param fpa_pool  FPA pool the command queues should come from.
 * @param pool_size Size of each buffer in the FPA pool (bytes)
 *
 * @return CVMX_CMD_QUEUE_SUCCESS or a failure code
 */
cvmx_cmd_queue_result_t cvmx_cmd_queue_initialize(cvmx_cmd_queue_id_t queue_id,
						  int max_depth, int fpa_pool,
						  int pool_size)
{
	__cvmx_cmd_queue_state_t *qstate;
	cvmx_cmd_queue_result_t result;
	unsigned int node;
	unsigned int index;
	int fpa_pool_min, fpa_pool_max;
	union cvmx_fpa_ctl_status status;
	void *buffer;

	node = __cvmx_cmd_queue_get_node(queue_id);

	index = __cvmx_cmd_queue_get_index(queue_id);
	if (index >= NUM_ELEMENTS(__cvmx_cmd_queue_state_ptrs[node]->state)) {
		printf("ERROR: %s: queue %#x out of range\n", __func__,
		       queue_id);
		return CVMX_CMD_QUEUE_INVALID_PARAM;
	}

	result = __cvmx_cmd_queue_init_state_ptr(node);
	if (result != CVMX_CMD_QUEUE_SUCCESS)
		return result;

	qstate = __cvmx_cmd_queue_get_state(queue_id);
	if (!qstate)
		return CVMX_CMD_QUEUE_INVALID_PARAM;

	/*
	 * We artificially limit max_depth to 1<<20 words. It is an
	 * arbitrary limit.
	 */
	if (CVMX_CMD_QUEUE_ENABLE_MAX_DEPTH) {
		if (max_depth < 0 || max_depth > 1 << 20)
			return CVMX_CMD_QUEUE_INVALID_PARAM;
	} else if (max_depth != 0) {
		return CVMX_CMD_QUEUE_INVALID_PARAM;
	}

	/* CVMX_FPA_NUM_POOLS maps to cvmx_fpa3_num_auras for FPA3 */
	fpa_pool_min = node << 10;
	fpa_pool_max = fpa_pool_min + CVMX_FPA_NUM_POOLS;

	if (fpa_pool < fpa_pool_min || fpa_pool >= fpa_pool_max)
		return CVMX_CMD_QUEUE_INVALID_PARAM;

	if (pool_size < 128 || pool_size > (1 << 17))
		return CVMX_CMD_QUEUE_INVALID_PARAM;

	if (pool_size & 3)
		debug("WARNING: %s: pool_size %d not multiple of 8\n", __func__,
		      pool_size);

	/* See if someone else has already initialized the queue */
	if (qstate->base_paddr) {
		int depth;
		static const char emsg[] = /* Common error message part */
			"Queue already initialized with different ";

		depth = (max_depth + qstate->pool_size_m1 - 1) /
			qstate->pool_size_m1;
		if (depth != qstate->max_depth) {
			depth = qstate->max_depth * qstate->pool_size_m1;
			debug("ERROR: %s: %s max_depth (%d).\n", __func__, emsg,
			      depth);
			return CVMX_CMD_QUEUE_INVALID_PARAM;
		}
		if (fpa_pool != qstate->fpa_pool) {
			debug("ERROR: %s: %s FPA pool (%d).\n", __func__, emsg,
			      (int)qstate->fpa_pool);
			return CVMX_CMD_QUEUE_INVALID_PARAM;
		}
		if ((pool_size >> 3) - 1 != qstate->pool_size_m1) {
			debug("ERROR: %s: %s FPA pool size (%u).\n", __func__,
			      emsg, (qstate->pool_size_m1 + 1) << 3);
			return CVMX_CMD_QUEUE_INVALID_PARAM;
		}
		return CVMX_CMD_QUEUE_ALREADY_SETUP;
	}

	if (!(octeon_has_feature(OCTEON_FEATURE_FPA3))) {
		status.u64 = csr_rd(CVMX_FPA_CTL_STATUS);
		if (!status.s.enb) {
			debug("ERROR: %s: FPA is not enabled.\n",
			      __func__);
			return CVMX_CMD_QUEUE_NO_MEMORY;
		}
	}
	buffer = cvmx_fpa_alloc(fpa_pool);
	if (!buffer) {
		debug("ERROR: %s: allocating first buffer.\n", __func__);
		return CVMX_CMD_QUEUE_NO_MEMORY;
	}

	index = (pool_size >> 3) - 1;
	qstate->pool_size_m1 = index;
	qstate->max_depth = (max_depth + index - 1) / index;
	qstate->index = 0;
	qstate->fpa_pool = fpa_pool;
	qstate->base_paddr = cvmx_ptr_to_phys(buffer);

	/* Initialize lock */
	__cvmx_cmd_queue_lock_init(queue_id);
	return CVMX_CMD_QUEUE_SUCCESS;
}

/**
 * Return the command buffer to be written to. The purpose of this
 * function is to allow CVMX routine access to the low level buffer
 * for initial hardware setup. User applications should not call this
 * function directly.
 *
 * @param queue_id Command queue to query
 *
 * @return Command buffer or NULL on failure
 */
void *cvmx_cmd_queue_buffer(cvmx_cmd_queue_id_t queue_id)
{
	__cvmx_cmd_queue_state_t *qptr = __cvmx_cmd_queue_get_state(queue_id);

	if (qptr && qptr->base_paddr)
		return cvmx_phys_to_ptr((u64)qptr->base_paddr);
	else
		return NULL;
}

static u64 *__cvmx_cmd_queue_add_blk(__cvmx_cmd_queue_state_t *qptr)
{
	u64 *cmd_ptr;
	u64 *new_buffer;
	u64 new_paddr;

	/* Get base vaddr of current (full) block */
	cmd_ptr = (u64 *)cvmx_phys_to_ptr((u64)qptr->base_paddr);

	/* Allocate a new block from the per-queue pool */
	new_buffer = (u64 *)cvmx_fpa_alloc(qptr->fpa_pool);

	/* Check for allocation failure */
	if (cvmx_unlikely(!new_buffer))
		return NULL;

	/* Zero out the new block link pointer,
	 * in case this block will be filled to the rim
	 */
	new_buffer[qptr->pool_size_m1] = ~0ull;

	/* Get physical address of the new buffer */
	new_paddr = cvmx_ptr_to_phys(new_buffer);

	/* Store the physical link address at the end of current full block */
	cmd_ptr[qptr->pool_size_m1] = new_paddr;

	/* Store the physical address in the queue state structure */
	qptr->base_paddr = new_paddr;
	qptr->index = 0;

	/* Return the virtual base of the new block */
	return new_buffer;
}

/**
 * @INTERNAL
 * Add command words into a queue, handles all the corener cases
 * where only some of the words might fit into the current block,
 * and a new block may need to be allocated.
 * Locking and argument checks are done in the front-end in-line
 * functions that call this one for the rare corner cases.
 */
cvmx_cmd_queue_result_t
__cvmx_cmd_queue_write_raw(cvmx_cmd_queue_id_t queue_id,
			   __cvmx_cmd_queue_state_t *qptr, int cmd_count,
			   const u64 *cmds)
{
	u64 *cmd_ptr;
	unsigned int index;

	cmd_ptr = (u64 *)cvmx_phys_to_ptr((u64)qptr->base_paddr);
	index = qptr->index;

	/* Enforce queue depth limit, if enabled, once per block */
	if (CVMX_CMD_QUEUE_ENABLE_MAX_DEPTH && cvmx_unlikely(qptr->max_depth)) {
		unsigned int depth = cvmx_cmd_queue_length(queue_id);

		depth /= qptr->pool_size_m1;

		if (cvmx_unlikely(depth > qptr->max_depth))
			return CVMX_CMD_QUEUE_FULL;
	}

	/*
	 * If the block allocation fails, even the words that we wrote
	 * to the current block will not count because the 'index' will
	 * not be comitted.
	 * The loop is run 'count + 1' times to take care of the tail
	 * case, where the buffer is full to the rim, so the link
	 * pointer must be filled with a valid address.
	 */
	while (cmd_count >= 0) {
		if (index >= qptr->pool_size_m1) {
			/* Block is full, get another one and proceed */
			cmd_ptr = __cvmx_cmd_queue_add_blk(qptr);

			/* Baul on allocation error w/o comitting anything */
			if (cvmx_unlikely(!cmd_ptr))
				return CVMX_CMD_QUEUE_NO_MEMORY;

			/* Reset index for start of new block */
			index = 0;
		}
		/* Exit Loop on 'count + 1' iterations */
		if (cmd_count <= 0)
			break;
		/* Store commands into queue block while there is space */
		cmd_ptr[index++] = *cmds++;
		cmd_count--;
	} /* while cmd_count */

	/* Commit added words if all is well */
	qptr->index = index;

	return CVMX_CMD_QUEUE_SUCCESS;
}
