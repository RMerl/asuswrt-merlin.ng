/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Freescale Semiconductor
 */

#ifndef _FSL_QBMAN_PORTAL_H
#define _FSL_QBMAN_PORTAL_H

#include <fsl-mc/fsl_qbman_base.h>

/* Create and destroy a functional object representing the given QBMan portal
 * descriptor. */
struct qbman_swp *qbman_swp_init(const struct qbman_swp_desc *);

	/************/
	/* Dequeues */
	/************/

/* See the QBMan driver API documentation for details on the enqueue
 * mechanisms. NB: the use of a 'ldpaa_' prefix for this type is because it is
 * primarily used by the "DPIO" layer that sits above (and hides) the QBMan
 * driver. The structure is defined in the DPIO interface, but to avoid circular
 * dependencies we just pre/re-declare it here opaquely. */
struct ldpaa_dq;


/* ------------------- */
/* Pull-mode dequeuing */
/* ------------------- */

struct qbman_pull_desc {
	uint32_t dont_manipulate_directly[6];
};

/* Clear the contents of a descriptor to default/starting state. */
void qbman_pull_desc_clear(struct qbman_pull_desc *);
/* If not called, or if called with 'storage' as NULL, the result pull dequeues
 * will produce results to DQRR. If 'storage' is non-NULL, then results are
 * produced to the given memory location (using the physical/DMA address which
 * the caller provides in 'storage_phys'), and 'stash' controls whether or not
 * those writes to main-memory express a cache-warming attribute. */
void qbman_pull_desc_set_storage(struct qbman_pull_desc *,
				 struct ldpaa_dq *storage,
				 dma_addr_t storage_phys,
				 int stash);
/* numframes must be between 1 and 16, inclusive */
void qbman_pull_desc_set_numframes(struct qbman_pull_desc *, uint8_t numframes);
/* token is the value that shows up in the dequeue results that can be used to
 * detect when the results have been published, and is not really used when
 * dequeue results go to DQRR. The easiest technique is to zero result "storage"
 * before issuing a pull dequeue, and use any non-zero 'token' value. */
void qbman_pull_desc_set_token(struct qbman_pull_desc *, uint8_t token);
/* Exactly one of the following descriptor "actions" should be set. (Calling any
 * one of these will replace the effect of any prior call to one of these.)
 * - pull dequeue from the given frame queue (FQ)
 * - pull dequeue from any FQ in the given work queue (WQ)
 * - pull dequeue from any FQ in any WQ in the given channel
 */
void qbman_pull_desc_set_fq(struct qbman_pull_desc *, uint32_t fqid);

/* Issue the pull dequeue command */
int qbman_swp_pull(struct qbman_swp *, struct qbman_pull_desc *);

/* -------------------------------- */
/* Polling DQRR for dequeue results */
/* -------------------------------- */

/* NULL return if there are no unconsumed DQRR entries. Returns a DQRR entry
 * only once, so repeated calls can return a sequence of DQRR entries, without
 * requiring they be consumed immediately or in any particular order. */
const struct ldpaa_dq *qbman_swp_dqrr_next(struct qbman_swp *);
/* Consume DQRR entries previously returned from qbman_swp_dqrr_next(). */
void qbman_swp_dqrr_consume(struct qbman_swp *, const struct ldpaa_dq *);

/* ------------------------------------------------- */
/* Polling user-provided storage for dequeue results */
/* ------------------------------------------------- */

/* Only used for user-provided storage of dequeue results, not DQRR. Prior to
 * being used, the storage must set "oldtoken", so that the driver notices when
 * hardware has filled it in with results using a "newtoken". NB, for efficiency
 * purposes, the driver will perform any required endianness conversion to
 * ensure that the user's dequeue result storage is in host-endian format
 * (whether or not that is the same as the little-endian format that hardware
 * DMA'd to the user's storage). As such, once the user has called
 * qbman_dq_entry_has_newtoken() and been returned a valid dequeue result, they
 * should not call it again on the same memory location (except of course if
 * another dequeue command has been executed to produce a new result to that
 * location).
 */
void qbman_dq_entry_set_oldtoken(struct ldpaa_dq *,
				 unsigned int num_entries,
				 uint8_t oldtoken);
int qbman_dq_entry_has_newtoken(struct qbman_swp *,
				const struct ldpaa_dq *,
				uint8_t newtoken);

/* -------------------------------------------------------- */
/* Parsing dequeue entries (DQRR and user-provided storage) */
/* -------------------------------------------------------- */

/* DQRR entries may contain non-dequeue results, ie. notifications */
int qbman_dq_entry_is_DQ(const struct ldpaa_dq *);

	/************/
	/* Enqueues */
	/************/

struct qbman_eq_desc {
	uint32_t dont_manipulate_directly[8];
};


/* Clear the contents of a descriptor to default/starting state. */
void qbman_eq_desc_clear(struct qbman_eq_desc *);
/* Exactly one of the following descriptor "actions" should be set. (Calling
 * any one of these will replace the effect of any prior call to one of these.)
 * - enqueue without order-restoration
 * - enqueue with order-restoration
 * - fill a hole in the order-restoration sequence, without any enqueue
 * - advance NESN (Next Expected Sequence Number), without any enqueue
 * 'respond_success' indicates whether an enqueue response should be DMA'd
 * after success (otherwise a response is DMA'd only after failure).
 * 'incomplete' indicates that other fragments of the same 'seqnum' are yet to
 * be enqueued.
 */
void qbman_eq_desc_set_no_orp(struct qbman_eq_desc *, int respond_success);
void qbman_eq_desc_set_response(struct qbman_eq_desc *,
				dma_addr_t storage_phys,
				int stash);
/* token is the value that shows up in an enqueue response that can be used to
 * detect when the results have been published. The easiest technique is to zero
 * result "storage" before issuing an enqueue, and use any non-zero 'token'
 * value. */
void qbman_eq_desc_set_token(struct qbman_eq_desc *, uint8_t token);
/* Exactly one of the following descriptor "targets" should be set. (Calling any
 * one of these will replace the effect of any prior call to one of these.)
 * - enqueue to a frame queue
 * - enqueue to a queuing destination
 * Note, that none of these will have any affect if the "action" type has been
 * set to "orp_hole" or "orp_nesn".
 */
void qbman_eq_desc_set_fq(struct qbman_eq_desc *, uint32_t fqid);
void qbman_eq_desc_set_qd(struct qbman_eq_desc *, uint32_t qdid,
			  uint32_t qd_bin, uint32_t qd_prio);

/* Issue an enqueue command. ('fd' should only be NULL if the "action" of the
 * descriptor is "orp_hole" or "orp_nesn".) */
int qbman_swp_enqueue(struct qbman_swp *, const struct qbman_eq_desc *,
		      const struct qbman_fd *fd);

	/*******************/
	/* Buffer releases */
	/*******************/

struct qbman_release_desc {
	uint32_t dont_manipulate_directly[1];
};

/* Clear the contents of a descriptor to default/starting state. */
void qbman_release_desc_clear(struct qbman_release_desc *);
/* Set the ID of the buffer pool to release to */
void qbman_release_desc_set_bpid(struct qbman_release_desc *, uint32_t bpid);
/* Issue a release command. 'num_buffers' must be less than 8. */
int qbman_swp_release(struct qbman_swp *, const struct qbman_release_desc *,
		      const uint64_t *buffers, unsigned int num_buffers);

	/*******************/
	/* Buffer acquires */
	/*******************/

int qbman_swp_acquire(struct qbman_swp *, uint32_t bpid, uint64_t *buffers,
		      unsigned int num_buffers);
#endif /* !_FSL_QBMAN_PORTAL_H */
