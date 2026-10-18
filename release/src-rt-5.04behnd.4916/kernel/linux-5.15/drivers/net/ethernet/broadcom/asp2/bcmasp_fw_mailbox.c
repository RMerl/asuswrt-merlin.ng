// SPDX-License-Identifier: GPL-2.0
/*
 * Broadcom STB ASP 2.0 Driver FW Mailbox
 *
 * Copyright (c) 2023 Broadcom
 */
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/etherdevice.h>

#include "bcmasp.h"
#include "bcmasp_fw_mailbox.h"

/* ASP MSG Mailbox */
#define FWIMG_SIG_INDEX		0
#define FWIMG_NONE		0
#define FWIMG_CORE		1
#define FWIMG_STDBY		2
#define FWIMG_CORE_SIGNATURE	(0x41535000 | (FWIMG_CORE & 0xFF))
#define FWIMG_STDBY_SIGNATURE   (0x41535000 | (FWIMG_STDBY & 0xFF))

#define ASP_VER_INDEX		1
#define MSG_VER_INDEX		2

#define MAILBOX_WR_ADDR_INDEX	6
#define MAILBOX_RD_ADDR_INDEX	7

#define MSG_API_VER_CUR		1
#define MSG_SUCCESS		0

#define ASP_MAILBOX_DOORBELL	BIT(1)

/* 16k buffer */
#define PKTS_BUFFER_SIZE	PAGE_SIZE * 4

#ifdef DEBUG_ASP_MAILBOX
static void bcmasp_fw_debug_print_msg_fw2h(struct bcmasp_priv *priv,
					   struct bcmasp_fw_fw2h_msg *msg)
{
	struct device *dev = &priv->pdev->dev;
	unsigned int len, i;

	dev_info(dev, "FW to Host msg\n");
	len = sizeof(struct bcmasp_fw_h2fw_msg);
	for (i = 0; i < len / 4; i++)
		dev_info(dev, "word #%d: 0x%x\n", i, ((u32 *)msg)[i]);
}

static void bcmasp_fw_debug_print_msg_h2fw(struct bcmasp_priv *priv,
					   struct bcmasp_fw_h2fw_msg *msg)
{
	struct device *dev = &priv->pdev->dev;
	unsigned int len, i;

	dev_info(dev, "Host to FW msg\n");
	len = sizeof(struct bcmasp_fw_h2fw_msg);
	for (i = 0; i < len / 4; i++)
		dev_info(dev, "word #%d: 0x%x\n", i, ((u32 *)msg)[i]);

}
#endif

static bool bcmasp_fw_initialized(struct bcmasp_priv *priv)
{
	return priv->asp_fw.initialized;
}

static void bcmasp_fw_mailbox_inc_wr_msg(struct bcmasp_priv *priv)
{
	struct bcmasp_fw_priv *fw_priv = &priv->asp_fw;

	if (fw_priv->wr_mailbox_counter == 0xff)
		fw_priv->wr_mailbox_counter = 0;

	fw_priv->wr_mailbox_counter++;
}

static void bcmasp_fw_mailbox_wr_msg(struct bcmasp_priv *priv,
				     struct bcmasp_fw_h2fw_msg *msg)
{
	struct bcmasp_fw_priv *fw_priv = &priv->asp_fw;
	unsigned int len, i;

	len = sizeof(struct bcmasp_fw_h2fw_msg);
	for (i = 0; i < len / 4; i++) {
		msg_core_wl(priv, ((u32 *)msg)[i],
				ASP_MSG_INDEX(fw_priv->wr_mailbox_index + i));
	}
}

static void bcmasp_fw_mailbox_rd_msg(struct bcmasp_priv *priv, struct
				     bcmasp_fw_fw2h_msg *msg)
{
	struct bcmasp_fw_priv *fw_priv = &priv->asp_fw;
	unsigned int len, i;

	len = sizeof(struct bcmasp_fw_fw2h_msg);
	for (i = 0; i < len / 4; i++) {
		((u32 *)msg)[i] = msg_core_rl(priv,
				ASP_MSG_INDEX(fw_priv->rd_mailbox_index + i));
	}
}

static irqreturn_t bcmasp_fw_mailbox_isr(int irq, void *data)
{
	struct bcmasp_priv *priv = data;
	struct bcmasp_fw_priv *fw_priv;
	struct bcmasp_fw_fw2h_msg msg;
	struct device *dev;
	u32 status;

	fw_priv = &priv->asp_fw;
	dev = &priv->pdev->dev;

	memset(&msg, 0, sizeof(struct bcmasp_fw_fw2h_msg));
	status = cpu1_fw2h_core_rl(priv, ASP_CPU1_FW2H_STATUS);

	if (unlikely(status == 0)) {
		dev_warn(dev, "asp_wifi spurious interrupt\n");
		return IRQ_NONE;
	}

	/* Shouldn't get anything else at this point */
	if (!(status & ASP_MAILBOX_DOORBELL)) {
		dev_warn(dev, "Invalid mailbox ring: 0x%x\n", status);
		goto irq_handled;
	}

	/*
	 * We must consume the message before clearing or message may be
	 * overwritten
	 */
	bcmasp_fw_mailbox_rd_msg(priv, &msg);

#ifdef DEBUG_ASP_MAILBOX
	bcmasp_fw_debug_print_msg_fw2h(priv, &msg);
#endif
	switch (msg.msg_header.message_id)
	{
	case ASP_MSG_TYPE_H2FW_RES:
	case ASP_MSG_TYPE_H2FW_S2_ENTER:
	case ASP_MSG_TYPE_H2FW_S2_WAKEUP:
		dev_warn(dev, "Received fw message huh?: 0x%x\n",
			 msg.msg_header.message_id);
		break;
	case ASP_MSG_TYPE_FW2H_TRANSFER_DATA:
		if (completion_done(&fw_priv->transfer_msg_found)) {
			dev_warn(dev, "Spurious transfer msg?\n");
			break;
		}

		/* No valid packet, so BH only frees buffer */
		if (!msg.transfer_s2.valid_wakeup_pkt)
			fw_priv->buf_for_fw->only_free = true;

		/* Move cb to consume list and schedule BH work */
		list_add(&fw_priv->buf_for_fw->list,
			 &fw_priv->bufs_to_parse);
		fw_priv->buf_for_fw = NULL;

		complete(&fw_priv->transfer_msg_found);
		schedule_work(&priv->process_pkt_work);
		break;
	case ASP_MSG_TYPE_FW2H_DONE:
		if (completion_done(&fw_priv->out_msg_consumed)) {
			dev_warn(dev, "Spurious done msg?\n");
			break;
		}

		fw_priv->out_msg_response = msg.done.status;
		complete(&fw_priv->out_msg_consumed);
		break;
	default:
		dev_warn(dev, "Invalid message?\n");
	}

irq_handled:
	cpu1_fw2h_core_wl(priv, 0xffffffff, ASP_CPU1_FW2H_CLEAR);

	return IRQ_HANDLED;
}

static void bcmasp_fw_mailbox_ring_doorbell(struct bcmasp_priv *priv)
{
	cpu1_h2fw_core_wl(priv, ASP_MAILBOX_DOORBELL, ASP_CPU1_H2FW_SET);
}

static void bcmasp_fw_mailbox_msg_init_header(struct bcmasp_priv *priv,
					      struct bcmasp_fw_h2fw_msg *msg,
					      enum bcmasp_fw_msg_types
					      msg_type)
{
	struct bcmasp_fw_priv *fw_priv = &priv->asp_fw;

	msg->msg_header.message_id = msg_type;
	msg->msg_header.message_cnt = fw_priv->wr_mailbox_counter;
	msg->msg_header.message_ver = MSG_API_VER_CUR;
}

static void bcmasp_fw_mailbox_send_msg(struct bcmasp_priv *priv,
				       struct bcmasp_fw_h2fw_msg *msg)
{
	struct bcmasp_fw_priv *fw_priv = &priv->asp_fw;

#ifdef DEBUG_ASP_MAILBOX
	bcmasp_fw_debug_print_msg_h2fw(priv, msg);
#endif
	bcmasp_fw_mailbox_wr_msg(priv, msg);
	bcmasp_fw_mailbox_inc_wr_msg(priv);

	reinit_completion(&fw_priv->out_msg_consumed);

	bcmasp_fw_mailbox_ring_doorbell(priv);
}

static int bcmasp_fw_mailbox_wait_msg(struct bcmasp_priv *priv,
				      struct bcmasp_fw_h2fw_msg *msg)
{
	struct bcmasp_fw_priv *fw_priv = &priv->asp_fw;
	struct device *dev = &priv->pdev->dev;
	unsigned long timeo = msecs_to_jiffies(1000);

	if (!wait_for_completion_timeout(&fw_priv->out_msg_consumed, timeo)) {
		dev_warn(dev, "No response from ASP FW");
		return -EINVAL;
	}

	if (fw_priv->out_msg_response != MSG_SUCCESS) {
		dev_warn(dev, "MSG failed: 0x%x\n", fw_priv->out_msg_response);
		return -EINVAL;
	};

	return 0;
}

static int bcmasp_fw_mailbox_send_msg_and_wait(struct bcmasp_priv *priv,
					       struct bcmasp_fw_h2fw_msg *msg)
{
	bcmasp_fw_mailbox_send_msg(priv, msg);

	return bcmasp_fw_mailbox_wait_msg(priv, msg);
}

int bcmasp_fw_mailbox_enter_suspend_msg(struct bcmasp_priv *priv)
{
	struct bcmasp_fw_priv *fw_priv = &priv->asp_fw;
	struct device *dev = &priv->pdev->dev;
	struct bcmasp_fw_h2fw_msg s2_enter_msg;
	struct bcmasp_fw_buf_cb *cb;
	dma_addr_t dma;
	int ret;

	memset(&s2_enter_msg, 0, sizeof(struct bcmasp_fw_h2fw_msg));

	cb = kzalloc(sizeof(struct bcmasp_fw_buf_cb), GFP_KERNEL);
	if (!cb)
		return -ENOMEM;

	INIT_LIST_HEAD(&cb->list);

	cb->buf_order = get_order(PKTS_BUFFER_SIZE);
	cb->buffer_pg = alloc_pages(GFP_KERNEL, cb->buf_order);
	if (!cb->buffer_pg) {
		ret = -ENOMEM;
		goto free_cb;
	}

	dma = dma_map_page(dev, cb->buffer_pg, 0, PKTS_BUFFER_SIZE,
			   DMA_FROM_DEVICE);
	if (dma_mapping_error(dev, dma)) {
		ret = -EINVAL;
		goto free_pages;
	}
	cb->buf_addr_dma = dma;
	cb->buf_addr_cpu = page_to_virt(cb->buffer_pg);

	fw_priv->buf_for_fw = cb;

	/* Ready to create the msg */
	bcmasp_fw_mailbox_msg_init_header(priv, &s2_enter_msg,
					  ASP_MSG_TYPE_H2FW_S2_ENTER);

	s2_enter_msg.enter_s2.netfilt_id_mask = priv->netfilt_offload_mask;
	s2_enter_msg.enter_s2.netfilt_id_mask_direct = priv->netfilt_wake_mask;

	/* Top 8 bits at [31:24] */
	s2_enter_msg.enter_s2.pkt_dump_dma_hi =
		(u32)(cb->buf_addr_dma >> 8) & 0xff000000;
	s2_enter_msg.enter_s2.pkt_dump_dma_lo =
		(u32)(cb->buf_addr_dma);
	s2_enter_msg.enter_s2.pkt_dump_dma_size = PKTS_BUFFER_SIZE;

	/* Mail it! */
	ret = bcmasp_fw_mailbox_send_msg_and_wait(priv, &s2_enter_msg);
	if (ret)
		goto free_dma;

	/* Ready to go to sleep, set flag to control the resume sequence */
	fw_priv->fw_wake = false;
	reinit_completion(&fw_priv->transfer_msg_found);

	return 0;

free_dma:
	dma_unmap_page(dev, cb->buf_addr_dma, PKTS_BUFFER_SIZE,
		       DMA_FROM_DEVICE);
free_pages:
	__free_pages(cb->buffer_pg, cb->buf_order);
free_cb:
	kfree(cb);
	return ret;
}

void bcmasp_fw_mailbox_resume_wake_msg(struct bcmasp_priv *priv)
{
	struct bcmasp_fw_priv *fw_priv = &priv->asp_fw;
	struct device *dev = &priv->pdev->dev;
	struct bcmasp_fw_h2fw_msg s2_wake_msg;
	unsigned long timeo = msecs_to_jiffies(1000);

	/* Ordering is important below */

	/* Transfer msg already arrived, we are all good */
	if (completion_done(&fw_priv->transfer_msg_found))
		return;

	/* Sync wol irq to check for FW wake. This IRQ triggers the PMSM, so we
	 * are guaranteed it will be set if we were woken by fw */
	synchronize_irq(priv->wol_irq);

	/* Wake from FW, no need to send wake msg, go to wait */
	if (fw_priv->fw_wake)
		goto wait_for_transfer;

	/* Time for a race. We might receive a FW match during resume and have a
	 * delayed transfer msg. The FW should ignore the wake msg if this is
	 * the case */

	/* Let FW know we are awake */
	memset(&s2_wake_msg, 0, sizeof(struct bcmasp_fw_h2fw_msg));
	bcmasp_fw_mailbox_msg_init_header(priv, &s2_wake_msg,
					  ASP_MSG_TYPE_H2FW_S2_WAKEUP);
	/* Mail wake message */
	if (bcmasp_fw_mailbox_send_msg_and_wait(priv, &s2_wake_msg))
		return;

	/* We may receive the done msg or transfer msg out of order, either way
	 * we need both to continue, so we wait for completion twice */

wait_for_transfer:
	/* Technically don't need to wait here, we can move on to resume until
	 * the FW is done pushing packets to FW, but to keep things less racey,
	 * let's just wait unless the delay becomes noticably too long */
	if (!wait_for_completion_timeout(&fw_priv->transfer_msg_found, timeo))
		dev_warn(dev, "No transfer msg from FW?");
}

int bcmasp_fw_mailbox_open(struct bcmasp_priv *priv)
{
	struct bcmasp_fw_priv *fw_priv = &priv->asp_fw;
	struct device *dev = &priv->pdev->dev;
	u32 val, asp_sig, asp_plat_ver, asp_fw_ver;

	if (!bcmasp_fw_initialized(priv))
		return -EINVAL;

	/* Sanity check before init */
	asp_sig = msg_core_rl(priv, ASP_MSG_INDEX(FWIMG_SIG_INDEX));
	if (asp_sig != FWIMG_STDBY_SIGNATURE) {
		dev_warn(dev, "FW not loaded: 0x%x\n", asp_sig);
		return -EINVAL;
	}

	asp_plat_ver = msg_core_rl(priv, ASP_MSG_INDEX(ASP_VER_INDEX));
	asp_fw_ver = msg_core_rl(priv, ASP_MSG_INDEX(MSG_VER_INDEX));

	val = msg_core_rl(priv, ASP_MSG_INDEX(MAILBOX_WR_ADDR_INDEX));
	fw_priv->wr_mailbox_index = val / 4;

	val = msg_core_rl(priv, ASP_MSG_INDEX(MAILBOX_RD_ADDR_INDEX));
	fw_priv->rd_mailbox_index = val / 4;

	/* Clear anything sticky */
	wakeup_intr2_core_wl(priv, 0xffffffff, ASP_WAKEUP_INTR2_CLEAR);
	/* Mask wake on netfilt as this should now come from FW */
	wakeup_intr2_core_wl(priv, 0xc, ASP_WAKEUP_INTR2_MASK_SET);

	cpu1_fw2h_core_wl(priv, 0xffffffff, ASP_CPU1_FW2H_CLEAR);
	cpu1_fw2h_core_wl(priv, ASP_MAILBOX_DOORBELL, ASP_CPU1_FW2H_MASK_CLEAR);

	dev_info(dev, "ASP FW Loaded: Offloading to FW "
		 "(FW Sig: 0x%x, FW Platform Ver: 0x%x, FW Ver: 0x%x)\n",
		 asp_sig, asp_plat_ver, asp_fw_ver);

	return 0;
}

void bcmasp_fw_mailbox_close(struct bcmasp_priv *priv)
{
	/* Close mailbox */

	synchronize_irq(priv->wol_irq);

	wakeup_intr2_core_wl(priv, 0xffffffff, ASP_WAKEUP_INTR2_CLEAR);
	wakeup_intr2_core_wl(priv, 0xffffffff, ASP_WAKEUP_INTR2_MASK_CLEAR);
	cpu1_fw2h_core_wl(priv, ASP_MAILBOX_DOORBELL, ASP_CPU1_FW2H_MASK_SET);
}

/* First page is statistics information */
#define BUFFER_OFFSET			4096

/* EPKT header defines */
#define EPKT_HEADER_SIZE_W		4 /* Header is 4 words */
#define ASP_EPKT_HDR_MAGIC_W0		0x4252434d
#define ASP_EPKT_HDR_PORT_W1(x)		(((x) >> 28) & 0xf)
#define ASP_EPKT_HDR_FRM_SZ_W1(x)	((x) & 0xfff)
static void bcmasp_fw_mailbox_consume_pkts(struct work_struct *w)
{
	struct bcmasp_priv *priv = container_of(w, typeof(*priv),
						process_pkt_work);
	struct bcmasp_fw_priv *fw_priv = &priv->asp_fw;
	struct device *dev = &priv->pdev->dev;
	struct sk_buff *skb;
	struct net_device *ndev = NULL;
	struct bcmasp_fw_buf_cb *cb, *temp;
	u32 buf[EPKT_HEADER_SIZE_W];
	int i, ret, port, len;

	list_for_each_entry_safe(cb, temp, &fw_priv->bufs_to_parse, list) {
		if (cb->only_free)
			goto clean_cb;

		dma_sync_single_for_cpu(dev, cb->buf_addr_dma,
					EPKT_HEADER_SIZE_W * 4, DMA_FROM_DEVICE);

		/* Grab header for packet information */
		for (i = 0; i < EPKT_HEADER_SIZE_W; i++) {
			memcpy(&buf[i], ((u32 *)(cb->buf_addr_cpu)) +
			       (BUFFER_OFFSET / 4) + i, 4);
#ifdef DEBUG_ASP_MAILBOX
			dev_info(dev, "EPKT HEADER WORD %d: 0x%x\n", i, buf[i]);
#endif
		}

		if (buf[0] != ASP_EPKT_HDR_MAGIC_W0) {
			dev_warn(dev, "Invalid magic value for EPKT HDR\n");
			goto clean_cb;
		}

		port = ASP_EPKT_HDR_PORT_W1(buf[1]);
		for (i = 0; i < priv->intf_count; i++) {
			if (priv->intfs[i]->port == port) {
				ndev = priv->intfs[i]->ndev;
			}
		}
		if (ndev == NULL) {
			dev_warn(dev, "Packet found for port %d, but no "
				 "interface up to accept it.\n", port);
			goto clean_cb;
		}

		len = ASP_EPKT_HDR_FRM_SZ_W1(buf[1]);

		skb = netdev_alloc_skb(ndev, len);
		if (!skb) {
			dev_warn(dev, "SKB alloc failed\n");
			goto clean_cb;
		}

		skb_put(skb, len);

		dma_sync_single_for_cpu(dev, cb->buf_addr_dma,
					PKTS_BUFFER_SIZE, DMA_FROM_DEVICE);

		/* Skip statistics and EPKT HDR */
		memcpy(skb->data, ((u8 *)(cb->buf_addr_cpu)) +
		       BUFFER_OFFSET + (EPKT_HEADER_SIZE_W * 4), len);

#ifdef DEBUG_ASP_MAILBOX
		skb_dump(KERN_ERR, skb, true);
#endif

		skb->protocol = eth_type_trans(skb, ndev);

		ndev->stats.rx_packets++;
		ndev->stats.rx_bytes += len;

		ret = netif_rx(skb);
		if (ret != NET_RX_SUCCESS)
			dev_warn(dev, "Pkt not accepted?, 0x%x\n", ret);

clean_cb:
		ndev = NULL;
		dma_unmap_page(dev, cb->buf_addr_dma, PKTS_BUFFER_SIZE,
			       DMA_FROM_DEVICE);
		__free_pages(cb->buffer_pg, cb->buf_order);
		list_del(&cb->list);
		kfree(cb);
	}
}

int bcmasp_fw_mailbox_init(struct bcmasp_priv *priv)
{
	struct bcmasp_fw_priv *fw_priv = &priv->asp_fw;
	struct platform_device *pdev = priv->pdev;
	int ret = 0;

	fw_priv->fw_irq = platform_get_irq_byname_optional(pdev, "asp_1");
	if (fw_priv->fw_irq > 0) {
		ret = devm_request_irq(&pdev->dev, fw_priv->fw_irq,
				       bcmasp_fw_mailbox_isr, 0, pdev->name,
				       priv);
		if (ret) {
			dev_err(&pdev->dev, "failed to request fw irq: %d\n",
				ret);
			return ret;
		}
	} else {
		/* FW mailbox doesn't exist, exit without setting init flag */
		return ret;
	}

	init_completion(&fw_priv->out_msg_consumed);
	init_completion(&fw_priv->transfer_msg_found);
	INIT_WORK(&priv->process_pkt_work, bcmasp_fw_mailbox_consume_pkts);
	INIT_LIST_HEAD(&fw_priv->bufs_to_parse);

	/* Clear stale mailbox signatures */
	msg_core_wl(priv, 0x0, ASP_MSG_INDEX(FWIMG_SIG_INDEX));
	msg_core_wl(priv, 0x0, ASP_MSG_INDEX(ASP_VER_INDEX));
	msg_core_wl(priv, 0x0, ASP_MSG_INDEX(MSG_VER_INDEX));

	fw_priv->initialized = true;

	return ret;
}

void bcmasp_fw_mailbox_uninit(struct bcmasp_priv *priv)
{
	struct bcmasp_fw_priv *fw_priv = &priv->asp_fw;

	free_irq(fw_priv->fw_irq, priv);

	fw_priv->initialized = false;
}
