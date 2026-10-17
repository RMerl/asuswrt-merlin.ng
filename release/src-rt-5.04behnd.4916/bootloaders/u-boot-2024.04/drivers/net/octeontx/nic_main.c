// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#include <config.h>
#include <net.h>
#include <netdev.h>
#include <malloc.h>
#include <miiphy.h>
#include <dm.h>
#include <misc.h>
#include <pci.h>
#include <pci_ids.h>
#include <asm/io.h>
#include <linux/delay.h>

#include "nic_reg.h"
#include "nic.h"
#include "q_struct.h"

unsigned long rounddown_pow_of_two(unsigned long n)
{
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n |= n >> 32;

	return(n + 1);
}

static void nic_config_cpi(struct nicpf *nic, struct cpi_cfg_msg *cfg);
static void nic_tx_channel_cfg(struct nicpf *nic, u8 vnic,
			       struct sq_cfg_msg *sq);
static int nic_update_hw_frs(struct nicpf *nic, int new_frs, int vf);
static int nic_rcv_queue_sw_sync(struct nicpf *nic);

/* Register read/write APIs */
static void nic_reg_write(struct nicpf *nic, u64 offset, u64 val)
{
	writeq(val, nic->reg_base + offset);
}

static u64 nic_reg_read(struct nicpf *nic, u64 offset)
{
	return readq(nic->reg_base + offset);
}

static u64 nic_get_mbx_addr(int vf)
{
	return NIC_PF_VF_0_127_MAILBOX_0_1 + (vf << NIC_VF_NUM_SHIFT);
}

static void nic_send_msg_to_vf(struct nicpf *nic, int vf, union nic_mbx *mbx)
{
	void __iomem *mbx_addr = (void *)(nic->reg_base + nic_get_mbx_addr(vf));
	u64 *msg = (u64 *)mbx;

	/* In first revision HW, mbox interrupt is triggerred
	 * when PF writes to MBOX(1), in next revisions when
	 * PF writes to MBOX(0)
	 */
	if (pass1_silicon(nic->rev_id, nic->hw->model_id)) {
		/* see the comment for nic_reg_write()/nic_reg_read()
		 * functions above
		 */
		writeq(msg[0], mbx_addr);
		writeq(msg[1], mbx_addr + 8);
	} else {
		writeq(msg[1], mbx_addr + 8);
		writeq(msg[0], mbx_addr);
	}
}

static void nic_mbx_send_ready(struct nicpf *nic, int vf)
{
	union nic_mbx mbx = {};
	int bgx_idx, lmac, timeout = 5, link = -1;
	const u8 *mac;

	mbx.nic_cfg.msg = NIC_MBOX_MSG_READY;
	mbx.nic_cfg.vf_id = vf;

	if (nic->flags & NIC_TNS_ENABLED)
		mbx.nic_cfg.tns_mode = NIC_TNS_MODE;
	else
		mbx.nic_cfg.tns_mode = NIC_TNS_BYPASS_MODE;

	if (vf < nic->num_vf_en) {
		bgx_idx = NIC_GET_BGX_FROM_VF_LMAC_MAP(nic->vf_lmac_map[vf]);
		lmac = NIC_GET_LMAC_FROM_VF_LMAC_MAP(nic->vf_lmac_map[vf]);

		mac = bgx_get_lmac_mac(nic->node, bgx_idx, lmac);
		if (mac)
			memcpy((u8 *)&mbx.nic_cfg.mac_addr, mac, 6);

		while (timeout-- && (link <= 0)) {
			link = bgx_poll_for_link(nic->node, bgx_idx, lmac);
			debug("Link status: %d\n", link);
			if (link <= 0)
				mdelay(2000);
		}
	}
#ifdef VNIC_MULTI_QSET_SUPPORT
	mbx.nic_cfg.sqs_mode = (vf >= nic->num_vf_en) ? true : false;
#endif
	mbx.nic_cfg.node_id = nic->node;

	mbx.nic_cfg.loopback_supported = vf < nic->num_vf_en;

	nic_send_msg_to_vf(nic, vf, &mbx);
}

/* ACKs VF's mailbox message
 * @vf: VF to which ACK to be sent
 */
static void nic_mbx_send_ack(struct nicpf *nic, int vf)
{
	union nic_mbx mbx = {};

	mbx.msg.msg = NIC_MBOX_MSG_ACK;
	nic_send_msg_to_vf(nic, vf, &mbx);
}

/* NACKs VF's mailbox message that PF is not able to
 * complete the action
 * @vf: VF to which ACK to be sent
 */
static void nic_mbx_send_nack(struct nicpf *nic, int vf)
{
	union nic_mbx mbx = {};

	mbx.msg.msg = NIC_MBOX_MSG_NACK;
	nic_send_msg_to_vf(nic, vf, &mbx);
}

static int nic_config_loopback(struct nicpf *nic, struct set_loopback *lbk)
{
	int bgx_idx, lmac_idx;

	if (lbk->vf_id > nic->num_vf_en)
		return -1;

	bgx_idx = NIC_GET_BGX_FROM_VF_LMAC_MAP(nic->vf_lmac_map[lbk->vf_id]);
	lmac_idx = NIC_GET_LMAC_FROM_VF_LMAC_MAP(nic->vf_lmac_map[lbk->vf_id]);

	bgx_lmac_internal_loopback(nic->node, bgx_idx, lmac_idx, lbk->enable);

	return 0;
}

/* Interrupt handler to handle mailbox messages from VFs */
void nic_handle_mbx_intr(struct nicpf *nic, int vf)
{
	union nic_mbx mbx = {};
	u64 *mbx_data;
	u64 mbx_addr;
	u64 reg_addr;
	u64 cfg;
	int bgx, lmac;
	int i;
	int ret = 0;

	nic->mbx_lock[vf] = true;

	mbx_addr = nic_get_mbx_addr(vf);
	mbx_data = (u64 *)&mbx;

	for (i = 0; i < NIC_PF_VF_MAILBOX_SIZE; i++) {
		*mbx_data = nic_reg_read(nic, mbx_addr);
		mbx_data++;
		mbx_addr += sizeof(u64);
	}

	debug("%s: Mailbox msg %d from VF%d\n", __func__, mbx.msg.msg, vf);
	switch (mbx.msg.msg) {
	case NIC_MBOX_MSG_READY:
		nic_mbx_send_ready(nic, vf);
		if (vf < nic->num_vf_en) {
			nic->link[vf] = 0;
			nic->duplex[vf] = 0;
			nic->speed[vf] = 0;
		}
		ret = 1;
		break;
	case NIC_MBOX_MSG_QS_CFG:
		reg_addr = NIC_PF_QSET_0_127_CFG |
			   (mbx.qs.num << NIC_QS_ID_SHIFT);
		cfg = mbx.qs.cfg;
#ifdef VNIC_MULTI_QSET_SUPPORT
		/* Check if its a secondary Qset */
		if (vf >= nic->num_vf_en) {
			cfg = cfg & (~0x7FULL);
			/* Assign this Qset to primary Qset's VF */
			cfg |= nic->pqs_vf[vf];
		}
#endif
		nic_reg_write(nic, reg_addr, cfg);
		break;
	case NIC_MBOX_MSG_RQ_CFG:
		reg_addr = NIC_PF_QSET_0_127_RQ_0_7_CFG |
			   (mbx.rq.qs_num << NIC_QS_ID_SHIFT) |
			   (mbx.rq.rq_num << NIC_Q_NUM_SHIFT);
		nic_reg_write(nic, reg_addr, mbx.rq.cfg);
		/* Enable CQE_RX2_S extension in CQE_RX descriptor.
		 * This gets appended by default on 81xx/83xx chips,
		 * for consistency enabling the same on 88xx pass2
		 * where this is introduced.
		 */
		if (pass2_silicon(nic->rev_id, nic->hw->model_id))
			nic_reg_write(nic, NIC_PF_RX_CFG, 0x01);
		break;
	case NIC_MBOX_MSG_RQ_BP_CFG:
		reg_addr = NIC_PF_QSET_0_127_RQ_0_7_BP_CFG |
			   (mbx.rq.qs_num << NIC_QS_ID_SHIFT) |
			   (mbx.rq.rq_num << NIC_Q_NUM_SHIFT);
		nic_reg_write(nic, reg_addr, mbx.rq.cfg);
		break;
	case NIC_MBOX_MSG_RQ_SW_SYNC:
		ret = nic_rcv_queue_sw_sync(nic);
		break;
	case NIC_MBOX_MSG_RQ_DROP_CFG:
		reg_addr = NIC_PF_QSET_0_127_RQ_0_7_DROP_CFG |
			   (mbx.rq.qs_num << NIC_QS_ID_SHIFT) |
			   (mbx.rq.rq_num << NIC_Q_NUM_SHIFT);
		nic_reg_write(nic, reg_addr, mbx.rq.cfg);
		break;
	case NIC_MBOX_MSG_SQ_CFG:
		reg_addr = NIC_PF_QSET_0_127_SQ_0_7_CFG |
			   (mbx.sq.qs_num << NIC_QS_ID_SHIFT) |
			   (mbx.sq.sq_num << NIC_Q_NUM_SHIFT);
		nic_reg_write(nic, reg_addr, mbx.sq.cfg);
		nic_tx_channel_cfg(nic, mbx.qs.num,
				   (struct sq_cfg_msg *)&mbx.sq);
		break;
	case NIC_MBOX_MSG_SET_MAC:
#ifdef VNIC_MULTI_QSET_SUPPORT
		if (vf >= nic->num_vf_en)
			break;
#endif
		lmac = mbx.mac.vf_id;
		bgx = NIC_GET_BGX_FROM_VF_LMAC_MAP(nic->vf_lmac_map[lmac]);
		lmac = NIC_GET_LMAC_FROM_VF_LMAC_MAP(nic->vf_lmac_map[lmac]);
		bgx_set_lmac_mac(nic->node, bgx, lmac, mbx.mac.mac_addr);
		break;
	case NIC_MBOX_MSG_SET_MAX_FRS:
		ret = nic_update_hw_frs(nic, mbx.frs.max_frs,
					mbx.frs.vf_id);
		break;
	case NIC_MBOX_MSG_CPI_CFG:
		nic_config_cpi(nic, &mbx.cpi_cfg);
		break;
#ifdef VNIC_RSS_SUPPORT
	case NIC_MBOX_MSG_RSS_SIZE:
		nic_send_rss_size(nic, vf);
		goto unlock;
	case NIC_MBOX_MSG_RSS_CFG:
	case NIC_MBOX_MSG_RSS_CFG_CONT:
		nic_config_rss(nic, &mbx.rss_cfg);
		break;
#endif
	case NIC_MBOX_MSG_CFG_DONE:
		/* Last message of VF config msg sequence */
		nic->vf_enabled[vf] = true;
		if (vf >= nic->lmac_cnt)
			goto unlock;

		bgx = NIC_GET_BGX_FROM_VF_LMAC_MAP(nic->vf_lmac_map[vf]);
		lmac = NIC_GET_LMAC_FROM_VF_LMAC_MAP(nic->vf_lmac_map[vf]);

		bgx_lmac_rx_tx_enable(nic->node, bgx, lmac, true);
		goto unlock;
	case NIC_MBOX_MSG_SHUTDOWN:
		/* First msg in VF teardown sequence */
		nic->vf_enabled[vf] = false;
#ifdef VNIC_MULTI_QSET_SUPPORT
		if (vf >= nic->num_vf_en)
			nic->sqs_used[vf - nic->num_vf_en] = false;
		nic->pqs_vf[vf] = 0;
#endif
		if (vf >= nic->lmac_cnt)
			break;

		bgx = NIC_GET_BGX_FROM_VF_LMAC_MAP(nic->vf_lmac_map[vf]);
		lmac = NIC_GET_LMAC_FROM_VF_LMAC_MAP(nic->vf_lmac_map[vf]);

		bgx_lmac_rx_tx_enable(nic->node, bgx, lmac, false);
		break;
#ifdef VNIC_MULTI_QSET_SUPPORT
	case NIC_MBOX_MSG_ALLOC_SQS:
		nic_alloc_sqs(nic, &mbx.sqs_alloc);
		goto unlock;
	case NIC_MBOX_MSG_NICVF_PTR:
		nic->nicvf[vf] = mbx.nicvf.nicvf;
		break;
	case NIC_MBOX_MSG_PNICVF_PTR:
		nic_send_pnicvf(nic, vf);
		goto unlock;
	case NIC_MBOX_MSG_SNICVF_PTR:
		nic_send_snicvf(nic, &mbx.nicvf);
		goto unlock;
#endif
	case NIC_MBOX_MSG_LOOPBACK:
		ret = nic_config_loopback(nic, &mbx.lbk);
		break;
	default:
		printf("Invalid msg from VF%d, msg 0x%x\n", vf, mbx.msg.msg);
		break;
	}

	if (!ret)
		nic_mbx_send_ack(nic, vf);
	else if (mbx.msg.msg != NIC_MBOX_MSG_READY)
		nic_mbx_send_nack(nic, vf);
unlock:
	nic->mbx_lock[vf] = false;
}

static int nic_rcv_queue_sw_sync(struct nicpf *nic)
{
	int timeout = 20;

	nic_reg_write(nic, NIC_PF_SW_SYNC_RX, 0x01);
	while (timeout) {
		if (nic_reg_read(nic, NIC_PF_SW_SYNC_RX_DONE) & 0x1)
			break;
		udelay(2000);
		timeout--;
	}
	nic_reg_write(nic, NIC_PF_SW_SYNC_RX, 0x00);
	if (!timeout) {
		printf("Recevie queue software sync failed");
		return 1;
	}
	return 0;
}

static int nic_update_hw_frs(struct nicpf *nic, int new_frs, int vf)
{
	u64 *pkind = (u64 *)&nic->pkind;

	if (new_frs > NIC_HW_MAX_FRS || new_frs < NIC_HW_MIN_FRS) {
		printf("Invalid MTU setting from VF%d rejected,", vf);
		printf(" should be between %d and %d\n", NIC_HW_MIN_FRS,
		       NIC_HW_MAX_FRS);
		return 1;
	}
	new_frs += ETH_HLEN;
	if (new_frs <= nic->pkind.maxlen)
		return 0;

	nic->pkind.maxlen = new_frs;

	nic_reg_write(nic, NIC_PF_PKIND_0_15_CFG, *pkind);
	return 0;
}

/* Set minimum transmit packet size */
static void nic_set_tx_pkt_pad(struct nicpf *nic, int size)
{
	int lmac;
	u64 lmac_cfg;
	struct hw_info *hw = nic->hw;
	int max_lmac = nic->hw->bgx_cnt * MAX_LMAC_PER_BGX;

	/* Max value that can be set is 60 */
	if (size > 52)
		size = 52;

	/* CN81XX has RGX configured as FAKE BGX, adjust mac_lmac accordingly */
	if (hw->chans_per_rgx)
		max_lmac = ((nic->hw->bgx_cnt - 1) * MAX_LMAC_PER_BGX) + 1;

	for (lmac = 0; lmac < max_lmac; lmac++) {
		lmac_cfg = nic_reg_read(nic, NIC_PF_LMAC_0_7_CFG | (lmac << 3));
		lmac_cfg &= ~(0xF << 2);
		lmac_cfg |= ((size / 4) << 2);
		nic_reg_write(nic, NIC_PF_LMAC_0_7_CFG | (lmac << 3), lmac_cfg);
	}
}

/* Function to check number of LMACs present and set VF to LMAC mapping.
 * Mapping will be used while initializing channels.
 */
static void nic_set_lmac_vf_mapping(struct nicpf *nic)
{
	int bgx, bgx_count, next_bgx_lmac = 0;
	int lmac, lmac_cnt = 0;
	u64 lmac_credit;

	nic->num_vf_en = 0;
	if (nic->flags & NIC_TNS_ENABLED) {
		nic->num_vf_en = DEFAULT_NUM_VF_ENABLED;
		return;
	}

	bgx_get_count(nic->node, &bgx_count);
	debug("bgx_count: %d\n", bgx_count);

	for (bgx = 0; bgx < nic->hw->bgx_cnt; bgx++) {
		if (!(bgx_count & (1 << bgx)))
			continue;
		nic->bgx_cnt++;
		lmac_cnt = bgx_get_lmac_count(nic->node, bgx);
		debug("lmac_cnt: %d for BGX%d\n", lmac_cnt, bgx);
		for (lmac = 0; lmac < lmac_cnt; lmac++)
			nic->vf_lmac_map[next_bgx_lmac++] =
						NIC_SET_VF_LMAC_MAP(bgx, lmac);
		nic->num_vf_en += lmac_cnt;

		/* Program LMAC credits */
		lmac_credit = (1ull << 1); /* chennel credit enable */
		lmac_credit |= (0x1ff << 2);
		lmac_credit |= (((((48 * 1024) / lmac_cnt) -
				NIC_HW_MAX_FRS) / 16) << 12);
		lmac = bgx * MAX_LMAC_PER_BGX;
		for (; lmac < lmac_cnt + (bgx * MAX_LMAC_PER_BGX); lmac++)
			nic_reg_write(nic, NIC_PF_LMAC_0_7_CREDIT + (lmac * 8),
				      lmac_credit);
	}
}

static void nic_get_hw_info(struct nicpf *nic)
{
	u16 sdevid;
	struct hw_info *hw = nic->hw;

	dm_pci_read_config16(nic->udev, PCI_SUBSYSTEM_ID, &sdevid);

	switch (sdevid) {
	case PCI_SUBSYS_DEVID_88XX_NIC_PF:
		hw->bgx_cnt = MAX_BGX_PER_NODE;
		hw->chans_per_lmac = 16;
		hw->chans_per_bgx = 128;
		hw->cpi_cnt = 2048;
		hw->rssi_cnt = 4096;
		hw->rss_ind_tbl_size = NIC_MAX_RSS_IDR_TBL_SIZE;
		hw->tl3_cnt = 256;
		hw->tl2_cnt = 64;
		hw->tl1_cnt = 2;
		hw->tl1_per_bgx = true;
		hw->model_id = 0x88;
		break;
	case PCI_SUBSYS_DEVID_81XX_NIC_PF:
		hw->bgx_cnt = MAX_BGX_PER_NODE;
		hw->chans_per_lmac = 8;
		hw->chans_per_bgx = 32;
		hw->chans_per_rgx = 8;
		hw->chans_per_lbk = 24;
		hw->cpi_cnt = 512;
		hw->rssi_cnt = 256;
		hw->rss_ind_tbl_size = 32; /* Max RSSI / Max interfaces */
		hw->tl3_cnt = 64;
		hw->tl2_cnt = 16;
		hw->tl1_cnt = 10;
		hw->tl1_per_bgx = false;
		hw->model_id = 0x81;
		break;
	case PCI_SUBSYS_DEVID_83XX_NIC_PF:
		hw->bgx_cnt = MAX_BGX_PER_NODE;
		hw->chans_per_lmac = 8;
		hw->chans_per_bgx = 32;
		hw->chans_per_lbk = 64;
		hw->cpi_cnt = 2048;
		hw->rssi_cnt = 1024;
		hw->rss_ind_tbl_size = 64; /* Max RSSI / Max interfaces */
		hw->tl3_cnt = 256;
		hw->tl2_cnt = 64;
		hw->tl1_cnt = 18;
		hw->tl1_per_bgx = false;
		hw->model_id = 0x83;
		break;
	}

	hw->tl4_cnt = MAX_QUEUES_PER_QSET * pci_sriov_get_totalvfs(nic->udev);
}

static void nic_init_hw(struct nicpf *nic)
{
	int i;
	u64 reg;
	u64 *pkind = (u64 *)&nic->pkind;

	/* Get HW capability info */
	nic_get_hw_info(nic);

	/* Enable NIC HW block */
	nic_reg_write(nic, NIC_PF_CFG, 0x3);

	/* Enable backpressure */
	nic_reg_write(nic, NIC_PF_BP_CFG, (1ULL << 6) | 0x03);
	nic_reg_write(nic, NIC_PF_INTF_0_1_BP_CFG, (1ULL << 63) | 0x08);
	nic_reg_write(nic, NIC_PF_INTF_0_1_BP_CFG + (1 << 8),
		      (1ULL << 63) | 0x09);

	for (i = 0; i < NIC_MAX_CHANS; i++)
		nic_reg_write(nic, NIC_PF_CHAN_0_255_TX_CFG | (i << 3), 1);

	if (nic->flags & NIC_TNS_ENABLED) {
		reg = NIC_TNS_MODE << 7;
		reg |= 0x06;
		nic_reg_write(nic, NIC_PF_INTF_0_1_SEND_CFG, reg);
		reg &= ~0xFull;
		reg |= 0x07;
		nic_reg_write(nic, NIC_PF_INTF_0_1_SEND_CFG | (1 << 8), reg);
	} else {
		/* Disable TNS mode on both interfaces */
		reg = NIC_TNS_BYPASS_MODE << 7;
		reg |= 0x08; /* Block identifier */
		nic_reg_write(nic, NIC_PF_INTF_0_1_SEND_CFG, reg);
		reg &= ~0xFull;
		reg |= 0x09;
		nic_reg_write(nic, NIC_PF_INTF_0_1_SEND_CFG | (1 << 8), reg);
	}

	/* PKIND configuration */
	nic->pkind.minlen = 0;
	nic->pkind.maxlen = NIC_HW_MAX_FRS + ETH_HLEN;
	nic->pkind.lenerr_en = 1;
	nic->pkind.rx_hdr = 0;
	nic->pkind.hdr_sl = 0;

	for (i = 0; i < NIC_MAX_PKIND; i++)
		nic_reg_write(nic, NIC_PF_PKIND_0_15_CFG | (i << 3), *pkind);

	nic_set_tx_pkt_pad(nic, NIC_HW_MIN_FRS);

	/* Timer config */
	nic_reg_write(nic, NIC_PF_INTR_TIMER_CFG, NICPF_CLK_PER_INT_TICK);
}

/* Channel parse index configuration */
static void nic_config_cpi(struct nicpf *nic, struct cpi_cfg_msg *cfg)
{
	struct hw_info *hw = nic->hw;
	u32 vnic, bgx, lmac, chan;
	u32 padd, cpi_count = 0;
	u64 cpi_base, cpi, rssi_base, rssi;
	u8  qset, rq_idx = 0;

	vnic = cfg->vf_id;
	bgx = NIC_GET_BGX_FROM_VF_LMAC_MAP(nic->vf_lmac_map[vnic]);
	lmac = NIC_GET_LMAC_FROM_VF_LMAC_MAP(nic->vf_lmac_map[vnic]);

	chan = (lmac * hw->chans_per_lmac) + (bgx * hw->chans_per_bgx);
	cpi_base = vnic * NIC_MAX_CPI_PER_LMAC;
	rssi_base = vnic * hw->rss_ind_tbl_size;

	/* Rx channel configuration */
	nic_reg_write(nic, NIC_PF_CHAN_0_255_RX_BP_CFG | (chan << 3),
		      (1ull << 63) | (vnic << 0));
	nic_reg_write(nic, NIC_PF_CHAN_0_255_RX_CFG | (chan << 3),
		      ((u64)cfg->cpi_alg << 62) | (cpi_base << 48));

	if (cfg->cpi_alg == CPI_ALG_NONE)
		cpi_count = 1;
	else if (cfg->cpi_alg == CPI_ALG_VLAN) /* 3 bits of PCP */
		cpi_count = 8;
	else if (cfg->cpi_alg == CPI_ALG_VLAN16) /* 3 bits PCP + DEI */
		cpi_count = 16;
	else if (cfg->cpi_alg == CPI_ALG_DIFF) /* 6bits DSCP */
		cpi_count = NIC_MAX_CPI_PER_LMAC;

	/* RSS Qset, Qidx mapping */
	qset = cfg->vf_id;
	rssi = rssi_base;
	for (; rssi < (rssi_base + cfg->rq_cnt); rssi++) {
		nic_reg_write(nic, NIC_PF_RSSI_0_4097_RQ | (rssi << 3),
			      (qset << 3) | rq_idx);
		rq_idx++;
	}

	rssi = 0;
	cpi = cpi_base;
	for (; cpi < (cpi_base + cpi_count); cpi++) {
		/* Determine port to channel adder */
		if (cfg->cpi_alg != CPI_ALG_DIFF)
			padd = cpi % cpi_count;
		else
			padd = cpi % 8; /* 3 bits CS out of 6bits DSCP */

		/* Leave RSS_SIZE as '0' to disable RSS */
		if (pass1_silicon(nic->rev_id, nic->hw->model_id)) {
			nic_reg_write(nic, NIC_PF_CPI_0_2047_CFG | (cpi << 3),
				      (vnic << 24) | (padd << 16) |
				      (rssi_base + rssi));
		} else {
			/* Set MPI_ALG to '0' to disable MCAM parsing */
			nic_reg_write(nic, NIC_PF_CPI_0_2047_CFG | (cpi << 3),
				      (padd << 16));
			/* MPI index is same as CPI if MPI_ALG is not enabled */
			nic_reg_write(nic, NIC_PF_MPI_0_2047_CFG | (cpi << 3),
				      (vnic << 24) | (rssi_base + rssi));
		}

		if ((rssi + 1) >= cfg->rq_cnt)
			continue;

		if (cfg->cpi_alg == CPI_ALG_VLAN)
			rssi++;
		else if (cfg->cpi_alg == CPI_ALG_VLAN16)
			rssi = ((cpi - cpi_base) & 0xe) >> 1;
		else if (cfg->cpi_alg == CPI_ALG_DIFF)
			rssi = ((cpi - cpi_base) & 0x38) >> 3;
	}
	nic->cpi_base[cfg->vf_id] = cpi_base;
	nic->rssi_base[cfg->vf_id] = rssi_base;
}

/* Transmit channel configuration (TL4 -> TL3 -> Chan)
 * VNIC0-SQ0 -> TL4(0)  -> TL4A(0) -> TL3[0] -> BGX0/LMAC0/Chan0
 * VNIC1-SQ0 -> TL4(8)  -> TL4A(2) -> TL3[2] -> BGX0/LMAC1/Chan0
 * VNIC2-SQ0 -> TL4(16) -> TL4A(4) -> TL3[4] -> BGX0/LMAC2/Chan0
 * VNIC3-SQ0 -> TL4(32) -> TL4A(6) -> TL3[6] -> BGX0/LMAC3/Chan0
 * VNIC4-SQ0 -> TL4(512)  -> TL4A(128) -> TL3[128] -> BGX1/LMAC0/Chan0
 * VNIC5-SQ0 -> TL4(520)  -> TL4A(130) -> TL3[130] -> BGX1/LMAC1/Chan0
 * VNIC6-SQ0 -> TL4(528)  -> TL4A(132) -> TL3[132] -> BGX1/LMAC2/Chan0
 * VNIC7-SQ0 -> TL4(536)  -> TL4A(134) -> TL3[134] -> BGX1/LMAC3/Chan0
 */
static void nic_tx_channel_cfg(struct nicpf *nic, u8 vnic,
			       struct sq_cfg_msg *sq)
{
	struct hw_info *hw = nic->hw;
	u32 bgx, lmac, chan;
	u32 tl2, tl3, tl4;
	u32 rr_quantum;
	u8 sq_idx = sq->sq_num;
	u8 pqs_vnic = vnic;
	int svf;
	u16 sdevid;

	dm_pci_read_config16(nic->udev, PCI_SUBSYSTEM_ID, &sdevid);

	bgx = NIC_GET_BGX_FROM_VF_LMAC_MAP(nic->vf_lmac_map[pqs_vnic]);
	lmac = NIC_GET_LMAC_FROM_VF_LMAC_MAP(nic->vf_lmac_map[pqs_vnic]);

	/* 24 bytes for FCS, IPG and preamble */
	rr_quantum = ((NIC_HW_MAX_FRS + 24) / 4);

	/* For 88xx 0-511 TL4 transmits via BGX0 and
	 * 512-1023 TL4s transmit via BGX1.
	 */
	if (hw->tl1_per_bgx) {
		tl4 = bgx * (hw->tl4_cnt / hw->bgx_cnt);
		if (!sq->sqs_mode) {
			tl4 += (lmac * MAX_QUEUES_PER_QSET);
		} else {
			for (svf = 0; svf < MAX_SQS_PER_VF_SINGLE_NODE; svf++) {
				if (nic->vf_sqs[pqs_vnic][svf] == vnic)
					break;
			}
			tl4 += (MAX_LMAC_PER_BGX * MAX_QUEUES_PER_QSET);
			tl4 += (lmac * MAX_QUEUES_PER_QSET *
				MAX_SQS_PER_VF_SINGLE_NODE);
			tl4 += (svf * MAX_QUEUES_PER_QSET);
		}
	} else {
		tl4 = (vnic * MAX_QUEUES_PER_QSET);
	}

	tl4 += sq_idx;

	tl3 = tl4 / (hw->tl4_cnt / hw->tl3_cnt);
	nic_reg_write(nic, NIC_PF_QSET_0_127_SQ_0_7_CFG2 |
		      ((u64)vnic << NIC_QS_ID_SHIFT) |
		      ((u32)sq_idx << NIC_Q_NUM_SHIFT), tl4);
	nic_reg_write(nic, NIC_PF_TL4_0_1023_CFG | (tl4 << 3),
		      ((u64)vnic << 27) | ((u32)sq_idx << 24) | rr_quantum);

	nic_reg_write(nic, NIC_PF_TL3_0_255_CFG | (tl3 << 3), rr_quantum);

	/* On 88xx 0-127 channels are for BGX0 and
	 * 127-255 channels for BGX1.
	 *
	 * On 81xx/83xx TL3_CHAN reg should be configured with channel
	 * within LMAC i.e 0-7 and not the actual channel number like on 88xx
	 */
	chan = (lmac * hw->chans_per_lmac) + (bgx * hw->chans_per_bgx);
	if (hw->tl1_per_bgx)
		nic_reg_write(nic, NIC_PF_TL3_0_255_CHAN | (tl3 << 3), chan);
	else
		nic_reg_write(nic, NIC_PF_TL3_0_255_CHAN | (tl3 << 3), 0);

	/* Enable backpressure on the channel */
	nic_reg_write(nic, NIC_PF_CHAN_0_255_TX_CFG | (chan << 3), 1);

	tl2 = tl3 >> 2;
	nic_reg_write(nic, NIC_PF_TL3A_0_63_CFG | (tl2 << 3), tl2);
	nic_reg_write(nic, NIC_PF_TL2_0_63_CFG | (tl2 << 3), rr_quantum);
	/* No priorities as of now */
	nic_reg_write(nic, NIC_PF_TL2_0_63_PRI | (tl2 << 3), 0x00);

	/* Unlike 88xx where TL2s 0-31 transmits to TL1 '0' and rest to TL1 '1'
	 * on 81xx/83xx TL2 needs to be configured to transmit to one of the
	 * possible LMACs.
	 *
	 * This register doesn't exist on 88xx.
	 */
	if (!hw->tl1_per_bgx)
		nic_reg_write(nic, NIC_PF_TL2_LMAC | (tl2 << 3),
			      lmac + (bgx * MAX_LMAC_PER_BGX));
}

int nic_initialize(struct udevice *dev)
{
	struct nicpf *nic = dev_get_priv(dev);

	nic->udev = dev;
	nic->hw = calloc(1, sizeof(struct hw_info));
	if (!nic->hw)
		return -ENOMEM;

	/* MAP PF's configuration registers */
	nic->reg_base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0, 0, 0, PCI_REGION_TYPE,
				       PCI_REGION_MEM);
	if (!nic->reg_base) {
		printf("Cannot map config register space, aborting\n");
		goto exit;
	}

	nic->node = node_id(nic->reg_base);
	dm_pci_read_config8(dev, PCI_REVISION_ID, &nic->rev_id);

	/* By default set NIC in TNS bypass mode */
	nic->flags &= ~NIC_TNS_ENABLED;

	/* Initialize hardware */
	nic_init_hw(nic);

	nic_set_lmac_vf_mapping(nic);

	/* Set RSS TBL size for each VF */
	nic->rss_ind_tbl_size = NIC_MAX_RSS_IDR_TBL_SIZE;

	nic->rss_ind_tbl_size = rounddown_pow_of_two(nic->rss_ind_tbl_size);

	return 0;
exit:
	free(nic->hw);
	return -ENODEV;
}

int octeontx_nic_probe(struct udevice *dev)
{
	int ret = 0;
	struct nicpf *nicpf = dev_get_priv(dev);

	nicpf->udev = dev;
	ret = nic_initialize(dev);
	if (ret < 0) {
		printf("couldn't initialize NIC PF\n");
		return ret;
	}

	ret = pci_sriov_init(dev, nicpf->num_vf_en);
	if (ret < 0)
		printf("enabling SRIOV failed for num VFs %d\n",
		       nicpf->num_vf_en);

	return ret;
}

U_BOOT_DRIVER(octeontx_nic) = {
	.name	= "octeontx_nic",
	.id	= UCLASS_MISC,
	.probe	= octeontx_nic_probe,
	.priv_auto	= sizeof(struct nicpf),
};

static struct pci_device_id octeontx_nic_supported[] = {
	{ PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_CAVIUM_NIC) },
	{}
};

U_BOOT_PCI_DEVICE(octeontx_nic, octeontx_nic_supported);
