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

//****************************************************************************
//  Description:
//      This contains the 6838 PMC driver.
//****************************************************************************

#include "pmc_drv.h"
#include "bl_os_wraper.h"
#include "shared_utils.h"
// to be defined to access BPCMs directly, no PMC
//#define PMB_DIRECT

#if defined _CFE_ && defined PMB_DIRECT
#undef offsetof
#define offsetof(TYPE, MEMBER) ((unsigned long) &((TYPE *)0)->MEMBER)
#endif

#ifndef _CFE_
#include <linux/spinlock.h>
#include <linux/slab.h>

static DEFINE_SPINLOCK(lock);
#endif

#ifndef PMB_DIRECT
#include "command.h"
#include "pmcapp.h"

#define PMC_BASE 0xb3e00000

#define PMC_MBOX0 *(volatile uint32 *)(PMC_BASE + 0x102c) 
#define PMC_MBOX1 *(volatile uint32 *)(PMC_BASE + 0x1028)

#define PMC_ADDRESS_1_WINDOW_MASK *(volatile uint32 *)(PMC_BASE + 0x106c)
#define PMC_ADDRESS_1_WINDOW_BASE_IN *(volatile uint32 *)(PMC_BASE + 0x1070)
#define PMC_ADDRESS_1_WINDOW_BASE_OUT *(volatile uint32 *)(PMC_BASE + 0x1074)

#define PMC_ADDRESS_2_WINDOW_MASK *(volatile uint32 *)(PMC_BASE + 0x1078)
#define PMC_ADDRESS_2_WINDOW_BASE_IN *(volatile uint32 *)(PMC_BASE + 0x107c)
#define PMC_ADDRESS_2_WINDOW_BASE_OUT *(volatile uint32 *)(PMC_BASE + 0x1080)

#define PMC_SOFT_RESETS *(volatile uint32 *)(PMC_BASE + 0x108c)
#define PMC_TIMER_2_CTL *(volatile uint32 *)(PMC_BASE + 0x10b4)
#define PMC_DQM_NOT_EMPTY_STS *(volatile uint32 *)(PMC_BASE + 0x1820)

#define PMC_DQM_1 ((volatile uint32 *)(PMC_BASE + 0x1c10))
#define PMC_DQM_2 ((volatile uint32 *)(PMC_BASE + 0x1c20))

static int SendAndWait(TCommand *cmd, TCommand *rsp)
{
    static uint32 reqdID = 1;
	int status = kPMC_COMMAND_TIMEOUT;
	TCommand dummy;

#ifndef _CFE_
	unsigned long flags;

	spin_lock_irqsave(&lock, flags);
#endif

	cmd->word0.Bits.msgID = reqdID;
	
    // send the command
    PMC_DQM_1[0] = cmd->word0.Reg32;
    PMC_DQM_1[1] = cmd->word1.Reg32;
    PMC_DQM_1[2] = cmd->u.cmdGenericParams.params[0];
    PMC_DQM_1[3] = cmd->u.cmdGenericParams.params[1];

    // wait for no more than 5ms for the reply
    PMC_TIMER_2_CTL = ((1 << 31) | (1 << 30) | (1 << 29) | 5000);

	while(!(PMC_DQM_NOT_EMPTY_STS & 0x4) && (PMC_TIMER_2_CTL & (1 << 31)));
	
	if(PMC_DQM_NOT_EMPTY_STS & 0x4)
	{
		if(!rsp)
			rsp = &dummy;
		
		// command didn't timeout, fill in the response
		rsp->word0.Reg32 = PMC_DQM_2[0];
		rsp->word1.Reg32 = PMC_DQM_2[1];
		rsp->u.cmdGenericParams.params[0] = PMC_DQM_2[2];
		rsp->u.cmdGenericParams.params[1] = PMC_DQM_2[3];

		if(rsp->word0.Bits.msgID == reqdID)
			status = rsp->word0.Bits.error;
		else
			status = kPMC_MESSAGE_ID_MISMATCH;
	}
	
	reqdID = (reqdID + 1) & 0xff;

#ifndef _CFE_
	spin_unlock_irqrestore(&lock, flags);
#endif

	return status;
}

static int SendCommand(int cmdID, int devAddr, int zone, uint32 word2, uint32 word3, TCommand *rsp)
{
    TCommand cmd;

	cmd.word0.Reg32 = 0;
	cmd.word0.Bits.cmdID = cmdID;
	cmd.word1.Reg32 = 0;
	cmd.word1.Bits.devAddr = devAddr;
	cmd.word1.Bits.zoneIdx = zone;
	cmd.u.cmdGenericParams.params[0] = word2;
	cmd.u.cmdGenericParams.params[1] = word3;
	
	return SendAndWait(&cmd, rsp);
}

static int SendStateCommand(int cmdID, int island, int state)
{
    TCommand cmd;
	int status;
	
    if((unsigned)island > 2) 
        status = kPMC_INVALID_ISLAND;
    else if((unsigned)state > 15)
        status = kPMC_INVALID_STATE;
    else
    {
		cmd.word0.Reg32 = 0;
        cmd.word0.Bits.cmdID = cmdID;
		cmd.word1.Reg32 = 0;
        cmd.word1.Bits.island = island;
        cmd.u.cmdStateOnlyParam.state = state;
        status = SendAndWait(&cmd, 0);
    }
	
	return status;
}

int GetDevPresence(int devAddr, int *value)
{
	TCommand rsp;
    int status = SendCommand(cmdGetDevPresence, devAddr, 0, 0, 0, &rsp);
	
    if(status == kPMC_NO_ERROR)
    {
        *value = (rsp.u.cmdResponse.word2 & 1) ? TRUE : FALSE;
    }
	else
	{
        *value = FALSE;
	}
	
    return status;
}
EXPORT_SYMBOL(GetDevPresence);

int GetSWStrap(int devAddr, int *value)
{
	TCommand rsp;
    int status = SendCommand(cmdGetSWStrap, devAddr, 0, 0, 0, &rsp);
	
    if(status == kPMC_NO_ERROR)
    {
        *value = rsp.u.cmdResponse.word2 & 0xffff;
    }
	
    return status;
}
EXPORT_SYMBOL(GetSWStrap);


int GetHWRev(int devAddr, int *value)
{
	TCommand rsp;
    int status = SendCommand(cmdGetHWRev, devAddr, 0, 0, 0, &rsp);
	
    if(status == kPMC_NO_ERROR)
    {
        *value = rsp.u.cmdResponse.word2 & 0xff;
    }
	
    return status;
}
EXPORT_SYMBOL(GetHWRev);

int GetNumZones(int devAddr, int *value)
{
	TCommand rsp;
    int status = SendCommand(cmdGetNumZones, devAddr, 0, 0, 0, &rsp);
	
    if(status == kPMC_NO_ERROR)
    {
        *value = rsp.u.cmdResponse.word2 & 0xff;
    }
	
    return status;
}
EXPORT_SYMBOL(GetNumZones);

int GetAvsDisableState(int ignored, int *state)
{
    return kPMC_INVALID_COMMAND;
}
EXPORT_SYMBOL(GetAvsDisableState);

int Ping(void)
{
    return SendCommand(cmdPing, 0, 0, 0, 0, 0);
}
EXPORT_SYMBOL(Ping);


int GetErrorLogEntry(TErrorLogEntry *logEntry)
{
    int status = SendCommand(cmdGetNextLogEntry, 0, 0, 0, 0, (TCommand *)logEntry);
	
    return status;
}
EXPORT_SYMBOL(GetErrorLogEntry);

int SetClockHighGear(int devAddr, int zone, int clkN)
{
    TCommand cmd;

    if(clkN < 0 || clkN > 7)
        return kPMC_INVALID_PARAM;
		
	cmd.word0.Reg32 = 0;
	cmd.word0.Bits.cmdID = cmdSetClockHighGear;
	cmd.word1.Reg32 = 0;
	cmd.word1.Bits.devAddr = devAddr;
	cmd.word1.Bits.zoneIdx = zone;
	cmd.u.cmdSetClockN.clkN = clkN;
	cmd.u.cmdGenericParams.params[1] = 0;
	
	return SendAndWait(&cmd, 0);
}
EXPORT_SYMBOL(SetClockHighGear);


int SetClockLowGear(int devAddr, int zone, int clkN)
{
    TCommand cmd;

    if(clkN < 0 || clkN > 7)
        return kPMC_INVALID_PARAM;
		
	cmd.word0.Reg32 = 0;
	cmd.word0.Bits.cmdID = cmdSetClockLowGear;
	cmd.word1.Reg32 = 0;
	cmd.word1.Bits.devAddr = devAddr;
	cmd.word1.Bits.zoneIdx = zone;
	cmd.u.cmdSetClockN.clkN = clkN;
	cmd.u.cmdGenericParams.params[1] = 0;
	
	return SendAndWait(&cmd, 0);
}
EXPORT_SYMBOL(SetClockLowGear);

int SetClockGear(int devAddr, int zone, int gear)
{
    TCommand cmd;

    if((unsigned)gear > 3)
        return kPMC_INVALID_PARAM;
		
	cmd.word0.Reg32 = 0;
	cmd.word0.Bits.cmdID = cmdSetClockGear;
	cmd.word1.Reg32 = 0;
	cmd.word1.Bits.devAddr = devAddr;
	cmd.word1.Bits.zoneIdx = zone;
	cmd.u.cmdSetClockGear.gear = gear;
	cmd.u.cmdGenericParams.params[1] = 0;
	
	return SendAndWait(&cmd, 0);
}
EXPORT_SYMBOL(SetClockGear);

int ReadBPCMRegister(int devAddr, int wordOffset, uint32 *value)
{
	TCommand rsp;
	int status;

	*value = 0;
#ifndef _CFE_	
	if((devAddr == PMB_ADDR_APM) && (UtilGetChipRev() <= 1))
 		return kPMC_INVALID_COMMAND;
#endif	
	status = SendCommand(cmdReadBpcmReg, devAddr, 0, wordOffset, 0, &rsp);
	
	if(status == kPMC_NO_ERROR)
	{
		*value = rsp.u.cmdResponse.word2;
	}

    return status;
}

int ReadZoneRegister(int devAddr, int zone, int wordOffset, uint32 *value)
{
	TCommand rsp;
	int status = kPMC_INVALID_PARAM;
	
    if((unsigned)wordOffset < 4)
    {
        status = SendCommand(cmdReadZoneReg, devAddr, zone, wordOffset, 0, &rsp);
        if(status == kPMC_NO_ERROR)
        {
            *value = rsp.u.cmdResponse.word2;
        }
    }

    return status;
}

int WriteBPCMRegister(int devAddr, int wordOffset, uint32 value)
{
#ifndef _CFE_   
    if((devAddr == PMB_ADDR_APM) && (UtilGetChipRev() <= 1))
        return kPMC_INVALID_COMMAND;
#endif    

    return SendCommand(cmdWriteBpcmReg, devAddr, 0, wordOffset, value, 0);
}

int WriteZoneRegister(int devAddr, int zone, int wordOffset, uint32 value)
{
    if((unsigned)wordOffset >= 4)
        return kPMC_INVALID_PARAM;

    return SendCommand(cmdWriteZoneReg, devAddr, 0, wordOffset, value, 0);
}
EXPORT_SYMBOL(WriteZoneRegister);


int SetRunState(int island, int state)
{
    if((unsigned)state >= 16)
        return kPMC_INVALID_PARAM;

    return SendStateCommand(cmdSetRunState, island, state);
}
EXPORT_SYMBOL(SetRunState);

int SetPowerState(int island, int state)
{
    if((unsigned)state >= 4)
        return kPMC_INVALID_PARAM;
    
    return SendStateCommand(cmdSetPowerState, island, state);
}
EXPORT_SYMBOL(SetPowerState);

int GetSelect0(void)
{
    TCommand rsp;
    int status = SendCommand(cmdGetSelect0, 0, 0, 0, 0, &rsp);
	
    if(status == kPMC_NO_ERROR)
        return rsp.u.cmdResponse.word2;
    else
        return -1000000;
}
EXPORT_SYMBOL(GetSelect0);

int GetSelect3(void)
{
    TCommand rsp;
    int status = SendCommand(cmdGetSelect3, 0, 0, 0, 0, &rsp);
	
    if(status == kPMC_NO_ERROR)
        return rsp.u.cmdResponse.word2;
    else
        return 0;
}
EXPORT_SYMBOL(GetSelect3);

int PowerOnDevice(int devAddr)
{
    TCommand cmd;

#ifndef _CFE_   
	if((devAddr == PMB_ADDR_APM) && (UtilGetChipRev() <= 1))
 		return kPMC_INVALID_COMMAND;
#endif	

	cmd.word0.Reg32 = 0;
	cmd.word0.Bits.cmdID = cmdPowerDevOnOff;
	cmd.word1.Reg32 = 0;
	cmd.word1.Bits.devAddr = devAddr;
	cmd.u.cmdPowerDevice.state = 1;
	cmd.u.cmdGenericParams.params[1] = 0;
	
	return SendAndWait(&cmd, 0);
}

int PowerOffDevice(int devAddr, int repower)
{
    TCommand cmd;

#ifndef _CFE_   
	if((devAddr == PMB_ADDR_APM) && (UtilGetChipRev() <= 1))
 		return kPMC_INVALID_COMMAND;
#endif	

	cmd.word0.Reg32 = 0;
	cmd.word0.Bits.cmdID = cmdPowerDevOnOff;
	cmd.word1.Reg32 = 0;
	cmd.word1.Bits.devAddr = devAddr;
	cmd.u.cmdPowerDevice.state = 0;
	cmd.u.cmdPowerDevice.restore = repower;
	cmd.u.cmdGenericParams.params[1] = 0;
	
	return SendAndWait(&cmd, 0);
}

int PowerOnZone(int devAddr, int zone)
{
#ifndef _CFE_   
	if((devAddr == PMB_ADDR_APM) && (UtilGetChipRev() <= 1))
 		return kPMC_INVALID_COMMAND;
	else
#endif	
		return SendCommand(cmdPowerZoneOnOff, devAddr, zone, 1, 0, 0);
}

int PowerOffZone(int devAddr, int zone)
{
#ifndef _CFE_   
	if((devAddr == PMB_ADDR_APM) && (UtilGetChipRev() <= 1))
 		return kPMC_INVALID_COMMAND;
	else
#endif	
		return SendCommand(cmdPowerZoneOnOff, devAddr, zone, 0, 0, 0);
}

int ResetDevice(int devAddr)
{
#ifndef _CFE_   
	if((devAddr == PMB_ADDR_APM) && (UtilGetChipRev() <= 1))
 		return kPMC_INVALID_COMMAND;
	else
#endif	
		return SendCommand(cmdResetDevice, devAddr, 0, 0, 0, 0);
}

int ResetZone(int devAddr, int zone)
{
#ifndef _CFE_   
	if((devAddr == PMB_ADDR_APM) && (UtilGetChipRev() <= 1))
 		return kPMC_INVALID_COMMAND;
	else
#endif	
    	return SendCommand(cmdResetZone, devAddr, zone, 0, 0, 0);
}

int TuneRunner(void)
{
	return SendCommand(cmdTuneRunner, 0, 0, 0, 0, 0);
}
EXPORT_SYMBOL(TuneRunner);

void WaitPmc(int runState)
{
	while(((PMC_MBOX0 >> 24) & 7) != runState);
}

void BootPmcNoRom(unsigned long physAddr)
{
	PMC_ADDRESS_1_WINDOW_MASK = 0xffffc000;
	PMC_ADDRESS_1_WINDOW_BASE_IN =0x1fc00000;
	PMC_ADDRESS_1_WINDOW_BASE_OUT = physAddr;
	
	PMC_SOFT_RESETS &= ~1;
}
EXPORT_SYMBOL(BootPmcNoRom);

#ifndef _CFE_
#define PMC_APP_MAX_SIZE 0x2000
int __init pmc_init(void)
{
	unsigned long physAddrPmc;
	unsigned long rosc_ctrl;
	unsigned long rosc_cnt;
	
	printk("PMC Driver Init...");

	if(sizeof(pmcapp) > PMC_APP_MAX_SIZE)
	{
		printk("\nPMC app larger then max allowed (%dKB).\n", PMC_APP_MAX_SIZE/0x400);
		return -ENOMEM;
	}
	
	physAddrPmc = (unsigned long)kmalloc(PMC_APP_MAX_SIZE, GFP_KERNEL) & ~0xA0000000;
	if(!physAddrPmc) 
	{
		printk("\nUnable to allocate memory for PMC app.\n");
		return -ENOMEM;
	}
	
	PMC_ADDRESS_1_WINDOW_MASK = 0xffffffff;
	PMC_ADDRESS_1_WINDOW_BASE_IN =0xffffffff;
	PMC_ADDRESS_1_WINDOW_BASE_OUT = 0xffffffff;
				
	PMC_ADDRESS_2_WINDOW_MASK = 0xffffe000;
	PMC_ADDRESS_2_WINDOW_BASE_IN =0;
	PMC_ADDRESS_2_WINDOW_BASE_OUT = physAddrPmc;
				
	PMC_MBOX1 = (unsigned long)pmcapp & ~0xA0000000;
	
	WaitPmc(kPMCRunStateRunning);
	ReadBPCMRegister(PMB_ADDR_APM, 4, &rosc_ctrl);
	ReadBPCMRegister(PMB_ADDR_APM, 6, &rosc_cnt);
	while(rosc_cnt)
	{
		WriteBPCMRegister(PMB_ADDR_APM, 4, rosc_ctrl);
		WriteBPCMRegister(PMB_ADDR_APM, 4, rosc_ctrl & ~3);
		PMC_TIMER_2_CTL = ((1 << 31) | (1 << 29) | 5000);
		while(PMC_TIMER_2_CTL & (1 << 31));
		ReadBPCMRegister(PMB_ADDR_APM, 6, &rosc_cnt);
	}
	printk(" done.\n");
	
	return 0;
}
postcore_initcall(pmc_init);
#endif

#else //#ifdef PMB_DIRECT

#include "BPCM.h"

#define PMC_PMBM_CONTROL(bus) *((volatile unsigned long *)0xb3e800c0 + bus*8)
	#define PMBM_CTL_START (1<<31)
	#define PMBM_CTL_TIMEOUT (1<<30)
	#define PMBM_CTL_EN (1 << 24)
	#define kPMBRead (0 << 20)
	#define kPMBWrite (1 << 20)
#define PMC_PMBM_WR_DATA(bus) *((volatile unsigned long *)0xb3e800c4 + bus*8)
#define PMC_PMBM_RD_DATA(bus) *((volatile unsigned long *)0xb3e800cc + bus*8)

int WriteBPCMRegister(int devAddr, int wordOffset, uint32 value)
{
	int bus = devAddr >> 8;
	int status = kPMC_NO_ERROR;
	
#ifndef _CFE_
	unsigned long flags;

	spin_lock_irqsave(&lock, flags);
#endif

    PMC_PMBM_WR_DATA(bus) = value;
    PMC_PMBM_CONTROL(bus) = PMBM_CTL_EN | kPMBWrite | ((devAddr & 0xff) << 12) | wordOffset;
    PMC_PMBM_CONTROL(bus) |= PMBM_CTL_START;
	
    while(PMC_PMBM_CONTROL(bus) & PMBM_CTL_START);
	
    if(PMC_PMBM_CONTROL(bus) & PMBM_CTL_TIMEOUT)
 		status = kPMC_COMMAND_TIMEOUT;

#ifndef _CFE_
	spin_unlock_irqrestore(&lock, flags);
#endif

	return status;
}

int ReadBPCMRegister(int devAddr, int wordOffset, uint32 *value)
{
	int bus = devAddr >> 8;
	int status = kPMC_NO_ERROR;
	
#ifndef _CFE_
	unsigned long flags;

	spin_lock_irqsave(&lock, flags);
#endif

	PMC_PMBM_CONTROL(bus) = PMBM_CTL_EN | kPMBRead | ((devAddr & 0xff) << 12) | wordOffset;
    PMC_PMBM_CONTROL(bus) |= PMBM_CTL_START;
	
    while(PMC_PMBM_CONTROL(bus) & PMBM_CTL_START);
	
    if(PMC_PMBM_CONTROL(bus) & PMBM_CTL_TIMEOUT)
	{
		status = kPMC_COMMAND_TIMEOUT;
	}
	else
	{
		*value = PMC_PMBM_RD_DATA(bus);
	}
	
#ifndef _CFE_
	spin_unlock_irqrestore(&lock, flags);
#endif

	return status;
}

int PowerOnDevice(int devAddr)
{
    int ix, status;
	BPCM_CAPABILITES_REG capabilities;

	status = ReadBPCMRegister(devAddr, BPCMOffset(capabilities), &capabilities.Reg32);
    for(ix = 0; (ix < capabilities.Bits.num_zones) && (status != kPMC_NO_ERROR); ix++)
    {
    	BPCM_PWR_ZONE_N_CONTROL ctl;
    	status = ReadBPCMRegister(devAddr, BPCMOffset(zones[ix].control), &ctl.Reg32);
        if(status == kPMC_NO_ERROR && ctl.Bits.pwr_on_state == 0)
        {
            ctl.Bits.mem_pwr_ctl_en = 1;
            ctl.Bits.blk_reset_assert = 1;
            ctl.Bits.dpg_ctl_en = 1;
        	ctl.Bits.pwr_up_req = 1;
        	WriteBPCMRegister(devAddr, BPCMOffset(zones[ix].control), ctl.Reg32);
        }
    }
	
	return status;
}

int PowerOffDevice(int devAddr, int repower)
{
    // we can power off the entire device by powering off the 0th zone.
    BPCM_PWR_ZONE_N_CONTROL reg;
    int status;

    status = ReadBPCMRegister(devAddr, BPCMOffset(zones[0].control), &reg.Reg32);
	
	if(status == kPMC_NO_ERROR && reg.Bits.pwr_off_state == 0)
	{
		reg.Bits.pwr_dn_req = 1;
		WriteBPCMRegister(devAddr, BPCMOffset(zones[0].control), reg.Reg32);
	}
	
	return status;
}

int PowerOnZone(int devAddr, int zone)
{
	BPCM_PWR_ZONE_N_CONTROL reg;
	int status;

	status = ReadBPCMRegister(devAddr, BPCMOffset(zones[zone].control), &reg.Reg32);
	
	if(status == kPMC_NO_ERROR)
	{
		reg.Bits.pwr_up_req = 1;
		status = WriteBPCMRegister(devAddr, BPCMOffset(zones[zone].control), reg.Reg32);
	}
	
	return status;
}

int PowerOffZone(int devAddr, int zone)
{
 	BPCM_PWR_ZONE_N_CONTROL reg;
	int status;

	status = ReadBPCMRegister(devAddr, BPCMOffset(zones[zone].control), &reg.Reg32);
	
	if(status == kPMC_NO_ERROR)
	{
		reg.Bits.pwr_dn_req = 1;
		status = WriteBPCMRegister(devAddr, BPCMOffset(zones[zone].control), reg.Reg32);
	}
	
	return status;
}

int ResetDevice(int devAddr)
{
	// all zones had their blk_reset_assert bits set at initial config time
	BPCM_PWR_ZONE_N_CONTROL reg;
	int status;
	
	status = PowerOffDevice(devAddr, 0);
	
	do
	{
		status = ReadBPCMRegister(devAddr, BPCMOffset(zones[0].control), &reg.Reg32);
	}
	while((reg.Bits.pwr_off_state != 1) && (status != kPMC_NO_ERROR));
	
	if(status == kPMC_NO_ERROR)
		status = PowerOnDevice(devAddr);
	
	return status;
}

int ResetZone(int devAddr, int zone)
{
	BPCM_PWR_ZONE_N_CONTROL reg;
	int status;
	
	status = PowerOffZone(devAddr, zone);
	
	do
	{
		status = ReadBPCMRegister(devAddr, BPCMOffset(zones[zone].control), &reg.Reg32);
	}
	while((reg.Bits.pwr_off_state != 1) && (status != kPMC_NO_ERROR));
	
	if(status == kPMC_NO_ERROR)
		status = PowerOnZone(devAddr, zone);
	
	return status;
}

void WaitPmc(int runState)
{
}

#endif //#ifdef PMB_DIRECT

#ifdef _CFE_
/* PMC common API */
int pmc_init(void)
{
	return 0;
}

void pmc_initmode(void)
{
	return;
}
#endif

EXPORT_SYMBOL(ReadBPCMRegister);
EXPORT_SYMBOL(WriteBPCMRegister);
EXPORT_SYMBOL(ResetZone);
EXPORT_SYMBOL(ResetDevice);
EXPORT_SYMBOL(PowerOffZone);
EXPORT_SYMBOL(PowerOnZone);
EXPORT_SYMBOL(PowerOffDevice);
EXPORT_SYMBOL(PowerOnDevice);
