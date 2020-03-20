/*
<:copyright-BRCM:2015:GPL/GPL:spu

   Copyright (c) 2015 Broadcom
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

//#define DEBUG

#include <generated/autoconf.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/scatterlist.h>
#include <crypto/scatterwalk.h>
#include <linux/dmapool.h>
#include <linux/kthread.h>
#include <linux/bcm_realtime.h>

#include "pdc.h"
#include "pdc_debug.h"
#include "pmc_drv.h"
#include "pmc_spu.h"

/* Interrupt mask and status definitions. */
#define PDC_LAZY_FRAMECOUNT  1
#define PDC_LAZY_TIMEOUT     0
#define PDC_LAZY_INT  (PDC_LAZY_TIMEOUT | (PDC_LAZY_FRAMECOUNT << 24))
#define PDC_INTMASK_OFFSET   0x24
#define PDC_INTSTATUS_OFFSET 0x20
#define PDC_RCVLAZY0_OFFSET  0x30

/* Sets
 *  0    - XmtEn - enable activity on the tx channel
 * 11    - PtyChkDisable - parity check is disabled
 * 20:18 - BurstLen = 4 -> 2^8 = 256 byte data reads from memory
 */
#define GMAC_TX_CTL              0x00100801

/* 0     - RcvEn - enable activity on the rx channel
 * 7:1   - RcvOffset - size in bytes of status region at start of rx frame buf
 * 9     - SepRxHdrDescEn - place start of new frames only in descriptors
 *                          that have StartOfFrame set
 * 10    - OflowContinue - on rx FIFO overflow, clear rx fifo, discard all
 *                         remaining bytes in current frame, report error
 *                         in rx frame status for current frame
 * 11    - PtyChkDisable - parity check is disabled
 * 20:18 - BurstLen = 2 -> 2^6 = 64 byte data reads from memory
 */
#define GMAC_RX_CTL              (0x00080E01 + (CRYPTO_RX_DMA_OFFSET << 1))

#define CRYPTO_D64_RS0_CD_MASK   ((PDC_RING_ENTRIES * RING_ENTRY_SIZE) - 1)

/* descriptor flags */
#define D64_CTRL1_EOT   ((u32) 1 << 28)    /* end of descriptor table */
#define D64_CTRL1_IOC   ((u32) 1 << 29)    /* interrupt on complete */
#define D64_CTRL1_EOF   ((u32) 1 << 30)    /* end of frame */
#define D64_CTRL1_SOF   ((u32) 1 << 31)    /* start of frame */

#define RX_STATUS_OVERFLOW       0x00800000
#define RX_STATUS_LEN            0x0000FFFF

#define PDC_TXREGS_OFFSET  0x200
#define PDC_RXREGS_OFFSET  0x220

/* Maximum size buffer the DMA engine can handle */
#define PDC_DMA_BUF_MAX 16384

/* Global variables */
char BCMHEADER[] = {0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28};

struct pdc_global *pdc_glob = NULL;

/* Private functions */
static int pdc_remove(struct platform_device *pdev);
static int pdc_send_queued_data(struct pdc_state * pdcs);

/*
 * Build DMA descriptor to receive SPU result.
 *
 * parms
 *   pdcs        - PDC state for SPU that will generate result
 *   dma_addr    - DMA address of buffer that descriptor is being built for
 *   buf_len     - Length of the receive buffer, in bytes
 *   flags       - flags to be stored in descriptor
 */
static inline void
pdc_build_rxd(struct pdc_state *pdcs, dma_addr_t dma_addr,
	      u32 buf_len, u32 flags)
{
	struct device *dev = &pdcs->pdev->dev;

	dev_dbg(dev,
		"Writing rx descriptor for ch %u at index %u with length %04u. flags %#x, %pad\n",
		pdcs->channel, pdcs->rxout, buf_len, flags, &dma_addr);

	/*
	 * these words are static and set at initialization:
	 * WRITE_DESCRIPTOR(&state->rxd_64[pdc_state->rxout].addrhigh, 0)
	 */
	pdcs->rxd_64[pdcs->rxout].addrlow = dma_addr;
	pdcs->rxd_64[pdcs->rxout].ctrl1 = flags;
	pdcs->rxd_64[pdcs->rxout].ctrl2 = buf_len;

	/* bump ring index and return */
	pdcs->rxout = NEXTD(pdcs->rxout);
}

/*
 * Build a DMA descriptor to transmit a SPU request to hardware.
 *
 * parms
 *   pdcs      - PDC state for the SPU that will process this request
 *   dma_addr  - DMA address of packet to be transmitted
 *   msg_len   - length of message in tx buffer
 *   flags     - flags to be stored in descriptor
 */
static inline void
pdc_build_txd(struct pdc_state *pdcs, dma_addr_t dma_addr, u32 msg_len,
	      u32 flags)
{
	struct device *dev = &pdcs->pdev->dev;

	dev_dbg(dev,
		"Writing tx descriptor for ch %u at index %u with length %04u, flags %#x, %pad\n",
		pdcs->channel, pdcs->txout, msg_len, flags, &dma_addr);

	/*
	 * this word is static and set at initialization:
	 * WRITE_DESCRIPTOR(&state->txd_64[pdc_state->txout].addrhigh, 0)
	 */
	pdcs->txd_64[pdcs->txout].addrlow = dma_addr;
	pdcs->txd_64[pdcs->txout].ctrl1 = flags;
	pdcs->txd_64[pdcs->txout].ctrl2 = msg_len;

	/* bump ring index and return */
	pdcs->txout = NEXTD(pdcs->txout);
}

/* Poll a given ch to see if a response message is ready. When the return
 * code indicates success, the response message is available in the
 * receive buffers provided prior to submission of the request.
 *
 * Input:
 *   pdcs - PDC state structure for the SPU to be polled
 *   mssg - message to be returned to client
 *
 * Returns:
 *   A positive return value indicates success and is the length in bytes
 *   of the result. A return value of -EAGAIN indicates that no response message
 *   is available. Any other negative return value indicates an error.
 */
static struct brcm_message *
pdc_crypto_engine_poll(struct pdc_state *pdcs)
{
	u32 rx_status;
	int descr_ready;
	struct brcm_message *msg = NULL;

	spin_lock(&pdcs->pdc_lock);

	descr_ready = DESCRCOUNT(pdcs->last_rx_curr, pdcs->rxin);
	if ((0 == descr_ready) || (descr_ready < pdcs->numd[pdcs->rxin].rx))
	{
		pdcs->last_rx_curr = (reg_read32((void *)&pdcs->rxregs_64->status0) &
		                     CRYPTO_D64_RS0_CD_MASK) / RING_ENTRY_SIZE;
		descr_ready = DESCRCOUNT(pdcs->last_rx_curr, pdcs->rxin);
	}

	if (descr_ready && (descr_ready >= pdcs->numd[pdcs->rxin].rx))
	{
		rx_status = *((u32 *)(pdcs->resp_header + (pdcs->rxin * PDC_RESP_HDR_LEN)));
		msg = pdcs->rxp_ctx[pdcs->rxin];
		dev_dbg(&pdcs->pdev->dev, "ch %u reclaimed %d rx descriptors, %d tx descriptors",
		        pdcs->channel, pdcs->numd[pdcs->rxin].rx, pdcs->numd[pdcs->rxin].tx);
		pdcs->txin = INCRD(pdcs->txin, pdcs->numd[pdcs->rxin].tx);
		pdcs->rxin = INCRD(pdcs->rxin, pdcs->numd[pdcs->rxin].rx);

		/* check length of response msg and rx overflow status.
		   resp_hdr is 32 bit aligned */
		if (unlikely(rx_status & RX_STATUS_OVERFLOW))
		{
			dev_err_ratelimited(&pdcs->pdev->dev, "crypto receive overflow");
			pdcs->rx_oflow++;
			msg->error = -EIO;
		}
		else if (unlikely(0 == (rx_status & RX_STATUS_LEN)))
		{
			dev_info_ratelimited(&pdcs->pdev->dev, "crypto rx len = 0");
			msg->error = -EIO;
		}
		else
		{
			msg->error = 0;
			pdcs->replies++;
		}
		pdc_send_queued_data(pdcs);
	}

	spin_unlock(&pdcs->pdc_lock);
	return msg;
}

int pdc_descr_check(struct pdc_state *pdcs, struct scatterlist *rsg, struct scatterlist *tsg)
{
	u32 avail;
	u32 cnt = 1;   /* Adding a single rx buffer */
	u32 bufcnt;
	dma_addr_t databufptr;  /* DMA address to put in descriptor */

	cnt = 1; /* always an rx buffer for DMA and BCM headers */
	while (rsg) {
		/* PDC DMA cannot accept a buffer larger than 16384 bytes. */
		bufcnt = sg_dma_len(rsg);
		databufptr = sg_dma_address(rsg);
		while (bufcnt > PDC_DMA_BUF_MAX) {
			bufcnt -= PDC_DMA_BUF_MAX;
			databufptr += PDC_DMA_BUF_MAX;
			cnt++;
		}
		if ( bufcnt ) {
			cnt++;
		}
		rsg = sg_next(rsg);
	}

	avail = DESCRSPACE(pdcs->rxout, pdcs->rxin);
	if (unlikely(cnt > avail)) {
		pdcs->rxnobuf++;
		return -ENOSPC;
	}

	cnt = 1; /* always one TX buffer for BCM header */
	while (tsg) {
		/* PDC DMA cannot accept a buffer larger than 16384 bytes. */
		bufcnt = sg_dma_len(tsg);
		databufptr = sg_dma_address(tsg);
		while (bufcnt > PDC_DMA_BUF_MAX) {
			bufcnt -= PDC_DMA_BUF_MAX;
			databufptr += PDC_DMA_BUF_MAX;
			cnt++;
		}
		if ( bufcnt ) {
			cnt++;
		}
		tsg = sg_next(tsg);
	}
	avail = DESCRSPACE(pdcs->txout, pdcs->txin);
	if (unlikely(cnt > avail)) {
		pdcs->txnobuf++;
		return -ENOSPC;
	}
	return PDC_SUCCESS;
}

/*
 * Start a new tx list. Posts a single tx descriptor
 * for the 8-byte BCM header.
 * Moves the msg_start descriptor index to indicate
 * the start of a new message.
 *
 * Returns
 *   PDC_SUCCESS if successful
 *   < 0 if an error
 */
static void
pdc_tx_list_init(struct pdc_state *pdcs)
{
	u32 flags = 0;

	/* build tx descriptor */
	/* This is always the first descriptor in the transmit sequence */
	flags = D64_CTRL1_SOF;
	if (unlikely(0 == NEXTD(pdcs->txout)))
		flags |= D64_CTRL1_EOT;

	/* First txd is for the fixed BCM header. */
	pdc_build_txd(pdcs, pdcs->bcm_header_dma, BCM_HDR_LEN, flags);
	return;
}

/*
 * Add the buffers in a scatterlist to the transmit descriptors for a given ch.
 *
 * Inputs:
 *   pdcs  - dma channel structure
 *   sg    - transmit scatter list
 *
 * Returns:
 *   PDC_SUCCESS if successful
 *   < 0 otherwise
 */
static int
pdc_tx_list_sg_add(struct pdc_state *pdcs, struct scatterlist *sg)
{
	u32  flags = 0;
	u32  desc_w = 0;
	u32  bufcnt;
	dma_addr_t databufptr;

	while (sg) {
		if (unlikely(0 == NEXTD(pdcs->txout)))
			flags = D64_CTRL1_EOT;
		else
			flags = 0;

		/* PDC DMA cannot accept a buffer larger than 16384 bytes. */
		bufcnt = sg_dma_len(sg);
		databufptr = sg_dma_address(sg);
		while (bufcnt > PDC_DMA_BUF_MAX) {
			pdc_build_txd(pdcs, databufptr, PDC_DMA_BUF_MAX, flags);
			desc_w++;
			bufcnt -= PDC_DMA_BUF_MAX;
			databufptr += PDC_DMA_BUF_MAX;
			if (unlikely(0 == NEXTD(pdcs->txout)))
				flags = D64_CTRL1_EOT;
			else
				flags = 0;
		}
		sg = sg_next(sg);
		if (sg == NULL)
			/* Writing last descriptor for frame */
			flags |= (D64_CTRL1_EOF);
		if ( bufcnt ) {
			pdc_build_txd(pdcs, databufptr, bufcnt, flags);
			desc_w++;
		}
	}
	return desc_w;
}

/* Initiate DMA transfer */
static void pdc_initiate_transfer(struct pdc_state *pdcs)
{
	wmb();
	reg_write32(pdcs->txout << 4, (void *)&pdcs->txregs_64->ptr);
	reg_write32(pdcs->rxout << 4, (void *)&pdcs->rxregs_64->ptr);
	pdcs->requests++;

	return;
}

/*
 * Start a new receive list for a given SPU. Posts a single receive
 * descriptor to hold the 32-byte DMA header and the 8-byte BCM header.
 * Moves the msg_start descriptor indexes for both tx and rx to indicate
 * the start of a new message.
 *
 * Returns
 *   PDC_SUCCESS if successful
 *   < 0 if an error
 */
static int
pdc_rx_list_init(struct pdc_state *pdcs, void *ctx)
{
	u32 flags = 0;
	u32 startd = pdcs->rxout;

	/* build rx descriptors */
	/* This is always the first descriptor in the receive sequence */
	flags = D64_CTRL1_SOF;
	if (unlikely(0 == NEXTD(startd)))
		flags |= D64_CTRL1_EOT;

	/* First rxd is for the fixed DMA header plus BCM header. The DMA
	 * mapping for this buffer is permanent. No need to save any
	 * information to facilitate unmapping.
	 */
	pdcs->rxp_ctx[startd] = ctx;
	pdc_build_rxd(pdcs, pdcs->resp_header_dma + (startd * PDC_RESP_HDR_LEN), PDC_RESP_HDR_LEN, flags);
	return startd;
}

/*
 * Add the buffers in a scatterlist to the receive descriptors for a given ch.
 *
 * Inputs:
 *   pdcs  - dma channel structure
 *   sg    - transmit scatter list
 *
 * Returns:
 *   PDC_SUCCESS if successful
 *   < 0 otherwise
 */
static int
pdc_rx_list_sg_add(struct pdc_state *pdcs, struct scatterlist *sg)
{
	u32  flags = 0;
	u32  desc_w = 0;  /* Number of rx descriptors written */
	u32  bufcnt;   /* Number of bytes of buffer pointed to by descriptor */
	dma_addr_t databufptr;  /* DMA address to put in descriptor */

	while (sg) {
		if (unlikely(0 == NEXTD(pdcs->rxout)))
			flags = D64_CTRL1_EOT;
		else
			flags = 0;

		/* DMA cannot accept a buffer larger than 16384 bytes. For
		 * larger buffers, write multiple descriptors.
		 */
		bufcnt = sg_dma_len(sg);
		databufptr = sg_dma_address(sg);
		while (bufcnt > PDC_DMA_BUF_MAX) {
			pdc_build_rxd(pdcs, databufptr, PDC_DMA_BUF_MAX, flags);
			desc_w++;
			bufcnt -= PDC_DMA_BUF_MAX;
			databufptr += PDC_DMA_BUF_MAX;
			if (unlikely(0 == NEXTD(pdcs->rxout)))
				flags = D64_CTRL1_EOT;
			else
				flags = 0;
		}
		if ( bufcnt )
		{
			pdc_build_rxd(pdcs, databufptr, bufcnt, flags);
			desc_w++;
		}
		sg = sg_next(sg);
	}
	return desc_w;
}

/* Interrupt handler called in interrupt context. Have to clear the device
 * interrupt status flags here. So cache the status for later use in the
 * thread function. Other than that, just return WAKE_THREAD to invoke the
 * thread function.
 */
static irqreturn_t pdc_irq_handler(int irq, void *cookie)
{
	struct platform_device *pdev = cookie;
	struct pdc_global *pdcg = (struct pdc_global *)platform_get_drvdata(pdev);
	u32 intstatus;

	/* clear and disable interrupts and schedule work thread */
	reg_write32(0, pdcg->pdc_reg_vbase + PDC_INTMASK_OFFSET);
	intstatus = reg_read32(pdcg->pdc_reg_vbase + PDC_INTSTATUS_OFFSET);
	reg_write32(intstatus, pdcg->pdc_reg_vbase + PDC_INTSTATUS_OFFSET);

	pdcg->work_avail = 1;
	wake_up_interruptible(&pdcg->wait_queue);
	return IRQ_HANDLED;
}

static int pdc_work_thread(void *cookie)
{
	struct platform_device *pdev = cookie;
	struct pdc_global *pdcg = (struct pdc_global *)platform_get_drvdata(pdev);
	struct brcm_message *msg;
	int ch, msg_rcvd;

	if ( !pdcg )
	{
		printk("pdc_work_thread thread has exited due to error\n");
		return -EINVAL;
	}

	while ( 1 ) {
		int count;

		wait_event_interruptible(pdcg->wait_queue, pdcg->work_avail || kthread_should_stop());
		if (kthread_should_stop())
		{
			dev_dbg(&pdev->dev, "kthread_should_stop detected");
			break;
		}

		count = 0;
		/* process up to a budgets worth of frames */
		while (count < PDC_RX_BUDGET)
		{
			msg_rcvd = 0;
			for ( ch=0; ch < pdcg->num_chan; ch++ )
			{
				/* check for received frames and reclaim descriptors */
				msg = pdc_crypto_engine_poll(&pdcg->pdc_state[ch]);
				if (msg)
				{
					dev_dbg(&pdev->dev, "%s(): ch %d invoking client rx cb", __func__, ch);
					msg->rx_callback(msg);
					msg_rcvd++;
				}
				else
				{
					dev_dbg(&pdev->dev, "%s(): ch %d no SPU response available", __func__, ch);
				}
			}
			// exit the while loop if there isn't any message on any channels
			if (!msg_rcvd)
				break;
			count = count + msg_rcvd;
		}

		/* budget was consumed */
		if ( PDC_RX_BUDGET == count )
		{
			/* Yield CPU to allow others to have a chance, then continue to
			   top of loop for more work.  */
			if ((current->policy == SCHED_FIFO) || (current->policy == SCHED_RR))
			{
				yield();
			}
		}
		else
		{
			int intstatus = reg_read32(pdcg->pdc_reg_vbase + PDC_INTSTATUS_OFFSET);
			reg_write32(intstatus, pdcg->pdc_reg_vbase + PDC_INTSTATUS_OFFSET);
			if ( 0 == (intstatus & pdcg->int_mask) )
			{
				/* finished work, enable interrupts and return */
				pdcg->work_avail = 0;
				reg_write32(pdcg->int_mask, pdcg->pdc_reg_vbase + PDC_INTMASK_OFFSET);
			}
		}
	}
	return 0;
}

static int pdc_send_queued_data(struct pdc_state *pdcs)
{
	unsigned int startd;
	struct brcm_message *msg;

	if ( pdcs->msg_count )
	{
		msg = list_first_entry(&pdcs->msg_list, struct brcm_message, list);

		/* verify there are enough descriptors */
		if ( PDC_SUCCESS == pdc_descr_check(pdcs, msg->dst, msg->src) )
		{
			pdcs->msg_count--;
			list_del(&msg->list);

			/* Create rx descriptors to catch SPU response */
			startd = pdc_rx_list_init(pdcs, msg);
			/* always one descriptor */
			pdcs->numd[startd].rx = 1 + pdc_rx_list_sg_add(pdcs, msg->dst);

			/* Create tx descriptors to submit SPU request */
			pdc_tx_list_init(pdcs);
			/* always one descriptor */
			pdcs->numd[startd].tx = 1 + pdc_tx_list_sg_add(pdcs, msg->src);

			pdc_initiate_transfer(pdcs);
		}
	}

	return pdcs->msg_count;
}

/*
 * Select a PDC channel to handle a DMA request. Selects channel in round
 * robin order every two calls (e.g. first two calls to this function will
 * return channel 0, the next two calls will return 1, etc.) The reason
 * for incrementing the channel every 2 calls is that by nature it takes
 * two stages to negotiate a single IPSec flow, one for authentication
 * and one for encryption, but since our chip can do hash and encryption
 * in combo mode, only one DMA channel would be used. If we were to assign
 * a DMA channel to each stage, only the DMA channel from the first call
 * will end up getting used, wasting the other channel. In the case where
 * we can have at most four channels, if we were to increment the index on
 * every call, only channel 0 and 2 will ever be used. Incrementing the
 * channel index every two calls will solve this problem.
 *
 * Returns:
 *   channel index
 */
int pdc_select_channel(void)
{
	u8 chan_idx = (atomic_inc_return(&pdc_glob->next_chan) - 1) / 2;
	return (chan_idx % pdc_glob->num_chan);
}
EXPORT_SYMBOL(pdc_select_channel);

int pdc_send_data(int chan, struct brcm_message *msg)
{
	struct pdc_state *pdcs;

	if ( chan > (pdc_glob->num_chan - 1) || !msg)
	{
		return -ENODEV;
	}
	pdcs = &pdc_glob->pdc_state[chan];

	spin_lock(&pdcs->pdc_lock);
	if ( pdcs->msg_count > 64 )
	{
		dev_dbg(&pdcs->pdev->dev,
		        "%s unable to queue request, %d", __func__, pdcs->msg_count);
		spin_unlock(&pdcs->pdc_lock);
		return -EBUSY;
	}
	list_add_tail(&msg->list, &pdcs->msg_list);
	pdcs->msg_count++;
	pdc_send_queued_data(pdcs);
	spin_unlock(&pdcs->pdc_lock);

	return PDC_SUCCESS;
}
EXPORT_SYMBOL(pdc_send_data);

static int pdc_interrupts_init(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct pdc_global *pdcg = (struct pdc_global *)platform_get_drvdata(pdev);
	struct device_node *dn = pdev->dev.of_node;
	int err, ch;

	/* interrupt configuration */
	for (ch = 0; ch < pdcg->num_chan; ch++)
	{
		reg_write32(PDC_LAZY_INT, pdcg->pdc_reg_vbase + PDC_RCVLAZY0_OFFSET + (ch * 4));
		pdcg->int_mask |= (1 << (ch + 16));
	}
	reg_write32(pdcg->int_mask, pdcg->pdc_reg_vbase + PDC_INTMASK_OFFSET);

	/* read irq from device tree */
	pdcg->pdc_irq = irq_of_parse_and_map(dn, 0);
	dev_dbg(dev, "pdc device %s irq %u", dev_name(dev), pdcg->pdc_irq);

	init_waitqueue_head(&pdcg->wait_queue);
	pdcg->work_thread = kthread_create(pdc_work_thread, pdev, "pdc_rx");
	pdcg->work_avail = 0;
	wake_up_process(pdcg->work_thread);
	err = devm_request_irq(dev, pdcg->pdc_irq,
	                       pdc_irq_handler,
	                       0, dev_name(dev), pdev);
	if (err) {
		dev_err(dev, "tx IRQ %u request failed with err %d\n",
			pdcg->pdc_irq, err);
		return err;
	}

	return PDC_SUCCESS;
}

int pdc_channel_init(struct platform_device *pdev, int chan)
{
	struct pdc_global *pdcg = (struct pdc_global *)platform_get_drvdata(pdev);
	struct pdc_state *pdcs = &pdcg->pdc_state[chan];

	spin_lock_init(&pdcs->pdc_lock);
	memset(pdcs, 0, sizeof(*pdcs));
	pdcs->pdev = pdev;
	pdcs->channel = chan;

	memcpy(&pdcs->bcm_header[0], BCMHEADER, BCM_HDR_LEN);
	pdcs->bcm_header[0] = (pdcs->bcm_header[0] | (chan << 3));
	pdcs->bcm_header_dma = dma_map_single(&pdev->dev, &pdcs->bcm_header[0],
	                                      BCM_HDR_LEN, DMA_TO_DEVICE);
	if (dma_mapping_error(&pdev->dev, pdcs->bcm_header_dma))
	{
		dev_crit(&pdev->dev, "Failed to map BCM header");
		return -ENOMEM;
	}

	pdcs->resp_header = dma_alloc_coherent(&pdev->dev,
	                                       PDC_RESP_HDR_LEN*PDC_RING_ENTRIES,
	                                       &pdcs->resp_header_dma, GFP_KERNEL);
	if (pdcs->resp_header == NULL) {
		dev_crit(&pdev->dev, "Failed to allocate response header");
		return -ENOMEM;
	}

	dev_dbg(&pdev->dev, "DMA rings allocation size %zu", PDC_RING_SIZE);
	pdcs->tx_ring_alloc.size = PDC_RING_SIZE;
	pdcs->tx_ring_alloc.vbase = dma_pool_alloc(pdcg->dma_pool, GFP_KERNEL,
	                                           &pdcs->tx_ring_alloc.pbase);
	if (pdcs->tx_ring_alloc.vbase == NULL) {
		dev_crit(&pdev->dev, "Failed to allocate TX DMA ring descriptors");
		return -ENOMEM;
	}
	memset(pdcs->tx_ring_alloc.vbase, 0, pdcs->tx_ring_alloc.size);

	pdcs->rx_ring_alloc.size = PDC_RING_SIZE;
	pdcs->rx_ring_alloc.vbase = dma_pool_alloc(pdcg->dma_pool, GFP_KERNEL,
	                                           &pdcs->rx_ring_alloc.pbase);
	if (pdcs->rx_ring_alloc.vbase == NULL) {
		dev_crit(&pdev->dev, "Failed to allocate RX DMA ring descriptors");
		return -ENOMEM;
	}
	memset(pdcs->rx_ring_alloc.vbase, 0, pdcs->rx_ring_alloc.size);

	pdcs->rxin = 0;
	pdcs->rxout = 0;
	pdcs->last_rx_curr = 0;
	pdcs->txin = 0;
	pdcs->txout = 0;
	INIT_LIST_HEAD(&pdcs->msg_list);
	pdcs->msg_count    =0;

	pdcs->txregs_64 = (struct dma64_regs *)((u8 *)pdcg->pdc_reg_vbase +
	                   PDC_TXREGS_OFFSET + (sizeof(struct dma64) * chan));
	pdcs->rxregs_64 = (struct dma64_regs *)((u8 *)pdcg->pdc_reg_vbase +
	                   PDC_RXREGS_OFFSET + (sizeof(struct dma64) * chan) );
	pdcs->txd_64    = (struct dma64dd *)pdcs->tx_ring_alloc.vbase;
	pdcs->rxd_64    = (struct dma64dd *)pdcs->rx_ring_alloc.vbase;

	reg_write32(0, (void *)&pdcs->rxregs_64->control);
	reg_write32(0, (void *)&pdcs->rxregs_64->ptr);
	reg_write32(0, (void *)&pdcs->txregs_64->control);
	reg_write32(0, (void *)&pdcs->txregs_64->ptr);

	reg_write32(0xFFFFFFFF & pdcs->tx_ring_alloc.pbase,
	            (void *)&pdcs->txregs_64->addrlow);
	reg_write32(0, (void *)&pdcs->txregs_64->addrhigh);
	reg_write32(GMAC_TX_CTL, (void *)&pdcs->txregs_64->control);

	reg_write32(0xFFFFFFFF & pdcs->rx_ring_alloc.pbase,
	            (void *)&pdcs->rxregs_64->addrlow);
	reg_write32(0, (void *)&pdcs->rxregs_64->addrhigh);
	reg_write32(GMAC_RX_CTL, (void *)&pdcs->rxregs_64->control);

	return PDC_SUCCESS;
}

void pdc_channel_deinit(struct platform_device *pdev, int chan)
{
	struct pdc_global *pdcg = (struct pdc_global *)platform_get_drvdata(pdev);
	struct pdc_state *pdcs = &pdcg->pdc_state[chan];

	/* disable DMA */
	reg_write32(GMAC_TX_CTL & ~0x1, (void *)&pdcs->txregs_64->control);
	reg_write32(GMAC_RX_CTL & ~0x1, (void *)&pdcs->rxregs_64->control);

	if (  pdcs->resp_header )
	{
		dma_free_coherent(&pdev->dev, PDC_RESP_HDR_LEN*PDC_RING_ENTRIES,
		                  pdcs->resp_header, pdcs->resp_header_dma);
	}

	if ( pdcs->tx_ring_alloc.vbase )
	{
		dma_pool_free(pdcg->dma_pool, pdcs->tx_ring_alloc.vbase,
		              pdcs->tx_ring_alloc.pbase);
	}

	if ( pdcs->rx_ring_alloc.vbase )
	{
		dma_pool_free(pdcg->dma_pool, pdcs->rx_ring_alloc.vbase,
		              pdcs->rx_ring_alloc.pbase);
	}
}

/* Initialize PDC driver.
 * Reserve register regions defined in device tree. For each SPU, there are
 * a set of hardware registers and a set of dma registers. Initialize a
 * mailbox controller for each SPU.
 */
static int pdc_probe(struct platform_device *pdev)
{
	int err = 0, ch;
	struct device *dev = &pdev->dev;
	struct device_node *dn = pdev->dev.of_node;
	struct resource *pdc_regs;
	struct pdc_global *pdcg;
	const void *dt_val_ptr;

	err = pmc_spu_power_up();
	if (err) {
		dev_crit(dev, "PDC device failed to power up. Error %d.", err);
		return err;
	}

	/* TODO - check if we can use 64 here */
	err = dma_set_mask_and_coherent(dev, DMA_BIT_MASK(32));
	if (err) {
		dev_crit(dev, "PDC device cannot perform DMA. Error %d.", err);
		return err;
	}

	if (!of_device_is_available(dn)) {
		dev_crit(dev, "PDC device not available");
		return -ENODEV;
	}

	pdcg = devm_kzalloc(dev, sizeof(*pdcg), GFP_KERNEL);
	if (pdcg == NULL) {
		err = -ENOMEM;
		goto cleanup;
	}
	platform_set_drvdata(pdev, pdcg);
	pdc_glob = pdcg;

	/* Get number of PDC channels from device tree */
	dt_val_ptr = of_get_property(dn, "brcm,num_chan", NULL);
	if (!dt_val_ptr) {
		dev_crit(dev,
			"%s failed to get num_chan from device tree",
			__func__);
		pdcg->num_chan = 1; /* assume one channel */
	} else {
		pdcg->num_chan = be32_to_cpup(dt_val_ptr);
		dev_dbg(dev, "Device has %d PDC channels",
			pdcg->num_chan);
	}
	atomic_set(&pdcg->next_chan, 0);

	pdcg->pdc_state = devm_kzalloc(dev, (sizeof(struct pdc_state) *
										pdcg->num_chan), GFP_KERNEL);
	if (!pdcg->pdc_state) {
		dev_crit(dev, "no memory for pdcg->pdc_state[]\n");
		err = -ENOMEM;
		goto cleanup;
	}

	pdc_regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (pdc_regs == NULL) {
		err = -ENODEV;
		goto cleanup;
	}

	pdcg->pdc_reg_vbase = devm_ioremap_resource(&pdev->dev, pdc_regs);
	if (IS_ERR(pdcg->pdc_reg_vbase)) {
		err = PTR_ERR(pdcg->pdc_reg_vbase);
		dev_crit(&pdev->dev, "Failed to map registers: %d\n", err);
		goto cleanup;
	}

	pdcg->dma_pool = dma_pool_create("pdc_dma", dev, PDC_RING_SIZE, PDC_RING_ALIGN, 0);
	if (!pdcg->dma_pool) {
		dev_crit(dev, "no memory for dma pool\n");
		err = -ENOMEM;
		goto cleanup;
	}

	pdcg->debugfs_stats = NULL;
	pdc_setup_debugfs(pdev);

	for (ch=0; ch < pdcg->num_chan; ch++) {
		pdc_channel_init(pdev, ch);
	}

	pdc_interrupts_init(pdev);

	dev_dbg(dev, "pdc_probe() successful");

cleanup:
	/* Failure. Free anything allocated here. */
	if (err)
		pdc_remove(pdev);
	return err;
}

static int pdc_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct pdc_global *pdcg = platform_get_drvdata(pdev);
	int ch;

	pr_debug("%s(): %p\n", __func__, pdev);

	/* disable interrupts */
	reg_write32(0, pdcg->pdc_reg_vbase + PDC_INTMASK_OFFSET);

	kthread_stop(pdcg->work_thread);

	devm_free_irq(dev, pdcg->pdc_irq, pdcg);

	for (ch=0; ch < pdcg->num_chan; ch++) {
		pdc_channel_deinit(pdev, ch);
	}

	pdc_free_debugfs_stats(pdev);
	pdc_free_debugfs(pdev);

	if (pdcg->pdc_reg_vbase)
		devm_iounmap(dev, pdcg->pdc_reg_vbase);

	if (pdcg->dma_pool) {
		dma_pool_destroy(pdcg->dma_pool);
	}

	if (pdcg->pdc_state) {
		devm_kfree(dev, pdcg->pdc_state);
	}

	devm_kfree(dev, pdcg);

	pmc_spu_power_down();

	return 0;
}

static const struct of_device_id bcm_iproc_dt_ids[] = {
	{ .compatible = "brcm,pdc"},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, bcm_iproc_dt_ids);

static struct platform_driver iproc_pdc_driver = {
	.probe = pdc_probe,
	.remove = pdc_remove,
	.driver = {
		.name = "bcmpdc",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(bcm_iproc_dt_ids),
	},
};
module_platform_driver(iproc_pdc_driver);

MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("GPL");
