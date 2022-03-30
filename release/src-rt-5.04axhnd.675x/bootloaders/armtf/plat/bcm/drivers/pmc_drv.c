/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2019 Broadcom Ltd.
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

#include <debug.h>
#include <platform_def.h>
#include <delay_timer.h>
#include "pmc_drv.h"
#include "BPCM.h"
#include "command.h"

#if IS_BCMCHIP(6858) || defined(PMC_IMPL_3_X)
#include "clk_rst.h"
#endif

int pmc_mode = PMC_MODE_DQM;

static int SendAndWait(TCommand * cmd, TCommand * rsp)
{
#if defined(PMC_ON_HOSTCPU)
	rsp->u.cmdGenericParams.params[0] = 0;
	rsp->u.cmdGenericParams.params[1] = 0;
	return 0;
#else
#if IS_BCMCHIP(63138)
	/* translate new cmdID into old cmdID that the pmc will understand NB:
	 * requires cmdIDs below to be the new versions
	 */

	static const unsigned char newToOldcmdIDMap[] = {
		[cmdSetRunState] = 64,	// cmdSetRunState,
		[cmdSetPowerState] = 65,	// cmdSetPowerState,
		[cmdShutdownAllowed] = 66,	// cmdShutdownAllowed,
		[cmdGetSelect0] = 67,	// cmdGetSelect0,
		[cmdGetSelect3] = 68,	// cmdGetSelect3,
		[cmdGetAvsDisableState] = 69,	// cmdGetAvsDisableState,
		[cmdGetPVT] = 70,	// cmdGetPVT,
		[cmdPowerDevOnOff] = 129,	// cmdPowerDevOnOff,
		[cmdPowerZoneOnOff] = 130,	// cmdPowerZoneOnOff,
		[cmdResetDevice] = 131,	// cmdResetDevice,
		[cmdResetZone] = 132,	// cmdResetZone,
		[cmdAllocateG2UDQM] = 133,	// cmdAllocateG2UDQM,
		[cmdQSMAvailable] = 134,	// cmdQSMAvailable,
		[cmdRevision] = 135,	// cmdRevision,
	};

	static int pmc_remap = 0;
#endif
	static uint32_t reqdID = 1;
	int status = kPMC_COMMAND_TIMEOUT;
	TCommand dummy;

#if defined(BOOT_MEMC_SRAM)
	reqdID = 1;
#endif
	pmc_spin_lock();

	/* clear previous rsp data if any */
	while (PMC->dqm.notEmptySts & PMC_DQM_RPL_STS) {
		if (!rsp)
			rsp = &dummy;

		rsp->word0.Reg32 = PMC->dqmQData[PMC_DQM_RPL_NUM].word[0];
		rsp->word1.Reg32 = PMC->dqmQData[PMC_DQM_RPL_NUM].word[1];
		rsp->u.cmdGenericParams.params[0] =
		    PMC->dqmQData[PMC_DQM_RPL_NUM].word[2];
		rsp->u.cmdGenericParams.params[1] =
		    PMC->dqmQData[PMC_DQM_RPL_NUM].word[3];

		printk
		    ("PMC reqdID=%d previous rsp.word[0-3]=0x[%08x %08x %08x %08x] status=%d\n",
		     reqdID, rsp->word0.Reg32, rsp->word1.Reg32,
		     rsp->u.cmdGenericParams.params[0],
		     rsp->u.cmdGenericParams.params[1], rsp->word0.Bits.error);
	}

#if IS_BCMCHIP(63138)
	if (pmc_remap && cmd->word0.Bits.cmdID < sizeof newToOldcmdIDMap &&
	    newToOldcmdIDMap[cmd->word0.Bits.cmdID])
		cmd->word0.Bits.cmdID = newToOldcmdIDMap[cmd->word0.Bits.cmdID];
#endif

#ifdef PMC_LOG_IN_DTCM
	if (cmd->word0.Bits.cmdID == cmdCloseAVS)
		PMC->ctrl.hostMboxOut = 1; // request sync dtcm log
#endif

	cmd->word0.Bits.msgID = reqdID;

	/* send the command */
	PMC->dqmQData[PMC_DQM_REQ_NUM].word[0] = cmd->word0.Reg32;
	PMC->dqmQData[PMC_DQM_REQ_NUM].word[1] = cmd->word1.Reg32;
	PMC->dqmQData[PMC_DQM_REQ_NUM].word[2] =
	    cmd->u.cmdGenericParams.params[0];
	PMC->dqmQData[PMC_DQM_REQ_NUM].word[3] =
	    cmd->u.cmdGenericParams.params[1];

#ifdef CONFIG_BRCM_IKOS
	/* We do not enable PMC TIMER here for IKOS, or it will wait forever */
	while (!(PMC->dqm.notEmptySts & PMC_DQM_RPL_STS)) ;
#elif defined(PMC_IMPL_3_X)
#ifdef PMC_LOG_IN_DTCM
	if (cmd->word0.Bits.cmdID == cmdCloseAVS) {
		while (!(PMC->dqm.notEmptySts & PMC_DQM_RPL_STS))
			pmc_show_log_item();

		PMC->ctrl.hostMboxOut = 0; // ignore dtcm log
	} 
	else
#endif // #ifdef PMC_LOG_IN_DTCM
	{
		PMC->ctrl.gpTmr0Ctl = ((1 << 31) | (1 << 29) |
				((400000 << 1) & 0x1fffffff));	// 400ms

		while (!(PMC->dqm.notEmptySts & PMC_DQM_RPL_STS) &&
				(PMC->ctrl.gpTmr0Ctl & (1 << 31))) ;
	}
#else
	PMC->ctrl.gpTmr2Ctl = ((1 << 31) | (1 << 29) | 400000);	// 400ms

	while (!(PMC->dqm.notEmptySts & PMC_DQM_RPL_STS) &&
	       (PMC->ctrl.gpTmr2Ctl & (1 << 31))) {
#if !defined(_CFE_) && (IS_BCMCHIP(63148) || IS_BCMCHIP(4908))
		/* Do not tight poll the PMC registers for longer command */
		if (cmd->word0.Bits.cmdID == cmdCloseAVS)
			udelay(1000);
#endif
	}
#endif /* CONFIG_BRCM_IKOS */

	if (PMC->dqm.notEmptySts & PMC_DQM_RPL_STS) {
		if (!rsp)
			rsp = &dummy;

		/* command didn't timeout, fill in the response */
		rsp->word0.Reg32 = PMC->dqmQData[PMC_DQM_RPL_NUM].word[0];
		rsp->word1.Reg32 = PMC->dqmQData[PMC_DQM_RPL_NUM].word[1];
		rsp->u.cmdGenericParams.params[0] =
		    PMC->dqmQData[PMC_DQM_RPL_NUM].word[2];
		rsp->u.cmdGenericParams.params[1] =
		    PMC->dqmQData[PMC_DQM_RPL_NUM].word[3];

		if (rsp->word0.Bits.msgID == reqdID)
			status = rsp->word0.Bits.error;
		else
			status = kPMC_MESSAGE_ID_MISMATCH;

		if (status != kPMC_NO_ERROR)
			printk
			    ("PMC reqdID=%d error=%d rsp.word[0-3]=0x[%08x %08x %08x %08x]\n",
			     reqdID, status, rsp->word0.Reg32, rsp->word1.Reg32,
			     rsp->u.cmdGenericParams.params[0],
			     rsp->u.cmdGenericParams.params[1]);
	}

	reqdID = (reqdID + 1) & 0xff;

	pmc_spin_unlock();

	return status;
#endif
}

static int SendCmd(TCommand * cmd, int cmdID, int devAddr, int zone, int island,
		   TCommand * rsp)
{
	cmd->word0.Reg32 = 0;
	cmd->word0.Bits.cmdID = cmdID;
	cmd->word1.Reg32 = 0;
	cmd->word1.Bits.devAddr = devAddr;
	cmd->word1.Bits.zoneIdx = zone;
	cmd->word1.Bits.island = island;

	return SendAndWait(cmd, rsp);
}

int SendCommand(int cmdID, int devAddr, int zone, int island, uint32_t word2,
		uint32_t word3, TCommand * rsp)
{
	TCommand cmd;

	cmd.u.cmdGenericParams.params[0] = word2;
	cmd.u.cmdGenericParams.params[1] = word3;

	return SendCmd(&cmd, cmdID, devAddr, zone, island, rsp);
}


#if defined(PMC_IMPL_3_X) || defined(PMC_ON_HOSTCPU)
#ifdef PMC_ON_HOSTCPU
#define KEYHOLE_IDX 1
#else
#define KEYHOLE_IDX 0
#endif
int read_bpcm_reg_direct(int devAddr, int wordOffset, uint32_t * value)
{
	int status = kPMC_NO_ERROR;
	int bus = (devAddr >> PMB_BUS_ID_SHIFT) & 0x3;
	uint32_t address, ctlSts;
	volatile PMB_keyhole_reg *keyhole = &PMB->keyhole[KEYHOLE_IDX];


	address =
	    ((devAddr & 0xff) *
	     ((PMB->
	       config >> PMB_NUM_REGS_SHIFT) & PMB_NUM_REGS_MASK)) |
	    (wordOffset);

	keyhole->control =
	    PMC_PMBM_START | (bus << PMC_PMBM_BUS_SHIFT) | (PMC_PMBM_Read) |
	    address;
	ctlSts = keyhole->control;
	while (ctlSts & PMC_PMBM_BUSY)
		ctlSts = keyhole->control;	/*wait for completion */

	if (ctlSts & PMC_PMBM_TIMEOUT)
		status = kPMC_COMMAND_TIMEOUT;
	else
		*value = keyhole->rd_data;

	return status;
}

int write_bpcm_reg_direct(int devAddr, int wordOffset, uint32_t value)
{
	int status = kPMC_NO_ERROR;
	int bus = (devAddr >> PMB_BUS_ID_SHIFT) & 0x3;
	uint32_t address, ctlSts;
	volatile PMB_keyhole_reg *keyhole = &PMB->keyhole[KEYHOLE_IDX];

	address =
	    ((devAddr & 0xff) *
	     ((PMB->
	       config >> PMB_NUM_REGS_SHIFT) & PMB_NUM_REGS_MASK)) |
	    (wordOffset);
	keyhole->wr_data = value;
	keyhole->control =
	    PMC_PMBM_START | (bus << PMC_PMBM_BUS_SHIFT) | (PMC_PMBM_Write) |
	    address;

	ctlSts = keyhole->control;
	while (ctlSts & PMC_PMBM_BUSY)
		ctlSts = keyhole->control;	/*wait for completion */

	if (ctlSts & PMC_PMBM_TIMEOUT)
		status = kPMC_COMMAND_TIMEOUT;

	return status;
}


#else // #if defined(PMC_IMPL_3_X) || defined(PMC_ON_HOSTCPU)
int read_bpcm_reg_direct(int devAddr, int wordOffset, uint32_t * value)
{
	int status = kPMC_NO_ERROR;
	int bus = (devAddr >> PMB_BUS_ID_SHIFT) & 0x3;
	volatile PMBMaster *pmbm_ptr;

	if (bus >= PMB_BUS_MAX)
		return kPMC_INVALID_BUS;

	pmbm_ptr = &(PROCMON->PMBM[bus]);

	/* Make sure PMBM is not busy */

	pmbm_ptr->ctrl = PMC_PMBM_START | PMC_PMBM_Read |
	    ((devAddr & 0xff) << 12) | wordOffset;

	while (pmbm_ptr->ctrl & PMC_PMBM_START) ;

	if (pmbm_ptr->ctrl & PMC_PMBM_TIMEOUT)
		status = kPMC_COMMAND_TIMEOUT;
	else
		*value = pmbm_ptr->rd_data;

	return status;
}

int write_bpcm_reg_direct(int devAddr, int wordOffset, uint32_t value)
{
	int bus = (devAddr >> PMB_BUS_ID_SHIFT) & 0x3;
	int status = kPMC_NO_ERROR;
	volatile PMBMaster *pmbm_ptr;
	if (bus >= PMB_BUS_MAX)
		return kPMC_INVALID_BUS;

	pmbm_ptr = &(PROCMON->PMBM[bus]);

	pmbm_ptr->wr_data = value;
	pmbm_ptr->ctrl = PMC_PMBM_START | PMC_PMBM_Write |
	    ((devAddr & 0xff) << 12) | wordOffset;

	while (pmbm_ptr->ctrl & PMC_PMBM_START) ;

	if (pmbm_ptr->ctrl & PMC_PMBM_TIMEOUT)
		status = kPMC_COMMAND_TIMEOUT;

	return status;
}
#endif // #if defined(PMC_IMPL_3_X) || defined(PMC_ON_HOSTCPU)

/* note: all the [Read|Write][BPCM|Zone]Register functions are different from
 * how they are defined in firmware code.  In the driver code, it takes in
 * wordOffset as the argument, but in the firmware code, it uses byteOffset */
int ReadBPCMRegister(int devAddr, int wordOffset, uint32_t * value)
{
	int status = kPMC_INVALID_STATE;

	if (PMC_ACCESS_BPCM_DIRECT || pmc_mode == PMC_MODE_PMB_DIRECT) {
		pmc_spin_lock();
		status = read_bpcm_reg_direct(devAddr, wordOffset, value);
		pmc_spin_unlock();
	} else if (pmc_mode == PMC_MODE_DQM) {
		TCommand rsp;
		status =
		    SendCommand(cmdReadBpcmReg, devAddr, 0, 0, wordOffset, 0,
				&rsp);

		if (status == kPMC_NO_ERROR)
			*value = rsp.u.cmdResponse.word2;
	}

	return status;
}

int WriteBPCMRegister(int devAddr, int wordOffset, uint32_t value)
{
	int status = kPMC_INVALID_STATE;

	if (PMC_ACCESS_BPCM_DIRECT || pmc_mode == PMC_MODE_PMB_DIRECT) {
		pmc_spin_lock();
		status = write_bpcm_reg_direct(devAddr, wordOffset, value);
		pmc_spin_unlock();
	} else if (pmc_mode == PMC_MODE_DQM) {
		status =
		    SendCommand(cmdWriteBpcmReg, devAddr, 0, 0, wordOffset,
				value, 0);
	}

	return status;
}


int PowerOnZone(int devAddr, int zone)
{
	BPCM_PWR_ZONE_N_CONTROL reg;
	int status;

#if IS_BCMCHIP(6858)
	/* Do not use DQM command cmdPowerZoneOnOff for non 6858 because this command is only available if a
	   PMC application has been uploaded to expand the PMC boot rom functionality */
	if (pmc_mode == PMC_MODE_DQM) {
		TCommand cmd = {0};
		cmd.u.cmdStateOnlyParam.state = 1;
		return SendCmd(&cmd, cmdPowerZoneOnOff, devAddr, zone, 0, 0);
	}
#endif

	status =
	    ReadBPCMRegister(devAddr, BPCMRegOffset(zones[zone].control),
			     &reg.Reg32);
	if (status == kPMC_NO_ERROR && reg.Bits.pwr_on_state == 0) {
		reg.Bits.pwr_dn_req = 0;
		reg.Bits.dpg_ctl_en = 1;
		reg.Bits.pwr_up_req = 1;
		reg.Bits.mem_pwr_ctl_en = 1;
		reg.Bits.blk_reset_assert = 1;
		status =
		    WriteBPCMRegister(devAddr,
				      BPCMRegOffset(zones[zone].control),
				      reg.Reg32);
	}
	return status;
}

int PowerOnDevice(int devAddr)
{
	if (PMC_ACCESS_BPCM_DIRECT || pmc_mode == PMC_MODE_PMB_DIRECT) {
		int ix, status;
		BPCM_CAPABILITES_REG capabilities;

		status =
		    ReadBPCMRegister(devAddr, BPCMRegOffset(capabilities),
				     &capabilities.Reg32);
		for (ix = 0;
		     (ix < capabilities.Bits.num_zones)
		     && (status == kPMC_NO_ERROR); ix++) {
			status = PowerOnZone(devAddr, ix);
		}

		return status;
	}

	if (pmc_mode == PMC_MODE_DQM) {
		TCommand cmd = {0};
		cmd.u.cmdPowerDevice.state = 1;
		return SendCmd(&cmd, cmdPowerDevOnOff, devAddr, 0, 0, 0);
	}

	return kPMC_INVALID_STATE;
}

int PowerOffDevice(int devAddr, int repower)
{
	if (PMC_ACCESS_BPCM_DIRECT || pmc_mode == PMC_MODE_PMB_DIRECT) {
		/* we can power off the entire device by powering off the 0th zone. */
		BPCM_PWR_ZONE_N_CONTROL reg;
		int status;

		status =
		    ReadBPCMRegister(devAddr, BPCMRegOffset(zones[0].control),
				     &reg.Reg32);

		if (status == kPMC_NO_ERROR && reg.Bits.pwr_off_state == 0) {
			reg.Bits.pwr_dn_req = 1;
			WriteBPCMRegister(devAddr,
					  BPCMRegOffset(zones[0].control),
					  reg.Reg32);
		}

		return status;
	}

	if (pmc_mode == PMC_MODE_DQM) {
		TCommand cmd = {0};
		cmd.u.cmdPowerDevice.state = 0;
		cmd.u.cmdPowerDevice.restore = repower;
		return SendCmd(&cmd, cmdPowerDevOnOff, devAddr, 0, 0, 0);
	}

	return kPMC_INVALID_STATE;
}

int ResetDevice(int devAddr)
{
	/* all zones had their blk_reset_assert bits set at initial config time */
	BPCM_PWR_ZONE_N_CONTROL reg;
	int status;

#if IS_BCMCHIP(6858)
	/* Do not use DQM command cmdResetDevice for non 6858 because this command is only available if a
	   PMC application has been uploaded to expand the PMC boot rom functionality */
	if (pmc_mode == PMC_MODE_DQM)
		return SendCommand(cmdResetDevice, devAddr, 0, 0, 0, 0, 0);
#endif

	status = PowerOffDevice(devAddr, 0);
	do {
		status =
		    ReadBPCMRegister(devAddr, BPCMRegOffset(zones[0].control),
				     &reg.Reg32);
	} while ((reg.Bits.pwr_off_state != 1) && (status == kPMC_NO_ERROR));
	if (status == kPMC_NO_ERROR)
		status = PowerOnDevice(devAddr);
	return status;
}

// initalize pmc_mode (possibly) before printk available
void pmc_initmode(void)
{
#if defined MISC_STRAP_BUS_PMC_ROM_BOOT
	/* read the strap pin and based on the strap pin, choose the mode */
	if ((MISC->miscStrapBus & MISC_STRAP_BUS_PMC_ROM_BOOT) == 0)
		pmc_mode = PMC_MODE_PMB_DIRECT;
#else // #if defined MISC_STRAP_BUS_PMC_ROM_BOOT
#if IS_BCMCHIP(6858)
	pmc_mode = PMC_MODE_DQM;
#else
	pmc_mode = PMC_MODE_PMB_DIRECT;
#ifdef CFG_RAMAPP
#if IS_BCMCHIP(63158)
	/* MIPS based PMC */
	if (PMC->ctrl.softResets == 0)
		pmc_mode = PMC_MODE_DQM;
#elif defined(PMC_IMPL_3_X)
	/* Maestro based PMC) */
	if (PROCMON->maestroReg.coreCtrl.coreEnable == 1)
		pmc_mode = PMC_MODE_DQM;
#endif // #if IS_BCMCHIP(63158)
#endif // #ifdef CFG_RAMAPP
#endif // #if IS_BCMCHIP(6858)
#endif // #if defined MISC_STRAP_BUS_PMC_ROM_BOOT
}




