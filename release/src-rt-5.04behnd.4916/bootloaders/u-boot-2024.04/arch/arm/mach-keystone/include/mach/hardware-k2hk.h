/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * K2HK: SoC definitions
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef __ASM_ARCH_HARDWARE_K2HK_H
#define __ASM_ARCH_HARDWARE_K2HK_H

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

#define KS2_ARM_PLL_EN			BIT(13)

/* PA SS Registers */
#define KS2_PASS_BASE			0x02000000

/* Power and Sleep Controller (PSC) Domains */
#define KS2_LPSC_MOD			0
#define KS2_LPSC_DUMMY1			1
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
#define KS2_LPSC_SRIO			11
#define KS2_LPSC_VUSR0			12
#define KS2_LPSC_CHIP_SRSS		13
#define KS2_LPSC_MSMC			14
#define KS2_LPSC_GEM_1			16
#define KS2_LPSC_GEM_2			17
#define KS2_LPSC_GEM_3			18
#define KS2_LPSC_GEM_4			19
#define KS2_LPSC_GEM_5			20
#define KS2_LPSC_GEM_6			21
#define KS2_LPSC_GEM_7			22
#define KS2_LPSC_EMIF4F_DDR3A		23
#define KS2_LPSC_EMIF4F_DDR3B		24
#define KS2_LPSC_TAC			25
#define KS2_LPSC_RAC			26
#define KS2_LPSC_RAC_1			27
#define KS2_LPSC_FFTC_A			28
#define KS2_LPSC_FFTC_B			29
#define KS2_LPSC_FFTC_C			30
#define KS2_LPSC_FFTC_D			31
#define KS2_LPSC_FFTC_E			32
#define KS2_LPSC_FFTC_F			33
#define KS2_LPSC_AI2			34
#define KS2_LPSC_TCP3D_0		35
#define KS2_LPSC_TCP3D_1		36
#define KS2_LPSC_TCP3D_2		37
#define KS2_LPSC_TCP3D_3		38
#define KS2_LPSC_VCP2X4_A		39
#define KS2_LPSC_CP2X4_B		40
#define KS2_LPSC_VCP2X4_C		41
#define KS2_LPSC_VCP2X4_D		42
#define KS2_LPSC_VCP2X4_E		43
#define KS2_LPSC_VCP2X4_F		44
#define KS2_LPSC_VCP2X4_G		45
#define KS2_LPSC_VCP2X4_H		46
#define KS2_LPSC_BCP			47
#define KS2_LPSC_DXB			48
#define KS2_LPSC_VUSR1			49
#define KS2_LPSC_XGE			50
#define KS2_LPSC_ARM_SREFLEX		51

/* DDR3B definitions */
#define KS2_DDR3B_EMIF_CTRL_BASE	0x21020000
#define KS2_DDR3B_EMIF_DATA_BASE	0x60000000
#define KS2_DDR3B_DDRPHYC		0x02328000

#define KS2_CIC2_DDR3_ECC_IRQ_NUM	0x0D3 /* DDR3 ECC system irq number */
#define KS2_CIC2_DDR3_ECC_CHAN_NUM	0x01D /* DDR3 ECC int mapped to CIC2
						 channel 29 */

/* SGMII SerDes */
#define KS2_LANES_PER_SGMII_SERDES	4

/* Number of DSP cores */
#define KS2_NUM_DSPS			8

/* NETCP pktdma */
#define KS2_NETCP_PDMA_CTRL_BASE	0x02004000
#define KS2_NETCP_PDMA_TX_BASE		0x02004400
#define KS2_NETCP_PDMA_TX_CH_NUM	9
#define KS2_NETCP_PDMA_RX_BASE		0x02004800
#define KS2_NETCP_PDMA_RX_CH_NUM	26
#define KS2_NETCP_PDMA_SCHED_BASE	0x02004c00
#define KS2_NETCP_PDMA_RX_FLOW_BASE	0x02005000
#define KS2_NETCP_PDMA_RX_FLOW_NUM	32
#define KS2_NETCP_PDMA_TX_SND_QUEUE	648

/* NETCP */
#define KS2_NETCP_BASE			0x02000000

#endif /* __ASM_ARCH_HARDWARE_H */
