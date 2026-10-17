/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2005-2006 Atmel Corporation
 */
#ifndef __DRIVERS_MACB_H__
#define __DRIVERS_MACB_H__

#define MACB_GREGS_NBR 16
#define MACB_GREGS_VERSION 2
#define MACB_MAX_QUEUES 8

/* MACB register offsets */
#define MACB_NCR		0x0000 /* Network Control */
#define MACB_NCFGR		0x0004 /* Network Config */
#define MACB_NSR		0x0008 /* Network Status */
#define MACB_TAR		0x000c /* AT91RM9200 only */
#define MACB_TCR		0x0010 /* AT91RM9200 only */
#define MACB_TSR		0x0014 /* Transmit Status */
#define MACB_RBQP		0x0018 /* RX Q Base Address */
#define MACB_TBQP		0x001c /* TX Q Base Address */
#define MACB_RSR		0x0020 /* Receive Status */
#define MACB_ISR		0x0024 /* Interrupt Status */
#define MACB_IER		0x0028 /* Interrupt Enable */
#define MACB_IDR		0x002c /* Interrupt Disable */
#define MACB_IMR		0x0030 /* Interrupt Mask */
#define MACB_MAN		0x0034 /* PHY Maintenance */
#define MACB_PTR		0x0038
#define MACB_PFR		0x003c
#define MACB_FTO		0x0040
#define MACB_SCF		0x0044
#define MACB_MCF		0x0048
#define MACB_FRO		0x004c
#define MACB_FCSE		0x0050
#define MACB_ALE		0x0054
#define MACB_DTF		0x0058
#define MACB_LCOL		0x005c
#define MACB_EXCOL		0x0060
#define MACB_TUND		0x0064
#define MACB_CSE		0x0068
#define MACB_RRE		0x006c
#define MACB_ROVR		0x0070
#define MACB_RSE		0x0074
#define MACB_ELE		0x0078
#define MACB_RJA		0x007c
#define MACB_USF		0x0080
#define MACB_STE		0x0084
#define MACB_RLE		0x0088
#define MACB_TPF		0x008c
#define MACB_HRB		0x0090
#define MACB_HRT		0x0094
#define MACB_SA1B		0x0098
#define MACB_SA1T		0x009c
#define MACB_SA2B		0x00a0
#define MACB_SA2T		0x00a4
#define MACB_SA3B		0x00a8
#define MACB_SA3T		0x00ac
#define MACB_SA4B		0x00b0
#define MACB_SA4T		0x00b4
#define MACB_TID		0x00b8
#define MACB_TPQ		0x00bc
#define MACB_USRIO		0x00c0
#define MACB_WOL		0x00c4
#define MACB_MID		0x00fc
#define MACB_TBQPH		0x04C8
#define MACB_RBQPH		0x04D4

/* GEM register offsets. */
#define GEM_NCFGR		0x0004 /* Network Config */
#define GEM_USRIO		0x000c /* User IO */
#define GEM_DMACFG		0x0010 /* DMA Configuration */
#define GEM_JML			0x0048 /* Jumbo Max Length */
#define GEM_HRB			0x0080 /* Hash Bottom */
#define GEM_HRT			0x0084 /* Hash Top */
#define GEM_SA1B		0x0088 /* Specific1 Bottom */
#define GEM_SA1T		0x008C /* Specific1 Top */
#define GEM_SA2B		0x0090 /* Specific2 Bottom */
#define GEM_SA2T		0x0094 /* Specific2 Top */
#define GEM_SA3B		0x0098 /* Specific3 Bottom */
#define GEM_SA3T		0x009C /* Specific3 Top */
#define GEM_SA4B		0x00A0 /* Specific4 Bottom */
#define GEM_SA4T		0x00A4 /* Specific4 Top */
#define GEM_EFTSH		0x00e8 /* PTP Event Frame Transmitted Seconds Register 47:32 */
#define GEM_EFRSH		0x00ec /* PTP Event Frame Received Seconds Register 47:32 */
#define GEM_PEFTSH		0x00f0 /* PTP Peer Event Frame Transmitted Seconds Register 47:32 */
#define GEM_PEFRSH		0x00f4 /* PTP Peer Event Frame Received Seconds Register 47:32 */
#define GEM_OTX			0x0100 /* Octets transmitted */
#define GEM_OCTTXL		0x0100 /* Octets transmitted [31:0] */
#define GEM_OCTTXH		0x0104 /* Octets transmitted [47:32] */
#define GEM_TXCNT		0x0108 /* Frames Transmitted counter */
#define GEM_TXBCCNT		0x010c /* Broadcast Frames counter */
#define GEM_TXMCCNT		0x0110 /* Multicast Frames counter */
#define GEM_TXPAUSECNT		0x0114 /* Pause Frames Transmitted Counter */
#define GEM_TX64CNT		0x0118 /* 64 byte Frames TX counter */
#define GEM_TX65CNT		0x011c /* 65-127 byte Frames TX counter */
#define GEM_TX128CNT		0x0120 /* 128-255 byte Frames TX counter */
#define GEM_TX256CNT		0x0124 /* 256-511 byte Frames TX counter */
#define GEM_TX512CNT		0x0128 /* 512-1023 byte Frames TX counter */
#define GEM_TX1024CNT		0x012c /* 1024-1518 byte Frames TX counter */
#define GEM_TX1519CNT		0x0130 /* 1519+ byte Frames TX counter */
#define GEM_TXURUNCNT		0x0134 /* TX under run error counter */
#define GEM_SNGLCOLLCNT		0x0138 /* Single Collision Frame Counter */
#define GEM_MULTICOLLCNT	0x013c /* Multiple Collision Frame Counter */
#define GEM_EXCESSCOLLCNT	0x0140 /* Excessive Collision Frame Counter */
#define GEM_LATECOLLCNT		0x0144 /* Late Collision Frame Counter */
#define GEM_TXDEFERCNT		0x0148 /* Deferred Transmission Frame Counter */
#define GEM_TXCSENSECNT		0x014c /* Carrier Sense Error Counter */
#define GEM_ORX			0x0150 /* Octets received */
#define GEM_OCTRXL		0x0150 /* Octets received [31:0] */
#define GEM_OCTRXH		0x0154 /* Octets received [47:32] */
#define GEM_RXCNT		0x0158 /* Frames Received Counter */
#define GEM_RXBROADCNT		0x015c /* Broadcast Frames Received Counter */
#define GEM_RXMULTICNT		0x0160 /* Multicast Frames Received Counter */
#define GEM_RXPAUSECNT		0x0164 /* Pause Frames Received Counter */
#define GEM_RX64CNT		0x0168 /* 64 byte Frames RX Counter */
#define GEM_RX65CNT		0x016c /* 65-127 byte Frames RX Counter */
#define GEM_RX128CNT		0x0170 /* 128-255 byte Frames RX Counter */
#define GEM_RX256CNT		0x0174 /* 256-511 byte Frames RX Counter */
#define GEM_RX512CNT		0x0178 /* 512-1023 byte Frames RX Counter */
#define GEM_RX1024CNT		0x017c /* 1024-1518 byte Frames RX Counter */
#define GEM_RX1519CNT		0x0180 /* 1519+ byte Frames RX Counter */
#define GEM_RXUNDRCNT		0x0184 /* Undersize Frames Received Counter */
#define GEM_RXOVRCNT		0x0188 /* Oversize Frames Received Counter */
#define GEM_RXJABCNT		0x018c /* Jabbers Received Counter */
#define GEM_RXFCSCNT		0x0190 /* Frame Check Sequence Error Counter */
#define GEM_RXLENGTHCNT		0x0194 /* Length Field Error Counter */
#define GEM_RXSYMBCNT		0x0198 /* Symbol Error Counter */
#define GEM_RXALIGNCNT		0x019c /* Alignment Error Counter */
#define GEM_RXRESERRCNT		0x01a0 /* Receive Resource Error Counter */
#define GEM_RXORCNT		0x01a4 /* Receive Overrun Counter */
#define GEM_RXIPCCNT		0x01a8 /* IP header Checksum Error Counter */
#define GEM_RXTCPCCNT		0x01ac /* TCP Checksum Error Counter */
#define GEM_RXUDPCCNT		0x01b0 /* UDP Checksum Error Counter */
#define GEM_TISUBN		0x01bc /* 1588 Timer Increment Sub-ns */
#define GEM_TSH			0x01c0 /* 1588 Timer Seconds High */
#define GEM_TSL			0x01d0 /* 1588 Timer Seconds Low */
#define GEM_TN			0x01d4 /* 1588 Timer Nanoseconds */
#define GEM_TA			0x01d8 /* 1588 Timer Adjust */
#define GEM_TI			0x01dc /* 1588 Timer Increment */
#define GEM_EFTSL		0x01e0 /* PTP Event Frame Tx Seconds Low */
#define GEM_EFTN		0x01e4 /* PTP Event Frame Tx Nanoseconds */
#define GEM_EFRSL		0x01e8 /* PTP Event Frame Rx Seconds Low */
#define GEM_EFRN		0x01ec /* PTP Event Frame Rx Nanoseconds */
#define GEM_PEFTSL		0x01f0 /* PTP Peer Event Frame Tx Secs Low */
#define GEM_PEFTN		0x01f4 /* PTP Peer Event Frame Tx Ns */
#define GEM_PEFRSL		0x01f8 /* PTP Peer Event Frame Rx Sec Low */
#define GEM_PEFRN		0x01fc /* PTP Peer Event Frame Rx Ns */
#define GEM_DCFG1		0x0280 /* Design Config 1 */
#define GEM_DCFG2		0x0284 /* Design Config 2 */
#define GEM_DCFG3		0x0288 /* Design Config 3 */
#define GEM_DCFG4		0x028c /* Design Config 4 */
#define GEM_DCFG5		0x0290 /* Design Config 5 */
#define GEM_DCFG6		0x0294 /* Design Config 6 */
#define GEM_DCFG7		0x0298 /* Design Config 7 */
#define GEM_DCFG8		0x029C /* Design Config 8 */
#define GEM_DCFG10		0x02A4 /* Design Config 10 */

#define GEM_TXBDCTRL	0x04cc /* TX Buffer Descriptor control register */
#define GEM_RXBDCTRL	0x04d0 /* RX Buffer Descriptor control register */

/* Screener Type 2 match registers */
#define GEM_SCRT2		0x540

/* EtherType registers */
#define GEM_ETHT		0x06E0

/* Type 2 compare registers */
#define GEM_T2CMPW0		0x0700
#define GEM_T2CMPW1		0x0704
#define T2CMP_OFST(t2idx)	(t2idx * 2)

/* type 2 compare registers
 * each location requires 3 compare regs
 */
#define GEM_IP4SRC_CMP(idx)		(idx * 3)
#define GEM_IP4DST_CMP(idx)		(idx * 3 + 1)
#define GEM_PORT_CMP(idx)		(idx * 3 + 2)

/* Which screening type 2 EtherType register will be used (0 - 7) */
#define SCRT2_ETHT		0

#define GEM_ISR(hw_q)		(0x0400 + ((hw_q) << 2))
#define GEM_TBQP(hw_q)		(0x0440 + ((hw_q) << 2))
#define GEM_TBQPH(hw_q)		(0x04C8)
#define GEM_RBQP(hw_q)		(0x0480 + ((hw_q) << 2))
#define GEM_RBQS(hw_q)		(0x04A0 + ((hw_q) << 2))
#define GEM_RBQPH(hw_q)		(0x04D4)
#define GEM_IER(hw_q)		(0x0600 + ((hw_q) << 2))
#define GEM_IDR(hw_q)		(0x0620 + ((hw_q) << 2))
#define GEM_IMR(hw_q)		(0x0640 + ((hw_q) << 2))

/* Bitfields in NCR */
#define MACB_LB_OFFSET		0 /* reserved */
#define MACB_LB_SIZE		1
#define MACB_LLB_OFFSET		1 /* Loop back local */
#define MACB_LLB_SIZE		1
#define MACB_RE_OFFSET		2 /* Receive enable */
#define MACB_RE_SIZE		1
#define MACB_TE_OFFSET		3 /* Transmit enable */
#define MACB_TE_SIZE		1
#define MACB_MPE_OFFSET		4 /* Management port enable */
#define MACB_MPE_SIZE		1
#define MACB_CLRSTAT_OFFSET	5 /* Clear stats regs */
#define MACB_CLRSTAT_SIZE	1
#define MACB_INCSTAT_OFFSET	6 /* Incremental stats regs */
#define MACB_INCSTAT_SIZE	1
#define MACB_WESTAT_OFFSET	7 /* Write enable stats regs */
#define MACB_WESTAT_SIZE	1
#define MACB_BP_OFFSET		8 /* Back pressure */
#define MACB_BP_SIZE		1
#define MACB_TSTART_OFFSET	9 /* Start transmission */
#define MACB_TSTART_SIZE	1
#define MACB_THALT_OFFSET	10 /* Transmit halt */
#define MACB_THALT_SIZE		1
#define MACB_NCR_TPF_OFFSET	11 /* Transmit pause frame */
#define MACB_NCR_TPF_SIZE	1
#define MACB_TZQ_OFFSET		12 /* Transmit zero quantum pause frame */
#define MACB_TZQ_SIZE		1
#define MACB_SRTSM_OFFSET	15
#define MACB_OSSMODE_OFFSET 24 /* Enable One Step Synchro Mode */
#define MACB_OSSMODE_SIZE	1

/* Bitfields in NCFGR */
#define MACB_SPD_OFFSET		0 /* Speed */
#define MACB_SPD_SIZE		1
#define MACB_FD_OFFSET		1 /* Full duplex */
#define MACB_FD_SIZE		1
#define MACB_BIT_RATE_OFFSET	2 /* Discard non-VLAN frames */
#define MACB_BIT_RATE_SIZE	1
#define MACB_JFRAME_OFFSET	3 /* reserved */
#define MACB_JFRAME_SIZE	1
#define MACB_CAF_OFFSET		4 /* Copy all frames */
#define MACB_CAF_SIZE		1
#define MACB_NBC_OFFSET		5 /* No broadcast */
#define MACB_NBC_SIZE		1
#define MACB_NCFGR_MTI_OFFSET	6 /* Multicast hash enable */
#define MACB_NCFGR_MTI_SIZE	1
#define MACB_UNI_OFFSET		7 /* Unicast hash enable */
#define MACB_UNI_SIZE		1
#define MACB_BIG_OFFSET		8 /* Receive 1536 byte frames */
#define MACB_BIG_SIZE		1
#define MACB_EAE_OFFSET		9 /* External address match enable */
#define MACB_EAE_SIZE		1
#define MACB_CLK_OFFSET		10
#define MACB_CLK_SIZE		2
#define MACB_RTY_OFFSET		12 /* Retry test */
#define MACB_RTY_SIZE		1
#define MACB_PAE_OFFSET		13 /* Pause enable */
#define MACB_PAE_SIZE		1
#define MACB_RM9200_RMII_OFFSET	13 /* AT91RM9200 only */
#define MACB_RM9200_RMII_SIZE	1  /* AT91RM9200 only */
#define MACB_RBOF_OFFSET	14 /* Receive buffer offset */
#define MACB_RBOF_SIZE		2
#define MACB_RLCE_OFFSET	16 /* Length field error frame discard */
#define MACB_RLCE_SIZE		1
#define MACB_DRFCS_OFFSET	17 /* FCS remove */
#define MACB_DRFCS_SIZE		1
#define MACB_EFRHD_OFFSET	18
#define MACB_EFRHD_SIZE		1
#define MACB_IRXFCS_OFFSET	19
#define MACB_IRXFCS_SIZE	1

/* GEM specific NCFGR bitfields. */
#define GEM_GBE_OFFSET		10 /* Gigabit mode enable */
#define GEM_GBE_SIZE		1
#define GEM_PCSSEL_OFFSET	11
#define GEM_PCSSEL_SIZE		1
#define GEM_CLK_OFFSET		18 /* MDC clock division */
#define GEM_CLK_SIZE		3
#define GEM_DBW_OFFSET		21 /* Data bus width */
#define GEM_DBW_SIZE		2
#define GEM_RXCOEN_OFFSET	24
#define GEM_RXCOEN_SIZE		1
#define GEM_SGMIIEN_OFFSET	27
#define GEM_SGMIIEN_SIZE	1


/* Constants for data bus width. */
#define GEM_DBW32		0 /* 32 bit AMBA AHB data bus width */
#define GEM_DBW64		1 /* 64 bit AMBA AHB data bus width */
#define GEM_DBW128		2 /* 128 bit AMBA AHB data bus width */

/* Bitfields in DMACFG. */
#define GEM_FBLDO_OFFSET	0 /* fixed burst length for DMA */
#define GEM_FBLDO_SIZE		5
#define GEM_ENDIA_DESC_OFFSET	6 /* endian swap mode for management descriptor access */
#define GEM_ENDIA_DESC_SIZE	1
#define GEM_ENDIA_PKT_OFFSET	7 /* endian swap mode for packet data access */
#define GEM_ENDIA_PKT_SIZE	1
#define GEM_RXBMS_OFFSET	8 /* RX packet buffer memory size select */
#define GEM_RXBMS_SIZE		2
#define GEM_TXPBMS_OFFSET	10 /* TX packet buffer memory size select */
#define GEM_TXPBMS_SIZE		1
#define GEM_TXCOEN_OFFSET	11 /* TX IP/TCP/UDP checksum gen offload */
#define GEM_TXCOEN_SIZE		1
#define GEM_RXBS_OFFSET		16 /* DMA receive buffer size */
#define GEM_RXBS_SIZE		8
#define GEM_DDRP_OFFSET		24 /* disc_when_no_ahb */
#define GEM_DDRP_SIZE		1
#define GEM_RXEXT_OFFSET	28 /* RX extended Buffer Descriptor mode */
#define GEM_RXEXT_SIZE		1
#define GEM_TXEXT_OFFSET	29 /* TX extended Buffer Descriptor mode */
#define GEM_TXEXT_SIZE		1
#define GEM_ADDR64_OFFSET	30 /* Address bus width - 64b or 32b */
#define GEM_ADDR64_SIZE		1


/* Bitfields in NSR */
#define MACB_NSR_LINK_OFFSET	0 /* pcs_link_state */
#define MACB_NSR_LINK_SIZE	1
#define MACB_MDIO_OFFSET	1 /* status of the mdio_in pin */
#define MACB_MDIO_SIZE		1
#define MACB_IDLE_OFFSET	2 /* The PHY management logic is idle */
#define MACB_IDLE_SIZE		1

/* Bitfields in TSR */
#define MACB_UBR_OFFSET		0 /* Used bit read */
#define MACB_UBR_SIZE		1
#define MACB_COL_OFFSET		1 /* Collision occurred */
#define MACB_COL_SIZE		1
#define MACB_TSR_RLE_OFFSET	2 /* Retry limit exceeded */
#define MACB_TSR_RLE_SIZE	1
#define MACB_TGO_OFFSET		3 /* Transmit go */
#define MACB_TGO_SIZE		1
#define MACB_BEX_OFFSET		4 /* TX frame corruption due to AHB error */
#define MACB_BEX_SIZE		1
#define MACB_RM9200_BNQ_OFFSET	4 /* AT91RM9200 only */
#define MACB_RM9200_BNQ_SIZE	1 /* AT91RM9200 only */
#define MACB_COMP_OFFSET	5 /* Trnasmit complete */
#define MACB_COMP_SIZE		1
#define MACB_UND_OFFSET		6 /* Trnasmit under run */
#define MACB_UND_SIZE		1

/* Bitfields in RSR */
#define MACB_BNA_OFFSET		0 /* Buffer not available */
#define MACB_BNA_SIZE		1
#define MACB_REC_OFFSET		1 /* Frame received */
#define MACB_REC_SIZE		1
#define MACB_OVR_OFFSET		2 /* Receive overrun */
#define MACB_OVR_SIZE		1

/* Bitfields in ISR/IER/IDR/IMR */
#define MACB_MFD_OFFSET		0 /* Management frame sent */
#define MACB_MFD_SIZE		1
#define MACB_RCOMP_OFFSET	1 /* Receive complete */
#define MACB_RCOMP_SIZE		1
#define MACB_RXUBR_OFFSET	2 /* RX used bit read */
#define MACB_RXUBR_SIZE		1
#define MACB_TXUBR_OFFSET	3 /* TX used bit read */
#define MACB_TXUBR_SIZE		1
#define MACB_ISR_TUND_OFFSET	4 /* Enable TX buffer under run interrupt */
#define MACB_ISR_TUND_SIZE	1
#define MACB_ISR_RLE_OFFSET	5 /* EN retry exceeded/late coll interrupt */
#define MACB_ISR_RLE_SIZE	1
#define MACB_TXERR_OFFSET	6 /* EN TX frame corrupt from error interrupt */
#define MACB_TXERR_SIZE		1
#define MACB_TCOMP_OFFSET	7 /* Enable transmit complete interrupt */
#define MACB_TCOMP_SIZE		1
#define MACB_ISR_LINK_OFFSET	9 /* Enable link change interrupt */
#define MACB_ISR_LINK_SIZE	1
#define MACB_ISR_ROVR_OFFSET	10 /* Enable receive overrun interrupt */
#define MACB_ISR_ROVR_SIZE	1
#define MACB_HRESP_OFFSET	11 /* Enable hrsep not OK interrupt */
#define MACB_HRESP_SIZE		1
#define MACB_PFR_OFFSET		12 /* Enable pause frame w/ quantum interrupt */
#define MACB_PFR_SIZE		1
#define MACB_PTZ_OFFSET		13 /* Enable pause time zero interrupt */
#define MACB_PTZ_SIZE		1
#define MACB_WOL_OFFSET		14 /* Enable wake-on-lan interrupt */
#define MACB_WOL_SIZE		1
#define MACB_DRQFR_OFFSET	18 /* PTP Delay Request Frame Received */
#define MACB_DRQFR_SIZE		1
#define MACB_SFR_OFFSET		19 /* PTP Sync Frame Received */
#define MACB_SFR_SIZE		1
#define MACB_DRQFT_OFFSET	20 /* PTP Delay Request Frame Transmitted */
#define MACB_DRQFT_SIZE		1
#define MACB_SFT_OFFSET		21 /* PTP Sync Frame Transmitted */
#define MACB_SFT_SIZE		1
#define MACB_PDRQFR_OFFSET	22 /* PDelay Request Frame Received */
#define MACB_PDRQFR_SIZE	1
#define MACB_PDRSFR_OFFSET	23 /* PDelay Response Frame Received */
#define MACB_PDRSFR_SIZE	1
#define MACB_PDRQFT_OFFSET	24 /* PDelay Request Frame Transmitted */
#define MACB_PDRQFT_SIZE	1
#define MACB_PDRSFT_OFFSET	25 /* PDelay Response Frame Transmitted */
#define MACB_PDRSFT_SIZE	1
#define MACB_SRI_OFFSET		26 /* TSU Seconds Register Increment */
#define MACB_SRI_SIZE		1

/* Timer increment fields */
#define MACB_TI_CNS_OFFSET	0
#define MACB_TI_CNS_SIZE	8
#define MACB_TI_ACNS_OFFSET	8
#define MACB_TI_ACNS_SIZE	8
#define MACB_TI_NIT_OFFSET	16
#define MACB_TI_NIT_SIZE	8

/* Bitfields in MAN */
#define MACB_DATA_OFFSET	0 /* data */
#define MACB_DATA_SIZE		16
#define MACB_CODE_OFFSET	16 /* Must be written to 10 */
#define MACB_CODE_SIZE		2
#define MACB_REGA_OFFSET	18 /* Register address */
#define MACB_REGA_SIZE		5
#define MACB_PHYA_OFFSET	23 /* PHY address */
#define MACB_PHYA_SIZE		5
#define MACB_RW_OFFSET		28 /* Operation. 10 is read. 01 is write. */
#define MACB_RW_SIZE		2
#define MACB_SOF_OFFSET		30 /* Must be written to 1 for Clause 22 */
#define MACB_SOF_SIZE		2

/* Bitfields in USRIO (AVR32) */
#define MACB_MII_OFFSET				0
#define MACB_MII_SIZE				1
#define MACB_EAM_OFFSET				1
#define MACB_EAM_SIZE				1
#define MACB_TX_PAUSE_OFFSET			2
#define MACB_TX_PAUSE_SIZE			1
#define MACB_TX_PAUSE_ZERO_OFFSET		3
#define MACB_TX_PAUSE_ZERO_SIZE			1

/* Bitfields in USRIO (AT91) */
#define MACB_RMII_OFFSET			0
#define MACB_RMII_SIZE				1
#define GEM_RGMII_OFFSET			0 /* GEM gigabit mode */
#define GEM_RGMII_SIZE				1
#define MACB_CLKEN_OFFSET			1
#define MACB_CLKEN_SIZE				1

/* Bitfields in WOL */
#define MACB_IP_OFFSET				0
#define MACB_IP_SIZE				16
#define MACB_MAG_OFFSET				16
#define MACB_MAG_SIZE				1
#define MACB_ARP_OFFSET				17
#define MACB_ARP_SIZE				1
#define MACB_SA1_OFFSET				18
#define MACB_SA1_SIZE				1
#define MACB_WOL_MTI_OFFSET			19
#define MACB_WOL_MTI_SIZE			1

/* Bitfields in MID */
#define MACB_IDNUM_OFFSET			16
#define MACB_IDNUM_SIZE				12
#define MACB_REV_OFFSET				0
#define MACB_REV_SIZE				16

/* Bitfields in DCFG1. */
#define GEM_IRQCOR_OFFSET			23
#define GEM_IRQCOR_SIZE				1
#define GEM_DBWDEF_OFFSET			25
#define GEM_DBWDEF_SIZE				3

/* Bitfields in DCFG2. */
#define GEM_RX_PKT_BUFF_OFFSET			20
#define GEM_RX_PKT_BUFF_SIZE			1
#define GEM_TX_PKT_BUFF_OFFSET			21
#define GEM_TX_PKT_BUFF_SIZE			1


/* Bitfields in DCFG5. */
#define GEM_TSU_OFFSET				8
#define GEM_TSU_SIZE				1

/* Bitfields in DCFG6. */
#define GEM_PBUF_LSO_OFFSET			27
#define GEM_PBUF_LSO_SIZE			1
#define GEM_DAW64_OFFSET			23
#define GEM_DAW64_SIZE				1

/* Bitfields in DCFG8. */
#define GEM_T1SCR_OFFSET			24
#define GEM_T1SCR_SIZE				8
#define GEM_T2SCR_OFFSET			16
#define GEM_T2SCR_SIZE				8
#define GEM_SCR2ETH_OFFSET			8
#define GEM_SCR2ETH_SIZE			8
#define GEM_SCR2CMP_OFFSET			0
#define GEM_SCR2CMP_SIZE			8

/* Bitfields in DCFG10 */
#define GEM_TXBD_RDBUFF_OFFSET			12
#define GEM_TXBD_RDBUFF_SIZE			4
#define GEM_RXBD_RDBUFF_OFFSET			8
#define GEM_RXBD_RDBUFF_SIZE			4

/* Bitfields in TISUBN */
#define GEM_SUBNSINCR_OFFSET			0
#define GEM_SUBNSINCR_SIZE			16

/* Bitfields in TI */
#define GEM_NSINCR_OFFSET			0
#define GEM_NSINCR_SIZE				8

/* Bitfields in TSH */
#define GEM_TSH_OFFSET				0 /* TSU timer value (s). MSB [47:32] of seconds timer count */
#define GEM_TSH_SIZE				16

/* Bitfields in TSL */
#define GEM_TSL_OFFSET				0 /* TSU timer value (s). LSB [31:0] of seconds timer count */
#define GEM_TSL_SIZE				32

/* Bitfields in TN */
#define GEM_TN_OFFSET				0 /* TSU timer value (ns) */
#define GEM_TN_SIZE					30

/* Bitfields in TXBDCTRL */
#define GEM_TXTSMODE_OFFSET			4 /* TX Descriptor Timestamp Insertion mode */
#define GEM_TXTSMODE_SIZE			2

/* Bitfields in RXBDCTRL */
#define GEM_RXTSMODE_OFFSET			4 /* RX Descriptor Timestamp Insertion mode */
#define GEM_RXTSMODE_SIZE			2

/* Bitfields in SCRT2 */
#define GEM_QUEUE_OFFSET			0 /* Queue Number */
#define GEM_QUEUE_SIZE				4
#define GEM_VLANPR_OFFSET			4 /* VLAN Priority */
#define GEM_VLANPR_SIZE				3
#define GEM_VLANEN_OFFSET			8 /* VLAN Enable */
#define GEM_VLANEN_SIZE				1
#define GEM_ETHT2IDX_OFFSET			9 /* Index to screener type 2 EtherType register */
#define GEM_ETHT2IDX_SIZE			3
#define GEM_ETHTEN_OFFSET			12 /* EtherType Enable */
#define GEM_ETHTEN_SIZE				1
#define GEM_CMPA_OFFSET				13 /* Compare A - Index to screener type 2 Compare register */
#define GEM_CMPA_SIZE				5
#define GEM_CMPAEN_OFFSET			18 /* Compare A Enable */
#define GEM_CMPAEN_SIZE				1
#define GEM_CMPB_OFFSET				19 /* Compare B - Index to screener type 2 Compare register */
#define GEM_CMPB_SIZE				5
#define GEM_CMPBEN_OFFSET			24 /* Compare B Enable */
#define GEM_CMPBEN_SIZE				1
#define GEM_CMPC_OFFSET				25 /* Compare C - Index to screener type 2 Compare register */
#define GEM_CMPC_SIZE				5
#define GEM_CMPCEN_OFFSET			30 /* Compare C Enable */
#define GEM_CMPCEN_SIZE				1

/* Bitfields in ETHT */
#define GEM_ETHTCMP_OFFSET			0 /* EtherType compare value */
#define GEM_ETHTCMP_SIZE			16

/* Bitfields in T2CMPW0 */
#define GEM_T2CMP_OFFSET			16 /* 0xFFFF0000 compare value */
#define GEM_T2CMP_SIZE				16
#define GEM_T2MASK_OFFSET			0 /* 0x0000FFFF compare value or mask */
#define GEM_T2MASK_SIZE				16

/* Bitfields in T2CMPW1 */
#define GEM_T2DISMSK_OFFSET			9 /* disable mask */
#define GEM_T2DISMSK_SIZE			1
#define GEM_T2CMPOFST_OFFSET			7 /* compare offset */
#define GEM_T2CMPOFST_SIZE			2
#define GEM_T2OFST_OFFSET			0 /* offset value */
#define GEM_T2OFST_SIZE				7

/* Offset for screener type 2 compare values (T2CMPOFST).
 * Note the offset is applied after the specified point,
 * e.g. GEM_T2COMPOFST_ETYPE denotes the EtherType field, so an offset
 * of 12 bytes from this would be the source IP address in an IP header
 */
#define GEM_T2COMPOFST_SOF		0
#define GEM_T2COMPOFST_ETYPE	1
#define GEM_T2COMPOFST_IPHDR	2
#define GEM_T2COMPOFST_TCPUDP	3

/* offset from EtherType to IP address */
#define ETYPE_SRCIP_OFFSET			12
#define ETYPE_DSTIP_OFFSET			16

/* offset from IP header to port */
#define IPHDR_SRCPORT_OFFSET		0
#define IPHDR_DSTPORT_OFFSET		2

/* Transmit DMA buffer descriptor Word 1 */
#define GEM_DMA_TXVALID_OFFSET		23 /* timestamp has been captured in the Buffer Descriptor */
#define GEM_DMA_TXVALID_SIZE		1

/* Receive DMA buffer descriptor Word 0 */
#define GEM_DMA_RXVALID_OFFSET		2 /* indicates a valid timestamp in the Buffer Descriptor */
#define GEM_DMA_RXVALID_SIZE		1

/* DMA buffer descriptor Word 2 (32 bit addressing) or Word 4 (64 bit addressing) */
#define GEM_DMA_SECL_OFFSET			30 /* Timestamp seconds[1:0]  */
#define GEM_DMA_SECL_SIZE			2
#define GEM_DMA_NSEC_OFFSET			0 /* Timestamp nanosecs [29:0] */
#define GEM_DMA_NSEC_SIZE			30

/* DMA buffer descriptor Word 3 (32 bit addressing) or Word 5 (64 bit addressing) */

/* New hardware supports 12 bit precision of timestamp in DMA buffer descriptor.
 * Old hardware supports only 6 bit precision but it is enough for PTP.
 * Less accuracy is used always instead of checking hardware version.
 */
#define GEM_DMA_SECH_OFFSET			0 /* Timestamp seconds[5:2] */
#define GEM_DMA_SECH_SIZE			4
#define GEM_DMA_SEC_WIDTH			(GEM_DMA_SECH_SIZE + GEM_DMA_SECL_SIZE)
#define GEM_DMA_SEC_TOP				(1 << GEM_DMA_SEC_WIDTH)
#define GEM_DMA_SEC_MASK			(GEM_DMA_SEC_TOP - 1)

/* Bitfields in ADJ */
#define GEM_ADDSUB_OFFSET			31
#define GEM_ADDSUB_SIZE				1
/* Constants for CLK */
#define MACB_CLK_DIV8				0
#define MACB_CLK_DIV16				1
#define MACB_CLK_DIV32				2
#define MACB_CLK_DIV64				3

/* GEM specific constants for CLK */
#define GEM_CLK_DIV8				0
#define GEM_CLK_DIV16				1
#define GEM_CLK_DIV32				2
#define GEM_CLK_DIV48				3
#define GEM_CLK_DIV64				4
#define GEM_CLK_DIV96				5
#define GEM_CLK_DIV128				6
#define GEM_CLK_DIV224				7

/* Constants for MAN register */
#define MACB_MAN_SOF				1
#define MACB_MAN_WRITE				1
#define MACB_MAN_READ				2
#define MACB_MAN_CODE				2

/* Capability mask bits */
#define MACB_CAPS_ISR_CLEAR_ON_WRITE		0x00000001
#define MACB_CAPS_USRIO_HAS_CLKEN		0x00000002
#define MACB_CAPS_USRIO_DEFAULT_IS_MII_GMII	0x00000004
#define MACB_CAPS_NO_GIGABIT_HALF		0x00000008
#define MACB_CAPS_USRIO_DISABLED		0x00000010
#define MACB_CAPS_JUMBO				0x00000020
#define MACB_CAPS_GEM_HAS_PTP			0x00000040
#define MACB_CAPS_BD_RD_PREFETCH		0x00000080
#define MACB_CAPS_NEEDS_RSTONUBR		0x00000100
#define MACB_CAPS_FIFO_MODE			0x10000000
#define MACB_CAPS_GIGABIT_MODE_AVAILABLE	0x20000000
#define MACB_CAPS_SG_DISABLED			0x40000000
#define MACB_CAPS_MACB_IS_GEM			0x80000000

/* LSO settings */
#define MACB_LSO_UFO_ENABLE			0x01
#define MACB_LSO_TSO_ENABLE			0x02

/* Bit manipulation macros */
#define MACB_BIT(name)					\
	(1 << MACB_##name##_OFFSET)
#define MACB_BF(name,value)				\
	(((value) & ((1 << MACB_##name##_SIZE) - 1))	\
	 << MACB_##name##_OFFSET)
#define MACB_BFEXT(name,value)\
	(((value) >> MACB_##name##_OFFSET)		\
	 & ((1 << MACB_##name##_SIZE) - 1))
#define MACB_BFINS(name,value,old)			\
	(((old) & ~(((1 << MACB_##name##_SIZE) - 1)	\
		    << MACB_##name##_OFFSET))		\
	 | MACB_BF(name,value))

#define GEM_BIT(name)					\
	(1 << GEM_##name##_OFFSET)
#define GEM_BF(name, value)				\
	(((value) & ((1 << GEM_##name##_SIZE) - 1))	\
	 << GEM_##name##_OFFSET)
#define GEM_BFEXT(name, value)\
	(((value) >> GEM_##name##_OFFSET)		\
	 & ((1 << GEM_##name##_SIZE) - 1))
#define GEM_BFINS(name, value, old)			\
	(((old) & ~(((1 << GEM_##name##_SIZE) - 1)	\
		    << GEM_##name##_OFFSET))		\
	 | GEM_BF(name, value))

/* Register access macros */
#define macb_readl(port, reg)				\
	readl((port)->regs + MACB_##reg)
#define macb_writel(port, reg, value)			\
	writel((value), (port)->regs + MACB_##reg)
#define gem_readl(port, reg)				\
	readl((port)->regs + GEM_##reg)
#define gem_writel(port, reg, value)			\
	writel((value), (port)->regs + GEM_##reg)

/* DMA descriptor bitfields */
#define MACB_RX_USED_OFFSET			0
#define MACB_RX_USED_SIZE			1
#define MACB_RX_WRAP_OFFSET			1
#define MACB_RX_WRAP_SIZE			1
#define MACB_RX_WADDR_OFFSET			2
#define MACB_RX_WADDR_SIZE			30

#define MACB_RX_FRMLEN_OFFSET			0
#define MACB_RX_FRMLEN_SIZE			12
#define MACB_RX_OFFSET_OFFSET			12
#define MACB_RX_OFFSET_SIZE			2
#define MACB_RX_SOF_OFFSET			14
#define MACB_RX_SOF_SIZE			1
#define MACB_RX_EOF_OFFSET			15
#define MACB_RX_EOF_SIZE			1
#define MACB_RX_CFI_OFFSET			16
#define MACB_RX_CFI_SIZE			1
#define MACB_RX_VLAN_PRI_OFFSET			17
#define MACB_RX_VLAN_PRI_SIZE			3
#define MACB_RX_PRI_TAG_OFFSET			20
#define MACB_RX_PRI_TAG_SIZE			1
#define MACB_RX_VLAN_TAG_OFFSET			21
#define MACB_RX_VLAN_TAG_SIZE			1
#define MACB_RX_TYPEID_MATCH_OFFSET		22
#define MACB_RX_TYPEID_MATCH_SIZE		1
#define MACB_RX_SA4_MATCH_OFFSET		23
#define MACB_RX_SA4_MATCH_SIZE			1
#define MACB_RX_SA3_MATCH_OFFSET		24
#define MACB_RX_SA3_MATCH_SIZE			1
#define MACB_RX_SA2_MATCH_OFFSET		25
#define MACB_RX_SA2_MATCH_SIZE			1
#define MACB_RX_SA1_MATCH_OFFSET		26
#define MACB_RX_SA1_MATCH_SIZE			1
#define MACB_RX_EXT_MATCH_OFFSET		28
#define MACB_RX_EXT_MATCH_SIZE			1
#define MACB_RX_UHASH_MATCH_OFFSET		29
#define MACB_RX_UHASH_MATCH_SIZE		1
#define MACB_RX_MHASH_MATCH_OFFSET		30
#define MACB_RX_MHASH_MATCH_SIZE		1
#define MACB_RX_BROADCAST_OFFSET		31
#define MACB_RX_BROADCAST_SIZE			1

#define MACB_RX_FRMLEN_MASK			0xFFF
#define MACB_RX_JFRMLEN_MASK			0x3FFF

/* RX checksum offload disabled: bit 24 clear in NCFGR */
#define GEM_RX_TYPEID_MATCH_OFFSET		22
#define GEM_RX_TYPEID_MATCH_SIZE		2

/* RX checksum offload enabled: bit 24 set in NCFGR */
#define GEM_RX_CSUM_OFFSET			22
#define GEM_RX_CSUM_SIZE			2

#define MACB_TX_FRMLEN_OFFSET			0
#define MACB_TX_FRMLEN_SIZE			11
#define MACB_TX_LAST_OFFSET			15
#define MACB_TX_LAST_SIZE			1
#define MACB_TX_NOCRC_OFFSET			16
#define MACB_TX_NOCRC_SIZE			1
#define MACB_MSS_MFS_OFFSET			16
#define MACB_MSS_MFS_SIZE			14
#define MACB_TX_LSO_OFFSET			17
#define MACB_TX_LSO_SIZE			2
#define MACB_TX_TCP_SEQ_SRC_OFFSET		19
#define MACB_TX_TCP_SEQ_SRC_SIZE		1
#define MACB_TX_BUF_EXHAUSTED_OFFSET		27
#define MACB_TX_BUF_EXHAUSTED_SIZE		1
#define MACB_TX_UNDERRUN_OFFSET			28
#define MACB_TX_UNDERRUN_SIZE			1
#define MACB_TX_ERROR_OFFSET			29
#define MACB_TX_ERROR_SIZE			1
#define MACB_TX_WRAP_OFFSET			30
#define MACB_TX_WRAP_SIZE			1
#define MACB_TX_USED_OFFSET			31
#define MACB_TX_USED_SIZE			1

#define GEM_TX_FRMLEN_OFFSET			0
#define GEM_TX_FRMLEN_SIZE			14

/* Buffer descriptor constants */
#define GEM_RX_CSUM_NONE			0
#define GEM_RX_CSUM_IP_ONLY			1
#define GEM_RX_CSUM_IP_TCP			2
#define GEM_RX_CSUM_IP_UDP			3

/* limit RX checksum offload to TCP and UDP packets */
#define GEM_RX_CSUM_CHECKED_MASK		2
#define gem_writel_queue_TBQP(port, value, queue_num)	\
	writel((value), (port)->regs + GEM_TBQP(queue_num))
#define gem_writel_queue_TBQPH(port, value, queue_num)	\
	writel((value), (port)->regs + GEM_TBQPH(queue_num))
#define gem_writel_queue_RBQP(port, value, queue_num)	\
	writel((value), (port)->regs + GEM_RBQP(queue_num))
#define gem_writel_queue_RBQPH(port, value, queue_num)	\
	writel((value), (port)->regs + GEM_RBQPH(queue_num))

#endif /* __DRIVERS_MACB_H__ */
