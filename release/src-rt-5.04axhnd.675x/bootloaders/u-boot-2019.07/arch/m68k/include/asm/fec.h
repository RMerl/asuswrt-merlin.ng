/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * fec.h -- Fast Ethernet Controller definitions
 *
 * Some definitions copied from commproc.h for MPC8xx:
 * MPC8xx Communication Processor Module.
 * Copyright (c) 1997 Dan Malek (dmalek@jlc.net)
 *
 * Add FEC Structure and definitions
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef	fec_h
#define	fec_h

#include <phy.h>

/* Buffer descriptors used FEC.
*/
typedef struct cpm_buf_desc {
	ushort cbd_sc;		/* Status and Control */
	ushort cbd_datlen;	/* Data length in buffer */
	uint cbd_bufaddr;	/* Buffer address in host memory */
} cbd_t;

#define BD_SC_EMPTY		((ushort)0x8000)	/* Receive is empty */
#define BD_SC_READY		((ushort)0x8000)	/* Transmit is ready */
#define BD_SC_WRAP		((ushort)0x2000)	/* Last buffer descriptor */
#define BD_SC_INTRPT		((ushort)0x1000)	/* Interrupt on change */
#define BD_SC_LAST		((ushort)0x0800)	/* Last buffer in frame */
#define BD_SC_TC		((ushort)0x0400)	/* Transmit CRC */
#define BD_SC_CM		((ushort)0x0200)	/* Continous mode */
#define BD_SC_ID		((ushort)0x0100)	/* Rec'd too many idles */
#define BD_SC_P			((ushort)0x0100)	/* xmt preamble */
#define BD_SC_BR		((ushort)0x0020)	/* Break received */
#define BD_SC_FR		((ushort)0x0010)	/* Framing error */
#define BD_SC_PR		((ushort)0x0008)	/* Parity error */
#define BD_SC_OV		((ushort)0x0002)	/* Overrun */
#define BD_SC_CD		((ushort)0x0001)	/* Carrier Detect lost */

/* Buffer descriptor control/status used by Ethernet receive.
*/
#define BD_ENET_RX_EMPTY	((ushort)0x8000)
#define BD_ENET_RX_RO1		((ushort)0x4000)
#define BD_ENET_RX_WRAP		((ushort)0x2000)
#define BD_ENET_RX_INTR		((ushort)0x1000)
#define BD_ENET_RX_RO2		BD_ENET_RX_INTR
#define BD_ENET_RX_LAST		((ushort)0x0800)
#define BD_ENET_RX_FIRST	((ushort)0x0400)
#define BD_ENET_RX_MISS		((ushort)0x0100)
#define BD_ENET_RX_BC		((ushort)0x0080)
#define BD_ENET_RX_MC		((ushort)0x0040)
#define BD_ENET_RX_LG		((ushort)0x0020)
#define BD_ENET_RX_NO		((ushort)0x0010)
#define BD_ENET_RX_SH		((ushort)0x0008)
#define BD_ENET_RX_CR		((ushort)0x0004)
#define BD_ENET_RX_OV		((ushort)0x0002)
#define BD_ENET_RX_CL		((ushort)0x0001)
#define BD_ENET_RX_TR		BD_ENET_RX_CL
#define BD_ENET_RX_STATS	((ushort)0x013f)	/* All status bits */

/* Buffer descriptor control/status used by Ethernet transmit.
*/
#define BD_ENET_TX_READY	((ushort)0x8000)
#define BD_ENET_TX_PAD		((ushort)0x4000)
#define BD_ENET_TX_TO1		BD_ENET_TX_PAD
#define BD_ENET_TX_WRAP		((ushort)0x2000)
#define BD_ENET_TX_INTR		((ushort)0x1000)
#define BD_ENET_TX_TO2		BD_ENET_TX_INTR_
#define BD_ENET_TX_LAST		((ushort)0x0800)
#define BD_ENET_TX_TC		((ushort)0x0400)
#define BD_ENET_TX_DEF		((ushort)0x0200)
#define BD_ENET_TX_ABC		BD_ENET_TX_DEF
#define BD_ENET_TX_HB		((ushort)0x0100)
#define BD_ENET_TX_LC		((ushort)0x0080)
#define BD_ENET_TX_RL		((ushort)0x0040)
#define BD_ENET_TX_RCMASK	((ushort)0x003c)
#define BD_ENET_TX_UN		((ushort)0x0002)
#define BD_ENET_TX_CSL		((ushort)0x0001)
#define BD_ENET_TX_STATS	((ushort)0x03ff)	/* All status bits */

/*********************************************************************
* Fast Ethernet Controller (FEC)
*********************************************************************/
/* FEC private information */
struct fec_info_s {
	int index;
	u32 iobase;
	u32 pinmux;
	u32 miibase;
	int phy_addr;
	int dup_spd;
	char *phy_name;
	int phyname_init;
	cbd_t *rxbd;		/* Rx BD */
	cbd_t *txbd;		/* Tx BD */
	uint rxIdx;
	uint txIdx;
	char *txbuf;
	int initialized;
	struct fec_info_s *next;
};

#ifdef CONFIG_MCFFEC
/* Register read/write struct */
typedef struct fec {
#ifdef CONFIG_M5272
	u32 ecr;		/* 0x00 */
	u32 eir;		/* 0x04 */
	u32 eimr;		/* 0x08 */
	u32 ivsr;		/* 0x0C */
	u32 rdar;		/* 0x10 */
	u32 tdar;		/* 0x14 */
	u8 resv1[0x28];		/* 0x18 */
	u32 mmfr;		/* 0x40 */
	u32 mscr;		/* 0x44 */
	u8 resv2[0x44];		/* 0x48 */
	u32 frbr;		/* 0x8C */
	u32 frsr;		/* 0x90 */
	u8 resv3[0x10];		/* 0x94 */
	u32 tfwr;		/* 0xA4 */
	u32 res4;		/* 0xA8 */
	u32 tfsr;		/* 0xAC */
	u8 resv4[0x50];		/* 0xB0 */
	u32 opd;		/* 0x100 - dummy  */
	u32 rcr;		/* 0x104 */
	u32 mibc;		/* 0x108 */
	u8 resv5[0x38];		/* 0x10C */
	u32 tcr;		/* 0x144 */
	u8 resv6[0x270];	/* 0x148 */
	u32 iaur;		/* 0x3B8 - dummy */
	u32 ialr;		/* 0x3BC - dummy  */
	u32 palr;		/* 0x3C0 */
	u32 paur;		/* 0x3C4 */
	u32 gaur;		/* 0x3C8 */
	u32 galr;		/* 0x3CC */
	u32 erdsr;		/* 0x3D0 */
	u32 etdsr;		/* 0x3D4 */
	u32 emrbr;		/* 0x3D8 */
	u8 resv12[0x74];	/* 0x18C */
#else
	u8 resv0[0x4];
	u32 eir;
	u32 eimr;
	u8 resv1[0x4];
	u32 rdar;
	u32 tdar;
	u8 resv2[0xC];
	u32 ecr;
	u8 resv3[0x18];
	u32 mmfr;
	u32 mscr;
	u8 resv4[0x1C];
	u32 mibc;
	u8 resv5[0x1C];
	u32 rcr;
	u8 resv6[0x3C];
	u32 tcr;
	u8 resv7[0x1C];
	u32 palr;
	u32 paur;
	u32 opd;
	u8 resv8[0x28];
	u32 iaur;
	u32 ialr;
	u32 gaur;
	u32 galr;
	u8 resv9[0x1C];
	u32 tfwr;
	u8 resv10[0x4];
	u32 frbr;
	u32 frsr;
	u8 resv11[0x2C];
	u32 erdsr;
	u32 etdsr;
	u32 emrbr;
	u8 resv12[0x74];
#endif

	u32 rmon_t_drop;
	u32 rmon_t_packets;
	u32 rmon_t_bc_pkt;
	u32 rmon_t_mc_pkt;
	u32 rmon_t_crc_align;
	u32 rmon_t_undersize;
	u32 rmon_t_oversize;
	u32 rmon_t_frag;
	u32 rmon_t_jab;
	u32 rmon_t_col;
	u32 rmon_t_p64;
	u32 rmon_t_p65to127;
	u32 rmon_t_p128to255;
	u32 rmon_t_p256to511;
	u32 rmon_t_p512to1023;
	u32 rmon_t_p1024to2047;
	u32 rmon_t_p_gte2048;
	u32 rmon_t_octets;

	u32 ieee_t_drop;
	u32 ieee_t_frame_ok;
	u32 ieee_t_1col;
	u32 ieee_t_mcol;
	u32 ieee_t_def;
	u32 ieee_t_lcol;
	u32 ieee_t_excol;
	u32 ieee_t_macerr;
	u32 ieee_t_cserr;
	u32 ieee_t_sqe;
	u32 ieee_t_fdxfc;
	u32 ieee_t_octets_ok;
	u8 resv13[0x8];

	u32 rmon_r_drop;
	u32 rmon_r_packets;
	u32 rmon_r_bc_pkt;
	u32 rmon_r_mc_pkt;
	u32 rmon_r_crc_align;
	u32 rmon_r_undersize;
	u32 rmon_r_oversize;
	u32 rmon_r_frag;
	u32 rmon_r_jab;
	u32 rmon_r_resvd_0;
	u32 rmon_r_p64;
	u32 rmon_r_p65to127;
	u32 rmon_r_p128to255;
	u32 rmon_r_p256to511;
	u32 rmon_r_p512to1023;
	u32 rmon_r_p1024to2047;
	u32 rmon_r_p_gte2048;
	u32 rmon_r_octets;

	u32 ieee_r_drop;
	u32 ieee_r_frame_ok;
	u32 ieee_r_crc;
	u32 ieee_r_align;
	u32 ieee_r_macerr;
	u32 ieee_r_fdxfc;
	u32 ieee_r_octets_ok;
} fec_t;
#endif				/* CONFIG_MCFFEC */

/*********************************************************************
* Fast Ethernet Controller (FEC)
*********************************************************************/
/* Bit definitions and macros for FEC_EIR */
#define FEC_EIR_CLEAR_ALL		(0xFFF80000)
#define FEC_EIR_HBERR			(0x80000000)
#define FEC_EIR_BABR			(0x40000000)
#define FEC_EIR_BABT			(0x20000000)
#define FEC_EIR_GRA			(0x10000000)
#define FEC_EIR_TXF			(0x08000000)
#define FEC_EIR_TXB			(0x04000000)
#define FEC_EIR_RXF			(0x02000000)
#define FEC_EIR_RXB			(0x01000000)
#define FEC_EIR_MII			(0x00800000)
#define FEC_EIR_EBERR			(0x00400000)
#define FEC_EIR_LC			(0x00200000)
#define FEC_EIR_RL			(0x00100000)
#define FEC_EIR_UN			(0x00080000)

/* Bit definitions and macros for FEC_RDAR */
#define FEC_RDAR_R_DES_ACTIVE		(0x01000000)

/* Bit definitions and macros for FEC_TDAR */
#define FEC_TDAR_X_DES_ACTIVE		(0x01000000)

/* Bit definitions and macros for FEC_ECR */
#define FEC_ECR_ETHER_EN		(0x00000002)
#define FEC_ECR_RESET			(0x00000001)

/* Bit definitions and macros for FEC_MMFR */
#define FEC_MMFR_DATA(x)		(((x)&0xFFFF))
#define FEC_MMFR_ST(x)			(((x)&0x03)<<30)
#define FEC_MMFR_ST_01			(0x40000000)
#define FEC_MMFR_OP_RD			(0x20000000)
#define FEC_MMFR_OP_WR			(0x10000000)
#define FEC_MMFR_PA(x)			(((x)&0x1F)<<23)
#define FEC_MMFR_RA(x)			(((x)&0x1F)<<18)
#define FEC_MMFR_TA(x)			(((x)&0x03)<<16)
#define FEC_MMFR_TA_10			(0x00020000)

/* Bit definitions and macros for FEC_MSCR */
#define FEC_MSCR_DIS_PREAMBLE		(0x00000080)
#define FEC_MSCR_MII_SPEED(x)		(((x)&0x3F)<<1)

/* Bit definitions and macros for FEC_MIBC */
#define FEC_MIBC_MIB_DISABLE		(0x80000000)
#define FEC_MIBC_MIB_IDLE		(0x40000000)

/* Bit definitions and macros for FEC_RCR */
#define FEC_RCR_MAX_FL(x)		(((x)&0x7FF)<<16)
#define FEC_RCR_FCE			(0x00000020)
#define FEC_RCR_BC_REJ			(0x00000010)
#define FEC_RCR_PROM			(0x00000008)
#define FEC_RCR_MII_MODE		(0x00000004)
#define FEC_RCR_DRT			(0x00000002)
#define FEC_RCR_LOOP			(0x00000001)

/* Bit definitions and macros for FEC_TCR */
#define FEC_TCR_RFC_PAUSE		(0x00000010)
#define FEC_TCR_TFC_PAUSE		(0x00000008)
#define FEC_TCR_FDEN			(0x00000004)
#define FEC_TCR_HBC			(0x00000002)
#define FEC_TCR_GTS			(0x00000001)

/* Bit definitions and macros for FEC_PAUR */
#define FEC_PAUR_PADDR2(x)		(((x)&0xFFFF)<<16)
#define FEC_PAUR_TYPE(x)		((x)&0xFFFF)

/* Bit definitions and macros for FEC_OPD */
#define FEC_OPD_PAUSE_DUR(x)		(((x)&0x0000FFFF)<<0)
#define FEC_OPD_OPCODE(x)		(((x)&0x0000FFFF)<<16)

/* Bit definitions and macros for FEC_TFWR */
#define FEC_TFWR_X_WMRK(x)		((x)&0x03)
#define FEC_TFWR_X_WMRK_64		(0x01)
#define FEC_TFWR_X_WMRK_128		(0x02)
#define FEC_TFWR_X_WMRK_192		(0x03)

/* Bit definitions and macros for FEC_FRBR */
#define FEC_FRBR_R_BOUND(x)		(((x)&0xFF)<<2)

/* Bit definitions and macros for FEC_FRSR */
#define FEC_FRSR_R_FSTART(x)		(((x)&0xFF)<<2)

/* Bit definitions and macros for FEC_ERDSR */
#define FEC_ERDSR_R_DES_START(x)	(((x)&0x3FFFFFFF)<<2)

/* Bit definitions and macros for FEC_ETDSR */
#define FEC_ETDSR_X_DES_START(x)	(((x)&0x3FFFFFFF)<<2)

/* Bit definitions and macros for FEC_EMRBR */
#define FEC_EMRBR_R_BUF_SIZE(x)		(((x)&0x7F)<<4)

#define	FEC_RESET_DELAY			100
#define FEC_RX_TOUT			100

int fecpin_setclear(struct eth_device *dev, int setclear);

#ifdef CONFIG_SYS_DISCOVER_PHY
void __mii_init(void);
uint mii_send(uint mii_cmd);
int mii_discover_phy(struct eth_device *dev);
int mcffec_miiphy_read(struct mii_dev *bus, int addr, int devad, int reg);
int mcffec_miiphy_write(struct mii_dev *bus, int addr, int devad, int reg,
			u16 value);
#endif

#endif				/* fec_h */
