/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015-2016, Freescale Semiconductor, Inc.
 */

#ifndef __ARCH_ARM_MACH_S32V234_LPDDR2_H__
#define __ARCH_ARM_MACH_S32V234_LPDDR2_H__

/* definitions for LPDDR2 PAD values */
#define LPDDR2_CLK0_PAD	\
	(SIUL2_MSCR_DDR_SEL_LPDDR2 | SIUL2_MSCR_DDR_INPUT_DIFF_DDR | SIUL2_MSCR_DDR_ODT_120ohm |\
	 SIUL2_MSCR_DSE_48ohm | SIUL2_MSCR_PUS_100K_UP | SIUL2_MSCR_CRPOINT_TRIM_1 |			\
	 SIUL2_MSCR_DCYCLE_TRIM_NONE)
#define LPDDR2_CKEn_PAD	\
	(SIUL2_MSCR_DDR_SEL_LPDDR2 | SIUL2_MSCR_DDR_INPUT_DIFF_DDR | SIUL2_MSCR_DDR_ODT_120ohm |\
	 SIUL2_MSCR_PUS_100K_UP | SIUL2_MSCR_DSE_48ohm)
#define LPDDR2_CS_Bn_PAD	\
	(SIUL2_MSCR_DDR_SEL_LPDDR2 | SIUL2_MSCR_DDR_INPUT_DIFF_DDR | SIUL2_MSCR_DDR_ODT_120ohm |\
	 SIUL2_MSCR_PUS_100K_UP | SIUL2_MSCR_DSE_48ohm)
#define LPDDR2_DMn_PAD	\
	(SIUL2_MSCR_DDR_SEL_LPDDR2 | SIUL2_MSCR_DDR_INPUT_DIFF_DDR | SIUL2_MSCR_DDR_ODT_120ohm |\
	 SIUL2_MSCR_PUS_100K_UP | SIUL2_MSCR_DSE_48ohm)
#define LPDDR2_DQSn_PAD	\
	(SIUL2_MSCR_DDR_SEL_LPDDR2 | SIUL2_MSCR_DDR_INPUT_DIFF_DDR | SIUL2_MSCR_DDR_ODT_120ohm |	\
	 SIUL2_MSCR_DSE_48ohm | SIUL2_MSCR_PUE_EN | SIUL2_MSCR_PUS_100K_DOWN |						\
	 SIUL2_MSCR_PKE_EN | SIUL2_MSCR_CRPOINT_TRIM_1 | SIUL2_MSCR_DCYCLE_TRIM_NONE)
#define LPDDR2_An_PAD	\
	(SIUL2_MSCR_DDR_SEL_LPDDR2 | SIUL2_MSCR_DDR_INPUT_DIFF_DDR | SIUL2_MSCR_DDR_ODT_120ohm |	\
	 SIUL2_MSCR_DSE_48ohm | SIUL2_MSCR_DDR_DO_TRIM_50PS | SIUL2_MSCR_DCYCLE_TRIM_LEFT		|	\
	 SIUL2_MSCR_PUS_100K_UP)
#define LPDDR2_Dn_PAD	\
	(SIUL2_MSCR_DDR_SEL_LPDDR2 | SIUL2_MSCR_DDR_INPUT_DIFF_DDR | SIUL2_MSCR_DDR_ODT_120ohm |	\
	 SIUL2_MSCR_DSE_48ohm | SIUL2_MSCR_DDR_DO_TRIM_50PS | SIUL2_MSCR_DCYCLE_TRIM_LEFT		|	\
	 SIUL2_MSCR_PUS_100K_UP)

#define _MDCTL							0x03010000

#define MMDC_MDSCR_CFG_VALUE			0x00008000	/* Set MDSCR[CON_REQ] (configuration request) */
#define MMDC_MDCFG0_VALUE				0x464F61A5	/* tRFCab=70 (=130ns),tXSR=80 (=tRFCab+10ns),tXP=4 (=7.5ns),tXPDLL=n/a,tFAW=27 (50 ns),tCL(RL)=8 */
#define MMDC_MDCFG1_VALUE				0x00180E63	/* tRCD=n/a,tRPpb=n/a,tRC=n/a ,tRAS=25 (=47ns),tRPA=n/a,tWR=8 (=15.0ns),tMRD=3,tWL=4 */
#define MMDC_MDCFG2_VALUE				0x000000DD	/* tDLLK=n/a,tRTP=4 (=7.5ns),tWTR=4 (=7.5ns),tRRD=6 (=10ns) */
#define MMDC_MDCFG3LP_VALUE				0x001F099B	/* RC_LP=tRAS+tRPab=32 (>60ns), tRCD_LP=10 (18ns) , tRPpb_LP=10 (18ns), tRPab_LP=12 (21ns) */
#define MMDC_MDOTC_VALUE				0x00000000	/* tAOFPD=n/a,tAONPD=n/a,tANPD=n/a,tAXPD=n/a,tODTLon=n/a,tODT_idle_off=n/a */
#define MMDC_MDMISC_VALUE				0x00001688	/* WALAT=0, BI bank interleave on, LPDDR2_S2=0, MIF3=3, RALAT=2, 8 banks, LPDDR2 */
#define MMDC_MDOR_VALUE					0x00000010	/* tXPR=n/a , SDE_to_RST=n/a, RST_to_CKE=14 */
#define MMDC_MPMUR0_VALUE				0x00000800	/* Force delay line initialisation */
#define MMDC_MDSCR_RST_VALUE			0x003F8030	/* Reset command CS0 */
#define MMDC_MPZQLP2CTL_VALUE			0x1B5F0109	/* ZQ_LP2_HW_ZQCS=0x1B (90ns spec), ZQ_LP2_HW_ZQCL=0x5F (160ns spec), ZQ_LP2_HW_ZQINIT=0x109 (1us spec) */
#define MMDC_MPZQHWCTRL_VALUE			0xA0010003	/* ZQ_EARLY_COMPARATOR_EN_TIMER=0x14, TZQ_CS=n/a, TZQ_OPER=n/a, TZQ_INIT=n/a, ZQ_HW_FOR=1, ZQ_HW_PER=0, ZQ_MODE=3 */
#define MMDC_MDSCR_MR1_VALUE			0xC2018030	/* Configure MR1: BL 4, burst type interleaved, wrap control no wrap, tWR cycles 8 */
#define MMDC_MDSCR_MR2_VALUE			0x06028030	/* Configure MR2: RL=8, WL=4 */
#define MMDC_MDSCR_MR3_VALUE			0x01038030	/* Configure MR3: DS=34R */
#define MMDC_MDSCR_MR10_VALUE			0xFF0A8030	/* Configure MR10: Calibration at init */
#define MMDC_MDASP_MODULE0_VALUE		0x0000007F	/* 2Gb, 256 MB memory so CS0 is 256 MB  (0x90000000) */
#define MMDC_MPRDDLCTL_MODULE0_VALUE	0x4D4B4F4B	/* Read delay line offsets */
#define MMDC_MPWRDLCTL_MODULE0_VALUE	0x38383737	/* Write delay line offsets */
#define MMDC_MPDGCTRL0_MODULE0_VALUE	0x20000000	/* Read DQS gating control 0 (disabled) */
#define MMDC_MPDGCTRL1_MODULE0_VALUE	0x00000000	/* Read DQS gating control 1 */
#define MMDC_MDASP_MODULE1_VALUE		0x0000007F	/* 2Gb, 256 MB memory so CS0 is 256 MB  (0xD0000000) */
#define MMDC_MPRDDLCTL_MODULE1_VALUE	0x4D4B4F4B	/* Read delay line offsets */
#define MMDC_MPWRDLCTL_MODULE1_VALUE	0x38383737	/* Write delay line offsets */
#define MMDC_MPDGCTRL0_MODULE1_VALUE	0x20000000	/* Read DQS gating control 0 (disabled) */
#define MMDC_MPDGCTRL1_MODULE1_VALUE	0x00000000	/* Read DQS gating control 1 */
#define MMDC_MDRWD_VALUE				0x0F9F26D2	/* Read/write command delay - default used */
#define MMDC_MDPDC_VALUE				0x00020024	/* Power down control */
#define MMDC_MDREF_VALUE				0x30B01800	/* Refresh control */
#define MMDC_MPODTCTRL_VALUE			0x00000000	/* No ODT */
#define MMDC_MDSCR_DEASSERT_VALUE				0x00000000	/* Deassert the configuration request */

/* set I/O pads for DDR */
void lpddr2_config_iomux(uint8_t module);
void config_mmdc(uint8_t module);

#endif
