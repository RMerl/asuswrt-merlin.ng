/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Support functions for managing command queues used for
 * various hardware blocks.
 *
 * The common command queue infrastructure abstracts out the
 * software necessary for adding to Octeon's chained queue
 * structures. These structures are used for commands to the
 * PKO, ZIP, DFA, RAID, HNA, and DMA engine blocks. Although each
 * hardware unit takes commands and CSRs of different types,
 * they all use basic linked command buffers to store the
 * pending request. In general, users of the CVMX API don't
 * call cvmx-cmd-queue functions directly. Instead the hardware
 * unit specific wrapper should be used. The wrappers perform
 * unit specific validation and CSR writes to submit the
 * commands.
 *
 * Even though most software will never directly interact with
 * cvmx-cmd-queue, knowledge of its internal workings can help
 * in diagnosing performance problems and help with debugging.
 *
 * Command queue pointers are stored in a global named block
 * called "cvmx_cmd_queues". Except for the PKO queues, each
 * hardware queue is stored in its own cache line to reduce SMP
 * contention on spin locks. The PKO queues are stored such that
 * every 16th queue is next to each other in memory. This scheme
 * allows for queues being in separate cache lines when there
 * are low number of queues per port. With 16 queues per port,
 * the first queue for each port is in the same cache area. The
 * second queues for each port are in another area, etc. This
 * allows software to implement very efficient lockless PKO with
 * 16 queues per port using a minimum of cache lines per core.
 * All queues for a given core will be isolated in the same
 * cache area.
 *
 * In addition to the memory pointer layout, cvmx-cmd-queue
 * provides an optimized fair ll/sc locking mechanism for the
 * queues. The lock uses a "ticket / now serving" model to
 * maintain fair order on contended locks. In addition, it uses
 * predicted locking time to limit cache contention. When a core
 * know it must wait in line for a lock, it spins on the
 * internal cycle counter to completely eliminate any causes of
 * bus traffic.
 */

#ifndef __CVMX_CMD_QUEUE_H__
#define __CVMX_CMD_QUEUE_H__

/**
 * By default we disable the max depth support. Most programs
 * don't use it and it slows down the command queue processing
 * significantly.
 */
#ifndef CVMX_CMD_QUEUE_ENABLE_MAX_DEPTH
#define CVMX_CMD_QUEUE_ENABLE_MAX_DEPTH 0
#endif

/**
 * Enumeration representing all hardware blocks that use command
 * queues. Each hardware block has up to 65536 sub identifiers for
 * multiple command queues. Not all chips support all hardware
 * units.
 */
typedef enum {
	CVMX_CMD_QUEUE_PKO_BASE = 0x00000,
#define CVMX_CMD_QUEUE_PKO(queue)                                                                  \
	((cvmx_cmd_queue_id_t)(CVMX_CMD_QUEUE_PKO_BASE + (0xffff & (queue))))
	CVMX_CMD_QUEUE_ZIP = 0x10000,
#define CVMX_CMD_QUEUE_ZIP_QUE(queue)                                                              \
	((cvmx_cmd_queue_id_t)(CVMX_CMD_QUEUE_ZIP + (0xffff & (queue))))
	CVMX_CMD_QUEUE_DFA = 0x20000,
	CVMX_CMD_QUEUE_RAID = 0x30000,
	CVMX_CMD_QUEUE_DMA_BASE = 0x40000,
#define CVMX_CMD_QUEUE_DMA(queue)                                                                  \
	((cvmx_cmd_queue_id_t)(CVMX_CMD_QUEUE_DMA_BASE + (0xffff & (queue))))
	CVMX_CMD_QUEUE_BCH = 0x50000,
#define CVMX_CMD_QUEUE_BCH(queue) ((cvmx_cmd_queue_id_t)(CVMX_CMD_QUEUE_BCH + (0xffff & (queue))))
	CVMX_CMD_QUEUE_HNA = 0x60000,
	CVMX_CMD_QUEUE_END = 0x70000,
} cvmx_cmd_queue_id_t;

#define CVMX_CMD_QUEUE_ZIP3_QUE(node, queue)                                                       \
	((cvmx_cmd_queue_id_t)((node) << 24 | CVMX_CMD_QUEUE_ZIP | (0xffff & (queue))))

/**
 * Command write operations can fail if the command queue needs
 * a new buffer and the associated FPA pool is empty. It can also
 * fail if the number of queued command words reaches the maximum
 * set at initialization.
 */
typedef enum {
	CVMX_CMD_QUEUE_SUCCESS = 0,
	CVMX_CMD_QUEUE_NO_MEMORY = -1,
	CVMX_CMD_QUEUE_FULL = -2,
	CVMX_CMD_QUEUE_INVALID_PARAM = -3,
	CVMX_CMD_QUEUE_ALREADY_SETUP = -4,
} cvmx_cmd_queue_result_t;

typedef struct {
	/* First 64-bit word: */
	u64 fpa_pool : 16;
	u64 base_paddr : 48;
	s32 index;
	u16 max_depth;
	u16 pool_size_m1;
} __cvmx_cmd_queue_state_t;

/**
 * command-queue locking uses a fair ticket spinlock algo,
 * with 64-bit tickets for endianness-neutrality and
 * counter overflow protection.
 * Lock is free when both counters are of equal value.
 */
typedef struct {
	u64 ticket;
	u64 now_serving;
} __cvmx_cmd_queue_lock_t;

/**
 * @INTERNAL
 * This structure contains the global state of all command queues.
 * It is stored in a bootmem named block and shared by all
 * applications running on Octeon. Tickets are stored in a different
 * cache line that queue information to reduce the contention on the
 * ll/sc used to get a ticket. If this is not the case, the update
 * of queue state causes the ll/sc to fail quite often.
 */
typedef struct {
	__cvmx_cmd_queue_lock_t lock[(CVMX_CMD_QUEUE_END >> 16) * 256];
	__cvmx_cmd_queue_state_t state[(CVMX_CMD_QUEUE_END >> 16) * 256];
} __cvmx_cmd_queue_all_state_t;

extern __cvmx_cmd_queue_all_state_t *__cvmx_cmd_queue_state_ptrs[CVMX_MAX_NODES];

/**
 * @INTERNAL
 * Internal function to handle the corner cases
 * of adding command words to a queue when the current
 * block is getting full.
 */
cvmx_cmd_queue_result_t __cvmx_cmd_queue_write_raw(cvmx_cmd_queue_id_t queue_id,
						   __cvmx_cmd_queue_state_t *qptr, int cmd_count,
						   const u64 *cmds);

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
 * Return: CVMX_CMD_QUEUE_SUCCESS or a failure code
 */
cvmx_cmd_queue_result_t cvmx_cmd_queue_initialize(cvmx_cmd_queue_id_t queue_id, int max_depth,
						  int fpa_pool, int pool_size);

/**
 * Shutdown a queue a free it's command buffers to the FPA. The
 * hardware connected to the queue must be stopped before this
 * function is called.
 *
 * @param queue_id Queue to shutdown
 *
 * Return: CVMX_CMD_QUEUE_SUCCESS or a failure code
 */
cvmx_cmd_queue_result_t cvmx_cmd_queue_shutdown(cvmx_cmd_queue_id_t queue_id);

/**
 * Return the number of command words pending in the queue. This
 * function may be relatively slow for some hardware units.
 *
 * @param queue_id Hardware command queue to query
 *
 * Return: Number of outstanding commands
 */
int cvmx_cmd_queue_length(cvmx_cmd_queue_id_t queue_id);

/**
 * Return the command buffer to be written to. The purpose of this
 * function is to allow CVMX routine access to the low level buffer
 * for initial hardware setup. User applications should not call this
 * function directly.
 *
 * @param queue_id Command queue to query
 *
 * Return: Command buffer or NULL on failure
 */
void *cvmx_cmd_queue_buffer(cvmx_cmd_queue_id_t queue_id);

/**
 * @INTERNAL
 * Retrieve or allocate command queue state named block
 */
cvmx_cmd_queue_result_t __cvmx_cmd_queue_init_state_ptr(unsigned int node);

/**
 * @INTERNAL
 * Get the index into the state arrays for the supplied queue id.
 *
 * @param queue_id Queue ID to get an index for
 *
 * Return: Index into the state arrays
 */
static inline unsigned int __cvmx_cmd_queue_get_index(cvmx_cmd_queue_id_t queue_id)
{
	/* Warning: This code currently only works with devices that have 256
	 * queues or less.  Devices with more than 16 queues are laid out in
	 * memory to allow cores quick access to every 16th queue. This reduces
	 * cache thrashing when you are running 16 queues per port to support
	 * lockless operation
	 */
	unsigned int unit = (queue_id >> 16) & 0xff;
	unsigned int q = (queue_id >> 4) & 0xf;
	unsigned int core = queue_id & 0xf;

	return (unit << 8) | (core << 4) | q;
}

static inline int __cvmx_cmd_queue_get_node(cvmx_cmd_queue_id_t queue_id)
{
	unsigned int node = queue_id >> 24;
	return node;
}

/**
 * @INTERNAL
 * Lock the supplied queue so nobody else is updating it at the same
 * time as us.
 *
 * @param queue_id Queue ID to lock
 *
 */
static inline void __cvmx_cmd_queue_lock(cvmx_cmd_queue_id_t queue_id)
{
}

/**
 * @INTERNAL
 * Unlock the queue, flushing all writes.
 *
 * @param queue_id Queue ID to lock
 *
 */
static inline void __cvmx_cmd_queue_unlock(cvmx_cmd_queue_id_t queue_id)
{
	CVMX_SYNCWS; /* nudge out the unlock. */
}

/**
 * @INTERNAL
 * Initialize a command-queue lock to "unlocked" state.
 */
static inline void __cvmx_cmd_queue_lock_init(cvmx_cmd_queue_id_t queue_id)
{
	unsigned int index = __cvmx_cmd_queue_get_index(queue_id);
	unsigned int node = __cvmx_cmd_queue_get_node(queue_id);

	__cvmx_cmd_queue_state_ptrs[node]->lock[index] = (__cvmx_cmd_queue_lock_t){ 0, 0 };
	CVMX_SYNCWS;
}

/**
 * @INTERNAL
 * Get the queue state structure for the given queue id
 *
 * @param queue_id Queue id to get
 *
 * Return: Queue structure or NULL on failure
 */
static inline __cvmx_cmd_queue_state_t *__cvmx_cmd_queue_get_state(cvmx_cmd_queue_id_t queue_id)
{
	unsigned int index;
	unsigned int node;
	__cvmx_cmd_queue_state_t *qptr;

	node = __cvmx_cmd_queue_get_node(queue_id);
	index = __cvmx_cmd_queue_get_index(queue_id);

	if (cvmx_unlikely(!__cvmx_cmd_queue_state_ptrs[node]))
		__cvmx_cmd_queue_init_state_ptr(node);

	qptr = &__cvmx_cmd_queue_state_ptrs[node]->state[index];
	return qptr;
}

/**
 * Write an arbitrary number of command words to a command queue.
 * This is a generic function; the fixed number of command word
 * functions yield higher performance.
 *
 * @param queue_id  Hardware command queue to write to
 * @param use_locking
 *                  Use internal locking to ensure exclusive access for queue
 *                  updates. If you don't use this locking you must ensure
 *                  exclusivity some other way. Locking is strongly recommended.
 * @param cmd_count Number of command words to write
 * @param cmds      Array of commands to write
 *
 * Return: CVMX_CMD_QUEUE_SUCCESS or a failure code
 */
static inline cvmx_cmd_queue_result_t
cvmx_cmd_queue_write(cvmx_cmd_queue_id_t queue_id, bool use_locking, int cmd_count, const u64 *cmds)
{
	cvmx_cmd_queue_result_t ret = CVMX_CMD_QUEUE_SUCCESS;
	u64 *cmd_ptr;

	__cvmx_cmd_queue_state_t *qptr = __cvmx_cmd_queue_get_state(queue_id);

	/* Make sure nobody else is updating the same queue */
	if (cvmx_likely(use_locking))
		__cvmx_cmd_queue_lock(queue_id);

	/* Most of the time there is lots of free words in current block */
	if (cvmx_unlikely((qptr->index + cmd_count) >= qptr->pool_size_m1)) {
		/* The rare case when nearing end of block */
		ret = __cvmx_cmd_queue_write_raw(queue_id, qptr, cmd_count, cmds);
	} else {
		cmd_ptr = (u64 *)cvmx_phys_to_ptr((u64)qptr->base_paddr);
		/* Loop easy for compiler to unroll for the likely case */
		while (cmd_count > 0) {
			cmd_ptr[qptr->index++] = *cmds++;
			cmd_count--;
		}
	}

	/* All updates are complete. Release the lock and return */
	if (cvmx_likely(use_locking))
		__cvmx_cmd_queue_unlock(queue_id);
	else
		CVMX_SYNCWS;

	return ret;
}

/**
 * Simple function to write two command words to a command queue.
 *
 * @param queue_id Hardware command queue to write to
 * @param use_locking
 *                 Use internal locking to ensure exclusive access for queue
 *                 updates. If you don't use this locking you must ensure
 *                 exclusivity some other way. Locking is strongly recommended.
 * @param cmd1     Command
 * @param cmd2     Command
 *
 * Return: CVMX_CMD_QUEUE_SUCCESS or a failure code
 */
static inline cvmx_cmd_queue_result_t cvmx_cmd_queue_write2(cvmx_cmd_queue_id_t queue_id,
							    bool use_locking, u64 cmd1, u64 cmd2)
{
	cvmx_cmd_queue_result_t ret = CVMX_CMD_QUEUE_SUCCESS;
	u64 *cmd_ptr;

	__cvmx_cmd_queue_state_t *qptr = __cvmx_cmd_queue_get_state(queue_id);

	/* Make sure nobody else is updating the same queue */
	if (cvmx_likely(use_locking))
		__cvmx_cmd_queue_lock(queue_id);

	if (cvmx_unlikely((qptr->index + 2) >= qptr->pool_size_m1)) {
		/* The rare case when nearing end of block */
		u64 cmds[2];

		cmds[0] = cmd1;
		cmds[1] = cmd2;
		ret = __cvmx_cmd_queue_write_raw(queue_id, qptr, 2, cmds);
	} else {
		/* Likely case to work fast */
		cmd_ptr = (u64 *)cvmx_phys_to_ptr((u64)qptr->base_paddr);
		cmd_ptr += qptr->index;
		qptr->index += 2;
		cmd_ptr[0] = cmd1;
		cmd_ptr[1] = cmd2;
	}

	/* All updates are complete. Release the lock and return */
	if (cvmx_likely(use_locking))
		__cvmx_cmd_queue_unlock(queue_id);
	else
		CVMX_SYNCWS;

	return ret;
}

/**
 * Simple function to write three command words to a command queue.
 *
 * @param queue_id Hardware command queue to write to
 * @param use_locking
 *                 Use internal locking to ensure exclusive access for queue
 *                 updates. If you don't use this locking you must ensure
 *                 exclusivity some other way. Locking is strongly recommended.
 * @param cmd1     Command
 * @param cmd2     Command
 * @param cmd3     Command
 *
 * Return: CVMX_CMD_QUEUE_SUCCESS or a failure code
 */
static inline cvmx_cmd_queue_result_t
cvmx_cmd_queue_write3(cvmx_cmd_queue_id_t queue_id, bool use_locking, u64 cmd1, u64 cmd2, u64 cmd3)
{
	cvmx_cmd_queue_result_t ret = CVMX_CMD_QUEUE_SUCCESS;
	__cvmx_cmd_queue_state_t *qptr = __cvmx_cmd_queue_get_state(queue_id);
	u64 *cmd_ptr;

	/* Make sure nobody else is updating the same queue */
	if (cvmx_likely(use_locking))
		__cvmx_cmd_queue_lock(queue_id);

	if (cvmx_unlikely((qptr->index + 3) >= qptr->pool_size_m1)) {
		/* Most of the time there is lots of free words in current block */
		u64 cmds[3];

		cmds[0] = cmd1;
		cmds[1] = cmd2;
		cmds[2] = cmd3;
		ret = __cvmx_cmd_queue_write_raw(queue_id, qptr, 3, cmds);
	} else {
		cmd_ptr = (u64 *)cvmx_phys_to_ptr((u64)qptr->base_paddr);
		cmd_ptr += qptr->index;
		qptr->index += 3;
		cmd_ptr[0] = cmd1;
		cmd_ptr[1] = cmd2;
		cmd_ptr[2] = cmd3;
	}

	/* All updates are complete. Release the lock and return */
	if (cvmx_likely(use_locking))
		__cvmx_cmd_queue_unlock(queue_id);
	else
		CVMX_SYNCWS;

	return ret;
}

#endif /* __CVMX_CMD_QUEUE_H__ */
