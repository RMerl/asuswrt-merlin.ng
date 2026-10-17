// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
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

#include <mach/cvmx-scratch.h>
#include <mach/cvmx-hwfau.h>
#include <mach/cvmx-fau.h>

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

/* #undef CVMX_ENABLE_PARAMETER_CHECKING */
/* #define CVMX_ENABLE_PARAMETER_CHECKING 1 */
/* #define	__PKO3_NATIVE_PTR */

static inline u64 cvmx_pko3_legacy_paddr(unsigned int node, u64 addr)
{
	u64 paddr;

	paddr = node;
	paddr = (addr & ((1ull << 40) - 1)) | (paddr << 40);
	return paddr;
}

#if CVMX_ENABLE_PARAMETER_CHECKING
/**
 * @INTERNAL
 *
 * Verify the integrity of a legacy buffer link pointer,
 *
 * Note that the IPD/PIP/PKO hardware would sometimes
 * round-up the buf_ptr->size field of the last buffer in a chain  to the next
 * cache line size, so the sum of buf_ptr->size
 * fields for a packet may exceed total_bytes by up to 127 bytes.
 *
 * @returns 0 on success, a negative number on error.
 */
static int cvmx_pko3_legacy_bufptr_validate(cvmx_buf_ptr_t buf_ptr,
					    unsigned int gather,
					    unsigned int buffers,
					    unsigned int total_bytes)
{
	unsigned int node = cvmx_get_node_num();
	unsigned int segs = 0, bytes = 0;
	unsigned int phys_addr;
	cvmx_buf_ptr_t ptr;
	int delta;

	if (buffers == 0) {
		return -1;
	} else if (buffers == 1) {
		delta = buf_ptr.s.size - total_bytes;
		if (delta < 0 || delta > 127)
			return -2;
	} else if (gather) {
		cvmx_buf_ptr_t *vptr;
		/* Validate gather list */
		if (buf_ptr.s.size < buffers)
			return -3;
		phys_addr = cvmx_pko3_legacy_paddr(node, buf_ptr.s.addr);
		vptr = cvmx_phys_to_ptr(phys_addr);
		for (segs = 0; segs < buffers; segs++)
			bytes += vptr[segs].s.size;
		delta = bytes - total_bytes;
		if (delta < 0 || delta > 127)
			return -4;
	} else {
		void *vptr;
		/* Validate linked buffers */
		ptr = buf_ptr;
		for (segs = 0; segs < buffers; segs++) {
			bytes += ptr.s.size;
			phys_addr = cvmx_pko3_legacy_paddr(node, ptr.s.addr);
			vptr = cvmx_phys_to_ptr(phys_addr);
			memcpy(&ptr, vptr - sizeof(u64), sizeof(u64));
		}
		delta = bytes - total_bytes;
		if (delta < 0 || delta > 127)
			return -5;
	}
	return 0;
}
#endif /* CVMX_ENABLE_PARAMETER_CHECKING */

/*
 * @INTERNAL
 *
 * Implementation note:
 * When the packet is sure to not need a jump_buf,
 * it will be written directly into cvmseg.
 * When the packet might not fit into cvmseg with all
 * of its descriptors, a jump_buf is allocated a priori,
 * and only header is first placed into cvmseg, all other
 * descriptors are placed into jump_buf, and finally
 * the PKO_SEND_JUMP_S is written to cvmseg.
 * This is because if there are no EXT or TSO descriptors,
 * then HDR must be first, and JMP second and that is all
 * that should go into cvmseg.
 */
struct __cvmx_pko3_legacy_desc {
	u64 *cmd_words;
	u64 *jump_buf_base_ptr;
	unsigned short word_count;
	short last_pool;
	u8 port_node;
	u8 aura_node;
	u8 jump_buf_size;
};

/**
 * @INTERNAL
 *
 * Add a subdescriptor into a command buffer,
 * and handle command-buffer overflow by allocating a JUMP_s buffer
 * from PKO3 internal AURA.
 */
static int __cvmx_pko3_cmd_subdc_add(struct __cvmx_pko3_legacy_desc *desc,
				     u64 subdc)
{
	/* SEND_JUMP_S missing on Pass1.X */
	if (desc->word_count >= 15) {
		printf("%s: ERROR: too many segments\n", __func__);
		return -EBADF;
	}

	/* Handle small commands simply */
	if (cvmx_likely(!desc->jump_buf_base_ptr)) {
		desc->cmd_words[desc->word_count] = subdc;
		(desc->word_count)++;
		return desc->word_count;
	}

	if (cvmx_unlikely(desc->jump_buf_size >= 255))
		return -ENOMEM;

	desc->jump_buf_base_ptr[desc->jump_buf_size++] = subdc;

	return desc->word_count + desc->jump_buf_size;
}

/**
 * @INTERNAL
 *
 * Finalize command buffer
 *
 * @returns: number of command words in command buffer and jump buffer
 * or negative number on error.
 */

static int __cvmx_pko3_cmd_done(struct __cvmx_pko3_legacy_desc *desc)
{
	short pko_aura;
	cvmx_pko_buf_ptr_t jump_s;
	cvmx_pko_send_aura_t aura_s;

	/* no jump buffer, nothing to do */
	if (!desc->jump_buf_base_ptr)
		return desc->word_count;

	desc->word_count++;

	/* Verify number of words is 15 */
	if (desc->word_count != 2) {
		printf("ERROR: %s: internal error, word_count=%d\n", __func__,
		       desc->word_count);
		return -EINVAL;
	}

	/* Add SEND_AURA_S at the end of jump_buf */
	pko_aura = __cvmx_pko3_aura_get(desc->port_node);

	aura_s.u64 = 0;
	aura_s.s.aura = pko_aura;
	aura_s.s.offset = 0;
	aura_s.s.alg = AURAALG_NOP;
	aura_s.s.subdc4 = CVMX_PKO_SENDSUBDC_AURA;

	desc->jump_buf_base_ptr[desc->jump_buf_size++] = aura_s.u64;

	/* Add SEND_JUMPS to point to jump_buf */
	jump_s.u64 = 0;
	jump_s.s.subdc3 = CVMX_PKO_SENDSUBDC_JUMP;
	jump_s.s.addr = cvmx_ptr_to_phys(desc->jump_buf_base_ptr);
	jump_s.s.i = 1; /* F=1: Free this buffer when done */
	jump_s.s.size = desc->jump_buf_size;
	desc->cmd_words[1] = jump_s.u64;

	return desc->word_count + desc->jump_buf_size;
}

/**
 * @INTERNAL
 *
 * Handle buffer pools for PKO legacy transmit operation
 */
static inline int cvmx_pko3_legacy_pool(struct __cvmx_pko3_legacy_desc *desc,
					int pool)
{
	cvmx_pko_send_aura_t aura_s;
	unsigned int aura;

	if (cvmx_unlikely(desc->last_pool == pool))
		return 0;

	aura = desc->aura_node << 10; /* LAURA=AURA[0..9] */
	aura |= pool;

	if (cvmx_likely(desc->last_pool < 0)) {
		cvmx_pko_send_hdr_t *hdr_s;

		hdr_s = (void *)&desc->cmd_words[0];
		/* Create AURA from legacy pool (assume LAURA==POOL */
		hdr_s->s.aura = aura;
		desc->last_pool = pool;
		return 0;
	}

	aura_s.u64 = 0;
	aura_s.s.subdc4 = CVMX_PKO_SENDSUBDC_AURA;
	aura_s.s.offset = 0;
	aura_s.s.alg = AURAALG_NOP;
	aura |= pool;
	aura_s.s.aura = aura;
	desc->last_pool = pool;
	return __cvmx_pko3_cmd_subdc_add(desc, aura_s.u64);
}

/**
 * @INTERNAL
 *
 * Backward compatibility for packet transmission using legacy PKO command.
 *
 * NOTE: Only supports output on node-local ports.
 *
 * TBD: Could embed destination node in extended DQ number.
 */
cvmx_pko_return_value_t
cvmx_pko3_legacy_xmit(unsigned int dq, cvmx_pko_command_word0_t pko_command,
		      cvmx_buf_ptr_t packet, u64 addr, bool tag_sw)
{
	cvmx_pko_query_rtn_t pko_status;
	cvmx_pko_send_hdr_t *hdr_s;
	struct __cvmx_pko3_legacy_desc desc;
	u8 *data_ptr;
	unsigned int node, seg_cnt;
	int res;
	cvmx_buf_ptr_pki_t bptr;

	seg_cnt = pko_command.s.segs;
	desc.cmd_words = cvmx_pko3_cvmseg_addr();

	/* Allocate from local aura, assume all old-pools are local */
	node = cvmx_get_node_num();
	desc.aura_node = node;

	/* Derive destination node from dq */
	desc.port_node = dq >> 10;
	dq &= (1 << 10) - 1;

	desc.word_count = 1;
	desc.last_pool = -1;

	/* For small packets, write descriptors directly to CVMSEG
	 * but for longer packets use jump_buf
	 */
	if (seg_cnt < 7 || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)) {
		desc.jump_buf_size = 0;
		desc.jump_buf_base_ptr = NULL;
	} else {
		unsigned int pko_aura = __cvmx_pko3_aura_get(desc.port_node);

		cvmx_fpa3_gaura_t aura =
			__cvmx_fpa3_gaura(pko_aura >> 10, pko_aura & 0x3ff);

		/* Allocate from internal AURA, size is 4KiB */
		desc.jump_buf_base_ptr = cvmx_fpa3_alloc(aura);

		if (!desc.jump_buf_base_ptr)
			return -ENOMEM;
		desc.jump_buf_size = 0;
	}

	/* Native buffer-pointer for error checiing */
	bptr.u64 = packet.u64;

#if CVMX_ENABLE_PARAMETER_CHECKING
	if (seg_cnt == 1 && bptr.size == pko_command.s.total_bytes) {
		/*
		 * Special case for native buffer pointer:
		 * This is the only case where the native pointer-style can be
		 * automatically identified, that is when an entire packet
		 * fits into a single buffer by the PKI.
		 * The use of the native buffers with this function
		 * should be avoided.
		 */
		debug("%s: WARNING: Native buffer-pointer\n", __func__);
	} else {
		/* The buffer ptr is assume to be received in legacy format */
		res = cvmx_pko3_legacy_bufptr_validate(
			packet, pko_command.s.gather, pko_command.s.segs,
			pko_command.s.total_bytes);
		if (res < 0) {
			debug("%s: ERROR: Not a valid packet pointer <%d>\n",
			      __func__, res);
			return CVMX_PKO_CMD_QUEUE_INIT_ERROR;
		}
	}
#endif /* CVMX_ENABLE_PARAMETER_CHECKING */

	/* Squash warnings */
	(void)bptr;

	/*** Translate legacy PKO fields into PKO3 PKO_SEND_HDR_S ***/

	/* PKO_SEND_HDR_S is alwasy the first word in the command */
	hdr_s = (void *)&desc.cmd_words[0];
	hdr_s->u64 = 0;

	/* Copy total packet size */
	hdr_s->s.total = pko_command.s.total_bytes;

	/* Endianness */
	hdr_s->s.le = pko_command.s.le;

	/* N2 is the same meaning */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
		hdr_s->s.n2 = 0; /* L2 allocate everything */
	else
		hdr_s->s.n2 = pko_command.s.n2;

	/* DF bit has the same meaning */
	hdr_s->s.df = pko_command.s.dontfree;

	/* II bit has the same meaning */
	hdr_s->s.ii = pko_command.s.ignore_i;

	/* non-zero IP header offset requires L3/L4 checksum calculation */
	if (cvmx_unlikely(pko_command.s.ipoffp1 > 0)) {
		u8 ipoff, ip0, l4_proto = 0;

		/* Get data pointer for header inspection below */
		if (pko_command.s.gather) {
			cvmx_buf_ptr_t *p_ptr;
			cvmx_buf_ptr_t blk;

			p_ptr = cvmx_phys_to_ptr(
				cvmx_pko3_legacy_paddr(node, packet.s.addr));
			blk = p_ptr[0];
			data_ptr = cvmx_phys_to_ptr(
				cvmx_pko3_legacy_paddr(node, blk.s.addr));
		} else {
			data_ptr = cvmx_phys_to_ptr(
				cvmx_pko3_legacy_paddr(node, packet.s.addr));
		}

		/* Get IP header offset */
		ipoff = pko_command.s.ipoffp1 - 1;

		/* Parse IP header, version, L4 protocol */
		hdr_s->s.l3ptr = ipoff;
		ip0 = data_ptr[ipoff];

		/* IPv4 header length, checksum offload */
		if ((ip0 >> 4) == 4) {
			hdr_s->s.l4ptr = hdr_s->s.l3ptr + ((ip0 & 0xf) << 2);
			l4_proto = data_ptr[ipoff + 9];
			hdr_s->s.ckl3 = 1; /* Only valid for IPv4 */
		}
		/* IPv6 header length is fixed, no checksum */
		if ((ip0 >> 4) == 6) {
			hdr_s->s.l4ptr = hdr_s->s.l3ptr + 40;
			l4_proto = data_ptr[ipoff + 6];
		}
		/* Set L4 checksum algo based on L4 protocol */
		if (l4_proto == 6)
			hdr_s->s.ckl4 = /* TCP */ 2;
		else if (l4_proto == 17)
			hdr_s->s.ckl4 = /* UDP */ 1;
		else if (l4_proto == 132)
			hdr_s->s.ckl4 = /* SCTP */ 3;
		else
			hdr_s->s.ckl4 = /* Unknown */ 0;
	}

	if (pko_command.s.gather) {
		/* Process legacy gather list */
		cvmx_pko_buf_ptr_t gather_s;
		cvmx_buf_ptr_t *p_ptr;
		cvmx_buf_ptr_t blk;
		unsigned int i;

		/* Get gather list pointer */
		p_ptr = cvmx_phys_to_ptr(
			cvmx_pko3_legacy_paddr(node, packet.s.addr));
		blk = p_ptr[0];
		/* setup data_ptr */
		data_ptr = cvmx_phys_to_ptr(
			cvmx_pko3_legacy_paddr(node, blk.s.addr));

		for (i = 0; i < seg_cnt; i++) {
			if (cvmx_unlikely(cvmx_pko3_legacy_pool(
						  &desc, blk.s.pool) < 0))
				return CVMX_PKO_NO_MEMORY;

			/* Insert PKO_SEND_GATHER_S for the current buffer */
			gather_s.u64 = 0;
			gather_s.s.subdc3 = CVMX_PKO_SENDSUBDC_GATHER;
			gather_s.s.size = blk.s.size;
			gather_s.s.i = blk.s.i;
			gather_s.s.addr =
				cvmx_pko3_legacy_paddr(node, blk.s.addr);

			res = __cvmx_pko3_cmd_subdc_add(&desc, gather_s.u64);
			if (res < 0)
				return CVMX_PKO_NO_MEMORY;

			/* get next bufptr */
			blk = p_ptr[i + 1];
		} /* for i */

		/* Free original gather-list buffer */
		if ((pko_command.s.ignore_i && !pko_command.s.dontfree) ||
		    packet.s.i == pko_command.s.dontfree)
			cvmx_fpa_free_nosync(p_ptr, packet.s.pool,
					     (i - 1) / 16 + 1);
	} else {
		/* Process legacy linked buffer list */
		cvmx_pko_buf_ptr_t gather_s;
		cvmx_buf_ptr_t blk;
		void *vptr;

		data_ptr = cvmx_phys_to_ptr(
			cvmx_pko3_legacy_paddr(node, packet.s.addr));
		blk = packet;

		/*
		 * Legacy linked-buffers converted into flat gather list
		 * so that the AURA can optionally be changed to reflect
		 * the POOL number in the legacy pointers
		 */
		do {
			/* Insert PKO_SEND_AURA_S if pool changes */
			if (cvmx_unlikely(cvmx_pko3_legacy_pool(
						  &desc, blk.s.pool) < 0))
				return CVMX_PKO_NO_MEMORY;

			/* Insert PKO_SEND_GATHER_S for the current buffer */
			gather_s.u64 = 0;
			gather_s.s.subdc3 = CVMX_PKO_SENDSUBDC_GATHER;
			gather_s.s.size = blk.s.size;
			gather_s.s.i = blk.s.i;
			gather_s.s.addr =
				cvmx_pko3_legacy_paddr(node, blk.s.addr);

			res = __cvmx_pko3_cmd_subdc_add(&desc, gather_s.u64);
			if (res < 0)
				return CVMX_PKO_NO_MEMORY;

			/* Get the next buffer pointer */
			vptr = cvmx_phys_to_ptr(
				cvmx_pko3_legacy_paddr(node, blk.s.addr));
			memcpy(&blk, vptr - sizeof(blk), sizeof(blk));

			/* Decrement segment count */
			seg_cnt--;

		} while (seg_cnt > 0);
	}

	/* This field indicates the presence of 3rd legacy command word */
	/* NOTE: legacy 3rd word may contain CN78XX native phys addr already */
	if (cvmx_unlikely(pko_command.s.rsp)) {
		/* PTP bit in word3 is not supported -
		 * can not be distibguished from larger phys_addr[42..41]
		 */
		if (pko_command.s.wqp) {
			/* <addr> is an SSO WQE */
			cvmx_wqe_word1_t *wqe_p;
			cvmx_pko_send_work_t work_s;

			work_s.u64 = 0;
			work_s.s.subdc4 = CVMX_PKO_SENDSUBDC_WORK;
			work_s.s.addr = addr;
			/* Assume WQE is legacy format too */
			wqe_p = cvmx_phys_to_ptr(addr + sizeof(u64));
			work_s.s.grp = wqe_p->cn38xx.grp;
			work_s.s.tt = wqe_p->tag_type;

			res = __cvmx_pko3_cmd_subdc_add(&desc, work_s.u64);
		} else {
			cvmx_pko_send_mem_t mem_s;
			/* MEMALG_SET broken on Pass1 */
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0)) {
				debug("%s: ERROR: PKO byte-clear not supported\n",
				      __func__);
			}
			/* <addr> is a physical address of byte clear */
			mem_s.u64 = 0;
			mem_s.s.subdc4 = CVMX_PKO_SENDSUBDC_MEM;
			mem_s.s.addr = addr;
			mem_s.s.dsz = MEMDSZ_B8;
			mem_s.s.alg = MEMALG_SET;
			mem_s.s.offset = 0;

			res = __cvmx_pko3_cmd_subdc_add(&desc, mem_s.u64);
		}
		if (res < 0)
			return CVMX_PKO_NO_MEMORY;
	}

	/* FAU counter binding reg0 */
	if (pko_command.s.reg0) {
		cvmx_pko_send_mem_t mem_s;

		debug("%s: Legacy FAU commands: reg0=%#x sz0=%#x\n", __func__,
		      pko_command.s.reg0, pko_command.s.size0);
		mem_s.u64 = 0;
		mem_s.s.subdc4 = CVMX_PKO_SENDSUBDC_MEM;
		mem_s.s.addr = cvmx_ptr_to_phys(
			CASTPTR(void, __cvmx_fau_sw_addr(pko_command.s.reg0)));
		if (cvmx_likely(pko_command.s.size0 == CVMX_FAU_OP_SIZE_64))
			mem_s.s.dsz = MEMDSZ_B64;
		else if (pko_command.s.size0 == CVMX_FAU_OP_SIZE_32)
			mem_s.s.dsz = MEMDSZ_B32;
		else if (pko_command.s.size0 == CVMX_FAU_OP_SIZE_16)
			mem_s.s.dsz = MEMDSZ_B16;
		else
			mem_s.s.dsz = MEMDSZ_B8;

		if (mem_s.s.dsz == MEMDSZ_B16 || mem_s.s.dsz == MEMDSZ_B8)
			debug("%s: ERROR: 8/16 bit decrement unsupported",
			      __func__);

		mem_s.s.offset = pko_command.s.subone0;
		if (mem_s.s.offset)
			mem_s.s.alg = MEMALG_SUB;
		else
			mem_s.s.alg = MEMALG_SUBLEN;

		res = __cvmx_pko3_cmd_subdc_add(&desc, mem_s.u64);
		if (res < 0)
			return CVMX_PKO_NO_MEMORY;
	}

	/* FAU counter binding reg1 */
	if (cvmx_unlikely(pko_command.s.reg1)) {
		cvmx_pko_send_mem_t mem_s;

		debug("%s: Legacy FAU commands: reg1=%#x sz1=%#x\n", __func__,
		      pko_command.s.reg1, pko_command.s.size1);
		mem_s.u64 = 0;
		mem_s.s.subdc4 = CVMX_PKO_SENDSUBDC_MEM;
		mem_s.s.addr = cvmx_ptr_to_phys(
			CASTPTR(void, __cvmx_fau_sw_addr(pko_command.s.reg1)));
		if (cvmx_likely(pko_command.s.size1 == CVMX_FAU_OP_SIZE_64))
			mem_s.s.dsz = MEMDSZ_B64;
		else if (pko_command.s.size1 == CVMX_FAU_OP_SIZE_32)
			mem_s.s.dsz = MEMDSZ_B32;
		else if (pko_command.s.size1 == CVMX_FAU_OP_SIZE_16)
			mem_s.s.dsz = MEMDSZ_B16;
		else
			mem_s.s.dsz = MEMDSZ_B8;

		if (mem_s.s.dsz == MEMDSZ_B16 || mem_s.s.dsz == MEMDSZ_B8)
			printf("%s: ERROR: 8/16 bit decrement unsupported",
			       __func__);

		mem_s.s.offset = pko_command.s.subone1;
		if (mem_s.s.offset)
			mem_s.s.alg = MEMALG_SUB;
		else
			mem_s.s.alg = MEMALG_SUBLEN;

		res = __cvmx_pko3_cmd_subdc_add(&desc, mem_s.u64);
		if (res < 0)
			return CVMX_PKO_NO_MEMORY;
	}

	/* These PKO_HDR_S fields are not used: */
	/* hdr_s->s.ds does not have legacy equivalent, remains 0 */
	/* hdr_s->s.format has no legacy equivalent, remains 0 */

	/*** Finalize command buffer ***/
	res = __cvmx_pko3_cmd_done(&desc);
	if (res < 0)
		return CVMX_PKO_NO_MEMORY;

	/*** Send the PKO3 command into the Descriptor Queue ***/
	pko_status =
		__cvmx_pko3_lmtdma(desc.port_node, dq, desc.word_count, tag_sw);

	/*** Map PKO3 result codes to legacy return values ***/
	if (cvmx_likely(pko_status.s.dqstatus == PKO_DQSTATUS_PASS))
		return CVMX_PKO_SUCCESS;

	debug("%s: ERROR: failed to enqueue: %s\n", __func__,
	      pko_dqstatus_error(pko_status.s.dqstatus));

	if (pko_status.s.dqstatus == PKO_DQSTATUS_ALREADY)
		return CVMX_PKO_PORT_ALREADY_SETUP;
	if (pko_status.s.dqstatus == PKO_DQSTATUS_NOFPABUF ||
	    pko_status.s.dqstatus == PKO_DQSTATUS_NOPKOBUF)
		return CVMX_PKO_NO_MEMORY;
	if (pko_status.s.dqstatus == PKO_DQSTATUS_NOTCREATED)
		return CVMX_PKO_INVALID_QUEUE;
	if (pko_status.s.dqstatus == PKO_DQSTATUS_BADSTATE)
		return CVMX_PKO_CMD_QUEUE_INIT_ERROR;
	if (pko_status.s.dqstatus == PKO_DQSTATUS_SENDPKTDROP)
		return CVMX_PKO_INVALID_PORT;

	return CVMX_PKO_INVALID_PORT;
}
