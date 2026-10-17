// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Freescale Semiconductor
 */

#include <log.h>
#include <malloc.h>
#include <asm/arch/clock.h>
#include <linux/bug.h>
#include "qbman_portal.h"

/* QBMan portal management command codes */
#define QBMAN_MC_ACQUIRE       0x30
#define QBMAN_WQCHAN_CONFIGURE 0x46

/* CINH register offsets */
#define QBMAN_CINH_SWP_EQAR    0x8c0
#define QBMAN_CINH_SWP_DCAP    0xac0
#define QBMAN_CINH_SWP_SDQCR   0xb00
#define QBMAN_CINH_SWP_RAR     0xcc0

/* CENA register offsets */
#define QBMAN_CENA_SWP_EQCR(n) (0x000 + ((uint32_t)(n) << 6))
#define QBMAN_CENA_SWP_DQRR(n) (0x200 + ((uint32_t)(n) << 6))
#define QBMAN_CENA_SWP_RCR(n)  (0x400 + ((uint32_t)(n) << 6))
#define QBMAN_CENA_SWP_CR      0x600
#define QBMAN_CENA_SWP_RR(vb)  (0x700 + ((uint32_t)(vb) >> 1))
#define QBMAN_CENA_SWP_VDQCR   0x780

/* Reverse mapping of QBMAN_CENA_SWP_DQRR() */
#define QBMAN_IDX_FROM_DQRR(p) (((unsigned long)p & 0x1ff) >> 6)

/*******************************/
/* Pre-defined attribute codes */
/*******************************/

struct qb_attr_code code_generic_verb = QB_CODE(0, 0, 7);
struct qb_attr_code code_generic_rslt = QB_CODE(0, 8, 8);

/*************************/
/* SDQCR attribute codes */
/*************************/

/* we put these here because at least some of them are required by
 * qbman_swp_init() */
struct qb_attr_code code_sdqcr_dct = QB_CODE(0, 24, 2);
struct qb_attr_code code_sdqcr_fc = QB_CODE(0, 29, 1);
struct qb_attr_code code_sdqcr_tok = QB_CODE(0, 16, 8);
#define CODE_SDQCR_DQSRC(n) QB_CODE(0, n, 1)
enum qbman_sdqcr_dct {
	qbman_sdqcr_dct_null = 0,
	qbman_sdqcr_dct_prio_ics,
	qbman_sdqcr_dct_active_ics,
	qbman_sdqcr_dct_active
};
enum qbman_sdqcr_fc {
	qbman_sdqcr_fc_one = 0,
	qbman_sdqcr_fc_up_to_3 = 1
};

/*********************************/
/* Portal constructor/destructor */
/*********************************/

/* Software portals should always be in the power-on state when we initialise,
 * due to the CCSR-based portal reset functionality that MC has. */
struct qbman_swp *qbman_swp_init(const struct qbman_swp_desc *d)
{
	int ret;
	struct qbman_swp *p = malloc(sizeof(struct qbman_swp));
	u32 major = 0, minor = 0;

	if (!p)
		return NULL;
	p->desc = d;
#ifdef QBMAN_CHECKING
	p->mc.check = swp_mc_can_start;
#endif
	p->mc.valid_bit = QB_VALID_BIT;
	p->sdq = 0;
	qb_attr_code_encode(&code_sdqcr_dct, &p->sdq, qbman_sdqcr_dct_prio_ics);
	qb_attr_code_encode(&code_sdqcr_fc, &p->sdq, qbman_sdqcr_fc_up_to_3);
	qb_attr_code_encode(&code_sdqcr_tok, &p->sdq, 0xbb);
	atomic_set(&p->vdq.busy, 1);
	p->vdq.valid_bit = QB_VALID_BIT;
	p->dqrr.next_idx = 0;

	qbman_version(&major, &minor);
	if (!major) {
		printf("invalid qbman version\n");
		return NULL;
	}

	if (major >= 4 && minor >= 1)
		p->dqrr.dqrr_size = QBMAN_VER_4_1_DQRR_SIZE;
	else
		p->dqrr.dqrr_size = QBMAN_VER_4_0_DQRR_SIZE;

	p->dqrr.valid_bit = QB_VALID_BIT;
	ret = qbman_swp_sys_init(&p->sys, d, p->dqrr.dqrr_size);
	if (ret) {
		free(p);
		printf("qbman_swp_sys_init() failed %d\n", ret);
		return NULL;
	}
	qbman_cinh_write(&p->sys, QBMAN_CINH_SWP_SDQCR, p->sdq);
	return p;
}

/***********************/
/* Management commands */
/***********************/

/*
 * Internal code common to all types of management commands.
 */

void *qbman_swp_mc_start(struct qbman_swp *p)
{
	void *ret;
	int *return_val;
#ifdef QBMAN_CHECKING
	BUG_ON(p->mc.check != swp_mc_can_start);
#endif
	ret = qbman_cena_write_start(&p->sys, QBMAN_CENA_SWP_CR);
#ifdef QBMAN_CHECKING
	return_val = (int *)ret;
	if (!(*return_val))
		p->mc.check = swp_mc_can_submit;
#endif
	return ret;
}

void qbman_swp_mc_submit(struct qbman_swp *p, void *cmd, uint32_t cmd_verb)
{
	uint32_t *v = cmd;
#ifdef QBMAN_CHECKING
	BUG_ON(p->mc.check != swp_mc_can_submit);
#endif
	lwsync();
	/* TBD: "|=" is going to hurt performance. Need to move as many fields
	 * out of word zero, and for those that remain, the "OR" needs to occur
	 * at the caller side. This debug check helps to catch cases where the
	 * caller wants to OR but has forgotten to do so. */
	BUG_ON((*v & cmd_verb) != *v);
	*v = cmd_verb | p->mc.valid_bit;
	qbman_cena_write_complete(&p->sys, QBMAN_CENA_SWP_CR, cmd);
	/* TODO: add prefetch support for GPP */
#ifdef QBMAN_CHECKING
	p->mc.check = swp_mc_can_poll;
#endif
}

void *qbman_swp_mc_result(struct qbman_swp *p)
{
	uint32_t *ret, verb;
#ifdef QBMAN_CHECKING
	BUG_ON(p->mc.check != swp_mc_can_poll);
#endif
	ret = qbman_cena_read(&p->sys, QBMAN_CENA_SWP_RR(p->mc.valid_bit));
	/* Remove the valid-bit - command completed iff the rest is non-zero */
	verb = ret[0] & ~QB_VALID_BIT;
	if (!verb)
		return NULL;
#ifdef QBMAN_CHECKING
	p->mc.check = swp_mc_can_start;
#endif
	p->mc.valid_bit ^= QB_VALID_BIT;
	return ret;
}

/***********/
/* Enqueue */
/***********/

/* These should be const, eventually */
static struct qb_attr_code code_eq_cmd = QB_CODE(0, 0, 2);
static struct qb_attr_code code_eq_orp_en = QB_CODE(0, 2, 1);
static struct qb_attr_code code_eq_tgt_id = QB_CODE(2, 0, 24);
/* static struct qb_attr_code code_eq_tag = QB_CODE(3, 0, 32); */
static struct qb_attr_code code_eq_qd_en = QB_CODE(0, 4, 1);
static struct qb_attr_code code_eq_qd_bin = QB_CODE(4, 0, 16);
static struct qb_attr_code code_eq_qd_pri = QB_CODE(4, 16, 4);
static struct qb_attr_code code_eq_rsp_stash = QB_CODE(5, 16, 1);
static struct qb_attr_code code_eq_rsp_lo = QB_CODE(6, 0, 32);

enum qbman_eq_cmd_e {
	/* No enqueue, primarily for plugging ORP gaps for dropped frames */
	qbman_eq_cmd_empty,
	/* DMA an enqueue response once complete */
	qbman_eq_cmd_respond,
	/* DMA an enqueue response only if the enqueue fails */
	qbman_eq_cmd_respond_reject
};

void qbman_eq_desc_clear(struct qbman_eq_desc *d)
{
	memset(d, 0, sizeof(*d));
}

void qbman_eq_desc_set_no_orp(struct qbman_eq_desc *d, int respond_success)
{
	uint32_t *cl = qb_cl(d);

	qb_attr_code_encode(&code_eq_orp_en, cl, 0);
	qb_attr_code_encode(&code_eq_cmd, cl,
			    respond_success ? qbman_eq_cmd_respond :
					      qbman_eq_cmd_respond_reject);
}

void qbman_eq_desc_set_response(struct qbman_eq_desc *d,
				dma_addr_t storage_phys,
				int stash)
{
	uint32_t *cl = qb_cl(d);

	qb_attr_code_encode_64(&code_eq_rsp_lo, (uint64_t *)cl, storage_phys);
	qb_attr_code_encode(&code_eq_rsp_stash, cl, !!stash);
}


void qbman_eq_desc_set_qd(struct qbman_eq_desc *d, uint32_t qdid,
			  uint32_t qd_bin, uint32_t qd_prio)
{
	uint32_t *cl = qb_cl(d);

	qb_attr_code_encode(&code_eq_qd_en, cl, 1);
	qb_attr_code_encode(&code_eq_tgt_id, cl, qdid);
	qb_attr_code_encode(&code_eq_qd_bin, cl, qd_bin);
	qb_attr_code_encode(&code_eq_qd_pri, cl, qd_prio);
}

#define EQAR_IDX(eqar)     ((eqar) & 0x7)
#define EQAR_VB(eqar)      ((eqar) & 0x80)
#define EQAR_SUCCESS(eqar) ((eqar) & 0x100)

int qbman_swp_enqueue(struct qbman_swp *s, const struct qbman_eq_desc *d,
		      const struct qbman_fd *fd)
{
	uint32_t *p;
	const uint32_t *cl = qb_cl(d);
	uint32_t eqar = qbman_cinh_read(&s->sys, QBMAN_CINH_SWP_EQAR);
	debug("EQAR=%08x\n", eqar);
	if (!EQAR_SUCCESS(eqar))
		return -EBUSY;
	p = qbman_cena_write_start(&s->sys,
				   QBMAN_CENA_SWP_EQCR(EQAR_IDX(eqar)));
	word_copy(&p[1], &cl[1], 7);
	word_copy(&p[8], fd, sizeof(*fd) >> 2);
	lwsync();
	/* Set the verb byte, have to substitute in the valid-bit */
	p[0] = cl[0] | EQAR_VB(eqar);
	qbman_cena_write_complete(&s->sys,
				  QBMAN_CENA_SWP_EQCR(EQAR_IDX(eqar)),
				  p);
	return 0;
}

/***************************/
/* Volatile (pull) dequeue */
/***************************/

/* These should be const, eventually */
static struct qb_attr_code code_pull_dct = QB_CODE(0, 0, 2);
static struct qb_attr_code code_pull_dt = QB_CODE(0, 2, 2);
static struct qb_attr_code code_pull_rls = QB_CODE(0, 4, 1);
static struct qb_attr_code code_pull_stash = QB_CODE(0, 5, 1);
static struct qb_attr_code code_pull_numframes = QB_CODE(0, 8, 4);
static struct qb_attr_code code_pull_token = QB_CODE(0, 16, 8);
static struct qb_attr_code code_pull_dqsource = QB_CODE(1, 0, 24);
static struct qb_attr_code code_pull_rsp_lo = QB_CODE(2, 0, 32);

enum qb_pull_dt_e {
	qb_pull_dt_channel,
	qb_pull_dt_workqueue,
	qb_pull_dt_framequeue
};

void qbman_pull_desc_clear(struct qbman_pull_desc *d)
{
	memset(d, 0, sizeof(*d));
}

void qbman_pull_desc_set_storage(struct qbman_pull_desc *d,
				 struct ldpaa_dq *storage,
				 dma_addr_t storage_phys,
				 int stash)
{
	uint32_t *cl = qb_cl(d);

	/* Squiggle the pointer 'storage' into the extra 2 words of the
	 * descriptor (which aren't copied to the hw command) */
	*(void **)&cl[4] = storage;
	if (!storage) {
		qb_attr_code_encode(&code_pull_rls, cl, 0);
		return;
	}
	qb_attr_code_encode(&code_pull_rls, cl, 1);
	qb_attr_code_encode(&code_pull_stash, cl, !!stash);
	qb_attr_code_encode_64(&code_pull_rsp_lo, (uint64_t *)cl, storage_phys);
}

void qbman_pull_desc_set_numframes(struct qbman_pull_desc *d, uint8_t numframes)
{
	uint32_t *cl = qb_cl(d);

	BUG_ON(!numframes || (numframes > 16));
	qb_attr_code_encode(&code_pull_numframes, cl,
			    (uint32_t)(numframes - 1));
}

void qbman_pull_desc_set_token(struct qbman_pull_desc *d, uint8_t token)
{
	uint32_t *cl = qb_cl(d);

	qb_attr_code_encode(&code_pull_token, cl, token);
}

void qbman_pull_desc_set_fq(struct qbman_pull_desc *d, uint32_t fqid)
{
	uint32_t *cl = qb_cl(d);

	qb_attr_code_encode(&code_pull_dct, cl, 1);
	qb_attr_code_encode(&code_pull_dt, cl, qb_pull_dt_framequeue);
	qb_attr_code_encode(&code_pull_dqsource, cl, fqid);
}

int qbman_swp_pull(struct qbman_swp *s, struct qbman_pull_desc *d)
{
	uint32_t *p;
	uint32_t *cl = qb_cl(d);

	if (!atomic_dec_and_test(&s->vdq.busy)) {
		atomic_inc(&s->vdq.busy);
		return -EBUSY;
	}
	s->vdq.storage = *(void **)&cl[4];
	s->vdq.token = qb_attr_code_decode(&code_pull_token, cl);
	p = qbman_cena_write_start(&s->sys, QBMAN_CENA_SWP_VDQCR);
	word_copy(&p[1], &cl[1], 3);
	lwsync();
	/* Set the verb byte, have to substitute in the valid-bit */
	p[0] = cl[0] | s->vdq.valid_bit;
	s->vdq.valid_bit ^= QB_VALID_BIT;
	qbman_cena_write_complete(&s->sys, QBMAN_CENA_SWP_VDQCR, p);
	return 0;
}

/****************/
/* Polling DQRR */
/****************/

static struct qb_attr_code code_dqrr_verb = QB_CODE(0, 0, 8);
static struct qb_attr_code code_dqrr_response = QB_CODE(0, 0, 7);
static struct qb_attr_code code_dqrr_stat = QB_CODE(0, 8, 8);

#define QBMAN_DQRR_RESPONSE_DQ        0x60
#define QBMAN_DQRR_RESPONSE_FQRN      0x21
#define QBMAN_DQRR_RESPONSE_FQRNI     0x22
#define QBMAN_DQRR_RESPONSE_FQPN      0x24
#define QBMAN_DQRR_RESPONSE_FQDAN     0x25
#define QBMAN_DQRR_RESPONSE_CDAN      0x26
#define QBMAN_DQRR_RESPONSE_CSCN_MEM  0x27
#define QBMAN_DQRR_RESPONSE_CGCU      0x28
#define QBMAN_DQRR_RESPONSE_BPSCN     0x29
#define QBMAN_DQRR_RESPONSE_CSCN_WQ   0x2a


/* NULL return if there are no unconsumed DQRR entries. Returns a DQRR entry
 * only once, so repeated calls can return a sequence of DQRR entries, without
 * requiring they be consumed immediately or in any particular order. */
const struct ldpaa_dq *qbman_swp_dqrr_next(struct qbman_swp *s)
{
	uint32_t verb;
	uint32_t response_verb;
	uint32_t flags;
	const struct ldpaa_dq *dq;
	const uint32_t *p;

	dq = qbman_cena_read(&s->sys, QBMAN_CENA_SWP_DQRR(s->dqrr.next_idx));
	p = qb_cl(dq);
	verb = qb_attr_code_decode(&code_dqrr_verb, p);

	/* If the valid-bit isn't of the expected polarity, nothing there. Note,
	 * in the DQRR reset bug workaround, we shouldn't need to skip these
	 * check, because we've already determined that a new entry is available
	 * and we've invalidated the cacheline before reading it, so the
	 * valid-bit behaviour is repaired and should tell us what we already
	 * knew from reading PI.
	 */
	if ((verb & QB_VALID_BIT) != s->dqrr.valid_bit) {
		qbman_cena_invalidate_prefetch(&s->sys,
					QBMAN_CENA_SWP_DQRR(s->dqrr.next_idx));
		return NULL;
	}
	/* There's something there. Move "next_idx" attention to the next ring
	 * entry (and prefetch it) before returning what we found. */
	s->dqrr.next_idx++;
	s->dqrr.next_idx &= s->dqrr.dqrr_size - 1;/* Wrap around at dqrr_size */
	/* TODO: it's possible to do all this without conditionals, optimise it
	 * later. */
	if (!s->dqrr.next_idx)
		s->dqrr.valid_bit ^= QB_VALID_BIT;

	/* If this is the final response to a volatile dequeue command
	   indicate that the vdq is no longer busy */
	flags = ldpaa_dq_flags(dq);
	response_verb = qb_attr_code_decode(&code_dqrr_response, &verb);
	if ((response_verb == QBMAN_DQRR_RESPONSE_DQ) &&
	    (flags & LDPAA_DQ_STAT_VOLATILE) &&
	    (flags & LDPAA_DQ_STAT_EXPIRED))
			atomic_inc(&s->vdq.busy);

	qbman_cena_invalidate_prefetch(&s->sys,
				       QBMAN_CENA_SWP_DQRR(s->dqrr.next_idx));
	return dq;
}

/* Consume DQRR entries previously returned from qbman_swp_dqrr_next(). */
void qbman_swp_dqrr_consume(struct qbman_swp *s, const struct ldpaa_dq *dq)
{
	qbman_cinh_write(&s->sys, QBMAN_CINH_SWP_DCAP, QBMAN_IDX_FROM_DQRR(dq));
}

/*********************************/
/* Polling user-provided storage */
/*********************************/

void qbman_dq_entry_set_oldtoken(struct ldpaa_dq *dq,
				 unsigned int num_entries,
				 uint8_t oldtoken)
{
	memset(dq, oldtoken, num_entries * sizeof(*dq));
}

int qbman_dq_entry_has_newtoken(struct qbman_swp *s,
				const struct ldpaa_dq *dq,
				uint8_t newtoken)
{
	/* To avoid converting the little-endian DQ entry to host-endian prior
	 * to us knowing whether there is a valid entry or not (and run the
	 * risk of corrupting the incoming hardware LE write), we detect in
	 * hardware endianness rather than host. This means we need a different
	 * "code" depending on whether we are BE or LE in software, which is
	 * where DQRR_TOK_OFFSET comes in... */
	static struct qb_attr_code code_dqrr_tok_detect =
					QB_CODE(0, DQRR_TOK_OFFSET, 8);
	/* The user trying to poll for a result treats "dq" as const. It is
	 * however the same address that was provided to us non-const in the
	 * first place, for directing hardware DMA to. So we can cast away the
	 * const because it is mutable from our perspective. */
	uint32_t *p = qb_cl((struct ldpaa_dq *)dq);
	uint32_t token;

	token = qb_attr_code_decode(&code_dqrr_tok_detect, &p[1]);
	if (token != newtoken)
		return 0;

	/* Only now do we convert from hardware to host endianness. Also, as we
	 * are returning success, the user has promised not to call us again, so
	 * there's no risk of us converting the endianness twice... */
	make_le32_n(p, 16);

	/* VDQCR "no longer busy" hook - not quite the same as DQRR, because the
	 * fact "VDQCR" shows busy doesn't mean that the result we're looking at
	 * is from the same command. Eg. we may be looking at our 10th dequeue
	 * result from our first VDQCR command, yet the second dequeue command
	 * could have been kicked off already, after seeing the 1st result. Ie.
	 * the result we're looking at is not necessarily proof that we can
	 * reset "busy".  We instead base the decision on whether the current
	 * result is sitting at the first 'storage' location of the busy
	 * command. */
	if (s->vdq.storage == dq) {
		s->vdq.storage = NULL;
			atomic_inc(&s->vdq.busy);
	}
	return 1;
}

/********************************/
/* Categorising dequeue entries */
/********************************/

static inline int __qbman_dq_entry_is_x(const struct ldpaa_dq *dq, uint32_t x)
{
	const uint32_t *p = qb_cl(dq);
	uint32_t response_verb = qb_attr_code_decode(&code_dqrr_response, p);

	return response_verb == x;
}

int qbman_dq_entry_is_DQ(const struct ldpaa_dq *dq)
{
	return __qbman_dq_entry_is_x(dq, QBMAN_DQRR_RESPONSE_DQ);
}

/*********************************/
/* Parsing frame dequeue results */
/*********************************/

/* These APIs assume qbman_dq_entry_is_DQ() is TRUE */

uint32_t ldpaa_dq_flags(const struct ldpaa_dq *dq)
{
	const uint32_t *p = qb_cl(dq);

	return qb_attr_code_decode(&code_dqrr_stat, p);
}

const struct dpaa_fd *ldpaa_dq_fd(const struct ldpaa_dq *dq)
{
	const uint32_t *p = qb_cl(dq);

	return (const struct dpaa_fd *)&p[8];
}

/******************/
/* Buffer release */
/******************/

/* These should be const, eventually */
/* static struct qb_attr_code code_release_num = QB_CODE(0, 0, 3); */
static struct qb_attr_code code_release_set_me = QB_CODE(0, 5, 1);
static struct qb_attr_code code_release_bpid = QB_CODE(0, 16, 16);

void qbman_release_desc_clear(struct qbman_release_desc *d)
{
	uint32_t *cl;

	memset(d, 0, sizeof(*d));
	cl = qb_cl(d);
	qb_attr_code_encode(&code_release_set_me, cl, 1);
}

void qbman_release_desc_set_bpid(struct qbman_release_desc *d, uint32_t bpid)
{
	uint32_t *cl = qb_cl(d);

	qb_attr_code_encode(&code_release_bpid, cl, bpid);
}

#define RAR_IDX(rar)     ((rar) & 0x7)
#define RAR_VB(rar)      ((rar) & 0x80)
#define RAR_SUCCESS(rar) ((rar) & 0x100)

int qbman_swp_release(struct qbman_swp *s, const struct qbman_release_desc *d,
		      const uint64_t *buffers, unsigned int num_buffers)
{
	uint32_t *p;
	const uint32_t *cl = qb_cl(d);
	uint32_t rar = qbman_cinh_read(&s->sys, QBMAN_CINH_SWP_RAR);
	debug("RAR=%08x\n", rar);
	if (!RAR_SUCCESS(rar))
		return -EBUSY;
	BUG_ON(!num_buffers || (num_buffers > 7));
	/* Start the release command */
	p = qbman_cena_write_start(&s->sys,
				   QBMAN_CENA_SWP_RCR(RAR_IDX(rar)));
	/* Copy the caller's buffer pointers to the command */
	u64_to_le32_copy(&p[2], buffers, num_buffers);
	lwsync();
	/* Set the verb byte, have to substitute in the valid-bit and the number
	 * of buffers. */
	p[0] = cl[0] | RAR_VB(rar) | num_buffers;
	qbman_cena_write_complete(&s->sys,
				  QBMAN_CENA_SWP_RCR(RAR_IDX(rar)),
				  p);
	return 0;
}

/*******************/
/* Buffer acquires */
/*******************/

/* These should be const, eventually */
static struct qb_attr_code code_acquire_bpid = QB_CODE(0, 16, 16);
static struct qb_attr_code code_acquire_num = QB_CODE(1, 0, 3);
static struct qb_attr_code code_acquire_r_num = QB_CODE(1, 0, 3);

int qbman_swp_acquire(struct qbman_swp *s, uint32_t bpid, uint64_t *buffers,
		      unsigned int num_buffers)
{
	uint32_t *p;
	uint32_t verb, rslt, num;

	BUG_ON(!num_buffers || (num_buffers > 7));

	/* Start the management command */
	p = qbman_swp_mc_start(s);

	if (!p)
		return -EBUSY;

	/* Encode the caller-provided attributes */
	qb_attr_code_encode(&code_acquire_bpid, p, bpid);
	qb_attr_code_encode(&code_acquire_num, p, num_buffers);

	/* Complete the management command */
	p = qbman_swp_mc_complete(s, p, p[0] | QBMAN_MC_ACQUIRE);

	/* Decode the outcome */
	verb = qb_attr_code_decode(&code_generic_verb, p);
	rslt = qb_attr_code_decode(&code_generic_rslt, p);
	num = qb_attr_code_decode(&code_acquire_r_num, p);
	BUG_ON(verb != QBMAN_MC_ACQUIRE);

	/* Determine success or failure */
	if (unlikely(rslt != QBMAN_MC_RSLT_OK)) {
		printf("Acquire buffers from BPID 0x%x failed, code=0x%02x\n",
		       bpid, rslt);
		return -EIO;
	}
	BUG_ON(num > num_buffers);
	/* Copy the acquired buffers to the caller's array */
	u64_from_le32_copy(buffers, &p[2], num);
	return (int)num;
}
