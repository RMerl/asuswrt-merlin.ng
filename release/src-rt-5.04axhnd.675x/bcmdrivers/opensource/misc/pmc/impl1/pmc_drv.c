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

/*****************************************************************************
 *  Description:
 *      Code for PMC Linux
 *****************************************************************************/

#include "pmc_drv.h"
#include "BPCM.h"
#include "command.h"
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/of_fdt.h>

#if IS_BCMCHIP(6858) || defined(PMC_IMPL_3_X)
#include "clk_rst.h"
#endif

int pmc_mode = PMC_MODE_DQM;

#ifdef PMC_LOG_IN_DTCM
void pmc_show_log_item(void)
{
	unsigned short i, item_sz;
	volatile char *dtcm = (char *) g_pmc->dtcm_base;  
	volatile Pmc *pmc = (volatile Pmc *)g_pmc->pmc_base;

	item_sz = pmc->ctrl.hostMboxOut >> 16;

	for (i = 0; i < item_sz; i++)
		printk("%c", dtcm[i]);  

	if (item_sz) // reset the item_sz to 0
		pmc->ctrl.hostMboxOut &= 0xffff;
}

void pmc_save_log_item(void)
{
	char *buf = (char *) phys_to_virt(CFG_BOOT_PMC_LOG_ADDR);
	unsigned short *plen = (unsigned short *) buf;
	char *dst = buf + sizeof(*plen) + *plen;
	volatile char *dtcm = (char *) g_pmc->dtcm_base; 
	volatile Pmc *pmc = (Pmc *)g_pmc->pmc_base;
	unsigned short item_sz = pmc->ctrl.hostMboxOut >> 16;
	int i = 0;
	
	while (i < item_sz && *plen < CFG_BOOT_PMC_LOG_SIZE - sizeof(*plen)) {
		dst[i] = dtcm[i];
		i++;
		(*plen)++;
	}

	if (item_sz) // reset the item_sz to 0
		pmc->ctrl.hostMboxOut &= 0xffff;
}
#endif // #ifdef PMC_LOG_IN_DTCM

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
    int early_call = 0;
	volatile Pmc *pmc;
#if defined(BOOT_MEMC_SRAM)
	reqdID = 1;
#endif
    if (!g_pmc)
    {
        /* FIXME: should be taken from device tree */
        early_call = 1;
        pmc = ioremap(EARLY_PMC_BASE, EARLY_PMC_SIZE);
    }
    else
    {
	    pmc = (volatile Pmc *)g_pmc->pmc_base;
    }

	pmc_spin_lock();

	/* clear previous rsp data if any */
	while (pmc->dqm.notEmptySts & PMC_DQM_RPL_STS) 
	{
		if (!rsp)
			rsp = &dummy;

		rsp->word0.Reg32 = pmc->dqmQData[PMC_DQM_RPL_NUM].word[0];
		rsp->word1.Reg32 = pmc->dqmQData[PMC_DQM_RPL_NUM].word[1];
		rsp->u.cmdGenericParams.params[0] =
		    pmc->dqmQData[PMC_DQM_RPL_NUM].word[2];
		rsp->u.cmdGenericParams.params[1] =
		    pmc->dqmQData[PMC_DQM_RPL_NUM].word[3];

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
		pmc->ctrl.hostMboxOut = 1; // request sync dtcm log
#endif

	cmd->word0.Bits.msgID = reqdID;

	/* send the command */
	pmc->dqmQData[PMC_DQM_REQ_NUM].word[0] = cmd->word0.Reg32;
	pmc->dqmQData[PMC_DQM_REQ_NUM].word[1] = cmd->word1.Reg32;
	pmc->dqmQData[PMC_DQM_REQ_NUM].word[2] =
	    cmd->u.cmdGenericParams.params[0];
	pmc->dqmQData[PMC_DQM_REQ_NUM].word[3] =
	    cmd->u.cmdGenericParams.params[1];

#ifdef CONFIG_BRCM_IKOS
	/* We do not enable PMC TIMER here for IKOS, or it will wait forever */
	while (!(pmc->dqm.notEmptySts & PMC_DQM_RPL_STS)) ;
#elif defined(PMC_IMPL_3_X)
#ifdef PMC_LOG_IN_DTCM
	if (cmd->word0.Bits.cmdID == cmdCloseAVS) 
	{
		while (!(pmc->dqm.notEmptySts & PMC_DQM_RPL_STS))
			pmc_show_log_item();

		pmc->ctrl.hostMboxOut = 0; // ignore dtcm log
	} 
	else
#endif // #ifdef PMC_LOG_IN_DTCM
	{
		pmc->ctrl.gpTmr0Ctl = ((1 << 31) | (1 << 29) |
				((400000 << 1) & 0x1fffffff));	// 400ms

		while (!(pmc->dqm.notEmptySts & PMC_DQM_RPL_STS) &&
				(pmc->ctrl.gpTmr0Ctl & (1 << 31))) ;
	}
#else
	pmc->ctrl.gpTmr2Ctl = ((1 << 31) | (1 << 29) | 400000);	// 400ms

	while (!(pmc->dqm.notEmptySts & PMC_DQM_RPL_STS) &&
	       (pmc->ctrl.gpTmr2Ctl & (1 << 31))) 
	{
#if (IS_BCMCHIP(63148) || IS_BCMCHIP(4908))
		/* Do not tight poll the PMC registers for longer command */
		if (cmd->word0.Bits.cmdID == cmdCloseAVS)
			udelay(1000);
#endif
	}
#endif /* CONFIG_BRCM_IKOS */

	if (pmc->dqm.notEmptySts & PMC_DQM_RPL_STS)
	{
		if (!rsp)
			rsp = &dummy;

		/* command didn't timeout, fill in the response */
		rsp->word0.Reg32 = pmc->dqmQData[PMC_DQM_RPL_NUM].word[0];
		rsp->word1.Reg32 = pmc->dqmQData[PMC_DQM_RPL_NUM].word[1];
		rsp->u.cmdGenericParams.params[0] =
		    pmc->dqmQData[PMC_DQM_RPL_NUM].word[2];
		rsp->u.cmdGenericParams.params[1] =
		    pmc->dqmQData[PMC_DQM_RPL_NUM].word[3];

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

    if (early_call)
        iounmap(pmc);
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

int GetRevision(uint32_t * change, uint32_t * revision)
{
	if (pmc_mode == PMC_MODE_DQM) {
		TCommand rsp;
		int status = SendCommand(cmdRevision, 0, 0, 0, 0, 0, &rsp);

		if (status == kPMC_NO_ERROR) {
			*change = rsp.u.cmdResponse.word2;
			*revision = rsp.u.cmdResponse.word3;
		}

		return status;
	} else
		return kPMC_INVALID_COMMAND;
}
EXPORT_SYMBOL(GetRevision);

#if defined(PMC_IMPL_3_X) || defined(PMC_ON_HOSTCPU)

static int read_bpcm_reg_direct_keyhole(int devAddr, int wordOffset, int keyhole_idx, uint32_t * value)
{
	int status = kPMC_NO_ERROR;
	int bus = (devAddr >> PMB_BUS_ID_SHIFT) & 0x3;
	uint32_t address, ctlSts;
	volatile PmbBus *pmb;
	volatile keyholeReg *keyhole;

#if defined(PMC_ON_HOSTCPU)
	pmb = &((volatile Pmc *)(g_pmc->pmc_base))->pmb;
#else
	pmb = &((volatile Procmon *)(g_pmc->procmon_base))->pmb;
#endif
	keyhole = &pmb->keyhole[keyhole_idx];

	address =
	    ((devAddr & 0xff) * ((pmb->config >> PMB_NUM_REGS_SHIFT) & PMB_NUM_REGS_MASK)) |
        (wordOffset);

	keyhole->control =
	    PMC_PMBM_START | (bus << PMC_PMBM_BUS_SHIFT) | (PMC_PMBM_Read) | address;
	ctlSts = keyhole->control;
	while (ctlSts & PMC_PMBM_BUSY)
		ctlSts = keyhole->control;	/*wait for completion */

	if (ctlSts & PMC_PMBM_TIMEOUT)
		status = kPMC_COMMAND_TIMEOUT;
	else
		*value = keyhole->rd_data;

	return status;
}

static int write_bpcm_reg_direct_keyhole(int devAddr, int wordOffset, int keyhole_idx, uint32_t value)
{
	int status = kPMC_NO_ERROR;
	int bus = (devAddr >> PMB_BUS_ID_SHIFT) & 0x3;
	uint32_t address, ctlSts;
	volatile PmbBus *pmb;
	volatile keyholeReg *keyhole;

#if defined(PMC_ON_HOSTCPU)
	pmb = &((volatile Pmc *)(g_pmc->pmc_base))->pmb;
#else
	pmb = &((volatile Procmon *)(g_pmc->procmon_base))->pmb;
#endif
	keyhole = &pmb->keyhole[keyhole_idx];

	address =
	    ((devAddr & 0xff) * ((pmb->config >> PMB_NUM_REGS_SHIFT) & PMB_NUM_REGS_MASK)) |
	    (wordOffset);
	keyhole->wr_data = value;
	keyhole->control =
	    PMC_PMBM_START | (bus << PMC_PMBM_BUS_SHIFT) | (PMC_PMBM_Write) | address;

	ctlSts = keyhole->control;
	while (ctlSts & PMC_PMBM_BUSY)
		ctlSts = keyhole->control;	/*wait for completion */

	if (ctlSts & PMC_PMBM_TIMEOUT)
		status = kPMC_COMMAND_TIMEOUT;

	return status;
}

int read_bpcm_reg_direct(int devAddr, int wordOffset, uint32_t * value)
{
    return read_bpcm_reg_direct_keyhole(devAddr, wordOffset, 0, value);
}

int read_bpcm_reg_direct_internal(int devAddr, int wordOffset, uint32_t * value)
{
    return read_bpcm_reg_direct_keyhole(devAddr, wordOffset, 1, value);
}

int write_bpcm_reg_direct(int devAddr, int wordOffset, uint32_t value)
{
    return write_bpcm_reg_direct_keyhole(devAddr, wordOffset, 0, value);
}

int write_bpcm_reg_direct_internal(int devAddr, int wordOffset, uint32_t value)
{
    return write_bpcm_reg_direct_keyhole(devAddr, wordOffset, 1, value);
}

static int is_pvtmon_enabled = 0;
static void pvtmon_enable(void)
{
	uint32_t index;
	uint32_t target;
#define PVTCLKDIV (5 << 8)

	pmc_spin_lock();

	// set up analog hardware to enable counting
	target = PVTCLKDIV | 4;	// 4 = clk_en|!pwr_dn|rstb
	write_bpcm_reg_direct(PMB_ADDR_PVTMON, 17, target);
	for (index = 0; index < 100000; index++) ;
	target = PVTCLKDIV | 5;	// 5 = clk_en|!pwr_dn|!rstb
	write_bpcm_reg_direct(PMB_ADDR_PVTMON, 17, target);

	// set sample size of ALL counters except TEST (7)
	// set enable bit for ONLY 0 (temperature and external) and V1p0<0> - these will be the only ones we use during Match and Closure
	for (index = 0; index < 8; index++) {
		target = (0x5 << 24) | 0x80000000;	//pvtmon samples (32 [2^5]) + enable
		write_bpcm_reg_direct(PMB_ADDR_PVTMON, 24 + index, target);
		read_bpcm_reg_direct(PMB_ADDR_PVTMON, 24 + index, &target);	// read once to clear valid bit
	}

	// enable accumulation:
	// 0x00000801 = skip_len = 8, enable accumulation
	target = 0x00000801;
	write_bpcm_reg_direct(PMB_ADDR_PVTMON, 20, target);
	is_pvtmon_enabled = 1;

	pmc_spin_unlock();
}

#ifndef unlikely
#define unlikely(x)	(x)
#endif
static int read_pvt_direct_keyhole(int index, int kh_idx, int *val)
{
	int status;
	uint32_t target;

	// assuming PVTMON already enabled in DQM mode
	if (unlikely((pmc_mode != PMC_MODE_DQM) && !is_pvtmon_enabled))
		pvtmon_enable();

	if (kh_idx == 0)
		pmc_spin_lock();
	status = read_bpcm_reg_direct_keyhole(PMB_ADDR_PVTMON, 24 + index, kh_idx, &target);
	if (unlikely(status))
		goto EXIT;

	while (!(target & (1 << 18))) {
		// the value SHOULD be valid immediatly, but just in case...
		status =
		    read_bpcm_reg_direct_keyhole(PMB_ADDR_PVTMON, 24 + index, kh_idx, &target);
		if (unlikely(status))
			goto EXIT;
	}

	*val = target & 0x3ff;

EXIT:
	if (kh_idx == 0)
		pmc_spin_unlock();
	return status;
}

int GetPVT(int sel, int island, int *value)
{
	return read_pvt_direct_keyhole(sel, 0, value);
}

int GetPVTKH2(int sel, int island, int *value)
{
	return read_pvt_direct_keyhole(sel, 2, value);
}
#else // #if defined(PMC_IMPL_3_X) || defined(PMC_ON_HOSTCPU)
int read_bpcm_reg_direct(int devAddr, int wordOffset, uint32_t * value)
{
	int status = kPMC_NO_ERROR;
	int bus = (devAddr >> PMB_BUS_ID_SHIFT) & 0x3;
	Procmon *procmon;
	volatile PMBMaster *pmbm_ptr;
	int early_call = 0;

	if (bus >= PMB_BUS_MAX)
		return kPMC_INVALID_BUS;

	if (!g_pmc)
	{
		early_call = 1;
		procmon = ioremap(EARLY_PROCMON_BASE, EARLY_PROCMON_SIZE);
	}
	else
	{
		procmon = (Procmon *)g_pmc->procmon_base;
	}

	pmbm_ptr = &(procmon->PMBM[bus]);
	/* Make sure PMBM is not busy */

	pmbm_ptr->ctrl = PMC_PMBM_START | PMC_PMBM_Read |
	    ((devAddr & 0xff) << 12) | wordOffset;

	while (pmbm_ptr->ctrl & PMC_PMBM_START) ;

	if (pmbm_ptr->ctrl & PMC_PMBM_TIMEOUT)
		status = kPMC_COMMAND_TIMEOUT;
	else
		*value = pmbm_ptr->rd_data;

	if (early_call)
		iounmap(procmon);
	return status;
}

int write_bpcm_reg_direct(int devAddr, int wordOffset, uint32_t value)
{
	int bus = (devAddr >> PMB_BUS_ID_SHIFT) & 0x3;
	int status = kPMC_NO_ERROR;
	Procmon *procmon;
	volatile PMBMaster *pmbm_ptr;
	int early_call = 0;

	if (bus >= PMB_BUS_MAX)
		return kPMC_INVALID_BUS;

	if (!g_pmc)
	{
		early_call = 1;
		procmon = ioremap(EARLY_PROCMON_BASE, EARLY_PROCMON_SIZE);
	}
	else
	{
		procmon = (Procmon *)g_pmc->procmon_base;
	}

	pmbm_ptr = &(procmon->PMBM[bus]);

	pmbm_ptr->wr_data = value;
	pmbm_ptr->ctrl = PMC_PMBM_START | PMC_PMBM_Write |
	    ((devAddr & 0xff) << 12) | wordOffset;

	while (pmbm_ptr->ctrl & PMC_PMBM_START) ;

	if (pmbm_ptr->ctrl & PMC_PMBM_TIMEOUT)
		status = kPMC_COMMAND_TIMEOUT;

	if (early_call)
		iounmap(procmon);

	return status;
}

int GetPVT(int sel, int island, int *value)
{
	if (pmc_mode == PMC_MODE_DQM) {
		TCommand rsp;
		int status = SendCommand(cmdGetPVT, 0, 0, island, sel, 0, &rsp);

		if (status == kPMC_NO_ERROR)
			*value = rsp.u.cmdResponse.word2;
		return status;
	} else
		return kPMC_INVALID_COMMAND;
}

int GetPVTKH2(int sel, int island, int *value)
{
	return GetPVT(sel, island, value);
}
#endif // #if defined(PMC_IMPL_3_X) || defined(PMC_ON_HOSTCPU)
EXPORT_SYMBOL(GetPVT);
EXPORT_SYMBOL(GetPVTKH2);

/* GetRCalSetting reads resistor value and calculates the calibration setting for the SGMII, PCIe, SATA
   and USB HW blocks that requires resistor calibration to meet specification requirement.
   The HW driver should call this function and write the value to calibration register during initialzation

   input param: resistor - the resistor type that specific HW calibration care about:
   inout param: rcal - 4 bit RCAL value [0 -15] representing the increment or decrement to the internal resistor.   
   return: kPMC_NO_ERROR or kPMC_INVALID_COMMAND if error condition
*/
int GetRCalSetting(int resistor, int *rcal)
{
#if defined(PMC_GETRCAL_SUPPORT) 
	int res_int, res_ext, ratio, ratio1;
	int rc = kPMC_NO_ERROR;

	if (pmc_mode == PMC_MODE_DQM) {
#if IS_BCMCHIP(63178) || IS_BCMCHIP(47622) || IS_BCMCHIP(6756)
		TCommand rsp;
		int status;

		/* make sure the resistor selection is valid */
		/* Not supporting the other resistors because there is no room in DQM RSP */
		if (resistor < RCAL_1UM_HORZ || resistor > RCAL_1UM_VERT) {
			pr_err("%s: Error in SW RCAL -- Resistor selection %d is invalid\n",
					__func__, resistor);
			return kPMC_INVALID_PARAM;
		}

		status = SendCommand(cmdGetRMON, 0, 0, 0, 0, 0, &rsp);
		if (status != kPMC_NO_ERROR) {
			pr_err("%s: Error in SW RCAL -- Sending cmdGetRMON failed with status %d\n",
					__func__, status);
			return status;
		}

		/* make sure the resistor data is collected by PMC */
		if (!(rsp.u.cmdResponse.word3 & (1 << 16))) {
			pr_err("%s: Error in SW RCAL -- Response of cmdGetRMON is invalid\n",
					__func__);
			return kPMC_INVALID_STATE;
		}

		res_int = rsp.u.cmdResponse.word2;
		res_ext = rsp.u.cmdResponse.word3 & 0xffff;
#else
		Procmon *procmon = (Procmon *)g_pmc->procmon_base;
		/* make sure the resistor selection is valid */
		if (resistor < RCAL_0P25UM_HORZ || resistor > RCAL_1UM_VERT) {
			pr_err("%s: Error in SW RCAL -- Resistor selection %d is invalid\n",
					__func__, resistor);
			return kPMC_INVALID_PARAM;
		}

		/* make sure the resistor data is collected by PMC */
		if ((procmon->Misc.
		     misc[PMMISC_RMON_EXT_REG] & PMMISC_RMON_VALID_MASK) == 0) {
			pr_err("%s: Error in SW RCAL -- PMMISC_RMON is invalid\n", __func__);
			return kPMC_INVALID_STATE;
		}

		res_int = procmon->Misc.misc[resistor >> 1];
		res_ext = (procmon->Misc.misc[PMMISC_RMON_EXT_REG]) & 0xffff;
#endif
		if (resistor % 2)
			res_int >>= 16;
		res_int &= 0xffff;

		/* Return error if the res_ext saturated such as
		   the ext resistor is not available */
		if (res_ext > 0x3a0) {
			pr_err("%s: Error in SW RCAL -- res_ext value 0x%x is saturated\n",
					__func__, res_ext);
			return kPMC_INVALID_STATE;
		}

		/* Ratio = CLAMP((INT) (128.0 * V(REXT)/V(RINT)), 0, 255) */
		ratio = (128 * res_ext) / res_int;
		if (ratio > 255)
			ratio = 255;

		/* Ratio1 = CLAMP(128 - (Ratio - 128) * 4, 0, 255) */
		ratio1 = (128 - (ratio - 128) * 4);
		if (ratio1 < 0)
			ratio1 = 0;
		if (ratio1 > 255)
			ratio1 = 255;

		/* convert to 4 bit rcal setting value */
		*rcal = (ratio1 >> 4) & 0xf;
		printk("getrcal for res select %d, int %d, ext %d, ratio %d ratio1 %d, rcal %d\n",
		     resistor, res_int, res_ext, ratio, ratio1, *rcal);
	} else {
		/* not supported if PMC is not running for now. To support that, need to copy the PMC rom 
		   code to read out resistor value manually */
		pr_err("%s: Error in SW RCAL -- PMC is not running in DQM mode\n", __func__);
		rc = kPMC_INVALID_STATE;
	}

	return rc;
#else
    printk("%s: Skipping SW RCAL as PMC_GETRCAL_SUPPORT is not defined\n", __func__);
    return kPMC_INVALID_COMMAND;
#endif
}

EXPORT_SYMBOL(GetRCalSetting);

int GetRCalSetting_1UM_VERT(int *rcal)
{
#ifdef RCAL_1UM_VERT
    return GetRCalSetting(RCAL_1UM_VERT, rcal);
#else
    printk("%s: Skipping SW RCAL as RCAL_1UM_VERT is not defined\n", __func__);
    return kPMC_INVALID_COMMAND;
#endif
}
EXPORT_SYMBOL(GetRCalSetting_1UM_VERT);

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

EXPORT_SYMBOL(ReadBPCMRegister);

int ReadZoneRegister(int devAddr, int zone, int wordOffset, uint32_t * value)
{
	int status = kPMC_INVALID_STATE;

	if ((unsigned)wordOffset >= 4)
		return kPMC_INVALID_PARAM;

	if (PMC_ACCESS_BPCM_DIRECT || pmc_mode == PMC_MODE_PMB_DIRECT)
		return ReadBPCMRegister(devAddr,
					BPCMRegOffset(zones[zone].control) +
					wordOffset, value);

	if (pmc_mode == PMC_MODE_DQM) {
		TCommand rsp;

		status =
		    SendCommand(cmdReadZoneReg, devAddr, zone, 0, wordOffset, 0,
				&rsp);
		if (status == kPMC_NO_ERROR)
			*value = rsp.u.cmdResponse.word2;
	}

	return status;
}

EXPORT_SYMBOL(ReadZoneRegister);

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

EXPORT_SYMBOL(WriteBPCMRegister);

int WriteZoneRegister(int devAddr, int zone, int wordOffset, uint32_t value)
{
	if ((unsigned)wordOffset >= 4)
		return kPMC_INVALID_PARAM;

	if (PMC_ACCESS_BPCM_DIRECT || pmc_mode == PMC_MODE_PMB_DIRECT)
		return WriteBPCMRegister(devAddr,
					 BPCMRegOffset(zones[zone].control) +
					 wordOffset, value);

	if (pmc_mode == PMC_MODE_DQM)
		return SendCommand(cmdWriteZoneReg, devAddr, zone, 0,
				   wordOffset, value, 0);

	return kPMC_INVALID_STATE;
}

EXPORT_SYMBOL(WriteZoneRegister);

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

EXPORT_SYMBOL(PowerOnDevice);

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

EXPORT_SYMBOL(PowerOffDevice);

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

EXPORT_SYMBOL(PowerOnZone);

int PowerOffZone(int devAddr, int zone)
{
	BPCM_PWR_ZONE_N_CONTROL reg;
	int status;

#if IS_BCMCHIP(6858)
	/* Do not use DQM command cmdPowerZoneOnOff for non 6858 because this command is only available if a
	   PMC application has been uploaded to expand the PMC boot rom functionality */
	if (pmc_mode == PMC_MODE_DQM) {
		TCommand cmd = {0};
		cmd.u.cmdStateOnlyParam.state = 0;
		return SendCmd(&cmd, cmdPowerZoneOnOff, devAddr, zone, 0, 0);
	}
#endif

	status =
	    ReadBPCMRegister(devAddr, BPCMRegOffset(zones[zone].control),
			     &reg.Reg32);
	if (status == kPMC_NO_ERROR) {
		reg.Bits.pwr_dn_req = 1;
		reg.Bits.pwr_up_req = 0;
		status =
		    WriteBPCMRegister(devAddr,
				      BPCMRegOffset(zones[zone].control),
				      reg.Reg32);
	}
	return status;
}

EXPORT_SYMBOL(PowerOffZone);

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

EXPORT_SYMBOL(ResetDevice);

int ResetZone(int devAddr, int zone)
{
	BPCM_PWR_ZONE_N_CONTROL reg;
	int status;

#if IS_BCMCHIP(6858)
	/* Do not use DQM command cmdResetZone for non 6858 because this command is only available if a
	   PMC application has been uploaded to expand the PMC boot rom functionality */
	if (pmc_mode == PMC_MODE_DQM)
		return SendCommand(cmdResetZone, devAddr, zone, 0, 0, 0, 0);
#endif

	status = PowerOffZone(devAddr, zone);
	do {
		status =
		    ReadBPCMRegister(devAddr,
				     BPCMRegOffset(zones[zone].control),
				     &reg.Reg32);
	} while ((reg.Bits.pwr_off_state != 1) && (status == kPMC_NO_ERROR));
	if (status == kPMC_NO_ERROR)
		status = PowerOnZone(devAddr, zone);
	return status;
}

EXPORT_SYMBOL(ResetZone);

/* close AVS with margin slow, fast, max, min (mV) */
int CloseAVS(int island, unsigned short margin_mv_slow,
	     unsigned short margin_mv_fast, unsigned short maximum_mv,
	     unsigned short minimum_mv)
{
	if (pmc_mode == PMC_MODE_DQM) {
		TCommand rsp;
		int status;

#if defined(PMC_IMPL_3_X) || IS_BCMCHIP(63158)
		TCommandCloseAVS ca;

		if (minimum_mv && maximum_mv && (minimum_mv > maximum_mv))
			return kPMC_INVALID_PARAM;

		ca.margin_mv_slow = margin_mv_slow;
		ca.margin_mv_fast = margin_mv_fast;
		ca.maximum_mv = maximum_mv;
		ca.minimum_mv = minimum_mv;

		status = SendCommand(cmdCloseAVS, 0, 0, island,
				     ca.word2, ca.word3, &rsp);
#else
		status = SendCommand(cmdCloseAVS, 0, 0, island,
				     margin_mv_slow, margin_mv_fast, &rsp);
#endif

		return status;
	} else
		return kPMC_INVALID_COMMAND;
}

EXPORT_SYMBOL(CloseAVS);

/* new pmc firmware implements stall command */
/* state indicated by stalled bit in run status */

#define PMC_STALLED (1 << 30)

/* return value doesn't appear to be used */
int StallPmc(void)
{
#if defined(PMC_STALL_SUPPORT) 
	TCommand rsp;
	volatile Pmc *pmc = (volatile Pmc *)g_pmc->pmc_base;
	volatile uint32_t *strap = (volatile uint32_t *)g_pmc->strap;

	/* ignore if pmc not booted from flash or already stalled */
	if (pmc_mode == PMC_MODE_PMB_DIRECT ||
#if defined MISC_STRAP_BUS_PMC_BOOT_FLASH
		(*strap & MISC_STRAP_BUS_PMC_BOOT_FLASH) == 0 ||
#else
		(*strap & MISC_STRAP_BUS_PMC_BOOT_FLASH_N) != 0 ||
#endif
		pmc->ctrl.hostMboxIn & PMC_STALLED)
	{
		return 0;
	}
	/* return non-zero if stall command fails */
	return SendCommand(cmdStall, 0, 0, 0, 0, 0, &rsp);
#else
    return kPMC_INVALID_COMMAND;
#endif
}

EXPORT_SYMBOL(StallPmc);

/* return value doesn't appear to be used */
int UnstallPmc(void)
{
#if defined(PMC_STALL_SUPPORT) 
	volatile Pmc *pmc = (volatile Pmc *)g_pmc->pmc_base;
	volatile uint32_t *strap = (volatile uint32_t *)g_pmc->strap;

	/* clear stalled bit if pmc booted from flash */
	if ((pmc_mode != PMC_MODE_PMB_DIRECT) &&
#if defined MISC_STRAP_BUS_PMC_BOOT_FLASH
		((*strap & MISC_STRAP_BUS_PMC_BOOT_FLASH) != 0))
#else
		((*strap & MISC_STRAP_BUS_PMC_BOOT_FLASH_N) == 0))
#endif
	{
		pmc->ctrl.hostMboxIn &= ~PMC_STALLED;
	}
#endif
	return 0;
}

EXPORT_SYMBOL(UnstallPmc);

// initalize pmc_mode (possibly) before printk available
void pmc_initmode(void)
{

	pmc_mode = PMC_MODE_PMB_DIRECT;

#if defined(PMC_IMPL_1_X)
	#if defined MISC_STRAP_BUS_PMC_ROM_BOOT
	{
		volatile uint32_t *strap = (volatile uint32_t *)g_pmc->strap;
		if ((*strap & MISC_STRAP_BUS_PMC_ROM_BOOT) !=0) 
			pmc_mode = PMC_MODE_DQM;
	}
	#else 
	{
		volatile Pmc *pmc = (volatile Pmc *)g_pmc->pmc_base;
		/* MIPS based PMC */
		if (pmc->ctrl.softResets == 0)
			pmc_mode = PMC_MODE_DQM;
	}
	#endif // #if defined MISC_STRAP_BUS_PMC_ROM_BOOT
#elif defined(PMC_IMPL_3_X)
	{
		volatile MaestroMisc *maestro = (volatile MaestroMisc *)g_pmc->maestro_base;

		/* Maestro based PMC */
		if (maestro->coreCtrl.coreEnable == 1)
			pmc_mode = PMC_MODE_DQM;
	}
#endif // #if defined PMC_IMPL_1_X
}

void pmc_reset(void)
{
#if IS_BCMCHIP(63158)
	volatile Pmc *pmc = (volatile Pmc *)g_pmc->pmc_base;
	// First, make sure PMC core is held in reset
	pmc->ctrl.softResets = 0x1;
	// Set PVTMON in non-AVS mode
	pmc->pvtmon[0].cfg_lo = pmc->pvtmon[0].cfg_lo & ~(0x7 << 10);
	pmc->pvtmon[1].cfg_lo = pmc->pvtmon[1].cfg_lo & ~(0x7 << 10);
	// PMC now in direct mode
	pmc_mode = PMC_MODE_PMB_DIRECT;
#endif
}

int pmc_convert_pvtmon(int sel, int value)
{
#if IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813)
	switch (sel) {
	case kTEMPERATURE:	// convert value to milli-degree Celsius
		return (45000000 - 54956 * value) / 100;
	case kV_0p85_0:		// convert value to milli-voltage
	case kV_0p85_1:
	case kV_VIN:
	case kV_1p00_1:
		return 9442 * value / (8 * 1024);
	case kV_1p80:
		return 9442 * value / (4 * 1024);
	case kV_3p30:
		return 9442 * value / (2 * 1024);
	case kTEST:
		return 9442 * value / 1024;
	}
#else // #if IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813)
	switch (sel) {
	case kTEMPERATURE:	// convert value to milli-degree Celsius
#if IS_BCMCHIP(63148)		// pvt2
		return (38887551 - 466415 * value / 10) / 100;
#elif defined(PMC_IMPL_3_X) || IS_BCMCHIP(6878) || IS_BCMCHIP(6855)
		return (41335000 - 49055 * value) / 100;
#else
		return (41004000 - 48705 * value) / 100;
#endif
	case kV_0p85_0:	// convert value to milli-voltage
	case kV_0p85_1:
		return 880 * value * 10 / (10 * 1024);
	case kV_VIN:
	case kV_1p00_1:
		return 880 * value * 10 / (7 * 1024);
	case kV_1p80:
		return 880 * value * 10 / (4 * 1024);
	case kV_3p30:
		return 880 * value * 10 / (2 * 1024);
	case kTEST:
		return 880 * value / 1024;
	}
#endif // #if IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813)

	return -1;
}

EXPORT_SYMBOL(pmc_convert_pvtmon);

int pmc_get_tracktemp(int *status)
{
	TCommand rsp;
	int ret;

	if (pmc_mode != PMC_MODE_DQM)
		return kPMC_INVALID_COMMAND;

	ret = SendCommand(cmdGetTrackTemp, 0, 0, 0, 0, 0, &rsp);
	if (ret == kPMC_NO_ERROR)
		*status = ! !rsp.u.cmdResponse.word2;

	return ret;
}

int pmc_set_tracktemp(int enable)
{
	if (pmc_mode != PMC_MODE_DQM)
		return kPMC_INVALID_COMMAND;

	return SendCommand(cmdSetTrackTemp, 0, 0, 0, ! !enable, 0, NULL);
}

