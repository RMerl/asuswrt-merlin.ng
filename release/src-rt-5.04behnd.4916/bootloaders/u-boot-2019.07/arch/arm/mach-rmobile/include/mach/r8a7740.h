/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2012 Renesas Solutions Corp.
 */

#ifndef __ASM_ARCH_R8A7740_H
#define __ASM_ARCH_R8A7740_H

/*
 * R8A7740 I/O Addresses
 */

#define MERAM_BASE	0xE5580000
#define DDRP_BASE	0xC12A0000
#define HPB_BASE	0xE6000000
#define RWDT0_BASE	0xE6020000
#define RWDT1_BASE	0xE6030000
#define GPIO_BASE	0xE6050000
#define CMT1_BASE	0xE6138000
#define CPG_BASE	0xE6150000
#define SYSC_BASE	0xE6180000
#define SDHI0_BASE	0xE6850000
#define SDHI1_BASE	0xE6860000
#define MMCIF_BASE	0xE6BD0000
#define SCIF5_BASE	0xE6CB0000
#define SCIF6_BASE	0xE6CC0000
#define DBSC_BASE	0xFE400000
#define BSC_BASE	0xFEC10000
#define I2C0_BASE	0xFFF20000
#define I2C1_BASE	0xE6C20000
#define TMU_BASE	0xFFF80000

#ifndef __ASSEMBLY__
#include <asm/types.h>

/* RWDT */
struct r8a7740_rwdt {
	u16 rwtcnt0;	/* 0x00 */
	u16 dummy0;		/* 0x02 */
	u16 rwtcsra0;	/* 0x04 */
	u16 dummy1;		/* 0x06 */
	u16 rwtcsrb0;	/* 0x08 */
	u16 dummy2;		/* 0x0A */
};

/* HPB Semaphore Control Registers */
struct r8a7740_hpb {
	u32 hpbctrl0;
	u32 hpbctrl1;
	u32 hpbctrl2;
	u32 cccr;
	u32 dummy0; /* 0x20 */
	u32 hpbctrl4;
	u32 hpbctrl5;
};

/* CPG */
struct r8a7740_cpg {
	u32 frqcra;
	u32 frqcrb;
	u32 vclkcr1;
	u32 vclkcr2;
	u32 fmsickcr;
	u32 fmsockcr;
	u32 fsiackcr;
	u32 dummy0; /* 0x1c */
	u32 rtstbcr;
	u32 systbcr;
	u32 pllc01cr;
	u32 pllc2cr;
	u32 mstpsr0;
	u32 dummy1; /* 0x34 */
	u32 mstpsr1;
	u32 mstpsr5;
	u32 mstpsr2;
	u32 dummy2; /* 0x44 */
	u32 mstpsr3;
	u32 mstpsr4;
	u32 dummy3; /* 0x50 */
	u32 astat;
	u32 dummy4[4]; /* 0x58 .. 0x64 */
	u32 ztrckcr;
	u32 dummy5[5]; /* 0x6c .. 0x7c */
	u32 subckcr;
	u32 spuckcr;
	u32 vouckcr;
	u32 usbckcr;
	u32 dummy6[3]; /* 0x90 .. 0x98 */
	u32 stprckcr;
	u32 srcr0;
	u32 dummy7; /* 0xa4 */
	u32 srcr1;
	u32 dummy8; /* 0xac */
	u32 srcr2;
	u32 dummy9; /* 0xb4 */
	u32 srcr3;
	u32 srcr4;
	u32 dummy10; /* 0xc0 */
	u32 srcr5;
	u32 pllc01stpcr;
	u32 dummy11[5]; /* 0xcc .. 0xdc */
	u32 frqcrc;
	u32 frqcrd;
	u32 dummy12[10]; /* 0xe8 .. 0x10c */
	u32 rmstpcr0;
	u32 rmstpcr1;
	u32 rmstpcr2;
	u32 rmstpcr3;
	u32 rmstpcr4;
	u32 rmstpcr5;
	u32 dummy13[2]; /* 0x128 .. 0x12c */
	u32 smstpcr0;
	u32 smstpcr1;
	u32 smstpcr2;
	u32 smstpcr3;
	u32 smstpcr4;
	u32 smstpcr5;
};

/* BSC */
struct r8a7740_bsc {
	u32 cmncr;
	u32 cs0bcr;
	u32 cs2bcr;
	u32 dummy0; /* 0x0c */
	u32 cs4bcr;
	u32 cs5abcr;
	u32 cs5bbcr;
	u32 cs6abcr;
	u32 dummy1; /* 0x20 */
	u32 cs0wcr;
	u32 cs2wcr;
	u32 dummy2; /* 0x2c */
	u32 cs4wcr;
	u32 cs5awcr;
	u32 cs5bwcr;
	u32 cs6awcr;
	u32 dummy3[5]; /* 0x40 .. 0x50 */
	u32 rbwtcnt;
	u32 busycr;
	u32 dummy4[5]; /* 0x5c .. 0x6c */
	u32 bromtimcr;
	u32 dummy5[7]; /* 0x74 .. 0x8c */
	u32 bptcr00;
	u32 bptcr01;
	u32 bptcr02;
	u32 bptcr03;
	u32 bptcr04;
	u32 bptcr05;
	u32 bptcr06;
	u32 bptcr07;
	u32 bptcr08;
	u32 bptcr09;
	u32 bptcr10;
	u32 bptcr11;
	u32 bptcr12;
	u32 bptcr13;
	u32 bptcr14;
	u32 bptcr15;
	u32 bptcr16;
	u32 bptcr17;
	u32 bptcr18;
	u32 bptcr19;
	u32 bptcr20;
	u32 bptcr21;
	u32 bptcr22;
	u32 bptcr23;
	u32 bptcr24;
	u32 bptcr25;
	u32 bptcr26;
	u32 bptcr27;
	u32 bptcr28;
	u32 bptcr29;
	u32 bptcr30;
	u32 bptcr31;
	u32 bswcr;
	u32 dummy6[68]; /* 0x114 .. 0x220 */
	u32 cs0wcr2;
	u32 cs2wcr2;
	u32 dummy7; /* 0x22c */
	u32 cs4wcr2;
};

#define CS0WCR2 0xFEC10224
#define CS2WCR2 0xFEC10228
#define CS4WCR2 0xFEC10230

/* DDRP */
struct r8a7740_ddrp {
	u32 funcctrl;
	u32 dllctrl;
	u32 zqcalctrl;
	u32 zqodtctrl;
	u32 rdctrl;
	u32 rdtmg;
	u32 fifoinit;
	u32 outctrl;
	u32 dummy0[50]; /* 0x20 .. 0xe4 */
	u32 dqcalofs1;
	u32 dqcalofs2;
	u32 dummy1[2]; /* 0xf0 .. 0xf4 */
	u32 dqcalexp;
};

#define DDRPNCNT 0xE605803C
#define DDRVREFCNT 0xE61500EC

/* DBSC */
struct r8a7740_dbsc {
	u32 dummy0;
	u32 dbsvcr;
	u32 dbstate0;
	u32 dbstate1;
	u32 dbacen;
	u32 dbrfen;
	u32 dbcmd;
	u32 dbwait;
	u32 dbkind;
	u32 dbconf0;
	u32 dummy1[2]; /* 0x28 .. 0x2c */
	u32 dbphytype;
	u32 dummy2[3]; /* 0x34 .. 0x3c */
	u32 dbtr0;
	u32 dbtr1;
	u32 dbtr2;
	u32 dummy3; /* 0x4c */
	u32 dbtr3;
	u32 dbtr4;
	u32 dbtr5;
	u32 dbtr6;
	u32 dbtr7;
	u32 dbtr8;
	u32 dbtr9;
	u32 dbtr10;
	u32 dbtr11;
	u32 dbtr12;
	u32 dbtr13;
	u32 dbtr14;
	u32 dbtr15;
	u32 dbtr16;
	u32 dbtr17;
	u32 dbtr18;
	u32 dbtr19;
	u32 dummy4[7]; /* 0x94 .. 0xac */
	u32 dbbl;
	u32 dummy5[3]; /* 0xb4 .. 0xbc */
	u32 dbadj0;
	u32 dbadj1;
	u32 dbadj2;
	u32 dummy6[5]; /* 0xcc .. 0xdc */
	u32 dbrfcnf0;
	u32 dbrfcnf1;
	u32 dbrfcnf2;
	u32 dbrfcnf3;
	u32 dummy7; /* 0xf0 */
	u32 dbcalcnf;
	u32 dbcaltr;
	u32 dummy8; /* 0xfc */;
	u32 dbrnk0;
	u32 dummy9[31]; /* 0x104 .. 0x17C */
	u32 dbpdncnf;
	u32 dummy10[7]; /* 0x184 .. 0x19C */
	u32 dbmrrdr;
	u32 dummy11[39]; /* 0x1A4 .. 0x23C */
	u32 dbdfistat;
	u32 dbdficnt;
	u32 dummy12[46]; /* 0x248 .. 0x2FC */
	u32 dbbs0cnt0;
	u32 dbbs0cnt1;
};

#endif

#endif /* __ASM_ARCH_R8A7740_H */
