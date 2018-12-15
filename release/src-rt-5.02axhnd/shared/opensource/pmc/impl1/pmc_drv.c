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
 *      This contains the PMC driver.
 *****************************************************************************/

#include "pmc_drv.h"
#include "bcm_map_part.h"

#define PMC_MODE_DQM		0
#define PMC_MODE_PMB_DIRECT	1

static int pmc_mode = PMC_MODE_DQM;

#ifdef _CFE_
#include "lib_printf.h"
#include "lib_types.h"
#include "lib_string.h"
#include "cfe_iocb.h"
#include "bsp_config.h"
#include "bcm63xx_util.h"
#define printk	printf
#else
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/module.h>
static DEFINE_SPINLOCK(lock);
#endif

#include "command.h"
#include "BPCM.h"
#include "shared_utils.h"

#if defined(_BCM96858_) || defined(CONFIG_BCM96858) || defined(_BCM96836_) || defined(CONFIG_BCM96836) || defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856)
#include "bcm_otp.h"
#include "clk_rst.h"
#endif

extern void _cfe_flushcache(int,uint8_t *,uint8_t *);
extern int  getAVSConfig(void);

#if defined (CFG_RAMAPP) && !defined(CONFIG_BRCM_IKOS) && (\
    defined (_BCM96836_)  || \
    defined (_BCM96846_) || \
    defined (_BCM96856_) || \
    defined (_BCM963158_) \
    )
#define PMC_RAM_BOOT
//#if defined(_BCM6846_) || defined(_BCM6856_) 
//#define AVS_DEBUG
//#endif
#if defined (_BCM963158_)
#define PMC_IN_MAIN_LOOP    kPMCRunStateRunning
#else
#define PMC_IN_MAIN_LOOP    6
#endif
#endif

#if defined(_BCM96836_) || defined(CONFIG_BCM96836)
static char gIsClassic[64];
#endif

#if defined CONFIG_BCM963138 || defined _BCM963381_
/* translate new cmdID into old cmdID that the pmc will understand
 * NB: requires cmdIDs below to be the new versions
 */
static const unsigned char newToOldcmdIDMap[] = {
	[cmdSetRunState]	=  64,	// cmdSetRunState,
	[cmdSetPowerState]	=  65,	// cmdSetPowerState,
	[cmdShutdownAllowed]	=  66,	// cmdShutdownAllowed,
	[cmdGetSelect0]		=  67,	// cmdGetSelect0,
	[cmdGetSelect3]		=  68,	// cmdGetSelect3,
	[cmdGetAvsDisableState]	=  69,	// cmdGetAvsDisableState,
	[cmdGetPVT]		=  70,	// cmdGetPVT,
	[cmdPowerDevOnOff]	= 129,	// cmdPowerDevOnOff,
	[cmdPowerZoneOnOff]	= 130,	// cmdPowerZoneOnOff,
	[cmdResetDevice]	= 131,	// cmdResetDevice,
	[cmdResetZone]		= 132,	// cmdResetZone,
	[cmdAllocateG2UDQM]	= 133,	// cmdAllocateG2UDQM,
	[cmdQSMAvailable]	= 134,	// cmdQSMAvailable,
	[cmdRevision]		= 135,	// cmdRevision,
};

static int pmc_remap = 0;
#endif

#define CHECK_DEV_ADDRESSABLE(devAddr)   \
	if ( !deviceAddressable(devAddr) ) \
		return kPMC_INVALID_DEVICE;

static bool deviceAddressable( int devAddr )
{
	switch (devAddr)
	{
#if defined(CONFIG_BCM96838)
		case PMB_ADDR_APM:
			if (UtilGetChipRev() <= 1)
			{
				return FALSE;
			}
			break;
#endif /* defined(CONFIG_BCM96838) */

		default:
			break;
	}
	return TRUE;
}

#if defined(BOOT_MEMC_SRAM)
static uint32 reqdID;
#endif

#ifndef _CFE_
/* PMC function can take very long time in a few ms. Driver should not call it in interrupt 
   context(hardware interrupt, bottom half and softirq).  Add check function to print out
   warning and call stack when this happens */
static void CheckIntrContext(void)
{
	if (in_interrupt()) {
		printk("PMC driver function called in interrupt context !!!!\n"); 
		/*dump_stack();*/
	}

	return;
}
#endif

#if defined(CONFIG_BCM96846) || defined(_BCM96846_) || defined(CONFIG_BCM96856) || defined(_BCM96856_)
#include "pmc_direct_access_pmc3.h"
#elif defined(CONFIG_BCM96836) || defined(_BCM96836_)
#include "pmc_direct_access_pmc2.h"
#else
#include "pmc_direct_access_pmc1.h"
#endif

#if !defined MISC_STRAP_BUS_PMC_ROM_BOOT
static int is_pmc_running(void)
{
    int pmc_state = 0;
#if defined (CONFIG_BCM963158) || defined(CONFIG_BCM96836) || defined(_BCM96836_) 
    /* MIPS pased PMC*/
	if (PMC->ctrl.softResets == 0)
        pmc_state = 1;
#elif defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856)
    /* Maestro based PMC) */
    if (PROCMON->maestroReg.coreCtrl.coreEnable == 1)
        pmc_state = 1;
#endif
        
    return pmc_state;
}
#endif

static int SendAndWait(TCommand *cmd, TCommand *rsp)
{
#if defined(BOOT_MEMC_SRAM)
	reqdID = 1;
#else
    static uint32 reqdID = 1;
#endif
	int status = kPMC_COMMAND_TIMEOUT;
	TCommand dummy;

#ifndef _CFE_
	CheckIntrContext();
	spin_lock(&lock);
#endif

#if defined CONFIG_BCM963138 || defined _BCM963381_
	if (pmc_remap && cmd->word0.Bits.cmdID < sizeof newToOldcmdIDMap &&
	    newToOldcmdIDMap[cmd->word0.Bits.cmdID])
		cmd->word0.Bits.cmdID = newToOldcmdIDMap[cmd->word0.Bits.cmdID];
#endif

	cmd->word0.Bits.msgID = reqdID;

	/* send the command */
	PMC->dqmQData[PMC_DQM_REQ_NUM].word[0] = cmd->word0.Reg32;
	PMC->dqmQData[PMC_DQM_REQ_NUM].word[1] = cmd->word1.Reg32;
	PMC->dqmQData[PMC_DQM_REQ_NUM].word[2] = cmd->u.cmdGenericParams.params[0];
	PMC->dqmQData[PMC_DQM_REQ_NUM].word[3] = cmd->u.cmdGenericParams.params[1];

	/* wait for no more than 5ms for the reply */
#ifdef CONFIG_BRCM_IKOS
	/* We do not enable PMC TIMER here for IKOS, or it will wait forever */
	while (!(PMC->dqm.notEmptySts & PMC_DQM_RPL_STS));
#else
	// XXX repeating timer never times out
	PMC->ctrl.gpTmr2Ctl = ((1 << 31) | (1 << 30) | (1 << 29) | 5000);

	while (!(PMC->dqm.notEmptySts & PMC_DQM_RPL_STS) &&
			(PMC->ctrl.gpTmr2Ctl & (1 << 31)));
#endif

	if (PMC->dqm.notEmptySts & PMC_DQM_RPL_STS) {
		if (!rsp)
			rsp = &dummy;

		/* command didn't timeout, fill in the response */
		rsp->word0.Reg32 = PMC->dqmQData[PMC_DQM_RPL_NUM].word[0];
		rsp->word1.Reg32 = PMC->dqmQData[PMC_DQM_RPL_NUM].word[1];
		rsp->u.cmdGenericParams.params[0] = PMC->dqmQData[PMC_DQM_RPL_NUM].word[2];
		rsp->u.cmdGenericParams.params[1] = PMC->dqmQData[PMC_DQM_RPL_NUM].word[3];


		if (rsp->word0.Bits.msgID == reqdID)
			status = rsp->word0.Bits.error;
		else
			status = kPMC_MESSAGE_ID_MISMATCH;
	}

	reqdID = (reqdID + 1) & 0xff;

#ifndef _CFE_
	spin_unlock(&lock);
#endif

	return status;
}

static int SendCmd(TCommand *cmd, int cmdID, int devAddr, int zone, int island, TCommand *rsp)
{
	cmd->word0.Reg32 = 0;
	cmd->word0.Bits.cmdID = cmdID;
	cmd->word1.Reg32 = 0;
	cmd->word1.Bits.devAddr = devAddr;
	cmd->word1.Bits.zoneIdx = zone;
	cmd->word1.Bits.island = island;

	return SendAndWait(cmd, rsp);
}

static int SendCommand(int cmdID, int devAddr, int zone, int island, uint32 word2,
		uint32 word3, TCommand *rsp)
{
	TCommand cmd;

	cmd.u.cmdGenericParams.params[0] = word2;
	cmd.u.cmdGenericParams.params[1] = word3;

	return SendCmd(&cmd, cmdID, devAddr, zone, island, rsp);
}

static int SendStateCommand(int cmdID, int island, int state)
{
	TCommand cmd;
	int status;

	if ((unsigned)island > 2)
		status = kPMC_INVALID_ISLAND;
	else if ((unsigned)state > 15)
		status = kPMC_INVALID_STATE;
	else {
		cmd.word0.Reg32 = 0;
		cmd.word0.Bits.cmdID = cmdID;
		cmd.word1.Reg32 = 0;
		cmd.word1.Bits.island = island;
		cmd.u.cmdStateOnlyParam.state = state;
		status = SendAndWait(&cmd, 0);
	}

	return status;
}

int GetRevision(uint32_t *change, uint32_t *revision)
{
	if (pmc_mode == PMC_MODE_DQM) {
		TCommand rsp;
		int status = SendCommand(cmdRevision, 0, 0, 0, 0, 0, &rsp);

		if (status == kPMC_NO_ERROR)
			*change   = rsp.u.cmdResponse.word2;
			*revision = rsp.u.cmdResponse.word3;

		return status;
	}
	else
		return kPMC_INVALID_COMMAND;
}

#ifndef _CFE_
int GetDevPresence(int devAddr, int *value)
{
	CHECK_DEV_ADDRESSABLE(devAddr);

	if (pmc_mode == PMC_MODE_DQM) {
		TCommand rsp;
		int status = SendCommand(cmdGetDevPresence, devAddr, 0, 0, 0, 0, &rsp);

		if (status == kPMC_NO_ERROR)
			*value = (rsp.u.cmdResponse.word2 & 1) ? TRUE : FALSE;
		else
			*value = FALSE;

		return status;
	} else
		return kPMC_INVALID_COMMAND;
}

int GetSWStrap(int devAddr, int *value)
{
	CHECK_DEV_ADDRESSABLE(devAddr);

	if (pmc_mode == PMC_MODE_DQM) {
		TCommand rsp;
		int status = SendCommand(cmdGetSWStrap, devAddr, 0, 0, 0, 0, &rsp);

		if (status == kPMC_NO_ERROR)
			*value = rsp.u.cmdResponse.word2 & 0xffff;

		return status;
	} else
		return kPMC_INVALID_COMMAND;
}

int GetHWRev(int devAddr, int *value)
{
	CHECK_DEV_ADDRESSABLE(devAddr);

	if (pmc_mode == PMC_MODE_DQM) {
		TCommand rsp;
		int status = SendCommand(cmdGetHWRev, devAddr, 0, 0, 0, 0, &rsp);

		if (status == kPMC_NO_ERROR)
			*value = rsp.u.cmdResponse.word2 & 0xff;

		return status;
	} else
		return kPMC_INVALID_COMMAND;
}

int GetNumZones(int devAddr, int *value)
{
	CHECK_DEV_ADDRESSABLE(devAddr);

	if (pmc_mode == PMC_MODE_DQM) {
		TCommand rsp;
		int status = SendCommand(cmdGetNumZones, devAddr, 0, 0, 0, 0, &rsp);

		if (status == kPMC_NO_ERROR)
			*value = rsp.u.cmdResponse.word2 & 0xff;

		return status;
	} else
		return kPMC_INVALID_COMMAND;
}

int GetAvsDisableState(int island, int *state)
{
	if (pmc_mode == PMC_MODE_DQM) {
		TCommand rsp;
		int status = SendCommand(cmdGetAvsDisableState, 0, 0, island, 0, 0, &rsp);

		if (status == kPMC_NO_ERROR)
			*state   = rsp.u.cmdResponse.word2;
		return status;
	}
	else
		return kPMC_INVALID_COMMAND;
}

#if defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856)
static int is_pvtmon_enabled = 0;
static void pvtmon_enable(void)
{
    uint32_t index;
    uint32_t target; 
#define PVTCLKDIV (5 << 8)

    // set up analog hardware to enable counting
    target = PVTCLKDIV | 4;  // 4 = clk_en|!pwr_dn|rstb
    WriteBPCMRegister(PMB_ADDR_PVTMON, 17, target);
    for (index = 0; index < 100000; index++); 
    target = PVTCLKDIV | 5; // 5 = clk_en|!pwr_dn|!rstb
    WriteBPCMRegister(PMB_ADDR_PVTMON, 17, target);

    // set sample size of ALL counters except TEST (7)
    // set enable bit for ONLY 0 (temperature and external) and V1p0<0> - these will be the only ones we use during Match, CalcSigma, and Closure
    for (index = 0; index <8; index++)
    {
        target = (0x5 << 24) | 0x80000000; //pvtmon samples (32 [2^5]) + enable
        WriteBPCMRegister(PMB_ADDR_PVTMON, 24 + index, target);
        ReadBPCMRegister(PMB_ADDR_PVTMON, 24 + index, &target);  // read once to clear valid bit
    }

    // enable accumulation:
    // 0x00000801 = skip_len = 8, enable accumulation
    target = 0x00000801;
    WriteBPCMRegister(PMB_ADDR_PVTMON, 20, target);
    is_pvtmon_enabled = 1;  
}

int ReadPVT(int index, int island, int *val)
{
    int status;
    uint32_t target;

    if (!is_pvtmon_enabled)
        pvtmon_enable();

    status = ReadBPCMRegister(PMB_ADDR_PVTMON, 24 + index, &target);

    while( !(target & (1 << 18) ))
    {
        // the value SHOULD be valid immediatly, but just in case...
        status = ReadBPCMRegister(PMB_ADDR_PVTMON, 24 + index, &target);
    }
    *val = target & 0x3ff;
    return kPMC_NO_ERROR;
}
#endif

int GetPVT(int sel, int island, int *value)
{
	if (pmc_mode == PMC_MODE_DQM) {
		TCommand rsp;
		int cmd = sel < 8 ? cmdGetPVT : cmdGetRMONandSigma;
		int status = SendCommand(cmd, 0, 0, island, sel, 0, &rsp);

		if (status == kPMC_NO_ERROR)
			*value = rsp.u.cmdResponse.word2;
		return status;
	}
	else
#if defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856)
		return ReadPVT(sel, island, value);
#else
		return kPMC_INVALID_COMMAND;
#endif
}

#if defined(CONFIG_BCM96856)
int GetAllROs(void *shmem)
{
	if (pmc_mode == PMC_MODE_DQM)
    {
        TCommand rsp;

        if (shmem == NULL)
            return kPMC_INVALID_PARAM;

        return SendCommand(cmdReadROs, 0, 0, 0, shmem, 0, &rsp);
    }
    else
        return kPMC_INVALID_COMMAND;
} 
#endif

int Ping(void)
{
#if defined(_BCM96836_) || defined(CONFIG_BCM96836) || defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856)
    return kPMC_INVALID_COMMAND;
#endif
	if (pmc_mode == PMC_MODE_DQM)
		return SendCommand(cmdPing, 0, 0, 0, 0, 0, 0);
	else
		return kPMC_INVALID_COMMAND;
}

int GetErrorLogEntry(TErrorLogEntry *logEntry)
{
	if (pmc_mode == PMC_MODE_DQM)
		return SendCommand(cmdGetNextLogEntry, 0, 0, 0, 0, 0, (TCommand *)logEntry);
	else
		return kPMC_INVALID_COMMAND;
}

int SetClockHighGear(int devAddr, int zone, int clkN)
{
	CHECK_DEV_ADDRESSABLE(devAddr);

	if (clkN < 0 || clkN > 7)
		return kPMC_INVALID_PARAM;

	if (pmc_mode == PMC_MODE_DQM) {
		TCommand cmd;
		cmd.u.cmdGenericParams.params[0] = 0;
		cmd.u.cmdGenericParams.params[1] = 0;
		cmd.u.cmdSetClockGear.gear = clkN & 0xff;
		return SendCmd(&cmd, cmdSetClockHighGear, devAddr, zone, 0, 0);
	}
	else
		return kPMC_INVALID_COMMAND;
}

int SetClockLowGear(int devAddr, int zone, int clkN)
{
	CHECK_DEV_ADDRESSABLE(devAddr);

	if (clkN < 0 || clkN > 7)
		return kPMC_INVALID_PARAM;

	if (pmc_mode == PMC_MODE_DQM) {
		TCommand cmd;
		cmd.u.cmdGenericParams.params[0] = 0;
		cmd.u.cmdGenericParams.params[1] = 0;
		cmd.u.cmdSetClockGear.gear = clkN & 0xff;
		return SendCmd(&cmd, cmdSetClockLowGear, devAddr, zone, 0, 0);
	}
	else
		return kPMC_INVALID_COMMAND;

}

int SetClockGear(int devAddr, int zone, int gear)
{
	CHECK_DEV_ADDRESSABLE(devAddr);

	if ((unsigned)gear > 3)
		return kPMC_INVALID_PARAM;

	if (pmc_mode == PMC_MODE_DQM) {
		TCommand cmd;
		cmd.u.cmdGenericParams.params[0] = 0;
		cmd.u.cmdGenericParams.params[1] = 0;
		cmd.u.cmdSetClockGear.gear = gear & 0xff;
		return SendCmd(&cmd, cmdSetClockGear, devAddr, zone, 0, 0);
	}
	else
		return kPMC_INVALID_COMMAND;
}
#endif

/* GetRCalSetting reads resistor value and calculates the calibration setting for the SGMII, PCIe, SATA
   and USB HW blocks that requires resistor calibration to meet specification requirement.
   The HW driver should call this function and write the value to calibration register during initialzation

   input param: resistor - the resistor type that specific HW calibration care about:
   inout param: rcal - 4 bit RCAL value [0 -15] representing the increment or decrement to the internal resistor.   
   return: kPMC_NO_ERROR or kPMC_INVALID_COMMAND if error condition
*/
#if !defined(_BCM96836_) && !defined(CONFIG_BCM96836) && !defined(_BCM96846_) && !defined(CONFIG_BCM96846) && !defined(_BCM96856_) && !defined(CONFIG_BCM96856) 
int GetRCalSetting(int resistor, int* rcal)
{
	int res_int, res_ext, ratio, ratio1;
	int rc = kPMC_NO_ERROR;

	if (pmc_mode == PMC_MODE_DQM) 
	{
		/* make sure the resistor selection is valid */
		if( resistor < RCAL_0P25UM_HORZ || resistor > RCAL_1UM_VERT )
			return kPMC_INVALID_COMMAND;

		/* make sure the resistor data is collected by PMC */
		if( (PROCMON->Misc.misc[PMMISC_RMON_EXT_REG]&PMMISC_RMON_VALID_MASK) == 0 )
			return kPMC_INVALID_COMMAND;

		res_int = PROCMON->Misc.misc[resistor>>1];
		if( resistor % 2 )
			res_int >>= 16;
		res_int &= 0xffff;
		res_ext = (PROCMON->Misc.misc[PMMISC_RMON_EXT_REG])&0xffff;
		/* Return error if the res_ext saturated such as the ext resistor is not available */
		if (res_ext > 0x3a0)
		{
			printk("%s:res_ext value 0x%x is saturated!\n", __func__, res_ext);
			return kPMC_INVALID_STATE;
		}

		/* Ratio = CLAMP((INT) (128.0 * V(REXT)/V(RINT)), 0, 255) */
		ratio = (128*res_ext)/res_int;
		if( ratio > 255 )
			ratio = 255;

		/* Ratio1 = CLAMP(128 - (Ratio - 128) * 4, 0, 255) */
		ratio1 = (128 - (ratio - 128 )*4);
		if( ratio1 < 0 )
			ratio1 = 0;
		if( ratio1 > 255 )
			ratio1 = 255;

		/* convert to 4 bit rcal setting value */
		*rcal = (ratio1>>4)&0xf;
#if 0
		printk("getrcal for res select %d, int %d, ext %d, ratio %d ratio1 %d, rcal %d\n", 
			resistor, res_int, res_ext, ratio, ratio1, *rcal);
#endif
	}
	else  
	{
		/* not supported if PMC is not running for now. To support that, need to copy the PMC rom 
		   code to read out resistor value manually */
		rc = kPMC_INVALID_COMMAND;
	}

	return rc;
}
#endif

/* note: all the [Read|Write][BPCM|Zone]Register functions are different from
 * how they are defined in firmware code.  In the driver code, it takes in
 * wordOffset as the argument, but in the firmware code, it uses byteOffset */
int ReadBPCMRegister(int devAddr, int wordOffset, uint32 *value)
{
	int status = kPMC_NO_ERROR;

	CHECK_DEV_ADDRESSABLE(devAddr);

	if (pmc_mode == PMC_MODE_DQM) {
		TCommand rsp;
		status = SendCommand(cmdReadBpcmReg, devAddr, 0, 0, wordOffset, 0, &rsp);

		if (status == kPMC_NO_ERROR)
			*value = rsp.u.cmdResponse.word2;
	}
	else {	/* if (pmc_mode == PMC_MODE_PMB_DIRECT) */
#ifndef _CFE_
		CheckIntrContext();
		spin_lock(&lock);
#endif
		status = read_bpcm_reg_direct(devAddr, wordOffset, value);

#ifndef _CFE_
		spin_unlock(&lock);
#endif
	}

	return status;
}

int ReadZoneRegister(int devAddr, int zone, int wordOffset, uint32 *value)
{
	int status = kPMC_NO_ERROR;

	CHECK_DEV_ADDRESSABLE(devAddr);

	if ((unsigned)wordOffset >= 4)
		return kPMC_INVALID_PARAM;

	if (pmc_mode == PMC_MODE_DQM) {
		TCommand rsp;

		status = SendCommand(cmdReadZoneReg, devAddr, zone, 0, wordOffset,
				0, &rsp);
		if (status == kPMC_NO_ERROR)
			*value = rsp.u.cmdResponse.word2;
	}
	else	/* if (pmc_mode == PMC_MODE_PMB_DIRECT) */
		return ReadBPCMRegister(devAddr,
				BPCMRegOffset(zones[zone].control) + wordOffset,
				value);

	return status;
}

int WriteBPCMRegister(int devAddr, int wordOffset, uint32 value)
{
	int status = kPMC_NO_ERROR;

	CHECK_DEV_ADDRESSABLE(devAddr);

	if (pmc_mode == PMC_MODE_DQM)
		status = SendCommand(cmdWriteBpcmReg, devAddr, 0, 0, wordOffset, value, 0);
	else {  /* if (pmc_mode == PMC_MODE_PMB_DIRECT) */
#ifndef _CFE_
		CheckIntrContext();
		spin_lock(&lock);
#endif
		status = write_bpcm_reg_direct(devAddr, wordOffset, value);
#ifndef _CFE_
		spin_unlock(&lock);
#endif	
	}

	return status;
}

int WriteZoneRegister(int devAddr, int zone, int wordOffset, uint32 value)
{
	CHECK_DEV_ADDRESSABLE(devAddr);

	if ((unsigned)wordOffset >= 4)
		return kPMC_INVALID_PARAM;

	if (pmc_mode == PMC_MODE_DQM)
		return SendCommand(cmdWriteZoneReg, devAddr, zone, 0, wordOffset, value, 0);
	else	/* if (pmc_mode == PMC_MODE_PMB_DIRECT) */
		return WriteBPCMRegister(devAddr,
				BPCMRegOffset(zones[zone].control) + wordOffset,
				value);
}

int SetRunState(int island, int state)
{
	if (pmc_mode == PMC_MODE_DQM) {
		if ((unsigned)state >= 16)
			return kPMC_INVALID_PARAM;

		return SendStateCommand(cmdSetRunState, island, state);
	} else
		return kPMC_INVALID_COMMAND;
}

int SetPowerState(int island, int state)
{
	if (pmc_mode == PMC_MODE_DQM) {
		if ((unsigned)state >= 4)
			return kPMC_INVALID_PARAM;

		return SendStateCommand(cmdSetPowerState, island, state);
	} else
		return kPMC_INVALID_COMMAND;
}

int GetSelect0(void)
{
	TCommand rsp;
	int status;

	status = SendCommand(cmdGetSelect0, 0, 0, 0, 0, 0, &rsp);
	if (status == kPMC_NO_ERROR)
		return rsp.u.cmdResponse.word2;
	else
		return -1000000;
}

int GetSelect3(void)
{
	TCommand rsp;
	int status;

	status = SendCommand(cmdGetSelect3, 0, 0, 0, 0, 0, &rsp);
	if (status == kPMC_NO_ERROR)
		return rsp.u.cmdResponse.word2;
	else
		return -1000000;
}

int PowerOnDevice(int devAddr)
{
	CHECK_DEV_ADDRESSABLE(devAddr);

	if (pmc_mode == PMC_MODE_DQM) {
		TCommand cmd;
		cmd.u.cmdGenericParams.params[0] = 0;
		cmd.u.cmdGenericParams.params[1] = 0;
		cmd.u.cmdPowerDevice.state = 1;
#if defined _BCM963381_
		/* Only needed for 63381A0 boards from cfe where */
		/* this command has to be remapped to the old number */
		/* and the old byte-swapping has to be kludged */
		/* This will work for both A0 and B0 */
		cmd.u.cmdPowerDevice.reserved[1] = 1;
#endif
		return SendCmd(&cmd, cmdPowerDevOnOff, devAddr, 0, 0, 0);
	}
	else {	/* if (pmc_mode == PMC_MODE_PMB_DIRECT) */
		int ix, status;
		BPCM_CAPABILITES_REG capabilities;

		status = ReadBPCMRegister(devAddr, BPCMRegOffset(capabilities), &capabilities.Reg32);
		for (ix = 0; (ix < capabilities.Bits.num_zones) && (status == kPMC_NO_ERROR); ix++) {
			status = PowerOnZone(devAddr, ix);
		}

		return status;
	}
}

int PowerOffDevice(int devAddr, int repower)
{
	CHECK_DEV_ADDRESSABLE(devAddr);

	if (pmc_mode == PMC_MODE_DQM) {
		TCommand cmd;
		cmd.u.cmdGenericParams.params[0] = 0;
		cmd.u.cmdGenericParams.params[1] = 0;
		cmd.u.cmdPowerDevice.state = 0;
		cmd.u.cmdPowerDevice.restore = repower;
		return SendCmd(&cmd, cmdPowerDevOnOff, devAddr, 0, 0, 0);
	}
	else
	{	/* if (pmc_mode == PMC_MODE_PMB_DIRECT) */
		/* we can power off the entire device by powering off the 0th zone. */
		BPCM_PWR_ZONE_N_CONTROL reg;
		int status;

		status = ReadBPCMRegister(devAddr, BPCMRegOffset(zones[0].control), &reg.Reg32);

		if (status == kPMC_NO_ERROR && reg.Bits.pwr_off_state == 0) {
			reg.Bits.pwr_dn_req = 1;
			WriteBPCMRegister(devAddr, BPCMRegOffset(zones[0].control), reg.Reg32);
		}

		return status;
	}
}

int PowerOnZone(int devAddr, int zone)
{
	CHECK_DEV_ADDRESSABLE(devAddr);

#if defined(_BCM96848_) || defined(CONFIG_BCM96848) || defined(_BCM96858_) || defined(CONFIG_BCM96858) || defined(_BCM96836_) || defined(CONFIG_BCM96836)
	/* Do not use DQM command cmdPowerZoneOnOff for non 6848 because this command is only available if a
	   PMC application has been uploaded to expand the PMC boot rom functionality */
	if (pmc_mode == PMC_MODE_DQM) {
		TCommand cmd;
		cmd.u.cmdGenericParams.params[0] = 0;
		cmd.u.cmdGenericParams.params[1] = 0;
		cmd.u.cmdStateOnlyParam.state = 1;
		return SendCmd(&cmd, cmdPowerZoneOnOff, devAddr, zone, 0, 0);
	}
	else
#endif
	{	/* if (pmc_mode == PMC_MODE_PMB_DIRECT) */
		BPCM_PWR_ZONE_N_CONTROL reg;
		int status;

#if defined(_BCM96836_) || defined(CONFIG_BCM96836)
        if (gIsClassic[devAddr&0xff])
            status = ReadBPCMRegister(devAddr, ClassicBPCMRegOffset(zones[zone].control), &reg.Reg32);
        else
#endif
		status = ReadBPCMRegister(devAddr, BPCMRegOffset(zones[zone].control), &reg.Reg32);

		if (status == kPMC_NO_ERROR && reg.Bits.pwr_on_state == 0) {
			reg.Bits.pwr_dn_req = 0;
			reg.Bits.dpg_ctl_en = 1;
			reg.Bits.pwr_up_req = 1;
			reg.Bits.mem_pwr_ctl_en = 1;
			reg.Bits.blk_reset_assert = 1;
#if defined(_BCM96836_) || defined(CONFIG_BCM96836)
            if (gIsClassic[devAddr&0xff])
                status = WriteBPCMRegister(devAddr, ClassicBPCMRegOffset(zones[zone].control), reg.Reg32);
            else
#endif
			status = WriteBPCMRegister(devAddr, BPCMRegOffset(zones[zone].control), reg.Reg32);
		}
		return status;
	}
}

int PowerOffZone(int devAddr, int zone)
{
	CHECK_DEV_ADDRESSABLE(devAddr);

#if defined(_BCM96848_) || defined(CONFIG_BCM96848) || defined(_BCM96858_) || defined(CONFIG_BCM96858) || defined(_BCM96836_) || defined(CONFIG_BCM96836) || defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856)
	/* Do not use DQM command cmdPowerZoneOnOff for non 6848 because this command is only available if a
	   PMC application has been uploaded to expand the PMC boot rom functionality */
	if (pmc_mode == PMC_MODE_DQM) {
		TCommand cmd;
		cmd.u.cmdGenericParams.params[0] = 0;
		cmd.u.cmdGenericParams.params[1] = 0;
		cmd.u.cmdStateOnlyParam.state = 0;
		return SendCmd(&cmd, cmdPowerZoneOnOff, devAddr, zone, 0, 0);
	}
	else
#endif
	{	/* if (pmc_mode == PMC_MODE_PMB_DIRECT) */
	 	BPCM_PWR_ZONE_N_CONTROL reg;
		int status;

#if defined(_BCM96836_) || defined(CONFIG_BCM96836)
        if (gIsClassic[devAddr&0xff])
            status = ReadBPCMRegister(devAddr, ClassicBPCMRegOffset(zones[zone].control), &reg.Reg32);
        else
#endif
		status = ReadBPCMRegister(devAddr, BPCMRegOffset(zones[zone].control), &reg.Reg32);

		if (status == kPMC_NO_ERROR) {
			reg.Bits.pwr_dn_req = 1;
			reg.Bits.pwr_up_req = 0;
#if defined(_BCM96836_) || defined(CONFIG_BCM96836)
            if (gIsClassic[devAddr&0xff])
                status = WriteBPCMRegister(devAddr, ClassicBPCMRegOffset(zones[zone].control), reg.Reg32);
            else
#endif
			status = WriteBPCMRegister(devAddr, BPCMRegOffset(zones[zone].control), reg.Reg32);
		}

		return status;
	}
}

int ResetDevice(int devAddr)
{
	CHECK_DEV_ADDRESSABLE(devAddr);

#if defined(_BCM96848_) || defined(CONFIG_BCM96848) || defined(_BCM96858_) || defined(CONFIG_BCM96858) || defined(_BCM96836_) || defined(CONFIG_BCM96836) || defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856)
	/* Do not use DQM command cmdResetDevice for non 6848 because this command is only available if a
	   PMC application has been uploaded to expand the PMC boot rom functionality */
	if (pmc_mode == PMC_MODE_DQM)
		return SendCommand(cmdResetDevice, devAddr, 0, 0, 0, 0, 0);
	else
#endif
	{	/* if (pmc_mode == PMC_MODE_PMB_DIRECT) */
		/* all zones had their blk_reset_assert bits set at initial config time */
		BPCM_PWR_ZONE_N_CONTROL reg;
		int status;

		status = PowerOffDevice(devAddr, 0);

		do {
			status = ReadBPCMRegister(devAddr, BPCMRegOffset(zones[0].control), &reg.Reg32);
		} while ((reg.Bits.pwr_off_state != 1) && (status == kPMC_NO_ERROR));

		if (status == kPMC_NO_ERROR)
			status = PowerOnDevice(devAddr);

		return status;
	}
}

int ResetZone(int devAddr, int zone)
{
	CHECK_DEV_ADDRESSABLE(devAddr);

#if defined(_BCM96848_) || defined(CONFIG_BCM96848) || defined(_BCM96858_) || defined(CONFIG_BCM96858) || defined(_BCM96836_) || defined(CONFIG_BCM96836) || defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856)
	/* Do not use DQM command cmdResetZone for non 6848 because this command is only available if a
	   PMC application has been uploaded to expand the PMC boot rom functionality */
	if (pmc_mode == PMC_MODE_DQM)
		return SendCommand(cmdResetZone, devAddr, zone, 0, 0, 0, 0);
	else
#endif
	{	/* if (pmc_mode == PMC_MODE_PMB_DIRECT) */
		BPCM_PWR_ZONE_N_CONTROL reg;
		int status;

		status = PowerOffZone(devAddr, zone);

		do {
			status = ReadBPCMRegister(devAddr, BPCMRegOffset(zones[zone].control), &reg.Reg32);
		} while ((reg.Bits.pwr_off_state != 1) && (status == kPMC_NO_ERROR));

		if (status == kPMC_NO_ERROR)
			status = PowerOnZone(devAddr, zone);

		return status;
	}
}

/* close AVS with margin(mV) */
int CloseAVS(int island, int margin1, int margin2)
{
	if (pmc_mode == PMC_MODE_DQM) {
		TCommand rsp;
		int status = SendCommand(cmdCloseAVS, 0, 0, island, margin1, margin2, &rsp);
		return status;
	}
	else
		return kPMC_INVALID_COMMAND;
}

#if defined _CFE_
int PMCcmd(int arg[4]);
int PMCcmd(int arg[4])
{
        TCommand *cmd = (TCommand *) arg;

        return SendAndWait(cmd, cmd);
}
#endif

void WaitPmc(int runState)
{
    if (pmc_mode == PMC_MODE_DQM) {
        do{
#ifdef _CFE_
#if !defined(_BCM96846_) && !defined(_BCM96856_)
            /* Check if PMC is failing */
            if(((PMC->ctrl.hostMboxIn >> 24) & 7) == kPMCRunStateStalled){
                printk("**WARN**: PMC failed to boot\n");
                printk("          Press any key to continue to CFE\n");
                while (!console_status());
                /* PMC is now in direct mode */
                pmc_mode = PMC_MODE_PMB_DIRECT;
                /* Leave PMC in reset state */
                PMC->ctrl.softResets = 0x1;
                return;
            }
#else /*defined(_BCM96846_) || defined(_BCM96856_)*/
#define PMC_CHIP_NOT_VALID (0x7<<5)
            if ((PMC->ctrl.hostMboxIn & PMC_CHIP_NOT_VALID) == PMC_CHIP_NOT_VALID)
            {
                printk("**ERR**: PMC firmware is not compatible to this chip\n");
                printk("          Press any key to continue to CFE\n");
                while (!console_status());
                /* PMC is now in direct mode */
                pmc_mode = PMC_MODE_PMB_DIRECT;

                PROCMON->maestroReg.coreCtrl.coreEnable = 0;
                return;
            }
#endif
#endif /* _CFE_ */
#if defined(_BCM96836_) || defined(CONFIG_BCM96836) || defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856)
            }while (((PMC->ctrl.hostMboxIn) & 7) != runState);
#else
        }while (((PMC->ctrl.hostMboxIn >> 24) & 7) != runState);
#endif
    }
    else {	/* if (pmc_mode == PMC_MODE_PMB_DIRECT) */
        /* do nothing */
    }
}

void BootPmcNoRom(unsigned long physAddr)
{
	PMC->ctrl.addr1WndwMask = 0xffffc000;
	PMC->ctrl.addr1WndwBaseIn = 0x1fc00000;
	PMC->ctrl.addr1WndwBaseOut = physAddr;

	PMC->ctrl.softResets &= ~1;
}

#if defined _BCM963138_ || defined CONFIG_BCM963138 || \
    defined _BCM963148_ || defined CONFIG_BCM963148 || \
    defined _BCM96858_  || defined CONFIG_BCM96858  || \
    defined _BCM94908_  || defined CONFIG_BCM94908  || \
    defined _BCM96836_  || defined CONFIG_BCM96836 

/* new pmc firmware implements stall command */
/* state indicated by stalled bit in run status */

#define PMC_STALLED (1 << 30)

/* return value doesn't appear to be used */
int StallPmc(void)
{
	TCommand rsp;

	/* ignore if pmc not booted from flash or already stalled */
	if (pmc_mode == PMC_MODE_PMB_DIRECT ||
#if defined MISC_STRAP_BUS_PMC_BOOT_FLASH
	(MISC->miscStrapBus & MISC_STRAP_BUS_PMC_BOOT_FLASH) == 0 ||
#else
	(MISC->miscStrapBus & MISC_STRAP_BUS_PMC_BOOT_FLASH_N) != 0 ||
#endif
	PMC->ctrl.hostMboxIn & PMC_STALLED)
		return 0;

	/* return non-zero if stall command fails */
	return SendCommand(cmdStall, 0, 0, 0, 0, 0, &rsp);
}

/* return value doesn't appear to be used */
int UnstallPmc(void)
{
	/* clear stalled bit if pmc booted from flash */
	if ((pmc_mode != PMC_MODE_PMB_DIRECT) &&
#if defined MISC_STRAP_BUS_PMC_BOOT_FLASH
	((MISC->miscStrapBus & MISC_STRAP_BUS_PMC_BOOT_FLASH) != 0))
#else
	((MISC->miscStrapBus & MISC_STRAP_BUS_PMC_BOOT_FLASH_N) == 0))
#endif
		PMC->ctrl.hostMboxIn &= ~PMC_STALLED;

	return 0;
}
#endif

// initalize pmc_mode (possibly) before printk available
void pmc_initmode(void)
{
#if defined MISC_STRAP_BUS_PMC_ROM_BOOT
#if defined(_BCM96858_) || defined(CONFIG_BCM96858)
	unsigned int pmc_boot_row;
	int ret;

	/* read the strap pin and based on the strap pin, choose the mode */
	ret = bcm_otp_get_pmc_boot_sts(&pmc_boot_row);
	if ((MISC->miscStrapBus & MISC_STRAP_BUS_PMC_ROM_BOOT) == 0 &&
		(ret || (pmc_boot_row & OTP_PMC_BOOT_MASK) == 0))
		pmc_mode = PMC_MODE_PMB_DIRECT;

    ret =  PowerOnDevice(PMB_ADDR_RDPPLL);

    ret =  pll_ch_freq_set(PMB_ADDR_RDPPLL, 0, 2);

    if ( (PERF->RevID & REV_ID_MASK) != 0xa0 )
    {
        ret =  pll_ch_freq_set(PMB_ADDR_RDPPLL, 1, 1);
    }

#else
	if ((MISC->miscStrapBus & MISC_STRAP_BUS_PMC_ROM_BOOT) == 0)
		pmc_mode = PMC_MODE_PMB_DIRECT;
#endif
#else
	// Linux checks if PMC is booted.
	if (is_pmc_running())
		pmc_mode = PMC_MODE_DQM;
	else
		pmc_mode = PMC_MODE_PMB_DIRECT;
#endif

#if defined(_BCM96836_) || defined(CONFIG_BCM96836)
	gIsClassic[7]  = 1;
	gIsClassic[32] = 1;
	gIsClassic[33] = 1;
	gIsClassic[36] = 1;
	gIsClassic[38] = 1;
	gIsClassic[39] = 1;
#endif
}

#if defined CONFIG_BCM94908
int RecloseAVS(int iscold)
{
	// check sec_chipvar and send command
	uint32 cap = ((uint32 *)JTAG_OTP_BASE)[12];
	static int last_slow, last_fast;
	int slow, fast; // margins

	if ((cap & 15) != 0) {
		slow = 80, fast = 55;
		if (iscold)
			slow += 10, fast += 10;
	} else
		slow = 100, fast = 100;
	if (slow == last_slow && fast == last_fast)
		return 0;
	return CloseAVS(0, last_slow = slow, last_fast = fast);

}
#endif

#if defined (PMC_RAM_BOOT)

/* Include the platform specific PMC boot code */
#if defined (_BCM96836_)
#include "pmc_romdata_6836.h"
#elif defined (_BCM963158_)
#include "pmc_romdata_63158.h"
#elif defined (_BCM96846_)
#include "pmc_firmware_68460.h"
#include "pmc_firmware_68460_A1.h"
#elif defined (_BCM96856_)
#include "pmc_firmware_68560.h"
#endif

TPMCBootParams *pmcBootParams = NULL;

void set_pmc_boot_param(unsigned int boot_option, unsigned int boot_param)
{
	void *bootParamEnd = (void*)pmcBootParams + sizeof(TPMCBootParams) + (pmcBootParams->pmc_boot_option_cnt + 1) * sizeof(TPMCBootOption);
	TPMCBootOption *bootOptions = NULL;

	// check boot parameters do not corss the boundary
	if ((unsigned long)bootParamEnd > pmcBootParams->pmc_image_addr + pmcBootParams->pmc_image_max_size) {
		printk ("Error: %s : Space not avaiable for PMC boot parameter\n");
		return;
	}

	// Locate boot option area
	bootOptions = (TPMCBootOption *)LOCATE_BOOT_OPTIONS(pmcBootParams);
	// set boot options
	switch (boot_option) {
	case kPMCBootDefault:
		pmcBootParams->pmc_boot_option_cnt = 0;
		break;
	case kPMCBootAVSDisable:
	case kPMCBootAVSTrackDisable:
	case kPMCBootLogBuffer:
	case kPMCBootLogSize:
		bootOptions[pmcBootParams->pmc_boot_option_cnt].option = boot_option;
		bootOptions[pmcBootParams->pmc_boot_option_cnt].opt_param = boot_param;
		pmcBootParams->pmc_boot_option_cnt++;
		break;
	default:
		printk ("Error: %s : Invalid PMC boot option\n", __func__);
	}
}

int get_pmc_boot_param(unsigned int boot_option, unsigned int *boot_param)
{
	TPMCBootOption *bootOptions;
	int i;
	// Locate boot option area
	bootOptions = (TPMCBootOption *)LOCATE_BOOT_OPTIONS(pmcBootParams);
	
	for (i = 0; pmcBootParams && i < pmcBootParams->pmc_boot_option_cnt; i++ ){
		if(bootOptions[i].option == boot_option){
			*boot_param = bootOptions[i].opt_param;
			return 0;
		}
	}
	return -1;
}
void init_pmc_boot_param(long image_addr, int image_size,  int max_size)
{
	// loacate boot parameter location at the end of image.
	pmcBootParams = (TPMCBootParams*)(image_addr + image_size - sizeof(TPMCBootParams));
	// Set boot param related global variables
	pmcBootParams->pmc_image_addr = image_addr;
	pmcBootParams->pmc_image_size = image_size;
	pmcBootParams->pmc_image_max_size = max_size;
	// Set default option
	set_pmc_boot_param (kPMCBootDefault, 0);
	printk("Boot pmc_image_addr 0x%08X\n", pmcBootParams->pmc_image_addr);
	printk("Boot pmc_image_size 0x%08X\n", pmcBootParams->pmc_image_size);
	printk("Boot pmc_image_max_size 0x%08X\n", pmcBootParams->pmc_image_max_size);
}

void pmc_log(int log_type)
{
#if defined(PMC_SHARED_MEMORY)
	static char cache_buffer[MAX_PMC_LOG_SIZE];
	unsigned int log_location = 0;
	unsigned int log_size = 0;

	/* Do log, only if log buffer and log buffer size is known */
	if ( (get_pmc_boot_param(kPMCBootLogBuffer, &log_location) != -1) &&
		(get_pmc_boot_param(kPMCBootLogSize, &log_size) != -1) ){
		TPMCBootLog *log_header = (TPMCBootLog*)PMC_SHARED_MEMORY;
		char *log_buffer = (char*)cache_to_uncache(log_location);
		char *format_str = NULL;
		unsigned int value[10];

		/* Let PMC know, CFE wants to read from the begining */
		log_header->cfe_rd_idx = 0;
		log_header->pmc_log_type = log_type;

		/* Keep pumping out the log */
		while (1) {
			int i = 0, fmt_cnt = 0, avl_to_process = 0;
			int pmc_wr, cfe_rd;

			/* Wait until log is available in the buffer */
			do {
				pmc_wr = PMC->ctrl.scratch;
				cfe_rd = log_header->cfe_rd_idx;
				/* Exit if key is pressed */
				if (console_status()){
					/* Let PMC know that CFE is not interested in reading log */
					log_header->cfe_rd_idx = -1;
					return;
				}
			}while(pmc_wr == cfe_rd);

			avl_to_process = (pmc_wr + log_size - cfe_rd) % log_size;

			/* Read from the circular log buffer into the cache buffer */
			if(cfe_rd > pmc_wr){
				/* step 1: Copy from cfe_rd to the end of log_buffer */
				memcpy((void*)cache_buffer, (void*)&log_buffer[cfe_rd], log_size - cfe_rd);
				/* step 2: Copy from the begining of the log_buffer to pmc_wr */
				memcpy((void*)(cache_buffer + log_size - cfe_rd), (void*)&log_buffer[0], pmc_wr);
			}
			else
				/* Copy everything between cfe_rd and pmc_wr */
				memcpy((void*)cache_buffer, (void*)&log_buffer[cfe_rd], pmc_wr - cfe_rd);

			/* Read is done. Let PMC know how far has been read */
			log_header->cfe_rd_idx = (log_header->cfe_rd_idx + avl_to_process) % log_size;

			/* Process all inside the cache_buffer that has been read */
			i = 0;
			while (i < avl_to_process){
				format_str = &cache_buffer[i];
				/* Scan for format count inside the NULL terminated format string */
				fmt_cnt = 0;
				while( cache_buffer[i] ){
					if(cache_buffer[i] == '%')
						fmt_cnt++;
					i++;
				}
				/* collect the values after the format string, if there is any */
				i++;
				memcpy((void*)value, (void*)&cache_buffer[i], sizeof(unsigned int) * fmt_cnt);
				printf (format_str, value[0], value[1], value[2], value[3], value[4], value[5], value[6], value[7], value[8], value[9]);
				/* Look for next format string, inside the cache_buffer */
				i += fmt_cnt * sizeof(unsigned int);
			}
		}
	}
#endif /* PMC_SHARED_MEMORY */
}

static void pmc_boot(void)
{
#if !defined (_BCM96846_) && !defined(_BCM96856_)
    volatile unsigned int dummy;
#endif
    unsigned long physAddrPmc;
    unsigned int len;

    // copy image to aligned address in pmc boot area

#if defined (_BCM96846_) || defined (_BCM96856_)
    physAddrPmc = CFG_BOOT_PMC3_ADDR; // if CFG_BOOT_PMC_ADDR is not aligned, alignmnet should be added to physAddrPmc
#else
    physAddrPmc = CFG_BOOT_PMC_ADDR; // if CFG_BOOT_PMC_ADDR is not aligned, alignmnet should be added to physAddrPmc
#endif

    len = sizeof(pmcappdata);

#if defined (_BCM96846_)
    if (UtilGetChipRev() == 0xA1)
    {
        len = sizeof(pmcappdata_a1);
        memcpy((void*)physAddrPmc, pmcappdata_a1, len);
    }
    else
#endif
        memcpy((void*)physAddrPmc, pmcappdata, len);

#if defined (_BCM963158_)
    pmc_reset();

    init_pmc_boot_param(physAddrPmc, len, MAX_PMC_ROM_SIZE);
    /* Setup PMC log buffer */
    set_pmc_boot_param(kPMCBootLogBuffer, CFG_BOOT_PMC_ADDR + MAX_PMC_ROM_SIZE);
    set_pmc_boot_param(kPMCBootLogSize, MAX_PMC_LOG_SIZE);

    if (getAVSConfig() != 0) {
       printk ("Info: %s PMC opted to disable AVS\n", __func__);
       set_pmc_boot_param(kPMCBootAVSDisable, 1);
    }
#endif
    /* Boot code and boot params are loaded. Time to flush the content */
    _cfe_flushcache(CFE_CACHE_FLUSH_D, 0, 0);

#if defined (_BCM96846_) || defined (_BCM96856_)
    printf("Take PMC out of reset\n");
    PROCMON->maestroReg.coreCtrl.resetVector = physAddrPmc;
    PROCMON->maestroReg.coreCtrl.coreEnable = 1;
#endif

#if !defined (_BCM96846_) && !defined(_BCM96856_)
    PMC->ctrl.addr2WndwMask = 0;
    PMC->ctrl.addr2WndwBaseIn = 0;
    PMC->ctrl.addr2WndwBaseOut = 0;

    /* open window for the  PMC to see DDR */
    PMC->ctrl.addr1WndwMask = ~(CFG_BOOT_PMC_SIZE-1);
    PMC->ctrl.addr1WndwBaseIn = 0x1fc00000;
    PMC->ctrl.addr1WndwBaseOut = physAddrPmc;

    dummy = PMC->ctrl.addr1WndwBaseOut;   // dummy, just for sync
    dummy = dummy;
    
    printf("Take PMC out of reset\n");
    PMC->ctrl.softResets = 0x0;
#endif

    pmc_mode = PMC_MODE_DQM;

    printf("waiting for PMC finish booting\n");
    WaitPmc(PMC_IN_MAIN_LOOP);
#if defined (_BCM96846_) || defined (_BCM96856_)
    {
        uint32_t change;
        uint32_t revision;
        if (!GetRevision(&change, &revision))
        {
          printf("PMC rev: %d.%d.%d.%d running\n", (revision>>28) & 0xf, (revision >> 20) & 0xff, (revision & 0xfffff), change);
        }
    
    }
#endif
}
#endif //defined(PMC_RAM_BOOT)

# if defined CONFIG_BCM963138 && !defined _CFE_
static void pmc_patch_63138(void)
{
# define logsize (8 << 10)
	static const __attribute__ (( aligned(logsize) ))
# include "restart.h"
	unsigned linkaddr = 0x9fc10000; // translated code start
	const void *physaddr = restart_bin;
	TCommand rsp;

	if (((PMC->ctrl.hostMboxIn >> 24) & 7) != kPMCRunStateAVSCompleteWaitingForImage)
		return;

	/* window 2 baseout specifies destination of pmcapp image */
	/* e4k will translate the block at BaseIn to BaseOut */
	/* pmc ram command at linkaddr */
	PMC->ctrl.addr2WndwMask = -(logsize);
	PMC->ctrl.addr2WndwBaseIn = linkaddr & 0x1fffffff;
	PMC->ctrl.addr2WndwBaseOut = virt_to_phys(physaddr);

	// register command
	SendCommand(cmdRegisterCmdHandler, 0, 0, 0, 96, linkaddr, &rsp);
	if (rsp.word0.Bits.error)
		printk("%s %d\n", "cmdRegisterCmdHandler", rsp.word0.Bits.error);
	else {
		// send command with option to enable AVS
		SendCommand(96, 1, 0, 0, 75, 75, &rsp);
		printk("%s:%x %x %x\n", __func__,
			rsp.word1.Reg32, rsp.u.cmdResponse.word2, rsp.u.cmdResponse.word3);
	}
}
#endif

#if defined _BCM94908_
static void pmc_patch_4908(void)
{
	static const
# include "track.h"
	// relocate to end of pmc shared memory
	const unsigned linkaddr = (0xb6004800 - sizeof track_bin) & ~15;
	TCommand rsp;

	memcpy((void *)&PMC->sharedMem[(linkaddr & 0x7ff) / 4],
		track_bin, sizeof track_bin);

	// register command
	if (SendCommand(cmdRegisterCmdHandler, 0, 0, 0, 96, linkaddr, &rsp)
		|| rsp.word0.Bits.error)
		printk("%s:%d %d\n", __func__,
			rsp.word0.Bits.cmdID, rsp.word0.Bits.error);
	else {
		// check sec_chipvar and send command
		uint32 cap = ((uint32 *)JTAG_OTP_BASE)[12];
		uint32 slow, fast; // margin

		if ((cap & 15) != 0) {
			slow =  80, fast =  55;
		} else {
			slow = 100, fast = 100;
		}

		if (SendCommand(96, 0, 0, 0, slow, fast, &rsp)
			|| rsp.word0.Bits.error)
			printk("%s:%d %d %x %x %x\n", __func__,
				rsp.word0.Bits.cmdID, rsp.word0.Bits.error,
				rsp.word1.Reg32, rsp.u.cmdResponse.word2,
				rsp.u.cmdResponse.word3);
		else {
			SendCommand(cmdGetRMONandSigma, 0, 0, 0, 0, 0, &rsp);
			printk("%s:%d %d %x %x\n", __func__,
				rsp.word0.Bits.cmdID, rsp.word0.Bits.error,
				rsp.u.cmdResponse.word2, rsp.u.cmdResponse.word3);
		}
	}
}
#endif

#if defined(_BCM96856_) && defined(CFG_RAMAPP)
static void pmc_patch_6856(void)
{
    // Disable force bit on CLKRST BPCM to allow correct pinmuxing
    uint32_t target; 
 
    ReadBPCMRegister(PMB_ADDR_CHIP_CLKRST, CLKRSTBPCMRegOffset(clkrst_control), &target); 
    target &= 0xfffbffff;
    WriteBPCMRegister(PMB_ADDR_CHIP_CLKRST, CLKRSTBPCMRegOffset(clkrst_control), target);
}
#endif

#if defined(_BCM96846_) || defined(_BCM96856_) || defined(_BCM96858_)
#if defined(_BCM96846_) || defined(_BCM96856_)
#define SWR_CTRL ( (volatile unsigned int*) 0xffb20060)
#define SWR_WR   ( (volatile unsigned int*) 0xffb20064)
#define SWR_RD   ( (volatile unsigned int*) 0xffb20068)
#define SWR_FIRST 0
#define SWR_LAST 4
#elif defined(_BCM96858_)
#define SWR_CTRL ( (volatile unsigned int*) 0x80280060)
#define SWR_WR   ( (volatile unsigned int*) 0x80280064)
#define SWR_RD   ( (volatile unsigned int*) 0x80280068)
#define SWR_FIRST 1
#define SWR_LAST 3
#endif
#define SWR_READ_CMD_P 0xB800
#define SWR_WR_CMD_P   0xB400
#define SWR_EN         0x1000
#define SET_ADDR(ps, reg)  (((ps) << 5 | ((reg) & 0x1f)) & 0x2ff)  


#define TEST(x)  {\
                     int num;\
                     for(num=1000;(((*SWR_CTRL) & 0x8000) && (num > 0)) ; num--) ;\
                         if(!num) \
                         {\
                             printf("Error num %d timeout num = %d!!!", (x),  num);\
                         }\
}

void swrw(unsigned int ps, unsigned int reg, unsigned int val)
{
		unsigned int cmd  = 0; 
		unsigned int cmd1  = 0;  
        unsigned int reg0 = 0;

		*SWR_CTRL = SWR_EN;
		if(reg == 0)
		{
			/* no need read reg0 in case that we write to it , we know wal :)*/
			reg0 = val;
		}
		else
		{
			/* read reg0*/
			cmd1  = SWR_READ_CMD_P | SET_ADDR( ps, 0);
			*SWR_CTRL = cmd1;
			TEST(1)
			reg0 = *SWR_RD;
		}
		/* write reg */
		*SWR_WR = val;
		cmd  = SWR_WR_CMD_P | SET_ADDR( ps, reg);
		*SWR_CTRL = cmd;
		TEST(2);
		/*toggele bit 1 reg0 this load the new regs value */
		cmd1  = SWR_WR_CMD_P | SET_ADDR( ps, 0);
		*SWR_WR = reg0 & ~0x2;
		*SWR_CTRL = cmd1;
		TEST(3);
		*SWR_WR = reg0 | 0x2;
		*SWR_CTRL = cmd1;
		TEST(4);
		*SWR_WR = reg0 & ~0x2;
		*SWR_CTRL = cmd1;
		TEST(5);
}  

#if defined(_BCM96858_) && defined(CFG_RAMAPP)
static void pmc_patch_6858(void)
{
    // adjust SWREG
    swrw(1,3,0x5170);
    swrw(1,7,0x4829);
    swrw(2,3,0x5172);
    swrw(2,7,0x4829);
}
#endif

#if defined(CFG_RAMAPP)				
static void  swr_read( unsigned int ps, unsigned int reg)
{
		unsigned int cmd = SWR_READ_CMD_P | SET_ADDR( ps, reg);	
		
		*SWR_CTRL = SWR_EN;
		*SWR_CTRL = cmd;
		TEST(22);
		printf("%s, reg=0x%02x, val=0x%04x\n", 
				(ps == 0 ? "1.0D" : (ps == 1 ? "1.8 " :(ps == 2 ? "1.5 " : "1.0A"))),
				reg,
				*SWR_RD);
}	

static void dump_swregs(void)
{
    int i,j;
    printf("Dump Current setting of SWREGs\n");
    for(i=SWR_FIRST ; i < SWR_LAST ; i++)
        for(j=0; j <10; j++)
            swr_read(i,j);
}
#endif /* CFG_RAMAPP */
#endif /* defined(_BCM96846_) || defined(_BCM96856_) || defined(_BCM96858_) */

#if defined(PMC_RAM_BOOT)
void pmc_log_dump(void)
{
    unsigned long phys = cache_to_uncache(0x4ffc000);
    uint32_t i;

    if (*((uint32_t *)(phys - 4)) != 0xc0ffee55)
    {
        printk("Log is not enabled in PMC\n");
        return;
    }

    for (i = 0; i< 0x4000; i++)
    {
        char *ch = (char *)(phys + i);

        printk("%c", *ch);
    }
}
#endif

int pmc_init(void)
{
	int  rc = 0;

	pmc_initmode();

#if (defined (_BCM96846_) || defined(_BCM96856_) || defined(_BCM96858_)) && defined(CFG_RAMAPP)
#if defined(_BCM96858_) && defined(CFG_RAMAPP)
	pmc_patch_6858();
#endif
    dump_swregs();
#if defined(_BCM96856_)
    pmc_patch_6856();
#endif
#endif

#if defined(PMC_RAM_BOOT)
#if defined(AVS_DEBUG)
    pmc_log_dump();
#endif
    pmc_boot();
#endif

	printk("%s:PMC using %s mode\n", __func__,
		pmc_mode == PMC_MODE_PMB_DIRECT ? "PMB_DIRECT" : "DQM");
	if (pmc_mode == PMC_MODE_PMB_DIRECT)
		return 0;

# if defined CONFIG_BCM963138 && !defined _CFE_
	pmc_patch_63138();
# endif

#if defined _BCM94908_
	if (getAVSConfig() == 0) {
		pmc_patch_4908();
	}
	else
		printk("%s:AVS disabled\n", __func__);
#endif
	return rc;
}

void pmc_reset (void)
{
#if defined (_BCM963158_)
	// First, make sure PMC core is held in reset
	PMC->ctrl.softResets = 0x1;
	// Set PVTMON in non-AVS mode
	PMC->pvtmon[0].cfg_lo = PMC->pvtmon[0].cfg_lo & ~(0x7 << 10);
	PMC->pvtmon[1].cfg_lo = PMC->pvtmon[1].cfg_lo & ~(0x7 << 10);
	// PMC now in direct mode
	pmc_mode = PMC_MODE_PMB_DIRECT;
#endif
}

/* stub for 6848 */
#if defined(CONFIG_BCM96848)
int TuneRunner(void)
{
	return 0;
}
#endif

#ifndef _CFE_

#if defined CONFIG_BCM963381
#define logsize ilog2(8 << 10)
static const __attribute__ (( aligned(1 << logsize) ))
#include "restart_a42.h"

int __init pmc_app_init(void)
{
	/* too early to alloc; statically align the buffer */
	unsigned linkaddr = 0x9fc10000; // translated code start
	const void *physaddr = restart_bin;
	TCommand rsp;

	if (((PMC->ctrl.hostMboxIn >> 24) & 7) != kPMCRunStateAVSCompleteWaitingForImage) {
		/* pmc bootrom disabled? */
		return pmc_init();
	}

	/* window 2 baseout specifies destination of pmcapp image */
	/* e4k will translate the block at BaseIn to BaseOut */
	/* pmc ram command at linkaddr */
	PMC->ctrl.addr2WndwMask = -(1 << logsize);
	PMC->ctrl.addr2WndwBaseIn = linkaddr & 0x1fffffff;
	PMC->ctrl.addr2WndwBaseOut = virt_to_phys(physaddr);

	// register new command 96
	SendCommand(cmdRegisterCmdHandler, 0, 0, 0, 96, linkaddr, &rsp);
	if (rsp.word0.Bits.error)
		printk("%s %d\n", "cmdRegisterCmdHandler", rsp.word0.Bits.error);
	else
		// Command=96, TrackAVS=1, marginSlow=75, marginFast=75
		SendCommand(96, 1, 0, 0, 75, 75, &rsp);

	return pmc_init();
}
// must be before pmc_powerup in arch_init
postcore_initcall(pmc_app_init);
#endif

EXPORT_SYMBOL(ReadBPCMRegister);
EXPORT_SYMBOL(ReadZoneRegister);
EXPORT_SYMBOL(WriteBPCMRegister);
EXPORT_SYMBOL(WriteZoneRegister);
EXPORT_SYMBOL(ResetZone);
EXPORT_SYMBOL(ResetDevice);
EXPORT_SYMBOL(PowerOffZone);
EXPORT_SYMBOL(PowerOnZone);
EXPORT_SYMBOL(PowerOffDevice);
EXPORT_SYMBOL(PowerOnDevice);
EXPORT_SYMBOL(GetSelect0);
EXPORT_SYMBOL(GetSelect3);
EXPORT_SYMBOL(GetDevPresence);
EXPORT_SYMBOL(GetSWStrap);
EXPORT_SYMBOL(GetHWRev);
EXPORT_SYMBOL(GetAvsDisableState);
EXPORT_SYMBOL(GetNumZones);
EXPORT_SYMBOL(GetRevision);
EXPORT_SYMBOL(GetPVT);
EXPORT_SYMBOL(Ping);
EXPORT_SYMBOL(GetErrorLogEntry);
EXPORT_SYMBOL(SetClockHighGear);
EXPORT_SYMBOL(SetClockLowGear);
EXPORT_SYMBOL(SetClockGear);
EXPORT_SYMBOL(SetRunState);
EXPORT_SYMBOL(CloseAVS);
EXPORT_SYMBOL(SetPowerState);
EXPORT_SYMBOL(BootPmcNoRom);
#if defined(PMC_STALL_GPIO) || defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM96858) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM96836)
EXPORT_SYMBOL(StallPmc);
EXPORT_SYMBOL(UnstallPmc);
#endif
#if  defined(CONFIG_BCM94908)
EXPORT_SYMBOL(RecloseAVS);
#endif
#if !defined(CONFIG_BCM96836) && !defined(CONFIG_BCM96846) && !defined(CONFIG_BCM96856)
EXPORT_SYMBOL(GetRCalSetting);
#endif
#if defined(CONFIG_BCM96848)
EXPORT_SYMBOL(TuneRunner);
#endif
#if defined(CONFIG_BCM96856)
EXPORT_SYMBOL(GetAllROs);
#endif
#endif
