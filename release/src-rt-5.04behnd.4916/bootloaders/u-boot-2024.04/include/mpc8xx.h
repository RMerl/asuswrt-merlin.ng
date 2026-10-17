/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * mpc8xx.h
 *
 * MPC8xx specific definitions
 */

#ifndef __MPCXX_H__
#define __MPCXX_H__


/*-----------------------------------------------------------------------
 * Exception offsets (PowerPC standard)
 */
#define EXC_OFF_SYS_RESET	0x0100	/* System reset				*/
#define _START_OFFSET		EXC_OFF_SYS_RESET

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control Register				11-9
 */
#define SYPCR_SWTC	0xFFFF0000	/* Software Watchdog Timer Count	*/
#define SYPCR_BMT	0x0000FF00	/* Bus Monitor Timing			*/
#define SYPCR_BME	0x00000080	/* Bus Monitor Enable			*/
#define SYPCR_SWF	0x00000008	/* Software Watchdog Freeze		*/
#define SYPCR_SWE	0x00000004	/* Software Watchdog Enable		*/
#define SYPCR_SWRI	0x00000002	/* Software Watchdog Reset/Int Select	*/
#define SYPCR_SWP	0x00000001	/* Software Watchdog Prescale		*/

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration Register				11-6
 */
#define SIUMCR_EARB	0x80000000	/* External Arbitration			*/
#define SIUMCR_EARP0	0x00000000	/* External Arbi. Request priority 0	*/
#define SIUMCR_EARP1	0x10000000	/* External Arbi. Request priority 1	*/
#define SIUMCR_EARP2	0x20000000	/* External Arbi. Request priority 2	*/
#define SIUMCR_EARP3	0x30000000	/* External Arbi. Request priority 3	*/
#define SIUMCR_EARP4	0x40000000	/* External Arbi. Request priority 4	*/
#define SIUMCR_EARP5	0x50000000	/* External Arbi. Request priority 5	*/
#define SIUMCR_EARP6	0x60000000	/* External Arbi. Request priority 6	*/
#define SIUMCR_EARP7	0x70000000	/* External Arbi. Request priority 7	*/
#define SIUMCR_DSHW	0x00800000	/* Data Showcycles			*/
#define SIUMCR_DBGC00	0x00000000	/* Debug pins configuration		*/
#define SIUMCR_DBGC01	0x00200000	/* - " -				*/
#define SIUMCR_DBGC10	0x00400000	/* - " -				*/
#define SIUMCR_DBGC11	0x00600000	/* - " -				*/
#define SIUMCR_DBPC00	0x00000000	/* Debug Port pins Config.		*/
#define SIUMCR_DBPC01	0x00080000	/* - " -				*/
#define SIUMCR_DBPC10	0x00100000	/* - " -				*/
#define SIUMCR_DBPC11	0x00180000	/* - " -				*/
#define SIUMCR_FRC	0x00020000	/* FRZ pin Configuration		*/
#define SIUMCR_DLK	0x00010000	/* Debug Register Lock			*/
#define SIUMCR_PNCS	0x00008000	/* Parity Non-mem Crtl reg		*/
#define SIUMCR_OPAR	0x00004000	/* Odd Parity				*/
#define SIUMCR_DPC	0x00002000	/* Data Parity pins Config.		*/
#define SIUMCR_MPRE	0x00001000	/* Multi CPU Reserva. Enable		*/
#define SIUMCR_MLRC00	0x00000000	/* Multi Level Reserva. Ctrl		*/
#define SIUMCR_MLRC01	0x00000400	/* - " -				*/
#define SIUMCR_MLRC10	0x00000800	/* - " -				*/
#define SIUMCR_MLRC11	0x00000C00	/* - " -				*/
#define SIUMCR_AEME	0x00000200	/* Asynchro External Master		*/
#define SIUMCR_SEME	0x00000100	/* Synchro External Master		*/
#define SIUMCR_BSC	0x00000080	/* Byte Select Configuration		*/
#define SIUMCR_GB5E	0x00000040	/* GPL_B(5) Enable			*/
#define SIUMCR_B2DD	0x00000020	/* Bank 2 Double Drive			*/
#define SIUMCR_B3DD	0x00000010	/* Bank 3 Double Drive			*/

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control Register			11-26
 */
#define TBSCR_TBIRQ7	0x8000		/* Time Base Interrupt Request 7	*/
#define TBSCR_TBIRQ6	0x4000		/* Time Base Interrupt Request 6	*/
#define TBSCR_TBIRQ5	0x2000		/* Time Base Interrupt Request 5	*/
#define TBSCR_TBIRQ4	0x1000		/* Time Base Interrupt Request 4	*/
#define TBSCR_TBIRQ3	0x0800		/* Time Base Interrupt Request 3	*/
#define TBSCR_TBIRQ2	0x0400		/* Time Base Interrupt Request 2	*/
#define TBSCR_TBIRQ1	0x0200		/* Time Base Interrupt Request 1	*/
#define TBSCR_TBIRQ0	0x0100		/* Time Base Interrupt Request 0	*/
#if 0	/* already in asm/immap_8xx.h */
#define TBSCR_REFA	0x0080		/* Reference Interrupt Status A		*/
#define TBSCR_REFB	0x0040		/* Reference Interrupt Status B		*/
#define TBSCR_REFAE	0x0008		/* Second Interrupt Enable A		*/
#define TBSCR_REFBE	0x0004		/* Second Interrupt Enable B		*/
#define TBSCR_TBF	0x0002		/* Time Base Freeze			*/
#define TBSCR_TBE	0x0001		/* Time Base Enable			*/
#endif

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control Register		11-31
 */
#undef	PISCR_PIRQ			/* TBD					*/
#define PISCR_PITF	0x0002		/* Periodic Interrupt Timer Freeze	*/
#if 0	/* already in asm/immap_8xx.h */
#define PISCR_PS	0x0080		/* Periodic interrupt Status		*/
#define PISCR_PIE	0x0004		/* Periodic Interrupt Enable		*/
#define PISCR_PTE	0x0001		/* Periodic Timer Enable		*/
#endif

/*-----------------------------------------------------------------------
 * RSR - Reset Status Register						 5-4
 */
#define RSR_JTRS	0x01000000	/* JTAG Reset Status		*/
#define RSR_DBSRS	0x02000000	/* Debug Port Soft Reset Status */
#define RSR_DBHRS	0x04000000	/* Debug Port Hard Reset Status */
#define RSR_CSRS	0x08000000	/* Check Stop Reset Status	*/
#define RSR_SWRS	0x10000000	/* Software Watchdog Reset Status*/
#define RSR_LLRS	0x20000000	/* Loss-of-Lock Reset Status	*/
#define RSR_ESRS	0x40000000	/* External Soft Reset Status	*/
#define RSR_EHRS	0x80000000	/* External Hard Reset Status	*/

#define RSR_ALLBITS	(RSR_JTRS|RSR_DBSRS|RSR_DBHRS|RSR_CSRS|RSR_SWRS|RSR_LLRS|RSR_ESRS|RSR_EHRS)

/*-----------------------------------------------------------------------
 * Newer chips (MPC866 family and MPC87x/88x family) have different
 * clock distribution system. Their IMMR lower half is >= 0x0800
 */
#define MPC8xx_NEW_CLK 0x0800

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register			15-30
 */
/* Newer chips (MPC866/87x/88x et al) defines */
#define PLPRCR_MFN_MSK	0xF8000000	/* Multiplication factor numerator bits */
#define PLPRCR_MFN_SHIFT	27	/* Multiplication factor numerator shift*/
#define PLPRCR_MFD_MSK	0x07C00000	/* Multiplication factor denominator bits */
#define PLPRCR_MFD_SHIFT	22	/* Multiplication factor denominator shift*/
#define PLPRCR_S_MSK	0x00300000	/* Multiplication factor integer bits	*/
#define PLPRCR_S_SHIFT		20	/* Multiplication factor integer shift	*/
#define PLPRCR_MFI_MSK	0x000F0000	/* Multiplication factor integer bits	*/
#define PLPRCR_MFI_SHIFT	16	/* Multiplication factor integer shift	*/

#define PLPRCR_PDF_MSK	0x0000001E	/* Predivision Factor bits		*/
#define PLPRCR_PDF_SHIFT	 1	/* Predivision Factor shift value	*/
#define PLPRCR_DBRMO	0x00000001	/* DPLL BRM Order bit			*/

/* Multiplication factor + PDF bits */
#define PLPRCR_MFACT_MSK (PLPRCR_MFN_MSK | \
			  PLPRCR_MFD_MSK | \
			  PLPRCR_S_MSK	 | \
			  PLPRCR_MFI_MSK | \
			  PLPRCR_PDF_MSK)

/* Common defines */
#define PLPRCR_TEXPS	0x00004000	/* TEXP Status				*/
#define PLPRCR_CSRC	0x00000400	/* Clock Source				*/

#define PLPRCR_CSR	0x00000080	/* CheskStop Reset value		*/
#define PLPRCR_LOLRE	0x00000040	/* Loss Of Lock Reset Enable		*/
#define PLPRCR_FIOPD	0x00000020	/* Force I/O Pull Down			*/

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register			15-27
 */
#define SCCR_COM00	0x00000000	/* full strength CLKOUT output buffer	*/
#define SCCR_COM01	0x20000000	/* half strength CLKOUT output buffer	*/
#define SCCR_COM10	0x40000000	/* reserved				*/
#define SCCR_COM11	0x60000000	/* CLKOUT output buffer disabled	*/
#define SCCR_TBS	0x02000000	/* Time Base Source			*/
#define SCCR_RTDIV	0x01000000	/* RTC Clock Divide			*/
#define SCCR_RTSEL	0x00800000	/* RTC circuit input source select	*/
#define SCCR_CRQEN	0x00400000	/* CPM Request Enable			*/
#define SCCR_PRQEN	0x00200000	/* Power Management Request Enable	*/
#define SCCR_EBDF00	0x00000000	/* CLKOUT is GCLK2 / 1 (normal op.)	*/
#define SCCR_EBDF01	0x00020000	/* CLKOUT is GCLK2 / 2			*/
#define SCCR_EBDF10	0x00040000	/* reserved				*/
#define SCCR_EBDF11	0x00060000	/* reserved				*/
#define SCCR_DFSYNC00	0x00000000	/* SyncCLK division by 1 (normal op.)	*/
#define SCCR_DFSYNC01	0x00002000	/* SyncCLK division by 4		*/
#define SCCR_DFSYNC10	0x00004000	/* SyncCLK division by 16		*/
#define SCCR_DFSYNC11	0x00006000	/* SyncCLK division by 64		*/
#define SCCR_DFBRG00	0x00000000	/* BRGCLK division by 1 (normal op.)	*/
#define SCCR_DFBRG01	0x00000800	/* BRGCLK division by 4			*/
#define SCCR_DFBRG10	0x00001000	/* BRGCLK division by 16		*/
#define SCCR_DFBRG11	0x00001800	/* BRGCLK division by 64		*/
#define SCCR_DFNL000	0x00000000	/* Division by 2 (default = minimum)	*/
#define SCCR_DFNL001	0x00000100	/* Division by 4	                */
#define SCCR_DFNL010	0x00000200	/* Division by 8	                */
#define SCCR_DFNL011	0x00000300	/* Division by 16	                */
#define SCCR_DFNL100	0x00000400	/* Division by 32	                */
#define SCCR_DFNL101	0x00000500	/* Division by 64	                */
#define SCCR_DFNL110	0x00000600	/* Division by 128	                */
#define SCCR_DFNL111	0x00000700	/* Division by 256 (maximum)		*/
#define SCCR_DFNH000	0x00000000	/* Division by 1 (default = minimum)	*/
#define SCCR_DFNH110	0x000000D0	/* Division by 64 (maximum)		*/
#define SCCR_DFNH111	0x000000E0	/* reserved				*/
#define SCCR_DFLCD000	0x00000000	/* Division by 1 (default = minimum)	*/
#define SCCR_DFLCD001	0x00000004	/* Division by 2			*/
#define SCCR_DFLCD010	0x00000008	/* Division by 4			*/
#define SCCR_DFLCD011	0x0000000C	/* Division by 8			*/
#define SCCR_DFLCD100	0x00000010	/* Division by 16			*/
#define SCCR_DFLCD101	0x00000014	/* Division by 32			*/
#define SCCR_DFLCD110	0x00000018	/* Division by 64 (maximum)		*/
#define SCCR_DFLCD111	0x0000001C	/* reserved				*/
#define SCCR_DFALCD00	0x00000000	/* Division by 1 (default = minimum)	*/
#define SCCR_DFALCD01	0x00000001	/* Division by 3			*/
#define SCCR_DFALCD10	0x00000002	/* Division by 5			*/
#define SCCR_DFALCD11	0x00000003	/* Division by 7 (maximum)		*/


/*-----------------------------------------------------------------------
 * BR - Memory Controler: Base Register					16-9
 */
#define BR_BA_MSK	0xFFFF8000	/* Base Address Mask			*/
#define BR_AT_MSK	0x00007000	/* Address Type Mask			*/
#define BR_PS_MSK	0x00000C00	/* Port Size Mask			*/
#define BR_PS_32	0x00000000	/* 32 bit port size			*/
#define BR_PS_16	0x00000800	/* 16 bit port size			*/
#define BR_PS_8		0x00000400	/*  8 bit port size			*/
#define BR_PARE		0x00000200	/* Parity Enable			*/
#define BR_WP		0x00000100	/* Write Protect			*/
#define BR_MS_MSK	0x000000C0	/* Machine Select Mask			*/
#define BR_MS_GPCM	0x00000000	/* G.P.C.M. Machine Select		*/
#define BR_MS_UPMA	0x00000080	/* U.P.M.A Machine Select		*/
#define BR_MS_UPMB	0x000000C0	/* U.P.M.B Machine Select		*/
#define BR_V		0x00000001	/* Bank Valid				*/

/*-----------------------------------------------------------------------
 * OR - Memory Controler: Option Register				16-11
 */
#define OR_AM_MSK	0xFFFF8000	/* Address Mask Mask			*/
#define OR_ATM_MSK	0x00007000	/* Address Type Mask Mask		*/
#define OR_CSNT_SAM	0x00000800	/* Chip Select Negation Time/ Start	*/
					/* Address Multiplex			*/
#define OR_ACS_MSK	0x00000600	/* Address to Chip Select Setup mask	*/
#define OR_ACS_DIV1	0x00000000	/* CS is output at the same time	*/
#define OR_ACS_DIV4	0x00000400	/* CS is output 1/4 a clock later	*/
#define OR_ACS_DIV2	0x00000600	/* CS is output 1/2 a clock later	*/
#define OR_G5LA		0x00000400	/* Output #GPL5 on #GPL_A5		*/
#define OR_G5LS		0x00000200	/* Drive #GPL high on falling edge of...*/
#define OR_BI		0x00000100	/* Burst inhibit			*/
#define OR_SCY_MSK	0x000000F0	/* Cycle Lenght in Clocks		*/
#define OR_SCY_0_CLK	0x00000000	/* 0 clock cycles wait states		*/
#define OR_SCY_1_CLK	0x00000010	/* 1 clock cycles wait states		*/
#define OR_SCY_2_CLK	0x00000020	/* 2 clock cycles wait states		*/
#define OR_SCY_3_CLK	0x00000030	/* 3 clock cycles wait states		*/
#define OR_SCY_4_CLK	0x00000040	/* 4 clock cycles wait states		*/
#define OR_SCY_5_CLK	0x00000050	/* 5 clock cycles wait states		*/
#define OR_SCY_6_CLK	0x00000060	/* 6 clock cycles wait states		*/
#define OR_SCY_7_CLK	0x00000070	/* 7 clock cycles wait states		*/
#define OR_SCY_8_CLK	0x00000080	/* 8 clock cycles wait states		*/
#define OR_SCY_9_CLK	0x00000090	/* 9 clock cycles wait states		*/
#define OR_SCY_10_CLK	0x000000A0	/* 10 clock cycles wait states		*/
#define OR_SCY_11_CLK	0x000000B0	/* 11 clock cycles wait states		*/
#define OR_SCY_12_CLK	0x000000C0	/* 12 clock cycles wait states		*/
#define OR_SCY_13_CLK	0x000000D0	/* 13 clock cycles wait states		*/
#define OR_SCY_14_CLK	0x000000E0	/* 14 clock cycles wait states		*/
#define OR_SCY_15_CLK	0x000000F0	/* 15 clock cycles wait states		*/
#define OR_SETA		0x00000008	/* External Transfer Acknowledge	*/
#define OR_TRLX		0x00000004	/* Timing Relaxed			*/
#define OR_EHTR		0x00000002	/* Extended Hold Time on Read		*/


/*-----------------------------------------------------------------------
 * MPTPR - Memory Periodic Timer Prescaler Register			16-17
 */
#define MPTPR_PTP_MSK	0xFF00		/* Periodic Timers Prescaler Mask	*/
#define MPTPR_PTP_DIV2	0x2000		/* BRGCLK divided by 2			*/
#define MPTPR_PTP_DIV4	0x1000		/* BRGCLK divided by 4			*/
#define MPTPR_PTP_DIV8	0x0800		/* BRGCLK divided by 8			*/
#define MPTPR_PTP_DIV16 0x0400		/* BRGCLK divided by 16			*/
#define MPTPR_PTP_DIV32 0x0200		/* BRGCLK divided by 32			*/
#define MPTPR_PTP_DIV64 0x0100		/* BRGCLK divided by 64			*/

/*-----------------------------------------------------------------------
 * MCR - Memory Command Register
 */
#define MCR_OP_WRITE	0x00000000	/* WRITE command			*/
#define MCR_OP_READ	0x40000000	/* READ	 command			*/
#define MCR_OP_RUN	0x80000000	/* RUN	 command			*/
#define MCR_UPM_A	0x00000000	/* Select UPM A				*/
#define MCR_UPM_B	0x00800000	/* Select UPM B				*/
#define MCR_MB_CS0	0x00000000	/* Use Chip Select /CS0			*/
#define MCR_MB_CS1	0x00002000	/* Use Chip Select /CS1			*/
#define MCR_MB_CS2	0x00004000	/* Use Chip Select /CS2			*/
#define MCR_MB_CS3	0x00006000	/* Use Chip Select /CS3			*/
#define MCR_MB_CS4	0x00008000	/* Use Chip Select /CS4			*/
#define MCR_MB_CS5	0x0000A000	/* Use Chip Select /CS5			*/
#define MCR_MB_CS6	0x0000C000	/* Use Chip Select /CS6			*/
#define MCR_MB_CS7	0x0000E000	/* Use Chip Select /CS7			*/
#define MCR_MLCF(n)	(((n)&0xF)<<8)	/* Memory Command Loop Count Field	*/
#define MCR_MAD(addr)	((addr)&0x3F)	/* Memory Array Index			*/

/*-----------------------------------------------------------------------
 * Machine A Mode Register						16-13
 */
#define MAMR_PTA_MSK	0xFF000000	/* Periodic Timer A period mask		*/
#define MAMR_PTA_SHIFT	0x00000018	/* Periodic Timer A period shift	*/
#define MAMR_PTAE	0x00800000	/* Periodic Timer A Enable		*/
#define MAMR_AMA_MSK	0x00700000	/* Addess Multiplexing size A		*/
#define MAMR_AMA_TYPE_0 0x00000000	/* Addess Multiplexing Type 0		*/
#define MAMR_AMA_TYPE_1 0x00100000	/* Addess Multiplexing Type 1		*/
#define MAMR_AMA_TYPE_2 0x00200000	/* Addess Multiplexing Type 2		*/
#define MAMR_AMA_TYPE_3 0x00300000	/* Addess Multiplexing Type 3		*/
#define MAMR_AMA_TYPE_4 0x00400000	/* Addess Multiplexing Type 4		*/
#define MAMR_AMA_TYPE_5 0x00500000	/* Addess Multiplexing Type 5		*/
#define MAMR_DSA_MSK	0x00060000	/* Disable Timer period mask		*/
#define MAMR_DSA_1_CYCL 0x00000000	/* 1 cycle Disable Period		*/
#define MAMR_DSA_2_CYCL 0x00020000	/* 2 cycle Disable Period		*/
#define MAMR_DSA_3_CYCL 0x00040000	/* 3 cycle Disable Period		*/
#define MAMR_DSA_4_CYCL 0x00060000	/* 4 cycle Disable Period		*/
#define MAMR_G0CLA_MSK	0x0000E000	/* General Line 0 Control A		*/
#define MAMR_G0CLA_A12	0x00000000	/* General Line 0 : A12			*/
#define MAMR_G0CLA_A11	0x00002000	/* General Line 0 : A11			*/
#define MAMR_G0CLA_A10	0x00004000	/* General Line 0 : A10			*/
#define MAMR_G0CLA_A9	0x00006000	/* General Line 0 : A9			*/
#define MAMR_G0CLA_A8	0x00008000	/* General Line 0 : A8			*/
#define MAMR_G0CLA_A7	0x0000A000	/* General Line 0 : A7			*/
#define MAMR_G0CLA_A6	0x0000C000	/* General Line 0 : A6			*/
#define MAMR_G0CLA_A5	0x0000E000	/* General Line 0 : A5			*/
#define MAMR_GPL_A4DIS	0x00001000	/* GPL_A4 ouput line Disable		*/
#define MAMR_RLFA_MSK	0x00000F00	/* Read Loop Field A mask		*/
#define MAMR_RLFA_1X	0x00000100	/* The Read Loop is executed 1 time	*/
#define MAMR_RLFA_2X	0x00000200	/* The Read Loop is executed 2 times	*/
#define MAMR_RLFA_3X	0x00000300	/* The Read Loop is executed 3 times	*/
#define MAMR_RLFA_4X	0x00000400	/* The Read Loop is executed 4 times	*/
#define MAMR_RLFA_5X	0x00000500	/* The Read Loop is executed 5 times	*/
#define MAMR_RLFA_6X	0x00000600	/* The Read Loop is executed 6 times	*/
#define MAMR_RLFA_7X	0x00000700	/* The Read Loop is executed 7 times	*/
#define MAMR_RLFA_8X	0x00000800	/* The Read Loop is executed 8 times	*/
#define MAMR_RLFA_9X	0x00000900	/* The Read Loop is executed 9 times	*/
#define MAMR_RLFA_10X	0x00000A00	/* The Read Loop is executed 10 times	*/
#define MAMR_RLFA_11X	0x00000B00	/* The Read Loop is executed 11 times	*/
#define MAMR_RLFA_12X	0x00000C00	/* The Read Loop is executed 12 times	*/
#define MAMR_RLFA_13X	0x00000D00	/* The Read Loop is executed 13 times	*/
#define MAMR_RLFA_14X	0x00000E00	/* The Read Loop is executed 14 times	*/
#define MAMR_RLFA_15X	0x00000F00	/* The Read Loop is executed 15 times	*/
#define MAMR_RLFA_16X	0x00000000	/* The Read Loop is executed 16 times	*/
#define MAMR_WLFA_MSK	0x000000F0	/* Write Loop Field A mask		*/
#define MAMR_WLFA_1X	0x00000010	/* The Write Loop is executed 1 time	*/
#define MAMR_WLFA_2X	0x00000020	/* The Write Loop is executed 2 times	*/
#define MAMR_WLFA_3X	0x00000030	/* The Write Loop is executed 3 times	*/
#define MAMR_WLFA_4X	0x00000040	/* The Write Loop is executed 4 times	*/
#define MAMR_WLFA_5X	0x00000050	/* The Write Loop is executed 5 times	*/
#define MAMR_WLFA_6X	0x00000060	/* The Write Loop is executed 6 times	*/
#define MAMR_WLFA_7X	0x00000070	/* The Write Loop is executed 7 times	*/
#define MAMR_WLFA_8X	0x00000080	/* The Write Loop is executed 8 times	*/
#define MAMR_WLFA_9X	0x00000090	/* The Write Loop is executed 9 times	*/
#define MAMR_WLFA_10X	0x000000A0	/* The Write Loop is executed 10 times	*/
#define MAMR_WLFA_11X	0x000000B0	/* The Write Loop is executed 11 times	*/
#define MAMR_WLFA_12X	0x000000C0	/* The Write Loop is executed 12 times	*/
#define MAMR_WLFA_13X	0x000000D0	/* The Write Loop is executed 13 times	*/
#define MAMR_WLFA_14X	0x000000E0	/* The Write Loop is executed 14 times	*/
#define MAMR_WLFA_15X	0x000000F0	/* The Write Loop is executed 15 times	*/
#define MAMR_WLFA_16X	0x00000000	/* The Write Loop is executed 16 times	*/
#define MAMR_TLFA_MSK	0x0000000F	/* Timer Loop Field A mask		*/
#define MAMR_TLFA_1X	0x00000001	/* The Timer Loop is executed 1 time	*/
#define MAMR_TLFA_2X	0x00000002	/* The Timer Loop is executed 2 times	*/
#define MAMR_TLFA_3X	0x00000003	/* The Timer Loop is executed 3 times	*/
#define MAMR_TLFA_4X	0x00000004	/* The Timer Loop is executed 4 times	*/
#define MAMR_TLFA_5X	0x00000005	/* The Timer Loop is executed 5 times	*/
#define MAMR_TLFA_6X	0x00000006	/* The Timer Loop is executed 6 times	*/
#define MAMR_TLFA_7X	0x00000007	/* The Timer Loop is executed 7 times	*/
#define MAMR_TLFA_8X	0x00000008	/* The Timer Loop is executed 8 times	*/
#define MAMR_TLFA_9X	0x00000009	/* The Timer Loop is executed 9 times	*/
#define MAMR_TLFA_10X	0x0000000A	/* The Timer Loop is executed 10 times	*/
#define MAMR_TLFA_11X	0x0000000B	/* The Timer Loop is executed 11 times	*/
#define MAMR_TLFA_12X	0x0000000C	/* The Timer Loop is executed 12 times	*/
#define MAMR_TLFA_13X	0x0000000D	/* The Timer Loop is executed 13 times	*/
#define MAMR_TLFA_14X	0x0000000E	/* The Timer Loop is executed 14 times	*/
#define MAMR_TLFA_15X	0x0000000F	/* The Timer Loop is executed 15 times	*/
#define MAMR_TLFA_16X	0x00000000	/* The Timer Loop is executed 16 times	*/

/*-----------------------------------------------------------------------
 * Machine B Mode Register						16-13
 */
#define MBMR_PTB_MSK	0xFF000000	/* Periodic Timer B period mask		*/
#define MBMR_PTB_SHIFT	0x00000018	/* Periodic Timer B period shift	*/
#define MBMR_PTBE	0x00800000	/* Periodic Timer B Enable		*/
#define MBMR_AMB_MSK	0x00700000	/* Addess Multiplex size B		*/
#define MBMR_AMB_TYPE_0 0x00000000	/* Addess Multiplexing Type 0		*/
#define MBMR_AMB_TYPE_1 0x00100000	/* Addess Multiplexing Type 1		*/
#define MBMR_AMB_TYPE_2 0x00200000	/* Addess Multiplexing Type 2		*/
#define MBMR_AMB_TYPE_3 0x00300000	/* Addess Multiplexing Type 3		*/
#define MBMR_AMB_TYPE_4 0x00400000	/* Addess Multiplexing Type 4		*/
#define MBMR_AMB_TYPE_5 0x00500000	/* Addess Multiplexing Type 5		*/
#define MBMR_DSB_MSK	0x00060000	/* Disable Timer period mask		*/
#define MBMR_DSB_1_CYCL 0x00000000	/* 1 cycle Disable Period		*/
#define MBMR_DSB_2_CYCL 0x00020000	/* 2 cycle Disable Period		*/
#define MBMR_DSB_3_CYCL 0x00040000	/* 3 cycle Disable Period		*/
#define MBMR_DSB_4_CYCL 0x00060000	/* 4 cycle Disable Period		*/
#define MBMR_G0CLB_MSK	0x0000E000	/* General Line 0 Control B		*/
#define MBMR_G0CLB_A12	0x00000000	/* General Line 0 : A12			*/
#define MBMR_G0CLB_A11	0x00002000	/* General Line 0 : A11			*/
#define MBMR_G0CLB_A10	0x00004000	/* General Line 0 : A10			*/
#define MBMR_G0CLB_A9	0x00006000	/* General Line 0 : A9			*/
#define MBMR_G0CLB_A8	0x00008000	/* General Line 0 : A8			*/
#define MBMR_G0CLB_A7	0x0000A000	/* General Line 0 : A7			*/
#define MBMR_G0CLB_A6	0x0000C000	/* General Line 0 : A6			*/
#define MBMR_G0CLB_A5	0x0000E000	/* General Line 0 : A5			*/
#define MBMR_GPL_B4DIS	0x00001000	/* GPL_B4 ouput line Disable		*/
#define MBMR_RLFB_MSK	0x00000F00	/* Read Loop Field B mask		*/
#define MBMR_RLFB_1X	0x00000100	/* The Read Loop is executed 1 time	*/
#define MBMR_RLFB_2X	0x00000200	/* The Read Loop is executed 2 times	*/
#define MBMR_RLFB_3X	0x00000300	/* The Read Loop is executed 3 times	*/
#define MBMR_RLFB_4X	0x00000400	/* The Read Loop is executed 4 times	*/
#define MBMR_RLFB_5X	0x00000500	/* The Read Loop is executed 5 times	*/
#define MBMR_RLFB_6X	0x00000600	/* The Read Loop is executed 6 times	*/
#define MBMR_RLFB_7X	0x00000700	/* The Read Loop is executed 7 times	*/
#define MBMR_RLFB_8X	0x00000800	/* The Read Loop is executed 8 times	*/
#define MBMR_RLFB_9X	0x00000900	/* The Read Loop is executed 9 times	*/
#define MBMR_RLFB_10X	0x00000A00	/* The Read Loop is executed 10 times	*/
#define MBMR_RLFB_11X	0x00000B00	/* The Read Loop is executed 11 times	*/
#define MBMR_RLFB_12X	0x00000C00	/* The Read Loop is executed 12 times	*/
#define MBMR_RLFB_13X	0x00000D00	/* The Read Loop is executed 13 times	*/
#define MBMR_RLFB_14X	0x00000E00	/* The Read Loop is executed 14 times	*/
#define MBMR_RLFB_15X	0x00000f00	/* The Read Loop is executed 15 times	*/
#define MBMR_RLFB_16X	0x00000000	/* The Read Loop is executed 16 times	*/
#define MBMR_WLFB_MSK	0x000000F0	/* Write Loop Field B mask		*/
#define MBMR_WLFB_1X	0x00000010	/* The Write Loop is executed 1 time	*/
#define MBMR_WLFB_2X	0x00000020	/* The Write Loop is executed 2 times	*/
#define MBMR_WLFB_3X	0x00000030	/* The Write Loop is executed 3 times	*/
#define MBMR_WLFB_4X	0x00000040	/* The Write Loop is executed 4 times	*/
#define MBMR_WLFB_5X	0x00000050	/* The Write Loop is executed 5 times	*/
#define MBMR_WLFB_6X	0x00000060	/* The Write Loop is executed 6 times	*/
#define MBMR_WLFB_7X	0x00000070	/* The Write Loop is executed 7 times	*/
#define MBMR_WLFB_8X	0x00000080	/* The Write Loop is executed 8 times	*/
#define MBMR_WLFB_9X	0x00000090	/* The Write Loop is executed 9 times	*/
#define MBMR_WLFB_10X	0x000000A0	/* The Write Loop is executed 10 times	*/
#define MBMR_WLFB_11X	0x000000B0	/* The Write Loop is executed 11 times	*/
#define MBMR_WLFB_12X	0x000000C0	/* The Write Loop is executed 12 times	*/
#define MBMR_WLFB_13X	0x000000D0	/* The Write Loop is executed 13 times	*/
#define MBMR_WLFB_14X	0x000000E0	/* The Write Loop is executed 14 times	*/
#define MBMR_WLFB_15X	0x000000F0	/* The Write Loop is executed 15 times	*/
#define MBMR_WLFB_16X	0x00000000	/* The Write Loop is executed 16 times	*/
#define MBMR_TLFB_MSK	0x0000000F	/* Timer Loop Field B mask		*/
#define MBMR_TLFB_1X	0x00000001	/* The Timer Loop is executed 1 time	*/
#define MBMR_TLFB_2X	0x00000002	/* The Timer Loop is executed 2 times	*/
#define MBMR_TLFB_3X	0x00000003	/* The Timer Loop is executed 3 times	*/
#define MBMR_TLFB_4X	0x00000004	/* The Timer Loop is executed 4 times	*/
#define MBMR_TLFB_5X	0x00000005	/* The Timer Loop is executed 5 times	*/
#define MBMR_TLFB_6X	0x00000006	/* The Timer Loop is executed 6 times	*/
#define MBMR_TLFB_7X	0x00000007	/* The Timer Loop is executed 7 times	*/
#define MBMR_TLFB_8X	0x00000008	/* The Timer Loop is executed 8 times	*/
#define MBMR_TLFB_9X	0x00000009	/* The Timer Loop is executed 9 times	*/
#define MBMR_TLFB_10X	0x0000000A	/* The Timer Loop is executed 10 times	*/
#define MBMR_TLFB_11X	0x0000000B	/* The Timer Loop is executed 11 times	*/
#define MBMR_TLFB_12X	0x0000000C	/* The Timer Loop is executed 12 times	*/
#define MBMR_TLFB_13X	0x0000000D	/* The Timer Loop is executed 13 times	*/
#define MBMR_TLFB_14X	0x0000000E	/* The Timer Loop is executed 14 times	*/
#define MBMR_TLFB_15X	0x0000000F	/* The Timer Loop is executed 15 times	*/
#define MBMR_TLFB_16X	0x00000000	/* The Timer Loop is executed 16 times	*/

/*-----------------------------------------------------------------------
 * Timer Global Configuration Register					18-8
 */
#define TGCR_CAS4	0x8000		/* Cascade Timer 3 and 4	*/
#define TGCR_FRZ4	0x4000		/* Freeze timer 4		*/
#define TGCR_STP4	0x2000		/* Stop timer	4		*/
#define TGCR_RST4	0x1000		/* Reset timer	4		*/
#define TGCR_GM2	0x0800		/* Gate Mode for Pin 2		*/
#define TGCR_FRZ3	0x0400		/* Freeze timer 3		*/
#define TGCR_STP3	0x0200		/* Stop timer	3		*/
#define TGCR_RST3	0x0100		/* Reset timer	3		*/
#define TGCR_CAS2	0x0080		/* Cascade Timer 1 and 2	*/
#define TGCR_FRZ2	0x0040		/* Freeze timer 2		*/
#define TGCR_STP2	0x0020		/* Stop timer	2		*/
#define TGCR_RST2	0x0010		/* Reset timer	2		*/
#define TGCR_GM1	0x0008		/* Gate Mode for Pin 1		*/
#define TGCR_FRZ1	0x0004		/* Freeze timer 1		*/
#define TGCR_STP1	0x0002		/* Stop timer	1		*/
#define TGCR_RST1	0x0001		/* Reset timer	1		*/


/*-----------------------------------------------------------------------
 * Timer Mode Register							18-9
 */
#define TMR_PS_MSK		0xFF00	/* Prescaler Value			*/
#define TMR_PS_SHIFT		     8	/* Prescaler position			*/
#define TMR_CE_MSK		0x00C0	/* Capture Edge and Enable Interrupt	*/
#define TMR_CE_INTR_DIS		0x0000	/* Disable Interrupt on capture event	*/
#define TMR_CE_RISING		0x0040	/* Capture on Rising TINx edge only	*/
#define TMR_CE_FALLING		0x0080	/* Capture on Falling TINx edge only	*/
#define TMR_CE_ANY		0x00C0	/* Capture on any TINx edge		*/
#define TMR_OM			0x0020	/* Output Mode				*/
#define TMR_ORI			0x0010	/* Output Reference Interrupt Enable	*/
#define TMR_FRR			0x0008	/* Free Run/Restart			*/
#define TMR_ICLK_MSK		0x0006	/* Timer Input Clock Source mask	*/
#define TMR_ICLK_IN_CAS		0x0000	/* Internally cascaded input		*/
#define TMR_ICLK_IN_GEN		0x0002	/* Internal General system clock	*/
#define TMR_ICLK_IN_GEN_DIV16	0x0004	/* Internal General system clk div 16	*/
#define TMR_ICLK_TIN_PIN	0x0006	/* TINx pin				*/
#define TMR_GE			0x0001	/* Gate Enable				*/


/*-----------------------------------------------------------------------
 * I2C Controller Registers
 */
#define I2MOD_REVD		0x20	/* Reverese Data			*/
#define I2MOD_GCD		0x10	/* General Call Disable			*/
#define I2MOD_FLT		0x08	/* Clock Filter				*/
#define I2MOD_PDIV32		0x00	/* Pre-Divider 32			*/
#define I2MOD_PDIV16		0x02	/* Pre-Divider 16			*/
#define I2MOD_PDIV8		0x04	/* Pre-Divider	8			*/
#define I2MOD_PDIV4		0x06	/* Pre-Divider	4			*/
#define I2MOD_EN		0x01	/* Enable				*/

#define I2CER_TXE		0x10	/* Tx Error				*/
#define I2CER_BSY		0x04	/* Busy Condition			*/
#define I2CER_TXB		0x02	/* Tx Buffer Transmitted		*/
#define I2CER_RXB		0x01	/* Rx Buffer Received			*/
#define I2CER_ALL		(I2CER_TXE | I2CER_BSY | I2CER_TXB | I2CER_RXB)

#define I2COM_STR		0x80	/* Start Transmit			*/
#define I2COM_MASTER		0x01	/* Master mode				*/

/*-----------------------------------------------------------------------
 * SPI Controller Registers						31-10
 */
#define SPI_EMASK		0x37	/* Event Mask				*/
#define SPI_MME			0x20	/* Multi-Master Error			*/
#define SPI_TXE			0x10	/* Transmit Error			*/
#define SPI_BSY			0x04	/* Busy					*/
#define SPI_TXB			0x02	/* Tx Buffer Empty			*/
#define SPI_RXB			0x01	/* RX Buffer full/closed		*/

#define SPI_STR			0x80	/* SPCOM: Start transmit		*/

/*-----------------------------------------------------------------------
 * PCMCIA Interface General Control Register				17-12
 */
#define PCMCIA_GCRX_CXRESET	0x00000040
#define PCMCIA_GCRX_CXOE	0x00000080

#define PCMCIA_VS1(slot)	(0x80000000 >> (slot << 4))
#define PCMCIA_VS2(slot)	(0x40000000 >> (slot << 4))
#define PCMCIA_VS_MASK(slot)	(0xC0000000 >> (slot << 4))
#define PCMCIA_VS_SHIFT(slot)	(30 - (slot << 4))

#define PCMCIA_WP(slot)		(0x20000000 >> (slot << 4))
#define PCMCIA_CD2(slot)	(0x10000000 >> (slot << 4))
#define PCMCIA_CD1(slot)	(0x08000000 >> (slot << 4))
#define PCMCIA_BVD2(slot)	(0x04000000 >> (slot << 4))
#define PCMCIA_BVD1(slot)	(0x02000000 >> (slot << 4))
#define PCMCIA_RDY(slot)	(0x01000000 >> (slot << 4))
#define PCMCIA_RDY_L(slot)	(0x00800000 >> (slot << 4))
#define PCMCIA_RDY_H(slot)	(0x00400000 >> (slot << 4))
#define PCMCIA_RDY_R(slot)	(0x00200000 >> (slot << 4))
#define PCMCIA_RDY_F(slot)	(0x00100000 >> (slot << 4))
#define PCMCIA_MASK(slot)	(0xFFFF0000 >> (slot << 4))

/*-----------------------------------------------------------------------
 * PCMCIA Option Register Definitions
 *
 * Bank Sizes:
 */
#define PCMCIA_BSIZE_1		0x00000000	/* Bank size:	1 Bytes */
#define PCMCIA_BSIZE_2		0x08000000	/* Bank size:	2 Bytes */
#define PCMCIA_BSIZE_4		0x18000000	/* Bank size:	4 Bytes */
#define PCMCIA_BSIZE_8		0x10000000	/* Bank size:	8 Bytes */
#define PCMCIA_BSIZE_16		0x30000000	/* Bank size:  16 Bytes */
#define PCMCIA_BSIZE_32		0x38000000	/* Bank size:  32 Bytes */
#define PCMCIA_BSIZE_64		0x28000000	/* Bank size:  64 Bytes */
#define PCMCIA_BSIZE_128	0x20000000	/* Bank size: 128 Bytes */
#define PCMCIA_BSIZE_256	0x60000000	/* Bank size: 256 Bytes */
#define PCMCIA_BSIZE_512	0x68000000	/* Bank size: 512 Bytes */
#define PCMCIA_BSIZE_1K		0x78000000	/* Bank size:	1 kB	*/
#define PCMCIA_BSIZE_2K		0x70000000	/* Bank size:	2 kB	*/
#define PCMCIA_BSIZE_4K		0x50000000	/* Bank size:	4 kB	*/
#define PCMCIA_BSIZE_8K		0x58000000	/* Bank size:	8 kB	*/
#define PCMCIA_BSIZE_16K	0x48000000	/* Bank size:  16 kB	*/
#define PCMCIA_BSIZE_32K	0x40000000	/* Bank size:  32 kB	*/
#define PCMCIA_BSIZE_64K	0xC0000000	/* Bank size:  64 kB	*/
#define PCMCIA_BSIZE_128K	0xC8000000	/* Bank size: 128 kB	*/
#define PCMCIA_BSIZE_256K	0xD8000000	/* Bank size: 256 kB	*/
#define PCMCIA_BSIZE_512K	0xD0000000	/* Bank size: 512 kB	*/
#define PCMCIA_BSIZE_1M		0xF0000000	/* Bank size:	1 MB	*/
#define PCMCIA_BSIZE_2M		0xF8000000	/* Bank size:	2 MB	*/
#define PCMCIA_BSIZE_4M		0xE8000000	/* Bank size:	4 MB	*/
#define PCMCIA_BSIZE_8M		0xE0000000	/* Bank size:	8 MB	*/
#define PCMCIA_BSIZE_16M	0xA0000000	/* Bank size:  16 MB	*/
#define PCMCIA_BSIZE_32M	0xA8000000	/* Bank size:  32 MB	*/
#define PCMCIA_BSIZE_64M	0xB8000000	/* Bank size:  64 MB	*/

/* PCMCIA Timing */
#define PCMCIA_SHT(t)	((t & 0x0F)<<16)	/* Strobe Hold	Time	*/
#define PCMCIA_SST(t)	((t & 0x0F)<<12)	/* Strobe Setup Time	*/
#define PCMCIA_SL(t) ((t==32) ? 0 : ((t & 0x1F)<<7)) /* Strobe Length	*/

/* PCMCIA Port Sizes */
#define PCMCIA_PPS_8		0x00000000	/*  8 bit port size	*/
#define PCMCIA_PPS_16		0x00000040	/* 16 bit port size	*/

/* PCMCIA Region Select */
#define PCMCIA_PRS_MEM		0x00000000	/* Common Memory Space	*/
#define PCMCIA_PRS_ATTR		0x00000010	/*     Attribute Space	*/
#define PCMCIA_PRS_IO		0x00000018	/*	     I/O Space	*/
#define PCMCIA_PRS_DMA		0x00000020	/* DMA, normal transfer */
#define PCMCIA_PRS_DMA_LAST	0x00000028	/* DMA, last transactn	*/
#define PCMCIA_PRS_CEx		0x00000030	/* A[22:23] ==> CE1,CE2 */

#define PCMCIA_PSLOT_A		0x00000000	/* Slot A		*/
#define PCMCIA_PSLOT_B		0x00000004	/* Slot B		*/
#define PCMCIA_WPROT		0x00000002	/* Write Protect	*/
#define PCMCIA_PV		0x00000001	/* Valid Bit		*/

#define UPMA	0x00000000
#define UPMB	0x00800000

#endif	/* __MPCXX_H__ */
