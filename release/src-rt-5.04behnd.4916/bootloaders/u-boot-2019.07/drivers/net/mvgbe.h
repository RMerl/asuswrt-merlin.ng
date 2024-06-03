/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * based on - Driver for MV64360X ethernet ports
 * Copyright (C) 2002 rabeeh@galileo.co.il
 */

#ifndef __MVGBE_H__
#define __MVGBE_H__

/* PHY_BASE_ADR is board specific and can be configured */
#if defined (CONFIG_PHY_BASE_ADR)
#define PHY_BASE_ADR		CONFIG_PHY_BASE_ADR
#else
#define PHY_BASE_ADR		0x08	/* default phy base addr */
#endif

/* Constants */
#define INT_CAUSE_UNMASK_ALL		0x0007ffff
#define INT_CAUSE_UNMASK_ALL_EXT	0x0011ffff
#define MRU_MASK			0xfff1ffff
#define PHYADR_MASK			0x0000001f
#define PHYREG_MASK			0x0000001f
#define QTKNBKT_DEF_VAL			0x3fffffff
#define QMTBS_DEF_VAL			0x000003ff
#define QTKNRT_DEF_VAL			0x0000fcff
#define RXUQ	0 /* Used Rx queue */
#define TXUQ	0 /* Used Rx queue */

#ifndef CONFIG_DM_ETH
#define to_mvgbe(_d) container_of(_d, struct mvgbe_device, dev)
#endif
#define MVGBE_REG_WR(adr, val)		writel(val, &adr)
#define MVGBE_REG_RD(adr)		readl(&adr)
#define MVGBE_REG_BITS_RESET(adr, val)	writel(readl(&adr) & ~(val), &adr)
#define MVGBE_REG_BITS_SET(adr, val)	writel(readl(&adr) | val, &adr)

/* Default port configuration value */
#define PRT_CFG_VAL			( \
	MVGBE_UCAST_MOD_NRML		| \
	MVGBE_DFLT_RXQ(RXUQ)		| \
	MVGBE_DFLT_RX_ARPQ(RXUQ)	| \
	MVGBE_RX_BC_IF_NOT_IP_OR_ARP	| \
	MVGBE_RX_BC_IF_IP		| \
	MVGBE_RX_BC_IF_ARP		| \
	MVGBE_CPTR_TCP_FRMS_DIS		| \
	MVGBE_CPTR_UDP_FRMS_DIS		| \
	MVGBE_DFLT_RX_TCPQ(RXUQ)	| \
	MVGBE_DFLT_RX_UDPQ(RXUQ)	| \
	MVGBE_DFLT_RX_BPDUQ(RXUQ))

/* Default port extend configuration value */
#define PORT_CFG_EXTEND_VALUE		\
	MVGBE_SPAN_BPDU_PACKETS_AS_NORMAL	| \
	MVGBE_PARTITION_DIS		| \
	MVGBE_TX_CRC_GENERATION_EN

#define GT_MVGBE_IPG_INT_RX(value)	((value & 0x3fff) << 8)

/* Default sdma control value */
#define PORT_SDMA_CFG_VALUE		( \
	MVGBE_RX_BURST_SIZE_16_64BIT	| \
	MVGBE_BLM_RX_NO_SWAP		| \
	MVGBE_BLM_TX_NO_SWAP		| \
	GT_MVGBE_IPG_INT_RX(RXUQ)	| \
	MVGBE_TX_BURST_SIZE_16_64BIT)

/* Default port serial control value */
#ifndef PORT_SERIAL_CONTROL_VALUE
#define PORT_SERIAL_CONTROL_VALUE		( \
	MVGBE_FORCE_LINK_PASS			| \
	MVGBE_DIS_AUTO_NEG_FOR_DUPLX		| \
	MVGBE_DIS_AUTO_NEG_FOR_FLOW_CTRL	| \
	MVGBE_ADV_NO_FLOW_CTRL			| \
	MVGBE_FORCE_FC_MODE_NO_PAUSE_DIS_TX	| \
	MVGBE_FORCE_BP_MODE_NO_JAM		| \
	(1 << 9) /* Reserved bit has to be 1 */	| \
	MVGBE_DO_NOT_FORCE_LINK_FAIL		| \
	MVGBE_EN_AUTO_NEG_SPEED_GMII		| \
	MVGBE_DTE_ADV_0				| \
	MVGBE_MIIPHY_MAC_MODE			| \
	MVGBE_AUTO_NEG_NO_CHANGE		| \
	MVGBE_MAX_RX_PACKET_1552BYTE		| \
	MVGBE_CLR_EXT_LOOPBACK			| \
	MVGBE_SET_FULL_DUPLEX_MODE		| \
	MVGBE_DIS_FLOW_CTRL_TX_RX_IN_FULL_DUPLEX)
#endif

/* Tx WRR confoguration macros */
#define PORT_MAX_TRAN_UNIT	0x24	/* MTU register (default) 9KByte */
#define PORT_MAX_TOKEN_BUCKET_SIZE	0x_FFFF	/* PMTBS reg (default) */
#define PORT_TOKEN_RATE		1023	/* PTTBRC reg (default) */
/* MAC accepet/reject macros */
#define ACCEPT_MAC_ADDR		0
#define REJECT_MAC_ADDR		1
/* Size of a Tx/Rx descriptor used in chain list data structure */
#define MV_RXQ_DESC_ALIGNED_SIZE	\
	(((sizeof(struct mvgbe_rxdesc) / PKTALIGN) + 1) * PKTALIGN)
/* Buffer offset from buffer pointer */
#define RX_BUF_OFFSET		0x2

/* Port serial status reg (PSR) */
#define MVGBE_INTERFACE_GMII_MII	0
#define MVGBE_INTERFACE_PCM		1
#define MVGBE_LINK_IS_DOWN		0
#define MVGBE_LINK_IS_UP		(1 << 1)
#define MVGBE_PORT_AT_HALF_DUPLEX	0
#define MVGBE_PORT_AT_FULL_DUPLEX	(1 << 2)
#define MVGBE_RX_FLOW_CTRL_DISD		0
#define MVGBE_RX_FLOW_CTRL_ENBALED	(1 << 3)
#define MVGBE_GMII_SPEED_100_10		0
#define MVGBE_GMII_SPEED_1000		(1 << 4)
#define MVGBE_MII_SPEED_10		0
#define MVGBE_MII_SPEED_100		(1 << 5)
#define MVGBE_NO_TX			0
#define MVGBE_TX_IN_PROGRESS		(1 << 7)
#define MVGBE_BYPASS_NO_ACTIVE		0
#define MVGBE_BYPASS_ACTIVE		(1 << 8)
#define MVGBE_PORT_NOT_AT_PARTN_STT	0
#define MVGBE_PORT_AT_PARTN_STT		(1 << 9)
#define MVGBE_PORT_TX_FIFO_NOT_EMPTY	0
#define MVGBE_PORT_TX_FIFO_EMPTY	(1 << 10)

/* These macros describes the Port configuration reg (Px_cR) bits */
#define MVGBE_UCAST_MOD_NRML		0
#define MVGBE_UNICAST_PROMISCUOUS_MODE	1
#define MVGBE_DFLT_RXQ(_x)		(_x << 1)
#define MVGBE_DFLT_RX_ARPQ(_x)		(_x << 4)
#define MVGBE_RX_BC_IF_NOT_IP_OR_ARP	0
#define MVGBE_REJECT_BC_IF_NOT_IP_OR_ARP (1 << 7)
#define MVGBE_RX_BC_IF_IP		0
#define MVGBE_REJECT_BC_IF_IP		(1 << 8)
#define MVGBE_RX_BC_IF_ARP		0
#define MVGBE_REJECT_BC_IF_ARP		(1 << 9)
#define MVGBE_TX_AM_NO_UPDATE_ERR_SMRY	(1 << 12)
#define MVGBE_CPTR_TCP_FRMS_DIS		0
#define MVGBE_CPTR_TCP_FRMS_EN		(1 << 14)
#define MVGBE_CPTR_UDP_FRMS_DIS		0
#define MVGBE_CPTR_UDP_FRMS_EN		(1 << 15)
#define MVGBE_DFLT_RX_TCPQ(_x)		(_x << 16)
#define MVGBE_DFLT_RX_UDPQ(_x)		(_x << 19)
#define MVGBE_DFLT_RX_BPDUQ(_x)		(_x << 22)
#define MVGBE_DFLT_RX_TCP_CHKSUM_MODE	(1 << 25)

/* These macros describes the Port configuration extend reg (Px_cXR) bits*/
#define MVGBE_CLASSIFY_EN			1
#define MVGBE_SPAN_BPDU_PACKETS_AS_NORMAL	0
#define MVGBE_SPAN_BPDU_PACKETS_TO_RX_Q7	(1 << 1)
#define MVGBE_PARTITION_DIS			0
#define MVGBE_PARTITION_EN			(1 << 2)
#define MVGBE_TX_CRC_GENERATION_EN		0
#define MVGBE_TX_CRC_GENERATION_DIS		(1 << 3)

/* These macros describes the Port Sdma configuration reg (SDCR) bits */
#define MVGBE_RIFB				1
#define MVGBE_RX_BURST_SIZE_1_64BIT		0
#define MVGBE_RX_BURST_SIZE_2_64BIT		(1 << 1)
#define MVGBE_RX_BURST_SIZE_4_64BIT		(1 << 2)
#define MVGBE_RX_BURST_SIZE_8_64BIT		((1 << 2) | (1 << 1))
#define MVGBE_RX_BURST_SIZE_16_64BIT		(1 << 3)
#define MVGBE_BLM_RX_NO_SWAP			(1 << 4)
#define MVGBE_BLM_RX_BYTE_SWAP			0
#define MVGBE_BLM_TX_NO_SWAP			(1 << 5)
#define MVGBE_BLM_TX_BYTE_SWAP			0
#define MVGBE_DESCRIPTORS_BYTE_SWAP		(1 << 6)
#define MVGBE_DESCRIPTORS_NO_SWAP		0
#define MVGBE_TX_BURST_SIZE_1_64BIT		0
#define MVGBE_TX_BURST_SIZE_2_64BIT		(1 << 22)
#define MVGBE_TX_BURST_SIZE_4_64BIT		(1 << 23)
#define MVGBE_TX_BURST_SIZE_8_64BIT		((1 << 23) | (1 << 22))
#define MVGBE_TX_BURST_SIZE_16_64BIT		(1 << 24)

/* These macros describes the Port serial control reg (PSCR) bits */
#define MVGBE_SERIAL_PORT_DIS			0
#define MVGBE_SERIAL_PORT_EN			1
#define MVGBE_FORCE_LINK_PASS			(1 << 1)
#define MVGBE_DO_NOT_FORCE_LINK_PASS		0
#define MVGBE_EN_AUTO_NEG_FOR_DUPLX		0
#define MVGBE_DIS_AUTO_NEG_FOR_DUPLX		(1 << 2)
#define MVGBE_EN_AUTO_NEG_FOR_FLOW_CTRL		0
#define MVGBE_DIS_AUTO_NEG_FOR_FLOW_CTRL	(1 << 3)
#define MVGBE_ADV_NO_FLOW_CTRL			0
#define MVGBE_ADV_SYMMETRIC_FLOW_CTRL		(1 << 4)
#define MVGBE_FORCE_FC_MODE_NO_PAUSE_DIS_TX	0
#define MVGBE_FORCE_FC_MODE_TX_PAUSE_DIS	(1 << 5)
#define MVGBE_FORCE_BP_MODE_NO_JAM		0
#define MVGBE_FORCE_BP_MODE_JAM_TX		(1 << 7)
#define MVGBE_FORCE_BP_MODE_JAM_TX_ON_RX_ERR	(1 << 8)
#define MVGBE_FORCE_LINK_FAIL			0
#define MVGBE_DO_NOT_FORCE_LINK_FAIL		(1 << 10)
#define MVGBE_DIS_AUTO_NEG_SPEED_GMII		(1 << 13)
#define MVGBE_EN_AUTO_NEG_SPEED_GMII		0
#define MVGBE_DTE_ADV_0				0
#define MVGBE_DTE_ADV_1				(1 << 14)
#define MVGBE_MIIPHY_MAC_MODE			0
#define MVGBE_MIIPHY_PHY_MODE			(1 << 15)
#define MVGBE_AUTO_NEG_NO_CHANGE		0
#define MVGBE_RESTART_AUTO_NEG			(1 << 16)
#define MVGBE_MAX_RX_PACKET_1518BYTE		0
#define MVGBE_MAX_RX_PACKET_1522BYTE		(1 << 17)
#define MVGBE_MAX_RX_PACKET_1552BYTE		(1 << 18)
#define MVGBE_MAX_RX_PACKET_9022BYTE		((1 << 18) | (1 << 17))
#define MVGBE_MAX_RX_PACKET_9192BYTE		(1 << 19)
#define MVGBE_MAX_RX_PACKET_9700BYTE		((1 << 19) | (1 << 17))
#define MVGBE_SET_EXT_LOOPBACK			(1 << 20)
#define MVGBE_CLR_EXT_LOOPBACK			0
#define MVGBE_SET_FULL_DUPLEX_MODE		(1 << 21)
#define MVGBE_SET_HALF_DUPLEX_MODE		0
#define MVGBE_EN_FLOW_CTRL_TX_RX_IN_FULL_DUPLEX	(1 << 22)
#define MVGBE_DIS_FLOW_CTRL_TX_RX_IN_FULL_DUPLEX 0
#define MVGBE_SET_GMII_SPEED_TO_10_100		0
#define MVGBE_SET_GMII_SPEED_TO_1000		(1 << 23)
#define MVGBE_SET_MII_SPEED_TO_10		0
#define MVGBE_SET_MII_SPEED_TO_100		(1 << 24)

/* SMI register fields */
#define MVGBE_PHY_SMI_TIMEOUT		10000
#define MVGBE_PHY_SMI_TIMEOUT_MS	1000
#define MVGBE_PHY_SMI_DATA_OFFS		0	/* Data */
#define MVGBE_PHY_SMI_DATA_MASK		(0xffff << MVGBE_PHY_SMI_DATA_OFFS)
#define MVGBE_PHY_SMI_DEV_ADDR_OFFS	16	/* PHY device address */
#define MVGBE_PHY_SMI_DEV_ADDR_MASK \
	(PHYADR_MASK << MVGBE_PHY_SMI_DEV_ADDR_OFFS)
#define MVGBE_SMI_REG_ADDR_OFFS		21	/* PHY device reg addr */
#define MVGBE_SMI_REG_ADDR_MASK \
	(PHYADR_MASK << MVGBE_SMI_REG_ADDR_OFFS)
#define MVGBE_PHY_SMI_OPCODE_OFFS	26	/* Write/Read opcode */
#define MVGBE_PHY_SMI_OPCODE_MASK	(3 << MVGBE_PHY_SMI_OPCODE_OFFS)
#define MVGBE_PHY_SMI_OPCODE_WRITE	(0 << MVGBE_PHY_SMI_OPCODE_OFFS)
#define MVGBE_PHY_SMI_OPCODE_READ	(1 << MVGBE_PHY_SMI_OPCODE_OFFS)
#define MVGBE_PHY_SMI_READ_VALID_MASK	(1 << 27)	/* Read Valid */
#define MVGBE_PHY_SMI_BUSY_MASK		(1 << 28)	/* Busy */

/* SDMA command status fields macros */
/* Tx & Rx descriptors status */
#define MVGBE_ERROR_SUMMARY		1
/* Tx & Rx descriptors command */
#define MVGBE_BUFFER_OWNED_BY_DMA	(1 << 31)
/* Tx descriptors status */
#define MVGBE_LC_ERROR			0
#define MVGBE_UR_ERROR			(1 << 1)
#define MVGBE_RL_ERROR			(1 << 2)
#define MVGBE_LLC_SNAP_FORMAT		(1 << 9)
#define MVGBE_TX_LAST_FRAME		(1 << 20)

/* Rx descriptors status */
#define MVGBE_CRC_ERROR			0
#define MVGBE_OVERRUN_ERROR		(1 << 1)
#define MVGBE_MAX_FRAME_LENGTH_ERROR	(1 << 2)
#define MVGBE_RESOURCE_ERROR		((1 << 2) | (1 << 1))
#define MVGBE_VLAN_TAGGED		(1 << 19)
#define MVGBE_BPDU_FRAME		(1 << 20)
#define MVGBE_TCP_FRAME_OVER_IP_V_4	0
#define MVGBE_UDP_FRAME_OVER_IP_V_4	(1 << 21)
#define MVGBE_OTHER_FRAME_TYPE		(1 << 22)
#define MVGBE_LAYER_2_IS_MVGBE_V_2	(1 << 23)
#define MVGBE_FRAME_TYPE_IP_V_4		(1 << 24)
#define MVGBE_FRAME_HEADER_OK		(1 << 25)
#define MVGBE_RX_LAST_DESC		(1 << 26)
#define MVGBE_RX_FIRST_DESC		(1 << 27)
#define MVGBE_UNKNOWN_DESTINATION_ADDR	(1 << 28)
#define MVGBE_RX_EN_INTERRUPT		(1 << 29)
#define MVGBE_LAYER_4_CHECKSUM_OK	(1 << 30)

/* Rx descriptors byte count */
#define MVGBE_FRAME_FRAGMENTED		(1 << 2)

/* Tx descriptors command */
#define MVGBE_LAYER_4_CHECKSUM_FIRST_DESC	(1 << 10)
#define MVGBE_FRAME_SET_TO_VLAN			(1 << 15)
#define MVGBE_TCP_FRAME				0
#define MVGBE_UDP_FRAME				(1 << 16)
#define MVGBE_GEN_TCP_UDP_CHECKSUM		(1 << 17)
#define MVGBE_GEN_IP_V_4_CHECKSUM		(1 << 18)
#define MVGBE_ZERO_PADDING			(1 << 19)
#define MVGBE_TX_LAST_DESC			(1 << 20)
#define MVGBE_TX_FIRST_DESC			(1 << 21)
#define MVGBE_GEN_CRC				(1 << 22)
#define MVGBE_TX_EN_INTERRUPT			(1 << 23)
#define MVGBE_AUTO_MODE				(1 << 30)

/* Address decode parameters */
/* Ethernet Base Address Register bits */
#define EBAR_TARGET_DRAM			0x00000000
#define EBAR_TARGET_DEVICE			0x00000001
#define EBAR_TARGET_CBS				0x00000002
#define EBAR_TARGET_PCI0			0x00000003
#define EBAR_TARGET_PCI1			0x00000004
#define EBAR_TARGET_CUNIT			0x00000005
#define EBAR_TARGET_AUNIT			0x00000006
#define EBAR_TARGET_GUNIT			0x00000007

/* Window attrib */
#define EBAR_DRAM_CS0				0x00000E00
#define EBAR_DRAM_CS1				0x00000D00
#define EBAR_DRAM_CS2				0x00000B00
#define EBAR_DRAM_CS3				0x00000700

/* DRAM Target interface */
#define EBAR_DRAM_NO_CACHE_COHERENCY		0x00000000
#define EBAR_DRAM_CACHE_COHERENCY_WT		0x00001000
#define EBAR_DRAM_CACHE_COHERENCY_WB		0x00002000

/* Device Bus Target interface */
#define EBAR_DEVICE_DEVCS0			0x00001E00
#define EBAR_DEVICE_DEVCS1			0x00001D00
#define EBAR_DEVICE_DEVCS2			0x00001B00
#define EBAR_DEVICE_DEVCS3			0x00001700
#define EBAR_DEVICE_BOOTCS3			0x00000F00

/* PCI Target interface */
#define EBAR_PCI_BYTE_SWAP			0x00000000
#define EBAR_PCI_NO_SWAP			0x00000100
#define EBAR_PCI_BYTE_WORD_SWAP			0x00000200
#define EBAR_PCI_WORD_SWAP			0x00000300
#define EBAR_PCI_NO_SNOOP_NOT_ASSERT		0x00000000
#define EBAR_PCI_NO_SNOOP_ASSERT		0x00000400
#define EBAR_PCI_IO_SPACE			0x00000000
#define EBAR_PCI_MEMORY_SPACE			0x00000800
#define EBAR_PCI_REQ64_FORCE			0x00000000
#define EBAR_PCI_REQ64_SIZE			0x00001000

/* Window access control */
#define EWIN_ACCESS_NOT_ALLOWED 0
#define EWIN_ACCESS_READ_ONLY	1
#define EWIN_ACCESS_FULL	((1 << 1) | 1)

/* structures represents Controller registers */
struct mvgbe_barsz {
	u32 bar;
	u32 size;
};

struct mvgbe_rxcdp {
	struct mvgbe_rxdesc *rxcdp;
	u32 rxcdp_pad[3];
};

struct mvgbe_tqx {
	u32 qxttbc;
	u32 tqxtbc;
	u32 tqxac;
	u32 tqxpad;
};

struct mvgbe_registers {
	u32 phyadr;
	u32 smi;
	u32 euda;
	u32 eudid;
	u8 pad1[0x080 - 0x00c - 4];
	u32 euic;
	u32 euim;
	u8 pad2[0x094 - 0x084 - 4];
	u32 euea;
	u32 euiae;
	u8 pad3[0x0b0 - 0x098 - 4];
	u32 euc;
	u8 pad3a[0x200 - 0x0b0 - 4];
	struct mvgbe_barsz barsz[6];
	u8 pad4[0x280 - 0x22c - 4];
	u32 ha_remap[4];
	u32 bare;
	u32 epap;
	u8 pad5[0x400 - 0x294 - 4];
	u32 pxc;
	u32 pxcx;
	u32 mii_ser_params;
	u8 pad6[0x410 - 0x408 - 4];
	u32 evlane;
	u32 macal;
	u32 macah;
	u32 sdc;
	u32 dscp[7];
	u32 psc0;
	u32 vpt2p;
	u32 ps0;
	u32 tqc;
	u32 psc1;
	u32 ps1;
	u32 mrvl_header;
	u8 pad7[0x460 - 0x454 - 4];
	u32 ic;
	u32 ice;
	u32 pim;
	u32 peim;
	u8 pad8[0x474 - 0x46c - 4];
	u32 pxtfut;
	u32 pad9;
	u32 pxmfs;
	u32 pad10;
	u32 pxdfc;
	u32 pxofc;
	u8 pad11[0x494 - 0x488 - 4];
	u32 peuiae;
	u8 pad12[0x4bc - 0x494 - 4];
	u32 eth_type_prio;
	u8 pad13[0x4dc - 0x4bc - 4];
	u32 tqfpc;
	u32 pttbrc;
	u32 tqc1;
	u32 pmtu;
	u32 pmtbs;
	u8 pad14[0x60c - 0x4ec - 4];
	struct mvgbe_rxcdp rxcdp[7];
	struct mvgbe_rxdesc *rxcdp7;
	u32 rqc;
	struct mvgbe_txdesc *tcsdp;
	u8 pad15[0x6c0 - 0x684 - 4];
	struct mvgbe_txdesc *tcqdp[8];
	u8 pad16[0x700 - 0x6dc - 4];
	struct mvgbe_tqx tqx[8];
	u32 pttbc;
	u8 pad17[0x7a8 - 0x780 - 4];
	u32 tqxipg0;
	u32 pad18[3];
	u32 tqxipg1;
	u8 pad19[0x7c0 - 0x7b8 - 4];
	u32 hitkninlopkt;
	u32 hitkninasyncpkt;
	u32 lotkninasyncpkt;
	u32 pad20;
	u32 ts;
	u8 pad21[0x3000 - 0x27d0 - 4];
	u32 pad20_1[32];	/* mib counter registes */
	u8 pad22[0x3400 - 0x3000 - sizeof(u32) * 32];
	u32 dfsmt[64];
	u32 dfomt[64];
	u32 dfut[4];
	u8 pad23[0xe20c0 - 0x7360c - 4];
	u32 pmbus_top_arbiter;
};

/* structures/enums needed by driver */
enum mvgbe_adrwin {
	MVGBE_WIN0,
	MVGBE_WIN1,
	MVGBE_WIN2,
	MVGBE_WIN3,
	MVGBE_WIN4,
	MVGBE_WIN5
};

enum mvgbe_target {
	MVGBE_TARGET_DRAM,
	MVGBE_TARGET_DEV,
	MVGBE_TARGET_CBS,
	MVGBE_TARGET_PCI0,
	MVGBE_TARGET_PCI1
};

struct mvgbe_winparam {
	enum mvgbe_adrwin win;	/* Window number */
	enum mvgbe_target target;	/* System targets */
	u16 attrib;		/* BAR attrib. See above macros */
	u32 base_addr;		/* Window base address in u32 form */
	u32 high_addr;		/* Window high address in u32 form */
	u32 size;		/* Size in MBytes. Must be % 64Kbyte. */
	int enable;		/* Enable/disable access to the window. */
	u16 access_ctrl;	/*Access ctrl register. see above macros */
};

struct mvgbe_rxdesc {
	u32 cmd_sts;		/* Descriptor command status */
	u16 buf_size;		/* Buffer size */
	u16 byte_cnt;		/* Descriptor buffer byte count */
	u8 *buf_ptr;		/* Descriptor buffer pointer */
	struct mvgbe_rxdesc *nxtdesc_p;	/* Next descriptor pointer */
};

struct mvgbe_txdesc {
	u32 cmd_sts;		/* Descriptor command status */
	u16 l4i_chk;		/* CPU provided TCP Checksum */
	u16 byte_cnt;		/* Descriptor buffer byte count */
	u8 *buf_ptr;		/* Descriptor buffer ptr */
	struct mvgbe_txdesc *nxtdesc_p;	/* Next descriptor ptr */
};

/* port device data struct */
struct mvgbe_device {
#ifndef CONFIG_DM_ETH
	struct eth_device dev;
#endif
	struct mvgbe_registers *regs;
	struct mvgbe_txdesc *p_txdesc;
	struct mvgbe_rxdesc *p_rxdesc;
	struct mvgbe_rxdesc *p_rxdesc_curr;
	u8 *p_rxbuf;
	u8 *p_aligned_txbuf;

#ifdef CONFIG_DM_ETH
	phy_interface_t phy_interface;
	unsigned int link;
	unsigned int duplex;
	unsigned int speed;

	int init;
	int phyaddr;
	struct phy_device *phydev;
	struct mii_dev *bus;
#endif
};

#endif /* __MVGBE_H__ */
