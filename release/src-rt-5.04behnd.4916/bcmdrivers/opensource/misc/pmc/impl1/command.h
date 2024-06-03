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

#ifndef PMCCOMMAND_H
#define PMCCOMMAND_H

#include "pmc_drv.h"

typedef union {
	struct {
		uint32_t cmdID:8;
		uint32_t error:8;
		uint32_t msgID:8;
		uint32_t srcPort:8;
	} Bits;
	uint32_t Reg32;
} TCommandWord0;

#ifdef PMC_IMPL_3_X
typedef union {
	struct {
		uint32_t devAddr:16;	// [15:00] bus in upper nibble (only values of 0-7 are allowed), device address in lower 12 bits (4096 devices = 0..4095)
		uint32_t zoneIdx:10;	// [25:16] maximum 1023 registers/zone (0..1022)
		uint32_t island:4;	// [27:26] maximum 15 power islands (0..15) (island 15 = ALL islands!
		uint32_t reserved:2;	// [31:28]
	} Bits;
	uint32_t Reg32;
} TCommandWord1;
#else
typedef union {
	struct {
		uint32_t zoneIdx:10;
		uint32_t devAddr:10;
		uint32_t island:4;
		uint32_t logNum:8;
	} Bits;
	uint32_t Reg32;
} TCommandWord1;
#endif

// Ping, GetNextLogEntry, GetRMON and Sigma
typedef struct {
	uint32_t unused[2];
} TCommandNoParams;

typedef struct {
	uint32_t params[2];
} TCommandGenericParams;

// PowerZoneOnOff, SetRunState, SetPowerState
typedef struct {
#if defined(PMC_CPU_BIG_ENDIAN)
	uint8_t reserved[3];
	uint8_t state;
#else
	uint8_t state;
	uint8_t reserved[3];
#endif
	uint32_t unused;
} TCommandStateOnly;

// PowerDevOnOff
typedef struct {
#if defined(PMC_CPU_BIG_ENDIAN)
	uint8_t reserved[2];
	uint8_t restore;
	uint8_t state;
#else
	uint8_t state;
	uint8_t restore;
	uint8_t reserved[2];
#endif
	uint32_t unused;
} TCommandPowerDevice;

// PowerOffIsland
typedef struct {
#if defined(PMC_CPU_BIG_ENDIAN)
	uint8_t reserved[3];
	uint8_t restore;
#else
	uint8_t restore;
	uint8_t reserved[3];
#endif
	uint32_t unused;
} TCommandPowerIsland;

// SetClockLowGear, SetClockHighGear
typedef struct {
#if defined(PMC_CPU_BIG_ENDIAN)
	uint8_t reserved[3];
	uint8_t clkN;
#else
	uint8_t clkN;
	uint8_t reserved[3];
#endif
	uint32_t unused;
} TCommandSetClockN;

// SetClockGear
typedef struct {
#if defined(PMC_CPU_BIG_ENDIAN)
	uint8_t reserved[3];
	uint8_t gear;
#else
	uint8_t gear;
	uint8_t reserved[3];
#endif
	uint32_t unused;
} TCommandSetClockGear;

typedef struct {
#if defined(PMC_CPU_BIG_ENDIAN)
	uint8_t unused1;
	uint8_t numTokens;
	uint8_t tokenSize;
	uint8_t queueNumber;
	uint16_t unused2;
	uint8_t high_watermark;
	uint8_t low_watermark;
#else
	uint8_t queueNumber;
	uint8_t tokenSize;
	uint8_t numTokens;
	uint8_t unused1;
	uint8_t low_watermark;
	uint8_t high_watermark;
	uint16_t unused2;
#endif
} TCommandAllocDQM;

typedef struct {
	uint32_t phy_src_addr;
	uint32_t dest_addr;	// lower 8 bits **may** be log2 window size
} TCommandJumpApp;

typedef struct {
#if defined(PMC_IMPL_3_X) || IS_BCMCHIP(63158)
	union {
		uint32_t word2;
		struct {
			uint16_t margin_mv_slow;
			uint16_t maximum_mv;
		};
	};
	union {
		uint32_t word3;
		struct {
			uint16_t margin_mv_fast;
			uint16_t minimum_mv;
		};
	};
#else
	uint32_t margin_mv_slow;
	uint32_t margin_mv_fast;
#endif
} TCommandCloseAVS;

typedef struct {
	uint32_t word2;
	uint32_t word3;
} TCommandResponse;

typedef struct {
	TCommandWord0 word0;
	TCommandWord1 word1;
	union {
		TCommandNoParams cmdNoParams;
		TCommandGenericParams cmdGenericParams;
		TCommandStateOnly cmdStateOnlyParam;
		TCommandPowerDevice cmdPowerDevice;
		TCommandPowerIsland cmdPowerIsland;
		TCommandSetClockN cmdSetClockN;
		TCommandSetClockGear cmdSetClockGear;
		TCommandAllocDQM cmdAllocDqm;
		TCommandJumpApp cmdJumpApp;
		TCommandCloseAVS cmdCloseAVS;
		TCommandResponse cmdResponse;
	} u;
} TCommand;

// special values to select all devices/zones
#define ALL_DEVICES	0x3ff
#define ALL_ZONES	0x3ff

// used in validate caller to prevent/allow restrictions on island, device and/or zone
#define ANY_ISLAND	998
#define ANY_DEVICE	1024
#define ANY_ZONE	1024
#define NO_ISLAND	999
#define NO_DEVICE	1025
#define NO_ZONE		1025

// error codes
enum {
	NO_ERROR = 0,
	INVALID_ISLAND,
	INVALID_DEVICE,
	INVALID_ZONE,
	INVALID_STATE,
	INVALID_COMMAND,
	LOG_EMPTY,
	INVALID_PARAM,
	BPCM_READ_TIMEOUT,
	INVALID_BUS,
	INVALID_QUEUE_NUMBER,
	QUEUE_NOT_AVAILABLE,
	INVALID_TOKEN_SIZE,
	INVALID_WATERMARKS,
	INSUFFIENT_QSM_MEMORY_AVAILABLE,
	INVALID_BOOT_COMMAND,
	BPCM_WRITE_TIMEOUT,
	CMD_TABLE_FULL,
	CMD_TABLE_LOCKED,
};

// command codes
enum {
	// low-level commands
	cmdReserved = 0,
	cmdGetDevPresence = 1,
	cmdGetSWStrap = 2,
	cmdGetHWRev = 3,
	cmdGetNumZones = 4,
	cmdPing = 5,
	cmdGetNextLogEntry = 6,
	cmdGetRMON = 7,
	cmdSetClockHighGear = 8,
	cmdSetClockLowGear = 9,
	cmdSetClockGear = 10,
	cmdReadBpcmReg = 11,
	cmdReadZoneReg = 12,
	cmdWriteBpcmReg = 13,
	cmdWriteZoneReg = 14,
	// general-purpose high-level commands
	cmdSetRunState = 15,
	cmdSetPowerState = 16,
	cmdShutdownAllowed = 17,
	cmdGetSelect0 = 18,
	cmdGetSelect3 = 19,
	cmdGetAvsDisableState = 20,
	cmdGetPVT = 21,
	// specific-purpose high-level commands
	cmdPowerDevOnOff = 22,
	cmdPowerZoneOnOff = 23,
	cmdResetDevice = 24,
	cmdResetZone = 25,
	cmdAllocateG2UDQM = 26,
	cmdQSMAvailable = 27,
	cmdRevision = 28,
	cmdRegisterCmdHandler = 29,
	cmdFindUnusedCommand = 30,
	cmdLockCmdTable = 31,
	cmdJumpApp = 32,
	cmdStall = 33,
	cmdCloseAVS = 34,
	cmdReadROs = 35,
	cmdGetTrackTemp = 36,
	cmdSetTrackTemp = 37,
#ifdef PMC_IMPL_3_X
	cmdGetIslandStatus = 38,
	cmdGetTMON = 39,
	cmdSetTemperatureThresholds = 40,
	cmdResetTemperatureWarning = 41,
#endif
};

extern void ProcessCommand(TCommand * cmd, TCommand * response);
extern int SendCommand(int cmdID, int devAddr, int zone, int island, uint32_t word2,
		uint32_t word3, TCommand *rsp);
int SendAndWait(TCommand * cmd, TCommand * rsp);


#endif // PMCCOMMAND_H
