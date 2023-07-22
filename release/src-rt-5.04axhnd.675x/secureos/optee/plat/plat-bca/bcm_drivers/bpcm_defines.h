/*
<:copyright-BRCM:2019:DUAL/GPL:standard

   Copyright (c) 2019 Broadcom 
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

#ifndef BPCM_DEFINES_H
#define BPCM_DEFINES_H

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;

#if defined(PLATFORM_FLAVOR_63138) || defined(PLATFORM_FLAVOR_63148)
#define PMB_BUS_ID_SHIFT		8
#else
#define PMB_BUS_ID_SHIFT		12
#endif


#if defined(PLATFORM_FLAVOR_63178) || defined(PLATFORM_FLAVOR_6846)
#define PMB_BUS		1
#else
#define PMB_BUS		0
#endif

#define PMB_MAX_ORION_CPU		3
#define PMB_ADDR_ORION_CPU0		(32 | PMB_BUS << PMB_BUS_ID_SHIFT)
#define PMB_ADDR_ORION_CPU1		(33 | PMB_BUS << PMB_BUS_ID_SHIFT)
#define PMB_ADDR_ORION_CPU2		(34 | PMB_BUS << PMB_BUS_ID_SHIFT)

#define PMB_ADDR_BIU			(39 | PMB_BUS << PMB_BUS_ID_SHIFT)
#define PMB_ADDR_AIP			(4  | PMB_BUS << PMB_BUS_ID_SHIFT)

typedef union
{
	struct {
#if defined(PLATFORM_FLAVOR_63138)
		uint32_t core_pwr_ctrl	:  2;	// 01:00    R/W
		uint32_t reserved2	:  6;	// 07:02    R/W
		uint32_t pll_pwr_on	:  1;	// 08:08    R/W
		uint32_t pll_ldo_pwr_on	:  1;	// 09:09    R/W
		uint32_t pll_clamp_on	:  1;	// 10:10    R/W
		uint32_t reserved1	:  2;	// 12:11    R/O
		uint32_t cpu0_reset_n	:  1;	// 13:13    R/W
		uint32_t cpu1_reset_n	:  1;	// 14:14    R/W
		uint32_t neon_reset_n	:  1;	// 15:15    R/W
		uint32_t reserved0	: 12;	// 27:16    R/O
		uint32_t pwr_ctrl_sts	:  2;	// 29:28    R/O
		uint32_t power_down	:  2;	// 31:30    R/O
#else
		uint32_t cpu_reset_n	:  8;   // 07:00    R/W
		uint32_t c0l2_reset	:  1;   // 08:08    R/W
		uint32_t c1l2_reset	:  1;   // 09:09    R/W
		uint32_t reserved0	:  6;   // 15:10    R/O
		uint32_t cpu_bpcm_init_on	:  8;   // 23:16    R/W
		uint32_t c0l2_bpcm_init_on	:  1;   // 24:24    R/W
		uint32_t c1l2_bpcm_init_on	:  1;   // 25:25    R/W
		uint32_t ubus_sr	:  1;   // 26:26    R/W
		uint32_t cci_sr		:  1;   // 27:27    R/W
		uint32_t webcores_sr	:  1;   // 28:28    R/W
		uint32_t hw_done	:  1;   // 29:29    R/O
		uint32_t sw_done	:  1;   // 30:30    R/W
		uint32_t start		:  1;   // 31:31    R/W
#endif
	} Bits;
	uint32_t Reg32;
} ARM_CONTROL_REG;

typedef union
{
	struct {
		uint32_t mem_pwr_ok	:  1;	// 00:00    R/W
		uint32_t mem_pwr_on	:  1;	// 01:01    R/W
		uint32_t mem_clamp_on	:  1;	// 02:02    R/W
		uint32_t reserved2	:  1;	// 03:03    R/W
		uint32_t mem_pwr_ok_status	:  1;	// 04:04    R/O
		uint32_t mem_pwr_on_status	:  1;	// 05:05    R/O
		uint32_t reserved1	:  2;	// 07:06    R/W
		uint32_t mem_pda		:  4;	// 11:08    R/W only LS bit for CPU0/1, all four bits for neon_l2
		uint32_t reserved0	:  3;	// 14:12    R/W
		uint32_t clamp_on		:  1;	// 15:15    R/W
		uint32_t pwr_ok		:  4;	// 19:16    R/W ditto
		uint32_t pwr_on		:  4;	// 23:20    R/W ditto
		uint32_t pwr_ok_status	:  4;	// 27:24    R/O ditto
		uint32_t pwr_on_status	:  4;	// 31:28    R/O only LS 2-bits for CPU1, only LS 1 bit for neon_l2
	} Bits;
	uint32_t Reg32;
} ARM_CPUx_PWR_CTRL_REG;

typedef union
{
	struct {
		uint32_t pmb_Addr	: 8;
		uint32_t hw_rev	: 8;
		uint32_t sw_strap	: 16;
	} Bits;
	uint32_t Reg32;
} BPCM_ID_REG;

typedef union
{
	struct {
		uint32_t num_zones	: 8;
		uint32_t num_sr_bits	: 8;
		uint32_t devType		: 4;	// see enum above
		uint32_t reserved1	: 12;
	} Bits;
	uint32_t Reg32;
} BPCM_CAPABILITES_REG;

typedef union
{
	struct {
		uint32_t pwd_alert	: 1;
		uint32_t reserved		: 31;
	} Bits;
	uint32_t Reg32;
} BPCM_STATUS_REG;

typedef union
{
	struct {
		uint32_t ro_en_s		: 1;
		uint32_t ro_en_h		: 1;
		uint32_t ectr_en_s	: 1;
		uint32_t ectr_en_h	: 1;
		uint32_t thresh_en_s	: 1;
		uint32_t thresh_en_h	: 1;
		uint32_t continuous_s	: 1;
		uint32_t continuous_h	: 1;
		uint32_t reserved		: 4;
		uint32_t valid_s		: 1;
		uint32_t alert_s		: 1;
		uint32_t valid_h		: 1;
		uint32_t alert_h		: 1;
		uint32_t interval		: 16;
	} Bits;
	uint32_t Reg32;
} BPCM_AVS_ROSC_CONTROL_REG;

typedef union
{
	struct {
		uint32_t thresh_lo	: 16;
		uint32_t thresh_hi	: 16;
	} Bits;
	uint32_t Reg32;
} BPCM_AVS_ROSC_THRESHOLD;

typedef union
{
	struct {
		uint32_t count_s	: 16;
		uint32_t count_h	: 16;
	} Bits;
	uint32_t Reg32;
} BPCM_AVS_ROSC_COUNT;

typedef union
{
	struct {
		uint32_t pwd_en		: 1;
		uint32_t pwd_alert_sel	: 1;
		uint32_t start		: 6;
		uint32_t pwd_tm_en	: 1;
		uint32_t reserved2	: 6;
		uint32_t alert		: 1;
		uint32_t ccfg		: 8;
		uint32_t rsel		: 3;
		uint32_t clr_cfg		: 3;
		uint32_t reserved1	: 2;
	} Bits;
	uint32_t Reg32;
} BPCM_AVS_PWD_CONTROL;

typedef union
{
	struct {
		uint32_t tbd		: 32;
	} Bits;
	uint32_t Reg32;
} BPCM_PWD_ACCUM_CONTROL;

typedef union
{
	struct {
		uint32_t sr		: 8;
		uint32_t gp		: 24;
	} Bits;
	uint32_t Reg32;
} BPCM_SR_CONTROL;

typedef union
{
	struct {
		uint32_t manual_clk_en		: 1;
		uint32_t manual_reset_ctl		: 1;
		uint32_t freq_scale_used		: 1;	// R/O
		uint32_t dpg_capable		: 1;	// R/O
		uint32_t manual_mem_pwr		: 2;
		uint32_t manual_iso_ctl		: 1;
		uint32_t manual_ctl		: 1;
		uint32_t dpg_ctl_en		: 1;
		uint32_t pwr_dn_req		: 1;
		uint32_t pwr_up_req		: 1;
		uint32_t mem_pwr_ctl_en		: 1;
		uint32_t blk_reset_assert		: 1;
		uint32_t mem_stby			: 1;
		uint32_t reserved			: 5;
		uint32_t pwr_cntl_state		: 5;
		uint32_t freq_scalar_dyn_sel	: 1;	// R/O
		uint32_t pwr_off_state		: 1;	// R/O
		uint32_t pwr_on_state		: 1;	// R/O
		uint32_t pwr_good			: 1;	// R/O
		uint32_t dpg_pwr_state		: 1;	// R/O
		uint32_t mem_pwr_state		: 1;	// R/O
		uint32_t iso_state		: 1;	// R/O
		uint32_t reset_state		: 1;	// R/O
	} Bits;
	uint32_t Reg32;
} BPCM_PWR_ZONE_N_CONTROL;

typedef union
{
	struct {
		uint32_t pwr_ok_delay_sel	: 3;
		uint32_t pwk_ok_thresh	: 2;
		uint32_t reserved		: 3;
		uint32_t iso_on_delay	: 4;
		uint32_t iso_off_delay	: 4;
		uint32_t clock_on_delay	: 4;
		uint32_t clock_off_delay	: 4;
		uint32_t reset_on_delay	: 4;
		uint32_t reset_off_delay	: 4;
	} Bits;
	uint32_t Reg32;
} BPCM_PWR_ZONE_N_CONFIG1;

typedef union
{
	struct {
		uint32_t delay_prescale_sel	: 3;
		uint32_t slew_prescale_sel	: 3;
		uint32_t reserved			: 6;
		uint32_t dpgn_on_delay		: 4;
		uint32_t dpg1_on_delay		: 4;
		uint32_t dpg_off_delay		: 4;
		uint32_t mem_on_delay		: 4;
		uint32_t mem_off_delay		: 4;
	} Bits;
	uint32_t Reg32;
} BPCM_PWR_ZONE_N_CONFIG2;

typedef union
{
	struct {
		uint32_t fs_bypass_en	: 1;
		uint32_t gear_sel		: 1;
		uint32_t use_dyn_gear_sel	: 1;
		uint32_t reserved2	: 1;
		uint32_t low_gear_div	: 3;
		uint32_t high_gear_div	: 3;
		uint32_t reserved		: 22;
	} Bits;
	uint32_t Reg32;
} BPCM_ZONE_N_FREQ_SCALAR_CONTROL;

typedef struct
{
	BPCM_PWR_ZONE_N_CONTROL		control;
	BPCM_PWR_ZONE_N_CONFIG1		config1;
	BPCM_PWR_ZONE_N_CONFIG2		config2;
	BPCM_ZONE_N_FREQ_SCALAR_CONTROL	freq_scalar_control;
} BPCM_ZONE;

// ARM BPCM addresses as used by 63138
typedef struct
{
	BPCM_ID_REG			id_reg;		// offset = 0x00, actual offset = 0
	BPCM_CAPABILITES_REG		capabilities;	// offset = 0x04, actual offset = 1
	uint32_t				control;	// offset = 0x08, actual offset = 2
	BPCM_STATUS_REG			status;		// offset = 0x0c, actual offset = 3
	BPCM_AVS_ROSC_CONTROL_REG	rosc_control;	// offset = 0x10, actual offset = 4
	BPCM_AVS_ROSC_THRESHOLD		rosc_thresh_h;	// offset = 0x14, actual offset = 5
	BPCM_AVS_ROSC_THRESHOLD		rosc_thresh_s;	// offset = 0x18, actual offset = 6
	BPCM_AVS_ROSC_COUNT		rosc_count;	// offset = 0x1c, actual offset = 7
	BPCM_AVS_PWD_CONTROL		pwd_control;	// offset = 0x20, actual offset = 8
	BPCM_PWD_ACCUM_CONTROL		pwd_accum_control; // offset = 0x24, actual offset = 9
	BPCM_SR_CONTROL			sr_control;	// offset = 0x28, actual offset = 10
	uint32_t				reserved;	// offset = 0x2c, actual offset = 11
	ARM_CONTROL_REG			arm_control;	// offset = 0x30, actual offset = 12
	ARM_CPUx_PWR_CTRL_REG		arm_pwr_ctrl_0;	// offset = 0x34, actual offset = 13
	ARM_CPUx_PWR_CTRL_REG		arm_pwr_ctrl_1;	// offset = 0x38, actual offset = 14
	ARM_CPUx_PWR_CTRL_REG		arm_neon_l2;	// offset = 0x3c, actua; offset = 15
	BPCM_ZONE			zones[1020];	// offset = 0x40..0x3FFC, actual offset = 16..4095 (1020 * 4 = 4080 + 16 = 4096)
} ARM_BPCM_REGS;

#define ARMBPCMOffset(reg)	offsetof(ARM_BPCM_REGS,reg)
#define ARMBPCMRegOffset(reg)	(ARMBPCMOffset(reg) >> 2)

// ---------------------------- Returned error codes --------------------------
enum {
    // 0..15 may come from either the interface or from the PMC command handler
    // 256 or greater only come from the interface
    kPMC_NO_ERROR,
    kPMC_INVALID_ISLAND,
    kPMC_INVALID_DEVICE,
    kPMC_INVALID_ZONE,
    kPMC_INVALID_STATE,
    kPMC_INVALID_COMMAND,
    kPMC_LOG_EMPTY,
    kPMC_INVALID_PARAM,
    kPMC_BPCM_READ_TIMEOUT,
    kPMC_INVALID_BUS,
    kPMC_INVALID_QUEUE_NUMBER,
    kPMC_QUEUE_NOT_AVAILABLE,
    kPMC_INVALID_TOKEN_SIZE,
    kPMC_INVALID_WATERMARKS,
    kPMC_INSUFFICIENT_QSM_MEMORY,
    kPMC_INVALID_BOOT_COMMAND,
    kPMC_COMMAND_TIMEOUT = 256,
    kPMC_MESSAGE_ID_MISMATCH,
};

typedef union {
	struct {
		uint32_t cmdID	: 8;
		uint32_t error	: 8;
		uint32_t msgID	: 8;
		uint32_t srcPort	: 8;
	} Bits;
	uint32_t Reg32; 
} TCommandWord0;

typedef union {
	struct {
#if defined(PMC_VERSION_3)
        uint32_t devAddr  : 16;   // [15:00] bus in upper nibble (only values of 0-7 are allowed),
                                  //          device address in lower 12 bits (4096 devices = 0..4095)
        uint32_t zoneIdx  : 10;   // [25:16] maximum 1023 registers/zone (0..1022)
        uint32_t island   : 4;    // [27:26] maximum 15 power islands (0..15) (island 15 = ALL islands!
        uint32_t reserved : 2;    // [31:28]
#else
		uint32_t zoneIdx  : 10;
		uint32_t devAddr	: 10;
		uint32_t island	: 4;
		uint32_t logNum	: 8;
#endif
	} Bits;
	uint32_t	Reg32;
} TCommandWord1;

// Ping, GetNextLogEntry, GetRMON and Sigma
typedef struct {
	uint32_t			unused[2];
} TCommandNoParams;

typedef struct {
	uint32_t			params[2];
} TCommandGenericParams;

// PowerZoneOnOff, SetRunState, SetPowerState
typedef struct {
	uint8			reserved[3];
	uint8			state;
	uint32_t			unused;
} TCommandStateOnly;

// PowerDevOnOff
typedef struct {
#if defined(PMC_VERSION_3)
	uint8			state;
	uint8			restore;
	uint8			reserved[2];
#else
	uint8			reserved[2];
	uint8			restore;
	uint8			state;
#endif
	uint32_t			unused;
} TCommandPowerDevice;

// PowerOffIsland
typedef struct {
	uint8			reserved[3];
	uint8			restore;
	uint32_t			unused;
} TCommandPowerIsland;

// SetClockLowGear, SetClockHighGear
typedef struct {
	uint8			reserved[3];
	uint8			clkN;
	uint32_t			unused;
} TCommandSetClockN;

// SetClockGear
typedef struct {
	uint8			reserved[3];
	uint8			gear;
	uint32_t			unused;
} TCommandSetClockGear;

typedef struct {
	uint8			unused1;
	uint8			numTokens;
	uint8			tokenSize;
	uint8			queueNumber;
	uint16			unused2;
	uint8			high_watermark;
	uint8			low_watermark;
} TCommandAllocDQM;

typedef struct {
	uint32_t			phy_src_addr;
	uint32_t			dest_addr;		// lower 8 bits **may** be log2 window size
} TCommandJumpApp;

typedef struct {
	uint32_t			margin_mv_slow;
	uint32_t			margin_mv_fast;
} TCommandCloseAVS;

typedef struct {
	uint32_t			word2;
	uint32_t			word3;
} TCommandResponse;

typedef struct {
	TCommandWord0		word0;
	TCommandWord1		word1;
	union {
		TCommandNoParams	cmdNoParams;
		TCommandGenericParams	cmdGenericParams;
		TCommandStateOnly	cmdStateOnlyParam;
		TCommandPowerDevice	cmdPowerDevice;
		TCommandPowerIsland	cmdPowerIsland;
		TCommandSetClockN	cmdSetClockN;
		TCommandSetClockGear	cmdSetClockGear;
		TCommandAllocDQM	cmdAllocDqm;
		TCommandJumpApp		cmdJumpApp;
		TCommandCloseAVS	cmdCloseAVS;
		TCommandResponse	cmdResponse;
	} u;
} TCommand;

// command codes
enum {
	// low-level commands
	cmdReserved = 0,
	cmdGetDevPresence,
	cmdGetSWStrap,
	cmdGetHWRev,
	cmdGetNumZones,
	cmdPing,
	cmdGetNextLogEntry,
	cmdGetRMONandSigma,
	cmdSetClockHighGear,
	cmdSetClockLowGear,
	cmdSetClockGear,
	cmdReadBpcmReg,
	cmdReadZoneReg,
	cmdWriteBpcmReg,
	cmdWriteZoneReg,
	// general-purpose high-level commands
	cmdSetRunState, // = 15
	cmdSetPowerState, // = 16
	cmdShutdownAllowed, // = 17
	cmdGetSelect0, // = 18
	cmdGetSelect3, // = 19
	cmdGetAvsDisableState, // = 20
	cmdGetPVT, // = 21
	// specific-purpose high-level commands
	cmdPowerDevOnOff, // = 22
	cmdPowerZoneOnOff, // = 23
	cmdResetDevice, // = 24
	cmdResetZone, // = 25
	cmdAllocateG2UDQM, // = 26
	cmdQSMAvailable, // = 27
	cmdRevision, // = 28
	cmdRegisterCmdHandler, // = 29
	cmdFindUnusedCommand, // = 30
	cmdLockCmdTable, // = 31
	cmdJumpApp, // = 32
	cmdStall, // = 33
	cmdCloseAVS, // = 34
};

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
   uint32_t unused0[3];   /* 0x4c-0x57 */
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
   uint32_t unused1[4];   /* 0xe4-0xf3 */
   /* 0xf4 */
   uint32_t iopPeriphBaseAddr;
   uint32_t lfsr;
   uint32_t unused2;      /* 0xfc-0xff */
} PmcCtrlReg;

typedef struct PmcOutFifoReg {
   uint32_t msgCtrl;      /* 0x00 */
   uint32_t msgSts;    /* 0x04 */
   uint32_t unused[14];   /* 0x08-0x3f */
   uint32_t msgData[16];  /* 0x40-0x7c */
} PmcOutFifoReg;

typedef struct PmcInFifoReg {
   uint32_t msgCtrl;      /* 0x00 */
   uint32_t msgSts;    /* 0x04 */
   uint32_t unused[13];   /* 0x08-0x3b */
   uint32_t msgLast;      /* 0x3c */
   uint32_t msgData[16];  /* 0x40-0x7c */
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
   uint32_t unused[6]; /* 0x28-0x3f */
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

#if defined(PLATFORM_FLAVOR_63138)
typedef struct Pmc {
   uint32_t baseReserved;    /* 0x0000 */
   uint32_t unused0[1023];
   PmcCtrlReg ctrl;     /* 0x1000 */

   PmcOutFifoReg outFifo;     /* 0x1100 */
   uint32_t unused1[32];     /* 0x1180-0x11ff */
   PmcInFifoReg inFifo;    /* 0x1200 */
   uint32_t unused2[32];     /* 0x1280-0x12ff */

   PmcDmaReg dma[2];    /* 0x1300 */
   uint32_t unused3[48];     /* 0x1340-0x13ff */

   PmcTokenReg token;      /* 0x1400 */
   uint32_t unused4[121];    /* 0x141c-0x15ff */

   PmcPerfPowReg perfPower;   /* 0x1600 */
   uint32_t unused5[47];     /* 0x1644-0x16ff */

   uint32_t msgId[32];    /* 0x1700 */
   uint32_t unused6[32];     /* 0x1780-0x17ff */

   PmcDQMReg dqm;       /* 0x1800 */
   uint32_t unused7[50];     /* 0x1838-0x18ff */

   PmcCntReg hwCounter;    /* 0x1900 */
   uint32_t unused8[46];     /* 0x1948-0x19ff */

   PmcDqmQCtrlReg dqmQCtrl[32];  /* 0x1a00 */
   PmcDqmQDataReg dqmQData[32];  /* 0x1c00 */
   uint32_t unused9[64];     /* 0x1e00-0x1eff */

   uint32_t qStatus[32];     /* 0x1f00 */
   uint32_t unused10[32];    /* 0x1f80-0x1fff */

   PmcDqmQMibReg qMib;     /* 0x2000 */
   uint32_t unused11[1952];     /* 0x2180-0x3ffff */

   uint32_t sharedMem[8192];    /* 0x4000-0xbffc */
} Pmc;
#else
typedef struct Pmc {
   uint32_t unused0[768];
   PmcDQMReg dqm;                /* 0xc00 */
   uint32_t unused1[3314];       /* 0xc38-0x4fff */
   PmcDqmQDataReg dqmQData[8];   /* 0x5000 */
} Pmc;
#endif


#define PMC_BASE           pmc_mapped_base
#define PMC                ((volatile Pmc * const)PMC_BASE)

/* there are 32 DQM, since REPLY DQM will always be one after the REQUEST
 * DQM, we should use use 0 to 30 for REQ DQM, so RPL DQM will be 1 to 31
 * 63138 has pair of DQM#0+DQM#1, #2+#3, #4+#5, and #6+#7.  We will use
 * DQM#0+DQM#1 pair */
#define PMC_DQM_REQ_NUM		0
#define PMC_DQM_RPL_NUM		(PMC_DQM_REQ_NUM + 1)
#define PMC_DQM_RPL_STS		(1 << PMC_DQM_RPL_NUM)

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

typedef struct PMSSBMasterControl {
   uint32_t control;
#define PMC_SSBM_CONTROL_SSB_START  (1<<15)
#define PMC_SSBM_CONTROL_SSB_ADPRE  (1<<13)
#define PMC_SSBM_CONTROL_SSB_EN     (1<<12)
#define PMC_SSBM_CONTROL_SSB_CMD_SHIFT (10)
#define PMC_SSBM_CONTROL_SSB_CMD_MASK  (0x3 << PMC_SSBM_CONTROL_SSB_CMD_SHIFT)
#define PMC_SSBM_CONTROL_SSB_CMD_READ  (2)
#define PMC_SSBM_CONTROL_SSB_CMD_WRITE (1)
#define PMC_SSBM_CONTROL_SSB_ADDR_SHIFT   (0)
#define PMC_SSBM_CONTROL_SSB_ADDR_MASK (0x3ff << PMC_SSBM_CONTROL_SSB_ADDR_SHIFT)
   uint32_t wr_data;
   uint32_t rd_data;
} PMSSBMasterControl;

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

#if defined(PLATFORM_FLAVOR_63138)
#include "bpcm_defines_63138.h"
#endif

#if defined(PLATFORM_FLAVOR_63148)
#include "bpcm_defines_63148.h"
#endif
#endif // BPCM_DEFINES_H
