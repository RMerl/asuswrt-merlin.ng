/* SPDX-License-Identifier: GPL-2.0+ */
/*------------------------------------------------------------------------
 * lan91c96.h
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Rolf Offermanns <rof@sysgo.de>
 * Copyright (C) 2001 Standard Microsystems Corporation (SMSC)
 *       Developed by Simple Network Magic Corporation (SNMC)
 * Copyright (C) 1996 by Erik Stahlman (ES)
 *
 * This file contains register information and access macros for
 * the LAN91C96 single chip ethernet controller.  It is a modified
 * version of the smc9111.h file.
 *
 * Information contained in this file was obtained from the LAN91C96
 * manual from SMC. To get a copy, if you really want one, you can find
 * information under www.smsc.com.
 *
 * Authors
 *	Erik Stahlman				( erik@vt.edu )
 *	Daris A Nevil				( dnevil@snmc.com )
 *
 * History
 * 04/30/03	Mathijs Haarman		Modified smc91111.h (u-boot version)
 *		                        for lan91c96
 *-------------------------------------------------------------------------
 */
#ifndef _LAN91C96_H_
#define _LAN91C96_H_

#include <asm/types.h>
#include <asm/io.h>
#include <config.h>

/* I want some simple types */

typedef unsigned char			byte;
typedef unsigned short			word;
typedef unsigned long int		dword;

/*
 * DEBUGGING LEVELS
 *
 * 0 for normal operation
 * 1 for slightly more details
 * >2 for various levels of increasingly useless information
 *    2 for interrupt tracking, status flags
 *    3 for packet info
 *    4 for complete packet dumps
 */
/*#define SMC_DEBUG 0 */

/* Because of bank switching, the LAN91xxx uses only 16 I/O ports */

#define	SMC_IO_EXTENT	16

#ifdef CONFIG_CPU_PXA25X

#define	SMC_IO_SHIFT	0

#define	SMCREG(edev, r)	((edev)->iobase+((r)<<SMC_IO_SHIFT))

#define	SMC_inl(edev, r)	(*((volatile dword *)SMCREG(edev, r)))
#define	SMC_inw(edev, r)	(*((volatile word *)SMCREG(edev, r)))
#define SMC_inb(edev, p) ({ \
	unsigned int __p = p; \
	unsigned int __v = SMC_inw(edev, __p & ~1); \
	if (__p & 1) __v >>= 8; \
	else __v &= 0xff; \
	__v; })

#define	SMC_outl(edev, d, r)	(*((volatile dword *)SMCREG(edev, r)) = d)
#define	SMC_outw(edev, d, r)	(*((volatile word *)SMCREG(edev, r)) = d)
#define	SMC_outb(edev, d, r)	({	word __d = (byte)(d);  \
				word __w = SMC_inw(edev, (r)&~1);  \
				__w &= ((r)&1) ? 0x00FF : 0xFF00;  \
				__w |= ((r)&1) ? __d<<8 : __d;  \
				SMC_outw(edev, __w, (r)&~1);  \
			})

#define SMC_outsl(edev, r, b, l)	({	int __i; \
					dword *__b2; \
					__b2 = (dword *) b; \
					for (__i = 0; __i < l; __i++) { \
						SMC_outl(edev, *(__b2 + __i),\
							r); \
					} \
				})

#define SMC_outsw(edev, r, b, l)	({	int __i; \
					word *__b2; \
					__b2 = (word *) b; \
					for (__i = 0; __i < l; __i++) { \
						SMC_outw(edev, *(__b2 + __i),\
							r); \
					} \
				})

#define SMC_insl(edev, r, b, l)		({	int __i ;  \
					dword *__b2;  \
					__b2 = (dword *) b;  \
					for (__i = 0; __i < l; __i++) {  \
						*(__b2 + __i) = SMC_inl(edev,\
							r);  \
						SMC_inl(edev, 0);  \
					};  \
				})

#define SMC_insw(edev, r, b, l)		({	int __i ;  \
					word *__b2;  \
					__b2 = (word *) b;  \
					for (__i = 0; __i < l; __i++) {  \
						*(__b2 + __i) = SMC_inw(edev,\
							r);  \
						SMC_inw(edev, 0);  \
					};  \
				})

#define SMC_insb(edev, r, b, l)		({	int __i ;  \
					byte *__b2;  \
					__b2 = (byte *) b;  \
					for (__i = 0; __i < l; __i++) {  \
						*(__b2 + __i) = SMC_inb(edev,\
							r);  \
						SMC_inb(edev, 0);  \
					};  \
				})

#else /* if not CONFIG_CPU_PXA25X */

/*
 * We have only 16 Bit PCMCIA access on Socket 0
 */

#define	SMC_inw(edev, r)	(*((volatile word *)((edev)->iobase+(r))))
#define  SMC_inb(edev, r)	(((r)&1) ? SMC_inw(edev, (r)&~1)>>8 :\
					SMC_inw(edev, r)&0xFF)

#define	SMC_outw(edev, d, r)	(*((volatile word *)((edev)->iobase+(r))) = d)
#define	SMC_outb(edev, d, r)	({	word __d = (byte)(d);  \
				word __w = SMC_inw(edev, (r)&~1);  \
				__w &= ((r)&1) ? 0x00FF : 0xFF00;  \
				__w |= ((r)&1) ? __d<<8 : __d;  \
				SMC_outw(edev, __w, (r)&~1);  \
			})
#define SMC_outsw(edev, r, b, l)	({	int __i; \
					word *__b2; \
					__b2 = (word *) b; \
					for (__i = 0; __i < l; __i++) { \
						SMC_outw(edev, *(__b2 + __i),\
							r); \
					} \
				})

#define SMC_insw(edev, r, b, l)	({	int __i ;  \
					word *__b2;  \
					__b2 = (word *) b;  \
					for (__i = 0; __i < l; __i++) {  \
						*(__b2 + __i) = SMC_inw(edev,\
							r);  \
						SMC_inw(edev, 0);  \
					};  \
				})

#endif

/*
 ****************************************************************************
 *	Bank Select Field
 ****************************************************************************
 */
#define LAN91C96_BANK_SELECT  14       /* Bank Select Register */
#define LAN91C96_BANKSELECT (0x3UC << 0)
#define BANK0               0x00
#define BANK1               0x01
#define BANK2               0x02
#define BANK3               0x03
#define BANK4               0x04

/*
 ****************************************************************************
 *	EEPROM Addresses.
 ****************************************************************************
 */
#define EEPROM_MAC_OFFSET_1    0x6020
#define EEPROM_MAC_OFFSET_2    0x6021
#define EEPROM_MAC_OFFSET_3    0x6022

/*
 ****************************************************************************
 *	Bank 0 Register Map in I/O Space
 ****************************************************************************
 */
#define LAN91C96_TCR          0        /* Transmit Control Register */
#define LAN91C96_EPH_STATUS   2        /* EPH Status Register */
#define LAN91C96_RCR          4        /* Receive Control Register */
#define LAN91C96_COUNTER      6        /* Counter Register */
#define LAN91C96_MIR          8        /* Memory Information Register */
#define LAN91C96_MCR          10       /* Memory Configuration Register */

/*
 ****************************************************************************
 *	Transmit Control Register - Bank 0 - Offset 0
 ****************************************************************************
 */
#define LAN91C96_TCR_TXENA        (0x1U << 0)
#define LAN91C96_TCR_LOOP         (0x1U << 1)
#define LAN91C96_TCR_FORCOL       (0x1U << 2)
#define LAN91C96_TCR_TXP_EN       (0x1U << 3)
#define LAN91C96_TCR_PAD_EN       (0x1U << 7)
#define LAN91C96_TCR_NOCRC        (0x1U << 8)
#define LAN91C96_TCR_MON_CSN      (0x1U << 10)
#define LAN91C96_TCR_FDUPLX       (0x1U << 11)
#define LAN91C96_TCR_STP_SQET     (0x1U << 12)
#define LAN91C96_TCR_EPH_LOOP     (0x1U << 13)
#define LAN91C96_TCR_ETEN_TYPE    (0x1U << 14)
#define LAN91C96_TCR_FDSE         (0x1U << 15)

/*
 ****************************************************************************
 *	EPH Status Register - Bank 0 - Offset 2
 ****************************************************************************
 */
#define LAN91C96_EPHSR_TX_SUC     (0x1U << 0)
#define LAN91C96_EPHSR_SNGL_COL   (0x1U << 1)
#define LAN91C96_EPHSR_MUL_COL    (0x1U << 2)
#define LAN91C96_EPHSR_LTX_MULT   (0x1U << 3)
#define LAN91C96_EPHSR_16COL      (0x1U << 4)
#define LAN91C96_EPHSR_SQET       (0x1U << 5)
#define LAN91C96_EPHSR_LTX_BRD    (0x1U << 6)
#define LAN91C96_EPHSR_TX_DEFR    (0x1U << 7)
#define LAN91C96_EPHSR_WAKEUP     (0x1U << 8)
#define LAN91C96_EPHSR_LATCOL     (0x1U << 9)
#define LAN91C96_EPHSR_LOST_CARR  (0x1U << 10)
#define LAN91C96_EPHSR_EXC_DEF    (0x1U << 11)
#define LAN91C96_EPHSR_CTR_ROL    (0x1U << 12)

#define LAN91C96_EPHSR_LINK_OK    (0x1U << 14)
#define LAN91C96_EPHSR_TX_UNRN    (0x1U << 15)

#define LAN91C96_EPHSR_ERRORS     (LAN91C96_EPHSR_SNGL_COL  |    \
				   LAN91C96_EPHSR_MUL_COL   |    \
				   LAN91C96_EPHSR_16COL     |    \
				   LAN91C96_EPHSR_SQET      |    \
				   LAN91C96_EPHSR_TX_DEFR   |    \
				   LAN91C96_EPHSR_LATCOL    |    \
				   LAN91C96_EPHSR_LOST_CARR |    \
				   LAN91C96_EPHSR_EXC_DEF   |    \
				   LAN91C96_EPHSR_LINK_OK   |    \
				   LAN91C96_EPHSR_TX_UNRN)

/*
 ****************************************************************************
 *	Receive Control Register - Bank 0 - Offset 4
 ****************************************************************************
 */
#define LAN91C96_RCR_RX_ABORT     (0x1U << 0)
#define LAN91C96_RCR_PRMS         (0x1U << 1)
#define LAN91C96_RCR_ALMUL        (0x1U << 2)
#define LAN91C96_RCR_RXEN         (0x1U << 8)
#define LAN91C96_RCR_STRIP_CRC    (0x1U << 9)
#define LAN91C96_RCR_FILT_CAR     (0x1U << 14)
#define LAN91C96_RCR_SOFT_RST     (0x1U << 15)

/*
 ****************************************************************************
 *	Counter Register - Bank 0 - Offset 6
 ****************************************************************************
 */
#define LAN91C96_ECR_SNGL_COL     (0xFU << 0)
#define LAN91C96_ECR_MULT_COL     (0xFU << 5)
#define LAN91C96_ECR_DEF_TX       (0xFU << 8)
#define LAN91C96_ECR_EXC_DEF_TX   (0xFU << 12)

/*
 ****************************************************************************
 *	Memory Information Register - Bank 0 - OFfset 8
 ****************************************************************************
 */
#define LAN91C96_MIR_SIZE        (0x18 << 0)    /* 6144 bytes */

/*
 ****************************************************************************
 *	Memory Configuration Register - Bank 0 - Offset 10
 ****************************************************************************
 */
#define LAN91C96_MCR_MEM_RES      (0xFFU << 0)
#define LAN91C96_MCR_MEM_MULT     (0x3U << 9)
#define LAN91C96_MCR_HIGH_ID      (0x3U << 12)

#define LAN91C96_MCR_TRANSMIT_PAGES 0x6

/*
 ****************************************************************************
 *	Bank 1 Register Map in I/O Space
 ****************************************************************************
 */
#define LAN91C96_CONFIG       0        /* Configuration Register */
#define LAN91C96_BASE         2        /* Base Address Register */
#define LAN91C96_IA0          4        /* Individual Address Register - 0 */
#define LAN91C96_IA1          5        /* Individual Address Register - 1 */
#define LAN91C96_IA2          6        /* Individual Address Register - 2 */
#define LAN91C96_IA3          7        /* Individual Address Register - 3 */
#define LAN91C96_IA4          8        /* Individual Address Register - 4 */
#define LAN91C96_IA5          9        /* Individual Address Register - 5 */
#define LAN91C96_GEN_PURPOSE  10       /* General Address Registers */
#define LAN91C96_CONTROL      12       /* Control Register */

/*
 ****************************************************************************
 *	Configuration Register - Bank 1 - Offset 0
 ****************************************************************************
 */
#define LAN91C96_CR_INT_SEL0      (0x1U << 1)
#define LAN91C96_CR_INT_SEL1      (0x1U << 2)
#define LAN91C96_CR_RES           (0x3U << 3)
#define LAN91C96_CR_DIS_LINK      (0x1U << 6)
#define LAN91C96_CR_16BIT         (0x1U << 7)
#define LAN91C96_CR_AUI_SELECT    (0x1U << 8)
#define LAN91C96_CR_SET_SQLCH     (0x1U << 9)
#define LAN91C96_CR_FULL_STEP     (0x1U << 10)
#define LAN91C96_CR_NO_WAIT       (0x1U << 12)

/*
 ****************************************************************************
 *	Base Address Register - Bank 1 - Offset 2
 ****************************************************************************
 */
#define LAN91C96_BAR_RA_BITS      (0x27U << 0)
#define LAN91C96_BAR_ROM_SIZE     (0x1U << 6)
#define LAN91C96_BAR_A_BITS       (0xFFU << 8)

/*
 ****************************************************************************
 *	Control Register - Bank 1 - Offset 12
 ****************************************************************************
 */
#define LAN91C96_CTR_STORE        (0x1U << 0)
#define LAN91C96_CTR_RELOAD       (0x1U << 1)
#define LAN91C96_CTR_EEPROM       (0x1U << 2)
#define LAN91C96_CTR_TE_ENABLE    (0x1U << 5)
#define LAN91C96_CTR_CR_ENABLE    (0x1U << 6)
#define LAN91C96_CTR_LE_ENABLE    (0x1U << 7)
#define LAN91C96_CTR_BIT_8        (0x1U << 8)
#define LAN91C96_CTR_AUTO_RELEASE (0x1U << 11)
#define LAN91C96_CTR_WAKEUP_EN    (0x1U << 12)
#define LAN91C96_CTR_PWRDN        (0x1U << 13)
#define LAN91C96_CTR_RCV_BAD      (0x1U << 14)

/*
 ****************************************************************************
 *	Bank 2 Register Map in I/O Space
 ****************************************************************************
 */
#define LAN91C96_MMU            0      /* MMU Command Register */
#define LAN91C96_AUTO_TX_START  1      /* Auto Tx Start Register */
#define LAN91C96_PNR            2      /* Packet Number Register */
#define LAN91C96_ARR            3      /* Allocation Result Register */
#define LAN91C96_FIFO           4      /* FIFO Ports Register */
#define LAN91C96_POINTER        6      /* Pointer Register */
#define LAN91C96_DATA_HIGH      8      /* Data High Register */
#define LAN91C96_DATA_LOW       10     /* Data Low Register */
#define LAN91C96_INT_STATS      12     /* Interrupt Status Register - RO */
#define LAN91C96_INT_ACK        12     /* Interrupt Acknowledge Register -WO */
#define LAN91C96_INT_MASK       13     /* Interrupt Mask Register */

/*
 ****************************************************************************
 *	MMU Command Register - Bank 2 - Offset 0
 ****************************************************************************
 */
#define LAN91C96_MMUCR_NO_BUSY    (0x1U << 0)
#define LAN91C96_MMUCR_N1         (0x1U << 1)
#define LAN91C96_MMUCR_N2         (0x1U << 2)
#define LAN91C96_MMUCR_COMMAND    (0xFU << 4)
#define LAN91C96_MMUCR_ALLOC_TX   (0x2U << 4)    /* WXYZ = 0010 */
#define LAN91C96_MMUCR_RESET_MMU  (0x4U << 4)    /* WXYZ = 0100 */
#define LAN91C96_MMUCR_REMOVE_RX  (0x6U << 4)    /* WXYZ = 0110 */
#define LAN91C96_MMUCR_REMOVE_TX  (0x7U << 4)    /* WXYZ = 0111 */
#define LAN91C96_MMUCR_RELEASE_RX (0x8U << 4)    /* WXYZ = 1000 */
#define LAN91C96_MMUCR_RELEASE_TX (0xAU << 4)    /* WXYZ = 1010 */
#define LAN91C96_MMUCR_ENQUEUE    (0xCU << 4)    /* WXYZ = 1100 */
#define LAN91C96_MMUCR_RESET_TX   (0xEU << 4)    /* WXYZ = 1110 */

/*
 ****************************************************************************
 *	Auto Tx Start Register - Bank 2 - Offset 1
 ****************************************************************************
 */
#define LAN91C96_AUTOTX           (0xFFU << 0)

/*
 ****************************************************************************
 *	Packet Number Register - Bank 2 - Offset 2
 ****************************************************************************
 */
#define LAN91C96_PNR_TX           (0x1FU << 0)

/*
 ****************************************************************************
 *	Allocation Result Register - Bank 2 - Offset 3
 ****************************************************************************
 */
#define LAN91C96_ARR_ALLOC_PN     (0x7FU << 0)
#define LAN91C96_ARR_FAILED       (0x1U << 7)

/*
 ****************************************************************************
 *	FIFO Ports Register - Bank 2 - Offset 4
 ****************************************************************************
 */
#define LAN91C96_FIFO_TX_DONE_PN  (0x1FU << 0)
#define LAN91C96_FIFO_TEMPTY      (0x1U << 7)
#define LAN91C96_FIFO_RX_DONE_PN  (0x1FU << 8)
#define LAN91C96_FIFO_RXEMPTY     (0x1U << 15)

/*
 ****************************************************************************
 *	Pointer Register - Bank 2 - Offset 6
 ****************************************************************************
 */
#define LAN91C96_PTR_LOW          (0xFFU << 0)
#define LAN91C96_PTR_HIGH         (0x7U << 8)
#define LAN91C96_PTR_AUTO_TX      (0x1U << 11)
#define LAN91C96_PTR_ETEN         (0x1U << 12)
#define LAN91C96_PTR_READ         (0x1U << 13)
#define LAN91C96_PTR_AUTO_INCR    (0x1U << 14)
#define LAN91C96_PTR_RCV          (0x1U << 15)

#define LAN91C96_PTR_RX_FRAME     (LAN91C96_PTR_RCV       |    \
				   LAN91C96_PTR_AUTO_INCR |    \
				   LAN91C96_PTR_READ)

/*
 ****************************************************************************
 *	Data Register - Bank 2 - Offset 8
 ****************************************************************************
 */
#define LAN91C96_CONTROL_CRC      (0x1U << 4)    /* CRC bit */
#define LAN91C96_CONTROL_ODD      (0x1U << 5)    /* ODD bit */

/*
 ****************************************************************************
 *	Interrupt Status Register - Bank 2 - Offset 12
 ****************************************************************************
 */
#define LAN91C96_IST_RCV_INT      (0x1U << 0)
#define LAN91C96_IST_TX_INT       (0x1U << 1)
#define LAN91C96_IST_TX_EMPTY_INT (0x1U << 2)
#define LAN91C96_IST_ALLOC_INT    (0x1U << 3)
#define LAN91C96_IST_RX_OVRN_INT  (0x1U << 4)
#define LAN91C96_IST_EPH_INT      (0x1U << 5)
#define LAN91C96_IST_ERCV_INT     (0x1U << 6)
#define LAN91C96_IST_RX_IDLE_INT  (0x1U << 7)

/*
 ****************************************************************************
 *	Interrupt Acknowledge Register - Bank 2 - Offset 12
 ****************************************************************************
 */
#define LAN91C96_ACK_TX_INT       (0x1U << 1)
#define LAN91C96_ACK_TX_EMPTY_INT (0x1U << 2)
#define LAN91C96_ACK_RX_OVRN_INT  (0x1U << 4)
#define LAN91C96_ACK_ERCV_INT     (0x1U << 6)

/*
 ****************************************************************************
 *	Interrupt Mask Register - Bank 2 - Offset 13
 ****************************************************************************
 */
#define LAN91C96_MSK_RCV_INT      (0x1U << 0)
#define LAN91C96_MSK_TX_INT       (0x1U << 1)
#define LAN91C96_MSK_TX_EMPTY_INT (0x1U << 2)
#define LAN91C96_MSK_ALLOC_INT    (0x1U << 3)
#define LAN91C96_MSK_RX_OVRN_INT  (0x1U << 4)
#define LAN91C96_MSK_EPH_INT      (0x1U << 5)
#define LAN91C96_MSK_ERCV_INT     (0x1U << 6)
#define LAN91C96_MSK_TX_IDLE_INT  (0x1U << 7)

/*
 ****************************************************************************
 *	Bank 3 Register Map in I/O Space
 **************************************************************************
 */
#define LAN91C96_MGMT_MDO         (0x1U << 0)
#define LAN91C96_MGMT_MDI         (0x1U << 1)
#define LAN91C96_MGMT_MCLK        (0x1U << 2)
#define LAN91C96_MGMT_MDOE        (0x1U << 3)
#define LAN91C96_MGMT_LOW_ID      (0x3U << 4)
#define LAN91C96_MGMT_IOS0        (0x1U << 8)
#define LAN91C96_MGMT_IOS1        (0x1U << 9)
#define LAN91C96_MGMT_IOS2        (0x1U << 10)
#define LAN91C96_MGMT_nXNDEC      (0x1U << 11)
#define LAN91C96_MGMT_HIGH_ID     (0x3U << 12)

/*
 ****************************************************************************
 *	Revision Register - Bank 3 - Offset 10
 ****************************************************************************
 */
#define LAN91C96_REV_REVID        (0xFU << 0)
#define LAN91C96_REV_CHIPID       (0xFU << 4)

/*
 ****************************************************************************
 *	Early RCV Register - Bank 3 - Offset 12
 ****************************************************************************
 */
#define LAN91C96_ERCV_THRESHOLD   (0x1FU << 0)
#define LAN91C96_ERCV_RCV_DISCRD  (0x1U << 7)

/*
 ****************************************************************************
 *	PCMCIA Configuration Registers
 ****************************************************************************
 */
#define LAN91C96_ECOR    0x8000        /* Ethernet Configuration Register */
#define LAN91C96_ECSR    0x8002        /* Ethernet Configuration and Status */

/*
 ****************************************************************************
 *	PCMCIA Ethernet Configuration Option Register (ECOR)
 ****************************************************************************
 */
#define LAN91C96_ECOR_ENABLE       (0x1U << 0)
#define LAN91C96_ECOR_WR_ATTRIB    (0x1U << 2)
#define LAN91C96_ECOR_LEVEL_REQ    (0x1U << 6)
#define LAN91C96_ECOR_SRESET       (0x1U << 7)

/*
 ****************************************************************************
 *	PCMCIA Ethernet Configuration and Status Register (ECSR)
 ****************************************************************************
 */
#define LAN91C96_ECSR_INTR        (0x1U << 1)
#define LAN91C96_ECSR_PWRDWN      (0x1U << 2)
#define LAN91C96_ECSR_IOIS8       (0x1U << 5)

/*
 ****************************************************************************
 *	Receive Frame Status Word - See page 38 of the LAN91C96 specification.
 ****************************************************************************
 */
#define LAN91C96_TOO_SHORT        (0x1U << 10)
#define LAN91C96_TOO_LONG         (0x1U << 11)
#define LAN91C96_ODD_FRM          (0x1U << 12)
#define LAN91C96_BAD_CRC          (0x1U << 13)
#define LAN91C96_BROD_CAST        (0x1U << 14)
#define LAN91C96_ALGN_ERR         (0x1U << 15)

#define FRAME_FILTER              (LAN91C96_TOO_SHORT | LAN91C96_TOO_LONG  | LAN91C96_BAD_CRC   | LAN91C96_ALGN_ERR)

/*
 ****************************************************************************
 *	Default MAC Address
 ****************************************************************************
 */
#define MAC_DEF_HI  0x0800
#define MAC_DEF_MED 0x3333
#define MAC_DEF_LO  0x0100

/*
 ****************************************************************************
 *	Default I/O Signature - 0x33
 ****************************************************************************
 */
#define LAN91C96_LOW_SIGNATURE        (0x33U << 0)
#define LAN91C96_HIGH_SIGNATURE       (0x33U << 8)
#define LAN91C96_SIGNATURE (LAN91C96_HIGH_SIGNATURE | LAN91C96_LOW_SIGNATURE)

#define LAN91C96_MAX_PAGES     6        /* Maximum number of 256 pages. */
#define ETHERNET_MAX_LENGTH 1514


/*-------------------------------------------------------------------------
 *  I define some macros to make it easier to do somewhat common
 * or slightly complicated, repeated tasks.
 *-------------------------------------------------------------------------
 */

/* select a register bank, 0 to 3  */

#define SMC_SELECT_BANK(edev, x)  { SMC_outw(edev, x, LAN91C96_BANK_SELECT); }

/* this enables an interrupt in the interrupt mask register */
#define SMC_ENABLE_INT(edev, x) {\
		unsigned char mask;\
		SMC_SELECT_BANK(edev, 2);\
		mask = SMC_inb(edev, LAN91C96_INT_MASK);\
		mask |= (x);\
		SMC_outb(edev, mask, LAN91C96_INT_MASK); \
}

/* this disables an interrupt from the interrupt mask register */

#define SMC_DISABLE_INT(edev, x) {\
		unsigned char mask;\
		SMC_SELECT_BANK(edev, 2);\
		mask = SMC_inb(edev, LAN91C96_INT_MASK);\
		mask &= ~(x);\
		SMC_outb(edev, mask, LAN91C96_INT_MASK); \
}

/*----------------------------------------------------------------------
 * Define the interrupts that I want to receive from the card
 *
 * I want:
 *  LAN91C96_IST_EPH_INT, for nasty errors
 *  LAN91C96_IST_RCV_INT, for happy received packets
 *  LAN91C96_IST_RX_OVRN_INT, because I have to kick the receiver
 *-------------------------------------------------------------------------
 */
#define SMC_INTERRUPT_MASK   (LAN91C96_IST_EPH_INT | LAN91C96_IST_RX_OVRN_INT | LAN91C96_IST_RCV_INT)

#endif  /* _LAN91C96_H_ */
