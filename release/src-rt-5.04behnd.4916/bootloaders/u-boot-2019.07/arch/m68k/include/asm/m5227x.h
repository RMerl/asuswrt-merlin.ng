/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MCF5227x Internal Memory Map
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __MCF5227X__
#define __MCF5227X__

/* Interrupt Controller (INTC) */
#define INT0_LO_RSVD0			(0)
#define INT0_LO_EPORT1			(1)
#define INT0_LO_EPORT4			(4)
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
#define INT0_LO_SCM_CWIC		(25)
#define INT0_LO_UART0			(26)
#define INT0_LO_UART1			(27)
#define INT0_LO_UART2			(28)
#define INT0_LO_I2C			(30)
#define INT0_LO_DSPI			(31)
#define INT0_HI_DTMR0			(32)
#define INT0_HI_DTMR1			(33)
#define INT0_HI_DTMR2			(34)
#define INT0_HI_DTMR3			(35)
#define INT0_HI_SCMIR			(62)
#define INT0_HI_RTC_ISR			(63)

#define INT1_HI_CAN_BOFFINT		(1)
#define INT1_HI_CAN_ERRINT		(3)
#define INT1_HI_CAN_BUF0I		(4)
#define INT1_HI_CAN_BUF1I		(5)
#define INT1_HI_CAN_BUF2I		(6)
#define INT1_HI_CAN_BUF3I		(7)
#define INT1_HI_CAN_BUF4I		(8)
#define INT1_HI_CAN_BUF5I		(9)
#define INT1_HI_CAN_BUF6I		(10)
#define INT1_HI_CAN_BUF7I		(11)
#define INT1_HI_CAN_BUF8I		(12)
#define INT1_HI_CAN_BUF9I		(13)
#define INT1_HI_CAN_BUF10I		(14)
#define INT1_HI_CAN_BUF11I		(15)
#define INT1_HI_CAN_BUF12I		(16)
#define INT1_HI_CAN_BUF13I		(17)
#define INT1_HI_CAN_BUF14I		(18)
#define INT1_HI_CAN_BUF15I		(19)
#define INT1_HI_PIT0_PIF		(43)
#define INT1_HI_PIT1_PIF		(44)
#define INT1_HI_USBOTG_STS		(47)
#define INT1_HI_SSI_ISR			(49)
#define INT1_HI_PWM_INT			(50)
#define INT1_HI_LCDC_ISR		(51)
#define INT1_HI_CCM_UOCSR		(53)
#define INT1_HI_DSPI_EOQF		(54)
#define INT1_HI_DSPI_TFFF		(55)
#define INT1_HI_DSPI_TCF		(56)
#define INT1_HI_DSPI_TFUF		(57)
#define INT1_HI_DSPI_RFDF		(58)
#define INT1_HI_DSPI_RFOF		(59)
#define INT1_HI_DSPI_RFOF_TFUF		(60)
#define INT1_HI_TOUCH_ADC		(61)
#define INT1_HI_PLL_LOCKS		(62)

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
* Chip Configuration Module (CCM)
*********************************************************************/

/* Bit definitions and macros for CCR */
#define CCM_CCR_DRAMSEL			(0x0100)
#define CCM_CCR_CSC_UNMASK		(0xFF3F)
#define CCM_CCR_CSC_FBCS5_CS4		(0x00C0)
#define CCM_CCR_CSC_FBCS5_A22		(0x0080)
#define CCM_CCR_CSC_FB_A23_A22		(0x0040)
#define CCM_CCR_LIMP			(0x0020)
#define CCM_CCR_LOAD			(0x0010)
#define CCM_CCR_BOOTPS_UNMASK		(0xFFF3)
#define CCM_CCR_BOOTPS_PS16		(0x0008)
#define CCM_CCR_BOOTPS_PS8		(0x0004)
#define CCM_CCR_BOOTPS_PS32		(0x0000)
#define CCM_CCR_OSCMODE_OSCBYPASS	(0x0002)

/* Bit definitions and macros for RCON */
#define CCM_RCON_CSC_UNMASK		(0xFF3F)
#define CCM_RCON_CSC_FBCS5_CS4		(0x00C0)
#define CCM_RCON_CSC_FBCS5_A22		(0x0080)
#define CCM_RCON_CSC_FB_A23_A22		(0x0040)
#define CCM_RCON_LIMP			(0x0020)
#define CCM_RCON_LOAD			(0x0010)
#define CCM_RCON_BOOTPS_UNMASK		(0xFFF3)
#define CCM_RCON_BOOTPS_PS16		(0x0008)
#define CCM_RCON_BOOTPS_PS8		(0x0004)
#define CCM_RCON_BOOTPS_PS32		(0x0000)
#define CCM_RCON_OSCMODE_OSCBYPASS	(0x0002)

/* Bit definitions and macros for CIR */
#define CCM_CIR_PIN(x)			(((x) & 0xFFC0) >> 6)
#define CCM_CIR_PRN(x)			((x) & 0x003F)
#define CCM_CIR_PIN_MCF52277		(0x0000)

/* Bit definitions and macros for MISCCR */
#define CCM_MISCCR_RTCSRC		(0x4000)
#define CCM_MISCCR_USBPUE		(0x2000)	/* USB transceiver pull-up */
#define CCM_MISCCR_LIMP			(0x1000)	/* Limp mode enable */

#define CCM_MISCCR_BME			(0x0800)	/* Bus monitor ext en bit */
#define CCM_MISCCR_BMT_65536		(0)
#define CCM_MISCCR_BMT_32768		(1)
#define CCM_MISCCR_BMT_16384		(2)
#define CCM_MISCCR_BMT_8192		(3)
#define CCM_MISCCR_BMT_4096		(4)
#define CCM_MISCCR_BMT_2048		(5)
#define CCM_MISCCR_BMT_1024		(6)
#define CCM_MISCCR_BMT_512		(7)

#define CCM_MISCCR_SSIPUE		(0x0080)	/* SSI RXD/TXD pull enable */
#define CCM_MISCCR_SSIPUS		(0x0040)	/* SSI RXD/TXD pull select */
#define CCM_MISCCR_TIMDMA		(0x0020)	/* Timer DMA mux selection */
#define CCM_MISCCR_SSISRC		(0x0010)	/* SSI clock source */
#define CCM_MISCCR_LCDCHEN		(0x0004)	/* LCD Int CLK en */
#define CCM_MISCCR_USBOC		(0x0002)	/* USB VBUS over-current sense pol */
#define CCM_MISCCR_USBSRC		(0x0001)	/* USB clock source */

/* Bit definitions and macros for CDR */
#define CCM_CDR_USBDIV(x)		(((x)&0x0003)<<12)
#define CCM_CDR_LPDIV(x)		(((x)&0x000F)<<8)	/* Low power clk div */
#define CCM_CDR_SSIDIV(x)		(((x)&0x00FF))	/* SSI oversampling clk div */

/* Bit definitions and macros for UOCSR */
#define CCM_UOCSR_DPPD			(0x2000)	/* D+ 15Kohm pull-down (rd-only) */
#define CCM_UOCSR_DMPD			(0x1000)	/* D- 15Kohm pull-down (rd-only) */
#define CCM_UOCSR_CRG_VBUS		(0x0400)	/* VBUS charge resistor enabled (rd-only) */
#define CCM_UOCSR_DCR_VBUS		(0x0200)	/* VBUS discharge resistor en (rd-only) */
#define CCM_UOCSR_DPPU			(0x0100)	/* D+ pull-up for FS enabled (rd-only) */
#define CCM_UOCSR_AVLD			(0x0080)	/* A-peripheral valid indicator */
#define CCM_UOCSR_BVLD			(0x0040)	/* B-peripheral valid indicator */
#define CCM_UOCSR_VVLD			(0x0020)	/* VBUS valid indicator */
#define CCM_UOCSR_SEND			(0x0010)	/* Session end */
#define CCM_UOCSR_WKUP			(0x0004)	/* USB OTG controller wake-up event */
#define CCM_UOCSR_UOMIE			(0x0002)	/* USB OTG misc interrupt en */
#define CCM_UOCSR_XPDE			(0x0001)	/* On-chip transceiver pull-down en */

/*********************************************************************
* General Purpose I/O Module (GPIO)
*********************************************************************/
/* Bit definitions and macros for PAR_BE */
#define GPIO_PAR_BE_UNMASK		(0x0F)
#define GPIO_PAR_BE_BE3_BE3		(0x08)
#define GPIO_PAR_BE_BE3_GPIO		(0x00)
#define GPIO_PAR_BE_BE2_BE2		(0x04)
#define GPIO_PAR_BE_BE2_GPIO		(0x00)
#define GPIO_PAR_BE_BE1_BE1		(0x02)
#define GPIO_PAR_BE_BE1_GPIO		(0x00)
#define GPIO_PAR_BE_BE0_BE0		(0x01)
#define GPIO_PAR_BE_BE0_GPIO		(0x00)

/* Bit definitions and macros for PAR_CS */
#define GPIO_PAR_CS_CS3			(0x10)
#define GPIO_PAR_CS_CS2			(0x08)
#define GPIO_PAR_CS_CS1_FBCS1		(0x06)
#define GPIO_PAR_CS_CS1_SDCS1		(0x04)
#define GPIO_PAR_CS_CS1_GPIO		(0x00)
#define GPIO_PAR_CS_CS0			(0x01)

/* Bit definitions and macros for PAR_FBCTL */
#define GPIO_PAR_FBCTL_OE		(0x80)
#define GPIO_PAR_FBCTL_TA		(0x40)
#define GPIO_PAR_FBCTL_RW		(0x20)
#define GPIO_PAR_FBCTL_TS_UNMASK	(0xE7)
#define GPIO_PAR_FBCTL_TS_FBTS		(0x18)
#define GPIO_PAR_FBCTL_TS_DMAACK	(0x10)
#define GPIO_PAR_FBCTL_TS_GPIO		(0x00)

/* Bit definitions and macros for PAR_FECI2C */
#define GPIO_PAR_I2C_SCL_UNMASK		(0xF3)
#define GPIO_PAR_I2C_SCL_SCL		(0x0C)
#define GPIO_PAR_I2C_SCL_CANTXD		(0x08)
#define GPIO_PAR_I2C_SCL_U2TXD		(0x04)
#define GPIO_PAR_I2C_SCL_GPIO		(0x00)

#define GPIO_PAR_I2C_SDA_UNMASK		(0xFC)
#define GPIO_PAR_I2C_SDA_SDA		(0x03)
#define GPIO_PAR_I2C_SDA_CANRXD		(0x02)
#define GPIO_PAR_I2C_SDA_U2RXD		(0x01)
#define GPIO_PAR_I2C_SDA_GPIO		(0x00)

/* Bit definitions and macros for PAR_UART */
#define GPIO_PAR_UART_U1CTS_UNMASK	(0x3FFF)
#define GPIO_PAR_UART_U1CTS_U1CTS	(0xC000)
#define GPIO_PAR_UART_U1CTS_SSIBCLK	(0x8000)
#define GPIO_PAR_UART_U1CTS_LCDCLS	(0x4000)
#define GPIO_PAR_UART_U1CTS_GPIO	(0x0000)

#define GPIO_PAR_UART_U1RTS_UNMASK	(0xCFFF)
#define GPIO_PAR_UART_U1RTS_U1RTS	(0x3000)
#define GPIO_PAR_UART_U1RTS_SSIFS	(0x2000)
#define GPIO_PAR_UART_U1RTS_LCDPS	(0x1000)
#define GPIO_PAR_UART_U1RTS_GPIO	(0x0000)

#define GPIO_PAR_UART_U1RXD_UNMASK	(0xF3FF)
#define GPIO_PAR_UART_U1RXD_U1RXD	(0x0C00)
#define GPIO_PAR_UART_U1RXD_SSIRXD	(0x0800)
#define GPIO_PAR_UART_U1RXD_GPIO	(0x0000)

#define GPIO_PAR_UART_U1TXD_UNMASK	(0xFCFF)
#define GPIO_PAR_UART_U1TXD_U1TXD	(0x0300)
#define GPIO_PAR_UART_U1TXD_SSITXD	(0x0200)
#define GPIO_PAR_UART_U1TXD_GPIO	(0x0000)

#define GPIO_PAR_UART_U0CTS_UNMASK	(0xFF3F)
#define GPIO_PAR_UART_U0CTS_U0CTS	(0x00C0)
#define GPIO_PAR_UART_U0CTS_T1OUT	(0x0080)
#define GPIO_PAR_UART_U0CTS_USBVBUSEN	(0x0040)
#define GPIO_PAR_UART_U0CTS_GPIO	(0x0000)

#define GPIO_PAR_UART_U0RTS_UNMASK	(0xFFCF)
#define GPIO_PAR_UART_U0RTS_U0RTS	(0x0030)
#define GPIO_PAR_UART_U0RTS_T1IN	(0x0020)
#define GPIO_PAR_UART_U0RTS_USBVBUSOC	(0x0010)
#define GPIO_PAR_UART_U0RTS_GPIO	(0x0000)

#define GPIO_PAR_UART_U0RXD_UNMASK	(0xFFF3)
#define GPIO_PAR_UART_U0RXD_U0RXD	(0x000C)
#define GPIO_PAR_UART_U0RXD_CANRX	(0x0008)
#define GPIO_PAR_UART_U0RXD_GPIO	(0x0000)

#define GPIO_PAR_UART_U0TXD_UNMASK	(0xFFFC)
#define GPIO_PAR_UART_U0TXD_U0TXD	(0x0003)
#define GPIO_PAR_UART_U0TXD_CANTX	(0x0002)
#define GPIO_PAR_UART_U0TXD_GPIO	(0x0000)

/* Bit definitions and macros for PAR_DSPI */
#define GPIO_PAR_DSPI_PCS0_UNMASK	(0x3F)
#define GPIO_PAR_DSPI_PCS0_PCS0		(0xC0)
#define GPIO_PAR_DSPI_PCS0_U2RTS	(0x80)
#define GPIO_PAR_DSPI_PCS0_GPIO		(0x00)
#define GPIO_PAR_DSPI_SIN_UNMASK	(0xCF)
#define GPIO_PAR_DSPI_SIN_SIN		(0x30)
#define GPIO_PAR_DSPI_SIN_U2RXD		(0x20)
#define GPIO_PAR_DSPI_SIN_GPIO		(0x00)
#define GPIO_PAR_DSPI_SOUT_UNMASK	(0xF3)
#define GPIO_PAR_DSPI_SOUT_SOUT		(0x0C)
#define GPIO_PAR_DSPI_SOUT_U2TXD	(0x08)
#define GPIO_PAR_DSPI_SOUT_GPIO		(0x00)
#define GPIO_PAR_DSPI_SCK_UNMASK	(0xFC)
#define GPIO_PAR_DSPI_SCK_SCK		(0x03)
#define GPIO_PAR_DSPI_SCK_U2CTS		(0x02)
#define GPIO_PAR_DSPI_SCK_GPIO		(0x00)

/* Bit definitions and macros for PAR_TIMER */
#define GPIO_PAR_TIMER_T3IN_UNMASK	(0x3F)
#define GPIO_PAR_TIMER_T3IN_T3IN	(0xC0)
#define GPIO_PAR_TIMER_T3IN_T3OUT	(0x80)
#define GPIO_PAR_TIMER_T3IN_SSIMCLK	(0x40)
#define GPIO_PAR_TIMER_T3IN_GPIO	(0x00)
#define GPIO_PAR_TIMER_T2IN_UNMASK	(0xCF)
#define GPIO_PAR_TIMER_T2IN_T2IN	(0x30)
#define GPIO_PAR_TIMER_T2IN_T2OUT	(0x20)
#define GPIO_PAR_TIMER_T2IN_DSPIPCS2	(0x10)
#define GPIO_PAR_TIMER_T2IN_GPIO	(0x00)
#define GPIO_PAR_TIMER_T1IN_UNMASK	(0xF3)
#define GPIO_PAR_TIMER_T1IN_T1IN	(0x0C)
#define GPIO_PAR_TIMER_T1IN_T1OUT	(0x08)
#define GPIO_PAR_TIMER_T1IN_LCDCONTRAST	(0x04)
#define GPIO_PAR_TIMER_T1IN_GPIO	(0x00)
#define GPIO_PAR_TIMER_T0IN_UNMASK	(0xFC)
#define GPIO_PAR_TIMER_T0IN_T0IN	(0x03)
#define GPIO_PAR_TIMER_T0IN_T0OUT	(0x02)
#define GPIO_PAR_TIMER_T0IN_LCDREV	(0x01)
#define GPIO_PAR_TIMER_T0IN_GPIO	(0x00)

/* Bit definitions and macros for GPIO_PAR_LCDCTL */
#define GPIO_PAR_LCDCTL_ACDOE_UNMASK	(0xE7)
#define GPIO_PAR_LCDCTL_ACDOE_ACDOE	(0x18)
#define GPIO_PAR_LCDCTL_ACDOE_SPLSPR	(0x10)
#define GPIO_PAR_LCDCTL_ACDOE_GPIO	(0x00)
#define GPIO_PAR_LCDCTL_FLM_VSYNC	(0x04)
#define GPIO_PAR_LCDCTL_LP_HSYNC	(0x02)
#define GPIO_PAR_LCDCTL_LSCLK		(0x01)

/* Bit definitions and macros for PAR_IRQ */
#define GPIO_PAR_IRQ_IRQ4_UNMASK	(0xF3)
#define GPIO_PAR_IRQ_IRQ4_SSIINPCLK	(0x0C)
#define GPIO_PAR_IRQ_IRQ4_DMAREQ0	(0x08)
#define GPIO_PAR_IRQ_IRQ4_GPIO		(0x00)
#define GPIO_PAR_IRQ_IRQ1_UNMASK	(0xFC)
#define GPIO_PAR_IRQ_IRQ1_PCIINT	(0x03)
#define GPIO_PAR_IRQ_IRQ1_USBCLKIN	(0x02)
#define GPIO_PAR_IRQ_IRQ1_SSICLKIN	(0x01)
#define GPIO_PAR_IRQ_IRQ1_GPIO		(0x00)

/* Bit definitions and macros for GPIO_PAR_LCDH */
#define GPIO_PAR_LCDH_LD17_UNMASK	(0xFFFFF3FF)
#define GPIO_PAR_LCDH_LD17_LD17		(0x00000C00)
#define GPIO_PAR_LCDH_LD17_LD11		(0x00000800)
#define GPIO_PAR_LCDH_LD17_GPIO		(0x00000000)

#define GPIO_PAR_LCDH_LD16_UNMASK	(0xFFFFFCFF)
#define GPIO_PAR_LCDH_LD16_LD16		(0x00000300)
#define GPIO_PAR_LCDH_LD16_LD10		(0x00000200)
#define GPIO_PAR_LCDH_LD16_GPIO		(0x00000000)

#define GPIO_PAR_LCDH_LD15_UNMASK	(0xFFFFFF3F)
#define GPIO_PAR_LCDH_LD15_LD15		(0x000000C0)
#define GPIO_PAR_LCDH_LD15_LD9		(0x00000080)
#define GPIO_PAR_LCDH_LD15_GPIO		(0x00000000)

#define GPIO_PAR_LCDH_LD14_UNMASK	(0xFFFFFFCF)
#define GPIO_PAR_LCDH_LD14_LD14		(0x00000030)
#define GPIO_PAR_LCDH_LD14_LD8		(0x00000020)
#define GPIO_PAR_LCDH_LD14_GPIO		(0x00000000)

#define GPIO_PAR_LCDH_LD13_UNMASK	(0xFFFFFFF3)
#define GPIO_PAR_LCDH_LD13_LD13		(0x0000000C)
#define GPIO_PAR_LCDH_LD13_CANTX	(0x00000008)
#define GPIO_PAR_LCDH_LD13_GPIO		(0x00000000)

#define GPIO_PAR_LCDH_LD12_UNMASK	(0xFFFFFFFC)
#define GPIO_PAR_LCDH_LD12_LD12		(0x00000003)
#define GPIO_PAR_LCDH_LD12_CANRX	(0x00000002)
#define GPIO_PAR_LCDH_LD12_GPIO		(0x00000000)

/* Bit definitions and macros for GPIO_PAR_LCDL */
#define GPIO_PAR_LCDL_LD11_UNMASK	(0x3FFFFFFF)
#define GPIO_PAR_LCDL_LD11_LD11		(0xC0000000)
#define GPIO_PAR_LCDL_LD11_LD7		(0x80000000)
#define GPIO_PAR_LCDL_LD11_GPIO		(0x00000000)

#define GPIO_PAR_LCDL_LD10_UNMASK	(0xCFFFFFFF)
#define GPIO_PAR_LCDL_LD10_LD10		(0x30000000)
#define GPIO_PAR_LCDL_LD10_LD6		(0x20000000)
#define GPIO_PAR_LCDL_LD10_GPIO		(0x00000000)

#define GPIO_PAR_LCDL_LD9_UNMASK	(0xF3FFFFFF)
#define GPIO_PAR_LCDL_LD9_LD9		(0x0C000000)
#define GPIO_PAR_LCDL_LD9_LD5		(0x08000000)
#define GPIO_PAR_LCDL_LD9_GPIO		(0x00000000)

#define GPIO_PAR_LCDL_LD8_UNMASK	(0xFCFFFFFF)
#define GPIO_PAR_LCDL_LD8_LD8		(0x03000000)
#define GPIO_PAR_LCDL_LD8_LD4		(0x02000000)
#define GPIO_PAR_LCDL_LD8_GPIO		(0x00000000)

#define GPIO_PAR_LCDL_LD7_UNMASK	(0xFF3FFFFF)
#define GPIO_PAR_LCDL_LD7_LD7		(0x00C00000)
#define GPIO_PAR_LCDL_LD7_PWM7		(0x00800000)
#define GPIO_PAR_LCDL_LD7_GPIO		(0x00000000)

#define GPIO_PAR_LCDL_LD6_UNMASK	(0xFFCFFFFF)
#define GPIO_PAR_LCDL_LD6_LD6		(0x00300000)
#define GPIO_PAR_LCDL_LD6_PWM5		(0x00200000)
#define GPIO_PAR_LCDL_LD6_GPIO		(0x00000000)

#define GPIO_PAR_LCDL_LD5_UNMASK	(0xFFF3FFFF)
#define GPIO_PAR_LCDL_LD5_LD5		(0x000C0000)
#define GPIO_PAR_LCDL_LD5_LD3		(0x00080000)
#define GPIO_PAR_LCDL_LD5_GPIO		(0x00000000)

#define GPIO_PAR_LCDL_LD4_UNMASK	(0xFFFCFFFF)
#define GPIO_PAR_LCDL_LD4_LD4		(0x00030000)
#define GPIO_PAR_LCDL_LD4_LD2		(0x00020000)
#define GPIO_PAR_LCDL_LD4_GPIO		(0x00000000)

#define GPIO_PAR_LCDL_LD3_UNMASK	(0xFFFF3FFF)
#define GPIO_PAR_LCDL_LD3_LD3		(0x0000C000)
#define GPIO_PAR_LCDL_LD3_LD1		(0x00008000)
#define GPIO_PAR_LCDL_LD3_GPIO		(0x00000000)

#define GPIO_PAR_LCDL_LD2_UNMASK	(0xFFFFCFFF)
#define GPIO_PAR_LCDL_LD2_LD2		(0x00003000)
#define GPIO_PAR_LCDL_LD2_LD0		(0x00002000)
#define GPIO_PAR_LCDL_LD2_GPIO		(0x00000000)

#define GPIO_PAR_LCDL_LD1_UNMASK	(0xFFFFF3FF)
#define GPIO_PAR_LCDL_LD1_LD1		(0x00000C00)
#define GPIO_PAR_LCDL_LD1_PWM3		(0x00000800)
#define GPIO_PAR_LCDL_LD1_GPIO		(0x00000000)

#define GPIO_PAR_LCDL_LD0_UNMASK	(0xFFFFFCFF)
#define GPIO_PAR_LCDL_LD0_LD0		(0x00000300)
#define GPIO_PAR_LCDL_LD0_PWM1		(0x00000200)
#define GPIO_PAR_LCDL_LD0_GPIO		(0x00000000)

/* Bit definitions and macros for MSCR_FB */
#define GPIO_MSCR_FB_DUPPER_UNMASK	(0xCF)
#define GPIO_MSCR_FB_DUPPER_25V_33V	(0x30)
#define GPIO_MSCR_FB_DUPPER_FULL_18V	(0x20)
#define GPIO_MSCR_FB_DUPPER_OD		(0x10)
#define GPIO_MSCR_FB_DUPPER_HALF_18V	(0x00)

#define GPIO_MSCR_FB_DLOWER_UNMASK	(0xF3)
#define GPIO_MSCR_FB_DLOWER_25V_33V	(0x0C)
#define GPIO_MSCR_FB_DLOWER_FULL_18V	(0x08)
#define GPIO_MSCR_FB_DLOWER_OD		(0x04)
#define GPIO_MSCR_FB_DLOWER_HALF_18V	(0x00)

#define GPIO_MSCR_FB_ADDRCTL_UNMASK	(0xFC)
#define GPIO_MSCR_FB_ADDRCTL_25V_33V	(0x03)
#define GPIO_MSCR_FB_ADDRCTL_FULL_18V	(0x02)
#define GPIO_MSCR_FB_ADDRCTL_OD		(0x01)
#define GPIO_MSCR_FB_ADDRCTL_HALF_18V	(0x00)

/* Bit definitions and macros for MSCR_SDRAM */
#define GPIO_MSCR_SDRAM_SDCLKB_UNMASK	(0xCF)
#define GPIO_MSCR_SDRAM_SDCLKB_25V_33V	(0x30)
#define GPIO_MSCR_SDRAM_SDCLKB_FULL_18V	(0x20)
#define GPIO_MSCR_SDRAM_SDCLKB_OD	(0x10)
#define GPIO_MSCR_SDRAM_SDCLKB_HALF_18V	(0x00)

#define GPIO_MSCR_SDRAM_SDCLK_UNMASK	(0xF3)
#define GPIO_MSCR_SDRAM_SDCLK_25V_33V	(0x0C)
#define GPIO_MSCR_SDRAM_SDCLK_FULL_18V	(0x08)
#define GPIO_MSCR_SDRAM_SDCLK_OPD	(0x04)
#define GPIO_MSCR_SDRAM_SDCLK_HALF_18V	(0x00)

#define GPIO_MSCR_SDRAM_SDCTL_UNMASK	(0xFC)
#define GPIO_MSCR_SDRAM_SDCTL_25V_33V	(0x03)
#define GPIO_MSCR_SDRAM_SDCTL_FULL_18V	(0x02)
#define GPIO_MSCR_SDRAM_SDCTL_OPD	(0x01)
#define GPIO_MSCR_SDRAM_SDCTL_HALF_18V	(0x00)

/* Bit definitions and macros for Drive Strength Control */
#define DSCR_LOAD_50PF	(0x03)
#define DSCR_LOAD_30PF	(0x02)
#define DSCR_LOAD_20PF	(0x01)
#define DSCR_LOAD_10PF	(0x00)

/*********************************************************************
* SDRAM Controller (SDRAMC)
*********************************************************************/

/* Bit definitions and macros for SDMR */
#define SDRAMC_SDMR_DDR2_AD(x)		(((x)&0x00003FFF))	/* Address for DDR2 */
#define SDRAMC_SDMR_CMD			(0x00010000)	/* Command */
#define SDRAMC_SDMR_AD(x)		(((x)&0x00000FFF)<<18)	/* Address */
#define SDRAMC_SDMR_BK(x)		(((x)&0x00000003)<<30)	/* Bank Address */
#define SDRAMC_SDMR_BK_LMR		(0x00000000)
#define SDRAMC_SDMR_BK_LEMR		(0x40000000)

/* Bit definitions and macros for SDCR */
#define SDRAMC_SDCR_DPD			(0x00000001)	/* Deep Power-Down Mode */
#define SDRAMC_SDCR_IPALL		(0x00000002)	/* Initiate Precharge All */
#define SDRAMC_SDCR_IREF		(0x00000004)	/* Initiate Refresh */
#define SDRAMC_SDCR_DQS_OE(x)		(((x)&0x00000003)<<10)	/* DQS Output Enable */
#define SDRAMC_SDCR_MEM_PS		(0x00002000)	/* Data Port Size */
#define SDRAMC_SDCR_REF_CNT(x)		(((x)&0x0000003F)<<16)	/* Periodic Refresh Counter */
#define SDRAMC_SDCR_OE_RULE		(0x00400000)	/* Drive Rule Selection */
#define SDRAMC_SDCR_ADDR_MUX(x)		(((x)&0x00000003)<<24)	/* Internal Address Mux Select */
#define SDRAMC_SDCR_DDR2_MODE		(0x08000000)	/* DDR2 Mode Select */
#define SDRAMC_SDCR_REF_EN		(0x10000000)	/* Refresh Enable */
#define SDRAMC_SDCR_DDR_MODE		(0x20000000)	/* DDR Mode Select */
#define SDRAMC_SDCR_CKE			(0x40000000)	/* Clock Enable */
#define SDRAMC_SDCR_MODE_EN		(0x80000000)	/* SDRAM Mode Register Programming Enable */
#define SDRAMC_SDCR_DQS_OE_BOTH		(0x00000C000)

/* Bit definitions and macros for SDCFG1 */
#define SDRAMC_SDCFG1_WT_LAT(x)		(((x)&0x00000007)<<4)	/* Write Latency */
#define SDRAMC_SDCFG1_REF2ACT(x)	(((x)&0x0000000F)<<8)	/* Refresh to active delay */
#define SDRAMC_SDCFG1_PRE2ACT(x)	(((x)&0x00000007)<<12)	/* Precharge to active delay */
#define SDRAMC_SDCFG1_ACT2RW(x)		(((x)&0x00000007)<<16)	/* Active to read/write delay */
#define SDRAMC_SDCFG1_RD_LAT(x)		(((x)&0x0000000F)<<20)	/* Read CAS Latency */
#define SDRAMC_SDCFG1_SWT2RWP(x)	(((x)&0x00000007)<<24)	/* Single write to read/write/precharge delay */
#define SDRAMC_SDCFG1_SRD2RWP(x)	(((x)&0x0000000F)<<28)	/* Single read to read/write/precharge delay */

/* Bit definitions and macros for SDCFG2 */
#define SDRAMC_SDCFG2_BL(x)		(((x)&0x0000000F)<<16)	/* Burst Length */
#define SDRAMC_SDCFG2_BRD2W(x)		(((x)&0x0000000F)<<20)	/* Burst read to write delay */
#define SDRAMC_SDCFG2_BWT2RWP(x)	(((x)&0x0000000F)<<24)	/* Burst write to read/write/precharge delay */
#define SDRAMC_SDCFG2_BRD2RP(x)		(((x)&0x0000000F)<<28)	/* Burst read to read/precharge delay */

/* Bit definitions and macros for SDCS group */
#define SDRAMC_SDCS_CSSZ(x)		(((x)&0x0000001F))	/* Chip-Select Size */
#define SDRAMC_SDCS_CSBA(x)		(((x)&0x00000FFF)<<20)	/* Chip-Select Base Address */
#define SDRAMC_SDCS_BA(x)		((x)&0xFFF00000)
#define SDRAMC_SDCS_CSSZ_DISABLE	(0x00000000)
#define SDRAMC_SDCS_CSSZ_1MBYTE		(0x00000013)
#define SDRAMC_SDCS_CSSZ_2MBYTE		(0x00000014)
#define SDRAMC_SDCS_CSSZ_4MBYTE		(0x00000015)
#define SDRAMC_SDCS_CSSZ_8MBYTE		(0x00000016)
#define SDRAMC_SDCS_CSSZ_16MBYTE	(0x00000017)
#define SDRAMC_SDCS_CSSZ_32MBYTE	(0x00000018)
#define SDRAMC_SDCS_CSSZ_64MBYTE	(0x00000019)
#define SDRAMC_SDCS_CSSZ_128MBYTE	(0x0000001A)
#define SDRAMC_SDCS_CSSZ_256MBYTE	(0x0000001B)
#define SDRAMC_SDCS_CSSZ_512MBYTE	(0x0000001C)
#define SDRAMC_SDCS_CSSZ_1GBYTE		(0x0000001D)
#define SDRAMC_SDCS_CSSZ_2GBYTE		(0x0000001E)
#define SDRAMC_SDCS_CSSZ_4GBYTE		(0x0000001F)

/*********************************************************************
* Phase Locked Loop (PLL)
*********************************************************************/

/* Bit definitions and macros for PCR */
#define PLL_PCR_OUTDIV1(x)		(((x)&0x0000000F))	/* Output divider for CPU clock frequency */
#define PLL_PCR_OUTDIV2(x)		(((x)&0x0000000F)<<4)	/* Output divider for bus/flexbus clock frequency */
#define PLL_PCR_OUTDIV3(x)		(((x)&0x0000000F)<<8)	/* Output divider for SDRAM clock frequency */
#define PLL_PCR_OUTDIV5(x)		(((x)&0x0000000F)<<16)	/* Output divider for USB clock frequency */
#define PLL_PCR_PFDR(x)			(((x)&0x000000FF)<<24)	/* Feedback divider for VCO frequency */
#define PLL_PCR_PFDR_MASK		(0x000F0000)
#define PLL_PCR_OUTDIV5_MASK		(0x000F0000)
#define PLL_PCR_OUTDIV3_MASK		(0x00000F00)
#define PLL_PCR_OUTDIV2_MASK		(0x000000F0)
#define PLL_PCR_OUTDIV1_MASK		(0x0000000F)

/* Bit definitions and macros for PSR */
#define PLL_PSR_LOCKS			(0x00000001)	/* PLL lost lock - sticky */
#define PLL_PSR_LOCK			(0x00000002)	/* PLL lock status */
#define PLL_PSR_LOLIRQ			(0x00000004)	/* PLL loss-of-lock interrupt enable */
#define PLL_PSR_LOLRE			(0x00000008)	/* PLL loss-of-lock reset enable */

/********************************************************************/

#endif				/* __MCF5227X__ */
