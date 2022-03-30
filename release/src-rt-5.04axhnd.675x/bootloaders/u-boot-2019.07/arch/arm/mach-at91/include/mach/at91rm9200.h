/* SPDX-License-Identifier: GPL-2.0+ */
/*
 */

#ifndef __AT91RM9200_H__
#define __AT91RM9200_H__

#define CONFIG_ARCH_CPU_INIT	/* we need arch_cpu_init() for hw timers */
#define CONFIG_AT91_GPIO	/* and require always gpio features */

/* Periperial Identifiers */

#define ATMEL_ID_SYS	1	/* System Peripheral */
#define ATMEL_ID_PIOA	2	/* PIO port A */
#define ATMEL_ID_PIOB	3	/* PIO port B */
#define ATMEL_ID_PIOC	4	/* PIO port C */
#define ATMEL_ID_PIOD	5	/* PIO port D BGA only */
#define ATMEL_ID_USART0	6	/* USART 0 */
#define ATMEL_ID_USART1	7	/* USART 1 */
#define ATMEL_ID_USART2	8	/* USART 2 */
#define ATMEL_ID_USART3	9	/* USART 3 */
#define ATMEL_ID_MCI	10	/* Multimedia Card Interface */
#define ATMEL_ID_UDP	11	/* USB Device Port */
#define ATMEL_ID_TWI	12	/* Two Wire Interface */
#define ATMEL_ID_SPI	13	/* Serial Peripheral Interface */
#define ATMEL_ID_SSC0	14	/* Synch. Serial Controller 0 */
#define ATMEL_ID_SSC1	15	/* Synch. Serial Controller 1 */
#define ATMEL_ID_SSC2	16	/* Synch. Serial Controller 2 */
#define ATMEL_ID_TC0	17	/* Timer Counter 0 */
#define ATMEL_ID_TC1	18	/* Timer Counter 1 */
#define ATMEL_ID_TC2	19	/* Timer Counter 2 */
#define ATMEL_ID_TC3	20	/* Timer Counter 3 */
#define ATMEL_ID_TC4	21	/* Timer Counter 4 */
#define ATMEL_ID_TC5	22	/* Timer Counter 5 */
#define ATMEL_ID_UHP	23	/* OHCI USB Host Port */
#define ATMEL_ID_EMAC	24	/* Ethernet MAC */
#define ATMEL_ID_IRQ0	25	/* Advanced Interrupt Controller */
#define ATMEL_ID_IRQ1	26	/* Advanced Interrupt Controller */
#define ATMEL_ID_IRQ2	27	/* Advanced Interrupt Controller */
#define ATMEL_ID_IRQ3	28	/* Advanced Interrupt Controller */
#define ATMEL_ID_IRQ4	29	/* Advanced Interrupt Controller */
#define ATMEL_ID_IRQ5	30	/* Advanced Interrupt Controller */
#define ATMEL_ID_IRQ6	31	/* Advanced Interrupt Controller */

#define ATMEL_USB_HOST_BASE	0x00300000

#define ATMEL_BASE_TC		0xFFFA0000
#define ATMEL_BASE_UDP		0xFFFB0000
#define ATMEL_BASE_MCI		0xFFFB4000
#define ATMEL_BASE_TWI		0xFFFB8000
#define ATMEL_BASE_EMAC		0xFFFBC000
#define ATMEL_BASE_USART	0xFFFC0000	/* 4x 0x4000 Offset */
#define ATMEL_BASE_USART0	ATMEL_BASE_USART
#define ATMEL_BASE_USART1	(ATMEL_BASE_USART + 0x4000)
#define ATMEL_BASE_USART2	(ATMEL_BASE_USART + 0x8000)
#define ATMEL_BASE_USART3	(ATMEL_BASE_USART + 0xC000)

#define ATMEL_BASE_SCC		0xFFFD0000	/* 4x 0x4000 Offset */
#define ATMEL_BASE_SPI		0xFFFE0000

#define ATMEL_BASE_AIC		0xFFFFF000
#define ATMEL_BASE_DBGU		0xFFFFF200
#define ATMEL_BASE_PIO		0xFFFFF400	/* 4x 0x200 Offset */
#define ATMEL_BASE_PIOA		0xFFFFF400
#define ATMEL_BASE_PIOB		0xFFFFF600
#define ATMEL_BASE_PIOC		0xFFFFF800
#define ATMEL_BASE_PIOD		0xFFFFFA00
#define ATMEL_BASE_PMC		0xFFFFFC00
#define ATMEL_BASE_ST		0xFFFFFD00
#define ATMEL_BASE_RTC		0xFFFFFE00
#define ATMEL_BASE_MC		0xFFFFFF00

#define AT91_PIO_BASE	ATMEL_BASE_PIO

/* AT91RM9200 Periperial Multiplexing A */
/* Port A */
#define ATMEL_PMX_AA_EREFCK	0x00000080
#define ATMEL_PMX_AA_ETXCK	0x00000080
#define ATMEL_PMX_AA_ETXEN	0x00000100
#define ATMEL_PMX_AA_ETX0	0x00000200
#define ATMEL_PMX_AA_ETX1	0x00000400
#define ATMEL_PMX_AA_ECRS	0x00000800
#define ATMEL_PMX_AA_ECRSDV	0x00000800
#define ATMEL_PMX_AA_ERX0	0x00001000
#define ATMEL_PMX_AA_ERX1	0x00002000
#define ATMEL_PMX_AA_ERXER	0x00004000
#define ATMEL_PMX_AA_EMDC	0x00008000
#define ATMEL_PMX_AA_EMDIO	0x00010000

#define ATMEL_PMX_AA_TXD2	0x00800000

#define ATMEL_PMX_AA_TWD	0x02000000
#define ATMEL_PMX_AA_TWCK	0x04000000

/* Port B */
#define ATMEL_PMX_BA_ERXCK	0x00080000
#define ATMEL_PMX_BA_ECOL	0x00040000
#define ATMEL_PMX_BA_ERXDV	0x00020000
#define ATMEL_PMX_BA_ERX3	0x00010000
#define ATMEL_PMX_BA_ERX2	0x00008000
#define ATMEL_PMX_BA_ETXER	0x00004000
#define ATMEL_PMX_BA_ETX3	0x00002000
#define ATMEL_PMX_BA_ETX2	0x00001000

/* Port B */

#define ATMEL_PMX_CA_BFCK	0x00000001
#define ATMEL_PMX_CA_BFRDY	0x00000002
#define ATMEL_PMX_CA_SMOE	0x00000002
#define ATMEL_PMX_CA_BFAVD	0x00000004
#define ATMEL_PMX_CA_BFBAA	0x00000008
#define ATMEL_PMX_CA_SMWE	0x00000008
#define ATMEL_PMX_CA_BFOE	0x00000010
#define ATMEL_PMX_CA_BFWE	0x00000020
#define ATMEL_PMX_CA_NWAIT	0x00000040
#define ATMEL_PMX_CA_A23	0x00000080
#define ATMEL_PMX_CA_A24	0x00000100
#define ATMEL_PMX_CA_A25	0x00000200
#define ATMEL_PMX_CA_CFRNW	0x00000200
#define ATMEL_PMX_CA_NCS4	0x00000400
#define ATMEL_PMX_CA_CFCS	0x00000400
#define ATMEL_PMX_CA_NCS5	0x00000800
#define ATMEL_PMX_CA_CFCE1	0x00001000
#define ATMEL_PMX_CA_NCS6	0x00001000
#define ATMEL_PMX_CA_CFCE2	0x00002000
#define ATMEL_PMX_CA_NCS7	0x00002000
#define ATMEL_PMX_CA_D16_31	0xFFFF0000

#define ATMEL_PIO_PORTS		4	/* theese SoCs have 4 PIO */
#define ATMEL_PMC_UHP		AT91RM9200_PMC_UHP

#define CONFIG_SYS_ATMEL_CPU_NAME	"AT91RM9200"

#endif
