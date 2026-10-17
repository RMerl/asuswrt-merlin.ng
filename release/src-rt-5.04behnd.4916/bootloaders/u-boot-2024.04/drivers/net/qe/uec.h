/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2006-2010 Freescale Semiconductor, Inc.
 *
 * Dave Liu <daveliu@freescale.com>
 * based on source code of Shlomi Gridish
 */

#ifndef __UEC_H__
#define __UEC_H__

#include "uccf.h"
#include <fsl_qe.h>
#include <phy.h>

#define MAX_TX_THREADS				8
#define MAX_RX_THREADS				8
#define MAX_TX_QUEUES				8
#define MAX_RX_QUEUES				8
#define MAX_PREFETCHED_BDS			4
#define MAX_IPH_OFFSET_ENTRY			8
#define MAX_ENET_INIT_PARAM_ENTRIES_RX		9
#define MAX_ENET_INIT_PARAM_ENTRIES_TX		8

/* UEC UPSMR (Protocol Specific Mode Register)
 */
#define UPSMR_ECM	0x04000000 /* Enable CAM Miss               */
#define UPSMR_HSE	0x02000000 /* Hardware Statistics Enable    */
#define UPSMR_PRO	0x00400000 /* Promiscuous                   */
#define UPSMR_CAP	0x00200000 /* CAM polarity                  */
#define UPSMR_RSH	0x00100000 /* Receive Short Frames          */
#define UPSMR_RPM	0x00080000 /* Reduced Pin Mode interfaces   */
#define UPSMR_R10M	0x00040000 /* RGMII/RMII 10 Mode            */
#define UPSMR_RLPB	0x00020000 /* RMII Loopback Mode            */
#define UPSMR_TBIM	0x00010000 /* Ten-bit Interface Mode        */
#define UPSMR_RMM	0x00001000 /* RMII/RGMII Mode               */
#define UPSMR_CAM	0x00000400 /* CAM Address Matching          */
#define UPSMR_BRO	0x00000200 /* Broadcast Address             */
#define UPSMR_RES1	0x00002000 /* Reserved feild - must be 1    */
#define UPSMR_SGMM	0x00000020 /* SGMII mode    */

#define UPSMR_INIT_VALUE	(UPSMR_HSE | UPSMR_RES1)

/* UEC MACCFG1 (MAC Configuration 1 Register)
 */
#define MACCFG1_FLOW_RX			0x00000020 /* Flow Control Rx */
#define MACCFG1_FLOW_TX			0x00000010 /* Flow Control Tx */
#define MACCFG1_ENABLE_SYNCHED_RX	0x00000008 /* Enable Rx Sync  */
#define MACCFG1_ENABLE_RX		0x00000004 /* Enable Rx       */
#define MACCFG1_ENABLE_SYNCHED_TX	0x00000002 /* Enable Tx Sync  */
#define MACCFG1_ENABLE_TX		0x00000001 /* Enable Tx       */

#define MACCFG1_INIT_VALUE		(0)

/* UEC MACCFG2 (MAC Configuration 2 Register)
 */
#define MACCFG2_PREL				0x00007000
#define MACCFG2_PREL_SHIFT			(31 - 19)
#define MACCFG2_PREL_MASK			0x0000f000
#define MACCFG2_SRP				0x00000080
#define MACCFG2_STP				0x00000040
#define MACCFG2_RESERVED_1			0x00000020 /* must be set  */
#define MACCFG2_LC				0x00000010 /* Length Check */
#define MACCFG2_MPE				0x00000008
#define MACCFG2_FDX				0x00000001 /* Full Duplex  */
#define MACCFG2_FDX_MASK			0x00000001
#define MACCFG2_PAD_CRC				0x00000004
#define MACCFG2_CRC_EN				0x00000002
#define MACCFG2_PAD_AND_CRC_MODE_NONE		0x00000000
#define MACCFG2_PAD_AND_CRC_MODE_CRC_ONLY	0x00000002
#define MACCFG2_PAD_AND_CRC_MODE_PAD_AND_CRC	0x00000004
#define MACCFG2_INTERFACE_MODE_NIBBLE		0x00000100
#define MACCFG2_INTERFACE_MODE_BYTE		0x00000200
#define MACCFG2_INTERFACE_MODE_MASK		0x00000300

#define MACCFG2_INIT_VALUE	(MACCFG2_PREL | MACCFG2_RESERVED_1 | \
				 MACCFG2_LC | MACCFG2_PAD_CRC | MACCFG2_FDX)

/* UEC Event Register */
#define UCCE_MPD				0x80000000
#define UCCE_SCAR				0x40000000
#define UCCE_GRA				0x20000000
#define UCCE_CBPR				0x10000000
#define UCCE_BSY				0x08000000
#define UCCE_RXC				0x04000000
#define UCCE_TXC				0x02000000
#define UCCE_TXE				0x01000000
#define UCCE_TXB7				0x00800000
#define UCCE_TXB6				0x00400000
#define UCCE_TXB5				0x00200000
#define UCCE_TXB4				0x00100000
#define UCCE_TXB3				0x00080000
#define UCCE_TXB2				0x00040000
#define UCCE_TXB1				0x00020000
#define UCCE_TXB0				0x00010000
#define UCCE_RXB7				0x00008000
#define UCCE_RXB6				0x00004000
#define UCCE_RXB5				0x00002000
#define UCCE_RXB4				0x00001000
#define UCCE_RXB3				0x00000800
#define UCCE_RXB2				0x00000400
#define UCCE_RXB1				0x00000200
#define UCCE_RXB0				0x00000100
#define UCCE_RXF7				0x00000080
#define UCCE_RXF6				0x00000040
#define UCCE_RXF5				0x00000020
#define UCCE_RXF4				0x00000010
#define UCCE_RXF3				0x00000008
#define UCCE_RXF2				0x00000004
#define UCCE_RXF1				0x00000002
#define UCCE_RXF0				0x00000001

#define UCCE_TXB	(UCCE_TXB7 | UCCE_TXB6 | UCCE_TXB5 | UCCE_TXB4 | \
			 UCCE_TXB3 | UCCE_TXB2 | UCCE_TXB1 | UCCE_TXB0)
#define UCCE_RXB	(UCCE_RXB7 | UCCE_RXB6 | UCCE_RXB5 | UCCE_RXB4 | \
			 UCCE_RXB3 | UCCE_RXB2 | UCCE_RXB1 | UCCE_RXB0)
#define UCCE_RXF	(UCCE_RXF7 | UCCE_RXF6 | UCCE_RXF5 | UCCE_RXF4 | \
			 UCCE_RXF3 | UCCE_RXF2 | UCCE_RXF1 | UCCE_RXF0)
#define UCCE_OTHER	(UCCE_SCAR | UCCE_GRA  | UCCE_CBPR | UCCE_BSY  | \
			 UCCE_RXC  | UCCE_TXC  | UCCE_TXE)

/* UEC TEMODR Register */
#define TEMODER_SCHEDULER_ENABLE		0x2000
#define TEMODER_IP_CHECKSUM_GENERATE		0x0400
#define TEMODER_PERFORMANCE_OPTIMIZATION_MODE1	0x0200
#define TEMODER_RMON_STATISTICS			0x0100
#define TEMODER_NUM_OF_QUEUES_SHIFT		(15 - 15)

#define TEMODER_INIT_VALUE			0xc000

/* UEC REMODR Register */
#define REMODER_RX_RMON_STATISTICS_ENABLE	0x00001000
#define REMODER_RX_EXTENDED_FEATURES		0x80000000
#define REMODER_VLAN_OPERATION_TAGGED_SHIFT	(31 - 9)
#define REMODER_VLAN_OPERATION_NON_TAGGED_SHIFT	(31 - 10)
#define REMODER_RX_QOS_MODE_SHIFT		(31 - 15)
#define REMODER_RMON_STATISTICS			0x00001000
#define REMODER_RX_EXTENDED_FILTERING		0x00000800
#define REMODER_NUM_OF_QUEUES_SHIFT		(31 - 23)
#define REMODER_DYNAMIC_MAX_FRAME_LENGTH	0x00000008
#define REMODER_DYNAMIC_MIN_FRAME_LENGTH	0x00000004
#define REMODER_IP_CHECKSUM_CHECK		0x00000002
#define REMODER_IP_ADDRESS_ALIGNMENT		0x00000001

#define REMODER_INIT_VALUE			0

/* BMRx - Bus Mode Register */
#define BMR_GLB					0x20
#define BMR_BO_BE				0x10
#define BMR_DTB_SECONDARY_BUS			0x02
#define BMR_BDB_SECONDARY_BUS			0x01

#define BMR_SHIFT				24
#define BMR_INIT_VALUE				(BMR_GLB | BMR_BO_BE)

/* UEC UCCS (Ethernet Status Register)
 */
#define UCCS_BPR				0x02
#define UCCS_PAU				0x02
#define UCCS_MPD				0x01

/* UEC MIIMCFG (MII Management Configuration Register)
 */
#define MIIMCFG_RESET_MANAGEMENT		0x80000000
#define MIIMCFG_NO_PREAMBLE			0x00000010
#define MIIMCFG_CLOCK_DIVIDE_SHIFT		(31 - 31)
#define MIIMCFG_CLOCK_DIVIDE_MASK		0x0000000f
#define MIIMCFG_MANAGEMENT_CLOCK_DIVIDE_BY_4	0x00000001
#define MIIMCFG_MANAGEMENT_CLOCK_DIVIDE_BY_6	0x00000002
#define MIIMCFG_MANAGEMENT_CLOCK_DIVIDE_BY_8	0x00000003
#define MIIMCFG_MANAGEMENT_CLOCK_DIVIDE_BY_10	0x00000004
#define MIIMCFG_MANAGEMENT_CLOCK_DIVIDE_BY_14	0x00000005
#define MIIMCFG_MANAGEMENT_CLOCK_DIVIDE_BY_20	0x00000006
#define MIIMCFG_MANAGEMENT_CLOCK_DIVIDE_BY_28	0x00000007

#define MIIMCFG_MNGMNT_CLC_DIV_INIT_VALUE	\
	MIIMCFG_MANAGEMENT_CLOCK_DIVIDE_BY_10

/* UEC MIIMCOM (MII Management Command Register)
 */
#define MIIMCOM_SCAN_CYCLE			0x00000002 /* Scan cycle */
#define MIIMCOM_READ_CYCLE			0x00000001 /* Read cycle */

/* UEC MIIMADD (MII Management Address Register)
 */
#define MIIMADD_PHY_ADDRESS_SHIFT		(31 - 23)
#define MIIMADD_PHY_REGISTER_SHIFT		(31 - 31)

/* UEC MIIMCON (MII Management Control Register)
 */
#define MIIMCON_PHY_CONTROL_SHIFT		(31 - 31)
#define MIIMCON_PHY_STATUS_SHIFT		(31 - 31)

/* UEC MIIMIND (MII Management Indicator Register)
 */
#define MIIMIND_NOT_VALID			0x00000004
#define MIIMIND_SCAN				0x00000002
#define MIIMIND_BUSY				0x00000001

/* UEC UTBIPAR (Ten Bit Interface Physical Address Register)
 */
#define UTBIPAR_PHY_ADDRESS_SHIFT		(31 - 31)
#define UTBIPAR_PHY_ADDRESS_MASK		0x0000001f

/* UEC UESCR (Ethernet Statistics Control Register)
 */
#define UESCR_AUTOZ				0x8000
#define UESCR_CLRCNT				0x4000
#define UESCR_MAXCOV_SHIFT			(15 -  7)
#define UESCR_SCOV_SHIFT			(15 - 15)

/****** Tx data struct collection ******/
/* Tx thread data, each Tx thread has one this struct. */
struct uec_thread_data_tx {
	u8   res0[136];
} __packed;

/* Tx thread parameter, each Tx thread has one this struct. */
struct uec_thread_tx_pram {
	u8   res0[64];
} __packed;

/* Send queue queue-descriptor, each Tx queue has one this QD */
struct uec_send_queue_qd {
	u32    bd_ring_base; /* pointer to BD ring base address */
	u8     res0[0x8];
	u32    last_bd_completed_address; /* last entry in BD ring */
	u8     res1[0x30];
} __packed;

/* Send queue memory region */
struct uec_send_queue_mem_region {
	struct uec_send_queue_qd   sqqd[MAX_TX_QUEUES];
} __packed;

/* Scheduler struct */
struct uec_scheduler {
	u16  cpucount0;        /* CPU packet counter */
	u16  cpucount1;        /* CPU packet counter */
	u16  cecount0;         /* QE  packet counter */
	u16  cecount1;         /* QE  packet counter */
	u16  cpucount2;        /* CPU packet counter */
	u16  cpucount3;        /* CPU packet counter */
	u16  cecount2;         /* QE  packet counter */
	u16  cecount3;         /* QE  packet counter */
	u16  cpucount4;        /* CPU packet counter */
	u16  cpucount5;        /* CPU packet counter */
	u16  cecount4;         /* QE  packet counter */
	u16  cecount5;         /* QE  packet counter */
	u16  cpucount6;        /* CPU packet counter */
	u16  cpucount7;        /* CPU packet counter */
	u16  cecount6;         /* QE  packet counter */
	u16  cecount7;         /* QE  packet counter */
	u32  weightstatus[MAX_TX_QUEUES]; /* accumulated weight factor */
	u32  rtsrshadow;       /* temporary variable handled by QE */
	u32  time;             /* temporary variable handled by QE */
	u32  ttl;              /* temporary variable handled by QE */
	u32  mblinterval;      /* max burst length interval        */
	u16  nortsrbytetime;   /* normalized value of byte time in tsr units */
	u8   fracsiz;
	u8   res0[1];
	u8   strictpriorityq;  /* Strict Priority Mask register */
	u8   txasap;           /* Transmit ASAP register        */
	u8   extrabw;          /* Extra BandWidth register      */
	u8   oldwfqmask;       /* temporary variable handled by QE */
	u8   weightfactor[MAX_TX_QUEUES]; /**< weight factor for queues */
	u32  minw;             /* temporary variable handled by QE */
	u8   res1[0x70 - 0x64];
} __packed;

/* Tx firmware counters */
struct uec_tx_firmware_statistics_pram {
	u32  sicoltx;            /* single collision */
	u32  mulcoltx;           /* multiple collision */
	u32  latecoltxfr;        /* late collision */
	u32  frabortduecol;      /* frames aborted due to tx collision */
	u32  frlostinmactxer;    /* frames lost due to internal MAC error tx */
	u32  carriersenseertx;   /* carrier sense error */
	u32  frtxok;             /* frames transmitted OK */
	u32  txfrexcessivedefer;
	u32  txpkts256;          /* total packets(including bad) 256~511 B */
	u32  txpkts512;          /* total packets(including bad) 512~1023B */
	u32  txpkts1024;         /* total packets(including bad) 1024~1518B */
	u32  txpktsjumbo;        /* total packets(including bad)  >1024 */
} __packed;

/* Tx global parameter table */
struct uec_tx_global_pram {
	u16  temoder;
	u8   res0[0x38 - 0x02];
	u32  sqptr;
	u32  schedulerbasepointer;
	u32  txrmonbaseptr;
	u32  tstate;
	u8   iphoffset[MAX_IPH_OFFSET_ENTRY];
	u32  vtagtable[0x8];
	u32  tqptr;
	u8   res2[0x80 - 0x74];
} __packed;

/****** Rx data struct collection ******/
/* Rx thread data, each Rx thread has one this struct. */
struct uec_thread_data_rx {
	u8   res0[40];
} __packed;

/* Rx thread parameter, each Rx thread has one this struct. */
struct uec_thread_rx_pram {
	u8   res0[128];
} __packed;

/* Rx firmware counters */
struct uec_rx_firmware_statistics_pram {
	u32   frrxfcser;         /* frames with crc error */
	u32   fraligner;         /* frames with alignment error */
	u32   inrangelenrxer;    /* in range length error */
	u32   outrangelenrxer;   /* out of range length error */
	u32   frtoolong;         /* frame too long */
	u32   runt;              /* runt */
	u32   verylongevent;     /* very long event */
	u32   symbolerror;       /* symbol error */
	u32   dropbsy;           /* drop because of BD not ready */
	u8    res0[0x8];
	u32   mismatchdrop;      /* drop because of MAC filtering */
	u32   underpkts;         /* total frames less than 64 octets */
	u32   pkts256;           /* total frames(including bad)256~511 B */
	u32   pkts512;           /* total frames(including bad)512~1023 B */
	u32   pkts1024;          /* total frames(including bad)1024~1518 B */
	u32   pktsjumbo;         /* total frames(including bad) >1024 B */
	u32   frlossinmacer;
	u32   pausefr;           /* pause frames */
	u8    res1[0x4];
	u32   removevlan;
	u32   replacevlan;
	u32   insertvlan;
} __packed;

/* Rx interrupt coalescing entry, each Rx queue has one this entry. */
struct uec_rx_interrupt_coalescing_entry {
	u32   maxvalue;
	u32   counter;
} __packed;

struct uec_rx_interrupt_coalescing_table {
	struct uec_rx_interrupt_coalescing_entry   entry[MAX_RX_QUEUES];
} __packed;

/* RxBD queue entry, each Rx queue has one this entry. */
struct uec_rx_bd_queues_entry {
	u32   bdbaseptr;         /* BD base pointer          */
	u32   bdptr;             /* BD pointer               */
	u32   externalbdbaseptr; /* external BD base pointer */
	u32   externalbdptr;     /* external BD pointer      */
} __packed;

/* Rx global parameter table */
struct uec_rx_global_pram {
	u32  remoder;             /* ethernet mode reg. */
	u32  rqptr;               /* base pointer to the Rx Queues */
	u32  res0[0x1];
	u8   res1[0x20 - 0xc];
	u16  typeorlen;
	u8   res2[0x1];
	u8   rxgstpack;           /* ack on GRACEFUL STOP RX command */
	u32  rxrmonbaseptr;       /* Rx RMON statistics base */
	u8   res3[0x30 - 0x28];
	u32  intcoalescingptr;    /* Interrupt coalescing table pointer */
	u8   res4[0x36 - 0x34];
	u8   rstate;
	u8   res5[0x46 - 0x37];
	u16  mrblr;               /* max receive buffer length reg. */
	u32  rbdqptr;             /* RxBD parameter table description */
	u16  mflr;                /* max frame length reg. */
	u16  minflr;              /* min frame length reg. */
	u16  maxd1;               /* max dma1 length reg. */
	u16  maxd2;               /* max dma2 length reg. */
	u32  ecamptr;             /* external CAM address */
	u32  l2qt;                /* VLAN priority mapping table. */
	u32  l3qt[0x8];           /* IP   priority mapping table. */
	u16  vlantype;            /* vlan type */
	u16  vlantci;             /* default vlan tci */
	u8   addressfiltering[64];/* address filtering data structure */
	u32  exf_global_param;      /* extended filtering global parameters */
	u8   res6[0x100 - 0xc4];    /* Initialize to zero */
} __packed;

#define GRACEFUL_STOP_ACKNOWLEDGE_RX            0x01

/****** UEC common ******/
/* UCC statistics - hardware counters */
struct uec_hardware_statistics {
	u32 tx64;
	u32 tx127;
	u32 tx255;
	u32 rx64;
	u32 rx127;
	u32 rx255;
	u32 txok;
	u16 txcf;
	u32 tmca;
	u32 tbca;
	u32 rxfok;
	u32 rxbok;
	u32 rbyt;
	u32 rmca;
	u32 rbca;
} __packed;

/* InitEnet command parameter */
struct uec_init_cmd_pram {
	u8   resinit0;
	u8   resinit1;
	u8   resinit2;
	u8   resinit3;
	u16  resinit4;
	u8   res1[0x1];
	u8   largestexternallookupkeysize;
	u32  rgftgfrxglobal;
	u32  rxthread[MAX_ENET_INIT_PARAM_ENTRIES_RX]; /* rx threads */
	u8   res2[0x38 - 0x30];
	u32  txglobal;				   /* tx global  */
	u32  txthread[MAX_ENET_INIT_PARAM_ENTRIES_TX]; /* tx threads */
	u8   res3[0x1];
} __packed;

#define ENET_INIT_PARAM_RGF_SHIFT		(32 - 4)
#define ENET_INIT_PARAM_TGF_SHIFT		(32 - 8)

#define ENET_INIT_PARAM_RISC_MASK		0x0000003f
#define ENET_INIT_PARAM_PTR_MASK		0x00ffffc0
#define ENET_INIT_PARAM_SNUM_MASK		0xff000000
#define ENET_INIT_PARAM_SNUM_SHIFT		24

#define ENET_INIT_PARAM_MAGIC_RES_INIT0		0x06
#define ENET_INIT_PARAM_MAGIC_RES_INIT1		0x30
#define ENET_INIT_PARAM_MAGIC_RES_INIT2		0xff
#define ENET_INIT_PARAM_MAGIC_RES_INIT3		0x00
#define ENET_INIT_PARAM_MAGIC_RES_INIT4		0x0400

/* structure representing 82xx Address Filtering Enet Address in PRAM */
struct uec_82xx_enet_addr {
	u8   res1[0x2];
	u16  h;       /* address (MSB) */
	u16  m;       /* address       */
	u16  l;       /* address (LSB) */
} __packed;

/* structure representing 82xx Address Filtering PRAM */
struct uec_82xx_add_filtering_pram {
	u32  iaddr_h;        /* individual address filter, high */
	u32  iaddr_l;        /* individual address filter, low  */
	u32  gaddr_h;        /* group address filter, high      */
	u32  gaddr_l;        /* group address filter, low       */
	struct uec_82xx_enet_addr    taddr;
	struct uec_82xx_enet_addr    paddr[4];
	u8                         res0[0x40 - 0x38];
} __packed;

/* Buffer Descriptor */
struct buffer_descriptor {
	u16 status;
	u16 len;
	u32 data;
} __packed;

#define	SIZEOFBD	sizeof(struct buffer_descriptor)

/* Common BD flags */
#define BD_WRAP			0x2000
#define BD_INT			0x1000
#define BD_LAST			0x0800
#define BD_CLEAN		0x3000

/* TxBD status flags */
#define TX_BD_READY		0x8000
#define TX_BD_PADCRC		0x4000
#define TX_BD_WRAP		BD_WRAP
#define TX_BD_INT		BD_INT
#define TX_BD_LAST		BD_LAST
#define TX_BD_TXCRC		0x0400
#define TX_BD_DEF		0x0200
#define TX_BD_PP			0x0100
#define TX_BD_LC			0x0080
#define TX_BD_RL			0x0040
#define TX_BD_RC			0x003C
#define TX_BD_UNDERRUN		0x0002
#define TX_BD_TRUNC		0x0001

#define TX_BD_ERROR		(TX_BD_UNDERRUN | TX_BD_TRUNC)

/* RxBD status flags */
#define RX_BD_EMPTY		0x8000
#define RX_BD_OWNER		0x4000
#define RX_BD_WRAP		BD_WRAP
#define RX_BD_INT		BD_INT
#define RX_BD_LAST		BD_LAST
#define RX_BD_FIRST		0x0400
#define RX_BD_CMR		0x0200
#define RX_BD_MISS		0x0100
#define RX_BD_BCAST		0x0080
#define RX_BD_MCAST		0x0040
#define RX_BD_LG			0x0020
#define RX_BD_NO			0x0010
#define RX_BD_SHORT		0x0008
#define RX_BD_CRCERR		0x0004
#define RX_BD_OVERRUN		0x0002
#define RX_BD_IPCH		0x0001

#define RX_BD_ERROR		(RX_BD_LG | RX_BD_NO | RX_BD_SHORT | \
				 RX_BD_CRCERR | RX_BD_OVERRUN)

/* BD access macros */
#define BD_STATUS(_bd)		(in_be16(&((_bd)->status)))
#define BD_STATUS_SET(_bd, _v)	(out_be16(&((_bd)->status), _v))
#define BD_LENGTH(_bd)		(in_be16(&((_bd)->len)))
#define BD_LENGTH_SET(_bd, _v)	(out_be16(&((_bd)->len), _v))
#define BD_DATA_CLEAR(_bd)	(out_be32(&((_bd)->data), 0))
#define BD_DATA(_bd)		((u8 *)(((_bd)->data)))
#define BD_DATA_SET(_bd, _data)	(out_be32(&((_bd)->data), (u32)_data))
#define BD_ADVANCE(_bd, _status, _base)	\
	(((_status) & BD_WRAP) ? (_bd) = \
	 ((struct buffer_descriptor *)(_base)) : ++(_bd))

/* Rx Prefetched BDs */
struct uec_rx_pref_bds {
	struct buffer_descriptor   bd[MAX_PREFETCHED_BDS]; /* prefetched bd */
} __packed;

/* Alignments */
#define UEC_RX_GLOBAL_PRAM_ALIGNMENT				64
#define UEC_TX_GLOBAL_PRAM_ALIGNMENT				64
#define UEC_THREAD_RX_PRAM_ALIGNMENT				128
#define UEC_THREAD_TX_PRAM_ALIGNMENT				64
#define UEC_THREAD_DATA_ALIGNMENT				256
#define UEC_SEND_QUEUE_QUEUE_DESCRIPTOR_ALIGNMENT		32
#define UEC_SCHEDULER_ALIGNMENT					4
#define UEC_TX_STATISTICS_ALIGNMENT				4
#define UEC_RX_STATISTICS_ALIGNMENT				4
#define UEC_RX_INTERRUPT_COALESCING_ALIGNMENT			4
#define UEC_RX_BD_QUEUES_ALIGNMENT				8
#define UEC_RX_PREFETCHED_BDS_ALIGNMENT				128
#define UEC_RX_EXTENDED_FILTERING_GLOBAL_PARAMETERS_ALIGNMENT	4
#define UEC_RX_BD_RING_ALIGNMENT				32
#define UEC_TX_BD_RING_ALIGNMENT				32
#define UEC_MRBLR_ALIGNMENT					128
#define UEC_RX_BD_RING_SIZE_ALIGNMENT				4
#define UEC_TX_BD_RING_SIZE_MEMORY_ALIGNMENT			32
#define UEC_RX_DATA_BUF_ALIGNMENT				64

#define UEC_VLAN_PRIORITY_MAX					8
#define UEC_IP_PRIORITY_MAX					64
#define UEC_TX_VTAG_TABLE_ENTRY_MAX				8
#define UEC_RX_BD_RING_SIZE_MIN					8
#define UEC_TX_BD_RING_SIZE_MIN					2

/* TBI / MII Set Register */
enum enet_tbi_mii_reg {
	ENET_TBI_MII_CR        = 0x00,
	ENET_TBI_MII_SR        = 0x01,
	ENET_TBI_MII_ANA       = 0x04,
	ENET_TBI_MII_ANLPBPA   = 0x05,
	ENET_TBI_MII_ANEX      = 0x06,
	ENET_TBI_MII_ANNPT     = 0x07,
	ENET_TBI_MII_ANLPANP   = 0x08,
	ENET_TBI_MII_EXST      = 0x0F,
	ENET_TBI_MII_JD        = 0x10,
	ENET_TBI_MII_TBICON    = 0x11
};

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

#define TBIANA_SETTINGS ( \
		TBIANA_ASYMMETRIC_PAUSE \
		| TBIANA_SYMMETRIC_PAUSE \
		| TBIANA_FULL_DUPLEX \
		)

#define TBICR_SETTINGS ( \
		TBICR_PHY_RESET \
		| TBICR_ANEG_ENABLE \
		| TBICR_FULL_DUPLEX \
		| TBICR_SPEED1_SET \
		)

/* UEC number of threads */
enum uec_num_of_threads {
	UEC_NUM_OF_THREADS_1  = 0x1,  /* 1 */
	UEC_NUM_OF_THREADS_2  = 0x2,  /* 2 */
	UEC_NUM_OF_THREADS_4  = 0x0,  /* 4 */
	UEC_NUM_OF_THREADS_6  = 0x3,  /* 6 */
	UEC_NUM_OF_THREADS_8  = 0x4   /* 8 */
};

/* UEC initialization info struct */
#define STD_UEC_INFO(num) \
{			\
	.uf_info		= {	\
		.ucc_num	= CFG_SYS_UEC##num##_UCC_NUM,\
		.rx_clock	= CFG_SYS_UEC##num##_RX_CLK,	\
		.tx_clock	= CFG_SYS_UEC##num##_TX_CLK,	\
		.eth_type	= CFG_SYS_UEC##num##_ETH_TYPE,\
	},	\
	.num_threads_tx		= UEC_NUM_OF_THREADS_1,	\
	.num_threads_rx		= UEC_NUM_OF_THREADS_1,	\
	.risc_tx		= QE_RISC_ALLOCATION_RISC1_AND_RISC2, \
	.risc_rx		= QE_RISC_ALLOCATION_RISC1_AND_RISC2, \
	.tx_bd_ring_len		= 16,	\
	.rx_bd_ring_len		= 16,	\
	.phy_address		= CFG_SYS_UEC##num##_PHY_ADDR, \
	.enet_interface_type	= CFG_SYS_UEC##num##_INTERFACE_TYPE, \
	.speed			= CFG_SYS_UEC##num##_INTERFACE_SPEED, \
}

struct uec_inf {
	struct ucc_fast_inf		uf_info;
	enum uec_num_of_threads		num_threads_tx;
	enum uec_num_of_threads		num_threads_rx;
	unsigned int			risc_tx;
	unsigned int			risc_rx;
	u16				rx_bd_ring_len;
	u16				tx_bd_ring_len;
	u8				phy_address;
	phy_interface_t			enet_interface_type;
	int				speed;
};

/* UEC driver initialized info */
#define MAX_RXBUF_LEN			1536
#define MAX_FRAME_LEN			1518
#define MIN_FRAME_LEN			64
#define MAX_DMA1_LEN			1520
#define MAX_DMA2_LEN			1520

/* UEC driver private struct */
struct uec_priv {
	struct uec_inf			*uec_info;
	struct ucc_fast_priv		*uccf;
	struct eth_device		*dev;
	uec_t				*uec_regs;
	/* enet init command parameter */
	struct uec_init_cmd_pram		*p_init_enet_param;
	u32				init_enet_param_offset;
	/* Rx and Tx parameter */
	struct uec_rx_global_pram		*p_rx_glbl_pram;
	u32				rx_glbl_pram_offset;
	struct uec_tx_global_pram		*p_tx_glbl_pram;
	u32				tx_glbl_pram_offset;
	struct uec_send_queue_mem_region	*p_send_q_mem_reg;
	u32				send_q_mem_reg_offset;
	struct uec_thread_data_tx		*p_thread_data_tx;
	u32				thread_dat_tx_offset;
	struct uec_thread_data_rx		*p_thread_data_rx;
	u32				thread_dat_rx_offset;
	struct uec_rx_bd_queues_entry	*p_rx_bd_qs_tbl;
	u32				rx_bd_qs_tbl_offset;
	/* BDs specific */
	u8				*p_tx_bd_ring;
	u32				tx_bd_ring_offset;
	u8				*p_rx_bd_ring;
	u32				rx_bd_ring_offset;
	u8				*p_rx_buf;
	u32				rx_buf_offset;
	struct buffer_descriptor	*tx_bd;
	struct buffer_descriptor	*rx_bd;
	/* Status */
	int				mac_tx_enabled;
	int				mac_rx_enabled;
	int				grace_stopped_tx;
	int				grace_stopped_rx;
	int				the_first_run;
#if !defined(CONFIG_DM)
	/* PHY specific */
	struct uec_mii_info		*mii_info;
	int				oldspeed;
	int				oldduplex;
	int				oldlink;
#endif
};

int uec_initialize(struct bd_info *bis, struct uec_inf *uec_info);
int uec_eth_init(struct bd_info *bis, struct uec_inf *uecs, int num);
#endif /* __UEC_H__ */
