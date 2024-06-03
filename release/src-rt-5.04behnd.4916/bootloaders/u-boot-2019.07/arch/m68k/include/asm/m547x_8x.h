/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * mcf547x_8x.h -- Definitions for Freescale Coldfire 547x_8x
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef mcf547x_8x_h
#define mcf547x_8x_h

/*********************************************************************
* XLB Arbiter (XLB)
*********************************************************************/
/* Bit definitions and macros for XARB_CFG */
#define XARB_CFG_AT			(0x00000002)
#define XARB_CFG_DT			(0x00000004)
#define XARB_CFG_BA			(0x00000008)
#define XARB_CFG_PM(x)			(((x)&0x00000003)<<5)
#define XARB_CFG_SP(x)			(((x)&0x00000007)<<8)
#define XARB_CFG_PLDIS			(0x80000000)

/* Bit definitions and macros for XARB_SR */
#define XARB_SR_AT			(0x00000001)
#define XARB_SR_DT			(0x00000002)
#define XARB_SR_BA			(0x00000004)
#define XARB_SR_TTM			(0x00000008)
#define XARB_SR_ECW			(0x00000010)
#define XARB_SR_TTR			(0x00000020)
#define XARB_SR_TTA			(0x00000040)
#define XARB_SR_MM			(0x00000080)
#define XARB_SR_SEA			(0x00000100)

/* Bit definitions and macros for XARB_IMR */
#define XARB_IMR_ATE			(0x00000001)
#define XARB_IMR_DTE			(0x00000002)
#define XARB_IMR_BAE			(0x00000004)
#define XARB_IMR_TTME			(0x00000008)
#define XARB_IMR_ECWE			(0x00000010)
#define XARB_IMR_TTRE			(0x00000020)
#define XARB_IMR_TTAE			(0x00000040)
#define XARB_IMR_MME			(0x00000080)
#define XARB_IMR_SEAE			(0x00000100)

/* Bit definitions and macros for XARB_SIGCAP */
#define XARB_SIGCAP_TT(x)		((x)&0x0000001F)
#define XARB_SIGCAP_TBST		(0x00000020)
#define XARB_SIGCAP_TSIZ(x)		(((x)&0x00000007)<<7)

/* Bit definitions and macros for XARB_PRIEN */
#define XARB_PRIEN_M0			(0x00000001)
#define XARB_PRIEN_M2			(0x00000004)
#define XARB_PRIEN_M3			(0x00000008)

/* Bit definitions and macros for XARB_PRI */
#define XARB_PRI_M0P(x)			(((x)&0x00000007)<<0)
#define XARB_PRI_M2P(x)			(((x)&0x00000007)<<8)
#define XARB_PRI_M3P(x)			(((x)&0x00000007)<<12)

/*********************************************************************
* General Purpose I/O (GPIO)
*********************************************************************/
/* Bit definitions and macros for GPIO_PAR_FBCTL */
#define GPIO_PAR_FBCTL_TS(x)		(((x)&0x0003)<<0)
#define GPIO_PAR_FBCTL_TA		(0x0004)
#define GPIO_PAR_FBCTL_RWB(x)		(((x)&0x0003)<<4)
#define GPIO_PAR_FBCTL_OE		(0x0040)
#define GPIO_PAR_FBCTL_BWE0		(0x0100)
#define GPIO_PAR_FBCTL_BWE1		(0x0400)
#define GPIO_PAR_FBCTL_BWE2		(0x1000)
#define GPIO_PAR_FBCTL_BWE3		(0x4000)
#define GPIO_PAR_FBCTL_TS_GPIO		(0)
#define GPIO_PAR_FBCTL_TS_TBST		(2)
#define GPIO_PAR_FBCTL_TS_TS		(3)
#define GPIO_PAR_FBCTL_RWB_GPIO		(0x0000)
#define GPIO_PAR_FBCTL_RWB_TBST		(0x0020)
#define GPIO_PAR_FBCTL_RWB_RWB		(0x0030)

/* Bit definitions and macros for GPIO_PAR_FBCS */
#define GPIO_PAR_FBCS_CS1		(0x02)
#define GPIO_PAR_FBCS_CS2		(0x04)
#define GPIO_PAR_FBCS_CS3		(0x08)
#define GPIO_PAR_FBCS_CS4		(0x10)
#define GPIO_PAR_FBCS_CS5		(0x20)

/* Bit definitions and macros for GPIO_PAR_DMA */
#define GPIO_PAR_DMA_DREQ0(x)		(((x)&0x03)<<0)
#define GPIO_PAR_DMA_DREQ1(x)		(((x)&0x03)<<2)
#define GPIO_PAR_DMA_DACK0(x)		(((x)&0x03)<<4)
#define GPIO_PAR_DMA_DACK1(x)		(((x)&0x03)<<6)
#define GPIO_PAR_DMA_DACKx_GPIO		(0)
#define GPIO_PAR_DMA_DACKx_TOUT		(2)
#define GPIO_PAR_DMA_DACKx_DACK		(3)
#define GPIO_PAR_DMA_DREQx_GPIO		(0)
#define GPIO_PAR_DMA_DREQx_TIN		(2)
#define GPIO_PAR_DMA_DREQx_DREQ		(3)

/* Bit definitions and macros for GPIO_PAR_FECI2CIRQ */
#define GPIO_PAR_FECI2CIRQ_IRQ5		(0x0001)
#define GPIO_PAR_FECI2CIRQ_IRQ6		(0x0002)
#define GPIO_PAR_FECI2CIRQ_SCL		(0x0004)
#define GPIO_PAR_FECI2CIRQ_SDA		(0x0008)
#define GPIO_PAR_FECI2CIRQ_E1MDC(x)	(((x)&0x0003)<<6)
#define GPIO_PAR_FECI2CIRQ_E1MDIO(x)	(((x)&0x0003)<<8)
#define GPIO_PAR_FECI2CIRQ_E1MII	(0x0400)
#define GPIO_PAR_FECI2CIRQ_E17		(0x0800)
#define GPIO_PAR_FECI2CIRQ_E0MDC	(0x1000)
#define GPIO_PAR_FECI2CIRQ_E0MDIO	(0x2000)
#define GPIO_PAR_FECI2CIRQ_E0MII	(0x4000)
#define GPIO_PAR_FECI2CIRQ_E07		(0x8000)
#define GPIO_PAR_FECI2CIRQ_E1MDIO_CANRX	(0x0000)
#define GPIO_PAR_FECI2CIRQ_E1MDIO_SDA	(0x0200)
#define GPIO_PAR_FECI2CIRQ_E1MDIO_EMDIO	(0x0300)
#define GPIO_PAR_FECI2CIRQ_E1MDC_CANTX	(0x0000)
#define GPIO_PAR_FECI2CIRQ_E1MDC_SCL	(0x0080)
#define GPIO_PAR_FECI2CIRQ_E1MDC_EMDC	(0x00C0)

/* Bit definitions and macros for GPIO_PAR_PCIBG */
#define GPIO_PAR_PCIBG_PCIBG0(x)	(((x)&0x0003)<<0)
#define GPIO_PAR_PCIBG_PCIBG1(x)	(((x)&0x0003)<<2)
#define GPIO_PAR_PCIBG_PCIBG2(x)	(((x)&0x0003)<<4)
#define GPIO_PAR_PCIBG_PCIBG3(x)	(((x)&0x0003)<<6)
#define GPIO_PAR_PCIBG_PCIBG4(x)	(((x)&0x0003)<<8)

/* Bit definitions and macros for GPIO_PAR_PCIBR */
#define GPIO_PAR_PCIBR_PCIBR0(x)	(((x)&0x0003)<<0)
#define GPIO_PAR_PCIBR_PCIBR1(x)	(((x)&0x0003)<<2)
#define GPIO_PAR_PCIBR_PCIBR2(x)	(((x)&0x0003)<<4)
#define GPIO_PAR_PCIBR_PCIBR3(x)	(((x)&0x0003)<<6)
#define GPIO_PAR_PCIBR_PCIBR4(x)	(((x)&0x0003)<<8)

/* Bit definitions and macros for GPIO_PAR_PSC3 */
#define GPIO_PAR_PSC3_TXD3		(0x04)
#define GPIO_PAR_PSC3_RXD3		(0x08)
#define GPIO_PAR_PSC3_RTS3(x)		(((x)&0x03)<<4)
#define GPIO_PAR_PSC3_CTS3(x)		(((x)&0x03)<<6)
#define GPIO_PAR_PSC3_CTS3_GPIO		(0x00)
#define GPIO_PAR_PSC3_CTS3_BCLK		(0x80)
#define GPIO_PAR_PSC3_CTS3_CTS		(0xC0)
#define GPIO_PAR_PSC3_RTS3_GPIO		(0x00)
#define GPIO_PAR_PSC3_RTS3_FSYNC	(0x20)
#define GPIO_PAR_PSC3_RTS3_RTS		(0x30)
#define GPIO_PAR_PSC3_CTS2_CANRX	(0x40)

/* Bit definitions and macros for GPIO_PAR_PSC2 */
#define GPIO_PAR_PSC2_TXD2		(0x04)
#define GPIO_PAR_PSC2_RXD2		(0x08)
#define GPIO_PAR_PSC2_RTS2(x)		(((x)&0x03)<<4)
#define GPIO_PAR_PSC2_CTS2(x)		(((x)&0x03)<<6)
#define GPIO_PAR_PSC2_CTS2_GPIO		(0x00)
#define GPIO_PAR_PSC2_CTS2_BCLK		(0x80)
#define GPIO_PAR_PSC2_CTS2_CTS		(0xC0)
#define GPIO_PAR_PSC2_RTS2_GPIO		(0x00)
#define GPIO_PAR_PSC2_RTS2_CANTX	(0x10)
#define GPIO_PAR_PSC2_RTS2_FSYNC	(0x20)
#define GPIO_PAR_PSC2_RTS2_RTS		(0x30)

/* Bit definitions and macros for GPIO_PAR_PSC1 */
#define GPIO_PAR_PSC1_TXD1		(0x04)
#define GPIO_PAR_PSC1_RXD1		(0x08)
#define GPIO_PAR_PSC1_RTS1(x)		(((x)&0x03)<<4)
#define GPIO_PAR_PSC1_CTS1(x)		(((x)&0x03)<<6)
#define GPIO_PAR_PSC1_CTS1_GPIO		(0x00)
#define GPIO_PAR_PSC1_CTS1_BCLK		(0x80)
#define GPIO_PAR_PSC1_CTS1_CTS		(0xC0)
#define GPIO_PAR_PSC1_RTS1_GPIO		(0x00)
#define GPIO_PAR_PSC1_RTS1_FSYNC	(0x20)
#define GPIO_PAR_PSC1_RTS1_RTS		(0x30)

/* Bit definitions and macros for GPIO_PAR_PSC0 */
#define GPIO_PAR_PSC0_TXD0		(0x04)
#define GPIO_PAR_PSC0_RXD0		(0x08)
#define GPIO_PAR_PSC0_RTS0(x)		(((x)&0x03)<<4)
#define GPIO_PAR_PSC0_CTS0(x)		(((x)&0x03)<<6)
#define GPIO_PAR_PSC0_CTS0_GPIO		(0x00)
#define GPIO_PAR_PSC0_CTS0_BCLK		(0x80)
#define GPIO_PAR_PSC0_CTS0_CTS		(0xC0)
#define GPIO_PAR_PSC0_RTS0_GPIO		(0x00)
#define GPIO_PAR_PSC0_RTS0_FSYNC	(0x20)
#define GPIO_PAR_PSC0_RTS0_RTS		(0x30)

/* Bit definitions and macros for GPIO_PAR_DSPI */
#define GPIO_PAR_DSPI_SOUT(x)		(((x)&0x0003)<<0)
#define GPIO_PAR_DSPI_SIN(x)		(((x)&0x0003)<<2)
#define GPIO_PAR_DSPI_SCK(x)		(((x)&0x0003)<<4)
#define GPIO_PAR_DSPI_CS0(x)		(((x)&0x0003)<<6)
#define GPIO_PAR_DSPI_CS2(x)		(((x)&0x0003)<<8)
#define GPIO_PAR_DSPI_CS3(x)		(((x)&0x0003)<<10)
#define GPIO_PAR_DSPI_CS5		(0x1000)
#define GPIO_PAR_DSPI_CS3_GPIO		(0x0000)
#define GPIO_PAR_DSPI_CS3_CANTX		(0x0400)
#define GPIO_PAR_DSPI_CS3_TOUT		(0x0800)
#define GPIO_PAR_DSPI_CS3_DSPICS	(0x0C00)
#define GPIO_PAR_DSPI_CS2_GPIO		(0x0000)
#define GPIO_PAR_DSPI_CS2_CANTX		(0x0100)
#define GPIO_PAR_DSPI_CS2_TOUT		(0x0200)
#define GPIO_PAR_DSPI_CS2_DSPICS	(0x0300)
#define GPIO_PAR_DSPI_CS0_GPIO		(0x0000)
#define GPIO_PAR_DSPI_CS0_FSYNC		(0x0040)
#define GPIO_PAR_DSPI_CS0_RTS		(0x0080)
#define GPIO_PAR_DSPI_CS0_DSPICS	(0x00C0)
#define GPIO_PAR_DSPI_SCK_GPIO		(0x0000)
#define GPIO_PAR_DSPI_SCK_BCLK		(0x0010)
#define GPIO_PAR_DSPI_SCK_CTS		(0x0020)
#define GPIO_PAR_DSPI_SCK_SCK		(0x0030)
#define GPIO_PAR_DSPI_SIN_GPIO		(0x0000)
#define GPIO_PAR_DSPI_SIN_RXD		(0x0008)
#define GPIO_PAR_DSPI_SIN_SIN		(0x000C)
#define GPIO_PAR_DSPI_SOUT_GPIO		(0x0000)
#define GPIO_PAR_DSPI_SOUT_TXD		(0x0002)
#define GPIO_PAR_DSPI_SOUT_SOUT		(0x0003)

/* Bit definitions and macros for GPIO_PAR_TIMER */
#define GPIO_PAR_TIMER_TOUT2		(0x01)
#define GPIO_PAR_TIMER_TIN2(x)		(((x)&0x03)<<1)
#define GPIO_PAR_TIMER_TOUT3		(0x08)
#define GPIO_PAR_TIMER_TIN3(x)		(((x)&0x03)<<4)
#define GPIO_PAR_TIMER_TIN3_CANRX	(0x00)
#define GPIO_PAR_TIMER_TIN3_IRQ		(0x20)
#define GPIO_PAR_TIMER_TIN3_TIN		(0x30)
#define GPIO_PAR_TIMER_TIN2_CANRX	(0x00)
#define GPIO_PAR_TIMER_TIN2_IRQ		(0x04)
#define GPIO_PAR_TIMER_TIN2_TIN		(0x06)

/*********************************************************************
* Slice Timer (SLT)
*********************************************************************/
#define SLT_CR_RUN			(0x04000000)
#define SLT_CR_IEN			(0x02000000)
#define SLT_CR_TEN			(0x01000000)

#define SLT_SR_BE			(0x02000000)
#define SLT_SR_ST			(0x01000000)

/*********************************************************************
* Interrupt Controller (INTC)
*********************************************************************/
#define INT0_LO_RSVD0			(0)
#define INT0_LO_EPORT1			(1)
#define INT0_LO_EPORT2			(2)
#define INT0_LO_EPORT3			(3)
#define INT0_LO_EPORT4			(4)
#define INT0_LO_EPORT5			(5)
#define INT0_LO_EPORT6			(6)
#define INT0_LO_EPORT7			(7)
#define INT0_LO_EP0ISR			(15)
#define INT0_LO_EP1ISR			(16)
#define INT0_LO_EP2ISR			(17)
#define INT0_LO_EP3ISR			(18)
#define INT0_LO_EP4ISR			(19)
#define INT0_LO_EP5ISR			(20)
#define INT0_LO_EP6ISR			(21)
#define INT0_LO_USBISR			(22)
#define INT0_LO_USBAISR			(23)
#define INT0_LO_USB			(24)
#define INT1_LO_DSPI_RFOF_TFUF		(25)
#define INT1_LO_DSPI_RFOF		(26)
#define INT1_LO_DSPI_RFDF		(27)
#define INT1_LO_DSPI_TFUF		(28)
#define INT1_LO_DSPI_TCF		(29)
#define INT1_LO_DSPI_TFFF		(30)
#define INT1_LO_DSPI_EOQF		(31)

#define INT0_HI_UART3			(32)
#define INT0_HI_UART2			(33)
#define INT0_HI_UART1			(34)
#define INT0_HI_UART0			(35)
#define INT0_HI_COMMTIM_TC		(36)
#define INT0_HI_SEC			(37)
#define INT0_HI_FEC1			(38)
#define INT0_HI_FEC0			(39)
#define INT0_HI_I2C			(40)
#define INT0_HI_PCIARB			(41)
#define INT0_HI_CBPCI			(42)
#define INT0_HI_XLBPCI			(43)
#define INT0_HI_XLBARB			(47)
#define INT0_HI_DMA			(48)
#define INT0_HI_CAN0_ERROR		(49)
#define INT0_HI_CAN0_BUSOFF		(50)
#define INT0_HI_CAN0_MBOR		(51)
#define INT0_HI_SLT1			(53)
#define INT0_HI_SLT0			(54)
#define INT0_HI_CAN1_ERROR		(55)
#define INT0_HI_CAN1_BUSOFF		(56)
#define INT0_HI_CAN1_MBOR		(57)
#define INT0_HI_GPT3			(59)
#define INT0_HI_GPT2			(60)
#define INT0_HI_GPT1			(61)
#define INT0_HI_GPT0			(62)

/*********************************************************************
* General Purpose Timers (GPTMR)
*********************************************************************/
/* Enable and Mode Select */
#define GPT_OCT(x)			(x & 0x3)<<4	/* Output Compare Type */
#define GPT_ICT(x)			(x & 0x3)	/* Input Capture Type */
#define GPT_CTRL_WDEN			0x80		/* Watchdog Enable */
#define GPT_CTRL_CE			0x10		/* Counter Enable */
#define GPT_CTRL_STPCNT			0x04		/* Stop continous */
#define GPT_CTRL_ODRAIN			0x02		/* Open Drain */
#define GPT_CTRL_INTEN			0x01		/* Interrupt Enable */
#define GPT_MODE_GPIO(x)		(x & 0x3)<<4	/* Gpio Mode Type */
#define GPT_TMS_ICT			0x01		/* Input Capture Enable */
#define GPT_TMS_OCT			0x02		/* Output Capture Enable */
#define GPT_TMS_PWM			0x03		/* PWM Capture Enable */
#define GPT_TMS_SGPIO			0x04		/* PWM Capture Enable */

#define GPT_PWM_WIDTH(x)		(x & 0xffff)

/* Status */
#define GPT_STA_CAPTURE(x)		(x & 0xffff)

#define GPT_OVFPIN_OVF(x)		(x & 0x70)
#define GPT_OVFPIN_PIN			0x01

#define GPT_INT_TEXP			0x08
#define GPT_INT_PWMP			0x04
#define GPT_INT_COMP			0x02
#define GPT_INT_CAPT			0x01

/*********************************************************************
* PCI
*********************************************************************/

/* Bit definitions and macros for SCR */
#define PCI_SCR_PE			(0x80000000)	/* Parity Error detected */
#define PCI_SCR_SE			(0x40000000)	/* System error signalled */
#define PCI_SCR_MA			(0x20000000)	/* Master aboart received */
#define PCI_SCR_TR			(0x10000000)	/* Target abort received */
#define PCI_SCR_TS			(0x08000000)	/* Target abort signalled */
#define PCI_SCR_DT			(0x06000000)	/* PCI_DEVSEL timing */
#define PCI_SCR_DP			(0x01000000)	/* Master data parity err */
#define PCI_SCR_FC			(0x00800000)	/* Fast back-to-back */
#define PCI_SCR_R			(0x00400000)	/* Reserved */
#define PCI_SCR_66M			(0x00200000)	/* 66Mhz */
#define PCI_SCR_C			(0x00100000)	/* Capabilities list */
#define PCI_SCR_F			(0x00000200)	/* Fast back-to-back enable */
#define PCI_SCR_S			(0x00000100)	/* SERR enable */
#define PCI_SCR_ST			(0x00000080)	/* Addr and Data stepping */
#define PCI_SCR_PER			(0x00000040)	/* Parity error response */
#define PCI_SCR_V			(0x00000020)	/* VGA palette snoop enable */
#define PCI_SCR_MW			(0x00000010)	/* Memory write and invalidate enable */
#define PCI_SCR_SP			(0x00000008)	/* Special cycle monitor or ignore */
#define PCI_SCR_B			(0x00000004)	/* Bus master enable */
#define PCI_SCR_M			(0x00000002)	/* Memory access control */
#define PCI_SCR_IO			(0x00000001)	/* I/O access control */

#define PCI_CR1_BIST(x)			((x & 0xFF) << 24)	/* Built in self test */
#define PCI_CR1_HDR(x)			((x & 0xFF) << 16)	/* Header type */
#define PCI_CR1_LTMR(x)			((x & 0xF8) << 8)	/* Latency timer */
#define PCI_CR1_CLS(x)			(x & 0x0F)		/* Cache line size */

#define PCI_BAR_BAR0(x)			(x & 0xFFFC0000)
#define PCI_BAR_BAR1(x)			(x & 0xC0000000)
#define PCI_BAR_PREF			(0x00000004)	/* Prefetchable access */
#define PCI_BAR_RANGE			(0x00000002)	/* Fixed to 00 */
#define PCI_BAR_IO_M			(0x00000001)	/* IO / memory space */

#define PCI_CR2_MAXLAT(x)		((x & 0xFF) << 24)	/* Maximum latency */
#define PCI_CR2_MINGNT(x)		((x & 0xFF) << 16)	/* Minimum grant */
#define PCI_CR2_INTPIN(x)		((x & 0xFF) << 8)	/* Interrupt Pin */
#define PCI_CR2_INTLIN(x)		(x & 0xFF)	/* Interrupt Line */

#define PCI_GSCR_DRD			(0x80000000)	/* Delayed read discarded */
#define PCI_GSCR_PE			(0x20000000)	/* PCI_PERR detected */
#define PCI_GSCR_SE			(0x10000000)	/* SERR detected */
#define PCI_GSCR_ER			(0x08000000)	/* Error response detected */
#define PCI_GSCR_DRDE			(0x00008000)	/* Delayed read discarded enable */
#define PCI_GSCR_PEE			(0x00002000)	/* PERR detected interrupt enable */
#define PCI_GSCR_SEE			(0x00001000)	/* SERR detected interrupt enable */
#define PCI_GSCR_PR			(0x00000001)	/* PCI reset */

#define PCI_TCR1_LD			(0x01000000)	/* Latency rule disable */
#define PCI_TCR1_PID			(0x00020000)	/* Prefetch invalidate and disable */
#define PCI_TCR1_P			(0x00010000)	/* Prefetch reads */
#define PCI_TCR1_WCD			(0x00000100)	/* Write combine disable */

#define PCI_TCR1_B5E			(0x00002000)	/*  */
#define PCI_TCR1_B4E			(0x00001000)	/*  */
#define PCI_TCR1_B3E			(0x00000800)	/*  */
#define PCI_TCR1_B2E			(0x00000400)	/*  */
#define PCI_TCR1_B1E			(0x00000200)	/*  */
#define PCI_TCR1_B0E			(0x00000100)	/*  */
#define PCI_TCR1_CR			(0x00000001)	/*  */

#define PCI_TBATR_BAT0(x)		(x & 0xFFFC0000)
#define PCI_TBATR_BAT1(x)		(x & 0xC0000000)
#define PCI_TBATR_EN			(0x00000001)	/* Enable */

#define PCI_IWCR_W0C_IO			(0x08000000)	/* Windows Maps to PCI I/O */
#define PCI_IWCR_W0C_PRC_RDMUL		(0x04000000)	/* PCI Memory Read multiple */
#define PCI_IWCR_W0C_PRC_RDLN		(0x02000000)	/* PCI Memory Read line */
#define PCI_IWCR_W0C_PRC_RD		(0x00000000)	/* PCI Memory Read */
#define PCI_IWCR_W0C_EN			(0x01000000)	/* Enable - Register initialize */
#define PCI_IWCR_W1C_IO			(0x00080000)	/* Windows Maps to PCI I/O */
#define PCI_IWCR_W1C_PRC_RDMUL		(0x00040000)	/* PCI Memory Read multiple */
#define PCI_IWCR_W1C_PRC_RDLN		(0x00020000)	/* PCI Memory Read line */
#define PCI_IWCR_W1C_PRC_RD		(0x00000000)	/* PCI Memory Read */
#define PCI_IWCR_W1C_EN			(0x00010000)	/* Enable - Register initialize */
#define PCI_IWCR_W2C_IO			(0x00000800)	/* Windows Maps to PCI I/O */
#define PCI_IWCR_W2C_PRC_RDMUL		(0x00000400)	/* PCI Memory Read multiple */
#define PCI_IWCR_W2C_PRC_RDLN		(0x00000200)	/* PCI Memory Read line */
#define PCI_IWCR_W2C_PRC_RD		(0x00000000)	/* PCI Memory Read */
#define PCI_IWCR_W2C_EN			(0x00000100)	/* Enable - Register initialize */

#define PCI_ICR_REE			(0x04000000)	/* Retry error enable */
#define PCI_ICR_IAE			(0x02000000)	/* Initiator abort enable */
#define PCI_ICR_TAE			(0x01000000)	/* Target abort enable */
#define PCI_ICR_MAXRETRY(x)		((x) & 0x000000FF)

#define PCIARB_ACR_DS			(0x80000000)
#define PCIARB_ARC_EXTMINTEN(x)		(((x)&0x1F) << 17)
#define PCIARB_ARC_INTMINTEN		(0x00010000)
#define PCIARB_ARC_EXTMPRI(x)		(((x)&0x1F) << 1)
#define PCIARB_ARC_INTMPRI		(0x00000001)

#endif				/* mcf547x_8x_h */
