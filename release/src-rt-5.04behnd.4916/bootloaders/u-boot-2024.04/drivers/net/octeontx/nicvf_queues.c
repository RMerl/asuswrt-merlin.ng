// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#include <cpu_func.h>
#include <dm/device.h>
#include <malloc.h>
#include <net.h>
#include <phy.h>
#include <linux/delay.h>

#include "nic_reg.h"
#include "nic.h"
#include "q_struct.h"
#include "nicvf_queues.h"

static int nicvf_poll_reg(struct nicvf *nic, int qidx,
			  u64 reg, int bit_pos, int bits, int val)
{
	u64 bit_mask;
	u64 reg_val;
	int timeout = 10;

	bit_mask = (1ULL << bits) - 1;
	bit_mask = (bit_mask << bit_pos);

	while (timeout) {
		reg_val = nicvf_queue_reg_read(nic, reg, qidx);
		if (((reg_val & bit_mask) >> bit_pos) == val)
			return 0;
		udelay(2000);
		timeout--;
	}
	printf("Poll on reg 0x%llx failed\n", reg);
	return 1;
}

static int nicvf_alloc_q_desc_mem(struct nicvf *nic, struct q_desc_mem *dmem,
				  int q_len, int desc_size, int align_bytes)
{
	dmem->q_len = q_len;
	dmem->size = (desc_size * q_len) + align_bytes;
	/* Save address, need it while freeing */
	dmem->unalign_base = calloc(1, dmem->size);
	dmem->dma = (uintptr_t)dmem->unalign_base;

	if (!dmem->unalign_base)
		return -1;

	/* Align memory address for 'align_bytes' */
	dmem->phys_base = NICVF_ALIGNED_ADDR((u64)dmem->dma, align_bytes);
	dmem->base = dmem->unalign_base + (dmem->phys_base - dmem->dma);

	return 0;
}

static void nicvf_free_q_desc_mem(struct nicvf *nic, struct q_desc_mem *dmem)
{
	if (!dmem)
		return;

	free(dmem->unalign_base);

	dmem->unalign_base = NULL;
	dmem->base = NULL;
}

static void *nicvf_rb_ptr_to_pkt(struct nicvf *nic, uintptr_t rb_ptr)
{
	return (void *)rb_ptr;
}

static int nicvf_init_rbdr(struct nicvf *nic, struct rbdr *rbdr,
			   int ring_len, int buf_size)
{
	int idx;
	uintptr_t rbuf;
	struct rbdr_entry_t *desc;

	if (nicvf_alloc_q_desc_mem(nic, &rbdr->dmem, ring_len,
				   sizeof(struct rbdr_entry_t),
				   NICVF_RCV_BUF_ALIGN_BYTES)) {
		printf("Unable to allocate memory for rcv buffer ring\n");
		return -1;
	}

	rbdr->desc = rbdr->dmem.base;
	/* Buffer size has to be in multiples of 128 bytes */
	rbdr->dma_size = buf_size;
	rbdr->enable = true;
	rbdr->thresh = RBDR_THRESH;

	debug("%s: %d: allocating %lld bytes for rcv buffers\n",
	      __func__, __LINE__,
	      ring_len * buf_size + NICVF_RCV_BUF_ALIGN_BYTES);
	rbdr->buf_mem = (uintptr_t)calloc(1, ring_len * buf_size
						+ NICVF_RCV_BUF_ALIGN_BYTES);

	if (!rbdr->buf_mem) {
		printf("Unable to allocate memory for rcv buffers\n");
		return -1;
	}

	rbdr->buffers = NICVF_ALIGNED_ADDR(rbdr->buf_mem,
					   NICVF_RCV_BUF_ALIGN_BYTES);

	debug("%s: %d: rbdr->buf_mem: %lx, rbdr->buffers: %lx\n",
	      __func__, __LINE__, rbdr->buf_mem, rbdr->buffers);

	for (idx = 0; idx < ring_len; idx++) {
		rbuf = rbdr->buffers + DMA_BUFFER_LEN * idx;
		desc = GET_RBDR_DESC(rbdr, idx);
		desc->buf_addr = rbuf >> NICVF_RCV_BUF_ALIGN;
		flush_dcache_range((uintptr_t)desc,
				   (uintptr_t)desc + sizeof(desc));
	}
	return 0;
}

static void nicvf_free_rbdr(struct nicvf *nic, struct rbdr *rbdr)
{
	if (!rbdr)
		return;

	rbdr->enable = false;
	if (!rbdr->dmem.base)
		return;

	debug("%s: %d: rbdr->buf_mem: %p\n", __func__,
	      __LINE__, (void *)rbdr->buf_mem);
	free((void *)rbdr->buf_mem);

	/* Free RBDR ring */
	nicvf_free_q_desc_mem(nic, &rbdr->dmem);
}

/* Refill receive buffer descriptors with new buffers.
 * This runs in softirq context .
 */
void nicvf_refill_rbdr(struct nicvf *nic)
{
	struct queue_set *qs = nic->qs;
	int rbdr_idx = qs->rbdr_cnt;
	unsigned long qcount, head, tail, rb_cnt;
	struct rbdr *rbdr;

	if (!rbdr_idx)
		return;
	rbdr_idx--;
	rbdr = &qs->rbdr[rbdr_idx];
	/* Check if it's enabled */
	if (!rbdr->enable) {
		printf("Receive queue %d is disabled\n", rbdr_idx);
		return;
	}

	/* check if valid descs reached or crossed threshold level */
	qcount = nicvf_queue_reg_read(nic, NIC_QSET_RBDR_0_1_STATUS0, rbdr_idx);
	head = nicvf_queue_reg_read(nic, NIC_QSET_RBDR_0_1_HEAD, rbdr_idx);
	tail = nicvf_queue_reg_read(nic, NIC_QSET_RBDR_0_1_TAIL, rbdr_idx);

	qcount &= 0x7FFFF;

	rb_cnt = qs->rbdr_len - qcount - 1;

	debug("%s: %d: qcount: %lu, head: %lx, tail: %lx, rb_cnt: %lu\n",
	      __func__, __LINE__, qcount, head, tail, rb_cnt);

	/* Notify HW */
	nicvf_queue_reg_write(nic, NIC_QSET_RBDR_0_1_DOOR, rbdr_idx, rb_cnt);

	asm volatile ("dsb sy");
}

/* TBD: how to handle full packets received in CQ
 * i.e conversion of buffers into SKBs
 */
static int nicvf_init_cmp_queue(struct nicvf *nic,
				struct cmp_queue *cq, int q_len)
{
	if (nicvf_alloc_q_desc_mem(nic, &cq->dmem, q_len,
				   CMP_QUEUE_DESC_SIZE,
				   NICVF_CQ_BASE_ALIGN_BYTES)) {
		printf("Unable to allocate memory for completion queue\n");
		return -1;
	}
	cq->desc = cq->dmem.base;
	if (!pass1_silicon(nic->rev_id, nic->nicpf->hw->model_id))
		cq->thresh = CMP_QUEUE_CQE_THRESH;
	else
		cq->thresh = 0;
	cq->intr_timer_thresh = CMP_QUEUE_TIMER_THRESH;

	return 0;
}

static void nicvf_free_cmp_queue(struct nicvf *nic, struct cmp_queue *cq)
{
	if (!cq)
		return;
	if (!cq->dmem.base)
		return;

	nicvf_free_q_desc_mem(nic, &cq->dmem);
}

static int nicvf_init_snd_queue(struct nicvf *nic,
				struct snd_queue *sq, int q_len)
{
	if (nicvf_alloc_q_desc_mem(nic, &sq->dmem, q_len,
				   SND_QUEUE_DESC_SIZE,
				   NICVF_SQ_BASE_ALIGN_BYTES)) {
		printf("Unable to allocate memory for send queue\n");
		return -1;
	}

	sq->desc = sq->dmem.base;
	sq->skbuff = calloc(q_len, sizeof(u64));
	sq->head = 0;
	sq->tail = 0;
	sq->free_cnt = q_len - 1;
	sq->thresh = SND_QUEUE_THRESH;

	return 0;
}

static void nicvf_free_snd_queue(struct nicvf *nic, struct snd_queue *sq)
{
	if (!sq)
		return;
	if (!sq->dmem.base)
		return;

	debug("%s: %d\n", __func__, __LINE__);
	free(sq->skbuff);

	nicvf_free_q_desc_mem(nic, &sq->dmem);
}

static void nicvf_reclaim_snd_queue(struct nicvf *nic,
				    struct queue_set *qs, int qidx)
{
	/* Disable send queue */
	nicvf_queue_reg_write(nic, NIC_QSET_SQ_0_7_CFG, qidx, 0);
	/* Check if SQ is stopped */
	if (nicvf_poll_reg(nic, qidx, NIC_QSET_SQ_0_7_STATUS, 21, 1, 0x01))
		return;
	/* Reset send queue */
	nicvf_queue_reg_write(nic, NIC_QSET_SQ_0_7_CFG, qidx, NICVF_SQ_RESET);
}

static void nicvf_reclaim_rcv_queue(struct nicvf *nic,
				    struct queue_set *qs, int qidx)
{
	union nic_mbx mbx = {};

	/* Make sure all packets in the pipeline are written back into mem */
	mbx.msg.msg = NIC_MBOX_MSG_RQ_SW_SYNC;
	nicvf_send_msg_to_pf(nic, &mbx);
}

static void nicvf_reclaim_cmp_queue(struct nicvf *nic,
				    struct queue_set *qs, int qidx)
{
	/* Disable timer threshold (doesn't get reset upon CQ reset */
	nicvf_queue_reg_write(nic, NIC_QSET_CQ_0_7_CFG2, qidx, 0);
	/* Disable completion queue */
	nicvf_queue_reg_write(nic, NIC_QSET_CQ_0_7_CFG, qidx, 0);
	/* Reset completion queue */
	nicvf_queue_reg_write(nic, NIC_QSET_CQ_0_7_CFG, qidx, NICVF_CQ_RESET);
}

static void nicvf_reclaim_rbdr(struct nicvf *nic,
			       struct rbdr *rbdr, int qidx)
{
	u64 tmp, fifo_state;
	int timeout = 10;

	/* Save head and tail pointers for feeing up buffers */
	rbdr->head = nicvf_queue_reg_read(nic,
					  NIC_QSET_RBDR_0_1_HEAD,
					  qidx) >> 3;
	rbdr->tail = nicvf_queue_reg_read(nic,
					  NIC_QSET_RBDR_0_1_TAIL,
					  qidx) >> 3;

	/* If RBDR FIFO is in 'FAIL' state then do a reset first
	 * before relaiming.
	 */
	fifo_state = nicvf_queue_reg_read(nic, NIC_QSET_RBDR_0_1_STATUS0, qidx);
	if (((fifo_state >> 62) & 0x03) == 0x3)
		nicvf_queue_reg_write(nic, NIC_QSET_RBDR_0_1_CFG,
				      qidx, NICVF_RBDR_RESET);

	/* Disable RBDR */
	nicvf_queue_reg_write(nic, NIC_QSET_RBDR_0_1_CFG, qidx, 0);
	if (nicvf_poll_reg(nic, qidx, NIC_QSET_RBDR_0_1_STATUS0, 62, 2, 0x00))
		return;
	while (1) {
		tmp = nicvf_queue_reg_read(nic,
					   NIC_QSET_RBDR_0_1_PREFETCH_STATUS,
					   qidx);
		if ((tmp & 0xFFFFFFFF) == ((tmp >> 32) & 0xFFFFFFFF))
			break;
		mdelay(2000);
		timeout--;
		if (!timeout) {
			printf("Failed polling on prefetch status\n");
			return;
		}
	}
	nicvf_queue_reg_write(nic, NIC_QSET_RBDR_0_1_CFG,
			      qidx, NICVF_RBDR_RESET);

	if (nicvf_poll_reg(nic, qidx, NIC_QSET_RBDR_0_1_STATUS0, 62, 2, 0x02))
		return;
	nicvf_queue_reg_write(nic, NIC_QSET_RBDR_0_1_CFG, qidx, 0x00);
	if (nicvf_poll_reg(nic, qidx, NIC_QSET_RBDR_0_1_STATUS0, 62, 2, 0x00))
		return;
}

/* Configures receive queue */
static void nicvf_rcv_queue_config(struct nicvf *nic, struct queue_set *qs,
				   int qidx, bool enable)
{
	union nic_mbx mbx = {};
	struct rcv_queue *rq;
	union {
		struct rq_cfg s;
		u64    u;
	} rq_cfg;

	rq = &qs->rq[qidx];
	rq->enable = enable;

	/* Disable receive queue */
	nicvf_queue_reg_write(nic, NIC_QSET_RQ_0_7_CFG, qidx, 0);

	if (!rq->enable) {
		nicvf_reclaim_rcv_queue(nic, qs, qidx);
		return;
	}

	rq->cq_qs = qs->vnic_id;
	rq->cq_idx = qidx;
	rq->start_rbdr_qs = qs->vnic_id;
	rq->start_qs_rbdr_idx = qs->rbdr_cnt - 1;
	rq->cont_rbdr_qs = qs->vnic_id;
	rq->cont_qs_rbdr_idx = qs->rbdr_cnt - 1;
	/* all writes of RBDR data to be loaded into L2 Cache as well*/
	rq->caching = 1;

	/* Send a mailbox msg to PF to config RQ */
	mbx.rq.msg = NIC_MBOX_MSG_RQ_CFG;
	mbx.rq.qs_num = qs->vnic_id;
	mbx.rq.rq_num = qidx;
	mbx.rq.cfg = (rq->caching << 26) | (rq->cq_qs << 19) |
			  (rq->cq_idx << 16) | (rq->cont_rbdr_qs << 9) |
			  (rq->cont_qs_rbdr_idx << 8) |
			  (rq->start_rbdr_qs << 1) | (rq->start_qs_rbdr_idx);
	nicvf_send_msg_to_pf(nic, &mbx);

	mbx.rq.msg = NIC_MBOX_MSG_RQ_BP_CFG;
	mbx.rq.cfg = (1ULL << 63) | (1ULL << 62) | (qs->vnic_id << 0);
	nicvf_send_msg_to_pf(nic, &mbx);

	/* RQ drop config
	 * Enable CQ drop to reserve sufficient CQEs for all tx packets
	 */
	mbx.rq.msg = NIC_MBOX_MSG_RQ_DROP_CFG;
	mbx.rq.cfg = (1ULL << 62) | (RQ_CQ_DROP << 8);
	nicvf_send_msg_to_pf(nic, &mbx);
	nicvf_queue_reg_write(nic, NIC_QSET_RQ_GEN_CFG, 0, 0x00);

	/* Enable Receive queue */
	rq_cfg.s.ena = 1;
	rq_cfg.s.tcp_ena = 0;
	nicvf_queue_reg_write(nic, NIC_QSET_RQ_0_7_CFG, qidx, rq_cfg.u);
}

void nicvf_cmp_queue_config(struct nicvf *nic, struct queue_set *qs,
			    int qidx, bool enable)
{
	struct cmp_queue *cq;
	union {
		u64 u;
		struct cq_cfg s;
	} cq_cfg;

	cq = &qs->cq[qidx];
	cq->enable = enable;

	if (!cq->enable) {
		nicvf_reclaim_cmp_queue(nic, qs, qidx);
		return;
	}

	/* Reset completion queue */
	nicvf_queue_reg_write(nic, NIC_QSET_CQ_0_7_CFG, qidx, NICVF_CQ_RESET);

	if (!cq->enable)
		return;

	/* Set completion queue base address */
	nicvf_queue_reg_write(nic, NIC_QSET_CQ_0_7_BASE,
			      qidx, (u64)(cq->dmem.phys_base));

	/* Enable Completion queue */
	cq_cfg.s.ena = 1;
	cq_cfg.s.reset = 0;
	cq_cfg.s.caching = 0;
	cq_cfg.s.qsize = CMP_QSIZE;
	cq_cfg.s.avg_con = 0;
	nicvf_queue_reg_write(nic, NIC_QSET_CQ_0_7_CFG, qidx, cq_cfg.u);

	/* Set threshold value for interrupt generation */
	nicvf_queue_reg_write(nic, NIC_QSET_CQ_0_7_THRESH, qidx, cq->thresh);
	nicvf_queue_reg_write(nic, NIC_QSET_CQ_0_7_CFG2, qidx,
			      cq->intr_timer_thresh);
}

/* Configures transmit queue */
static void nicvf_snd_queue_config(struct nicvf *nic, struct queue_set *qs,
				   int qidx, bool enable)
{
	union nic_mbx mbx = {};
	struct snd_queue *sq;

	union {
		struct sq_cfg s;
		u64 u;
	} sq_cfg;

	sq = &qs->sq[qidx];
	sq->enable = enable;

	if (!sq->enable) {
		nicvf_reclaim_snd_queue(nic, qs, qidx);
		return;
	}

	/* Reset send queue */
	nicvf_queue_reg_write(nic, NIC_QSET_SQ_0_7_CFG, qidx, NICVF_SQ_RESET);

	sq->cq_qs = qs->vnic_id;
	sq->cq_idx = qidx;

	/* Send a mailbox msg to PF to config SQ */
	mbx.sq.msg = NIC_MBOX_MSG_SQ_CFG;
	mbx.sq.qs_num = qs->vnic_id;
	mbx.sq.sq_num = qidx;
	mbx.sq.sqs_mode = nic->sqs_mode;
	mbx.sq.cfg = (sq->cq_qs << 3) | sq->cq_idx;
	nicvf_send_msg_to_pf(nic, &mbx);

	/* Set queue base address */
	nicvf_queue_reg_write(nic, NIC_QSET_SQ_0_7_BASE,
			      qidx, (u64)(sq->dmem.phys_base));

	/* Enable send queue  & set queue size */
	sq_cfg.s.ena = 1;
	sq_cfg.s.reset = 0;
	sq_cfg.s.ldwb = 0;
	sq_cfg.s.qsize = SND_QSIZE;
	sq_cfg.s.tstmp_bgx_intf = 0;
	nicvf_queue_reg_write(nic, NIC_QSET_SQ_0_7_CFG, qidx, sq_cfg.u);

	/* Set threshold value for interrupt generation */
	nicvf_queue_reg_write(nic, NIC_QSET_SQ_0_7_THRESH, qidx, sq->thresh);
}

/* Configures receive buffer descriptor ring */
static void nicvf_rbdr_config(struct nicvf *nic, struct queue_set *qs,
			      int qidx, bool enable)
{
	struct rbdr *rbdr;
	union {
		struct rbdr_cfg s;
		u64 u;
	} rbdr_cfg;

	rbdr = &qs->rbdr[qidx];
	nicvf_reclaim_rbdr(nic, rbdr, qidx);
	if (!enable)
		return;

	/* Set descriptor base address */
	nicvf_queue_reg_write(nic, NIC_QSET_RBDR_0_1_BASE,
			      qidx, (u64)(rbdr->dmem.phys_base));

	/* Enable RBDR  & set queue size */
	/* Buffer size should be in multiples of 128 bytes */
	rbdr_cfg.s.ena = 1;
	rbdr_cfg.s.reset = 0;
	rbdr_cfg.s.ldwb = 0;
	rbdr_cfg.s.qsize = RBDR_SIZE;
	rbdr_cfg.s.avg_con = 0;
	rbdr_cfg.s.lines = rbdr->dma_size / 128;
	nicvf_queue_reg_write(nic, NIC_QSET_RBDR_0_1_CFG,
			      qidx, rbdr_cfg.u);

	/* Notify HW */
	nicvf_queue_reg_write(nic, NIC_QSET_RBDR_0_1_DOOR,
			      qidx, qs->rbdr_len - 1);

	/* Set threshold value for interrupt generation */
	nicvf_queue_reg_write(nic, NIC_QSET_RBDR_0_1_THRESH,
			      qidx, rbdr->thresh - 1);
}

/* Requests PF to assign and enable Qset */
void nicvf_qset_config(struct nicvf *nic, bool enable)
{
	union nic_mbx mbx = {};
	struct queue_set *qs = nic->qs;
	struct qs_cfg *qs_cfg;

	if (!qs) {
		printf("Qset is still not allocated, don't init queues\n");
		return;
	}

	qs->enable = enable;
	qs->vnic_id = nic->vf_id;

	/* Send a mailbox msg to PF to config Qset */
	mbx.qs.msg = NIC_MBOX_MSG_QS_CFG;
	mbx.qs.num = qs->vnic_id;
#ifdef VNIC_MULTI_QSET_SUPPORT
	mbx.qs.sqs_count = nic->sqs_count;
#endif

	mbx.qs.cfg = 0;
	qs_cfg = (struct qs_cfg *)&mbx.qs.cfg;
	if (qs->enable) {
		qs_cfg->ena = 1;
#ifdef __BIG_ENDIAN
		qs_cfg->be = 1;
#endif
		qs_cfg->vnic = qs->vnic_id;
	}
	nicvf_send_msg_to_pf(nic, &mbx);
}

static void nicvf_free_resources(struct nicvf *nic)
{
	int qidx;
	struct queue_set *qs = nic->qs;

	/* Free receive buffer descriptor ring */
	for (qidx = 0; qidx < qs->rbdr_cnt; qidx++)
		nicvf_free_rbdr(nic, &qs->rbdr[qidx]);

	/* Free completion queue */
	for (qidx = 0; qidx < qs->cq_cnt; qidx++)
		nicvf_free_cmp_queue(nic, &qs->cq[qidx]);

	/* Free send queue */
	for (qidx = 0; qidx < qs->sq_cnt; qidx++)
		nicvf_free_snd_queue(nic, &qs->sq[qidx]);
}

static int nicvf_alloc_resources(struct nicvf *nic)
{
	int qidx;
	struct queue_set *qs = nic->qs;

	/* Alloc receive buffer descriptor ring */
	for (qidx = 0; qidx < qs->rbdr_cnt; qidx++) {
		if (nicvf_init_rbdr(nic, &qs->rbdr[qidx], qs->rbdr_len,
				    DMA_BUFFER_LEN))
			goto alloc_fail;
	}

	/* Alloc send queue */
	for (qidx = 0; qidx < qs->sq_cnt; qidx++) {
		if (nicvf_init_snd_queue(nic, &qs->sq[qidx], qs->sq_len))
			goto alloc_fail;
	}

	/* Alloc completion queue */
	for (qidx = 0; qidx < qs->cq_cnt; qidx++) {
		if (nicvf_init_cmp_queue(nic, &qs->cq[qidx], qs->cq_len))
			goto alloc_fail;
	}

	return 0;
alloc_fail:
	nicvf_free_resources(nic);
	return -1;
}

int nicvf_set_qset_resources(struct nicvf *nic)
{
	struct queue_set *qs;

	qs = calloc(1, sizeof(struct queue_set));
	if (!qs)
		return -1;
	nic->qs = qs;

	/* Set count of each queue */
	qs->rbdr_cnt = RBDR_CNT;
	qs->rq_cnt = 1;
	qs->sq_cnt = SND_QUEUE_CNT;
	qs->cq_cnt = CMP_QUEUE_CNT;

	/* Set queue lengths */
	qs->rbdr_len = RCV_BUF_COUNT;
	qs->sq_len = SND_QUEUE_LEN;
	qs->cq_len = CMP_QUEUE_LEN;

	nic->rx_queues = qs->rq_cnt;
	nic->tx_queues = qs->sq_cnt;

	return 0;
}

int nicvf_config_data_transfer(struct nicvf *nic, bool enable)
{
	bool disable = false;
	struct queue_set *qs = nic->qs;
	int qidx;

	if (!qs)
		return 0;

	if (enable) {
		if (nicvf_alloc_resources(nic))
			return -1;

		for (qidx = 0; qidx < qs->sq_cnt; qidx++)
			nicvf_snd_queue_config(nic, qs, qidx, enable);
		for (qidx = 0; qidx < qs->cq_cnt; qidx++)
			nicvf_cmp_queue_config(nic, qs, qidx, enable);
		for (qidx = 0; qidx < qs->rbdr_cnt; qidx++)
			nicvf_rbdr_config(nic, qs, qidx, enable);
		for (qidx = 0; qidx < qs->rq_cnt; qidx++)
			nicvf_rcv_queue_config(nic, qs, qidx, enable);
	} else {
		for (qidx = 0; qidx < qs->rq_cnt; qidx++)
			nicvf_rcv_queue_config(nic, qs, qidx, disable);
		for (qidx = 0; qidx < qs->rbdr_cnt; qidx++)
			nicvf_rbdr_config(nic, qs, qidx, disable);
		for (qidx = 0; qidx < qs->sq_cnt; qidx++)
			nicvf_snd_queue_config(nic, qs, qidx, disable);
		for (qidx = 0; qidx < qs->cq_cnt; qidx++)
			nicvf_cmp_queue_config(nic, qs, qidx, disable);

		nicvf_free_resources(nic);
	}

	return 0;
}

/* Get a free desc from SQ
 * returns descriptor ponter & descriptor number
 */
static int nicvf_get_sq_desc(struct snd_queue *sq, int desc_cnt)
{
	int qentry;

	qentry = sq->tail;
	sq->free_cnt -= desc_cnt;
	sq->tail += desc_cnt;
	sq->tail &= (sq->dmem.q_len - 1);

	return qentry;
}

/* Free descriptor back to SQ for future use */
void nicvf_put_sq_desc(struct snd_queue *sq, int desc_cnt)
{
	sq->free_cnt += desc_cnt;
	sq->head += desc_cnt;
	sq->head &= (sq->dmem.q_len - 1);
}

static int nicvf_get_nxt_sqentry(struct snd_queue *sq, int qentry)
{
	qentry++;
	qentry &= (sq->dmem.q_len - 1);
	return qentry;
}

void nicvf_sq_enable(struct nicvf *nic, struct snd_queue *sq, int qidx)
{
	u64 sq_cfg;

	sq_cfg = nicvf_queue_reg_read(nic, NIC_QSET_SQ_0_7_CFG, qidx);
	sq_cfg |= NICVF_SQ_EN;
	nicvf_queue_reg_write(nic, NIC_QSET_SQ_0_7_CFG, qidx, sq_cfg);
	/* Ring doorbell so that H/W restarts processing SQEs */
	nicvf_queue_reg_write(nic, NIC_QSET_SQ_0_7_DOOR, qidx, 0);
}

void nicvf_sq_disable(struct nicvf *nic, int qidx)
{
	u64 sq_cfg;

	sq_cfg = nicvf_queue_reg_read(nic, NIC_QSET_SQ_0_7_CFG, qidx);
	sq_cfg &= ~NICVF_SQ_EN;
	nicvf_queue_reg_write(nic, NIC_QSET_SQ_0_7_CFG, qidx, sq_cfg);
}

void nicvf_sq_free_used_descs(struct udevice *dev, struct snd_queue *sq,
			      int qidx)
{
	u64 head;
	struct nicvf *nic = dev_get_priv(dev);
	struct sq_hdr_subdesc *hdr;

	head = nicvf_queue_reg_read(nic, NIC_QSET_SQ_0_7_HEAD, qidx) >> 4;

	while (sq->head != head) {
		hdr = (struct sq_hdr_subdesc *)GET_SQ_DESC(sq, sq->head);
		if (hdr->subdesc_type != SQ_DESC_TYPE_HEADER) {
			nicvf_put_sq_desc(sq, 1);
			continue;
		}
		nicvf_put_sq_desc(sq, hdr->subdesc_cnt + 1);
	}
}

/* Get the number of SQ descriptors needed to xmit this skb */
static int nicvf_sq_subdesc_required(struct nicvf *nic)
{
	int subdesc_cnt = MIN_SQ_DESC_PER_PKT_XMIT;

	return subdesc_cnt;
}

/* Add SQ HEADER subdescriptor.
 * First subdescriptor for every send descriptor.
 */
static inline void
nicvf_sq_add_hdr_subdesc(struct nicvf *nic, struct snd_queue *sq, int qentry,
			 int subdesc_cnt, void *pkt, size_t pkt_len)
{
	struct sq_hdr_subdesc *hdr;

	hdr = (struct sq_hdr_subdesc *)GET_SQ_DESC(sq, qentry);
	sq->skbuff[qentry] = (uintptr_t)pkt;

	memset(hdr, 0, SND_QUEUE_DESC_SIZE);
	hdr->subdesc_type = SQ_DESC_TYPE_HEADER;
	/* Enable notification via CQE after processing SQE */
	hdr->post_cqe = 1;
	/* No of subdescriptors following this */
	hdr->subdesc_cnt = subdesc_cnt;
	hdr->tot_len = pkt_len;

	flush_dcache_range((uintptr_t)hdr,
			   (uintptr_t)hdr + sizeof(struct sq_hdr_subdesc));
}

/* SQ GATHER subdescriptor
 * Must follow HDR descriptor
 */
static inline void nicvf_sq_add_gather_subdesc(struct snd_queue *sq, int qentry,
					       size_t size, uintptr_t data)
{
	struct sq_gather_subdesc *gather;

	qentry &= (sq->dmem.q_len - 1);
	gather = (struct sq_gather_subdesc *)GET_SQ_DESC(sq, qentry);

	memset(gather, 0, SND_QUEUE_DESC_SIZE);
	gather->subdesc_type = SQ_DESC_TYPE_GATHER;
	gather->ld_type = NIC_SEND_LD_TYPE_E_LDD;
	gather->size = size;
	gather->addr = data;

	flush_dcache_range((uintptr_t)gather,
			   (uintptr_t)gather + sizeof(struct sq_hdr_subdesc));
}

/* Append an skb to a SQ for packet transfer. */
int nicvf_sq_append_pkt(struct nicvf *nic, void *pkt, size_t pkt_size)
{
	int subdesc_cnt;
	int sq_num = 0, qentry;
	struct queue_set *qs;
	struct snd_queue *sq;

	qs = nic->qs;
	sq = &qs->sq[sq_num];

	subdesc_cnt = nicvf_sq_subdesc_required(nic);
	if (subdesc_cnt > sq->free_cnt)
		goto append_fail;

	qentry = nicvf_get_sq_desc(sq, subdesc_cnt);

	/* Add SQ header subdesc */
	nicvf_sq_add_hdr_subdesc(nic, sq, qentry, subdesc_cnt - 1,
				 pkt, pkt_size);

	/* Add SQ gather subdescs */
	qentry = nicvf_get_nxt_sqentry(sq, qentry);
	nicvf_sq_add_gather_subdesc(sq, qentry, pkt_size, (uintptr_t)(pkt));

	flush_dcache_range((uintptr_t)pkt,
			   (uintptr_t)pkt + pkt_size);

	/* make sure all memory stores are done before ringing doorbell */
	asm volatile ("dsb sy");

	/* Inform HW to xmit new packet */
	nicvf_queue_reg_write(nic, NIC_QSET_SQ_0_7_DOOR,
			      sq_num, subdesc_cnt);
	return 1;

append_fail:
	printf("Not enough SQ descriptors to xmit pkt\n");
	return 0;
}

static unsigned int frag_num(unsigned int i)
{
#ifdef __BIG_ENDIAN
	return (i & ~3) + 3 - (i & 3);
#else
	return i;
#endif
}

void *nicvf_get_rcv_pkt(struct nicvf *nic, void *cq_desc, size_t *pkt_len)
{
	int frag;
	int payload_len = 0, tot_len;
	void *pkt = NULL, *pkt_buf = NULL, *buffer;
	struct cqe_rx_t *cqe_rx;
	struct rbdr *rbdr;
	struct rcv_queue *rq;
	struct queue_set *qs = nic->qs;
	u16 *rb_lens = NULL;
	u64 *rb_ptrs = NULL;

	cqe_rx = (struct cqe_rx_t *)cq_desc;

	rq = &qs->rq[cqe_rx->rq_idx];
	rbdr = &qs->rbdr[rq->start_qs_rbdr_idx];
	rb_lens = cq_desc + (3 * sizeof(u64)); /* Use offsetof */
	/* Except 88xx pass1 on all other chips CQE_RX2_S is added to
	 * CQE_RX at word6, hence buffer pointers move by word
	 *
	 * Use existing 'hw_tso' flag which will be set for all chips
	 * except 88xx pass1 instead of a additional cache line
	 * access (or miss) by using pci dev's revision.
	 */
	if (!nic->hw_tso)
		rb_ptrs = (void *)cqe_rx + (6 * sizeof(u64));
	else
		rb_ptrs = (void *)cqe_rx + (7 * sizeof(u64));

	/*
	 * Figure out packet length to create packet buffer
	 */
	for (frag = 0; frag < cqe_rx->rb_cnt; frag++)
		payload_len += rb_lens[frag_num(frag)];
	*pkt_len = payload_len;
	/* round up size to 8 byte multiple */
	tot_len = (payload_len & (~0x7)) + 8;
	buffer = calloc(1, tot_len);
	if (!buffer) {
		printf("%s - Failed to allocate packet buffer\n", __func__);
		return NULL;
	}
	pkt_buf = buffer;
	debug("total pkt buf %p len %ld tot_len %d\n", pkt_buf, *pkt_len,
	      tot_len);
	for (frag = 0; frag < cqe_rx->rb_cnt; frag++) {
		payload_len = rb_lens[frag_num(frag)];

		invalidate_dcache_range((uintptr_t)(*rb_ptrs),
					(uintptr_t)(*rb_ptrs) + rbdr->dma_size);

		/* First fragment */
		*rb_ptrs = *rb_ptrs - cqe_rx->align_pad;

		pkt = nicvf_rb_ptr_to_pkt(nic, *rb_ptrs);

		invalidate_dcache_range((uintptr_t)pkt,
					(uintptr_t)pkt + payload_len);

		if (cqe_rx->align_pad)
			pkt += cqe_rx->align_pad;
		debug("pkt_buf %p, pkt %p payload_len %d\n", pkt_buf, pkt,
		      payload_len);
		memcpy(buffer, pkt, payload_len);
		buffer += payload_len;
		/* Next buffer pointer */
		rb_ptrs++;
	}
	return pkt_buf;
}

/* Clear interrupt */
void nicvf_clear_intr(struct nicvf *nic, int int_type, int q_idx)
{
	u64 reg_val = 0;

	switch (int_type) {
	case NICVF_INTR_CQ:
		reg_val = ((1ULL << q_idx) << NICVF_INTR_CQ_SHIFT);
	break;
	case NICVF_INTR_SQ:
		reg_val = ((1ULL << q_idx) << NICVF_INTR_SQ_SHIFT);
	break;
	case NICVF_INTR_RBDR:
		reg_val = ((1ULL << q_idx) << NICVF_INTR_RBDR_SHIFT);
	break;
	case NICVF_INTR_PKT_DROP:
		reg_val = (1ULL << NICVF_INTR_PKT_DROP_SHIFT);
	break;
	case NICVF_INTR_TCP_TIMER:
		reg_val = (1ULL << NICVF_INTR_TCP_TIMER_SHIFT);
	break;
	case NICVF_INTR_MBOX:
		reg_val = (1ULL << NICVF_INTR_MBOX_SHIFT);
	break;
	case NICVF_INTR_QS_ERR:
		reg_val |= (1ULL << NICVF_INTR_QS_ERR_SHIFT);
	break;
	default:
		printf("Failed to clear interrupt: unknown type\n");
	break;
	}

	nicvf_reg_write(nic, NIC_VF_INT, reg_val);
}

void nicvf_update_rq_stats(struct nicvf *nic, int rq_idx)
{
	struct rcv_queue *rq;

#define GET_RQ_STATS(reg) \
	nicvf_reg_read(nic, NIC_QSET_RQ_0_7_STAT_0_1 |\
			    (rq_idx << NIC_Q_NUM_SHIFT) | ((reg) << 3))

	rq = &nic->qs->rq[rq_idx];
	rq->stats.bytes = GET_RQ_STATS(RQ_SQ_STATS_OCTS);
	rq->stats.pkts = GET_RQ_STATS(RQ_SQ_STATS_PKTS);
}

void nicvf_update_sq_stats(struct nicvf *nic, int sq_idx)
{
	struct snd_queue *sq;

#define GET_SQ_STATS(reg) \
	nicvf_reg_read(nic, NIC_QSET_SQ_0_7_STAT_0_1 |\
			    (sq_idx << NIC_Q_NUM_SHIFT) | ((reg) << 3))

	sq = &nic->qs->sq[sq_idx];
	sq->stats.bytes = GET_SQ_STATS(RQ_SQ_STATS_OCTS);
	sq->stats.pkts = GET_SQ_STATS(RQ_SQ_STATS_PKTS);
}

/* Check for errors in the receive cmp.queue entry */
int nicvf_check_cqe_rx_errs(struct nicvf *nic,
			    struct cmp_queue *cq, void *cq_desc)
{
	struct cqe_rx_t *cqe_rx;
	struct cmp_queue_stats *stats = &cq->stats;

	cqe_rx = (struct cqe_rx_t *)cq_desc;
	if (!cqe_rx->err_level && !cqe_rx->err_opcode) {
		stats->rx.errop.good++;
		return 0;
	}

	switch (cqe_rx->err_level) {
	case CQ_ERRLVL_MAC:
		stats->rx.errlvl.mac_errs++;
	break;
	case CQ_ERRLVL_L2:
		stats->rx.errlvl.l2_errs++;
	break;
	case CQ_ERRLVL_L3:
		stats->rx.errlvl.l3_errs++;
	break;
	case CQ_ERRLVL_L4:
		stats->rx.errlvl.l4_errs++;
	break;
	}

	switch (cqe_rx->err_opcode) {
	case CQ_RX_ERROP_RE_PARTIAL:
		stats->rx.errop.partial_pkts++;
	break;
	case CQ_RX_ERROP_RE_JABBER:
		stats->rx.errop.jabber_errs++;
	break;
	case CQ_RX_ERROP_RE_FCS:
		stats->rx.errop.fcs_errs++;
	break;
	case CQ_RX_ERROP_RE_TERMINATE:
		stats->rx.errop.terminate_errs++;
	break;
	case CQ_RX_ERROP_RE_RX_CTL:
		stats->rx.errop.bgx_rx_errs++;
	break;
	case CQ_RX_ERROP_PREL2_ERR:
		stats->rx.errop.prel2_errs++;
	break;
	case CQ_RX_ERROP_L2_FRAGMENT:
		stats->rx.errop.l2_frags++;
	break;
	case CQ_RX_ERROP_L2_OVERRUN:
		stats->rx.errop.l2_overruns++;
	break;
	case CQ_RX_ERROP_L2_PFCS:
		stats->rx.errop.l2_pfcs++;
	break;
	case CQ_RX_ERROP_L2_PUNY:
		stats->rx.errop.l2_puny++;
	break;
	case CQ_RX_ERROP_L2_MAL:
		stats->rx.errop.l2_hdr_malformed++;
	break;
	case CQ_RX_ERROP_L2_OVERSIZE:
		stats->rx.errop.l2_oversize++;
	break;
	case CQ_RX_ERROP_L2_UNDERSIZE:
		stats->rx.errop.l2_undersize++;
	break;
	case CQ_RX_ERROP_L2_LENMISM:
		stats->rx.errop.l2_len_mismatch++;
	break;
	case CQ_RX_ERROP_L2_PCLP:
		stats->rx.errop.l2_pclp++;
	break;
	case CQ_RX_ERROP_IP_NOT:
		stats->rx.errop.non_ip++;
	break;
	case CQ_RX_ERROP_IP_CSUM_ERR:
		stats->rx.errop.ip_csum_err++;
	break;
	case CQ_RX_ERROP_IP_MAL:
		stats->rx.errop.ip_hdr_malformed++;
	break;
	case CQ_RX_ERROP_IP_MALD:
		stats->rx.errop.ip_payload_malformed++;
	break;
	case CQ_RX_ERROP_IP_HOP:
		stats->rx.errop.ip_hop_errs++;
	break;
	case CQ_RX_ERROP_L3_ICRC:
		stats->rx.errop.l3_icrc_errs++;
	break;
	case CQ_RX_ERROP_L3_PCLP:
		stats->rx.errop.l3_pclp++;
	break;
	case CQ_RX_ERROP_L4_MAL:
		stats->rx.errop.l4_malformed++;
	break;
	case CQ_RX_ERROP_L4_CHK:
		stats->rx.errop.l4_csum_errs++;
	break;
	case CQ_RX_ERROP_UDP_LEN:
		stats->rx.errop.udp_len_err++;
	break;
	case CQ_RX_ERROP_L4_PORT:
		stats->rx.errop.bad_l4_port++;
	break;
	case CQ_RX_ERROP_TCP_FLAG:
		stats->rx.errop.bad_tcp_flag++;
	break;
	case CQ_RX_ERROP_TCP_OFFSET:
		stats->rx.errop.tcp_offset_errs++;
	break;
	case CQ_RX_ERROP_L4_PCLP:
		stats->rx.errop.l4_pclp++;
	break;
	case CQ_RX_ERROP_RBDR_TRUNC:
		stats->rx.errop.pkt_truncated++;
	break;
	}

	return 1;
}

/* Check for errors in the send cmp.queue entry */
int nicvf_check_cqe_tx_errs(struct nicvf *nic,
			    struct cmp_queue *cq, void *cq_desc)
{
	struct cqe_send_t *cqe_tx;
	struct cmp_queue_stats *stats = &cq->stats;

	cqe_tx = (struct cqe_send_t *)cq_desc;
	switch (cqe_tx->send_status) {
	case CQ_TX_ERROP_GOOD:
		stats->tx.good++;
		return 0;
	break;
	case CQ_TX_ERROP_DESC_FAULT:
		stats->tx.desc_fault++;
	break;
	case CQ_TX_ERROP_HDR_CONS_ERR:
		stats->tx.hdr_cons_err++;
	break;
	case CQ_TX_ERROP_SUBDC_ERR:
		stats->tx.subdesc_err++;
	break;
	case CQ_TX_ERROP_IMM_SIZE_OFLOW:
		stats->tx.imm_size_oflow++;
	break;
	case CQ_TX_ERROP_DATA_SEQUENCE_ERR:
		stats->tx.data_seq_err++;
	break;
	case CQ_TX_ERROP_MEM_SEQUENCE_ERR:
		stats->tx.mem_seq_err++;
	break;
	case CQ_TX_ERROP_LOCK_VIOL:
		stats->tx.lock_viol++;
	break;
	case CQ_TX_ERROP_DATA_FAULT:
		stats->tx.data_fault++;
	break;
	case CQ_TX_ERROP_TSTMP_CONFLICT:
		stats->tx.tstmp_conflict++;
	break;
	case CQ_TX_ERROP_TSTMP_TIMEOUT:
		stats->tx.tstmp_timeout++;
	break;
	case CQ_TX_ERROP_MEM_FAULT:
		stats->tx.mem_fault++;
	break;
	case CQ_TX_ERROP_CK_OVERLAP:
		stats->tx.csum_overlap++;
	break;
	case CQ_TX_ERROP_CK_OFLOW:
		stats->tx.csum_overflow++;
	break;
	}

	return 1;
}
