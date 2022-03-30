/*
 *  linux/include/asm-arm/arch-pxa/pxa-regs.h
 *
 *  Author:	Nicolas Pitre
 *  Created:	Jun 15, 2001
 *  Copyright:	MontaVista Software Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * - 2003/01/20: Robert Schwebel <r.schwebel@pengutronix.de
 *   Original file taken from linux-2.4.19-rmk4-pxa1. Added some definitions.
 *   Added include for hardware.h (for __REG definition)
 */
#ifndef _PXA_REGS_H_
#define _PXA_REGS_H_

#include "bitfield.h"
#include "hardware.h"

/* FIXME hack so that SA-1111.h will work [cb] */

#ifndef __ASSEMBLY__
typedef unsigned short	Word16 ;
typedef unsigned int	Word32 ;
typedef Word32		Word ;
typedef Word		Quad [4] ;
typedef void		*Address ;
typedef void		(*ExcpHndlr) (void) ;
#endif

/*
 * PXA Chip selects
 */
#ifdef CONFIG_CPU_MONAHANS
#define PXA_CS0_PHYS   0x00000000 /* for both small and large same start */
#define PXA_CS1_PHYS   0x04000000 /* Small partition start address (64MB) */
#define PXA_CS1_LPHYS  0x30000000 /* Large partition start address (256MB) */
#define PXA_CS2_PHYS   0x10000000 /* (64MB) */
#define PXA_CS3_PHYS   0x14000000 /* (64MB) */
#define PXA_PCMCIA_PHYS        0x20000000 /* (256MB) */
#else
#define PXA_CS0_PHYS	0x00000000
#define PXA_CS1_PHYS	0x04000000
#define PXA_CS2_PHYS	0x08000000
#define PXA_CS3_PHYS	0x0C000000
#define PXA_CS4_PHYS	0x10000000
#define PXA_CS5_PHYS	0x14000000
#endif /* CONFIG_CPU_MONAHANS */

/*
 * Personal Computer Memory Card International Association (PCMCIA) sockets
 */
#define PCMCIAPrtSp	0x04000000	/* PCMCIA Partition Space [byte]   */
#define PCMCIASp	(4*PCMCIAPrtSp) /* PCMCIA Space [byte]		   */
#define PCMCIAIOSp	PCMCIAPrtSp	/* PCMCIA I/O Space [byte]	   */
#define PCMCIAAttrSp	PCMCIAPrtSp	/* PCMCIA Attribute Space [byte]   */
#define PCMCIAMemSp	PCMCIAPrtSp	/* PCMCIA Memory Space [byte]	   */

#ifndef CONFIG_CPU_MONAHANS             /* Monahans supports only one slot */
#define PCMCIA0Sp	PCMCIASp	/* PCMCIA 0 Space [byte]	   */
#define PCMCIA0IOSp	PCMCIAIOSp	/* PCMCIA 0 I/O Space [byte]	   */
#define PCMCIA0AttrSp	PCMCIAAttrSp	/* PCMCIA 0 Attribute Space [byte] */
#define PCMCIA0MemSp	PCMCIAMemSp	/* PCMCIA 0 Memory Space [byte]	   */
#endif

#define PCMCIA1Sp	PCMCIASp	/* PCMCIA 1 Space [byte]	   */
#define PCMCIA1IOSp	PCMCIAIOSp	/* PCMCIA 1 I/O Space [byte]	   */
#define PCMCIA1AttrSp	PCMCIAAttrSp	/* PCMCIA 1 Attribute Space [byte] */
#define PCMCIA1MemSp	PCMCIAMemSp	/* PCMCIA 1 Memory Space [byte]	   */

#define _PCMCIA(Nb)			/* PCMCIA [0..1]		   */ \
			(0x20000000 + (Nb)*PCMCIASp)
#define _PCMCIAIO(Nb)	_PCMCIA (Nb)	/* PCMCIA I/O [0..1]		   */
#define _PCMCIAAttr(Nb)			/* PCMCIA Attribute [0..1]	   */ \
			(_PCMCIA (Nb) + 2*PCMCIAPrtSp)
#define _PCMCIAMem(Nb)			/* PCMCIA Memory [0..1]		   */ \
			(_PCMCIA (Nb) + 3*PCMCIAPrtSp)

#define _PCMCIA0	_PCMCIA (0)	/* PCMCIA 0			   */
#define _PCMCIA0IO	_PCMCIAIO (0)	/* PCMCIA 0 I/O			   */
#define _PCMCIA0Attr	_PCMCIAAttr (0) /* PCMCIA 0 Attribute		   */
#define _PCMCIA0Mem	_PCMCIAMem (0)	/* PCMCIA 0 Memory		   */

#ifndef CONFIG_CPU_MONAHANS             /* Monahans supports only one slot */
#define _PCMCIA1	_PCMCIA (1)	/* PCMCIA 1			   */
#define _PCMCIA1IO	_PCMCIAIO (1)	/* PCMCIA 1 I/O			   */
#define _PCMCIA1Attr	_PCMCIAAttr (1) /* PCMCIA 1 Attribute		   */
#define _PCMCIA1Mem	_PCMCIAMem (1)	/* PCMCIA 1 Memory		   */
#endif

/*
 * DMA Controller
 */
#define DCSR0		0x40000000  /* DMA Control / Status Register for Channel 0 */
#define DCSR1		0x40000004  /* DMA Control / Status Register for Channel 1 */
#define DCSR2		0x40000008  /* DMA Control / Status Register for Channel 2 */
#define DCSR3		0x4000000c  /* DMA Control / Status Register for Channel 3 */
#define DCSR4		0x40000010  /* DMA Control / Status Register for Channel 4 */
#define DCSR5		0x40000014  /* DMA Control / Status Register for Channel 5 */
#define DCSR6		0x40000018  /* DMA Control / Status Register for Channel 6 */
#define DCSR7		0x4000001c  /* DMA Control / Status Register for Channel 7 */
#define DCSR8		0x40000020  /* DMA Control / Status Register for Channel 8 */
#define DCSR9		0x40000024  /* DMA Control / Status Register for Channel 9 */
#define DCSR10		0x40000028  /* DMA Control / Status Register for Channel 10 */
#define DCSR11		0x4000002c  /* DMA Control / Status Register for Channel 11 */
#define DCSR12		0x40000030  /* DMA Control / Status Register for Channel 12 */
#define DCSR13		0x40000034  /* DMA Control / Status Register for Channel 13 */
#define DCSR14		0x40000038  /* DMA Control / Status Register for Channel 14 */
#define DCSR15		0x4000003c  /* DMA Control / Status Register for Channel 15 */
#if defined(CONFIG_CPU_PXA27X) || defined(CONFIG_CPU_MONAHANS)
#define DCSR16		0x40000040  /* DMA Control / Status Register for Channel 16 */
#define DCSR17		0x40000044  /* DMA Control / Status Register for Channel 17 */
#define DCSR18		0x40000048  /* DMA Control / Status Register for Channel 18 */
#define DCSR19		0x4000004c  /* DMA Control / Status Register for Channel 19 */
#define DCSR20		0x40000050  /* DMA Control / Status Register for Channel 20 */
#define DCSR21		0x40000054  /* DMA Control / Status Register for Channel 21 */
#define DCSR22		0x40000058  /* DMA Control / Status Register for Channel 22 */
#define DCSR23		0x4000005c  /* DMA Control / Status Register for Channel 23 */
#define DCSR24		0x40000060  /* DMA Control / Status Register for Channel 24 */
#define DCSR25		0x40000064  /* DMA Control / Status Register for Channel 25 */
#define DCSR26		0x40000068  /* DMA Control / Status Register for Channel 26 */
#define DCSR27		0x4000006c  /* DMA Control / Status Register for Channel 27 */
#define DCSR28		0x40000070  /* DMA Control / Status Register for Channel 28 */
#define DCSR29		0x40000074  /* DMA Control / Status Register for Channel 29 */
#define DCSR30		0x40000078  /* DMA Control / Status Register for Channel 30 */
#define DCSR31		0x4000007c  /* DMA Control / Status Register for Channel 31 */
#endif /* CONFIG_CPU_PXA27X || CONFIG_CPU_MONAHANS */

#define DCSR(x)		(0x40000000 | ((x) << 2))

#define DCSR_RUN	(1 << 31)	/* Run Bit (read / write) */
#define DCSR_NODESC	(1 << 30)	/* No-Descriptor Fetch (read / write) */
#define DCSR_STOPIRQEN	(1 << 29)	/* Stop Interrupt Enable (read / write) */

#if defined(CONFIG_CPU_PXA27X) || defined(CONFIG_CPU_MONAHANS)
#define DCSR_EORIRQEN	(1 << 28)	/* End of Receive Interrupt Enable (R/W) */
#define DCSR_EORJMPEN	(1 << 27)	/* Jump to next descriptor on EOR */
#define DCSR_EORSTOPEN	(1 << 26)	/* STOP on an EOR */
#define DCSR_SETCMPST	(1 << 25)	/* Set Descriptor Compare Status */
#define DCSR_CLRCMPST	(1 << 24)	/* Clear Descriptor Compare Status */
#define DCSR_CMPST	(1 << 10)	/* The Descriptor Compare Status */
#define DCSR_ENRINTR	(1 << 9)	/* The end of Receive */
#endif

#define DCSR_REQPEND	(1 << 8)	/* Request Pending (read-only) */
#define DCSR_STOPSTATE	(1 << 3)	/* Stop State (read-only) */
#define DCSR_ENDINTR	(1 << 2)	/* End Interrupt (read / write) */
#define DCSR_STARTINTR	(1 << 1)	/* Start Interrupt (read / write) */
#define DCSR_BUSERR	(1 << 0)	/* Bus Error Interrupt (read / write) */

#define DINT		0x400000f0  /* DMA Interrupt Register */

#define DRCMR0		0x40000100  /* Request to Channel Map Register for DREQ 0 */
#define DRCMR1		0x40000104  /* Request to Channel Map Register for DREQ 1 */
#define DRCMR2		0x40000108  /* Request to Channel Map Register for I2S receive Request */
#define DRCMR3		0x4000010c  /* Request to Channel Map Register for I2S transmit Request */
#define DRCMR4		0x40000110  /* Request to Channel Map Register for BTUART receive Request */
#define DRCMR5		0x40000114  /* Request to Channel Map Register for BTUART transmit Request. */
#define DRCMR6		0x40000118  /* Request to Channel Map Register for FFUART receive Request */
#define DRCMR7		0x4000011c  /* Request to Channel Map Register for FFUART transmit Request */
#define DRCMR8		0x40000120  /* Request to Channel Map Register for AC97 microphone Request */
#define DRCMR9		0x40000124  /* Request to Channel Map Register for AC97 modem receive Request */
#define DRCMR10		0x40000128  /* Request to Channel Map Register for AC97 modem transmit Request */
#define DRCMR11		0x4000012c  /* Request to Channel Map Register for AC97 audio receive Request */
#define DRCMR12		0x40000130  /* Request to Channel Map Register for AC97 audio transmit Request */
#define DRCMR13		0x40000134  /* Request to Channel Map Register for SSP receive Request */
#define DRCMR14		0x40000138  /* Request to Channel Map Register for SSP transmit Request */
#define DRCMR15		0x4000013c  /* Reserved */
#define DRCMR16		0x40000140  /* Reserved */
#define DRCMR17		0x40000144  /* Request to Channel Map Register for ICP receive Request */
#define DRCMR18		0x40000148  /* Request to Channel Map Register for ICP transmit Request */
#define DRCMR19		0x4000014c  /* Request to Channel Map Register for STUART receive Request */
#define DRCMR20		0x40000150  /* Request to Channel Map Register for STUART transmit Request */
#define DRCMR21		0x40000154  /* Request to Channel Map Register for MMC receive Request */
#define DRCMR22		0x40000158  /* Request to Channel Map Register for MMC transmit Request */
#define DRCMR23		0x4000015c  /* Reserved */
#define DRCMR24		0x40000160  /* Reserved */
#define DRCMR25		0x40000164  /* Request to Channel Map Register for USB endpoint 1 Request */
#define DRCMR26		0x40000168  /* Request to Channel Map Register for USB endpoint 2 Request */
#define DRCMR27		0x4000016C  /* Request to Channel Map Register for USB endpoint 3 Request */
#define DRCMR28		0x40000170  /* Request to Channel Map Register for USB endpoint 4 Request */
#define DRCMR29		0x40000174  /* Reserved */
#define DRCMR30		0x40000178  /* Request to Channel Map Register for USB endpoint 6 Request */
#define DRCMR31		0x4000017C  /* Request to Channel Map Register for USB endpoint 7 Request */
#define DRCMR32		0x40000180  /* Request to Channel Map Register for USB endpoint 8 Request */
#define DRCMR33		0x40000184  /* Request to Channel Map Register for USB endpoint 9 Request */
#define DRCMR34		0x40000188  /* Reserved */
#define DRCMR35		0x4000018C  /* Request to Channel Map Register for USB endpoint 11 Request */
#define DRCMR36		0x40000190  /* Request to Channel Map Register for USB endpoint 12 Request */
#define DRCMR37		0x40000194  /* Request to Channel Map Register for USB endpoint 13 Request */
#define DRCMR38		0x40000198  /* Request to Channel Map Register for USB endpoint 14 Request */
#define DRCMR39		0x4000019C  /* Reserved */

#define DRCMR68		       0x40001110  /* Request to Channel Map Register for Camera FIFO 0 Request */
#define DRCMR69		       0x40001114  /* Request to Channel Map Register for Camera FIFO 1 Request */
#define DRCMR70		       0x40001118  /* Request to Channel Map Register for Camera FIFO 2 Request */

#define DRCMRRXSADR	DRCMR2
#define DRCMRTXSADR	DRCMR3
#define DRCMRRXBTRBR	DRCMR4
#define DRCMRTXBTTHR	DRCMR5
#define DRCMRRXFFRBR	DRCMR6
#define DRCMRTXFFTHR	DRCMR7
#define DRCMRRXMCDR	DRCMR8
#define DRCMRRXMODR	DRCMR9
#define DRCMRTXMODR	DRCMR10
#define DRCMRRXPCDR	DRCMR11
#define DRCMRTXPCDR	DRCMR12
#define DRCMRRXSSDR	DRCMR13
#define DRCMRTXSSDR	DRCMR14
#define DRCMRRXICDR	DRCMR17
#define DRCMRTXICDR	DRCMR18
#define DRCMRRXSTRBR	DRCMR19
#define DRCMRTXSTTHR	DRCMR20
#define DRCMRRXMMC	DRCMR21
#define DRCMRTXMMC	DRCMR22

#define DRCMR_MAPVLD	(1 << 7)	/* Map Valid (read / write) */
#define DRCMR_CHLNUM	0x0f		/* mask for Channel Number (read / write) */

#define DDADR0		0x40000200  /* DMA Descriptor Address Register Channel 0 */
#define DSADR0		0x40000204  /* DMA Source Address Register Channel 0 */
#define DTADR0		0x40000208  /* DMA Target Address Register Channel 0 */
#define DCMD0		0x4000020c  /* DMA Command Address Register Channel 0 */
#define DDADR1		0x40000210  /* DMA Descriptor Address Register Channel 1 */
#define DSADR1		0x40000214  /* DMA Source Address Register Channel 1 */
#define DTADR1		0x40000218  /* DMA Target Address Register Channel 1 */
#define DCMD1		0x4000021c  /* DMA Command Address Register Channel 1 */
#define DDADR2		0x40000220  /* DMA Descriptor Address Register Channel 2 */
#define DSADR2		0x40000224  /* DMA Source Address Register Channel 2 */
#define DTADR2		0x40000228  /* DMA Target Address Register Channel 2 */
#define DCMD2		0x4000022c  /* DMA Command Address Register Channel 2 */
#define DDADR3		0x40000230  /* DMA Descriptor Address Register Channel 3 */
#define DSADR3		0x40000234  /* DMA Source Address Register Channel 3 */
#define DTADR3		0x40000238  /* DMA Target Address Register Channel 3 */
#define DCMD3		0x4000023c  /* DMA Command Address Register Channel 3 */
#define DDADR4		0x40000240  /* DMA Descriptor Address Register Channel 4 */
#define DSADR4		0x40000244  /* DMA Source Address Register Channel 4 */
#define DTADR4		0x40000248  /* DMA Target Address Register Channel 4 */
#define DCMD4		0x4000024c  /* DMA Command Address Register Channel 4 */
#define DDADR5		0x40000250  /* DMA Descriptor Address Register Channel 5 */
#define DSADR5		0x40000254  /* DMA Source Address Register Channel 5 */
#define DTADR5		0x40000258  /* DMA Target Address Register Channel 5 */
#define DCMD5		0x4000025c  /* DMA Command Address Register Channel 5 */
#define DDADR6		0x40000260  /* DMA Descriptor Address Register Channel 6 */
#define DSADR6		0x40000264  /* DMA Source Address Register Channel 6 */
#define DTADR6		0x40000268  /* DMA Target Address Register Channel 6 */
#define DCMD6		0x4000026c  /* DMA Command Address Register Channel 6 */
#define DDADR7		0x40000270  /* DMA Descriptor Address Register Channel 7 */
#define DSADR7		0x40000274  /* DMA Source Address Register Channel 7 */
#define DTADR7		0x40000278  /* DMA Target Address Register Channel 7 */
#define DCMD7		0x4000027c  /* DMA Command Address Register Channel 7 */
#define DDADR8		0x40000280  /* DMA Descriptor Address Register Channel 8 */
#define DSADR8		0x40000284  /* DMA Source Address Register Channel 8 */
#define DTADR8		0x40000288  /* DMA Target Address Register Channel 8 */
#define DCMD8		0x4000028c  /* DMA Command Address Register Channel 8 */
#define DDADR9		0x40000290  /* DMA Descriptor Address Register Channel 9 */
#define DSADR9		0x40000294  /* DMA Source Address Register Channel 9 */
#define DTADR9		0x40000298  /* DMA Target Address Register Channel 9 */
#define DCMD9		0x4000029c  /* DMA Command Address Register Channel 9 */
#define DDADR10		0x400002a0  /* DMA Descriptor Address Register Channel 10 */
#define DSADR10		0x400002a4  /* DMA Source Address Register Channel 10 */
#define DTADR10		0x400002a8  /* DMA Target Address Register Channel 10 */
#define DCMD10		0x400002ac  /* DMA Command Address Register Channel 10 */
#define DDADR11		0x400002b0  /* DMA Descriptor Address Register Channel 11 */
#define DSADR11		0x400002b4  /* DMA Source Address Register Channel 11 */
#define DTADR11		0x400002b8  /* DMA Target Address Register Channel 11 */
#define DCMD11		0x400002bc  /* DMA Command Address Register Channel 11 */
#define DDADR12		0x400002c0  /* DMA Descriptor Address Register Channel 12 */
#define DSADR12		0x400002c4  /* DMA Source Address Register Channel 12 */
#define DTADR12		0x400002c8  /* DMA Target Address Register Channel 12 */
#define DCMD12		0x400002cc  /* DMA Command Address Register Channel 12 */
#define DDADR13		0x400002d0  /* DMA Descriptor Address Register Channel 13 */
#define DSADR13		0x400002d4  /* DMA Source Address Register Channel 13 */
#define DTADR13		0x400002d8  /* DMA Target Address Register Channel 13 */
#define DCMD13		0x400002dc  /* DMA Command Address Register Channel 13 */
#define DDADR14		0x400002e0  /* DMA Descriptor Address Register Channel 14 */
#define DSADR14		0x400002e4  /* DMA Source Address Register Channel 14 */
#define DTADR14		0x400002e8  /* DMA Target Address Register Channel 14 */
#define DCMD14		0x400002ec  /* DMA Command Address Register Channel 14 */
#define DDADR15		0x400002f0  /* DMA Descriptor Address Register Channel 15 */
#define DSADR15		0x400002f4  /* DMA Source Address Register Channel 15 */
#define DTADR15		0x400002f8  /* DMA Target Address Register Channel 15 */
#define DCMD15		0x400002fc  /* DMA Command Address Register Channel 15 */

#define DDADR(x)	(0x40000200 | ((x) << 4))
#define DSADR(x)	(0x40000204 | ((x) << 4))
#define DTADR(x)	(0x40000208 | ((x) << 4))
#define DCMD(x)		(0x4000020c | ((x) << 4))

#define DDADR_DESCADDR	0xfffffff0	/* Address of next descriptor (mask) */
#define DDADR_STOP	(1 << 0)	/* Stop (read / write) */

#define DCMD_INCSRCADDR (1 << 31)	/* Source Address Increment Setting. */
#define DCMD_INCTRGADDR (1 << 30)	/* Target Address Increment Setting. */
#define DCMD_FLOWSRC	(1 << 29)	/* Flow Control by the source. */
#define DCMD_FLOWTRG	(1 << 28)	/* Flow Control by the target. */
#define DCMD_STARTIRQEN (1 << 22)	/* Start Interrupt Enable */
#define DCMD_ENDIRQEN	(1 << 21)	/* End Interrupt Enable */
#define DCMD_ENDIAN	(1 << 18)	/* Device Endian-ness. */
#define DCMD_BURST8	(1 << 16)	/* 8 byte burst */
#define DCMD_BURST16	(2 << 16)	/* 16 byte burst */
#define DCMD_BURST32	(3 << 16)	/* 32 byte burst */
#define DCMD_WIDTH1	(1 << 14)	/* 1 byte width */
#define DCMD_WIDTH2	(2 << 14)	/* 2 byte width (HalfWord) */
#define DCMD_WIDTH4	(3 << 14)	/* 4 byte width (Word) */
#define DCMD_LENGTH	0x01fff		/* length mask (max = 8K - 1) */

/* default combinations */
#define DCMD_RXPCDR	(DCMD_INCTRGADDR|DCMD_FLOWSRC|DCMD_BURST32|DCMD_WIDTH4)
#define DCMD_RXMCDR	(DCMD_INCTRGADDR|DCMD_FLOWSRC|DCMD_BURST32|DCMD_WIDTH4)
#define DCMD_TXPCDR	(DCMD_INCSRCADDR|DCMD_FLOWTRG|DCMD_BURST32|DCMD_WIDTH4)

/******************************************************************************/
/*
 * IrSR (Infrared Selection Register)
 */
#define IrSR_OFFSET 0x20

#define IrSR_RXPL_NEG_IS_ZERO (1<<4)
#define IrSR_RXPL_POS_IS_ZERO 0x0
#define IrSR_TXPL_NEG_IS_ZERO (1<<3)
#define IrSR_TXPL_POS_IS_ZERO 0x0
#define IrSR_XMODE_PULSE_1_6  (1<<2)
#define IrSR_XMODE_PULSE_3_16 0x0
#define IrSR_RCVEIR_IR_MODE   (1<<1)
#define IrSR_RCVEIR_UART_MODE 0x0
#define IrSR_XMITIR_IR_MODE   (1<<0)
#define IrSR_XMITIR_UART_MODE 0x0

#define IrSR_IR_RECEIVE_ON (\
		IrSR_RXPL_NEG_IS_ZERO | \
		IrSR_TXPL_POS_IS_ZERO | \
		IrSR_XMODE_PULSE_3_16 | \
		IrSR_RCVEIR_IR_MODE   | \
		IrSR_XMITIR_UART_MODE)

#define IrSR_IR_TRANSMIT_ON (\
		IrSR_RXPL_NEG_IS_ZERO | \
		IrSR_TXPL_POS_IS_ZERO | \
		IrSR_XMODE_PULSE_3_16 | \
		IrSR_RCVEIR_UART_MODE | \
		IrSR_XMITIR_IR_MODE)

/*
 * Serial Audio Controller
 */
/* FIXME the audio defines collide w/ the SA1111 defines.  I don't like these
 * short defines because there is too much chance of namespace collision
 */
#define SACR0		0x40400000  /*  Global Control Register */
#define SACR1		0x40400004  /*  Serial Audio I 2 S/MSB-Justified Control Register */
#define SASR0		0x4040000C  /*  Serial Audio I 2 S/MSB-Justified Interface and FIFO Status Register */
#define SAIMR		0x40400014  /*  Serial Audio Interrupt Mask Register */
#define SAICR		0x40400018  /*  Serial Audio Interrupt Clear Register */
#define SADIV		0x40400060  /*  Audio Clock Divider Register. */
#define SADR		0x40400080  /*  Serial Audio Data Register (TX and RX FIFO access Register). */

/*
 * AC97 Controller registers
 */
#define POCR		0x40500000  /* PCM Out Control Register */
#define POCR_FEIE	(1 << 3)	/* FIFO Error Interrupt Enable */

#define PICR		0x40500004  /* PCM In Control Register */
#define PICR_FEIE	(1 << 3)	/* FIFO Error Interrupt Enable */

#define MCCR		0x40500008  /* Mic In Control Register */
#define MCCR_FEIE	(1 << 3)	/* FIFO Error Interrupt Enable */

#define GCR		0x4050000C  /* Global Control Register */
#define GCR_CDONE_IE	(1 << 19)	/* Command Done Interrupt Enable */
#define GCR_SDONE_IE	(1 << 18)	/* Status Done Interrupt Enable */
#define GCR_SECRDY_IEN	(1 << 9)	/* Secondary Ready Interrupt Enable */
#define GCR_PRIRDY_IEN	(1 << 8)	/* Primary Ready Interrupt Enable */
#define GCR_SECRES_IEN	(1 << 5)	/* Secondary Resume Interrupt Enable */
#define GCR_PRIRES_IEN	(1 << 4)	/* Primary Resume Interrupt Enable */
#define GCR_ACLINK_OFF	(1 << 3)	/* AC-link Shut Off */
#define GCR_WARM_RST	(1 << 2)	/* AC97 Warm Reset */
#define GCR_COLD_RST	(1 << 1)	/* AC'97 Cold Reset (0 = active) */
#define GCR_GIE		(1 << 0)	/* Codec GPI Interrupt Enable */

#define POSR		0x40500010  /* PCM Out Status Register */
#define POSR_FIFOE	(1 << 4)	/* FIFO error */

#define PISR		0x40500014  /* PCM In Status Register */
#define PISR_FIFOE	(1 << 4)	/* FIFO error */

#define MCSR		0x40500018  /* Mic In Status Register */
#define MCSR_FIFOE	(1 << 4)	/* FIFO error */

#define GSR		0x4050001C  /* Global Status Register */
#define GSR_CDONE	(1 << 19)	/* Command Done */
#define GSR_SDONE	(1 << 18)	/* Status Done */
#define GSR_RDCS	(1 << 15)	/* Read Completion Status */
#define GSR_BIT3SLT12	(1 << 14)	/* Bit 3 of slot 12 */
#define GSR_BIT2SLT12	(1 << 13)	/* Bit 2 of slot 12 */
#define GSR_BIT1SLT12	(1 << 12)	/* Bit 1 of slot 12 */
#define GSR_SECRES	(1 << 11)	/* Secondary Resume Interrupt */
#define GSR_PRIRES	(1 << 10)	/* Primary Resume Interrupt */
#define GSR_SCR		(1 << 9)	/* Secondary Codec Ready */
#define GSR_PCR		(1 << 8)	/*  Primary Codec Ready */
#define GSR_MINT	(1 << 7)	/* Mic In Interrupt */
#define GSR_POINT	(1 << 6)	/* PCM Out Interrupt */
#define GSR_PIINT	(1 << 5)	/* PCM In Interrupt */
#define GSR_MOINT	(1 << 2)	/* Modem Out Interrupt */
#define GSR_MIINT	(1 << 1)	/* Modem In Interrupt */
#define GSR_GSCI	(1 << 0)	/* Codec GPI Status Change Interrupt */

#define CAR		0x40500020  /* CODEC Access Register */
#define CAR_CAIP	(1 << 0)	/* Codec Access In Progress */

#define PCDR		0x40500040  /* PCM FIFO Data Register */
#define MCDR		0x40500060  /* Mic-in FIFO Data Register */

#define MOCR		0x40500100  /* Modem Out Control Register */
#define MOCR_FEIE	(1 << 3)	/* FIFO Error */

#define MICR		0x40500108  /* Modem In Control Register */
#define MICR_FEIE	(1 << 3)	/* FIFO Error */

#define MOSR		0x40500110  /* Modem Out Status Register */
#define MOSR_FIFOE	(1 << 4)	/* FIFO error */

#define MISR		0x40500118  /* Modem In Status Register */
#define MISR_FIFOE	(1 << 4)	/* FIFO error */

#define MODR		0x40500140  /* Modem FIFO Data Register */

#define PAC_REG_BASE	0x40500200  /* Primary Audio Codec */
#define SAC_REG_BASE	0x40500300  /* Secondary Audio Codec */
#define PMC_REG_BASE	0x40500400  /* Primary Modem Codec */
#define SMC_REG_BASE	0x40500500  /* Secondary Modem Codec */


/*
 * USB Device Controller
 */
#if defined(CONFIG_CPU_PXA27X) || defined(CONFIG_CPU_MONAHANS)

#define UDCCR		0x40600000	/* UDC Control Register */
#define UDCCR_UDE	(1 << 0)		/* UDC enable */
#define UDCCR_UDA	(1 << 1)		/* UDC active */
#define UDCCR_RSM	(1 << 2)		/* Device resume */
#define UDCCR_EMCE	(1 << 3)		/* Endpoint Memory Configuration Error */
#define UDCCR_SMAC	(1 << 4)		/* Switch Endpoint Memory to Active Configuration */
#define UDCCR_RESIR	(1 << 29)		/* Resume interrupt request */
#define UDCCR_SUSIR	(1 << 28)		/* Suspend interrupt request */
#define UDCCR_SM	(1 << 28)		/* Suspend interrupt mask */
#define UDCCR_RSTIR	(1 << 27)		/* Reset interrupt request */
#define UDCCR_REM	(1 << 27)		/* Reset interrupt mask */
#define UDCCR_RM	(1 << 29)		/* resume interrupt mask */
#define UDCCR_SRM	(UDCCR_SM|UDCCR_RM)
#define UDCCR_OEN	(1 << 31)		/* On-the-Go Enable */
#define UDCCR_AALTHNP	(1 << 30)		/* A-device Alternate Host Negotiation Protocol Port Support */
#define UDCCR_AHNP	(1 << 29)		/* A-device Host Negotiation Protocol Support */
#define UDCCR_BHNP	(1 << 28)		/* B-device Host Negotiation Protocol Enable */
#define UDCCR_DWRE	(1 << 16)		/* Device Remote Wake-up Enable */
#define UDCCR_ACN	(0x03 << 11)		/* Active UDC configuration Number */
#define UDCCR_ACN_S	11
#define UDCCR_AIN	(0x07 << 8)		/* Active UDC interface Number */
#define UDCCR_AIN_S	8
#define UDCCR_AAISN	(0x07 << 5)		/* Active UDC Alternate Interface  Setting Number */
#define UDCCR_AAISN_S	5

#define UDCCS0		0x40600100	/* UDC Endpoint 0 Control/Status Register */
#define UDCCS0_OPR	(1 << 0)		/* OUT packet ready */
#define UDCCS0_IPR	(1 << 1)		/* IN packet ready */
#define UDCCS0_FTF	(1 << 2)		/* Flush Tx FIFO */
#define UDCCS0_DRWF	(1 << 16)		/* Device remote wakeup feature */
#define UDCCS0_SST	(1 << 4)		/* Sent stall */
#define UDCCS0_FST	(1 << 5)		/* Force stall */
#define UDCCS0_RNE	(1 << 6)		/* Receive FIFO no empty */
#define UDCCS0_SA	(1 << 7)		/* Setup active */

/* Bulk IN - Endpoint 1,6,11 */
#define UDCCS1		0x40600104  /* UDC Endpoint 1 (IN) Control/Status Register */
#define UDCCS6		0x40600028  /* UDC Endpoint 6 (IN) Control/Status Register */
#define UDCCS11		0x4060003C  /* UDC Endpoint 11 (IN) Control/Status Register */

#define UDCCS_BI_TFS	(1 << 0)	/* Transmit FIFO service */
#define UDCCS_BI_TPC	(1 << 1)	/* Transmit packet complete */
#define UDCCS_BI_FTF	(1 << 8)	/* Flush Tx FIFO */
#define UDCCS_BI_TUR	(1 << 3)	/* Transmit FIFO underrun */
#define UDCCS_BI_SST	(1 << 4)	/* Sent stall */
#define UDCCS_BI_FST	(1 << 5)	/* Force stall */
#define UDCCS_BI_TSP	(1 << 7)	/* Transmit short packet */

/* Bulk OUT - Endpoint 2,7,12 */
#define UDCCS2		0x40600108  /* UDC Endpoint 2 (OUT) Control/Status Register */
#define UDCCS7		0x4060002C  /* UDC Endpoint 7 (OUT) Control/Status Register */
#define UDCCS12		0x40600040  /* UDC Endpoint 12 (OUT) Control/Status Register */

#define UDCCS_BO_RFS	(1 << 0)	/* Receive FIFO service */
#define UDCCS_BO_RPC	(1 << 1)	/* Receive packet complete */
#define UDCCS_BO_DME	(1 << 3)	/* DMA enable */
#define UDCCS_BO_SST	(1 << 4)	/* Sent stall */
#define UDCCS_BO_FST	(1 << 5)	/* Force stall */
#define UDCCS_BO_RNE	(1 << 6)	/* Receive FIFO not empty */
#define UDCCS_BO_RSP	(1 << 7)	/* Receive short packet */

/* Isochronous IN - Endpoint 3,8,13 */
#define UDCCS3		0x4060001C  /* UDC Endpoint 3 (IN) Control/Status Register */
#define UDCCS8		0x40600030  /* UDC Endpoint 8 (IN) Control/Status Register */
#define UDCCS13		0x40600044  /* UDC Endpoint 13 (IN) Control/Status Register */

#define UDCCS_II_TFS	(1 << 0)	/* Transmit FIFO service */
#define UDCCS_II_TPC	(1 << 1)	/* Transmit packet complete */
#define UDCCS_II_FTF	(1 << 2)	/* Flush Tx FIFO */
#define UDCCS_II_TUR	(1 << 3)	/* Transmit FIFO underrun */
#define UDCCS_II_TSP	(1 << 7)	/* Transmit short packet */

/* Isochronous OUT - Endpoint 4,9,14 */
#define UDCCS4		0x40600020  /* UDC Endpoint 4 (OUT) Control/Status Register */
#define UDCCS9		0x40600034  /* UDC Endpoint 9 (OUT) Control/Status Register */
#define UDCCS14		0x40600048  /* UDC Endpoint 14 (OUT) Control/Status Register */

#define UDCCS_IO_RFS	(1 << 0)	/* Receive FIFO service */
#define UDCCS_IO_RPC	(1 << 1)	/* Receive packet complete */
#define UDCCS_IO_ROF	(1 << 3)	/* Receive overflow */
#define UDCCS_IO_DME	(1 << 3)	/* DMA enable */
#define UDCCS_IO_RNE	(1 << 6)	/* Receive FIFO not empty */
#define UDCCS_IO_RSP	(1 << 7)	/* Receive short packet */

/* Interrupt IN - Endpoint 5,10,15 */
#define UDCCS5		0x40600024  /* UDC Endpoint 5 (Interrupt) Control/Status Register */
#define UDCCS10		0x40600038  /* UDC Endpoint 10 (Interrupt) Control/Status Register */
#define UDCCS15		0x4060004C  /* UDC Endpoint 15 (Interrupt) Control/Status Register */

#define UDCCS_INT_TFS	(1 << 0)	/* Transmit FIFO service */
#define UDCCS_INT_TPC	(1 << 1)	/* Transmit packet complete */
#define UDCCS_INT_FTF	(1 << 2)	/* Flush Tx FIFO */
#define UDCCS_INT_TUR	(1 << 3)	/* Transmit FIFO underrun */
#define UDCCS_INT_SST	(1 << 4)	/* Sent stall */
#define UDCCS_INT_FST	(1 << 5)	/* Force stall */
#define UDCCS_INT_TSP	(1 << 7)	/* Transmit short packet */

#define UFNRH		0x40600060  /* UDC Frame Number Register High */
#define UFNRL		0x40600014  /* UDC Frame Number Register Low */
#define UBCR2		0x40600208  /* UDC Byte Count Reg 2 */
#define UBCR4		0x4060006c  /* UDC Byte Count Reg 4 */
#define UBCR7		0x40600070  /* UDC Byte Count Reg 7 */
#define UBCR9		0x40600074  /* UDC Byte Count Reg 9 */
#define UBCR12		0x40600078  /* UDC Byte Count Reg 12 */
#define UBCR14		0x4060007c  /* UDC Byte Count Reg 14 */
#define UDDR0		0x40600300  /* UDC Endpoint 0 Data Register */
#define UDDR1		0x40600304  /* UDC Endpoint 1 Data Register */
#define UDDR2		0x40600308  /* UDC Endpoint 2 Data Register */
#define UDDR3		0x40600200  /* UDC Endpoint 3 Data Register */
#define UDDR4		0x40600400  /* UDC Endpoint 4 Data Register */
#define UDDR5		0x406000A0  /* UDC Endpoint 5 Data Register */
#define UDDR6		0x40600600  /* UDC Endpoint 6 Data Register */
#define UDDR7		0x40600680  /* UDC Endpoint 7 Data Register */
#define UDDR8		0x40600700  /* UDC Endpoint 8 Data Register */
#define UDDR9		0x40600900  /* UDC Endpoint 9 Data Register */
#define UDDR10		0x406000C0  /* UDC Endpoint 10 Data Register */
#define UDDR11		0x40600B00  /* UDC Endpoint 11 Data Register */
#define UDDR12		0x40600B80  /* UDC Endpoint 12 Data Register */
#define UDDR13		0x40600C00  /* UDC Endpoint 13 Data Register */
#define UDDR14		0x40600E00  /* UDC Endpoint 14 Data Register */
#define UDDR15		0x406000E0  /* UDC Endpoint 15 Data Register */

#define UICR0		0x40600004  /* UDC Interrupt Control Register 0 */

#define UICR0_IM0	(1 << 0)	/* Interrupt mask ep 0 */
#define UICR0_IM1	(1 << 1)	/* Interrupt mask ep 1 */
#define UICR0_IM2	(1 << 2)	/* Interrupt mask ep 2 */
#define UICR0_IM3	(1 << 3)	/* Interrupt mask ep 3 */
#define UICR0_IM4	(1 << 4)	/* Interrupt mask ep 4 */
#define UICR0_IM5	(1 << 5)	/* Interrupt mask ep 5 */
#define UICR0_IM6	(1 << 6)	/* Interrupt mask ep 6 */
#define UICR0_IM7	(1 << 7)	/* Interrupt mask ep 7 */

#define UICR1		0x40600008  /* UDC Interrupt Control Register 1 */

#define UICR1_IM8	(1 << 0)	/* Interrupt mask ep 8 */
#define UICR1_IM9	(1 << 1)	/* Interrupt mask ep 9 */
#define UICR1_IM10	(1 << 2)	/* Interrupt mask ep 10 */
#define UICR1_IM11	(1 << 3)	/* Interrupt mask ep 11 */
#define UICR1_IM12	(1 << 4)	/* Interrupt mask ep 12 */
#define UICR1_IM13	(1 << 5)	/* Interrupt mask ep 13 */
#define UICR1_IM14	(1 << 6)	/* Interrupt mask ep 14 */
#define UICR1_IM15	(1 << 7)	/* Interrupt mask ep 15 */

#define USIR0		0x4060000C  /* UDC Status Interrupt Register 0 */

#define USIR0_IR0	(1 << 0)	/* Interrup request ep 0 */
#define USIR0_IR1	(1 << 2)	/* Interrup request ep 1 */
#define USIR0_IR2	(1 << 4)	/* Interrup request ep 2 */
#define USIR0_IR3	(1 << 3)	/* Interrup request ep 3 */
#define USIR0_IR4	(1 << 4)	/* Interrup request ep 4 */
#define USIR0_IR5	(1 << 5)	/* Interrup request ep 5 */
#define USIR0_IR6	(1 << 6)	/* Interrup request ep 6 */
#define USIR0_IR7	(1 << 7)	/* Interrup request ep 7 */

#define USIR1		0x40600010  /* UDC Status Interrupt Register 1 */

#define USIR1_IR8	(1 << 0)	/* Interrup request ep 8 */
#define USIR1_IR9	(1 << 1)	/* Interrup request ep 9 */
#define USIR1_IR10	(1 << 2)	/* Interrup request ep 10 */
#define USIR1_IR11	(1 << 3)	/* Interrup request ep 11 */
#define USIR1_IR12	(1 << 4)	/* Interrup request ep 12 */
#define USIR1_IR13	(1 << 5)	/* Interrup request ep 13 */
#define USIR1_IR14	(1 << 6)	/* Interrup request ep 14 */
#define USIR1_IR15	(1 << 7)	/* Interrup request ep 15 */


#define UDCICR0         0x40600004	/* UDC Interrupt Control Register0 */
#define UDCICR1         0x40600008	/* UDC Interrupt Control Register1 */
#define UDCICR_FIFOERR	(1 << 1)			/* FIFO Error interrupt for EP */
#define UDCICR_PKTCOMPL (1 << 0)			/* Packet Complete interrupt for EP */

#define UDCICR_INT(n, intr) (((intr) & 0x03) << (((n) & 0x0F) * 2))
#define UDCICR1_IECC	(1 << 31)	/* IntEn - Configuration Change */
#define UDCICR1_IESOF	(1 << 30)	/* IntEn - Start of Frame */
#define UDCICR1_IERU	(1 << 29)	/* IntEn - Resume */
#define UDCICR1_IESU	(1 << 28)	/* IntEn - Suspend */
#define UDCICR1_IERS	(1 << 27)	/* IntEn - Reset */

#define UDCISR0         0x4060000C /* UDC Interrupt Status Register 0 */
#define UDCISR1         0x40600010 /* UDC Interrupt Status Register 1 */
#define UDCISR_INT(n, intr) (((intr) & 0x03) << (((n) & 0x0F) * 2))
#define UDCISR1_IRCC	(1 << 31)	/* IntEn - Configuration Change */
#define UDCISR1_IRSOF	(1 << 30)	/* IntEn - Start of Frame */
#define UDCISR1_IRRU	(1 << 29)	/* IntEn - Resume */
#define UDCISR1_IRSU	(1 << 28)	/* IntEn - Suspend */
#define UDCISR1_IRRS	(1 << 27)	/* IntEn - Reset */


#define UDCFNR			0x40600014 /* UDC Frame Number Register */
#define UDCOTGICR		0x40600018 /* UDC On-The-Go interrupt control */
#define UDCOTGICR_IESF		(1 << 24)	/* OTG SET_FEATURE command recvd */
#define UDCOTGICR_IEXR		(1 << 17)	/* Extra Transciever Interrupt Rising Edge Interrupt Enable */
#define UDCOTGICR_IEXF		(1 << 16)	/* Extra Transciever Interrupt Falling Edge Interrupt Enable */
#define UDCOTGICR_IEVV40R	(1 << 9)	/* OTG Vbus Valid 4.0V Rising Edge Interrupt Enable */
#define UDCOTGICR_IEVV40F	(1 << 8)	/* OTG Vbus Valid 4.0V Falling Edge Interrupt Enable */
#define UDCOTGICR_IEVV44R	(1 << 7)	/* OTG Vbus Valid 4.4V Rising Edge  Interrupt Enable */
#define UDCOTGICR_IEVV44F	(1 << 6)	/* OTG Vbus Valid 4.4V Falling Edge Interrupt Enable */
#define UDCOTGICR_IESVR		(1 << 5)	/* OTG Session Valid Rising Edge Interrupt Enable */
#define UDCOTGICR_IESVF		(1 << 4)	/* OTG Session Valid Falling Edge Interrupt Enable */
#define UDCOTGICR_IESDR		(1 << 3)	/* OTG A-Device SRP Detect Rising Edge Interrupt Enable */
#define UDCOTGICR_IESDF		(1 << 2)	/* OTG A-Device SRP Detect Falling  Edge Interrupt Enable */
#define UDCOTGICR_IEIDR		(1 << 1)	/* OTG ID Change Rising Edge Interrupt Enable */
#define UDCOTGICR_IEIDF		(1 << 0)	/* OTG ID Change Falling Edge Interrupt Enable */

#define UDCCSN(x)	(0x40600100 + ((x) << 2))
#define UDCCSR0		0x40600100 /* UDC Control/Status register - Endpoint 0 */

#define UDCCSR0_SA	(1 << 7)	/* Setup Active */
#define UDCCSR0_RNE	(1 << 6)	/* Receive FIFO Not Empty */
#define UDCCSR0_FST	(1 << 5)	/* Force Stall */
#define UDCCSR0_SST	(1 << 4)	/* Sent Stall */
#define UDCCSR0_DME	(1 << 3)	/* DMA Enable */
#define UDCCSR0_FTF	(1 << 2)	/* Flush Transmit FIFO */
#define UDCCSR0_IPR	(1 << 1)	/* IN Packet Ready */
#define UDCCSR0_OPC	(1 << 0)	/* OUT Packet Complete */

#define UDCCSRA         0x40600104 /* UDC Control/Status register - Endpoint A */
#define UDCCSRB         0x40600108 /* UDC Control/Status register - Endpoint B */
#define UDCCSRC         0x4060010C /* UDC Control/Status register - Endpoint C */
#define UDCCSRD         0x40600110 /* UDC Control/Status register - Endpoint D */
#define UDCCSRE         0x40600114 /* UDC Control/Status register - Endpoint E */
#define UDCCSRF         0x40600118 /* UDC Control/Status register - Endpoint F */
#define UDCCSRG         0x4060011C /* UDC Control/Status register - Endpoint G */
#define UDCCSRH         0x40600120 /* UDC Control/Status register - Endpoint H */
#define UDCCSRI         0x40600124 /* UDC Control/Status register - Endpoint I */
#define UDCCSRJ         0x40600128 /* UDC Control/Status register - Endpoint J */
#define UDCCSRK         0x4060012C /* UDC Control/Status register - Endpoint K */
#define UDCCSRL         0x40600130 /* UDC Control/Status register - Endpoint L */
#define UDCCSRM         0x40600134 /* UDC Control/Status register - Endpoint M */
#define UDCCSRN         0x40600138 /* UDC Control/Status register - Endpoint N */
#define UDCCSRP         0x4060013C /* UDC Control/Status register - Endpoint P */
#define UDCCSRQ         0x40600140 /* UDC Control/Status register - Endpoint Q */
#define UDCCSRR         0x40600144 /* UDC Control/Status register - Endpoint R */
#define UDCCSRS         0x40600148 /* UDC Control/Status register - Endpoint S */
#define UDCCSRT         0x4060014C /* UDC Control/Status register - Endpoint T */
#define UDCCSRU         0x40600150 /* UDC Control/Status register - Endpoint U */
#define UDCCSRV         0x40600154 /* UDC Control/Status register - Endpoint V */
#define UDCCSRW         0x40600158 /* UDC Control/Status register - Endpoint W */
#define UDCCSRX         0x4060015C /* UDC Control/Status register - Endpoint X */

#define UDCCSR_DPE	(1 << 9)	/* Data Packet Error */
#define UDCCSR_FEF	(1 << 8)	/* Flush Endpoint FIFO */
#define UDCCSR_SP	(1 << 7)	/* Short Packet Control/Status */
#define UDCCSR_BNE	(1 << 6)	/* Buffer Not Empty (IN endpoints) */
#define UDCCSR_BNF	(1 << 6)	/* Buffer Not Full (OUT endpoints) */
#define UDCCSR_FST	(1 << 5)	/* Force STALL */
#define UDCCSR_SST	(1 << 4)	/* Sent STALL */
#define UDCCSR_DME	(1 << 3)	/* DMA Enable */
#define UDCCSR_TRN	(1 << 2)	/* Tx/Rx NAK */
#define UDCCSR_PC	(1 << 1)	/* Packet Complete */
#define UDCCSR_FS	(1 << 0)	/* FIFO needs service */

#define UDCBCN(x)	(0x40600200 + ((x) << 2))
#define UDCBCR0         0x40600200 /* Byte Count Register - EP0 */
#define UDCBCRA         0x40600204 /* Byte Count Register - EPA */
#define UDCBCRB         0x40600208 /* Byte Count Register - EPB */
#define UDCBCRC         0x4060020C /* Byte Count Register - EPC */
#define UDCBCRD         0x40600210 /* Byte Count Register - EPD */
#define UDCBCRE         0x40600214 /* Byte Count Register - EPE */
#define UDCBCRF         0x40600218 /* Byte Count Register - EPF */
#define UDCBCRG         0x4060021C /* Byte Count Register - EPG */
#define UDCBCRH         0x40600220 /* Byte Count Register - EPH */
#define UDCBCRI         0x40600224 /* Byte Count Register - EPI */
#define UDCBCRJ         0x40600228 /* Byte Count Register - EPJ */
#define UDCBCRK         0x4060022C /* Byte Count Register - EPK */
#define UDCBCRL         0x40600230 /* Byte Count Register - EPL */
#define UDCBCRM         0x40600234 /* Byte Count Register - EPM */
#define UDCBCRN         0x40600238 /* Byte Count Register - EPN */
#define UDCBCRP         0x4060023C /* Byte Count Register - EPP */
#define UDCBCRQ         0x40600240 /* Byte Count Register - EPQ */
#define UDCBCRR         0x40600244 /* Byte Count Register - EPR */
#define UDCBCRS         0x40600248 /* Byte Count Register - EPS */
#define UDCBCRT         0x4060024C /* Byte Count Register - EPT */
#define UDCBCRU         0x40600250 /* Byte Count Register - EPU */
#define UDCBCRV         0x40600254 /* Byte Count Register - EPV */
#define UDCBCRW         0x40600258 /* Byte Count Register - EPW */
#define UDCBCRX         0x4060025C /* Byte Count Register - EPX */

#define UDCDN(x)	(0x40600300 + ((x) << 2))
#define UDCDR0          0x40600300 /* Data Register - EP0 */
#define UDCDRA          0x40600304 /* Data Register - EPA */
#define UDCDRB          0x40600308 /* Data Register - EPB */
#define UDCDRC          0x4060030C /* Data Register - EPC */
#define UDCDRD          0x40600310 /* Data Register - EPD */
#define UDCDRE          0x40600314 /* Data Register - EPE */
#define UDCDRF          0x40600318 /* Data Register - EPF */
#define UDCDRG          0x4060031C /* Data Register - EPG */
#define UDCDRH          0x40600320 /* Data Register - EPH */
#define UDCDRI          0x40600324 /* Data Register - EPI */
#define UDCDRJ          0x40600328 /* Data Register - EPJ */
#define UDCDRK          0x4060032C /* Data Register - EPK */
#define UDCDRL          0x40600330 /* Data Register - EPL */
#define UDCDRM          0x40600334 /* Data Register - EPM */
#define UDCDRN          0x40600338 /* Data Register - EPN */
#define UDCDRP          0x4060033C /* Data Register - EPP */
#define UDCDRQ          0x40600340 /* Data Register - EPQ */
#define UDCDRR          0x40600344 /* Data Register - EPR */
#define UDCDRS          0x40600348 /* Data Register - EPS */
#define UDCDRT          0x4060034C /* Data Register - EPT */
#define UDCDRU          0x40600350 /* Data Register - EPU */
#define UDCDRV          0x40600354 /* Data Register - EPV */
#define UDCDRW          0x40600358 /* Data Register - EPW */
#define UDCDRX          0x4060035C /* Data Register - EPX */

#define UDCCN(x)	(0x40600400 + ((x) << 2))
#define UDCCRA          0x40600404 /* Configuration register EPA */
#define UDCCRB          0x40600408 /* Configuration register EPB */
#define UDCCRC          0x4060040C /* Configuration register EPC */
#define UDCCRD          0x40600410 /* Configuration register EPD */
#define UDCCRE          0x40600414 /* Configuration register EPE */
#define UDCCRF          0x40600418 /* Configuration register EPF */
#define UDCCRG          0x4060041C /* Configuration register EPG */
#define UDCCRH          0x40600420 /* Configuration register EPH */
#define UDCCRI          0x40600424 /* Configuration register EPI */
#define UDCCRJ          0x40600428 /* Configuration register EPJ */
#define UDCCRK          0x4060042C /* Configuration register EPK */
#define UDCCRL          0x40600430 /* Configuration register EPL */
#define UDCCRM          0x40600434 /* Configuration register EPM */
#define UDCCRN          0x40600438 /* Configuration register EPN */
#define UDCCRP          0x4060043C /* Configuration register EPP */
#define UDCCRQ          0x40600440 /* Configuration register EPQ */
#define UDCCRR          0x40600444 /* Configuration register EPR */
#define UDCCRS          0x40600448 /* Configuration register EPS */
#define UDCCRT          0x4060044C /* Configuration register EPT */
#define UDCCRU          0x40600450 /* Configuration register EPU */
#define UDCCRV          0x40600454 /* Configuration register EPV */
#define UDCCRW          0x40600458 /* Configuration register EPW */
#define UDCCRX          0x4060045C /* Configuration register EPX */

#define UDCCONR_CN	(0x03 << 25)	/* Configuration Number */
#define UDCCONR_CN_S	(25)
#define UDCCONR_IN	(0x07 << 22)	/* Interface Number */
#define UDCCONR_IN_S	(22)
#define UDCCONR_AISN	(0x07 << 19)	/* Alternate Interface Number */
#define UDCCONR_AISN_S	(19)
#define UDCCONR_EN	(0x0f << 15)	/* Endpoint Number */
#define UDCCONR_EN_S	(15)
#define UDCCONR_ET	(0x03 << 13)	/* Endpoint Type: */
#define UDCCONR_ET_S	(13)
#define UDCCONR_ET_INT	(0x03 << 13)	/* Interrupt */
#define UDCCONR_ET_BULK	(0x02 << 13)	/* Bulk */
#define UDCCONR_ET_ISO	(0x01 << 13)	/* Isochronous */
#define UDCCONR_ET_NU	(0x00 << 13)	/* Not used */
#define UDCCONR_ED	(1 << 12)	/* Endpoint Direction */
#define UDCCONR_MPS	(0x3ff << 2)	/* Maximum Packet Size */
#define UDCCONR_MPS_S	(2)
#define UDCCONR_DE	(1 << 1)	/* Double Buffering Enable */
#define UDCCONR_EE	(1 << 0)	/* Endpoint Enable */


#define UDC_INT_FIFOERROR	(0x2)
#define UDC_INT_PACKETCMP	(0x1)
#define UDC_FNR_MASK		(0x7ff)
#define UDCCSR_WR_MASK		(UDCCSR_DME|UDCCSR_FST)
#define UDC_BCR_MASK		(0x3ff)

#endif /* CONFIG_CPU_PXA27X */

#if defined(CONFIG_CPU_PXA27X) || defined(CONFIG_CPU_MONAHANS)

/******************************************************************************/
/*
 * USB Host Controller
 */
#define OHCI_REGS_BASE	0x4C000000	/* required for ohci driver */
#define UHCREV		0x4C000000
#define UHCHCON		0x4C000004
#define UHCCOMS		0x4C000008
#define UHCINTS		0x4C00000C
#define UHCINTE		0x4C000010
#define UHCINTD		0x4C000014
#define UHCHCCA		0x4C000018
#define UHCPCED		0x4C00001C
#define UHCCHED		0x4C000020
#define UHCCCED		0x4C000024
#define UHCBHED		0x4C000028
#define UHCBCED		0x4C00002C
#define UHCDHEAD	0x4C000030
#define UHCFMI		0x4C000034
#define UHCFMR		0x4C000038
#define UHCFMN		0x4C00003C
#define UHCPERS		0x4C000040
#define UHCLST		0x4C000044
#define UHCRHDA		0x4C000048
#define UHCRHDB		0x4C00004C
#define UHCRHS		0x4C000050
#define UHCRHPS1	0x4C000054
#define UHCRHPS2	0x4C000058
#define UHCRHPS3	0x4C00005C
#define UHCSTAT		0x4C000060
#define UHCHR		0x4C000064
#define UHCHIE		0x4C000068
#define UHCHIT		0x4C00006C

#define UHCCOMS_HCR	(1<<0)

#define UHCHR_FSBIR	(1<<0)
#define UHCHR_FHR	(1<<1)
#define UHCHR_CGR	(1<<2)
#define UHCHR_SSDC	(1<<3)
#define UHCHR_UIT	(1<<4)
#define UHCHR_SSE	(1<<5)
#define UHCHR_PSPL	(1<<6)
#define UHCHR_PCPL	(1<<7)
#define UHCHR_SSEP0	(1<<9)
#define UHCHR_SSEP1	(1<<10)
#define UHCHR_SSEP2	(1<<11)

#define UHCHIE_UPRIE	(1<<13)
#define UHCHIE_UPS2IE	(1<<12)
#define UHCHIE_UPS1IE	(1<<11)
#define UHCHIE_TAIE	(1<<10)
#define UHCHIE_HBAIE	(1<<8)
#define UHCHIE_RWIE	(1<<7)

#define UP2OCR		0x40600020

#define UP2OCR_HXOE	(1<<17)
#define UP2OCR_HXS	(1<<16)
#define UP2OCR_IDON	(1<<10)
#define UP2OCR_EXSUS	(1<<9)
#define UP2OCR_EXSP	(1<<8)
#define UP2OCR_DMSTATE	(1<<7)
#define UP2OCR_VPM	(1<<6)
#define UP2OCR_DPSTATE	(1<<5)
#define UP2OCR_DPPUE	(1<<4)
#define UP2OCR_DMPDE	(1<<3)
#define UP2OCR_DPPDE	(1<<2)
#define UP2OCR_CPVPE	(1<<1)
#define UP2OCR_CPVEN	(1<<0)

#endif	/* CONFIG_CPU_PXA27X || CONFIG_CPU_MONAHANS */

/******************************************************************************/
/*
 * Fast Infrared Communication Port
 */
#define ICCR0		0x40800000  /* ICP Control Register 0 */
#define ICCR1		0x40800004  /* ICP Control Register 1 */
#define ICCR2		0x40800008  /* ICP Control Register 2 */
#define ICDR		0x4080000c  /* ICP Data Register */
#define ICSR0		0x40800014  /* ICP Status Register 0 */
#define ICSR1		0x40800018  /* ICP Status Register 1 */

/*
 * Real Time Clock
 */
#define RCNR		0x40900000  /* RTC Count Register */
#define RTAR		0x40900004  /* RTC Alarm Register */
#define RTSR		0x40900008  /* RTC Status Register */
#define RTTR		0x4090000C  /* RTC Timer Trim Register */
#define RDAR1		0x40900018  /* Wristwatch Day Alarm Reg 1 */
#define RDAR2		0x40900020  /* Wristwatch Day Alarm Reg 2 */
#define RYAR1		0x4090001C  /* Wristwatch Year Alarm Reg 1 */
#define RYAR2		0x40900024  /* Wristwatch Year Alarm Reg 2 */
#define SWAR1		0x4090002C  /* Stopwatch Alarm Register 1 */
#define SWAR2		0x40900030  /* Stopwatch Alarm Register 2 */
#define PIAR		0x40900038  /* Periodic Interrupt Alarm Register */
#define RDCR		0x40900010  /* RTC Day Count Register. */
#define RYCR		0x40900014  /* RTC Year Count Register. */
#define SWCR		0x40900028  /* Stopwatch Count Register */
#define RTCPICR		0x40900034  /* Periodic Interrupt Counter Register */

#define RTSR_PICE	(1 << 15)	/* Peridoc interrupt count enable */
#define RTSR_PIALE	(1 << 14)	/* Peridoc interrupt Alarm enable */
#define RTSR_PIAL	(1 << 13)	/* Peridoc  interrupt Alarm status */
#define RTSR_HZE	(1 << 3)	/* HZ interrupt enable */
#define RTSR_ALE	(1 << 2)	/* RTC alarm interrupt enable */
#define RTSR_HZ		(1 << 1)	/* HZ rising-edge detected */
#define RTSR_AL		(1 << 0)	/* RTC alarm detected */

/******************************************************************************/
/*
 * OS Timer & Match Registers
 */
#define OSMR0		0x40A00000  /* OS Timer Match Register 0 */
#define OSMR1		0x40A00004  /* OS Timer Match Register 1 */
#define OSMR2		0x40A00008  /* OS Timer Match Register 2 */
#define OSMR3		0x40A0000C  /* OS Timer Match Register 3 */
#define OSCR		0x40A00010  /* OS Timer Counter Register */
#define OSSR		0x40A00014  /* OS Timer Status Register */
#define OWER		0x40A00018  /* OS Timer Watchdog Enable Register */
#define OIER		0x40A0001C  /* OS Timer Interrupt Enable Register */

#if defined(CONFIG_CPU_PXA27X) || defined(CONFIG_CPU_MONAHANS)
#define OSCR4		0x40A00040  /* OS Timer Counter Register 4 */
#define OSCR5		0x40A00044  /* OS Timer Counter Register 5 */
#define OSCR6		0x40A00048  /* OS Timer Counter Register 6 */
#define OSCR7		0x40A0004C  /* OS Timer Counter Register 7 */
#define OSCR8		0x40A00050  /* OS Timer Counter Register 8 */
#define OSCR9		0x40A00054  /* OS Timer Counter Register 9 */
#define OSCR10		0x40A00058  /* OS Timer Counter Register 10 */
#define OSCR11		0x40A0005C  /* OS Timer Counter Register 11 */

#define OSMR4		0x40A00080  /* OS Timer Match Register 4 */
#define OSMR5		0x40A00084  /* OS Timer Match Register 5 */
#define OSMR6		0x40A00088  /* OS Timer Match Register 6 */
#define OSMR7		0x40A0008C  /* OS Timer Match Register 7 */
#define OSMR8		0x40A00090  /* OS Timer Match Register 8 */
#define OSMR9		0x40A00094  /* OS Timer Match Register 9 */
#define OSMR10		0x40A00098  /* OS Timer Match Register 10 */
#define OSMR11		0x40A0009C  /* OS Timer Match Register 11 */

#define OMCR4		0x40A000C0  /* OS Match Control Register 4 */
#define OMCR5		0x40A000C4  /* OS Match Control Register 5 */
#define OMCR6		0x40A000C8  /* OS Match Control Register 6 */
#define OMCR7		0x40A000CC  /* OS Match Control Register 7 */
#define OMCR8		0x40A000D0  /* OS Match Control Register 8 */
#define OMCR9		0x40A000D4  /* OS Match Control Register 9 */
#define OMCR10		0x40A000D8  /* OS Match Control Register 10 */
#define OMCR11		0x40A000DC  /* OS Match Control Register 11 */

#endif /* CONFIG_CPU_PXA27X || CONFIG_CPU_MONAHANS */

#define OSSR_M4		(1 << 4)	/* Match status channel 4 */
#define OSSR_M3		(1 << 3)	/* Match status channel 3 */
#define OSSR_M2		(1 << 2)	/* Match status channel 2 */
#define OSSR_M1		(1 << 1)	/* Match status channel 1 */
#define OSSR_M0		(1 << 0)	/* Match status channel 0 */

#define OWER_WME	(1 << 0)	/* Watchdog Match Enable */

#define OIER_E4		(1 << 4)	/* Interrupt enable channel 4 */
#define OIER_E3		(1 << 3)	/* Interrupt enable channel 3 */
#define OIER_E2		(1 << 2)	/* Interrupt enable channel 2 */
#define OIER_E1		(1 << 1)	/* Interrupt enable channel 1 */
#define OIER_E0		(1 << 0)	/* Interrupt enable channel 0 */

#define	OSCR_CLK_FREQ	3250

/******************************************************************************/
/*
 * Core Clock
 */

#if defined(CONFIG_CPU_MONAHANS)
#define ACCR		0x41340000  /* Application Subsystem Clock Configuration Register */
#define ACSR		0x41340004  /* Application Subsystem Clock Status Register */
#define AICSR		0x41340008  /* Application Subsystem Interrupt Control/Status Register */
#define CKENA		0x4134000C  /* A Clock Enable Register */
#define CKENB		0x41340010  /* B Clock Enable Register */
#define AC97_DIV	0x41340014  /* AC97 clock divisor value register */

#define ACCR_SMC_MASK	0x03800000	/* Static Memory Controller Frequency Select */
#define ACCR_SRAM_MASK	0x000c0000	/* SRAM Controller Frequency Select */
#define ACCR_FC_MASK	0x00030000	/* Frequency Change Frequency Select */
#define ACCR_HSIO_MASK	0x0000c000	/* High Speed IO Frequency Select */
#define ACCR_DDR_MASK	0x00003000	/* DDR Memory Controller Frequency Select */
#define ACCR_XN_MASK	0x00000700	/* Run Mode Frequency to Turbo Mode Frequency Multiplier */
#define ACCR_XL_MASK	0x0000001f	/* Crystal Frequency to Memory Frequency Multiplier */
#define ACCR_XPDIS	(1 << 31)
#define ACCR_SPDIS	(1 << 30)
#define ACCR_13MEND1	(1 << 27)
#define ACCR_D0CS	(1 << 26)
#define ACCR_13MEND2	(1 << 21)
#define ACCR_PCCE	(1 << 11)

#define CKENA_30_MSL0	(1 << 30)	/* MSL0 Interface Unit Clock Enable */
#define CKENA_29_SSP4	(1 << 29)	/* SSP3 Unit Clock Enable */
#define CKENA_28_SSP3	(1 << 28)	/* SSP2 Unit Clock Enable */
#define CKENA_27_SSP2	(1 << 27)	/* SSP1 Unit Clock Enable */
#define CKENA_26_SSP1	(1 << 26)	/* SSP0 Unit Clock Enable */
#define CKENA_25_TSI	(1 << 25)	/* TSI Clock Enable */
#define CKENA_24_AC97	(1 << 24)	/* AC97 Unit Clock Enable */
#define CKENA_23_STUART	(1 << 23)	/* STUART Unit Clock Enable */
#define CKENA_22_FFUART	(1 << 22)	/* FFUART Unit Clock Enable */
#define CKENA_21_BTUART	(1 << 21)	/* BTUART Unit Clock Enable */
#define CKENA_20_UDC	(1 << 20)	/* UDC Clock Enable */
#define CKENA_19_TPM	(1 << 19)	/* TPM Unit Clock Enable */
#define CKENA_18_USIM1	(1 << 18)	/* USIM1 Unit Clock Enable */
#define CKENA_17_USIM0	(1 << 17)	/* USIM0 Unit Clock Enable */
#define CKENA_15_CIR	(1 << 15)	/* Consumer IR Clock Enable */
#define CKENA_14_KEY	(1 << 14)	/* Keypad Controller Clock Enable */
#define CKENA_13_MMC1	(1 << 13)	/* MMC1 Clock Enable */
#define CKENA_12_MMC0	(1 << 12)	/* MMC0 Clock Enable */
#define CKENA_11_FLASH	(1 << 11)	/* Boot ROM Clock Enable */
#define CKENA_10_SRAM	(1 << 10)	/* SRAM Controller Clock Enable */
#define CKENA_9_SMC	(1 << 9)	/* Static Memory Controller */
#define CKENA_8_DMC	(1 << 8)	/* Dynamic Memory Controller */
#define CKENA_7_GRAPHICS (1 << 7)	/* 2D Graphics Clock Enable */
#define CKENA_6_USBCLI	(1 << 6)	/* USB Client Unit Clock Enable */
#define CKENA_4_NAND	(1 << 4)	/* NAND Flash Controller Clock Enable */
#define CKENA_3_CAMERA	(1 << 3)	/* Camera Interface Clock Enable */
#define CKENA_2_USBHOST	(1 << 2)	/* USB Host Unit Clock Enable */
#define CKENA_1_LCD	(1 << 1)	/* LCD Unit Clock Enable */

#define CKENB_9_SYSBUS2	(1 << 9)	/* System bus 2 */
#define CKENB_8_1WIRE	(1 << 8)	/* One Wire Interface Unit Clock Enable */
#define CKENB_7_GPIO	(1 << 7)	/* GPIO Clock Enable */
#define CKENB_6_IRQ	(1 << 6)	/* Interrupt Controller Clock Enable */
#define CKENB_4_I2C	(1 << 4)	/* I2C Unit Clock Enable */
#define CKENB_1_PWM1	(1 << 1)	/* PWM2 & PWM3 Clock Enable */
#define CKENB_0_PWM0	(1 << 0)	/* PWM0 & PWM1 Clock Enable */

#else /* if defined CONFIG_CPU_MONAHANS */

#define CCCR		0x41300000  /* Core Clock Configuration Register */
#define CKEN		0x41300004  /* Clock Enable Register */
#define OSCC		0x41300008  /* Oscillator Configuration Register */
#define CCSR		0x4130000C /* Core Clock Status Register */

#define CKEN23_SSP1	(1 << 23) /* SSP1 Unit Clock Enable */
#define CKEN22_MEMC	(1 << 22) /* Memory Controler */
#define CKEN21_MSHC	(1 << 21) /* Memery Stick Host Controller */
#define CKEN20_IM	(1 << 20) /* Internal Memory Clock Enable */
#define CKEN19_KEYPAD	(1 << 19) /* Keypad Interface Clock Enable */
#define CKEN18_USIM	(1 << 18) /* USIM Unit Clock Enable */
#define CKEN17_MSL	(1 << 17) /* MSL Interface Unit Clock Enable */
#define CKEN15_PWR_I2C	(1 << 15) /* PWR_I2C Unit Clock Enable */
#define CKEN9_OST	(1 << 9)  /* OS Timer Unit Clock Enable */
#define CKEN4_SSP3	(1 << 4)  /* SSP3 Unit Clock Enable */

#define CCCR_N_MASK	0x0380		/* Run Mode Frequency to Turbo Mode Frequency Multiplier */
#if !defined(CONFIG_CPU_PXA27X)
#define CCCR_M_MASK	0x0060		/* Memory Frequency to Run Mode Frequency Multiplier */
#endif
#define CCCR_L_MASK	0x001f		/* Crystal Frequency to Memory Frequency Multiplier */

#define CKEN24_CAMERA	(1 << 24)	/* Camera Interface Clock Enable */
#define CKEN23_SSP1	(1 << 23)	/* SSP1 Unit Clock Enable */
#define CKEN22_MEMC	(1 << 22)	/* Memory Controller Clock Enable */
#define CKEN21_MEMSTK	(1 << 21)	/* Memory Stick Host Controller */
#define CKEN20_IM	(1 << 20)	/* Internal Memory Clock Enable */
#define CKEN19_KEYPAD	(1 << 19)	/* Keypad Interface Clock Enable */
#define CKEN18_USIM	(1 << 18)	/* USIM Unit Clock Enable */
#define CKEN17_MSL	(1 << 17)	/* MSL Unit Clock Enable */
#define CKEN16_LCD	(1 << 16)	/* LCD Unit Clock Enable */
#define CKEN15_PWRI2C	(1 << 15)	/* PWR I2C Unit Clock Enable */
#define CKEN14_I2C	(1 << 14)	/* I2C Unit Clock Enable */
#define CKEN13_FICP	(1 << 13)	/* FICP Unit Clock Enable */
#define CKEN12_MMC	(1 << 12)	/* MMC Unit Clock Enable */
#define CKEN11_USB	(1 << 11)	/* USB Unit Clock Enable */
#if defined(CONFIG_CPU_PXA27X)
#define CKEN10_USBHOST	(1 << 10)	/* USB Host Unit Clock Enable */
#define CKEN24_CAMERA	(1 << 24)	/* Camera Unit Clock Enable */
#endif
#define CKEN8_I2S	(1 << 8)	/* I2S Unit Clock Enable */
#define CKEN7_BTUART	(1 << 7)	/* BTUART Unit Clock Enable */
#define CKEN6_FFUART	(1 << 6)	/* FFUART Unit Clock Enable */
#define CKEN5_STUART	(1 << 5)	/* STUART Unit Clock Enable */
#define CKEN3_SSP	(1 << 3)	/* SSP Unit Clock Enable */
#define CKEN2_AC97	(1 << 2)	/* AC97 Unit Clock Enable */
#define CKEN1_PWM1	(1 << 1)	/* PWM1 Clock Enable */
#define CKEN0_PWM0	(1 << 0)	/* PWM0 Clock Enable */

#define OSCC_OON	(1 << 1)	/* 32.768kHz OON (write-once only bit) */
#define OSCC_OOK	(1 << 0)	/* 32.768kHz OOK (read-only bit) */

#if !defined(CONFIG_CPU_PXA27X)
#define	 CCCR_L09      (0x1F)
#define	 CCCR_L27      (0x1)
#define	 CCCR_L32      (0x2)
#define	 CCCR_L36      (0x3)
#define	 CCCR_L40      (0x4)
#define	 CCCR_L45      (0x5)

#define	 CCCR_M1       (0x1 << 5)
#define	 CCCR_M2       (0x2 << 5)
#define	 CCCR_M4       (0x3 << 5)

#define	 CCCR_N10      (0x2 << 7)
#define	 CCCR_N15      (0x3 << 7)
#define	 CCCR_N20      (0x4 << 7)
#define	 CCCR_N25      (0x5 << 7)
#define	 CCCR_N30      (0x6 << 7)
#endif

#endif /* CONFIG_CPU_MONAHANS */

/******************************************************************************/
/*
 * Pulse Width Modulator
 */
#define PWM_CTRL0	0x40B00000  /* PWM 0 Control Register */
#define PWM_PWDUTY0	0x40B00004  /* PWM 0 Duty Cycle Register */
#define PWM_PERVAL0	0x40B00008  /* PWM 0 Period Control Register */

#define PWM_CTRL1	0x40C00000  /* PWM 1 Control Register */
#define PWM_PWDUTY1	0x40C00004  /* PWM 1 Duty Cycle Register */
#define PWM_PERVAL1	0x40C00008  /* PWM 1 Period Control Register */

#if defined(CONFIG_CPU_PXA27X) || defined(CONFIG_CPU_MONAHANS)
#define PWM_CTRL2	0x40B00010  /* PWM 2 Control Register */
#define PWM_PWDUTY2	0x40B00014  /* PWM 2 Duty Cycle Register */
#define PWM_PERVAL2	0x40B00018  /* PWM 2 Period Control Register */

#define PWM_CTRL3	0x40C00010  /* PWM 3 Control Register */
#define PWM_PWDUTY3	0x40C00014  /* PWM 3 Duty Cycle Register */
#define PWM_PERVAL3	0x40C00018  /* PWM 3 Period Control Register */
#endif /* CONFIG_CPU_PXA27X || CONFIG_CPU_MONAHANS */

/*
 * Interrupt Controller
 */
#define ICIP		0x40D00000  /* Interrupt Controller IRQ Pending Register */
#define ICMR		0x40D00004  /* Interrupt Controller Mask Register */
#define ICLR		0x40D00008  /* Interrupt Controller Level Register */
#define ICFP		0x40D0000C  /* Interrupt Controller FIQ Pending Register */
#define ICPR		0x40D00010  /* Interrupt Controller Pending Register */
#define ICCR		0x40D00014  /* Interrupt Controller Control Register */

#if defined(CONFIG_CPU_PXA27X) || defined(CONFIG_CPU_MONAHANS)
#define ICHP		0x40D00018  /* Interrupt Controller Highest Priority Register */
#define ICIP2		0x40D0009C  /* Interrupt Controller IRQ Pending Register 2 */
#define ICMR2		0x40D000A0  /* Interrupt Controller Mask Register 2 */
#define ICLR2		0x40D000A4  /* Interrupt Controller Level Register 2 */
#define ICFP2		0x40D000A8  /* Interrupt Controller FIQ Pending Register 2 */
#define ICPR2		0x40D000AC  /* Interrupt Controller Pending Register 2 */
#endif /* CONFIG_CPU_PXA27X || CONFIG_CPU_MONAHANS */

/******************************************************************************/
/*
 * General Purpose I/O
 */
#define GPLR0		0x40E00000  /* GPIO Pin-Level Register GPIO<31:0> */
#define GPLR1		0x40E00004  /* GPIO Pin-Level Register GPIO<63:32> */
#define GPLR2		0x40E00008  /* GPIO Pin-Level Register GPIO<80:64> */

#define GPDR0		0x40E0000C  /* GPIO Pin Direction Register GPIO<31:0> */
#define GPDR1		0x40E00010  /* GPIO Pin Direction Register GPIO<63:32> */
#define GPDR2		0x40E00014  /* GPIO Pin Direction Register GPIO<80:64> */

#define GPSR0		0x40E00018  /* GPIO Pin Output Set Register GPIO<31:0> */
#define GPSR1		0x40E0001C  /* GPIO Pin Output Set Register GPIO<63:32> */
#define GPSR2		0x40E00020  /* GPIO Pin Output Set Register GPIO<80:64> */

#define GPCR0		0x40E00024  /* GPIO Pin Output Clear Register GPIO<31:0> */
#define GPCR1		0x40E00028  /* GPIO Pin Output Clear Register GPIO <63:32> */
#define GPCR2		0x40E0002C  /* GPIO Pin Output Clear Register GPIO <80:64> */

#define GRER0		0x40E00030  /* GPIO Rising-Edge Detect Register GPIO<31:0> */
#define GRER1		0x40E00034  /* GPIO Rising-Edge Detect Register GPIO<63:32> */
#define GRER2		0x40E00038  /* GPIO Rising-Edge Detect Register GPIO<80:64> */

#define GFER0		0x40E0003C  /* GPIO Falling-Edge Detect Register GPIO<31:0> */
#define GFER1		0x40E00040  /* GPIO Falling-Edge Detect Register GPIO<63:32> */
#define GFER2		0x40E00044  /* GPIO Falling-Edge Detect Register GPIO<80:64> */

#define GEDR0		0x40E00048  /* GPIO Edge Detect Status Register GPIO<31:0> */
#define GEDR1		0x40E0004C  /* GPIO Edge Detect Status Register GPIO<63:32> */
#define GEDR2		0x40E00050  /* GPIO Edge Detect Status Register GPIO<80:64> */

#define GAFR0_L		0x40E00054  /* GPIO Alternate Function Select Register GPIO<15:0> */
#define GAFR0_U		0x40E00058  /* GPIO Alternate Function Select Register GPIO<31:16> */
#define GAFR1_L		0x40E0005C  /* GPIO Alternate Function Select Register GPIO<47:32> */
#define GAFR1_U		0x40E00060  /* GPIO Alternate Function Select Register GPIO<63:48> */
#define GAFR2_L		0x40E00064  /* GPIO Alternate Function Select Register GPIO<79:64> */
#define GAFR2_U		0x40E00068  /* GPIO Alternate Function Select Register GPIO 80 */

#if defined(CONFIG_CPU_PXA27X) || defined(CONFIG_CPU_MONAHANS)
#define GPLR3		0x40E00100  /* GPIO Pin-Level Register GPIO<127:96> */
#define GPDR3		0x40E0010C  /* GPIO Pin Direction Register GPIO<127:96> */
#define GPSR3		0x40E00118  /* GPIO Pin Output Set Register GPIO<127:96> */
#define GPCR3		0x40E00124  /* GPIO Pin Output Clear Register GPIO<127:96> */
#define GRER3		0x40E00130  /* GPIO Rising-Edge Detect Register GPIO<127:96> */
#define GFER3		0x40E0013C  /* GPIO Falling-Edge Detect Register GPIO<127:96> */
#define GEDR3		0x40E00148  /* GPIO Edge Detect Status Register GPIO<127:96> */
#define GAFR3_L		0x40E0006C  /* GPIO Alternate Function Select Register GPIO<111:96> */
#define GAFR3_U		0x40E00070  /* GPIO Alternate Function Select Register GPIO<127:112> */
#endif /* CONFIG_CPU_PXA27X || CONFIG_CPU_MONAHANS */

#ifdef CONFIG_CPU_MONAHANS
#define GSDR0		0x40E00400 /* Bit-wise Set of GPDR[31:0] */
#define GSDR1		0x40E00404 /* Bit-wise Set of GPDR[63:32] */
#define GSDR2		0x40E00408 /* Bit-wise Set of GPDR[95:64] */
#define GSDR3		0x40E0040C /* Bit-wise Set of GPDR[127:96] */

#define GCDR0		0x40E00420 /* Bit-wise Clear of GPDR[31:0] */
#define GCDR1		0x40E00424 /* Bit-wise Clear of GPDR[63:32] */
#define GCDR2		0x40E00428 /* Bit-wise Clear of GPDR[95:64] */
#define GCDR3		0x40E0042C /* Bit-wise Clear of GPDR[127:96] */

#define GSRER0		0x40E00440 /* Set Rising Edge Det. Enable [31:0] */
#define GSRER1		0x40E00444 /* Set Rising Edge Det. Enable [63:32] */
#define GSRER2		0x40E00448 /* Set Rising Edge Det. Enable [95:64] */
#define GSRER3		0x40E0044C /* Set Rising Edge Det. Enable [127:96] */

#define GCRER0		0x40E00460 /* Clear Rising Edge Det. Enable [31:0] */
#define GCRER1		0x40E00464 /* Clear Rising Edge Det. Enable [63:32] */
#define GCRER2		0x40E00468 /* Clear Rising Edge Det. Enable [95:64] */
#define GCRER3		0x40E0046C /* Clear Rising Edge Det. Enable[127:96] */

#define GSFER0		0x40E00480 /* Set Falling Edge Det. Enable [31:0] */
#define GSFER1		0x40E00484 /* Set Falling Edge Det. Enable [63:32] */
#define GSFER2		0x40E00488 /* Set Falling Edge Det. Enable [95:64] */
#define GSFER3		0x40E0048C /* Set Falling Edge Det. Enable[127:96] */

#define GCFER0		0x40E004A0 /* Clr Falling Edge Det. Enable [31:0] */
#define GCFER1		0x40E004A4 /* Clr Falling Edge Det. Enable [63:32] */
#define GCFER2		0x40E004A8 /* Clr Falling Edge Det. Enable [95:64] */
#define GCFER3		0x40E004AC /* Clr Falling Edge Det. Enable[127:96] */

#define GSDR(x)		(0x40E00400 | ((x) & 0x60) >> 3)
#define GCDR(x)		(0x40E00420 | ((x) & 0x60) >> 3)
#endif

#define _GPLR(x)	(0x40E00000 + (((x) & 0x60) >> 3))
#define _GPDR(x)	(0x40E0000C + (((x) & 0x60) >> 3))
#define _GPSR(x)	(0x40E00018 + (((x) & 0x60) >> 3))
#define _GPCR(x)	(0x40E00024 + (((x) & 0x60) >> 3))
#define _GRER(x)	(0x40E00030 + (((x) & 0x60) >> 3))
#define _GFER(x)	(0x40E0003C + (((x) & 0x60) >> 3))
#define _GEDR(x)	(0x40E00048 + (((x) & 0x60) >> 3))
#define _GAFR(x)	(0x40E00054 + (((x) & 0x70) >> 2))

#if defined(CONFIG_CPU_PXA27X) || defined(CONFIG_CPU_MONAHANS)
#define GPLR(x)		(((((x) & 0x7f) < 96) ? _GPLR(x) : GPLR3))
#define GPDR(x)		(((((x) & 0x7f) < 96) ? _GPDR(x) : GPDR3))
#define GPSR(x)		(((((x) & 0x7f) < 96) ? _GPSR(x) : GPSR3))
#define GPCR(x)		(((((x) & 0x7f) < 96) ? _GPCR(x) : GPCR3))
#define GRER(x)		(((((x) & 0x7f) < 96) ? _GRER(x) : GRER3))
#define GFER(x)		(((((x) & 0x7f) < 96) ? _GFER(x) : GFER3))
#define GEDR(x)		(((((x) & 0x7f) < 96) ? _GEDR(x) : GEDR3))
#define GAFR(x)		(((((x) & 0x7f) < 96) ? _GAFR(x) : \
			((((x) & 0x7f) < 112) ? GAFR3_L : GAFR3_U)))
#else
#define GPLR(x)		_GPLR(x)
#define GPDR(x)		_GPDR(x)
#define GPSR(x)		_GPSR(x)
#define GPCR(x)		_GPCR(x)
#define GRER(x)		_GRER(x)
#define GFER(x)		_GFER(x)
#define GEDR(x)		_GEDR(x)
#define GAFR(x)		_GAFR(x)
#endif

#define GPIO_bit(x)	(1 << ((x) & 0x1f))

/******************************************************************************/
/*
 * Multi-function Pin Registers:
 */
/* PXA320 */
#if defined(CONFIG_CPU_PXA320)
#define	DF_IO0		0x40e1024c
#define	DF_IO1		0x40e10254
#define	DF_IO2		0x40e1025c
#define	DF_IO3		0x40e10264
#define	DF_IO4		0x40e1026c
#define	DF_IO5		0x40e10274
#define	DF_IO6		0x40e1027c
#define	DF_IO7		0x40e10284
#define	DF_IO8		0x40e10250
#define	DF_IO9		0x40e10258
#define	DF_IO10		0x40e10260
#define	DF_IO11		0x40e10268
#define	DF_IO12		0x40e10270
#define	DF_IO13		0x40e10278
#define	DF_IO14		0x40e10280
#define	DF_IO15		0x40e10288
#define	DF_CLE_nOE	0x40e10204
#define	DF_ALE_nWE1	0x40e10208
#define	DF_ALE_nWE2	0x40e1021c
#define	DF_SCLK_E	0x40e10210
#define	DF_nCS0		0x40e10224
#define	DF_nCS1		0x40e10228
#define	nBE0		0x40e10214
#define	nBE1		0x40e10218
#define	nLUA		0x40e10234
#define	nLLA		0x40e10238
#define	DF_ADDR0	0x40e1023c
#define	DF_ADDR1	0x40e10240
#define	DF_ADDR2	0x40e10244
#define	DF_ADDR3	0x40e10248
#define	DF_INT_RnB	0x40e10220
#define	DF_nCS0		0x40e10224
#define	DF_nCS1		0x40e10228
#define	DF_nWE		0x40e1022c
#define	DF_nRE		0x40e10230

#define	nXCVREN		0x40e10138

#define	GPIO0		0x40e10124
#define	GPIO1		0x40e10128
#define	GPIO2		0x40e1012c
#define	GPIO3		0x40e10130
#define	GPIO4		0x40e10134
#define	GPIO5		0x40e1028c
#define	GPIO6		0x40e10290
#define	GPIO7		0x40e10294
#define	GPIO8		0x40e10298
#define	GPIO9		0x40e1029c
#define	GPIO10		0x40e10458
#define	GPIO11		0x40e102a0
#define	GPIO12		0x40e102a4
#define	GPIO13		0x40e102a8
#define	GPIO14		0x40e102ac
#define	GPIO15		0x40e102b0
#define	GPIO16		0x40e102b4
#define	GPIO17		0x40e102b8
#define	GPIO18		0x40e102bc
#define	GPIO19		0x40e102c0
#define	GPIO20		0x40e102c4
#define	GPIO21		0x40e102c8
#define	GPIO22		0x40e102cc
#define	GPIO23		0x40e102d0
#define	GPIO24		0x40e102d4
#define	GPIO25		0x40e102d8
#define	GPIO26		0x40e102dc

#define	GPIO27		0x40e10400
#define	GPIO28		0x40e10404
#define	GPIO29		0x40e10408
#define	GPIO30		0x40e1040c
#define	GPIO31		0x40e10410
#define	GPIO32		0x40e10414
#define	GPIO33		0x40e10418
#define	GPIO34		0x40e1041c
#define	GPIO35		0x40e10420
#define	GPIO36		0x40e10424
#define	GPIO37		0x40e10428
#define	GPIO38		0x40e1042c
#define	GPIO39		0x40e10430
#define	GPIO40		0x40e10434
#define	GPIO41		0x40e10438
#define	GPIO42		0x40e1043c
#define	GPIO43		0x40e10440
#define	GPIO44		0x40e10444
#define	GPIO45		0x40e10448
#define	GPIO46		0x40e1044c
#define	GPIO47		0x40e10450
#define	GPIO48		0x40e10454
#define	GPIO49		0x40e1045c
#define	GPIO50		0x40e10460
#define	GPIO51		0x40e10464
#define	GPIO52		0x40e10468
#define	GPIO53		0x40e1046c
#define	GPIO54		0x40e10470
#define	GPIO55		0x40e10474
#define	GPIO56		0x40e10478
#define	GPIO57		0x40e1047c
#define	GPIO58		0x40e10480
#define	GPIO59		0x40e10484
#define	GPIO60		0x40e10488
#define	GPIO61		0x40e1048c
#define	GPIO62		0x40e10490

#define	GPIO6_2		0x40e10494
#define	GPIO7_2		0x40e10498
#define	GPIO8_2		0x40e1049c
#define	GPIO9_2		0x40e104a0
#define	GPIO10_2	0x40e104a4
#define	GPIO11_2	0x40e104a8
#define	GPIO12_2	0x40e104ac
#define	GPIO13_2	0x40e104b0

#define	GPIO63		0x40e104b4
#define	GPIO64		0x40e104b8
#define	GPIO65		0x40e104bc
#define	GPIO66		0x40e104c0
#define	GPIO67		0x40e104c4
#define	GPIO68		0x40e104c8
#define	GPIO69		0x40e104cc
#define	GPIO70		0x40e104d0
#define	GPIO71		0x40e104d4
#define	GPIO72		0x40e104d8
#define	GPIO73		0x40e104dc

#define	GPIO14_2	0x40e104e0
#define	GPIO15_2	0x40e104e4
#define	GPIO16_2	0x40e104e8
#define	GPIO17_2	0x40e104ec

#define	GPIO74		0x40e104f0
#define	GPIO75		0x40e104f4
#define	GPIO76		0x40e104f8
#define	GPIO77		0x40e104fc
#define	GPIO78		0x40e10500
#define	GPIO79		0x40e10504
#define	GPIO80		0x40e10508
#define	GPIO81		0x40e1050c
#define	GPIO82		0x40e10510
#define	GPIO83		0x40e10514
#define	GPIO84		0x40e10518
#define	GPIO85		0x40e1051c
#define	GPIO86		0x40e10520
#define	GPIO87		0x40e10524
#define	GPIO88		0x40e10528
#define	GPIO89		0x40e1052c
#define	GPIO90		0x40e10530
#define	GPIO91		0x40e10534
#define	GPIO92		0x40e10538
#define	GPIO93		0x40e1053c
#define	GPIO94		0x40e10540
#define	GPIO95		0x40e10544
#define	GPIO96		0x40e10548
#define	GPIO97		0x40e1054c
#define	GPIO98		0x40e10550

#define	GPIO99		0x40e10600
#define	GPIO100		0x40e10604
#define	GPIO101		0x40e10608
#define	GPIO102		0x40e1060c
#define	GPIO103		0x40e10610
#define	GPIO104		0x40e10614
#define	GPIO105		0x40e10618
#define	GPIO106		0x40e1061c
#define	GPIO107		0x40e10620
#define	GPIO108		0x40e10624
#define	GPIO109		0x40e10628
#define	GPIO110		0x40e1062c
#define	GPIO111		0x40e10630
#define	GPIO112		0x40e10634

#define	GPIO113		0x40e10638
#define	GPIO114		0x40e1063c
#define	GPIO115		0x40e10640
#define	GPIO116		0x40e10644
#define	GPIO117		0x40e10648
#define	GPIO118		0x40e1064c
#define	GPIO119		0x40e10650
#define	GPIO120		0x40e10654
#define	GPIO121		0x40e10658
#define	GPIO122		0x40e1065c
#define	GPIO123		0x40e10660
#define	GPIO124		0x40e10664
#define	GPIO125		0x40e10668
#define	GPIO126		0x40e1066c
#define	GPIO127		0x40e10670

#define	GPIO0_2		0x40e10674
#define	GPIO1_2		0x40e10678
#define	GPIO2_2		0x40e1067c
#define	GPIO3_2		0x40e10680
#define	GPIO4_2		0x40e10684
#define	GPIO5_2		0x40e10688

/* PXA300 and PXA310 */
#elif	defined(CONFIG_CPU_PXA300) || defined(CONFIG_CPU_PXA310)
#define	DF_IO0		0x40e10220
#define	DF_IO1		0x40e10228
#define	DF_IO2		0x40e10230
#define	DF_IO3		0x40e10238
#define	DF_IO4		0x40e10258
#define	DF_IO5		0x40e10260
#define	DF_IO7		0x40e10270
#define	DF_IO6		0x40e10268
#define	DF_IO8		0x40e10224
#define	DF_IO9		0x40e1022c
#define	DF_IO10		0x40e10234
#define	DF_IO11		0x40e1023c
#define	DF_IO12		0x40e1025c
#define	DF_IO13		0x40e10264
#define	DF_IO14		0x40e1026c
#define	DF_IO15		0x40e10274
#define	DF_CLE_NOE	0x40e10240
#define	DF_ALE_nWE	0x40e1020c
#define	DF_SCLK_E	0x40e10250
#define	nCS0		0x40e100c4
#define	nCS1		0x40e100c0
#define	nBE0		0x40e10204
#define	nBE1		0x40e10208
#define	nLUA		0x40e10244
#define	nLLA		0x40e10254
#define	DF_ADDR0	0x40e10210
#define	DF_ADDR1	0x40e10214
#define	DF_ADDR2	0x40e10218
#define	DF_ADDR3	0x40e1021c
#define	DF_INT_RnB	0x40e100c8
#define	DF_nCS0		0x40e10248
#define	DF_nCS1		0x40e10278
#define	DF_nWE		0x40e100cc
#define	DF_nRE		0x40e10200

#define	GPIO0		0x40e100b4
#define	GPIO1		0x40e100b8
#define	GPIO2		0x40e100bc
#define	GPIO3		0x40e1027c
#define	GPIO4		0x40e10280

#define	GPIO5		0x40e10284
#define	GPIO6		0x40e10288
#define	GPIO7		0x40e1028c
#define	GPIO8		0x40e10290
#define	GPIO9		0x40e10294
#define	GPIO10		0x40e10298
#define	GPIO11		0x40e1029c
#define	GPIO12		0x40e102a0
#define	GPIO13		0x40e102a4
#define	GPIO14		0x40e102a8
#define	GPIO15		0x40e102ac
#define	GPIO16		0x40e102b0
#define	GPIO17		0x40e102b4
#define	GPIO18		0x40e102b8
#define	GPIO19		0x40e102bc
#define	GPIO20		0x40e102c0
#define	GPIO21		0x40e102c4
#define	GPIO22		0x40e102c8
#define	GPIO23		0x40e102cc
#define	GPIO24		0x40e102d0
#define	GPIO25		0x40e102d4
#define	GPIO26		0x40e102d8

#define	GPIO27		0x40e10400
#define	GPIO28		0x40e10404
#define	GPIO29		0x40e10408
#define	ULPI_STP	0x40e1040c
#define	ULPI_NXT	0x40e10410
#define	ULPI_DIR	0x40e10414
#define	GPIO30		0x40e10418
#define	GPIO31		0x40e1041c
#define	GPIO32		0x40e10420
#define	GPIO33		0x40e10424
#define	GPIO34		0x40e10428
#define	GPIO35		0x40e1042c
#define	GPIO36		0x40e10430
#define	GPIO37		0x40e10434
#define	GPIO38		0x40e10438
#define	GPIO39		0x40e1043c
#define	GPIO40		0x40e10440
#define	GPIO41		0x40e10444
#define	GPIO42		0x40e10448
#define	GPIO43		0x40e1044c
#define	GPIO44		0x40e10450
#define	GPIO45		0x40e10454
#define	GPIO46		0x40e10458
#define	GPIO47		0x40e1045c
#define	GPIO48		0x40e10460

#define	GPIO49		0x40e10464
#define	GPIO50		0x40e10468
#define	GPIO51		0x40e1046c
#define	GPIO52		0x40e10470
#define	GPIO53		0x40e10474
#define	GPIO54		0x40e10478
#define	GPIO55		0x40e1047c
#define	GPIO56		0x40e10480
#define	GPIO57		0x40e10484
#define	GPIO58		0x40e10488
#define	GPIO59		0x40e1048c
#define	GPIO60		0x40e10490
#define	GPIO61		0x40e10494
#define	GPIO62		0x40e10498
#define	GPIO63		0x40e1049c
#define	GPIO64		0x40e104a0
#define	GPIO65		0x40e104a4
#define	GPIO66		0x40e104a8
#define	GPIO67		0x40e104ac
#define	GPIO68		0x40e104b0
#define	GPIO69		0x40e104b4
#define	GPIO70		0x40e104b8
#define	GPIO71		0x40e104bc
#define	GPIO72		0x40e104c0
#define	GPIO73		0x40e104c4
#define	GPIO74		0x40e104c8
#define	GPIO75		0x40e104cc
#define	GPIO76		0x40e104d0
#define	GPIO77		0x40e104d4
#define	GPIO78		0x40e104d8
#define	GPIO79		0x40e104dc
#define	GPIO80		0x40e104e0
#define	GPIO81		0x40e104e4
#define	GPIO82		0x40e104e8
#define	GPIO83		0x40e104ec
#define	GPIO84		0x40e104f0
#define	GPIO85		0x40e104f4
#define	GPIO86		0x40e104f8
#define	GPIO87		0x40e104fc
#define	GPIO88		0x40e10500
#define	GPIO89		0x40e10504
#define	GPIO90		0x40e10508
#define	GPIO91		0x40e1050c
#define	GPIO92		0x40e10510
#define	GPIO93		0x40e10514
#define	GPIO94		0x40e10518
#define	GPIO95		0x40e1051c
#define	GPIO96		0x40e10520
#define	GPIO97		0x40e10524
#define	GPIO98		0x40e10528

#define	GPIO99		0x40e10600
#define	GPIO100		0x40e10604
#define	GPIO101		0x40e10608
#define	GPIO102		0x40e1060c
#define	GPIO103		0x40e10610
#define	GPIO104		0x40e10614
#define	GPIO105		0x40e10618
#define	GPIO106		0x40e1061c
#define	GPIO107		0x40e10620
#define	GPIO108		0x40e10624
#define	GPIO109		0x40e10628
#define	GPIO110		0x40e1062c
#define	GPIO111		0x40e10630
#define	GPIO112		0x40e10634

#define	GPIO113		0x40e10638
#define	GPIO114		0x40e1063c
#define	GPIO115		0x40e10640
#define	GPIO116		0x40e10644
#define	GPIO117		0x40e10648
#define	GPIO118		0x40e1064c
#define	GPIO119		0x40e10650
#define	GPIO120		0x40e10654
#define	GPIO121		0x40e10658
#define	GPIO122		0x40e1065c
#define	GPIO123		0x40e10660
#define	GPIO124		0x40e10664
#define	GPIO125		0x40e10668
#define	GPIO126		0x40e1066c
#define	GPIO127		0x40e10670

#define	GPIO0_2		0x40e10674
#define	GPIO1_2		0x40e10678
#define	GPIO2_2		0x40e102dc
#define	GPIO3_2		0x40e102e0
#define	GPIO4_2		0x40e102e4
#define	GPIO5_2		0x40e102e8
#define	GPIO6_2		0x40e102ec

#ifndef	CONFIG_CPU_PXA300	/* PXA310 only */
#define	GPIO7_2		0x40e1052c
#define	GPIO8_2		0x40e10530
#define	GPIO9_2		0x40e10534
#define	GPIO10_2	0x40e10538
#endif
#endif

#ifdef CONFIG_CPU_MONAHANS
/* MFPR Bit Definitions, see 4-10, Vol. 1 */
#define PULL_SEL	0x8000
#define PULLUP_EN	0x4000
#define PULLDOWN_EN	0x2000

#define DRIVE_FAST_1mA	0x0
#define DRIVE_FAST_2mA	0x400
#define DRIVE_FAST_3mA	0x800
#define DRIVE_FAST_4mA	0xC00
#define DRIVE_SLOW_6mA	0x1000
#define DRIVE_FAST_6mA	0x1400
#define DRIVE_SLOW_10mA	0x1800
#define DRIVE_FAST_10mA	0x1C00

#define SLEEP_SEL	0x200
#define SLEEP_DATA	0x100
#define SLEEP_OE_N	0x80
#define EDGE_CLEAR	0x40
#define EDGE_FALL_EN	0x20
#define EDGE_RISE_EN	0x10

#define AF_SEL_0	0x0	/* Alternate function 0 (reset state) */
#define AF_SEL_1	0x1	/* Alternate function 1 */
#define AF_SEL_2	0x2	/* Alternate function 2 */
#define AF_SEL_3	0x3	/* Alternate function 3 */
#define AF_SEL_4	0x4	/* Alternate function 4 */
#define AF_SEL_5	0x5	/* Alternate function 5 */
#define AF_SEL_6	0x6	/* Alternate function 6 */
#define AF_SEL_7	0x7	/* Alternate function 7 */

#endif /* CONFIG_CPU_MONAHANS */

/* GPIO alternate function assignments */

#define GPIO1_RST		1	/* reset */
#define GPIO6_MMCCLK		6	/* MMC Clock */
#define GPIO8_48MHz		7	/* 48 MHz clock output */
#define GPIO8_MMCCS0		8	/* MMC Chip Select 0 */
#define GPIO9_MMCCS1		9	/* MMC Chip Select 1 */
#define GPIO10_RTCCLK		10	/* real time clock (1 Hz) */
#define GPIO11_3_6MHz		11	/* 3.6 MHz oscillator out */
#define GPIO12_32KHz		12	/* 32 kHz out */
#define GPIO13_MBGNT		13	/* memory controller grant */
#define GPIO14_MBREQ		14	/* alternate bus master request */
#define GPIO15_nCS_1		15	/* chip select 1 */
#define GPIO16_PWM0		16	/* PWM0 output */
#define GPIO17_PWM1		17	/* PWM1 output */
#define GPIO18_RDY		18	/* Ext. Bus Ready */
#define GPIO19_DREQ1		19	/* External DMA Request */
#define GPIO20_DREQ0		20	/* External DMA Request */
#define GPIO23_SCLK		23	/* SSP clock */
#define GPIO24_SFRM		24	/* SSP Frame */
#define GPIO25_STXD		25	/* SSP transmit */
#define GPIO26_SRXD		26	/* SSP receive */
#define GPIO27_SEXTCLK		27	/* SSP ext_clk */
#define GPIO28_BITCLK		28	/* AC97/I2S bit_clk */
#define GPIO29_SDATA_IN		29	/* AC97 Sdata_in0 / I2S Sdata_in */
#define GPIO30_SDATA_OUT	30	/* AC97/I2S Sdata_out */
#define GPIO31_SYNC		31	/* AC97/I2S sync */
#define GPIO32_SDATA_IN1	32	/* AC97 Sdata_in1 */
#define GPIO33_nCS_5		33	/* chip select 5 */
#define GPIO34_FFRXD		34	/* FFUART receive */
#define GPIO34_MMCCS0		34	/* MMC Chip Select 0 */
#define GPIO35_FFCTS		35	/* FFUART Clear to send */
#define GPIO36_FFDCD		36	/* FFUART Data carrier detect */
#define GPIO37_FFDSR		37	/* FFUART data set ready */
#define GPIO38_FFRI		38	/* FFUART Ring Indicator */
#define GPIO39_MMCCS1		39	/* MMC Chip Select 1 */
#define GPIO39_FFTXD		39	/* FFUART transmit data */
#define GPIO40_FFDTR		40	/* FFUART data terminal Ready */
#define GPIO41_FFRTS		41	/* FFUART request to send */
#define GPIO42_BTRXD		42	/* BTUART receive data */
#define GPIO43_BTTXD		43	/* BTUART transmit data */
#define GPIO44_BTCTS		44	/* BTUART clear to send */
#define GPIO45_BTRTS		45	/* BTUART request to send */
#define GPIO46_ICPRXD		46	/* ICP receive data */
#define GPIO46_STRXD		46	/* STD_UART receive data */
#define GPIO47_ICPTXD		47	/* ICP transmit data */
#define GPIO47_STTXD		47	/* STD_UART transmit data */
#define GPIO48_nPOE		48	/* Output Enable for Card Space */
#define GPIO49_nPWE		49	/* Write Enable for Card Space */
#define GPIO50_nPIOR		50	/* I/O Read for Card Space */
#define GPIO51_nPIOW		51	/* I/O Write for Card Space */
#define GPIO52_nPCE_1		52	/* Card Enable for Card Space */
#define GPIO53_nPCE_2		53	/* Card Enable for Card Space */
#define GPIO53_MMCCLK		53	/* MMC Clock */
#define GPIO54_MMCCLK		54	/* MMC Clock */
#define GPIO54_pSKTSEL		54	/* Socket Select for Card Space */
#define GPIO55_nPREG		55	/* Card Address bit 26 */
#define GPIO56_nPWAIT		56	/* Wait signal for Card Space */
#define GPIO57_nIOIS16		57	/* Bus Width select for I/O Card Space */
#define GPIO58_LDD_0		58	/* LCD data pin 0 */
#define GPIO59_LDD_1		59	/* LCD data pin 1 */
#define GPIO60_LDD_2		60	/* LCD data pin 2 */
#define GPIO61_LDD_3		61	/* LCD data pin 3 */
#define GPIO62_LDD_4		62	/* LCD data pin 4 */
#define GPIO63_LDD_5		63	/* LCD data pin 5 */
#define GPIO64_LDD_6		64	/* LCD data pin 6 */
#define GPIO65_LDD_7		65	/* LCD data pin 7 */
#define GPIO66_LDD_8		66	/* LCD data pin 8 */
#define GPIO66_MBREQ		66	/* alternate bus master req */
#define GPIO67_LDD_9		67	/* LCD data pin 9 */
#define GPIO67_MMCCS0		67	/* MMC Chip Select 0 */
#define GPIO68_LDD_10		68	/* LCD data pin 10 */
#define GPIO68_MMCCS1		68	/* MMC Chip Select 1 */
#define GPIO69_LDD_11		69	/* LCD data pin 11 */
#define GPIO69_MMCCLK		69	/* MMC_CLK */
#define GPIO70_LDD_12		70	/* LCD data pin 12 */
#define GPIO70_RTCCLK		70	/* Real Time clock (1 Hz) */
#define GPIO71_LDD_13		71	/* LCD data pin 13 */
#define GPIO71_3_6MHz		71	/* 3.6 MHz Oscillator clock */
#define GPIO72_LDD_14		72	/* LCD data pin 14 */
#define GPIO72_32kHz		72	/* 32 kHz clock */
#define GPIO73_LDD_15		73	/* LCD data pin 15 */
#define GPIO73_MBGNT		73	/* Memory controller grant */
#define GPIO74_LCD_FCLK		74	/* LCD Frame clock */
#define GPIO75_LCD_LCLK		75	/* LCD line clock */
#define GPIO76_LCD_PCLK		76	/* LCD Pixel clock */
#define GPIO77_LCD_ACBIAS	77	/* LCD AC Bias */
#define GPIO78_nCS_2		78	/* chip select 2 */
#define GPIO79_nCS_3		79	/* chip select 3 */
#define GPIO80_nCS_4		80	/* chip select 4 */

/* GPIO alternate function mode & direction */

#define GPIO_IN			0x000
#define GPIO_OUT		0x080
#define GPIO_ALT_FN_1_IN	0x100
#define GPIO_ALT_FN_1_OUT	0x180
#define GPIO_ALT_FN_2_IN	0x200
#define GPIO_ALT_FN_2_OUT	0x280
#define GPIO_ALT_FN_3_IN	0x300
#define GPIO_ALT_FN_3_OUT	0x380
#define GPIO_MD_MASK_NR		0x07f
#define GPIO_MD_MASK_DIR	0x080
#define GPIO_MD_MASK_FN		0x300

#define GPIO1_RTS_MD		( 1 | GPIO_ALT_FN_1_IN)
#define GPIO6_MMCCLK_MD		( 6 | GPIO_ALT_FN_1_OUT)
#define GPIO8_48MHz_MD		( 8 | GPIO_ALT_FN_1_OUT)
#define GPIO8_MMCCS0_MD		( 8 | GPIO_ALT_FN_1_OUT)
#define GPIO9_MMCCS1_MD		( 9 | GPIO_ALT_FN_1_OUT)
#define GPIO10_RTCCLK_MD	(10 | GPIO_ALT_FN_1_OUT)
#define GPIO11_3_6MHz_MD	(11 | GPIO_ALT_FN_1_OUT)
#define GPIO12_32KHz_MD		(12 | GPIO_ALT_FN_1_OUT)
#define GPIO13_MBGNT_MD		(13 | GPIO_ALT_FN_2_OUT)
#define GPIO14_MBREQ_MD		(14 | GPIO_ALT_FN_1_IN)
#define GPIO15_nCS_1_MD		(15 | GPIO_ALT_FN_2_OUT)
#define GPIO16_PWM0_MD		(16 | GPIO_ALT_FN_2_OUT)
#define GPIO17_PWM1_MD		(17 | GPIO_ALT_FN_2_OUT)
#define GPIO18_RDY_MD		(18 | GPIO_ALT_FN_1_IN)
#define GPIO19_DREQ1_MD		(19 | GPIO_ALT_FN_1_IN)
#define GPIO20_DREQ0_MD		(20 | GPIO_ALT_FN_1_IN)
#define GPIO23_SCLK_md		(23 | GPIO_ALT_FN_2_OUT)
#define GPIO24_SFRM_MD		(24 | GPIO_ALT_FN_2_OUT)
#define GPIO25_STXD_MD		(25 | GPIO_ALT_FN_2_OUT)
#define GPIO26_SRXD_MD		(26 | GPIO_ALT_FN_1_IN)
#define GPIO27_SEXTCLK_MD	(27 | GPIO_ALT_FN_1_IN)
#define GPIO28_BITCLK_AC97_MD	(28 | GPIO_ALT_FN_1_IN)
#define GPIO28_BITCLK_I2S_MD	(28 | GPIO_ALT_FN_2_IN)
#define GPIO29_SDATA_IN_AC97_MD (29 | GPIO_ALT_FN_1_IN)
#define GPIO29_SDATA_IN_I2S_MD	(29 | GPIO_ALT_FN_2_IN)
#define GPIO30_SDATA_OUT_AC97_MD	(30 | GPIO_ALT_FN_2_OUT)
#define GPIO30_SDATA_OUT_I2S_MD (30 | GPIO_ALT_FN_1_OUT)
#define GPIO31_SYNC_AC97_MD	(31 | GPIO_ALT_FN_2_OUT)
#define GPIO31_SYNC_I2S_MD	(31 | GPIO_ALT_FN_1_OUT)
#define GPIO32_SDATA_IN1_AC97_MD	(32 | GPIO_ALT_FN_1_IN)
#define GPIO33_nCS_5_MD		(33 | GPIO_ALT_FN_2_OUT)
#define GPIO34_FFRXD_MD		(34 | GPIO_ALT_FN_1_IN)
#define GPIO34_MMCCS0_MD	(34 | GPIO_ALT_FN_2_OUT)
#define GPIO35_FFCTS_MD		(35 | GPIO_ALT_FN_1_IN)
#define GPIO36_FFDCD_MD		(36 | GPIO_ALT_FN_1_IN)
#define GPIO37_FFDSR_MD		(37 | GPIO_ALT_FN_1_IN)
#define GPIO38_FFRI_MD		(38 | GPIO_ALT_FN_1_IN)
#define GPIO39_MMCCS1_MD	(39 | GPIO_ALT_FN_1_OUT)
#define GPIO39_FFTXD_MD		(39 | GPIO_ALT_FN_2_OUT)
#define GPIO40_FFDTR_MD		(40 | GPIO_ALT_FN_2_OUT)
#define GPIO41_FFRTS_MD		(41 | GPIO_ALT_FN_2_OUT)
#define GPIO42_BTRXD_MD		(42 | GPIO_ALT_FN_1_IN)
#define GPIO43_BTTXD_MD		(43 | GPIO_ALT_FN_2_OUT)
#define GPIO44_BTCTS_MD		(44 | GPIO_ALT_FN_1_IN)
#define GPIO45_BTRTS_MD		(45 | GPIO_ALT_FN_2_OUT)
#define GPIO46_ICPRXD_MD	(46 | GPIO_ALT_FN_1_IN)
#define GPIO46_STRXD_MD		(46 | GPIO_ALT_FN_2_IN)
#define GPIO47_ICPTXD_MD	(47 | GPIO_ALT_FN_2_OUT)
#define GPIO47_STTXD_MD		(47 | GPIO_ALT_FN_1_OUT)
#define GPIO48_nPOE_MD		(48 | GPIO_ALT_FN_2_OUT)
#define GPIO49_nPWE_MD		(49 | GPIO_ALT_FN_2_OUT)
#define GPIO50_nPIOR_MD		(50 | GPIO_ALT_FN_2_OUT)
#define GPIO51_nPIOW_MD		(51 | GPIO_ALT_FN_2_OUT)
#define GPIO52_nPCE_1_MD	(52 | GPIO_ALT_FN_2_OUT)
#define GPIO53_nPCE_2_MD	(53 | GPIO_ALT_FN_2_OUT)
#define GPIO53_MMCCLK_MD	(53 | GPIO_ALT_FN_1_OUT)
#define GPIO54_MMCCLK_MD	(54 | GPIO_ALT_FN_1_OUT)
#define GPIO54_pSKTSEL_MD	(54 | GPIO_ALT_FN_2_OUT)
#define GPIO55_nPREG_MD		(55 | GPIO_ALT_FN_2_OUT)
#define GPIO56_nPWAIT_MD	(56 | GPIO_ALT_FN_1_IN)
#define GPIO57_nIOIS16_MD	(57 | GPIO_ALT_FN_1_IN)
#define GPIO58_LDD_0_MD		(58 | GPIO_ALT_FN_2_OUT)
#define GPIO59_LDD_1_MD		(59 | GPIO_ALT_FN_2_OUT)
#define GPIO60_LDD_2_MD		(60 | GPIO_ALT_FN_2_OUT)
#define GPIO61_LDD_3_MD		(61 | GPIO_ALT_FN_2_OUT)
#define GPIO62_LDD_4_MD		(62 | GPIO_ALT_FN_2_OUT)
#define GPIO63_LDD_5_MD		(63 | GPIO_ALT_FN_2_OUT)
#define GPIO64_LDD_6_MD		(64 | GPIO_ALT_FN_2_OUT)
#define GPIO65_LDD_7_MD		(65 | GPIO_ALT_FN_2_OUT)
#define GPIO66_LDD_8_MD		(66 | GPIO_ALT_FN_2_OUT)
#define GPIO66_MBREQ_MD		(66 | GPIO_ALT_FN_1_IN)
#define GPIO67_LDD_9_MD		(67 | GPIO_ALT_FN_2_OUT)
#define GPIO67_MMCCS0_MD	(67 | GPIO_ALT_FN_1_OUT)
#define GPIO68_LDD_10_MD	(68 | GPIO_ALT_FN_2_OUT)
#define GPIO68_MMCCS1_MD	(68 | GPIO_ALT_FN_1_OUT)
#define GPIO69_LDD_11_MD	(69 | GPIO_ALT_FN_2_OUT)
#define GPIO69_MMCCLK_MD	(69 | GPIO_ALT_FN_1_OUT)
#define GPIO70_LDD_12_MD	(70 | GPIO_ALT_FN_2_OUT)
#define GPIO70_RTCCLK_MD	(70 | GPIO_ALT_FN_1_OUT)
#define GPIO71_LDD_13_MD	(71 | GPIO_ALT_FN_2_OUT)
#define GPIO71_3_6MHz_MD	(71 | GPIO_ALT_FN_1_OUT)
#define GPIO72_LDD_14_MD	(72 | GPIO_ALT_FN_2_OUT)
#define GPIO72_32kHz_MD		(72 | GPIO_ALT_FN_1_OUT)
#define GPIO73_LDD_15_MD	(73 | GPIO_ALT_FN_2_OUT)
#define GPIO73_MBGNT_MD		(73 | GPIO_ALT_FN_1_OUT)
#define GPIO74_LCD_FCLK_MD	(74 | GPIO_ALT_FN_2_OUT)
#define GPIO75_LCD_LCLK_MD	(75 | GPIO_ALT_FN_2_OUT)
#define GPIO76_LCD_PCLK_MD	(76 | GPIO_ALT_FN_2_OUT)
#define GPIO77_LCD_ACBIAS_MD	(77 | GPIO_ALT_FN_2_OUT)
#define GPIO78_nCS_2_MD		(78 | GPIO_ALT_FN_2_OUT)
#define GPIO79_nCS_3_MD		(79 | GPIO_ALT_FN_2_OUT)
#define GPIO80_nCS_4_MD		(80 | GPIO_ALT_FN_2_OUT)

#define GPIO117_SCL		(117 | GPIO_ALT_FN_1_OUT)
#define GPIO118_SDA		(118 | GPIO_ALT_FN_1_OUT)

/*
 * Power Manager
 */
#ifdef CONFIG_CPU_MONAHANS

#define ASCR		0x40F40000  /* Application Subsystem Power Status/Control Register */
#define ARSR		0x40F40004  /* Application Subsystem Reset Status Register */
#define AD3ER		0x40F40008  /* Application Subsystem D3 state Wakeup Enable Register */
#define AD3SR		0x40F4000C  /* Application Subsystem D3 state Wakeup Status Register */
#define AD2D0ER		0x40F40010  /* Application Subsystem D2 to D0 state Wakeup Enable Register */
#define AD2D0SR		0x40F40014  /* Application Subsystem D2 to D0 state Wakeup Status Register */
#define AD2D1ER		0x40F40018  /* Application Subsystem D2 to D1 state Wakeup Enable Register */
#define AD2D1SR		0x40F4001C  /* Application Subsystem D2 to D1 state Wakeup Status Register */
#define AD1D0ER		0x40F40020  /* Application Subsystem D1 to D0 state Wakeup Enable Register */
#define AD1D0SR		0x40F40024  /* Application Subsystem D1 to D0 state Wakeup Status Register */
#define ASDCNT		0x40F40028  /* Application Subsystem SRAM Drowsy Count Register */
#define AD3R		0x40F40030  /* Application Subsystem D3 State Configuration Register */
#define AD2R		0x40F40034  /* Application Subsystem D2 State Configuration Register */
#define AD1R		0x40F40038  /* Application Subsystem D1 State Configuration Register */

#define PMCR		0x40F50000  /* Power Manager Control Register */
#define PSR		0x40F50004  /* Power Manager S2 Status Register */
#define PSPR		0x40F50008  /* Power Manager Scratch Pad Register */
#define PCFR		0x40F5000C  /* Power Manager General Configuration Register */
#define PWER		0x40F50010  /* Power Manager Wake-up Enable Register */
#define PWSR		0x40F50014  /* Power Manager Wake-up Status Register */
#define PECR		0x40F50018  /* Power Manager EXT_WAKEUP[1:0] Control Register */
#define DCDCSR		0x40F50080  /* DC-DC Controller Status Register */
#define PVCR		0x40F50100  /* Power Manager Voltage Change Control Register */
#define    PCMD(x) (0x40F50110 + x*4)
#define    PCMD0   (0x40F50110 + 0 * 4)
#define    PCMD1   (0x40F50110 + 1 * 4)
#define    PCMD2   (0x40F50110 + 2 * 4)
#define    PCMD3   (0x40F50110 + 3 * 4)
#define    PCMD4   (0x40F50110 + 4 * 4)
#define    PCMD5   (0x40F50110 + 5 * 4)
#define    PCMD6   (0x40F50110 + 6 * 4)
#define    PCMD7   (0x40F50110 + 7 * 4)
#define    PCMD8   (0x40F50110 + 8 * 4)
#define    PCMD9   (0x40F50110 + 9 * 4)
#define    PCMD10  (0x40F50110 + 10 * 4)
#define    PCMD11  (0x40F50110 + 11 * 4)
#define    PCMD12  (0x40F50110 + 12 * 4)
#define    PCMD13  (0x40F50110 + 13 * 4)
#define    PCMD14  (0x40F50110 + 14 * 4)
#define    PCMD15  (0x40F50110 + 15 * 4)
#define    PCMD16  (0x40F50110 + 16 * 4)
#define    PCMD17  (0x40F50110 + 17 * 4)
#define    PCMD18  (0x40F50110 + 18 * 4)
#define    PCMD19  (0x40F50110 + 19 * 4)
#define    PCMD20  (0x40F50110 + 20 * 4)
#define    PCMD21  (0x40F50110 + 21 * 4)
#define    PCMD22  (0x40F50110 + 22 * 4)
#define    PCMD23  (0x40F50110 + 23 * 4)
#define    PCMD24  (0x40F50110 + 24 * 4)
#define    PCMD25  (0x40F50110 + 25 * 4)
#define    PCMD26  (0x40F50110 + 26 * 4)
#define    PCMD27  (0x40F50110 + 27 * 4)
#define    PCMD28  (0x40F50110 + 28 * 4)
#define    PCMD29  (0x40F50110 + 29 * 4)
#define    PCMD30  (0x40F50110 + 30 * 4)
#define    PCMD31  (0x40F50110 + 31 * 4)

#define    PCMD_MBC    (1<<12)
#define    PCMD_DCE    (1<<11)
#define    PCMD_LC     (1<<10)
#define    PCMD_SQC    (3<<8)  /* only 00 and 01 are valid */

#define PVCR_FVC                   (0x1 << 28)
#define PVCR_VCSA                  (0x1<<14)
#define PVCR_CommandDelay          (0xf80)
#define PVCR_ReadPointer           0x01f00000
#define PVCR_SlaveAddress          (0x7f)

#else /* ifdef CONFIG_CPU_MONAHANS */

#define PMCR		0x40F00000  /* Power Manager Control Register */
#define PSSR		0x40F00004  /* Power Manager Sleep Status Register */
#define PSPR		0x40F00008  /* Power Manager Scratch Pad Register */
#define PWER		0x40F0000C  /* Power Manager Wake-up Enable Register */
#define PRER		0x40F00010  /* Power Manager GPIO Rising-Edge Detect Enable Register */
#define PFER		0x40F00014  /* Power Manager GPIO Falling-Edge Detect Enable Register */
#define PEDR		0x40F00018  /* Power Manager GPIO Edge Detect Status Register */
#define PCFR		0x40F0001C  /* Power Manager General Configuration Register */
#define PGSR0		0x40F00020  /* Power Manager GPIO Sleep State Register for GP[31-0] */
#define PGSR1		0x40F00024  /* Power Manager GPIO Sleep State Register for GP[63-32] */
#define PGSR2		0x40F00028  /* Power Manager GPIO Sleep State Register for GP[84-64] */
#define PGSR3		0x40F0002C  /* Power Manager GPIO Sleep State Register for GP[118-96] */
#define RCSR		0x40F00030  /* Reset Controller Status Register */

#define	   PSLR	   0x40F00034	/* Power Manager Sleep Config Register */
#define	   PSTR	   0x40F00038	/* Power Manager Standby Config Register */
#define	   PSNR	   0x40F0003C	/* Power Manager Sense Config Register */
#define	   PVCR	   0x40F00040	/* Power Manager VoltageControl Register */
#define	   PKWR	   0x40F00050	/* Power Manager KB Wake-up Enable Reg */
#define	   PKSR	   0x40F00054	/* Power Manager KB Level-Detect Register */
#define	   PCMD(x) (0x40F00080 + x*4)
#define	   PCMD0   (0x40F00080 + 0 * 4)
#define	   PCMD1   (0x40F00080 + 1 * 4)
#define	   PCMD2   (0x40F00080 + 2 * 4)
#define	   PCMD3   (0x40F00080 + 3 * 4)
#define	   PCMD4   (0x40F00080 + 4 * 4)
#define	   PCMD5   (0x40F00080 + 5 * 4)
#define	   PCMD6   (0x40F00080 + 6 * 4)
#define	   PCMD7   (0x40F00080 + 7 * 4)
#define	   PCMD8   (0x40F00080 + 8 * 4)
#define	   PCMD9   (0x40F00080 + 9 * 4)
#define	   PCMD10  (0x40F00080 + 10 * 4)
#define	   PCMD11  (0x40F00080 + 11 * 4)
#define	   PCMD12  (0x40F00080 + 12 * 4)
#define	   PCMD13  (0x40F00080 + 13 * 4)
#define	   PCMD14  (0x40F00080 + 14 * 4)
#define	   PCMD15  (0x40F00080 + 15 * 4)
#define	   PCMD16  (0x40F00080 + 16 * 4)
#define	   PCMD17  (0x40F00080 + 17 * 4)
#define	   PCMD18  (0x40F00080 + 18 * 4)
#define	   PCMD19  (0x40F00080 + 19 * 4)
#define	   PCMD20  (0x40F00080 + 20 * 4)
#define	   PCMD21  (0x40F00080 + 21 * 4)
#define	   PCMD22  (0x40F00080 + 22 * 4)
#define	   PCMD23  (0x40F00080 + 23 * 4)
#define	   PCMD24  (0x40F00080 + 24 * 4)
#define	   PCMD25  (0x40F00080 + 25 * 4)
#define	   PCMD26  (0x40F00080 + 26 * 4)
#define	   PCMD27  (0x40F00080 + 27 * 4)
#define	   PCMD28  (0x40F00080 + 28 * 4)
#define	   PCMD29  (0x40F00080 + 29 * 4)
#define	   PCMD30  (0x40F00080 + 30 * 4)
#define	   PCMD31  (0x40F00080 + 31 * 4)

#define	   PCMD_MBC    (1<<12)
#define	   PCMD_DCE    (1<<11)
#define	   PCMD_LC     (1<<10)
/* FIXME:  PCMD_SQC need be checked.   */
#define	   PCMD_SQC    (3<<8)  /* currently only bit 8 is changerable, */
				/* bit 9 should be 0 all day. */
#define PVCR_VCSA		   (0x1<<14)
#define PVCR_CommandDelay	   (0xf80)
/* define MACRO for Power Manager General Configuration Register (PCFR) */
#define PCFR_FVC		   (0x1 << 10)
#define PCFR_PI2C_EN		   (0x1 << 6)

#define PSSR_OTGPH	(1 << 6)	/* OTG Peripheral control Hold */
#define PSSR_RDH	(1 << 5)	/* Read Disable Hold */
#define PSSR_PH		(1 << 4)	/* Peripheral Control Hold */
#define PSSR_VFS	(1 << 2)	/* VDD Fault Status */
#define PSSR_BFS	(1 << 1)	/* Battery Fault Status */
#define PSSR_SSS	(1 << 0)	/* Software Sleep Status */

#define PCFR_DS		(1 << 3)	/* Deep Sleep Mode */
#define PCFR_FS		(1 << 2)	/* Float Static Chip Selects */
#define PCFR_FP		(1 << 1)	/* Float PCMCIA controls */
#define PCFR_OPDE	(1 << 0)	/* 3.6864 MHz oscillator power-down enable */

#define RCSR_GPR	(1 << 3)	/* GPIO Reset */
#define RCSR_SMR	(1 << 2)	/* Sleep Mode */
#define RCSR_WDR	(1 << 1)	/* Watchdog Reset */
#define RCSR_HWR	(1 << 0)	/* Hardware Reset */

#endif /* CONFIG_CPU_MONAHANS */

/*
 * SSP Serial Port Registers
 */
#define SSCR0		0x41000000  /* SSP Control Register 0 */
#define SSCR1		0x41000004  /* SSP Control Register 1 */
#define SSSR		0x41000008  /* SSP Status Register */
#define SSITR		0x4100000C  /* SSP Interrupt Test Register */
#define SSDR		0x41000010  /* (Write / Read) SSP Data Write Register/SSP Data Read Register */

/*
 * MultiMediaCard (MMC) controller
 */
#define MMC_STRPCL	0x41100000  /* Control to start and stop MMC clock */
#define MMC_STAT	0x41100004  /* MMC Status Register (read only) */
#define MMC_CLKRT	0x41100008  /* MMC clock rate */
#define MMC_SPI		0x4110000c  /* SPI mode control bits */
#define MMC_CMDAT	0x41100010  /* Command/response/data sequence control */
#define MMC_RESTO	0x41100014  /* Expected response time out */
#define MMC_RDTO	0x41100018  /* Expected data read time out */
#define MMC_BLKLEN	0x4110001c  /* Block length of data transaction */
#define MMC_NOB		0x41100020  /* Number of blocks, for block mode */
#define MMC_PRTBUF	0x41100024  /* Partial MMC_TXFIFO FIFO written */
#define MMC_I_MASK	0x41100028  /* Interrupt Mask */
#define MMC_I_REG	0x4110002c  /* Interrupt Register (read only) */
#define MMC_CMD		0x41100030  /* Index of current command */
#define MMC_ARGH	0x41100034  /* MSW part of the current command argument */
#define MMC_ARGL	0x41100038  /* LSW part of the current command argument */
#define MMC_RES		0x4110003c  /* Response FIFO (read only) */
#define MMC_RXFIFO	0x41100040  /* Receive FIFO (read only) */
#define MMC_TXFIFO	0x41100044  /* Transmit FIFO (write only) */


/*
 * LCD
 */
#define LCCR0		0x44000000  /* LCD Controller Control Register 0 */
#define LCCR1		0x44000004  /* LCD Controller Control Register 1 */
#define LCCR2		0x44000008  /* LCD Controller Control Register 2 */
#define LCCR3		0x4400000C  /* LCD Controller Control Register 3 */
#define DFBR0		0x44000020  /* DMA Channel 0 Frame Branch Register */
#define DFBR1		0x44000024  /* DMA Channel 1 Frame Branch Register */
#define LCSR0		0x44000038  /* LCD Controller Status Register */
#define LCSR1		0x44000034  /* LCD Controller Status Register */
#define LIIDR		0x4400003C  /* LCD Controller Interrupt ID Register */
#define TMEDRGBR	0x44000040  /* TMED RGB Seed Register */
#define TMEDCR		0x44000044  /* TMED Control Register */

#define FDADR0		0x44000200  /* DMA Channel 0 Frame Descriptor Address Register */
#define FSADR0		0x44000204  /* DMA Channel 0 Frame Source Address Register */
#define FIDR0		0x44000208  /* DMA Channel 0 Frame ID Register */
#define LDCMD0		0x4400020C  /* DMA Channel 0 Command Register */
#define FDADR1		0x44000210  /* DMA Channel 1 Frame Descriptor Address Register */
#define FSADR1		0x44000214  /* DMA Channel 1 Frame Source Address Register */
#define FIDR1		0x44000218  /* DMA Channel 1 Frame ID Register */
#define LDCMD1		0x4400021C  /* DMA Channel 1 Command Register */

#define LCCR0_ENB	(1 << 0)	/* LCD Controller enable */
#define LCCR0_CMS	(1 << 1)	/* Color = 0, Monochrome = 1 */
#define LCCR0_SDS	(1 << 2)	/* Single Panel = 0, Dual Panel = 1 */
#define LCCR0_LDM	(1 << 3)	/* LCD Disable Done Mask */
#define LCCR0_SFM	(1 << 4)	/* Start of frame mask */
#define LCCR0_IUM	(1 << 5)	/* Input FIFO underrun mask */
#define LCCR0_EFM	(1 << 6)	/* End of Frame mask */
#define LCCR0_PAS	(1 << 7)	/* Passive = 0, Active = 1 */
#define LCCR0_BLE	(1 << 8)	/* Little Endian = 0, Big Endian = 1 */
#define LCCR0_DPD	(1 << 9)	/* Double Pixel mode, 4 pixel value = 0, 8 pixle values = 1 */
#define LCCR0_DIS	(1 << 10)	/* LCD Disable */
#define LCCR0_QDM	(1 << 11)	/* LCD Quick Disable mask */
#define LCCR0_PDD	(0xff << 12)	/* Palette DMA request delay */
#define LCCR0_PDD_S	12
#define LCCR0_BM	(1 << 20)	/* Branch mask */
#define LCCR0_OUM	(1 << 21)	/* Output FIFO underrun mask */
#if defined(CONFIG_CPU_PXA27X)
#define LCCR0_LCDT	(1 << 22)	/* LCD Panel Type */
#define LCCR0_RDSTM	(1 << 23)	/* Read Status Interrupt Mask */
#define LCCR0_CMDIM	(1 << 24)	/* Command Interrupt Mask */
#endif

#define LCCR1_PPL	Fld (10, 0)	 /* Pixels Per Line - 1 */
#define LCCR1_DisWdth(Pixel)		/* Display Width [1..800 pix.]	*/ \
			(((Pixel) - 1) << FShft (LCCR1_PPL))

#define LCCR1_HSW	Fld (6, 10)	/* Horizontal Synchronization	  */
#define LCCR1_HorSnchWdth(Tpix)		/* Horizontal Synchronization	  */ \
					/* pulse Width [1..64 Tpix]	  */ \
			(((Tpix) - 1) << FShft (LCCR1_HSW))

#define LCCR1_ELW	Fld (8, 16)	/* End-of-Line pixel clock Wait	   */
					/* count - 1 [Tpix]		   */
#define LCCR1_EndLnDel(Tpix)		/*  End-of-Line Delay		   */ \
					/*  [1..256 Tpix]		   */ \
			(((Tpix) - 1) << FShft (LCCR1_ELW))

#define LCCR1_BLW	Fld (8, 24)	/* Beginning-of-Line pixel clock   */
					/* Wait count - 1 [Tpix]	   */
#define LCCR1_BegLnDel(Tpix)		/*  Beginning-of-Line Delay	   */ \
					/*  [1..256 Tpix]		   */ \
			(((Tpix) - 1) << FShft (LCCR1_BLW))


#define LCCR2_LPP	Fld (10, 0)	/* Line Per Panel - 1		   */
#define LCCR2_DisHght(Line)		/*  Display Height [1..1024 lines] */ \
			(((Line) - 1) << FShft (LCCR2_LPP))

#define LCCR2_VSW	Fld (6, 10)	/* Vertical Synchronization pulse  */
					/* Width - 1 [Tln] (L_FCLK)	   */
#define LCCR2_VrtSnchWdth(Tln)		/*  Vertical Synchronization pulse */ \
					/*  Width [1..64 Tln]		   */ \
			(((Tln) - 1) << FShft (LCCR2_VSW))

#define LCCR2_EFW	Fld (8, 16)	/* End-of-Frame line clock Wait	   */
					/* count [Tln]			   */
#define LCCR2_EndFrmDel(Tln)		/*  End-of-Frame Delay		   */ \
					/*  [0..255 Tln]		   */ \
			((Tln) << FShft (LCCR2_EFW))

#define LCCR2_BFW	Fld (8, 24)	/* Beginning-of-Frame line clock   */
					/* Wait count [Tln]		   */
#define LCCR2_BegFrmDel(Tln)		/*  Beginning-of-Frame Delay	   */ \
					/*  [0..255 Tln]		   */ \
			((Tln) << FShft (LCCR2_BFW))

#define LCCR3_API	(0xf << 16)	/* AC Bias pin trasitions per interrupt */
#define LCCR3_API_S	16
#define LCCR3_VSP	(1 << 20)	/* vertical sync polarity */
#define LCCR3_HSP	(1 << 21)	/* horizontal sync polarity */
#define LCCR3_PCP	(1 << 22)	/* pixel clock polarity */
#define LCCR3_OEP	(1 << 23)	/* output enable polarity */
#define LCCR3_DPC	(1 << 27)	/* double pixel clock mode */

#define LCCR3_PDFOR_0	 (0 << 30)
#define LCCR3_PDFOR_1	 (1 << 30)
#define LCCR3_PDFOR_2	 (2 << 30)
#define LCCR3_PDFOR_3	 (3 << 30)


#define LCCR3_PCD	Fld (8, 0)	/* Pixel Clock Divisor */
#define LCCR3_PixClkDiv(Div)		/* Pixel Clock Divisor */ \
			(((Div) << FShft (LCCR3_PCD)))


#define LCCR3_BPP	Fld (3, 24)	/* Bit Per Pixel */
#define LCCR3_Bpp(Bpp)			/* Bit Per Pixel */ \
			((((Bpp&0x7) << FShft (LCCR3_BPP)))|(((Bpp&0x8)<<26)))

#define LCCR3_ACB	Fld (8, 8)	/* AC Bias */
#define LCCR3_Acb(Acb)			/* BAC Bias */ \
			(((Acb) << FShft (LCCR3_ACB)))

#define LCCR3_HorSnchH	(LCCR3_HSP*0)	/*  Horizontal Synchronization	   */
					/*  pulse active High		   */
#define LCCR3_HorSnchL	(LCCR3_HSP*1)	/*  Horizontal Synchronization	   */

#define LCCR3_VrtSnchH	(LCCR3_VSP*0)	/*  Vertical Synchronization pulse */
					/*  active High			   */
#define LCCR3_VrtSnchL	(LCCR3_VSP*1)	/*  Vertical Synchronization pulse */
					/*  active Low			   */

#define LCSR0_LDD	(1 << 0)	/* LCD Disable Done */
#define LCSR0_SOF	(1 << 1)	/* Start of frame */
#define LCSR0_BER	(1 << 2)	/* Bus error */
#define LCSR0_ABC	(1 << 3)	/* AC Bias count */
#define LCSR0_IUL	(1 << 4)	/* input FIFO underrun Lower panel */
#define LCSR0_IUU	(1 << 5)	/* input FIFO underrun Upper panel */
#define LCSR0_OU	(1 << 6)	/* output FIFO underrun */
#define LCSR0_QD	(1 << 7)	/* quick disable */
#define LCSR0_EOF0	(1 << 8)	/* end of frame */
#define LCSR0_BS	(1 << 9)	/* branch status */
#define LCSR0_SINT	(1 << 10)	/* subsequent interrupt */

#define LCSR1_SOF1	(1 << 0)
#define LCSR1_SOF2	(1 << 1)
#define LCSR1_SOF3	(1 << 2)
#define LCSR1_SOF4	(1 << 3)
#define LCSR1_SOF5	(1 << 4)
#define LCSR1_SOF6	(1 << 5)

#define LCSR1_EOF1	(1 << 8)
#define LCSR1_EOF2	(1 << 9)
#define LCSR1_EOF3	(1 << 10)
#define LCSR1_EOF4	(1 << 11)
#define LCSR1_EOF5	(1 << 12)
#define LCSR1_EOF6	(1 << 13)

#define LCSR1_BS1	(1 << 16)
#define LCSR1_BS2	(1 << 17)
#define LCSR1_BS3	(1 << 18)
#define LCSR1_BS4	(1 << 19)
#define LCSR1_BS5	(1 << 20)
#define LCSR1_BS6	(1 << 21)

#define LCSR1_IU2	(1 << 25)
#define LCSR1_IU3	(1 << 26)
#define LCSR1_IU4	(1 << 27)
#define LCSR1_IU5	(1 << 28)
#define LCSR1_IU6	(1 << 29)

#define LDCMD_PAL	(1 << 26)	/* instructs DMA to load palette buffer */
#if defined(CONFIG_CPU_PXA27X)
#define LDCMD_SOFINT	(1 << 22)
#define LDCMD_EOFINT	(1 << 21)
#endif

/*
 * Memory controller
 */

#ifdef CONFIG_CPU_MONAHANS

/* PXA3xx */

/* Static Memory Controller Registers */
#define	MSC0		0x4A000008 /* Static Memory Control Register 0 */
#define	MSC1		0x4A00000C /* Static Memory Control Register 1 */
#define	MECR		0x4A000014 /* Expansion Memory (PCMCIA/Compact Flash) Bus Configuration */
#define	SXCNFG		0x4A00001C /* Synchronous Static Memory Control Register */
#define	MCMEM0		0x4A000028 /* Card interface Common Memory Space Socket 0 Timing */
#define	MCATT0		0x4A000030 /* Card interface Attribute Space Socket 0 Timing Configuration */
#define	MCIO0		0x4A000038 /* Card interface I/O Space Socket 0 Timing Configuration */
#define	MEMCLKCFG	0x4A000068 /* SCLK speed configuration */
#define	CSADRCFG0	0x4A000080 /* Address Configuration for chip select 0 */
#define	CSADRCFG1	0x4A000084 /* Address Configuration for chip select 1 */
#define	CSADRCFG2	0x4A000088 /* Address Configuration for chip select 2 */
#define	CSADRCFG3	0x4A00008C /* Address Configuration for chip select 3 */
#define	CSADRCFG_P	0x4A000090 /* Address Configuration for pcmcia card interface */
#define	CSMSADRCFG	0x4A0000A0 /* Master Address Configuration Register */
#define	CLK_RET_DEL	0x4A0000B0 /* Delay line and mux selects for return data latching for sync. flash */
#define	ADV_RET_DEL	0x4A0000B4 /* Delay line and mux selects for return data latching for sync. flash */

/* Dynamic Memory Controller Registers */
#define	MDCNFG		0x48100000 /* SDRAM Configuration Register 0 */
#define	MDREFR		0x48100004 /* SDRAM Refresh Control Register */
#define	FLYCNFG		0x48100020 /* Fly-by DMA DVAL[1:0] polarities */
#define	MDMRS		0x48100040 /* MRS value to be written to SDRAM */
#define	DDR_SCAL	0x48100050 /* Software Delay Line Calibration/Configuration for external DDR memory. */
#define	DDR_HCAL	0x48100060 /* Hardware Delay Line Calibration/Configuration for external DDR memory. */
#define	DDR_WCAL	0x48100068 /* DDR Write Strobe Calibration Register */
#define	DMCIER		0x48100070 /* Dynamic MC Interrupt Enable Register. */
#define	DMCISR		0x48100078 /* Dynamic MC Interrupt Status Register. */
#define	DDR_DLS		0x48100080 /* DDR Delay Line Value Status register for external DDR memory. */
#define	EMPI		0x48100090 /* EMPI Control Register */
#define	RCOMP		0x48100100
#define	PAD_MA		0x48100110
#define	PAD_MDMSB	0x48100114
#define	PAD_MDLSB	0x48100118
#define	PAD_DMEM	0x4810011c
#define	PAD_SDCLK	0x48100120
#define	PAD_SDCS	0x48100124
#define	PAD_SMEM	0x48100128
#define	PAD_SCLK	0x4810012C
#define	TAI		0x48100F00 /* TAI Tavor Address Isolation Register */

/* Some frequently used bits */
#define MDCNFG_DMAP	0x80000000	/* SDRAM 1GB Memory Map Enable */
#define MDCNFG_DMCEN	0x40000000	/* Enable Dynamic Memory Controller */
#define MDCNFG_HWFREQ	0x20000000	/* Hardware Frequency Change Calibration */
#define MDCNFG_DTYPE	0x400		/* SDRAM Type: 1=DDR SDRAM */

#define MDCNFG_DTC_0	0x0		/* Timing Category of SDRAM */
#define MDCNFG_DTC_1	0x100
#define MDCNFG_DTC_2	0x200
#define MDCNFG_DTC_3	0x300

#define MDCNFG_DRAC_12	0x0		/* Number of Row Access Bits */
#define MDCNFG_DRAC_13	0x20
#define MDCNFG_DRAC_14	0x40

#define MDCNFG_DCAC_9	0x0		/* Number of Column Acess Bits */
#define MDCNFG_DCAC_10	0x08
#define MDCNFG_DCAC_11	0x10

#define MDCNFG_DBW_16	0x4		/* SDRAM Data Bus width 16bit */
#define MDCNFG_DCSE1	0x2		/* SDRAM CS 1 Enable */
#define MDCNFG_DCSE0	0x1		/* SDRAM CS 0 Enable */


/* Data Flash Controller Registers */

#define NDCR		0x43100000  /* Data Flash Control register */
#define NDTR0CS0	0x43100004  /* Data Controller Timing Parameter 0 Register for ND_nCS0 */
/* #define NDTR0CS1	0x43100008  /\* Data Controller Timing Parameter 0 Register for ND_nCS1 *\/ */
#define NDTR1CS0	0x4310000C  /* Data Controller Timing Parameter 1 Register for ND_nCS0 */
/* #define NDTR1CS1	0x43100010  /\* Data Controller Timing Parameter 1 Register for ND_nCS1 *\/ */
#define NDSR		0x43100014  /* Data Controller Status Register */
#define NDPCR		0x43100018  /* Data Controller Page Count Register */
#define NDBDR0		0x4310001C  /* Data Controller Bad Block Register 0 */
#define NDBDR1		0x43100020  /* Data Controller Bad Block Register 1 */
#define NDDB		0x43100040  /* Data Controller Data Buffer */
#define NDCB0		0x43100048  /* Data Controller Command Buffer0 */
#define NDCB1		0x4310004C  /* Data Controller Command Buffer1 */
#define NDCB2		0x43100050  /* Data Controller Command Buffer2 */

#define NDCR_SPARE_EN	(0x1<<31)
#define NDCR_ECC_EN	(0x1<<30)
#define NDCR_DMA_EN	(0x1<<29)
#define NDCR_ND_RUN	(0x1<<28)
#define NDCR_DWIDTH_C	(0x1<<27)
#define NDCR_DWIDTH_M	(0x1<<26)
#define NDCR_PAGE_SZ	(0x3<<24)
#define NDCR_NCSX	(0x1<<23)
#define NDCR_ND_STOP	(0x1<<22)
/* reserved:
 * #define NDCR_ND_MODE	(0x3<<21)
 * #define NDCR_NAND_MODE   0x0 */
#define NDCR_CLR_PG_CNT	(0x1<<20)
#define NDCR_CLR_ECC	(0x1<<19)
#define NDCR_RD_ID_CNT	(0x7<<16)
#define NDCR_RA_START	(0x1<<15)
#define NDCR_PG_PER_BLK	(0x1<<14)
#define NDCR_ND_ARB_EN	(0x1<<12)
#define NDCR_RDYM	(0x1<<11)
#define NDCR_CS0_PAGEDM	(0x1<<10)
#define NDCR_CS1_PAGEDM	(0x1<<9)
#define NDCR_CS0_CMDDM	(0x1<<8)
#define NDCR_CS1_CMDDM	(0x1<<7)
#define NDCR_CS0_BBDM	(0x1<<6)
#define NDCR_CS1_BBDM	(0x1<<5)
#define NDCR_DBERRM	(0x1<<4)
#define NDCR_SBERRM	(0x1<<3)
#define NDCR_WRDREQM	(0x1<<2)
#define NDCR_RDDREQM	(0x1<<1)
#define NDCR_WRCMDREQM	(0x1)

#define NDSR_RDY	(0x1<<11)
#define NDSR_CS0_PAGED	(0x1<<10)
#define NDSR_CS1_PAGED	(0x1<<9)
#define NDSR_CS0_CMDD	(0x1<<8)
#define NDSR_CS1_CMDD	(0x1<<7)
#define NDSR_CS0_BBD	(0x1<<6)
#define NDSR_CS1_BBD	(0x1<<5)
#define NDSR_DBERR	(0x1<<4)
#define NDSR_SBERR	(0x1<<3)
#define NDSR_WRDREQ	(0x1<<2)
#define NDSR_RDDREQ	(0x1<<1)
#define NDSR_WRCMDREQ	(0x1)

#define NDCB0_AUTO_RS	(0x1<<25)
#define NDCB0_CSEL	(0x1<<24)
#define NDCB0_CMD_TYPE	(0x7<<21)
#define NDCB0_NC	(0x1<<20)
#define NDCB0_DBC	(0x1<<19)
#define NDCB0_ADDR_CYC	(0x7<<16)
#define NDCB0_CMD2	(0xff<<8)
#define NDCB0_CMD1	(0xff)
#define MCMEM(s) MCMEM0
#define MCATT(s) MCATT0
#define MCIO(s) MCIO0
#define MECR_CIT	(1 << 1)/* Card Is There: 0 -> no card, 1 -> card inserted */

/* Maximum values for NAND Interface Timing Registers in DFC clock
 * periods */
#define DFC_MAX_tCH	7
#define DFC_MAX_tCS	7
#define DFC_MAX_tWH	7
#define DFC_MAX_tWP	7
#define DFC_MAX_tRH	7
#define DFC_MAX_tRP	15
#define DFC_MAX_tR	65535
#define DFC_MAX_tWHR	15
#define DFC_MAX_tAR	15

#define DFC_CLOCK	104		/* DFC Clock is 104 MHz */
#define DFC_CLK_PER_US	DFC_CLOCK/1000	/* clock period in ns */

#else /* CONFIG_CPU_MONAHANS */

/* PXA2xx */

#define MEMC_BASE	0x48000000  /* Base of Memory Controller */
#define MDCNFG_OFFSET	0x0
#define MDREFR_OFFSET	0x4
#define MSC0_OFFSET	0x8
#define MSC1_OFFSET	0xC
#define MSC2_OFFSET	0x10
#define MECR_OFFSET	0x14
#define SXLCR_OFFSET	0x18
#define SXCNFG_OFFSET	0x1C
#define FLYCNFG_OFFSET	0x20
#define SXMRS_OFFSET	0x24
#define MCMEM0_OFFSET	0x28
#define MCMEM1_OFFSET	0x2C
#define MCATT0_OFFSET	0x30
#define MCATT1_OFFSET	0x34
#define MCIO0_OFFSET	0x38
#define MCIO1_OFFSET	0x3C
#define MDMRS_OFFSET	0x40

#define MDCNFG		0x48000000  /* SDRAM Configuration Register 0 */
#define MDCNFG_DE0	0x00000001
#define MDCNFG_DE1	0x00000002
#define MDCNFG_DE2	0x00010000
#define MDCNFG_DE3	0x00020000
#define MDCNFG_DWID0	0x00000004

#define MDREFR		0x48000004  /* SDRAM Refresh Control Register */
#define MSC0		0x48000008  /* Static Memory Control Register 0 */
#define MSC1		0x4800000C  /* Static Memory Control Register 1 */
#define MSC2		0x48000010  /* Static Memory Control Register 2 */
#define MECR		0x48000014  /* Expansion Memory (PCMCIA/Compact Flash) Bus Configuration */
#define SXLCR		0x48000018  /* LCR value to be written to SDRAM-Timing Synchronous Flash */
#define SXCNFG		0x4800001C  /* Synchronous Static Memory Control Register */
#define FLYCNFG		0x48000020
#define SXMRS		0x48000024  /* MRS value to be written to Synchronous Flash or SMROM */
#define MCMEM0		0x48000028  /* Card interface Common Memory Space Socket 0 Timing */
#define MCMEM1		0x4800002C  /* Card interface Common Memory Space Socket 1 Timing */
#define MCATT0		0x48000030  /* Card interface Attribute Space Socket 0 Timing Configuration */
#define MCATT1		0x48000034  /* Card interface Attribute Space Socket 1 Timing Configuration */
#define MCIO0		0x48000038  /* Card interface I/O Space Socket 0 Timing Configuration */
#define MCIO1		0x4800003C  /* Card interface I/O Space Socket 1 Timing Configuration */
#define MDMRS		0x48000040  /* MRS value to be written to SDRAM */
#define BOOT_DEF	0x48000044  /* Read-Only Boot-Time Register. Contains BOOT_SEL and PKG_SEL */

#define MDREFR_ALTREFA	(1 << 31)	/* Exiting Alternate Bus Master Mode Refresh Control */
#define MDREFR_ALTREFB	(1 << 30)	/* Entering Alternate Bus Master Mode Refresh Control */
#define MDREFR_K0DB4	(1 << 29)	/* SDCLK0 Divide by 4 Control/Status */
#define MDREFR_K2FREE	(1 << 25)	/* SDRAM Free-Running Control */
#define MDREFR_K1FREE	(1 << 24)	/* SDRAM Free-Running Control */
#define MDREFR_K0FREE	(1 << 23)	/* SDRAM Free-Running Control */
#define MDREFR_SLFRSH	(1 << 22)	/* SDRAM Self-Refresh Control/Status */
#define MDREFR_APD	(1 << 20)	/* SDRAM/SSRAM Auto-Power-Down Enable */
#define MDREFR_K2DB2	(1 << 19)	/* SDCLK2 Divide by 2 Control/Status */
#define MDREFR_K2RUN	(1 << 18)	/* SDCLK2 Run Control/Status */
#define MDREFR_K1DB2	(1 << 17)	/* SDCLK1 Divide by 2 Control/Status */
#define MDREFR_K1RUN	(1 << 16)	/* SDCLK1 Run Control/Status */
#define MDREFR_E1PIN	(1 << 15)	/* SDCKE1 Level Control/Status */
#define MDREFR_K0DB2	(1 << 14)	/* SDCLK0 Divide by 2 Control/Status */
#define MDREFR_K0RUN	(1 << 13)	/* SDCLK0 Run Control/Status */
#define MDREFR_E0PIN	(1 << 12)	/* SDCKE0 Level Control/Status */

#if defined(CONFIG_CPU_PXA27X)

#define ARB_CNTRL	0x48000048  /* Arbiter Control Register */

#define ARB_DMA_SLV_PARK	(1<<31)	   /* Be parked with DMA slave when idle */
#define ARB_CI_PARK		(1<<30)	   /* Be parked with Camera Interface when idle */
#define ARB_EX_MEM_PARK		(1<<29)	   /* Be parked with external MEMC when idle */
#define ARB_INT_MEM_PARK	(1<<28)	   /* Be parked with internal MEMC when idle */
#define ARB_USB_PARK		(1<<27)	   /* Be parked with USB when idle */
#define ARB_LCD_PARK		(1<<26)	   /* Be parked with LCD when idle */
#define ARB_DMA_PARK		(1<<25)	   /* Be parked with DMA when idle */
#define ARB_CORE_PARK		(1<<24)	   /* Be parked with core when idle */
#define ARB_LOCK_FLAG		(1<<23)	   /* Only Locking masters gain access to the bus */

#endif /* CONFIG_CPU_PXA27X */

/* LCD registers */
#define LCCR4		0x44000010  /* LCD Controller Control Register 4 */
#define LCCR5		0x44000014  /* LCD Controller Control Register 5 */
#define FBR0		0x44000020  /* DMA Channel 0 Frame Branch Register */
#define FBR1		0x44000024  /* DMA Channel 1 Frame Branch Register */
#define FBR2		0x44000028  /* DMA Channel 2 Frame Branch Register */
#define FBR3		0x4400002C  /* DMA Channel 3 Frame Branch Register */
#define FBR4		0x44000030  /* DMA Channel 4 Frame Branch Register */
#define FDADR2		0x44000220  /* DMA Channel 2 Frame Descriptor Address Register */
#define FSADR2		0x44000224  /* DMA Channel 2 Frame Source Address Register */
#define FIDR2		0x44000228  /* DMA Channel 2 Frame ID Register */
#define LDCMD2		0x4400022C  /* DMA Channel 2 Command Register */
#define FDADR3		0x44000230  /* DMA Channel 3 Frame Descriptor Address Register */
#define FSADR3		0x44000234  /* DMA Channel 3 Frame Source Address Register */
#define FIDR3		0x44000238  /* DMA Channel 3 Frame ID Register */
#define LDCMD3		0x4400023C  /* DMA Channel 3 Command Register */
#define FDADR4		0x44000240  /* DMA Channel 4 Frame Descriptor Address Register */
#define FSADR4		0x44000244  /* DMA Channel 4 Frame Source Address Register */
#define FIDR4		0x44000248  /* DMA Channel 4 Frame ID Register */
#define LDCMD4		0x4400024C  /* DMA Channel 4 Command Register */
#define FDADR5		0x44000250  /* DMA Channel 5 Frame Descriptor Address Register */
#define FSADR5		0x44000254  /* DMA Channel 5 Frame Source Address Register */
#define FIDR5		0x44000258  /* DMA Channel 5 Frame ID Register */
#define LDCMD5		0x4400025C  /* DMA Channel 5 Command Register */

#define OVL1C1		0x44000050  /* Overlay 1 Control Register 1 */
#define OVL1C2		0x44000060  /* Overlay 1 Control Register 2 */
#define OVL2C1		0x44000070  /* Overlay 2 Control Register 1 */
#define OVL2C2		0x44000080  /* Overlay 2 Control Register 2 */
#define CCR		0x44000090  /* Cursor Control Register */

#define FBR5		0x44000110  /* DMA Channel 5 Frame Branch Register */
#define FBR6		0x44000114  /* DMA Channel 6 Frame Branch Register */

#define LCCR0_LDDALT	(1<<26)		/* LDD Alternate mapping bit when base pixel is RGBT16 */
#define LCCR0_OUC	(1<<25)		/* Overlay Underlay Control Bit */

#define LCCR5_SOFM1	(1<<0)		/* Start Of Frame Mask for Overlay 1 (channel 1) */
#define LCCR5_SOFM2	(1<<1)		/* Start Of Frame Mask for Overlay 2 (channel 2) */
#define LCCR5_SOFM3	(1<<2)		/* Start Of Frame Mask for Overlay 2 (channel 3) */
#define LCCR5_SOFM4	(1<<3)		/* Start Of Frame Mask for Overlay 2 (channel 4) */
#define LCCR5_SOFM5	(1<<4)		/* Start Of Frame Mask for cursor (channel 5) */
#define LCCR5_SOFM6	(1<<5)		/* Start Of Frame Mask for command data (channel 6) */

#define LCCR5_EOFM1	(1<<8)		/* End Of Frame Mask for Overlay 1 (channel 1) */
#define LCCR5_EOFM2	(1<<9)		/* End Of Frame Mask for Overlay 2 (channel 2) */
#define LCCR5_EOFM3	(1<<10)		/* End Of Frame Mask for Overlay 2 (channel 3) */
#define LCCR5_EOFM4	(1<<11)		/* End Of Frame Mask for Overlay 2 (channel 4) */
#define LCCR5_EOFM5	(1<<12)		/* End Of Frame Mask for cursor (channel 5) */
#define LCCR5_EOFM6	(1<<13)		/* End Of Frame Mask for command data (channel 6) */

#define LCCR5_BSM1	(1<<16)		/* Branch mask for Overlay 1 (channel 1) */
#define LCCR5_BSM2	(1<<17)		/* Branch mask for Overlay 2 (channel 2) */
#define LCCR5_BSM3	(1<<18)		/* Branch mask for Overlay 2 (channel 3) */
#define LCCR5_BSM4	(1<<19)		/* Branch mask for Overlay 2 (channel 4) */
#define LCCR5_BSM5	(1<<20)		/* Branch mask for cursor (channel 5) */
#define LCCR5_BSM6	(1<<21)		/* Branch mask for data command	 (channel 6) */

#define LCCR5_IUM1	(1<<24)		/* Input FIFO Underrun Mask for Overlay 1  */
#define LCCR5_IUM2	(1<<25)		/* Input FIFO Underrun Mask for Overlay 2  */
#define LCCR5_IUM3	(1<<26)		/* Input FIFO Underrun Mask for Overlay 2  */
#define LCCR5_IUM4	(1<<27)		/* Input FIFO Underrun Mask for Overlay 2  */
#define LCCR5_IUM5	(1<<28)		/* Input FIFO Underrun Mask for cursor */
#define LCCR5_IUM6	(1<<29)		/* Input FIFO Underrun Mask for data command */

#define OVL1C1_O1EN	(1<<31)		/* Enable bit for Overlay 1 */
#define OVL2C1_O2EN	(1<<31)		/* Enable bit for Overlay 2 */
#define CCR_CEN		(1<<31)		/* Enable bit for Cursor */

/* Keypad controller */

#define KPC		0x41500000 /* Keypad Interface Control register */
#define KPDK		0x41500008 /* Keypad Interface Direct Key register */
#define KPREC		0x41500010 /* Keypad Intefcace Rotary Encoder register */
#define KPMK		0x41500018 /* Keypad Intefcace Matrix Key register */
#define KPAS		0x41500020 /* Keypad Interface Automatic Scan register */
#define KPASMKP0	0x41500028 /* Keypad Interface Automatic Scan Multiple Key Presser register 0 */
#define KPASMKP1	0x41500030 /* Keypad Interface Automatic Scan Multiple Key Presser register 1 */
#define KPASMKP2	0x41500038 /* Keypad Interface Automatic Scan Multiple Key Presser register 2 */
#define KPASMKP3	0x41500040 /* Keypad Interface Automatic Scan Multiple Key Presser register 3 */
#define KPKDI		0x41500048 /* Keypad Interface Key Debounce Interval register */

#define KPC_AS		(0x1 << 30)  /* Automatic Scan bit */
#define KPC_ASACT	(0x1 << 29)  /* Automatic Scan on Activity */
#define KPC_MI		(0x1 << 22)  /* Matrix interrupt bit */
#define KPC_IMKP	(0x1 << 21)  /* Ignore Multiple Key Press */
#define KPC_MS7		(0x1 << 20)  /* Matrix scan line 7 */
#define KPC_MS6		(0x1 << 19)  /* Matrix scan line 6 */
#define KPC_MS5		(0x1 << 18)  /* Matrix scan line 5 */
#define KPC_MS4		(0x1 << 17)  /* Matrix scan line 4 */
#define KPC_MS3		(0x1 << 16)  /* Matrix scan line 3 */
#define KPC_MS2		(0x1 << 15)  /* Matrix scan line 2 */
#define KPC_MS1		(0x1 << 14)  /* Matrix scan line 1 */
#define KPC_MS0		(0x1 << 13)  /* Matrix scan line 0 */
#define KPC_ME		(0x1 << 12)  /* Matrix Keypad Enable */
#define KPC_MIE		(0x1 << 11)  /* Matrix Interrupt Enable */
#define KPC_DK_DEB_SEL	(0x1 <<	 9)  /* Direct Key Debounce select */
#define KPC_DI		(0x1 <<	 5)  /* Direct key interrupt bit */
#define KPC_DEE0	(0x1 <<	 2)  /* Rotary Encoder 0 Enable */
#define KPC_DE		(0x1 <<	 1)  /* Direct Keypad Enable */
#define KPC_DIE		(0x1 <<	 0)  /* Direct Keypad interrupt Enable */

#define KPDK_DKP	(0x1 << 31)
#define KPDK_DK7	(0x1 <<	 7)
#define KPDK_DK6	(0x1 <<	 6)
#define KPDK_DK5	(0x1 <<	 5)
#define KPDK_DK4	(0x1 <<	 4)
#define KPDK_DK3	(0x1 <<	 3)
#define KPDK_DK2	(0x1 <<	 2)
#define KPDK_DK1	(0x1 <<	 1)
#define KPDK_DK0	(0x1 <<	 0)

#define KPREC_OF1	(0x1 << 31)
#define kPREC_UF1	(0x1 << 30)
#define KPREC_OF0	(0x1 << 15)
#define KPREC_UF0	(0x1 << 14)

#define KPMK_MKP	(0x1 << 31)
#define KPAS_SO		(0x1 << 31)
#define KPASMKPx_SO	(0x1 << 31)

#define GPIO113_BIT	(1 << 17)/* GPIO113 in GPSR, GPCR, bit 17 */
#define PSLR		0x40F00034
#define PSTR		0x40F00038  /* Power Manager Standby Configuration Reg */
#define PSNR		0x40F0003C  /* Power Manager Sense Configuration Reg */
#define PVCR		0x40F00040  /* Power Manager Voltage Change Control Reg */
#define PKWR		0x40F00050  /* Power Manager KB Wake-Up Enable Reg */
#define PKSR		0x40F00054  /* Power Manager KB Level-Detect Status Reg */
#define OSMR4		0x40A00080  /* */
#define OSCR4		0x40A00040  /* OS Timer Counter Register */
#define OMCR4		0x40A000C0  /* */

#endif	/* CONFIG_CPU_PXA27X */

#endif	/* _PXA_REGS_H_ */
