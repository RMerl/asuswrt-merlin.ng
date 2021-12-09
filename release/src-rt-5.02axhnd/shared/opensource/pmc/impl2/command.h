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

#include "bcmtypes.h"

typedef union {
	struct {
		uint32 srcPort	: 8;
		uint32 msgID	: 8;
		uint32 error	: 8;
		uint32 cmdID	: 8;
	} Bits;
	uint32 Reg32; 
} TCommandWord0;

typedef union {
	struct {
		uint32 logNum	: 8;
		uint32 island	: 4;
		uint32 devAddr	: 10;
		uint32 zoneIdx  : 10;
	} Bits;
	uint32	Reg32;
} TCommandWord1;

// Ping, GetNextLogEntry, GetRMON and Sigma
typedef struct {
	uint32			unused[2];
} TCommandNoParams;

typedef struct {
	uint32			params[2];
} TCommandGenericParams;

// PowerZoneOnOff, SetRunState, SetPowerState
typedef struct {
	uint8			state;
	uint8			reserved[3];
	uint32			unused;
} TCommandStateOnly;

// PowerDevOnOff
typedef struct {
	uint8			state;
	uint8			restore;
	uint8			reserved[2];
	uint32			unused;
} TCommandPowerDevice;

// PowerOffIsland
typedef struct {
	uint8			restore;
	uint8			reserved[3];
	uint32			unused;
} TCommandPowerIsland;

// SetClockLowGear, SetClockHighGear
typedef struct {
	uint8			clkN;
	uint8			reserved[3];
	uint32			unused;
} TCommandSetClockN;

// SetClockGear
typedef struct {
	uint8			gear;
	uint8			reserved[3];
	uint32			unused;
} TCommandSetClockGear;

typedef struct {
	uint8			queueNumber;
	uint8			tokenSize;
	uint8			numTokens;
	uint8			unused1;
	uint8			low_watermark;
	uint8			high_watermark;
	uint16			unused2;
} TCommandAllocDQM;

typedef struct {
	uint32			word2;
	uint32			word3;
} TCommandResponse;

typedef struct {
	TCommandWord0	word0;
	TCommandWord1	word1;
	union {
		TCommandNoParams		cmdNoParams;
		TCommandGenericParams	cmdGenericParams;
		TCommandStateOnly		cmdStateOnlyParam;
		TCommandPowerDevice		cmdPowerDevice;
		TCommandPowerIsland		cmdPowerIsland;
		TCommandSetClockN		cmdSetClockN;
		TCommandSetClockGear	cmdSetClockGear;
        TCommandAllocDQM        cmdAllocDqm;
		TCommandResponse		cmdResponse;
	} u;
} TCommand;

// special values to select all devices/zones
#define ALL_DEVICES 0x3ff
#define ALL_ZONES   0x3ff
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
    INVALID_BOOT_COMMAND
};

// command codes
enum {
	// low-level commands
	cmdreserved = 0,
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
	// reserved 15..63
	// general-purpose high-level commands
	cmdSetRunState = 64,
	cmdSetPowerState,
	cmdShutdownAllowed,
	cmdGetSelect0,
	cmdGetSelect3,
	cmdTuneRunner,
	// reserved 70..127
	// specific-purpose high-level commands
	cmdPowerOffIsland = 128,
	cmdPowerDevOnOff,
	cmdPowerZoneOnOff,
	cmdResetDevice,
	cmdResetZone,
#ifndef BCM_6838
    cmdAllocateG2UDQM,
	cmdQSMAvailable,
#endif //#ifndef BCM_6838
	// reserved 135..255
};

extern void ProcessCommand(TCommand *cmd, TCommand *response);

#endif // PMCCOMMAND_H

