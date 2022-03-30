/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2006-2008
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 */

#ifndef _MEM_H_
#define _MEM_H_

#define CS0		0x0
#define CS1		0x1 /* mirror CS1 regs appear offset 0x30 from CS0 */

#ifndef __ASSEMBLY__
enum {
	STACKED = 0,
	IP_DDR = 1,
	COMBO_DDR = 2,
	IP_SDR = 3,
};
#endif /* __ASSEMBLY__ */

#define EARLY_INIT	1

/*
 * For a full explanation of these registers and values please see
 * the Technical Reference Manual (TRM) for any of the processors in
 * this family.
 */

/* Slower full frequency range default timings for x32 operation*/
#define SDRC_SHARING	0x00000100
#define SDRC_MR_0_SDR	0x00000031

/*
 * SDRC autorefresh control values.  This register consists of autorefresh
 * enable at bits 0:1 and an autorefresh counter value in bits 8:23.  The
 * counter is a result of ( tREFI / tCK ) - 50.
 */
#define SDP_3430_SDRC_RFR_CTRL_100MHz	0x0002da01
#define SDP_3430_SDRC_RFR_CTRL_133MHz	0x0003de01 /* 7.8us/7.5ns - 50=0x3de */
#define SDP_3430_SDRC_RFR_CTRL_165MHz	0x0004e201 /* 7.8us/6ns - 50=0x4e2 */
#define SDP_3430_SDRC_RFR_CTRL_200MHz	0x0005e601 /* 7.8us/5ns - 50=0x5e6 */

#define DLL_OFFSET		0
#define DLL_WRITEDDRCLKX2DIS	1
#define DLL_ENADLL		1
#define DLL_LOCKDLL		0
#define DLL_DLLPHASE_72		0
#define DLL_DLLPHASE_90		1

/* rkw - need to find of 90/72 degree recommendation for speed like before */
#define SDP_SDRC_DLLAB_CTRL	((DLL_ENADLL << 3) | \
				(DLL_LOCKDLL << 2) | (DLL_DLLPHASE_90 << 1))

/* Helper macros to arrive at value of the SDRC_ACTIM_CTRLA register. */
#define ACTIM_CTRLA_TRFC(v)	(((v) & 0x1F) << 27)	/* 31:27 */
#define ACTIM_CTRLA_TRC(v)	(((v) & 0x1F) << 22)	/* 26:22 */
#define ACTIM_CTRLA_TRAS(v)	(((v) & 0x0F) << 18)	/* 21:18 */
#define ACTIM_CTRLA_TRP(v)	(((v) & 0x07) << 15)	/* 17:15 */
#define ACTIM_CTRLA_TRCD(v)	(((v) & 0x07) << 12)	/* 14:12 */
#define ACTIM_CTRLA_TRRD(v)	(((v) & 0x07) << 9)	/* 11:9  */
#define ACTIM_CTRLA_TDPL(v)	(((v) & 0x07) << 6)	/*  8:6  */
#define ACTIM_CTRLA_TDAL(v)	(v & 0x1F)		/*  4:0  */

#define ACTIM_CTRLA(trfc, trc, tras, trp, trcd, trrd, tdpl, tdal)	\
		ACTIM_CTRLA_TRFC(trfc)	|	\
		ACTIM_CTRLA_TRC(trc)	|	\
		ACTIM_CTRLA_TRAS(tras)	|	\
		ACTIM_CTRLA_TRP(trp)	|	\
		ACTIM_CTRLA_TRCD(trcd)	|	\
		ACTIM_CTRLA_TRRD(trrd)	|	\
		ACTIM_CTRLA_TDPL(tdpl)	|	\
		ACTIM_CTRLA_TDAL(tdal)

/* Helper macros to arrive at value of the SDRC_ACTIM_CTRLB register. */
#define ACTIM_CTRLB_TWTR(v)	(((v) & 0x03) << 16)	/* 17:16 */
#define ACTIM_CTRLB_TCKE(v)	(((v) & 0x07) << 12)	/* 14:12 */
#define ACTIM_CTRLB_TXP(v)	(((v) & 0x07) << 8)	/* 10:8  */
#define ACTIM_CTRLB_TXSR(v)	(v & 0xFF)		/*  7:0  */

#define ACTIM_CTRLB(twtr, tcke, txp, txsr)		\
		ACTIM_CTRLB_TWTR(twtr)	|	\
		ACTIM_CTRLB_TCKE(tcke)	|	\
		ACTIM_CTRLB_TXP(txp)	|	\
		ACTIM_CTRLB_TXSR(txsr)

/*
 * Values used in the MCFG register.  Only values we use today
 * are defined and the rest can be found in the TRM.  Unless otherwise
 * noted all fields are one bit.
 */
#define V_MCFG_RAMTYPE_DDR		(0x1)
#define V_MCFG_DEEPPD_EN		(0x1 << 3)
#define V_MCFG_B32NOT16_32		(0x1 << 4)
#define V_MCFG_BANKALLOCATION_RBC	(0x2 << 6)		/* 6:7 */
#define V_MCFG_RAMSIZE(ramsize)		((((ramsize) >> 20)/2) << 8) /* 8:17 */
#define V_MCFG_ADDRMUXLEGACY_FLEX	(0x1 << 19)
#define V_MCFG_CASWIDTH(caswidth)	(((caswidth)-5) << 20)	/* 20:22 */
#define V_MCFG_CASWIDTH_10B		V_MCFG_CASWIDTH(10)
#define V_MCFG_RASWIDTH(raswidth)	(((raswidth)-11) << 24)	/* 24:26 */

/* Macro to construct MCFG */
#define MCFG(ramsize, raswidth)						\
		V_MCFG_RASWIDTH(raswidth) | V_MCFG_CASWIDTH_10B |	\
		V_MCFG_ADDRMUXLEGACY_FLEX | V_MCFG_RAMSIZE(ramsize) |	\
		V_MCFG_BANKALLOCATION_RBC | V_MCFG_B32NOT16_32 |	\
		V_MCFG_DEEPPD_EN | V_MCFG_RAMTYPE_DDR

/* Hynix part of Overo (165MHz optimized) 6.06ns */
#define HYNIX_TDAL_165   6
#define HYNIX_TDPL_165   3
#define HYNIX_TRRD_165   2
#define HYNIX_TRCD_165   3
#define HYNIX_TRP_165    3
#define HYNIX_TRAS_165   7
#define HYNIX_TRC_165   10
#define HYNIX_TRFC_165  21
#define HYNIX_V_ACTIMA_165	\
		ACTIM_CTRLA(HYNIX_TRFC_165, HYNIX_TRC_165,	\
				HYNIX_TRAS_165, HYNIX_TRP_165,	\
				HYNIX_TRCD_165, HYNIX_TRRD_165,	\
				HYNIX_TDPL_165, HYNIX_TDAL_165)

#define HYNIX_TWTR_165   1
#define HYNIX_TCKE_165   1
#define HYNIX_TXP_165    2
#define HYNIX_XSR_165    24
#define HYNIX_V_ACTIMB_165	\
		ACTIM_CTRLB(HYNIX_TWTR_165, HYNIX_TCKE_165,	\
				HYNIX_TXP_165, HYNIX_XSR_165)

#define HYNIX_RASWIDTH_165	13
#define HYNIX_V_MCFG_165(size)	MCFG((size), HYNIX_RASWIDTH_165)

/* Hynix part of AM/DM37xEVM (200MHz optimized) */
#define HYNIX_TDAL_200		6
#define HYNIX_TDPL_200		3
#define HYNIX_TRRD_200		2
#define HYNIX_TRCD_200		4
#define HYNIX_TRP_200		3
#define HYNIX_TRAS_200		8
#define HYNIX_TRC_200		11
#define HYNIX_TRFC_200		18
#define HYNIX_V_ACTIMA_200	\
		ACTIM_CTRLA(HYNIX_TRFC_200, HYNIX_TRC_200,	\
				HYNIX_TRAS_200, HYNIX_TRP_200,	\
				HYNIX_TRCD_200, HYNIX_TRRD_200,	\
				HYNIX_TDPL_200, HYNIX_TDAL_200)

#define HYNIX_TWTR_200		2
#define HYNIX_TCKE_200		1
#define HYNIX_TXP_200		1
#define HYNIX_XSR_200		28
#define HYNIX_V_ACTIMB_200	\
		ACTIM_CTRLB(HYNIX_TWTR_200, HYNIX_TCKE_200,	\
				HYNIX_TXP_200, HYNIX_XSR_200)

#define HYNIX_RASWIDTH_200	14
#define HYNIX_V_MCFG_200(size)	MCFG((size), HYNIX_RASWIDTH_200)

/* Infineon part of 3430SDP (165MHz optimized) 6.06ns */
#define INFINEON_TDAL_165	6	/* Twr/Tck + Trp/tck		*/
					/* 15/6 + 18/6 = 5.5 -> 6	*/
#define INFINEON_TDPL_165	3	/* 15/6 = 2.5 -> 3 (Twr)	*/
#define INFINEON_TRRD_165	2	/* 12/6 = 2			*/
#define INFINEON_TRCD_165	3	/* 18/6 = 3			*/
#define INFINEON_TRP_165	3	/* 18/6 = 3			*/
#define INFINEON_TRAS_165	7	/* 42/6 = 7			*/
#define INFINEON_TRC_165	10	/* 60/6 = 10			*/
#define INFINEON_TRFC_165	12	/* 72/6 = 12			*/

#define INFINEON_V_ACTIMA_165	\
		ACTIM_CTRLA(INFINEON_TRFC_165, INFINEON_TRC_165,	\
				INFINEON_TRAS_165, INFINEON_TRP_165,	\
				INFINEON_TRCD_165, INFINEON_TRRD_165,	\
				INFINEON_TDPL_165, INFINEON_TDAL_165)

#define INFINEON_TWTR_165	1
#define INFINEON_TCKE_165	2
#define INFINEON_TXP_165	2
#define INFINEON_XSR_165	20	/* 120/6 = 20	*/

#define INFINEON_V_ACTIMB_165	\
		ACTIM_CTRLB(INFINEON_TWTR_165, INFINEON_TCKE_165,	\
				INFINEON_TXP_165, INFINEON_XSR_165)

/* Micron part of 3430 EVM (165MHz optimized) 6.06ns */
#define MICRON_TDAL_165		6	/* Twr/Tck + Trp/tck		*/
					/* 15/6 + 18/6 = 5.5 -> 6	*/
#define MICRON_TDPL_165		3	/* 15/6 = 2.5 -> 3 (Twr)	*/
#define MICRON_TRRD_165		2	/* 12/6 = 2			*/
#define MICRON_TRCD_165		3	/* 18/6 = 3			*/
#define MICRON_TRP_165		3	/* 18/6 = 3			*/
#define MICRON_TRAS_165		7	/* 42/6 = 7			*/
#define MICRON_TRC_165		10	/* 60/6 = 10			*/
#define MICRON_TRFC_165		21	/* 125/6 = 21			*/

#define MICRON_V_ACTIMA_165	\
		ACTIM_CTRLA(MICRON_TRFC_165, MICRON_TRC_165,		\
				MICRON_TRAS_165, MICRON_TRP_165,	\
				MICRON_TRCD_165, MICRON_TRRD_165,	\
				MICRON_TDPL_165, MICRON_TDAL_165)

#define MICRON_TWTR_165		1
#define MICRON_TCKE_165		1
#define MICRON_XSR_165		23	/* 138/6 = 23		*/
#define MICRON_TXP_165		5	/* 25/6 = 4.1 => ~5	*/

#define MICRON_V_ACTIMB_165	\
		ACTIM_CTRLB(MICRON_TWTR_165, MICRON_TCKE_165,	\
				MICRON_TXP_165,	MICRON_XSR_165)

#define MICRON_RASWIDTH_165	13
#define MICRON_V_MCFG_165(size)	MCFG((size), MICRON_RASWIDTH_165)

#define MICRON_BL_165			0x2
#define MICRON_SIL_165			0x0
#define MICRON_CASL_165			0x3
#define MICRON_WBST_165			0x0
#define MICRON_V_MR_165			((MICRON_WBST_165 << 9) | \
		(MICRON_CASL_165 << 4) | (MICRON_SIL_165 << 3) | \
		(MICRON_BL_165))

/* Micron part (200MHz optimized) 5 ns */
#define MICRON_TDAL_200		6
#define MICRON_TDPL_200		3
#define MICRON_TRRD_200		2
#define MICRON_TRCD_200		3
#define MICRON_TRP_200		3
#define MICRON_TRAS_200		8
#define MICRON_TRC_200		11
#define MICRON_TRFC_200		15
#define MICRON_V_ACTIMA_200	\
		ACTIM_CTRLA(MICRON_TRFC_200, MICRON_TRC_200,		\
				MICRON_TRAS_200, MICRON_TRP_200,	\
				MICRON_TRCD_200, MICRON_TRRD_200,	\
				MICRON_TDPL_200, MICRON_TDAL_200)

#define MICRON_TWTR_200		2
#define MICRON_TCKE_200		4
#define MICRON_TXP_200		2
#define MICRON_XSR_200		23
#define MICRON_V_ACTIMB_200	\
		ACTIM_CTRLB(MICRON_TWTR_200, MICRON_TCKE_200,	\
				MICRON_TXP_200,	MICRON_XSR_200)

#define MICRON_RASWIDTH_200	14
#define MICRON_V_MCFG_200(size)	MCFG((size), MICRON_RASWIDTH_200)

/* Samsung K4X51163PG - FGC6 (165MHz optimized) 6.06ns - from 2010.90 src */
#define SAMSUNG_TDAL_165	5
#define SAMSUNG_TDPL_165	2
#define SAMSUNG_TRRD_165	2
#define SAMSUNG_TRCD_165	3
#define SAMSUNG_TRP_165		3
#define SAMSUNG_TRAS_165	7
#define SAMSUNG_TRC_165		10
#define SAMSUNG_TRFC_165	12

#define SAMSUNG_V_ACTIMA_165	\
		ACTIM_CTRLA(SAMSUNG_TRFC_165, SAMSUNG_TRC_165,		\
				SAMSUNG_TRAS_165, SAMSUNG_TRP_165,	\
				SAMSUNG_TRCD_165, SAMSUNG_TRRD_165,	\
				SAMSUNG_TDPL_165, SAMSUNG_TDAL_165)

#define SAMSUNG_TWTR_165	1
#define SAMSUNG_TCKE_165	2
#define SAMSUNG_XSR_165		20
#define SAMSUNG_TXP_165		5

#define SAMSUNG_V_ACTIMB_165	\
		ACTIM_CTRLB(SAMSUNG_TWTR_165, SAMSUNG_TCKE_165,	\
				SAMSUNG_TXP_165, SAMSUNG_XSR_165)

#define SAMSUNG_RASWIDTH_165	14
#define SAMSUNG_V_MCFG_165(size) \
	V_MCFG_RASWIDTH(SAMSUNG_RASWIDTH_165) | V_MCFG_CASWIDTH_10B | \
	V_MCFG_ADDRMUXLEGACY_FLEX | V_MCFG_RAMSIZE(size) | \
	V_MCFG_BANKALLOCATION_RBC | V_MCFG_RAMTYPE_DDR

/* TODO: find which register these were taken from */

#define SAMSUNG_BL_165				0x2
#define SAMSUNG_SIL_165				0x0
#define SAMSUNG_CASL_165			0x3
#define SAMSUNG_WBST_165			0x0
#define SAMSUNG_V_MR_165			((SAMSUNG_WBST_165 << 9) | \
		(SAMSUNG_CASL_165 << 4) | (SAMSUNG_SIL_165 << 3) | \
		(SAMSUNG_BL_165))

#define SAMSUNG_SHARING 0x00003700

/* NUMONYX part of IGEP v2 (165MHz optimized) 6.06ns */
#define NUMONYX_TDAL_165	6	/* Twr/Tck + Trp/tck		*/
					/* 15/6 + 18/6 = 5.5 -> 6	*/
#define NUMONYX_TDPL_165	3	/* 15/6 = 2.5 -> 3 (Twr)	*/
#define NUMONYX_TRRD_165	2	/* 12/6 = 2			*/
#define NUMONYX_TRCD_165	4	/* 22.5/6 = 3.75 -> 4		*/
#define NUMONYX_TRP_165		3	/* 18/6 = 3			*/
#define NUMONYX_TRAS_165	7	/* 42/6 = 7			*/
#define NUMONYX_TRC_165		10	/* 60/6 = 10			*/
#define NUMONYX_TRFC_165	24	/* 140/6 = 23.3 -> 24		*/

#define NUMONYX_V_ACTIMA_165	\
		ACTIM_CTRLA(NUMONYX_TRFC_165, NUMONYX_TRC_165,		\
				NUMONYX_TRAS_165, NUMONYX_TRP_165,	\
				NUMONYX_TRCD_165, NUMONYX_TRRD_165,	\
				NUMONYX_TDPL_165, NUMONYX_TDAL_165)

#define NUMONYX_TWTR_165	2
#define NUMONYX_TCKE_165	2
#define NUMONYX_TXP_165		3	/* 200/6 =  33.3 -> 34	*/
#define NUMONYX_XSR_165		34	/* 1.0 + 1.1 = 2.1 -> 3	*/

#define NUMONYX_V_ACTIMB_165	\
		ACTIM_CTRLB(NUMONYX_TWTR_165, NUMONYX_TCKE_165,	\
				NUMONYX_TXP_165, NUMONYX_XSR_165)

#define NUMONYX_RASWIDTH_165		15
#define NUMONYX_V_MCFG_165(size)	MCFG((size), NUMONYX_RASWIDTH_165)

/* NUMONYX part of IGEP v2 (200MHz optimized) 5 ns */
#define NUMONYX_TDAL_200	6	/* Twr/Tck + Trp/tck		*/
					/* 15/5 + 15/5 = 3 + 3 -> 6	*/
#define NUMONYX_TDPL_200	3	/* 15/5 = 3 -> 3 (Twr)	        */
#define NUMONYX_TRRD_200	2	/* 10/5 = 2			*/
#define NUMONYX_TRCD_200	4	/* 16.2/5 = 3.24 -> 4		*/
#define NUMONYX_TRP_200		3	/* 15/5 = 3			*/
#define NUMONYX_TRAS_200	8	/* 40/5 = 8			*/
#define NUMONYX_TRC_200		11	/* 55/5 = 11			*/
#define NUMONYX_TRFC_200        28      /* 140/5 = 28                   */

#define NUMONYX_V_ACTIMA_200	\
		ACTIM_CTRLA(NUMONYX_TRFC_200, NUMONYX_TRC_200,		\
				NUMONYX_TRAS_200, NUMONYX_TRP_200,	\
				NUMONYX_TRCD_200, NUMONYX_TRRD_200,	\
				NUMONYX_TDPL_200, NUMONYX_TDAL_200)

#define NUMONYX_TWTR_200	2
#define NUMONYX_TCKE_200	2
#define NUMONYX_TXP_200		3
#define NUMONYX_XSR_200		40

#define NUMONYX_V_ACTIMB_200	\
		ACTIM_CTRLB(NUMONYX_TWTR_200, NUMONYX_TCKE_200,	\
				NUMONYX_TXP_200, NUMONYX_XSR_200)

#define NUMONYX_RASWIDTH_200		15
#define NUMONYX_V_MCFG_200(size)	MCFG((size), NUMONYX_RASWIDTH_200)

/*
 * GPMC settings -
 * Definitions is as per the following format
 * #define <PART>_GPMC_CONFIG<x> <value>
 * Where:
 * PART is the part name e.g. STNOR - Intel Strata Flash
 * x is GPMC config registers from 1 to 6 (there will be 6 macros)
 * Value is corresponding value
 *
 * For every valid PRCM configuration there should be only one definition of
 * the same. if values are independent of the board, this definition will be
 * present in this file if values are dependent on the board, then this should
 * go into corresponding mem-boardName.h file
 *
 * Currently valid part Names are (PART):
 * STNOR - Intel Strata Flash
 * SMNAND - Samsung NAND
 * MPDB - H4 MPDB board
 * SBNOR - Sibley NOR
 * MNAND - Micron Large page x16 NAND
 * ONNAND - Samsung One NAND
 *
 * include/configs/file.h contains the defn - for all CS we are interested
 * #define OMAP34XX_GPMC_CSx PART
 * #define OMAP34XX_GPMC_CSx_SIZE Size
 * #define OMAP34XX_GPMC_CSx_MAP Map
 * Where:
 * x - CS number
 * PART - Part Name as defined above
 * SIZE - how big is the mapping to be
 *   GPMC_SIZE_128M - 0x8
 *   GPMC_SIZE_64M  - 0xC
 *   GPMC_SIZE_32M  - 0xE
 *   GPMC_SIZE_16M  - 0xF
 * MAP  - Map this CS to which address(GPMC address space)- Absolute address
 *   >>24 before being used.
 */
#define GPMC_SIZE_256M	0x0
#define GPMC_SIZE_128M	0x8
#define GPMC_SIZE_64M	0xC
#define GPMC_SIZE_32M	0xE
#define GPMC_SIZE_16M	0xF

#define GPMC_BASEADDR_MASK	0x3F

#define GPMC_CS_ENABLE		0x1

#define M_NAND_GPMC_CONFIG1	0x00001800
#define M_NAND_GPMC_CONFIG2	0x00141400
#define M_NAND_GPMC_CONFIG3	0x00141400
#define M_NAND_GPMC_CONFIG4	0x0F010F01
#define M_NAND_GPMC_CONFIG5	0x010C1414
#define M_NAND_GPMC_CONFIG6	0x1f0f0A80
#define M_NAND_GPMC_CONFIG7	0x00000C44

#define STNOR_GPMC_CONFIG1	0x3
#define STNOR_GPMC_CONFIG2	0x00151501
#define STNOR_GPMC_CONFIG3	0x00060602
#define STNOR_GPMC_CONFIG4	0x11091109
#define STNOR_GPMC_CONFIG5	0x01141F1F
#define STNOR_GPMC_CONFIG6	0x000004c4

#define SIBNOR_GPMC_CONFIG1	0x1200
#define SIBNOR_GPMC_CONFIG2	0x001f1f00
#define SIBNOR_GPMC_CONFIG3	0x00080802
#define SIBNOR_GPMC_CONFIG4	0x1C091C09
#define SIBNOR_GPMC_CONFIG5	0x01131F1F
#define SIBNOR_GPMC_CONFIG6	0x1F0F03C2

#define SDPV2_MPDB_GPMC_CONFIG1	0x00611200
#define SDPV2_MPDB_GPMC_CONFIG2	0x001F1F01
#define SDPV2_MPDB_GPMC_CONFIG3	0x00080803
#define SDPV2_MPDB_GPMC_CONFIG4	0x1D091D09
#define SDPV2_MPDB_GPMC_CONFIG5	0x041D1F1F
#define SDPV2_MPDB_GPMC_CONFIG6	0x1D0904C4

#define MPDB_GPMC_CONFIG1	0x00011000
#define MPDB_GPMC_CONFIG2	0x001f1f01
#define MPDB_GPMC_CONFIG3	0x00080803
#define MPDB_GPMC_CONFIG4	0x1c0b1c0a
#define MPDB_GPMC_CONFIG5	0x041f1F1F
#define MPDB_GPMC_CONFIG6	0x1F0F04C4

#define P2_GPMC_CONFIG1	0x0
#define P2_GPMC_CONFIG2	0x0
#define P2_GPMC_CONFIG3	0x0
#define P2_GPMC_CONFIG4	0x0
#define P2_GPMC_CONFIG5	0x0
#define P2_GPMC_CONFIG6	0x0

#define ONENAND_GPMC_CONFIG1	0x00001200
#define ONENAND_GPMC_CONFIG2	0x000F0F01
#define ONENAND_GPMC_CONFIG3	0x00030301
#define ONENAND_GPMC_CONFIG4	0x0F040F04
#define ONENAND_GPMC_CONFIG5	0x010F1010
#define ONENAND_GPMC_CONFIG6	0x1F060000

#define NET_GPMC_CONFIG1	0x00001000
#define NET_GPMC_CONFIG2	0x001e1e01
#define NET_GPMC_CONFIG3	0x00080300
#define NET_GPMC_CONFIG4	0x1c091c09
#define NET_GPMC_CONFIG5	0x04181f1f
#define NET_GPMC_CONFIG6	0x00000FCF
#define NET_GPMC_CONFIG7	0x00000f6c

/* GPMC CS configuration for an SMSC LAN9221 ethernet controller */
#define NET_LAN9221_GPMC_CONFIG1    0x00001000
#define NET_LAN9221_GPMC_CONFIG2    0x00060700
#define NET_LAN9221_GPMC_CONFIG3    0x00020201
#define NET_LAN9221_GPMC_CONFIG4    0x06000700
#define NET_LAN9221_GPMC_CONFIG5    0x0006090A
#define NET_LAN9221_GPMC_CONFIG6    0x87030000
#define NET_LAN9221_GPMC_CONFIG7    0x00000f6c


/* max number of GPMC Chip Selects */
#define GPMC_MAX_CS	8
/* max number of GPMC regs */
#define GPMC_MAX_REG	7

#define DBG_MPDB	6
#define DBG_MPDB_BASE		DEBUG_BASE

#ifndef __ASSEMBLY__

/* Function prototypes */
void mem_init(void);

u32 is_mem_sdr(void);
u32 mem_ok(u32 cs);

u32 get_sdr_cs_size(u32);
u32 get_sdr_cs_offset(u32);

#endif	/* __ASSEMBLY__ */

#endif /* endif _MEM_H_ */
