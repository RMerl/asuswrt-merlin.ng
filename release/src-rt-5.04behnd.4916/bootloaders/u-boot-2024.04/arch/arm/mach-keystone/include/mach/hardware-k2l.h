/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * K2L: SoC definitions
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef __ASM_ARCH_HARDWARE_K2L_H
#define __ASM_ARCH_HARDWARE_K2L_H

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

#define KS2_ARM_PLL_EN			BIT(13)

/* PA SS Registers */
#define KS2_PASS_BASE			0x26000000

/* Power and Sleep Controller (PSC) Domains */
#define KS2_LPSC_MOD			0
#define KS2_LPSC_DFE_IQN_SYS		1
#define KS2_LPSC_USB			2
#define KS2_LPSC_EMIF25_SPI		3
#define KS2_LPSC_TSIP                   4
#define KS2_LPSC_DEBUGSS_TRC		5
#define KS2_LPSC_TETB_TRC		6
#define KS2_LPSC_PKTPROC		7
#define KS2_LPSC_PA			KS2_LPSC_PKTPROC
#define KS2_LPSC_SGMII			8
#define KS2_LPSC_CPGMAC			KS2_LPSC_SGMII
#define KS2_LPSC_CRYPTO			9
#define KS2_LPSC_PCIE0			10
#define KS2_LPSC_PCIE1			11
#define KS2_LPSC_JESD_MISC		12
#define KS2_LPSC_CHIP_SRSS		13
#define KS2_LPSC_MSMC			14
#define KS2_LPSC_GEM_1			16
#define KS2_LPSC_GEM_2			17
#define KS2_LPSC_GEM_3			18
#define KS2_LPSC_EMIF4F_DDR3		23
#define KS2_LPSC_TAC			25
#define KS2_LPSC_RAC			26
#define KS2_LPSC_DDUC4X_CFR2X_BB	27
#define KS2_LPSC_FFTC_A			28
#define KS2_LPSC_OSR			34
#define KS2_LPSC_TCP3D_0		35
#define KS2_LPSC_TCP3D_1		37
#define KS2_LPSC_VCP2X4_A		39
#define KS2_LPSC_VCP2X4_B		40
#define KS2_LPSC_VCP2X4_C		41
#define KS2_LPSC_VCP2X4_D		42
#define KS2_LPSC_BCP			47
#define KS2_LPSC_DPD4X			48
#define KS2_LPSC_FFTC_B			49
#define KS2_LPSC_IQN_AIL		50

/* Chip Interrupt Controller */
#define KS2_CIC2_DDR3_ECC_IRQ_NUM	0x0D3
#define KS2_CIC2_DDR3_ECC_CHAN_NUM	0x01D

/* OSR */
#define KS2_OSR_DATA_BASE		0x70000000	/* OSR data base */
#define KS2_OSR_CFG_BASE		0x02348c00	/* OSR config base */
#define KS2_OSR_ECC_VEC			0x08		/* ECC Vector reg */
#define KS2_OSR_ECC_CTRL		0x14		/* ECC control reg */

/* OSR ECC Vector register */
#define KS2_OSR_ECC_VEC_TRIG_RD		BIT(15)		/* trigger a read op */
#define KS2_OSR_ECC_VEC_RD_DONE		BIT(24)		/* read complete */

#define KS2_OSR_ECC_VEC_RAM_ID_SH	0		/* RAM ID shift */
#define KS2_OSR_ECC_VEC_RD_ADDR_SH	16		/* read address shift */

/* OSR ECC control register */
#define KS2_OSR_ECC_CTRL_EN		BIT(0)		/* ECC enable bit */
#define KS2_OSR_ECC_CTRL_CHK		BIT(1)		/* ECC check bit */
#define KS2_OSR_ECC_CTRL_RMW		BIT(2)		/* ECC check bit */

/* Number of OSR RAM banks */
#define KS2_OSR_NUM_RAM_BANKS		4

/* OSR memory size */
#define KS2_OSR_SIZE			0x100000

/* SGMII SerDes */
#define KS2_SGMII_SERDES2_BASE		0x02320000
#define KS2_LANES_PER_SGMII_SERDES	2

/* Number of DSP cores */
#define KS2_NUM_DSPS			4

/* NETCP pktdma */
#define KS2_NETCP_PDMA_CTRL_BASE	0x26186000
#define KS2_NETCP_PDMA_TX_BASE		0x26187000
#define KS2_NETCP_PDMA_TX_CH_NUM	21
#define KS2_NETCP_PDMA_RX_BASE		0x26188000
#define KS2_NETCP_PDMA_RX_CH_NUM	91
#define KS2_NETCP_PDMA_SCHED_BASE	0x26186100
#define KS2_NETCP_PDMA_RX_FLOW_BASE	0x26189000
#define KS2_NETCP_PDMA_RX_FLOW_NUM	96
#define KS2_NETCP_PDMA_TX_SND_QUEUE	896

/* NETCP */
#define KS2_NETCP_BASE			0x26000000

#ifndef __ASSEMBLY__
static inline int ddr3_get_size(void)
{
	return 2;
}
#endif

#endif /* __ASM_ARCH_HARDWARE_K2L_H */
