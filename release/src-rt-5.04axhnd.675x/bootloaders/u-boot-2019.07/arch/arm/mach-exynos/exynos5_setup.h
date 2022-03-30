/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Machine Specific Values for SMDK5250 board based on EXYNOS5
 *
 * Copyright (C) 2012 Samsung Electronics
 */

#ifndef _SMDK5250_SETUP_H
#define _SMDK5250_SETUP_H

#include <config.h>
#include <asm/arch/dmc.h>

#define NOT_AVAILABLE		0
#define DATA_MASK		0xFFFFF

#define ENABLE_BIT		0x1
#define DISABLE_BIT		0x0
#define CA_SWAP_EN		(1 << 0)

/* Set PLL */
#define set_pll(mdiv, pdiv, sdiv)	(1<<31 | mdiv<<16 | pdiv<<8 | sdiv)

/* MEMCONTROL register bit fields */
#define DMC_MEMCONTROL_CLK_STOP_DISABLE	(0 << 0)
#define DMC_MEMCONTROL_DPWRDN_DISABLE	(0 << 1)
#define DMC_MEMCONTROL_DPWRDN_ACTIVE_PRECHARGE	(0 << 2)
#define DMC_MEMCONTROL_TP_DISABLE	(0 << 4)
#define DMC_MEMCONTROL_DSREF_DISABLE	(0 << 5)
#define DMC_MEMCONTROL_DSREF_ENABLE	(1 << 5)
#define DMC_MEMCONTROL_ADD_LAT_PALL_CYCLE(x)    (x << 6)

#define DMC_MEMCONTROL_MEM_TYPE_LPDDR3  (7 << 8)
#define DMC_MEMCONTROL_MEM_TYPE_DDR3    (6 << 8)
#define DMC_MEMCONTROL_MEM_TYPE_LPDDR2  (5 << 8)

#define DMC_MEMCONTROL_MEM_WIDTH_32BIT  (2 << 12)

#define DMC_MEMCONTROL_NUM_CHIP_1       (0 << 16)
#define DMC_MEMCONTROL_NUM_CHIP_2       (1 << 16)

#define DMC_MEMCONTROL_BL_8             (3 << 20)
#define DMC_MEMCONTROL_BL_4             (2 << 20)

#define DMC_MEMCONTROL_PZQ_DISABLE      (0 << 24)

#define DMC_MEMCONTROL_MRR_BYTE_7_0     (0 << 25)
#define DMC_MEMCONTROL_MRR_BYTE_15_8    (1 << 25)
#define DMC_MEMCONTROL_MRR_BYTE_23_16   (2 << 25)
#define DMC_MEMCONTROL_MRR_BYTE_31_24   (3 << 25)

/* MEMCONFIG0 register bit fields */
#define DMC_MEMCONFIGX_CHIP_MAP_INTERLEAVED     (1 << 12)
#define DMC_MEMCONFIG_CHIP_MAP_SPLIT		(2 << 12)
#define DMC_MEMCONFIGX_CHIP_COL_10              (3 << 8)
#define DMC_MEMCONFIGX_CHIP_ROW_14              (2 << 4)
#define DMC_MEMCONFIGX_CHIP_ROW_15              (3 << 4)
#define DMC_MEMCONFIGX_CHIP_BANK_8              (3 << 0)

#define DMC_MEMBASECONFIGX_CHIP_BASE(x)         (x << 16)
#define DMC_MEMBASECONFIGX_CHIP_MASK(x)         (x << 0)
#define DMC_MEMBASECONFIG_VAL(x)        (       \
	DMC_MEMBASECONFIGX_CHIP_BASE(x) |       \
	DMC_MEMBASECONFIGX_CHIP_MASK(0x780)     \
)

/*
 * As we use channel interleaving, therefore value of the base address
 * register must be set as half of the bus base address
 * RAM start addess is 0x2000_0000 which means chip_base is 0x20, so
 * we need to set half 0x10 to the membaseconfigx registers
 * see exynos5420 UM section 17.17.3.21 for more.
 */
#define DMC_CHIP_BASE_0 0x10
#define DMC_CHIP_BASE_1 0x50
#define DMC_CHIP_MASK	0x7C0

#define DMC_MEMBASECONFIG0_VAL  DMC_MEMBASECONFIG_VAL(0x40)
#define DMC_MEMBASECONFIG1_VAL  DMC_MEMBASECONFIG_VAL(0x80)

#define DMC_PRECHCONFIG_VAL             0xFF000000
#define DMC_PWRDNCONFIG_VAL             0xFFFF00FF

#define DMC_CONCONTROL_RESET_VAL	0x0FFF0000
#define DFI_INIT_START		(1 << 28)
#define EMPTY			(1 << 8)
#define AREF_EN			(1 << 5)

#define DFI_INIT_COMPLETE_CHO	(1 << 2)
#define DFI_INIT_COMPLETE_CH1	(1 << 3)

#define RDLVL_COMPLETE_CHO	(1 << 14)
#define RDLVL_COMPLETE_CH1	(1 << 15)

#define CLK_STOP_EN	(1 << 0)
#define DPWRDN_EN	(1 << 1)
#define DSREF_EN	(1 << 5)

/* COJCONTROL register bit fields */
#define DMC_CONCONTROL_IO_PD_CON_DISABLE	(0 << 3)
#define DMC_CONCONTROL_IO_PD_CON_ENABLE		(1 << 3)
#define DMC_CONCONTROL_AREF_EN_DISABLE		(0 << 5)
#define DMC_CONCONTROL_AREF_EN_ENABLE		(1 << 5)
#define DMC_CONCONTROL_EMPTY_DISABLE		(0 << 8)
#define DMC_CONCONTROL_EMPTY_ENABLE		(1 << 8)
#define DMC_CONCONTROL_RD_FETCH_DISABLE		(0x0 << 12)
#define DMC_CONCONTROL_TIMEOUT_LEVEL0		(0xFFF << 16)
#define DMC_CONCONTROL_DFI_INIT_START_DISABLE	(0 << 28)

#define DMC_CONCONTROL_VAL	0x1FFF2101

#define DREX_CONCONTROL_VAL	DMC_CONCONTROL_VAL			\
				| DMC_CONCONTROL_AREF_EN_ENABLE		\
				| DMC_CONCONTROL_IO_PD_CON_ENABLE

#define DMC_CONCONTROL_IO_PD_CON(x)		(x << 6)

/* CLK_DIV_CPU1 */
#define HPM_RATIO               0x2
#define COPY_RATIO              0x0

/* CLK_DIV_CPU1 = 0x00000003 */
#define CLK_DIV_CPU1_VAL        ((HPM_RATIO << 4)		\
				| (COPY_RATIO))

/* CLK_SRC_CORE0 */
#define CLK_SRC_CORE0_VAL       0x00000000

/* CLK_SRC_CORE1 */
#define CLK_SRC_CORE1_VAL       0x100

/* CLK_DIV_CORE0 */
#define CLK_DIV_CORE0_VAL       0x00120000

/* CLK_DIV_CORE1 */
#define CLK_DIV_CORE1_VAL       0x07070700

/* CLK_DIV_SYSRGT */
#define CLK_DIV_SYSRGT_VAL      0x00000111

/* CLK_DIV_ACP */
#define CLK_DIV_ACP_VAL         0x12

/* CLK_DIV_SYSLFT */
#define CLK_DIV_SYSLFT_VAL      0x00000311

#define MUX_APLL_SEL_MASK	(1 << 0)
#define MUX_MPLL_SEL_MASK	(1 << 8)
#define MPLL_SEL_MOUT_MPLLFOUT	(2 << 8)
#define MUX_CPLL_SEL_MASK	(1 << 8)
#define MUX_EPLL_SEL_MASK	(1 << 12)
#define MUX_VPLL_SEL_MASK	(1 << 16)
#define MUX_GPLL_SEL_MASK	(1 << 28)
#define MUX_BPLL_SEL_MASK	(1 << 0)
#define MUX_HPM_SEL_MASK	(1 << 20)
#define HPM_SEL_SCLK_MPLL	(1 << 21)
#define PLL_LOCKED		(1 << 29)
#define APLL_CON0_LOCKED	(1 << 29)
#define MPLL_CON0_LOCKED	(1 << 29)
#define BPLL_CON0_LOCKED	(1 << 29)
#define CPLL_CON0_LOCKED	(1 << 29)
#define EPLL_CON0_LOCKED	(1 << 29)
#define GPLL_CON0_LOCKED	(1 << 29)
#define VPLL_CON0_LOCKED	(1 << 29)
#define CLK_REG_DISABLE		0x0
#define TOP2_VAL		0x0110000

/* SCLK_SRC_ISP - set SPI0/1 to 6 = SCLK_MPLL_USER */
#define SPI0_ISP_SEL		6
#define SPI1_ISP_SEL		6
#define SCLK_SRC_ISP_VAL	(SPI1_ISP_SEL << 4) \
				| (SPI0_ISP_SEL << 0)

/* SCLK_DIV_ISP - set SPI0/1 to 0xf = divide by 16 */
#define SPI0_ISP_RATIO		0xf
#define SPI1_ISP_RATIO		0xf
#define SCLK_DIV_ISP_VAL	(SPI1_ISP_RATIO << 12) \
				| (SPI0_ISP_RATIO << 0)

/* CLK_DIV_FSYS2 */
#define MMC2_RATIO_MASK		0xf
#define MMC2_RATIO_VAL		0x3
#define MMC2_RATIO_OFFSET	0

#define MMC2_PRE_RATIO_MASK	0xff
#define MMC2_PRE_RATIO_VAL	0x9
#define MMC2_PRE_RATIO_OFFSET	8

#define MMC3_RATIO_MASK		0xf
#define MMC3_RATIO_VAL		0x1
#define MMC3_RATIO_OFFSET	16

#define MMC3_PRE_RATIO_MASK	0xff
#define MMC3_PRE_RATIO_VAL	0x0
#define MMC3_PRE_RATIO_OFFSET	24

/* CLK_SRC_LEX */
#define CLK_SRC_LEX_VAL         0x0

/* CLK_DIV_LEX */
#define CLK_DIV_LEX_VAL         0x10

/* CLK_DIV_R0X */
#define CLK_DIV_R0X_VAL         0x10

/* CLK_DIV_L0X */
#define CLK_DIV_R1X_VAL         0x10

/* CLK_DIV_ISP2 */
#define CLK_DIV_ISP2_VAL        0x1

/* CLK_SRC_KFC */
#define SRC_KFC_HPM_SEL		(1 << 15)

/* CLK_SRC_KFC */
#define CLK_SRC_KFC_VAL		0x00008001

/* CLK_DIV_KFC */
#define CLK_DIV_KFC_VAL		0x03300110

/* CLK_DIV2_RATIO */
#define CLK_DIV2_RATIO		0x10111150

/* CLK_DIV4_RATIO */
#define CLK_DIV4_RATIO		0x00000003

/* CLK_DIV_G2D */
#define CLK_DIV_G2D		0x00000010

/*
 * DIV_DISP1_0
 * For DP, divisor should be 2
 */
#define CLK_DIV_DISP1_0_FIMD1	(2 << 0)

/* CLK_GATE_IP_DISP1 */
#define CLK_GATE_DP1_ALLOW	(1 << 4)

/* AUDIO CLK SEL */
#define AUDIO0_SEL_EPLL		(0x6 << 28)
#define AUDIO0_RATIO		0x5
#define PCM0_RATIO		0x3
#define DIV_MAU_VAL		(PCM0_RATIO << 24 | AUDIO0_RATIO << 20)

/* CLK_SRC_CDREX */
#define MUX_MCLK_CDR_MSPLL	(1 << 4)
#define MUX_BPLL_SEL_FOUTBPLL   (1 << 0)
#define BPLL_SEL_MASK   0x7
#define FOUTBPLL        2

#define DDR3PHY_CTRL_PHY_RESET	(1 << 0)
#define DDR3PHY_CTRL_PHY_RESET_OFF	(0 << 0)

#define PHY_CON0_RESET_VAL	0x17020a40
#define P0_CMD_EN		(1 << 14)
#define BYTE_RDLVL_EN		(1 << 13)
#define CTRL_SHGATE		(1 << 8)

#define PHY_CON1_RESET_VAL	0x09210100
#define RDLVL_PASS_ADJ_VAL	0x6
#define RDLVL_PASS_ADJ_OFFSET	16
#define CTRL_GATEDURADJ_MASK	(0xf << 20)
#define READ_LEVELLING_DDR3	0x0100

#define PHY_CON2_RESET_VAL	0x00010004
#define INIT_DESKEW_EN		(1 << 6)
#define DLL_DESKEW_EN		(1 << 12)
#define RDLVL_GATE_EN		(1 << 24)
#define RDLVL_EN		(1 << 25)
#define RDLVL_INCR_ADJ		(0x1 << 16)

/* DREX_PAUSE */
#define DREX_PAUSE_EN	(1 << 0)

#define BYPASS_EN	(1 << 22)

/* MEMMORY VAL */
#define PHY_CON0_VAL	0x17021A00

#define PHY_CON12_RESET_VAL	0x10100070
#define PHY_CON12_VAL		0x10107F50
#define CTRL_START		(1 << 6)
#define CTRL_DLL_ON		(1 << 5)
#define CTRL_LOCK_COARSE_OFFSET	10
#define CTRL_LOCK_COARSE_MASK	(0x7F << CTRL_LOCK_COARSE_OFFSET)
#define CTRL_LOCK_COARSE(x)	(((x) & CTRL_LOCK_COARSE_MASK) >> \
				 CTRL_LOCK_COARSE_OFFSET)
#define CTRL_FORCE_MASK		(0x7F << 8)
#define CTRL_FINE_LOCKED	0x7

#define CTRL_OFFSETD_RESET_VAL	0x8
#define CTRL_OFFSETD_VAL	0x7F

#define CTRL_OFFSETR0		0x7F
#define CTRL_OFFSETR1		0x7F
#define CTRL_OFFSETR2		0x7F
#define CTRL_OFFSETR3		0x7F
#define PHY_CON4_VAL	(CTRL_OFFSETR0 << 0 | \
				CTRL_OFFSETR1 << 8 | \
				CTRL_OFFSETR2 << 16 | \
				CTRL_OFFSETR3 << 24)
#define PHY_CON4_RESET_VAL	0x08080808

#define CTRL_OFFSETW0		0x7F
#define CTRL_OFFSETW1		0x7F
#define CTRL_OFFSETW2		0x7F
#define CTRL_OFFSETW3		0x7F
#define PHY_CON6_VAL	(CTRL_OFFSETW0 << 0 | \
				CTRL_OFFSETW1 << 8 | \
				CTRL_OFFSETW2 << 16 | \
				CTRL_OFFSETW3 << 24)
#define PHY_CON6_RESET_VAL	0x08080808

#define PHY_CON14_RESET_VAL	0x001F0000
#define CTRL_PULLD_DQS		0xF
#define CTRL_PULLD_DQS_OFFSET	0

/* ZQ Configurations */
#define PHY_CON16_RESET_VAL	0x08000304

#define ZQ_CLK_EN		(1 << 27)
#define ZQ_CLK_DIV_EN		(1 << 18)
#define ZQ_MANUAL_STR		(1 << 1)
#define ZQ_DONE			(1 << 0)
#define ZQ_MODE_DDS_OFFSET	24

#define CTRL_RDLVL_GATE_ENABLE	1
#define CTRL_RDLVL_GATE_DISABLE	0
#define CTRL_RDLVL_DATA_ENABLE	2

/* Direct Command */
#define DIRECT_CMD_NOP			0x07000000
#define DIRECT_CMD_PALL			0x01000000
#define DIRECT_CMD_ZQINIT		0x0a000000
#define DIRECT_CMD_CHANNEL_SHIFT	28
#define DIRECT_CMD_CHIP_SHIFT		20
#define DIRECT_CMD_BANK_SHIFT		16
#define DIRECT_CMD_REFA		(5 << 24)
#define DIRECT_CMD_MRS1		0x71C00
#define DIRECT_CMD_MRS2		0x10BFC
#define DIRECT_CMD_MRS3		0x0050C
#define DIRECT_CMD_MRS4		0x00868
#define DIRECT_CMD_MRS5		0x00C04

/* Drive Strength */
#define IMPEDANCE_48_OHM	4
#define IMPEDANCE_40_OHM	5
#define IMPEDANCE_34_OHM	6
#define IMPEDANCE_30_OHM	7
#define PHY_CON39_VAL_48_OHM	0x09240924
#define PHY_CON39_VAL_40_OHM	0x0B6D0B6D
#define PHY_CON39_VAL_34_OHM	0x0DB60DB6
#define PHY_CON39_VAL_30_OHM	0x0FFF0FFF

#define CTRL_BSTLEN_OFFSET	8
#define CTRL_RDLAT_OFFSET	0

#define CMD_DEFAULT_LPDDR3	0xF
#define CMD_DEFUALT_OFFSET	0
#define T_WRDATA_EN		0x7
#define T_WRDATA_EN_DDR3	0x8
#define T_WRDATA_EN_OFFSET	16
#define T_WRDATA_EN_MASK	0x1f

#define PHY_CON31_VAL	0x0C183060
#define PHY_CON32_VAL	0x60C18306
#define PHY_CON33_VAL	0x00000030

#define PHY_CON31_RESET_VAL	0x0
#define PHY_CON32_RESET_VAL	0x0
#define PHY_CON33_RESET_VAL	0x0

#define SL_DLL_DYN_CON_EN	(1 << 1)
#define FP_RESYNC	(1 << 3)
#define CTRL_START	(1 << 6)

#define DMC_AREF_EN		(1 << 5)
#define DMC_CONCONTROL_EMPTY	(1 << 8)
#define DFI_INIT_START		(1 << 28)

#define DMC_MEMCONTROL_VAL	0x00312700
#define CLK_STOP_EN		(1 << 0)
#define DPWRDN_EN		(1 << 1)
#define DSREF_EN		(1 << 5)

#define MEMBASECONFIG_CHIP_MASK_VAL	0x7E0
#define MEMBASECONFIG_CHIP_MASK_OFFSET	0
#define MEMBASECONFIG0_CHIP_BASE_VAL	0x20
#define MEMBASECONFIG1_CHIP_BASE_VAL	0x40
#define CHIP_BASE_OFFSET		16

#define MEMCONFIG_VAL	0x1323
#define PRECHCONFIG_DEFAULT_VAL	0xFF000000
#define PWRDNCONFIG_DEFAULT_VAL	0xFFFF00FF

#define TIMINGAREF_VAL	0x5d
#define TIMINGROW_VAL	0x345A8692
#define TIMINGDATA_VAL	0x3630065C
#define TIMINGPOWER_VAL	0x50380336
#define DFI_INIT_COMPLETE	(1 << 3)

#define BRBRSVCONTROL_VAL	0x00000033
#define BRBRSVCONFIG_VAL	0x88778877

/* Clock Gating Control (CGCONTROL) register */
#define MEMIF_CG_EN	(1 << 3) /* Memory interface clock gating */
#define SCG_CG_EN	(1 << 2) /* Scheduler clock gating */
#define BUSIF_WR_CG_EN	(1 << 1) /* Bus interface write channel clock gating */
#define BUSIF_RD_CG_EN	(1 << 0) /* Bus interface read channel clock gating */
#define DMC_INTERNAL_CG	(MEMIF_CG_EN | SCG_CG_EN | \
				 BUSIF_WR_CG_EN | BUSIF_RD_CG_EN)

/* DMC PHY Control0 register */
#define PHY_CONTROL0_RESET_VAL	0x0
#define MEM_TERM_EN	(1 << 31)	/* Termination enable for memory */
#define PHY_TERM_EN	(1 << 30)	/* Termination enable for PHY */
#define DMC_CTRL_SHGATE	(1 << 29)	/* Duration of DQS gating signal */
#define FP_RSYNC	(1 << 3)	/* Force DLL resyncronization */

/* Driver strength for CK, CKE, CS & CA */
#define IMP_OUTPUT_DRV_40_OHM	0x5
#define IMP_OUTPUT_DRV_30_OHM	0x7
#define DA_3_DS_OFFSET		25
#define DA_2_DS_OFFSET		22
#define DA_1_DS_OFFSET		19
#define DA_0_DS_OFFSET		16
#define CA_CK_DRVR_DS_OFFSET	9
#define CA_CKE_DRVR_DS_OFFSET	6
#define CA_CS_DRVR_DS_OFFSET	3
#define CA_ADR_DRVR_DS_OFFSET	0

#define PHY_CON42_CTRL_BSTLEN_SHIFT	8
#define PHY_CON42_CTRL_RDLAT_SHIFT	0

/*
 * Definitions that differ with SoC's.
 * Below is the part defining macros for Exynos5250.
 * Else part introduces macros for Exynos5420.
 */
#ifndef CONFIG_EXYNOS5420

/* APLL_CON1 */
#define APLL_CON1_VAL	(0x00203800)

/* MPLL_CON1 */
#define MPLL_CON1_VAL   (0x00203800)

/* CPLL_CON1 */
#define CPLL_CON1_VAL	(0x00203800)

/* DPLL_CON1 */
#define DPLL_CON1_VAL	(NOT_AVAILABLE)

/* GPLL_CON1 */
#define GPLL_CON1_VAL	(0x00203800)

/* EPLL_CON1, CON2 */
#define EPLL_CON1_VAL	0x00000000
#define EPLL_CON2_VAL	0x00000080

/* VPLL_CON1, CON2 */
#define VPLL_CON1_VAL	0x00000000
#define VPLL_CON2_VAL	0x00000080

/* RPLL_CON1, CON2 */
#define RPLL_CON1_VAL	NOT_AVAILABLE
#define RPLL_CON2_VAL	NOT_AVAILABLE

/* BPLL_CON1 */
#define BPLL_CON1_VAL	0x00203800

/* SPLL_CON1 */
#define SPLL_CON1_VAL	NOT_AVAILABLE

/* IPLL_CON1 */
#define IPLL_CON1_VAL	NOT_AVAILABLE

/* KPLL_CON1 */
#define KPLL_CON1_VAL	NOT_AVAILABLE

/* CLK_SRC_ISP */
#define CLK_SRC_ISP_VAL		NOT_AVAILABLE
#define CLK_DIV_ISP0_VAL	0x31
#define CLK_DIV_ISP1_VAL	0x0

/* CLK_FSYS */
#define CLK_SRC_FSYS0_VAL              0x66666
#define CLK_DIV_FSYS0_VAL	       0x0BB00000
#define CLK_DIV_FSYS1_VAL	       NOT_AVAILABLE
#define CLK_DIV_FSYS2_VAL	       NOT_AVAILABLE

/* CLK_SRC_CPU */
/* 0 = MOUTAPLL,  1 = SCLKMPLL */
#define MUX_HPM_SEL             0
#define MUX_CPU_SEL             0
#define MUX_APLL_SEL            1

#define CLK_SRC_CPU_VAL		((MUX_HPM_SEL << 20)    \
				| (MUX_CPU_SEL << 16)  \
				| (MUX_APLL_SEL))

/* CLK_SRC_CDREX */
#define CLK_SRC_CDREX_VAL       0x1

/* CLK_DIV_CDREX */
#define CLK_DIV_CDREX0_VAL	NOT_AVAILABLE
#define CLK_DIV_CDREX1_VAL	NOT_AVAILABLE

/* CLK_DIV_CPU0_VAL */
#define CLK_DIV_CPU0_VAL	NOT_AVAILABLE

#define MCLK_CDREX2_RATIO       0x0
#define ACLK_EFCON_RATIO        0x1
#define MCLK_DPHY_RATIO		0x1
#define MCLK_CDREX_RATIO	0x1
#define ACLK_C2C_200_RATIO	0x1
#define C2C_CLK_400_RATIO	0x1
#define PCLK_CDREX_RATIO	0x1
#define ACLK_CDREX_RATIO	0x1

#define CLK_DIV_CDREX_VAL	((MCLK_DPHY_RATIO << 24)        \
				| (C2C_CLK_400_RATIO << 6)	\
				| (PCLK_CDREX_RATIO << 4)	\
				| (ACLK_CDREX_RATIO))

/* CLK_SRC_TOP0	*/
#define MUX_ACLK_300_GSCL_SEL           0x0
#define MUX_ACLK_300_GSCL_MID_SEL       0x0
#define MUX_ACLK_400_G3D_MID_SEL        0x0
#define MUX_ACLK_333_SEL	        0x0
#define MUX_ACLK_300_DISP1_SEL	        0x0
#define MUX_ACLK_300_DISP1_MID_SEL      0x0
#define MUX_ACLK_200_SEL	        0x0
#define MUX_ACLK_166_SEL	        0x0
#define CLK_SRC_TOP0_VAL	((MUX_ACLK_300_GSCL_SEL  << 25)		\
				| (MUX_ACLK_300_GSCL_MID_SEL << 24)	\
				| (MUX_ACLK_400_G3D_MID_SEL << 20)	\
				| (MUX_ACLK_333_SEL << 16)		\
				| (MUX_ACLK_300_DISP1_SEL << 15)	\
				| (MUX_ACLK_300_DISP1_MID_SEL << 14)	\
				| (MUX_ACLK_200_SEL << 12)		\
				| (MUX_ACLK_166_SEL << 8))

/* CLK_SRC_TOP1	*/
#define MUX_ACLK_400_G3D_SEL            0x1
#define MUX_ACLK_400_ISP_SEL            0x0
#define MUX_ACLK_400_IOP_SEL            0x0
#define MUX_ACLK_MIPI_HSI_TXBASE_SEL    0x0
#define MUX_ACLK_300_GSCL_MID1_SEL      0x0
#define MUX_ACLK_300_DISP1_MID1_SEL     0x0
#define CLK_SRC_TOP1_VAL	((MUX_ACLK_400_G3D_SEL << 28)           \
				|(MUX_ACLK_400_ISP_SEL << 24)           \
				|(MUX_ACLK_400_IOP_SEL << 20)           \
				|(MUX_ACLK_MIPI_HSI_TXBASE_SEL << 16)   \
				|(MUX_ACLK_300_GSCL_MID1_SEL << 12)     \
				|(MUX_ACLK_300_DISP1_MID1_SEL << 8))

/* CLK_SRC_TOP2 */
#define MUX_GPLL_SEL                    0x1
#define MUX_BPLL_USER_SEL               0x0
#define MUX_MPLL_USER_SEL               0x0
#define MUX_VPLL_SEL                    0x1
#define MUX_EPLL_SEL                    0x1
#define MUX_CPLL_SEL                    0x1
#define VPLLSRC_SEL                     0x0
#define CLK_SRC_TOP2_VAL	((MUX_GPLL_SEL << 28)		\
				| (MUX_BPLL_USER_SEL << 24)	\
				| (MUX_MPLL_USER_SEL << 20)	\
				| (MUX_VPLL_SEL << 16)	        \
				| (MUX_EPLL_SEL << 12)	        \
				| (MUX_CPLL_SEL << 8)           \
				| (VPLLSRC_SEL))
/* CLK_SRC_TOP3 */
#define MUX_ACLK_333_SUB_SEL            0x1
#define MUX_ACLK_400_SUB_SEL            0x1
#define MUX_ACLK_266_ISP_SUB_SEL        0x1
#define MUX_ACLK_266_GPS_SUB_SEL        0x0
#define MUX_ACLK_300_GSCL_SUB_SEL       0x1
#define MUX_ACLK_266_GSCL_SUB_SEL       0x1
#define MUX_ACLK_300_DISP1_SUB_SEL      0x1
#define MUX_ACLK_200_DISP1_SUB_SEL      0x1
#define CLK_SRC_TOP3_VAL	((MUX_ACLK_333_SUB_SEL << 24)	        \
				| (MUX_ACLK_400_SUB_SEL << 20)	        \
				| (MUX_ACLK_266_ISP_SUB_SEL << 16)	\
				| (MUX_ACLK_266_GPS_SUB_SEL << 12)      \
				| (MUX_ACLK_300_GSCL_SUB_SEL << 10)     \
				| (MUX_ACLK_266_GSCL_SUB_SEL << 8)      \
				| (MUX_ACLK_300_DISP1_SUB_SEL << 6)     \
				| (MUX_ACLK_200_DISP1_SUB_SEL << 4))

#define CLK_SRC_TOP4_VAL	NOT_AVAILABLE
#define CLK_SRC_TOP5_VAL	NOT_AVAILABLE
#define CLK_SRC_TOP6_VAL	NOT_AVAILABLE
#define CLK_SRC_TOP7_VAL	NOT_AVAILABLE

/* CLK_DIV_TOP0	*/
#define ACLK_300_DISP1_RATIO	0x2
#define ACLK_400_G3D_RATIO	0x0
#define ACLK_333_RATIO		0x0
#define ACLK_266_RATIO		0x2
#define ACLK_200_RATIO		0x3
#define ACLK_166_RATIO		0x1
#define ACLK_133_RATIO		0x1
#define ACLK_66_RATIO		0x5

#define CLK_DIV_TOP0_VAL	((ACLK_300_DISP1_RATIO << 28)	\
				| (ACLK_400_G3D_RATIO << 24)	\
				| (ACLK_333_RATIO  << 20)	\
				| (ACLK_266_RATIO << 16)	\
				| (ACLK_200_RATIO << 12)	\
				| (ACLK_166_RATIO << 8)		\
				| (ACLK_133_RATIO << 4)		\
				| (ACLK_66_RATIO))

/* CLK_DIV_TOP1	*/
#define ACLK_MIPI_HSI_TX_BASE_RATIO     0x3
#define ACLK_66_PRE_RATIO               0x1
#define ACLK_400_ISP_RATIO              0x1
#define ACLK_400_IOP_RATIO              0x1
#define ACLK_300_GSCL_RATIO             0x2

#define CLK_DIV_TOP1_VAL	((ACLK_MIPI_HSI_TX_BASE_RATIO << 28)	\
				| (ACLK_66_PRE_RATIO << 24)		\
				| (ACLK_400_ISP_RATIO  << 20)		\
				| (ACLK_400_IOP_RATIO << 16)		\
				| (ACLK_300_GSCL_RATIO << 12))

#define CLK_DIV_TOP2_VAL	NOT_AVAILABLE

/* PLL Lock Value Factor */
#define PLL_LOCK_FACTOR		250
#define PLL_X_LOCK_FACTOR	3000

/* CLK_SRC_PERIC0 */
#define PWM_SEL		6
#define UART3_SEL	6
#define UART2_SEL	6
#define UART1_SEL	6
#define UART0_SEL	6
/* SRC_CLOCK = SCLK_MPLL */
#define CLK_SRC_PERIC0_VAL	((PWM_SEL << 24)        \
				| (UART3_SEL << 12)     \
				| (UART2_SEL << 8)       \
				| (UART1_SEL << 4)      \
				| (UART0_SEL))

/* CLK_SRC_PERIC1 */
/* SRC_CLOCK = SCLK_MPLL */
#define SPI0_SEL		6
#define SPI1_SEL		6
#define SPI2_SEL		6
#define CLK_SRC_PERIC1_VAL	((SPI2_SEL << 24) \
				| (SPI1_SEL << 20) \
				| (SPI0_SEL << 16))

/* CLK_DIV_PERIL0	*/
#define UART5_RATIO	7
#define UART4_RATIO	7
#define UART3_RATIO	7
#define UART2_RATIO	7
#define UART1_RATIO	7
#define UART0_RATIO	7

#define CLK_DIV_PERIC0_VAL	((UART3_RATIO << 12)    \
				| (UART2_RATIO << 8)    \
				| (UART1_RATIO << 4)    \
				| (UART0_RATIO))
/* CLK_DIV_PERIC1 */
#define SPI1_RATIO		0x7
#define SPI0_RATIO		0xf
#define SPI1_SUB_RATIO		0x0
#define SPI0_SUB_RATIO		0x0
#define CLK_DIV_PERIC1_VAL	((SPI1_SUB_RATIO << 24) \
				| ((SPI1_RATIO << 16) \
				| (SPI0_SUB_RATIO << 8) \
				| (SPI0_RATIO << 0)))

/* CLK_DIV_PERIC2 */
#define SPI2_RATIO		0xf
#define SPI2_SUB_RATIO		0x0
#define CLK_DIV_PERIC2_VAL	((SPI2_SUB_RATIO << 8) \
				| (SPI2_RATIO << 0))

/* CLK_DIV_PERIC3 */
#define PWM_RATIO		8
#define CLK_DIV_PERIC3_VAL	(PWM_RATIO << 0)


/* CLK_DIV_PERIC4 */
#define CLK_DIV_PERIC4_VAL	NOT_AVAILABLE

/* CLK_SRC_DISP1_0 */
#define CLK_SRC_DISP1_0_VAL	0x6
#define CLK_DIV_DISP1_0_VAL	NOT_AVAILABLE

#define APLL_FOUT		(1 << 0)
#define KPLL_FOUT		NOT_AVAILABLE

#define CLK_DIV_CPERI1_VAL	NOT_AVAILABLE

#else

#define CPU_CONFIG_STATUS_OFFSET	0x80
#define CPU_RST_FLAG_VAL		0xFCBA0D10
#define PAD_RETENTION_DRAM_COREBLK_VAL	0x10000000

/* APLL_CON1 */
#define APLL_CON1_VAL	(0x0020F300)

/* MPLL_CON1 */
#define MPLL_CON1_VAL   (0x0020F300)


/* CPLL_CON1 */
#define CPLL_CON1_VAL	0x0020f300

/* DPLL_CON1 */
#define DPLL_CON1_VAL	(0x0020F300)

/* GPLL_CON1 */
#define GPLL_CON1_VAL	(NOT_AVAILABLE)


/* EPLL_CON1, CON2 */
#define EPLL_CON1_VAL	0x00000000
#define EPLL_CON2_VAL	0x00000080

/* VPLL_CON1, CON2 */
#define VPLL_CON1_VAL	0x0020f300
#define VPLL_CON2_VAL	NOT_AVAILABLE

/* RPLL_CON1, CON2 */
#define RPLL_CON1_VAL	0x00000000
#define RPLL_CON2_VAL	0x00000080

/* BPLL_CON1 */
#define BPLL_CON1_VAL	0x0020f300

/* SPLL_CON1 */
#define SPLL_CON1_VAL	0x0020f300

/* IPLL_CON1 */
#define IPLL_CON1_VAL	0x00000080

/* KPLL_CON1 */
#define KPLL_CON1_VAL	0x200000

/* CLK_SRC_ISP */
#define CLK_SRC_ISP_VAL		0x33366000
#define CLK_DIV_ISP0_VAL	0x13131300
#define CLK_DIV_ISP1_VAL	0xbb110202


/* CLK_FSYS */
#define CLK_SRC_FSYS0_VAL              0x33033300
#define CLK_DIV_FSYS0_VAL	       0x0
#define CLK_DIV_FSYS1_VAL	       0x04f13c4f
#define CLK_DIV_FSYS2_VAL	       0x041d0000

/* CLK_SRC_CPU */
/* 0 = MOUTAPLL,  1 = SCLKMPLL */
#define MUX_HPM_SEL             1
#define MUX_CPU_SEL             0
#define MUX_APLL_SEL            1

#define CLK_SRC_CPU_VAL		((MUX_HPM_SEL << 20)    \
				| (MUX_CPU_SEL << 16)  \
				| (MUX_APLL_SEL))

/* CLK_SRC_CDREX */
#define CLK_SRC_CDREX_VAL       0x00000011

/* CLK_DIV_CDREX */
#define CLK_DIV_CDREX0_VAL	0x30010100
#define CLK_DIV_CDREX1_VAL	0x300

#define CLK_DIV_CDREX_VAL       0x17010100

/* CLK_DIV_CPU0_VAL */
#define CLK_DIV_CPU0_VAL	0x01440020

/* CLK_SRC_TOP */
#define CLK_SRC_TOP0_VAL	0x12221222
#define CLK_SRC_TOP1_VAL	0x00100200
#define CLK_SRC_TOP2_VAL	0x11101000
#define CLK_SRC_TOP3_VAL	0x11111111
#define CLK_SRC_TOP4_VAL	0x11110111
#define CLK_SRC_TOP5_VAL	0x11111101
#define CLK_SRC_TOP6_VAL	0x11110111
#define CLK_SRC_TOP7_VAL	0x00022200

/* CLK_DIV_TOP */
#define CLK_DIV_TOP0_VAL	0x23712311
#define CLK_DIV_TOP1_VAL	0x13100B00
#define CLK_DIV_TOP2_VAL	0x11101100

/* PLL Lock Value Factor */
#define PLL_LOCK_FACTOR		200
#define PLL_X_LOCK_FACTOR	3000

/* CLK_SRC_PERIC0 */
#define SPDIF_SEL	1
#define PWM_SEL		3
#define UART4_SEL	3
#define UART3_SEL	3
#define UART2_SEL	3
#define UART1_SEL	3
#define UART0_SEL	3
/* SRC_CLOCK = SCLK_RPLL */
#define CLK_SRC_PERIC0_VAL	((SPDIF_SEL << 28)	\
				| (PWM_SEL << 24)	\
				| (UART4_SEL << 20)	\
				| (UART3_SEL << 16)	\
				| (UART2_SEL << 12)	\
				| (UART1_SEL << 8)	\
				| (UART0_SEL << 4))

/* CLK_SRC_PERIC1 */
/* SRC_CLOCK = SCLK_EPLL */
#define SPI0_SEL		6
#define SPI1_SEL		6
#define SPI2_SEL		6
#define AUDIO0_SEL		6
#define AUDIO1_SEL		6
#define AUDIO2_SEL		6
#define CLK_SRC_PERIC1_VAL	((SPI2_SEL << 28)	\
				| (SPI1_SEL << 24)	\
				| (SPI0_SEL << 20)	\
				| (AUDIO2_SEL << 16)	\
				| (AUDIO2_SEL << 12)	\
				| (AUDIO2_SEL << 8))

/* CLK_DIV_PERIC0 */
#define PWM_RATIO	8
#define UART4_RATIO	9
#define UART3_RATIO	9
#define UART2_RATIO	9
#define UART1_RATIO	9
#define UART0_RATIO	9

#define CLK_DIV_PERIC0_VAL	((PWM_RATIO << 28)	\
				| (UART4_RATIO << 24)	\
				| (UART3_RATIO << 20)    \
				| (UART2_RATIO << 16)    \
				| (UART1_RATIO << 12)    \
				| (UART0_RATIO << 8))
/* CLK_DIV_PERIC1 */
#define SPI2_RATIO		0x1
#define SPI1_RATIO		0x1
#define SPI0_RATIO		0x1
#define CLK_DIV_PERIC1_VAL	((SPI2_RATIO << 28)	\
				| (SPI1_RATIO << 24)	\
				| (SPI0_RATIO << 20))

/* CLK_DIV_PERIC2 */
#define PCM2_RATIO		0x3
#define PCM1_RATIO		0x3
#define CLK_DIV_PERIC2_VAL	((PCM2_RATIO << 24) \
				| (PCM1_RATIO << 16))

/* CLK_DIV_PERIC3 */
#define AUDIO2_RATIO		0x5
#define AUDIO1_RATIO		0x5
#define AUDIO0_RATIO		0x5
#define CLK_DIV_PERIC3_VAL	((AUDIO2_RATIO << 28)	\
				| (AUDIO1_RATIO << 24)	\
				| (AUDIO0_RATIO << 20))

/* CLK_DIV_PERIC4 */
#define SPI2_PRE_RATIO		0x2
#define SPI1_PRE_RATIO		0x2
#define SPI0_PRE_RATIO		0x2
#define CLK_DIV_PERIC4_VAL	((SPI2_PRE_RATIO << 24)	\
				| (SPI1_PRE_RATIO << 16) \
				| (SPI0_PRE_RATIO << 8))

/* CLK_SRC_DISP1_0 */
#define CLK_SRC_DISP1_0_VAL	0x10666600
#define CLK_DIV_DISP1_0_VAL	0x01050211

#define APLL_FOUT		(1 << 0)
#define KPLL_FOUT		(1 << 0)

#define CLK_DIV_CPERI1_VAL	0x3f3f0000
#endif

struct mem_timings;

/* Errors that we can encourter in low-level setup */
enum {
	SETUP_ERR_OK,
	SETUP_ERR_RDLV_COMPLETE_TIMEOUT = -1,
	SETUP_ERR_ZQ_CALIBRATION_FAILURE = -2,
};

/*
 * Memory variant specific initialization code for DDR3
 *
 * @param mem          Memory timings for this memory type.
 * @param reset         Reset DDR PHY during initialization.
 * @return 0 if ok, SETUP_ERR_... if there is a problem
 */
int ddr3_mem_ctrl_init(struct mem_timings *mem, int reset);

/* Memory variant specific initialization code for LPDDR3 */
void lpddr3_mem_ctrl_init(void);

/*
 * Configure ZQ I/O interface
 *
 * @param mem		Memory timings for this memory type.
 * @param phy0_con16	Register address for dmc_phy0->phy_con16
 * @param phy1_con16	Register address for dmc_phy1->phy_con16
 * @param phy0_con17	Register address for dmc_phy0->phy_con17
 * @param phy1_con17	Register address for dmc_phy1->phy_con17
 * @return 0 if ok, -1 on error
 */
int dmc_config_zq(struct mem_timings *mem, uint32_t *phy0_con16,
			uint32_t *phy1_con16, uint32_t *phy0_con17,
			uint32_t *phy1_con17);
/*
 * Send NOP and MRS/EMRS Direct commands
 *
 * @param mem		Memory timings for this memory type.
 * @param directcmd	Register address for dmc_phy->directcmd
 */
void dmc_config_mrs(struct mem_timings *mem, uint32_t *directcmd);

/*
 * Send PALL Direct commands
 *
 * @param mem		Memory timings for this memory type.
 * @param directcmd	Register address for dmc_phy->directcmd
 */
void dmc_config_prech(struct mem_timings *mem, uint32_t *directcmd);

/*
 * Reset the DLL. This function is common between DDR3 and LPDDR2.
 * However, the reset value is different. So we are passing a flag
 * ddr_mode to distinguish between LPDDR2 and DDR3.
 *
 * @param phycontrol0	Register address for dmc_phy->phycontrol0
 * @param ddr_mode	Type of DDR memory
 */
void update_reset_dll(uint32_t *phycontrol0, enum ddr_mode);
#endif
