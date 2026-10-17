// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <memalign.h>
#include <misc.h>
#include <net.h>
#include <pci.h>
#include <watchdog.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/log2.h>
#include <asm/arch/board.h>
#include <asm/arch/csrs/csrs-npc.h>
#include <asm/arch/csrs/csrs-lmt.h>
#include <asm/io.h>

#include "nix.h"
#include "lmt.h"
#include "cgx.h"

static struct nix_aq_cq_dis cq_dis ALIGNED;
static struct nix_aq_rq_dis rq_dis ALIGNED;
static struct nix_aq_sq_dis sq_dis ALIGNED;

/***************
 * NPA API
 ***************/
int npa_attach_aura(struct nix_af *nix_af, int lf,
		    const union npa_aura_s *desc, u32 aura_id)
{
	struct npa_af *npa = nix_af->npa_af;
	union npa_aq_inst_s *inst;
	union npa_aq_res_s *res;
	union npa_af_aq_status aq_stat;
	union npa_aura_s *context;
	u64 head;
	ulong start;

	debug("%s(%p, %d, %p, %u)\n", __func__, nix_af, lf, desc, aura_id);
	aq_stat.u = npa_af_reg_read(npa, NPA_AF_AQ_STATUS());
	head = aq_stat.s.head_ptr;
	inst = (union npa_aq_inst_s *)(npa->aq.inst.base) + head;
	res = (union npa_aq_res_s *)(npa->aq.res.base);

	memset(inst, 0, sizeof(*inst));
	inst->s.lf = lf;
	inst->s.doneint = 0;
	inst->s.ctype = NPA_AQ_CTYPE_E_AURA;
	inst->s.op = NPA_AQ_INSTOP_E_INIT;
	inst->s.res_addr = npa->aq.res.iova;
	inst->s.cindex = aura_id;

	context = (union npa_aura_s *)(npa->aq.res.base +
						CONFIG_SYS_CACHELINE_SIZE);
	memset(npa->aq.res.base, 0, npa->aq.res.entry_sz);
	memcpy(context, desc, sizeof(union npa_aura_s));
	__iowmb();
	npa_af_reg_write(npa, NPA_AF_AQ_DOOR(), 1);

	start = get_timer(0);
	while ((res->s.compcode == NPA_AQ_COMP_E_NOTDONE) &&
	       (get_timer(start) < 1000))
		schedule();
	if (res->s.compcode != NPA_AQ_COMP_E_GOOD) {
		printf("%s: Error: result 0x%x not good\n",
		       __func__, res->s.compcode);
		return -1;
	}

	return 0;
}

int npa_attach_pool(struct nix_af *nix_af, int lf,
		    const union npa_pool_s *desc, u32 pool_id)
{
	union npa_aq_inst_s *inst;
	union npa_aq_res_s *res;
	union npa_af_aq_status aq_stat;
	struct npa_af *npa = nix_af->npa_af;
	union npa_aura_s *context;
	u64 head;
	ulong start;

	debug("%s(%p, %d, %p, %u)\n", __func__, nix_af, lf, desc, pool_id);
	aq_stat.u = npa_af_reg_read(npa, NPA_AF_AQ_STATUS());
	head = aq_stat.s.head_ptr;

	inst = (union npa_aq_inst_s *)(npa->aq.inst.base) + head;
	res = (union npa_aq_res_s *)(npa->aq.res.base);

	memset(inst, 0, sizeof(*inst));
	inst->s.cindex = pool_id;
	inst->s.lf = lf;
	inst->s.doneint = 0;
	inst->s.ctype = NPA_AQ_CTYPE_E_POOL;
	inst->s.op = NPA_AQ_INSTOP_E_INIT;
	inst->s.res_addr = npa->aq.res.iova;

	context = (union npa_aura_s *)(npa->aq.res.base +
						CONFIG_SYS_CACHELINE_SIZE);
	memset(npa->aq.res.base, 0, npa->aq.res.entry_sz);
	memcpy(context, desc, sizeof(union npa_aura_s));
	__iowmb();
	npa_af_reg_write(npa, NPA_AF_AQ_DOOR(), 1);

	start = get_timer(0);
	while ((res->s.compcode == NPA_AQ_COMP_E_NOTDONE) &&
	       (get_timer(start) < 1000))
		schedule();

	if (res->s.compcode != NPA_AQ_COMP_E_GOOD) {
		printf("%s: Error: result 0x%x not good\n",
		       __func__, res->s.compcode);
		return -1;
	}

	return 0;
}

int npa_lf_admin_setup(struct npa *npa, int lf, dma_addr_t aura_base)
{
	union npa_af_lf_rst lf_rst;
	union npa_af_lfx_auras_cfg auras_cfg;
	struct npa_af *npa_af = npa->npa_af;

	debug("%s(%p, %d, 0x%llx)\n", __func__, npa_af, lf, aura_base);
	lf_rst.u = 0;
	lf_rst.s.exec = 1;
	lf_rst.s.lf = lf;
	npa_af_reg_write(npa_af, NPA_AF_LF_RST(), lf_rst.u);

	do {
		lf_rst.u = npa_af_reg_read(npa_af, NPA_AF_LF_RST());
		schedule();
	} while (lf_rst.s.exec);

	/* Set Aura size and enable caching of contexts */
	auras_cfg.u = npa_af_reg_read(npa_af, NPA_AF_LFX_AURAS_CFG(lf));
	auras_cfg.s.loc_aura_size = NPA_AURA_SIZE_DEFAULT; //FIXME aura_size;
	auras_cfg.s.caching = 1;
	auras_cfg.s.rmt_aura_size = 0;
	auras_cfg.s.rmt_aura_offset = 0;
	auras_cfg.s.rmt_lf = 0;
	npa_af_reg_write(npa_af, NPA_AF_LFX_AURAS_CFG(lf), auras_cfg.u);
	/* Configure aura HW context base */
	npa_af_reg_write(npa_af, NPA_AF_LFX_LOC_AURAS_BASE(lf),
			 aura_base);

	return 0;
}

int npa_lf_admin_shutdown(struct nix_af *nix_af, int lf, u32 pool_count)
{
	int pool_id;
	u32 head;
	union npa_aq_inst_s *inst;
	union npa_aq_res_s *res;
	struct npa_aq_pool_request {
		union npa_aq_res_s	resp ALIGNED;
		union npa_pool_s p0 ALIGNED;
		union npa_pool_s p1 ALIGNED;
	} pool_req ALIGNED;
	struct npa_aq_aura_request {
		union npa_aq_res_s	resp ALIGNED;
		union npa_aura_s a0 ALIGNED;
		union npa_aura_s a1 ALIGNED;
	} aura_req ALIGNED;
	union npa_af_aq_status aq_stat;
	union npa_af_lf_rst lf_rst;
	struct npa_af *npa = nix_af->npa_af;
	ulong start;

	for (pool_id = 0; pool_id < pool_count; pool_id++) {
		aq_stat.u = npa_af_reg_read(npa, NPA_AF_AQ_STATUS());
		head = aq_stat.s.head_ptr;
		inst = (union npa_aq_inst_s *)(npa->aq.inst.base) + head;
		res = &pool_req.resp;

		memset(inst, 0, sizeof(*inst));
		inst->s.cindex = pool_id;
		inst->s.lf = lf;
		inst->s.doneint = 0;
		inst->s.ctype = NPA_AQ_CTYPE_E_POOL;
		inst->s.op = NPA_AQ_INSTOP_E_WRITE;
		inst->s.res_addr = (u64)&pool_req.resp;

		memset((void *)&pool_req, 0, sizeof(pool_req));
		pool_req.p0.s.ena = 0;
		pool_req.p1.s.ena = 1;	/* Write mask */
		__iowmb();

		npa_af_reg_write(npa, NPA_AF_AQ_DOOR(), 1);

		start = get_timer(0);
		while ((res->s.compcode == NPA_AQ_COMP_E_NOTDONE) &&
		       (get_timer(start) < 1000))
			schedule();

		if (res->s.compcode != NPA_AQ_COMP_E_GOOD) {
			printf("%s: Error: result 0x%x not good for lf %d\n"
			       " aura id %d", __func__, res->s.compcode, lf,
				pool_id);
			return -1;
		}
		debug("%s(LF %d, pool id %d) disabled\n", __func__, lf,
		      pool_id);
	}

	for (pool_id = 0; pool_id < pool_count; pool_id++) {
		aq_stat.u = npa_af_reg_read(npa, NPA_AF_AQ_STATUS());
		head = aq_stat.s.head_ptr;
		inst = (union npa_aq_inst_s *)(npa->aq.inst.base) + head;
		res = &aura_req.resp;

		memset(inst, 0, sizeof(*inst));
		inst->s.cindex = pool_id;
		inst->s.lf = lf;
		inst->s.doneint = 0;
		inst->s.ctype = NPA_AQ_CTYPE_E_AURA;
		inst->s.op = NPA_AQ_INSTOP_E_WRITE;
		inst->s.res_addr = (u64)&aura_req.resp;

		memset((void *)&aura_req, 0, sizeof(aura_req));
		aura_req.a0.s.ena = 0;
		aura_req.a1.s.ena = 1;	/* Write mask */
		__iowmb();

		npa_af_reg_write(npa, NPA_AF_AQ_DOOR(), 1);

		start = get_timer(0);
		while ((res->s.compcode == NPA_AQ_COMP_E_NOTDONE) &&
		       (get_timer(start) < 1000))
			schedule();

		if (res->s.compcode != NPA_AQ_COMP_E_GOOD) {
			printf("%s: Error: result 0x%x not good for lf %d\n"
			       " aura id %d", __func__, res->s.compcode, lf,
			       pool_id);
			return -1;
		}
		debug("%s(LF %d, aura id %d) disabled\n", __func__, lf,
		      pool_id);
	}

	/* Reset the LF */
	lf_rst.u = 0;
	lf_rst.s.exec = 1;
	lf_rst.s.lf = lf;
	npa_af_reg_write(npa, NPA_AF_LF_RST(), lf_rst.u);

	do {
		lf_rst.u = npa_af_reg_read(npa, NPA_AF_LF_RST());
		schedule();
	} while (lf_rst.s.exec);

	return 0;
}

int npa_af_setup(struct npa_af *npa_af)
{
	int err;
	union npa_af_gen_cfg npa_cfg;
	union npa_af_ndc_cfg ndc_cfg;
	union npa_af_aq_cfg aq_cfg;
	union npa_af_blk_rst blk_rst;

	err = rvu_aq_alloc(&npa_af->aq, Q_COUNT(AQ_SIZE),
			   sizeof(union npa_aq_inst_s),
			   sizeof(union npa_aq_res_s));
	if (err) {
		printf("%s: Error %d allocating admin queue\n", __func__, err);
		return err;
	}
	debug("%s: NPA admin queue allocated at %p %llx\n", __func__,
	      npa_af->aq.inst.base, npa_af->aq.inst.iova);

	blk_rst.u = 0;
	blk_rst.s.rst = 1;
	npa_af_reg_write(npa_af, NPA_AF_BLK_RST(), blk_rst.u);

	/* Wait for reset to complete */
	do {
		blk_rst.u = npa_af_reg_read(npa_af, NPA_AF_BLK_RST());
		schedule();
	} while (blk_rst.s.busy);

	/* Set little Endian */
	npa_cfg.u = npa_af_reg_read(npa_af, NPA_AF_GEN_CFG());
	npa_cfg.s.af_be = 0;
	npa_af_reg_write(npa_af, NPA_AF_GEN_CFG(), npa_cfg.u);
	/* Enable NDC cache */
	ndc_cfg.u = npa_af_reg_read(npa_af, NPA_AF_NDC_CFG());
	ndc_cfg.s.ndc_bypass = 0;
	npa_af_reg_write(npa_af, NPA_AF_NDC_CFG(), ndc_cfg.u);
	/* Set up queue size */
	aq_cfg.u = npa_af_reg_read(npa_af, NPA_AF_AQ_CFG());
	aq_cfg.s.qsize = AQ_SIZE;
	npa_af_reg_write(npa_af, NPA_AF_AQ_CFG(), aq_cfg.u);
	/* Set up queue base address */
	npa_af_reg_write(npa_af, NPA_AF_AQ_BASE(), npa_af->aq.inst.iova);

	return 0;
}

int npa_af_shutdown(struct npa_af *npa_af)
{
	union npa_af_blk_rst blk_rst;

	blk_rst.u = 0;
	blk_rst.s.rst = 1;
	npa_af_reg_write(npa_af, NPA_AF_BLK_RST(), blk_rst.u);

	/* Wait for reset to complete */
	do {
		blk_rst.u = npa_af_reg_read(npa_af, NPA_AF_BLK_RST());
		schedule();
	} while (blk_rst.s.busy);

	rvu_aq_free(&npa_af->aq);

	debug("%s: npa af reset --\n", __func__);

	return 0;
}

/***************
 * NIX API
 ***************/
/**
 * Setup SMQ -> TL4 -> TL3 -> TL2 -> TL1 -> MAC mapping
 *
 * @param nix     Handle to setup
 *
 * Return: 0, or negative on failure
 */
static int nix_af_setup_sq(struct nix *nix)
{
	union nixx_af_tl1x_schedule tl1_sched;
	union nixx_af_tl2x_parent tl2_parent;
	union nixx_af_tl3x_parent tl3_parent;
	union nixx_af_tl3_tl2x_cfg tl3_tl2_cfg;
	union nixx_af_tl3_tl2x_linkx_cfg tl3_tl2_link_cfg;
	union nixx_af_tl4x_parent tl4_parent;
	union nixx_af_tl4x_sdp_link_cfg tl4_sdp_link_cfg;
	union nixx_af_smqx_cfg smq_cfg;
	union nixx_af_mdqx_schedule mdq_sched;
	union nixx_af_mdqx_parent mdq_parent;
	union nixx_af_rx_linkx_cfg link_cfg;
	int tl1_index = nix->lmac->link_num; /* NIX_LINK_E enum */
	int tl2_index = tl1_index;
	int tl3_index = tl2_index;
	int tl4_index = tl3_index;
	int smq_index = tl4_index;
	struct nix_af *nix_af = nix->nix_af;
	u64 offset = 0;

	tl1_sched.u = nix_af_reg_read(nix_af,
				      NIXX_AF_TL1X_SCHEDULE(tl1_index));
	tl1_sched.s.rr_quantum = MAX_MTU;
	nix_af_reg_write(nix_af, NIXX_AF_TL1X_SCHEDULE(tl1_index),
			 tl1_sched.u);

	tl2_parent.u = nix_af_reg_read(nix_af,
				       NIXX_AF_TL2X_PARENT(tl2_index));
	tl2_parent.s.parent = tl1_index;
	nix_af_reg_write(nix_af, NIXX_AF_TL2X_PARENT(tl2_index),
			 tl2_parent.u);

	tl3_parent.u = nix_af_reg_read(nix_af,
				       NIXX_AF_TL3X_PARENT(tl3_index));
	tl3_parent.s.parent = tl2_index;
	nix_af_reg_write(nix_af, NIXX_AF_TL3X_PARENT(tl3_index),
			 tl3_parent.u);
	tl3_tl2_cfg.u = nix_af_reg_read(nix_af,
					NIXX_AF_TL3_TL2X_CFG(tl3_index));
	tl3_tl2_cfg.s.express = 0;
	nix_af_reg_write(nix_af, NIXX_AF_TL3_TL2X_CFG(tl3_index),
			 tl3_tl2_cfg.u);

	offset = NIXX_AF_TL3_TL2X_LINKX_CFG(tl3_index,
					    nix->lmac->link_num);
	tl3_tl2_link_cfg.u = nix_af_reg_read(nix_af, offset);
	tl3_tl2_link_cfg.s.bp_ena = 1;
	tl3_tl2_link_cfg.s.ena = 1;
	tl3_tl2_link_cfg.s.relchan = 0;
	offset = NIXX_AF_TL3_TL2X_LINKX_CFG(tl3_index,
					    nix->lmac->link_num);
	nix_af_reg_write(nix_af, offset, tl3_tl2_link_cfg.u);

	tl4_parent.u = nix_af_reg_read(nix_af,
				       NIXX_AF_TL4X_PARENT(tl4_index));
	tl4_parent.s.parent = tl3_index;
	nix_af_reg_write(nix_af, NIXX_AF_TL4X_PARENT(tl4_index),
			 tl4_parent.u);

	offset = NIXX_AF_TL4X_SDP_LINK_CFG(tl4_index);
	tl4_sdp_link_cfg.u = nix_af_reg_read(nix_af, offset);
	tl4_sdp_link_cfg.s.bp_ena = 0;
	tl4_sdp_link_cfg.s.ena = 0;
	tl4_sdp_link_cfg.s.relchan = 0;
	offset = NIXX_AF_TL4X_SDP_LINK_CFG(tl4_index);
	nix_af_reg_write(nix_af, offset, tl4_sdp_link_cfg.u);

	smq_cfg.u = nix_af_reg_read(nix_af, NIXX_AF_SMQX_CFG(smq_index));
	smq_cfg.s.express = 0;
	smq_cfg.s.lf = nix->lf;
	smq_cfg.s.desc_shp_ctl_dis = 1;
	smq_cfg.s.maxlen = MAX_MTU;
	smq_cfg.s.minlen = NIX_MIN_HW_MTU;
	nix_af_reg_write(nix_af, NIXX_AF_SMQX_CFG(smq_index), smq_cfg.u);

	mdq_sched.u = nix_af_reg_read(nix_af,
				      NIXX_AF_MDQX_SCHEDULE(smq_index));
	mdq_sched.s.rr_quantum = MAX_MTU;
	offset = NIXX_AF_MDQX_SCHEDULE(smq_index);
	nix_af_reg_write(nix_af, offset, mdq_sched.u);
	mdq_parent.u = nix_af_reg_read(nix_af,
				       NIXX_AF_MDQX_PARENT(smq_index));
	mdq_parent.s.parent = tl4_index;
	nix_af_reg_write(nix_af, NIXX_AF_MDQX_PARENT(smq_index),
			 mdq_parent.u);

	link_cfg.u = 0;
	link_cfg.s.maxlen = NIX_MAX_HW_MTU;
	link_cfg.s.minlen = NIX_MIN_HW_MTU;
	nix_af_reg_write(nix->nix_af,
			 NIXX_AF_RX_LINKX_CFG(nix->lmac->link_num),
			 link_cfg.u);

	return 0;
}

/**
 * Issue a command to the NIX AF Admin Queue
 *
 * @param nix    nix handle
 * @param lf     Logical function number for command
 * @param op     Operation
 * @param ctype  Context type
 * @param cindex Context index
 * @param resp   Result pointer
 *
 * Return:	0 for success, -EBUSY on failure
 */
static int nix_aq_issue_command(struct nix_af *nix_af,
				int lf,
				int op,
				int ctype,
				int cindex, union nix_aq_res_s *resp)
{
	union nixx_af_aq_status aq_status;
	union nix_aq_inst_s *aq_inst;
	union nix_aq_res_s *result = resp;
	ulong start;

	debug("%s(%p, 0x%x, 0x%x, 0x%x, 0x%x, %p)\n", __func__, nix_af, lf,
	      op, ctype, cindex, resp);
	aq_status.u = nix_af_reg_read(nix_af, NIXX_AF_AQ_STATUS());
	aq_inst = (union nix_aq_inst_s *)(nix_af->aq.inst.base) +
						aq_status.s.head_ptr;
	aq_inst->u[0] = 0;
	aq_inst->u[1] = 0;
	aq_inst->s.op = op;
	aq_inst->s.ctype = ctype;
	aq_inst->s.lf = lf;
	aq_inst->s.cindex = cindex;
	aq_inst->s.doneint = 0;
	aq_inst->s.res_addr = (u64)resp;
	debug("%s: inst@%p: 0x%llx 0x%llx\n", __func__, aq_inst,
	      aq_inst->u[0], aq_inst->u[1]);
	__iowmb();

	/* Ring doorbell and wait for result */
	nix_af_reg_write(nix_af, NIXX_AF_AQ_DOOR(), 1);

	start = get_timer(0);
	/* Wait for completion */
	do {
		schedule();
		dsb();
	} while (result->s.compcode == 0 && get_timer(start) < 2);

	if (result->s.compcode != NIX_AQ_COMP_E_GOOD) {
		printf("NIX:AQ fail or time out with code %d after %ld ms\n",
		       result->s.compcode, get_timer(start));
		return -EBUSY;
	}
	return 0;
}

static int nix_attach_receive_queue(struct nix_af *nix_af, int lf)
{
	struct nix_aq_rq_request rq_req ALIGNED;
	int err;

	debug("%s(%p, %d)\n", __func__, nix_af, lf);

	memset(&rq_req, 0, sizeof(struct nix_aq_rq_request));

	rq_req.rq.s.ena = 1;
	rq_req.rq.s.spb_ena = 1;
	rq_req.rq.s.ipsech_ena = 0;
	rq_req.rq.s.ena_wqwd = 0;
	rq_req.rq.s.cq = NIX_CQ_RX;
	rq_req.rq.s.substream = 0;	/* FIXME: Substream IDs? */
	rq_req.rq.s.wqe_aura = -1;	/* No WQE aura */
	rq_req.rq.s.spb_aura = NPA_POOL_RX;
	rq_req.rq.s.lpb_aura = NPA_POOL_RX;
	/* U-Boot doesn't use WQE group for anything */
	rq_req.rq.s.pb_caching = 1;
	rq_req.rq.s.xqe_drop_ena = 0;	/* Disable RED dropping */
	rq_req.rq.s.spb_drop_ena = 0;
	rq_req.rq.s.lpb_drop_ena = 0;
	rq_req.rq.s.spb_sizem1 = (MAX_MTU / (3 * 8)) - 1; /* 512 bytes */
	rq_req.rq.s.lpb_sizem1 = (MAX_MTU / 8) - 1;
	rq_req.rq.s.first_skip = 0;
	rq_req.rq.s.later_skip = 0;
	rq_req.rq.s.xqe_imm_copy = 0;
	rq_req.rq.s.xqe_hdr_split = 0;
	rq_req.rq.s.xqe_drop = 0;
	rq_req.rq.s.xqe_pass = 0;
	rq_req.rq.s.wqe_pool_drop = 0;	/* No WQE pool */
	rq_req.rq.s.wqe_pool_pass = 0;	/* No WQE pool */
	rq_req.rq.s.spb_aura_drop = 255;
	rq_req.rq.s.spb_aura_pass = 255;
	rq_req.rq.s.spb_pool_drop = 0;
	rq_req.rq.s.spb_pool_pass = 0;
	rq_req.rq.s.lpb_aura_drop = 255;
	rq_req.rq.s.lpb_aura_pass = 255;
	rq_req.rq.s.lpb_pool_drop = 0;
	rq_req.rq.s.lpb_pool_pass = 0;
	rq_req.rq.s.qint_idx = 0;

	err = nix_aq_issue_command(nix_af, lf,
				   NIX_AQ_INSTOP_E_INIT,
				   NIX_AQ_CTYPE_E_RQ,
				   0, &rq_req.resp);
	if (err) {
		printf("%s: Error requesting send queue\n", __func__);
		return err;
	}

	return 0;
}

static int nix_attach_send_queue(struct nix *nix)
{
	struct nix_af *nix_af = nix->nix_af;
	struct nix_aq_sq_request sq_req ALIGNED;
	int err;

	debug("%s(%p)\n", __func__, nix_af);
	err = nix_af_setup_sq(nix);

	memset(&sq_req, 0, sizeof(sq_req));

	sq_req.sq.s.ena = 1;
	sq_req.sq.s.cq_ena = 1;
	sq_req.sq.s.max_sqe_size = NIX_MAXSQESZ_E_W16;
	sq_req.sq.s.substream = 0; // FIXME: Substream IDs?
	sq_req.sq.s.sdp_mcast = 0;
	sq_req.sq.s.cq = NIX_CQ_TX;
	sq_req.sq.s.cq_limit = 0;
	sq_req.sq.s.smq = nix->lmac->link_num; // scheduling index
	sq_req.sq.s.sso_ena = 0;
	sq_req.sq.s.smq_rr_quantum = MAX_MTU / 4;
	sq_req.sq.s.default_chan = nix->lmac->chan_num;
	sq_req.sq.s.sqe_stype = NIX_STYPE_E_STP;
	sq_req.sq.s.qint_idx = 0;
	sq_req.sq.s.sqb_aura = NPA_POOL_SQB;

	err = nix_aq_issue_command(nix_af, nix->lf,
				   NIX_AQ_INSTOP_E_INIT,
				   NIX_AQ_CTYPE_E_SQ,
				   0, &sq_req.resp);
	if (err) {
		printf("%s: Error requesting send queue\n", __func__);
		return err;
	}

	return 0;
}

static int nix_attach_completion_queue(struct nix *nix, int cq_idx)
{
	struct nix_af *nix_af = nix->nix_af;
	struct nix_aq_cq_request cq_req ALIGNED;
	int err;

	debug("%s(%p)\n", __func__, nix_af);
	memset(&cq_req, 0, sizeof(cq_req));
	cq_req.cq.s.ena = 1;
	cq_req.cq.s.bpid = nix->lmac->pknd;
	cq_req.cq.s.substream = 0;	/* FIXME: Substream IDs? */
	cq_req.cq.s.drop_ena = 0;
	cq_req.cq.s.caching = 1;
	cq_req.cq.s.qsize = CQS_QSIZE;
	cq_req.cq.s.drop = 255 * 7 / 8;
	cq_req.cq.s.qint_idx = 0;
	cq_req.cq.s.cint_idx = 0;
	cq_req.cq.s.base = nix->cq[cq_idx].iova;
	debug("%s: CQ(%d)  base %p\n", __func__, cq_idx,
	      nix->cq[cq_idx].base);

	err = nix_aq_issue_command(nix_af, nix->lf,
				   NIX_AQ_INSTOP_E_INIT,
				   NIX_AQ_CTYPE_E_CQ,
				   cq_idx, &cq_req.resp);
	if (err) {
		printf("%s: Error requesting completion queue\n", __func__);
		return err;
	}
	debug("%s: CQ(%d) allocated, base %p\n", __func__, cq_idx,
	      nix->cq[cq_idx].base);

	return 0;
}

int nix_lf_admin_setup(struct nix *nix)
{
	union nixx_af_lfx_rqs_cfg rqs_cfg;
	union nixx_af_lfx_sqs_cfg sqs_cfg;
	union nixx_af_lfx_cqs_cfg cqs_cfg;
	union nixx_af_lfx_rss_cfg rss_cfg;
	union nixx_af_lfx_cints_cfg cints_cfg;
	union nixx_af_lfx_qints_cfg qints_cfg;
	union nixx_af_lfx_rss_grpx rss_grp;
	union nixx_af_lfx_tx_cfg2 tx_cfg2;
	union nixx_af_lfx_cfg lfx_cfg;
	union nixx_af_lf_rst lf_rst;
	u32 index;
	struct nix_af *nix_af = nix->nix_af;
	int err;

	/* Reset the LF */
	lf_rst.u = 0;
	lf_rst.s.lf = nix->lf;
	lf_rst.s.exec = 1;
	nix_af_reg_write(nix_af, NIXX_AF_LF_RST(), lf_rst.u);

	do {
		lf_rst.u = nix_af_reg_read(nix_af, NIXX_AF_LF_RST());
		schedule();
	} while (lf_rst.s.exec);

	/* Config NIX RQ HW context and base*/
	nix_af_reg_write(nix_af, NIXX_AF_LFX_RQS_BASE(nix->lf),
			 (u64)nix->rq_ctx_base);
	/* Set caching and queue count in HW */
	rqs_cfg.u = nix_af_reg_read(nix_af, NIXX_AF_LFX_RQS_CFG(nix->lf));
	rqs_cfg.s.caching = 1;
	rqs_cfg.s.max_queuesm1 = nix->rq_cnt - 1;
	nix_af_reg_write(nix_af, NIXX_AF_LFX_RQS_CFG(nix->lf), rqs_cfg.u);

	/* Config NIX SQ HW context and base*/
	nix_af_reg_write(nix_af, NIXX_AF_LFX_SQS_BASE(nix->lf),
			 (u64)nix->sq_ctx_base);
	sqs_cfg.u = nix_af_reg_read(nix_af, NIXX_AF_LFX_SQS_CFG(nix->lf));
	sqs_cfg.s.caching = 1;
	sqs_cfg.s.max_queuesm1 = nix->sq_cnt - 1;
	nix_af_reg_write(nix_af, NIXX_AF_LFX_SQS_CFG(nix->lf), sqs_cfg.u);

	/* Config NIX CQ HW context and base*/
	nix_af_reg_write(nix_af, NIXX_AF_LFX_CQS_BASE(nix->lf),
			 (u64)nix->cq_ctx_base);
	cqs_cfg.u = nix_af_reg_read(nix_af, NIXX_AF_LFX_CQS_CFG(nix->lf));
	cqs_cfg.s.caching = 1;
	cqs_cfg.s.max_queuesm1 = nix->cq_cnt - 1;
	nix_af_reg_write(nix_af, NIXX_AF_LFX_CQS_CFG(nix->lf), cqs_cfg.u);

	/* Config NIX RSS HW context and base */
	nix_af_reg_write(nix_af, NIXX_AF_LFX_RSS_BASE(nix->lf),
			 (u64)nix->rss_base);
	rss_cfg.u = nix_af_reg_read(nix_af, NIXX_AF_LFX_RSS_CFG(nix->lf));
	rss_cfg.s.ena = 1;
	rss_cfg.s.size = ilog2(nix->rss_sz) / 256;
	nix_af_reg_write(nix_af, NIXX_AF_LFX_RSS_CFG(nix->lf), rss_cfg.u);

	for (index = 0; index < nix->rss_grps; index++) {
		rss_grp.u = 0;
		rss_grp.s.sizem1 = 0x7;
		rss_grp.s.offset = nix->rss_sz * index;
		nix_af_reg_write(nix_af,
				 NIXX_AF_LFX_RSS_GRPX(nix->lf, index),
				 rss_grp.u);
	}

	/* Config CQints HW contexts and base */
	nix_af_reg_write(nix_af, NIXX_AF_LFX_CINTS_BASE(nix->lf),
			 (u64)nix->cint_base);
	cints_cfg.u = nix_af_reg_read(nix_af,
				      NIXX_AF_LFX_CINTS_CFG(nix->lf));
	cints_cfg.s.caching = 1;
	nix_af_reg_write(nix_af, NIXX_AF_LFX_CINTS_CFG(nix->lf),
			 cints_cfg.u);

	/* Config Qints HW context and base */
	nix_af_reg_write(nix_af, NIXX_AF_LFX_QINTS_BASE(nix->lf),
			 (u64)nix->qint_base);
	qints_cfg.u = nix_af_reg_read(nix_af,
				      NIXX_AF_LFX_QINTS_CFG(nix->lf));
	qints_cfg.s.caching = 1;
	nix_af_reg_write(nix_af, NIXX_AF_LFX_QINTS_CFG(nix->lf),
			 qints_cfg.u);

	debug("%s(%p, %d, %d)\n", __func__, nix_af, nix->lf, nix->pf);

	/* Enable LMTST for this NIX LF */
	tx_cfg2.u = nix_af_reg_read(nix_af, NIXX_AF_LFX_TX_CFG2(nix->lf));
	tx_cfg2.s.lmt_ena = 1;
	nix_af_reg_write(nix_af, NIXX_AF_LFX_TX_CFG2(nix->lf), tx_cfg2.u);

	/* Use 16-word XQEs, write the npa pf_func number only */
	lfx_cfg.u = nix_af_reg_read(nix_af, NIXX_AF_LFX_CFG(nix->lf));
	lfx_cfg.s.xqe_size = NIX_XQESZ_E_W16;
	lfx_cfg.s.npa_pf_func = nix->pf_func;
	nix_af_reg_write(nix_af, NIXX_AF_LFX_CFG(nix->lf), lfx_cfg.u);

	nix_af_reg_write(nix_af, NIXX_AF_LFX_RX_CFG(nix->lf), 0);

	for (index = 0; index < nix->cq_cnt; index++) {
		err = nix_attach_completion_queue(nix, index);
		if (err) {
			printf("%s: Error attaching completion queue %d\n",
			       __func__, index);
			return err;
		}
	}

	for (index = 0; index < nix->rq_cnt; index++) {
		err = nix_attach_receive_queue(nix_af, nix->lf);
		if (err) {
			printf("%s: Error attaching receive queue %d\n",
			       __func__, index);
			return err;
		}
	}

	for (index = 0; index < nix->sq_cnt; index++) {
		err = nix_attach_send_queue(nix);
		if (err) {
			printf("%s: Error attaching send queue %d\n",
			       __func__, index);
			return err;
		}
	}

	return 0;
}

int nix_lf_admin_shutdown(struct nix_af *nix_af, int lf,
			  u32 cq_count, u32 rq_count, u32 sq_count)
{
	union nixx_af_rx_sw_sync sw_sync;
	union nixx_af_lf_rst lf_rst;
	int index, err;

	/* Flush all tx packets */
	sw_sync.u = 0;
	sw_sync.s.ena = 1;
	nix_af_reg_write(nix_af, NIXX_AF_RX_SW_SYNC(), sw_sync.u);

	do {
		sw_sync.u = nix_af_reg_read(nix_af, NIXX_AF_RX_SW_SYNC());
		schedule();
	} while (sw_sync.s.ena);

	for (index = 0; index < rq_count; index++) {
		memset((void *)&rq_dis, 0, sizeof(rq_dis));
		rq_dis.rq.s.ena = 0;	/* Context */
		rq_dis.mrq.s.ena = 1;	/* Mask */
		__iowmb();

		err = nix_aq_issue_command(nix_af, lf,
					   NIX_AQ_INSTOP_E_WRITE,
					   NIX_AQ_CTYPE_E_RQ,
					   index, &rq_dis.resp);
		if (err) {
			printf("%s: Error disabling LF %d RQ(%d)\n",
			       __func__, lf, index);
			return err;
		}
		debug("%s: LF %d RQ(%d) disabled\n", __func__, lf, index);
	}

	for (index = 0; index < sq_count; index++) {
		memset((void *)&sq_dis, 0, sizeof(sq_dis));
		sq_dis.sq.s.ena = 0;	/* Context */
		sq_dis.msq.s.ena = 1;	/* Mask */
		__iowmb();

		err = nix_aq_issue_command(nix_af, lf,
					   NIX_AQ_INSTOP_E_WRITE,
					   NIX_AQ_CTYPE_E_SQ,
					   index, &sq_dis.resp);
		if (err) {
			printf("%s: Error disabling LF %d SQ(%d)\n",
			       __func__, lf, index);
			return err;
		}
		debug("%s: LF %d SQ(%d) disabled\n", __func__, lf, index);
	}

	for (index = 0; index < cq_count; index++) {
		memset((void *)&cq_dis, 0, sizeof(cq_dis));
		cq_dis.cq.s.ena = 0;	/* Context */
		cq_dis.mcq.s.ena = 1;	/* Mask */
		__iowmb();

		err = nix_aq_issue_command(nix_af, lf,
					   NIX_AQ_INSTOP_E_WRITE,
					   NIX_AQ_CTYPE_E_CQ,
					   index, &cq_dis.resp);
		if (err) {
			printf("%s: Error disabling LF %d CQ(%d)\n",
			       __func__, lf, index);
			return err;
		}
		debug("%s: LF %d CQ(%d) disabled\n", __func__, lf, index);
	}

	/* Reset the LF */
	lf_rst.u = 0;
	lf_rst.s.lf = lf;
	lf_rst.s.exec = 1;
	nix_af_reg_write(nix_af, NIXX_AF_LF_RST(), lf_rst.u);

	do {
		lf_rst.u = nix_af_reg_read(nix_af, NIXX_AF_LF_RST());
		schedule();
	} while (lf_rst.s.exec);

	return 0;
}

int npc_lf_admin_setup(struct nix *nix)
{
	union npc_af_const af_const;
	union npc_af_pkindx_action0 action0;
	union npc_af_pkindx_action1 action1;
	union npc_af_intfx_kex_cfg kex_cfg;
	union npc_af_intfx_miss_stat_act intfx_stat_act;
	union npc_af_mcamex_bankx_camx_intf camx_intf;
	union npc_af_mcamex_bankx_camx_w0 camx_w0;
	union npc_af_mcamex_bankx_cfg bankx_cfg;
	union npc_af_mcamex_bankx_stat_act mcamex_stat_act;

	union nix_rx_action_s rx_action;
	union nix_tx_action_s tx_action;

	struct nix_af *nix_af = nix->nix_af;
	u32 kpus;
	int pkind = nix->lmac->link_num;
	int index;
	u64 offset;

	debug("%s(%p, pkind 0x%x)\n", __func__, nix_af, pkind);
	af_const.u = npc_af_reg_read(nix_af, NPC_AF_CONST());
	kpus = af_const.s.kpus;

	action0.u = 0;
	action0.s.parse_done = 1;
	npc_af_reg_write(nix_af, NPC_AF_PKINDX_ACTION0(pkind), action0.u);

	action1.u = 0;
	npc_af_reg_write(nix_af, NPC_AF_PKINDX_ACTION1(pkind), action1.u);

	kex_cfg.u = 0;
	kex_cfg.s.keyw = NPC_MCAMKEYW_E_X1;
	kex_cfg.s.parse_nibble_ena = 0x7;
	npc_af_reg_write(nix_af,
			 NPC_AF_INTFX_KEX_CFG(NPC_INTF_E_NIXX_RX(0)),
			 kex_cfg.u);

	/* HW Issue */
	kex_cfg.u = 0;
	kex_cfg.s.parse_nibble_ena = 0x7;
	npc_af_reg_write(nix_af,
			 NPC_AF_INTFX_KEX_CFG(NPC_INTF_E_NIXX_TX(0)),
			 kex_cfg.u);

	camx_intf.u = 0;
	camx_intf.s.intf = ~NPC_INTF_E_NIXX_RX(0);
	npc_af_reg_write(nix_af,
			 NPC_AF_MCAMEX_BANKX_CAMX_INTF(pkind, 0, 0),
			 camx_intf.u);

	camx_intf.u = 0;
	camx_intf.s.intf = NPC_INTF_E_NIXX_RX(0);
	npc_af_reg_write(nix_af,
			 NPC_AF_MCAMEX_BANKX_CAMX_INTF(pkind, 0, 1),
			 camx_intf.u);

	camx_w0.u = 0;
	camx_w0.s.md = ~(nix->lmac->chan_num) & (~((~0x0ull) << 12));
	debug("NPC LF ADMIN camx_w0.u %llx\n", camx_w0.u);
	npc_af_reg_write(nix_af,
			 NPC_AF_MCAMEX_BANKX_CAMX_W0(pkind, 0, 0),
			 camx_w0.u);

	camx_w0.u = 0;
	camx_w0.s.md = nix->lmac->chan_num;
	npc_af_reg_write(nix_af,
			 NPC_AF_MCAMEX_BANKX_CAMX_W0(pkind, 0, 1),
			 camx_w0.u);

	npc_af_reg_write(nix_af, NPC_AF_MCAMEX_BANKX_CAMX_W1(pkind, 0, 0),
			 0);

	npc_af_reg_write(nix_af, NPC_AF_MCAMEX_BANKX_CAMX_W1(pkind, 0, 1),
			 0);

	/* Enable stats for NPC INTF RX */
	mcamex_stat_act.u = 0;
	mcamex_stat_act.s.ena = 1;
	mcamex_stat_act.s.stat_sel = pkind;
	npc_af_reg_write(nix_af,
			 NPC_AF_MCAMEX_BANKX_STAT_ACT(pkind, 0),
			 mcamex_stat_act.u);
	intfx_stat_act.u = 0;
	intfx_stat_act.s.ena = 1;
	intfx_stat_act.s.stat_sel = 16;
	offset = NPC_AF_INTFX_MISS_STAT_ACT(NPC_INTF_E_NIXX_RX(0));
	npc_af_reg_write(nix_af, offset, intfx_stat_act.u);
	rx_action.u = 0;
	rx_action.s.pf_func = nix->pf_func;
	rx_action.s.op = NIX_RX_ACTIONOP_E_UCAST;
	npc_af_reg_write(nix_af, NPC_AF_MCAMEX_BANKX_ACTION(pkind, 0),
			 rx_action.u);

	for (index = 0; index < kpus; index++)
		npc_af_reg_write(nix_af, NPC_AF_KPUX_CFG(index), 0);

	rx_action.u = 0;
	rx_action.s.pf_func = nix->pf_func;
	rx_action.s.op = NIX_RX_ACTIONOP_E_DROP;
	npc_af_reg_write(nix_af,
			 NPC_AF_INTFX_MISS_ACT(NPC_INTF_E_NIXX_RX(0)),
			 rx_action.u);
	bankx_cfg.u = 0;
	bankx_cfg.s.ena = 1;
	npc_af_reg_write(nix_af, NPC_AF_MCAMEX_BANKX_CFG(pkind, 0),
			 bankx_cfg.u);

	tx_action.u = 0;
	tx_action.s.op = NIX_TX_ACTIONOP_E_UCAST_DEFAULT;
	npc_af_reg_write(nix_af,
			 NPC_AF_INTFX_MISS_ACT(NPC_INTF_E_NIXX_TX(0)),
			 tx_action.u);

#ifdef DEBUG
	/* Enable debug capture on RX intf */
	npc_af_reg_write(nix_af, NPC_AF_DBG_CTL(), 0x4);
#endif

	return 0;
}

int npc_af_shutdown(struct nix_af *nix_af)
{
	union npc_af_blk_rst blk_rst;

	blk_rst.u = 0;
	blk_rst.s.rst = 1;
	npc_af_reg_write(nix_af, NPC_AF_BLK_RST(), blk_rst.u);

	/* Wait for reset to complete */
	do {
		blk_rst.u = npc_af_reg_read(nix_af, NPC_AF_BLK_RST());
		schedule();
	} while (blk_rst.s.busy);

	debug("%s: npc af reset --\n", __func__);

	return 0;
}

int nix_af_setup(struct nix_af *nix_af)
{
	int err;
	union nixx_af_const2 af_const2;
	union nixx_af_const3 af_const3;
	union nixx_af_sq_const sq_const;
	union nixx_af_cfg af_cfg;
	union nixx_af_status af_status;
	union nixx_af_ndc_cfg ndc_cfg;
	union nixx_af_aq_cfg aq_cfg;
	union nixx_af_blk_rst blk_rst;

	debug("%s(%p)\n", __func__, nix_af);
	err = rvu_aq_alloc(&nix_af->aq, Q_COUNT(AQ_SIZE),
			   sizeof(union nix_aq_inst_s),
			   sizeof(union nix_aq_res_s));
	if (err) {
		printf("%s: Error allocating nix admin queue\n", __func__);
		return err;
	}

	blk_rst.u = 0;
	blk_rst.s.rst = 1;
	nix_af_reg_write(nix_af, NIXX_AF_BLK_RST(), blk_rst.u);

	/* Wait for reset to complete */
	do {
		blk_rst.u = nix_af_reg_read(nix_af, NIXX_AF_BLK_RST());
		schedule();
	} while (blk_rst.s.busy);

	/* Put in LE mode */
	af_cfg.u = nix_af_reg_read(nix_af, NIXX_AF_CFG());
	if (af_cfg.s.force_cond_clk_en || af_cfg.s.calibrate_x2p ||
	    af_cfg.s.force_intf_clk_en) {
		printf("%s: Error: Invalid NIX_AF_CFG value 0x%llx\n",
		       __func__, af_cfg.u);
		return -1;
	}
	af_cfg.s.af_be = 0;
	af_cfg.u |= 0x5E;	/* HW Issue */
	nix_af_reg_write(nix_af, NIXX_AF_CFG(), af_cfg.u);

	/* Perform Calibration */
	af_cfg.u = nix_af_reg_read(nix_af, NIXX_AF_CFG());
	af_cfg.s.calibrate_x2p = 1;
	nix_af_reg_write(nix_af, NIXX_AF_CFG(), af_cfg.u);

	/* Wait for calibration to complete */
	do {
		af_status.u = nix_af_reg_read(nix_af, NIXX_AF_STATUS());
		schedule();
	} while (af_status.s.calibrate_done == 0);

	af_cfg.u = nix_af_reg_read(nix_af, NIXX_AF_CFG());
	af_cfg.s.calibrate_x2p = 0;
	nix_af_reg_write(nix_af, NIXX_AF_CFG(), af_cfg.u);

	/* Enable NDC cache */
	ndc_cfg.u = nix_af_reg_read(nix_af, NIXX_AF_NDC_CFG());
	ndc_cfg.s.ndc_ign_pois = 0;
	ndc_cfg.s.byp_sq = 0;
	ndc_cfg.s.byp_sqb = 0;
	ndc_cfg.s.byp_cqs = 0;
	ndc_cfg.s.byp_cints = 0;
	ndc_cfg.s.byp_dyno = 0;
	ndc_cfg.s.byp_mce = 0;
	ndc_cfg.s.byp_rqc = 0;
	ndc_cfg.s.byp_rsse = 0;
	ndc_cfg.s.byp_mc_data = 0;
	ndc_cfg.s.byp_mc_wqe = 0;
	ndc_cfg.s.byp_mr_data = 0;
	ndc_cfg.s.byp_mr_wqe = 0;
	ndc_cfg.s.byp_qints = 0;
	nix_af_reg_write(nix_af, NIXX_AF_NDC_CFG(), ndc_cfg.u);

	/* Set up queue size */
	aq_cfg.u = 0;
	aq_cfg.s.qsize = AQ_SIZE;
	nix_af_reg_write(nix_af, NIXX_AF_AQ_CFG(), aq_cfg.u);

	/* Set up queue base address */
	nix_af_reg_write(nix_af, NIXX_AF_AQ_BASE(), nix_af->aq.inst.iova);

	af_const3.u = nix_af_reg_read(nix_af, NIXX_AF_CONST3());
	af_const2.u = nix_af_reg_read(nix_af, NIXX_AF_CONST2());
	sq_const.u = nix_af_reg_read(nix_af, NIXX_AF_SQ_CONST());
	nix_af->rq_ctx_sz = 1ULL << af_const3.s.rq_ctx_log2bytes;
	nix_af->sq_ctx_sz = 1ULL << af_const3.s.sq_ctx_log2bytes;
	nix_af->cq_ctx_sz = 1ULL << af_const3.s.cq_ctx_log2bytes;
	nix_af->rsse_ctx_sz = 1ULL << af_const3.s.rsse_log2bytes;
	nix_af->qints = af_const2.s.qints;
	nix_af->cints = af_const2.s.cints;
	nix_af->cint_ctx_sz = 1ULL << af_const3.s.cint_log2bytes;
	nix_af->qint_ctx_sz = 1ULL << af_const3.s.qint_log2bytes;
	nix_af->sqb_size = sq_const.s.sqb_size;

	return 0;
}

int nix_af_shutdown(struct nix_af *nix_af)
{
	union nixx_af_blk_rst blk_rst;

	blk_rst.u = 0;
	blk_rst.s.rst = 1;
	nix_af_reg_write(nix_af, NIXX_AF_BLK_RST(), blk_rst.u);

	/* Wait for reset to complete */
	do {
		blk_rst.u = nix_af_reg_read(nix_af, NIXX_AF_BLK_RST());
		schedule();
	} while (blk_rst.s.busy);

	rvu_aq_free(&nix_af->aq);

	debug("%s: nix af reset --\n", __func__);

	return 0;
}
