// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#include <dm.h>
#include <malloc.h>
#include <misc.h>
#include <net.h>
#include <pci.h>
#include <pci_ids.h>
#include <phy.h>
#include <asm/io.h>
#include <linux/delay.h>

#include "nic_reg.h"
#include "nic.h"
#include "nicvf_queues.h"

/* Register read/write APIs */
void nicvf_reg_write(struct nicvf *nic, u64 offset, u64 val)
{
	writeq(val, nic->reg_base + offset);
}

u64 nicvf_reg_read(struct nicvf *nic, u64 offset)
{
	return readq(nic->reg_base + offset);
}

void nicvf_queue_reg_write(struct nicvf *nic, u64 offset,
			   u64 qidx, u64 val)
{
	void *addr = nic->reg_base + offset;

	writeq(val, (void *)(addr + (qidx << NIC_Q_NUM_SHIFT)));
}

u64 nicvf_queue_reg_read(struct nicvf *nic, u64 offset, u64 qidx)
{
	void *addr = nic->reg_base + offset;

	return readq((void *)(addr + (qidx << NIC_Q_NUM_SHIFT)));
}

static void  nicvf_handle_mbx_intr(struct nicvf *nic);

/* VF -> PF mailbox communication */
static void nicvf_write_to_mbx(struct nicvf *nic, union nic_mbx *mbx)
{
	u64 *msg = (u64 *)mbx;

	nicvf_reg_write(nic, NIC_VF_PF_MAILBOX_0_1 + 0, msg[0]);
	nicvf_reg_write(nic, NIC_VF_PF_MAILBOX_0_1 + 8, msg[1]);
}

int nicvf_send_msg_to_pf(struct nicvf *nic, union nic_mbx *mbx)
{
	int timeout = NIC_PF_VF_MBX_TIMEOUT;
	int sleep = 10;

	nic->pf_acked = false;
	nic->pf_nacked = false;

	nicvf_write_to_mbx(nic, mbx);

	nic_handle_mbx_intr(nic->nicpf, nic->vf_id);

	/* Wait for previous message to be acked, timeout 2sec */
	while (!nic->pf_acked) {
		if (nic->pf_nacked)
			return -1;
		mdelay(sleep);
		nicvf_handle_mbx_intr(nic);

		if (nic->pf_acked)
			break;
		timeout -= sleep;
		if (!timeout) {
			printf("PF didn't ack to mbox msg %d from VF%d\n",
			       (mbx->msg.msg & 0xFF), nic->vf_id);
			return -1;
		}
	}

	return 0;
}

/* Checks if VF is able to comminicate with PF
 * and also gets the VNIC number this VF is associated to.
 */
static int nicvf_check_pf_ready(struct nicvf *nic)
{
	union nic_mbx mbx = {};

	mbx.msg.msg = NIC_MBOX_MSG_READY;
	if (nicvf_send_msg_to_pf(nic, &mbx)) {
		printf("PF didn't respond to READY msg\n");
		return 0;
	}

	return 1;
}

static void  nicvf_handle_mbx_intr(struct nicvf *nic)
{
	union nic_mbx mbx = {};
	struct eth_pdata *pdata = dev_get_plat(nic->dev);
	u64 *mbx_data;
	u64 mbx_addr;
	int i;

	mbx_addr = NIC_VF_PF_MAILBOX_0_1;
	mbx_data = (u64 *)&mbx;

	for (i = 0; i < NIC_PF_VF_MAILBOX_SIZE; i++) {
		*mbx_data = nicvf_reg_read(nic, mbx_addr);
		mbx_data++;
		mbx_addr += sizeof(u64);
	}

	debug("Mbox message: msg: 0x%x\n", mbx.msg.msg);
	switch (mbx.msg.msg) {
	case NIC_MBOX_MSG_READY:
		nic->pf_acked = true;
		nic->vf_id = mbx.nic_cfg.vf_id & 0x7F;
		nic->tns_mode = mbx.nic_cfg.tns_mode & 0x7F;
		nic->node = mbx.nic_cfg.node_id;
		if (!nic->set_mac_pending)
			memcpy(pdata->enetaddr,
			       mbx.nic_cfg.mac_addr, 6);
		nic->loopback_supported = mbx.nic_cfg.loopback_supported;
		nic->link_up = false;
		nic->duplex = 0;
		nic->speed = 0;
		break;
	case NIC_MBOX_MSG_ACK:
		nic->pf_acked = true;
		break;
	case NIC_MBOX_MSG_NACK:
		nic->pf_nacked = true;
		break;
	case NIC_MBOX_MSG_BGX_LINK_CHANGE:
		nic->pf_acked = true;
		nic->link_up = mbx.link_status.link_up;
		nic->duplex = mbx.link_status.duplex;
		nic->speed = mbx.link_status.speed;
		if (nic->link_up) {
			printf("%s: Link is Up %d Mbps %s\n",
			       nic->dev->name, nic->speed,
			       nic->duplex == 1 ?
			       "Full duplex" : "Half duplex");
		} else {
			printf("%s: Link is Down\n", nic->dev->name);
		}
		break;
	default:
		printf("Invalid message from PF, msg 0x%x\n", mbx.msg.msg);
		break;
	}

	nicvf_clear_intr(nic, NICVF_INTR_MBOX, 0);
}

static int nicvf_hw_set_mac_addr(struct nicvf *nic, struct udevice *dev)
{
	union nic_mbx mbx = {};
	struct eth_pdata *pdata = dev_get_plat(dev);

	mbx.mac.msg = NIC_MBOX_MSG_SET_MAC;
	mbx.mac.vf_id = nic->vf_id;
	memcpy(mbx.mac.mac_addr, pdata->enetaddr, 6);

	return nicvf_send_msg_to_pf(nic, &mbx);
}

static void nicvf_config_cpi(struct nicvf *nic)
{
	union nic_mbx mbx = {};

	mbx.cpi_cfg.msg = NIC_MBOX_MSG_CPI_CFG;
	mbx.cpi_cfg.vf_id = nic->vf_id;
	mbx.cpi_cfg.cpi_alg = nic->cpi_alg;
	mbx.cpi_cfg.rq_cnt = nic->qs->rq_cnt;

	nicvf_send_msg_to_pf(nic, &mbx);
}

static int nicvf_init_resources(struct nicvf *nic)
{
	int err;

	nic->num_qs = 1;

	/* Enable Qset */
	nicvf_qset_config(nic, true);

	/* Initialize queues and HW for data transfer */
	err = nicvf_config_data_transfer(nic, true);

	if (err) {
		printf("Failed to alloc/config VF's QSet resources\n");
		return err;
	}
	return 0;
}

static void nicvf_snd_pkt_handler(struct nicvf *nic,
				  struct cmp_queue *cq,
				  void *cq_desc, int cqe_type)
{
	struct cqe_send_t *cqe_tx;
	struct snd_queue *sq;
	struct sq_hdr_subdesc *hdr;

	cqe_tx = (struct cqe_send_t *)cq_desc;
	sq = &nic->qs->sq[cqe_tx->sq_idx];

	hdr = (struct sq_hdr_subdesc *)GET_SQ_DESC(sq, cqe_tx->sqe_ptr);
	if (hdr->subdesc_type != SQ_DESC_TYPE_HEADER)
		return;

	nicvf_check_cqe_tx_errs(nic, cq, cq_desc);
	nicvf_put_sq_desc(sq, hdr->subdesc_cnt + 1);
}

static int nicvf_rcv_pkt_handler(struct nicvf *nic,
				 struct cmp_queue *cq, void *cq_desc,
				 void **ppkt, int cqe_type)
{
	void *pkt;

	size_t pkt_len;
	struct cqe_rx_t *cqe_rx = (struct cqe_rx_t *)cq_desc;
	int err = 0;

	/* Check for errors */
	err = nicvf_check_cqe_rx_errs(nic, cq, cq_desc);
	if (err && !cqe_rx->rb_cnt)
		return -1;

	pkt = nicvf_get_rcv_pkt(nic, cq_desc, &pkt_len);
	if (!pkt) {
		debug("Packet not received\n");
		return -1;
	}

	if (pkt)
		*ppkt = pkt;

	return pkt_len;
}

int nicvf_cq_handler(struct nicvf *nic, void **ppkt, int *pkt_len)
{
	int cq_qnum = 0;
	int processed_sq_cqe = 0;
	int processed_rq_cqe = 0;
	int processed_cqe = 0;

	unsigned long cqe_count, cqe_head;
	struct queue_set *qs = nic->qs;
	struct cmp_queue *cq = &qs->cq[cq_qnum];
	struct cqe_rx_t *cq_desc;

	/* Get num of valid CQ entries expect next one to be SQ completion */
	cqe_count = nicvf_queue_reg_read(nic, NIC_QSET_CQ_0_7_STATUS, cq_qnum);
	cqe_count &= 0xFFFF;
	if (!cqe_count)
		return 0;

	/* Get head of the valid CQ entries */
	cqe_head = nicvf_queue_reg_read(nic, NIC_QSET_CQ_0_7_HEAD, cq_qnum);
	cqe_head >>= 9;
	cqe_head &= 0xFFFF;

	if (cqe_count) {
		/* Get the CQ descriptor */
		cq_desc = (struct cqe_rx_t *)GET_CQ_DESC(cq, cqe_head);
		cqe_head++;
		cqe_head &= (cq->dmem.q_len - 1);
		/* Initiate prefetch for next descriptor */
		prefetch((struct cqe_rx_t *)GET_CQ_DESC(cq, cqe_head));

		switch (cq_desc->cqe_type) {
		case CQE_TYPE_RX:
			debug("%s: Got Rx CQE\n", nic->dev->name);
			*pkt_len = nicvf_rcv_pkt_handler(nic, cq, cq_desc,
							 ppkt, CQE_TYPE_RX);
			processed_rq_cqe++;
			break;
		case CQE_TYPE_SEND:
			debug("%s: Got Tx CQE\n", nic->dev->name);
			nicvf_snd_pkt_handler(nic, cq, cq_desc, CQE_TYPE_SEND);
			processed_sq_cqe++;
			break;
		default:
			debug("%s: Got CQ type %u\n", nic->dev->name,
			      cq_desc->cqe_type);
			break;
		}
		processed_cqe++;
	}

	/* Dequeue CQE */
	nicvf_queue_reg_write(nic, NIC_QSET_CQ_0_7_DOOR,
			      cq_qnum, processed_cqe);

	asm volatile ("dsb sy");

	return (processed_sq_cqe | processed_rq_cqe);
}

/* Qset error interrupt handler
 *
 * As of now only CQ errors are handled
 */
void nicvf_handle_qs_err(struct nicvf *nic)
{
	struct queue_set *qs = nic->qs;
	int qidx;
	u64 status;

	/* Check if it is CQ err */
	for (qidx = 0; qidx < qs->cq_cnt; qidx++) {
		status = nicvf_queue_reg_read(nic, NIC_QSET_CQ_0_7_STATUS,
					      qidx);
		if (!(status & CQ_ERR_MASK))
			continue;
		/* Process already queued CQEs and reconfig CQ */
		nicvf_sq_disable(nic, qidx);
		nicvf_cmp_queue_config(nic, qs, qidx, true);
		nicvf_sq_free_used_descs(nic->dev, &qs->sq[qidx], qidx);
		nicvf_sq_enable(nic, &qs->sq[qidx], qidx);
	}
}

static int nicvf_free_pkt(struct udevice *dev, uchar *pkt, int pkt_len)
{
	struct nicvf *nic = dev_get_priv(dev);

	if (pkt && pkt_len)
		free(pkt);
	nicvf_refill_rbdr(nic);
	return 0;
}

static int nicvf_xmit(struct udevice *dev, void *pkt, int pkt_len)
{
	struct nicvf *nic = dev_get_priv(dev);
	int ret = 0;
	int rcv_len = 0;
	unsigned int timeout = 5000;
	void *rpkt = NULL;

	if (!nicvf_sq_append_pkt(nic, pkt, pkt_len)) {
		printf("VF%d: TX ring full\n", nic->vf_id);
		return -1;
	}

	/* check and update CQ for pkt sent */
	while (!ret && timeout--) {
		ret = nicvf_cq_handler(nic, &rpkt, &rcv_len);
		if (!ret) {
			debug("%s: %d, Not sent\n", __func__, __LINE__);
			udelay(10);
		}
	}

	return 0;
}

static int nicvf_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct nicvf *nic = dev_get_priv(dev);
	void *pkt;
	int pkt_len = 0;
#ifdef DEBUG
	u8 *dpkt;
	int i, j;
#endif

	nicvf_cq_handler(nic, &pkt, &pkt_len);

	if (pkt_len) {
#ifdef DEBUG
		dpkt = pkt;
		printf("RX packet contents:\n");
		for (i = 0; i < 8; i++) {
			puts("\t");
			for (j = 0; j < 10; j++)
				printf("%02x ", dpkt[i * 10 + j]);
			puts("\n");
		}
#endif
		*packetp = pkt;
	}

	return pkt_len;
}

void nicvf_stop(struct udevice *dev)
{
	struct nicvf *nic = dev_get_priv(dev);

	if (!nic->open)
		return;

	/* Free resources */
	nicvf_config_data_transfer(nic, false);

	/* Disable HW Qset */
	nicvf_qset_config(nic, false);

	nic->open = false;
}

int nicvf_open(struct udevice *dev)
{
	int err;
	struct nicvf *nic = dev_get_priv(dev);

	nicvf_hw_set_mac_addr(nic, dev);

	/* Configure CPI alorithm */
	nic->cpi_alg = CPI_ALG_NONE;
	nicvf_config_cpi(nic);

	/* Initialize the queues */
	err = nicvf_init_resources(nic);
	if (err)
		return -1;

	if (!nicvf_check_pf_ready(nic))
		return -1;

	nic->open = true;

	/* Make sure queue initialization is written */
	asm volatile("dsb sy");

	return 0;
}

int nicvf_write_hwaddr(struct udevice *dev)
{
	unsigned char ethaddr[ARP_HLEN];
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct nicvf *nic = dev_get_priv(dev);

	/* If lower level firmware fails to set proper MAC
	 * u-boot framework updates MAC to random address.
	 * Use this hook to update mac address in environment.
	 */
	if (!eth_env_get_enetaddr_by_index("eth", dev_seq(dev), ethaddr)) {
		eth_env_set_enetaddr_by_index("eth", dev_seq(dev),
					      pdata->enetaddr);
		debug("%s: pMAC %pM\n", __func__, pdata->enetaddr);
	}
	eth_env_get_enetaddr_by_index("eth", dev_seq(dev), ethaddr);
	if (memcmp(ethaddr, pdata->enetaddr, ARP_HLEN)) {
		debug("%s: pMAC %pM\n", __func__, pdata->enetaddr);
		nicvf_hw_set_mac_addr(nic, dev);
	}
	return 0;
}

static void nicvf_probe_mdio_devices(void)
{
	struct udevice *pdev;
	int err;
	static int probed;

	if (probed)
		return;

	err = dm_pci_find_device(PCI_VENDOR_ID_CAVIUM,
				 PCI_DEVICE_ID_CAVIUM_SMI, 0,
				 &pdev);
	if (err)
		debug("%s couldn't find SMI device\n", __func__);
	probed = 1;
}

int nicvf_initialize(struct udevice *dev)
{
	struct nicvf *nicvf = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	int    ret = 0, bgx, lmac;
	char   name[16];
	unsigned char ethaddr[ARP_HLEN];
	struct udevice *pfdev;
	struct nicpf *pf;
	static int vfid;

	if (dm_pci_find_device(PCI_VENDOR_ID_CAVIUM,
			       PCI_DEVICE_ID_CAVIUM_NIC, 0, &pfdev)) {
		printf("%s NIC PF device not found..VF probe failed\n",
		       __func__);
		return -1;
	}
	pf = dev_get_priv(pfdev);
	nicvf->vf_id = vfid++;
	nicvf->dev = dev;
	nicvf->nicpf = pf;

	nicvf_probe_mdio_devices();

	/* Enable TSO support */
	nicvf->hw_tso = true;

	nicvf->reg_base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0, 0, 0, PCI_REGION_TYPE,
					 PCI_REGION_MEM);

	debug("nicvf->reg_base: %p\n", nicvf->reg_base);

	if (!nicvf->reg_base) {
		printf("Cannot map config register space, aborting\n");
		ret = -1;
		goto fail;
	}

	ret = nicvf_set_qset_resources(nicvf);
	if (ret)
		return -1;

	sprintf(name, "vnic%u", nicvf->vf_id);
	debug("%s name %s\n", __func__, name);
	device_set_name(dev, name);

	bgx = NIC_GET_BGX_FROM_VF_LMAC_MAP(pf->vf_lmac_map[nicvf->vf_id]);
	lmac = NIC_GET_LMAC_FROM_VF_LMAC_MAP(pf->vf_lmac_map[nicvf->vf_id]);
	debug("%s VF %d BGX %d LMAC %d\n", __func__, nicvf->vf_id, bgx, lmac);
	debug("%s PF %p pfdev %p VF %p vfdev %p vf->pdata %p\n",
	      __func__, nicvf->nicpf, nicvf->nicpf->udev, nicvf, nicvf->dev,
	      pdata);

	fdt_board_get_ethaddr(bgx, lmac, ethaddr);

	debug("%s bgx %d lmac %d ethaddr %pM\n", __func__, bgx, lmac, ethaddr);

	if (is_valid_ethaddr(ethaddr)) {
		memcpy(pdata->enetaddr, ethaddr, ARP_HLEN);
		eth_env_set_enetaddr_by_index("eth", dev_seq(dev), ethaddr);
	}
	debug("%s enetaddr %pM ethaddr %pM\n", __func__,
	      pdata->enetaddr, ethaddr);

fail:
	return ret;
}

int octeontx_vnic_probe(struct udevice *dev)
{
	return nicvf_initialize(dev);
}

static const struct eth_ops octeontx_vnic_ops = {
	.start = nicvf_open,
	.stop  = nicvf_stop,
	.send  = nicvf_xmit,
	.recv  = nicvf_recv,
	.free_pkt = nicvf_free_pkt,
	.write_hwaddr = nicvf_write_hwaddr,
};

U_BOOT_DRIVER(octeontx_vnic) = {
	.name	= "vnic",
	.id	= UCLASS_ETH,
	.probe	= octeontx_vnic_probe,
	.ops	= &octeontx_vnic_ops,
	.priv_auto	= sizeof(struct nicvf),
	.plat_auto	= sizeof(struct eth_pdata),
};

static struct pci_device_id octeontx_vnic_supported[] = {
	{ PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_CAVIUM_NICVF) },
	{ PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_CAVIUM_NICVF_1) },
	{}
};

U_BOOT_PCI_DEVICE(octeontx_vnic, octeontx_vnic_supported);
