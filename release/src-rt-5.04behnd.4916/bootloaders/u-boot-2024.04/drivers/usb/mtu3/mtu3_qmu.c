// SPDX-License-Identifier: GPL-2.0
/*
 * mtu3_qmu.c - Queue Management Unit driver for device controller
 *
 * Copyright (C) 2016 MediaTek Inc.
 *
 * Author: Chunfeng Yun <chunfeng.yun@mediatek.com>
 */

/*
 * Queue Management Unit (QMU) is designed to unload SW effort
 * to serve DMA interrupts.
 * By preparing General Purpose Descriptor (GPD) and Buffer Descriptor (BD),
 * SW links data buffers and triggers QMU to send / receive data to
 * host / from device at a time.
 * And now only GPD is supported.
 *
 * For more detailed information, please refer to QMU Programming Guide
 */

#include <asm/cache.h>
#include <cpu_func.h>
#include <linux/iopoll.h>
#include <linux/types.h>

#include "mtu3.h"

#define QMU_CHECKSUM_LEN	16

#define GPD_FLAGS_HWO	BIT(0)
#define GPD_FLAGS_BDP	BIT(1)
#define GPD_FLAGS_BPS	BIT(2)
#define GPD_FLAGS_IOC	BIT(7)

#define GPD_EXT_FLAG_ZLP	BIT(5)

#define DCACHELINE_SIZE		CONFIG_SYS_CACHELINE_SIZE

void mtu3_flush_cache(uintptr_t addr, u32 len)
{
	WARN_ON(!(void *)addr || len == 0);

	flush_dcache_range(addr & ~(DCACHELINE_SIZE - 1),
			   ALIGN(addr + len, DCACHELINE_SIZE));
}

void mtu3_inval_cache(uintptr_t addr, u32 len)
{
	WARN_ON(!(void *)addr || len == 0);

	invalidate_dcache_range(addr & ~(DCACHELINE_SIZE - 1),
				ALIGN(addr + len, DCACHELINE_SIZE));
}

static struct qmu_gpd *gpd_dma_to_virt(struct mtu3_gpd_ring *ring,
				       dma_addr_t dma_addr)
{
	dma_addr_t dma_base = ring->dma;
	struct qmu_gpd *gpd_head = ring->start;
	u32 offset = (dma_addr - dma_base) / sizeof(*gpd_head);

	if (offset >= MAX_GPD_NUM)
		return NULL;

	return gpd_head + offset;
}

static dma_addr_t gpd_virt_to_dma(struct mtu3_gpd_ring *ring,
				  struct qmu_gpd *gpd)
{
	dma_addr_t dma_base = ring->dma;
	struct qmu_gpd *gpd_head = ring->start;
	u32 offset;

	offset = gpd - gpd_head;
	if (offset >= MAX_GPD_NUM)
		return 0;

	return dma_base + (offset * sizeof(*gpd));
}

static void gpd_ring_init(struct mtu3_gpd_ring *ring, struct qmu_gpd *gpd)
{
	ring->start = gpd;
	ring->enqueue = gpd;
	ring->dequeue = gpd;
	ring->end = gpd + MAX_GPD_NUM - 1;
}

static void reset_gpd_list(struct mtu3_ep *mep)
{
	struct mtu3_gpd_ring *ring = &mep->gpd_ring;
	struct qmu_gpd *gpd = ring->start;

	if (gpd) {
		gpd->flag &= ~GPD_FLAGS_HWO;
		gpd_ring_init(ring, gpd);
		mtu3_flush_cache((uintptr_t)gpd, sizeof(*gpd));
	}
}

int mtu3_gpd_ring_alloc(struct mtu3_ep *mep)
{
	struct qmu_gpd *gpd;
	struct mtu3_gpd_ring *ring = &mep->gpd_ring;

	/* software own all gpds as default */
	gpd = memalign(DCACHELINE_SIZE, QMU_GPD_RING_SIZE);
	if (!gpd)
		return -ENOMEM;

	memset(gpd, 0, QMU_GPD_RING_SIZE);
	ring->dma = (dma_addr_t)gpd;
	gpd_ring_init(ring, gpd);
	mtu3_flush_cache((uintptr_t)gpd, sizeof(*gpd));

	return 0;
}

void mtu3_gpd_ring_free(struct mtu3_ep *mep)
{
	struct mtu3_gpd_ring *ring = &mep->gpd_ring;

	kfree(ring->start);
	memset(ring, 0, sizeof(*ring));
}

void mtu3_qmu_resume(struct mtu3_ep *mep)
{
	struct mtu3 *mtu = mep->mtu;
	void __iomem *mbase = mtu->mac_base;
	int epnum = mep->epnum;
	u32 offset;

	offset = mep->is_in ? USB_QMU_TQCSR(epnum) : USB_QMU_RQCSR(epnum);

	mtu3_writel(mbase, offset, QMU_Q_RESUME);
	if (!(mtu3_readl(mbase, offset) & QMU_Q_ACTIVE))
		mtu3_writel(mbase, offset, QMU_Q_RESUME);
}

static struct qmu_gpd *advance_enq_gpd(struct mtu3_gpd_ring *ring)
{
	if (ring->enqueue < ring->end)
		ring->enqueue++;
	else
		ring->enqueue = ring->start;

	return ring->enqueue;
}

static struct qmu_gpd *advance_deq_gpd(struct mtu3_gpd_ring *ring)
{
	if (ring->dequeue < ring->end)
		ring->dequeue++;
	else
		ring->dequeue = ring->start;

	return ring->dequeue;
}

/* check if a ring is emtpy */
static int gpd_ring_empty(struct mtu3_gpd_ring *ring)
{
	struct qmu_gpd *enq = ring->enqueue;
	struct qmu_gpd *next;

	if (ring->enqueue < ring->end)
		next = enq + 1;
	else
		next = ring->start;

	/* one gpd is reserved to simplify gpd preparation */
	return next == ring->dequeue;
}

int mtu3_prepare_transfer(struct mtu3_ep *mep)
{
	return gpd_ring_empty(&mep->gpd_ring);
}

static int mtu3_prepare_tx_gpd(struct mtu3_ep *mep, struct mtu3_request *mreq)
{
	struct qmu_gpd *enq;
	struct mtu3_gpd_ring *ring = &mep->gpd_ring;
	struct qmu_gpd *gpd = ring->enqueue;
	struct usb_request *req = &mreq->request;

	/* set all fields to zero as default value */
	memset(gpd, 0, sizeof(*gpd));

	gpd->buffer = cpu_to_le32((u32)req->dma);
	gpd->buf_len = cpu_to_le16(req->length);

	/* get the next GPD */
	enq = advance_enq_gpd(ring);
	dev_dbg(mep->mtu->dev, "TX-EP%d queue gpd=%p, enq=%p\n",
		mep->epnum, gpd, enq);

	enq->flag &= ~GPD_FLAGS_HWO;
	gpd->next_gpd = cpu_to_le32((u32)gpd_virt_to_dma(ring, enq));
	mtu3_flush_cache((uintptr_t)enq, sizeof(*gpd));

	if (req->zero)
		gpd->ext_flag |= GPD_EXT_FLAG_ZLP;

	gpd->flag |= GPD_FLAGS_IOC | GPD_FLAGS_HWO;

	mreq->gpd = gpd;

	if (req->length)
		mtu3_flush_cache((uintptr_t)req->buf, req->length);

	mtu3_flush_cache((uintptr_t)gpd, sizeof(*gpd));

	return 0;
}

static int mtu3_prepare_rx_gpd(struct mtu3_ep *mep, struct mtu3_request *mreq)
{
	struct qmu_gpd *enq;
	struct mtu3_gpd_ring *ring = &mep->gpd_ring;
	struct qmu_gpd *gpd = ring->enqueue;
	struct usb_request *req = &mreq->request;

	/* set all fields to zero as default value */
	memset(gpd, 0, sizeof(*gpd));

	gpd->buffer = cpu_to_le32((u32)req->dma);
	gpd->data_buf_len = cpu_to_le16(req->length);

	/* get the next GPD */
	enq = advance_enq_gpd(ring);
	dev_dbg(mep->mtu->dev, "RX-EP%d queue gpd=%p, enq=%p\n",
		mep->epnum, gpd, enq);

	enq->flag &= ~GPD_FLAGS_HWO;
	gpd->next_gpd = cpu_to_le32((u32)gpd_virt_to_dma(ring, enq));
	mtu3_flush_cache((uintptr_t)enq, sizeof(*gpd));

	gpd->flag |= GPD_FLAGS_IOC | GPD_FLAGS_HWO;

	mreq->gpd = gpd;

	mtu3_inval_cache((uintptr_t)req->buf, req->length);
	mtu3_flush_cache((uintptr_t)gpd, sizeof(*gpd));

	return 0;
}

void mtu3_insert_gpd(struct mtu3_ep *mep, struct mtu3_request *mreq)
{
	if (mep->is_in)
		mtu3_prepare_tx_gpd(mep, mreq);
	else
		mtu3_prepare_rx_gpd(mep, mreq);
}

int mtu3_qmu_start(struct mtu3_ep *mep)
{
	struct mtu3 *mtu = mep->mtu;
	void __iomem *mbase = mtu->mac_base;
	struct mtu3_gpd_ring *ring = &mep->gpd_ring;
	u8 epnum = mep->epnum;

	if (mep->is_in) {
		/* set QMU start address */
		mtu3_writel(mbase, USB_QMU_TQSAR(epnum), ring->dma);
		mtu3_setbits(mbase, MU3D_EP_TXCR0(epnum), TX_DMAREQEN);
		/* send zero length packet according to ZLP flag in GPD */
		mtu3_setbits(mbase, U3D_QCR1, QMU_TX_ZLP(epnum));
		mtu3_writel(mbase, U3D_TQERRIESR0,
			    QMU_TX_LEN_ERR(epnum) | QMU_TX_CS_ERR(epnum));

		if (mtu3_readl(mbase, USB_QMU_TQCSR(epnum)) & QMU_Q_ACTIVE) {
			dev_warn(mtu->dev, "Tx %d Active Now!\n", epnum);
			return 0;
		}
		mtu3_writel(mbase, USB_QMU_TQCSR(epnum), QMU_Q_START);

	} else {
		mtu3_writel(mbase, USB_QMU_RQSAR(epnum), ring->dma);
		mtu3_setbits(mbase, MU3D_EP_RXCR0(epnum), RX_DMAREQEN);
		/* don't expect ZLP */
		mtu3_clrbits(mbase, U3D_QCR3, QMU_RX_ZLP(epnum));
		/* move to next GPD when receive ZLP */
		mtu3_setbits(mbase, U3D_QCR3, QMU_RX_COZ(epnum));
		mtu3_writel(mbase, U3D_RQERRIESR0,
			    QMU_RX_LEN_ERR(epnum) | QMU_RX_CS_ERR(epnum));
		mtu3_writel(mbase, U3D_RQERRIESR1, QMU_RX_ZLP_ERR(epnum));

		if (mtu3_readl(mbase, USB_QMU_RQCSR(epnum)) & QMU_Q_ACTIVE) {
			dev_warn(mtu->dev, "Rx %d Active Now!\n", epnum);
			return 0;
		}
		mtu3_writel(mbase, USB_QMU_RQCSR(epnum), QMU_Q_START);
	}

	return 0;
}

/* may called in atomic context */
void mtu3_qmu_stop(struct mtu3_ep *mep)
{
	struct mtu3 *mtu = mep->mtu;
	void __iomem *mbase = mtu->mac_base;
	int epnum = mep->epnum;
	u32 value = 0;
	u32 qcsr;
	int ret;

	qcsr = mep->is_in ? USB_QMU_TQCSR(epnum) : USB_QMU_RQCSR(epnum);

	if (!(mtu3_readl(mbase, qcsr) & QMU_Q_ACTIVE)) {
		dev_dbg(mtu->dev, "%s's qmu is inactive now!\n", mep->name);
		return;
	}
	mtu3_writel(mbase, qcsr, QMU_Q_STOP);

	ret = readl_poll_timeout(mbase + qcsr, value,
				 !(value & QMU_Q_ACTIVE), 1000);
	if (ret) {
		dev_err(mtu->dev, "stop %s's qmu failed\n", mep->name);
		return;
	}

	dev_dbg(mtu->dev, "%s's qmu stop now!\n", mep->name);
}

void mtu3_qmu_flush(struct mtu3_ep *mep)
{
	dev_dbg(mep->mtu->dev, "%s flush QMU %s\n", __func__,
		((mep->is_in) ? "TX" : "RX"));

	/*Stop QMU */
	mtu3_qmu_stop(mep);
	reset_gpd_list(mep);
}

/*
 * NOTE: request list maybe is already empty as following case:
 * queue_tx --> qmu_interrupt(clear interrupt pending, schedule tasklet)-->
 * queue_tx --> process_tasklet(meanwhile, the second one is transferred,
 * tasklet process both of them)-->qmu_interrupt for second one.
 * To avoid upper case, put qmu_done_tx in ISR directly to process it.
 */
static void qmu_done_tx(struct mtu3 *mtu, u8 epnum)
{
	struct mtu3_ep *mep = mtu->in_eps + epnum;
	struct mtu3_gpd_ring *ring = &mep->gpd_ring;
	void __iomem *mbase = mtu->mac_base;
	struct qmu_gpd *gpd = ring->dequeue;
	struct qmu_gpd *gpd_current = NULL;
	struct usb_request *req = NULL;
	struct mtu3_request *mreq;
	dma_addr_t cur_gpd_dma;

	/*transfer phy address got from QMU register to virtual address */
	cur_gpd_dma = mtu3_readl(mbase, USB_QMU_TQCPR(epnum));
	gpd_current = gpd_dma_to_virt(ring, cur_gpd_dma);
	mtu3_inval_cache((uintptr_t)gpd, sizeof(*gpd));

	dev_dbg(mtu->dev, "%s EP%d, last=%p, current=%p, enq=%p\n",
		__func__, epnum, gpd, gpd_current, ring->enqueue);

	while (gpd != gpd_current && !(gpd->flag & GPD_FLAGS_HWO)) {
		mreq = next_request(mep);

		if (!mreq || mreq->gpd != gpd) {
			dev_err(mtu->dev, "no correct TX req is found\n");
			break;
		}

		req = &mreq->request;
		req->actual = le16_to_cpu(gpd->buf_len);
		mtu3_req_complete(mep, req, 0);

		gpd = advance_deq_gpd(ring);
		mtu3_inval_cache((uintptr_t)gpd, sizeof(*gpd));
	}

	dev_dbg(mtu->dev, "%s EP%d, deq=%p, enq=%p, complete\n",
		__func__, epnum, ring->dequeue, ring->enqueue);
}

static void qmu_done_rx(struct mtu3 *mtu, u8 epnum)
{
	struct mtu3_ep *mep = mtu->out_eps + epnum;
	struct mtu3_gpd_ring *ring = &mep->gpd_ring;
	void __iomem *mbase = mtu->mac_base;
	struct qmu_gpd *gpd = ring->dequeue;
	struct qmu_gpd *gpd_current = NULL;
	struct usb_request *req = NULL;
	struct mtu3_request *mreq;
	dma_addr_t cur_gpd_dma;

	cur_gpd_dma = mtu3_readl(mbase, USB_QMU_RQCPR(epnum));
	gpd_current = gpd_dma_to_virt(ring, cur_gpd_dma);
	mtu3_inval_cache((uintptr_t)gpd, sizeof(*gpd));

	dev_dbg(mtu->dev, "%s EP%d, last=%p, current=%p, enq=%p\n",
		__func__, epnum, gpd, gpd_current, ring->enqueue);

	while (gpd != gpd_current && !(gpd->flag & GPD_FLAGS_HWO)) {
		mreq = next_request(mep);

		if (!mreq || mreq->gpd != gpd) {
			dev_err(mtu->dev, "no correct RX req is found\n");
			break;
		}
		req = &mreq->request;

		req->actual = le16_to_cpu(gpd->buf_len);
		mtu3_req_complete(mep, req, 0);

		gpd = advance_deq_gpd(ring);
		mtu3_inval_cache((uintptr_t)gpd, sizeof(*gpd));
	}

	dev_dbg(mtu->dev, "%s EP%d, deq=%p, enq=%p, complete\n",
		__func__, epnum, ring->dequeue, ring->enqueue);
}

static void qmu_done_isr(struct mtu3 *mtu, u32 done_status)
{
	int i;

	for (i = 1; i < mtu->num_eps; i++) {
		if (done_status & QMU_RX_DONE_INT(i))
			qmu_done_rx(mtu, i);
		if (done_status & QMU_TX_DONE_INT(i))
			qmu_done_tx(mtu, i);
	}
}

static void qmu_exception_isr(struct mtu3 *mtu, u32 qmu_status)
{
	void __iomem *mbase = mtu->mac_base;
	u32 errval;
	int i;

	if ((qmu_status & RXQ_CSERR_INT) || (qmu_status & RXQ_LENERR_INT)) {
		errval = mtu3_readl(mbase, U3D_RQERRIR0);
		for (i = 1; i < mtu->num_eps; i++) {
			if (errval & QMU_RX_CS_ERR(i))
				dev_err(mtu->dev, "Rx %d CS error!\n", i);

			if (errval & QMU_RX_LEN_ERR(i))
				dev_err(mtu->dev, "RX %d Length error\n", i);
		}
		mtu3_writel(mbase, U3D_RQERRIR0, errval);
	}

	if (qmu_status & RXQ_ZLPERR_INT) {
		errval = mtu3_readl(mbase, U3D_RQERRIR1);
		for (i = 1; i < mtu->num_eps; i++) {
			if (errval & QMU_RX_ZLP_ERR(i))
				dev_dbg(mtu->dev, "RX EP%d Recv ZLP\n", i);
		}
		mtu3_writel(mbase, U3D_RQERRIR1, errval);
	}

	if ((qmu_status & TXQ_CSERR_INT) || (qmu_status & TXQ_LENERR_INT)) {
		errval = mtu3_readl(mbase, U3D_TQERRIR0);
		for (i = 1; i < mtu->num_eps; i++) {
			if (errval & QMU_TX_CS_ERR(i))
				dev_err(mtu->dev, "Tx %d checksum error!\n", i);

			if (errval & QMU_TX_LEN_ERR(i))
				dev_err(mtu->dev, "Tx %d zlp error!\n", i);
		}
		mtu3_writel(mbase, U3D_TQERRIR0, errval);
	}
}

irqreturn_t mtu3_qmu_isr(struct mtu3 *mtu)
{
	void __iomem *mbase = mtu->mac_base;
	u32 qmu_status;
	u32 qmu_done_status;

	/* U3D_QISAR1 is read update */
	qmu_status = mtu3_readl(mbase, U3D_QISAR1);
	qmu_status &= mtu3_readl(mbase, U3D_QIER1);

	qmu_done_status = mtu3_readl(mbase, U3D_QISAR0);
	qmu_done_status &= mtu3_readl(mbase, U3D_QIER0);
	mtu3_writel(mbase, U3D_QISAR0, qmu_done_status); /* W1C */
	dev_dbg(mtu->dev, "=== QMUdone[tx=%x, rx=%x] QMUexp[%x] ===\n",
		(qmu_done_status & 0xFFFF), qmu_done_status >> 16,
		qmu_status);

	if (qmu_done_status)
		qmu_done_isr(mtu, qmu_done_status);

	if (qmu_status)
		qmu_exception_isr(mtu, qmu_status);

	return IRQ_HANDLED;
}

void mtu3_qmu_init(struct mtu3 *mtu)
{
	compiletime_assert(QMU_GPD_SIZE == 16, "QMU_GPD size SHOULD be 16B");
}

void mtu3_qmu_exit(struct mtu3 *mtu)
{
}
