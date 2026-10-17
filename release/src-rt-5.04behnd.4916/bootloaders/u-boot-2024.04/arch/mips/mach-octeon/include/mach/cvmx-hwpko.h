/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Interface to the hardware Packet Output unit.
 *
 * Starting with SDK 1.7.0, the PKO output functions now support
 * two types of locking. CVMX_PKO_LOCK_ATOMIC_TAG continues to
 * function similarly to previous SDKs by using POW atomic tags
 * to preserve ordering and exclusivity. As a new option, you
 * can now pass CVMX_PKO_LOCK_CMD_QUEUE which uses a ll/sc
 * memory based locking instead. This locking has the advantage
 * of not affecting the tag state but doesn't preserve packet
 * ordering. CVMX_PKO_LOCK_CMD_QUEUE is appropriate in most
 * generic code while CVMX_PKO_LOCK_CMD_QUEUE should be used
 * with hand tuned fast path code.
 *
 * Some of other SDK differences visible to the command command
 * queuing:
 * - PKO indexes are no longer stored in the FAU. A large
 *   percentage of the FAU register block used to be tied up
 *   maintaining PKO queue pointers. These are now stored in a
 *   global named block.
 * - The PKO <b>use_locking</b> parameter can now have a global
 *   effect. Since all application use the same named block,
 *   queue locking correctly applies across all operating
 *   systems when using CVMX_PKO_LOCK_CMD_QUEUE.
 * - PKO 3 word commands are now supported. Use
 *   cvmx_pko_send_packet_finish3().
 */

#ifndef __CVMX_HWPKO_H__
#define __CVMX_HWPKO_H__

#include "cvmx-hwfau.h"
#include "cvmx-fpa.h"
#include "cvmx-pow.h"
#include "cvmx-cmd-queue.h"
#include "cvmx-helper.h"
#include "cvmx-helper-util.h"
#include "cvmx-helper-cfg.h"

/* Adjust the command buffer size by 1 word so that in the case of using only
** two word PKO commands no command words stradle buffers.  The useful values
** for this are 0 and 1. */
#define CVMX_PKO_COMMAND_BUFFER_SIZE_ADJUST (1)

#define CVMX_PKO_MAX_OUTPUT_QUEUES_STATIC 256
#define CVMX_PKO_MAX_OUTPUT_QUEUES                                                                 \
	((OCTEON_IS_OCTEON2() || OCTEON_IS_MODEL(OCTEON_CN70XX)) ? 256 : 128)
#define CVMX_PKO_NUM_OUTPUT_PORTS                                                                  \
	((OCTEON_IS_MODEL(OCTEON_CN63XX)) ? 44 : (OCTEON_IS_MODEL(OCTEON_CN66XX) ? 48 : 40))
#define CVMX_PKO_MEM_QUEUE_PTRS_ILLEGAL_PID 63
#define CVMX_PKO_QUEUE_STATIC_PRIORITY	    9
#define CVMX_PKO_ILLEGAL_QUEUE		    0xFFFF
#define CVMX_PKO_MAX_QUEUE_DEPTH	    0

typedef enum {
	CVMX_PKO_SUCCESS,
	CVMX_PKO_INVALID_PORT,
	CVMX_PKO_INVALID_QUEUE,
	CVMX_PKO_INVALID_PRIORITY,
	CVMX_PKO_NO_MEMORY,
	CVMX_PKO_PORT_ALREADY_SETUP,
	CVMX_PKO_CMD_QUEUE_INIT_ERROR
} cvmx_pko_return_value_t;

/**
 * This enumeration represents the differnet locking modes supported by PKO.
 */
typedef enum {
	CVMX_PKO_LOCK_NONE = 0,
	CVMX_PKO_LOCK_ATOMIC_TAG = 1,
	CVMX_PKO_LOCK_CMD_QUEUE = 2,
} cvmx_pko_lock_t;

typedef struct cvmx_pko_port_status {
	u32 packets;
	u64 octets;
	u64 doorbell;
} cvmx_pko_port_status_t;

/**
 * This structure defines the address to use on a packet enqueue
 */
typedef union {
	u64 u64;
	struct {
		cvmx_mips_space_t mem_space : 2;
		u64 reserved : 13;
		u64 is_io : 1;
		u64 did : 8;
		u64 reserved2 : 4;
		u64 reserved3 : 15;
		u64 port : 9;
		u64 queue : 9;
		u64 reserved4 : 3;
	} s;
} cvmx_pko_doorbell_address_t;

/**
 * Structure of the first packet output command word.
 */
typedef union {
	u64 u64;
	struct {
		cvmx_fau_op_size_t size1 : 2;
		cvmx_fau_op_size_t size0 : 2;
		u64 subone1 : 1;
		u64 reg1 : 11;
		u64 subone0 : 1;
		u64 reg0 : 11;
		u64 le : 1;
		u64 n2 : 1;
		u64 wqp : 1;
		u64 rsp : 1;
		u64 gather : 1;
		u64 ipoffp1 : 7;
		u64 ignore_i : 1;
		u64 dontfree : 1;
		u64 segs : 6;
		u64 total_bytes : 16;
	} s;
} cvmx_pko_command_word0_t;

/**
 * Call before any other calls to initialize the packet
 * output system.
 */

void cvmx_pko_hw_init(u8 pool, unsigned int bufsize);

/**
 * Enables the packet output hardware. It must already be
 * configured.
 */
void cvmx_pko_enable(void);

/**
 * Disables the packet output. Does not affect any configuration.
 */
void cvmx_pko_disable(void);

/**
 * Shutdown and free resources required by packet output.
 */

void cvmx_pko_shutdown(void);

/**
 * Configure a output port and the associated queues for use.
 *
 * @param port       Port to configure.
 * @param base_queue First queue number to associate with this port.
 * @param num_queues Number of queues t oassociate with this port
 * @param priority   Array of priority levels for each queue. Values are
 *                   allowed to be 1-8. A value of 8 get 8 times the traffic
 *                   of a value of 1. There must be num_queues elements in the
 *                   array.
 */
cvmx_pko_return_value_t cvmx_pko_config_port(int port, int base_queue, int num_queues,
					     const u8 priority[]);

/**
 * Ring the packet output doorbell. This tells the packet
 * output hardware that "len" command words have been added
 * to its pending list.  This command includes the required
 * CVMX_SYNCWS before the doorbell ring.
 *
 * WARNING: This function may have to look up the proper PKO port in
 * the IPD port to PKO port map, and is thus slower than calling
 * cvmx_pko_doorbell_pkoid() directly if the PKO port identifier is
 * known.
 *
 * @param ipd_port   The IPD port corresponding the to pko port the packet is for
 * @param queue  Queue the packet is for
 * @param len    Length of the command in 64 bit words
 */
static inline void cvmx_pko_doorbell(u64 ipd_port, u64 queue, u64 len)
{
	cvmx_pko_doorbell_address_t ptr;
	u64 pko_port;

	pko_port = ipd_port;
	if (octeon_has_feature(OCTEON_FEATURE_PKND))
		pko_port = cvmx_helper_cfg_ipd2pko_port_base(ipd_port);

	ptr.u64 = 0;
	ptr.s.mem_space = CVMX_IO_SEG;
	ptr.s.did = CVMX_OCT_DID_PKT_SEND;
	ptr.s.is_io = 1;
	ptr.s.port = pko_port;
	ptr.s.queue = queue;
	/* Need to make sure output queue data is in DRAM before doorbell write */
	CVMX_SYNCWS;
	cvmx_write_io(ptr.u64, len);
}

/**
 * Prepare to send a packet.  This may initiate a tag switch to
 * get exclusive access to the output queue structure, and
 * performs other prep work for the packet send operation.
 *
 * cvmx_pko_send_packet_finish() MUST be called after this function is called,
 * and must be called with the same port/queue/use_locking arguments.
 *
 * The use_locking parameter allows the caller to use three
 * possible locking modes.
 * - CVMX_PKO_LOCK_NONE
 *      - PKO doesn't do any locking. It is the responsibility
 *          of the application to make sure that no other core
 *          is accessing the same queue at the same time.
 * - CVMX_PKO_LOCK_ATOMIC_TAG
 *      - PKO performs an atomic tagswitch to insure exclusive
 *          access to the output queue. This will maintain
 *          packet ordering on output.
 * - CVMX_PKO_LOCK_CMD_QUEUE
 *      - PKO uses the common command queue locks to insure
 *          exclusive access to the output queue. This is a
 *          memory based ll/sc. This is the most portable
 *          locking mechanism.
 *
 * NOTE: If atomic locking is used, the POW entry CANNOT be
 * descheduled, as it does not contain a valid WQE pointer.
 *
 * @param port   Port to send it on, this can be either IPD port or PKO
 *		 port.
 * @param queue  Queue to use
 * @param use_locking
 *               CVMX_PKO_LOCK_NONE, CVMX_PKO_LOCK_ATOMIC_TAG, or CVMX_PKO_LOCK_CMD_QUEUE
 */
static inline void cvmx_pko_send_packet_prepare(u64 port __attribute__((unused)), u64 queue,
						cvmx_pko_lock_t use_locking)
{
	if (use_locking == CVMX_PKO_LOCK_ATOMIC_TAG) {
		/*
		 * Must do a full switch here to handle all cases.  We use a
		 * fake WQE pointer, as the POW does not access this memory.
		 * The WQE pointer and group are only used if this work is
		 * descheduled, which is not supported by the
		 * cvmx_pko_send_packet_prepare/cvmx_pko_send_packet_finish
		 * combination. Note that this is a special case in which these
		 * fake values can be used - this is not a general technique.
		 */
		u32 tag = CVMX_TAG_SW_BITS_INTERNAL << CVMX_TAG_SW_SHIFT |
			  CVMX_TAG_SUBGROUP_PKO << CVMX_TAG_SUBGROUP_SHIFT |
			  (CVMX_TAG_SUBGROUP_MASK & queue);
		cvmx_pow_tag_sw_full((cvmx_wqe_t *)cvmx_phys_to_ptr(0x80), tag,
				     CVMX_POW_TAG_TYPE_ATOMIC, 0);
	}
}

#define cvmx_pko_send_packet_prepare_pkoid cvmx_pko_send_packet_prepare

/**
 * Complete packet output. cvmx_pko_send_packet_prepare() must be called exactly once before this,
 * and the same parameters must be passed to both cvmx_pko_send_packet_prepare() and
 * cvmx_pko_send_packet_finish().
 *
 * WARNING: This function may have to look up the proper PKO port in
 * the IPD port to PKO port map, and is thus slower than calling
 * cvmx_pko_send_packet_finish_pkoid() directly if the PKO port
 * identifier is known.
 *
 * @param ipd_port   The IPD port corresponding the to pko port the packet is for
 * @param queue  Queue to use
 * @param pko_command
 *               PKO HW command word
 * @param packet Packet to send
 * @param use_locking
 *               CVMX_PKO_LOCK_NONE, CVMX_PKO_LOCK_ATOMIC_TAG, or CVMX_PKO_LOCK_CMD_QUEUE
 *
 * Return: returns CVMX_PKO_SUCCESS on success, or error code on failure of output
 */
static inline cvmx_pko_return_value_t
cvmx_hwpko_send_packet_finish(u64 ipd_port, u64 queue, cvmx_pko_command_word0_t pko_command,
			      cvmx_buf_ptr_t packet, cvmx_pko_lock_t use_locking)
{
	cvmx_cmd_queue_result_t result;

	if (use_locking == CVMX_PKO_LOCK_ATOMIC_TAG)
		cvmx_pow_tag_sw_wait();

	result = cvmx_cmd_queue_write2(CVMX_CMD_QUEUE_PKO(queue),
				       (use_locking == CVMX_PKO_LOCK_CMD_QUEUE), pko_command.u64,
				       packet.u64);
	if (cvmx_likely(result == CVMX_CMD_QUEUE_SUCCESS)) {
		cvmx_pko_doorbell(ipd_port, queue, 2);
		return CVMX_PKO_SUCCESS;
	} else if ((result == CVMX_CMD_QUEUE_NO_MEMORY) || (result == CVMX_CMD_QUEUE_FULL)) {
		return CVMX_PKO_NO_MEMORY;
	} else {
		return CVMX_PKO_INVALID_QUEUE;
	}
}

/**
 * Complete packet output. cvmx_pko_send_packet_prepare() must be called exactly once before this,
 * and the same parameters must be passed to both cvmx_pko_send_packet_prepare() and
 * cvmx_pko_send_packet_finish().
 *
 * WARNING: This function may have to look up the proper PKO port in
 * the IPD port to PKO port map, and is thus slower than calling
 * cvmx_pko_send_packet_finish3_pkoid() directly if the PKO port
 * identifier is known.
 *
 * @param ipd_port   The IPD port corresponding the to pko port the packet is for
 * @param queue  Queue to use
 * @param pko_command
 *               PKO HW command word
 * @param packet Packet to send
 * @param addr   Plysical address of a work queue entry or physical address to zero on complete.
 * @param use_locking
 *               CVMX_PKO_LOCK_NONE, CVMX_PKO_LOCK_ATOMIC_TAG, or CVMX_PKO_LOCK_CMD_QUEUE
 *
 * Return: returns CVMX_PKO_SUCCESS on success, or error code on failure of output
 */
static inline cvmx_pko_return_value_t
cvmx_hwpko_send_packet_finish3(u64 ipd_port, u64 queue, cvmx_pko_command_word0_t pko_command,
			       cvmx_buf_ptr_t packet, u64 addr, cvmx_pko_lock_t use_locking)
{
	cvmx_cmd_queue_result_t result;

	if (use_locking == CVMX_PKO_LOCK_ATOMIC_TAG)
		cvmx_pow_tag_sw_wait();

	result = cvmx_cmd_queue_write3(CVMX_CMD_QUEUE_PKO(queue),
				       (use_locking == CVMX_PKO_LOCK_CMD_QUEUE), pko_command.u64,
				       packet.u64, addr);
	if (cvmx_likely(result == CVMX_CMD_QUEUE_SUCCESS)) {
		cvmx_pko_doorbell(ipd_port, queue, 3);
		return CVMX_PKO_SUCCESS;
	} else if ((result == CVMX_CMD_QUEUE_NO_MEMORY) || (result == CVMX_CMD_QUEUE_FULL)) {
		return CVMX_PKO_NO_MEMORY;
	} else {
		return CVMX_PKO_INVALID_QUEUE;
	}
}

/**
 * Get the first pko_port for the (interface, index)
 *
 * @param interface
 * @param index
 */
int cvmx_pko_get_base_pko_port(int interface, int index);

/**
 * Get the number of pko_ports for the (interface, index)
 *
 * @param interface
 * @param index
 */
int cvmx_pko_get_num_pko_ports(int interface, int index);

/**
 * For a given port number, return the base pko output queue
 * for the port.
 *
 * @param port   IPD port number
 * Return: Base output queue
 */
int cvmx_pko_get_base_queue(int port);

/**
 * For a given port number, return the number of pko output queues.
 *
 * @param port   IPD port number
 * Return: Number of output queues
 */
int cvmx_pko_get_num_queues(int port);

/**
 * Sets the internal FPA pool data structure for PKO comamnd queue.
 * @param pool	fpa pool number yo use
 * @param buffer_size	buffer size of pool
 * @param buffer_count	number of buufers to allocate to pool
 *
 * @note the caller is responsable for setting up the pool with
 * an appropriate buffer size and sufficient buffer count.
 */
void cvmx_pko_set_cmd_que_pool_config(s64 pool, u64 buffer_size, u64 buffer_count);

/**
 * Get the status counters for a port.
 *
 * @param ipd_port Port number (ipd_port) to get statistics for.
 * @param clear    Set to 1 to clear the counters after they are read
 * @param status   Where to put the results.
 *
 * Note:
 *     - Only the doorbell for the base queue of the ipd_port is
 *       collected.
 *     - Retrieving the stats involves writing the index through
 *       CVMX_PKO_REG_READ_IDX and reading the stat CSRs, in that
 *       order. It is not MP-safe and caller should guarantee
 *       atomicity.
 */
void cvmx_pko_get_port_status(u64 ipd_port, u64 clear, cvmx_pko_port_status_t *status);

/**
 * Rate limit a PKO port to a max packets/sec. This function is only
 * supported on CN57XX, CN56XX, CN55XX, and CN54XX.
 *
 * @param port      Port to rate limit
 * @param packets_s Maximum packet/sec
 * @param burst     Maximum number of packets to burst in a row before rate
 *                  limiting cuts in.
 *
 * Return: Zero on success, negative on failure
 */
int cvmx_pko_rate_limit_packets(int port, int packets_s, int burst);

/**
 * Rate limit a PKO port to a max bits/sec. This function is only
 * supported on CN57XX, CN56XX, CN55XX, and CN54XX.
 *
 * @param port   Port to rate limit
 * @param bits_s PKO rate limit in bits/sec
 * @param burst  Maximum number of bits to burst before rate
 *               limiting cuts in.
 *
 * Return: Zero on success, negative on failure
 */
int cvmx_pko_rate_limit_bits(int port, u64 bits_s, int burst);

/**
 * @INTERNAL
 *
 * Retrieve the PKO pipe number for a port
 *
 * @param interface
 * @param index
 *
 * Return: negative on error.
 *
 * This applies only to the non-loopback interfaces.
 *
 */
int __cvmx_pko_get_pipe(int interface, int index);

/**
 * For a given PKO port number, return the base output queue
 * for the port.
 *
 * @param pko_port   PKO port number
 * Return:           Base output queue
 */
int cvmx_pko_get_base_queue_pkoid(int pko_port);

/**
 * For a given PKO port number, return the number of output queues
 * for the port.
 *
 * @param pko_port	PKO port number
 * Return:		the number of output queues
 */
int cvmx_pko_get_num_queues_pkoid(int pko_port);

/**
 * Ring the packet output doorbell. This tells the packet
 * output hardware that "len" command words have been added
 * to its pending list.  This command includes the required
 * CVMX_SYNCWS before the doorbell ring.
 *
 * @param pko_port   Port the packet is for
 * @param queue  Queue the packet is for
 * @param len    Length of the command in 64 bit words
 */
static inline void cvmx_pko_doorbell_pkoid(u64 pko_port, u64 queue, u64 len)
{
	cvmx_pko_doorbell_address_t ptr;

	ptr.u64 = 0;
	ptr.s.mem_space = CVMX_IO_SEG;
	ptr.s.did = CVMX_OCT_DID_PKT_SEND;
	ptr.s.is_io = 1;
	ptr.s.port = pko_port;
	ptr.s.queue = queue;
	/* Need to make sure output queue data is in DRAM before doorbell write */
	CVMX_SYNCWS;
	cvmx_write_io(ptr.u64, len);
}

/**
 * Complete packet output. cvmx_pko_send_packet_prepare() must be called exactly once before this,
 * and the same parameters must be passed to both cvmx_pko_send_packet_prepare() and
 * cvmx_pko_send_packet_finish_pkoid().
 *
 * @param pko_port   Port to send it on
 * @param queue  Queue to use
 * @param pko_command
 *               PKO HW command word
 * @param packet Packet to send
 * @param use_locking
 *               CVMX_PKO_LOCK_NONE, CVMX_PKO_LOCK_ATOMIC_TAG, or CVMX_PKO_LOCK_CMD_QUEUE
 *
 * Return: returns CVMX_PKO_SUCCESS on success, or error code on failure of output
 */
static inline cvmx_pko_return_value_t
cvmx_hwpko_send_packet_finish_pkoid(int pko_port, u64 queue, cvmx_pko_command_word0_t pko_command,
				    cvmx_buf_ptr_t packet, cvmx_pko_lock_t use_locking)
{
	cvmx_cmd_queue_result_t result;

	if (use_locking == CVMX_PKO_LOCK_ATOMIC_TAG)
		cvmx_pow_tag_sw_wait();

	result = cvmx_cmd_queue_write2(CVMX_CMD_QUEUE_PKO(queue),
				       (use_locking == CVMX_PKO_LOCK_CMD_QUEUE), pko_command.u64,
				       packet.u64);
	if (cvmx_likely(result == CVMX_CMD_QUEUE_SUCCESS)) {
		cvmx_pko_doorbell_pkoid(pko_port, queue, 2);
		return CVMX_PKO_SUCCESS;
	} else if ((result == CVMX_CMD_QUEUE_NO_MEMORY) || (result == CVMX_CMD_QUEUE_FULL)) {
		return CVMX_PKO_NO_MEMORY;
	} else {
		return CVMX_PKO_INVALID_QUEUE;
	}
}

/**
 * Complete packet output. cvmx_pko_send_packet_prepare() must be called exactly once before this,
 * and the same parameters must be passed to both cvmx_pko_send_packet_prepare() and
 * cvmx_pko_send_packet_finish_pkoid().
 *
 * @param pko_port   The PKO port the packet is for
 * @param queue  Queue to use
 * @param pko_command
 *               PKO HW command word
 * @param packet Packet to send
 * @param addr   Plysical address of a work queue entry or physical address to zero on complete.
 * @param use_locking
 *               CVMX_PKO_LOCK_NONE, CVMX_PKO_LOCK_ATOMIC_TAG, or CVMX_PKO_LOCK_CMD_QUEUE
 *
 * Return: returns CVMX_PKO_SUCCESS on success, or error code on failure of output
 */
static inline cvmx_pko_return_value_t
cvmx_hwpko_send_packet_finish3_pkoid(u64 pko_port, u64 queue, cvmx_pko_command_word0_t pko_command,
				     cvmx_buf_ptr_t packet, u64 addr, cvmx_pko_lock_t use_locking)
{
	cvmx_cmd_queue_result_t result;

	if (use_locking == CVMX_PKO_LOCK_ATOMIC_TAG)
		cvmx_pow_tag_sw_wait();

	result = cvmx_cmd_queue_write3(CVMX_CMD_QUEUE_PKO(queue),
				       (use_locking == CVMX_PKO_LOCK_CMD_QUEUE), pko_command.u64,
				       packet.u64, addr);
	if (cvmx_likely(result == CVMX_CMD_QUEUE_SUCCESS)) {
		cvmx_pko_doorbell_pkoid(pko_port, queue, 3);
		return CVMX_PKO_SUCCESS;
	} else if ((result == CVMX_CMD_QUEUE_NO_MEMORY) || (result == CVMX_CMD_QUEUE_FULL)) {
		return CVMX_PKO_NO_MEMORY;
	} else {
		return CVMX_PKO_INVALID_QUEUE;
	}
}

/*
 * Obtain the number of PKO commands pending in a queue
 *
 * @param queue is the queue identifier to be queried
 * Return: the number of commands pending transmission or -1 on error
 */
int cvmx_pko_queue_pend_count(cvmx_cmd_queue_id_t queue);

void cvmx_pko_set_cmd_queue_pool_buffer_count(u64 buffer_count);

#endif /* __CVMX_HWPKO_H__ */
