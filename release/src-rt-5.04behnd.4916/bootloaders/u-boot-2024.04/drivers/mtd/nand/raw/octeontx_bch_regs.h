/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2018 Marvell International Ltd.
 */

#ifndef __OCTEONTX_BCH_REGS_H__
#define __OCTEONTX_BCH_REGS_H__

#define BCH_NR_VF	1

union bch_cmd {
	u64 u[4];
	struct fields {
	    struct {
		u64 size:12;
		u64 reserved_12_31:20;
		u64 ecc_level:4;
		u64 reserved_36_61:26;
		u64 ecc_gen:2;
	    } cword;
	    struct {
		u64 ptr:49;
		u64 reserved_49_55:7;
		u64 nc:1;
		u64 fw:1;
		u64 reserved_58_63:6;
	    } oword;
	    struct {
		u64 ptr:49;
		u64 reserved_49_55:7;
		u64 nc:1;
		u64 reserved_57_63:7;
	    } iword;
	    struct {
		u64 ptr:49;
		u64 reserved_49_63:15;
	    } rword;
	} s;
};

enum ecc_gen {
	eg_correct,
	eg_copy,
	eg_gen,
	eg_copy3,
};

/** Response from BCH instruction */
union bch_resp {
	u16  u16;
	struct {
		u16	num_errors:7;	/** Number of errors in block */
		u16	zero:6;		/** Always zero, ignore */
		u16	erased:1;	/** Block is erased */
		u16	uncorrectable:1;/** too many bits flipped */
		u16	done:1;		/** Block is done */
	} s;
};

union bch_vqx_ctl {
	u64 u;
	struct {
		u64 reserved_0:1;
		u64 cmd_be:1;
		u64 max_read:4;
		u64 reserved_6_15:10;
		u64 erase_disable:1;
		u64 one_cmd:1;
		u64 early_term:4;
		u64 reserved_22_63:42;
	} s;
};

union bch_vqx_cmd_buf {
	u64 u;
	struct {
		u64 reserved_0_32:33;
		u64 size:13;
		u64 dfb:1;
		u64 ldwb:1;
		u64 reserved_48_63:16;
	} s;
};

/* keep queue state indexed, even though just one supported here,
 * for later generalization to similarly-shaped queues on other Cavium devices
 */
enum {
	QID_BCH,
	QID_MAX
};

struct bch_q {
	struct udevice *dev;
	int index;
	u16 max_depth;
	u16 pool_size_m1;
	u64 *base_vaddr;
	dma_addr_t base_paddr;
};

extern struct bch_q octeontx_bch_q[QID_MAX];

/* with one dma-mapped area, virt<->phys conversions by +/- (vaddr-paddr) */
static inline dma_addr_t qphys(int qid, void *v)
{
	struct bch_q *q = &octeontx_bch_q[qid];
	int off = (u8 *)v - (u8 *)q->base_vaddr;

	return q->base_paddr + off;
}

#define octeontx_ptr_to_phys(v) qphys(QID_BCH, (v))

static inline void *qvirt(int qid, dma_addr_t p)
{
	struct bch_q *q = &octeontx_bch_q[qid];
	int off = p - q->base_paddr;

	return q->base_vaddr + off;
}

#define octeontx_phys_to_ptr(p) qvirt(QID_BCH, (p))

/* plenty for interleaved r/w on two planes with 16k page, ecc_size 1k */
/* QDEPTH >= 16, as successive chunks must align on 128-byte boundaries */
#define QDEPTH	256	/* u64s in a command queue chunk, incl next-pointer */
#define NQS	1	/* linked chunks in the chain */

/**
 * Write an arbitrary number of command words to a command queue.
 * This is a generic function; the fixed number of command word
 * functions yield higher performance.
 *
 * Could merge with crypto version for FPA use on cn83xx
 */
static inline int octeontx_cmd_queue_write(int queue_id, bool use_locking,
					   int cmd_count, const u64 *cmds)
{
	int ret = 0;
	u64 *cmd_ptr;
	struct bch_q *qptr = &octeontx_bch_q[queue_id];

	if (unlikely(cmd_count < 1 || cmd_count > 32))
		return -EINVAL;
	if (unlikely(!cmds))
		return -EINVAL;

	cmd_ptr = qptr->base_vaddr;

	while (cmd_count > 0) {
		int slot = qptr->index % (QDEPTH * NQS);

		if (slot % QDEPTH != QDEPTH - 1) {
			cmd_ptr[slot] = *cmds++;
			cmd_count--;
		}

		qptr->index++;
	}

	__iowmb();	/* flush commands before ringing bell */

	return ret;
}

#endif /* __OCTEONTX_BCH_REGS_H__ */
