/* SPDX-License-Identifier: GPL-2.0 */
/*
 * R8A66597 HCD (Host Controller Driver) for u-boot
 *
 * Copyright (C) 2008  Yoshihiro Shimoda <shimoda.yoshihiro@renesas.com>
 */

#ifndef __R8A66597_H__
#define __R8A66597_H__

#include <linux/bitops.h>
#define SYSCFG0		0x00
#define SYSCFG1		0x02
#define SYSSTS0		0x04
#define SYSSTS1		0x06
#define DVSTCTR0	0x08
#define DVSTCTR1	0x0A
#define TESTMODE	0x0C
#define PINCFG		0x0E
#define DMA0CFG		0x10
#define DMA1CFG		0x12
#define CFIFO		0x14
#define D0FIFO		0x18
#define D1FIFO		0x1C
#define CFIFOSEL	0x20
#define CFIFOCTR	0x22
#define CFIFOSIE	0x24
#define D0FIFOSEL	0x28
#define D0FIFOCTR	0x2A
#define D1FIFOSEL	0x2C
#define D1FIFOCTR	0x2E
#define INTENB0		0x30
#define INTENB1		0x32
#define INTENB2		0x34
#define BRDYENB		0x36
#define NRDYENB		0x38
#define BEMPENB		0x3A
#define SOFCFG		0x3C
#define INTSTS0		0x40
#define INTSTS1		0x42
#define INTSTS2		0x44
#define BRDYSTS		0x46
#define NRDYSTS		0x48
#define BEMPSTS		0x4A
#define FRMNUM		0x4C
#define UFRMNUM		0x4E
#define USBADDR		0x50
#define USBREQ		0x54
#define USBVAL		0x56
#define USBINDX		0x58
#define USBLENG		0x5A
#define DCPCFG		0x5C
#define DCPMAXP		0x5E
#define DCPCTR		0x60
#define PIPESEL		0x64
#define PIPECFG		0x68
#define PIPEBUF		0x6A
#define PIPEMAXP	0x6C
#define PIPEPERI	0x6E
#define PIPE1CTR	0x70
#define PIPE2CTR	0x72
#define PIPE3CTR	0x74
#define PIPE4CTR	0x76
#define PIPE5CTR	0x78
#define PIPE6CTR	0x7A
#define PIPE7CTR	0x7C
#define PIPE8CTR	0x7E
#define PIPE9CTR	0x80
#define PIPE1TRE	0x90
#define PIPE1TRN	0x92
#define PIPE2TRE	0x94
#define PIPE2TRN	0x96
#define PIPE3TRE	0x98
#define PIPE3TRN	0x9A
#define PIPE4TRE	0x9C
#define PIPE4TRN	0x9E
#define PIPE5TRE	0xA0
#define PIPE5TRN	0xA2
#define DEVADD0		0xD0
#define DEVADD1		0xD2
#define DEVADD2		0xD4
#define DEVADD3		0xD6
#define DEVADD4		0xD8
#define DEVADD5		0xDA
#define DEVADD6		0xDC
#define DEVADD7		0xDE
#define DEVADD8		0xE0
#define DEVADD9		0xE2
#define DEVADDA		0xE4
#define SUSPMODE0	0x102	/* RZ/A only */

/* System Configuration Control Register */
#define HSE		0x0080	/* b7: Hi-speed enable */
#define DCFM		0x0040	/* b6: Controller function select  */
#define DRPD		0x0020	/* b5: D+/- pull down control */
#define DPRPU		0x0010	/* b4: D+ pull up control */
#define XTAL		0x0004	/* b2: Crystal selection */
#define XTAL12		0x0004	/* 12MHz */
#define XTAL48		0x0000	/* 48MHz */
#define UPLLE		0x0002	/* b1: internal PLL control */
#define USBE		0x0001	/* b0: USB module operation enable */

/* System Configuration Status Register */
#define OVCBIT		0x8000	/* b15-14: Over-current bit */
#define OVCMON		0xC000	/* b15-14: Over-current monitor */
#define SOFEA		0x0020	/* b5: SOF monitor */
#define IDMON		0x0004	/* b3: ID-pin monitor */
#define LNST		0x0003	/* b1-0: D+, D- line status */
#define SE1		0x0003	/* SE1 */
#define FS_KSTS		0x0002	/* Full-Speed K State */
#define FS_JSTS		0x0001	/* Full-Speed J State */
#define LS_JSTS		0x0002	/* Low-Speed J State */
#define LS_KSTS		0x0001	/* Low-Speed K State */
#define SE0		0x0000	/* SE0 */

/* Device State Control Register */
#define EXTLP0		0x0400	/* b10: External port */
#define VBOUT		0x0200	/* b9: VBUS output */
#define WKUP		0x0100	/* b8: Remote wakeup */
#define RWUPE		0x0080	/* b7: Remote wakeup sense */
#define USBRST		0x0040	/* b6: USB reset enable */
#define RESUME		0x0020	/* b5: Resume enable */
#define UACT		0x0010	/* b4: USB bus enable */
#define RHST		0x0007	/* b1-0: Reset handshake status */
#define HSPROC		0x0004	/* HS handshake is processing */
#define HSMODE		0x0003	/* Hi-Speed mode */
#define FSMODE		0x0002	/* Full-Speed mode */
#define LSMODE		0x0001	/* Low-Speed mode */
#define UNDECID		0x0000	/* Undecided */

/* Test Mode Register */
#define UTST		0x000F	/* b3-0: Test select */
#define H_TST_PACKET	0x000C	/* HOST TEST Packet */
#define H_TST_SE0_NAK	0x000B	/* HOST TEST SE0 NAK */
#define H_TST_K		0x000A	/* HOST TEST K */
#define H_TST_J		0x0009	/* HOST TEST J */
#define H_TST_NORMAL	0x0000	/* HOST Normal Mode */
#define P_TST_PACKET	0x0004	/* PERI TEST Packet */
#define P_TST_SE0_NAK	0x0003	/* PERI TEST SE0 NAK */
#define P_TST_K		0x0002	/* PERI TEST K */
#define P_TST_J		0x0001	/* PERI TEST J */
#define P_TST_NORMAL	0x0000	/* PERI Normal Mode */

/* Data Pin Configuration Register */
#define LDRV		0x8000	/* b15: Drive Current Adjust */
#define VIF1		0x0000	/* VIF = 1.8V */
#define VIF3		0x8000	/* VIF = 3.3V */
#define INTA		0x0001	/* b1: USB INT-pin active */

/* DMAx Pin Configuration Register */
#define DREQA		0x4000	/* b14: Dreq active select */
#define BURST		0x2000	/* b13: Burst mode */
#define DACKA		0x0400	/* b10: Dack active select */
#define DFORM		0x0380	/* b9-7: DMA mode select */
#define CPU_ADR_RD_WR	0x0000	/* Address + RD/WR mode (CPU bus) */
#define CPU_DACK_RD_WR	0x0100	/* DACK + RD/WR mode (CPU bus) */
#define CPU_DACK_ONLY	0x0180	/* DACK only mode (CPU bus) */
#define SPLIT_DACK_ONLY	0x0200	/* DACK only mode (SPLIT bus) */
#define DENDA		0x0040	/* b6: Dend active select */
#define PKTM		0x0020	/* b5: Packet mode */
#define DENDE		0x0010	/* b4: Dend enable */
#define OBUS		0x0004	/* b2: OUTbus mode */

/* CFIFO/DxFIFO Port Select Register */
#define RCNT		0x8000	/* b15: Read count mode */
#define REW		0x4000	/* b14: Buffer rewind */
#define DCLRM		0x2000	/* b13: DMA buffer clear mode */
#define DREQE		0x1000	/* b12: DREQ output enable */
#define MBW		0x0800	/* b10: Maximum bit width for FIFO access */
#define MBW_8		0x0000	/*  8bit */
#define MBW_16		0x0400	/* 16bit */
#define MBW_32		0x0800	/* 32bit */
#define BIGEND		0x0100	/* b8: Big endian mode */
#define BYTE_LITTLE	0x0000	/* little dendian */
#define BYTE_BIG	0x0100	/* big endifan */
#define ISEL		0x0020	/* b5: DCP FIFO port direction select */
#define CURPIPE		0x000F	/* b2-0: PIPE select */

/* CFIFO/DxFIFO Port Control Register */
#define BVAL		0x8000	/* b15: Buffer valid flag */
#define BCLR		0x4000	/* b14: Buffer clear */
#define FRDY		0x2000	/* b13: FIFO ready */
#define DTLN		0x0FFF	/* b11-0: FIFO received data length */

/* Interrupt Enable Register 0 */
#define VBSE	0x8000	/* b15: VBUS interrupt */
#define RSME	0x4000	/* b14: Resume interrupt */
#define SOFE	0x2000	/* b13: Frame update interrupt */
#define DVSE	0x1000	/* b12: Device state transition interrupt */
#define CTRE	0x0800	/* b11: Control transfer stage transition interrupt */
#define BEMPE	0x0400	/* b10: Buffer empty interrupt */
#define NRDYE	0x0200	/* b9: Buffer not ready interrupt */
#define BRDYE	0x0100	/* b8: Buffer ready interrupt */

/* Interrupt Enable Register 1 */
#define OVRCRE		0x8000	/* b15: Over-current interrupt */
#define BCHGE		0x4000	/* b14: USB us chenge interrupt */
#define DTCHE		0x1000	/* b12: Detach sense interrupt */
#define ATTCHE		0x0800	/* b11: Attach sense interrupt */
#define EOFERRE		0x0040	/* b6: EOF error interrupt */
#define SIGNE		0x0020	/* b5: SETUP IGNORE interrupt */
#define SACKE		0x0010	/* b4: SETUP ACK interrupt */

/* BRDY Interrupt Enable/Status Register */
#define BRDY9		0x0200	/* b9: PIPE9 */
#define BRDY8		0x0100	/* b8: PIPE8 */
#define BRDY7		0x0080	/* b7: PIPE7 */
#define BRDY6		0x0040	/* b6: PIPE6 */
#define BRDY5		0x0020	/* b5: PIPE5 */
#define BRDY4		0x0010	/* b4: PIPE4 */
#define BRDY3		0x0008	/* b3: PIPE3 */
#define BRDY2		0x0004	/* b2: PIPE2 */
#define BRDY1		0x0002	/* b1: PIPE1 */
#define BRDY0		0x0001	/* b1: PIPE0 */

/* NRDY Interrupt Enable/Status Register */
#define NRDY9		0x0200	/* b9: PIPE9 */
#define NRDY8		0x0100	/* b8: PIPE8 */
#define NRDY7		0x0080	/* b7: PIPE7 */
#define NRDY6		0x0040	/* b6: PIPE6 */
#define NRDY5		0x0020	/* b5: PIPE5 */
#define NRDY4		0x0010	/* b4: PIPE4 */
#define NRDY3		0x0008	/* b3: PIPE3 */
#define NRDY2		0x0004	/* b2: PIPE2 */
#define NRDY1		0x0002	/* b1: PIPE1 */
#define NRDY0		0x0001	/* b1: PIPE0 */

/* BEMP Interrupt Enable/Status Register */
#define BEMP9		0x0200	/* b9: PIPE9 */
#define BEMP8		0x0100	/* b8: PIPE8 */
#define BEMP7		0x0080	/* b7: PIPE7 */
#define BEMP6		0x0040	/* b6: PIPE6 */
#define BEMP5		0x0020	/* b5: PIPE5 */
#define BEMP4		0x0010	/* b4: PIPE4 */
#define BEMP3		0x0008	/* b3: PIPE3 */
#define BEMP2		0x0004	/* b2: PIPE2 */
#define BEMP1		0x0002	/* b1: PIPE1 */
#define BEMP0		0x0001	/* b0: PIPE0 */

/* SOF Pin Configuration Register */
#define TRNENSEL	0x0100	/* b8: Select transaction enable period */
#define BRDYM		0x0040	/* b6: BRDY clear timing */
#define INTL		0x0020	/* b5: Interrupt sense select */
#define EDGESTS		0x0010	/* b4:  */
#define SOFMODE		0x000C	/* b3-2: SOF pin select */
#define SOF_125US	0x0008	/* SOF OUT 125us Frame Signal */
#define SOF_1MS		0x0004	/* SOF OUT 1ms Frame Signal */
#define SOF_DISABLE	0x0000	/* SOF OUT Disable */

/* Interrupt Status Register 0 */
#define VBINT	0x8000	/* b15: VBUS interrupt */
#define RESM	0x4000	/* b14: Resume interrupt */
#define SOFR	0x2000	/* b13: SOF frame update interrupt */
#define DVST	0x1000	/* b12: Device state transition interrupt */
#define CTRT	0x0800	/* b11: Control transfer stage transition interrupt */
#define BEMP	0x0400	/* b10: Buffer empty interrupt */
#define NRDY	0x0200	/* b9: Buffer not ready interrupt */
#define BRDY	0x0100	/* b8: Buffer ready interrupt */
#define VBSTS	0x0080	/* b7: VBUS input port */
#define DVSQ	0x0070	/* b6-4: Device state */
#define DS_SPD_CNFG	0x0070	/* Suspend Configured */
#define DS_SPD_ADDR	0x0060	/* Suspend Address */
#define DS_SPD_DFLT	0x0050	/* Suspend Default */
#define DS_SPD_POWR	0x0040	/* Suspend Powered */
#define DS_SUSP		0x0040	/* Suspend */
#define DS_CNFG		0x0030	/* Configured */
#define DS_ADDS		0x0020	/* Address */
#define DS_DFLT		0x0010	/* Default */
#define DS_POWR		0x0000	/* Powered */
#define DVSQS		0x0030	/* b5-4: Device state */
#define VALID		0x0008	/* b3: Setup packet detected flag */
#define CTSQ		0x0007	/* b2-0: Control transfer stage */
#define CS_SQER		0x0006	/* Sequence error */
#define CS_WRND		0x0005	/* Control write nodata status stage */
#define CS_WRSS		0x0004	/* Control write status stage */
#define CS_WRDS		0x0003	/* Control write data stage */
#define CS_RDSS		0x0002	/* Control read status stage */
#define CS_RDDS		0x0001	/* Control read data stage */
#define CS_IDST		0x0000	/* Idle or setup stage */

/* Interrupt Status Register 1 */
#define OVRCR		0x8000	/* b15: Over-current interrupt */
#define BCHG		0x4000	/* b14: USB bus chenge interrupt */
#define DTCH		0x1000	/* b12: Detach sense interrupt */
#define ATTCH		0x0800	/* b11: Attach sense interrupt */
#define EOFERR		0x0040	/* b6: EOF-error interrupt */
#define SIGN		0x0020	/* b5: Setup ignore interrupt */
#define SACK		0x0010	/* b4: Setup acknowledge interrupt */

/* Frame Number Register */
#define OVRN		0x8000	/* b15: Overrun error */
#define CRCE		0x4000	/* b14: Received data error */
#define FRNM		0x07FF	/* b10-0: Frame number */

/* Micro Frame Number Register */
#define UFRNM		0x0007	/* b2-0: Micro frame number */

/* Default Control Pipe Maxpacket Size Register */
/* Pipe Maxpacket Size Register */
#define DEVSEL	0xF000	/* b15-14: Device address select */
#define MAXP	0x007F	/* b6-0: Maxpacket size of default control pipe */

/* Default Control Pipe Control Register */
#define BSTS		0x8000	/* b15: Buffer status */
#define SUREQ		0x4000	/* b14: Send USB request  */
#define CSCLR		0x2000	/* b13: complete-split status clear */
#define CSSTS		0x1000	/* b12: complete-split status */
#define SUREQCLR	0x0800	/* b11: stop setup request */
#define SQCLR		0x0100	/* b8: Sequence toggle bit clear */
#define SQSET		0x0080	/* b7: Sequence toggle bit set */
#define SQMON		0x0040	/* b6: Sequence toggle bit monitor */
#define PBUSY		0x0020	/* b5: pipe busy */
#define PINGE		0x0010	/* b4: ping enable */
#define CCPL		0x0004	/* b2: Enable control transfer complete */
#define PID		0x0003	/* b1-0: Response PID */
#define PID_STALL11	0x0003	/* STALL */
#define PID_STALL	0x0002	/* STALL */
#define PID_BUF		0x0001	/* BUF */
#define PID_NAK		0x0000	/* NAK */

/* Pipe Window Select Register */
#define PIPENM		0x0007	/* b2-0: Pipe select */

/* Pipe Configuration Register */
#define R8A66597_TYP	0xC000	/* b15-14: Transfer type */
#define R8A66597_ISO	0xC000	/* Isochronous */
#define R8A66597_INT	0x8000	/* Interrupt */
#define R8A66597_BULK	0x4000	/* Bulk */
#define R8A66597_BFRE	0x0400	/* b10: Buffer ready interrupt mode select */
#define R8A66597_DBLB	0x0200	/* b9: Double buffer mode select */
#define R8A66597_CNTMD	0x0100	/* b8: Continuous transfer mode select */
#define R8A66597_SHTNAK	0x0080	/* b7: Transfer end NAK */
#define R8A66597_DIR	0x0010	/* b4: Transfer direction select */
#define R8A66597_EPNUM	0x000F	/* b3-0: Eendpoint number select */

/* Pipe Buffer Configuration Register */
#define BUFSIZE		0x7C00	/* b14-10: Pipe buffer size */
#define BUFNMB		0x007F	/* b6-0: Pipe buffer number */
#define PIPE0BUF	256
#define PIPExBUF	64

/* Pipe Maxpacket Size Register */
#define MXPS	0x07FF	/* b10-0: Maxpacket size */

/* Pipe Cycle Configuration Register */
#define IFIS	0x1000	/* b12: Isochronous in-buffer flush mode select */
#define IITV	0x0007	/* b2-0: Isochronous interval */

/* Pipex Control Register */
#define BSTS	0x8000	/* b15: Buffer status */
#define INBUFM	0x4000	/* b14: IN buffer monitor (Only for PIPE1 to 5) */
#define CSCLR	0x2000	/* b13: complete-split status clear */
#define CSSTS	0x1000	/* b12: complete-split status */
#define ATREPM	0x0400	/* b10: Auto repeat mode */
#define ACLRM	0x0200	/* b9: Out buffer auto clear mode */
#define SQCLR	0x0100	/* b8: Sequence toggle bit clear */
#define SQSET	0x0080	/* b7: Sequence toggle bit set */
#define SQMON	0x0040	/* b6: Sequence toggle bit monitor */
#define PBUSY	0x0020	/* b5: pipe busy */
#define PID	0x0003	/* b1-0: Response PID */

/* PIPExTRE */
#define TRENB		0x0200	/* b9: Transaction counter enable */
#define TRCLR		0x0100	/* b8: Transaction counter clear */

/* PIPExTRN */
#define TRNCNT		0xFFFF	/* b15-0: Transaction counter */

/* DEVADDx */
#define UPPHUB		0x7800
#define HUBPORT		0x0700
#define USBSPD		0x00C0
#define RTPORT		0x0001

/* Suspend Mode Register */
#define SUSPM		0x4000	/* b14: Suspend */

#define R8A66597_MAX_NUM_PIPE		10
#define R8A66597_BUF_BSIZE		8
#define R8A66597_MAX_DEVICE		10
#define R8A66597_MAX_ROOT_HUB		2
#define R8A66597_MAX_SAMPLING		5
#define R8A66597_RH_POLL_TIME		10

#define BULK_IN_PIPENUM		3
#define BULK_IN_BUFNUM		8

#define BULK_OUT_PIPENUM	4
#define BULK_OUT_BUFNUM		40

#define make_devsel(addr)		((addr) << 12)

struct r8a66597 {
	unsigned long reg;
	unsigned short pipe_config;	/* bit field */
	unsigned short port_status;
	unsigned short port_change;
	u16 speed;	/* HSMODE or FSMODE or LSMODE */
	unsigned char rh_devnum;
	struct udevice *vbus_supply;
};

static inline u16 r8a66597_read(struct r8a66597 *r8a66597, unsigned long offset)
{
	return readw(r8a66597->reg + offset);
}

static inline void r8a66597_read_fifo(struct r8a66597 *r8a66597,
				      unsigned long offset, void *buf,
				      int len)
{
	int i;
	unsigned long fifoaddr = r8a66597->reg + offset;
	unsigned long count;
	unsigned long *p = buf;

	count = len / 4;
	for (i = 0; i < count; i++)
		p[i] = readl(r8a66597->reg + offset);

	if (len & 0x00000003) {
		unsigned long tmp = readl(fifoaddr);

		memcpy((unsigned char *)buf + count * 4, &tmp, len & 0x03);
	}
}

static inline void r8a66597_write(struct r8a66597 *r8a66597, u16 val,
				  unsigned long offset)
{
	writew(val, r8a66597->reg + offset);
}

static inline void r8a66597_write_fifo(struct r8a66597 *r8a66597,
				       unsigned long offset, void *buf,
				       int len)
{
	int i;
	unsigned long fifoaddr = r8a66597->reg + offset;
	unsigned long count;
	unsigned char *pb;
	unsigned long *p = buf;

	count = len / 4;
	for (i = 0; i < count; i++)
		writel(p[i], fifoaddr);

	if (len & 0x00000003) {
		pb = (unsigned char *)buf + count * 4;
		for (i = 0; i < (len & 0x00000003); i++) {
			if (r8a66597_read(r8a66597, CFIFOSEL) & BIGEND)
				writeb(pb[i], fifoaddr + i);
			else
				writeb(pb[i], fifoaddr + 3 - i);
		}
	}
}

static inline void r8a66597_mdfy(struct r8a66597 *r8a66597,
				 u16 val, u16 pat, unsigned long offset)
{
	u16 tmp;

	tmp = r8a66597_read(r8a66597, offset);
	tmp = tmp & (~pat);
	tmp = tmp | val;
	r8a66597_write(r8a66597, tmp, offset);
}

#define r8a66597_bclr(r8a66597, val, offset)	\
			r8a66597_mdfy(r8a66597, 0, val, offset)
#define r8a66597_bset(r8a66597, val, offset)	\
			r8a66597_mdfy(r8a66597, val, 0, offset)

static inline unsigned long get_syscfg_reg(int port)
{
	return port == 0 ? SYSCFG0 : SYSCFG1;
}

static inline unsigned long get_syssts_reg(int port)
{
	return port == 0 ? SYSSTS0 : SYSSTS1;
}

static inline unsigned long get_dvstctr_reg(int port)
{
	return port == 0 ? DVSTCTR0 : DVSTCTR1;
}

static inline unsigned long get_dmacfg_reg(int port)
{
	return port == 0 ? DMA0CFG : DMA1CFG;
}

static inline unsigned long get_intenb_reg(int port)
{
	return port == 0 ? INTENB1 : INTENB2;
}

static inline unsigned long get_intsts_reg(int port)
{
	return port == 0 ? INTSTS1 : INTSTS2;
}

static inline u16 get_rh_usb_speed(struct r8a66597 *r8a66597, int port)
{
	unsigned long dvstctr_reg = get_dvstctr_reg(port);

	return r8a66597_read(r8a66597, dvstctr_reg) & RHST;
}

static inline void r8a66597_port_power(struct r8a66597 *r8a66597, int port,
				       int power)
{
	unsigned long dvstctr_reg = get_dvstctr_reg(port);

	if (power)
		r8a66597_bset(r8a66597, VBOUT, dvstctr_reg);
	else
		r8a66597_bclr(r8a66597, VBOUT, dvstctr_reg);
}

#define get_pipectr_addr(pipenum)	(PIPE1CTR + (pipenum - 1) * 2)
#define get_pipetre_addr(pipenum)	(PIPE1TRE + (pipenum - 1) * 4)
#define get_pipetrn_addr(pipenum)	(PIPE1TRN + (pipenum - 1) * 4)
#define get_devadd_addr(address)	(DEVADD0 + address * 2)

/* USB HUB CONSTANTS (not OHCI-specific; see hub.h, based on usb_ohci.h) */

/* destination of request */
#define RH_INTERFACE		   0x01
#define RH_ENDPOINT		   0x02
#define RH_OTHER		   0x03

#define RH_CLASS		   0x20
#define RH_VENDOR		   0x40

/* Requests: bRequest << 8 | bmRequestType */
#define RH_GET_STATUS		0x0080
#define RH_CLEAR_FEATURE	0x0100
#define RH_SET_FEATURE		0x0300
#define RH_SET_ADDRESS		0x0500
#define RH_GET_DESCRIPTOR	0x0680
#define RH_SET_DESCRIPTOR	0x0700
#define RH_GET_CONFIGURATION	0x0880
#define RH_SET_CONFIGURATION	0x0900
#define RH_GET_STATE		0x0280
#define RH_GET_INTERFACE	0x0A80
#define RH_SET_INTERFACE	0x0B00
#define RH_SYNC_FRAME		0x0C80
/* Our Vendor Specific Request */
#define RH_SET_EP		0x2000

/* Hub port features */
#define RH_PORT_CONNECTION	   0x00
#define RH_PORT_ENABLE		   0x01
#define RH_PORT_SUSPEND		   0x02
#define RH_PORT_OVER_CURRENT	   0x03
#define RH_PORT_RESET		   0x04
#define RH_PORT_POWER		   0x08
#define RH_PORT_LOW_SPEED	   0x09

#define RH_C_PORT_CONNECTION	   0x10
#define RH_C_PORT_ENABLE	   0x11
#define RH_C_PORT_SUSPEND	   0x12
#define RH_C_PORT_OVER_CURRENT	   0x13
#define RH_C_PORT_RESET		   0x14

/* Hub features */
#define RH_C_HUB_LOCAL_POWER	   0x00
#define RH_C_HUB_OVER_CURRENT	   0x01

#define RH_DEVICE_REMOTE_WAKEUP	   0x00
#define RH_ENDPOINT_STALL	   0x01

#define RH_ACK			   0x01
#define RH_REQ_ERR		   -1
#define RH_NACK			   0x00

/* OHCI ROOT HUB REGISTER MASKS */

/* roothub.portstatus [i] bits */
#define RH_PS_CCS	0x00000001	/* current connect status */
#define RH_PS_PES	0x00000002	/* port enable status*/
#define RH_PS_PSS	0x00000004	/* port suspend status */
#define RH_PS_POCI	0x00000008	/* port over current indicator */
#define RH_PS_PRS	0x00000010	/* port reset status */
#define RH_PS_PPS	0x00000100	/* port power status */
#define RH_PS_LSDA	0x00000200	/* low speed device attached */
#define RH_PS_CSC	0x00010000	/* connect status change */
#define RH_PS_PESC	0x00020000	/* port enable status change */
#define RH_PS_PSSC	0x00040000	/* port suspend status change */
#define RH_PS_OCIC	0x00080000	/* over current indicator change */
#define RH_PS_PRSC	0x00100000	/* port reset status change */

/* roothub.status bits */
#define RH_HS_LPS	0x00000001	/* local power status */
#define RH_HS_OCI	0x00000002	/* over current indicator */
#define RH_HS_DRWE	0x00008000	/* device remote wakeup enable */
#define RH_HS_LPSC	0x00010000	/* local power status change */
#define RH_HS_OCIC	0x00020000	/* over current indicator change */
#define RH_HS_CRWE	0x80000000	/* clear remote wakeup enable */

/* roothub.b masks */
#define RH_B_DR		0x0000ffff	/* device removable flags */
#define RH_B_PPCM	0xffff0000	/* port power control mask */

/* roothub.a masks */
#define RH_A_NDP	(0xff << 0)	/* number of downstream ports */
#define RH_A_PSM	BIT(8)	/* power switching mode */
#define RH_A_NPS	BIT(9)	/* no power switching */
#define RH_A_DT		BIT(10)	/* device type (mbz) */
#define RH_A_OCPM	BIT(11)	/* over current protection mode */
#define RH_A_NOCP	BIT(12)	/* no over current protection */
#define RH_A_POTPGT	(0xff << 24)	/* power on to power good time */

#endif	/* __R8A66597_H__ */
