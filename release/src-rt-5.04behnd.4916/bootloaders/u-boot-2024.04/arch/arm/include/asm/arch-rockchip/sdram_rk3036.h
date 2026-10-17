/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Rockchip Electronics Co., Ltd
 */
#ifndef _ASM_ARCH_SDRAM_RK3036_H
#define _ASM_ARCH_SDRAM_RK3036_H

struct rk3036_ddr_pctl {
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
	u32 reserved5[47];
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
check_member(rk3036_ddr_pctl, iptr, 0x03fc);

struct rk3036_ddr_phy {
	u32 ddrphy_reg1;
	u32 ddrphy_reg3;
	u32 ddrphy_reg2;
	u32 reserve[11];
	u32 ddrphy_reg4a;
	u32 ddrphy_reg4b;
	u32 reserve1[5];
	u32 ddrphy_reg16;
	u32 reserve2;
	u32 ddrphy_reg18;
	u32 ddrphy_reg19;
	u32 reserve3;
	u32 ddrphy_reg21;
	u32 reserve4;
	u32 ddrphy_reg22;
	u32 reserve5[3];
	u32 ddrphy_reg25;
	u32 ddrphy_reg26;
	u32 ddrphy_reg27;
	u32 ddrphy_reg28;
	u32 reserve6[17];
	u32 ddrphy_reg6;
	u32 ddrphy_reg7;
	u32 reserve7;
	u32 ddrphy_reg8;
	u32 ddrphy_reg0e4;
	u32 reserve8[11];
	u32 ddrphy_reg9;
	u32 ddrphy_reg10;
	u32 reserve9;
	u32 ddrphy_reg11;
	u32 ddrphy_reg124;
	u32 reserve10[38];
	u32 ddrphy_reg29;
	u32 reserve11[40];
	u32 ddrphy_reg264;
	u32 reserve12[18];
	u32 ddrphy_reg2a;
	u32 reserve13[4];
	u32 ddrphy_reg30;
	u32 ddrphy_reg31;
	u32 ddrphy_reg32;
	u32 ddrphy_reg33;
	u32 ddrphy_reg34;
	u32 ddrphy_reg35;
	u32 ddrphy_reg36;
	u32 ddrphy_reg37;
	u32 ddrphy_reg38;
	u32 ddrphy_reg39;
	u32 ddrphy_reg40;
	u32 ddrphy_reg41;
	u32 ddrphy_reg42;
	u32 ddrphy_reg43;
	u32 ddrphy_reg44;
	u32 ddrphy_reg45;
	u32 ddrphy_reg46;
	u32 ddrphy_reg47;
	u32 ddrphy_reg48;
	u32 ddrphy_reg49;
	u32 ddrphy_reg50;
	u32 ddrphy_reg51;
	u32 ddrphy_reg52;
	u32 ddrphy_reg53;
	u32 reserve14;
	u32 ddrphy_reg54;
	u32 ddrphy_reg55;
	u32 ddrphy_reg56;
	u32 ddrphy_reg57;
	u32 ddrphy_reg58;
	u32 ddrphy_reg59;
	u32 ddrphy_reg5a;
	u32 ddrphy_reg5b;
	u32 ddrphy_reg5c;
	u32 ddrphy_reg5d;
	u32 ddrphy_reg5e;
	u32 reserve15[28];
	u32 ddrphy_reg5f;
	u32 reserve16[6];
	u32 ddrphy_reg60;
	u32 ddrphy_reg61;
	u32 ddrphy_reg62;
};
check_member(rk3036_ddr_phy, ddrphy_reg62, 0x03e8);

struct rk3036_pctl_timing {
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
};

struct rk3036_phy_timing {
	u32 mr[4];
	u32 bl;
	u32 cl_al;
};

typedef union {
	u32 noc_timing;
	struct {
		u32 acttoact:6;
		u32 rdtomiss:6;
		u32 wrtomiss:6;
		u32 burstlen:3;
		u32 rdtowr:5;
		u32 wrtord:5;
		u32 bwratio:1;
	};
} rk3036_noc_timing;

struct rk3036_ddr_timing {
	u32 freq;
	struct rk3036_pctl_timing pctl_timing;
	struct rk3036_phy_timing phy_timing;
	rk3036_noc_timing noc_timing;
};

struct rk3036_service_sys {
	u32 id_coreid;
	u32 id_revisionid;
	u32 ddrconf;
	u32 ddrtiming;
	u32 ddrmode;
	u32 readlatency;
};

struct rk3036_ddr_config {
	/*
	 * 000: lpddr
	 * 001: ddr
	 * 010: ddr2
	 * 011: ddr3
	 * 100: lpddr2-s2
	 * 101: lpddr2-s4
	 * 110: lpddr3
	 */
	u32 ddr_type;
	u32 rank;
	u32 cs0_row;
	u32 cs1_row;

	/* 2: 4bank, 3: 8bank */
	u32 bank;
	u32 col;

	/* bw(0: 8bit, 1: 16bit, 2: 32bit) */
	u32 bw;
};

/* rk3036 sdram initial */
void sdram_init(void);

/* get ddr die config, implement in specific board */
void get_ddr_config(struct rk3036_ddr_config *config);

/* get ddr size on board */
size_t sdram_size(void);
#endif
