// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019-2021 Broadcom.
 */

#include <common.h>

#include <asm/io.h>
#include <dm.h>
#include <linux/delay.h>
#include <memalign.h>
#include <net.h>

#include "bnxt.h"
#include "bnxt_dbg.h"

#define bnxt_down_chip(bp)     bnxt_hwrm_run(down_chip, bp, 0)
#define bnxt_bring_chip(bp)    bnxt_hwrm_run(bring_chip, bp, 1)

/* Broadcom ethernet driver PCI APIs. */
static void bnxt_bring_pci(struct bnxt *bp)
{
	u16 cmd_reg = 0;

	dm_pci_read_config16(bp->pdev, PCI_VENDOR_ID, &bp->vendor_id);
	dm_pci_read_config16(bp->pdev, PCI_DEVICE_ID, &bp->device_id);
	dm_pci_read_config16(bp->pdev, PCI_SUBSYSTEM_VENDOR_ID, &bp->subsystem_vendor);
	dm_pci_read_config16(bp->pdev, PCI_SUBSYSTEM_ID, &bp->subsystem_device);
	dm_pci_read_config16(bp->pdev, PCI_COMMAND, &bp->cmd_reg);
	dm_pci_read_config8(bp->pdev, PCI_INTERRUPT_LINE, &bp->irq);
	bp->bar0 = dm_pci_map_bar(bp->pdev, PCI_BASE_ADDRESS_0, 0, 0,
				  PCI_REGION_TYPE, PCI_REGION_MEM);
	bp->bar1 = dm_pci_map_bar(bp->pdev, PCI_BASE_ADDRESS_2, 0, 0,
				  PCI_REGION_TYPE, PCI_REGION_MEM);
	bp->bar2 = dm_pci_map_bar(bp->pdev, PCI_BASE_ADDRESS_4, 0, 0,
				  PCI_REGION_TYPE, PCI_REGION_MEM);
	cmd_reg = bp->cmd_reg | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER;
	cmd_reg |= PCI_COMMAND_INTX_DISABLE; /* disable intr */
	dm_pci_write_config16(bp->pdev, PCI_COMMAND, cmd_reg);
	dm_pci_read_config16(bp->pdev, PCI_COMMAND, &cmd_reg);
	dbg_pci(bp, __func__, cmd_reg);
}

int bnxt_free_rx_iob(struct bnxt *bp)
{
	unsigned int i;

	if (!(FLAG_TEST(bp->flag_hwrm, VALID_RX_IOB)))
		return STATUS_SUCCESS;

	for (i = 0; i < bp->rx.buf_cnt; i++) {
		if (bp->rx.iob[i]) {
			free(bp->rx.iob[i]);
			bp->rx.iob[i] = NULL;
		}
	}

	FLAG_RESET(bp->flag_hwrm, VALID_RX_IOB);

	return STATUS_SUCCESS;
}

static void set_rx_desc(u8 *buf, void *iob, u16 cons_id, u32 iob_idx)
{
	struct rx_prod_pkt_bd *desc;
	u16 off = cons_id * sizeof(struct rx_prod_pkt_bd);

	desc = (struct rx_prod_pkt_bd *)&buf[off];
	desc->flags_type = RX_PROD_PKT_BD_TYPE_RX_PROD_PKT;
	desc->len	 = MAX_ETHERNET_PACKET_BUFFER_SIZE;
	desc->opaque	 = iob_idx;
	desc->dma.addr = virt_to_bus(iob);
}

static int bnxt_alloc_rx_iob(struct bnxt *bp, u16 cons_id, u16 iob_idx)
{
	void *iob;

	iob = memalign(BNXT_DMA_ALIGNMENT, RX_STD_DMA_ALIGNED);
	if (!iob)
		return -ENOMEM;

	dbg_rx_iob(iob, iob_idx, cons_id);
	set_rx_desc((u8 *)bp->rx.bd_virt, iob, cons_id, (u32)iob_idx);
	bp->rx.iob[iob_idx] = iob;

	return 0;
}

void bnxt_mm_init(struct bnxt *bp, const char *func)
{
	memset(bp->hwrm_addr_req, 0, REQ_BUFFER_SIZE);
	memset(bp->hwrm_addr_resp, 0, RESP_BUFFER_SIZE);
	memset(bp->cq.bd_virt, 0, CQ_RING_DMA_BUFFER_SIZE);
	memset(bp->tx.bd_virt, 0, TX_RING_DMA_BUFFER_SIZE);
	memset(bp->rx.bd_virt, 0, RX_RING_DMA_BUFFER_SIZE);

	bp->data_addr_mapping = virt_to_bus(bp->hwrm_addr_data);
	bp->req_addr_mapping  = virt_to_bus(bp->hwrm_addr_req);
	bp->resp_addr_mapping = virt_to_bus(bp->hwrm_addr_resp);
	bp->wait_link_timeout = LINK_DEFAULT_TIMEOUT;
	bp->link_status       = STATUS_LINK_DOWN;
	bp->media_change      = 1;
	bp->mtu               = MAX_ETHERNET_PACKET_BUFFER_SIZE;
	bp->hwrm_max_req_len  = HWRM_MAX_REQ_LEN;
	bp->rx.buf_cnt        = NUM_RX_BUFFERS;
	bp->rx.ring_cnt       = MAX_RX_DESC_CNT;
	bp->tx.ring_cnt       = MAX_TX_DESC_CNT;
	bp->cq.ring_cnt       = MAX_CQ_DESC_CNT;
	bp->cq.completion_bit = 0x1;
	bp->link_set          = LINK_SPEED_DRV_100G;
	dbg_mem(bp, func);
}

void bnxt_free_mem(struct bnxt *bp)
{
	if (bp->cq.bd_virt) {
		free(bp->cq.bd_virt);
		bp->cq.bd_virt = NULL;
	}

	if (bp->rx.bd_virt) {
		free(bp->rx.bd_virt);
		bp->rx.bd_virt = NULL;
	}

	if (bp->tx.bd_virt) {
		free(bp->tx.bd_virt);
		bp->tx.bd_virt = NULL;
	}

	if (bp->hwrm_addr_resp) {
		free(bp->hwrm_addr_resp);
		bp->resp_addr_mapping = 0;
		bp->hwrm_addr_resp = NULL;
	}

	if (bp->hwrm_addr_req) {
		free(bp->hwrm_addr_req);
		bp->req_addr_mapping = 0;
		bp->hwrm_addr_req = NULL;
	}

	if (bp->hwrm_addr_data) {
		free(bp->hwrm_addr_data);
		bp->data_addr_mapping = 0;
		bp->hwrm_addr_data = NULL;
	}

	dbg_mem_free_done(__func__);
}

int bnxt_alloc_mem(struct bnxt *bp)
{
	bp->hwrm_addr_data = memalign(BNXT_DMA_ALIGNMENT, DMA_BUF_SIZE_ALIGNED);
	bp->hwrm_addr_req = memalign(BNXT_DMA_ALIGNMENT, REQ_BUF_SIZE_ALIGNED);
	bp->hwrm_addr_resp = MEM_HWRM_RESP;

	memset(&bp->tx, 0, sizeof(struct lm_tx_info_t));
	memset(&bp->rx, 0, sizeof(struct lm_rx_info_t));
	memset(&bp->cq, 0, sizeof(struct lm_cmp_info_t));

	bp->tx.bd_virt = memalign(BNXT_DMA_ALIGNMENT, TX_RING_DMA_BUFFER_SIZE);
	bp->rx.bd_virt = memalign(BNXT_DMA_ALIGNMENT, RX_RING_DMA_BUFFER_SIZE);
	bp->cq.bd_virt = memalign(BNXT_DMA_ALIGNMENT, CQ_RING_DMA_BUFFER_SIZE);

	if (bp->hwrm_addr_req &&
	    bp->hwrm_addr_resp &&
	    bp->hwrm_addr_data &&
	    bp->tx.bd_virt &&
	    bp->rx.bd_virt &&
	    bp->cq.bd_virt) {
		bnxt_mm_init(bp, __func__);
		return STATUS_SUCCESS;
	}

	dbg_mem_alloc_fail(__func__);
	bnxt_free_mem(bp);

	return -ENOMEM;
}

static void hwrm_init(struct bnxt *bp, struct input *req, u16 cmd, u16 len)
{
	memset(req, 0, len);
	req->req_type  = cmd;
	req->cmpl_ring = (u16)HWRM_NA_SIGNATURE;
	req->target_id = (u16)HWRM_NA_SIGNATURE;
	req->resp_addr = bp->resp_addr_mapping;
	req->seq_id    = bp->seq_id++;
}

static void hwrm_write_req(struct bnxt *bp, void *req, u32 cnt)
{
	u32 i = 0;

	for (i = 0; i < cnt; i++)
		writel(((u32 *)req)[i], bp->bar0 + GRC_COM_CHAN_BASE + (i * 4));

	writel(0x1, (bp->bar0 + GRC_COM_CHAN_BASE + GRC_COM_CHAN_TRIG));
}

static void short_hwrm_cmd_req(struct bnxt *bp, u16 len)
{
	struct hwrm_short_input sreq;

	memset(&sreq, 0, sizeof(struct hwrm_short_input));
	sreq.req_type  = (u16)((struct input *)bp->hwrm_addr_req)->req_type;
	sreq.signature = SHORT_REQ_SIGNATURE_SHORT_CMD;
	sreq.size      = len;
	sreq.req_addr  = bp->req_addr_mapping;
	dbg_short_cmd((u8 *)&sreq, __func__, sizeof(struct hwrm_short_input));
	hwrm_write_req(bp, &sreq, sizeof(struct hwrm_short_input) / 4);
}

static int wait_resp(struct bnxt *bp, u32 tmo, u16 len, const char *func)
{
	struct input *req = (struct input *)bp->hwrm_addr_req;
	struct output *resp = (struct output *)bp->hwrm_addr_resp;
	u8  *ptr = (u8 *)resp;
	u32 idx;
	u32 wait_cnt = HWRM_CMD_DEFAULT_MULTIPLAYER((u32)tmo);
	u16 resp_len = 0;
	u16 ret = STATUS_TIMEOUT;

	if (len > bp->hwrm_max_req_len)
		short_hwrm_cmd_req(bp, len);
	else
		hwrm_write_req(bp, req, (u32)(len / 4));

	for (idx = 0; idx < wait_cnt; idx++) {
		resp_len = resp->resp_len;
		if (resp->seq_id == req->seq_id && resp->req_type == req->req_type &&
		    ptr[resp_len - 1] == 1) {
			bp->last_resp_code = resp->error_code;
			ret = resp->error_code;
			break;
		}

		udelay(HWRM_CMD_POLL_WAIT_TIME);
	}

	dbg_hw_cmd(bp, func, len, resp_len, tmo, ret);

	return (int)ret;
}

static void bnxt_db_cq(struct bnxt *bp)
{
	writel(CQ_DOORBELL_KEY_IDX(bp->cq.cons_idx), bp->bar1);
}

static void bnxt_db_rx(struct bnxt *bp, u32 idx)
{
	writel(RX_DOORBELL_KEY_RX | idx, bp->bar1);
}

static void bnxt_db_tx(struct bnxt *bp, u32 idx)
{
	writel((u32)(TX_DOORBELL_KEY_TX | idx), bp->bar1);
}

int iob_pad(void *packet, int length)
{
	if (length >= ETH_ZLEN)
		return length;

	memset(((u8 *)packet + length), 0x00, (ETH_ZLEN - length));

	return ETH_ZLEN;
}

static inline u32 bnxt_tx_avail(struct bnxt *bp)
{
	barrier();

	return TX_AVAIL(bp->tx.ring_cnt) -
			((bp->tx.prod_id - bp->tx.cons_id) &
			(bp->tx.ring_cnt - 1));
}

void set_txq(struct bnxt *bp, int entry, dma_addr_t mapping, int len)
{
	struct tx_bd_short *prod_bd;

	prod_bd = (struct tx_bd_short *)BD_NOW(bp->tx.bd_virt,
					       entry,
					       sizeof(struct tx_bd_short));
	if (len < 512)
		prod_bd->flags_type = TX_BD_SHORT_FLAGS_LHINT_LT512;
	else if (len < 1024)
		prod_bd->flags_type = TX_BD_SHORT_FLAGS_LHINT_LT1K;
	else if (len < 2048)
		prod_bd->flags_type = TX_BD_SHORT_FLAGS_LHINT_LT2K;
	else
		prod_bd->flags_type = TX_BD_SHORT_FLAGS_LHINT_GTE2K;

	prod_bd->flags_type |= TX_BD_FLAGS;
	prod_bd->dma.addr = mapping;
	prod_bd->len      = len;
	prod_bd->opaque   = (u32)entry;
	dump_tx_bd(prod_bd, (u16)(sizeof(struct tx_bd_short)));
}

static void bnxt_tx_complete(struct bnxt *bp)
{
	bp->tx.cons_id = NEXT_IDX(bp->tx.cons_id, bp->tx.ring_cnt);
	bp->tx.cnt++;
	dump_tx_stat(bp);
}

int post_rx_buffers(struct bnxt *bp)
{
	u16 cons_id = (bp->rx.cons_idx % bp->rx.ring_cnt);
	u16 iob_idx;

	while (bp->rx.iob_cnt < bp->rx.buf_cnt) {
		iob_idx = (cons_id % bp->rx.buf_cnt);
		if (!bp->rx.iob[iob_idx]) {
			if (bnxt_alloc_rx_iob(bp, cons_id, iob_idx) < 0) {
				dbg_rx_alloc_iob_fail(iob_idx, cons_id);
				break;
			}
		}

		cons_id = NEXT_IDX(cons_id, bp->rx.ring_cnt);
		bp->rx.iob_cnt++;
	}

	if (cons_id != bp->rx.cons_idx) {
		dbg_rx_cid(bp->rx.cons_idx, cons_id);
		bp->rx.cons_idx = cons_id;
		bnxt_db_rx(bp, (u32)cons_id);
	}

	FLAG_SET(bp->flag_hwrm, VALID_RX_IOB);

	return STATUS_SUCCESS;
}

u8 bnxt_rx_drop(struct bnxt *bp, u8 *rx_buf, struct rx_pkt_cmpl_hi *rx_cmp_hi)
{
	u8  chksum_err = 0;
	u8  i;
	u16 error_flags;

	error_flags = (rx_cmp_hi->errors_v2 >>
		RX_PKT_CMPL_ERRORS_BUFFER_ERROR_SFT);
	if (rx_cmp_hi->errors_v2 == 0x20 || rx_cmp_hi->errors_v2 == 0x21)
		chksum_err = 1;

	if (error_flags && !chksum_err) {
		bp->rx.err++;
		return 1;
	}

	for (i = 0; i < 6; i++) {
		if (rx_buf[6 + i] != bp->mac_set[i])
			break;
	}

	if (i == 6) {
		bp->rx.dropped++;
		return 2; /* Drop the loopback packets */
	}

	return 0;
}

static void bnxt_adv_cq_index(struct bnxt *bp, u16 count)
{
	u16 cons_idx = bp->cq.cons_idx + count;

	if (cons_idx >= MAX_CQ_DESC_CNT) {
		/* Toggle completion bit when the ring wraps. */
		bp->cq.completion_bit ^= 1;
		cons_idx = cons_idx - MAX_CQ_DESC_CNT;
	}

	bp->cq.cons_idx = cons_idx;
}

void bnxt_adv_rx_index(struct bnxt *bp, u8 *iob, u32 iob_idx)
{
	u16 cons_id = (bp->rx.cons_idx % bp->rx.ring_cnt);

	set_rx_desc((u8 *)bp->rx.bd_virt, (void *)iob, cons_id, iob_idx);
	cons_id = NEXT_IDX(cons_id, bp->rx.ring_cnt);
	if (cons_id != bp->rx.cons_idx) {
		dbg_rx_cid(bp->rx.cons_idx, cons_id);
		bp->rx.cons_idx = cons_id;
		bnxt_db_rx(bp, (u32)cons_id);
	}
}

void rx_process(struct bnxt *bp, struct rx_pkt_cmpl *rx_cmp,
		struct rx_pkt_cmpl_hi *rx_cmp_hi)
{
	u32 desc_idx = rx_cmp->opaque;
	u8  *iob = bp->rx.iob[desc_idx];

	dump_rx_bd(rx_cmp, rx_cmp_hi, desc_idx);
	bp->rx.iob_len = rx_cmp->len;
	bp->rx.iob_rx  = iob;
	if (bnxt_rx_drop(bp, iob, rx_cmp_hi))
		bp->rx.iob_recv = PKT_DROPPED;
	else
		bp->rx.iob_recv = PKT_RECEIVED;

	bp->rx.rx_cnt++;

	dbg_rxp(bp->rx.iob_rx, bp->rx.iob_len, bp->rx.iob_recv);
	bnxt_adv_rx_index(bp, iob, desc_idx);
	bnxt_adv_cq_index(bp, 2); /* Rx completion is 2 entries. */
}

static int bnxt_rx_complete(struct bnxt *bp, struct rx_pkt_cmpl *rx_cmp)
{
	struct rx_pkt_cmpl_hi *rx_cmp_hi;
	u8  completion_bit = bp->cq.completion_bit;

	if (bp->cq.cons_idx == (bp->cq.ring_cnt - 1)) {
		rx_cmp_hi = (struct rx_pkt_cmpl_hi *)bp->cq.bd_virt;
		completion_bit ^= 0x1; /* Ring has wrapped. */
	} else {
		rx_cmp_hi = (struct rx_pkt_cmpl_hi *)(rx_cmp + 1);
	}

	if (!((rx_cmp_hi->errors_v2 & RX_PKT_CMPL_V2) ^ completion_bit))
		rx_process(bp, rx_cmp, rx_cmp_hi);

	return NO_MORE_CQ_BD_TO_SERVICE;
}

static int bnxt_hwrm_ver_get(struct bnxt *bp)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_ver_get_input);
	struct hwrm_ver_get_input *req;
	struct hwrm_ver_get_output *resp;
	int rc;

	req = (struct hwrm_ver_get_input *)bp->hwrm_addr_req;
	resp = (struct hwrm_ver_get_output *)bp->hwrm_addr_resp;
	hwrm_init(bp, (void *)req, (u16)HWRM_VER_GET, cmd_len);
	req->hwrm_intf_maj = HWRM_VERSION_MAJOR;
	req->hwrm_intf_min = HWRM_VERSION_MINOR;
	req->hwrm_intf_upd = HWRM_VERSION_UPDATE;
	rc = wait_resp(bp, HWRM_CMD_DEFAULT_TIMEOUT, cmd_len, __func__);
	if (rc)
		return STATUS_FAILURE;

	bp->hwrm_spec_code =
		resp->hwrm_intf_maj_8b << 16 |
		resp->hwrm_intf_min_8b << 8 |
		resp->hwrm_intf_upd_8b;
	bp->hwrm_cmd_timeout = (u32)resp->def_req_timeout;
	if (!bp->hwrm_cmd_timeout)
		bp->hwrm_cmd_timeout = (u32)HWRM_CMD_DEFAULT_TIMEOUT;

	if (resp->hwrm_intf_maj_8b >= 1)
		bp->hwrm_max_req_len = resp->max_req_win_len;

	bp->chip_id =
		resp->chip_rev << 24 |
		resp->chip_metal << 16 |
		resp->chip_bond_id << 8 |
		resp->chip_platform_type;
	bp->chip_num = resp->chip_num;
	if ((resp->dev_caps_cfg & SHORT_CMD_SUPPORTED) &&
	    (resp->dev_caps_cfg & SHORT_CMD_REQUIRED))
		FLAG_SET(bp->flags, BNXT_FLAG_HWRM_SHORT_CMD_SUPP);

	bp->hwrm_max_ext_req_len = resp->max_ext_req_len;
	bp->fw_maj  = resp->hwrm_fw_maj_8b;
	bp->fw_min  = resp->hwrm_fw_min_8b;
	bp->fw_bld  = resp->hwrm_fw_bld_8b;
	bp->fw_rsvd = resp->hwrm_fw_rsvd_8b;
	print_fw_ver(resp, bp->hwrm_cmd_timeout);

	return STATUS_SUCCESS;
}

/* Broadcom ethernet driver Function HW cmds APIs. */
static int bnxt_hwrm_func_resource_qcaps(struct bnxt *bp)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_func_resource_qcaps_input);
	struct hwrm_func_resource_qcaps_input *req;
	struct hwrm_func_resource_qcaps_output *resp;
	int rc;

	req = (struct hwrm_func_resource_qcaps_input *)bp->hwrm_addr_req;
	resp = (struct hwrm_func_resource_qcaps_output *)bp->hwrm_addr_resp;
	hwrm_init(bp, (void *)req, (u16)HWRM_FUNC_RESOURCE_QCAPS, cmd_len);
	req->fid = (u16)HWRM_NA_SIGNATURE;
	rc = wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);
	if (rc != STATUS_SUCCESS)
		return STATUS_SUCCESS;

	FLAG_SET(bp->flags, BNXT_FLAG_RESOURCE_QCAPS_SUPPORT);
	/* VFs */
	bp->max_vfs = resp->max_vfs;
	bp->vf_res_strategy = resp->vf_reservation_strategy;
	/* vNICs */
	bp->min_vnics = resp->min_vnics;
	bp->max_vnics = resp->max_vnics;
	/* MSI-X */
	bp->max_msix = resp->max_msix;
	/* Ring Groups */
	bp->min_hw_ring_grps = resp->min_hw_ring_grps;
	bp->max_hw_ring_grps = resp->max_hw_ring_grps;
	/* TX Rings */
	bp->min_tx_rings = resp->min_tx_rings;
	bp->max_tx_rings = resp->max_tx_rings;
	/* RX Rings */
	bp->min_rx_rings = resp->min_rx_rings;
	bp->max_rx_rings = resp->max_rx_rings;
	/* Completion Rings */
	bp->min_cp_rings = resp->min_cmpl_rings;
	bp->max_cp_rings = resp->max_cmpl_rings;
	/* RSS Contexts */
	bp->min_rsscos_ctxs = resp->min_rsscos_ctx;
	bp->max_rsscos_ctxs = resp->max_rsscos_ctx;
	/* L2 Contexts */
	bp->min_l2_ctxs = resp->min_l2_ctxs;
	bp->max_l2_ctxs = resp->max_l2_ctxs;
	/* Statistic Contexts */
	bp->min_stat_ctxs = resp->min_stat_ctx;
	bp->max_stat_ctxs = resp->max_stat_ctx;
	dbg_func_resource_qcaps(bp);

	return STATUS_SUCCESS;
}

static u32 set_ring_info(struct bnxt *bp)
{
	u32 enables = 0;

	bp->num_cmpl_rings   = DEFAULT_NUMBER_OF_CMPL_RINGS;
	bp->num_tx_rings     = DEFAULT_NUMBER_OF_TX_RINGS;
	bp->num_rx_rings     = DEFAULT_NUMBER_OF_RX_RINGS;
	bp->num_hw_ring_grps = DEFAULT_NUMBER_OF_RING_GRPS;
	bp->num_stat_ctxs    = DEFAULT_NUMBER_OF_STAT_CTXS;
	if (bp->min_cp_rings <= DEFAULT_NUMBER_OF_CMPL_RINGS)
		bp->num_cmpl_rings = bp->min_cp_rings;

	if (bp->min_tx_rings <= DEFAULT_NUMBER_OF_TX_RINGS)
		bp->num_tx_rings = bp->min_tx_rings;

	if (bp->min_rx_rings <= DEFAULT_NUMBER_OF_RX_RINGS)
		bp->num_rx_rings = bp->min_rx_rings;

	if (bp->min_hw_ring_grps <= DEFAULT_NUMBER_OF_RING_GRPS)
		bp->num_hw_ring_grps = bp->min_hw_ring_grps;

	if (bp->min_stat_ctxs <= DEFAULT_NUMBER_OF_STAT_CTXS)
		bp->num_stat_ctxs = bp->min_stat_ctxs;

	print_num_rings(bp);
	enables = (FUNC_CFG_REQ_ENABLES_NUM_CMPL_RINGS |
		FUNC_CFG_REQ_ENABLES_NUM_TX_RINGS |
		FUNC_CFG_REQ_ENABLES_NUM_RX_RINGS |
		FUNC_CFG_REQ_ENABLES_NUM_STAT_CTXS |
		FUNC_CFG_REQ_ENABLES_NUM_HW_RING_GRPS);

	return enables;
}

static void bnxt_hwrm_assign_resources(struct bnxt *bp)
{
	struct hwrm_func_cfg_input *req;
	u32 enables = 0;

	if (FLAG_TEST(bp->flags, BNXT_FLAG_RESOURCE_QCAPS_SUPPORT))
		enables = set_ring_info(bp);

	req = (struct hwrm_func_cfg_input *)bp->hwrm_addr_req;
	req->num_cmpl_rings   = bp->num_cmpl_rings;
	req->num_tx_rings     = bp->num_tx_rings;
	req->num_rx_rings     = bp->num_rx_rings;
	req->num_stat_ctxs    = bp->num_stat_ctxs;
	req->num_hw_ring_grps = bp->num_hw_ring_grps;
	req->enables = enables;
}

int bnxt_hwrm_nvm_flush(struct bnxt *bp)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_nvm_flush_input);
	struct hwrm_nvm_flush_input *req;
	int rc;

	req = (struct hwrm_nvm_flush_input *)bp->hwrm_addr_req;

	hwrm_init(bp, (void *)req, (u16)HWRM_NVM_FLUSH, cmd_len);

	rc = wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);
	if (rc)
		return STATUS_FAILURE;

	return STATUS_SUCCESS;
}

static int bnxt_hwrm_func_qcaps_req(struct bnxt *bp)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_func_qcaps_input);
	struct hwrm_func_qcaps_input *req;
	struct hwrm_func_qcaps_output *resp;
	int rc;

	req = (struct hwrm_func_qcaps_input *)bp->hwrm_addr_req;
	resp = (struct hwrm_func_qcaps_output *)bp->hwrm_addr_resp;
	hwrm_init(bp, (void *)req, (u16)HWRM_FUNC_QCAPS, cmd_len);
	req->fid = (u16)HWRM_NA_SIGNATURE;
	rc = wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);
	if (rc)
		return STATUS_FAILURE;

	bp->fid = resp->fid;
	bp->port_idx = (u8)resp->port_id;

	/* Get MAC address for this PF */
	memcpy(&bp->mac_addr[0], &resp->mac_address[0], ETH_ALEN);

	memcpy(&bp->mac_set[0], &bp->mac_addr[0], ETH_ALEN);

	print_func_qcaps(bp);

	return STATUS_SUCCESS;
}

static int bnxt_hwrm_func_qcfg_req(struct bnxt *bp)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_func_qcfg_input);
	struct hwrm_func_qcfg_input *req;
	struct hwrm_func_qcfg_output *resp;
	int rc;

	req = (struct hwrm_func_qcfg_input *)bp->hwrm_addr_req;
	resp = (struct hwrm_func_qcfg_output *)bp->hwrm_addr_resp;
	hwrm_init(bp, (void *)req, (u16)HWRM_FUNC_QCFG, cmd_len);
	req->fid = (u16)HWRM_NA_SIGNATURE;
	rc = wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);
	if (rc)
		return STATUS_FAILURE;

	if (resp->flags & FUNC_QCFG_RESP_FLAGS_MULTI_HOST)
		FLAG_SET(bp->flags, BNXT_FLAG_MULTI_HOST);

	if (resp->port_partition_type &
		FUNC_QCFG_RESP_PORT_PARTITION_TYPE_NPAR1_0)
		FLAG_SET(bp->flags, BNXT_FLAG_NPAR_MODE);

	bp->ordinal_value = (u8)resp->pci_id & 0x0F;
	bp->stat_ctx_id   = resp->stat_ctx_id;
	memcpy(&bp->mac_addr[0], &resp->mac_address[0], ETH_ALEN);
	print_func_qcfg(bp);
	dbg_flags(__func__, bp->flags);

	return STATUS_SUCCESS;
}

static int bnxt_hwrm_func_reset_req(struct bnxt *bp)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_func_reset_input);
	struct hwrm_func_reset_input *req;

	req = (struct hwrm_func_reset_input *)bp->hwrm_addr_req;
	hwrm_init(bp, (void *)req, (u16)HWRM_FUNC_RESET, cmd_len);
	req->func_reset_level = FUNC_RESET_REQ_FUNC_RESET_LEVEL_RESETME;

	return wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);
}

static int bnxt_hwrm_func_cfg_req(struct bnxt *bp)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_func_cfg_input);
	struct hwrm_func_cfg_input *req;

	req = (struct hwrm_func_cfg_input *)bp->hwrm_addr_req;
	hwrm_init(bp, (void *)req, (u16)HWRM_FUNC_CFG, cmd_len);
	req->fid = (u16)HWRM_NA_SIGNATURE;
	bnxt_hwrm_assign_resources(bp);

	return wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);
}

static int bnxt_hwrm_func_drv_rgtr(struct bnxt *bp)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_func_drv_rgtr_input);
	struct hwrm_func_drv_rgtr_input *req;
	int rc;

	req = (struct hwrm_func_drv_rgtr_input *)bp->hwrm_addr_req;
	hwrm_init(bp, (void *)req, (u16)HWRM_FUNC_DRV_RGTR, cmd_len);
	/* Register with HWRM */
	req->enables = FUNC_DRV_RGTR_REQ_ENABLES_OS_TYPE |
			FUNC_DRV_RGTR_REQ_ENABLES_ASYNC_EVENT_FWD |
			FUNC_DRV_RGTR_REQ_ENABLES_VER;
	req->async_event_fwd[0] |= 0x01;
	req->os_type = FUNC_DRV_RGTR_REQ_OS_TYPE_OTHER;
	req->ver_maj = DRIVER_VERSION_MAJOR;
	req->ver_min = DRIVER_VERSION_MINOR;
	req->ver_upd = DRIVER_VERSION_UPDATE;
	rc = wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);
	if (rc)
		return STATUS_FAILURE;

	FLAG_SET(bp->flag_hwrm, VALID_DRIVER_REG);

	return STATUS_SUCCESS;
}

static int bnxt_hwrm_func_drv_unrgtr(struct bnxt *bp)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_func_drv_unrgtr_input);
	struct hwrm_func_drv_unrgtr_input *req;
	int rc;

	if (!(FLAG_TEST(bp->flag_hwrm, VALID_DRIVER_REG)))
		return STATUS_SUCCESS;

	req = (struct hwrm_func_drv_unrgtr_input *)bp->hwrm_addr_req;
	hwrm_init(bp, (void *)req, (u16)HWRM_FUNC_DRV_UNRGTR, cmd_len);
	req->flags = FUNC_DRV_UNRGTR_REQ_FLAGS_PREPARE_FOR_SHUTDOWN;
	rc = wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);
	if (rc)
		return STATUS_FAILURE;

	FLAG_RESET(bp->flag_hwrm, VALID_DRIVER_REG);

	return STATUS_SUCCESS;
}

static int bnxt_hwrm_cfa_l2_filter_alloc(struct bnxt *bp)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_cfa_l2_filter_alloc_input);
	struct hwrm_cfa_l2_filter_alloc_input *req;
	struct hwrm_cfa_l2_filter_alloc_output *resp;
	int rc;
	u32 flags = CFA_L2_FILTER_ALLOC_REQ_FLAGS_PATH_RX;
	u32 enables;

	req = (struct hwrm_cfa_l2_filter_alloc_input *)bp->hwrm_addr_req;
	resp = (struct hwrm_cfa_l2_filter_alloc_output *)bp->hwrm_addr_resp;
	enables = CFA_L2_FILTER_ALLOC_REQ_ENABLES_DST_ID |
		CFA_L2_FILTER_ALLOC_REQ_ENABLES_L2_ADDR |
		CFA_L2_FILTER_ALLOC_REQ_ENABLES_L2_ADDR_MASK;

	hwrm_init(bp, (void *)req, (u16)HWRM_CFA_L2_FILTER_ALLOC, cmd_len);
	req->flags      = flags;
	req->enables    = enables;
	memcpy((char *)&req->l2_addr[0], (char *)&bp->mac_set[0], ETH_ALEN);
	memset((char *)&req->l2_addr_mask[0], 0xff, ETH_ALEN);
	memcpy((char *)&req->t_l2_addr[0], (char *)&bp->mac_set[0], ETH_ALEN);
	memset((char *)&req->t_l2_addr_mask[0], 0xff, ETH_ALEN);
	req->src_type = CFA_L2_FILTER_ALLOC_REQ_SRC_TYPE_NPORT;
	req->src_id   = (u32)bp->port_idx;
	req->dst_id   = bp->vnic_id;
	rc = wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);
	if (rc)
		return STATUS_FAILURE;

	FLAG_SET(bp->flag_hwrm, VALID_L2_FILTER);
	bp->l2_filter_id = resp->l2_filter_id;

	return STATUS_SUCCESS;
}

static int bnxt_hwrm_cfa_l2_filter_free(struct bnxt *bp)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_cfa_l2_filter_free_input);
	struct hwrm_cfa_l2_filter_free_input *req;
	int rc;

	if (!(FLAG_TEST(bp->flag_hwrm, VALID_L2_FILTER)))
		return STATUS_SUCCESS;

	req = (struct hwrm_cfa_l2_filter_free_input *)bp->hwrm_addr_req;
	hwrm_init(bp, (void *)req, (u16)HWRM_CFA_L2_FILTER_FREE, cmd_len);
	req->l2_filter_id = bp->l2_filter_id;
	rc = wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);
	if (rc)
		return STATUS_FAILURE;

	FLAG_RESET(bp->flag_hwrm, VALID_L2_FILTER);

	return STATUS_SUCCESS;
}

u32 bnxt_set_rx_mask(u32 rx_mask)
{
	u32 mask = 0;

	if (!rx_mask)
		return mask;
	mask = CFA_L2_SET_RX_MASK_REQ_MASK_BCAST;
	if (rx_mask != RX_MASK_ACCEPT_NONE) {
		if (rx_mask & RX_MASK_ACCEPT_MULTICAST)
			mask |= CFA_L2_SET_RX_MASK_REQ_MASK_MCAST;

		if (rx_mask & RX_MASK_ACCEPT_ALL_MULTICAST)
			mask |= CFA_L2_SET_RX_MASK_REQ_MASK_ALL_MCAST;

		if (rx_mask & RX_MASK_PROMISCUOUS_MODE)
			mask |= CFA_L2_SET_RX_MASK_REQ_MASK_PROMISCUOUS;
	}

	return mask;
}

static int bnxt_hwrm_set_rx_mask(struct bnxt *bp, u32 rx_mask)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_cfa_l2_set_rx_mask_input);
	struct hwrm_cfa_l2_set_rx_mask_input *req;
	u32 mask = bnxt_set_rx_mask(rx_mask);

	req = (struct hwrm_cfa_l2_set_rx_mask_input *)bp->hwrm_addr_req;
	hwrm_init(bp, (void *)req, (u16)HWRM_CFA_L2_SET_RX_MASK, cmd_len);
	req->vnic_id = bp->vnic_id;
	req->mask    = mask;

	return wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);
}

static int bnxt_hwrm_port_mac_cfg(struct bnxt *bp)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_port_mac_cfg_input);
	struct hwrm_port_mac_cfg_input *req;

	req = (struct hwrm_port_mac_cfg_input *)bp->hwrm_addr_req;
	hwrm_init(bp, (void *)req, (u16)HWRM_PORT_MAC_CFG, cmd_len);
	req->lpbk = PORT_MAC_CFG_REQ_LPBK_NONE;

	return wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);
}

static int bnxt_hwrm_port_phy_qcfg(struct bnxt *bp, u16 idx)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_port_phy_qcfg_input);
	struct hwrm_port_phy_qcfg_input *req;
	struct hwrm_port_phy_qcfg_output *resp;
	int rc;

	req = (struct hwrm_port_phy_qcfg_input *)bp->hwrm_addr_req;
	resp = (struct hwrm_port_phy_qcfg_output *)bp->hwrm_addr_resp;
	hwrm_init(bp, (void *)req, (u16)HWRM_PORT_PHY_QCFG, cmd_len);
	rc = wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);
	if (rc)
		return STATUS_FAILURE;

	if (idx & SUPPORT_SPEEDS)
		bp->support_speeds = resp->support_speeds;

	if (idx & DETECT_MEDIA)
		bp->media_detect = resp->module_status;

	if (idx & PHY_SPEED)
		bp->current_link_speed = resp->link_speed;

	if (idx & PHY_STATUS) {
		if (resp->link == PORT_PHY_QCFG_RESP_LINK_LINK)
			bp->link_status = STATUS_LINK_ACTIVE;
		else
			bp->link_status = STATUS_LINK_DOWN;
	}

	return STATUS_SUCCESS;
}

u16 set_link_speed_mask(u16 link_cap)
{
	u16 speed_mask = 0;

	if (link_cap & SPEED_CAPABILITY_DRV_100M)
		speed_mask |= PORT_PHY_CFG_REQ_AUTO_LINK_SPEED_MASK_100MB;

	if (link_cap & SPEED_CAPABILITY_DRV_1G)
		speed_mask |= PORT_PHY_CFG_REQ_AUTO_LINK_SPEED_MASK_1GB;

	if (link_cap & SPEED_CAPABILITY_DRV_10G)
		speed_mask |= PORT_PHY_CFG_REQ_AUTO_LINK_SPEED_MASK_10GB;

	if (link_cap & SPEED_CAPABILITY_DRV_25G)
		speed_mask |= PORT_PHY_CFG_REQ_AUTO_LINK_SPEED_MASK_25GB;

	if (link_cap & SPEED_CAPABILITY_DRV_40G)
		speed_mask |= PORT_PHY_CFG_REQ_AUTO_LINK_SPEED_MASK_40GB;

	if (link_cap & SPEED_CAPABILITY_DRV_50G)
		speed_mask |= PORT_PHY_CFG_REQ_AUTO_LINK_SPEED_MASK_50GB;

	if (link_cap & SPEED_CAPABILITY_DRV_100G)
		speed_mask |= PORT_PHY_CFG_REQ_AUTO_LINK_SPEED_MASK_100GB;

	return speed_mask;
}

static int bnxt_hwrm_port_phy_cfg(struct bnxt *bp)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_port_phy_cfg_input);
	struct hwrm_port_phy_cfg_input *req;
	u32 flags;
	u32 enables = 0;
	u16 force_link_speed = 0;
	u16 auto_link_speed_mask = 0;
	u8  auto_mode = 0;
	u8  auto_pause = 0;
	u8  auto_duplex = 0;

	/*
	 * If multi_host or NPAR is set to TRUE,
	 * do not issue hwrm_port_phy_cfg
	 */
	if (FLAG_TEST(bp->flags, PORT_PHY_FLAGS)) {
		dbg_flags(__func__, bp->flags);
		return STATUS_SUCCESS;
	}

	req = (struct hwrm_port_phy_cfg_input *)bp->hwrm_addr_req;
	flags = PORT_PHY_CFG_REQ_FLAGS_FORCE |
		PORT_PHY_CFG_REQ_FLAGS_RESET_PHY;

	switch (GET_MEDIUM_SPEED(bp->medium)) {
	case MEDIUM_SPEED_1000MBPS:
		force_link_speed = PORT_PHY_CFG_REQ_FORCE_LINK_SPEED_1GB;
		break;
	case MEDIUM_SPEED_10GBPS:
		force_link_speed = PORT_PHY_CFG_REQ_FORCE_LINK_SPEED_10GB;
		break;
	case MEDIUM_SPEED_25GBPS:
		force_link_speed = PORT_PHY_CFG_REQ_FORCE_LINK_SPEED_25GB;
		break;
	case MEDIUM_SPEED_40GBPS:
		force_link_speed = PORT_PHY_CFG_REQ_FORCE_LINK_SPEED_40GB;
		break;
	case MEDIUM_SPEED_50GBPS:
		force_link_speed = PORT_PHY_CFG_REQ_FORCE_LINK_SPEED_50GB;
		break;
	case MEDIUM_SPEED_100GBPS:
		force_link_speed = PORT_PHY_CFG_REQ_FORCE_LINK_SPEED_100GB;
		break;
	default:
		/* Enable AUTONEG by default */
		auto_mode = PORT_PHY_CFG_REQ_AUTO_MODE_SPEED_MASK;
		flags &= ~PORT_PHY_CFG_REQ_FLAGS_FORCE;
		enables |= PORT_PHY_CFG_REQ_ENABLES_AUTO_MODE |
			PORT_PHY_CFG_REQ_ENABLES_AUTO_LINK_SPEED_MASK |
			PORT_PHY_CFG_REQ_ENABLES_AUTO_DUPLEX |
			PORT_PHY_CFG_REQ_ENABLES_AUTO_PAUSE;
		auto_pause = PORT_PHY_CFG_REQ_AUTO_PAUSE_TX |
				PORT_PHY_CFG_REQ_AUTO_PAUSE_RX;
		auto_duplex = PORT_PHY_CFG_REQ_AUTO_DUPLEX_BOTH;
		auto_link_speed_mask = bp->support_speeds;
		break;
	}

	hwrm_init(bp, (void *)req, (u16)HWRM_PORT_PHY_CFG, cmd_len);
	req->flags                = flags;
	req->enables              = enables;
	req->port_id              = bp->port_idx;
	req->force_link_speed     = force_link_speed;
	req->auto_mode            = auto_mode;
	req->auto_duplex          = auto_duplex;
	req->auto_pause           = auto_pause;
	req->auto_link_speed_mask = auto_link_speed_mask;

	return wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);
}

static int bnxt_qphy_link(struct bnxt *bp)
{
	u16 flag = QCFG_PHY_ALL;

	/* Query Link Status */
	if (bnxt_hwrm_port_phy_qcfg(bp, flag) != STATUS_SUCCESS)
		return STATUS_FAILURE;

	if (bp->link_status != STATUS_LINK_ACTIVE) {
		/*
		 * Configure link if it is not up.
		 * try to bring link up, but don't return
		 * failure if port_phy_cfg() fails
		 */
		bnxt_hwrm_port_phy_cfg(bp);
		/* refresh link speed values after bringing link up */
		if (bnxt_hwrm_port_phy_qcfg(bp, flag) != STATUS_SUCCESS)
			return STATUS_FAILURE;
	}

	return STATUS_SUCCESS;
}

static int bnxt_hwrm_stat_ctx_alloc(struct bnxt *bp)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_stat_ctx_alloc_input);
	struct hwrm_stat_ctx_alloc_input *req;
	struct hwrm_stat_ctx_alloc_output *resp;
	int rc;

	req = (struct hwrm_stat_ctx_alloc_input *)bp->hwrm_addr_req;
	resp = (struct hwrm_stat_ctx_alloc_output *)bp->hwrm_addr_resp;
	hwrm_init(bp, (void *)req, (u16)HWRM_STAT_CTX_ALLOC, cmd_len);
	rc = wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);
	if (rc)
		return STATUS_FAILURE;

	FLAG_SET(bp->flag_hwrm, VALID_STAT_CTX);
	bp->stat_ctx_id = (u16)resp->stat_ctx_id;

	return STATUS_SUCCESS;
}

static int bnxt_hwrm_stat_ctx_free(struct bnxt *bp)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_stat_ctx_free_input);
	struct hwrm_stat_ctx_free_input *req;
	int rc;

	if (!(FLAG_TEST(bp->flag_hwrm, VALID_STAT_CTX)))
		return STATUS_SUCCESS;

	req = (struct hwrm_stat_ctx_free_input *)bp->hwrm_addr_req;
	hwrm_init(bp, (void *)req, (u16)HWRM_STAT_CTX_FREE, cmd_len);
	req->stat_ctx_id = (u32)bp->stat_ctx_id;
	rc = wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);
	if (rc)
		return STATUS_FAILURE;

	FLAG_RESET(bp->flag_hwrm, VALID_STAT_CTX);

	return STATUS_SUCCESS;
}

static int bnxt_hwrm_ring_free_grp(struct bnxt *bp)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_ring_grp_free_input);
	struct hwrm_ring_grp_free_input *req;
	int rc;

	if (!(FLAG_TEST(bp->flag_hwrm, VALID_RING_GRP)))
		return STATUS_SUCCESS;

	req = (struct hwrm_ring_grp_free_input *)bp->hwrm_addr_req;
	hwrm_init(bp, (void *)req, (u16)HWRM_RING_GRP_FREE, cmd_len);
	req->ring_group_id = (u32)bp->ring_grp_id;
	rc = wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);
	if (rc)
		return STATUS_FAILURE;

	FLAG_RESET(bp->flag_hwrm, VALID_RING_GRP);

	return STATUS_SUCCESS;
}

static int bnxt_hwrm_ring_alloc_grp(struct bnxt *bp)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_ring_grp_alloc_input);
	struct hwrm_ring_grp_alloc_input *req;
	struct hwrm_ring_grp_alloc_output *resp;
	int rc;

	req = (struct hwrm_ring_grp_alloc_input *)bp->hwrm_addr_req;
	resp = (struct hwrm_ring_grp_alloc_output *)bp->hwrm_addr_resp;
	hwrm_init(bp, (void *)req, (u16)HWRM_RING_GRP_ALLOC, cmd_len);
	req->cr = bp->cq_ring_id;
	req->rr = bp->rx_ring_id;
	req->ar = (u16)HWRM_NA_SIGNATURE;
	rc = wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);
	if (rc)
		return STATUS_FAILURE;

	FLAG_SET(bp->flag_hwrm, VALID_RING_GRP);
	bp->ring_grp_id = (u16)resp->ring_group_id;

	return STATUS_SUCCESS;
}

int bnxt_hwrm_ring_free(struct bnxt *bp, u16 ring_id, u8 ring_type)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_ring_free_input);
	struct hwrm_ring_free_input *req;

	req = (struct hwrm_ring_free_input *)bp->hwrm_addr_req;
	hwrm_init(bp, (void *)req, (u16)HWRM_RING_FREE, cmd_len);
	req->ring_type = ring_type;
	req->ring_id   = ring_id;

	return wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);
}

static int bnxt_hwrm_ring_alloc(struct bnxt *bp,
				dma_addr_t ring_map,
				u16 length,
				u16 ring_id,
				u8 ring_type,
				u8 int_mode)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_ring_alloc_input);
	struct hwrm_ring_alloc_input *req;
	struct hwrm_ring_alloc_output *resp;
	int rc;

	req = (struct hwrm_ring_alloc_input *)bp->hwrm_addr_req;
	resp = (struct hwrm_ring_alloc_output *)bp->hwrm_addr_resp;
	hwrm_init(bp, (void *)req, (u16)HWRM_RING_ALLOC, cmd_len);
	req->ring_type     = ring_type;
	req->page_tbl_addr = ring_map;
	req->page_size     = LM_PAGE_SIZE;
	req->length        = (u32)length;
	req->cmpl_ring_id  = ring_id;
	req->int_mode      = int_mode;
	if (ring_type == RING_ALLOC_REQ_RING_TYPE_TX) {
		req->queue_id    = TX_RING_QID;
	} else if (ring_type == RING_ALLOC_REQ_RING_TYPE_RX) {
		req->queue_id    = RX_RING_QID;
		req->enables     = RING_ALLOC_REQ_ENABLES_RX_BUF_SIZE_VALID;
		req->rx_buf_size = MAX_ETHERNET_PACKET_BUFFER_SIZE;
	}

	rc = wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);
	if (rc)
		return STATUS_FAILURE;

	if (ring_type == RING_ALLOC_REQ_RING_TYPE_L2_CMPL) {
		FLAG_SET(bp->flag_hwrm, VALID_RING_CQ);
		bp->cq_ring_id = resp->ring_id;
	} else if (ring_type == RING_ALLOC_REQ_RING_TYPE_TX) {
		FLAG_SET(bp->flag_hwrm, VALID_RING_TX);
		bp->tx_ring_id = resp->ring_id;
	} else if (ring_type == RING_ALLOC_REQ_RING_TYPE_RX) {
		FLAG_SET(bp->flag_hwrm, VALID_RING_RX);
		bp->rx_ring_id = resp->ring_id;
	}

	return STATUS_SUCCESS;
}

static int bnxt_hwrm_ring_alloc_cq(struct bnxt *bp)
{
	return bnxt_hwrm_ring_alloc(bp,
				    virt_to_bus(bp->cq.bd_virt),
				    bp->cq.ring_cnt,
				    0,
				    RING_ALLOC_REQ_RING_TYPE_L2_CMPL,
				    BNXT_CQ_INTR_MODE());
}

static int bnxt_hwrm_ring_alloc_tx(struct bnxt *bp)
{
	return bnxt_hwrm_ring_alloc(bp,
				    virt_to_bus(bp->tx.bd_virt),
				    bp->tx.ring_cnt, bp->cq_ring_id,
				    RING_ALLOC_REQ_RING_TYPE_TX,
				    BNXT_INTR_MODE());
}

static int bnxt_hwrm_ring_alloc_rx(struct bnxt *bp)
{
	return bnxt_hwrm_ring_alloc(bp,
				    virt_to_bus(bp->rx.bd_virt),
				    bp->rx.ring_cnt,
				    bp->cq_ring_id,
				    RING_ALLOC_REQ_RING_TYPE_RX,
				    BNXT_INTR_MODE());
}

static int bnxt_hwrm_ring_free_cq(struct bnxt *bp)
{
	int ret = STATUS_SUCCESS;

	if (!(FLAG_TEST(bp->flag_hwrm, VALID_RING_CQ)))
		return ret;

	ret = RING_FREE(bp, bp->cq_ring_id, RING_FREE_REQ_RING_TYPE_L2_CMPL);
	if (ret == STATUS_SUCCESS)
		FLAG_RESET(bp->flag_hwrm, VALID_RING_CQ);

	return ret;
}

static int bnxt_hwrm_ring_free_tx(struct bnxt *bp)
{
	int ret = STATUS_SUCCESS;

	if (!(FLAG_TEST(bp->flag_hwrm, VALID_RING_TX)))
		return ret;

	ret = RING_FREE(bp, bp->tx_ring_id, RING_FREE_REQ_RING_TYPE_TX);
	if (ret == STATUS_SUCCESS)
		FLAG_RESET(bp->flag_hwrm, VALID_RING_TX);

	return ret;
}

static int bnxt_hwrm_ring_free_rx(struct bnxt *bp)
{
	int ret = STATUS_SUCCESS;

	if (!(FLAG_TEST(bp->flag_hwrm, VALID_RING_RX)))
		return ret;

	ret = RING_FREE(bp, bp->rx_ring_id, RING_FREE_REQ_RING_TYPE_RX);
	if (ret == STATUS_SUCCESS)
		FLAG_RESET(bp->flag_hwrm, VALID_RING_RX);

	return ret;
}

static int bnxt_hwrm_vnic_alloc(struct bnxt *bp)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_vnic_alloc_input);
	struct hwrm_vnic_alloc_input *req;
	struct hwrm_vnic_alloc_output *resp;
	int rc;

	req = (struct hwrm_vnic_alloc_input *)bp->hwrm_addr_req;
	resp = (struct hwrm_vnic_alloc_output *)bp->hwrm_addr_resp;
	hwrm_init(bp, (void *)req, (u16)HWRM_VNIC_ALLOC, cmd_len);
	req->flags = VNIC_ALLOC_REQ_FLAGS_DEFAULT;
	rc = wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);
	if (rc)
		return STATUS_FAILURE;

	FLAG_SET(bp->flag_hwrm, VALID_VNIC_ID);
	bp->vnic_id = resp->vnic_id;

	return STATUS_SUCCESS;
}

static int bnxt_hwrm_vnic_free(struct bnxt *bp)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_vnic_free_input);
	struct hwrm_vnic_free_input *req;
	int rc;

	if (!(FLAG_TEST(bp->flag_hwrm, VALID_VNIC_ID)))
		return STATUS_SUCCESS;

	req = (struct hwrm_vnic_free_input *)bp->hwrm_addr_req;
	hwrm_init(bp, (void *)req, (u16)HWRM_VNIC_FREE, cmd_len);
	req->vnic_id = bp->vnic_id;
	rc = wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);
	if (rc)
		return STATUS_FAILURE;

	FLAG_RESET(bp->flag_hwrm, VALID_VNIC_ID);

	return STATUS_SUCCESS;
}

static int bnxt_hwrm_vnic_cfg(struct bnxt *bp)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_vnic_cfg_input);
	struct hwrm_vnic_cfg_input *req;

	req = (struct hwrm_vnic_cfg_input *)bp->hwrm_addr_req;
	hwrm_init(bp, (void *)req, (u16)HWRM_VNIC_CFG, cmd_len);
	req->enables = VNIC_CFG_REQ_ENABLES_MRU;
	req->mru     = bp->mtu;
	req->enables |= VNIC_CFG_REQ_ENABLES_DFLT_RING_GRP;
	req->dflt_ring_grp = bp->ring_grp_id;
	req->vnic_id = bp->vnic_id;

	return wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);
}

static int set_phy_speed(struct bnxt *bp)
{
	char name[20];
	u16 flag = PHY_STATUS | PHY_SPEED | DETECT_MEDIA;

	/* Query Link Status */
	if (bnxt_hwrm_port_phy_qcfg(bp, flag) != STATUS_SUCCESS)
		return STATUS_FAILURE;

	switch (bp->current_link_speed) {
	case PORT_PHY_QCFG_RESP_LINK_SPEED_100GB:
		sprintf(name, "%s %s", str_100, str_gbps);
		break;
	case PORT_PHY_QCFG_RESP_LINK_SPEED_50GB:
		sprintf(name, "%s %s", str_50, str_gbps);
		break;
	case PORT_PHY_QCFG_RESP_LINK_SPEED_40GB:
		sprintf(name, "%s %s", str_40, str_gbps);
		break;
	case PORT_PHY_QCFG_RESP_LINK_SPEED_25GB:
		sprintf(name, "%s %s", str_25, str_gbps);
		break;
	case PORT_PHY_QCFG_RESP_LINK_SPEED_20GB:
		sprintf(name, "%s %s", str_20, str_gbps);
		break;
	case PORT_PHY_QCFG_RESP_LINK_SPEED_10GB:
		sprintf(name, "%s %s", str_10, str_gbps);
		break;
	case PORT_PHY_QCFG_RESP_LINK_SPEED_2_5GB:
		sprintf(name, "%s %s", str_2_5, str_gbps);
		break;
	case PORT_PHY_QCFG_RESP_LINK_SPEED_2GB:
		sprintf(name, "%s %s", str_2, str_gbps);
		break;
	case PORT_PHY_QCFG_RESP_LINK_SPEED_1GB:
		sprintf(name, "%s %s", str_1, str_gbps);
		break;
	case PORT_PHY_QCFG_RESP_LINK_SPEED_100MB:
		sprintf(name, "%s %s", str_100, str_mbps);
		break;
	case PORT_PHY_QCFG_RESP_LINK_SPEED_10MB:
		sprintf(name, "%s %s", str_10, str_mbps);
		break;
	default:
		sprintf(name, "%s %x", str_unknown, bp->current_link_speed);
	}

	dbg_phy_speed(bp, name);

	return STATUS_SUCCESS;
}

static int set_phy_link(struct bnxt *bp, u32 tmo)
{
	int ret;

	set_phy_speed(bp);
	dbg_link_status(bp);
	ret = STATUS_FAILURE;
	if (bp->link_status == STATUS_LINK_ACTIVE) {
		dbg_link_state(bp, tmo);
		ret = STATUS_SUCCESS;
	}

	return ret;
}

static int get_phy_link(struct bnxt *bp)
{
	u16 flag = PHY_STATUS | PHY_SPEED | DETECT_MEDIA;

	dbg_chip_info(bp);
	/* Query Link Status */
	if (bnxt_hwrm_port_phy_qcfg(bp, flag) != STATUS_SUCCESS)
		return STATUS_FAILURE;

	set_phy_link(bp, 100);

	return STATUS_SUCCESS;
}

static int bnxt_hwrm_set_async_event(struct bnxt *bp)
{
	int rc;
	u16 cmd_len = (u16)sizeof(struct hwrm_func_cfg_input);
	struct hwrm_func_cfg_input *req;

	req = (struct hwrm_func_cfg_input *)bp->hwrm_addr_req;
	hwrm_init(bp, (void *)req, (u16)HWRM_FUNC_CFG, cmd_len);
	req->fid = (u16)HWRM_NA_SIGNATURE;
	req->enables = FUNC_CFG_REQ_ENABLES_ASYNC_EVENT_CR;
	req->async_event_cr = bp->cq_ring_id;
	rc = wait_resp(bp, bp->hwrm_cmd_timeout, cmd_len, __func__);

	return rc;
}

int bnxt_hwrm_get_nvmem(struct bnxt *bp,
			u16 data_len,
			u16 option_num,
			u16 dimensions,
			u16 index_0)
{
	u16 cmd_len = (u16)sizeof(struct hwrm_nvm_get_variable_input);
	struct hwrm_nvm_get_variable_input *req;

	req = (struct hwrm_nvm_get_variable_input *)bp->hwrm_addr_req;
	hwrm_init(bp, (void *)req, (u16)HWRM_NVM_GET_VARIABLE, cmd_len);
	req->dest_data_addr = bp->data_addr_mapping;
	req->data_len       = data_len;
	req->option_num     = option_num;
	req->dimensions     = dimensions;
	req->index_0        = index_0;

	return wait_resp(bp,
			 HWRM_CMD_FLASH_MULTIPLAYER(bp->hwrm_cmd_timeout),
			 cmd_len,
			 __func__);
}

static void set_medium(struct bnxt *bp)
{
	switch (bp->link_set & LINK_SPEED_DRV_MASK) {
	case LINK_SPEED_DRV_1G:
		bp->medium = SET_MEDIUM_SPEED(bp, MEDIUM_SPEED_1000MBPS);
		break;
	case LINK_SPEED_DRV_2_5G:
		bp->medium = SET_MEDIUM_SPEED(bp, MEDIUM_SPEED_2500MBPS);
		break;
	case LINK_SPEED_DRV_10G:
		bp->medium = SET_MEDIUM_SPEED(bp, MEDIUM_SPEED_10GBPS);
		break;
	case LINK_SPEED_DRV_25G:
		bp->medium = SET_MEDIUM_SPEED(bp, MEDIUM_SPEED_25GBPS);
		break;
	case LINK_SPEED_DRV_40G:
		bp->medium = SET_MEDIUM_SPEED(bp, MEDIUM_SPEED_40GBPS);
		break;
	case LINK_SPEED_DRV_50G:
		bp->medium = SET_MEDIUM_SPEED(bp, MEDIUM_SPEED_50GBPS);
		break;
	case LINK_SPEED_DRV_100G:
		bp->medium = SET_MEDIUM_SPEED(bp, MEDIUM_SPEED_100GBPS);
		break;
	case LINK_SPEED_DRV_200G:
		bp->medium = SET_MEDIUM_SPEED(bp, MEDIUM_SPEED_200GBPS);
		break;
	case LINK_SPEED_DRV_AUTONEG:
		bp->medium = SET_MEDIUM_SPEED(bp, MEDIUM_SPEED_AUTONEG);
		break;
	default:
		bp->medium = SET_MEDIUM_DUPLEX(bp, MEDIUM_FULL_DUPLEX);
		break;
	}
}

static int bnxt_hwrm_get_link_speed(struct bnxt *bp)
{
	u32 *ptr32 = (u32 *)bp->hwrm_addr_data;

	if (bnxt_hwrm_get_nvmem(bp,
				4,
				(u16)LINK_SPEED_DRV_NUM,
				1,
				(u16)bp->port_idx) != STATUS_SUCCESS)
		return STATUS_FAILURE;

	bp->link_set  = *ptr32;
	bp->link_set &= SPEED_DRV_MASK;
	set_medium(bp);

	return STATUS_SUCCESS;
}

typedef int (*hwrm_func_t)(struct bnxt *bp);

hwrm_func_t down_chip[] = {
	bnxt_hwrm_cfa_l2_filter_free,    /* Free l2 filter  */
	bnxt_free_rx_iob,                /* Free rx iob     */
	bnxt_hwrm_vnic_free,             /* Free vnic       */
	bnxt_hwrm_ring_free_grp,         /* Free ring group */
	bnxt_hwrm_ring_free_rx,          /* Free rx ring    */
	bnxt_hwrm_ring_free_tx,          /* Free tx ring    */
	bnxt_hwrm_ring_free_cq,          /* Free CQ ring    */
	bnxt_hwrm_stat_ctx_free,         /* Free Stat ctx   */
	bnxt_hwrm_func_drv_unrgtr,       /* unreg driver    */
	NULL,
};

hwrm_func_t bring_chip[] = {
	bnxt_hwrm_ver_get,              /* HWRM_VER_GET                 */
	bnxt_hwrm_func_reset_req,       /* HWRM_FUNC_RESET              */
	bnxt_hwrm_func_drv_rgtr,        /* HWRM_FUNC_DRV_RGTR           */
	bnxt_hwrm_func_resource_qcaps,  /* HWRM_FUNC_RESOURCE_QCAPS     */
	bnxt_hwrm_func_qcfg_req,        /* HWRM_FUNC_QCFG               */
	bnxt_hwrm_func_qcaps_req,       /* HWRM_FUNC_QCAPS              */
	bnxt_hwrm_get_link_speed,       /* HWRM_NVM_GET_VARIABLE - 203  */
	bnxt_hwrm_port_mac_cfg,         /* HWRM_PORT_MAC_CFG            */
	bnxt_qphy_link,                 /* HWRM_PORT_PHY_QCFG           */
	bnxt_hwrm_func_cfg_req,         /* HWRM_FUNC_CFG - ring resource*/
	bnxt_hwrm_stat_ctx_alloc,       /* Allocate Stat Ctx ID         */
	bnxt_hwrm_ring_alloc_cq,        /* Allocate CQ Ring             */
	bnxt_hwrm_ring_alloc_tx,        /* Allocate Tx ring             */
	bnxt_hwrm_ring_alloc_rx,        /* Allocate Rx Ring             */
	bnxt_hwrm_ring_alloc_grp,       /* Create Ring Group            */
	post_rx_buffers,                /* Post RX buffers              */
	bnxt_hwrm_set_async_event,      /* ENABLES_ASYNC_EVENT_CR       */
	bnxt_hwrm_vnic_alloc,           /* Alloc VNIC                   */
	bnxt_hwrm_vnic_cfg,             /* Config VNIC                  */
	bnxt_hwrm_cfa_l2_filter_alloc,  /* Alloc L2 Filter              */
	get_phy_link,                   /* Get Physical Link            */
	NULL,
};

int bnxt_hwrm_run(hwrm_func_t cmds[], struct bnxt *bp, int flag)
{
	hwrm_func_t *ptr;
	int ret;
	int status = STATUS_SUCCESS;

	for (ptr = cmds; *ptr; ++ptr) {
		ret = (*ptr)(bp);
		if (ret) {
			status = STATUS_FAILURE;
			/* Continue till all cleanup routines are called */
			if (flag)
				return STATUS_FAILURE;
		}
	}

	return status;
}

/* Broadcom ethernet driver Network interface APIs. */
static int bnxt_start(struct udevice *dev)
{
	struct bnxt *bp = dev_get_priv(dev);

	if (bnxt_hwrm_set_rx_mask(bp, RX_MASK) != STATUS_SUCCESS)
		return STATUS_FAILURE;

	bp->card_en = true;
	return STATUS_SUCCESS;
}

static int bnxt_send(struct udevice *dev, void *packet, int length)
{
	struct bnxt *bp = dev_get_priv(dev);
	int len;
	u16 entry;
	dma_addr_t mapping;

	if (bnxt_tx_avail(bp) < 1) {
		dbg_no_tx_bd();
		return -ENOBUFS;
	}

	entry   = bp->tx.prod_id;
	len     = iob_pad(packet, length);
	mapping = virt_to_bus(packet);
	set_txq(bp, entry, mapping, len);
	entry   = NEXT_IDX(entry, bp->tx.ring_cnt);
	dump_tx_pkt(packet, mapping, len);
	bnxt_db_tx(bp, (u32)entry);
	bp->tx.prod_id = entry;
	bp->tx.cnt_req++;
	bnxt_tx_complete(bp);

	return 0;
}

static void bnxt_link_evt(struct bnxt *bp, struct cmpl_base *cmp)
{
	struct hwrm_async_event_cmpl *evt;

	evt = (struct hwrm_async_event_cmpl *)cmp;
	switch (evt->event_id) {
	case ASYNC_EVENT_CMPL_EVENT_ID_LINK_STATUS_CHANGE:
		if (evt->event_data1 & 0x01)
			bp->link_status = STATUS_LINK_ACTIVE;
		else
			bp->link_status = STATUS_LINK_DOWN;

		set_phy_link(bp, 0);
		break;
	default:
		break;
	}
}

static int bnxt_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct bnxt *bp = dev_get_priv(dev);
	struct cmpl_base *cmp;
	u16 old_cons_idx = bp->cq.cons_idx;
	int done = SERVICE_NEXT_CQ_BD;
	u32 cq_type;

	while (done == SERVICE_NEXT_CQ_BD) {
		cmp = (struct cmpl_base *)BD_NOW(bp->cq.bd_virt,
			bp->cq.cons_idx,
			sizeof(struct cmpl_base));
		if ((cmp->info3_v & CMPL_BASE_V) ^ bp->cq.completion_bit)
			break;

		cq_type = cmp->type & CMPL_BASE_TYPE_MASK;
		dump_evt((u8 *)cmp, cq_type, bp->cq.cons_idx);
		dump_CQ(cmp, bp->cq.cons_idx);
		switch (cq_type) {
		case CMPL_BASE_TYPE_HWRM_ASYNC_EVENT:
			bnxt_link_evt(bp, cmp);
			fallthrough;
		case CMPL_BASE_TYPE_TX_L2:
		case CMPL_BASE_TYPE_STAT_EJECT:
			bnxt_adv_cq_index(bp, 1);
			break;
		case CMPL_BASE_TYPE_RX_L2:
			done = bnxt_rx_complete(bp, (struct rx_pkt_cmpl *)cmp);
			break;
		default:
			done = NO_MORE_CQ_BD_TO_SERVICE;
			break;
		}
	}

	if (bp->cq.cons_idx != old_cons_idx)
		bnxt_db_cq(bp);

	if (bp->rx.iob_recv == PKT_RECEIVED) {
		*packetp = bp->rx.iob_rx;
		return bp->rx.iob_len;
	}

	return -EAGAIN;
}

static void bnxt_stop(struct udevice *dev)
{
	struct bnxt *bp = dev_get_priv(dev);

	if (bp->card_en) {
		bnxt_hwrm_set_rx_mask(bp, 0);
		bp->card_en = false;
	}
}

static int bnxt_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct bnxt *bp = dev_get_priv(dev);

	dbg_rx_pkt(bp, __func__, packet, length);
	bp->rx.iob_recv = PKT_DONE;
	bp->rx.iob_len  = 0;
	bp->rx.iob_rx   = NULL;

	return 0;
}

static int bnxt_read_rom_hwaddr(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_plat(dev);
	struct bnxt *bp = dev_get_priv(dev);

	memcpy(plat->enetaddr, bp->mac_set, ETH_ALEN);

	return 0;
}

static const struct eth_ops bnxt_eth_ops = {
	.start           = bnxt_start,
	.send            = bnxt_send,
	.recv            = bnxt_recv,
	.stop            = bnxt_stop,
	.free_pkt        = bnxt_free_pkt,
	.read_rom_hwaddr = bnxt_read_rom_hwaddr,
};

static const struct udevice_id bnxt_eth_ids[] = {
	{ .compatible = "broadcom,nxe" },
	{ }
};

static int bnxt_eth_bind(struct udevice *dev)
{
	char name[20];

	sprintf(name, "bnxt_eth%u", dev_seq(dev));

	return device_set_name(dev, name);
}

static int bnxt_eth_probe(struct udevice *dev)
{
	struct bnxt *bp = dev_get_priv(dev);
	int ret;

	ret = bnxt_alloc_mem(bp);
	if (ret) {
		printf("*** error: bnxt_alloc_mem failed! ***\n");
		return ret;
	}

	bp->cardnum = dev_seq(dev);
	bp->name = dev->name;
	bp->pdev = (struct udevice *)dev;

	bnxt_bring_pci(bp);

	ret = bnxt_bring_chip(bp);
	if (ret) {
		printf("*** error: bnxt_bring_chip failed! ***\n");
		return -ENODATA;
	}

	return 0;
}

static int bnxt_eth_remove(struct udevice *dev)
{
	struct bnxt *bp = dev_get_priv(dev);

	bnxt_down_chip(bp);
	bnxt_free_mem(bp);

	return 0;
}

static struct pci_device_id bnxt_nics[] = {
	{PCI_DEVICE(PCI_VENDOR_ID_BROADCOM, PCI_DEVICE_ID_NXT_57320)},
	{}
};

U_BOOT_DRIVER(eth_bnxt) = {
	.name                     = "eth_bnxt",
	.id                       = UCLASS_ETH,
	.of_match                 = bnxt_eth_ids,
	.bind                     = bnxt_eth_bind,
	.probe                    = bnxt_eth_probe,
	.remove                   = bnxt_eth_remove,
	.ops                      = &bnxt_eth_ops,
	.priv_auto                = sizeof(struct bnxt),
	.plat_auto                = sizeof(struct eth_pdata),
	.flags                    = DM_FLAG_ACTIVE_DMA,
};

U_BOOT_PCI_DEVICE(eth_bnxt, bnxt_nics);
