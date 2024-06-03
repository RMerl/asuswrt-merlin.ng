/* $Id$ */

#ifndef _REGS_H
#define _REGS_H

/* Special Purpose Registers */

#define SPR_CR		-1
#define SPR_MSR		-2

#define SPR_XER		1
#define SPR_LR		8
#define SPR_CTR		9
#define SPR_DSISR	18
#define SPR_DAR		19
#define SPR_DEC		22
#define SPR_SRR0	26
#define SPR_SRR1	27
#define SPR_EIE		80
#define SPR_EID		81
#define SPR_CMPA	144
#define SPR_CMPB	145
#define SPR_CMPC	146
#define SPR_CMPD	147
#define SPR_ICR		148
#define SPR_DER		149
#define SPR_COUNTA	150
#define SPR_COUNTB	151
#define SPR_CMPE	152
#define SPR_CMPF	153
#define SPR_CMPG	154
#define SPR_CMPH	155
#define SPR_LCTRL1	156
#define SPR_LCTRL2	157
#define SPR_ICTRL	158
#define SPR_BAR		159
#define SPR_USPRG0      256
#define SPR_SPRG4_RO    260
#define SPR_SPRG5_RO    261
#define SPR_SPRG6_RO    262
#define SPR_SPRG7_RO    263
#define SPR_SPRG0	272
#define SPR_SPRG1	273
#define SPR_SPRG2	274
#define SPR_SPRG3	275
#define SPR_SPRG4       276
#define SPR_SPRG5       277
#define SPR_SPRG6       278
#define SPR_SPRG7       279
#define SPR_EAR         282	/* MPC603e core */
#define SPR_TBL         284
#define SPR_TBU         285
#define SPR_PVR		287
#define SPR_IC_CST	560
#define SPR_IC_ADR	561
#define SPR_IC_DAT	562
#define SPR_DC_CST	568
#define SPR_DC_ADR	569
#define SPR_DC_DAT	570
#define SPR_DPDR	630
#define SPR_IMMR	638
#define SPR_MI_CTR	784
#define SPR_MI_AP	786
#define SPR_MI_EPN	787
#define SPR_MI_TWC	789
#define SPR_MI_RPN	790
#define SPR_MD_CTR	792
#define SPR_M_CASID	793
#define SPR_MD_AP	794
#define SPR_MD_EPN	795
#define SPR_M_TWB	796
#define SPR_MD_TWC	797
#define SPR_MD_RPN	798
#define SPR_M_TW	799
#define SPR_MI_DBCAM	816
#define SPR_MI_DBRAM0	817
#define SPR_MI_DBRAM1	818
#define SPR_MD_DBCAM	824
#define SPR_MD_DBRAM0	825
#define SPR_MD_DBRAM1	826
#define SPR_ZPR         944
#define SPR_PID         945
#define SPR_CCR0        947
#define SPR_IAC3        948
#define SPR_IAC4        949
#define SPR_DVC1        950
#define SPR_DVC2        951
#define SPR_SGR         953
#define SPR_DCWR        954
#define SPR_SLER        955
#define SPR_SU0R        956
#define SPR_DBCR1       957
#define SPR_ICDBDR      979
#define SPR_ESR         980
#define SPR_DEAR        981
#define SPR_EVPR        982
#define SPR_TSR         984
#define SPR_TCR         986
#define SPR_PIT         987
#define SPR_SRR2        990
#define SPR_SRR3        991
#define SPR_DBSR        1008
#define SPR_DBCR0       1010
#define SPR_IABR        1010	/* MPC603e core */
#define SPR_IAC1        1012
#define SPR_IAC2        1013
#define SPR_DAC1        1014
#define SPR_DAC2        1015
#define SPR_DCCR        1018
#define SPR_ICCR        1019

/* Bits for the DBCR0 register */
#define DBCR0_EDM	0x80000000
#define DBCR0_IDM	0x40000000
#define DBCR0_RST	0x30000000
#define DBCR0_IC	0x08000000
#define DBCR0_BT	0x04000000
#define DBCR0_EDE	0x02000000
#define DBCR0_TDE	0x01000000
#define DBCR0_IA1	0x00800000
#define DBCR0_IA2	0x00400000
#define DBCR0_IA12	0x00200000
#define DBCR0_IA12X	0x00100000
#define DBCR0_IA3	0x00080000
#define DBCR0_IA4	0x00040000
#define DBCR0_IA34	0x00020000
#define DBCR0_IA34X	0x00010000
#define DBCR0_IA12T	0x00008000
#define DBCR0_IA34T	0x00004000
#define DBCR0_FT	0x00000001

/* Bits for the DBCR1 register */
#define DBCR1_D1R	0x80000000
#define DBCR1_D2R	0x40000000
#define DBCR1_D1W	0x20000000
#define DBCR1_D2W	0x10000000
#define DBCR1_D1S	0x0C000000
#define DBCR1_D2S	0x03000000
#define DBCR1_DA12	0x00800000
#define DBCR1_DA12X	0x00400000
#define DBCR1_DV1M	0x000C0000
#define DBCR1_DV2M	0x00030000
#define DBCR1_DV1BE	0x0000F000
#define DBCR1_DV2BE	0x00000F00

/*
 * DBSR bits which have conflicting definitions on true Book E versus PPC40x
 */
#ifdef CONFIG_BOOKE
#define DBSR_IA1	0x00800000	/* Instr Address Compare 1 Event */
#define DBSR_IA2	0x00400000	/* Instr Address Compare 2 Event */
#define DBSR_IA3	0x00200000	/* Instr Address Compare 3 Event */
#define DBSR_IA4	0x00100000	/* Instr Address Compare 4 Event */
#endif
#define DBSR_IA1	0x04000000	/* Instr Address Compare 1 Event */
#define DBSR_IA2	0x02000000	/* Instr Address Compare 2 Event */
#define DBSR_IA3	0x00080000	/* Instr Address Compare 3 Event */
#define DBSR_IA4	0x00040000	/* Instr Address Compare 4 Event */

struct spr_info {
  int  spr_val;
  char spr_name[ 10 ];
};

extern struct spr_info spr_map[];
extern const unsigned int n_sprs;


#define SET_REGISTER( str, val ) \
({ unsigned long __value = (val); \
  asm volatile( str : : "r" (__value)); \
  __value; })

#define	GET_REGISTER( str ) \
({ unsigned long __value; \
  asm volatile( str : "=r" (__value) : ); \
  __value; })

#define	 GET_CR()	     GET_REGISTER( "mfcr %0" )
#define	 SET_CR(val)	     SET_REGISTER( "mtcr %0", val )
#define	 GET_MSR()	     GET_REGISTER( "mfmsr %0" )
#define	 SET_MSR(val)	     SET_REGISTER( "mtmsr %0", val )
#define	 GET_XER()	     GET_REGISTER( "mfspr %0,1" )
#define	 SET_XER(val)	     SET_REGISTER( "mtspr 1,%0", val )
#define	 GET_LR()	     GET_REGISTER( "mfspr %0,8" )
#define	 SET_LR(val)	     SET_REGISTER( "mtspr 8,%0", val )
#define	 GET_CTR()	     GET_REGISTER( "mfspr %0,9" )
#define	 SET_CTR(val)	     SET_REGISTER( "mtspr 9,%0", val )
#define	 GET_DSISR()	     GET_REGISTER( "mfspr %0,18" )
#define	 SET_DSISR(val)	     SET_REGISTER( "mtspr 18,%0", val )
#define	 GET_DAR()	     GET_REGISTER( "mfspr %0,19" )
#define	 SET_DAR(val)	     SET_REGISTER( "mtspr 19,%0", val )
#define	 GET_DEC()	     GET_REGISTER( "mfspr %0,22" )
#define	 SET_DEC(val)	     SET_REGISTER( "mtspr 22,%0", val )
#define	 GET_SRR0()	     GET_REGISTER( "mfspr %0,26" )
#define	 SET_SRR0(val)       SET_REGISTER( "mtspr 26,%0", val )
#define	 GET_SRR1()	     GET_REGISTER( "mfspr %0,27" )
#define	 SET_SRR1(val)	     SET_REGISTER( "mtspr 27,%0", val )
#define	 GET_EIE()	     GET_REGISTER( "mfspr %0,80" )
#define	 SET_EIE(val)	     SET_REGISTER( "mtspr 80,%0", val )
#define	 GET_EID()	     GET_REGISTER( "mfspr %0,81" )
#define	 SET_EID(val)	     SET_REGISTER( "mtspr 81,%0", val )
#define	 GET_CMPA()	     GET_REGISTER( "mfspr %0,144" )
#define	 SET_CMPA(val)	     SET_REGISTER( "mtspr 144,%0", val )
#define	 GET_CMPB()	     GET_REGISTER( "mfspr %0,145" )
#define	 SET_CMPB(val)	     SET_REGISTER( "mtspr 145,%0", val )
#define	 GET_CMPC()	     GET_REGISTER( "mfspr %0,146" )
#define	 SET_CMPC(val)	     SET_REGISTER( "mtspr 146,%0", val )
#define	 GET_CMPD()	     GET_REGISTER( "mfspr %0,147" )
#define	 SET_CMPD(val)	     SET_REGISTER( "mtspr 147,%0", val )
#define	 GET_ICR()	     GET_REGISTER( "mfspr %0,148" )
#define	 SET_ICR(val)	     SET_REGISTER( "mtspr 148,%0", val )
#define	 GET_DER()	     GET_REGISTER( "mfspr %0,149" )
#define	 SET_DER(val)	     SET_REGISTER( "mtspr 149,%0", val )
#define	 GET_COUNTA()	     GET_REGISTER( "mfspr %0,150" )
#define	 SET_COUNTA(val)     SET_REGISTER( "mtspr 150,%0", val )
#define	 GET_COUNTB()	     GET_REGISTER( "mfspr %0,151" )
#define	 SET_COUNTB(val)     SET_REGISTER( "mtspr 151,%0", val )
#define	 GET_CMPE()	     GET_REGISTER( "mfspr %0,152" )
#define	 SET_CMPE(val)	     SET_REGISTER( "mtspr 152,%0", val )
#define	 GET_CMPF()	     GET_REGISTER( "mfspr %0,153" )
#define	 SET_CMPF(val)	     SET_REGISTER( "mtspr 153,%0", val )
#define	 GET_CMPG()	     GET_REGISTER( "mfspr %0,154" )
#define	 SET_CMPG(val)	     SET_REGISTER( "mtspr 154,%0", val )
#define	 GET_CMPH()	     GET_REGISTER( "mfspr %0,155" )
#define	 SET_CMPH(val)	     SET_REGISTER( "mtspr 155,%0", val )
#define  GET_LCTRL1()	     GET_REGISTER( "mfspr %0,156" )
#define	 SET_LCTRL1(val)     SET_REGISTER( "mtspr 156,%0", val )
#define  GET_LCTRL2()	     GET_REGISTER( "mfspr %0,157" )
#define	 SET_LCTRL2(val)     SET_REGISTER( "mtspr 157,%0", val )
#define  GET_ICTRL()	     GET_REGISTER( "mfspr %0,158" )
#define	 SET_ICTRL(val)	     SET_REGISTER( "mtspr 158,%0", val )
#define  GET_BAR()	     GET_REGISTER( "mfspr %0,159" )
#define	 SET_BAR(val)	     SET_REGISTER( "mtspr 159,%0", val )
#define  GET_USPRG0()	     GET_REGISTER( "mfspr %0,256" )
#define	 SET_USPRG0(val)     SET_REGISTER( "mtspr 256,%0", val )
#define  GET_SPRG4_RO()	     GET_REGISTER( "mfspr %0,260" )
#define	 SET_SPRG4_RO(val)   SET_REGISTER( "mtspr 260,%0", val )
#define  GET_SPRG5_RO()	     GET_REGISTER( "mfspr %0,261" )
#define	 SET_SPRG5_RO(val)   SET_REGISTER( "mtspr 261,%0", val )
#define  GET_SPRG6_RO()	     GET_REGISTER( "mfspr %0,262" )
#define	 SET_SPRG6_RO(val)   SET_REGISTER( "mtspr 262,%0", val )
#define  GET_SPRG7_RO()	     GET_REGISTER( "mfspr %0,263" )
#define	 SET_SPRG7_RO(val)   SET_REGISTER( "mtspr 263,%0", val )
#define  GET_SPRG0()	     GET_REGISTER( "mfspr %0,272" )
#define	 SET_SPRG0(val)	     SET_REGISTER( "mtspr 272,%0", val )
#define  GET_SPRG1()	     GET_REGISTER( "mfspr %0,273" )
#define	 SET_SPRG1(val)	     SET_REGISTER( "mtspr 273,%0", val )
#define  GET_SPRG2()	     GET_REGISTER( "mfspr %0,274" )
#define	 SET_SPRG2(val)	     SET_REGISTER( "mtspr 274,%0", val )
#define  GET_SPRG3()	     GET_REGISTER( "mfspr %0,275" )
#define	 SET_SPRG3(val)	     SET_REGISTER( "mtspr 275,%0", val )
#define  GET_SPRG4()	     GET_REGISTER( "mfspr %0,276" )
#define	 SET_SPRG4(val)      SET_REGISTER( "mtspr 276,%0", val )
#define  GET_SPRG5()	     GET_REGISTER( "mfspr %0,277" )
#define	 SET_SPRG5(val)	     SET_REGISTER( "mtspr 277,%0", val )
#define  GET_SPRG6()	     GET_REGISTER( "mfspr %0,278" )
#define	 SET_SPRG6(val)	     SET_REGISTER( "mtspr 278,%0", val )
#define  GET_SPRG7()	     GET_REGISTER( "mfspr %0,279" )
#define	 SET_SPRG7(val)	     SET_REGISTER( "mtspr 279,%0", val )
#define  GET_EAR()	     GET_REGISTER( "mfspr %0,282" )
#define	 SET_EAR(val)	     SET_REGISTER( "mtspr 282,%0", val )
#define  GET_TBL()	     GET_REGISTER( "mfspr %0,284" )
#define	 SET_TBL(val)	     SET_REGISTER( "mtspr 284,%0", val )
#define  GET_TBU()	     GET_REGISTER( "mfspr %0,285" )
#define	 SET_TBU(val)	     SET_REGISTER( "mtspr 285,%0", val )
#define  GET_PVR()	     GET_REGISTER( "mfspr %0,287" )
#define	 SET_PVR(val)	     SET_REGISTER( "mtspr 287,%0", val )
#define  GET_IC_CST()	     GET_REGISTER( "mfspr %0,560" )
#define	 SET_IC_CST(val)     SET_REGISTER( "mtspr 560,%0", val )
#define  GET_IC_ADR()	     GET_REGISTER( "mfspr %0,561" )
#define	 SET_IC_ADR(val)     SET_REGISTER( "mtspr 561,%0", val )
#define  GET_IC_DAT()	     GET_REGISTER( "mfspr %0,562" )
#define	 SET_IC_DAT(val)     SET_REGISTER( "mtspr 562,%0", val )
#define  GET_DC_CST()	     GET_REGISTER( "mfspr %0,568" )
#define	 SET_DC_CST(val)     SET_REGISTER( "mtspr 568,%0", val )
#define  GET_DC_ADR()	     GET_REGISTER( "mfspr %0,569" )
#define	 SET_DC_ADR(val)     SET_REGISTER( "mtspr 569,%0", val )
#define  GET_DC_DAT()	     GET_REGISTER( "mfspr %0,570" )
#define	 SET_DC_DAT(val)     SET_REGISTER( "mtspr 570,%0", val )
#define  GET_DPDR()	     GET_REGISTER( "mfspr %0,630" )
#define	 SET_DPDR(val)	     SET_REGISTER( "mtspr 630,%0", val )
#define  GET_IMMR()	     GET_REGISTER( "mfspr %0,638" )
#define	 SET_IMMR(val)	     SET_REGISTER( "mtspr 638,%0", val )
#define  GET_MI_CTR()	     GET_REGISTER( "mfspr %0,784" )
#define	 SET_MI_CTR(val)     SET_REGISTER( "mtspr 784,%0", val )
#define  GET_MI_AP()	     GET_REGISTER( "mfspr %0,786" )
#define	 SET_MI_AP(val)	     SET_REGISTER( "mtspr 786,%0", val )
#define  GET_MI_EPN()	     GET_REGISTER( "mfspr %0,787" )
#define	 SET_MI_EPN(val)     SET_REGISTER( "mtspr 787,%0", val )
#define  GET_MI_TWC()	     GET_REGISTER( "mfspr %0,789" )
#define	 SET_MI_TWC(val)     SET_REGISTER( "mtspr 789,%0", val )
#define  GET_MI_RPN()	     GET_REGISTER( "mfspr %0,790" )
#define	 SET_MI_RPN(val)     SET_REGISTER( "mtspr 790,%0", val )
#define  GET_MD_CTR()	     GET_REGISTER( "mfspr %0,792" )
#define	 SET_MD_CTR(val)     SET_REGISTER( "mtspr 792,%0", val )
#define  GET_M_CASID()	     GET_REGISTER( "mfspr %0,793" )
#define	 SET_M_CASID(val)    SET_REGISTER( "mtspr 793,%0", val )
#define  GET_MD_AP()	     GET_REGISTER( "mfspr %0,794" )
#define	 SET_MD_AP(val)	     SET_REGISTER( "mtspr ,794%0", val )
#define  GET_MD_EPN()	     GET_REGISTER( "mfspr %0,795" )
#define	 SET_MD_EPN(val)     SET_REGISTER( "mtspr 795,%0", val )
#define  GET_M_TWB()	     GET_REGISTER( "mfspr %0,796" )
#define	 SET_M_TWB(val)	     SET_REGISTER( "mtspr 796,%0", val )
#define  GET_MD_TWC()	     GET_REGISTER( "mfspr %0,797" )
#define	 SET_MD_TWC(val)     SET_REGISTER( "mtspr 797,%0", val )
#define  GET_MD_RPN()	     GET_REGISTER( "mfspr %0,798" )
#define	 SET_MD_RPN(val)     SET_REGISTER( "mtspr 798,%0", val )
#define  GET_M_TW()	     GET_REGISTER( "mfspr %0,799" )
#define	 SET_M_TW(val)	     SET_REGISTER( "mtspr 799,%0", val )
#define  GET_MI_DBCAM()      GET_REGISTER( "mfspr %0,816" )
#define	 SET_MI_DBCAM(val)   SET_REGISTER( "mtspr 816,%0", val )
#define  GET_MI_DBRAM0()     GET_REGISTER( "mfspr %0,817" )
#define	 SET_MI_DBRAM0(val)  SET_REGISTER( "mtspr 817,%0", val )
#define  GET_MI_DBRAM1()     GET_REGISTER( "mfspr %0,818" )
#define	 SET_MI_DBRAM1(val)  SET_REGISTER( "mtspr 818,%0", val )
#define  GET_MD_DBCAM()      GET_REGISTER( "mfspr %0,824" )
#define	 SET_MD_DBCA(val)    SET_REGISTER( "mtspr 824,%0", val )
#define  GET_MD_DBRAM0()     GET_REGISTER( "mfspr %0,825" )
#define	 SET_MD_DBRAM0(val)  SET_REGISTER( "mtspr 825,%0", val )
#define  GET_MD_DBRAM1()     GET_REGISTER( "mfspr %0,826" )
#define	 SET_MD_DBRAM1(val)  SET_REGISTER( "mtspr 826,%0", val )
#define  GET_ZPR()           GET_REGISTER( "mfspr %0,944" )
#define	 SET_ZPR(val)        SET_REGISTER( "mtspr 944,%0", val )
#define  GET_PID()	     GET_REGISTER( "mfspr %0,945" )
#define	 SET_PID(val)	     SET_REGISTER( "mtspr 945,%0", val )
#define  GET_CCR0()	     GET_REGISTER( "mfspr %0,947" )
#define	 SET_CCR0(val)	     SET_REGISTER( "mtspr 947,%0", val )
#define	 GET_IAC3()	     GET_REGISTER( "mfspr %0,948" )
#define	 SET_IAC3(val)	     SET_REGISTER( "mtspr 948,%0", val )
#define	 GET_IAC4()	     GET_REGISTER( "mfspr %0,949" )
#define	 SET_IAC4(val)	     SET_REGISTER( "mtspr 949,%0", val )
#define	 GET_DVC1()	     GET_REGISTER( "mfspr %0,950" )
#define	 SET_DVC1(val)	     SET_REGISTER( "mtspr 950,%0", val )
#define	 GET_DVC2()	     GET_REGISTER( "mfspr %0,951" )
#define	 SET_DVC2(val)	     SET_REGISTER( "mtspr 951,%0", val )
#define	 GET_SGR()	     GET_REGISTER( "mfspr %0,953" )
#define	 SET_SGR(val)	     SET_REGISTER( "mtspr 953,%0", val )
#define	 GET_DCWR()	     GET_REGISTER( "mfspr %0,954" )
#define	 SET_DCWR(val)	     SET_REGISTER( "mtspr 954,%0", val )
#define	 GET_SLER()	     GET_REGISTER( "mfspr %0,955" )
#define	 SET_SLER(val)	     SET_REGISTER( "mtspr 955,%0", val )
#define	 GET_SU0R()	     GET_REGISTER( "mfspr %0,956" )
#define	 SET_SU0R(val)	     SET_REGISTER( "mtspr 956,%0", val )
#define	 GET_DBCR1()	     GET_REGISTER( "mfspr %0,957" )
#define	 SET_DBCR1(val)	     SET_REGISTER( "mtspr 957,%0", val )
#define	 GET_ICDBDR()	     GET_REGISTER( "mfspr %0,979" )
#define	 SET_ICDBDR(val)     SET_REGISTER( "mtspr 979,%0", val )
#define	 GET_ESR()	     GET_REGISTER( "mfspr %0,980" )
#define	 SET_ESR(val)	     SET_REGISTER( "mtspr 980,%0", val )
#define	 GET_DEAR()	     GET_REGISTER( "mfspr %0,981" )
#define	 SET_DEAR(val)	     SET_REGISTER( "mtspr 981,%0", val )
#define	 GET_EVPR()	     GET_REGISTER( "mfspr %0,982" )
#define	 SET_EVPR(val)	     SET_REGISTER( "mtspr 982,%0", val )
#define	 GET_TSR()	     GET_REGISTER( "mfspr %0,984" )
#define	 SET_TSR(val)	     SET_REGISTER( "mtspr 984,%0", val )
#define	 GET_TCR()	     GET_REGISTER( "mfspr %0,986" )
#define	 SET_TCR(val)	     SET_REGISTER( "mtspr 986,%0", val )
#define	 GET_PIT()	     GET_REGISTER( "mfspr %0,987" )
#define	 SET_PIT(val)	     SET_REGISTER( "mtspr 987,%0", val )
#define	 GET_SRR2()	     GET_REGISTER( "mfspr %0,990" )
#define	 SET_SRR2(val)	     SET_REGISTER( "mtspr 990,%0", val )
#define	 GET_SRR3()	     GET_REGISTER( "mfspr %0,991" )
#define	 SET_SRR3(val)	     SET_REGISTER( "mtspr 991,%0", val )
#define	 GET_DBSR()	     GET_REGISTER( "mfspr %0,1008" )
#define	 SET_DBSR(val)	     SET_REGISTER( "mtspr 1008,%0", val )
#define	 GET_DBCR0()	     GET_REGISTER( "mfspr %0,1010" )
#define	 SET_DBCR0(val)	     SET_REGISTER( "mtspr 1010,%0", val )
#define	 GET_IABR()	     GET_REGISTER( "mfspr %0,1010" )
#define	 SET_IABR(val)	     SET_REGISTER( "mtspr 1010,%0", val )
#define	 GET_IAC1()	     GET_REGISTER( "mfspr %0,1012" )
#define	 SET_IAC1(val)	     SET_REGISTER( "mtspr 1012,%0", val )
#define	 GET_IAC2()	     GET_REGISTER( "mfspr %0,1013" )
#define	 SET_IAC2(val)	     SET_REGISTER( "mtspr 1013,%0", val )
#define	 GET_DAC1()	     GET_REGISTER( "mfspr %0,1014" )
#define	 SET_DAC1(val)	     SET_REGISTER( "mtspr 1014,%0", val )
#define	 GET_DAC2()	     GET_REGISTER( "mfspr %0,1015" )
#define	 SET_DAC2(val)	     SET_REGISTER( "mtspr 1015,%0", val )
#define	 GET_DCCR()	     GET_REGISTER( "mfspr %0,1018" )
#define	 SET_DCCR(val)	     SET_REGISTER( "mtspr 1018,%0", val )
#define	 GET_ICCR()	     GET_REGISTER( "mfspr %0,1019" )
#define	 SET_ICCR(val)	     SET_REGISTER( "mtspr 1019,%0", val )

#endif /* _REGS_H */


/*
 * Copyright (c) 2000 William L. Pitts and W. Gerald Hicks
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are freely
 * permitted provided that the above copyright notice and this
 * paragraph and the following disclaimer are duplicated in all
 * such forms.
 *
 * This software is provided "AS IS" and without any express or
 * implied warranties, including, without limitation, the implied
 * warranties of merchantability and fitness for a particular
 * purpose.
 */
