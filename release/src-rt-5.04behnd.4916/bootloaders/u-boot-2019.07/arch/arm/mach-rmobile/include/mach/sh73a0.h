#ifndef __ASM_ARCH_RMOBILE_SH73A0_H
#define __ASM_ARCH_RMOBILE_SH73A0_H

/* Global Timer */
#define GLOBAL_TIMER_BASE_ADDR	(0xF0000200)
#define MERAM_BASE	(0xE5580000)

/* GIC */
#define GIC_BASE	(0xF0000100)
#define ICCICR	GIC_BASE

/* Secure control register */
#define LIFEC_SEC_SRC	(0xE6110008)

/* RWDT */
#define	RWDT_BASE   (0xE6020000)

/* HPB Semaphore Control Registers */
#define HPB_BASE	(0xE6001010)

/* Bus Semaphore Control Registers */
#define HPBSCR_BASE (0xE6001600)

/* SBSC1 */
#define SBSC1_BASE	(0xFE400000)
#define	SDMRA1A		(SBSC1_BASE + 0x100000)
#define	SDMRA2A		(SBSC1_BASE + 0x1C0000)
#define	SDMRA3A		(SBSC1_BASE + 0x104000)

/* SBSC2 */
#define SBSC2_BASE	(0xFB400000)
#define	SDMRA1B		(SBSC2_BASE + 0x100000)
#define	SDMRA2B		(SBSC2_BASE + 0x1C0000)
#define	SDMRA3B		(SBSC2_BASE + 0x104000)

/* CPG */
#define CPG_BASE   (0xE6150000)
#define	CPG_SRCR_BASE	(CPG_BASE + 0x80A0)
#define WUPCR	(CPG_BASE + 0x1010)
#define SRESCR	(CPG_BASE + 0x1018)
#define PCLKCR	(CPG_BASE + 0x1020)

/* SYSC */
#define SYSC_BASE   (0xE6180000)
#define RESCNT2	(SYSC_BASE + 0x8020)

/* BSC */
#define BSC_BASE (0xFEC10000)

/* SCIF */
#define SCIF0_BASE	(0xE6C40000)
#define SCIF1_BASE	(0xE6C50000)
#define SCIF2_BASE	(0xE6C60000)
#define SCIF3_BASE	(0xE6C70000)
#define SCIF4_BASE	(0xE6C80000)
#define SCIF5_BASE	(0xE6CB0000)
#define SCIF6_BASE	(0xE6CC0000)
#define SCIF7_BASE	(0xE6CD0000)

#ifndef __ASSEMBLY__
#include <asm/types.h>

/* RWDT */
struct sh73a0_rwdt {
	u16 rwtcnt0;	/* 0x00 */
	u16 dummy0;	/* 0x02 */
	u16 rwtcsra0;	/* 0x04 */
	u16 dummy1;	/* 0x06 */
	u16 rwtcsrb0;	/* 0x08 */
};

/* HPB Semaphore Control Registers */
struct sh73a0_hpb {
	u32 hpbctrl0;
	u32 hpbctrl1;
	u32 hpbctrl2;
	u32 cccr;
	u32 dummy0; /* 0x20 */
	u32 hpbctrl4;
	u32 hpbctrl5;
	u32 dummy1; /* 0x2C */
	u32 hpbctrl6;
};

/* Bus Semaphore Control Registers */
struct sh73a0_hpb_bscr {
	u32 mpsrc; /* 0x00 */
	u32 mpacctl; /* 0x04 */
	u32 dummy0[6];
	u32 smgpiosrc; /* 0x20 */
	u32 smgpioerr;
	u32 smgpiotime;
	u32 smgpiocnt;
	u32 dummy1[4]; /* 0x30 .. 0x3C */
	u32 smcmt2src;
	u32 smcmt2err;
	u32 smcmt2time;
	u32 smcmt2cnt;
	u32 smcpgsrc;
	u32 smcpgerr;
	u32 smcpgtime;
	u32 smcpgcnt;
	u32 dummy2[4]; /* 0x60 - 0x6C */
	u32 smsyscsrc;
	u32 smsyscerr;
	u32 smsysctime;
	u32 smsysccnt;
};

/* SBSC */
struct sh73a0_sbsc {
	u32 dummy0[2]; /* 0x00, 0x04 */
	u32 sdcr0;
	u32 sdcr1;
	u32 sdpcr;
	u32 dummy1; /* 0x14 */
	u32 sdcr0s;
	u32 sdcr1s;
	u32 rtcsr;
	u32 dummy2; /* 0x24 */
	u32 rtcor;
	u32 rtcorh;
	u32 rtcors;
	u32 rtcorsh;
	u32 dummy3[2]; /* 0x38, 0x3C */
	u32 sdwcrc0;
	u32 sdwcrc1;
	u32 sdwcr00;
	u32 sdwcr01;
	u32 sdwcr10;
	u32 sdwcr11;
	u32 sdpdcr0;
	u32 dummy4; /* 0x5C */
	u32 sdwcr2;
	u32 sdwcrc2;
	u32 zqccr;
	u32 dummy5[6]; /* 0x6C .. 0x80 */
	u32 sdmracr0;
	u32 dummy6; /* 0x88 */
	u32 sdmrtmpcr;
	u32 dummy7; /* 0x90 */
	u32 sdmrtmpmsk;
	u32 dummy8; /* 0x98 */
	u32 sdgencnt;
	u32 dphycnt0;
	u32 dphycnt1;
	u32 dphycnt2;
	u32 dummy9[2]; /* 0xAC .. 0xB0 */
	u32 sddrvcr0;
	u32 dummy10[14]; /* 0xB8 .. 0xEC */
	u32 dptdivcr0;
	u32 dptdivcr1;
	u32 dptdivcr2;
	u32 dummy11; /* 0xFC */
	u32 sdptcr0;
	u32 sdptcr1;
	u32 sdptcr2;
	u32 sdptcr3; /* 0x10C */
	u32 dummy12[145]; /* 0x110 .. 0x350 */
	u32 dllcnt0; /* 0x354 */
	u32 sbscmon0;
};

/* CPG */
struct sh73a0_sbsc_cpg {
	u32 frqcra; /* 0x00 */
	u32 frqcrb;
	u32 vclkcr1;
	u32 vclkcr2;
	u32 zbckcr;
	u32 flckcr;
	u32 fsiackcr;
	u32 vclkcr3;
	u32 rtstbcr;
	u32 systbcr;
	u32 pll1cr;
	u32 pll2cr;
	u32 mstpsr0;
	u32 dummy0; /* 0x34 */
	u32 mstpsr1;
	u32 mstpsr5;
	u32 mstpsr2;
	u32 dummy1; /* 0x44 */
	u32 mstpsr3;
	u32 mstpsr4;
	u32 dummy2; /* 0x50 */
	u32 astat;
	u32 dvfscr0;
	u32 dvfscr1;
	u32 dsitckcr;
	u32 dsi0pckcr;
	u32 dsi1pckcr;
	u32 dsi0phycr;
	u32 dsi1phycr;
	u32 sd0ckcr;
	u32 sd1ckcr;
	u32 sd2ckcr;
	u32 subckcr;
	u32 spuackcr;
	u32 msuckcr;
	u32 hsickcr;
	u32 fsibckcr;
	u32 spuvckcr;
	u32 mfck1cr;
	u32 mfck2cr;
	u32 dummy3[8]; /* 0xA0 .. 0xBC */
	u32 ckscr;
	u32 dummy4; /* 0xC4 */
	u32 pll1stpcr;
	u32 mpmode;
	u32 pllecr;
	u32 dummy5; /* 0xD4 */
	u32 pll0cr;
	u32 pll3cr;
	u32 dummy6; /* 0xE0 */
	u32 frqcrd;
	u32 dummyi7; /* 0xE8 */
	u32 vrefcr;
	u32 pll0stpcr;
	u32 dummy8; /* 0xF4 */
	u32 pll2stpcr;
	u32 pll3stpcr;
	u32 dummy9[4]; /* 0x100 .. 0x10c */
	u32 rmstpcr0;
	u32 rmstpcr1;
	u32 rmstpcr2;
	u32 rmstpcr3;
	u32 rmstpcr4;
	u32 rmstpcr5;
	u32 dummy10[2]; /* 0x128 .. 0x12c */
	u32 smstpcr0;
	u32 smstpcr1;
	u32 smstpcr2;
	u32 smstpcr3;
	u32 smstpcr4;
	u32 smstpcr5;
	u32 dummy11[2]; /* 0x148 .. 0x14c */
	u32 cpgxxcs4;
	u32 dummy12[7]; /* 0x154 .. 0x16c */
	u32 dvfscr2;
	u32 dvfscr3;
	u32 dvfscr4;
	u32 dvfscr5; /* 0x17C */
};

/* CPG SRCR part OK */
struct sh73a0_sbsc_cpg_srcr {
	u32 srcr0;
	u32 dummy0; /* 0xA4 */
	u32 srcr1;
	u32 dummy1; /* 0xAC */
	u32 srcr2;
	u32 dummy2; /* 0xB4 */
	u32 srcr3;
	u32 srcr4;
	u32 dummy3; /* 0xC0 */
	u32 srcr5;
};

/* BSC */
struct sh73a0_bsc {
	u32 cmncr;
	u32 cs0bcr;
	u32 cs2bcr;
	u32 dummy0; /* 0x0C */
	u32 cs4bcr;
	u32 cs5abcr;
	u32 cs5bbcr;
	u32 cs6abcr;
	u32 cs6bbcr;
	u32 cs0wcr;
	u32 cs2wcr;
	u32 dummy1; /* 0x2C */
	u32 cs4wcr;
	u32 cs5awcr;
	u32 cs5bwcr;
	u32 cs6awcr;
	u32 cs6bwcr;
	u32 rbwtcnt;
	u32 busycr;
	u32 dummy2; /* 0x5c */
	u32 cs7abcr;
	u32 cs7awcr;
	u32 dummy3[2]; /* 0x68, 0x6C */
	u32 bromtimcr;
};
#endif /* __ASSEMBLY__ */

#endif /* __ASM_ARCH_RMOBILE_SH73A0_H */
