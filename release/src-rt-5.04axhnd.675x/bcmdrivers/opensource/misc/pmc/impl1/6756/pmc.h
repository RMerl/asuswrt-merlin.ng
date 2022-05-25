/*
<:copyright-BRCM:2020:DUAL/GPL:standard 

   Copyright (c) 2020 Broadcom 
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
#ifndef _PMC_REG_H
#define _PMC_REG_H

/*
 * Power Management Control
 */
typedef struct PmcCtrlReg {
    uint32_t gpTmr0Ctl;           /* 0x018 */
    uint32_t gpTmr0Cnt;           /* 0x01c */
    uint32_t gpTmr1Ctl;           /* 0x020 */
    uint32_t gpTmr1Cnt;           /* 0x024 */
    uint32_t hostMboxIn;          /* 0x028 */
    uint32_t hostMboxOut;         /* 0x02c */
    uint32_t reserved[4];         /* 0x030 */
    uint32_t dmaCtrl;             /* 0x040 */
    uint32_t dmaStatus;           /* 0x044 */
    uint32_t dma0_3FifoStatus;    /* 0x048 */
    uint32_t reserved1[4];	    /* 0x04c */
    uint32_t diagControl;         /* 0x05c */
    uint32_t diagHigh;            /* 0x060 */
    uint32_t diagLow;             /* 0x064 */
    uint32_t reserved8;           /* 0x068 */
    uint32_t addr1WndwMask;       /* 0x06c */
    uint32_t addr1WndwBaseIn;     /* 0x070 */
    uint32_t addr1WndwBaseOut;    /* 0x074 */
    uint32_t addr2WndwMask;       /* 0x078 */
    uint32_t addr2WndwBaseIn;     /* 0x07c */
    uint32_t addr2WndwBaseOut;    /* 0x080 */
    uint32_t scratch;             /* 0x084 */
    uint32_t reserved9;           /* 0x088 */
    uint32_t softResets;          /* 0x08c */
    uint32_t reserved2;           /* 0x090 */
    uint32_t m4keCoreStatus;      /* 0x094 */
    uint32_t reserved3;           /* 0x098 */
    uint32_t ubSlaveTimeout;      /* 0x09c */
    uint32_t diagEn;              /* 0x0a0 */
    uint32_t devTimeout;          /* 0x0a4 */
    uint32_t ubusErrorOutMask;    /* 0x0a8 */
    uint32_t diagCaptStopMask;    /* 0x0ac */
    uint32_t revId;               /* 0x0b0 */
    uint32_t reserved4[4];        /* 0x0b4 */
    uint32_t diagCtrl;            /* 0x0c4 */
    uint32_t diagStat;            /* 0x0c8 */
    uint32_t diagMask;            /* 0x0cc */
    uint32_t diagRslt;            /* 0x0d0 */
    uint32_t diagCmp;             /* 0x0d4 */
    uint32_t diagCapt;            /* 0x0d8 */    
    uint32_t diagCnt;             /* 0x0dc */
    uint32_t diagEdgeCnt;         /* 0x0e0 */
    uint32_t reserved5[4];	    /* 0x0e4 */
    uint32_t smisc_bus_config;    /* 0x0f4 */
    uint32_t lfsr;                /* 0x0f8 */
    uint32_t dqm_pac_lock;        /* 0x0fc */
    uint32_t l1_irq_4ke_mask;     /* 0x100 */
    uint32_t l1_irq_4ke_status;   /* 0x104 */
    uint32_t l1_irq_mips_mask;    /* 0x108 */
    uint32_t l1_irq_mips_status;  /* 0x10c */
    uint32_t l1_irq_mips1_mask;   /* 0x110 */
    uint32_t reserved6[3];        /* 0x114 */
    uint32_t l2_irq_gp_mask;      /* 0x120 */
    uint32_t l2_irq_gp_status;    /* 0x124 */
    uint32_t l2_irq_gp_set;       /* 0x128 */
    uint32_t reserved7;           /* 0x12c */
    uint32_t gp_in_irq_mask;      /* 0x130 */
    uint32_t gp_in_irq_status;    /* 0x134 */
    uint32_t gp_in_irq_set;       /* 0x138 */
    uint32_t gp_in_irq_sense;     /* 0x13c */
    uint32_t gp_in;               /* 0x140 */
    uint32_t gp_out;              /* 0x144 */
} PmcCtrlReg;

typedef struct PmcDmaReg {
	/* 0x00 */
	uint32_t src;
	uint32_t dest;
	uint32_t cmdList;
	uint32_t lenCtl;
	/* 0x10 */
	uint32_t rsltSrc;
	uint32_t rsltDest;
	uint32_t rsltHcs;
	uint32_t rsltLenStat;
} PmcDmaReg;

typedef struct PmcTokenReg {
	/* 0x00 */
	uint32_t bufSize;
	uint32_t bufBase;
	uint32_t idx2ptrIdx;
	uint32_t idx2ptrPtr;
	/* 0x10 */
	uint32_t unused[2];
	uint32_t bufSize2;
} PmcTokenReg;

typedef struct PmcPerfPowReg {
	uint32_t freqScalarCtrl; /* 0x3c */
	uint32_t freqScalarMask; /* 0x40 */
} PmcPerfPowReg;

typedef struct PmcDQMPac {
    uint32_t dqmPac[32];
} PmcDQMPac;

typedef struct PmcDQMReg {
	uint32_t cfg;                     /* 0x1c00 */
	uint32_t _4keLowWtmkIrqMask;      /* 0x1c04 */
	uint32_t mipsLowWtmkIrqMask;      /* 0x1c08 */
	uint32_t lowWtmkIrqMask;          /* 0x1c0c */
	uint32_t _4keNotEmptyIrqMask;     /* 0x1c10 */
	uint32_t mipsNotEmptyIrqMask;     /* 0x1c14 */
	uint32_t notEmptyIrqSts;          /* 0x1c18 */
	uint32_t queueRst;                /* 0x1c1c */
	uint32_t notEmptySts;             /* 0x1c20 */
	uint32_t nextAvailMask;           /* 0x1c24 */
	uint32_t nextAvailQueue;          /* 0x1c28 */
	uint32_t mips1LowWtmkIrqMask;     /* 0x1c2c */
	uint32_t mips1NotEmptyIrqMask;    /* 0x1c30 */
	uint32_t autoSrcPidInsert;        /* 0x1c34 */
    uint32_t timerIrqStatus;          /* 0x1c38 */
    uint32_t timerStatus;             /* 0x1c3c */
    uint32_t _4keTimerIrqMask;        /* 0x1c40 */
    uint32_t mipsTimerIrqMask;        /* 0x1c44 */
    uint32_t mips1TimerIrqMask;       /* 0x1c48 */
} PmcDQMReg;

typedef struct PmcCntReg {
	uint32_t cntr[10];
	uint32_t unused[6];	/* 0x28-0x3f */
	uint32_t cntrIrqMask;
	uint32_t cntrIrqSts;
} PmcCntReg;

typedef struct PmcDqmQCtrlReg {
	uint32_t size;
	uint32_t cfga;
	uint32_t cfgb;
	uint32_t cfgc;
} PmcDqmQCtrlReg;

typedef struct PmcDqmQDataReg {
	uint32_t word[4];
} PmcDqmQDataReg;

typedef struct PmcDqmQMibReg {
	uint32_t qNumFull[32];
	uint32_t qNumEmpty[32];
	uint32_t qNumPushed[32];
} PmcDqmQMibReg;

typedef struct SSBMaster {
    uint32_t ssbmControl;     /* 0x0060 */
    uint32_t ssbmWrData;      /* 0x0064 */
    uint32_t ssbmRdData;      /* 0x0068 */
    uint32_t ssbmStatus;      /* 0x006c */
} SSBMaster;

typedef struct PmmReg {
    uint32_t memPowerCtrl;            /* 0x0000 */
    uint32_t regSecurityConfig;       /* 0x0004 */
} PmmReg;

typedef struct keyholeReg {
    uint32_t control;
#define PMC_PMBM_START		            (1 << 31)
#define PMC_PMBM_TIMEOUT	            (1 << 30)
#define PMC_PMBM_SLAVE_ERR	            (1 << 29)
#define PMC_PMBM_BUSY		            (1 << 28)
#define PMC_PMBM_BUS_SHIFT              (20)
#define PMC_PMBM_Read		            (0 << 24)
#define PMC_PMBM_Write		            (1 << 24)
    uint32_t wr_data;
    uint32_t mutex;
    uint32_t rd_data;
} keyholeReg;

typedef struct PmbBus {
    uint32_t config;          /* 0x0100 */
#define PMB_NUM_REGS_SHIFT (20)
#define PMB_NUM_REGS_MASK  (0x3ff)
    uint32_t arbiter;         /* 0x0104 */
    uint32_t timeout;         /* 0x0108 */
    uint32_t unused1;         /* 0x010c */
    keyholeReg keyhole[4];  /* 0x0110-0x014f */
    uint32_t unused2[44];     /* 0x0150-0x01ff */
    uint32_t map[64];         /* 0x0200-0x02ff */ 
}PmbBus;

typedef struct  CoreCtrl {
    uint32_t  coreEnable;         /* 0x0400 */
    uint32_t  autoresetControl;   /* 0x0404 */
    uint32_t  coreIdle;           /* 0x0408 */
    uint32_t  coreResetCause;     /* 0x040c */
    uint32_t  memPwrDownCtrl0;    /* 0x0410 */
    uint32_t  memPwrDownSts0;     /* 0x0414 */
    uint32_t  memPwrDownCtrl1;    /* 0x0418 */
    uint32_t  memPwrDownSts1;     /* 0x041c */
    uint32_t  sysFlg0Status;      /* 0x0420 */
    uint32_t  sysFlg0Set;         /* 0x0424 */
    uint32_t  sysFlg0Clear;       /* 0x0428 */
    uint32_t  unused1;            /* 0x042c */
    uint32_t  usrFlg0Status;      /* 0x0430 */
    uint32_t  usrFlg0Set;         /* 0x0434 */
    uint32_t  usrFlg0Clear;       /* 0x0438 */
    uint32_t  unused2;            /* 0x043c */
    uint32_t  subsystemRev;       /* 0x0440 */
    uint32_t  resetVector;        /* 0x0444 */
} CoreCtrl;

typedef struct  CoreState {
    uint32_t  sysMbx[8];          /* 0x0480 */
    uint32_t  usrMbx[8];          /* 0x04a0 */
    uint32_t  sysMtx[4];          /* 0x04c0 */
    uint32_t  usrMtx[8];          /* 0x04d0 */
} CoreState;

typedef struct  CoreIntr {
    uint32_t  irqStatus;          /* 0x0500 */
    uint32_t  irqSet;             /* 0x0504 */
    uint32_t  irqClear;           /* 0x0508 */
    uint32_t  unused1;            /* 0x050c */
    uint32_t  srqStatus;          /* 0x0510 */
    uint32_t  srqSet;             /* 0x0514 */
    uint32_t  srqClear;           /* 0x0518 */
    uint32_t  unused2;            /* 0x051c */
    uint32_t  drqStatus;          /* 0x0520 */
    uint32_t  drqSet;             /* 0x0524 */
    uint32_t  drqClear;           /* 0x0528 */
    uint32_t  unused3;            /* 0x052c */
    uint32_t  frqStatus;          /* 0x0530 */
    uint32_t  frqSet;             /* 0x0534 */
    uint32_t  frqClear;           /* 0x0538 */
    uint32_t  unused4;            /* 0x053c */
    uint32_t  hostIrqLatched;     /* 0x0540 */
    uint32_t  hostIrqSet;         /* 0x0544 */
    uint32_t  hostIrqClear;       /* 0x0548 */
    uint32_t  hostIrqEnable;      /* 0x054c */
    uint32_t  obusFaultStatus;    /* 0x0550 */
    uint32_t  obusFaultClear;     /* 0x0554 */
    uint32_t  obusFaultAddr;      /* 0x0558 */
} CoreIntr;

typedef struct CoreProfile {
    uint32_t  mutex;              /* 0x0580 */
    uint32_t  lastConfPcLo;       /* 0x0584 */
    uint32_t  lastConfPcHi;       /* 0x0588 */
    uint32_t  lastPcLo;           /* 0x058c */
    uint32_t  lastPcHi;           /* 0x0590 */
    uint32_t  braTargetPc0Lo;     /* 0x0594 */
    uint32_t  braTargetPc0Hi;     /* 0x0598 */
    uint32_t  braTargetPc1Lo;     /* 0x059c */
    uint32_t  braTargetPc1Hi;     /* 0x05a0 */
    uint32_t  braTargetPc2Lo;     /* 0x05a4 */
    uint32_t  braTargetPc2Hi;     /* 0x05a8 */
    uint32_t  braTargetPc3Lo;     /* 0x05ac */
    uint32_t  braTargetPc3Hi;     /* 0x05b0 */
    uint32_t  unused[3];          /* 0x05b4-0x05bf */
    uint32_t  profSampleW[4];     /* 0x05c0 */
} CoreProfile;

typedef struct MaestroMisc {
    CoreCtrl coreCtrl;          /* 0x0400 */
    uint32_t   unused1[14];       /* 0x0448-0x047f */
    CoreState coreState;        /* 0x0480 */
    uint32_t   unused2[4];        /* 0x04f0-0x04ff */
    CoreIntr interrupt;         /* 0x0500 */
    uint32_t   unused3[9];        /* 0x055c-0x057f */
    CoreProfile profile;        /* 0x0580 */
} MaestroMisc;

typedef struct Pmc {
    PmcCtrlReg ctrl;		            /* 0x1018 */
    uint32_t unused1[622];	            /* 0x1148-0x1cff */
    PmcDQMPac dqmPac;                   /* 0x1b00 */
    uint32_t unused5[32];                 /* 0x1b80-0x1bff */
    PmcDQMReg dqm;			            /* 0x1c00 */
    uint32_t unused6[749];		        /* 0x1c4c-0x27ff */
    uint32_t qStatus[32];		            /* 0x2800 */
    uint32_t unused7[480];	            /* 0x2880-0x2fff */
    PmcDqmQMibReg qMib;		            /* 0x3000 */
    uint32_t unused8[928];	            /* 0x3180-0x3fff */
    PmcDqmQCtrlReg dqmQCtrl[8]; 	    /* 0x4000 */
    uint32_t unused9[992];                /* 0x4080-0x4fff */
    PmcDqmQDataReg dqmQData[8]; 	    /* 0x5000 */
} Pmc;

typedef struct Procmon {
    PmmReg  pmm;                        /* 0x20000 */
    uint32_t unused11[22];                /* 0x20008-0x2005f */
    SSBMaster ssbMasterCtrl;            /* 0x20060 */
    uint32_t unused12[36];                /* 0x20070-0x200ff */
    PmbBus pmb;                         /* 0x20100 */
} Procmon;

#define BAC_CPU_REG_OFFSET          0x81060300
#define BAC_CPU_REG_SIZE            0x100
#define BAC_CPU_THERM_OFFSET        0x7c
#define PCMBUS_PHYS_BASE            0x83010A00

#define EARLY_PMC_BASE              0x80301018
#define EARLY_PMC_SIZE              0x5080
#endif
