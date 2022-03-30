/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015, Freescale Semiconductor, Inc.
 */

#ifndef __ARCH_ARM_MACH_S32V234_MCME_REGS_H__
#define __ARCH_ARM_MACH_S32V234_MCME_REGS_H__

#ifndef __ASSEMBLY__

/* MC_ME registers definitions */

/* MC_ME_GS */
#define MC_ME_GS						(MC_ME_BASE_ADDR + 0x00000000)

#define MC_ME_GS_S_SYSCLK_FIRC			(0x0 << 0)
#define MC_ME_GS_S_SYSCLK_FXOSC			(0x1 << 0)
#define MC_ME_GS_S_SYSCLK_ARMPLL		(0x2 << 0)
#define MC_ME_GS_S_STSCLK_DISABLE		(0xF << 0)
#define MC_ME_GS_S_FIRC					(1 << 4)
#define MC_ME_GS_S_XOSC					(1 << 5)
#define MC_ME_GS_S_ARMPLL				(1 << 6)
#define MC_ME_GS_S_PERPLL				(1 << 7)
#define MC_ME_GS_S_ENETPLL				(1 << 8)
#define MC_ME_GS_S_DDRPLL				(1 << 9)
#define MC_ME_GS_S_VIDEOPLL				(1 << 10)
#define MC_ME_GS_S_MVR					(1 << 20)
#define MC_ME_GS_S_PDO					(1 << 23)
#define MC_ME_GS_S_MTRANS				(1 << 27)
#define MC_ME_GS_S_CRT_MODE_RESET		(0x0 << 28)
#define MC_ME_GS_S_CRT_MODE_TEST		(0x1 << 28)
#define MC_ME_GS_S_CRT_MODE_DRUN		(0x3 << 28)
#define MC_ME_GS_S_CRT_MODE_RUN0		(0x4 << 28)
#define MC_ME_GS_S_CRT_MODE_RUN1		(0x5 << 28)
#define MC_ME_GS_S_CRT_MODE_RUN2		(0x6 << 28)
#define MC_ME_GS_S_CRT_MODE_RUN3		(0x7 << 28)

/* MC_ME_MCTL */
#define MC_ME_MCTL						(MC_ME_BASE_ADDR + 0x00000004)

#define MC_ME_MCTL_KEY					(0x00005AF0)
#define MC_ME_MCTL_INVERTEDKEY			(0x0000A50F)
#define MC_ME_MCTL_RESET				(0x0 << 28)
#define MC_ME_MCTL_TEST					(0x1 << 28)
#define MC_ME_MCTL_DRUN					(0x3 << 28)
#define MC_ME_MCTL_RUN0					(0x4 << 28)
#define MC_ME_MCTL_RUN1					(0x5 << 28)
#define MC_ME_MCTL_RUN2					(0x6 << 28)
#define MC_ME_MCTL_RUN3					(0x7 << 28)

/* MC_ME_ME */
#define MC_ME_ME						(MC_ME_BASE_ADDR + 0x00000008)

#define MC_ME_ME_RESET_FUNC				(1 << 0)
#define MC_ME_ME_TEST					(1 << 1)
#define MC_ME_ME_DRUN					(1 << 3)
#define MC_ME_ME_RUN0					(1 << 4)
#define MC_ME_ME_RUN1					(1 << 5)
#define MC_ME_ME_RUN2					(1 << 6)
#define MC_ME_ME_RUN3					(1 << 7)

/* MC_ME_RUN_PCn */
#define MC_ME_RUN_PCn(n)				(MC_ME_BASE_ADDR + 0x00000080 + 0x4 * (n))

#define MC_ME_RUN_PCn_RESET				(1 << 0)
#define MC_ME_RUN_PCn_TEST				(1 << 1)
#define MC_ME_RUN_PCn_DRUN				(1 << 3)
#define MC_ME_RUN_PCn_RUN0				(1 << 4)
#define MC_ME_RUN_PCn_RUN1				(1 << 5)
#define MC_ME_RUN_PCn_RUN2				(1 << 6)
#define MC_ME_RUN_PCn_RUN3				(1 << 7)

/*
 * MC_ME_RESET_MC/MC_ME_TEST_MC
 * MC_ME_DRUN_MC
 * MC_ME_RUNn_MC
 */
#define MC_ME_RESET_MC						(MC_ME_BASE_ADDR + 0x00000020)
#define MC_ME_TEST_MC						(MC_ME_BASE_ADDR + 0x00000024)
#define MC_ME_DRUN_MC						(MC_ME_BASE_ADDR + 0x0000002C)
#define MC_ME_RUNn_MC(n)					(MC_ME_BASE_ADDR + 0x00000030 + 0x4 * (n))

#define MC_ME_RUNMODE_MC_SYSCLK(val)	(MC_ME_RUNMODE_MC_SYSCLK_MASK & (val))
#define MC_ME_RUNMODE_MC_SYSCLK_MASK	(0x0000000F)
#define MC_ME_RUNMODE_MC_FIRCON			(1 << 4)
#define MC_ME_RUNMODE_MC_XOSCON			(1 << 5)
#define MC_ME_RUNMODE_MC_PLL(pll)		(1 << (6 + (pll)))
#define MC_ME_RUNMODE_MC_MVRON			(1 << 20)
#define MC_ME_RUNMODE_MC_PDO			(1 << 23)
#define MC_ME_RUNMODE_MC_PWRLVL0		(1 << 28)
#define MC_ME_RUNMODE_MC_PWRLVL1		(1 << 29)
#define MC_ME_RUNMODE_MC_PWRLVL2		(1 << 30)

/* MC_ME_DRUN_SEC_CC_I */
#define MC_ME_DRUN_SEC_CC_I					(MC_ME_BASE_ADDR + 0x260)
/* MC_ME_RUNn_SEC_CC_I */
#define MC_ME_RUNn_SEC_CC_I(n)				(MC_ME_BASE_ADDR + 0x270 + (n) * 0x10)
#define MC_ME_RUNMODE_SEC_CC_I_SYSCLK(val,offset)	((MC_ME_RUNMODE_SEC_CC_I_SYSCLK_MASK & (val)) << offset)
#define MC_ME_RUNMODE_SEC_CC_I_SYSCLK1_OFFSET	(4)
#define MC_ME_RUNMODE_SEC_CC_I_SYSCLK2_OFFSET	(8)
#define MC_ME_RUNMODE_SEC_CC_I_SYSCLK3_OFFSET	(12)
#define MC_ME_RUNMODE_SEC_CC_I_SYSCLK_MASK		(0x3)

/*
 * ME_PCTLn
 * Please note that these registers are 8 bits width, so
 * the operations over them should be done using 8 bits operations.
 */
#define MC_ME_PCTLn_RUNPCm(n)			( (n) & MC_ME_PCTLn_RUNPCm_MASK )
#define MC_ME_PCTLn_RUNPCm_MASK			(0x7)

/* DEC200 Peripheral Control Register		*/
#define MC_ME_PCTL39	(MC_ME_BASE_ADDR + 0x000000E4)
/* 2D-ACE Peripheral Control Register		*/
#define MC_ME_PCTL40	(MC_ME_BASE_ADDR + 0x000000EB)
/* ENET Peripheral Control Register		*/
#define MC_ME_PCTL50	(MC_ME_BASE_ADDR + 0x000000F1)
/* DMACHMUX0 Peripheral Control Register	*/
#define MC_ME_PCTL49	(MC_ME_BASE_ADDR + 0x000000F2)
/* CSI0 Peripheral Control Register			*/
#define MC_ME_PCTL48	(MC_ME_BASE_ADDR + 0x000000F3)
/* MMDC0 Peripheral Control Register		*/
#define MC_ME_PCTL54	(MC_ME_BASE_ADDR + 0x000000F5)
/* FRAY Peripheral Control Register			*/
#define MC_ME_PCTL52	(MC_ME_BASE_ADDR + 0x000000F7)
/* PIT0 Peripheral Control Register			*/
#define MC_ME_PCTL58	(MC_ME_BASE_ADDR + 0x000000F9)
/* FlexTIMER0 Peripheral Control Register	*/
#define MC_ME_PCTL79	(MC_ME_BASE_ADDR + 0x0000010C)
/* SARADC0 Peripheral Control Register		*/
#define MC_ME_PCTL77	(MC_ME_BASE_ADDR + 0x0000010E)
/* LINFLEX0 Peripheral Control Register		*/
#define MC_ME_PCTL83	(MC_ME_BASE_ADDR + 0x00000110)
/* IIC0 Peripheral Control Register			*/
#define MC_ME_PCTL81	(MC_ME_BASE_ADDR + 0x00000112)
/* DSPI0 Peripheral Control Register		*/
#define MC_ME_PCTL87	(MC_ME_BASE_ADDR + 0x00000114)
/* CANFD0 Peripheral Control Register		*/
#define MC_ME_PCTL85	(MC_ME_BASE_ADDR + 0x00000116)
/* CRC0 Peripheral Control Register			*/
#define MC_ME_PCTL91	(MC_ME_BASE_ADDR + 0x00000118)
/* DSPI2 Peripheral Control Register		*/
#define MC_ME_PCTL89	(MC_ME_BASE_ADDR + 0x0000011A)
/* SDHC Peripheral Control Register			*/
#define MC_ME_PCTL93	(MC_ME_BASE_ADDR + 0x0000011E)
/* VIU0 Peripheral Control Register			*/
#define MC_ME_PCTL100	(MC_ME_BASE_ADDR + 0x00000127)
/* HPSMI Peripheral Control Register		*/
#define MC_ME_PCTL104	(MC_ME_BASE_ADDR + 0x0000012B)
/* SIPI Peripheral Control Register			*/
#define MC_ME_PCTL116	(MC_ME_BASE_ADDR + 0x00000137)
/* LFAST Peripheral Control Register		*/
#define MC_ME_PCTL120	(MC_ME_BASE_ADDR + 0x0000013B)
/* MMDC1 Peripheral Control Register		*/
#define MC_ME_PCTL162	(MC_ME_BASE_ADDR + 0x00000161)
/* DMACHMUX1 Peripheral Control Register	*/
#define MC_ME_PCTL161	(MC_ME_BASE_ADDR + 0x00000162)
/* CSI1 Peripheral Control Register			*/
#define MC_ME_PCTL160	(MC_ME_BASE_ADDR + 0x00000163)
/* QUADSPI0 Peripheral Control Register		*/
#define MC_ME_PCTL166	(MC_ME_BASE_ADDR + 0x00000165)
/* PIT1 Peripheral Control Register			*/
#define MC_ME_PCTL170	(MC_ME_BASE_ADDR + 0x00000169)
/* FlexTIMER1 Peripheral Control Register	*/
#define MC_ME_PCTL182	(MC_ME_BASE_ADDR + 0x00000175)
/* IIC2 Peripheral Control Register			*/
#define MC_ME_PCTL186	(MC_ME_BASE_ADDR + 0x00000179)
/* IIC1 Peripheral Control Register			*/
#define MC_ME_PCTL184	(MC_ME_BASE_ADDR + 0x0000017B)
/* CANFD1 Peripheral Control Register		*/
#define MC_ME_PCTL190	(MC_ME_BASE_ADDR + 0x0000017D)
/* LINFLEX1 Peripheral Control Register		*/
#define MC_ME_PCTL188	(MC_ME_BASE_ADDR + 0x0000017F)
/* DSPI3 Peripheral Control Register		*/
#define MC_ME_PCTL194	(MC_ME_BASE_ADDR + 0x00000181)
/* DSPI1 Peripheral Control Register		*/
#define MC_ME_PCTL192	(MC_ME_BASE_ADDR + 0x00000183)
/* TSENS Peripheral Control Register		*/
#define MC_ME_PCTL206	(MC_ME_BASE_ADDR + 0x0000018D)
/* CRC1 Peripheral Control Register			*/
#define MC_ME_PCTL204	(MC_ME_BASE_ADDR + 0x0000018F)
/* VIU1 Peripheral Control Register		*/
#define MC_ME_PCTL208	(MC_ME_BASE_ADDR + 0x00000193)
/* JPEG Peripheral Control Register		*/
#define MC_ME_PCTL212	(MC_ME_BASE_ADDR + 0x00000197)
/* H264_DEC Peripheral Control Register	*/
#define MC_ME_PCTL216	(MC_ME_BASE_ADDR + 0x0000019B)
/* H264_ENC Peripheral Control Register	*/
#define MC_ME_PCTL220	(MC_ME_BASE_ADDR + 0x0000019F)
/* MBIST Peripheral Control Register	*/
#define MC_ME_PCTL236	(MC_ME_BASE_ADDR + 0x000001A9)

/* Core status register */
#define MC_ME_CS               (MC_ME_BASE_ADDR + 0x000001C0)

#endif

#endif /*__ARCH_ARM_MACH_S32V234_MCME_REGS_H__ */
