/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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

#ifndef __BCM4908_MAP_PART_H
#define __BCM4908_MAP_PART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bcmtypes.h"

#define MEMC_PHYS_BASE		0x80018000  /* DDR IO Buf Control */
#define MEMC_SIZE		0x4000

#define PMC_PHYS_BASE		0x80200000
#define PMC_SIZE		0x5000
#define PMC_OFFSET		0x0000
#define PROC_MON_PHYS_BASE	0x80280000
#define PROC_MON_SIZE		0x1000
#define PROC_MON_OFFSET		0x0000

/* Perf block base address and size */
#define PERF_PHYS_BASE		0xff800000
#define PERF_SIZE		0x3000
#define TIMR_OFFSET		0x0400   /* timer registers */

#define URB_PHYS_BASE		0x81060000
#define URB_SIZE		0x4000
#define URB_OFFSET		0x0000
#define URB_BIUARCH_OFFSET	0x1000
#define URB_BIUCTRL_OFFSET	0x2000

#define GIC_PHYS_BASE		0x81000000
#define GIC_SIZE		0x10000
#define GIC_OFFSET		0x0000
#define GICD_OFFSET		0x1000
#define GICC_OFFSET		0x2000

#define PMC_BASE		(PMC_PHYS_BASE + PMC_OFFSET)
#define PROC_MON_BASE		(PROC_MON_PHYS_BASE + PROC_MON_OFFSET)

#define URB_BASE		(URB_PHYS_BASE + URB_OFFSET)
#define BIUARCH_BASE		(URB_PHYS_BASE + URB_BIUARCH_OFFSET)
#define BIUCTRL_BASE		(URB_PHYS_BASE + URB_BIUCTRL_OFFSET)

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
   PMAPVTMONControl APvtmonCtrl; /* 0x100 */
   uint32 unused4[9];
   PMUBUSCfg UBUSCfg;      /* 0x160 */
} ProcessMonitorRegs;

#define PROCMON ((volatile ProcessMonitorRegs * const) PROC_MON_BASE)

/*
 * URB UBUS RBUS Block register
 */
typedef struct BIUArchRegion {
    uint32 addr_ulimit;
    uint32 addr_llimit;
    uint32 permission;
    uint32 access_right_ctrl;
} BIUArchRegion;

typedef struct BIUArch {
    BIUArchRegion region[8];    /* 0x0 */
    uint32 unused[95];          /* 0x80 */
    uint32 scratch;             /* 0x1fc */    
} BIUArch;

#define BIUARCH ((volatile BIUArch * const) BIUARCH_BASE)

typedef struct BIUCpuBusRange {
#define ULIMIT_SHIFT 4
#define BUSNUM_MASK 0x0000000FU

#define BUSNUM_UBUS 1
#define BUSNUM_RBUS 2
#define BUSNUM_RSVD 3
#define BUSNUM_MCP0 4
#define BUSNUM_MCP1 5
#define BUSNUM_MCP2 6

    uint32 ulimit;
    uint32 llimit;
} BIUCpuBusRange;

typedef struct BIUCpuAccessRightViol {
    uint32 addr;
    uint32 upper_addr;
    uint32 detail_addr;
} BIUCpuAccessRightViol;

typedef struct BIUCpuBPCMAVS {
    uint32 bpcm_id;
    uint32 bpcm_capability;
} BIUCpuBPCMAVS;

typedef struct BIUCtrl {
    BIUCpuBusRange bus_range[11];                   /* 0x0 */
    uint32 secure_reset_hndshake;
    uint32 secure_soft_reset;
    BIUCpuAccessRightViol access_right_viol[2];     /* 0x60 */
    uint32 rac_cfg0;
    uint32 rac_cfg1;
    uint32 rac_cfg2;                                /* 0x80 */
    uint32 rac_flush;        
    uint32 power_cfg;
#define BIU_CPU_CTRL_PWR_CFG_CPU0_BPCM_INIT_ON_SHIFT 4
#define BIU_CPU_CTRL_PWR_CFG_CPU0_BPCM_INIT_ON      (0x1<<BIU_CPU_CTRL_PWR_CFG_CPU0_BPCM_INIT_ON_SHIFT)
    uint32 reset_cfg;
#define BIU_CPU_CTRL_RST_CFG_CPU0_RESET_SHIFT       0
#define BIU_CPU_CTRL_RST_CFG_CPU0_RESET             (0x1<<BIU_CPU_CTRL_RST_CFG_CPU0_RESET_SHIFT)
    uint32 clock_cfg;                               /* 0x90 */
#define BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_SHIFT    8
#define BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_MASK     (0xf<<BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_SHIFT)
#define BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_DIV1     (0<<BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_SHIFT)
#define BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_DIV2     (1<<BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_SHIFT)
#define BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_DIV3     (2<<BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_SHIFT)
#define BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_DIV4     (3<<BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_SHIFT)
#define BIU_CPU_CTRL_CLK_CFG_SAFE_CLOCK_MODE_SHIFT  4
#define BIU_CPU_CTRL_CLK_CFG_SAFE_CLOCK_MODE_MASK   (1<<BIU_CPU_CTRL_CLK_CFG_SAFE_CLOCK_MODE_SHIFT)
#define BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_SHIFT        0
#define BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_MASK         (0xf<<BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_SHIFT)
#define BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_DIV1         (0<<BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_SHIFT)
#define BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_DIV2         (1<<BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_SHIFT)
#define BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_DIV4         (2<<BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_SHIFT)
#define BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_DIV8         (3<<BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_SHIFT)
#define BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_DIV16        (4<<BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_SHIFT)
    uint32 cluster_clk_ctrl[2];
    uint32 cluster_clk_pattern[2];
    uint32 cluster_clk_ramp[2];
    uint32 misc_cfg;
    uint32 credit;                                  /* 0xb0 */
    uint32 mcp_flow;
    uint32 periphbase_gic;
    uint32 periphbase_gic_web;
    uint32 wfx_state;                               /* 0xc0 */
    uint32 cpu_pwr_zone_ctrl[8];
#define BIU_CPU_CTRL_PWR_ZONE_CTRL_ZONE_RESET_SHIFT 31
#define BIU_CPU_CTRL_PWR_ZONE_CTRL_ZONE_RESET       (1<<BIU_CPU_CTRL_PWR_ZONE_CTRL_ZONE_RESET_SHIFT)
#define BIU_CPU_CTRL_PWR_ZONE_CTRL_PWR_UP_REQ_SHIFT 10
#define BIU_CPU_CTRL_PWR_ZONE_CTRL_PWR_UP_REQ       (1<<BIU_CPU_CTRL_PWR_ZONE_CTRL_PWR_UP_REQ_SHIFT)
#define BIU_CPU_CTRL_PWR_ZONE_CTRL_PWR_DN_REQ_SHIFT 9
#define BIU_CPU_CTRL_PWR_ZONE_CTRL_PWR_DN_REQ       (1<<BIU_CPU_CTRL_PWR_ZONE_CTRL_PWR_DN_REQ_SHIFT)
    uint32 cpu_pwr_zone_cfg1[8];
    uint32 cpu_pwr_zone_cfg2[8];
    uint32 c0l2_pwr_zone_ctrl;                      /* 0x124 */
    uint32 c0l2_pwr_zone_cfg1;
    uint32 c0l2_pwr_zone_cfg2;
    uint32 c1l2_pwr_zone_ctrl;
    uint32 c1l2_pwr_zone_cfg1;
    uint32 c1l2_pwr_zone_cfg2;
    uint32 sysif_pwr_zone_ctrl;
    uint32 sysif_pwr_zone_cfg1;
    uint32 sysif_pwr_zone_cfg2;
    BIUCpuBPCMAVS cpu_bpcm_avs[8];                  /* 0x148 */
    BIUCpuBPCMAVS l2biu_bpcm_avs[3];                /* 0x188 */
    uint32 therm_throttle_temp;                     /* 0x1a0 */
    uint32 term_throttle_irq_cfg;
    uint32 therm_irq_high;
    uint32 therm_irq_low;
    uint32 therm_misc_threshold;                    /* 0x1b0 */
    uint32 therm_irq_misc;
    uint32 defeature;
    uint32 defeature2;
    uint32 defeature_key;                           /* 0x1c0 */
    uint32 debug_rom_addr;
    uint32 debug_tracectrl;
    uint32 axi_cfg;
    uint32 revision;                                /* 0x1d0 */
    uint32 patchlevel;
    uint32 ubus_cfg;                                /* 0x1d8 */
    uint32 ubus_cfg_window[8];
    uint32 power_state;
    uint32 phys_config;                             /* 0x200 */
    uint32 unused[126];                             /* 0x204 */
    uint32 scratch;                                 /* 0x3fc */
} BIUCtrl;

#define BIUCTRL ((volatile BIUCtrl * const) BIUCTRL_BASE)

typedef struct URB_Ctrl {
    uint32 unused0[1024];                           /* 0x0000 */
    BIUArch arch;                                   /* 0x1000 */
    uint32 unused1[896];                            /* 0x1200 */
    BIUCtrl cpu_ctrl;                               /* 0x2000 */
} URB_Ctrl;

#define URB_CTRL ((volatile URB_Ctrl *const) URB_BASE)

#endif /* __ASSEMBLER__ */

#ifdef __cplusplus
}
#endif

#endif

