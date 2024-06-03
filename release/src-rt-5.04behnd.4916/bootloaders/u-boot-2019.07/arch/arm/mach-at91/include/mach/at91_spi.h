/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * [origin: Linux kernel include/asm-arm/arch-at91/at91_spi.h]
 *
 * Copyright (C) 2005 Ivan Kokshaysky
 * Copyright (C) SAN People
 *
 * Serial Peripheral Interface (SPI) registers.
 * Based on AT91RM9200 datasheet revision E.
 */

#ifndef AT91_SPI_H
#define AT91_SPI_H

#include <asm/arch/at91_pdc.h>

typedef struct at91_spi {
	u32		cr;		/* 0x00 Control Register */
	u32		mr;		/* 0x04 Mode Register */
	u32		rdr;		/* 0x08 Receive Data Register */
	u32		tdr;		/* 0x0C Transmit Data Register */
	u32		sr;		/* 0x10 Status Register */
	u32		ier;		/* 0x14 Interrupt Enable Register */
	u32		idr;		/* 0x18 Interrupt Disable Register */
	u32		imr;		/* 0x1C Interrupt Mask Register */
	u32		reserve1[4];
	u32		csr[4];		/* 0x30 Chip Select Register 0-3 */
	u32		reserve2[48];
	at91_pdc_t	pdc;
} at91_spi_t;

#ifdef CONFIG_ATMEL_LEGACY

#define AT91_SPI_CR			0x00		/* Control Register */
#define		AT91_SPI_SPIEN		(1 <<  0)		/* SPI Enable */
#define		AT91_SPI_SPIDIS		(1 <<  1)		/* SPI Disable */
#define		AT91_SPI_SWRST		(1 <<  7)		/* SPI Software Reset */
#define		AT91_SPI_LASTXFER	(1 << 24)		/* Last Transfer [SAM9261 only] */

#define AT91_SPI_MR			0x04		/* Mode Register */
#define		AT91_SPI_MSTR		(1    <<  0)		/* Master/Slave Mode */
#define		AT91_SPI_PS		(1    <<  1)		/* Peripheral Select */
#define			AT91_SPI_PS_FIXED	(0 << 1)
#define			AT91_SPI_PS_VARIABLE	(1 << 1)
#define		AT91_SPI_PCSDEC		(1    <<  2)		/* Chip Select Decode */
#define		AT91_SPI_DIV32		(1    <<  3)		/* Clock Selection [AT91RM9200 only] */
#define		AT91_SPI_MODFDIS	(1    <<  4)		/* Mode Fault Detection */
#define		AT91_SPI_LLB		(1    <<  7)		/* Local Loopback Enable */
#define		AT91_SPI_PCS		(0xf  << 16)		/* Peripheral Chip Select */
#define		AT91_SPI_DLYBCS		(0xff << 24)		/* Delay Between Chip Selects */

#define AT91_SPI_RDR		0x08			/* Receive Data Register */
#define		AT91_SPI_RD		(0xffff <<  0)		/* Receive Data */
#define		AT91_SPI_PCS		(0xf	<< 16)		/* Peripheral Chip Select */

#define AT91_SPI_TDR		0x0c			/* Transmit Data Register */
#define		AT91_SPI_TD		(0xffff <<  0)		/* Transmit Data */
#define		AT91_SPI_PCS		(0xf	<< 16)		/* Peripheral Chip Select */
#define		AT91_SPI_LASTXFER	(1	<< 24)		/* Last Transfer [SAM9261 only] */

#define AT91_SPI_SR		0x10			/* Status Register */
#define		AT91_SPI_RDRF		(1 <<  0)		/* Receive Data Register Full */
#define		AT91_SPI_TDRE		(1 <<  1)		/* Transmit Data Register Full */
#define		AT91_SPI_MODF		(1 <<  2)		/* Mode Fault Error */
#define		AT91_SPI_OVRES		(1 <<  3)		/* Overrun Error Status */
#define		AT91_SPI_ENDRX		(1 <<  4)		/* End of RX buffer */
#define		AT91_SPI_ENDTX		(1 <<  5)		/* End of TX buffer */
#define		AT91_SPI_RXBUFF		(1 <<  6)		/* RX Buffer Full */
#define		AT91_SPI_TXBUFE		(1 <<  7)		/* TX Buffer Empty */
#define		AT91_SPI_NSSR		(1 <<  8)		/* NSS Rising [SAM9261 only] */
#define		AT91_SPI_TXEMPTY	(1 <<  9)		/* Transmission Register Empty [SAM9261 only] */
#define		AT91_SPI_SPIENS		(1 << 16)		/* SPI Enable Status */

#define AT91_SPI_IER		0x14			/* Interrupt Enable Register */
#define AT91_SPI_IDR		0x18			/* Interrupt Disable Register */
#define AT91_SPI_IMR		0x1c			/* Interrupt Mask Register */

#define AT91_SPI_CSR(n)		(0x30 + ((n) * 4))	/* Chip Select Registers 0-3 */
#define		AT91_SPI_CPOL		(1    <<  0)		/* Clock Polarity */
#define		AT91_SPI_NCPHA		(1    <<  1)		/* Clock Phase */
#define		AT91_SPI_CSAAT		(1    <<  3)		/* Chip Select Active After Transfer [SAM9261 only] */
#define		AT91_SPI_BITS		(0xf  <<  4)		/* Bits Per Transfer */
#define			AT91_SPI_BITS_8		(0 << 4)
#define			AT91_SPI_BITS_9		(1 << 4)
#define			AT91_SPI_BITS_10	(2 << 4)
#define			AT91_SPI_BITS_11	(3 << 4)
#define			AT91_SPI_BITS_12	(4 << 4)
#define			AT91_SPI_BITS_13	(5 << 4)
#define			AT91_SPI_BITS_14	(6 << 4)
#define			AT91_SPI_BITS_15	(7 << 4)
#define			AT91_SPI_BITS_16	(8 << 4)
#define		AT91_SPI_SCBR		(0xff <<  8)		/* Serial Clock Baud Rate */
#define		AT91_SPI_DLYBS		(0xff << 16)		/* Delay before SPCK */
#define		AT91_SPI_DLYBCT		(0xff << 24)		/* Delay between Consecutive Transfers */

#define AT91_SPI_RPR		0x0100			/* Receive Pointer Register */

#define AT91_SPI_RCR		0x0104			/* Receive Counter Register */

#define AT91_SPI_TPR		0x0108			/* Transmit Pointer Register */

#define AT91_SPI_TCR		0x010c			/* Transmit Counter Register */

#define AT91_SPI_RNPR		0x0110			/* Receive Next Pointer Register */

#define AT91_SPI_RNCR		0x0114			/* Receive Next Counter Register */

#define AT91_SPI_TNPR		0x0118			/* Transmit Next Pointer Register */

#define AT91_SPI_TNCR		0x011c			/* Transmit Next Counter Register */

#define AT91_SPI_PTCR		0x0120			/* PDC Transfer Control Register */
#define		AT91_SPI_RXTEN		(0x1 << 0)		/* Receiver Transfer Enable */
#define		AT91_SPI_RXTDIS		(0x1 << 1)		/* Receiver Transfer Disable */
#define		AT91_SPI_TXTEN		(0x1 << 8)		/* Transmitter Transfer Enable */
#define		AT91_SPI_TXTDIS		(0x1 << 9)		/* Transmitter Transfer Disable */

#define AT91_SPI_PTSR		0x0124			/* PDC Transfer Status Register */

#endif /* CONFIG_ATMEL_LEGACY */

#endif
