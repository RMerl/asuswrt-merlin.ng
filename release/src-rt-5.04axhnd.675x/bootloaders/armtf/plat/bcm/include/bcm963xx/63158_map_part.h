/*
<:copyright-BRCM:2016:DUAL/GPL:standard

   Copyright (c) 2016 Broadcom 
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

#ifndef __BCM63158_MAP_PART_H
#define __BCM63158_MAP_PART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bcmtypes.h"

#define MEMC_PHYS_BASE		0x80180000  /* DDR IO Buf Control */
#define MEMC_SIZE		0x40000

#define PMC_PHYS_BASE		0x80200000
#define PMC_SIZE		0x81000
#define PMC_OFFSET		0x0000
#define PROC_MON_PHYS_BASE	0x80280000
#define PROC_MON_SIZE		0x1000
#define PROC_MON_OFFSET		0x0000

#define PERF_PHYS_BASE		0xff800000
#define PERF_SIZE		0x3000
#define TIMR_OFFSET		0x0400   /* 64 bit timer registers */
#define WDTIMR0_OFFSET		0x0480
#define WDTIMR1_OFFSET		0x04c0

#define BIUCFG_PHYS_BASE	0x81060000
#define BIUCFG_SIZE		0x3000
#define BIUCFG_OFFSET		0x0000

#define GIC_PHYS_BASE		0x81000000
#define GIC_SIZE		0x10000
#define GIC_OFFSET		0x0000
#define GICD_OFFSET		0x1000
#define GICC_OFFSET		0x2000

#define PMC_BASE		(PMC_PHYS_BASE + PMC_OFFSET)
#define PROC_MON_BASE		(PROC_MON_PHYS_BASE + PROC_MON_OFFSET)

#define TIMR_BASE		(PERF_PHYS_BASE + TIMR_OFFSET)
#define WDTIMR0_BASE		(PERF_PHYS_BASE + WDTIMR0_OFFSET)

#define BIUCFG_BASE		(BIUCFG_PHYS_BASE + BIUCFG_OFFSET)

#ifndef __ASSEMBLER__

/*
 * Power Management Control
 */
typedef struct PmcCtrlReg {
   /* 0x00 */
   uint32 l1Irq4keMask;
   uint32 l1Irq4keStatus;
   uint32 l1IrqMipsMask;
   uint32 l1IrqMipsStatus;
   /* 0x10 */
   uint32 l2IrqGpMask;
   uint32 l2IrqGpStatus;
   uint32 gpTmr0Ctl;
   uint32 gpTmr0Cnt;
   /* 0x20 */
   uint32 gpTmr1Ctl;
   uint32 gpTmr1Cnt;
   uint32 hostMboxIn;
   uint32 hostMboxOut;
   /* 0x30 */
#define PMC_CTRL_GP_FLASH_BOOT_STALL                  0x00000080
   uint32 gpOut;
   uint32 gpIn;
   uint32 gpInIrqMask;
   uint32 gpInIrqStatus;
   /* 0x40 */
   uint32 dmaCtrl;
   uint32 dmaStatus;
   uint32 dma0_3FifoStatus;
   uint32 unused0[3];   /* 0x4c-0x57 */
   /* 0x58 */
   uint32 l1IrqMips1Mask;
   uint32 diagControl;
   /* 0x60 */
   uint32 diagHigh;
   uint32 diagLow;
   uint32 badAddr;
   uint32 addr1WndwMask;
   /* 0x70 */
   uint32 addr1WndwBaseIn;
   uint32 addr1WndwBaseOut;
   uint32 addr2WndwMask;
   uint32 addr2WndwBaseIn;
   /* 0x80 */
   uint32 addr2WndwBaseOut;
   uint32 scratch;
   uint32 tm;
   uint32 softResets;
   /* 0x90 */
   uint32 eb2ubusTimeout;
   uint32 m4keCoreStatus;
   uint32 gpInIrqSense;
   uint32 ubSlaveTimeout;
   /* 0xa0 */
   uint32 diagEn;
   uint32 devTimeout;
   uint32 ubusErrorOutMask;
   uint32 diagCaptStopMask;
   /* 0xb0 */
   uint32 revId;
   uint32 gpTmr2Ctl;
   uint32 gpTmr2Cnt;
   uint32 legacyMode;
   /* 0xc0 */
   uint32 smisbMonitor;
   uint32 diagCtrl;
   uint32 diagStat;
   uint32 diagMask;
   /* 0xd0 */
   uint32 diagRslt;
   uint32 diagCmp;
   uint32 diagCapt;
   uint32 diagCnt;
   /* 0xe0 */
   uint32 diagEdgeCnt;
   uint32 unused1[4];   /* 0xe4-0xf3 */
   /* 0xf4 */
   uint32 iopPeriphBaseAddr;
   uint32 lfsr;
   uint32 unused2;      /* 0xfc-0xff */
} PmcCtrlReg;

typedef struct PmcOutFifoReg {
   uint32 msgCtrl;      /* 0x00 */
   uint32 msgSts;       /* 0x04 */
   uint32 unused[14];   /* 0x08-0x3f */
   uint32 msgData[16];  /* 0x40-0x7c */
} PmcOutFifoReg;

typedef struct PmcInFifoReg {
   uint32 msgCtrl;      /* 0x00 */
   uint32 msgSts;       /* 0x04 */
   uint32 unused[13];   /* 0x08-0x3b */
   uint32 msgLast;      /* 0x3c */
   uint32 msgData[16];  /* 0x40-0x7c */
} PmcInFifoReg;

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
   /* 0x00 */
   uint32 dcacheHit;
   uint32 dcacheMiss;
   uint32 icacheHit;
   uint32 icacheMiss;
   /* 0x10 */
   uint32 instnComplete;
   uint32 wtbMerge;
   uint32 wtbNoMerge;
   uint32 itlbHit;
   /* 0x20 */
   uint32 itlbMiss;
   uint32 dtlbHit;
   uint32 dtlbMiss;
   uint32 jtlbHit;
   /* 0x30 */
   uint32 jtlbMiss;
   uint32 powerSubZone;
   uint32 powerMemPda;
   uint32 freqScalarCtrl;
   /* 0x40 */
   uint32 freqScalarMask;
} PmcPerfPowReg;

typedef struct PmcDQMReg {
   /* 0x00 */
   uint32 cfg;
   uint32 _4keLowWtmkIrqMask;
   uint32 mipsLowWtmkIrqMask;
   uint32 lowWtmkIrqMask;
   /* 0x10 */
   uint32 _4keNotEmptyIrqMask;
   uint32 mipsNotEmptyIrqMask;
   uint32 notEmptyIrqSts;
   uint32 queueRst;
   /* 0x20 */
   uint32 notEmptySts;
   uint32 nextAvailMask;
   uint32 nextAvailQueue;
   uint32 mips1LowWtmkIrqMask;
   /* 0x30 */
   uint32 mips1NotEmptyIrqMask;
   uint32 autoSrcPidInsert;
} PmcDQMReg;

typedef struct PmcCntReg {
   uint32 cntr[10];
   uint32 unused[6]; /* 0x28-0x3f */
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

#define PVTMON_REG
typedef struct {
   uint32 control;      // 0x100
   uint32 reserved;     // 0x104
   uint32 cfg_lo;       // 0x108
   uint32 reserved1;    // 0x10c
   uint32 adc_data;     // 0x110
   uint32 vref_data;    // 0x114
   uint32 reserved2[2]; // 0x118
   uint32 ascan_config; // 0x120
   uint32 reserved3[119];
} pvtmon_regs;

typedef struct Pmc {
   uint32 baseReserved;    /* 0x0000 */
   uint32 unused0[1023];
   PmcCtrlReg ctrl;     /* 0x1000 */

   PmcOutFifoReg outFifo;     /* 0x1100 */
   uint32 unused1[32];     /* 0x1180-0x11ff */
   PmcInFifoReg inFifo;    /* 0x1200 */
   uint32 unused2[32];     /* 0x1280-0x12ff */

   PmcDmaReg dma[2];    /* 0x1300 */
   uint32 unused3[48];     /* 0x1340-0x13ff */

   PmcTokenReg token;      /* 0x1400 */
   uint32 unused4[121];    /* 0x141c-0x15ff */

   PmcPerfPowReg perfPower;   /* 0x1600 */
   uint32 unused5[47];     /* 0x1644-0x16ff */

   uint32 msgId[32];    /* 0x1700 */
   uint32 unused6[32];     /* 0x1780-0x17ff */

   PmcDQMReg dqm;       /* 0x1800 */
   uint32 unused7[50];     /* 0x1838-0x18ff */

   PmcCntReg hwCounter;    /* 0x1900 */
   uint32 unused8[46];     /* 0x1948-0x19ff */

   PmcDqmQCtrlReg dqmQCtrl[32];  /* 0x1a00 */
   PmcDqmQDataReg dqmQData[32];  /* 0x1c00 */
   uint32 unused9[64];     /* 0x1e00-0x1eff */

   uint32 qStatus[32];     /* 0x1f00 */
   uint32 unused10[32];    /* 0x1f80-0x1fff */

   PmcDqmQMibReg qMib;     /* 0x2000 */
   uint32 unused11[1952];     /* 0x2180-0x3ffff */

   uint32 sharedMem[512];    /* 0x4000-0x47ff */
   uint32 unused12[126528];  /* 0x4800-0x80100 */

   pvtmon_regs pvtmon[2];
} Pmc;

#define PMC ((volatile Pmc * const) PMC_BASE)

/*
 * Process Monitor Module
 */
typedef struct PMRingOscillatorControl {
   uint32 control;
   uint32 en_lo;
   uint32 en_mid;
   uint32 en_hi;
   uint32 idle_lo;
   uint32 idle_mid;
   uint32 idle_hi;
} PMRingOscillatorControl;

#define RCAL_0P25UM_HORZ          0
#define RCAL_0P25UM_VERT          1
#define RCAL_0P5UM_HORZ           2
#define RCAL_0P5UM_VERT           3
#define RCAL_1UM_HORZ             4
#define RCAL_1UM_VERT             5
#define PMMISC_RMON_EXT_REG       ((RCAL_1UM_VERT + 1)/2)
#define PMMISC_RMON_VALID_MASK    (0x1<<16)
typedef struct PMMiscControl {
   uint32 gp_out;
   uint32 clock_select;
   uint32 unused[2];
   uint32 misc[4];
} PMMiscControl;

typedef struct PMSSBMasterControl {
   uint32 control;
#define PMC_SSBM_CONTROL_SSB_START  (1<<15)
#define PMC_SSBM_CONTROL_SSB_ADPRE  (1<<13)
#define PMC_SSBM_CONTROL_SSB_EN     (1<<12)
#define PMC_SSBM_CONTROL_SSB_CMD_SHIFT (10)
#define PMC_SSBM_CONTROL_SSB_CMD_MASK  (0x3 << PMC_SSBM_CONTROL_SSB_CMD_SHIFT)
#define PMC_SSBM_CONTROL_SSB_CMD_READ  (2)
#define PMC_SSBM_CONTROL_SSB_CMD_WRITE (1)
#define PMC_SSBM_CONTROL_SSB_ADDR_SHIFT   (0)
#define PMC_SSBM_CONTROL_SSB_ADDR_MASK (0x3ff << PMC_SSBM_CONTROL_SSB_ADDR_SHIFT)
   uint32 wr_data;
   uint32 rd_data;
} PMSSBMasterControl;

typedef struct PMEctrControl {
   uint32 control;
   uint32 interval;
   uint32 thresh_lo;
   uint32 thresh_hi;
   uint32 count;
} PMEctrControl;

typedef struct PMBMaster {
   uint32 ctrl;
#define PMC_PMBM_START     (1 << 31)
#define PMC_PMBM_TIMEOUT   (1 << 30)
#define PMC_PMBM_SLAVE_ERR (1 << 29)
#define PMC_PMBM_BUSY      (1 << 28)
#define PMC_PMBM_Read      (0 << 20)
#define PMC_PMBM_Write     (1 << 20)
   uint32 wr_data;
   uint32 timeout;
   uint32 rd_data;
   uint32 unused[4];
} PMBMaster;

typedef struct PMAPVTMONControl {
   uint32 control;
   uint32 reserved;
   uint32 cfg_lo;
   uint32 cfg_hi;
   uint32 data;
   uint32 vref_data;
   uint32 unused[2];
   uint32 ascan_cfg;
   uint32 warn_temp;
   uint32 reset_temp;
   uint32 temp_value;
   uint32 data1_value;
   uint32 data2_value;
   uint32 data3_value;
} PMAPVTMONControl;

typedef struct PMUBUSCfg {
   uint32 window[8];
   uint32 control;
} PMUBUSCfg;

typedef struct ProcessMonitorRegs {
   uint32 MonitorCtrl;     /* 0x00 */
   uint32 unused0[7];
   PMRingOscillatorControl ROSC; /* 0x20 */
   uint32 unused1;
   PMMiscControl Misc;     /* 0x40 */
   PMSSBMasterControl SSBMaster; /* 0x60 */
   uint32 unused2[5];
   PMEctrControl Ectr;     /* 0x80 */
   uint32 unused3[11];
   PMBMaster PMBM[2];      /* 0xc0 */
   PMAPVTMONControl APvtmonCtrl; /* 0x100 - 0x13b */
   uint32 unused4[41];
   PMUBUSCfg UBUSCfg;      /* 0x1e0 */
} ProcessMonitorRegs;

#define PROCMON ((volatile ProcessMonitorRegs * const) PROC_MON_BASE)

/*
 * Timer
 */
#define TIMER_64BIT
typedef struct Timer {
    uint64        TimerCtl0; /* 0x00 */
    uint64        TimerCtl1; /* 0x08 */
    uint64        TimerCtl2; /* 0x10 */
    uint64        TimerCtl3; /* 0x18 */
#define TIMERENABLE     (1ULL << 63)
#define RSTCNTCLR       (1ULL << 62)
    uint64        TimerCnt0; /* 0x20 */
    uint64        TimerCnt1; /* 0x28 */
    uint64        TimerCnt2; /* 0x30 */
    uint64        TimerCnt3; /* 0x38 */
#define TIMER_COUNT_MASK   0x3FFFFFFFFFFFFFFFULL
    uint32        TimerMask; /* 0x40 */
#define TIMER0EN        0x01
#define TIMER1EN        0x02
#define TIMER2EN        0x04
#define TIMER3EN        0x08
    uint32        TimerInts; /* 0x44 */
#define TIMER0          0x01
#define TIMER1          0x02
#define TIMER2          0x04
#define TIMER3          0x08
    uint32        ResetReason;/* 0x4c */
    uint32        spare[3];   /* 0x50 - 0x5b */
    uint32        reserved1[9]; /* 0x5c - 0x7f */
} Timer;

#define TIMER ((volatile Timer * const) TIMR_BASE)

typedef struct WDTimer {
    uint32        WatchDogDefCount;
    /* Write 0xff00 0x00ff to Start timer
     * Write 0xee00 0x00ee to Stop and re-load default count
     * Read from this register returns current watch dog count
     */
    uint32        WatchDogCtl;

    /* Number of 50-MHz ticks for WD Reset pulse to last */
    uint32        WDResetCount;

    uint32        SoftRst;
#define SOFT_RESET              0x00000001 
    uint32        WDAccessCtl;
} WDTimer;

#define WDTIMER0 ((volatile WDTimer * const) WDTIMR0_BASE)

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
