/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * mcf5329.h -- Definitions for Freescale Coldfire 5329
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef mcf5329_h
#define mcf5329_h
/****************************************************************************/

/*********************************************************************
* System Control Module (SCM)
*********************************************************************/
/* Bit definitions and macros for SCM_MPR */
#define SCM_MPR_MPROT0(x)		(((x)&0x0F)<<28)
#define SCM_MPR_MPROT1(x)		(((x)&0x0F)<<24)
#define SCM_MPR_MPROT2(x)		(((x)&0x0F)<<20)
#define SCM_MPR_MPROT4(x)		(((x)&0x0F)<<12)
#define SCM_MPR_MPROT5(x)		(((x)&0x0F)<<8)
#define SCM_MPR_MPROT6(x)		(((x)&0x0F)<<4)
#define MPROT_MTR			4
#define MPROT_MTW			2
#define MPROT_MPL			1

/* Bit definitions and macros for SCM_BMT */
#define BMT_BME				(0x08)
#define BMT_8				(0x07)
#define BMT_16				(0x06)
#define BMT_32				(0x05)
#define BMT_64				(0x04)
#define BMT_128				(0x03)
#define BMT_256				(0x02)
#define BMT_512				(0x01)
#define BMT_1024			(0x00)

/* Bit definitions and macros for SCM_PACRA */
#define SCM_PACRA_PACR0(x)		(((x)&0x0F)<<28)
#define SCM_PACRA_PACR1(x)		(((x)&0x0F)<<24)
#define SCM_PACRA_PACR2(x)		(((x)&0x0F)<<20)
#define PACR_SP	4
#define PACR_WP	2
#define PACR_TP	1

/* Bit definitions and macros for SCM_PACRB */
#define SCM_PACRB_PACR8(x)		(((x)&0x0F)<<28)
#define SCM_PACRB_PACR12(x)		(((x)&0x0F)<<12)

/* Bit definitions and macros for SCM_PACRC */
#define SCM_PACRC_PACR16(x)		(((x)&0x0F)<<28)
#define SCM_PACRC_PACR17(x)		(((x)&0x0F)<<24)
#define SCM_PACRC_PACR18(x)		(((x)&0x0F)<<20)
#define SCM_PACRC_PACR19(x)		(((x)&0x0F)<<16)
#define SCM_PACRC_PACR21(x)		(((x)&0x0F)<<8)
#define SCM_PACRC_PACR22(x)		(((x)&0x0F)<<4)
#define SCM_PACRC_PACR23(x)		(((x)&0x0F)<<0)

/* Bit definitions and macros for SCM_PACRD */
#define SCM_PACRD_PACR24(x)		(((x)&0x0F)<<28)
#define SCM_PACRD_PACR25(x)		(((x)&0x0F)<<24)
#define SCM_PACRD_PACR26(x)		(((x)&0x0F)<<20)
#define SCM_PACRD_PACR28(x)		(((x)&0x0F)<<12)
#define SCM_PACRD_PACR29(x)		(((x)&0x0F)<<8)
#define SCM_PACRD_PACR30(x)		(((x)&0x0F)<<4)
#define SCM_PACRD_PACR31(x)		(((x)&0x0F)<<0)

/* Bit definitions and macros for SCM_PACRE */
#define SCM_PACRE_PACR32(x)		(((x)&0x0F)<<28)
#define SCM_PACRE_PACR33(x)		(((x)&0x0F)<<24)
#define SCM_PACRE_PACR34(x)		(((x)&0x0F)<<20)
#define SCM_PACRE_PACR35(x)		(((x)&0x0F)<<16)
#define SCM_PACRE_PACR36(x)		(((x)&0x0F)<<12)
#define SCM_PACRE_PACR37(x)		(((x)&0x0F)<<8)
#define SCM_PACRE_PACR38(x)		(((x)&0x0F)<<4)

/* Bit definitions and macros for SCM_PACRF */
#define SCM_PACRF_PACR40(x)		(((x)&0x0F)<<28)
#define SCM_PACRF_PACR41(x)		(((x)&0x0F)<<24)
#define SCM_PACRF_PACR42(x)		(((x)&0x0F)<<20)
#define SCM_PACRF_PACR43(x)		(((x)&0x0F)<<16)
#define SCM_PACRF_PACR44(x)		(((x)&0x0F)<<12)
#define SCM_PACRF_PACR45(x)		(((x)&0x0F)<<8)
#define SCM_PACRF_PACR46(x)		(((x)&0x0F)<<4)
#define SCM_PACRF_PACR47(x)		(((x)&0x0F)<<0)

/* Bit definitions and macros for SCM_PACRG */
#define SCM_PACRG_PACR48(x)		(((x)&0x0F)<<28)

/* Bit definitions and macros for SCM_PACRH */
#define SCM_PACRH_PACR56(x)		(((x)&0x0F)<<28)
#define SCM_PACRH_PACR57(x)		(((x)&0x0F)<<24)
#define SCM_PACRH_PACR58(x)		(((x)&0x0F)<<20)

/* PACRn Assignments */
#define PACR0(x)			SCM_PACRA_PACR0(x)
#define PACR1(x)			SCM_PACRA_PACR1(x)
#define PACR2(x)			SCM_PACRA_PACR2(x)
#define PACR8(x)			SCM_PACRB_PACR8(x)
#define PACR12(x)			SCM_PACRB_PACR12(x)
#define PACR16(x)			SCM_PACRC_PACR16(x)
#define PACR17(x)			SCM_PACRC_PACR17(x)
#define PACR18(x)			SCM_PACRC_PACR18(x)
#define PACR19(x)			SCM_PACRC_PACR19(x)
#define PACR21(x)			SCM_PACRC_PACR21(x)
#define PACR22(x)			SCM_PACRC_PACR22(x)
#define PACR23(x)			SCM_PACRC_PACR23(x)
#define PACR24(x)			SCM_PACRD_PACR24(x)
#define PACR25(x)			SCM_PACRD_PACR25(x)
#define PACR26(x)			SCM_PACRD_PACR26(x)
#define PACR28(x)			SCM_PACRD_PACR28(x)
#define PACR29(x)			SCM_PACRD_PACR29(x)
#define PACR30(x)			SCM_PACRD_PACR30(x)
#define PACR31(x)			SCM_PACRD_PACR31(x)
#define PACR32(x)			SCM_PACRE_PACR32(x)
#define PACR33(x)			SCM_PACRE_PACR33(x)
#define PACR34(x)			SCM_PACRE_PACR34(x)
#define PACR35(x)			SCM_PACRE_PACR35(x)
#define PACR36(x)			SCM_PACRE_PACR36(x)
#define PACR37(x)			SCM_PACRE_PACR37(x)
#define PACR38(x)			SCM_PACRE_PACR38(x)
#define PACR40(x)			SCM_PACRF_PACR40(x)
#define PACR41(x)			SCM_PACRF_PACR41(x)
#define PACR42(x)			SCM_PACRF_PACR42(x)
#define PACR43(x)			SCM_PACRF_PACR43(x)
#define PACR44(x)			SCM_PACRF_PACR44(x)
#define PACR45(x)			SCM_PACRF_PACR45(x)
#define PACR46(x)			SCM_PACRF_PACR46(x)
#define PACR47(x)			SCM_PACRF_PACR47(x)
#define PACR48(x)			SCM_PACRG_PACR48(x)
#define PACR56(x)			SCM_PACRH_PACR56(x)
#define PACR57(x)			SCM_PACRH_PACR57(x)
#define PACR58(x)			SCM_PACRH_PACR58(x)

/* Bit definitions and macros for SCM_CWCR */
#define CWCR_RO				(0x8000)
#define CWCR_CWR_WH			(0x0100)
#define CWCR_CWE			(0x0080)
#define CWRI_WINDOW			(0x0060)
#define CWRI_RESET			(0x0040)
#define CWRI_INT_RESET			(0x0020)
#define CWRI_INT			(0x0000)
#define CWCR_CWT(x)			(((x)&0x001F))

/* Bit definitions and macros for SCM_ISR */
#define SCMISR_CFEI			(0x02)
#define SCMISR_CWIC			(0x01)

/* Bit definitions and macros for SCM_BCR */
#define BCR_GBR				(0x00000200)
#define BCR_GBW				(0x00000100)
#define BCR_S7				(0x00000080)
#define BCR_S6				(0x00000040)
#define BCR_S4				(0x00000010)
#define BCR_S1				(0x00000002)

/* Bit definitions and macros for SCM_CFIER */
#define CFIER_ECFEI			(0x01)

/* Bit definitions and macros for SCM_CFLOC */
#define CFLOC_LOC			(0x80)

/* Bit definitions and macros for SCM_CFATR */
#define CFATR_WRITE			(0x80)
#define CFATR_SZ32			(0x20)
#define CFATR_SZ16			(0x10)
#define CFATR_SZ08			(0x00)
#define CFATR_CACHE			(0x08)
#define CFATR_MODE			(0x02)
#define CFATR_TYPE			(0x01)

/*********************************************************************
* Reset Controller Module (RCM)
*********************************************************************/

/* Bit definitions and macros for RCR */
#define RCM_RCR_FRCRSTOUT		(0x40)
#define RCM_RCR_SOFTRST			(0x80)

/* Bit definitions and macros for RSR */
#define RCM_RSR_LOL			(0x01)
#define RCM_RSR_WDR_CORE		(0x02)
#define RCM_RSR_EXT			(0x04)
#define RCM_RSR_POR			(0x08)
#define RCM_RSR_SOFT			(0x20)

/*********************************************************************
* Interrupt Controller (INTC)
*********************************************************************/
#define INTC0_EPORT			INTC_IPRL_INT1

#define INT0_LO_RSVD0			(0)
#define INT0_LO_EPORT1			(1)
#define INT0_LO_EPORT2			(2)
#define INT0_LO_EPORT3			(3)
#define INT0_LO_EPORT4			(4)
#define INT0_LO_EPORT5			(5)
#define INT0_LO_EPORT6			(6)
#define INT0_LO_EPORT7			(7)
#define INT0_LO_EDMA_00			(8)
#define INT0_LO_EDMA_01			(9)
#define INT0_LO_EDMA_02			(10)
#define INT0_LO_EDMA_03			(11)
#define INT0_LO_EDMA_04			(12)
#define INT0_LO_EDMA_05			(13)
#define INT0_LO_EDMA_06			(14)
#define INT0_LO_EDMA_07			(15)
#define INT0_LO_EDMA_08			(16)
#define INT0_LO_EDMA_09			(17)
#define INT0_LO_EDMA_10			(18)
#define INT0_LO_EDMA_11			(19)
#define INT0_LO_EDMA_12			(20)
#define INT0_LO_EDMA_13			(21)
#define INT0_LO_EDMA_14			(22)
#define INT0_LO_EDMA_15			(23)
#define INT0_LO_EDMA_ERR		(24)
#define INT0_LO_SCM			(25)
#define INT0_LO_UART0			(26)
#define INT0_LO_UART1			(27)
#define INT0_LO_UART2			(28)
#define INT0_LO_RSVD1			(29)
#define INT0_LO_I2C			(30)
#define INT0_LO_QSPI			(31)
#define INT0_HI_DTMR0			(32)
#define INT0_HI_DTMR1			(33)
#define INT0_HI_DTMR2			(34)
#define INT0_HI_DTMR3			(35)
#define INT0_HI_FEC_TXF			(36)
#define INT0_HI_FEC_TXB			(37)
#define INT0_HI_FEC_UN			(38)
#define INT0_HI_FEC_RL			(39)
#define INT0_HI_FEC_RXF			(40)
#define INT0_HI_FEC_RXB			(41)
#define INT0_HI_FEC_MII			(42)
#define INT0_HI_FEC_LC			(43)
#define INT0_HI_FEC_HBERR		(44)
#define INT0_HI_FEC_GRA			(45)
#define INT0_HI_FEC_EBERR		(46)
#define INT0_HI_FEC_BABT		(47)
#define INT0_HI_FEC_BABR		(48)
/* 49 - 61 Reserved */
#define INT0_HI_SCM			(62)

/*********************************************************************
* Watchdog Timer Modules (WTM)
*********************************************************************/
/* Bit definitions and macros for WTM_WCR */
#define WTM_WCR_WAIT			(0x0008)
#define WTM_WCR_DOZE			(0x0004)
#define WTM_WCR_HALTED			(0x0002)
#define WTM_WCR_EN			(0x0001)

/*********************************************************************
* Chip Configuration Module (CCM)
*********************************************************************/
/* Bit definitions and macros for CCM_CCR */
#define CCM_CCR_CSC(x)			(((x)&0x0003)<<8|0x0001)
#define CCM_CCR_LIMP			(0x0041)
#define CCM_CCR_LOAD			(0x0021)
#define CCM_CCR_BOOTPS(x)		(((x)&0x0003)<<3|0x0001)
#define CCM_CCR_OSC_MODE		(0x0005)
#define CCM_CCR_PLL_MODE		(0x0003)
#define CCM_CCR_RESERVED		(0x0001)

/* Bit definitions and macros for CCM_RCON */
#define CCM_RCON_CSC(x)			(((x)&0x0003)<<8|0x0001)
#define CCM_RCON_LIMP			(0x0041)
#define CCM_RCON_LOAD			(0x0021)
#define CCM_RCON_BOOTPS(x)		(((x)&0x0003)<<3|0x0001)
#define CCM_RCON_OSC_MODE		(0x0005)
#define CCM_RCON_PLL_MODE		(0x0003)
#define CCM_RCON_RESERVED		(0x0001)

/* Bit definitions and macros for CCM_CIR */
#define CCM_CIR_PIN(x)			(((x)&0x03FF)<<6)
#define CCM_CIR_PRN(x)			((x)&0x003F)

/* Bit definitions and macros for CCM_MISCCR */
#define CCM_MISCCR_PLL_LOCK		(0x2000)
#define CCM_MISCCR_LIMP			(0x1000)
#define CCM_MISCCR_LCD_CHEN		(0x0100)
#define CCM_MISCCR_SSI_PUE		(0x0080)
#define CCM_MISCCR_SSI_PUS		(0x0040)
#define CCM_MISCCR_TIM_DMA		(0x0020)
#define CCM_MISCCR_SSI_SRC		(0x0010)
#define CCM_MISCCR_USBDIV		(0x0002)
#define CCM_MISCCR_USBSRC		(0x0001)

/* Bit definitions and macros for CCM_CDR */
#define CCM_CDR_LPDIV(x)		(((x)&0x000F)<<8)
#define CCM_CDR_SSIDIV(x)		((x)&0x000F)

/* Bit definitions and macros for CCM_UHCSR */
#define CCM_UHCSR_PORTIND(x)		(((x)&0x0003)<<14)
#define CCM_UHCSR_WKUP			(0x0004)
#define CCM_UHCSR_UHMIE			(0x0002)
#define CCM_UHCSR_XPDE			(0x0001)

/* Bit definitions and macros for CCM_UOCSR */
#define CCM_UOCSR_PORTIND(x)		(((x)&0x0003)<<14)
#define CCM_UOCSR_DPPD			(0x2000)
#define CCM_UOCSR_DMPD			(0x1000)
#define CCM_UOCSR_DRV_VBUS		(0x0800)
#define CCM_UOCSR_CRG_VBUS		(0x0400)
#define CCM_UOCSR_DCR_VBUS		(0x0200)
#define CCM_UOCSR_DPPU			(0x0100)
#define CCM_UOCSR_AVLD			(0x0080)
#define CCM_UOCSR_BVLD			(0x0040)
#define CCM_UOCSR_VVLD			(0x0020)
#define CCM_UOCSR_SEND			(0x0010)
#define CCM_UOCSR_PWRFLT		(0x0008)
#define CCM_UOCSR_WKUP			(0x0004)
#define CCM_UOCSR_UOMIE			(0x0002)
#define CCM_UOCSR_XPDE			(0x0001)

/* not done yet */
/*********************************************************************
* General Purpose I/O (GPIO)
*********************************************************************/
/* Bit definitions and macros for GPIO_PODR_FECH_L */
#define GPIO_PODR_FECH_L7		(0x80)
#define GPIO_PODR_FECH_L6		(0x40)
#define GPIO_PODR_FECH_L5		(0x20)
#define GPIO_PODR_FECH_L4		(0x10)
#define GPIO_PODR_FECH_L3		(0x08)
#define GPIO_PODR_FECH_L2		(0x04)
#define GPIO_PODR_FECH_L1		(0x02)
#define GPIO_PODR_FECH_L0		(0x01)

/* Bit definitions and macros for GPIO_PODR_SSI */
#define GPIO_PODR_SSI_4			(0x10)
#define GPIO_PODR_SSI_3			(0x08)
#define GPIO_PODR_SSI_2			(0x04)
#define GPIO_PODR_SSI_1			(0x02)
#define GPIO_PODR_SSI_0			(0x01)

/* Bit definitions and macros for GPIO_PODR_BUSCTL */
#define GPIO_PODR_BUSCTL_3		(0x08)
#define GPIO_PODR_BUSCTL_2		(0x04)
#define GPIO_PODR_BUSCTL_1		(0x02)
#define GPIO_PODR_BUSCTL_0		(0x01)

/* Bit definitions and macros for GPIO_PODR_BE */
#define GPIO_PODR_BE_3			(0x08)
#define GPIO_PODR_BE_2			(0x04)
#define GPIO_PODR_BE_1			(0x02)
#define GPIO_PODR_BE_0			(0x01)

/* Bit definitions and macros for GPIO_PODR_CS */
#define GPIO_PODR_CS_5			(0x20)
#define GPIO_PODR_CS_4			(0x10)
#define GPIO_PODR_CS_3			(0x08)
#define GPIO_PODR_CS_2			(0x04)
#define GPIO_PODR_CS_1			(0x02)

/* Bit definitions and macros for GPIO_PODR_PWM */
#define GPIO_PODR_PWM_5			(0x20)
#define GPIO_PODR_PWM_4			(0x10)
#define GPIO_PODR_PWM_3			(0x08)
#define GPIO_PODR_PWM_2			(0x04)

/* Bit definitions and macros for GPIO_PODR_FECI2C */
#define GPIO_PODR_FECI2C_3		(0x08)
#define GPIO_PODR_FECI2C_2		(0x04)
#define GPIO_PODR_FECI2C_1		(0x02)
#define GPIO_PODR_FECI2C_0		(0x01)

/* Bit definitions and macros for GPIO_PODR_UART */
#define GPIO_PODR_UART_7		(0x80)
#define GPIO_PODR_UART_6		(0x40)
#define GPIO_PODR_UART_5		(0x20)
#define GPIO_PODR_UART_4		(0x10)
#define GPIO_PODR_UART_3		(0x08)
#define GPIO_PODR_UART_2		(0x04)
#define GPIO_PODR_UART_1		(0x02)
#define GPIO_PODR_UART_0		(0x01)

/* Bit definitions and macros for GPIO_PODR_QSPI */
#define GPIO_PODR_QSPI_5		(0x20)
#define GPIO_PODR_QSPI_4		(0x10)
#define GPIO_PODR_QSPI_3		(0x08)
#define GPIO_PODR_QSPI_2		(0x04)
#define GPIO_PODR_QSPI_1		(0x02)
#define GPIO_PODR_QSPI_0		(0x01)

/* Bit definitions and macros for GPIO_PODR_TIMER */
#define GPIO_PODR_TIMER_3		(0x08)
#define GPIO_PODR_TIMER_2		(0x04)
#define GPIO_PODR_TIMER_1		(0x02)
#define GPIO_PODR_TIMER_0		(0x01)

/* Bit definitions and macros for GPIO_PODR_LCDDATAH */
#define GPIO_PODR_LCDDATAH_1		(0x02)
#define GPIO_PODR_LCDDATAH_0		(0x01)

/* Bit definitions and macros for GPIO_PODR_LCDDATAM */
#define GPIO_PODR_LCDDATAM_7		(0x80)
#define GPIO_PODR_LCDDATAM_6		(0x40)
#define GPIO_PODR_LCDDATAM_5		(0x20)
#define GPIO_PODR_LCDDATAM_4		(0x10)
#define GPIO_PODR_LCDDATAM_3		(0x08)
#define GPIO_PODR_LCDDATAM_2		(0x04)
#define GPIO_PODR_LCDDATAM_1		(0x02)
#define GPIO_PODR_LCDDATAM_0		(0x01)

/* Bit definitions and macros for GPIO_PODR_LCDDATAL */
#define GPIO_PODR_LCDDATAL_7		(0x80)
#define GPIO_PODR_LCDDATAL_6		(0x40)
#define GPIO_PODR_LCDDATAL_5		(0x20)
#define GPIO_PODR_LCDDATAL_4		(0x10)
#define GPIO_PODR_LCDDATAL_3		(0x08)
#define GPIO_PODR_LCDDATAL_2		(0x04)
#define GPIO_PODR_LCDDATAL_1		(0x02)
#define GPIO_PODR_LCDDATAL_0		(0x01)

/* Bit definitions and macros for GPIO_PODR_LCDCTLH */
#define GPIO_PODR_LCDCTLH_0		(0x01)

/* Bit definitions and macros for GPIO_PODR_LCDCTLL */
#define GPIO_PODR_LCDCTLL_7		(0x80)
#define GPIO_PODR_LCDCTLL_6		(0x40)
#define GPIO_PODR_LCDCTLL_5		(0x20)
#define GPIO_PODR_LCDCTLL_4		(0x10)
#define GPIO_PODR_LCDCTLL_3		(0x08)
#define GPIO_PODR_LCDCTLL_2		(0x04)
#define GPIO_PODR_LCDCTLL_1		(0x02)
#define GPIO_PODR_LCDCTLL_0		(0x01)

/* Bit definitions and macros for GPIO_PDDR_FECH */
#define GPIO_PDDR_FECH_L7		(0x80)
#define GPIO_PDDR_FECH_L6		(0x40)
#define GPIO_PDDR_FECH_L5		(0x20)
#define GPIO_PDDR_FECH_L4		(0x10)
#define GPIO_PDDR_FECH_L3		(0x08)
#define GPIO_PDDR_FECH_L2		(0x04)
#define GPIO_PDDR_FECH_L1		(0x02)
#define GPIO_PDDR_FECH_L0		(0x01)

/* Bit definitions and macros for GPIO_PDDR_SSI */
#define GPIO_PDDR_SSI_4			(0x10)
#define GPIO_PDDR_SSI_3			(0x08)
#define GPIO_PDDR_SSI_2			(0x04)
#define GPIO_PDDR_SSI_1			(0x02)
#define GPIO_PDDR_SSI_0			(0x01)

/* Bit definitions and macros for GPIO_PDDR_BUSCTL */
#define GPIO_PDDR_BUSCTL_3		(0x08)
#define GPIO_PDDR_BUSCTL_2		(0x04)
#define GPIO_PDDR_BUSCTL_1		(0x02)
#define GPIO_PDDR_BUSCTL_0		(0x01)

/* Bit definitions and macros for GPIO_PDDR_BE */
#define GPIO_PDDR_BE_3			(0x08)
#define GPIO_PDDR_BE_2			(0x04)
#define GPIO_PDDR_BE_1			(0x02)
#define GPIO_PDDR_BE_0			(0x01)

/* Bit definitions and macros for GPIO_PDDR_CS */
#define GPIO_PDDR_CS_1			(0x02)
#define GPIO_PDDR_CS_2			(0x04)
#define GPIO_PDDR_CS_3			(0x08)
#define GPIO_PDDR_CS_4			(0x10)
#define GPIO_PDDR_CS_5			(0x20)

/* Bit definitions and macros for GPIO_PDDR_PWM */
#define GPIO_PDDR_PWM_2			(0x04)
#define GPIO_PDDR_PWM_3			(0x08)
#define GPIO_PDDR_PWM_4			(0x10)
#define GPIO_PDDR_PWM_5			(0x20)

/* Bit definitions and macros for GPIO_PDDR_FECI2C */
#define GPIO_PDDR_FECI2C_0		(0x01)
#define GPIO_PDDR_FECI2C_1		(0x02)
#define GPIO_PDDR_FECI2C_2		(0x04)
#define GPIO_PDDR_FECI2C_3		(0x08)

/* Bit definitions and macros for GPIO_PDDR_UART */
#define GPIO_PDDR_UART_0		(0x01)
#define GPIO_PDDR_UART_1		(0x02)
#define GPIO_PDDR_UART_2		(0x04)
#define GPIO_PDDR_UART_3		(0x08)
#define GPIO_PDDR_UART_4		(0x10)
#define GPIO_PDDR_UART_5		(0x20)
#define GPIO_PDDR_UART_6		(0x40)
#define GPIO_PDDR_UART_7		(0x80)

/* Bit definitions and macros for GPIO_PDDR_QSPI */
#define GPIO_PDDR_QSPI_0		(0x01)
#define GPIO_PDDR_QSPI_1		(0x02)
#define GPIO_PDDR_QSPI_2		(0x04)
#define GPIO_PDDR_QSPI_3		(0x08)
#define GPIO_PDDR_QSPI_4		(0x10)
#define GPIO_PDDR_QSPI_5		(0x20)

/* Bit definitions and macros for GPIO_PDDR_TIMER */
#define GPIO_PDDR_TIMER_0		(0x01)
#define GPIO_PDDR_TIMER_1		(0x02)
#define GPIO_PDDR_TIMER_2		(0x04)
#define GPIO_PDDR_TIMER_3		(0x08)

/* Bit definitions and macros for GPIO_PDDR_LCDDATAH */
#define GPIO_PDDR_LCDDATAH_0		(0x01)
#define GPIO_PDDR_LCDDATAH_1		(0x02)

/* Bit definitions and macros for GPIO_PDDR_LCDDATAM */
#define GPIO_PDDR_LCDDATAM_0		(0x01)
#define GPIO_PDDR_LCDDATAM_1		(0x02)
#define GPIO_PDDR_LCDDATAM_2		(0x04)
#define GPIO_PDDR_LCDDATAM_3		(0x08)
#define GPIO_PDDR_LCDDATAM_4		(0x10)
#define GPIO_PDDR_LCDDATAM_5		(0x20)
#define GPIO_PDDR_LCDDATAM_6		(0x40)
#define GPIO_PDDR_LCDDATAM_7		(0x80)

/* Bit definitions and macros for GPIO_PDDR_LCDDATAL */
#define GPIO_PDDR_LCDDATAL_0		(0x01)
#define GPIO_PDDR_LCDDATAL_1		(0x02)
#define GPIO_PDDR_LCDDATAL_2		(0x04)
#define GPIO_PDDR_LCDDATAL_3		(0x08)
#define GPIO_PDDR_LCDDATAL_4		(0x10)
#define GPIO_PDDR_LCDDATAL_5		(0x20)
#define GPIO_PDDR_LCDDATAL_6		(0x40)
#define GPIO_PDDR_LCDDATAL_7		(0x80)

/* Bit definitions and macros for GPIO_PDDR_LCDCTLH */
#define GPIO_PDDR_LCDCTLH_0		(0x01)

/* Bit definitions and macros for GPIO_PDDR_LCDCTLL */
#define GPIO_PDDR_LCDCTLL_0		(0x01)
#define GPIO_PDDR_LCDCTLL_1		(0x02)
#define GPIO_PDDR_LCDCTLL_2		(0x04)
#define GPIO_PDDR_LCDCTLL_3		(0x08)
#define GPIO_PDDR_LCDCTLL_4		(0x10)
#define GPIO_PDDR_LCDCTLL_5		(0x20)
#define GPIO_PDDR_LCDCTLL_6		(0x40)
#define GPIO_PDDR_LCDCTLL_7		(0x80)

/* Bit definitions and macros for GPIO_PPDSDR_FECH */
#define GPIO_PPDSDR_FECH_L0		(0x01)
#define GPIO_PPDSDR_FECH_L1		(0x02)
#define GPIO_PPDSDR_FECH_L2		(0x04)
#define GPIO_PPDSDR_FECH_L3		(0x08)
#define GPIO_PPDSDR_FECH_L4		(0x10)
#define GPIO_PPDSDR_FECH_L5		(0x20)
#define GPIO_PPDSDR_FECH_L6		(0x40)
#define GPIO_PPDSDR_FECH_L7		(0x80)

/* Bit definitions and macros for GPIO_PPDSDR_SSI */
#define GPIO_PPDSDR_SSI_0		(0x01)
#define GPIO_PPDSDR_SSI_1		(0x02)
#define GPIO_PPDSDR_SSI_2		(0x04)
#define GPIO_PPDSDR_SSI_3		(0x08)
#define GPIO_PPDSDR_SSI_4		(0x10)

/* Bit definitions and macros for GPIO_PPDSDR_BUSCTL */
#define GPIO_PPDSDR_BUSCTL_0		(0x01)
#define GPIO_PPDSDR_BUSCTL_1		(0x02)
#define GPIO_PPDSDR_BUSCTL_2		(0x04)
#define GPIO_PPDSDR_BUSCTL_3		(0x08)

/* Bit definitions and macros for GPIO_PPDSDR_BE */
#define GPIO_PPDSDR_BE_0		(0x01)
#define GPIO_PPDSDR_BE_1		(0x02)
#define GPIO_PPDSDR_BE_2		(0x04)
#define GPIO_PPDSDR_BE_3		(0x08)

/* Bit definitions and macros for GPIO_PPDSDR_CS */
#define GPIO_PPDSDR_CS_1		(0x02)
#define GPIO_PPDSDR_CS_2		(0x04)
#define GPIO_PPDSDR_CS_3		(0x08)
#define GPIO_PPDSDR_CS_4		(0x10)
#define GPIO_PPDSDR_CS_5		(0x20)

/* Bit definitions and macros for GPIO_PPDSDR_PWM */
#define GPIO_PPDSDR_PWM_2		(0x04)
#define GPIO_PPDSDR_PWM_3		(0x08)
#define GPIO_PPDSDR_PWM_4		(0x10)
#define GPIO_PPDSDR_PWM_5		(0x20)

/* Bit definitions and macros for GPIO_PPDSDR_FECI2C */
#define GPIO_PPDSDR_FECI2C_0		(0x01)
#define GPIO_PPDSDR_FECI2C_1		(0x02)
#define GPIO_PPDSDR_FECI2C_2		(0x04)
#define GPIO_PPDSDR_FECI2C_3		(0x08)

/* Bit definitions and macros for GPIO_PPDSDR_UART */
#define GPIO_PPDSDR_UART_0		(0x01)
#define GPIO_PPDSDR_UART_1		(0x02)
#define GPIO_PPDSDR_UART_2		(0x04)
#define GPIO_PPDSDR_UART_3		(0x08)
#define GPIO_PPDSDR_UART_4		(0x10)
#define GPIO_PPDSDR_UART_5		(0x20)
#define GPIO_PPDSDR_UART_6		(0x40)
#define GPIO_PPDSDR_UART_7		(0x80)

/* Bit definitions and macros for GPIO_PPDSDR_QSPI */
#define GPIO_PPDSDR_QSPI_0		(0x01)
#define GPIO_PPDSDR_QSPI_1		(0x02)
#define GPIO_PPDSDR_QSPI_2		(0x04)
#define GPIO_PPDSDR_QSPI_3		(0x08)
#define GPIO_PPDSDR_QSPI_4		(0x10)
#define GPIO_PPDSDR_QSPI_5		(0x20)

/* Bit definitions and macros for GPIO_PPDSDR_TIMER */
#define GPIO_PPDSDR_TIMER_0		(0x01)
#define GPIO_PPDSDR_TIMER_1		(0x02)
#define GPIO_PPDSDR_TIMER_2		(0x04)
#define GPIO_PPDSDR_TIMER_3		(0x08)

/* Bit definitions and macros for GPIO_PPDSDR_LCDDATAH */
#define GPIO_PPDSDR_LCDDATAH_0		(0x01)
#define GPIO_PPDSDR_LCDDATAH_1		(0x02)

/* Bit definitions and macros for GPIO_PPDSDR_LCDDATAM */
#define GPIO_PPDSDR_LCDDATAM_0		(0x01)
#define GPIO_PPDSDR_LCDDATAM_1		(0x02)
#define GPIO_PPDSDR_LCDDATAM_2		(0x04)
#define GPIO_PPDSDR_LCDDATAM_3		(0x08)
#define GPIO_PPDSDR_LCDDATAM_4		(0x10)
#define GPIO_PPDSDR_LCDDATAM_5		(0x20)
#define GPIO_PPDSDR_LCDDATAM_6		(0x40)
#define GPIO_PPDSDR_LCDDATAM_7		(0x80)

/* Bit definitions and macros for GPIO_PPDSDR_LCDDATAL */
#define GPIO_PPDSDR_LCDDATAL_0		(0x01)
#define GPIO_PPDSDR_LCDDATAL_1		(0x02)
#define GPIO_PPDSDR_LCDDATAL_2		(0x04)
#define GPIO_PPDSDR_LCDDATAL_3		(0x08)
#define GPIO_PPDSDR_LCDDATAL_4		(0x10)
#define GPIO_PPDSDR_LCDDATAL_5		(0x20)
#define GPIO_PPDSDR_LCDDATAL_6		(0x40)
#define GPIO_PPDSDR_LCDDATAL_7		(0x80)

/* Bit definitions and macros for GPIO_PPDSDR_LCDCTLH */
#define GPIO_PPDSDR_LCDCTLH_0		(0x01)

/* Bit definitions and macros for GPIO_PPDSDR_LCDCTLL */
#define GPIO_PPDSDR_LCDCTLL_0		(0x01)
#define GPIO_PPDSDR_LCDCTLL_1		(0x02)
#define GPIO_PPDSDR_LCDCTLL_2		(0x04)
#define GPIO_PPDSDR_LCDCTLL_3		(0x08)
#define GPIO_PPDSDR_LCDCTLL_4		(0x10)
#define GPIO_PPDSDR_LCDCTLL_5		(0x20)
#define GPIO_PPDSDR_LCDCTLL_6		(0x40)
#define GPIO_PPDSDR_LCDCTLL_7		(0x80)

/* Bit definitions and macros for GPIO_PCLRR_FECH */
#define GPIO_PCLRR_FECH_L0		(0x01)
#define GPIO_PCLRR_FECH_L1		(0x02)
#define GPIO_PCLRR_FECH_L2		(0x04)
#define GPIO_PCLRR_FECH_L3		(0x08)
#define GPIO_PCLRR_FECH_L4		(0x10)
#define GPIO_PCLRR_FECH_L5		(0x20)
#define GPIO_PCLRR_FECH_L6		(0x40)
#define GPIO_PCLRR_FECH_L7		(0x80)

/* Bit definitions and macros for GPIO_PCLRR_SSI */
#define GPIO_PCLRR_SSI_0		(0x01)
#define GPIO_PCLRR_SSI_1		(0x02)
#define GPIO_PCLRR_SSI_2		(0x04)
#define GPIO_PCLRR_SSI_3		(0x08)
#define GPIO_PCLRR_SSI_4		(0x10)

/* Bit definitions and macros for GPIO_PCLRR_BUSCTL */
#define GPIO_PCLRR_BUSCTL_L0		(0x01)
#define GPIO_PCLRR_BUSCTL_L1		(0x02)
#define GPIO_PCLRR_BUSCTL_L2		(0x04)
#define GPIO_PCLRR_BUSCTL_L3		(0x08)

/* Bit definitions and macros for GPIO_PCLRR_BE */
#define GPIO_PCLRR_BE_0			(0x01)
#define GPIO_PCLRR_BE_1			(0x02)
#define GPIO_PCLRR_BE_2			(0x04)
#define GPIO_PCLRR_BE_3			(0x08)

/* Bit definitions and macros for GPIO_PCLRR_CS */
#define GPIO_PCLRR_CS_1			(0x02)
#define GPIO_PCLRR_CS_2			(0x04)
#define GPIO_PCLRR_CS_3			(0x08)
#define GPIO_PCLRR_CS_4			(0x10)
#define GPIO_PCLRR_CS_5			(0x20)

/* Bit definitions and macros for GPIO_PCLRR_PWM */
#define GPIO_PCLRR_PWM_2		(0x04)
#define GPIO_PCLRR_PWM_3		(0x08)
#define GPIO_PCLRR_PWM_4		(0x10)
#define GPIO_PCLRR_PWM_5		(0x20)

/* Bit definitions and macros for GPIO_PCLRR_FECI2C */
#define GPIO_PCLRR_FECI2C_0		(0x01)
#define GPIO_PCLRR_FECI2C_1		(0x02)
#define GPIO_PCLRR_FECI2C_2		(0x04)
#define GPIO_PCLRR_FECI2C_3		(0x08)

/* Bit definitions and macros for GPIO_PCLRR_UART */
#define GPIO_PCLRR_UART0		(0x01)
#define GPIO_PCLRR_UART1		(0x02)
#define GPIO_PCLRR_UART2		(0x04)
#define GPIO_PCLRR_UART3		(0x08)
#define GPIO_PCLRR_UART4		(0x10)
#define GPIO_PCLRR_UART5		(0x20)
#define GPIO_PCLRR_UART6		(0x40)
#define GPIO_PCLRR_UART7		(0x80)

/* Bit definitions and macros for GPIO_PCLRR_QSPI */
#define GPIO_PCLRR_QSPI0		(0x01)
#define GPIO_PCLRR_QSPI1		(0x02)
#define GPIO_PCLRR_QSPI2		(0x04)
#define GPIO_PCLRR_QSPI3		(0x08)
#define GPIO_PCLRR_QSPI4		(0x10)
#define GPIO_PCLRR_QSPI5		(0x20)

/* Bit definitions and macros for GPIO_PCLRR_TIMER */
#define GPIO_PCLRR_TIMER0		(0x01)
#define GPIO_PCLRR_TIMER1		(0x02)
#define GPIO_PCLRR_TIMER2		(0x04)
#define GPIO_PCLRR_TIMER3		(0x08)

/* Bit definitions and macros for GPIO_PCLRR_LCDDATAH */
#define GPIO_PCLRR_LCDDATAH0		(0x01)
#define GPIO_PCLRR_LCDDATAH1		(0x02)

/* Bit definitions and macros for GPIO_PCLRR_LCDDATAM */
#define GPIO_PCLRR_LCDDATAM0		(0x01)
#define GPIO_PCLRR_LCDDATAM1		(0x02)
#define GPIO_PCLRR_LCDDATAM2		(0x04)
#define GPIO_PCLRR_LCDDATAM3		(0x08)
#define GPIO_PCLRR_LCDDATAM4		(0x10)
#define GPIO_PCLRR_LCDDATAM5		(0x20)
#define GPIO_PCLRR_LCDDATAM6		(0x40)
#define GPIO_PCLRR_LCDDATAM7		(0x80)

/* Bit definitions and macros for GPIO_PCLRR_LCDDATAL */
#define GPIO_PCLRR_LCDDATAL0		(0x01)
#define GPIO_PCLRR_LCDDATAL1		(0x02)
#define GPIO_PCLRR_LCDDATAL2		(0x04)
#define GPIO_PCLRR_LCDDATAL3		(0x08)
#define GPIO_PCLRR_LCDDATAL4		(0x10)
#define GPIO_PCLRR_LCDDATAL5		(0x20)
#define GPIO_PCLRR_LCDDATAL6		(0x40)
#define GPIO_PCLRR_LCDDATAL7		(0x80)

/* Bit definitions and macros for GPIO_PCLRR_LCDCTLH */
#define GPIO_PCLRR_LCDCTLH_PCLRR_LCDCTLH0		(0x01)

/* Bit definitions and macros for GPIO_PCLRR_LCDCTLL */
#define GPIO_PCLRR_LCDCTLL0		(0x01)
#define GPIO_PCLRR_LCDCTLL1		(0x02)
#define GPIO_PCLRR_LCDCTLL2		(0x04)
#define GPIO_PCLRR_LCDCTLL3		(0x08)
#define GPIO_PCLRR_LCDCTLL4		(0x10)
#define GPIO_PCLRR_LCDCTLL5		(0x20)
#define GPIO_PCLRR_LCDCTLL6		(0x40)
#define GPIO_PCLRR_LCDCTLL7		(0x80)

/* Bit definitions and macros for GPIO_PAR_FEC */
#ifdef CONFIG_M5329
#define GPIO_PAR_FEC_MII(x)		(((x)&0x03)<<0)
#define GPIO_PAR_FEC_7W(x)		(((x)&0x03)<<2)
#define GPIO_PAR_FEC_7W_GPIO		(0x00)
#define GPIO_PAR_FEC_7W_URTS1		(0x04)
#define GPIO_PAR_FEC_7W_FEC		(0x0C)
#define GPIO_PAR_FEC_MII_GPIO		(0x00)
#define GPIO_PAR_FEC_MII_UART		(0x01)
#define GPIO_PAR_FEC_MII_FEC		(0x03)
#else
#define GPIO_PAR_FEC_7W_FEC		(0x08)
#define GPIO_PAR_FEC_MII_FEC		(0x02)
#endif

/* Bit definitions and macros for GPIO_PAR_PWM */
#define GPIO_PAR_PWM1(x)		(((x)&0x03)<<0)
#define GPIO_PAR_PWM3(x)		(((x)&0x03)<<2)
#define GPIO_PAR_PWM5			(0x10)
#define GPIO_PAR_PWM7			(0x20)

/* Bit definitions and macros for GPIO_PAR_BUSCTL */
#define GPIO_PAR_BUSCTL_TS(x)		(((x)&0x03)<<3)
#define GPIO_PAR_BUSCTL_RWB		(0x20)
#define GPIO_PAR_BUSCTL_TA		(0x40)
#define GPIO_PAR_BUSCTL_OE		(0x80)
#define GPIO_PAR_BUSCTL_OE_GPIO		(0x00)
#define GPIO_PAR_BUSCTL_OE_OE		(0x80)
#define GPIO_PAR_BUSCTL_TA_GPIO		(0x00)
#define GPIO_PAR_BUSCTL_TA_TA		(0x40)
#define GPIO_PAR_BUSCTL_RWB_GPIO	(0x00)
#define GPIO_PAR_BUSCTL_RWB_RWB		(0x20)
#define GPIO_PAR_BUSCTL_TS_GPIO		(0x00)
#define GPIO_PAR_BUSCTL_TS_DACK0	(0x10)
#define GPIO_PAR_BUSCTL_TS_TS		(0x18)

/* Bit definitions and macros for GPIO_PAR_FECI2C */
#define GPIO_PAR_FECI2C_SDA(x)		(((x)&0x03)<<0)
#define GPIO_PAR_FECI2C_SCL(x)		(((x)&0x03)<<2)
#define GPIO_PAR_FECI2C_MDIO(x)		(((x)&0x03)<<4)
#define GPIO_PAR_FECI2C_MDC(x)		(((x)&0x03)<<6)
#define GPIO_PAR_FECI2C_MDC_GPIO	(0x00)
#define GPIO_PAR_FECI2C_MDC_UTXD2	(0x40)
#define GPIO_PAR_FECI2C_MDC_SCL		(0x80)
#define GPIO_PAR_FECI2C_MDC_EMDC	(0xC0)
#define GPIO_PAR_FECI2C_MDIO_GPIO	(0x00)
#define GPIO_PAR_FECI2C_MDIO_URXD2	(0x10)
#define GPIO_PAR_FECI2C_MDIO_SDA	(0x20)
#define GPIO_PAR_FECI2C_MDIO_EMDIO	(0x30)
#define GPIO_PAR_FECI2C_SCL_GPIO	(0x00)
#define GPIO_PAR_FECI2C_SCL_UTXD2	(0x04)
#define GPIO_PAR_FECI2C_SCL_SCL		(0x0C)
#define GPIO_PAR_FECI2C_SDA_GPIO	(0x00)
#define GPIO_PAR_FECI2C_SDA_URXD2	(0x02)
#define GPIO_PAR_FECI2C_SDA_SDA		(0x03)

/* Bit definitions and macros for GPIO_PAR_BE */
#define GPIO_PAR_BE0			(0x01)
#define GPIO_PAR_BE1			(0x02)
#define GPIO_PAR_BE2			(0x04)
#define GPIO_PAR_BE3			(0x08)

/* Bit definitions and macros for GPIO_PAR_CS */
#define GPIO_PAR_CS1			(0x02)
#define GPIO_PAR_CS2			(0x04)
#define GPIO_PAR_CS3			(0x08)
#define GPIO_PAR_CS4			(0x10)
#define GPIO_PAR_CS5			(0x20)
#define GPIO_PAR_CS1_GPIO		(0x00)
#define GPIO_PAR_CS1_SDCS1		(0x01)
#define GPIO_PAR_CS1_CS1		(0x03)

/* Bit definitions and macros for GPIO_PAR_SSI */
#define GPIO_PAR_SSI_MCLK		(0x0080)
#define GPIO_PAR_SSI_TXD(x)		(((x)&0x0003)<<8)
#define GPIO_PAR_SSI_RXD(x)		(((x)&0x0003)<<10)
#define GPIO_PAR_SSI_FS(x)		(((x)&0x0003)<<12)
#define GPIO_PAR_SSI_BCLK(x)		(((x)&0x0003)<<14)

/* Bit definitions and macros for GPIO_PAR_UART */
#define GPIO_PAR_UART_TXD0		(0x0001)
#define GPIO_PAR_UART_RXD0		(0x0002)
#define GPIO_PAR_UART_RTS0		(0x0004)
#define GPIO_PAR_UART_CTS0		(0x0008)
#define GPIO_PAR_UART_TXD1(x)		(((x)&0x0003)<<4)
#define GPIO_PAR_UART_RXD1(x)		(((x)&0x0003)<<6)
#define GPIO_PAR_UART_RTS1(x)		(((x)&0x0003)<<8)
#define GPIO_PAR_UART_CTS1(x)		(((x)&0x0003)<<10)
#define GPIO_PAR_UART_CTS1_GPIO		(0x0000)
#define GPIO_PAR_UART_CTS1_SSI_BCLK	(0x0800)
#define GPIO_PAR_UART_CTS1_ULPI_D7	(0x0400)
#define GPIO_PAR_UART_CTS1_UCTS1	(0x0C00)
#define GPIO_PAR_UART_RTS1_GPIO		(0x0000)
#define GPIO_PAR_UART_RTS1_SSI_FS	(0x0200)
#define GPIO_PAR_UART_RTS1_ULPI_D6	(0x0100)
#define GPIO_PAR_UART_RTS1_URTS1	(0x0300)
#define GPIO_PAR_UART_RXD1_GPIO		(0x0000)
#define GPIO_PAR_UART_RXD1_SSI_RXD	(0x0080)
#define GPIO_PAR_UART_RXD1_ULPI_D5	(0x0040)
#define GPIO_PAR_UART_RXD1_URXD1	(0x00C0)
#define GPIO_PAR_UART_TXD1_GPIO		(0x0000)
#define GPIO_PAR_UART_TXD1_SSI_TXD	(0x0020)
#define GPIO_PAR_UART_TXD1_ULPI_D4	(0x0010)
#define GPIO_PAR_UART_TXD1_UTXD1	(0x0030)

/* Bit definitions and macros for GPIO_PAR_QSPI */
#define GPIO_PAR_QSPI_SCK(x)		(((x)&0x0003)<<4)
#define GPIO_PAR_QSPI_DOUT(x)		(((x)&0x0003)<<6)
#define GPIO_PAR_QSPI_DIN(x)		(((x)&0x0003)<<8)
#define GPIO_PAR_QSPI_PCS0(x)		(((x)&0x0003)<<10)
#define GPIO_PAR_QSPI_PCS1(x)		(((x)&0x0003)<<12)
#define GPIO_PAR_QSPI_PCS2(x)		(((x)&0x0003)<<14)

/* Bit definitions and macros for GPIO_PAR_TIMER */
#define GPIO_PAR_TIN0(x)		(((x)&0x03)<<0)
#define GPIO_PAR_TIN1(x)		(((x)&0x03)<<2)
#define GPIO_PAR_TIN2(x)		(((x)&0x03)<<4)
#define GPIO_PAR_TIN3(x)		(((x)&0x03)<<6)
#define GPIO_PAR_TIN3_GPIO		(0x00)
#define GPIO_PAR_TIN3_TOUT3		(0x80)
#define GPIO_PAR_TIN3_URXD2		(0x40)
#define GPIO_PAR_TIN3_TIN3		(0xC0)
#define GPIO_PAR_TIN2_GPIO		(0x00)
#define GPIO_PAR_TIN2_TOUT2		(0x20)
#define GPIO_PAR_TIN2_UTXD2		(0x10)
#define GPIO_PAR_TIN2_TIN2		(0x30)
#define GPIO_PAR_TIN1_GPIO		(0x00)
#define GPIO_PAR_TIN1_TOUT1		(0x08)
#define GPIO_PAR_TIN1_DACK1		(0x04)
#define GPIO_PAR_TIN1_TIN1		(0x0C)
#define GPIO_PAR_TIN0_GPIO		(0x00)
#define GPIO_PAR_TIN0_TOUT0		(0x02)
#define GPIO_PAR_TIN0_DREQ0		(0x01)
#define GPIO_PAR_TIN0_TIN0		(0x03)

/* Bit definitions and macros for GPIO_PAR_LCDDATA */
#define GPIO_PAR_LCDDATA_LD7_0(x)	((x)&0x03)
#define GPIO_PAR_LCDDATA_LD15_8(x)	(((x)&0x03)<<2)
#define GPIO_PAR_LCDDATA_LD16(x)	(((x)&0x03)<<4)
#define GPIO_PAR_LCDDATA_LD17(x)	(((x)&0x03)<<6)

/* Bit definitions and macros for GPIO_PAR_LCDCTL */
#define GPIO_PAR_LCDCTL_CLS		(0x0001)
#define GPIO_PAR_LCDCTL_PS		(0x0002)
#define GPIO_PAR_LCDCTL_REV		(0x0004)
#define GPIO_PAR_LCDCTL_SPL_SPR		(0x0008)
#define GPIO_PAR_LCDCTL_CONTRAST	(0x0010)
#define GPIO_PAR_LCDCTL_LSCLK		(0x0020)
#define GPIO_PAR_LCDCTL_LP_HSYNC	(0x0040)
#define GPIO_PAR_LCDCTL_FLM_VSYNC	(0x0080)
#define GPIO_PAR_LCDCTL_ACD_OE		(0x0100)

/* Bit definitions and macros for GPIO_PAR_IRQ */
#define GPIO_PAR_IRQ1(x)		(((x)&0x0003)<<4)
#define GPIO_PAR_IRQ2(x)		(((x)&0x0003)<<6)
#define GPIO_PAR_IRQ4(x)		(((x)&0x0003)<<8)
#define GPIO_PAR_IRQ5(x)		(((x)&0x0003)<<10)
#define GPIO_PAR_IRQ6(x)		(((x)&0x0003)<<12)

/* Bit definitions and macros for GPIO_MSCR_FLEXBUS */
#define GPIO_MSCR_FLEXBUS_ADDRCTL(x)	((x)&0x03)
#define GPIO_MSCR_FLEXBUS_DLOWER(x)	(((x)&0x03)<<2)
#define GPIO_MSCR_FLEXBUS_DUPPER(x)	(((x)&0x03)<<4)

/* Bit definitions and macros for GPIO_MSCR_SDRAM */
#define GPIO_MSCR_SDRAM_SDRAM(x)	((x)&0x03)
#define GPIO_MSCR_SDRAM_SDCLK(x)	(((x)&0x03)<<2)
#define GPIO_MSCR_SDRAM_SDCLKB(x)	(((x)&0x03)<<4)

/* Bit definitions and macros for GPIO_DSCR_I2C */
#define GPIO_DSCR_I2C_DSE(x)		((x)&0x03)

/* Bit definitions and macros for GPIO_DSCR_PWM */
#define GPIO_DSCR_PWM_DSE(x)		((x)&0x03)

/* Bit definitions and macros for GPIO_DSCR_FEC */
#define GPIO_DSCR_FEC_DSE(x)		((x)&0x03)

/* Bit definitions and macros for GPIO_DSCR_UART */
#define GPIO_DSCR_UART0_DSE(x)		((x)&0x03)
#define GPIO_DSCR_UART1_DSE(x)		(((x)&0x03)<<2)

/* Bit definitions and macros for GPIO_DSCR_QSPI */
#define GPIO_DSCR_QSPI_DSE(x)		((x)&0x03)

/* Bit definitions and macros for GPIO_DSCR_TIMER */
#define GPIO_DSCR_TIMER_DSE(x)		((x)&0x03)

/* Bit definitions and macros for GPIO_DSCR_SSI */
#define GPIO_DSCR_SSI_DSE(x)		((x)&0x03)

/* Bit definitions and macros for GPIO_DSCR_LCD */
#define GPIO_DSCR_LCD_DSE(x)		((x)&0x03)

/* Bit definitions and macros for GPIO_DSCR_DEBUG */
#define GPIO_DSCR_DEBUG_DSE(x)		((x)&0x03)

/* Bit definitions and macros for GPIO_DSCR_CLKRST */
#define GPIO_DSCR_CLKRST_DSE(x)		((x)&0x03)

/* Bit definitions and macros for GPIO_DSCR_IRQ */
#define GPIO_DSCR_IRQ_DSE(x)		((x)&0x03)

/*********************************************************************
* SDRAM Controller (SDRAMC)
*********************************************************************/
/* Bit definitions and macros for SDRAMC_SDMR */
#define SDRAMC_SDMR_BNKAD_LEMR		(0x40000000)
#define SDRAMC_SDMR_BNKAD_LMR		(0x00000000)
#define SDRAMC_SDMR_AD(x)		(((x)&0x00000FFF)<<18)
#define SDRAMC_SDMR_CMD			(0x00010000)

/* Bit definitions and macros for SDRAMC_SDCR */
#define SDRAMC_SDCR_MODE_EN		(0x80000000)
#define SDRAMC_SDCR_CKE			(0x40000000)
#define SDRAMC_SDCR_DDR			(0x20000000)
#define SDRAMC_SDCR_REF			(0x10000000)
#define SDRAMC_SDCR_MUX(x)		(((x)&0x00000003)<<24)
#define SDRAMC_SDCR_OE_RULE		(0x00400000)
#define SDRAMC_SDCR_RCNT(x)		(((x)&0x0000003F)<<16)
#define SDRAMC_SDCR_PS_32		(0x00000000)
#define SDRAMC_SDCR_PS_16		(0x00002000)
#define SDRAMC_SDCR_DQS_OE(x)		(((x)&0x0000000F)<<8)
#define SDRAMC_SDCR_IREF		(0x00000004)
#define SDRAMC_SDCR_IPALL		(0x00000002)

/* Bit definitions and macros for SDRAMC_SDCFG1 */
#define SDRAMC_SDCFG1_SRD2RW(x)		(((x)&0x0000000F)<<28)
#define SDRAMC_SDCFG1_SWT2RD(x)		(((x)&0x00000007)<<24)
#define SDRAMC_SDCFG1_RDLAT(x)		(((x)&0x0000000F)<<20)
#define SDRAMC_SDCFG1_ACT2RW(x)		(((x)&0x00000007)<<16)
#define SDRAMC_SDCFG1_PRE2ACT(x)	(((x)&0x00000007)<<12)
#define SDRAMC_SDCFG1_REF2ACT(x)	(((x)&0x0000000F)<<8)
#define SDRAMC_SDCFG1_WTLAT(x)		(((x)&0x00000007)<<4)

/* Bit definitions and macros for SDRAMC_SDCFG2 */
#define SDRAMC_SDCFG2_BRD2PRE(x)	(((x)&0x0000000F)<<28)
#define SDRAMC_SDCFG2_BWT2RW(x)		(((x)&0x0000000F)<<24)
#define SDRAMC_SDCFG2_BRD2WT(x)		(((x)&0x0000000F)<<20)
#define SDRAMC_SDCFG2_BL(x)		(((x)&0x0000000F)<<16)

/* Bit definitions and macros for SDRAMC_SDDS */
#define SDRAMC_SDDS_SB_E(x)		(((x)&0x00000003)<<8)
#define SDRAMC_SDDS_SB_C(x)		(((x)&0x00000003)<<6)
#define SDRAMC_SDDS_SB_A(x)		(((x)&0x00000003)<<4)
#define SDRAMC_SDDS_SB_S(x)		(((x)&0x00000003)<<2)
#define SDRAMC_SDDS_SB_D(x)		((x)&0x00000003)

/* Bit definitions and macros for SDRAMC_SDCS */
#define SDRAMC_SDCS_BASE(x)		(((x)&0x00000FFF)<<20)
#define SDRAMC_SDCS_CSSZ(x)		((x)&0x0000001F)
#define SDRAMC_SDCS_CSSZ_4GBYTE		(0x0000001F)
#define SDRAMC_SDCS_CSSZ_2GBYTE		(0x0000001E)
#define SDRAMC_SDCS_CSSZ_1GBYTE		(0x0000001D)
#define SDRAMC_SDCS_CSSZ_512MBYTE	(0x0000001C)
#define SDRAMC_SDCS_CSSZ_256MBYTE	(0x0000001B)
#define SDRAMC_SDCS_CSSZ_128MBYTE	(0x0000001A)
#define SDRAMC_SDCS_CSSZ_64MBYTE	(0x00000019)
#define SDRAMC_SDCS_CSSZ_32MBYTE	(0x00000018)
#define SDRAMC_SDCS_CSSZ_16MBYTE	(0x00000017)
#define SDRAMC_SDCS_CSSZ_8MBYTE		(0x00000016)
#define SDRAMC_SDCS_CSSZ_4MBYTE		(0x00000015)
#define SDRAMC_SDCS_CSSZ_2MBYTE		(0x00000014)
#define SDRAMC_SDCS_CSSZ_1MBYTE		(0x00000013)
#define SDRAMC_SDCS_CSSZ_DIABLE		(0x00000000)

/*********************************************************************
* Phase Locked Loop (PLL)
*********************************************************************/
/* Bit definitions and macros for PLL_PODR */
#define PLL_PODR_CPUDIV(x)		(((x)&0x0F)<<4)
#define PLL_PODR_BUSDIV(x)		((x)&0x0F)

/* Bit definitions and macros for PLL_PLLCR */
#define PLL_PLLCR_DITHEN		(0x80)
#define PLL_PLLCR_DITHDEV(x)		((x)&0x07)

#endif				/* mcf5329_h */
