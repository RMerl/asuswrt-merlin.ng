/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
 */

#ifndef __DTSEC_H__
#define __DTSEC_H__

#include <asm/types.h>

struct dtsec {
	u32	tsec_id;	/* controller ID and version */
	u32	tsec_id2;	/* controller ID and configuration */
	u32	ievent;		/* interrupt event */
	u32	imask;		/* interrupt mask */
	u32	res0;
	u32	ecntrl;		/* ethernet control and configuration */
	u32	ptv;		/* pause time value */
	u32	tbipa;		/* TBI PHY address */
	u32	res1[8];
	u32	tctrl;		/* Transmit control register */
	u32	res2[3];
	u32	rctrl;		/* Receive control register */
	u32	res3[11];
	u32	igaddr[8];	/* Individual group address */
	u32	gaddr[8];	/* group address */
	u32	res4[16];
	u32	maccfg1;	/* MAC configuration register 1 */
	u32	maccfg2;	/* MAC configuration register 2 */
	u32	ipgifg;		/* inter-packet/inter-frame gap */
	u32	hafdup;		/* half-duplex control */
	u32	maxfrm;		/* Maximum frame size */
	u32	res5[3];
	u32	miimcfg;	/* MII management configuration */
	u32	miimcom;	/* MII management command */
	u32	miimadd;	/* MII management address */
	u32	miimcon;	/* MII management control */
	u32	miimstat;	/* MII management status */
	u32	miimind;	/* MII management indicator */
	u32	res6;
	u32	ifstat;		/* Interface status */
	u32	macstnaddr1;	/* MAC station address 1 */
	u32	macstnaddr2;	/* MAC station address 2 */
	u32	res7[46];
	/* transmit and receive counter */
	u32	tr64;		/* Tx and Rx 64 bytes frame */
	u32	tr127;		/* Tx and Rx 65 to 127 bytes frame */
	u32	tr255;		/* Tx and Rx 128 to 255 bytes frame */
	u32	tr511;		/* Tx and Rx 256 to 511 bytes frame */
	u32	tr1k;		/* Tx and Rx 512 to 1023 bytes frame */
	u32	trmax;		/* Tx and Rx 1024 to 1518 bytes frame */
	u32	trmgv;		/* Tx and Rx 1519 to 1522 good VLAN frame */
	/* receive counters */
	u32	rbyt;		/* Receive byte counter */
	u32	rpkt;		/* Receive packet counter */
	u32	rfcs;		/* Receive FCS error */
	u32	rmca;		/* Receive multicast packet */
	u32	rbca;		/* Receive broadcast packet */
	u32	rxcf;		/* Receive control frame */
	u32	rxpf;		/* Receive pause frame */
	u32	rxuo;		/* Receive unknown OP code */
	u32	raln;		/* Receive alignment error */
	u32	rflr;		/* Receive frame length error */
	u32	rcde;		/* Receive code error */
	u32	rcse;		/* Receive carrier sense error */
	u32	rund;		/* Receive undersize packet */
	u32	rovr;		/* Receive oversize packet */
	u32	rfrg;		/* Receive fragments counter */
	u32	rjbr;		/* Receive jabber counter */
	u32	rdrp;		/* Receive drop counter */
	/* transmit counters */
	u32	tbyt;		/* Transmit byte counter */
	u32	tpkt;		/* Transmit packet */
	u32	tmca;		/* Transmit multicast packet */
	u32	tbca;		/* Transmit broadcast packet */
	u32	txpf;		/* Transmit pause control frame */
	u32	tdfr;		/* Transmit deferral packet */
	u32	tedf;		/* Transmit excessive deferral pkt */
	u32	tscl;		/* Transmit single collision pkt */
	u32	tmcl;		/* Transmit multiple collision pkt */
	u32	tlcl;		/* Transmit late collision pkt */
	u32	txcl;		/* Transmit excessive collision */
	u32	tncl;		/* Transmit total collision */
	u32	res8;
	u32	tdrp;		/* Transmit drop frame */
	u32	tjbr;		/* Transmit jabber frame */
	u32	tfcs;		/* Transmit FCS error */
	u32	txcf;		/* Transmit control frame */
	u32	tovr;		/* Transmit oversize frame */
	u32	tund;		/* Transmit undersize frame */
	u32	tfrg;		/* Transmit fragments frame */
	/* counter controls */
	u32	car1;		/* carry register 1 */
	u32	car2;		/* carry register 2 */
	u32	cam1;		/* carry register 1 mask */
	u32	cam2;		/* carry register 2 mask */
	u32	res9[80];
};


/* TBI register addresses */
#define TBI_CR			0x00
#define TBI_SR			0x01
#define TBI_ANA			0x04
#define TBI_ANLPBPA		0x05
#define TBI_ANEX		0x06
#define TBI_TBICON		0x11

/* TBI MDIO register bit fields*/
#define TBICON_CLK_SELECT	0x0020
#define TBIANA_ASYMMETRIC_PAUSE 0x0100
#define TBIANA_SYMMETRIC_PAUSE  0x0080
#define TBIANA_HALF_DUPLEX	0x0040
#define TBIANA_FULL_DUPLEX	0x0020
#define TBICR_PHY_RESET		0x8000
#define TBICR_ANEG_ENABLE	0x1000
#define TBICR_RESTART_ANEG	0x0200
#define TBICR_FULL_DUPLEX	0x0100
#define TBICR_SPEED1_SET	0x0040

/* IEVENT - interrupt events register */
#define IEVENT_BABR	0x80000000 /* Babbling receive error */
#define IEVENT_RXC	0x40000000 /* pause control frame received */
#define IEVENT_MSRO	0x04000000 /* MIB counter overflow */
#define IEVENT_GTSC	0x02000000 /* Graceful transmit stop complete */
#define IEVENT_BABT	0x01000000 /* Babbling transmit error */
#define IEVENT_TXC	0x00800000 /* control frame transmitted */
#define IEVENT_TXE	0x00400000 /* Transmit channel error */
#define IEVENT_LC	0x00040000 /* Late collision occurred */
#define IEVENT_CRL	0x00020000 /* Collision retry exceed limit */
#define IEVENT_XFUN	0x00010000 /* Transmit FIFO underrun */
#define IEVENT_ABRT	0x00008000 /* Transmit packet abort */
#define IEVENT_MMRD	0x00000400 /* MII management read complete */
#define IEVENT_MMWR	0x00000200 /* MII management write complete */
#define IEVENT_GRSC	0x00000100 /* Graceful stop complete */
#define IEVENT_TDPE	0x00000002 /* Internal data parity error on Tx */
#define IEVENT_RDPE	0x00000001 /* Internal data parity error on Rx */

#define IEVENT_CLEAR_ALL	0xffffffff

/* IMASK - interrupt mask register */
#define IMASK_BREN	0x80000000 /* Babbling receive enable */
#define IMASK_RXCEN	0x40000000 /* receive control enable */
#define IMASK_MSROEN	0x04000000 /* MIB counter overflow enable */
#define IMASK_GTSCEN	0x02000000 /* Graceful Tx stop complete enable */
#define IMASK_BTEN	0x01000000 /* Babbling transmit error enable */
#define IMASK_TXCEN	0x00800000 /* control frame transmitted enable */
#define IMASK_TXEEN	0x00400000 /* Transmit channel error enable */
#define IMASK_LCEN	0x00040000 /* Late collision interrupt enable */
#define IMASK_CRLEN	0x00020000 /* Collision retry exceed limit */
#define IMASK_XFUNEN	0x00010000 /* Transmit FIFO underrun enable */
#define IMASK_ABRTEN	0x00008000 /* Transmit packet abort enable */
#define IMASK_MMRDEN	0x00000400 /* MII management read complete enable */
#define IMASK_MMWREN	0x00000200 /* MII management write complete enable */
#define IMASK_GRSCEN	0x00000100 /* Graceful stop complete interrupt enable */
#define IMASK_TDPEEN	0x00000002 /* Internal data parity error on Tx enable */
#define IMASK_RDPEEN	0x00000001 /* Internal data parity error on Rx enable */

#define IMASK_MASK_ALL	0x00000000

/* ECNTRL - ethernet control register */
#define ECNTRL_CFG_RO	0x80000000 /* GMIIM, RPM, R100M, SGMIIM bits are RO */
#define ECNTRL_CLRCNT	0x00004000 /* clear all statistics */
#define ECNTRL_AUTOZ	0x00002000 /* auto zero MIB counter */
#define ECNTRL_STEN	0x00001000 /* enable internal counters to update */
#define ECNTRL_GMIIM	0x00000040 /* 1- GMII or RGMII interface mode */
#define ECNTRL_TBIM	0x00000020 /* 1- Ten-bit interface mode */
#define ECNTRL_RPM	0x00000010 /* 1- RGMII reduced-pin mode */
#define ECNTRL_R100M	0x00000008 /* 1- RGMII 100 Mbps, SGMII 100 Mbps
				      0- RGMII 10 Mbps, SGMII 10 Mbps */
#define ECNTRL_SGMIIM	0x00000002 /* 1- SGMII interface mode */
#define ECNTRL_TBIM	0x00000020 /* 1- TBI Interface mode (for SGMII) */

#define ECNTRL_DEFAULT	(ECNTRL_TBIM | ECNTRL_R100M | ECNTRL_SGMIIM)

/* TCTRL - Transmit control register */
#define TCTRL_THDF	0x00000800 /* Transmit half-duplex flow control */
#define TCTRL_TTSE	0x00000040 /* Transmit time-stamp enable */
#define TCTRL_GTS	0x00000020 /* Graceful transmit stop */
#define TCTRL_RFC_PAUSE	0x00000010 /* Receive flow control pause frame */

/* RCTRL - Receive control register */
#define RCTRL_PAL_MASK	0x001f0000 /* packet alignment padding length */
#define RCTRL_PAL_SHIFT	16
#define RCTRL_CFA	0x00008000 /* control frame accept enable */
#define RCTRL_GHTX	0x00000800 /* group address hash table extend */
#define RCTRL_RTSE	0x00000040 /* receive 1588 time-stamp enable */
#define RCTRL_GRS	0x00000020 /* graceful receive stop */
#define RCTRL_BC_REJ	0x00000010 /* broadcast frame reject */
#define RCTRL_BC_MPROM	0x00000008 /* all multicast/broadcast frames received */
#define RCTRL_RSF	0x00000004 /* receive short frame(17~63 bytes) enable */
#define RCTRL_EMEN	0x00000002 /* Exact match MAC address enable */
#define RCTRL_UPROM	0x00000001 /* all unicast frame received */

/* MACCFG1 - MAC configuration 1 register */
#define MACCFG1_SOFT_RST	0x80000000 /* place the MAC in reset */
#define MACCFG1_RST_RXMAC	0x00080000 /* reset receive MAC control block */
#define MACCFG1_RST_TXMAC	0x00040000 /* reet transmit MAC control block */
#define MACCFG1_RST_RXFUN	0x00020000 /* reset receive function block */
#define MACCFG1_RST_TXFUN	0x00010000 /* reset transmit function block */
#define MACCFG1_LOOPBACK	0x00000100 /* MAC loopback */
#define MACCFG1_RX_FLOW		0x00000020 /* Receive flow */
#define MACCFG1_TX_FLOW		0x00000010 /* Transmit flow */
#define MACCFG1_SYNC_RXEN	0x00000008 /* Frame reception enabled */
#define MACCFG1_RX_EN		0x00000004 /* Rx enable */
#define MACCFG1_SYNC_TXEN	0x00000002 /* Frame transmission is enabled */
#define MACCFG1_TX_EN		0x00000001 /* Tx enable */
#define MACCFG1_RXTX_EN		(MACCFG1_RX_EN | MACCFG1_TX_EN)

/* MACCFG2 - MAC configuration 2 register */
#define MACCFG2_PRE_LEN_MASK	0x0000f000 /* preamble length */
#define MACCFG2_PRE_LEN(x)	((x << 12) & MACCFG2_PRE_LEN_MASK)
#define MACCFG2_IF_MODE_MASK	0x00000300
#define MACCFG2_IF_MODE_NIBBLE	0x00000100 /* MII, 10/100 Mbps MII/RMII */
#define MACCFG2_IF_MODE_BYTE	0x00000200 /* GMII/TBI, 1000 GMII/TBI */
#define MACCFG2_PRE_RX_EN	0x00000080 /* receive preamble enable */
#define MACCFG2_PRE_TX_EN	0x00000040 /* tx preable enable */
#define MACCFG2_HUGE_FRAME	0x00000020 /* >= max frame len enable */
#define MACCFG2_LEN_CHECK	0x00000010 /* MAC check frame's length Rx */
#define MACCFG2_MAG_EN		0x00000008 /* magic packet enable */
#define MACCFG2_PAD_CRC		0x00000004 /* pad and append CRC */
#define MACCFG2_CRC_EN		0x00000002 /* MAC appends a CRC on all frames */
#define MACCFG2_FULL_DUPLEX	0x00000001 /* Full deplex mode */

struct fsl_enet_mac;

void init_dtsec(struct fsl_enet_mac *mac, void *base, void *phyregs,
		int max_rx_len);

#endif
