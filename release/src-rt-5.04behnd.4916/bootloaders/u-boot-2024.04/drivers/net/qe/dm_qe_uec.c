// SPDX-License-Identifier: GPL-2.0+
/*
 * QE UEC ethernet controller driver
 *
 * based on drivers/qe/uec.c from NXP
 *
 * Copyright (C) 2020 Heiko Schocher <hs@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <memalign.h>
#include <miiphy.h>
#include <asm/global_data.h>
#include <asm/io.h>

#include "dm_qe_uec.h"

#define QE_UEC_DRIVER_NAME	"ucc_geth"

/* Default UTBIPAR SMI address */
#ifndef CFG_UTBIPAR_INIT_TBIPA
#define CFG_UTBIPAR_INIT_TBIPA 0x1F
#endif

static int uec_mac_enable(struct uec_priv *uec, comm_dir_e mode)
{
	uec_t		*uec_regs;
	u32		maccfg1;

	uec_regs = uec->uec_regs;
	maccfg1 = in_be32(&uec_regs->maccfg1);

	if (mode & COMM_DIR_TX)	{
		maccfg1 |= MACCFG1_ENABLE_TX;
		out_be32(&uec_regs->maccfg1, maccfg1);
		uec->mac_tx_enabled = 1;
	}

	if (mode & COMM_DIR_RX)	{
		maccfg1 |= MACCFG1_ENABLE_RX;
		out_be32(&uec_regs->maccfg1, maccfg1);
		uec->mac_rx_enabled = 1;
	}

	return 0;
}

static int uec_mac_disable(struct uec_priv *uec, comm_dir_e mode)
{
	uec_t		*uec_regs;
	u32		maccfg1;

	uec_regs = uec->uec_regs;
	maccfg1 = in_be32(&uec_regs->maccfg1);

	if (mode & COMM_DIR_TX)	{
		maccfg1 &= ~MACCFG1_ENABLE_TX;
		out_be32(&uec_regs->maccfg1, maccfg1);
		uec->mac_tx_enabled = 0;
	}

	if (mode & COMM_DIR_RX)	{
		maccfg1 &= ~MACCFG1_ENABLE_RX;
		out_be32(&uec_regs->maccfg1, maccfg1);
		uec->mac_rx_enabled = 0;
	}

	return 0;
}

static int uec_restart_tx(struct uec_priv *uec)
{
	struct uec_inf	*ui = uec->uec_info;
	u32		cecr_subblock;

	cecr_subblock = ucc_fast_get_qe_cr_subblock(ui->uf_info.ucc_num);
	qe_issue_cmd(QE_RESTART_TX, cecr_subblock,
		     (u8)QE_CR_PROTOCOL_ETHERNET, 0);

	uec->grace_stopped_tx = 0;

	return 0;
}

static int uec_restart_rx(struct uec_priv *uec)
{
	struct uec_inf	*ui = uec->uec_info;
	u32		cecr_subblock;

	cecr_subblock = ucc_fast_get_qe_cr_subblock(ui->uf_info.ucc_num);
	qe_issue_cmd(QE_RESTART_RX, cecr_subblock,
		     (u8)QE_CR_PROTOCOL_ETHERNET, 0);

	uec->grace_stopped_rx = 0;

	return 0;
}

static int uec_open(struct uec_priv *uec, comm_dir_e mode)
{
	struct ucc_fast_priv	*uccf;

	uccf = uec->uccf;

	/* check if the UCC number is in range. */
	if (uec->uec_info->uf_info.ucc_num >= UCC_MAX_NUM) {
		printf("%s: ucc_num out of range.\n", __func__);
		return -EINVAL;
	}

	/* Enable MAC */
	uec_mac_enable(uec, mode);

	/* Enable UCC fast */
	ucc_fast_enable(uccf, mode);

	/* RISC microcode start */
	if ((mode & COMM_DIR_TX) && uec->grace_stopped_tx)
		uec_restart_tx(uec);

	if ((mode & COMM_DIR_RX) && uec->grace_stopped_rx)
		uec_restart_rx(uec);

	return 0;
}

static int uec_set_mac_if_mode(struct uec_priv *uec)
{
	struct uec_inf		*uec_info = uec->uec_info;
	phy_interface_t		enet_if_mode;
	uec_t			*uec_regs;
	u32			upsmr;
	u32			maccfg2;

	uec_regs = uec->uec_regs;
	enet_if_mode = uec_info->enet_interface_type;

	maccfg2 = in_be32(&uec_regs->maccfg2);
	maccfg2 &= ~MACCFG2_INTERFACE_MODE_MASK;

	upsmr = in_be32(&uec->uccf->uf_regs->upsmr);
	upsmr &= ~(UPSMR_RPM | UPSMR_TBIM | UPSMR_R10M | UPSMR_RMM);

	switch (uec_info->speed) {
	case SPEED_10:
		maccfg2 |= MACCFG2_INTERFACE_MODE_NIBBLE;
		switch (enet_if_mode) {
		case PHY_INTERFACE_MODE_MII:
			break;
		case PHY_INTERFACE_MODE_RGMII:
			upsmr |= (UPSMR_RPM | UPSMR_R10M);
			break;
		case PHY_INTERFACE_MODE_RMII:
			upsmr |= (UPSMR_R10M | UPSMR_RMM);
			break;
		default:
			return -EINVAL;
		}
		break;
	case SPEED_100:
		maccfg2 |= MACCFG2_INTERFACE_MODE_NIBBLE;
		switch (enet_if_mode) {
		case PHY_INTERFACE_MODE_MII:
			break;
		case PHY_INTERFACE_MODE_RGMII:
			upsmr |= UPSMR_RPM;
			break;
		case PHY_INTERFACE_MODE_RMII:
			upsmr |= UPSMR_RMM;
			break;
		default:
			return -EINVAL;
		}
		break;
	case SPEED_1000:
		maccfg2 |= MACCFG2_INTERFACE_MODE_BYTE;
		switch (enet_if_mode) {
		case PHY_INTERFACE_MODE_GMII:
			break;
		case PHY_INTERFACE_MODE_TBI:
			upsmr |= UPSMR_TBIM;
			break;
		case PHY_INTERFACE_MODE_RTBI:
			upsmr |= (UPSMR_RPM | UPSMR_TBIM);
			break;
		case PHY_INTERFACE_MODE_RGMII_RXID:
		case PHY_INTERFACE_MODE_RGMII_TXID:
		case PHY_INTERFACE_MODE_RGMII_ID:
		case PHY_INTERFACE_MODE_RGMII:
			upsmr |= UPSMR_RPM;
			break;
		case PHY_INTERFACE_MODE_SGMII:
			upsmr |= UPSMR_SGMM;
			break;
		default:
			return -EINVAL;
		}
		break;
	default:
		return -EINVAL;
	}

	out_be32(&uec_regs->maccfg2, maccfg2);
	out_be32(&uec->uccf->uf_regs->upsmr, upsmr);

	return 0;
}

static int qe_uec_start(struct udevice *dev)
{
	struct qe_uec_priv *priv = dev_get_priv(dev);
	struct uec_priv		*uec = priv->uec;
	struct phy_device	*phydev = priv->phydev;
	struct uec_inf		*uec_info = uec->uec_info;
	int			err;

	if (!phydev)
		return -ENODEV;

	/* Setup MAC interface mode */
	genphy_update_link(phydev);
	genphy_parse_link(phydev);
	uec_info->speed = phydev->speed;
	uec_set_mac_if_mode(uec);

	err = uec_open(uec, COMM_DIR_RX_AND_TX);
	if (err) {
		printf("%s: cannot enable UEC device\n", dev->name);
		return -EINVAL;
	}

	return (phydev->link ? 0 : -EINVAL);
}

static int qe_uec_send(struct udevice *dev, void *packet, int length)
{
	struct qe_uec_priv *priv = dev_get_priv(dev);
	struct uec_priv		*uec = priv->uec;
	struct ucc_fast_priv	*uccf = uec->uccf;
	struct buffer_descriptor	*bd;
	u16			status;
	int			i;
	int			result = 0;

	uccf = uec->uccf;
	bd = uec->tx_bd;

	/* Find an empty TxBD */
	for (i = 0; BD_STATUS(bd) & TX_BD_READY; i++) {
		if (i > 0x100000) {
			printf("%s: tx buffer not ready\n", dev->name);
			return result;
		}
	}

	/* Init TxBD */
	BD_DATA_SET(bd, packet);
	BD_LENGTH_SET(bd, length);
	status = BD_STATUS(bd);
	status &= BD_WRAP;
	status |= (TX_BD_READY | TX_BD_LAST);
	BD_STATUS_SET(bd, status);

	/* Tell UCC to transmit the buffer */
	ucc_fast_transmit_on_demand(uccf);

	/* Wait for buffer to be transmitted */
	for (i = 0; BD_STATUS(bd) & TX_BD_READY; i++) {
		if (i > 0x100000) {
			printf("%s: tx error\n", dev->name);
			return result;
		}
	}

	/* Ok, the buffer be transimitted */
	BD_ADVANCE(bd, status, uec->p_tx_bd_ring);
	uec->tx_bd = bd;
	result = 1;

	return result;
}

/*
 * Receive frame:
 * - wait for the next BD to get ready bit set
 * - clean up the descriptor
 * - move on and indicate to HW that the cleaned BD is available for Rx
 */
static int qe_uec_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct qe_uec_priv *priv = dev_get_priv(dev);
	struct uec_priv		*uec = priv->uec;
	struct buffer_descriptor	*bd;
	u16			status;
	u16			len = 0;
	u8			*data;

	*packetp = memalign(ARCH_DMA_MINALIGN, MAX_RXBUF_LEN);
	if (*packetp == 0) {
		printf("%s: error allocating packetp\n", __func__);
		return -ENOMEM;
	}

	bd = uec->rx_bd;
	status = BD_STATUS(bd);

	while (!(status & RX_BD_EMPTY)) {
		if (!(status & RX_BD_ERROR)) {
			data = BD_DATA(bd);
			len = BD_LENGTH(bd);
			memcpy(*packetp, (char *)data, len);
		} else {
			printf("%s: Rx error\n", dev->name);
		}
		status &= BD_CLEAN;
		BD_LENGTH_SET(bd, 0);
		BD_STATUS_SET(bd, status | RX_BD_EMPTY);
		BD_ADVANCE(bd, status, uec->p_rx_bd_ring);
		status = BD_STATUS(bd);
	}
	uec->rx_bd = bd;

	return len;
}

static int uec_graceful_stop_tx(struct uec_priv *uec)
{
	ucc_fast_t		*uf_regs;
	u32			cecr_subblock;
	u32			ucce;

	uf_regs = uec->uccf->uf_regs;

	/* Clear the grace stop event */
	out_be32(&uf_regs->ucce, UCCE_GRA);

	/* Issue host command */
	cecr_subblock =
		 ucc_fast_get_qe_cr_subblock(uec->uec_info->uf_info.ucc_num);
	qe_issue_cmd(QE_GRACEFUL_STOP_TX, cecr_subblock,
		     (u8)QE_CR_PROTOCOL_ETHERNET, 0);

	/* Wait for command to complete */
	do {
		ucce = in_be32(&uf_regs->ucce);
	} while (!(ucce & UCCE_GRA));

	uec->grace_stopped_tx = 1;

	return 0;
}

static int uec_graceful_stop_rx(struct uec_priv *uec)
{
	u32		cecr_subblock;
	u8		ack;

	if (!uec->p_rx_glbl_pram) {
		printf("%s: No init rx global parameter\n", __func__);
		return -EINVAL;
	}

	/* Clear acknowledge bit */
	ack = uec->p_rx_glbl_pram->rxgstpack;
	ack &= ~GRACEFUL_STOP_ACKNOWLEDGE_RX;
	uec->p_rx_glbl_pram->rxgstpack = ack;

	/* Keep issuing cmd and checking ack bit until it is asserted */
	do {
		/* Issue host command */
		cecr_subblock =
		ucc_fast_get_qe_cr_subblock(uec->uec_info->uf_info.ucc_num);
		qe_issue_cmd(QE_GRACEFUL_STOP_RX, cecr_subblock,
			     (u8)QE_CR_PROTOCOL_ETHERNET, 0);
		ack = uec->p_rx_glbl_pram->rxgstpack;
	} while (!(ack & GRACEFUL_STOP_ACKNOWLEDGE_RX));

	uec->grace_stopped_rx = 1;

	return 0;
}

static int uec_stop(struct uec_priv *uec, comm_dir_e mode)
{
	/* check if the UCC number is in range. */
	if (uec->uec_info->uf_info.ucc_num >= UCC_MAX_NUM) {
		printf("%s: ucc_num out of range.\n", __func__);
		return -EINVAL;
	}
	/* Stop any transmissions */
	if ((mode & COMM_DIR_TX) && !uec->grace_stopped_tx)
		uec_graceful_stop_tx(uec);

	/* Stop any receptions */
	if ((mode & COMM_DIR_RX) && !uec->grace_stopped_rx)
		uec_graceful_stop_rx(uec);

	/* Disable the UCC fast */
	ucc_fast_disable(uec->uccf, mode);

	/* Disable the MAC */
	uec_mac_disable(uec, mode);

	return 0;
}

static void qe_uec_stop(struct udevice *dev)
{
	struct qe_uec_priv *priv = dev_get_priv(dev);
	struct uec_priv		*uec = priv->uec;

	uec_stop(uec, COMM_DIR_RX_AND_TX);
}

static int qe_uec_set_hwaddr(struct udevice *dev)
{
	struct qe_uec_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct uec_priv *uec = priv->uec;
	uec_t *uec_regs = uec->uec_regs;
	uchar *mac = pdata->enetaddr;
	u32		mac_addr1;
	u32		mac_addr2;

	/*
	 * if a station address of 0x12345678ABCD, perform a write to
	 * MACSTNADDR1 of 0xCDAB7856,
	 * MACSTNADDR2 of 0x34120000
	 */

	mac_addr1 = (mac[5] << 24) | (mac[4] << 16) |
			(mac[3] << 8)  | (mac[2]);
	out_be32(&uec_regs->macstnaddr1, mac_addr1);

	mac_addr2 = ((mac[1] << 24) | (mac[0] << 16)) & 0xffff0000;
	out_be32(&uec_regs->macstnaddr2, mac_addr2);

	return 0;
}

static int qe_uec_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	if (packet)
		free(packet);

	return 0;
}

static const struct eth_ops qe_uec_eth_ops = {
	.start		= qe_uec_start,
	.send		= qe_uec_send,
	.recv		= qe_uec_recv,
	.free_pkt	= qe_uec_free_pkt,
	.stop		= qe_uec_stop,
	.write_hwaddr	= qe_uec_set_hwaddr,
};

static int uec_convert_threads_num(enum uec_num_of_threads threads_num,
				   int *threads_num_ret)
{
	int	num_threads_numerica;

	switch (threads_num) {
	case UEC_NUM_OF_THREADS_1:
		num_threads_numerica = 1;
		break;
	case UEC_NUM_OF_THREADS_2:
		num_threads_numerica = 2;
		break;
	case UEC_NUM_OF_THREADS_4:
		num_threads_numerica = 4;
		break;
	case UEC_NUM_OF_THREADS_6:
		num_threads_numerica = 6;
		break;
	case UEC_NUM_OF_THREADS_8:
		num_threads_numerica = 8;
		break;
	default:
		printf("%s: Bad number of threads value.",
		       __func__);
		return -EINVAL;
	}

	*threads_num_ret = num_threads_numerica;

	return 0;
}

static void uec_init_tx_parameter(struct uec_priv *uec, int num_threads_tx)
{
	struct uec_inf	*uec_info;
	u32		end_bd;
	u8		bmrx = 0;
	int		i;

	uec_info = uec->uec_info;

	/* Alloc global Tx parameter RAM page */
	uec->tx_glbl_pram_offset =
		qe_muram_alloc(sizeof(struct uec_tx_global_pram),
			       UEC_TX_GLOBAL_PRAM_ALIGNMENT);
	uec->p_tx_glbl_pram = (struct uec_tx_global_pram *)
				qe_muram_addr(uec->tx_glbl_pram_offset);

	/* Zero the global Tx prameter RAM */
	memset(uec->p_tx_glbl_pram, 0, sizeof(struct uec_tx_global_pram));

	/* Init global Tx parameter RAM */

	/* TEMODER, RMON statistics disable, one Tx queue */
	out_be16(&uec->p_tx_glbl_pram->temoder, TEMODER_INIT_VALUE);

	/* SQPTR */
	uec->send_q_mem_reg_offset =
		qe_muram_alloc(sizeof(struct uec_send_queue_qd),
			       UEC_SEND_QUEUE_QUEUE_DESCRIPTOR_ALIGNMENT);
	uec->p_send_q_mem_reg = (struct uec_send_queue_mem_region *)
				qe_muram_addr(uec->send_q_mem_reg_offset);
	out_be32(&uec->p_tx_glbl_pram->sqptr, uec->send_q_mem_reg_offset);

	/* Setup the table with TxBDs ring */
	end_bd = (u32)uec->p_tx_bd_ring + (uec_info->tx_bd_ring_len - 1)
					 * SIZEOFBD;
	out_be32(&uec->p_send_q_mem_reg->sqqd[0].bd_ring_base,
		 (u32)(uec->p_tx_bd_ring));
	out_be32(&uec->p_send_q_mem_reg->sqqd[0].last_bd_completed_address,
		 end_bd);

	/* Scheduler Base Pointer, we have only one Tx queue, no need it */
	out_be32(&uec->p_tx_glbl_pram->schedulerbasepointer, 0);

	/* TxRMON Base Pointer, TxRMON disable, we don't need it */
	out_be32(&uec->p_tx_glbl_pram->txrmonbaseptr, 0);

	/* TSTATE, global snooping, big endian, the CSB bus selected */
	bmrx = BMR_INIT_VALUE;
	out_be32(&uec->p_tx_glbl_pram->tstate, ((u32)(bmrx) << BMR_SHIFT));

	/* IPH_Offset */
	for (i = 0; i < MAX_IPH_OFFSET_ENTRY; i++)
		out_8(&uec->p_tx_glbl_pram->iphoffset[i], 0);

	/* VTAG table */
	for (i = 0; i < UEC_TX_VTAG_TABLE_ENTRY_MAX; i++)
		out_be32(&uec->p_tx_glbl_pram->vtagtable[i], 0);

	/* TQPTR */
	uec->thread_dat_tx_offset =
		qe_muram_alloc(num_threads_tx *
			       sizeof(struct uec_thread_data_tx) +
			       32 * (num_threads_tx == 1),
			       UEC_THREAD_DATA_ALIGNMENT);

	uec->p_thread_data_tx = (struct uec_thread_data_tx *)
				qe_muram_addr(uec->thread_dat_tx_offset);
	out_be32(&uec->p_tx_glbl_pram->tqptr, uec->thread_dat_tx_offset);
}

static void uec_init_rx_parameter(struct uec_priv *uec, int num_threads_rx)
{
	u8	bmrx = 0;
	int	i;
	struct uec_82xx_add_filtering_pram	*p_af_pram;

	/* Allocate global Rx parameter RAM page */
	uec->rx_glbl_pram_offset =
		qe_muram_alloc(sizeof(struct uec_rx_global_pram),
			       UEC_RX_GLOBAL_PRAM_ALIGNMENT);
	uec->p_rx_glbl_pram = (struct uec_rx_global_pram *)
				qe_muram_addr(uec->rx_glbl_pram_offset);

	/* Zero Global Rx parameter RAM */
	memset(uec->p_rx_glbl_pram, 0, sizeof(struct uec_rx_global_pram));

	/* Init global Rx parameter RAM */
	/*
	 * REMODER, Extended feature mode disable, VLAN disable,
	 * LossLess flow control disable, Receive firmware statisic disable,
	 * Extended address parsing mode disable, One Rx queues,
	 * Dynamic maximum/minimum frame length disable, IP checksum check
	 * disable, IP address alignment disable
	 */
	out_be32(&uec->p_rx_glbl_pram->remoder, REMODER_INIT_VALUE);

	/* RQPTR */
	uec->thread_dat_rx_offset =
		qe_muram_alloc(num_threads_rx *
			       sizeof(struct uec_thread_data_rx),
			       UEC_THREAD_DATA_ALIGNMENT);
	uec->p_thread_data_rx = (struct uec_thread_data_rx *)
				qe_muram_addr(uec->thread_dat_rx_offset);
	out_be32(&uec->p_rx_glbl_pram->rqptr, uec->thread_dat_rx_offset);

	/* Type_or_Len */
	out_be16(&uec->p_rx_glbl_pram->typeorlen, 3072);

	/* RxRMON base pointer, we don't need it */
	out_be32(&uec->p_rx_glbl_pram->rxrmonbaseptr, 0);

	/* IntCoalescingPTR, we don't need it, no interrupt */
	out_be32(&uec->p_rx_glbl_pram->intcoalescingptr, 0);

	/* RSTATE, global snooping, big endian, the CSB bus selected */
	bmrx = BMR_INIT_VALUE;
	out_8(&uec->p_rx_glbl_pram->rstate, bmrx);

	/* MRBLR */
	out_be16(&uec->p_rx_glbl_pram->mrblr, MAX_RXBUF_LEN);

	/* RBDQPTR */
	uec->rx_bd_qs_tbl_offset =
		qe_muram_alloc(sizeof(struct uec_rx_bd_queues_entry) +
			       sizeof(struct uec_rx_pref_bds),
			       UEC_RX_BD_QUEUES_ALIGNMENT);
	uec->p_rx_bd_qs_tbl = (struct uec_rx_bd_queues_entry *)
				qe_muram_addr(uec->rx_bd_qs_tbl_offset);

	/* Zero it */
	memset(uec->p_rx_bd_qs_tbl, 0, sizeof(struct uec_rx_bd_queues_entry) +
	       sizeof(struct uec_rx_pref_bds));
	out_be32(&uec->p_rx_glbl_pram->rbdqptr, uec->rx_bd_qs_tbl_offset);
	out_be32(&uec->p_rx_bd_qs_tbl->externalbdbaseptr,
		 (u32)uec->p_rx_bd_ring);

	/* MFLR */
	out_be16(&uec->p_rx_glbl_pram->mflr, MAX_FRAME_LEN);
	/* MINFLR */
	out_be16(&uec->p_rx_glbl_pram->minflr, MIN_FRAME_LEN);
	/* MAXD1 */
	out_be16(&uec->p_rx_glbl_pram->maxd1, MAX_DMA1_LEN);
	/* MAXD2 */
	out_be16(&uec->p_rx_glbl_pram->maxd2, MAX_DMA2_LEN);
	/* ECAM_PTR */
	out_be32(&uec->p_rx_glbl_pram->ecamptr, 0);
	/* L2QT */
	out_be32(&uec->p_rx_glbl_pram->l2qt, 0);
	/* L3QT */
	for (i = 0; i < 8; i++)
		out_be32(&uec->p_rx_glbl_pram->l3qt[i], 0);

	/* VLAN_TYPE */
	out_be16(&uec->p_rx_glbl_pram->vlantype, 0x8100);
	/* TCI */
	out_be16(&uec->p_rx_glbl_pram->vlantci, 0);

	/* Clear PQ2 style address filtering hash table */
	p_af_pram = (struct uec_82xx_add_filtering_pram *)
			uec->p_rx_glbl_pram->addressfiltering;

	p_af_pram->iaddr_h = 0;
	p_af_pram->iaddr_l = 0;
	p_af_pram->gaddr_h = 0;
	p_af_pram->gaddr_l = 0;
}

static int uec_issue_init_enet_rxtx_cmd(struct uec_priv *uec,
					int thread_tx, int thread_rx)
{
	struct uec_init_cmd_pram		*p_init_enet_param;
	u32				init_enet_param_offset;
	struct uec_inf			*uec_info;
	struct ucc_fast_inf			*uf_info;
	int				i;
	int				snum;
	u32				off;
	u32				entry_val;
	u32				command;
	u32				cecr_subblock;

	uec_info = uec->uec_info;
	uf_info = &uec_info->uf_info;

	/* Allocate init enet command parameter */
	uec->init_enet_param_offset =
		qe_muram_alloc(sizeof(struct uec_init_cmd_pram), 4);
	init_enet_param_offset = uec->init_enet_param_offset;
	uec->p_init_enet_param = (struct uec_init_cmd_pram *)
				qe_muram_addr(uec->init_enet_param_offset);

	/* Zero init enet command struct */
	memset((void *)uec->p_init_enet_param, 0,
	       sizeof(struct uec_init_cmd_pram));

	/* Init the command struct */
	p_init_enet_param = uec->p_init_enet_param;
	p_init_enet_param->resinit0 = ENET_INIT_PARAM_MAGIC_RES_INIT0;
	p_init_enet_param->resinit1 = ENET_INIT_PARAM_MAGIC_RES_INIT1;
	p_init_enet_param->resinit2 = ENET_INIT_PARAM_MAGIC_RES_INIT2;
	p_init_enet_param->resinit3 = ENET_INIT_PARAM_MAGIC_RES_INIT3;
	p_init_enet_param->resinit4 = ENET_INIT_PARAM_MAGIC_RES_INIT4;
	p_init_enet_param->largestexternallookupkeysize = 0;

	p_init_enet_param->rgftgfrxglobal |= ((u32)uec_info->num_threads_rx)
					 << ENET_INIT_PARAM_RGF_SHIFT;
	p_init_enet_param->rgftgfrxglobal |= ((u32)uec_info->num_threads_tx)
					 << ENET_INIT_PARAM_TGF_SHIFT;

	/* Init Rx global parameter pointer */
	p_init_enet_param->rgftgfrxglobal |= uec->rx_glbl_pram_offset |
						 (u32)uec_info->risc_rx;

	/* Init Rx threads */
	for (i = 0; i < (thread_rx + 1); i++) {
		snum = qe_get_snum();
		if (snum < 0) {
			printf("%s can not get snum\n", __func__);
			return -ENOMEM;
		}

		if (i == 0) {
			off = 0;
		} else {
			off = qe_muram_alloc(sizeof(struct uec_thread_rx_pram),
					     UEC_THREAD_RX_PRAM_ALIGNMENT);
		}

		entry_val = ((u32)snum << ENET_INIT_PARAM_SNUM_SHIFT) |
				 off | (u32)uec_info->risc_rx;
		p_init_enet_param->rxthread[i] = entry_val;
	}

	/* Init Tx global parameter pointer */
	p_init_enet_param->txglobal = uec->tx_glbl_pram_offset |
					 (u32)uec_info->risc_tx;

	/* Init Tx threads */
	for (i = 0; i < thread_tx; i++) {
		snum = qe_get_snum();
		if (snum  < 0)	{
			printf("%s can not get snum\n", __func__);
			return -ENOMEM;
		}

		off = qe_muram_alloc(sizeof(struct uec_thread_tx_pram),
				     UEC_THREAD_TX_PRAM_ALIGNMENT);

		entry_val = ((u32)snum << ENET_INIT_PARAM_SNUM_SHIFT) |
				 off | (u32)uec_info->risc_tx;
		p_init_enet_param->txthread[i] = entry_val;
	}

	__asm__ __volatile__("sync");

	/* Issue QE command */
	command = QE_INIT_TX_RX;
	cecr_subblock =	ucc_fast_get_qe_cr_subblock(uf_info->ucc_num);
	qe_issue_cmd(command, cecr_subblock, (u8)QE_CR_PROTOCOL_ETHERNET,
		     init_enet_param_offset);

	return 0;
}

static int uec_startup(struct udevice *dev)
{
	struct qe_uec_priv *priv = dev_get_priv(dev);
	struct uec_priv *uec = priv->uec;
	struct uec_inf			*uec_info;
	struct ucc_fast_inf			*uf_info;
	struct ucc_fast_priv		*uccf;
	ucc_fast_t			*uf_regs;
	uec_t				*uec_regs;
	int				num_threads_tx;
	int				num_threads_rx;
	u32				utbipar;
	u32				length;
	u32				align;
	struct buffer_descriptor	*bd;
	u8				*buf;
	int				i;

	uec_info = uec->uec_info;
	uf_info = &uec_info->uf_info;

	/* Check if Rx BD ring len is illegal */
	if (uec_info->rx_bd_ring_len < UEC_RX_BD_RING_SIZE_MIN ||
	    uec_info->rx_bd_ring_len % UEC_RX_BD_RING_SIZE_ALIGNMENT) {
		printf("%s: Rx BD ring len must be multiple of 4, and > 8.\n",
		       __func__);
		return -EINVAL;
	}

	/* Check if Tx BD ring len is illegal */
	if (uec_info->tx_bd_ring_len < UEC_TX_BD_RING_SIZE_MIN) {
		printf("%s: Tx BD ring length must not be smaller than 2.\n",
		       __func__);
		return -EINVAL;
	}

	/* Check if MRBLR is illegal */
	if (MAX_RXBUF_LEN == 0 || (MAX_RXBUF_LEN  % UEC_MRBLR_ALIGNMENT)) {
		printf("%s: max rx buffer length must be mutliple of 128.\n",
		       __func__);
		return -EINVAL;
	}

	/* Both Rx and Tx are stopped */
	uec->grace_stopped_rx = 1;
	uec->grace_stopped_tx = 1;

	/* Init UCC fast */
	if (ucc_fast_init(uf_info, &uccf)) {
		printf("%s: failed to init ucc fast\n", __func__);
		return -ENOMEM;
	}

	/* Save uccf */
	uec->uccf = uccf;

	/* Convert the Tx threads number */
	if (uec_convert_threads_num(uec_info->num_threads_tx,
				    &num_threads_tx))
		return -EINVAL;

	/* Convert the Rx threads number */
	if (uec_convert_threads_num(uec_info->num_threads_rx,
				    &num_threads_rx))
		return -EINVAL;

	uf_regs = uccf->uf_regs;

	/* UEC register is following UCC fast registers */
	uec_regs = (uec_t *)(&uf_regs->ucc_eth);

	/* Save the UEC register pointer to UEC private struct */
	uec->uec_regs = uec_regs;

	/* Init UPSMR, enable hardware statistics (UCC) */
	out_be32(&uec->uccf->uf_regs->upsmr, UPSMR_INIT_VALUE);

	/* Init MACCFG1, flow control disable, disable Tx and Rx */
	out_be32(&uec_regs->maccfg1, MACCFG1_INIT_VALUE);

	/* Init MACCFG2, length check, MAC PAD and CRC enable */
	out_be32(&uec_regs->maccfg2, MACCFG2_INIT_VALUE);

	/* Setup UTBIPAR */
	utbipar = in_be32(&uec_regs->utbipar);
	utbipar &= ~UTBIPAR_PHY_ADDRESS_MASK;

	/* Initialize UTBIPAR address to CFG_UTBIPAR_INIT_TBIPA for ALL UEC.
	 * This frees up the remaining SMI addresses for use.
	 */
	utbipar |= CFG_UTBIPAR_INIT_TBIPA << UTBIPAR_PHY_ADDRESS_SHIFT;
	out_be32(&uec_regs->utbipar, utbipar);

	/* Allocate Tx BDs */
	length = ((uec_info->tx_bd_ring_len * SIZEOFBD) /
		 UEC_TX_BD_RING_SIZE_MEMORY_ALIGNMENT) *
		 UEC_TX_BD_RING_SIZE_MEMORY_ALIGNMENT;
	if ((uec_info->tx_bd_ring_len * SIZEOFBD) %
	    UEC_TX_BD_RING_SIZE_MEMORY_ALIGNMENT)
		length += UEC_TX_BD_RING_SIZE_MEMORY_ALIGNMENT;

	align = UEC_TX_BD_RING_ALIGNMENT;
	uec->tx_bd_ring_offset = (u32)malloc((u32)(length + align));
	if (uec->tx_bd_ring_offset != 0)
		uec->p_tx_bd_ring = (u8 *)((uec->tx_bd_ring_offset + align)
					   & ~(align - 1));

	/* Zero all of Tx BDs */
	memset((void *)(uec->tx_bd_ring_offset), 0, length + align);

	/* Allocate Rx BDs */
	length = uec_info->rx_bd_ring_len * SIZEOFBD;
	align = UEC_RX_BD_RING_ALIGNMENT;
	uec->rx_bd_ring_offset = (u32)(malloc((u32)(length + align)));
	if (uec->rx_bd_ring_offset != 0)
		uec->p_rx_bd_ring = (u8 *)((uec->rx_bd_ring_offset + align)
					   & ~(align - 1));

	/* Zero all of Rx BDs */
	memset((void *)(uec->rx_bd_ring_offset), 0, length + align);

	/* Allocate Rx buffer */
	length = uec_info->rx_bd_ring_len * MAX_RXBUF_LEN;
	align = UEC_RX_DATA_BUF_ALIGNMENT;
	uec->rx_buf_offset = (u32)malloc(length + align);
	if (uec->rx_buf_offset != 0)
		uec->p_rx_buf = (u8 *)((uec->rx_buf_offset + align)
				       & ~(align - 1));

	/* Zero all of the Rx buffer */
	memset((void *)(uec->rx_buf_offset), 0, length + align);

	/* Init TxBD ring */
	bd = (struct buffer_descriptor *)uec->p_tx_bd_ring;
	uec->tx_bd = bd;

	for (i = 0; i < uec_info->tx_bd_ring_len; i++) {
		BD_DATA_CLEAR(bd);
		BD_STATUS_SET(bd, 0);
		BD_LENGTH_SET(bd, 0);
		bd++;
	}
	BD_STATUS_SET((--bd), TX_BD_WRAP);

	/* Init RxBD ring */
	bd = (struct buffer_descriptor *)uec->p_rx_bd_ring;
	uec->rx_bd = bd;
	buf = uec->p_rx_buf;
	for (i = 0; i < uec_info->rx_bd_ring_len; i++) {
		BD_DATA_SET(bd, buf);
		BD_LENGTH_SET(bd, 0);
		BD_STATUS_SET(bd, RX_BD_EMPTY);
		buf += MAX_RXBUF_LEN;
		bd++;
	}
	BD_STATUS_SET((--bd), RX_BD_WRAP | RX_BD_EMPTY);

	/* Init global Tx parameter RAM */
	uec_init_tx_parameter(uec, num_threads_tx);

	/* Init global Rx parameter RAM */
	uec_init_rx_parameter(uec, num_threads_rx);

	/* Init ethernet Tx and Rx parameter command */
	if (uec_issue_init_enet_rxtx_cmd(uec, num_threads_tx,
					 num_threads_rx)) {
		printf("%s issue init enet cmd failed\n", __func__);
		return -ENOMEM;
	}
	return 0;
}

/* Convert a string to a QE clock source enum
 *
 * This function takes a string, typically from a property in the device
 * tree, and returns the corresponding "enum qe_clock" value.
 */
enum qe_clock qe_clock_source(const char *source)
{
	unsigned int i;

	if (strcasecmp(source, "none") == 0)
		return QE_CLK_NONE;

	if (strncasecmp(source, "brg", 3) == 0) {
		i = dectoul(source + 3, NULL);
		if (i >= 1 && i <= 16)
			return (QE_BRG1 - 1) + i;
		else
			return QE_CLK_DUMMY;
	}

	if (strncasecmp(source, "clk", 3) == 0) {
		i = dectoul(source + 3, NULL);
		if (i >= 1 && i <= 24)
			return (QE_CLK1 - 1) + i;
		else
			return QE_CLK_DUMMY;
	}

	return QE_CLK_DUMMY;
}

static void qe_uec_set_eth_type(struct udevice *dev)
{
	struct qe_uec_priv *priv = dev_get_priv(dev);
	struct uec_priv		*uec = priv->uec;
	struct uec_inf *uec_info  = uec->uec_info;
	struct ucc_fast_inf *uf_info = &uec_info->uf_info;

	switch (uec_info->enet_interface_type) {
	case PHY_INTERFACE_MODE_GMII:
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
	case PHY_INTERFACE_MODE_TBI:
	case PHY_INTERFACE_MODE_RTBI:
	case PHY_INTERFACE_MODE_SGMII:
		uf_info->eth_type = GIGA_ETH;
		break;
	default:
		uf_info->eth_type = FAST_ETH;
		break;
	}
}

static int qe_uec_set_uec_info(struct udevice *dev)
{
	struct qe_uec_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct uec_priv *uec = priv->uec;
	struct uec_inf *uec_info;
	struct ucc_fast_inf *uf_info;
	const char *s;
	int ret;
	u32 val;

	uec_info = (struct uec_inf *)malloc(sizeof(struct uec_inf));
	if (!uec_info)
		return -ENOMEM;

	uf_info = &uec_info->uf_info;

	ret = dev_read_u32(dev, "cell-index", &val);
	if (ret) {
		ret = dev_read_u32(dev, "device-id", &val);
		if (ret) {
			pr_err("no cell-index nor device-id found!");
			goto out;
		}
	}

	uf_info->ucc_num = val - 1;
	if (uf_info->ucc_num < 0 || uf_info->ucc_num > 7) {
		ret = -ENODEV;
		goto out;
	}

	ret = dev_read_string_index(dev, "rx-clock-name", 0, &s);
	if (!ret) {
		uf_info->rx_clock = qe_clock_source(s);
		if (uf_info->rx_clock < QE_CLK_NONE ||
		    uf_info->rx_clock > QE_CLK24) {
			pr_err("invalid rx-clock-name property\n");
			ret = -EINVAL;
			goto out;
		}
	} else {
		ret = dev_read_u32(dev, "rx-clock", &val);
		if (ret) {
			/*
			 * If both rx-clock-name and rx-clock are missing,
			 * we want to tell people to use rx-clock-name.
			 */
			pr_err("missing rx-clock-name property\n");
			goto out;
		}
		if (val < QE_CLK_NONE || val > QE_CLK24) {
			pr_err("invalid rx-clock property\n");
			ret = -EINVAL;
			goto out;
		}
		uf_info->rx_clock = val;
	}

	ret = dev_read_string_index(dev, "tx-clock-name", 0, &s);
	if (!ret) {
		uf_info->tx_clock = qe_clock_source(s);
		if (uf_info->tx_clock < QE_CLK_NONE ||
		    uf_info->tx_clock > QE_CLK24) {
			pr_err("invalid tx-clock-name property\n");
			ret = -EINVAL;
			goto out;
		}
	} else {
		ret = dev_read_u32(dev, "tx-clock", &val);
		if (ret) {
			pr_err("missing tx-clock-name property\n");
			goto out;
		}
		if (val < QE_CLK_NONE || val > QE_CLK24) {
			pr_err("invalid tx-clock property\n");
			ret = -EINVAL;
			goto out;
		}
		uf_info->tx_clock = val;
	}

	uec_info->num_threads_tx = UEC_NUM_OF_THREADS_1;
	uec_info->num_threads_rx = UEC_NUM_OF_THREADS_1;
	uec_info->risc_tx = QE_RISC_ALLOCATION_RISC1_AND_RISC2;
	uec_info->risc_rx = QE_RISC_ALLOCATION_RISC1_AND_RISC2;
	uec_info->tx_bd_ring_len = 16;
	uec_info->rx_bd_ring_len = 16;
#if (MAX_QE_RISC == 4)
	uec_info->risc_tx = QE_RISC_ALLOCATION_FOUR_RISCS;
	uec_info->risc_rx = QE_RISC_ALLOCATION_FOUR_RISCS;
#endif

	uec_info->enet_interface_type = pdata->phy_interface;

	uec->uec_info = uec_info;
	qe_uec_set_eth_type(dev);

	return 0;
out:
	free(uec_info);
	return ret;
}

static int qe_uec_probe(struct udevice *dev)
{
	struct qe_uec_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct uec_priv		*uec;
	int ret;

	/* Allocate the UEC private struct */
	uec = (struct uec_priv *)malloc(sizeof(struct uec_priv));
	if (!uec)
		return -ENOMEM;

	memset(uec, 0, sizeof(struct uec_priv));
	priv->uec = uec;
	uec->uec_regs = (uec_t *)pdata->iobase;

	/* setup uec info struct */
	ret = qe_uec_set_uec_info(dev);
	if (ret) {
		free(uec);
		return ret;
	}

	ret = uec_startup(dev);
	if (ret) {
		free(uec->uec_info);
		free(uec);
		return ret;
	}

	priv->phydev = dm_eth_phy_connect(dev);
	return 0;
}

/*
 * Remove the driver from an interface:
 * - free up allocated memory
 */
static int qe_uec_remove(struct udevice *dev)
{
	struct qe_uec_priv *priv = dev_get_priv(dev);

	free(priv->uec);
	return 0;
}

static int qe_uec_of_to_plat(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);

	pdata->iobase = (phys_addr_t)devfdt_get_addr(dev);

	pdata->phy_interface = dev_read_phy_mode(dev);
	if (pdata->phy_interface == PHY_INTERFACE_MODE_NA)
		return -EINVAL;

	return 0;
}

static const struct udevice_id qe_uec_ids[] = {
	{ .compatible = QE_UEC_DRIVER_NAME },
	{ }
};

U_BOOT_DRIVER(eth_qe_uec) = {
	.name	= QE_UEC_DRIVER_NAME,
	.id	= UCLASS_ETH,
	.of_match = qe_uec_ids,
	.of_to_plat = qe_uec_of_to_plat,
	.probe	= qe_uec_probe,
	.remove = qe_uec_remove,
	.ops	= &qe_uec_eth_ops,
	.priv_auto	= sizeof(struct qe_uec_priv),
	.plat_auto	= sizeof(struct eth_pdata),
};
