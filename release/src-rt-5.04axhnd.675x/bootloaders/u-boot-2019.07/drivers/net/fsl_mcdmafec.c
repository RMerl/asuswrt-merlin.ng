// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#include <common.h>
#include <environment.h>
#include <malloc.h>
#include <command.h>
#include <config.h>
#include <net.h>
#include <miiphy.h>

#undef	ET_DEBUG
#undef	MII_DEBUG

/* Ethernet Transmit and Receive Buffers */
#define DBUF_LENGTH		1520
#define PKT_MAXBUF_SIZE		1518
#define PKT_MINBUF_SIZE		64
#define PKT_MAXBLR_SIZE		1536
#define LAST_PKTBUFSRX		PKTBUFSRX - 1
#define BD_ENET_RX_W_E		(BD_ENET_RX_WRAP | BD_ENET_RX_EMPTY)
#define BD_ENET_TX_RDY_LST	(BD_ENET_TX_READY | BD_ENET_TX_LAST)
#define FIFO_ERRSTAT		(FIFO_STAT_RXW | FIFO_STAT_UF | FIFO_STAT_OF)

/* RxBD bits definitions */
#define BD_ENET_RX_ERR	(BD_ENET_RX_LG | BD_ENET_RX_NO | BD_ENET_RX_CR | \
			 BD_ENET_RX_OV | BD_ENET_RX_TR)

#include <asm/immap.h>
#include <asm/fsl_mcdmafec.h>

#include "MCD_dma.h"

struct fec_info_dma fec_info[] = {
#ifdef CONFIG_SYS_FEC0_IOBASE
	{
	 0,			/* index */
	 CONFIG_SYS_FEC0_IOBASE,	/* io base */
	 CONFIG_SYS_FEC0_PINMUX,	/* gpio pin muxing */
	 CONFIG_SYS_FEC0_MIIBASE,	/* mii base */
	 -1,			/* phy_addr */
	 0,			/* duplex and speed */
	 0,			/* phy name */
	 0,			/* phyname init */
	 0,			/* RX BD */
	 0,			/* TX BD */
	 0,			/* rx Index */
	 0,			/* tx Index */
	 0,			/* tx buffer */
	 0,			/* initialized flag */
	 (struct fec_info_dma *)-1,	/* next */
	 FEC0_RX_TASK,		/* rxTask */
	 FEC0_TX_TASK,		/* txTask */
	 FEC0_RX_PRIORITY,	/* rxPri */
	 FEC0_TX_PRIORITY,	/* txPri */
	 FEC0_RX_INIT,		/* rxInit */
	 FEC0_TX_INIT,		/* txInit */
	 0,			/* usedTbdIndex */
	 0,			/* cleanTbdNum */
	 },
#endif
#ifdef CONFIG_SYS_FEC1_IOBASE
	{
	 1,			/* index */
	 CONFIG_SYS_FEC1_IOBASE,	/* io base */
	 CONFIG_SYS_FEC1_PINMUX,	/* gpio pin muxing */
	 CONFIG_SYS_FEC1_MIIBASE,	/* mii base */
	 -1,			/* phy_addr */
	 0,			/* duplex and speed */
	 0,			/* phy name */
	 0,			/* phy name init */
#ifdef CONFIG_SYS_DMA_USE_INTSRAM
	 (cbd_t *)DBUF_LENGTH,	/* RX BD */
#else
	 0,			/* RX BD */
#endif
	 0,			/* TX BD */
	 0,			/* rx Index */
	 0,			/* tx Index */
	 0,			/* tx buffer */
	 0,			/* initialized flag */
	 (struct fec_info_dma *)-1,	/* next */
	 FEC1_RX_TASK,		/* rxTask */
	 FEC1_TX_TASK,		/* txTask */
	 FEC1_RX_PRIORITY,	/* rxPri */
	 FEC1_TX_PRIORITY,	/* txPri */
	 FEC1_RX_INIT,		/* rxInit */
	 FEC1_TX_INIT,		/* txInit */
	 0,			/* usedTbdIndex */
	 0,			/* cleanTbdNum */
	 }
#endif
};

static int fec_send(struct eth_device *dev, void *packet, int length);
static int fec_recv(struct eth_device *dev);
static int fec_init(struct eth_device *dev, bd_t * bd);
static void fec_halt(struct eth_device *dev);

#ifdef ET_DEBUG
static void dbg_fec_regs(struct eth_device *dev)
{
	struct fec_info_dma *info = dev->priv;
	volatile fecdma_t *fecp = (fecdma_t *) (info->iobase);

	printf("=====\n");
	printf("ievent       %x - %x\n", (int)&fecp->eir, fecp->eir);
	printf("imask        %x - %x\n", (int)&fecp->eimr, fecp->eimr);
	printf("ecntrl       %x - %x\n", (int)&fecp->ecr, fecp->ecr);
	printf("mii_mframe   %x - %x\n", (int)&fecp->mmfr, fecp->mmfr);
	printf("mii_speed    %x - %x\n", (int)&fecp->mscr, fecp->mscr);
	printf("mii_ctrlstat %x - %x\n", (int)&fecp->mibc, fecp->mibc);
	printf("r_cntrl      %x - %x\n", (int)&fecp->rcr, fecp->rcr);
	printf("r hash       %x - %x\n", (int)&fecp->rhr, fecp->rhr);
	printf("x_cntrl      %x - %x\n", (int)&fecp->tcr, fecp->tcr);
	printf("padr_l       %x - %x\n", (int)&fecp->palr, fecp->palr);
	printf("padr_u       %x - %x\n", (int)&fecp->paur, fecp->paur);
	printf("op_pause     %x - %x\n", (int)&fecp->opd, fecp->opd);
	printf("iadr_u       %x - %x\n", (int)&fecp->iaur, fecp->iaur);
	printf("iadr_l       %x - %x\n", (int)&fecp->ialr, fecp->ialr);
	printf("gadr_u       %x - %x\n", (int)&fecp->gaur, fecp->gaur);
	printf("gadr_l       %x - %x\n", (int)&fecp->galr, fecp->galr);
	printf("x_wmrk       %x - %x\n", (int)&fecp->tfwr, fecp->tfwr);
	printf("r_fdata      %x - %x\n", (int)&fecp->rfdr, fecp->rfdr);
	printf("r_fstat      %x - %x\n", (int)&fecp->rfsr, fecp->rfsr);
	printf("r_fctrl      %x - %x\n", (int)&fecp->rfcr, fecp->rfcr);
	printf("r_flrfp      %x - %x\n", (int)&fecp->rlrfp, fecp->rlrfp);
	printf("r_flwfp      %x - %x\n", (int)&fecp->rlwfp, fecp->rlwfp);
	printf("r_frfar      %x - %x\n", (int)&fecp->rfar, fecp->rfar);
	printf("r_frfrp      %x - %x\n", (int)&fecp->rfrp, fecp->rfrp);
	printf("r_frfwp      %x - %x\n", (int)&fecp->rfwp, fecp->rfwp);
	printf("t_fdata      %x - %x\n", (int)&fecp->tfdr, fecp->tfdr);
	printf("t_fstat      %x - %x\n", (int)&fecp->tfsr, fecp->tfsr);
	printf("t_fctrl      %x - %x\n", (int)&fecp->tfcr, fecp->tfcr);
	printf("t_flrfp      %x - %x\n", (int)&fecp->tlrfp, fecp->tlrfp);
	printf("t_flwfp      %x - %x\n", (int)&fecp->tlwfp, fecp->tlwfp);
	printf("t_ftfar      %x - %x\n", (int)&fecp->tfar, fecp->tfar);
	printf("t_ftfrp      %x - %x\n", (int)&fecp->tfrp, fecp->tfrp);
	printf("t_ftfwp      %x - %x\n", (int)&fecp->tfwp, fecp->tfwp);
	printf("frst         %x - %x\n", (int)&fecp->frst, fecp->frst);
	printf("ctcwr        %x - %x\n", (int)&fecp->ctcwr, fecp->ctcwr);
}
#endif

static void set_fec_duplex_speed(volatile fecdma_t * fecp, bd_t * bd,
				 int dup_spd)
{
	if ((dup_spd >> 16) == FULL) {
		/* Set maximum frame length */
		fecp->rcr = FEC_RCR_MAX_FL(PKT_MAXBUF_SIZE) | FEC_RCR_MII_MODE |
		    FEC_RCR_PROM | 0x100;
		fecp->tcr = FEC_TCR_FDEN;
	} else {
		/* Half duplex mode */
		fecp->rcr = FEC_RCR_MAX_FL(PKT_MAXBUF_SIZE) |
		    FEC_RCR_MII_MODE | FEC_RCR_DRT;
		fecp->tcr &= ~FEC_TCR_FDEN;
	}

	if ((dup_spd & 0xFFFF) == _100BASET) {
#ifdef MII_DEBUG
		printf("100Mbps\n");
#endif
		bd->bi_ethspeed = 100;
	} else {
#ifdef MII_DEBUG
		printf("10Mbps\n");
#endif
		bd->bi_ethspeed = 10;
	}
}

static int fec_send(struct eth_device *dev, void *packet, int length)
{
	struct fec_info_dma *info = dev->priv;
	cbd_t *pTbd, *pUsedTbd;
	u16 phyStatus;

	miiphy_read(dev->name, info->phy_addr, MII_BMSR, &phyStatus);

	/* process all the consumed TBDs */
	while (info->cleanTbdNum < CONFIG_SYS_TX_ETH_BUFFER) {
		pUsedTbd = &info->txbd[info->usedTbdIdx];
		if (pUsedTbd->cbd_sc & BD_ENET_TX_READY) {
#ifdef ET_DEBUG
			printf("Cannot clean TBD %d, in use\n",
			       info->cleanTbdNum);
#endif
			return 0;
		}

		/* clean this buffer descriptor */
		if (info->usedTbdIdx == (CONFIG_SYS_TX_ETH_BUFFER - 1))
			pUsedTbd->cbd_sc = BD_ENET_TX_WRAP;
		else
			pUsedTbd->cbd_sc = 0;

		/* update some indeces for a correct handling of the TBD ring */
		info->cleanTbdNum++;
		info->usedTbdIdx = (info->usedTbdIdx + 1) % CONFIG_SYS_TX_ETH_BUFFER;
	}

	/* Check for valid length of data. */
	if ((length > 1500) || (length <= 0)) {
		return -1;
	}

	/* Check the number of vacant TxBDs. */
	if (info->cleanTbdNum < 1) {
		printf("No available TxBDs ...\n");
		return -1;
	}

	/* Get the first TxBD to send the mac header */
	pTbd = &info->txbd[info->txIdx];
	pTbd->cbd_datlen = length;
	pTbd->cbd_bufaddr = (u32) packet;
	pTbd->cbd_sc |= BD_ENET_TX_LAST | BD_ENET_TX_TC | BD_ENET_TX_READY;
	info->txIdx = (info->txIdx + 1) % CONFIG_SYS_TX_ETH_BUFFER;

	/* Enable DMA transmit task */
	MCD_continDma(info->txTask);

	info->cleanTbdNum -= 1;

	/* wait until frame is sent . */
	while (pTbd->cbd_sc & BD_ENET_TX_READY) {
		udelay(10);
	}

	return (int)(info->txbd[info->txIdx].cbd_sc & BD_ENET_TX_STATS);
}

static int fec_recv(struct eth_device *dev)
{
	struct fec_info_dma *info = dev->priv;
	volatile fecdma_t *fecp = (fecdma_t *) (info->iobase);

	cbd_t *prbd = &info->rxbd[info->rxIdx];
	u32 ievent;
	int frame_length, len = 0;

	/* Check if any critical events have happened */
	ievent = fecp->eir;
	if (ievent != 0) {
		fecp->eir = ievent;

		if (ievent & (FEC_EIR_BABT | FEC_EIR_TXERR | FEC_EIR_RXERR)) {
			printf("fec_recv: error\n");
			fec_halt(dev);
			fec_init(dev, NULL);
			return 0;
		}

		if (ievent & FEC_EIR_HBERR) {
			/* Heartbeat error */
			fecp->tcr |= FEC_TCR_GTS;
		}

		if (ievent & FEC_EIR_GRA) {
			/* Graceful stop complete */
			if (fecp->tcr & FEC_TCR_GTS) {
				printf("fec_recv: tcr_gts\n");
				fec_halt(dev);
				fecp->tcr &= ~FEC_TCR_GTS;
				fec_init(dev, NULL);
			}
		}
	}

	if (!(prbd->cbd_sc & BD_ENET_RX_EMPTY)) {
		if ((prbd->cbd_sc & BD_ENET_RX_LAST) &&
		    !(prbd->cbd_sc & BD_ENET_RX_ERR) &&
		    ((prbd->cbd_datlen - 4) > 14)) {

			/* Get buffer address and size */
			frame_length = prbd->cbd_datlen - 4;

			/* Fill the buffer and pass it to upper layers */
			net_process_received_packet((uchar *)prbd->cbd_bufaddr,
						    frame_length);
			len = frame_length;
		}

		/* Reset buffer descriptor as empty */
		if ((info->rxIdx) == (PKTBUFSRX - 1))
			prbd->cbd_sc = (BD_ENET_RX_WRAP | BD_ENET_RX_EMPTY);
		else
			prbd->cbd_sc = BD_ENET_RX_EMPTY;

		prbd->cbd_datlen = PKTSIZE_ALIGN;

		/* Now, we have an empty RxBD, restart the DMA receive task */
		MCD_continDma(info->rxTask);

		/* Increment BD count */
		info->rxIdx = (info->rxIdx + 1) % PKTBUFSRX;
	}

	return len;
}

static void fec_set_hwaddr(volatile fecdma_t * fecp, u8 * mac)
{
	u8 currByte;		/* byte for which to compute the CRC */
	int byte;		/* loop - counter */
	int bit;		/* loop - counter */
	u32 crc = 0xffffffff;	/* initial value */

	for (byte = 0; byte < 6; byte++) {
		currByte = mac[byte];
		for (bit = 0; bit < 8; bit++) {
			if ((currByte & 0x01) ^ (crc & 0x01)) {
				crc >>= 1;
				crc = crc ^ 0xedb88320;
			} else {
				crc >>= 1;
			}
			currByte >>= 1;
		}
	}

	crc = crc >> 26;

	/* Set individual hash table register */
	if (crc >= 32) {
		fecp->ialr = (1 << (crc - 32));
		fecp->iaur = 0;
	} else {
		fecp->ialr = 0;
		fecp->iaur = (1 << crc);
	}

	/* Set physical address */
	fecp->palr = (mac[0] << 24) + (mac[1] << 16) + (mac[2] << 8) + mac[3];
	fecp->paur = (mac[4] << 24) + (mac[5] << 16) + 0x8808;

	/* Clear multicast address hash table */
	fecp->gaur = 0;
	fecp->galr = 0;
}

static int fec_init(struct eth_device *dev, bd_t * bd)
{
	struct fec_info_dma *info = dev->priv;
	volatile fecdma_t *fecp = (fecdma_t *) (info->iobase);
	int i;
	uchar enetaddr[6];

#ifdef ET_DEBUG
	printf("fec_init: iobase 0x%08x ...\n", info->iobase);
#endif

	fecpin_setclear(dev, 1);

	fec_halt(dev);

#if defined(CONFIG_CMD_MII) || defined (CONFIG_MII) || \
	defined (CONFIG_SYS_DISCOVER_PHY)

	mii_init();

	set_fec_duplex_speed(fecp, bd, info->dup_spd);
#else
#ifndef CONFIG_SYS_DISCOVER_PHY
	set_fec_duplex_speed(fecp, bd, (FECDUPLEX << 16) | FECSPEED);
#endif				/* ifndef CONFIG_SYS_DISCOVER_PHY */
#endif				/* CONFIG_CMD_MII || CONFIG_MII */

	/* We use strictly polling mode only */
	fecp->eimr = 0;

	/* Clear any pending interrupt */
	fecp->eir = 0xffffffff;

	/* Set station address   */
	if ((u32) fecp == CONFIG_SYS_FEC0_IOBASE)
		eth_env_get_enetaddr("ethaddr", enetaddr);
	else
		eth_env_get_enetaddr("eth1addr", enetaddr);
	fec_set_hwaddr(fecp, enetaddr);

	/* Set Opcode/Pause Duration Register */
	fecp->opd = 0x00010020;

	/* Setup Buffers and Buffer Descriptors */
	info->rxIdx = 0;
	info->txIdx = 0;

	/* Setup Receiver Buffer Descriptors (13.14.24.18)
	 * Settings:     Empty, Wrap */
	for (i = 0; i < PKTBUFSRX; i++) {
		info->rxbd[i].cbd_sc = BD_ENET_RX_EMPTY;
		info->rxbd[i].cbd_datlen = PKTSIZE_ALIGN;
		info->rxbd[i].cbd_bufaddr = (uint) net_rx_packets[i];
	}
	info->rxbd[PKTBUFSRX - 1].cbd_sc |= BD_ENET_RX_WRAP;

	/* Setup Ethernet Transmitter Buffer Descriptors (13.14.24.19)
	 * Settings:    Last, Tx CRC */
	for (i = 0; i < CONFIG_SYS_TX_ETH_BUFFER; i++) {
		info->txbd[i].cbd_sc = 0;
		info->txbd[i].cbd_datlen = 0;
		info->txbd[i].cbd_bufaddr = (uint) (&info->txbuf[0]);
	}
	info->txbd[CONFIG_SYS_TX_ETH_BUFFER - 1].cbd_sc |= BD_ENET_TX_WRAP;

	info->usedTbdIdx = 0;
	info->cleanTbdNum = CONFIG_SYS_TX_ETH_BUFFER;

	/* Set Rx FIFO alarm and granularity value */
	fecp->rfcr = 0x0c000000;
	fecp->rfar = 0x0000030c;

	/* Set Tx FIFO granularity value */
	fecp->tfcr = FIFO_CTRL_FRAME | FIFO_CTRL_GR(6) | 0x00040000;
	fecp->tfar = 0x00000080;

	fecp->tfwr = 0x2;
	fecp->ctcwr = 0x03000000;

	/* Enable DMA receive task */
	MCD_startDma(info->rxTask,	/* Dma channel */
		     (s8 *) info->rxbd,	/*Source Address */
		     0,		/* Source increment */
		     (s8 *) (&fecp->rfdr),	/* dest */
		     4,		/* dest increment */
		     0,		/* DMA size */
		     4,		/* xfer size */
		     info->rxInit,	/* initiator */
		     info->rxPri,	/* priority */
		     (MCD_FECRX_DMA | MCD_TT_FLAGS_DEF),	/* Flags */
		     (MCD_NO_CSUM | MCD_NO_BYTE_SWAP)	/* Function description */
	    );

	/* Enable DMA tx task with no ready buffer descriptors */
	MCD_startDma(info->txTask,	/* Dma channel */
		     (s8 *) info->txbd,	/*Source Address */
		     0,		/* Source increment */
		     (s8 *) (&fecp->tfdr),	/* dest */
		     4,		/* dest incr */
		     0,		/* DMA size */
		     4,		/* xfer size */
		     info->txInit,	/* initiator */
		     info->txPri,	/* priority */
		     (MCD_FECTX_DMA | MCD_TT_FLAGS_DEF),	/* Flags */
		     (MCD_NO_CSUM | MCD_NO_BYTE_SWAP)	/* Function description */
	    );

	/* Now enable the transmit and receive processing */
	fecp->ecr |= FEC_ECR_ETHER_EN;

	return 1;
}

static void fec_halt(struct eth_device *dev)
{
	struct fec_info_dma *info = dev->priv;
	volatile fecdma_t *fecp = (fecdma_t *) (info->iobase);
	int counter = 0xffff;

	/* issue graceful stop command to the FEC transmitter if necessary */
	fecp->tcr |= FEC_TCR_GTS;

	/* wait for graceful stop to register */
	while ((counter--) && (!(fecp->eir & FEC_EIR_GRA))) ;

	/* Disable DMA tasks */
	MCD_killDma(info->txTask);
	MCD_killDma(info->rxTask);

	/* Disable the Ethernet Controller */
	fecp->ecr &= ~FEC_ECR_ETHER_EN;

	/* Clear FIFO status registers */
	fecp->rfsr &= FIFO_ERRSTAT;
	fecp->tfsr &= FIFO_ERRSTAT;

	fecp->frst = 0x01000000;

	/* Issue a reset command to the FEC chip */
	fecp->ecr |= FEC_ECR_RESET;

	/* wait at least 20 clock cycles */
	udelay(10000);

#ifdef ET_DEBUG
	printf("Ethernet task stopped\n");
#endif
}

int mcdmafec_initialize(bd_t * bis)
{
	struct eth_device *dev;
	int i;
#ifdef CONFIG_SYS_DMA_USE_INTSRAM
	u32 tmp = CONFIG_SYS_INTSRAM + 0x2000;
#endif

	for (i = 0; i < ARRAY_SIZE(fec_info); i++) {

		dev =
		    (struct eth_device *)memalign(CONFIG_SYS_CACHELINE_SIZE,
						  sizeof *dev);
		if (dev == NULL)
			hang();

		memset(dev, 0, sizeof(*dev));

		sprintf(dev->name, "FEC%d", fec_info[i].index);

		dev->priv = &fec_info[i];
		dev->init = fec_init;
		dev->halt = fec_halt;
		dev->send = fec_send;
		dev->recv = fec_recv;

		/* setup Receive and Transmit buffer descriptor */
#ifdef CONFIG_SYS_DMA_USE_INTSRAM
		fec_info[i].rxbd = (cbd_t *)((u32)fec_info[i].rxbd + tmp);
		tmp = (u32)fec_info[i].rxbd;
		fec_info[i].txbd =
		    (cbd_t *)((u32)fec_info[i].txbd + tmp +
		    (PKTBUFSRX * sizeof(cbd_t)));
		tmp = (u32)fec_info[i].txbd;
		fec_info[i].txbuf =
		    (char *)((u32)fec_info[i].txbuf + tmp +
		    (CONFIG_SYS_TX_ETH_BUFFER * sizeof(cbd_t)));
		tmp = (u32)fec_info[i].txbuf;
#else
		fec_info[i].rxbd =
		    (cbd_t *) memalign(CONFIG_SYS_CACHELINE_SIZE,
				       (PKTBUFSRX * sizeof(cbd_t)));
		fec_info[i].txbd =
		    (cbd_t *) memalign(CONFIG_SYS_CACHELINE_SIZE,
				       (CONFIG_SYS_TX_ETH_BUFFER * sizeof(cbd_t)));
		fec_info[i].txbuf =
		    (char *)memalign(CONFIG_SYS_CACHELINE_SIZE, DBUF_LENGTH);
#endif

#ifdef ET_DEBUG
		printf("rxbd %x txbd %x\n",
		       (int)fec_info[i].rxbd, (int)fec_info[i].txbd);
#endif

		fec_info[i].phy_name = (char *)memalign(CONFIG_SYS_CACHELINE_SIZE, 32);

		eth_register(dev);

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
		int retval;
		struct mii_dev *mdiodev = mdio_alloc();
		if (!mdiodev)
			return -ENOMEM;
		strncpy(mdiodev->name, dev->name, MDIO_NAME_LEN);
		mdiodev->read = mcffec_miiphy_read;
		mdiodev->write = mcffec_miiphy_write;

		retval = mdio_register(mdiodev);
		if (retval < 0)
			return retval;
#endif

		if (i > 0)
			fec_info[i - 1].next = &fec_info[i];
	}
	fec_info[i - 1].next = &fec_info[0];

	/* default speed */
	bis->bi_ethspeed = 10;

	return 0;
}
