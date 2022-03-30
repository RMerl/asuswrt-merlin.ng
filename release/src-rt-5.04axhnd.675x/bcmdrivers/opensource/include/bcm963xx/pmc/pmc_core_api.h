/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */
/*
<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:> 
*/

/*****************************************************************************
 *  Description:
 *      This contains header for PMC driver.
 *****************************************************************************/

#ifndef PMC_CORE_API_H
#define PMC_CORE_API_H

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

#ifndef CONFIG_BCM_PMC
static inline int pmc_convert_pvtmon(int sel, int value)
{
    return 0;
}

static inline int GetPVT(int sel, int island, int *value)
{
	return 0;
}

static inline int GetRCalSetting_1UM_VERT(int *rcal)
{
	return 0;
}
#define GetPVTKH2(s, i, v) (0)
#else
int pmc_convert_pvtmon(int sel, int value);
int GetPVT(int sel, int island, int *value);
int GetPVTKH2(int sel, int island, int *value);
#endif
int StallPmc(void);
int UnstallPmc(void);
int GetAvsDisableState(int island, int *state);
int GetRCalSetting(int resistor, int *rcal);
int GetRCalSetting_1UM_VERT(int *rcal);
int RecloseAVS(int iscold);

#endif
