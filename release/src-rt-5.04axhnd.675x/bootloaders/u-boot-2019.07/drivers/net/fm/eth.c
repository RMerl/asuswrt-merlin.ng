// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2009-2012 Freescale Semiconductor, Inc.
 *	Dave Liu <daveliu@freescale.com>
 */
#include <common.h>
#include <asm/io.h>
#include <malloc.h>
#include <net.h>
#include <hwconfig.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <miiphy.h>
#include <phy.h>
#include <fsl_dtsec.h>
#include <fsl_tgec.h>
#include <fsl_memac.h>

#include "fm.h"

static struct eth_device *devlist[NUM_FM_PORTS];
static int num_controllers;

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII) && !defined(BITBANGMII)

#define TBIANA_SETTINGS (TBIANA_ASYMMETRIC_PAUSE | TBIANA_SYMMETRIC_PAUSE | \
			 TBIANA_FULL_DUPLEX)

#define TBIANA_SGMII_ACK 0x4001

#define TBICR_SETTINGS (TBICR_ANEG_ENABLE | TBICR_RESTART_ANEG | \
			TBICR_FULL_DUPLEX | TBICR_SPEED1_SET)

/* Configure the TBI for SGMII operation */
static void dtsec_configure_serdes(struct fm_eth *priv)
{
#ifdef CONFIG_SYS_FMAN_V3
	u32 value;
	struct mii_dev bus;
	bus.priv = priv->mac->phyregs;
	bool sgmii_2500 = (priv->enet_if ==
			PHY_INTERFACE_MODE_SGMII_2500) ? true : false;
	int i = 0;

qsgmii_loop:
	/* SGMII IF mode + AN enable only for 1G SGMII, not for 2.5G */
	if (sgmii_2500)
		value = PHY_SGMII_CR_PHY_RESET |
			PHY_SGMII_IF_SPEED_GIGABIT |
			PHY_SGMII_IF_MODE_SGMII;
	else
		value = PHY_SGMII_IF_MODE_SGMII | PHY_SGMII_IF_MODE_AN;

	memac_mdio_write(&bus, i, MDIO_DEVAD_NONE, 0x14, value);

	/* Dev ability according to SGMII specification */
	value = PHY_SGMII_DEV_ABILITY_SGMII;
	memac_mdio_write(&bus, i, MDIO_DEVAD_NONE, 0x4, value);

	if (sgmii_2500) {
		/* Adjust link timer for 2.5G SGMII,
		 * 1.6 ms in units of 3.2 ns:
		 * 1.6ms / 3.2ns = 5 * 10^5 = 0x7a120.
		 */
		memac_mdio_write(&bus, i, MDIO_DEVAD_NONE, 0x13, 0x0007);
		memac_mdio_write(&bus, i, MDIO_DEVAD_NONE, 0x12, 0xa120);
	} else {
		/* Adjust link timer for SGMII,
		 * 1.6 ms in units of 8 ns:
		 * 1.6ms / 8ns = 2 * 10^5 = 0x30d40.
		 */
		memac_mdio_write(&bus, i, MDIO_DEVAD_NONE, 0x13, 0x0003);
		memac_mdio_write(&bus, i, MDIO_DEVAD_NONE, 0x12, 0x0d40);
	}

	/* Restart AN */
	value = PHY_SGMII_CR_DEF_VAL | PHY_SGMII_CR_RESET_AN;
	memac_mdio_write(&bus, i, MDIO_DEVAD_NONE, 0, value);

	if ((priv->enet_if == PHY_INTERFACE_MODE_QSGMII) && (i < 3)) {
		i++;
		goto qsgmii_loop;
	}
#else
	struct dtsec *regs = priv->mac->base;
	struct tsec_mii_mng *phyregs = priv->mac->phyregs;

	/*
	 * Access TBI PHY registers at given TSEC register offset as
	 * opposed to the register offset used for external PHY accesses
	 */
	tsec_local_mdio_write(phyregs, in_be32(&regs->tbipa), 0, TBI_TBICON,
			TBICON_CLK_SELECT);
	tsec_local_mdio_write(phyregs, in_be32(&regs->tbipa), 0, TBI_ANA,
			TBIANA_SGMII_ACK);
	tsec_local_mdio_write(phyregs, in_be32(&regs->tbipa), 0,
			TBI_CR, TBICR_SETTINGS);
#endif
}

static void dtsec_init_phy(struct eth_device *dev)
{
	struct fm_eth *fm_eth = dev->priv;
#ifndef CONFIG_SYS_FMAN_V3
	struct dtsec *regs = (struct dtsec *)CONFIG_SYS_FSL_FM1_DTSEC1_ADDR;

	/* Assign a Physical address to the TBI */
	out_be32(&regs->tbipa, CONFIG_SYS_TBIPA_VALUE);
#endif

	if (fm_eth->enet_if == PHY_INTERFACE_MODE_SGMII ||
	    fm_eth->enet_if == PHY_INTERFACE_MODE_QSGMII ||
	    fm_eth->enet_if == PHY_INTERFACE_MODE_SGMII_2500)
		dtsec_configure_serdes(fm_eth);
}

#ifdef CONFIG_PHYLIB
static int tgec_is_fibre(struct eth_device *dev)
{
	struct fm_eth *fm = dev->priv;
	char phyopt[20];

	sprintf(phyopt, "fsl_fm%d_xaui_phy", fm->fm_index + 1);

	return hwconfig_arg_cmp(phyopt, "xfi");
}
#endif
#endif

static u16 muram_readw(u16 *addr)
{
	ulong base = (ulong)addr & ~0x3UL;
	u32 val32 = in_be32((void *)base);
	int byte_pos;
	u16 ret;

	byte_pos = (ulong)addr & 0x3UL;
	if (byte_pos)
		ret = (u16)(val32 & 0x0000ffff);
	else
		ret = (u16)((val32 & 0xffff0000) >> 16);

	return ret;
}

static void muram_writew(u16 *addr, u16 val)
{
	ulong base = (ulong)addr & ~0x3UL;
	u32 org32 = in_be32((void *)base);
	u32 val32;
	int byte_pos;

	byte_pos = (ulong)addr & 0x3UL;
	if (byte_pos)
		val32 = (org32 & 0xffff0000) | val;
	else
		val32 = (org32 & 0x0000ffff) | ((u32)val << 16);

	out_be32((void *)base, val32);
}

static void bmi_rx_port_disable(struct fm_bmi_rx_port *rx_port)
{
	int timeout = 1000000;

	clrbits_be32(&rx_port->fmbm_rcfg, FMBM_RCFG_EN);

	/* wait until the rx port is not busy */
	while ((in_be32(&rx_port->fmbm_rst) & FMBM_RST_BSY) && timeout--)
		;
}

static void bmi_rx_port_init(struct fm_bmi_rx_port *rx_port)
{
	/* set BMI to independent mode, Rx port disable */
	out_be32(&rx_port->fmbm_rcfg, FMBM_RCFG_IM);
	/* clear FOF in IM case */
	out_be32(&rx_port->fmbm_rim, 0);
	/* Rx frame next engine -RISC */
	out_be32(&rx_port->fmbm_rfne, NIA_ENG_RISC | NIA_RISC_AC_IM_RX);
	/* Rx command attribute - no order, MR[3] = 1 */
	clrbits_be32(&rx_port->fmbm_rfca, FMBM_RFCA_ORDER | FMBM_RFCA_MR_MASK);
	setbits_be32(&rx_port->fmbm_rfca, FMBM_RFCA_MR(4));
	/* enable Rx statistic counters */
	out_be32(&rx_port->fmbm_rstc, FMBM_RSTC_EN);
	/* disable Rx performance counters */
	out_be32(&rx_port->fmbm_rpc, 0);
}

static void bmi_tx_port_disable(struct fm_bmi_tx_port *tx_port)
{
	int timeout = 1000000;

	clrbits_be32(&tx_port->fmbm_tcfg, FMBM_TCFG_EN);

	/* wait until the tx port is not busy */
	while ((in_be32(&tx_port->fmbm_tst) & FMBM_TST_BSY) && timeout--)
		;
}

static void bmi_tx_port_init(struct fm_bmi_tx_port *tx_port)
{
	/* set BMI to independent mode, Tx port disable */
	out_be32(&tx_port->fmbm_tcfg, FMBM_TCFG_IM);
	/* Tx frame next engine -RISC */
	out_be32(&tx_port->fmbm_tfne, NIA_ENG_RISC | NIA_RISC_AC_IM_TX);
	out_be32(&tx_port->fmbm_tfene, NIA_ENG_RISC | NIA_RISC_AC_IM_TX);
	/* Tx command attribute - no order, MR[3] = 1 */
	clrbits_be32(&tx_port->fmbm_tfca, FMBM_TFCA_ORDER | FMBM_TFCA_MR_MASK);
	setbits_be32(&tx_port->fmbm_tfca, FMBM_TFCA_MR(4));
	/* enable Tx statistic counters */
	out_be32(&tx_port->fmbm_tstc, FMBM_TSTC_EN);
	/* disable Tx performance counters */
	out_be32(&tx_port->fmbm_tpc, 0);
}

static int fm_eth_rx_port_parameter_init(struct fm_eth *fm_eth)
{
	struct fm_port_global_pram *pram;
	u32 pram_page_offset;
	void *rx_bd_ring_base;
	void *rx_buf_pool;
	u32 bd_ring_base_lo, bd_ring_base_hi;
	u32 buf_lo, buf_hi;
	struct fm_port_bd *rxbd;
	struct fm_port_qd *rxqd;
	struct fm_bmi_rx_port *bmi_rx_port = fm_eth->rx_port;
	int i;

	/* alloc global parameter ram at MURAM */
	pram = (struct fm_port_global_pram *)fm_muram_alloc(fm_eth->fm_index,
		FM_PRAM_SIZE, FM_PRAM_ALIGN);
	if (!pram) {
		printf("%s: No muram for Rx global parameter\n", __func__);
		return -ENOMEM;
	}

	fm_eth->rx_pram = pram;

	/* parameter page offset to MURAM */
	pram_page_offset = (void *)pram - fm_muram_base(fm_eth->fm_index);

	/* enable global mode- snooping data buffers and BDs */
	out_be32(&pram->mode, PRAM_MODE_GLOBAL);

	/* init the Rx queue descriptor pionter */
	out_be32(&pram->rxqd_ptr, pram_page_offset + 0x20);

	/* set the max receive buffer length, power of 2 */
	muram_writew(&pram->mrblr, MAX_RXBUF_LOG2);

	/* alloc Rx buffer descriptors from main memory */
	rx_bd_ring_base = malloc(sizeof(struct fm_port_bd)
			* RX_BD_RING_SIZE);
	if (!rx_bd_ring_base)
		return -ENOMEM;

	memset(rx_bd_ring_base, 0, sizeof(struct fm_port_bd)
			* RX_BD_RING_SIZE);

	/* alloc Rx buffer from main memory */
	rx_buf_pool = malloc(MAX_RXBUF_LEN * RX_BD_RING_SIZE);
	if (!rx_buf_pool)
		return -ENOMEM;

	memset(rx_buf_pool, 0, MAX_RXBUF_LEN * RX_BD_RING_SIZE);
	debug("%s: rx_buf_pool = %p\n", __func__, rx_buf_pool);

	/* save them to fm_eth */
	fm_eth->rx_bd_ring = rx_bd_ring_base;
	fm_eth->cur_rxbd = rx_bd_ring_base;
	fm_eth->rx_buf = rx_buf_pool;

	/* init Rx BDs ring */
	rxbd = (struct fm_port_bd *)rx_bd_ring_base;
	for (i = 0; i < RX_BD_RING_SIZE; i++) {
		muram_writew(&rxbd->status, RxBD_EMPTY);
		muram_writew(&rxbd->len, 0);
		buf_hi = upper_32_bits(virt_to_phys(rx_buf_pool +
					i * MAX_RXBUF_LEN));
		buf_lo = lower_32_bits(virt_to_phys(rx_buf_pool +
					i * MAX_RXBUF_LEN));
		muram_writew(&rxbd->buf_ptr_hi, (u16)buf_hi);
		out_be32(&rxbd->buf_ptr_lo, buf_lo);
		rxbd++;
	}

	/* set the Rx queue descriptor */
	rxqd = &pram->rxqd;
	muram_writew(&rxqd->gen, 0);
	bd_ring_base_hi = upper_32_bits(virt_to_phys(rx_bd_ring_base));
	bd_ring_base_lo = lower_32_bits(virt_to_phys(rx_bd_ring_base));
	muram_writew(&rxqd->bd_ring_base_hi, (u16)bd_ring_base_hi);
	out_be32(&rxqd->bd_ring_base_lo, bd_ring_base_lo);
	muram_writew(&rxqd->bd_ring_size, sizeof(struct fm_port_bd)
			* RX_BD_RING_SIZE);
	muram_writew(&rxqd->offset_in, 0);
	muram_writew(&rxqd->offset_out, 0);

	/* set IM parameter ram pointer to Rx Frame Queue ID */
	out_be32(&bmi_rx_port->fmbm_rfqid, pram_page_offset);

	return 0;
}

static int fm_eth_tx_port_parameter_init(struct fm_eth *fm_eth)
{
	struct fm_port_global_pram *pram;
	u32 pram_page_offset;
	void *tx_bd_ring_base;
	u32 bd_ring_base_lo, bd_ring_base_hi;
	struct fm_port_bd *txbd;
	struct fm_port_qd *txqd;
	struct fm_bmi_tx_port *bmi_tx_port = fm_eth->tx_port;
	int i;

	/* alloc global parameter ram at MURAM */
	pram = (struct fm_port_global_pram *)fm_muram_alloc(fm_eth->fm_index,
		FM_PRAM_SIZE, FM_PRAM_ALIGN);
	if (!pram) {
		printf("%s: No muram for Tx global parameter\n", __func__);
		return -ENOMEM;
	}
	fm_eth->tx_pram = pram;

	/* parameter page offset to MURAM */
	pram_page_offset = (void *)pram - fm_muram_base(fm_eth->fm_index);

	/* enable global mode- snooping data buffers and BDs */
	out_be32(&pram->mode, PRAM_MODE_GLOBAL);

	/* init the Tx queue descriptor pionter */
	out_be32(&pram->txqd_ptr, pram_page_offset + 0x40);

	/* alloc Tx buffer descriptors from main memory */
	tx_bd_ring_base = malloc(sizeof(struct fm_port_bd)
			* TX_BD_RING_SIZE);
	if (!tx_bd_ring_base)
		return -ENOMEM;

	memset(tx_bd_ring_base, 0, sizeof(struct fm_port_bd)
			* TX_BD_RING_SIZE);
	/* save it to fm_eth */
	fm_eth->tx_bd_ring = tx_bd_ring_base;
	fm_eth->cur_txbd = tx_bd_ring_base;

	/* init Tx BDs ring */
	txbd = (struct fm_port_bd *)tx_bd_ring_base;
	for (i = 0; i < TX_BD_RING_SIZE; i++) {
		muram_writew(&txbd->status, TxBD_LAST);
		muram_writew(&txbd->len, 0);
		muram_writew(&txbd->buf_ptr_hi, 0);
		out_be32(&txbd->buf_ptr_lo, 0);
		txbd++;
	}

	/* set the Tx queue decriptor */
	txqd = &pram->txqd;
	bd_ring_base_hi = upper_32_bits(virt_to_phys(tx_bd_ring_base));
	bd_ring_base_lo = lower_32_bits(virt_to_phys(tx_bd_ring_base));
	muram_writew(&txqd->bd_ring_base_hi, (u16)bd_ring_base_hi);
	out_be32(&txqd->bd_ring_base_lo, bd_ring_base_lo);
	muram_writew(&txqd->bd_ring_size, sizeof(struct fm_port_bd)
			* TX_BD_RING_SIZE);
	muram_writew(&txqd->offset_in, 0);
	muram_writew(&txqd->offset_out, 0);

	/* set IM parameter ram pointer to Tx Confirmation Frame Queue ID */
	out_be32(&bmi_tx_port->fmbm_tcfqid, pram_page_offset);

	return 0;
}

static int fm_eth_init(struct fm_eth *fm_eth)
{
	int ret;

	ret = fm_eth_rx_port_parameter_init(fm_eth);
	if (ret)
		return ret;

	ret = fm_eth_tx_port_parameter_init(fm_eth);
	if (ret)
		return ret;

	return 0;
}

static int fm_eth_startup(struct fm_eth *fm_eth)
{
	struct fsl_enet_mac *mac;
	int ret;

	mac = fm_eth->mac;

	/* Rx/TxBDs, Rx/TxQDs, Rx buff and parameter ram init */
	ret = fm_eth_init(fm_eth);
	if (ret)
		return ret;
	/* setup the MAC controller */
	mac->init_mac(mac);

	/* For some reason we need to set SPEED_100 */
	if (((fm_eth->enet_if == PHY_INTERFACE_MODE_SGMII) ||
	     (fm_eth->enet_if == PHY_INTERFACE_MODE_SGMII_2500) ||
	     (fm_eth->enet_if == PHY_INTERFACE_MODE_QSGMII)) &&
	      mac->set_if_mode)
		mac->set_if_mode(mac, fm_eth->enet_if, SPEED_100);

	/* init bmi rx port, IM mode and disable */
	bmi_rx_port_init(fm_eth->rx_port);
	/* init bmi tx port, IM mode and disable */
	bmi_tx_port_init(fm_eth->tx_port);

	return 0;
}

static void fmc_tx_port_graceful_stop_enable(struct fm_eth *fm_eth)
{
	struct fm_port_global_pram *pram;

	pram = fm_eth->tx_pram;
	/* graceful stop transmission of frames */
	setbits_be32(&pram->mode, PRAM_MODE_GRACEFUL_STOP);
	sync();
}

static void fmc_tx_port_graceful_stop_disable(struct fm_eth *fm_eth)
{
	struct fm_port_global_pram *pram;

	pram = fm_eth->tx_pram;
	/* re-enable transmission of frames */
	clrbits_be32(&pram->mode, PRAM_MODE_GRACEFUL_STOP);
	sync();
}

static int fm_eth_open(struct eth_device *dev, bd_t *bd)
{
	struct fm_eth *fm_eth;
	struct fsl_enet_mac *mac;
#ifdef CONFIG_PHYLIB
	int ret;
#endif

	fm_eth = (struct fm_eth *)dev->priv;
	mac = fm_eth->mac;

	/* setup the MAC address */
	if (dev->enetaddr[0] & 0x01) {
		printf("%s: MacAddress is multcast address\n",	__func__);
		return 1;
	}
	mac->set_mac_addr(mac, dev->enetaddr);

	/* enable bmi Rx port */
	setbits_be32(&fm_eth->rx_port->fmbm_rcfg, FMBM_RCFG_EN);
	/* enable MAC rx/tx port */
	mac->enable_mac(mac);
	/* enable bmi Tx port */
	setbits_be32(&fm_eth->tx_port->fmbm_tcfg, FMBM_TCFG_EN);
	/* re-enable transmission of frame */
	fmc_tx_port_graceful_stop_disable(fm_eth);

#ifdef CONFIG_PHYLIB
	if (fm_eth->phydev) {
		ret = phy_startup(fm_eth->phydev);
		if (ret) {
			printf("%s: Could not initialize\n",
			       fm_eth->phydev->dev->name);
			return ret;
		}
	} else {
		return 0;
	}
#else
	fm_eth->phydev->speed = SPEED_1000;
	fm_eth->phydev->link = 1;
	fm_eth->phydev->duplex = DUPLEX_FULL;
#endif

	/* set the MAC-PHY mode */
	mac->set_if_mode(mac, fm_eth->enet_if, fm_eth->phydev->speed);

	if (!fm_eth->phydev->link)
		printf("%s: No link.\n", fm_eth->phydev->dev->name);

	return fm_eth->phydev->link ? 0 : -1;
}

static void fm_eth_halt(struct eth_device *dev)
{
	struct fm_eth *fm_eth;
	struct fsl_enet_mac *mac;

	fm_eth = (struct fm_eth *)dev->priv;
	mac = fm_eth->mac;

	/* graceful stop the transmission of frames */
	fmc_tx_port_graceful_stop_enable(fm_eth);
	/* disable bmi Tx port */
	bmi_tx_port_disable(fm_eth->tx_port);
	/* disable MAC rx/tx port */
	mac->disable_mac(mac);
	/* disable bmi Rx port */
	bmi_rx_port_disable(fm_eth->rx_port);

#ifdef CONFIG_PHYLIB
	if (fm_eth->phydev)
		phy_shutdown(fm_eth->phydev);
#endif
}

static int fm_eth_send(struct eth_device *dev, void *buf, int len)
{
	struct fm_eth *fm_eth;
	struct fm_port_global_pram *pram;
	struct fm_port_bd *txbd, *txbd_base;
	u16 offset_in;
	int i;

	fm_eth = (struct fm_eth *)dev->priv;
	pram = fm_eth->tx_pram;
	txbd = fm_eth->cur_txbd;

	/* find one empty TxBD */
	for (i = 0; muram_readw(&txbd->status) & TxBD_READY; i++) {
		udelay(100);
		if (i > 0x1000) {
			printf("%s: Tx buffer not ready, txbd->status = 0x%x\n",
			       dev->name, muram_readw(&txbd->status));
			return 0;
		}
	}
	/* setup TxBD */
	muram_writew(&txbd->buf_ptr_hi, (u16)upper_32_bits(virt_to_phys(buf)));
	out_be32(&txbd->buf_ptr_lo, lower_32_bits(virt_to_phys(buf)));
	muram_writew(&txbd->len, len);
	sync();
	muram_writew(&txbd->status, TxBD_READY | TxBD_LAST);
	sync();

	/* update TxQD, let RISC to send the packet */
	offset_in = muram_readw(&pram->txqd.offset_in);
	offset_in += sizeof(struct fm_port_bd);
	if (offset_in >= muram_readw(&pram->txqd.bd_ring_size))
		offset_in = 0;
	muram_writew(&pram->txqd.offset_in, offset_in);
	sync();

	/* wait for buffer to be transmitted */
	for (i = 0; muram_readw(&txbd->status) & TxBD_READY; i++) {
		udelay(100);
		if (i > 0x10000) {
			printf("%s: Tx error, txbd->status = 0x%x\n",
			       dev->name, muram_readw(&txbd->status));
			return 0;
		}
	}

	/* advance the TxBD */
	txbd++;
	txbd_base = (struct fm_port_bd *)fm_eth->tx_bd_ring;
	if (txbd >= (txbd_base + TX_BD_RING_SIZE))
		txbd = txbd_base;
	/* update current txbd */
	fm_eth->cur_txbd = (void *)txbd;

	return 1;
}

static int fm_eth_recv(struct eth_device *dev)
{
	struct fm_eth *fm_eth;
	struct fm_port_global_pram *pram;
	struct fm_port_bd *rxbd, *rxbd_base;
	u16 status, len;
	u32 buf_lo, buf_hi;
	u8 *data;
	u16 offset_out;
	int ret = 1;

	fm_eth = (struct fm_eth *)dev->priv;
	pram = fm_eth->rx_pram;
	rxbd = fm_eth->cur_rxbd;
	status = muram_readw(&rxbd->status);

	while (!(status & RxBD_EMPTY)) {
		if (!(status & RxBD_ERROR)) {
			buf_hi = muram_readw(&rxbd->buf_ptr_hi);
			buf_lo = in_be32(&rxbd->buf_ptr_lo);
			data = (u8 *)((ulong)(buf_hi << 16) << 16 | buf_lo);
			len = muram_readw(&rxbd->len);
			net_process_received_packet(data, len);
		} else {
			printf("%s: Rx error\n", dev->name);
			ret = 0;
		}

		/* clear the RxBDs */
		muram_writew(&rxbd->status, RxBD_EMPTY);
		muram_writew(&rxbd->len, 0);
		sync();

		/* advance RxBD */
		rxbd++;
		rxbd_base = (struct fm_port_bd *)fm_eth->rx_bd_ring;
		if (rxbd >= (rxbd_base + RX_BD_RING_SIZE))
			rxbd = rxbd_base;
		/* read next status */
		status = muram_readw(&rxbd->status);

		/* update RxQD */
		offset_out = muram_readw(&pram->rxqd.offset_out);
		offset_out += sizeof(struct fm_port_bd);
		if (offset_out >= muram_readw(&pram->rxqd.bd_ring_size))
			offset_out = 0;
		muram_writew(&pram->rxqd.offset_out, offset_out);
		sync();
	}
	fm_eth->cur_rxbd = (void *)rxbd;

	return ret;
}

static int fm_eth_init_mac(struct fm_eth *fm_eth, struct ccsr_fman *reg)
{
	struct fsl_enet_mac *mac;
	int num;
	void *base, *phyregs = NULL;

	num = fm_eth->num;

#ifdef CONFIG_SYS_FMAN_V3
#ifndef CONFIG_FSL_FM_10GEC_REGULAR_NOTATION
	if (fm_eth->type == FM_ETH_10G_E) {
		/* 10GEC1/10GEC2 use mEMAC9/mEMAC10 on T2080/T4240.
		 * 10GEC3/10GEC4 use mEMAC1/mEMAC2 on T2080.
		 * 10GEC1 uses mEMAC1 on T1024.
		 * so it needs to change the num.
		 */
		if (fm_eth->num >= 2)
			num -= 2;
		else
			num += 8;
	}
#endif
	base = &reg->memac[num].fm_memac;
	phyregs = &reg->memac[num].fm_memac_mdio;
#else
	/* Get the mac registers base address */
	if (fm_eth->type == FM_ETH_1G_E) {
		base = &reg->mac_1g[num].fm_dtesc;
		phyregs = &reg->mac_1g[num].fm_mdio.miimcfg;
	} else {
		base = &reg->mac_10g[num].fm_10gec;
		phyregs = &reg->mac_10g[num].fm_10gec_mdio;
	}
#endif

	/* alloc mac controller */
	mac = malloc(sizeof(struct fsl_enet_mac));
	if (!mac)
		return -ENOMEM;
	memset(mac, 0, sizeof(struct fsl_enet_mac));

	/* save the mac to fm_eth struct */
	fm_eth->mac = mac;

#ifdef CONFIG_SYS_FMAN_V3
	init_memac(mac, base, phyregs, MAX_RXBUF_LEN);
#else
	if (fm_eth->type == FM_ETH_1G_E)
		init_dtsec(mac, base, phyregs, MAX_RXBUF_LEN);
	else
		init_tgec(mac, base, phyregs, MAX_RXBUF_LEN);
#endif

	return 0;
}

static int init_phy(struct eth_device *dev)
{
	struct fm_eth *fm_eth = dev->priv;
#ifdef CONFIG_PHYLIB
	struct phy_device *phydev = NULL;
	u32 supported;
#endif

	if (fm_eth->type == FM_ETH_1G_E)
		dtsec_init_phy(dev);

#ifdef CONFIG_PHYLIB
	if (fm_eth->bus) {
		phydev = phy_connect(fm_eth->bus, fm_eth->phyaddr, dev,
					fm_eth->enet_if);
		if (!phydev) {
			printf("Failed to connect\n");
			return -1;
		}
	} else {
		return 0;
	}

	if (fm_eth->type == FM_ETH_1G_E) {
		supported = (SUPPORTED_10baseT_Half |
				SUPPORTED_10baseT_Full |
				SUPPORTED_100baseT_Half |
				SUPPORTED_100baseT_Full |
				SUPPORTED_1000baseT_Full);
	} else {
		supported = SUPPORTED_10000baseT_Full;

		if (tgec_is_fibre(dev))
			phydev->port = PORT_FIBRE;
	}

	phydev->supported &= supported;
	phydev->advertising = phydev->supported;

	fm_eth->phydev = phydev;

	phy_config(phydev);
#endif

	return 0;
}

int fm_eth_initialize(struct ccsr_fman *reg, struct fm_eth_info *info)
{
	struct eth_device *dev;
	struct fm_eth *fm_eth;
	int i, num = info->num;
	int ret;

	/* alloc eth device */
	dev = (struct eth_device *)malloc(sizeof(struct eth_device));
	if (!dev)
		return -ENOMEM;
	memset(dev, 0, sizeof(struct eth_device));

	/* alloc the FMan ethernet private struct */
	fm_eth = (struct fm_eth *)malloc(sizeof(struct fm_eth));
	if (!fm_eth)
		return -ENOMEM;
	memset(fm_eth, 0, sizeof(struct fm_eth));

	/* save off some things we need from the info struct */
	fm_eth->fm_index = info->index - 1; /* keep as 0 based for muram */
	fm_eth->num = num;
	fm_eth->type = info->type;

	fm_eth->rx_port = (void *)&reg->port[info->rx_port_id - 1].fm_bmi;
	fm_eth->tx_port = (void *)&reg->port[info->tx_port_id - 1].fm_bmi;

	/* set the ethernet max receive length */
	fm_eth->max_rx_len = MAX_RXBUF_LEN;

	/* init global mac structure */
	ret = fm_eth_init_mac(fm_eth, reg);
	if (ret)
		return ret;

	/* keep same as the manual, we call FMAN1, FMAN2, DTSEC1, DTSEC2, etc */
	if (fm_eth->type == FM_ETH_1G_E)
		sprintf(dev->name, "FM%d@DTSEC%d", info->index, num + 1);
	else
		sprintf(dev->name, "FM%d@TGEC%d", info->index, num + 1);

	devlist[num_controllers++] = dev;
	dev->iobase = 0;
	dev->priv = (void *)fm_eth;
	dev->init = fm_eth_open;
	dev->halt = fm_eth_halt;
	dev->send = fm_eth_send;
	dev->recv = fm_eth_recv;
	fm_eth->dev = dev;
	fm_eth->bus = info->bus;
	fm_eth->phyaddr = info->phy_addr;
	fm_eth->enet_if = info->enet_if;

	/* startup the FM im */
	ret = fm_eth_startup(fm_eth);
	if (ret)
		return ret;

	init_phy(dev);

	/* clear the ethernet address */
	for (i = 0; i < 6; i++)
		dev->enetaddr[i] = 0;
	eth_register(dev);

	return 0;
}
