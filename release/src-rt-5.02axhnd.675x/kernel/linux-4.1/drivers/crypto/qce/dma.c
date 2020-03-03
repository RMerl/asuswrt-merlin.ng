/*
 * Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/dmaengine.h>
#include <crypto/scatterwalk.h>

#include "dma.h"

int qce_dma_request(struct device *dev, struct qce_dma_data *dma)
{
	int ret;

	dma->txchan = dma_request_slave_channel_reason(dev, "tx");
	if (IS_ERR(dma->txchan))
		return PTR_ERR(dma->txchan);

	dma->rxchan = dma_request_slave_channel_reason(dev, "rx");
	if (IS_ERR(dma->rxchan)) {
		ret = PTR_ERR(dma->rxchan);
		goto error_rx;
	}

	dma->result_buf = kmalloc(QCE_RESULT_BUF_SZ + QCE_IGNORE_BUF_SZ,
				  GFP_KERNEL);
	if (!dma->result_buf) {
		ret = -ENOMEM;
		goto error_nomem;
	}

	dma->ignore_buf = dma->result_buf + QCE_RESULT_BUF_SZ;

	return 0;
error_nomem:
	dma_release_channel(dma->rxchan);
error_rx:
	dma_release_channel(dma->txchan);
	return ret;
}

void qce_dma_release(struct qce_dma_data *dma)
{
	dma_release_channel(dma->txchan);
	dma_release_channel(dma->rxchan);
	kfree(dma->result_buf);
}

int qce_mapsg(struct device *dev, struct scatterlist *sg, int nents,
	      enum dma_data_direction dir, bool chained)
{
	int err;

	if (chained) {
		while (sg) {
			err = dma_map_sg(dev, sg, 1, dir);
			if (!err)
				return -EFAULT;
			sg = sg_next(sg);
		}
	} else {
		err = dma_map_sg(dev, sg, nents, dir);
		if (!err)
			return -EFAULT;
	}

	return nents;
}

void qce_unmapsg(struct device *dev, struct scatterlist *sg, int nents,
		 enum dma_data_direction dir, bool chained)
{
	if (chained)
		while (sg) {
			dma_unmap_sg(dev, sg, 1, dir);
			sg = sg_next(sg);
		}
	else
		dma_unmap_sg(dev, sg, nents, dir);
}

int qce_countsg(struct scatterlist *sglist, int nbytes, bool *chained)
{
	struct scatterlist *sg = sglist;
	int nents = 0;

	if (chained)
		*chained = false;

	while (nbytes > 0 && sg) {
		nents++;
		nbytes -= sg->length;
		if (!sg_is_last(sg) && (sg + 1)->length == 0 && chained)
			*chained = true;
		sg = sg_next(sg);
	}

	return nents;
}

struct scatterlist *
qce_sgtable_add(struct sg_table *sgt, struct scatterlist *new_sgl)
{
	struct scatterlist *sg = sgt->sgl, *sg_last = NULL;

	while (sg) {
		if (!sg_page(sg))
			break;
		sg = sg_next(sg);
	}

	if (!sg)
		return ERR_PTR(-EINVAL);

	while (new_sgl && sg) {
		sg_set_page(sg, sg_page(new_sgl), new_sgl->length,
			    new_sgl->offset);
		sg_last = sg;
		sg = sg_next(sg);
		new_sgl = sg_next(new_sgl);
	}

	return sg_last;
}

static int qce_dma_prep_sg(struct dma_chan *chan, struct scatterlist *sg,
			   int nents, unsigned long flags,
			   enum dma_transfer_direction dir,
			   dma_async_tx_callback cb, void *cb_param)
{
	struct dma_async_tx_descriptor *desc;
	dma_cookie_t cookie;

	if (!sg || !nents)
		return -EINVAL;

	desc = dmaengine_prep_slave_sg(chan, sg, nents, dir, flags);
	if (!desc)
		return -EINVAL;

	desc->callback = cb;
	desc->callback_param = cb_param;
	cookie = dmaengine_submit(desc);

	return dma_submit_error(cookie);
}

int qce_dma_prep_sgs(struct qce_dma_data *dma, struct scatterlist *rx_sg,
		     int rx_nents, struct scatterlist *tx_sg, int tx_nents,
		     dma_async_tx_callback cb, void *cb_param)
{
	struct dma_chan *rxchan = dma->rxchan;
	struct dma_chan *txchan = dma->txchan;
	unsigned long flags = DMA_PREP_INTERRUPT | DMA_CTRL_ACK;
	int ret;

	ret = qce_dma_prep_sg(rxchan, rx_sg, rx_nents, flags, DMA_MEM_TO_DEV,
			     NULL, NULL);
	if (ret)
		return ret;

	return qce_dma_prep_sg(txchan, tx_sg, tx_nents, flags, DMA_DEV_TO_MEM,
			       cb, cb_param);
}

void qce_dma_issue_pending(struct qce_dma_data *dma)
{
	dma_async_issue_pending(dma->rxchan);
	dma_async_issue_pending(dma->txchan);
}

int qce_dma_terminate_all(struct qce_dma_data *dma)
{
	int ret;

	ret = dmaengine_terminate_all(dma->rxchan);
	return ret ?: dmaengine_terminate_all(dma->txchan);
}
