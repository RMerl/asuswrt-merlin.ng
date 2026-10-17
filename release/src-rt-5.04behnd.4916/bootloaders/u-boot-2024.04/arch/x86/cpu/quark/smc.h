/* SPDX-License-Identifier: Intel */
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * Ported from Intel released Quark UEFI BIOS
 * QuarkSocPkg/QuarkNorthCluster/MemoryInit/Pei
 */

#ifndef _SMC_H_
#define _SMC_H_

/* System Memory Controller Register Defines */

/* Memory Controller Message Bus Registers Offsets */
#define DRP			0x00
#define DTR0			0x01
#define DTR1			0x02
#define DTR2			0x03
#define DTR3			0x04
#define DTR4			0x05
#define DPMC0			0x06
#define DPMC1			0x07
#define DRFC			0x08
#define DSCH			0x09
#define DCAL			0x0a
#define DRMC			0x0b
#define PMSTS			0x0c
#define DCO			0x0f
#define DSTAT			0x20
#define SSKPD0			0x4a
#define SSKPD1			0x4b
#define DECCCTRL		0x60
#define DECCSTAT		0x61
#define DECCSBECNT		0x62
#define DECCSBECA		0x68
#define DECCSBECS		0x69
#define DECCDBECA		0x6a
#define DECCDBECS		0x6b
#define DFUSESTAT		0x70
#define SCRMSEED		0x80
#define SCRMLO			0x81
#define SCRMHI			0x82

/* DRP register defines */
#define DRP_RKEN0		(1 << 0)
#define DRP_RKEN1		(1 << 1)
#define DRP_PRI64BSPLITEN	(1 << 13)
#define DRP_ADDRMAP_MAP0	(1 << 14)
#define DRP_ADDRMAP_MAP1	(1 << 15)
#define DRP_ADDRMAP_MASK	0x0000c000

/* DTR0 register defines */
#define DTR0_DFREQ_MASK		0x00000003
#define DTR0_TRP_MASK		0x000000f0
#define DTR0_TRCD_MASK		0x00000f00
#define DTR0_TCL_MASK		0x00007000

/* DTR1 register defines */
#define DTR1_TWCL_MASK		0x00000007
#define DTR1_TCMD_MASK		0x00000030
#define DTR1_TWTP_MASK		0x00000f00
#define DTR1_TCCD_12CLK		(1 << 12)
#define DTR1_TCCD_18CLK		(1 << 13)
#define DTR1_TCCD_MASK		0x00003000
#define DTR1_TFAW_MASK		0x000f0000
#define DTR1_TRAS_MASK		0x00f00000
#define DTR1_TRRD_MASK		0x03000000
#define DTR1_TRTP_MASK		0x70000000

/* DTR2 register defines */
#define DTR2_TRRDR_MASK		0x00000007
#define DTR2_TWWDR_MASK		0x00000700
#define DTR2_TRWDR_MASK		0x000f0000

/* DTR3 register defines */
#define DTR3_TWRDR_MASK		0x00000007
#define DTR3_TXXXX_MASK		0x00000070
#define DTR3_TRWSR_MASK		0x00000f00
#define DTR3_TWRSR_MASK		0x0001e000
#define DTR3_TXP_MASK		0x00c00000

/* DTR4 register defines */
#define DTR4_WRODTSTRT_MASK	0x00000003
#define DTR4_WRODTSTOP_MASK	0x00000070
#define DTR4_XXXX1_MASK		0x00000700
#define DTR4_XXXX2_MASK		0x00007000
#define DTR4_ODTDIS		(1 << 15)
#define DTR4_TRGSTRDIS		(1 << 16)

/* DPMC0 register defines */
#define DPMC0_PCLSTO_MASK	0x00070000
#define DPMC0_PREAPWDEN		(1 << 21)
#define DPMC0_DYNSREN		(1 << 23)
#define DPMC0_CLKGTDIS		(1 << 24)
#define DPMC0_DISPWRDN		(1 << 25)
#define DPMC0_ENPHYCLKGATE	(1 << 29)

/* DRFC register defines */
#define DRFC_TREFI_MASK		0x00007000
#define DRFC_REFDBTCLR		(1 << 21)

/* DSCH register defines */
#define DSCH_OOODIS		(1 << 8)
#define DSCH_OOOST3DIS		(1 << 9)
#define DSCH_NEWBYPDIS		(1 << 12)

/* DCAL register defines */
#define DCAL_ZQCINT_MASK	0x00000700
#define DCAL_SRXZQCL_MASK	0x00003000

/* DRMC register defines */
#define DRMC_CKEMODE		(1 << 4)
#define DRMC_ODTMODE		(1 << 12)
#define DRMC_COLDWAKE		(1 << 16)

/* PMSTS register defines */
#define PMSTS_DISR		(1 << 0)

/* DCO register defines */
#define DCO_DRPLOCK		(1 << 0)
#define DCO_CPGCLOCK		(1 << 8)
#define DCO_PMICTL		(1 << 28)
#define DCO_PMIDIS		(1 << 29)
#define DCO_IC			(1 << 31)

/* DECCCTRL register defines */
#define DECCCTRL_SBEEN		(1 << 0)
#define DECCCTRL_DBEEN		(1 << 1)
#define DECCCTRL_ENCBGEN	(1 << 17)

/* DRAM init command */
#define DCMD_MRS1(rnk, dat)	(0 | ((rnk) << 22) | (1 << 3) | ((dat) << 6))
#define DCMD_REF(rnk)		(1 | ((rnk) << 22))
#define DCMD_PRE(rnk)		(2 | ((rnk) << 22))
#define DCMD_PREA(rnk)		(2 | ((rnk) << 22) | (0x400 << 6))
#define DCMD_ACT(rnk, row)	(3 | ((rnk) << 22) | ((row) << 6))
#define DCMD_WR(rnk, col)	(4 | ((rnk) << 22) | ((col) << 6))
#define DCMD_RD(rnk, col)	(5 | ((rnk) << 22) | ((col) << 6))
#define DCMD_ZQCS(rnk)		(6 | ((rnk) << 22))
#define DCMD_ZQCL(rnk)		(6 | ((rnk) << 22) | (0x400 << 6))
#define DCMD_NOP(rnk)		(7 | ((rnk) << 22))

#define DDR3_EMRS1_DIC_40	0
#define DDR3_EMRS1_DIC_34	1

#define DDR3_EMRS1_RTTNOM_0	0
#define DDR3_EMRS1_RTTNOM_60	0x04
#define DDR3_EMRS1_RTTNOM_120	0x40
#define DDR3_EMRS1_RTTNOM_40	0x44
#define DDR3_EMRS1_RTTNOM_20	0x200
#define DDR3_EMRS1_RTTNOM_30	0x204

#define DDR3_EMRS2_RTTWR_60	(1 << 9)
#define DDR3_EMRS2_RTTWR_120	(1 << 10)

/* BEGIN DDRIO Registers */

/* DDR IOs & COMPs */
#define DDRIODQ_BL_OFFSET	0x0800
#define DDRIODQ_CH_OFFSET	((NUM_BYTE_LANES / 2) * DDRIODQ_BL_OFFSET)
#define DDRIOCCC_CH_OFFSET	0x0800
#define DDRCOMP_CH_OFFSET	0x0100

/* CH0-BL01-DQ */
#define DQOBSCKEBBCTL		0x0000
#define DQDLLTXCTL		0x0004
#define DQDLLRXCTL		0x0008
#define DQMDLLCTL		0x000c
#define B0RXIOBUFCTL		0x0010
#define B0VREFCTL		0x0014
#define B0RXOFFSET1		0x0018
#define B0RXOFFSET0		0x001c
#define B1RXIOBUFCTL		0x0020
#define B1VREFCTL		0x0024
#define B1RXOFFSET1		0x0028
#define B1RXOFFSET0		0x002c
#define DQDFTCTL		0x0030
#define DQTRAINSTS		0x0034
#define B1DLLPICODER0		0x0038
#define B0DLLPICODER0		0x003c
#define B1DLLPICODER1		0x0040
#define B0DLLPICODER1		0x0044
#define B1DLLPICODER2		0x0048
#define B0DLLPICODER2		0x004c
#define B1DLLPICODER3		0x0050
#define B0DLLPICODER3		0x0054
#define B1RXDQSPICODE		0x0058
#define B0RXDQSPICODE		0x005c
#define B1RXDQPICODER32		0x0060
#define B1RXDQPICODER10		0x0064
#define B0RXDQPICODER32		0x0068
#define B0RXDQPICODER10		0x006c
#define B01PTRCTL0		0x0070
#define B01PTRCTL1		0x0074
#define B01DBCTL0		0x0078
#define B01DBCTL1		0x007c
#define B0LATCTL0		0x0080
#define B1LATCTL0		0x0084
#define B01LATCTL1		0x0088
#define B0ONDURCTL		0x008c
#define B1ONDURCTL		0x0090
#define B0OVRCTL		0x0094
#define B1OVRCTL		0x0098
#define DQCTL			0x009c
#define B0RK2RKCHGPTRCTRL	0x00a0
#define B1RK2RKCHGPTRCTRL	0x00a4
#define DQRK2RKCTL		0x00a8
#define DQRK2RKPTRCTL		0x00ac
#define B0RK2RKLAT		0x00b0
#define B1RK2RKLAT		0x00b4
#define DQCLKALIGNREG0		0x00b8
#define DQCLKALIGNREG1		0x00bc
#define DQCLKALIGNREG2		0x00c0
#define DQCLKALIGNSTS0		0x00c4
#define DQCLKALIGNSTS1		0x00c8
#define DQCLKGATE		0x00cc
#define B0COMPSLV1		0x00d0
#define B1COMPSLV1		0x00d4
#define B0COMPSLV2		0x00d8
#define B1COMPSLV2		0x00dc
#define B0COMPSLV3		0x00e0
#define B1COMPSLV3		0x00e4
#define DQVISALANECR0TOP	0x00e8
#define DQVISALANECR1TOP	0x00ec
#define DQVISACONTROLCRTOP	0x00f0
#define DQVISALANECR0BL		0x00f4
#define DQVISALANECR1BL		0x00f8
#define DQVISACONTROLCRBL	0x00fc
#define DQTIMINGCTRL		0x010c

/* CH0-ECC */
#define ECCDLLTXCTL		0x2004
#define ECCDLLRXCTL		0x2008
#define ECCMDLLCTL		0x200c
#define ECCB1DLLPICODER0	0x2038
#define ECCB1DLLPICODER1	0x2040
#define ECCB1DLLPICODER2	0x2048
#define ECCB1DLLPICODER3	0x2050
#define ECCB01DBCTL0		0x2078
#define ECCB01DBCTL1		0x207c
#define ECCCLKALIGNREG0		0x20b8
#define ECCCLKALIGNREG1		0x20bc
#define ECCCLKALIGNREG2		0x20c0

/* CH0-CMD */
#define CMDOBSCKEBBCTL		0x4800
#define CMDDLLTXCTL		0x4808
#define CMDDLLRXCTL		0x480c
#define CMDMDLLCTL		0x4810
#define CMDRCOMPODT		0x4814
#define CMDDLLPICODER0		0x4820
#define CMDDLLPICODER1		0x4824
#define CMDCFGREG0		0x4840
#define CMDPTRREG		0x4844
#define CMDCLKALIGNREG0		0x4850
#define CMDCLKALIGNREG1		0x4854
#define CMDCLKALIGNREG2		0x4858
#define CMDPMCONFIG0		0x485c
#define CMDPMDLYREG0		0x4860
#define CMDPMDLYREG1		0x4864
#define CMDPMDLYREG2		0x4868
#define CMDPMDLYREG3		0x486c
#define CMDPMDLYREG4		0x4870
#define CMDCLKALIGNSTS0		0x4874
#define CMDCLKALIGNSTS1		0x4878
#define CMDPMSTS0		0x487c
#define CMDPMSTS1		0x4880
#define CMDCOMPSLV		0x4884
#define CMDBONUS0		0x488c
#define CMDBONUS1		0x4890
#define CMDVISALANECR0		0x4894
#define CMDVISALANECR1		0x4898
#define CMDVISACONTROLCR	0x489c
#define CMDCLKGATE		0x48a0
#define CMDTIMINGCTRL		0x48a4

/* CH0-CLK-CTL */
#define CCOBSCKEBBCTL		0x5800
#define CCRCOMPIO		0x5804
#define CCDLLTXCTL		0x5808
#define CCDLLRXCTL		0x580c
#define CCMDLLCTL		0x5810
#define CCRCOMPODT		0x5814
#define CCDLLPICODER0		0x5820
#define CCDLLPICODER1		0x5824
#define CCDDR3RESETCTL		0x5830
#define CCCFGREG0		0x5838
#define CCCFGREG1		0x5840
#define CCPTRREG		0x5844
#define CCCLKALIGNREG0		0x5850
#define CCCLKALIGNREG1		0x5854
#define CCCLKALIGNREG2		0x5858
#define CCPMCONFIG0		0x585c
#define CCPMDLYREG0		0x5860
#define CCPMDLYREG1		0x5864
#define CCPMDLYREG2		0x5868
#define CCPMDLYREG3		0x586c
#define CCPMDLYREG4		0x5870
#define CCCLKALIGNSTS0		0x5874
#define CCCLKALIGNSTS1		0x5878
#define CCPMSTS0		0x587c
#define CCPMSTS1		0x5880
#define CCCOMPSLV1		0x5884
#define CCCOMPSLV2		0x5888
#define CCCOMPSLV3		0x588c
#define CCBONUS0		0x5894
#define CCBONUS1		0x5898
#define CCVISALANECR0		0x589c
#define CCVISALANECR1		0x58a0
#define CCVISACONTROLCR		0x58a4
#define CCCLKGATE		0x58a8
#define CCTIMINGCTL		0x58ac

/* COMP */
#define CMPCTRL			0x6800
#define SOFTRSTCNTL		0x6804
#define MSCNTR			0x6808
#define NMSCNTRL		0x680c
#define LATCH1CTL		0x6814
#define COMPVISALANECR0		0x681c
#define COMPVISALANECR1		0x6820
#define COMPVISACONTROLCR	0x6824
#define COMPBONUS0		0x6830
#define TCOCNTCTRL		0x683c
#define DQANAODTPUCTL		0x6840
#define DQANAODTPDCTL		0x6844
#define DQANADRVPUCTL		0x6848
#define DQANADRVPDCTL		0x684c
#define DQANADLYPUCTL		0x6850
#define DQANADLYPDCTL		0x6854
#define DQANATCOPUCTL		0x6858
#define DQANATCOPDCTL		0x685c
#define CMDANADRVPUCTL		0x6868
#define CMDANADRVPDCTL		0x686c
#define CMDANADLYPUCTL		0x6870
#define CMDANADLYPDCTL		0x6874
#define CLKANAODTPUCTL		0x6880
#define CLKANAODTPDCTL		0x6884
#define CLKANADRVPUCTL		0x6888
#define CLKANADRVPDCTL		0x688c
#define CLKANADLYPUCTL		0x6890
#define CLKANADLYPDCTL		0x6894
#define CLKANATCOPUCTL		0x6898
#define CLKANATCOPDCTL		0x689c
#define DQSANAODTPUCTL		0x68a0
#define DQSANAODTPDCTL		0x68a4
#define DQSANADRVPUCTL		0x68a8
#define DQSANADRVPDCTL		0x68ac
#define DQSANADLYPUCTL		0x68b0
#define DQSANADLYPDCTL		0x68b4
#define DQSANATCOPUCTL		0x68b8
#define DQSANATCOPDCTL		0x68bc
#define CTLANADRVPUCTL		0x68c8
#define CTLANADRVPDCTL		0x68cc
#define CTLANADLYPUCTL		0x68d0
#define CTLANADLYPDCTL		0x68d4
#define CHNLBUFSTATIC		0x68f0
#define COMPOBSCNTRL		0x68f4
#define COMPBUFFDBG0		0x68f8
#define COMPBUFFDBG1		0x68fc
#define CFGMISCCH0		0x6900
#define COMPEN0CH0		0x6904
#define COMPEN1CH0		0x6908
#define COMPEN2CH0		0x690c
#define STATLEGEN0CH0		0x6910
#define STATLEGEN1CH0		0x6914
#define DQVREFCH0		0x6918
#define CMDVREFCH0		0x691c
#define CLKVREFCH0		0x6920
#define DQSVREFCH0		0x6924
#define CTLVREFCH0		0x6928
#define TCOVREFCH0		0x692c
#define DLYSELCH0		0x6930
#define TCODRAMBUFODTCH0	0x6934
#define CCBUFODTCH0		0x6938
#define RXOFFSETCH0		0x693c
#define DQODTPUCTLCH0		0x6940
#define DQODTPDCTLCH0		0x6944
#define DQDRVPUCTLCH0		0x6948
#define DQDRVPDCTLCH0		0x694c
#define DQDLYPUCTLCH0		0x6950
#define DQDLYPDCTLCH0		0x6954
#define DQTCOPUCTLCH0		0x6958
#define DQTCOPDCTLCH0		0x695c
#define CMDDRVPUCTLCH0		0x6968
#define CMDDRVPDCTLCH0		0x696c
#define CMDDLYPUCTLCH0		0x6970
#define CMDDLYPDCTLCH0		0x6974
#define CLKODTPUCTLCH0		0x6980
#define CLKODTPDCTLCH0		0x6984
#define CLKDRVPUCTLCH0		0x6988
#define CLKDRVPDCTLCH0		0x698c
#define CLKDLYPUCTLCH0		0x6990
#define CLKDLYPDCTLCH0		0x6994
#define CLKTCOPUCTLCH0		0x6998
#define CLKTCOPDCTLCH0		0x699c
#define DQSODTPUCTLCH0		0x69a0
#define DQSODTPDCTLCH0		0x69a4
#define DQSDRVPUCTLCH0		0x69a8
#define DQSDRVPDCTLCH0		0x69ac
#define DQSDLYPUCTLCH0		0x69b0
#define DQSDLYPDCTLCH0		0x69b4
#define DQSTCOPUCTLCH0		0x69b8
#define DQSTCOPDCTLCH0		0x69bc
#define CTLDRVPUCTLCH0		0x69c8
#define CTLDRVPDCTLCH0		0x69cc
#define CTLDLYPUCTLCH0		0x69d0
#define CTLDLYPDCTLCH0		0x69d4
#define FNLUPDTCTLCH0		0x69f0

/* PLL */
#define MPLLCTRL0		0x7800
#define MPLLCTRL1		0x7808
#define MPLLCSR0		0x7810
#define MPLLCSR1		0x7814
#define MPLLCSR2		0x7820
#define MPLLDFT			0x7828
#define MPLLMON0CTL		0x7830
#define MPLLMON1CTL		0x7838
#define MPLLMON2CTL		0x783c
#define SFRTRIM			0x7850
#define MPLLDFTOUT0		0x7858
#define MPLLDFTOUT1		0x785c
#define MASTERRSTN		0x7880
#define PLLLOCKDEL		0x7884
#define SFRDEL			0x7888
#define CRUVISALANECR0		0x78f0
#define CRUVISALANECR1		0x78f4
#define CRUVISACONTROLCR	0x78f8
#define IOSFVISALANECR0		0x78fc
#define IOSFVISALANECR1		0x7900
#define IOSFVISACONTROLCR	0x7904

/* END DDRIO Registers */

/* DRAM Specific Message Bus OpCodes */
#define MSG_OP_DRAM_INIT	0x68
#define MSG_OP_DRAM_WAKE	0xca

#define SAMPLE_SIZE		6

/* must be less than this number to enable early deadband */
#define EARLY_DB		0x12
/* must be greater than this number to enable late deadband */
#define LATE_DB			0x34

#define CHX_REGS		(11 * 4)
#define FULL_CLK		128
#define HALF_CLK		64
#define QRTR_CLK		32

#define MCEIL(num, den)		((uint8_t)((num + den - 1) / den))
#define MMAX(a, b)		((a) > (b) ? (a) : (b))
#define DEAD_LOOP()		for (;;);

#define MIN_RDQS_EYE		10	/* in PI Codes */
#define MIN_VREF_EYE		10	/* in VREF Codes */
/* how many RDQS codes to jump while margining */
#define RDQS_STEP		1
/* how many VREF codes to jump while margining */
#define VREF_STEP		1
/* offset into "vref_codes[]" for minimum allowed VREF setting */
#define VREF_MIN		0x00
/* offset into "vref_codes[]" for maximum allowed VREF setting */
#define VREF_MAX		0x3f
#define RDQS_MIN		0x00	/* minimum RDQS delay value */
#define RDQS_MAX		0x3f	/* maximum RDQS delay value */

/* how many WDQ codes to jump while margining */
#define WDQ_STEP		1

enum {
	B,	/* BOTTOM VREF */
	T	/* TOP VREF */
};

enum {
	L,	/* LEFT RDQS */
	R	/* RIGHT RDQS */
};

/* Memory Options */

/* enable STATIC timing settings for RCVN (BACKUP_MODE) */
#undef BACKUP_RCVN
/* enable STATIC timing settings for WDQS (BACKUP_MODE) */
#undef BACKUP_WDQS
/* enable STATIC timing settings for RDQS (BACKUP_MODE) */
#undef BACKUP_RDQS
/* enable STATIC timing settings for WDQ (BACKUP_MODE) */
#undef BACKUP_WDQ
/* enable *COMP overrides (BACKUP_MODE) */
#undef BACKUP_COMPS
/* enable the RD_TRAIN eye check */
#undef RX_EYE_CHECK

/* enable Host to Memory Clock Alignment */
#define HMC_TEST
/* enable multi-rank support via rank2rank sharing */
#define R2R_SHARING
/* disable signals not used in 16bit mode of DDRIO */
#define FORCE_16BIT_DDRIO

#define PLATFORM_ID		1

void clear_self_refresh(struct mrc_params *mrc_params);
void prog_ddr_timing_control(struct mrc_params *mrc_params);
void prog_decode_before_jedec(struct mrc_params *mrc_params);
void perform_ddr_reset(struct mrc_params *mrc_params);
void ddrphy_init(struct mrc_params *mrc_params);
void perform_jedec_init(struct mrc_params *mrc_params);
void set_ddr_init_complete(struct mrc_params *mrc_params);
void restore_timings(struct mrc_params *mrc_params);
void default_timings(struct mrc_params *mrc_params);
void rcvn_cal(struct mrc_params *mrc_params);
void wr_level(struct mrc_params *mrc_params);
void prog_page_ctrl(struct mrc_params *mrc_params);
void rd_train(struct mrc_params *mrc_params);
void wr_train(struct mrc_params *mrc_params);
void store_timings(struct mrc_params *mrc_params);
void enable_scrambling(struct mrc_params *mrc_params);
void prog_ddr_control(struct mrc_params *mrc_params);
void prog_dra_drb(struct mrc_params *mrc_params);
void perform_wake(struct mrc_params *mrc_params);
void change_refresh_period(struct mrc_params *mrc_params);
void set_auto_refresh(struct mrc_params *mrc_params);
void ecc_enable(struct mrc_params *mrc_params);
void memory_test(struct mrc_params *mrc_params);
void lock_registers(struct mrc_params *mrc_params);

#endif /* _SMC_H_ */
