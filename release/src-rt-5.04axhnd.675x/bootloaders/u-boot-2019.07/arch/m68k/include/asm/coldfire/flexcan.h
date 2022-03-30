/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Flex CAN Memory Map
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __FLEXCAN_H__
#define __FLEXCAN_H__

/* FlexCan Message Buffer */
typedef struct can_msgbuf_ctrl {
#ifdef CONFIG_M5282
	u8 tmstamp;		/* 0x00 Timestamp */
	u8 ctrl;		/* 0x01 Control */
	u16 idh;		/* 0x02 ID High */
	u16 idl;		/* 0x04 ID High */
	u8 data[8];		/* 0x06 8 Byte Data Field */
	u16 res;		/* 0x0E */
#else
	u16 ctrl;		/* 0x00 Control/Status */
	u16 tmstamp;		/* 0x02 Timestamp */
	u32 id;			/* 0x04 Identifier */
	u8 data[8];		/* 0x08 8 Byte Data Field */
#endif
} can_msg_t;

#ifdef CONFIG_M5282
/* MSGBUF CTRL */
#define CAN_MSGBUF_CTRL_CODE(x)		(((x) & 0x0F) << 4)
#define CAN_MSGBUF_CTRL_CODE_MASK	(0x0F)
#define CAN_MSGBUF_CTRL_LEN(x)		((x) & 0x0F)
#define CAN_MSGBUF_CTRL_LEN_MASK	(0xF0)

/* MSGBUF ID */
#define CAN_MSGBUF_IDH_STD(x)		(((x) & 0x07FF) << 5)
#define CAN_MSGBUF_IDH_STD_MASK		(0xE003FFFF)
#define CAN_MSGBUF_IDH_SRR		(0x0010)
#define CAN_MSGBUF_IDH_IDE		(0x0080)
#define CAN_MSGBUF_IDH_EXTH(x)		((x) & 0x07)
#define CAN_MSGBUF_IDH_EXTH_MASK	(0xFFF8)
#define CAN_MSGBUF_IDL_EXTL(x)		(((x) & 0x7FFF) << 1)
#define CAN_MSGBUF_IDL_EXTL_MASK		(0xFFFE)
#define CAN_MSGBUF_IDL_RTR		(0x0001)
#else
/* MSGBUF CTRL */
#define CAN_MSGBUF_CTRL_CODE(x)		(((x) & 0x000F) << 8)
#define CAN_MSGBUF_CTRL_CODE_MASK	(0xF0FF)
#define CAN_MSGBUF_CTRL_SRR		(0x0040)
#define CAN_MSGBUF_CTRL_IDE		(0x0020)
#define CAN_MSGBUF_CTRL_RTR		(0x0010)
#define CAN_MSGBUF_CTRL_LEN(x)		((x) & 0x000F)
#define CAN_MSGBUF_CTRL_LEN_MASK	(0xFFF0)

/* MSGBUF ID */
#define CAN_MSGBUF_ID_STD(x)		(((x) & 0x000007FF) << 18)
#define CAN_MSGBUF_ID_STD_MASK		(0xE003FFFF)
#define CAN_MSGBUF_ID_EXT(x)		((x) & 0x0003FFFF)
#define CAN_MSGBUF_ID_EXT_MASK		(0xFFFC0000)
#endif

/* FlexCan module */
typedef struct can_ctrl {
	u32 mcr;		/* 0x00 Module Configuration */
	u32 ctrl;		/* 0x04 Control */
	u32 timer;		/* 0x08 Free Running Timer */
	u32 res1;		/* 0x0C */
	u32 rxgmsk;		/* 0x10 Rx Global Mask */
	u32 rx14msk;		/* 0x14 RxBuffer 14 Mask */
	u32 rx15msk;		/* 0x18 RxBuffer 15 Mask */
#ifdef CONFIG_M5282
	u32 res2;		/* 0x1C */
	u16 errstat;		/* 0x20 Error and status */
	u16 imsk;		/* 0x22 Interrupt Mask */
	u16 iflag;		/* 0x24 Interrupt Flag */
	u16 errcnt;		/* 0x26 Error Counter */
	u32 res3[3];		/* 0x28 - 0x33 */
#else
	u16 res2;		/* 0x1C */
	u16 errcnt;		/* 0x1E Error Counter */
	u16 res3;		/* 0x20 */
	u16 errstat;		/* 0x22 Error and status */
	u32 res4;		/* 0x24 */
	u32 imsk;		/* 0x28 Interrupt Mask */
	u32 res5;		/* 0x2C */
	u16 iflag;		/* 0x30 Interrupt Flag */
#endif
	u32 res6[19];		/* 0x34 - 0x7F */
	void *msgbuf;		/* 0x80 Message Buffer 0-15 */
} can_t;

/* MCR */
#define CAN_MCR_MDIS			(0x80000000)
#define CAN_MCR_FRZ			(0x40000000)
#define CAN_MCR_HALT			(0x10000000)
#define CAN_MCR_NORDY			(0x08000000)
#define CAN_MCF_WAKEMSK			(0x04000000)	/* 5282 */
#define CAN_MCR_SOFTRST			(0x02000000)
#define CAN_MCR_FRZACK			(0x01000000)
#define CAN_MCR_SUPV			(0x00800000)
#define CAN_MCR_SELFWAKE		(0x00400000)	/* 5282 */
#define CAN_MCR_APS			(0x00200000)	/* 5282 */
#define CAN_MCR_LPMACK			(0x00100000)
#define CAN_MCF_BCC			(0x00010000)
#define CAN_MCR_MAXMB(x)		((x) & 0x0F)
#define CAN_MCR_MAXMB_MASK		(0xFFFFFFF0)

/* CTRL */
#define CAN_CTRL_PRESDIV(x)		(((x) & 0xFF) << 24)
#define CAN_CTRL_PRESDIV_MASK		(0x00FFFFFF)
#define CAN_CTRL_RJW(x)			(((x) & 0x03) << 22)
#define CAN_CTRL_RJW_MASK		(0xFF3FFFFF)
#define CAN_CTRL_PSEG1(x)		(((x) & 0x07) << 19)
#define CAN_CTRL_PSEG1_MASK		(0xFFC7FFFF)
#define CAN_CTRL_PSEG2(x)		(((x) & 0x07) << 16)
#define CAN_CTRL_PSEG2_MASK		(0xFFF8FFFF)
#define CAN_CTRL_BOFFMSK		(0x00008000)
#define CAN_CTRL_ERRMSK			(0x00004000)
#define CAN_CTRL_CLKSRC			(0x00002000)
#define CAN_CTRL_LPB			(0x00001000)
#define CAN_CTRL_RXMODE			(0x00000400)	/* 5282 */
#define CAN_CTRL_TXMODE(x)		(((x) & 0x03) << 8)	/* 5282 */
#define CAN_CTRL_TXMODE_MASK		(0xFFFFFCFF)	/* 5282 */
#define CAN_CTRL_TXMODE_CAN0		(0x00000000)	/* 5282 */
#define CAN_CTRL_TXMODE_CAN1		(0x00000100)	/* 5282 */
#define CAN_CTRL_TXMODE_OPEN		(0x00000200)	/* 5282 */
#define CAN_CTRL_SMP			(0x00000080)
#define CAN_CTRL_BOFFREC		(0x00000040)
#define CAN_CTRL_TSYNC			(0x00000020)
#define CAN_CTRL_LBUF			(0x00000010)
#define CAN_CTRL_LOM			(0x00000008)
#define CAN_CTRL_PROPSEG(x)		((x) & 0x07)
#define CAN_CTRL_PROPSEG_MASK		(0xFFFFFFF8)

/* TIMER */
/* Note: PRESDIV, RJW, PSG1, and PSG2 are part of timer in 5282 */
#define CAN_TIMER(x)			((x) & 0xFFFF)
#define CAN_TIMER_MASK			(0xFFFF0000)

/* RXGMASK */
#ifdef CONFIG_M5282
#define CAN_RXGMSK_MI_STD(x)		(((x) & 0x000007FF) << 21)
#define CAN_RXGMSK_MI_STD_MASK		(0x001FFFFF)
#define CAN_RXGMSK_MI_EXT(x)		(((x) & 0x0003FFFF) << 1)
#define CAN_RXGMSK_MI_EXT_MASK		(0xFFF80001)
#else
#define CAN_RXGMSK_MI_STD(x)		(((x) & 0x000007FF) << 18)
#define CAN_RXGMSK_MI_STD_MASK		(0xE003FFFF)
#define CAN_RXGMSK_MI_EXT(x)		((x) & 0x0003FFFF)
#define CAN_RXGMSK_MI_EXT_MASK		(0xFFFC0000)
#endif

/* ERRCNT */
#define CAN_ERRCNT_RXECTR(x)		(((x) & 0xFF) << 8)
#define CAN_ERRCNT_RXECTR_MASK		(0x00FF)
#define CAN_ERRCNT_TXECTR(x)		((x) & 0xFF)
#define CAN_ERRCNT_TXECTR_MASK		(0xFF00)

/* ERRSTAT */
#define CAN_ERRSTAT_BITERR1		(0x8000)
#define CAN_ERRSTAT_BITERR0		(0x4000)
#define CAN_ERRSTAT_ACKERR		(0x2000)
#define CAN_ERRSTAT_CRCERR		(0x1000)
#define CAN_ERRSTAT_FRMERR		(0x0800)
#define CAN_ERRSTAT_STFERR		(0x0400)
#define CAN_ERRSTAT_TXWRN		(0x0200)
#define CAN_ERRSTAT_RXWRN		(0x0100)
#define CAN_ERRSTAT_IDLE		(0x0080)
#define CAN_ERRSTAT_TXRX		(0x0040)
#define CAN_ERRSTAT_FLT_MASK		(0xFFCF)
#define CAN_ERRSTAT_FLT_BUSOFF		(0x0020)
#define CAN_ERRSTAT_FLT_PASSIVE		(0x0010)
#define CAN_ERRSTAT_FLT_ACTIVE		(0x0000)
#ifdef CONFIG_M5282
#define CAN_ERRSTAT_BOFFINT		(0x0004)
#define CAN_ERRSTAT_ERRINT		(0x0002)
#else
#define CAN_ERRSTAT_ERRINT		(0x0004)
#define CAN_ERRSTAT_BOFFINT		(0x0002)
#define CAN_ERRSTAT_WAKEINT		(0x0001)
#endif

/* IMASK */
#ifdef CONFIG_M5253
#define CAN_IMASK_BUFnM(x)		(1 << (x & 0xFFFFFFFF))
#define CAN_IMASK_BUFnM_MASKBIT(x)	~CAN_IMASK_BUFnM(x)
#else
#define CAN_IMASK_BUFnM(x)		(1 << (x & 0xFFFF))
#define CAN_IMASK_BUFnM_MASKBIT(x)	~CAN_IMASK_BUFnM(x)
#endif

/* IFLAG */
#ifdef CONFIG_M5253
#define CAN_IFLAG_BUFnM(x)		(1 << (x & 0xFFFFFFFF))
#define CAN_IFLAG_BUFnM_MASKBIT(x)	~CAN_IFLAG_BUFnM(x)
#else
#define CAN_IFLAG_BUFnM(x)		(1 << (x & 0xFFFF))
#define CAN_IFLAG_BUFnM_MASKBIT(x)	~CAN_IFLAG_BUFnM(x)
#endif

#endif				/* __FLEXCAN_H__ */
