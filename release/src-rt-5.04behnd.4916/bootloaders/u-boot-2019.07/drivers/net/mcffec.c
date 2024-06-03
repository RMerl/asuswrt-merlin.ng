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
#include <net.h>
#include <netdev.h>
#include <miiphy.h>

#include <asm/fec.h>
#include <asm/immap.h>

#undef	ET_DEBUG
#undef	MII_DEBUG

/* Ethernet Transmit and Receive Buffers */
#define DBUF_LENGTH		1520
#define TX_BUF_CNT		2
#define PKT_MAXBUF_SIZE		1518
#define PKT_MINBUF_SIZE		64
#define PKT_MAXBLR_SIZE		1520
#define LAST_PKTBUFSRX		PKTBUFSRX - 1
#define BD_ENET_RX_W_E		(BD_ENET_RX_WRAP | BD_ENET_RX_EMPTY)
#define BD_ENET_TX_RDY_LST	(BD_ENET_TX_READY | BD_ENET_TX_LAST)

struct fec_info_s fec_info[] = {
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
	 (struct fec_info_s *)-1,
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
#ifdef CONFIG_SYS_FEC_BUF_USE_SRAM
	 (cbd_t *)DBUF_LENGTH,	/* RX BD */
#else
	 0,			/* RX BD */
#endif
	 0,			/* TX BD */
	 0,			/* rx Index */
	 0,			/* tx Index */
	 0,			/* tx buffer */
	 0,			/* initialized flag */
	 (struct fec_info_s *)-1,
	 }
#endif
};

int fec_recv(struct eth_device *dev);
int fec_init(struct eth_device *dev, bd_t * bd);
void fec_halt(struct eth_device *dev);
void fec_reset(struct eth_device *dev);

void setFecDuplexSpeed(volatile fec_t * fecp, bd_t * bd, int dup_spd)
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
#ifdef CONFIG_MCF5445x
		fecp->rcr &= ~0x200;	/* disabled 10T base */
#endif
#ifdef MII_DEBUG
		printf("100Mbps\n");
#endif
		bd->bi_ethspeed = 100;
	} else {
#ifdef CONFIG_MCF5445x
		fecp->rcr |= 0x200;	/* enabled 10T base */
#endif
#ifdef MII_DEBUG
		printf("10Mbps\n");
#endif
		bd->bi_ethspeed = 10;
	}
}

static int fec_send(struct eth_device *dev, void *packet, int length)
{
	struct fec_info_s *info = dev->priv;
	volatile fec_t *fecp = (fec_t *) (info->iobase);
	int j, rc;
	u16 phyStatus;

	miiphy_read(dev->name, info->phy_addr, MII_BMSR, &phyStatus);

	/* section 16.9.23.3
	 * Wait for ready
	 */
	j = 0;
	while ((info->txbd[info->txIdx].cbd_sc & BD_ENET_TX_READY) &&
	       (j < MCFFEC_TOUT_LOOP)) {
		udelay(1);
		j++;
	}
	if (j >= MCFFEC_TOUT_LOOP) {
		printf("TX not ready\n");
	}

	info->txbd[info->txIdx].cbd_bufaddr = (uint) packet;
	info->txbd[info->txIdx].cbd_datlen = length;
	info->txbd[info->txIdx].cbd_sc |= BD_ENET_TX_RDY_LST;

	/* Activate transmit Buffer Descriptor polling */
	fecp->tdar = 0x01000000;	/* Descriptor polling active    */

#ifndef CONFIG_SYS_FEC_BUF_USE_SRAM
	/*
	 * FEC unable to initial transmit data packet.
	 * A nop will ensure the descriptor polling active completed.
	 * CF Internal RAM has shorter cycle access than DRAM. If use
	 * DRAM as Buffer descriptor and data, a nop is a must.
	 * Affect only V2 and V3.
	 */
	__asm__ ("nop");

#endif

#ifdef CONFIG_SYS_UNIFY_CACHE
	icache_invalid();
#endif

	j = 0;
	while ((info->txbd[info->txIdx].cbd_sc & BD_ENET_TX_READY) &&
	       (j < MCFFEC_TOUT_LOOP)) {
		udelay(1);
		j++;
	}
	if (j >= MCFFEC_TOUT_LOOP) {
		printf("TX timeout\n");
	}

#ifdef ET_DEBUG
	printf("%s[%d] %s: cycles: %d    status: %x  retry cnt: %d\n",
	       __FILE__, __LINE__, __FUNCTION__, j,
	       info->txbd[info->txIdx].cbd_sc,
	       (info->txbd[info->txIdx].cbd_sc & 0x003C) >> 2);
#endif

	/* return only status bits */
	rc = (info->txbd[info->txIdx].cbd_sc & BD_ENET_TX_STATS);
	info->txIdx = (info->txIdx + 1) % TX_BUF_CNT;

	return rc;
}

int fec_recv(struct eth_device *dev)
{
	struct fec_info_s *info = dev->priv;
	volatile fec_t *fecp = (fec_t *) (info->iobase);
	int length;

	for (;;) {
#ifndef CONFIG_SYS_FEC_BUF_USE_SRAM
#endif
#ifdef CONFIG_SYS_UNIFY_CACHE
		icache_invalid();
#endif
		/* section 16.9.23.2 */
		if (info->rxbd[info->rxIdx].cbd_sc & BD_ENET_RX_EMPTY) {
			length = -1;
			break;	/* nothing received - leave for() loop */
		}

		length = info->rxbd[info->rxIdx].cbd_datlen;

		if (info->rxbd[info->rxIdx].cbd_sc & 0x003f) {
			printf("%s[%d] err: %x\n",
			       __FUNCTION__, __LINE__,
			       info->rxbd[info->rxIdx].cbd_sc);
#ifdef ET_DEBUG
			printf("%s[%d] err: %x\n",
			       __FUNCTION__, __LINE__,
			       info->rxbd[info->rxIdx].cbd_sc);
#endif
		} else {

			length -= 4;
			/* Pass the packet up to the protocol layers. */
			net_process_received_packet(net_rx_packets[info->rxIdx],
						    length);

			fecp->eir |= FEC_EIR_RXF;
		}

		/* Give the buffer back to the FEC. */
		info->rxbd[info->rxIdx].cbd_datlen = 0;

		/* wrap around buffer index when necessary */
		if (info->rxIdx == LAST_PKTBUFSRX) {
			info->rxbd[PKTBUFSRX - 1].cbd_sc = BD_ENET_RX_W_E;
			info->rxIdx = 0;
		} else {
			info->rxbd[info->rxIdx].cbd_sc = BD_ENET_RX_EMPTY;
			info->rxIdx++;
		}

		/* Try to fill Buffer Descriptors */
		fecp->rdar = 0x01000000;	/* Descriptor polling active    */
	}

	return length;
}

#ifdef ET_DEBUG
void dbgFecRegs(struct eth_device *dev)
{
	struct fec_info_s *info = dev->priv;
	volatile fec_t *fecp = (fec_t *) (info->iobase);

	printf("=====\n");
	printf("ievent       %x - %x\n", (int)&fecp->eir, fecp->eir);
	printf("imask        %x - %x\n", (int)&fecp->eimr, fecp->eimr);
	printf("r_des_active %x - %x\n", (int)&fecp->rdar, fecp->rdar);
	printf("x_des_active %x - %x\n", (int)&fecp->tdar, fecp->tdar);
	printf("ecntrl       %x - %x\n", (int)&fecp->ecr, fecp->ecr);
	printf("mii_mframe   %x - %x\n", (int)&fecp->mmfr, fecp->mmfr);
	printf("mii_speed    %x - %x\n", (int)&fecp->mscr, fecp->mscr);
	printf("mii_ctrlstat %x - %x\n", (int)&fecp->mibc, fecp->mibc);
	printf("r_cntrl      %x - %x\n", (int)&fecp->rcr, fecp->rcr);
	printf("x_cntrl      %x - %x\n", (int)&fecp->tcr, fecp->tcr);
	printf("padr_l       %x - %x\n", (int)&fecp->palr, fecp->palr);
	printf("padr_u       %x - %x\n", (int)&fecp->paur, fecp->paur);
	printf("op_pause     %x - %x\n", (int)&fecp->opd, fecp->opd);
	printf("iadr_u       %x - %x\n", (int)&fecp->iaur, fecp->iaur);
	printf("iadr_l       %x - %x\n", (int)&fecp->ialr, fecp->ialr);
	printf("gadr_u       %x - %x\n", (int)&fecp->gaur, fecp->gaur);
	printf("gadr_l       %x - %x\n", (int)&fecp->galr, fecp->galr);
	printf("x_wmrk       %x - %x\n", (int)&fecp->tfwr, fecp->tfwr);
	printf("r_bound      %x - %x\n", (int)&fecp->frbr, fecp->frbr);
	printf("r_fstart     %x - %x\n", (int)&fecp->frsr, fecp->frsr);
	printf("r_drng       %x - %x\n", (int)&fecp->erdsr, fecp->erdsr);
	printf("x_drng       %x - %x\n", (int)&fecp->etdsr, fecp->etdsr);
	printf("r_bufsz      %x - %x\n", (int)&fecp->emrbr, fecp->emrbr);

	printf("\n");
	printf("rmon_t_drop        %x - %x\n", (int)&fecp->rmon_t_drop,
	       fecp->rmon_t_drop);
	printf("rmon_t_packets     %x - %x\n", (int)&fecp->rmon_t_packets,
	       fecp->rmon_t_packets);
	printf("rmon_t_bc_pkt      %x - %x\n", (int)&fecp->rmon_t_bc_pkt,
	       fecp->rmon_t_bc_pkt);
	printf("rmon_t_mc_pkt      %x - %x\n", (int)&fecp->rmon_t_mc_pkt,
	       fecp->rmon_t_mc_pkt);
	printf("rmon_t_crc_align   %x - %x\n", (int)&fecp->rmon_t_crc_align,
	       fecp->rmon_t_crc_align);
	printf("rmon_t_undersize   %x - %x\n", (int)&fecp->rmon_t_undersize,
	       fecp->rmon_t_undersize);
	printf("rmon_t_oversize    %x - %x\n", (int)&fecp->rmon_t_oversize,
	       fecp->rmon_t_oversize);
	printf("rmon_t_frag        %x - %x\n", (int)&fecp->rmon_t_frag,
	       fecp->rmon_t_frag);
	printf("rmon_t_jab         %x - %x\n", (int)&fecp->rmon_t_jab,
	       fecp->rmon_t_jab);
	printf("rmon_t_col         %x - %x\n", (int)&fecp->rmon_t_col,
	       fecp->rmon_t_col);
	printf("rmon_t_p64         %x - %x\n", (int)&fecp->rmon_t_p64,
	       fecp->rmon_t_p64);
	printf("rmon_t_p65to127    %x - %x\n", (int)&fecp->rmon_t_p65to127,
	       fecp->rmon_t_p65to127);
	printf("rmon_t_p128to255   %x - %x\n", (int)&fecp->rmon_t_p128to255,
	       fecp->rmon_t_p128to255);
	printf("rmon_t_p256to511   %x - %x\n", (int)&fecp->rmon_t_p256to511,
	       fecp->rmon_t_p256to511);
	printf("rmon_t_p512to1023  %x - %x\n", (int)&fecp->rmon_t_p512to1023,
	       fecp->rmon_t_p512to1023);
	printf("rmon_t_p1024to2047 %x - %x\n", (int)&fecp->rmon_t_p1024to2047,
	       fecp->rmon_t_p1024to2047);
	printf("rmon_t_p_gte2048   %x - %x\n", (int)&fecp->rmon_t_p_gte2048,
	       fecp->rmon_t_p_gte2048);
	printf("rmon_t_octets      %x - %x\n", (int)&fecp->rmon_t_octets,
	       fecp->rmon_t_octets);

	printf("\n");
	printf("ieee_t_drop      %x - %x\n", (int)&fecp->ieee_t_drop,
	       fecp->ieee_t_drop);
	printf("ieee_t_frame_ok  %x - %x\n", (int)&fecp->ieee_t_frame_ok,
	       fecp->ieee_t_frame_ok);
	printf("ieee_t_1col      %x - %x\n", (int)&fecp->ieee_t_1col,
	       fecp->ieee_t_1col);
	printf("ieee_t_mcol      %x - %x\n", (int)&fecp->ieee_t_mcol,
	       fecp->ieee_t_mcol);
	printf("ieee_t_def       %x - %x\n", (int)&fecp->ieee_t_def,
	       fecp->ieee_t_def);
	printf("ieee_t_lcol      %x - %x\n", (int)&fecp->ieee_t_lcol,
	       fecp->ieee_t_lcol);
	printf("ieee_t_excol     %x - %x\n", (int)&fecp->ieee_t_excol,
	       fecp->ieee_t_excol);
	printf("ieee_t_macerr    %x - %x\n", (int)&fecp->ieee_t_macerr,
	       fecp->ieee_t_macerr);
	printf("ieee_t_cserr     %x - %x\n", (int)&fecp->ieee_t_cserr,
	       fecp->ieee_t_cserr);
	printf("ieee_t_sqe       %x - %x\n", (int)&fecp->ieee_t_sqe,
	       fecp->ieee_t_sqe);
	printf("ieee_t_fdxfc     %x - %x\n", (int)&fecp->ieee_t_fdxfc,
	       fecp->ieee_t_fdxfc);
	printf("ieee_t_octets_ok %x - %x\n", (int)&fecp->ieee_t_octets_ok,
	       fecp->ieee_t_octets_ok);

	printf("\n");
	printf("rmon_r_drop        %x - %x\n", (int)&fecp->rmon_r_drop,
	       fecp->rmon_r_drop);
	printf("rmon_r_packets     %x - %x\n", (int)&fecp->rmon_r_packets,
	       fecp->rmon_r_packets);
	printf("rmon_r_bc_pkt      %x - %x\n", (int)&fecp->rmon_r_bc_pkt,
	       fecp->rmon_r_bc_pkt);
	printf("rmon_r_mc_pkt      %x - %x\n", (int)&fecp->rmon_r_mc_pkt,
	       fecp->rmon_r_mc_pkt);
	printf("rmon_r_crc_align   %x - %x\n", (int)&fecp->rmon_r_crc_align,
	       fecp->rmon_r_crc_align);
	printf("rmon_r_undersize   %x - %x\n", (int)&fecp->rmon_r_undersize,
	       fecp->rmon_r_undersize);
	printf("rmon_r_oversize    %x - %x\n", (int)&fecp->rmon_r_oversize,
	       fecp->rmon_r_oversize);
	printf("rmon_r_frag        %x - %x\n", (int)&fecp->rmon_r_frag,
	       fecp->rmon_r_frag);
	printf("rmon_r_jab         %x - %x\n", (int)&fecp->rmon_r_jab,
	       fecp->rmon_r_jab);
	printf("rmon_r_p64         %x - %x\n", (int)&fecp->rmon_r_p64,
	       fecp->rmon_r_p64);
	printf("rmon_r_p65to127    %x - %x\n", (int)&fecp->rmon_r_p65to127,
	       fecp->rmon_r_p65to127);
	printf("rmon_r_p128to255   %x - %x\n", (int)&fecp->rmon_r_p128to255,
	       fecp->rmon_r_p128to255);
	printf("rmon_r_p256to511   %x - %x\n", (int)&fecp->rmon_r_p256to511,
	       fecp->rmon_r_p256to511);
	printf("rmon_r_p512to1023  %x - %x\n", (int)&fecp->rmon_r_p512to1023,
	       fecp->rmon_r_p512to1023);
	printf("rmon_r_p1024to2047 %x - %x\n", (int)&fecp->rmon_r_p1024to2047,
	       fecp->rmon_r_p1024to2047);
	printf("rmon_r_p_gte2048   %x - %x\n", (int)&fecp->rmon_r_p_gte2048,
	       fecp->rmon_r_p_gte2048);
	printf("rmon_r_octets      %x - %x\n", (int)&fecp->rmon_r_octets,
	       fecp->rmon_r_octets);

	printf("\n");
	printf("ieee_r_drop      %x - %x\n", (int)&fecp->ieee_r_drop,
	       fecp->ieee_r_drop);
	printf("ieee_r_frame_ok  %x - %x\n", (int)&fecp->ieee_r_frame_ok,
	       fecp->ieee_r_frame_ok);
	printf("ieee_r_crc       %x - %x\n", (int)&fecp->ieee_r_crc,
	       fecp->ieee_r_crc);
	printf("ieee_r_align     %x - %x\n", (int)&fecp->ieee_r_align,
	       fecp->ieee_r_align);
	printf("ieee_r_macerr    %x - %x\n", (int)&fecp->ieee_r_macerr,
	       fecp->ieee_r_macerr);
	printf("ieee_r_fdxfc     %x - %x\n", (int)&fecp->ieee_r_fdxfc,
	       fecp->ieee_r_fdxfc);
	printf("ieee_r_octets_ok %x - %x\n", (int)&fecp->ieee_r_octets_ok,
	       fecp->ieee_r_octets_ok);

	printf("\n\n\n");
}
#endif

int fec_init(struct eth_device *dev, bd_t * bd)
{
	struct fec_info_s *info = dev->priv;
	volatile fec_t *fecp = (fec_t *) (info->iobase);
	int i;
	uchar ea[6];

	fecpin_setclear(dev, 1);

	fec_reset(dev);

#if defined(CONFIG_CMD_MII) || defined (CONFIG_MII) || \
	defined (CONFIG_SYS_DISCOVER_PHY)

	mii_init();

	setFecDuplexSpeed(fecp, bd, info->dup_spd);
#else
#ifndef CONFIG_SYS_DISCOVER_PHY
	setFecDuplexSpeed(fecp, bd, (FECDUPLEX << 16) | FECSPEED);
#endif				/* ifndef CONFIG_SYS_DISCOVER_PHY */
#endif				/* CONFIG_CMD_MII || CONFIG_MII */

	/* We use strictly polling mode only */
	fecp->eimr = 0;

	/* Clear any pending interrupt */
	fecp->eir = 0xffffffff;

	/* Set station address   */
	if ((u32) fecp == CONFIG_SYS_FEC0_IOBASE) {
#ifdef CONFIG_SYS_FEC1_IOBASE
		volatile fec_t *fecp1 = (fec_t *) (CONFIG_SYS_FEC1_IOBASE);
		eth_env_get_enetaddr("eth1addr", ea);
		fecp1->palr =
		    (ea[0] << 24) | (ea[1] << 16) | (ea[2] << 8) | (ea[3]);
		fecp1->paur = (ea[4] << 24) | (ea[5] << 16);
#endif
		eth_env_get_enetaddr("ethaddr", ea);
		fecp->palr =
		    (ea[0] << 24) | (ea[1] << 16) | (ea[2] << 8) | (ea[3]);
		fecp->paur = (ea[4] << 24) | (ea[5] << 16);
	} else {
#ifdef CONFIG_SYS_FEC0_IOBASE
		volatile fec_t *fecp0 = (fec_t *) (CONFIG_SYS_FEC0_IOBASE);
		eth_env_get_enetaddr("ethaddr", ea);
		fecp0->palr =
		    (ea[0] << 24) | (ea[1] << 16) | (ea[2] << 8) | (ea[3]);
		fecp0->paur = (ea[4] << 24) | (ea[5] << 16);
#endif
#ifdef CONFIG_SYS_FEC1_IOBASE
		eth_env_get_enetaddr("eth1addr", ea);
		fecp->palr =
		    (ea[0] << 24) | (ea[1] << 16) | (ea[2] << 8) | (ea[3]);
		fecp->paur = (ea[4] << 24) | (ea[5] << 16);
#endif
	}

	/* Clear unicast address hash table */
	fecp->iaur = 0;
	fecp->ialr = 0;

	/* Clear multicast address hash table */
	fecp->gaur = 0;
	fecp->galr = 0;

	/* Set maximum receive buffer size. */
	fecp->emrbr = PKT_MAXBLR_SIZE;

	/*
	 * Setup Buffers and Buffer Descriptors
	 */
	info->rxIdx = 0;
	info->txIdx = 0;

	/*
	 * Setup Receiver Buffer Descriptors (13.14.24.18)
	 * Settings:
	 *     Empty, Wrap
	 */
	for (i = 0; i < PKTBUFSRX; i++) {
		info->rxbd[i].cbd_sc = BD_ENET_RX_EMPTY;
		info->rxbd[i].cbd_datlen = 0;	/* Reset */
		info->rxbd[i].cbd_bufaddr = (uint) net_rx_packets[i];
	}
	info->rxbd[PKTBUFSRX - 1].cbd_sc |= BD_ENET_RX_WRAP;

	/*
	 * Setup Ethernet Transmitter Buffer Descriptors (13.14.24.19)
	 * Settings:
	 *    Last, Tx CRC
	 */
	for (i = 0; i < TX_BUF_CNT; i++) {
		info->txbd[i].cbd_sc = BD_ENET_TX_LAST | BD_ENET_TX_TC;
		info->txbd[i].cbd_datlen = 0;	/* Reset */
		info->txbd[i].cbd_bufaddr = (uint) (&info->txbuf[0]);
	}
	info->txbd[TX_BUF_CNT - 1].cbd_sc |= BD_ENET_TX_WRAP;

	/* Set receive and transmit descriptor base */
	fecp->erdsr = (unsigned int)(&info->rxbd[0]);
	fecp->etdsr = (unsigned int)(&info->txbd[0]);

	/* Now enable the transmit and receive processing */
	fecp->ecr |= FEC_ECR_ETHER_EN;

	/* And last, try to fill Rx Buffer Descriptors */
	fecp->rdar = 0x01000000;	/* Descriptor polling active    */

	return 1;
}

void fec_reset(struct eth_device *dev)
{
	struct fec_info_s *info = dev->priv;
	volatile fec_t *fecp = (fec_t *) (info->iobase);
	int i;

	fecp->ecr = FEC_ECR_RESET;
	for (i = 0; (fecp->ecr & FEC_ECR_RESET) && (i < FEC_RESET_DELAY); ++i) {
		udelay(1);
	}
	if (i == FEC_RESET_DELAY) {
		printf("FEC_RESET_DELAY timeout\n");
	}
}

void fec_halt(struct eth_device *dev)
{
	struct fec_info_s *info = dev->priv;

	fec_reset(dev);

	fecpin_setclear(dev, 0);

	info->rxIdx = info->txIdx = 0;
	memset(info->rxbd, 0, PKTBUFSRX * sizeof(cbd_t));
	memset(info->txbd, 0, TX_BUF_CNT * sizeof(cbd_t));
	memset(info->txbuf, 0, DBUF_LENGTH);
}

int mcffec_initialize(bd_t * bis)
{
	struct eth_device *dev;
	int i;
#ifdef CONFIG_SYS_FEC_BUF_USE_SRAM
	u32 tmp = CONFIG_SYS_INIT_RAM_ADDR + 0x1000;
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
#ifdef CONFIG_SYS_FEC_BUF_USE_SRAM
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
				       (TX_BUF_CNT * sizeof(cbd_t)));
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
