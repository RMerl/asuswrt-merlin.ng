/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Backward compatibility for packet transmission using legacy PKO command.
 */

#ifndef __CVMX_PKO_H__
#define __CVMX_PKO_H__

extern cvmx_pko_return_value_t
cvmx_pko3_legacy_xmit(unsigned int dq, cvmx_pko_command_word0_t pko_command,
		      cvmx_buf_ptr_t packet, uint64_t addr, bool tag_sw);

/**
 * Complete packet output. cvmx_pko_send_packet_prepare() must be called exactly
 * once before this, and the same parameters must be passed to both
 * cvmx_pko_send_packet_prepare() and cvmx_pko_send_packet_finish().
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
 * @param packet to send
 * @param use_locking
 *               CVMX_PKO_LOCK_NONE, CVMX_PKO_LOCK_ATOMIC_TAG,
 *               or CVMX_PKO_LOCK_CMD_QUEUE
 *
 * @return returns CVMX_PKO_SUCCESS on success, or error code on failure of output
 */
static inline cvmx_pko_return_value_t
cvmx_pko_send_packet_finish(u64 ipd_port, uint64_t queue,
			    cvmx_pko_command_word0_t pko_command,
			    cvmx_buf_ptr_t packet, cvmx_pko_lock_t use_locking)
{
	cvmx_cmd_queue_result_t result;

	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		return cvmx_pko3_legacy_xmit(queue, pko_command, packet, 0,
					     use_locking ==
						     CVMX_PKO_LOCK_ATOMIC_TAG);
	}

	if (use_locking == CVMX_PKO_LOCK_ATOMIC_TAG)
		cvmx_pow_tag_sw_wait();

	result = cvmx_cmd_queue_write2(CVMX_CMD_QUEUE_PKO(queue),
				       (use_locking == CVMX_PKO_LOCK_CMD_QUEUE),
				       pko_command.u64, packet.u64);
	if (cvmx_likely(result == CVMX_CMD_QUEUE_SUCCESS)) {
		cvmx_pko_doorbell(ipd_port, queue, 2);
		return CVMX_PKO_SUCCESS;
	} else if ((result == CVMX_CMD_QUEUE_NO_MEMORY) ||
		   (result == CVMX_CMD_QUEUE_FULL)) {
		return CVMX_PKO_NO_MEMORY;
	} else {
		return CVMX_PKO_INVALID_QUEUE;
	}
}

/**
 * Complete packet output. cvmx_pko_send_packet_prepare() must be called exactly
 * once before this, and the same parameters must be passed to both
 * cvmx_pko_send_packet_prepare() and cvmx_pko_send_packet_finish().
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
 * @param packet to send
 * @param addr   Physical address of a work queue entry or physical address to zero
 *               on complete.
 * @param use_locking
 *               CVMX_PKO_LOCK_NONE, CVMX_PKO_LOCK_ATOMIC_TAG,
 *               or CVMX_PKO_LOCK_CMD_QUEUE
 *
 * @return returns CVMX_PKO_SUCCESS on success, or error code on failure of output
 */
static inline cvmx_pko_return_value_t
cvmx_pko_send_packet_finish3(u64 ipd_port, uint64_t queue,
			     cvmx_pko_command_word0_t pko_command,
			     cvmx_buf_ptr_t packet, uint64_t addr,
			     cvmx_pko_lock_t use_locking)
{
	cvmx_cmd_queue_result_t result;

	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		return cvmx_pko3_legacy_xmit(queue, pko_command, packet, addr,
					     use_locking ==
						     CVMX_PKO_LOCK_ATOMIC_TAG);
	}

	if (use_locking == CVMX_PKO_LOCK_ATOMIC_TAG)
		cvmx_pow_tag_sw_wait();

	result = cvmx_cmd_queue_write3(CVMX_CMD_QUEUE_PKO(queue),
				       (use_locking == CVMX_PKO_LOCK_CMD_QUEUE),
				       pko_command.u64, packet.u64, addr);
	if (cvmx_likely(result == CVMX_CMD_QUEUE_SUCCESS)) {
		cvmx_pko_doorbell(ipd_port, queue, 3);
		return CVMX_PKO_SUCCESS;
	} else if ((result == CVMX_CMD_QUEUE_NO_MEMORY) ||
		   (result == CVMX_CMD_QUEUE_FULL)) {
		return CVMX_PKO_NO_MEMORY;
	} else {
		return CVMX_PKO_INVALID_QUEUE;
	}
}

/**
 * Complete packet output. cvmx_pko_send_packet_prepare() must be called exactly
 * once before this, and the same parameters must be passed to both
 * cvmx_pko_send_packet_prepare() and cvmx_pko_send_packet_finish_pkoid().
 *
 * @param pko_port   Port to send it on
 * @param queue  Queue to use
 * @param pko_command
 *               PKO HW command word
 * @param packet to send
 * @param use_locking
 *               CVMX_PKO_LOCK_NONE, CVMX_PKO_LOCK_ATOMIC_TAG,
 *               or CVMX_PKO_LOCK_CMD_QUEUE
 *
 * @return returns CVMX_PKO_SUCCESS on success, or error code on failure of output
 */
static inline cvmx_pko_return_value_t
cvmx_pko_send_packet_finish_pkoid(int pko_port, uint64_t queue,
				  cvmx_pko_command_word0_t pko_command,
				  cvmx_buf_ptr_t packet, cvmx_pko_lock_t use_locking)
{
	cvmx_cmd_queue_result_t result;

	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		return cvmx_pko3_legacy_xmit(queue, pko_command, packet, 0,
					     use_locking ==
						     CVMX_PKO_LOCK_ATOMIC_TAG);
	}

	if (use_locking == CVMX_PKO_LOCK_ATOMIC_TAG)
		cvmx_pow_tag_sw_wait();
	result = cvmx_cmd_queue_write2(CVMX_CMD_QUEUE_PKO(queue),
				       (use_locking == CVMX_PKO_LOCK_CMD_QUEUE),
				       pko_command.u64, packet.u64);
	if (cvmx_likely(result == CVMX_CMD_QUEUE_SUCCESS)) {
		cvmx_pko_doorbell_pkoid(pko_port, queue, 2);
		return CVMX_PKO_SUCCESS;
	} else if ((result == CVMX_CMD_QUEUE_NO_MEMORY) ||
		   (result == CVMX_CMD_QUEUE_FULL)) {
		return CVMX_PKO_NO_MEMORY;
	} else {
		return CVMX_PKO_INVALID_QUEUE;
	}
}

/**
 * Complete packet output. cvmx_pko_send_packet_prepare() must be called exactly
 * once before this, and the same parameters must be passed to both
 * cvmx_pko_send_packet_prepare() and cvmx_pko_send_packet_finish_pkoid().
 *
 * @param pko_port   The PKO port the packet is for
 * @param queue  Queue to use
 * @param pko_command
 *               PKO HW command word
 * @param packet to send
 * @param addr   Plysical address of a work queue entry or physical address to zero
 *               on complete.
 * @param use_locking
 *               CVMX_PKO_LOCK_NONE, CVMX_PKO_LOCK_ATOMIC_TAG,
 *               or CVMX_PKO_LOCK_CMD_QUEUE
 *
 * @return returns CVMX_PKO_SUCCESS on success, or error code on failure of output
 */
static inline cvmx_pko_return_value_t
cvmx_pko_send_packet_finish3_pkoid(u64 pko_port, uint64_t queue,
				   cvmx_pko_command_word0_t pko_command,
				   cvmx_buf_ptr_t packet, uint64_t addr,
				   cvmx_pko_lock_t use_locking)
{
	cvmx_cmd_queue_result_t result;

	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		return cvmx_pko3_legacy_xmit(queue, pko_command, packet, addr,
					     use_locking ==
						     CVMX_PKO_LOCK_ATOMIC_TAG);
	}

	if (use_locking == CVMX_PKO_LOCK_ATOMIC_TAG)
		cvmx_pow_tag_sw_wait();
	result = cvmx_cmd_queue_write3(CVMX_CMD_QUEUE_PKO(queue),
				       (use_locking == CVMX_PKO_LOCK_CMD_QUEUE),
				       pko_command.u64, packet.u64, addr);
	if (cvmx_likely(result == CVMX_CMD_QUEUE_SUCCESS)) {
		cvmx_pko_doorbell_pkoid(pko_port, queue, 3);
		return CVMX_PKO_SUCCESS;
	} else if ((result == CVMX_CMD_QUEUE_NO_MEMORY) ||
		   (result == CVMX_CMD_QUEUE_FULL)) {
		return CVMX_PKO_NO_MEMORY;
	} else {
		return CVMX_PKO_INVALID_QUEUE;
	}
}

#endif /* __CVMX_PKO_H__ */
