/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _63148_PMC_H
#define _63148_PMC_H

#ifdef __ASSEMBLER__
#define PMC_BASE                               0x80400000
#define PROC_MON_BASE                          0x80480000
#define PMBM0_BASE                             0x804800c0
#define PMBM1_BASE                             0x804800e0

/*
#####################################################################
# PMC registers
#####################################################################
*/

#define PMC_CONTROL_HOST_MBOX_IN               0x1028
#define PMC_CONTROL_HOST_MBOX_OUT              0x102c

#define PMC_DQM_BASE                           (PMC_BASE+0x1800)
#define PMC_DQM_NOT_EMPTY_IRQ_STS              0x18
#define PMC_DQM_QUEUE_RST                      0x1c
#define PMC_DQM_NOT_EMPTY_STS                  0x20

#define PMC_DQM_QUEUE_CNTRL_BASE               (PMC_BASE+0x1a00)
#define PMC_DQM_QUEUE_DATA_BASE                (PMC_BASE+0x1c00)

#define PMC_DQM_QUEUE_DATA_HOST_TO_PMC         0x0
#define PMC_DQM_QUEUE_DATA_PMC_TO_HOST         0x10

/*
#####################################################################
# PMB registers
#####################################################################
*/
#define PMB_CNTRL                             0x0
#define PMB_CNTRL_START_SHIFT                 31
#define PMB_CNTRL_START_MASK                  (1<<PMB_CNTRL_START_SHIFT)
#define PMB_CNTRL_TIMEOUT_ERR_SHIFT           30
#define PMB_CNTRL_TIMEOUT_ERR_MASK            (1<<PMB_CNTRL_TIMEOUT_ERR_SHIFT)
#define PMB_CNTRL_SLAVE_ERR_SHIFT             29
#define PMB_CNTRL_SLAVE_ERR_MASK              (1<<PMB_CNTRL_SLAVE_ERR_SHIFT)
#define PMB_CNTRL_BUSY_SHIFT                  28
#define PMB_CNTRL_BUSY_MASK                   (1<<PMB_CNTRL_BUSY_SHIFT)
#define PMB_CNTRL_CMD_SHIFT                   20
#define PMB_CNTRL_CMD_MASK                    (0xf<<PMB_CNTRL_CMD_SHIFT)
#define PMB_CNTRL_ADDR_SHIFT                  0
#define PMB_CNTRL_ADDR_MASK                   (0xfffff<<PMB_CNTRL_ADDR_SHIFT)

#define PMB_WR_DATA                           0x4
#define PMB_TIMEOUT                           0x8
#define PMB_RD_DATA                           0xc

#else

/*
 * Power Management Control
 */
typedef struct PmcCtrlReg {
	/* 0x00 */
	uint32_t l1Irq4keMask;
	uint32_t l1Irq4keStatus;
	uint32_t l1IrqMipsMask;
	uint32_t l1IrqMipsStatus;
	/* 0x10 */
	uint32_t l2IrqGpMask;
	uint32_t l2IrqGpStatus;
	uint32_t gpTmr0Ctl;
	uint32_t gpTmr0Cnt;
	/* 0x20 */
	uint32_t gpTmr1Ctl;
	uint32_t gpTmr1Cnt;
	uint32_t hostMboxIn;
	uint32_t hostMboxOut;
	/* 0x30 */
#define PMC_CTRL_GP_FLASH_BOOT_STALL                  0x00000080
	uint32_t gpOut;
	uint32_t gpIn;
	uint32_t gpInIrqMask;
	uint32_t gpInIrqStatus;
	/* 0x40 */
	uint32_t dmaCtrl;
	uint32_t dmaStatus;
	uint32_t dma0_3FifoStatus;
	uint32_t unused0[3];	/* 0x4c-0x57 */
	/* 0x58 */
	uint32_t l1IrqMips1Mask;
	uint32_t diagControl;
	/* 0x60 */
	uint32_t diagHigh;
	uint32_t diagLow;
	uint32_t badAddr;
	uint32_t addr1WndwMask;
	/* 0x70 */
	uint32_t addr1WndwBaseIn;
	uint32_t addr1WndwBaseOut;
	uint32_t addr2WndwMask;
	uint32_t addr2WndwBaseIn;
	/* 0x80 */
	uint32_t addr2WndwBaseOut;
	uint32_t scratch;
	uint32_t tm;
	uint32_t softResets;
	/* 0x90 */
	uint32_t eb2ubusTimeout;
	uint32_t m4keCoreStatus;
	uint32_t gpInIrqSense;
	uint32_t ubSlaveTimeout;
	/* 0xa0 */
	uint32_t diagEn;
	uint32_t devTimeout;
	uint32_t ubusErrorOutMask;
	uint32_t diagCaptStopMask;
	/* 0xb0 */
	uint32_t revId;
	uint32_t gpTmr2Ctl;
	uint32_t gpTmr2Cnt;
	uint32_t legacyMode;
	/* 0xc0 */
	uint32_t smisbMonitor;
	uint32_t diagCtrl;
	uint32_t diagStat;
	uint32_t diagMask;
	/* 0xd0 */
	uint32_t diagRslt;
	uint32_t diagCmp;
	uint32_t diagCapt;
	uint32_t diagCnt;
	/* 0xe0 */
	uint32_t diagEdgeCnt;
	uint32_t unused1[4];	/* 0xe4-0xf3 */
	/* 0xf4 */
	uint32_t iopPeriphBaseAddr;
	uint32_t lfsr;
	uint32_t unused2;	/* 0xfc-0xff */
} PmcCtrlReg;

typedef struct PmcOutFifoReg {
	uint32_t msgCtrl;	/* 0x00 */
	uint32_t msgSts;	/* 0x04 */
	uint32_t unused[14];	/* 0x08-0x3f */
	uint32_t msgData[16];	/* 0x40-0x7c */
} PmcOutFifoReg;

typedef struct PmcInFifoReg {
	uint32_t msgCtrl;	/* 0x00 */
	uint32_t msgSts;	/* 0x04 */
	uint32_t unused[13];	/* 0x08-0x3b */
	uint32_t msgLast;	/* 0x3c */
	uint32_t msgData[16];	/* 0x40-0x7c */
} PmcInFifoReg;

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
	/* 0x00 */
	uint32_t dcacheHit;
	uint32_t dcacheMiss;
	uint32_t icacheHit;
	uint32_t icacheMiss;
	/* 0x10 */
	uint32_t instnComplete;
	uint32_t wtbMerge;
	uint32_t wtbNoMerge;
	uint32_t itlbHit;
	/* 0x20 */
	uint32_t itlbMiss;
	uint32_t dtlbHit;
	uint32_t dtlbMiss;
	uint32_t jtlbHit;
	/* 0x30 */
	uint32_t jtlbMiss;
	uint32_t powerSubZone;
	uint32_t powerMemPda;
	uint32_t freqScalarCtrl;
	/* 0x40 */
	uint32_t freqScalarMask;
} PmcPerfPowReg;

typedef struct PmcDQMReg {
	/* 0x00 */
	uint32_t cfg;
	uint32_t _4keLowWtmkIrqMask;
	uint32_t mipsLowWtmkIrqMask;
	uint32_t lowWtmkIrqMask;
	/* 0x10 */
	uint32_t _4keNotEmptyIrqMask;
	uint32_t mipsNotEmptyIrqMask;
	uint32_t notEmptyIrqSts;
	uint32_t queueRst;
	/* 0x20 */
	uint32_t notEmptySts;
	uint32_t nextAvailMask;
	uint32_t nextAvailQueue;
	uint32_t mips1LowWtmkIrqMask;
	/* 0x30 */
	uint32_t mips1NotEmptyIrqMask;
	uint32_t autoSrcPidInsert;
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

typedef struct Pmc {
	PmcCtrlReg ctrl;	/* 0x1000 */

	PmcOutFifoReg outFifo;	/* 0x1100 */
	uint32_t unused1[32];	/* 0x1180-0x11ff */
	PmcInFifoReg inFifo;	/* 0x1200 */
	uint32_t unused2[32];	/* 0x1280-0x12ff */

	PmcDmaReg dma[2];	/* 0x1300 */
	uint32_t unused3[48];	/* 0x1340-0x13ff */

	PmcTokenReg token;	/* 0x1400 */
	uint32_t unused4[121];	/* 0x141c-0x15ff */

	PmcPerfPowReg perfPower;	/* 0x1600 */
	uint32_t unused5[47];	/* 0x1644-0x16ff */

	uint32_t msgId[32];	/* 0x1700 */
	uint32_t unused6[32];	/* 0x1780-0x17ff */

	PmcDQMReg dqm;		/* 0x1800 */
	uint32_t unused7[50];	/* 0x1838-0x18ff */

	PmcCntReg hwCounter;	/* 0x1900 */
	uint32_t unused8[46];	/* 0x1948-0x19ff */

	PmcDqmQCtrlReg dqmQCtrl[32];	/* 0x1a00 */
	PmcDqmQDataReg dqmQData[32];	/* 0x1c00 */
	uint32_t unused9[64];	/* 0x1e00-0x1eff */

	uint32_t qStatus[32];	/* 0x1f00 */
	uint32_t unused10[32];	/* 0x1f80-0x1fff */

	PmcDqmQMibReg qMib;	/* 0x2000 */
	uint32_t unused11[1952];	/* 0x2180-0x3ffff */

	uint32_t sharedMem[8192];	/* 0x4000-0xbffc */
} Pmc;

/*
 * Process Monitor Module
 */
typedef struct PMRingOscillatorControl {
	uint32_t control;
	uint32_t en_lo;
	uint32_t en_mid;
	uint32_t en_hi;
	uint32_t idle_lo;
	uint32_t idle_mid;
	uint32_t idle_hi;
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
	uint32_t gp_out;
	uint32_t clock_select;
	uint32_t unused[2];
	uint32_t misc[4];
} PMMiscControl;

#define SWR_FIRST 0
#define SWR_LAST 0

typedef struct SSBMaster {
	uint32_t control;
	uint32_t wr_data;
	uint32_t rd_data;
} SSBMaster;

typedef struct PMEctrControl {
	uint32_t control;
	uint32_t interval;
	uint32_t thresh_lo;
	uint32_t thresh_hi;
	uint32_t count;
} PMEctrControl;

typedef struct PMBMaster {
	uint32_t ctrl;
#define PMC_PMBM_START     (1 << 31)
#define PMC_PMBM_TIMEOUT   (1 << 30)
#define PMC_PMBM_SLAVE_ERR (1 << 29)
#define PMC_PMBM_BUSY      (1 << 28)
#define PMC_PMBM_Read      (0 << 20)
#define PMC_PMBM_Write     (1 << 20)
	uint32_t wr_data;
	uint32_t timeout;
	uint32_t rd_data;
	uint32_t unused[4];
} PMBMaster;

typedef struct PMAPVTMONControl {
	uint32_t control;
	uint32_t reserved;
	uint32_t cfg_lo;
	uint32_t cfg_hi;
	uint32_t data;
	uint32_t vref_data;
	uint32_t unused[2];
	uint32_t ascan_cfg;
	uint32_t warn_temp;
	uint32_t reset_temp;
	uint32_t temp_value;
	uint32_t data1_value;
	uint32_t data2_value;
	uint32_t data3_value;
} PMAPVTMONControl;

typedef struct PMUBUSCfg {
	uint32_t window[8];
	uint32_t control;
} PMUBUSCfg;

typedef struct ProcessMonitorRegs {
	uint32_t MonitorCtrl;	/* 0x00 */
	uint32_t unused0[7];
	PMRingOscillatorControl ROSC;	/* 0x20 */
	uint32_t unused1;
	PMMiscControl Misc;	/* 0x40 */
	SSBMaster ssbMaster;	/* 0x60 */
	uint32_t unused2[5];
	PMEctrControl Ectr;	/* 0x80 */
	uint32_t unused3[11];
	PMBMaster PMBM[2];	/* 0xc0 */
	PMAPVTMONControl APvtmonCtrl;	/* 0x100 */
	uint32_t unused4[9];
	PMUBUSCfg UBUSCfg;	/* 0x160 */
} Procmon;

#endif

#endif
