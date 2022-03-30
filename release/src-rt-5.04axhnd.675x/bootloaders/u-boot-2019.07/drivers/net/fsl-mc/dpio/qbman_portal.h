/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Freescale Semiconductor
 */

#include "qbman_private.h"
#include <fsl-mc/fsl_qbman_portal.h>
#include <fsl-mc/fsl_dpaa_fd.h>

/* All QBMan command and result structures use this "valid bit" encoding */
#define QB_VALID_BIT ((uint32_t)0x80)

/* Management command result codes */
#define QBMAN_MC_RSLT_OK      0xf0

#define QBMAN_VER_4_0_DQRR_SIZE 4
#define QBMAN_VER_4_1_DQRR_SIZE 8


/* --------------------- */
/* portal data structure */
/* --------------------- */

struct qbman_swp {
	const struct qbman_swp_desc *desc;
	/* The qbman_sys (ie. arch/OS-specific) support code can put anything it
	 * needs in here. */
	struct qbman_swp_sys sys;
	/* Management commands */
	struct {
#ifdef QBMAN_CHECKING
		enum swp_mc_check {
			swp_mc_can_start, /* call __qbman_swp_mc_start() */
			swp_mc_can_submit, /* call __qbman_swp_mc_submit() */
			swp_mc_can_poll, /* call __qbman_swp_mc_result() */
		} check;
#endif
		uint32_t valid_bit; /* 0x00 or 0x80 */
	} mc;
	/* Push dequeues */
	uint32_t sdq;
	/* Volatile dequeues */
	struct {
		/* VDQCR supports a "1 deep pipeline", meaning that if you know
		 * the last-submitted command is already executing in the
		 * hardware (as evidenced by at least 1 valid dequeue result),
		 * you can write another dequeue command to the register, the
		 * hardware will start executing it as soon as the
		 * already-executing command terminates. (This minimises latency
		 * and stalls.) With that in mind, this "busy" variable refers
		 * to whether or not a command can be submitted, not whether or
		 * not a previously-submitted command is still executing. In
		 * other words, once proof is seen that the previously-submitted
		 * command is executing, "vdq" is no longer "busy".
		 */
		atomic_t busy;
		uint32_t valid_bit; /* 0x00 or 0x80 */
		/* We need to determine when vdq is no longer busy. This depends
		 * on whether the "busy" (last-submitted) dequeue command is
		 * targeting DQRR or main-memory, and detected is based on the
		 * presence of the dequeue command's "token" showing up in
		 * dequeue entries in DQRR or main-memory (respectively). Debug
		 * builds will, when submitting vdq commands, verify that the
		 * dequeue result location is not already equal to the command's
		 * token value. */
		struct ldpaa_dq *storage; /* NULL if DQRR */
		uint32_t token;
	} vdq;
	/* DQRR */
	struct {
		uint32_t next_idx;
		uint32_t valid_bit;
		uint8_t dqrr_size;
	} dqrr;
};

/* -------------------------- */
/* portal management commands */
/* -------------------------- */

/* Different management commands all use this common base layer of code to issue
 * commands and poll for results. The first function returns a pointer to where
 * the caller should fill in their MC command (though they should ignore the
 * verb byte), the second function commits merges in the caller-supplied command
 * verb (which should not include the valid-bit) and submits the command to
 * hardware, and the third function checks for a completed response (returns
 * non-NULL if only if the response is complete). */
void *qbman_swp_mc_start(struct qbman_swp *p);
void qbman_swp_mc_submit(struct qbman_swp *p, void *cmd, uint32_t cmd_verb);
void *qbman_swp_mc_result(struct qbman_swp *p);

/* Wraps up submit + poll-for-result */
static inline void *qbman_swp_mc_complete(struct qbman_swp *swp, void *cmd,
					  uint32_t cmd_verb)
{
	int loopvar;

	qbman_swp_mc_submit(swp, cmd, cmd_verb);
	DBG_POLL_START(loopvar);
	do {
		DBG_POLL_CHECK(loopvar);
		cmd = qbman_swp_mc_result(swp);
	} while (!cmd);
	return cmd;
}

/* ------------ */
/* qb_attr_code */
/* ------------ */

/* This struct locates a sub-field within a QBMan portal (CENA) cacheline which
 * is either serving as a configuration command or a query result. The
 * representation is inherently little-endian, as the indexing of the words is
 * itself little-endian in nature and layerscape is little endian for anything
 * that crosses a word boundary too (64-bit fields are the obvious examples).
 */
struct qb_attr_code {
	unsigned int word; /* which uint32_t[] array member encodes the field */
	unsigned int lsoffset; /* encoding offset from ls-bit */
	unsigned int width; /* encoding width. (bool must be 1.) */
};

/* Macros to define codes */
#define QB_CODE(a, b, c) { a, b, c}

/* decode a field from a cacheline */
static inline uint32_t qb_attr_code_decode(const struct qb_attr_code *code,
				      const uint32_t *cacheline)
{
	return d32_uint32_t(code->lsoffset, code->width, cacheline[code->word]);
}


/* encode a field to a cacheline */
static inline void qb_attr_code_encode(const struct qb_attr_code *code,
				       uint32_t *cacheline, uint32_t val)
{
	cacheline[code->word] =
		r32_uint32_t(code->lsoffset, code->width, cacheline[code->word])
		| e32_uint32_t(code->lsoffset, code->width, val);
}

static inline void qb_attr_code_encode_64(const struct qb_attr_code *code,
				       uint64_t *cacheline, uint64_t val)
{
	cacheline[code->word / 2] = val;
}

/* ---------------------- */
/* Descriptors/cachelines */
/* ---------------------- */

/* To avoid needless dynamic allocation, the driver API often gives the caller
 * a "descriptor" type that the caller can instantiate however they like.
 * Ultimately though, it is just a cacheline of binary storage (or something
 * smaller when it is known that the descriptor doesn't need all 64 bytes) for
 * holding pre-formatted pieces of hardware commands. The performance-critical
 * code can then copy these descriptors directly into hardware command
 * registers more efficiently than trying to construct/format commands
 * on-the-fly. The API user sees the descriptor as an array of 32-bit words in
 * order for the compiler to know its size, but the internal details are not
 * exposed. The following macro is used within the driver for converting *any*
 * descriptor pointer to a usable array pointer. The use of a macro (instead of
 * an inline) is necessary to work with different descriptor types and to work
 * correctly with const and non-const inputs (and similarly-qualified outputs).
 */
#define qb_cl(d) (&(d)->dont_manipulate_directly[0])
