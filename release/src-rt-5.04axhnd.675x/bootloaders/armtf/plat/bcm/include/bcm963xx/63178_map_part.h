/*
<:copyright-BRCM:2015:DUAL/GPL:standard 

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>

*/

#ifndef __BCM63178_MAP_PART_H
#define __BCM63178_MAP_PART_H
#ifdef __cplusplus
extern "C" {
#endif

#include "bcmtypes.h"

#define MEMC_PHYS_BASE		0x80180000
#define MEMC_SIZE		0x24000

#define PMC_PHYS_BASE		0x80200000
#define PMC_SIZE		0x00200000
#define PMC_OFFSET		0x00100000
#define PROC_MON_OFFSET		0x00100000
#define PMB_OFFSET		0x00120100

#define PERF_PHYS_BASE		0xff800000
#define PERF_SIZE		0x13000
#define TIMR_OFFSET		0x0400
#define WDTIMR0_OFFSET		0x0480
#define WDTIMR1_OFFSET		0x04c0

#define BIUCFG_PHYS_BASE	0x81060000
#define BIUCFG_SIZE		0x3000
#define BIUCFG_OFFSET		0x0000

#define BOOTLUT_PHYS_BASE	0xffff0000
#define BOOTLUT_SIZE		0x1000

#define GIC_PHYS_BASE           0x81000000
#define GIC_SIZE                0x10000
#define GIC_OFFSET              0x0000
#define GICD_OFFSET             0x1000
#define GICC_OFFSET             0x2000

#define PMC_BASE		(PMC_PHYS_BASE + PMC_OFFSET)
#define PMB_BASE		(PMC_PHYS_BASE + PMB_OFFSET)
#define PROC_MON_BASE		(PMC_PHYS_BASE + PROC_MON_OFFSET)

#define WDTIMR0_BASE            (PERF_PHYS_BASE + WDTIMR0_OFFSET)
#define TIMR_BASE               (PERF_PHYS_BASE + TIMR_OFFSET)

#define BIUCFG_BASE             (BIUCFG_PHYS_BASE + BIUCFG_OFFSET)


#ifndef __ASSEMBLER__

/*
 * Power Management Control
 */
typedef struct PmcCtrlReg {
    uint32 gpTmr0Ctl;           /* 0x018 */
    uint32 gpTmr0Cnt;           /* 0x01c */
    uint32 gpTmr1Ctl;           /* 0x020 */
    uint32 gpTmr1Cnt;           /* 0x024 */
    uint32 hostMboxIn;          /* 0x028 */
    uint32 hostMboxOut;         /* 0x02c */
    uint32 reserved[4];         /* 0x030 */
    uint32 dmaCtrl;             /* 0x040 */
    uint32 dmaStatus;           /* 0x044 */
    uint32 dma0_3FifoStatus;    /* 0x048 */
    uint32 reserved1[4];	    /* 0x04c */
    uint32 diagControl;         /* 0x05c */
    uint32 diagHigh;            /* 0x060 */
    uint32 diagLow;             /* 0x064 */
    uint32 reserved8;           /* 0x068 */
    uint32 addr1WndwMask;       /* 0x06c */
    uint32 addr1WndwBaseIn;     /* 0x070 */
    uint32 addr1WndwBaseOut;    /* 0x074 */
    uint32 addr2WndwMask;       /* 0x078 */
    uint32 addr2WndwBaseIn;     /* 0x07c */
    uint32 addr2WndwBaseOut;    /* 0x080 */
    uint32 scratch;             /* 0x084 */
    uint32 reserved9;           /* 0x088 */
    uint32 softResets;          /* 0x08c */
    uint32 reserved2;           /* 0x090 */
    uint32 m4keCoreStatus;      /* 0x094 */
    uint32 reserved3;           /* 0x098 */
    uint32 ubSlaveTimeout;      /* 0x09c */
    uint32 diagEn;              /* 0x0a0 */
    uint32 devTimeout;          /* 0x0a4 */
    uint32 ubusErrorOutMask;    /* 0x0a8 */
    uint32 diagCaptStopMask;    /* 0x0ac */
    uint32 revId;               /* 0x0b0 */
    uint32 reserved4[4];        /* 0x0b4 */
    uint32 diagCtrl;            /* 0x0c4 */
    uint32 diagStat;            /* 0x0c8 */
    uint32 diagMask;            /* 0x0cc */
    uint32 diagRslt;            /* 0x0d0 */
    uint32 diagCmp;             /* 0x0d4 */
    uint32 diagCapt;            /* 0x0d8 */
    uint32 diagCnt;             /* 0x0dc */
    uint32 diagEdgeCnt;         /* 0x0e0 */
    uint32 reserved5[4];	    /* 0x0e4 */
    uint32 smisc_bus_config;    /* 0x0f4 */
    uint32 lfsr;                /* 0x0f8 */
    uint32 dqm_pac_lock;        /* 0x0fc */
    uint32 l1_irq_4ke_mask;     /* 0x100 */
    uint32 l1_irq_4ke_status;   /* 0x104 */
    uint32 l1_irq_mips_mask;    /* 0x108 */
    uint32 l1_irq_mips_status;  /* 0x10c */
    uint32 l1_irq_mips1_mask;   /* 0x110 */
    uint32 reserved6[3];        /* 0x114 */
    uint32 l2_irq_gp_mask;      /* 0x120 */
    uint32 l2_irq_gp_status;    /* 0x124 */
    uint32 l2_irq_gp_set;       /* 0x128 */
    uint32 reserved7;           /* 0x12c */
    uint32 gp_in_irq_mask;      /* 0x130 */
    uint32 gp_in_irq_status;    /* 0x134 */
    uint32 gp_in_irq_set;       /* 0x138 */
    uint32 gp_in_irq_sense;     /* 0x13c */
    uint32 gp_in;               /* 0x140 */
    uint32 gp_out;              /* 0x144 */
} PmcCtrlReg;

typedef struct PmcDmaReg {
	/* 0x00 */
	uint32 src;
	uint32 dest;
	uint32 cmdList;
	uint32 lenCtl;
	/* 0x10 */
	uint32 rsltSrc;
	uint32 rsltDest;
	uint32 rsltHcs;
	uint32 rsltLenStat;
} PmcDmaReg;

typedef struct PmcTokenReg {
	/* 0x00 */
	uint32 bufSize;
	uint32 bufBase;
	uint32 idx2ptrIdx;
	uint32 idx2ptrPtr;
	/* 0x10 */
	uint32 unused[2];
	uint32 bufSize2;
} PmcTokenReg;

typedef struct PmcPerfPowReg {
	uint32 freqScalarCtrl; /* 0x3c */
	uint32 freqScalarMask; /* 0x40 */
} PmcPerfPowReg;

typedef struct PmcDQMPac {
    uint32 dqmPac[32];
} PmcDQMPac;

typedef struct PmcDQMReg {
	uint32 cfg;                     /* 0x1c00 */
	uint32 _4keLowWtmkIrqMask;      /* 0x1c04 */
	uint32 mipsLowWtmkIrqMask;      /* 0x1c08 */
	uint32 lowWtmkIrqMask;          /* 0x1c0c */
	uint32 _4keNotEmptyIrqMask;     /* 0x1c10 */
	uint32 mipsNotEmptyIrqMask;     /* 0x1c14 */
	uint32 notEmptyIrqSts;          /* 0x1c18 */
	uint32 queueRst;                /* 0x1c1c */
	uint32 notEmptySts;             /* 0x1c20 */
	uint32 nextAvailMask;           /* 0x1c24 */
	uint32 nextAvailQueue;          /* 0x1c28 */
	uint32 mips1LowWtmkIrqMask;     /* 0x1c2c */
	uint32 mips1NotEmptyIrqMask;    /* 0x1c30 */
	uint32 autoSrcPidInsert;        /* 0x1c34 */
    uint32 timerIrqStatus;          /* 0x1c38 */
    uint32 timerStatus;             /* 0x1c3c */
    uint32 _4keTimerIrqMask;        /* 0x1c40 */
    uint32 mipsTimerIrqMask;        /* 0x1c44 */
    uint32 mips1TimerIrqMask;       /* 0x1c48 */
} PmcDQMReg;

typedef struct PmcCntReg {
	uint32 cntr[10];
	uint32 unused[6];	/* 0x28-0x3f */
	uint32 cntrIrqMask;
	uint32 cntrIrqSts;
} PmcCntReg;

typedef struct PmcDqmQCtrlReg {
	uint32 size;
	uint32 cfga;
	uint32 cfgb;
	uint32 cfgc;
} PmcDqmQCtrlReg;

typedef struct PmcDqmQDataReg {
	uint32 word[4];
} PmcDqmQDataReg;

typedef struct PmcDqmQMibReg {
	uint32 qNumFull[32];
	uint32 qNumEmpty[32];
	uint32 qNumPushed[32];
} PmcDqmQMibReg;

typedef struct SSBMaster {
    uint32 ssbmControl;     /* 0x0060 */
    uint32 ssbmWrData;      /* 0x0064 */
    uint32 ssbmRdData;      /* 0x0068 */
    uint32 ssbmStatus;      /* 0x006c */
} SSBMaster;

typedef struct PmmReg {
    uint32 memPowerCtrl;            /* 0x0000 */
    uint32 regSecurityConfig;       /* 0x0004 */
} PmmReg;

typedef struct keyholeReg {
    uint32 ctrlSts;
    uint32 wrData;
    uint32 mutex;
    uint32 rdData;
} keyholeReg;

typedef struct PmbBus {
    uint32 config;          /* 0x0100 */
    uint32 arbiter;         /* 0x0104 */
    uint32 timeout;         /* 0x0108 */
    uint32 unused1;         /* 0x010c */
    keyholeReg keyhole[4];  /* 0x0110-0x014f */
    uint32 unused2[44];     /* 0x0150-0x01ff */
    uint32 map[64];         /* 0x0200-0x02ff */ 
}PmbBus;

typedef struct  CoreCtrl {
    uint32  coreEnable;         /* 0x0400 */
    uint32  autoresetControl;   /* 0x0404 */
    uint32  coreIdle;           /* 0x0408 */
    uint32  coreResetCause;     /* 0x040c */
    uint32  memPwrDownCtrl0;    /* 0x0410 */
    uint32  memPwrDownSts0;     /* 0x0414 */
    uint32  memPwrDownCtrl1;    /* 0x0418 */
    uint32  memPwrDownSts1;     /* 0x041c */
    uint32  sysFlg0Status;      /* 0x0420 */
    uint32  sysFlg0Set;         /* 0x0424 */
    uint32  sysFlg0Clear;       /* 0x0428 */
    uint32  unused1;            /* 0x042c */
    uint32  usrFlg0Status;      /* 0x0430 */
    uint32  usrFlg0Set;         /* 0x0434 */
    uint32  usrFlg0Clear;       /* 0x0438 */
    uint32  unused2;            /* 0x043c */
    uint32  subsystemRev;       /* 0x0440 */
    uint32  resetVector;        /* 0x0444 */
} CoreCtrl;

typedef struct  CoreState {
    uint32  sysMbx[8];          /* 0x0480 */
    uint32  usrMbx[8];          /* 0x04a0 */
    uint32  sysMtx[4];          /* 0x04c0 */
    uint32  usrMtx[8];          /* 0x04d0 */
} CoreState;

typedef struct  CoreIntr {
    uint32  irqStatus;          /* 0x0500 */
    uint32  irqSet;             /* 0x0504 */
    uint32  irqClear;           /* 0x0508 */
    uint32  unused1;            /* 0x050c */
    uint32  srqStatus;          /* 0x0510 */
    uint32  srqSet;             /* 0x0514 */
    uint32  srqClear;           /* 0x0518 */
    uint32  unused2;            /* 0x051c */
    uint32  drqStatus;          /* 0x0520 */
    uint32  drqSet;             /* 0x0524 */
    uint32  drqClear;           /* 0x0528 */
    uint32  unused3;            /* 0x052c */
    uint32  frqStatus;          /* 0x0530 */
    uint32  frqSet;             /* 0x0534 */
    uint32  frqClear;           /* 0x0538 */
    uint32  unused4;            /* 0x053c */
    uint32  hostIrqLatched;     /* 0x0540 */
    uint32  hostIrqSet;         /* 0x0544 */
    uint32  hostIrqClear;       /* 0x0548 */
    uint32  hostIrqEnable;      /* 0x054c */
    uint32  obusFaultStatus;    /* 0x0550 */
    uint32  obusFaultClear;     /* 0x0554 */
    uint32  obusFaultAddr;      /* 0x0558 */
} CoreIntr;

typedef struct CoreProfile {
    uint32  mutex;              /* 0x0580 */
    uint32  lastConfPcLo;       /* 0x0584 */
    uint32  lastConfPcHi;       /* 0x0588 */
    uint32  lastPcLo;           /* 0x058c */
    uint32  lastPcHi;           /* 0x0590 */
    uint32  braTargetPc0Lo;     /* 0x0594 */
    uint32  braTargetPc0Hi;     /* 0x0598 */
    uint32  braTargetPc1Lo;     /* 0x059c */
    uint32  braTargetPc1Hi;     /* 0x05a0 */
    uint32  braTargetPc2Lo;     /* 0x05a4 */
    uint32  braTargetPc2Hi;     /* 0x05a8 */
    uint32  braTargetPc3Lo;     /* 0x05ac */
    uint32  braTargetPc3Hi;     /* 0x05b0 */
    uint32  unused[3];          /* 0x05b4-0x05bf */
    uint32  profSampleW[4];     /* 0x05c0 */
} CoreProfile;

typedef struct MaestroMisc {
    CoreCtrl coreCtrl;          /* 0x0400 */
    uint32   unused1[14];       /* 0x0448-0x047f */
    CoreState coreState;        /* 0x0480 */
    uint32   unused2[4];        /* 0x04f0-0x04ff */
    CoreIntr interrupt;         /* 0x0500 */
    uint32   unused3[9];        /* 0x055c-0x057f */
    CoreProfile profile;        /* 0x0580 */
} MaestroMisc;

typedef struct Pmc {
    uint32 unused0[1030];
    PmcCtrlReg ctrl;		            /* 0x1018 */
    uint32 unused1[622];	            /* 0x1148-0x1cff */
    PmcDQMPac dqmPac;                   /* 0x1b00 */
    uint32 unused5[32];                 /* 0x1b80-0x1bff */
    PmcDQMReg dqm;			            /* 0x1c00 */
    uint32 unused6[749];		        /* 0x1c4c-0x27ff */
    uint32 qStatus[32];		            /* 0x2800 */
    uint32 unused7[480];	            /* 0x2880-0x2fff */
    PmcDqmQMibReg qMib;		            /* 0x3000 */
    uint32 unused8[928];	            /* 0x3180-0x3fff */
    PmcDqmQCtrlReg dqmQCtrl[8]; 	    /* 0x4000 */
    uint32 unused9[992];                /* 0x4080-0x4fff */
    PmcDqmQDataReg dqmQData[8]; 	    /* 0x5000 */
} Pmc;
#define PMC ((volatile Pmc * const) PMC_BASE)

typedef struct Procmon {
    uint32 unused00[256];
    MaestroMisc maestroReg;             /* 0x00400 */
    uint32 unused10[32396];             /* 0x005d0-0x1ffff */
    PmmReg  pmm;                        /* 0x20000 */
    uint32 unused11[22];                /* 0x20008-0x2005f */
    SSBMaster ssbMasterCtrl;            /* 0x20060 */
    uint32 unused12[36];                /* 0x20070-0x200ff */
    PmbBus pmb;                         /* 0x20100 */
    uint32 unused13[32576];             /* 0x20300-0x3ffff */
    uint32 qsm[128];                    /* 0x40000-0x401ff */
    uint32 unused14[65408];             /* 0x40200-0x7ffff */
    uint32 dtcm[1024];                  /* 0x80000-0x80fff */
} Procmon;
#define PROCMON ((volatile Procmon * const) PROC_MON_BASE)

typedef struct PMSSBMasterControl {
	uint32 control;
	uint32 wr_data;
	uint32 rd_data;
} PMSSBMasterControl;

typedef struct
{
    uint32  control;
#define PMC_PMBM_START		            (1 << 31)
#define PMC_PMBM_TIMEOUT	            (1 << 30)
#define PMC_PMBM_SLAVE_ERR	            (1 << 29)
#define PMC_PMBM_BUSY		            (1 << 28)
#define PMC_PMBM_BUS_SHIFT		    (20)
#define PMC_PMBM_Read		            (0 << 24)
#define PMC_PMBM_Write		            (1 << 24)
    uint32  wr_data;
    uint32  mutex;
    uint32  rd_data;
} PMB_keyhole_reg;

typedef struct PMBMaster {
    uint32 config;
#define PMB_NUM_REGS_SHIFT (20)
#define PMB_NUM_REGS_MASK  (0x3ff)
    uint32 arbitger;
    uint32 timeout;
    uint32 reserved;
    PMB_keyhole_reg keyhole[4];
    uint32 reserved1[44];
    uint32 map[64];
} PMBMaster;
#define PMB ((volatile PMBMaster * const) PMB_BASE)

/* WatchDog */
typedef struct WDTimer {
    uint32        WatchDogDefCount;/* Write 0xff00 0x00ff to Start timer
     * Write 0xee00 0x00ee to Stop and re-load default count
     *      *      * Read from this register returns current watch dog count
     *           *           */
    uint32        WatchDogCtl;

    /* Number of 50-MHz ticks for WD Reset pulse to last */
    uint32        WDResetCount;

#define SOFT_RESET              0x00000001
    uint32        WDTimerCtl;

    uint32        WDAccessCtl;
} WDTimer;

#define TIMER ((volatile Timer * const) TIMR_BASE)
#define WDTIMER0 ((volatile WDTimer * const) WDTIMR0_BASE)


/* BIU - Bus Interface Unit */
typedef struct BIUCFG_Access {
    uint32 permission;        /* 0x0 */
    uint32 sbox;              /* 0x4 */
    uint32 cpu_defeature;     /* 0x8 */
    uint32 dbg_security;      /* 0xc */
    uint32 rsvd1[32];         /* 0x10 - 0x8f */
    uint64 violation[2];      /* 0x90 - 0x9f */
    uint32 ts_access[2];      /* 0xa0 - 0xa7 */
    uint32 rsvd2[22];         /* 0xa8 - 0xff */
}BIUCFG_Access;




typedef struct BIUCFG_Cluster {
    uint32 permission;        /* 0x0 */
    uint32 config;            /* 0x4 */
    uint32 status;            /* 0x8 */
    uint32 control;           /* 0xc */
    uint32 cpucfg;            /* 0x10 */
    uint32 dbgrom;            /* 0x14 */
    uint32 rsvd1[2];          /* 0x18 - 0x1f */
    uint64 rvbar_addr[4];     /* 0x20 - 0x3f */
    uint32 rsvd2[48];         /* 0x40 - 0xff */
}BIUCFG_Cluster;

typedef struct BIUCFG_Bac {
    uint32 bac_permission;    /* 0x00 */
    uint32 bac_periphbase;    /* 0x04 */
    uint32 rsvd[2];           /* 0x08 - 0x0f */
    uint32 bac_event;         /* 0x10 */
    uint32 rsvd_1[3];         /* 0x14 - 0x1f */
    uint32 bac_ccicfg;        /* 0x20 */
    uint32 bac_cciaddr;       /* 0x24 */
    uint32 rsvd_2[4];         /* 0x28 - 0x37 */
    uint32 bac_ccievs2;       /* 0x38 */
    uint32 bac_ccievs3;       /* 0x3c */
    uint32 bac_ccievs4;       /* 0x40 */
    uint32 rsvd_3[3];         /* 0x44 - 0x4f */
    uint32 bac_ccievm0;       /* 0x50 */
    uint32 bac_ccievm1;       /* 0x54 */
    uint32 rsvd_4[2];         /* 0x58 - 0x5f */
    uint32 bac_dapapbcfg;     /* 0x60 */
    uint32 bac_status;        /* 0x64 */
    uint32 rsvd_5[2];         /* 0x68 - 0x6f */
    uint32 cpu_therm_irq_cfg; /* 0x70 */
    uint32 cpu_therm_threshold_cfg; /* 0x74 */
    uint32 rsvd_6;            /* 0x78 */
    uint32 cpu_therm_temp;    /* 0x7c */
    uint32 rsvd_7[32];        /* 0x80 - 0xff */
} BIUCFG_Bac;

typedef struct BIUCFG_Aux {
    uint32 aux_permission;    /* 0x00 */
    uint32 rsvd[3];           /* 0x04 - 0x0f */
    uint32 c0_clk_control;    /* 0x10 */
    uint32 c0_clk_ramp;       /* 0x14 */
    uint32 c0_clk_pattern;    /* 0x18 */
    uint32 rsvd_1;            /* 0x1c */
    uint32 c1_clk_control;    /* 0x20 */
    uint32 c1_clk_ramp;       /* 0x24 */
    uint32 c1_clk_pattern;    /* 0x28 */
    uint32 rsvd_2[53];        /* 0x2c - 0xff */
} BIUCFG_Aux;

typedef struct BIUCFG {
    BIUCFG_Access access;         /* 0x0 - 0xff*/
    BIUCFG_Cluster cluster[2];    /* 0x100 - 0x2ff*/
    BIUCFG_Bac bac;               /* 0x300 - 0x3ff */
    uint32 anonymous[192];        /* 0x400 - 0x6ff */
    BIUCFG_Aux aux;               /* 0x700 - 0x7ff */
    uint32 anonymous_1[2560];     /* 0x800 - 0x2fff */
}BIUCFG;
#define BIUCFG ((volatile BIUCFG * const) BIUCFG_BASE)

#endif /* __ASSEMBLER__ */

#ifdef __cplusplus
}
#endif

#endif

