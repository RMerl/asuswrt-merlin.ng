/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2015 Google, Inc
 */

#ifndef _ASM_ARCH_DDR_RK3288_H
#define _ASM_ARCH_DDR_RK3288_H

struct rk3288_ddr_pctl {
	u32 scfg;
	u32 sctl;
	u32 stat;
	u32 intrstat;
	u32 reserved0[12];
	u32 mcmd;
	u32 powctl;
	u32 powstat;
	u32 cmdtstat;
	u32 tstaten;
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
	u32 reserved5[14];
	u32 ecccfg;
	u32 ecctst;
	u32 eccclr;
	u32 ecclog;
	u32 reserved6[28];
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
check_member(rk3288_ddr_pctl, iptr, 0x03fc);

struct rk3288_ddr_publ_datx {
	u32 dxgcr;
	u32 dxgsr[2];
	u32 dxdllcr;
	u32 dxdqtr;
	u32 dxdqstr;
	u32 reserved[10];
};

struct rk3288_ddr_publ {
	u32 ridr;
	u32 pir;
	u32 pgcr;
	u32 pgsr;
	u32 dllgcr;
	u32 acdllcr;
	u32 ptr[3];
	u32 aciocr;
	u32 dxccr;
	u32 dsgcr;
	u32 dcr;
	u32 dtpr[3];
	u32 mr[4];
	u32 odtcr;
	u32 dtar;
	u32 dtdr[2];
	u32 reserved1[24];
	u32 dcuar;
	u32 dcudr;
	u32 dcurr;
	u32 dculr;
	u32 dcugcr;
	u32 dcutpr;
	u32 dcusr[2];
	u32 reserved2[8];
	u32 bist[17];
	u32 reserved3[15];
	u32 zq0cr[2];
	u32 zq0sr[2];
	u32 zq1cr[2];
	u32 zq1sr[2];
	u32 zq2cr[2];
	u32 zq2sr[2];
	u32 zq3cr[2];
	u32 zq3sr[2];
	struct rk3288_ddr_publ_datx datx8[4];
};
check_member(rk3288_ddr_publ, datx8[3].dxdqstr, 0x0294);

struct rk3288_msch {
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
check_member(rk3288_msch, devtodev, 0x003c);

/* PCT_DFISTCFG0 */
#define DFI_INIT_START			(1 << 0)

/* PCT_DFISTCFG1 */
#define DFI_DRAM_CLK_SR_EN		(1 << 0)
#define DFI_DRAM_CLK_DPD_EN		(1 << 1)

/* PCT_DFISTCFG2 */
#define DFI_PARITY_INTR_EN		(1 << 0)
#define DFI_PARITY_EN			(1 << 1)

/* PCT_DFILPCFG0 */
#define TLP_RESP_TIME_SHIFT		16
#define LP_SR_EN			(1 << 8)
#define LP_PD_EN			(1 << 0)

/* PCT_DFITCTRLDELAY */
#define TCTRL_DELAY_TIME_SHIFT		0

/* PCT_DFITPHYWRDATA */
#define TPHY_WRDATA_TIME_SHIFT		0

/* PCT_DFITPHYRDLAT */
#define TPHY_RDLAT_TIME_SHIFT		0

/* PCT_DFITDRAMCLKDIS */
#define TDRAM_CLK_DIS_TIME_SHIFT	0

/* PCT_DFITDRAMCLKEN */
#define TDRAM_CLK_EN_TIME_SHIFT		0

/* PCTL_DFIODTCFG */
#define RANK0_ODT_WRITE_SEL		(1 << 3)
#define RANK1_ODT_WRITE_SEL		(1 << 11)

/* PCTL_DFIODTCFG1 */
#define ODT_LEN_BL8_W_SHIFT		16

/* PUBL_ACDLLCR */
#define ACDLLCR_DLLDIS			(1 << 31)
#define ACDLLCR_DLLSRST			(1 << 30)

/* PUBL_DXDLLCR */
#define DXDLLCR_DLLDIS			(1 << 31)
#define DXDLLCR_DLLSRST			(1 << 30)

/* PUBL_DLLGCR */
#define DLLGCR_SBIAS			(1 << 30)

/* PUBL_DXGCR */
#define DQSRTT				(1 << 9)
#define DQRTT				(1 << 10)

/* PIR */
#define PIR_INIT			(1 << 0)
#define PIR_DLLSRST			(1 << 1)
#define PIR_DLLLOCK			(1 << 2)
#define PIR_ZCAL			(1 << 3)
#define PIR_ITMSRST			(1 << 4)
#define PIR_DRAMRST			(1 << 5)
#define PIR_DRAMINIT			(1 << 6)
#define PIR_QSTRN			(1 << 7)
#define PIR_RVTRN			(1 << 8)
#define PIR_ICPC			(1 << 16)
#define PIR_DLLBYP			(1 << 17)
#define PIR_CTLDINIT			(1 << 18)
#define PIR_CLRSR			(1 << 28)
#define PIR_LOCKBYP			(1 << 29)
#define PIR_ZCALBYP			(1 << 30)
#define PIR_INITBYP			(1u << 31)

/* PGCR */
#define PGCR_DFTLMT_SHIFT		3
#define PGCR_DFTCMP_SHIFT		2
#define PGCR_DQSCFG_SHIFT		1
#define PGCR_ITMDMD_SHIFT		0

/* PGSR */
#define PGSR_IDONE			(1 << 0)
#define PGSR_DLDONE			(1 << 1)
#define PGSR_ZCDONE			(1 << 2)
#define PGSR_DIDONE			(1 << 3)
#define PGSR_DTDONE			(1 << 4)
#define PGSR_DTERR			(1 << 5)
#define PGSR_DTIERR			(1 << 6)
#define PGSR_DFTERR			(1 << 7)
#define PGSR_RVERR			(1 << 8)
#define PGSR_RVEIRR			(1 << 9)

/* PTR0 */
#define PRT_ITMSRST_SHIFT		18
#define PRT_DLLLOCK_SHIFT		6
#define PRT_DLLSRST_SHIFT		0

/* PTR1 */
#define PRT_DINIT0_SHIFT		0
#define PRT_DINIT1_SHIFT		19

/* PTR2 */
#define PRT_DINIT2_SHIFT		0
#define PRT_DINIT3_SHIFT		17

/* DCR */
#define DDRMD_LPDDR			0
#define DDRMD_DDR			1
#define DDRMD_DDR2			2
#define DDRMD_DDR3			3
#define DDRMD_LPDDR2_LPDDR3		4
#define DDRMD_MASK			7
#define DDRMD_SHIFT			0
#define PDQ_MASK			7
#define PDQ_SHIFT			4

/* DXCCR */
#define DQSNRES_MASK			0xf
#define DQSNRES_SHIFT			8
#define DQSRES_MASK			0xf
#define DQSRES_SHIFT			4

/* DTPR */
#define TDQSCKMAX_SHIFT			27
#define TDQSCKMAX_MASK			7
#define TDQSCK_SHIFT			24
#define TDQSCK_MASK			7

/* DSGCR */
#define DQSGX_SHIFT			5
#define DQSGX_MASK			7
#define DQSGE_SHIFT			8
#define DQSGE_MASK			7

/* SCTL */
#define INIT_STATE			0
#define CFG_STATE			1
#define GO_STATE			2
#define SLEEP_STATE			3
#define WAKEUP_STATE			4

/* STAT */
#define LP_TRIG_SHIFT			4
#define LP_TRIG_MASK			7
#define PCTL_STAT_MSK			7
#define INIT_MEM			0
#define CONFIG				1
#define CONFIG_REQ			2
#define ACCESS				3
#define ACCESS_REQ			4
#define LOW_POWER			5
#define LOW_POWER_ENTRY_REQ		6
#define LOW_POWER_EXIT_REQ		7

/* ZQCR*/
#define PD_OUTPUT_SHIFT			0
#define PU_OUTPUT_SHIFT			5
#define PD_ONDIE_SHIFT			10
#define PU_ONDIE_SHIFT			15
#define ZDEN_SHIFT			28

/* DDLGCR */
#define SBIAS_BYPASS			(1 << 23)

/* MCFG */
#define MDDR_LPDDR2_CLK_STOP_IDLE_SHIFT	24
#define PD_IDLE_SHIFT			8
#define MDDR_EN				(2 << 22)
#define LPDDR2_EN			(3 << 22)
#define DDR2_EN				(0 << 5)
#define DDR3_EN				(1 << 5)
#define LPDDR2_S2			(0 << 6)
#define LPDDR2_S4			(1 << 6)
#define MDDR_LPDDR2_BL_2		(0 << 20)
#define MDDR_LPDDR2_BL_4		(1 << 20)
#define MDDR_LPDDR2_BL_8		(2 << 20)
#define MDDR_LPDDR2_BL_16		(3 << 20)
#define DDR2_DDR3_BL_4			0
#define DDR2_DDR3_BL_8			1
#define TFAW_SHIFT			18
#define PD_EXIT_SLOW			(0 << 17)
#define PD_EXIT_FAST			(1 << 17)
#define PD_TYPE_SHIFT			16
#define BURSTLENGTH_SHIFT		20

/* POWCTL */
#define POWER_UP_START			(1 << 0)

/* POWSTAT */
#define POWER_UP_DONE			(1 << 0)

/* MCMD */
enum {
	DESELECT_CMD			= 0,
	PREA_CMD,
	REF_CMD,
	MRS_CMD,
	ZQCS_CMD,
	ZQCL_CMD,
	RSTL_CMD,
	MRR_CMD				= 8,
	DPDE_CMD,
};

#define LPDDR2_MA_SHIFT			4
#define LPDDR2_MA_MASK			0xff
#define LPDDR2_OP_SHIFT			12
#define LPDDR2_OP_MASK			0xff

#define START_CMD			(1u << 31)

/*
 * DDRCONF
 * [5:4] row(13+n)
 * [1:0] col(9+n), assume bw=2
 */
#define DDRCONF_ROW_SHIFT		4
#define DDRCONF_COL_SHIFT		0

/* DEVTODEV */
#define BUSWRTORD_SHIFT			4
#define BUSRDTOWR_SHIFT			2
#define BUSRDTORD_SHIFT			0

/* mr1 for ddr3 */
#define DDR3_DLL_DISABLE		1

#endif
