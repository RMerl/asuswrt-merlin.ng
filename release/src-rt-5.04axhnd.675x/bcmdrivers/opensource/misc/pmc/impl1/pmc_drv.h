/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */
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

/*****************************************************************************
 *  Description:
 *      This contains header for PMC driver.
 *****************************************************************************/

#ifndef PMC_DRV_H
#define PMC_DRV_H

#ifndef IS_BCMCHIP
#define IS_BCMCHIP(num) (defined(_BCM9##num##_) || \
		defined(CONFIG_BCM9##num) || defined(CONFIG_BCM##num))
#endif

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/io.h>
void pmc_spin_lock(void);
void pmc_spin_unlock(void);

struct g_pmc_t {
    void __iomem *pmc_base;
    void __iomem *procmon_base;
    void __iomem *maestro_base;
    void __iomem *dtcm_base;
    void __iomem *strap;
    void __iomem *bac_cpu_base;
    void (*unmap)(struct g_pmc_t *);
};

extern struct g_pmc_t *g_pmc;

#include "pmc.h"
#include "bcm_mem_reserve.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define PMB_NO_RESET 0xDEADBEAF
typedef struct {
    char * name;
    unsigned int pmb_addr;
    unsigned int reset_value;
}pmb_init_t;

#include "pmc_drv_cfg.h"

#include "pmc_addr.h"

#ifndef PMC_BOOT_TMO_SECONDS
#define PMC_BOOT_TMO_SECONDS	0
#endif

/* there are 32 DQM, since REPLY DQM will always be one after the REQUEST
 * DQM, we should use use 0 to 30 for REQ DQM, so RPL DQM will be 1 to 31 */
/* We will use DQM#0+DQM#1 pair */
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

#include "pmc_core_api.h"
void pmc_reset(void);
void pmc_initmode(void);
void pmc_log(int log_type);
void pmc_save_log_item(void);
void pmc_show_log_item(void);
int read_bpcm_reg_direct(int devAddr, int wordOffset, uint32_t * value);
int write_bpcm_reg_direct(int devAddr, int wordOffset, uint32_t value);
int read_bpcm_reg_direct_internal(int devAddr, int wordOffset, uint32_t * value);
int write_bpcm_reg_direct_internal(int devAddr, int wordOffset, uint32_t value);
int GetRevision(unsigned int *change, unsigned int *revision);
int GetDevPresence(int devAddr, int *value);
int GetSWStrap(int devAddr, int *value);
int GetHWRev(int devAddr, int *value);
int GetNumZones(int devAddr, int *value);
int Ping(void);
int GetErrorLogEntry(TErrorLogEntry * logEntry);
int SetClockHighGear(int devAddr, int zone, int clkN);
int SetClockLowGear(int devAddr, int zone, int clkN);
int SetClockGear(int devAddr, int zone, int gear);
int SetRunState(int island, int state);
int SetPowerState(int island, int state);
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
void WaitPmc(int runState, void* pmc_log);
int GetAllROs(uint32_t pa);
int pmc_get_tracktemp(int *status);
int pmc_set_tracktemp(int enable);
int pmc_init(void);
#endif // PMC_DRV
