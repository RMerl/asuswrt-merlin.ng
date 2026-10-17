// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2009-2012 Freescale Semiconductor, Inc.
 * Copyright 2020 NXP
 *	Dave Liu <daveliu@freescale.com>
 */
#include <common.h>
#include <log.h>
#include <part.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/ofnode.h>
#include <linux/compat.h>
#include <phy_interface.h>
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
#include <linux/delay.h>

#include "fm.h"

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
	bool sgmii_2500 = (priv->enet_if ==
			PHY_INTERFACE_MODE_2500BASEX) ? true : false;
	int i = 0, j;

	bus.priv = priv->pcs_mdio;
	bus.read = memac_mdio_read;
	bus.write = memac_mdio_write;
	bus.reset = memac_mdio_reset;

qsgmii_loop:
	/* SGMII IF mode + AN enable only for 1G SGMII, not for 2.5G */
	if (sgmii_2500)
		value = PHY_SGMII_CR_PHY_RESET |
			PHY_SGMII_IF_SPEED_GIGABIT |
			PHY_SGMII_IF_MODE_SGMII;
	else
		value = PHY_SGMII_IF_MODE_SGMII | PHY_SGMII_IF_MODE_AN;

	for (j = 0; j <= 3; j++)
		debug("dump PCS reg %#x: %#x\n", j,
		      memac_mdio_read(&bus, i, MDIO_DEVAD_NONE, j));

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

static void dtsec_init_phy(struct fm_eth *fm_eth)
{
#ifndef CONFIG_SYS_FMAN_V3
	struct dtsec *regs = (struct dtsec *)CFG_SYS_FSL_FM1_DTSEC1_ADDR;

	/* Assign a Physical address to the TBI */
	out_be32(&regs->tbipa, CFG_SYS_TBIPA_VALUE);
#endif

	if (fm_eth->enet_if == PHY_INTERFACE_MODE_SGMII ||
	    fm_eth->enet_if == PHY_INTERFACE_MODE_QSGMII ||
	    fm_eth->enet_if == PHY_INTERFACE_MODE_2500BASEX)
		dtsec_configure_serdes(fm_eth);
}
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
	if (!timeout)
		printf("%s - timeout\n", __func__);
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
	if (!timeout)
		printf("%s - timeout\n", __func__);
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
	if (!rx_buf_pool) {
		free(rx_bd_ring_base);
		return -ENOMEM;
	}

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
	     (fm_eth->enet_if == PHY_INTERFACE_MODE_2500BASEX) ||
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

static int fm_eth_open(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct fm_eth *fm_eth = dev_get_priv(dev);
	unsigned char *enetaddr;
	struct fsl_enet_mac *mac;
#ifdef CONFIG_PHYLIB
	int ret;
#endif

	mac = fm_eth->mac;

	enetaddr = pdata->enetaddr;

	/* setup the MAC address */
	if (enetaddr[0] & 0x01) {
		printf("%s: MacAddress is multicast address\n",	__func__);
		enetaddr[0] = 0;
		enetaddr[5] = fm_eth->num;
	}
	mac->set_mac_addr(mac, enetaddr);

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
			printf("%s: Could not initialize\n", dev->name);
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
	debug("MAC IF mode %d, speed %d, link %d\n", fm_eth->enet_if,
	      fm_eth->phydev->speed, fm_eth->phydev->link);

	if (!fm_eth->phydev->link)
		printf("%s: No link.\n", fm_eth->phydev->dev->name);

	return fm_eth->phydev->link ? 0 : -1;
}

static void fm_eth_halt(struct udevice *dev)
{
	struct fm_eth *fm_eth;
	struct fsl_enet_mac *mac;

	fm_eth = dev_get_priv(dev);
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

static int fm_eth_send(struct udevice *dev, void *buf, int len)
{
	struct fm_eth *fm_eth;
	struct fm_port_global_pram *pram;
	struct fm_port_bd *txbd, *txbd_base;
	u16 offset_in;
	int i;

	fm_eth = dev_get_priv(dev);
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

static struct fm_port_bd *fm_eth_free_one(struct fm_eth *fm_eth,
					  struct fm_port_bd *rxbd)
{
	struct fm_port_global_pram *pram;
	struct fm_port_bd *rxbd_base;
	u16 offset_out;

	pram = fm_eth->rx_pram;

	/* clear the RxBDs */
	muram_writew(&rxbd->status, RxBD_EMPTY);
	muram_writew(&rxbd->len, 0);
	sync();

	/* advance RxBD */
	rxbd++;
	rxbd_base = (struct fm_port_bd *)fm_eth->rx_bd_ring;
	if (rxbd >= (rxbd_base + RX_BD_RING_SIZE))
		rxbd = rxbd_base;

	/* update RxQD */
	offset_out = muram_readw(&pram->rxqd.offset_out);
	offset_out += sizeof(struct fm_port_bd);
	if (offset_out >= muram_readw(&pram->rxqd.bd_ring_size))
		offset_out = 0;
	muram_writew(&pram->rxqd.offset_out, offset_out);
	sync();

	return rxbd;
}

static int fm_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct fm_eth *fm_eth;
	struct fm_port_bd *rxbd;
	u32 buf_lo, buf_hi;
	u16 status, len;
	int ret = -1;
	u8 *data;

	fm_eth = dev_get_priv(dev);
	rxbd = fm_eth->cur_rxbd;
	status = muram_readw(&rxbd->status);

	while (!(status & RxBD_EMPTY)) {
		if (!(status & RxBD_ERROR)) {
			buf_hi = muram_readw(&rxbd->buf_ptr_hi);
			buf_lo = in_be32(&rxbd->buf_ptr_lo);
			data = (u8 *)((ulong)(buf_hi << 16) << 16 | buf_lo);
			len = muram_readw(&rxbd->len);
			*packetp = data;
			return len;
		} else {
			printf("%s: Rx error\n", dev->name);
			ret = 0;
		}

		/* free current bd, advance to next one */
		rxbd = fm_eth_free_one(fm_eth, rxbd);

		/* read next status */
		status = muram_readw(&rxbd->status);
	}
	fm_eth->cur_rxbd = (void *)rxbd;

	return ret;
}

static int fm_eth_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct fm_eth *fm_eth = (struct fm_eth *)dev_get_priv(dev);

	fm_eth->cur_rxbd = fm_eth_free_one(fm_eth, fm_eth->cur_rxbd);

	return 0;
}

static int fm_eth_init_mac(struct fm_eth *fm_eth, void *reg)
{
#ifndef CONFIG_SYS_FMAN_V3
	void *mdio;
#endif

	fm_eth->mac = kzalloc(sizeof(*fm_eth->mac), GFP_KERNEL);
	if (!fm_eth->mac)
		return -ENOMEM;

#ifndef CONFIG_SYS_FMAN_V3
	mdio = fman_mdio(fm_eth->dev->parent, fm_eth->mac_type, fm_eth->num);
	debug("MDIO %d @ %p\n", fm_eth->num, mdio);
#endif

	switch (fm_eth->mac_type) {
#ifdef CONFIG_SYS_FMAN_V3
	case FM_MEMAC:
		init_memac(fm_eth->mac, reg, NULL, MAX_RXBUF_LEN);
		break;
#else
	case FM_DTSEC:
		init_dtsec(fm_eth->mac, reg, mdio, MAX_RXBUF_LEN);
		break;
	case FM_TGEC:
		init_tgec(fm_eth->mac, reg, mdio, MAX_RXBUF_LEN);
		break;
#endif
	}

	return 0;
}

static int init_phy(struct fm_eth *fm_eth)
{
#ifdef CONFIG_PHYLIB
	u32 supported = PHY_GBIT_FEATURES;

	if (fm_eth->type == FM_ETH_10G_E)
		supported = PHY_10G_FEATURES;
	if (fm_eth->enet_if == PHY_INTERFACE_MODE_2500BASEX)
		supported |= SUPPORTED_2500baseX_Full;
#endif

	if (fm_eth->type == FM_ETH_1G_E)
		dtsec_init_phy(fm_eth);

#ifdef CONFIG_PHYLIB
#ifdef CONFIG_DM_MDIO
	fm_eth->phydev = dm_eth_phy_connect(fm_eth->dev);
	if (!fm_eth->phydev)
		return -ENODEV;
#endif
	fm_eth->phydev->advertising &= supported;
	fm_eth->phydev->supported &= supported;

	phy_config(fm_eth->phydev);
#endif
	return 0;
}

static int fm_eth_bind(struct udevice *dev)
{
	char mac_name[11];
	u32 fm, num;

	if (ofnode_read_u32(ofnode_get_parent(dev_ofnode(dev)), "cell-index", &fm)) {
		printf("FMan node property cell-index missing\n");
		return -EINVAL;
	}

	if (dev && dev_read_u32(dev, "cell-index", &num)) {
		printf("FMan MAC node property cell-index missing\n");
		return -EINVAL;
	}

	sprintf(mac_name, "fm%d-mac%d", fm + 1, num + 1);
	device_set_name(dev, mac_name);

	debug("%s - binding %s\n", __func__, mac_name);

	return 0;
}

static struct udevice *fm_get_internal_mdio(struct udevice *dev)
{
	struct ofnode_phandle_args phandle = {.node = ofnode_null()};
	struct udevice *mdiodev;

	if (dev_read_phandle_with_args(dev, "pcsphy-handle", NULL,
				       0, 0, &phandle) ||
	    !ofnode_valid(phandle.node)) {
		if (dev_read_phandle_with_args(dev, "tbi-handle", NULL,
					       0, 0, &phandle) ||
		    !ofnode_valid(phandle.node)) {
			printf("Issue reading pcsphy-handle/tbi-handle for MAC %s\n",
			       dev->name);
			return NULL;
		}
	}

	if (uclass_get_device_by_ofnode(UCLASS_MDIO,
					ofnode_get_parent(phandle.node),
					&mdiodev)) {
		printf("can't find MDIO bus for node %s\n",
		       ofnode_get_name(ofnode_get_parent(phandle.node)));
		return NULL;
	}
	debug("Found internal MDIO bus %p\n", mdiodev);

	return mdiodev;
}

static int fm_eth_probe(struct udevice *dev)
{
	struct fm_eth *fm_eth = (struct fm_eth *)dev_get_priv(dev);
	struct ofnode_phandle_args args;
	void *reg;
	int ret, index;

	debug("%s enter for dev %p fm_eth %p - %s\n", __func__, dev, fm_eth,
	      (dev) ? dev->name : "-");

	if (fm_eth->dev) {
		printf("%s already probed, exit\n", (dev) ? dev->name : "-");
		return 0;
	}

	fm_eth->dev = dev;
	fm_eth->fm_index = fman_id(dev->parent);
	reg = (void *)(uintptr_t)dev_read_addr(dev);
	fm_eth->mac_type = dev_get_driver_data(dev);
#ifdef CONFIG_PHYLIB
	fm_eth->enet_if = dev_read_phy_mode(dev);
#else
	fm_eth->enet_if = PHY_INTERFACE_MODE_SGMII;
	printf("%s: warning - unable to determine interface type\n", __func__);
#endif
	switch (fm_eth->mac_type) {
#ifndef CONFIG_SYS_FMAN_V3
	case FM_TGEC:
		fm_eth->type = FM_ETH_10G_E;
		break;
	case FM_DTSEC:
#else
	case FM_MEMAC:
		/* default to 1G, 10G is indicated by port property in dts */
#endif
		fm_eth->type = FM_ETH_1G_E;
		break;
	}

	if (dev_read_u32(dev, "cell-index", &fm_eth->num)) {
		printf("FMan MAC node property cell-index missing\n");
		return -EINVAL;
	}

	if (dev_read_phandle_with_args(dev, "fsl,fman-ports", NULL,
				       0, 0, &args))
		goto ports_ref_failure;
	index = ofnode_read_u32_default(args.node, "cell-index", 0);
	if (index <= 0)
		goto ports_ref_failure;
	fm_eth->rx_port = fman_port(dev->parent, index);

	if (ofnode_read_bool(args.node, "fsl,fman-10g-port"))
		fm_eth->type = FM_ETH_10G_E;

	if (dev_read_phandle_with_args(dev, "fsl,fman-ports", NULL,
				       0, 1, &args))
		goto ports_ref_failure;
	index = ofnode_read_u32_default(args.node, "cell-index", 0);
	if (index <= 0)
		goto ports_ref_failure;
	fm_eth->tx_port = fman_port(dev->parent, index);

	/* set the ethernet max receive length */
	fm_eth->max_rx_len = MAX_RXBUF_LEN;

	switch (fm_eth->enet_if) {
	case PHY_INTERFACE_MODE_QSGMII:
		/* all PCS blocks are accessed on one controller */
		if (fm_eth->num != 0)
			break;
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_2500BASEX:
		fm_eth->pcs_mdio = fm_get_internal_mdio(dev);
		break;
	default:
		break;
	}

	/* init global mac structure */
	ret = fm_eth_init_mac(fm_eth, reg);
	if (ret)
		return ret;

	/* startup the FM im */
	ret = fm_eth_startup(fm_eth);

	if (!ret)
		ret = init_phy(fm_eth);

	return ret;

ports_ref_failure:
	printf("Issue reading fsl,fman-ports for MAC %s\n", dev->name);
	return -ENOENT;
}

static int fm_eth_remove(struct udevice *dev)
{
	return 0;
}

static const struct eth_ops fm_eth_ops = {
	.start = fm_eth_open,
	.send = fm_eth_send,
	.recv = fm_eth_recv,
	.free_pkt = fm_eth_free_pkt,
	.stop = fm_eth_halt,
};

static const struct udevice_id fm_eth_ids[] = {
#ifdef CONFIG_SYS_FMAN_V3
	{ .compatible = "fsl,fman-memac", .data = FM_MEMAC },
#else
	{ .compatible = "fsl,fman-dtsec", .data = FM_DTSEC },
	{ .compatible = "fsl,fman-xgec", .data = FM_TGEC },
#endif
	{}
};

U_BOOT_DRIVER(eth_fman) = {
	.name = "eth_fman",
	.id = UCLASS_ETH,
	.of_match = fm_eth_ids,
	.bind = fm_eth_bind,
	.probe = fm_eth_probe,
	.remove = fm_eth_remove,
	.ops = &fm_eth_ops,
	.priv_auto	= sizeof(struct fm_eth),
	.plat_auto	= sizeof(struct eth_pdata),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
