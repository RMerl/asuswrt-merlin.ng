/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  tsec.h
 *
 *  Driver for the Motorola Triple Speed Ethernet Controller
 *
 * Copyright 2004, 2007, 2009, 2011, 2013 Freescale Semiconductor, Inc.
 * (C) Copyright 2003, Motorola, Inc.
 * maintained by Xianghua Xiao (x.xiao@motorola.com)
 * author Andy Fleming
 */

#ifndef __TSEC_H
#define __TSEC_H

#include <net.h>
#include <config.h>
#include <phy.h>

#ifndef CONFIG_DM_ETH

#ifdef CONFIG_ARCH_LS1021A
#define TSEC_SIZE		0x40000
#define TSEC_MDIO_OFFSET	0x40000
#else
#define TSEC_SIZE 		0x01000
#define TSEC_MDIO_OFFSET	0x01000
#endif

#define CONFIG_SYS_MDIO_BASE_ADDR (MDIO_BASE_ADDR + 0x520)

#define TSEC_GET_REGS(num, offset) \
	(struct tsec __iomem *)\
	(TSEC_BASE_ADDR + (((num) - 1) * (offset)))

#define TSEC_GET_REGS_BASE(num) \
	TSEC_GET_REGS((num), TSEC_SIZE)

#define TSEC_GET_MDIO_REGS(num, offset) \
	(struct tsec_mii_mng __iomem *)\
	(CONFIG_SYS_MDIO_BASE_ADDR  + ((num) - 1) * (offset))

#define TSEC_GET_MDIO_REGS_BASE(num) \
	TSEC_GET_MDIO_REGS((num), TSEC_MDIO_OFFSET)

#define DEFAULT_MII_NAME "FSL_MDIO"

#define STD_TSEC_INFO(num) \
{			\
	.regs = TSEC_GET_REGS_BASE(num), \
	.miiregs_sgmii = TSEC_GET_MDIO_REGS_BASE(num), \
	.devname = CONFIG_TSEC##num##_NAME, \
	.phyaddr = TSEC##num##_PHY_ADDR, \
	.flags = TSEC##num##_FLAGS, \
	.mii_devname = DEFAULT_MII_NAME \
}

#define SET_STD_TSEC_INFO(x, num) \
{			\
	x.regs = TSEC_GET_REGS_BASE(num); \
	x.miiregs_sgmii = TSEC_GET_MDIO_REGS_BASE(num); \
	x.devname = CONFIG_TSEC##num##_NAME; \
	x.phyaddr = TSEC##num##_PHY_ADDR; \
	x.flags = TSEC##num##_FLAGS;\
	x.mii_devname = DEFAULT_MII_NAME;\
}

#endif /* CONFIG_DM_ETH */

#define MAC_ADDR_LEN		6

/* #define TSEC_TIMEOUT	1000000 */
#define TSEC_TIMEOUT		1000
#define TOUT_LOOP		1000000

/* TBI register addresses */
#define TBI_CR			0x00
#define TBI_SR			0x01
#define TBI_ANA			0x04
#define TBI_ANLPBPA		0x05
#define TBI_ANEX		0x06
#define TBI_TBICON		0x11

/* TBI MDIO register bit fields*/
#define TBICON_CLK_SELECT	0x0020
#define TBIANA_ASYMMETRIC_PAUSE	0x0100
#define TBIANA_SYMMETRIC_PAUSE	0x0080
#define TBIANA_HALF_DUPLEX	0x0040
#define TBIANA_FULL_DUPLEX	0x0020
#define TBICR_PHY_RESET		0x8000
#define TBICR_ANEG_ENABLE	0x1000
#define TBICR_RESTART_ANEG	0x0200
#define TBICR_FULL_DUPLEX	0x0100
#define TBICR_SPEED1_SET	0x0040

/* MAC register bits */
#define MACCFG1_SOFT_RESET	0x80000000
#define MACCFG1_RESET_RX_MC	0x00080000
#define MACCFG1_RESET_TX_MC	0x00040000
#define MACCFG1_RESET_RX_FUN	0x00020000
#define MACCFG1_RESET_TX_FUN	0x00010000
#define MACCFG1_LOOPBACK	0x00000100
#define MACCFG1_RX_FLOW		0x00000020
#define MACCFG1_TX_FLOW		0x00000010
#define MACCFG1_SYNCD_RX_EN	0x00000008
#define MACCFG1_RX_EN		0x00000004
#define MACCFG1_SYNCD_TX_EN	0x00000002
#define MACCFG1_TX_EN		0x00000001

#define MACCFG2_INIT_SETTINGS	0x00007205
#define MACCFG2_FULL_DUPLEX	0x00000001
#define MACCFG2_IF		0x00000300
#define MACCFG2_GMII		0x00000200
#define MACCFG2_MII		0x00000100

#define ECNTRL_INIT_SETTINGS	0x00001000
#define ECNTRL_TBI_MODE		0x00000020
#define ECNTRL_REDUCED_MODE	0x00000010
#define ECNTRL_R100		0x00000008
#define ECNTRL_REDUCED_MII_MODE	0x00000004
#define ECNTRL_SGMII_MODE	0x00000002

#ifndef CONFIG_SYS_TBIPA_VALUE
# define CONFIG_SYS_TBIPA_VALUE	0x1f
#endif

#define MRBLR_INIT_SETTINGS	PKTSIZE_ALIGN

#define MINFLR_INIT_SETTINGS	0x00000040

#define DMACTRL_INIT_SETTINGS	0x000000c3
#define DMACTRL_GRS		0x00000010
#define DMACTRL_GTS		0x00000008
#define DMACTRL_LE		0x00008000

#define TSTAT_CLEAR_THALT	0x80000000
#define RSTAT_CLEAR_RHALT	0x00800000

#define IEVENT_INIT_CLEAR	0xffffffff
#define IEVENT_BABR		0x80000000
#define IEVENT_RXC		0x40000000
#define IEVENT_BSY		0x20000000
#define IEVENT_EBERR		0x10000000
#define IEVENT_MSRO		0x04000000
#define IEVENT_GTSC		0x02000000
#define IEVENT_BABT		0x01000000
#define IEVENT_TXC		0x00800000
#define IEVENT_TXE		0x00400000
#define IEVENT_TXB		0x00200000
#define IEVENT_TXF		0x00100000
#define IEVENT_IE		0x00080000
#define IEVENT_LC		0x00040000
#define IEVENT_CRL		0x00020000
#define IEVENT_XFUN		0x00010000
#define IEVENT_RXB0		0x00008000
#define IEVENT_GRSC		0x00000100
#define IEVENT_RXF0		0x00000080

#define IMASK_INIT_CLEAR	0x00000000
#define IMASK_TXEEN		0x00400000
#define IMASK_TXBEN		0x00200000
#define IMASK_TXFEN		0x00100000
#define IMASK_RXFEN0		0x00000080

/* Default Attribute fields */
#define ATTR_INIT_SETTINGS	0x000000c0
#define ATTRELI_INIT_SETTINGS	0x00000000

/* TxBD status field bits */
#define TXBD_READY		0x8000
#define TXBD_PADCRC		0x4000
#define TXBD_WRAP		0x2000
#define TXBD_INTERRUPT		0x1000
#define TXBD_LAST		0x0800
#define TXBD_CRC		0x0400
#define TXBD_DEF		0x0200
#define TXBD_HUGEFRAME		0x0080
#define TXBD_LATECOLLISION	0x0080
#define TXBD_RETRYLIMIT		0x0040
#define TXBD_RETRYCOUNTMASK	0x003c
#define TXBD_UNDERRUN		0x0002
#define TXBD_STATS		0x03ff

/* RxBD status field bits */
#define RXBD_EMPTY		0x8000
#define RXBD_RO1		0x4000
#define RXBD_WRAP		0x2000
#define RXBD_INTERRUPT		0x1000
#define RXBD_LAST		0x0800
#define RXBD_FIRST		0x0400
#define RXBD_MISS		0x0100
#define RXBD_BROADCAST		0x0080
#define RXBD_MULTICAST		0x0040
#define RXBD_LARGE		0x0020
#define RXBD_NONOCTET		0x0010
#define RXBD_SHORT		0x0008
#define RXBD_CRCERR		0x0004
#define RXBD_OVERRUN		0x0002
#define RXBD_TRUNCATED		0x0001
#define RXBD_STATS		0x003f

struct txbd8 {
	uint16_t status;	/* Status Fields */
	uint16_t length;	/* Buffer length */
	uint32_t bufptr;	/* Buffer Pointer */
};

struct rxbd8 {
	uint16_t status;	/* Status Fields */
	uint16_t length;	/* Buffer Length */
	uint32_t bufptr;	/* Buffer Pointer */
};

struct tsec_rmon_mib {
	/* Transmit and Receive Counters */
	u32	tr64;		/* Tx/Rx 64-byte Frame Counter */
	u32	tr127;		/* Tx/Rx 65-127 byte Frame Counter */
	u32	tr255;		/* Tx/Rx 128-255 byte Frame Counter */
	u32	tr511;		/* Tx/Rx 256-511 byte Frame Counter */
	u32	tr1k;		/* Tx/Rx 512-1023 byte Frame Counter */
	u32	trmax;		/* Tx/Rx 1024-1518 byte Frame Counter */
	u32	trmgv;		/* Tx/Rx 1519-1522 byte Good VLAN Frame */
	/* Receive Counters */
	u32	rbyt;		/* Receive Byte Counter */
	u32	rpkt;		/* Receive Packet Counter */
	u32	rfcs;		/* Receive FCS Error Counter */
	u32	rmca;		/* Receive Multicast Packet (Counter) */
	u32	rbca;		/* Receive Broadcast Packet */
	u32	rxcf;		/* Receive Control Frame Packet */
	u32	rxpf;		/* Receive Pause Frame Packet */
	u32	rxuo;		/* Receive Unknown OP Code */
	u32	raln;		/* Receive Alignment Error */
	u32	rflr;		/* Receive Frame Length Error */
	u32	rcde;		/* Receive Code Error */
	u32	rcse;		/* Receive Carrier Sense Error */
	u32	rund;		/* Receive Undersize Packet */
	u32	rovr;		/* Receive Oversize Packet */
	u32	rfrg;		/* Receive Fragments */
	u32	rjbr;		/* Receive Jabber */
	u32	rdrp;		/* Receive Drop */
	/* Transmit Counters */
	u32	tbyt;		/* Transmit Byte Counter */
	u32	tpkt;		/* Transmit Packet */
	u32	tmca;		/* Transmit Multicast Packet */
	u32	tbca;		/* Transmit Broadcast Packet */
	u32	txpf;		/* Transmit Pause Control Frame */
	u32	tdfr;		/* Transmit Deferral Packet */
	u32	tedf;		/* Transmit Excessive Deferral Packet */
	u32	tscl;		/* Transmit Single Collision Packet */
	/* (0x2_n700) */
	u32	tmcl;		/* Transmit Multiple Collision Packet */
	u32	tlcl;		/* Transmit Late Collision Packet */
	u32	txcl;		/* Transmit Excessive Collision Packet */
	u32	tncl;		/* Transmit Total Collision */

	u32	res2;

	u32	tdrp;		/* Transmit Drop Frame */
	u32	tjbr;		/* Transmit Jabber Frame */
	u32	tfcs;		/* Transmit FCS Error */
	u32	txcf;		/* Transmit Control Frame */
	u32	tovr;		/* Transmit Oversize Frame */
	u32	tund;		/* Transmit Undersize Frame */
	u32	tfrg;		/* Transmit Fragments Frame */
	/* General Registers */
	u32	car1;		/* Carry Register One */
	u32	car2;		/* Carry Register Two */
	u32	cam1;		/* Carry Register One Mask */
	u32	cam2;		/* Carry Register Two Mask */
};

struct tsec_hash_regs {
	u32	iaddr0;		/* Individual Address Register 0 */
	u32	iaddr1;		/* Individual Address Register 1 */
	u32	iaddr2;		/* Individual Address Register 2 */
	u32	iaddr3;		/* Individual Address Register 3 */
	u32	iaddr4;		/* Individual Address Register 4 */
	u32	iaddr5;		/* Individual Address Register 5 */
	u32	iaddr6;		/* Individual Address Register 6 */
	u32	iaddr7;		/* Individual Address Register 7 */
	u32	res1[24];
	u32	gaddr0;		/* Group Address Register 0 */
	u32	gaddr1;		/* Group Address Register 1 */
	u32	gaddr2;		/* Group Address Register 2 */
	u32	gaddr3;		/* Group Address Register 3 */
	u32	gaddr4;		/* Group Address Register 4 */
	u32	gaddr5;		/* Group Address Register 5 */
	u32	gaddr6;		/* Group Address Register 6 */
	u32	gaddr7;		/* Group Address Register 7 */
	u32	res2[24];
};

struct tsec {
	/* General Control and Status Registers (0x2_n000) */
	u32	res000[4];

	u32	ievent;		/* Interrupt Event */
	u32	imask;		/* Interrupt Mask */
	u32	edis;		/* Error Disabled */
	u32	res01c;
	u32	ecntrl;		/* Ethernet Control */
	u32	minflr;		/* Minimum Frame Length */
	u32	ptv;		/* Pause Time Value */
	u32	dmactrl;	/* DMA Control */
	u32	tbipa;		/* TBI PHY Address */

	u32	res034[3];
	u32	res040[48];

	/* Transmit Control and Status Registers (0x2_n100) */
	u32	tctrl;		/* Transmit Control */
	u32	tstat;		/* Transmit Status */
	u32	res108;
	u32	tbdlen;		/* Tx BD Data Length */
	u32	res110[5];
	u32	ctbptr;		/* Current TxBD Pointer */
	u32	res128[23];
	u32	tbptr;		/* TxBD Pointer */
	u32	res188[30];
	/* (0x2_n200) */
	u32	res200;
	u32	tbase;		/* TxBD Base Address */
	u32	res208[42];
	u32	ostbd;		/* Out of Sequence TxBD */
	u32	ostbdp;		/* Out of Sequence Tx Data Buffer Pointer */
	u32	res2b8[18];

	/* Receive Control and Status Registers (0x2_n300) */
	u32	rctrl;		/* Receive Control */
	u32	rstat;		/* Receive Status */
	u32	res308;
	u32	rbdlen;		/* RxBD Data Length */
	u32	res310[4];
	u32	res320;
	u32	crbptr;		/* Current Receive Buffer Pointer */
	u32	res328[6];
	u32	mrblr;		/* Maximum Receive Buffer Length */
	u32	res344[16];
	u32	rbptr;		/* RxBD Pointer */
	u32	res388[30];
	/* (0x2_n400) */
	u32	res400;
	u32	rbase;		/* RxBD Base Address */
	u32	res408[62];

	/* MAC Registers (0x2_n500) */
	u32	maccfg1;	/* MAC Configuration #1 */
	u32	maccfg2;	/* MAC Configuration #2 */
	u32	ipgifg;		/* Inter Packet Gap/Inter Frame Gap */
	u32	hafdup;		/* Half-duplex */
	u32	maxfrm;		/* Maximum Frame */
	u32	res514;
	u32	res518;

	u32	res51c;

	u32	resmdio[6];

	u32	res538;

	u32	ifstat;		/* Interface Status */
	u32	macstnaddr1;	/* Station Address, part 1 */
	u32	macstnaddr2;	/* Station Address, part 2 */
	u32	res548[46];

	/* (0x2_n600) */
	u32	res600[32];

	/* RMON MIB Registers (0x2_n680-0x2_n73c) */
	struct tsec_rmon_mib	rmon;
	u32	res740[48];

	/* Hash Function Registers (0x2_n800) */
	struct tsec_hash_regs	hash;

	u32	res900[128];

	/* Pattern Registers (0x2_nb00) */
	u32	resb00[62];
	u32	attr; /* Default Attribute Register */
	u32	attreli; /* Default Attribute Extract Length and Index */

	/* TSEC Future Expansion Space (0x2_nc00-0x2_nffc) */
	u32	resc00[256];
};

#define TSEC_GIGABIT	(1 << 0)

/* These flags currently only have meaning if we're using the eTSEC */
#define TSEC_REDUCED	(1 << 1)	/* MAC-PHY interface uses RGMII */
#define TSEC_SGMII	(1 << 2)	/* MAC-PHY interface uses SGMII */

#define TX_BUF_CNT	2

struct tsec_private {
	struct txbd8 __iomem txbd[TX_BUF_CNT];
	struct rxbd8 __iomem rxbd[PKTBUFSRX];
	struct tsec __iomem *regs;
	struct tsec_mii_mng __iomem *phyregs_sgmii;
	struct phy_device *phydev;
	phy_interface_t interface;
	struct mii_dev *bus;
	uint phyaddr;
	uint tbiaddr;
	char mii_devname[16];
	u32 flags;
	uint rx_idx;	/* index of the current RX buffer */
	uint tx_idx;	/* index of the current TX buffer */
#ifndef CONFIG_DM_ETH
	struct eth_device *dev;
#else
	struct udevice *dev;
#endif
};

struct tsec_info_struct {
	struct tsec __iomem *regs;
	struct tsec_mii_mng __iomem *miiregs_sgmii;
	char *devname;
	char *mii_devname;
	phy_interface_t interface;
	unsigned int phyaddr;
	u32 flags;
};

#ifndef CONFIG_DM_ETH
int tsec_standard_init(bd_t *bis);
int tsec_eth_init(bd_t *bis, struct tsec_info_struct *tsec_info, int num);
#endif

#endif /* __TSEC_H */
