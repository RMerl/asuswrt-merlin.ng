// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */
/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */
/*
 
*/

/*****************************************************************************
 *  Description:
 *      This contains header for PMC driver.
 *****************************************************************************/

#ifndef PMC_DRV_H
#define PMC_DRV_H

#include "asm/arch/pmc_drv_cfg.h"

#ifndef CONFIG_SPL_BUILD
#define CFG_RAMAPP
#endif

#include <common.h>

#if defined(CONFIG_BCMBCA_IKOS) && !defined(CONFIG_BRCM_IKOS)
#define CONFIG_BRCM_IKOS
#endif

#ifndef EXPORT_SYMBOL
#define EXPORT_SYMBOL(a)
#endif

#ifndef IS_BCMCHIP
#define IS_BCMCHIP(num) (defined(_BCM9##num##_) || \
		defined(CONFIG_BCM9##num) || defined(CONFIG_BCM##num))
#endif

#if defined(__ASSEMBLER__) && !defined(_LANGUAGE_ASSEMBLY)
#define _LANGUAGE_ASSEMBLY 
#endif

#include <asm/arch/pmc_addr.h>
#include <asm/arch/pmc.h>
#include <asm/arch/misc.h>

#define MAX_PMC_ROM_SIZE    0x8000
#define MAX_PMC_LOG_SIZE    0x8000

/* PMC reserved area.
   **NOTE**: Please make sure it matches with
   shared/opensource/include/bcm963xx/bcm_mem_reserve.h
*/
#define PMC_RESERVED_MEM_START  0x000C0000
#define PMC_RESERVED_MEM_SIZE   0x00040000	// Total PMC reserved memory size 256KB
#define CFG_BOOT_PMC_LOG_SIZE   0x00010000	// Leave 64K reserved memory for PMC log


#define CFG_BOOT_PMC_ADDR       (PMC_RESERVED_MEM_START)
#define CFG_BOOT_PMC_SIZE       (PMC_RESERVED_MEM_SIZE  - CFG_BOOT_PMC_LOG_SIZE) // Memory reserved for PMC firmware
#define CFG_BOOT_PMC_LOG_ADDR   (PMC_RESERVED_MEM_START + CFG_BOOT_PMC_SIZE)

#if MAX_PMC_ROM_SIZE + MAX_PMC_LOG_SIZE > CFG_BOOT_PMC_SIZE
#error ROM and LOG buffer size needs to be re-adjusted
#endif

#ifndef _LANGUAGE_ASSEMBLY 
#define pmc_spin_lock(...) do { } while (0)
#define pmc_spin_unlock(...) do { } while (0)

extern int getAVSConfig(void);
extern int is_pmcfw_code_loaded(void);
extern int is_pmcfw_data_loaded(void);

#define cache_to_uncache(va) (va)

#define console_status serial_tstc

#ifndef phys_to_virt
#define phys_to_virt(a) (a)
#endif

#endif // #ifndef _LANGUAGE_ASSEMBLY 

#ifndef PMC_IN_MAIN_LOOP 
    #define PMC_IN_MAIN_LOOP	6
#endif

#ifndef PMC_BOOT_TMO_SECONDS
    #define PMC_BOOT_TMO_SECONDS	0
#endif

/* there are 32 DQM, since REPLY DQM will always be one after the REQUEST
 * DQM, we should use use 0 to 30 for REQ DQM, so RPL DQM will be 1 to 31 */
/* 63138 has pair of DQM#0+DQM#1, #2+#3, #4+#5, and #6+#7.  We will use
 * DQM#0+DQM#1 pair */
#define PMC_DQM_REQ_NUM		0

#define PMC_DQM_RPL_NUM		(PMC_DQM_REQ_NUM + 1)
#define PMC_DQM_RPL_STS		(1 << PMC_DQM_RPL_NUM)

#define PMC_MODE_DQM		0
#define PMC_MODE_PMB_DIRECT	1
#ifdef PMC_IMPL_3_X
#define PMC_ACCESS_BPCM_DIRECT	1
#else
#define PMC_ACCESS_BPCM_DIRECT	0
#endif

#ifndef _LANGUAGE_ASSEMBLY
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
	kPMC_BOOT_FAILED,
	kPMC_COMMAND_TIMEOUT = 256,
	kPMC_MESSAGE_ID_MISMATCH,
};

// ---------------------------- Returned log entry structure --------------------------
typedef struct {
	uint8_t reserved;
	uint8_t logMsgID;
	uint8_t errorCode;
	uint8_t logCmdID;
	uint8_t srcPort;
	uint8_t e_msgID;
	uint8_t e_errorCode;
	uint8_t e_cmdID;
	struct {
		uint32_t logReplyNum:8;
		uint32_t e_Island:4;
		uint32_t e_Bus:2;
		uint32_t e_DevAddr:8;
		uint32_t e_Zone:10;
	} s;
	uint32_t e_Data0;
} TErrorLogEntry;

// ---------------------------- Power states --------------------------
enum {
	kPMCPowerState_Unknown,
	kPMCPowerState_NoPower,
	kPMCPowerState_LowPower,
	kPMCPowerState_FullPower,
};

// PMC run-state:
enum {
	kPMCRunStateExecutingBootROM = 0,
	kPMCRunStateWaitingBMUComplete,
	kPMCRunStateAVSCompleteWaitingForImage,
	kPMCRunStateAuthenticatingImage,
	kPMCRunStateAuthenticationFailed,
	kPMCRunStateReserved,
	kPMCRunStateStalled,
	kPMCRunStateRunning
};

// the only valid "gear" values for "SetClockGear" function
enum {
	kClockGearLow,
	kClockGearHigh,
	kClockGearDynamic,
	kClockGearBypass
};

// PMC Boot options ( parameter for pmc_boot function )
enum {
	kPMCBootDefault = 0,
	kPMCBootAVSDisable,
	kPMCBootAVSTrackDisable,
	kPMCBootLogBuffer,
	kPMCBootLogSize
};

// int TuneRunner(void);
// int GetSelect0(void);
// int GetSelect3(void);
int pmc_init(void);
void pmc_reset(void);
void pmc_initmode(void);
// int get_pmc_boot_param(unsigned boot_option, unsigned *boot_param);
void pmc_log(int log_type);
void pmc_save_log_item(void);
void pmc_show_log_item(void);
int read_bpcm_reg_direct(int devAddr, int wordOffset, uint32_t * value);
int write_bpcm_reg_direct(int devAddr, int wordOffset, uint32_t value);
int GetRevision(unsigned int *change, unsigned int *revision);
int GetPVT(int sel, int island, int *value);
int GetRCalSetting(int resistor, int *rcal);
int GetDevPresence(int devAddr, int *value);
int GetSWStrap(int devAddr, int *value);
int GetHWRev(int devAddr, int *value);
int GetNumZones(int devAddr, int *value);
int GetAvsDisableState(int island, int *state);
int Ping(void);
int GetErrorLogEntry(TErrorLogEntry * logEntry);
int SetClockHighGear(int devAddr, int zone, int clkN);
int SetClockLowGear(int devAddr, int zone, int clkN);
int SetClockGear(int devAddr, int zone, int gear);
int SetRunState(int island, int state);
int SetPowerState(int island, int state);
#if !defined(PMC_ON_HOSTCPU)
void BootPmcNoRom(unsigned long physAddr);
#endif
int ReadBPCMRegister(int devAddr, int wordOffset, uint32_t * value);
int WriteBPCMRegister(int devAddr, int wordOffset, uint32_t value);
int ReadZoneRegister(int devAddr, int zone, int wordOffset, uint32_t * value);
int WriteZoneRegister(int devAddr, int zone, int wordOffset, uint32_t value);
int PowerOnDevice(int devAddr);
int PowerOffDevice(int devAddr, int repower);
int PowerOnZone(int devAddr, int zone);
int PowerOffZone(int devAddr, int zone);
int ResetDevice(int devAddr);
int ResetZone(int devAddr, int zone);
int CloseAVS(int island, unsigned short margin_mv_slow,
	     unsigned short margin_mv_fast, unsigned short maximum_mv,
	     unsigned short minimum_mv);
int RecloseAVS(int iscold);
void WaitPmc(int runState, void* pmc_log);
int StallPmc(void);
int UnstallPmc(void);
int GetAllROs(void *shmem);

enum pvtctl_sel {
	kTEMPERATURE = 0,
	kV_0p85_0 = 1,
	kV_0p85_1 = 2,
	kV_VIN = 3,
	kV_1p00_1 = 4,
	kV_1p80 = 5,
	kV_3p30 = 6,
	kTEST = 7,
};
int pmc_convert_pvtmon(int sel, int value);
int pmc_get_tracktemp(int *status);
int pmc_set_tracktemp(int enable);
#endif //_LANGUAGE_ASSEMBLY

#endif // PMC_DRV_H
