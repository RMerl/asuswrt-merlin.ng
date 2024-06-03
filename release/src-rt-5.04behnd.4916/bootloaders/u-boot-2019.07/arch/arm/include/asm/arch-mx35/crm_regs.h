/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2004-2009 Freescale Semiconductor, Inc.
 */

#ifndef __CPU_ARM1136_MX35_CRM_REGS_H__
#define __CPU_ARM1136_MX35_CRM_REGS_H__

/* Register bit definitions */
#define MXC_CCM_CCMR_WFI                        (1 << 30)
#define MXC_CCM_CCMR_STBY_EXIT_SRC              (1 << 29)
#define MXC_CCM_CCMR_VSTBY                      (1 << 28)
#define MXC_CCM_CCMR_WBEN                       (1 << 27)
#define MXC_CCM_CCMR_VOL_RDY_CNT_OFFSET        20
#define MXC_CCM_CCMR_VOL_RDY_CNT_MASK          (0xF << 20)
#define MXC_CCM_CCMR_ROMW_OFFSET               18
#define MXC_CCM_CCMR_ROMW_MASK                 (0x3 << 18)
#define MXC_CCM_CCMR_RAMW_OFFSET               16
#define MXC_CCM_CCMR_RAMW_MASK                 (0x3 << 16)
#define MXC_CCM_CCMR_LPM_OFFSET                 14
#define MXC_CCM_CCMR_LPM_MASK                   (0x3 << 14)
#define MXC_CCM_CCMR_UPE                        (1 << 9)
#define MXC_CCM_CCMR_MPE                        (1 << 3)

#define MXC_CCM_PDR0_PER_SEL			(1 << 26)
#define MXC_CCM_PDR0_IPU_HND_BYP                (1 << 23)
#define MXC_CCM_PDR0_HSP_PODF_OFFSET            20
#define MXC_CCM_PDR0_HSP_PODF_MASK              (0x3 << 20)
#define MXC_CCM_PDR0_CON_MUX_DIV_OFFSET		16
#define MXC_CCM_PDR0_CON_MUX_DIV_MASK           (0xF << 16)
#define MXC_CCM_PDR0_CKIL_SEL			(1 << 15)
#define MXC_CCM_PDR0_PER_PODF_OFFSET            12
#define MXC_CCM_PDR0_PER_PODF_MASK              (0x7 << 12)
#define MXC_CCM_PDR0_AUTO_MUX_DIV_OFFSET        9
#define MXC_CCM_PDR0_AUTO_MUX_DIV_MASK          (0x7 << 9)
#define MXC_CCM_PDR0_AUTO_CON	                0x1

#define MXC_CCM_PDR1_MSHC_PRDF_OFFSET           28
#define MXC_CCM_PDR1_MSHC_PRDF_MASK             (0x7 << 28)
#define MXC_CCM_PDR1_MSHC_PODF_OFFSET           22
#define MXC_CCM_PDR1_MSHC_PODF_MASK             (0x3F << 22)
#define MXC_CCM_PDR1_MSHC_M_U			(1 << 7)

#define MXC_CCM_PDR2_SSI2_PRDF_OFFSET           27
#define MXC_CCM_PDR2_SSI2_PRDF_MASK             (0x7 << 27)
#define MXC_CCM_PDR2_SSI1_PRDF_OFFSET           24
#define MXC_CCM_PDR2_SSI1_PRDF_MASK             (0x7 << 24)
#define MXC_CCM_PDR2_CSI_PODF_OFFSET            16
#define MXC_CCM_PDR2_CSI_PODF_MASK              (0x3F << 16)
#define MXC_CCM_PDR2_SSI2_PODF_OFFSET           8
#define MXC_CCM_PDR2_SSI2_PODF_MASK             (0x3F << 8)
#define MXC_CCM_PDR2_CSI_M_U			(1 << 7)
#define MXC_CCM_PDR2_SSI_M_U			(1 << 6)
#define MXC_CCM_PDR2_SSI1_PODF_OFFSET           0
#define MXC_CCM_PDR2_SSI1_PODF_MASK             (0x3F)

#define MXC_CCM_PDR3_SPDIF_PRDF_OFFSET          29
#define MXC_CCM_PDR3_SPDIF_PRDF_MASK            (0x7 << 29)
#define MXC_CCM_PDR3_SPDIF_PODF_OFFSET          23
#define MXC_CCM_PDR3_SPDIF_PODF_MASK            (0x3F << 23)
#define MXC_CCM_PDR3_SPDIF_M_U			(1 << 22)
#define MXC_CCM_PDR3_ESDHC3_PODF_OFFSET         16
#define MXC_CCM_PDR3_ESDHC3_PODF_MASK           (0x3F << 16)
#define MXC_CCM_PDR3_UART_M_U			(1 << 14)
#define MXC_CCM_PDR3_ESDHC2_PODF_OFFSET         8
#define MXC_CCM_PDR3_ESDHC2_PODF_MASK           (0x3F << 8)
#define MXC_CCM_PDR3_ESDHC_M_U			(1 << 6)
#define MXC_CCM_PDR3_ESDHC1_PODF_OFFSET         0
#define MXC_CCM_PDR3_ESDHC1_PODF_MASK           (0x3F)

#define MXC_CCM_PDR4_NFC_PODF_OFFSET		28
#define MXC_CCM_PDR4_NFC_PODF_MASK		(0xF << 28)
#define MXC_CCM_PDR4_USB_PODF_OFFSET		22
#define MXC_CCM_PDR4_USB_PODF_MASK		(0x3F << 22)
#define MXC_CCM_PDR4_PER0_PODF_OFFSET		16
#define MXC_CCM_PDR4_PER0_PODF_MASK		(0x3F << 16)
#define MXC_CCM_PDR4_UART_PODF_OFFSET		10
#define MXC_CCM_PDR4_UART_PODF_MASK		(0x3F << 10)
#define MXC_CCM_PDR4_USB_M_U			(1 << 9)

/* Bit definitions for RCSR */
#define MXC_CCM_RCSR_BUS_WIDTH			(1 << 29)
#define MXC_CCM_RCSR_BUS_16BIT			(1 << 29)
#define MXC_CCM_RCSR_PAGE_SIZE			(3 << 27)
#define MXC_CCM_RCSR_PAGE_512			(0 << 27)
#define MXC_CCM_RCSR_PAGE_2K			(1 << 27)
#define MXC_CCM_RCSR_PAGE_4K1			(2 << 27)
#define MXC_CCM_RCSR_PAGE_4K2			(3 << 27)
#define MXC_CCM_RCSR_SOFT_RESET			(1 << 15)
#define MXC_CCM_RCSR_NF16B			(1 << 14)
#define MXC_CCM_RCSR_NFC_4K			(1 << 9)
#define MXC_CCM_RCSR_NFC_FMS			(1 << 8)

/* Bit definitions for both MCU, PERIPHERAL PLL control registers */
#define MXC_CCM_PCTL_BRM                        0x80000000
#define MXC_CCM_PCTL_PD_OFFSET                  26
#define MXC_CCM_PCTL_PD_MASK                    (0xF << 26)
#define MXC_CCM_PCTL_MFD_OFFSET                 16
#define MXC_CCM_PCTL_MFD_MASK                   (0x3FF << 16)
#define MXC_CCM_PCTL_MFI_OFFSET                 10
#define MXC_CCM_PCTL_MFI_MASK                   (0xF << 10)
#define MXC_CCM_PCTL_MFN_OFFSET                 0
#define MXC_CCM_PCTL_MFN_MASK                   0x3FF

/* Bit definitions for Audio clock mux register*/
#define MXC_CCM_ACMR_ESAI_CLK_SEL_OFFSET	12
#define MXC_CCM_ACMR_ESAI_CLK_SEL_MASK		(0xF << 12)
#define MXC_CCM_ACMR_SPDIF_CLK_SEL_OFFSET	8
#define MXC_CCM_ACMR_SPDIF_CLK_SEL_MASK		(0xF << 8)
#define MXC_CCM_ACMR_SSI1_CLK_SEL_OFFSET	4
#define MXC_CCM_ACMR_SSI1_CLK_SEL_MASK		(0xF << 4)
#define MXC_CCM_ACMR_SSI2_CLK_SEL_OFFSET	0
#define MXC_CCM_ACMR_SSI2_CLK_SEL_MASK		(0xF << 0)

/* Bit definitions for Clock gating Register*/
#define MXC_CCM_CGR_CG_MASK			0x3
#define MXC_CCM_CGR_CG_OFF			0x0
#define MXC_CCM_CGR_CG_RUN_ON			0x1
#define MXC_CCM_CGR_CG_RUN_WAIT_ON		0x2
#define MXC_CCM_CGR_CG_ON			0x3

#define MXC_CCM_CGR0_ASRC_OFFSET		0
#define MXC_CCM_CGR0_ASRC_MASK			(0x3 << 0)
#define MXC_CCM_CGR0_ATA_OFFSET			2
#define MXC_CCM_CGR0_ATA_MASK			(0x3 << 2)
#define MXC_CCM_CGR0_CAN1_OFFSET		6
#define MXC_CCM_CGR0_CAN1_MASK			(0x3 << 6)
#define MXC_CCM_CGR0_CAN2_OFFSET		8
#define MXC_CCM_CGR0_CAN2_MASK			(0x3 << 8)
#define MXC_CCM_CGR0_CSPI1_OFFSET		10
#define MXC_CCM_CGR0_CSPI1_MASK			(0x3 << 10)
#define MXC_CCM_CGR0_CSPI2_OFFSET		12
#define MXC_CCM_CGR0_CSPI2_MASK			(0x3 << 12)
#define MXC_CCM_CGR0_ECT_OFFSET			14
#define MXC_CCM_CGR0_ECT_MASK			(0x3 << 14)
#define MXC_CCM_CGR0_EDIO_OFFSET		16
#define MXC_CCM_CGR0_EDIO_MASK			(0x3 << 16)
#define MXC_CCM_CGR0_EMI_OFFSET			18
#define MXC_CCM_CGR0_EMI_MASK			(0x3 << 18)
#define MXC_CCM_CGR0_EPIT1_OFFSET		20
#define MXC_CCM_CGR0_EPIT1_MASK			(0x3 << 20)
#define MXC_CCM_CGR0_EPIT2_OFFSET		22
#define MXC_CCM_CGR0_EPIT2_MASK			(0x3 << 22)
#define MXC_CCM_CGR0_ESAI_OFFSET		24
#define MXC_CCM_CGR0_ESAI_MASK			(0x3 << 24)
#define MXC_CCM_CGR0_ESDHC1_OFFSET		26
#define MXC_CCM_CGR0_ESDHC1_MASK		(0x3 << 26)
#define MXC_CCM_CGR0_ESDHC2_OFFSET		28
#define MXC_CCM_CGR0_ESDHC2_MASK		(0x3 << 28)
#define MXC_CCM_CGR0_ESDHC3_OFFSET		30
#define MXC_CCM_CGR0_ESDHC3_MASK		(0x3 << 30)

#define MXC_CCM_CGR1_FEC_OFFSET			0
#define MXC_CCM_CGR1_FEC_MASK			(0x3 << 0)
#define MXC_CCM_CGR1_GPIO1_OFFSET		2
#define MXC_CCM_CGR1_GPIO1_MASK			(0x3 << 2)
#define MXC_CCM_CGR1_GPIO2_OFFSET		4
#define MXC_CCM_CGR1_GPIO2_MASK			(0x3 << 4)
#define MXC_CCM_CGR1_GPIO3_OFFSET		6
#define MXC_CCM_CGR1_GPIO3_MASK			(0x3 << 6)
#define MXC_CCM_CGR1_GPT_OFFSET			8
#define MXC_CCM_CGR1_GPT_MASK			(0x3 << 8)
#define MXC_CCM_CGR1_I2C1_OFFSET		10
#define MXC_CCM_CGR1_I2C1_MASK			(0x3 << 10)
#define MXC_CCM_CGR1_I2C2_OFFSET		12
#define MXC_CCM_CGR1_I2C2_MASK			(0x3 << 12)
#define MXC_CCM_CGR1_I2C3_OFFSET		14
#define MXC_CCM_CGR1_I2C3_MASK			(0x3 << 14)
#define MXC_CCM_CGR1_IOMUXC_OFFSET		16
#define MXC_CCM_CGR1_IOMUXC_MASK		(0x3 << 16)
#define MXC_CCM_CGR1_IPU_OFFSET			18
#define MXC_CCM_CGR1_IPU_MASK			(0x3 << 18)
#define MXC_CCM_CGR1_KPP_OFFSET			20
#define MXC_CCM_CGR1_KPP_MASK			(0x3 << 20)
#define MXC_CCM_CGR1_MLB_OFFSET			22
#define MXC_CCM_CGR1_MLB_MASK			(0x3 << 22)
#define MXC_CCM_CGR1_MSHC_OFFSET		24
#define MXC_CCM_CGR1_MSHC_MASK			(0x3 << 24)
#define MXC_CCM_CGR1_OWIRE_OFFSET		26
#define MXC_CCM_CGR1_OWIRE_MASK			(0x3 << 26)
#define MXC_CCM_CGR1_PWM_OFFSET			28
#define MXC_CCM_CGR1_PWM_MASK			(0x3 << 28)
#define MXC_CCM_CGR1_RNGC_OFFSET		30
#define MXC_CCM_CGR1_RNGC_MASK			(0x3 << 30)

#define MXC_CCM_CGR2_RTC_OFFSET			0
#define MXC_CCM_CGR2_RTC_MASK			(0x3 << 0)
#define MXC_CCM_CGR2_RTIC_OFFSET		2
#define MXC_CCM_CGR2_RTIC_MASK			(0x3 << 2)
#define MXC_CCM_CGR2_SCC_OFFSET			4
#define MXC_CCM_CGR2_SCC_MASK			(0x3 << 4)
#define MXC_CCM_CGR2_SDMA_OFFSET		6
#define MXC_CCM_CGR2_SDMA_MASK			(0x3 << 6)
#define MXC_CCM_CGR2_SPBA_OFFSET		8
#define MXC_CCM_CGR2_SPBA_MASK			(0x3 << 8)
#define MXC_CCM_CGR2_SPDIF_OFFSET		10
#define MXC_CCM_CGR2_SPDIF_MASK			(0x3 << 10)
#define MXC_CCM_CGR2_SSI1_OFFSET		12
#define MXC_CCM_CGR2_SSI1_MASK			(0x3 << 12)
#define MXC_CCM_CGR2_SSI2_OFFSET		14
#define MXC_CCM_CGR2_SSI2_MASK			(0x3 << 14)
#define MXC_CCM_CGR2_UART1_OFFSET		16
#define MXC_CCM_CGR2_UART1_MASK			(0x3 << 16)
#define MXC_CCM_CGR2_UART2_OFFSET		18
#define MXC_CCM_CGR2_UART2_MASK			(0x3 << 18)
#define MXC_CCM_CGR2_UART3_OFFSET		20
#define MXC_CCM_CGR2_UART3_MASK			(0x3 << 20)
#define MXC_CCM_CGR2_USBOTG_OFFSET		22
#define MXC_CCM_CGR2_USBOTG_MASK		(0x3 << 22)
#define MXC_CCM_CGR2_WDOG_OFFSET		24
#define MXC_CCM_CGR2_WDOG_MASK			(0x3 << 24)
#define MXC_CCM_CGR2_MAX_OFFSET			26
#define MXC_CCM_CGR2_MAX_MASK			(0x3 << 26)
#define MXC_CCM_CGR2_MAX_ENABLE			(0x2 << 26)
#define MXC_CCM_CGR2_AUDMUX_OFFSET		30
#define MXC_CCM_CGR2_AUDMUX_MASK		(0x3 << 30)

#define MXC_CCM_CGR3_CSI_OFFSET			0
#define MXC_CCM_CGR3_CSI_MASK			(0x3 << 0)
#define MXC_CCM_CGR3_IIM_OFFSET			2
#define MXC_CCM_CGR3_IIM_MASK			(0x3 << 2)
#define MXC_CCM_CGR3_GPU2D_OFFSET		4
#define MXC_CCM_CGR3_GPU2D_MASK			(0x3 << 4)

#define MXC_CCM_COSR_CLKOSEL_MASK		0x1F
#define MXC_CCM_COSR_CLKOSEL_OFFSET		0
#define MXC_CCM_COSR_CLKOEN			(1 << 5)
#define MXC_CCM_COSR_CLKOUTDIV_1		(1 << 6)
#define MXC_CCM_COSR_CLKOUT_DIV_MASK		(0x3F << 10)
#define MXC_CCM_COSR_CLKOUT_DIV_OFFSET		10
#define MXC_CCM_COSR_SSI1_RX_SRC_SEL_MASK	(0x3 << 16)
#define MXC_CCM_COSR_SSI1_RX_SRC_SEL_OFFSET	16
#define MXC_CCM_COSR_SSI1_TX_SRC_SEL_MASK	(0x3 << 18)
#define MXC_CCM_COSR_SSI1_TX_SRC_SEL_OFFSET	18
#define MXC_CCM_COSR_SSI2_RX_SRC_SEL_MASK	(0x3 << 20)
#define MXC_CCM_COSR_SSI2_RX_SRC_SEL_OFFSET	20
#define MXC_CCM_COSR_SSI2_TX_SRC_SEL_MASK	(0x3 << 22)
#define MXC_CCM_COSR_SSI2_TX_SRC_SEL_OFFSET	22
#define MXC_CCM_COSR_ASRC_AUDIO_EN		(1 << 24)
#define MXC_CCM_COSR_ASRC_AUDIO_PODF_MASK	(0x3F << 26)
#define MXC_CCM_COSR_ASRC_AUDIO_PODF_OFFSET	26

#endif
