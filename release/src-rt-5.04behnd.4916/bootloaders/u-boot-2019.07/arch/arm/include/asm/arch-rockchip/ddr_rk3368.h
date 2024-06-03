/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2017 Theobroma Systems Design und Consulting GmbH
 */

#ifndef __ASM_ARCH_DDR_RK3368_H__
#define __ASM_ARCH_DDR_RK3368_H__

/*
 * The RK3368 DDR PCTL differs from the incarnation in the RK3288 only
 * in a few details. Most notably, it has an additional field to track
 * tREFI in controller cycles (i.e. trefi_mem_ddr3).
 */
struct rk3368_ddr_pctl {
	u32 scfg;
	u32 sctl;
	u32 stat;
	u32 intrstat;
	u32 reserved0[12];
	u32 mcmd;
	u32 powctl;
	u32 powstat;
	u32 cmdtstat;
	u32 cmdtstaten;
	u32 reserved1[3];
	u32 mrrcfg0;
	u32 mrrstat0;
	u32 mrrstat1;
	u32 reserved2[4];
	u32 mcfg1;
	u32 mcfg;
	u32 ppcfg;
	u32 mstat;
	u32 lpddr2zqcfg;
	u32 reserved3;
	u32 dtupdes;
	u32 dtuna;
	u32 dtune;
	u32 dtuprd0;
	u32 dtuprd1;
	u32 dtuprd2;
	u32 dtuprd3;
	u32 dtuawdt;
	u32 reserved4[3];
	u32 togcnt1u;
	u32 tinit;
	u32 trsth;
	u32 togcnt100n;
	u32 trefi;
	u32 tmrd;
	u32 trfc;
	u32 trp;
	u32 trtw;
	u32 tal;
	u32 tcl;
	u32 tcwl;
	u32 tras;
	u32 trc;
	u32 trcd;
	u32 trrd;
	u32 trtp;
	u32 twr;
	u32 twtr;
	u32 texsr;
	u32 txp;
	u32 txpdll;
	u32 tzqcs;
	u32 tzqcsi;
	u32 tdqs;
	u32 tcksre;
	u32 tcksrx;
	u32 tcke;
	u32 tmod;
	u32 trstl;
	u32 tzqcl;
	u32 tmrr;
	u32 tckesr;
	u32 tdpd;
	u32 trefi_mem_ddr3;
	u32 reserved5[45];
	u32 dtuwactl;
	u32 dturactl;
	u32 dtucfg;
	u32 dtuectl;
	u32 dtuwd0;
	u32 dtuwd1;
	u32 dtuwd2;
	u32 dtuwd3;
	u32 dtuwdm;
	u32 dturd0;
	u32 dturd1;
	u32 dturd2;
	u32 dturd3;
	u32 dtulfsrwd;
	u32 dtulfsrrd;
	u32 dtueaf;
	u32 dfitctrldelay;
	u32 dfiodtcfg;
	u32 dfiodtcfg1;
	u32 dfiodtrankmap;
	u32 dfitphywrdata;
	u32 dfitphywrlat;
	u32 reserved7[2];
	u32 dfitrddataen;
	u32 dfitphyrdlat;
	u32 reserved8[2];
	u32 dfitphyupdtype0;
	u32 dfitphyupdtype1;
	u32 dfitphyupdtype2;
	u32 dfitphyupdtype3;
	u32 dfitctrlupdmin;
	u32 dfitctrlupdmax;
	u32 dfitctrlupddly;
	u32 reserved9;
	u32 dfiupdcfg;
	u32 dfitrefmski;
	u32 dfitctrlupdi;
	u32 reserved10[4];
	u32 dfitrcfg0;
	u32 dfitrstat0;
	u32 dfitrwrlvlen;
	u32 dfitrrdlvlen;
	u32 dfitrrdlvlgateen;
	u32 dfiststat0;
	u32 dfistcfg0;
	u32 dfistcfg1;
	u32 reserved11;
	u32 dfitdramclken;
	u32 dfitdramclkdis;
	u32 dfistcfg2;
	u32 dfistparclr;
	u32 dfistparlog;
	u32 reserved12[3];
	u32 dfilpcfg0;
	u32 reserved13[3];
	u32 dfitrwrlvlresp0;
	u32 dfitrwrlvlresp1;
	u32 dfitrwrlvlresp2;
	u32 dfitrrdlvlresp0;
	u32 dfitrrdlvlresp1;
	u32 dfitrrdlvlresp2;
	u32 dfitrwrlvldelay0;
	u32 dfitrwrlvldelay1;
	u32 dfitrwrlvldelay2;
	u32 dfitrrdlvldelay0;
	u32 dfitrrdlvldelay1;
	u32 dfitrrdlvldelay2;
	u32 dfitrrdlvlgatedelay0;
	u32 dfitrrdlvlgatedelay1;
	u32 dfitrrdlvlgatedelay2;
	u32 dfitrcmd;
	u32 reserved14[46];
	u32 ipvr;
	u32 iptr;
};
check_member(rk3368_ddr_pctl, iptr, 0x03fc);

struct rk3368_ddrphy {
	u32 reg[0x100];
};
check_member(rk3368_ddrphy, reg[0xff], 0x03fc);

struct rk3368_msch {
	u32 coreid;
	u32 revisionid;
	u32 ddrconf;
	u32 ddrtiming;
	u32 ddrmode;
	u32 readlatency;
	u32 reserved1[8];
	u32 activate;
	u32 devtodev;
};
check_member(rk3368_msch, devtodev, 0x003c);

/* GRF_SOC_CON0 */
enum {
	NOC_RSP_ERR_STALL = BIT(9),
	MOBILE_DDR_SEL = BIT(4),
	DDR0_16BIT_EN = BIT(3),
	MSCH0_MAINDDR3_DDR3 = BIT(2),
	MSCH0_MAINPARTIALPOP = BIT(1),
	UPCTL_C_ACTIVE = BIT(0),
};

#endif
