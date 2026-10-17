// SPDX-License-Identifier: GPL-2.0+
/*
 * Multicore Navigator driver for TI Keystone 2 devices.
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */
#include <common.h>
#include <asm/io.h>
#include <asm/ti-common/keystone_nav.h>
#include <linux/delay.h>

struct qm_config qm_memmap = {
	.stat_cfg	= KS2_QM_QUEUE_STATUS_BASE,
	.queue		= (void *)KS2_QM_MANAGER_QUEUES_BASE,
	.mngr_vbusm	= KS2_QM_BASE_ADDRESS,
	.i_lram		= KS2_QM_LINK_RAM_BASE,
	.proxy		= (void *)KS2_QM_MANAGER_Q_PROXY_BASE,
	.status_ram	= KS2_QM_STATUS_RAM_BASE,
	.mngr_cfg	= (void *)KS2_QM_CONF_BASE,
	.intd_cfg	= KS2_QM_INTD_CONF_BASE,
	.desc_mem	= (void *)KS2_QM_DESC_SETUP_BASE,
	.region_num	= KS2_QM_REGION_NUM,
	.pdsp_cmd	= KS2_QM_PDSP1_CMD_BASE,
	.pdsp_ctl	= KS2_QM_PDSP1_CTRL_BASE,
	.pdsp_iram	= KS2_QM_PDSP1_IRAM_BASE,
	.qpool_num	= KS2_QM_QPOOL_NUM,
};

/*
 * We are going to use only one type of descriptors - host packet
 * descriptors. We staticaly allocate memory for them here
 */
struct qm_host_desc desc_pool[HDESC_NUM] __aligned(sizeof(struct qm_host_desc));

static struct qm_config *qm_cfg;

inline int num_of_desc_to_reg(int num_descr)
{
	int j, num;

	for (j = 0, num = 32; j < 15; j++, num *= 2) {
		if (num_descr <= num)
			return j;
	}

	return 15;
}

int _qm_init(struct qm_config *cfg)
{
	u32 j;

	qm_cfg = cfg;

	qm_cfg->mngr_cfg->link_ram_base0	= qm_cfg->i_lram;
	qm_cfg->mngr_cfg->link_ram_size0	= HDESC_NUM * 8 - 1;
	qm_cfg->mngr_cfg->link_ram_base1	= 0;
	qm_cfg->mngr_cfg->link_ram_size1	= 0;
	qm_cfg->mngr_cfg->link_ram_base2	= 0;

	qm_cfg->desc_mem[0].base_addr = (u32)desc_pool;
	qm_cfg->desc_mem[0].start_idx = 0;
	qm_cfg->desc_mem[0].desc_reg_size =
		(((sizeof(struct qm_host_desc) >> 4) - 1) << 16) |
		num_of_desc_to_reg(HDESC_NUM);

	memset(desc_pool, 0, sizeof(desc_pool));
	for (j = 0; j < HDESC_NUM; j++)
		qm_push(&desc_pool[j], qm_cfg->qpool_num);

	return QM_OK;
}

int qm_init(void)
{
	return _qm_init(&qm_memmap);
}

void qm_close(void)
{
	u32	j;

	queue_close(qm_cfg->qpool_num);

	qm_cfg->mngr_cfg->link_ram_base0	= 0;
	qm_cfg->mngr_cfg->link_ram_size0	= 0;
	qm_cfg->mngr_cfg->link_ram_base1	= 0;
	qm_cfg->mngr_cfg->link_ram_size1	= 0;
	qm_cfg->mngr_cfg->link_ram_base2	= 0;

	for (j = 0; j < qm_cfg->region_num; j++) {
		qm_cfg->desc_mem[j].base_addr = 0;
		qm_cfg->desc_mem[j].start_idx = 0;
		qm_cfg->desc_mem[j].desc_reg_size = 0;
	}

	qm_cfg = NULL;
}

void qm_push(struct qm_host_desc *hd, u32 qnum)
{
	u32 regd;

	cpu_to_bus((u32 *)hd, sizeof(struct qm_host_desc)/4);
	regd = (u32)hd | ((sizeof(struct qm_host_desc) >> 4) - 1);
	writel(regd, &qm_cfg->queue[qnum].ptr_size_thresh);
}

void qm_buff_push(struct qm_host_desc *hd, u32 qnum,
		    void *buff_ptr, u32 buff_len)
{
	hd->orig_buff_len = buff_len;
	hd->buff_len = buff_len;
	hd->orig_buff_ptr = (u32)buff_ptr;
	hd->buff_ptr = (u32)buff_ptr;
	qm_push(hd, qnum);
}

struct qm_host_desc *qm_pop(u32 qnum)
{
	u32 uhd;

	uhd = readl(&qm_cfg->queue[qnum].ptr_size_thresh) & ~0xf;
	if (uhd)
		cpu_to_bus((u32 *)uhd, sizeof(struct qm_host_desc)/4);

	return (struct qm_host_desc *)uhd;
}

struct qm_host_desc *qm_pop_from_free_pool(void)
{
	return qm_pop(qm_cfg->qpool_num);
}

void queue_close(u32 qnum)
{
	struct qm_host_desc *hd;

	while ((hd = qm_pop(qnum)))
		;
}

/**
 * DMA API
 */

static int ksnav_rx_disable(struct pktdma_cfg *pktdma)
{
	u32 j, v, k;

	for (j = 0; j < pktdma->rx_ch_num; j++) {
		v = readl(&pktdma->rx_ch[j].cfg_a);
		if (!(v & CPDMA_CHAN_A_ENABLE))
			continue;

		writel(v | CPDMA_CHAN_A_TDOWN, &pktdma->rx_ch[j].cfg_a);
		for (k = 0; k < TDOWN_TIMEOUT_COUNT; k++) {
			udelay(100);
			v = readl(&pktdma->rx_ch[j].cfg_a);
			if (!(v & CPDMA_CHAN_A_ENABLE))
				continue;
		}
		/* TODO: teardown error on if TDOWN_TIMEOUT_COUNT is reached */
	}

	/* Clear all of the flow registers */
	for (j = 0; j < pktdma->rx_flow_num; j++) {
		writel(0, &pktdma->rx_flows[j].control);
		writel(0, &pktdma->rx_flows[j].tags);
		writel(0, &pktdma->rx_flows[j].tag_sel);
		writel(0, &pktdma->rx_flows[j].fdq_sel[0]);
		writel(0, &pktdma->rx_flows[j].fdq_sel[1]);
		writel(0, &pktdma->rx_flows[j].thresh[0]);
		writel(0, &pktdma->rx_flows[j].thresh[1]);
		writel(0, &pktdma->rx_flows[j].thresh[2]);
	}

	return QM_OK;
}

static int ksnav_tx_disable(struct pktdma_cfg *pktdma)
{
	u32 j, v, k;

	for (j = 0; j < pktdma->tx_ch_num; j++) {
		v = readl(&pktdma->tx_ch[j].cfg_a);
		if (!(v & CPDMA_CHAN_A_ENABLE))
			continue;

		writel(v | CPDMA_CHAN_A_TDOWN, &pktdma->tx_ch[j].cfg_a);
		for (k = 0; k < TDOWN_TIMEOUT_COUNT; k++) {
			udelay(100);
			v = readl(&pktdma->tx_ch[j].cfg_a);
			if (!(v & CPDMA_CHAN_A_ENABLE))
				continue;
		}
		/* TODO: teardown error on if TDOWN_TIMEOUT_COUNT is reached */
	}

	return QM_OK;
}

int ksnav_init(struct pktdma_cfg *pktdma, struct rx_buff_desc *rx_buffers)
{
	u32 j, v;
	struct qm_host_desc *hd;
	u8 *rx_ptr;

	if (pktdma == NULL || rx_buffers == NULL ||
	    rx_buffers->buff_ptr == NULL || qm_cfg == NULL)
		return QM_ERR;

	pktdma->rx_flow = rx_buffers->rx_flow;

	/* init rx queue */
	rx_ptr = rx_buffers->buff_ptr;

	for (j = 0; j < rx_buffers->num_buffs; j++) {
		hd = qm_pop(qm_cfg->qpool_num);
		if (hd == NULL)
			return QM_ERR;

		qm_buff_push(hd, pktdma->rx_free_q,
			     rx_ptr, rx_buffers->buff_len);

		rx_ptr += rx_buffers->buff_len;
	}

	ksnav_rx_disable(pktdma);

	/* configure rx channels */
	v = CPDMA_REG_VAL_MAKE_RX_FLOW_A(1, 1, 0, 0, 0, 0, 0, pktdma->rx_rcv_q);
	writel(v, &pktdma->rx_flows[pktdma->rx_flow].control);
	writel(0, &pktdma->rx_flows[pktdma->rx_flow].tags);
	writel(0, &pktdma->rx_flows[pktdma->rx_flow].tag_sel);

	v = CPDMA_REG_VAL_MAKE_RX_FLOW_D(0, pktdma->rx_free_q, 0,
					 pktdma->rx_free_q);

	writel(v, &pktdma->rx_flows[pktdma->rx_flow].fdq_sel[0]);
	writel(v, &pktdma->rx_flows[pktdma->rx_flow].fdq_sel[1]);
	writel(0, &pktdma->rx_flows[pktdma->rx_flow].thresh[0]);
	writel(0, &pktdma->rx_flows[pktdma->rx_flow].thresh[1]);
	writel(0, &pktdma->rx_flows[pktdma->rx_flow].thresh[2]);

	for (j = 0; j < pktdma->rx_ch_num; j++)
		writel(CPDMA_CHAN_A_ENABLE, &pktdma->rx_ch[j].cfg_a);

	/* configure tx channels */
	/* Disable loopback in the tx direction */
	writel(0, &pktdma->global->emulation_control);

	/* Set QM base address, only for K2x devices */
	writel(KS2_QM_BASE_ADDRESS, &pktdma->global->qm_base_addr[0]);

	/* Enable all channels. The current state isn't important */
	for (j = 0; j < pktdma->tx_ch_num; j++)  {
		writel(0, &pktdma->tx_ch[j].cfg_b);
		writel(CPDMA_CHAN_A_ENABLE, &pktdma->tx_ch[j].cfg_a);
	}

	return QM_OK;
}

int ksnav_close(struct pktdma_cfg *pktdma)
{
	if (!pktdma)
		return QM_ERR;

	ksnav_tx_disable(pktdma);
	ksnav_rx_disable(pktdma);

	queue_close(pktdma->rx_free_q);
	queue_close(pktdma->rx_rcv_q);
	queue_close(pktdma->tx_snd_q);

	return QM_OK;
}

int ksnav_send(struct pktdma_cfg *pktdma, u32 *pkt, int num_bytes, u32 swinfo2)
{
	struct qm_host_desc *hd;

	hd = qm_pop(qm_cfg->qpool_num);
	if (hd == NULL)
		return QM_ERR;

	hd->desc_info	= num_bytes;
	hd->swinfo[2]	= swinfo2;
	hd->packet_info = qm_cfg->qpool_num;

	qm_buff_push(hd, pktdma->tx_snd_q, pkt, num_bytes);

	return QM_OK;
}

void *ksnav_recv(struct pktdma_cfg *pktdma, u32 **pkt, int *num_bytes)
{
	struct qm_host_desc *hd;

	hd = qm_pop(pktdma->rx_rcv_q);
	if (!hd)
		return NULL;

	*pkt = (u32 *)hd->buff_ptr;
	*num_bytes = hd->desc_info & 0x3fffff;

	return hd;
}

void ksnav_release_rxhd(struct pktdma_cfg *pktdma, void *hd)
{
	struct qm_host_desc *_hd = (struct qm_host_desc *)hd;

	_hd->buff_len = _hd->orig_buff_len;
	_hd->buff_ptr = _hd->orig_buff_ptr;

	qm_push(_hd, pktdma->rx_free_q);
}
