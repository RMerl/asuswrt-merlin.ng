/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * K2E: SoC definitions
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef __ASM_ARCH_HARDWARE_K2E_H
#define __ASM_ARCH_HARDWARE_K2E_H

/* PA SS Registers */
#define KS2_PASS_BASE			0x24000000

/* Power and Sleep Controller (PSC) Domains */
#define KS2_LPSC_MOD_RST		0
#define KS2_LPSC_USB_1			1
#define KS2_LPSC_USB			2
#define KS2_LPSC_EMIF25_SPI		3
#define KS2_LPSC_TSIP			4
#define KS2_LPSC_DEBUGSS_TRC		5
#define KS2_LPSC_TETB_TRC		6
#define KS2_LPSC_PKTPROC		7
#define KS2_LPSC_PA			KS2_LPSC_PKTPROC
#define KS2_LPSC_SGMII			8
#define KS2_LPSC_CPGMAC			KS2_LPSC_SGMII
#define KS2_LPSC_CRYPTO			9
#define KS2_LPSC_PCIE			10
#define KS2_LPSC_VUSR0			12
#define KS2_LPSC_CHIP_SRSS		13
#define KS2_LPSC_MSMC			14
#define KS2_LPSC_EMIF4F_DDR3		23
#define KS2_LPSC_PCIE_1			27
#define KS2_LPSC_XGE			50

/* Chip Interrupt Controller */
#define KS2_CIC2_DDR3_ECC_IRQ_NUM	-1	/* not defined in K2E */
#define KS2_CIC2_DDR3_ECC_CHAN_NUM	-1	/* not defined in K2E */

/* SGMII SerDes */
#define KS2_SGMII_SERDES2_BASE		0x02324000
#define KS2_LANES_PER_SGMII_SERDES	4

/* Number of DSP cores */
#define KS2_NUM_DSPS			1

/* NETCP pktdma */
#define KS2_NETCP_PDMA_CTRL_BASE	0x24186000
#define KS2_NETCP_PDMA_TX_BASE		0x24187000
#define KS2_NETCP_PDMA_TX_CH_NUM	21
#define KS2_NETCP_PDMA_RX_BASE		0x24188000
#define KS2_NETCP_PDMA_RX_CH_NUM	91
#define KS2_NETCP_PDMA_SCHED_BASE	0x24186100
#define KS2_NETCP_PDMA_RX_FLOW_BASE	0x24189000
#define KS2_NETCP_PDMA_RX_FLOW_NUM	96
#define KS2_NETCP_PDMA_TX_SND_QUEUE	896

/* NETCP */
#define KS2_NETCP_BASE			0x24000000

#endif
